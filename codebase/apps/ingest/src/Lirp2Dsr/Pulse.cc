// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
// Pulse.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2003
//
////////////////////////////////////////////////////////////////

#include <iomanip>
#include <iostream>
#include <fstream>
#include <cerrno>
#include <cmath>
#include <toolsa/DateTime.hh>
#include <toolsa/toolsa_macros.h>
#include "Pulse.hh"
using namespace std;

// Constructor

Pulse::Pulse(const Params &params, bool highSnrPack) :
  _nClients(0),
  _params(params),
  _highSnrPack(highSnrPack)
  
{
  memset(&_rocHdr, 0, sizeof(_rocHdr));
  memset(&_rvp8Hdr, 0, sizeof(_rvp8Hdr));
  memset(_iq, 0, sizeof(_iq));
}

// destructor

Pulse::~Pulse()

{

}

///////////////////////////////////////////////////////////
// read pulse data
//
// Returns 0 on success, -1 on failure

int Pulse::read(FILE *in, bool hasRocHdr)

{

  // read in headers
  
  if (_readHeaders(in, hasRocHdr)) {
    if (_params.debug && !feof(in)) {
      cerr << "ERROR - Pulse::read()" << endl;
      cerr << "  Cannot read in pulse header" << endl;
    }
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    printRvp8Hdr(cerr);
  }
  
  if (_rvp8Hdr.iFlags != PHDRFLG_VALID) {
    if (_params.debug) {
      cerr << "ERROR - Pulse::read()" << endl;
      cerr << "  Invalid rvp8Hdr" << endl;
    }
    return -1;
  }
    
  _time  = (double) _rvp8Hdr.iTimeUTC + _rvp8Hdr.iMSecUTC / 1000.0;
  if (_rvp8Hdr.iPrevPRT == 0) {
    return -1;
  }
  _prt = _rvp8Hdr.iPrevPRT / (_params.ifd_sampling_freq * 1.0e6);
    
  _el = (_rvp8Hdr.iEl / 65536.0) * 360.0;
  _az = (_rvp8Hdr.iAz / 65536.0) * 360.0;
  _phaseDiff0 =  (_rvp8Hdr.Rx[0].iBurstArg / 65536.0) * 360.0;
  
  // read in iq data
  
  _nGates = (int) _rvp8Hdr.iNumVecs;
  if (_nGates > MAXBINS) {
    _nGates = MAXBINS;
  }
  int nIQ = _nGates * 2;

  _seqNum = _rvp8Hdr.iSeqNum;
    
  bool isPacked = false;
  if (!hasRocHdr || _rocHdr.packedData) {
    isPacked = true;
  }

  if (isPacked) {
    
    // read packed
    
    UINT2 packed[nIQ];
    if ((int) fread(packed, sizeof(UINT2), nIQ, in) != nIQ) {
      int errNum = errno;
      cerr << "ERROR - Pulse::read()" << endl;
      cerr << "  Cannot read packed data, nIQ: " << nIQ << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
    
    // convert packed data to floats
    _vecFloatIQFromPackIQ(_iq, packed, nIQ, (UINT1) _highSnrPack);
    
  } else {
    
    // read unpacked iq
    
    if ((int) fread(_iq, sizeof(FLT4), nIQ, in) != nIQ) {
      int errNum = errno;
      cerr << "ERROR - Pulse::read()" << endl;
      cerr << "  Cannot read unpacked data, nIQ: " << nIQ << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
    
  }
  
  return 0;

}
  
/////////////////////////////////////////////////////////////////
// Compute phase differences between this pulse and previous ones
// to be able to cohere to multiple trips
//
// Before this method is called, this pulse should be added to
// the queue.

int Pulse::computePhaseDiffs(const deque<Pulse *> &pulseQueue,
			     int maxTrips)
  
{
  
  if (pulseQueue[0] != this) {
    cerr << "ERROR - Pulse::computePhaseDiffs()" << endl;
    cerr << "  You must add this pulse before calling this function" << endl;
    return -1;
  }

  _phaseDiffs.clear();
  
  // phase diffs for maxTrips previous beams
  
  int qSize = (int) pulseQueue.size();
  
  for (int ii = 0; ii < maxTrips; ii++) { // ii is (trip - 1)
    if (ii == 0) {
      _phaseDiffs.push_back(0.0);
    } else {
      if (ii <= qSize) {
	double sum = _phaseDiffs[ii-1] + pulseQueue[ii-1]->getPhaseDiff0();
	while (sum > 360.0) {
	  sum -= 360.0;
	}
	_phaseDiffs.push_back(sum);
      } else {
	// actual phase diff not available
	// use previous trip's value
	_phaseDiffs.push_back(_phaseDiffs[ii-1]);
      }
    }
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "----> _seqNum: " << _seqNum << endl;
    cerr << "----> _time: " << DateTime::strn((time_t)_time) << endl;
    cerr << "----> _az: " << _az << endl;
    cerr << "----> _phaseDiff0: " << _phaseDiff0 << endl;
    cerr << "----> _phaseDiffs:" << endl;
    for (size_t ii = 0; ii < _phaseDiffs.size(); ii++) {
      cerr << "------> Trip " << ii+1
	   << " phase diff: " << _phaseDiffs[ii] << endl;
    }
  }
  
#ifdef CHECK_PHASE_DIFF
  vector<Complex_t> burstIQ;
  for (int ii = 0; ii < maxTrips; ii++) {
    Complex_t iq;
    if (ii < qSize) {
      const FLT4* fiq = pulseQueue[ii]->getIq();
      iq.re = fiq[0];
      iq.im = fiq[1];
    } else {
      iq.re = 1.0;
      iq.im = 0.0;
    }
    burstIQ.push_back(iq);
  }

  for (int ii = 0; ii < (int) burstIQ.size(); ii++) {
    Complex_t result;
    double angle;
    _subtract(burstIQ[ii], burstIQ[0], result, angle);
    if (angle < 0) {
      angle += 360.0;
    }
    if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
      cerr << "--------> check, trip: " << ii+1
	   << " , angle: " << angle << endl;
    }
  }
#endif
  
  return 0;

}

/////////////////////////////////////
// print

void Pulse::printRocHdr(ostream &out) const
{

  out << "==== ROC Pulse Header ===" << endl;
  out << "  id: " << hex << _rocHdr.id << dec << endl;
  out << "  recordSize: " << _rocHdr.recordSize << endl;
  out << "  version: " << _rocHdr.version << endl;
  out << "  packedData: " << _rocHdr.packedData << endl;
  out << "  startGate: " << _rocHdr.startGate << endl;
  out << "  endGate: " << _rocHdr.endGate << endl;

}

void Pulse::printRvp8Hdr(ostream &out) const
{
  
  out << "==================== Pulse header ==================" << endl;
  out << "  iVersion: " << (int) _rvp8Hdr.iVersion << endl;
  out << "  iFlags: " << (int) _rvp8Hdr.iFlags << endl;
  out << "  iMSecUTC: " << (int) _rvp8Hdr.iMSecUTC << endl;
  out << "  iTimeUTC: " << (int) _rvp8Hdr.iTimeUTC << endl;
  out << "  iBtime: " << (int) _rvp8Hdr.iBtime << endl;
  out << "  iSysTime: " << (int) _rvp8Hdr.iSysTime << endl;
  out << "  iPrevPRT: " << (int) _rvp8Hdr.iPrevPRT << endl;
  out << "  iNextPRT: " << (int) _rvp8Hdr.iNextPRT << endl;
  out << "  iSeqNum: " << (int) _rvp8Hdr.iSeqNum << endl;
  out << "  iAqMode: " << (int) _rvp8Hdr.iAqMode << endl;
  out << "  iAz: " << (int) _rvp8Hdr.iAz << endl;
  out << "  iEl: " << (int) _rvp8Hdr.iEl << endl;
  out << "  iNumVecs: " << _rvp8Hdr.iNumVecs << endl;
  out << "  iMaxVecs: " << _rvp8Hdr.iMaxVecs << endl;
  out << "  iVIQPerBin: " << (int) _rvp8Hdr.iVIQPerBin << endl;
  out << "  iTgBank: " << (int) _rvp8Hdr.iTgBank << endl;
  out << "  iTgWave: " << (int) _rvp8Hdr.iTgWave << endl;
  out << "  uiqPerm.iLong[0]: " << (int) _rvp8Hdr.uiqPerm.iLong[0] << endl;
  out << "  uiqPerm.iLong[1]: " << (int) _rvp8Hdr.uiqPerm.iLong[1] << endl;
  out << "  uiqOnce.iLong[0]: " << (int) _rvp8Hdr.uiqOnce.iLong[0] << endl;
  out << "  uiqOnce.iLong[1]: " << (int) _rvp8Hdr.uiqOnce.iLong[1] << endl;
  for (int i = 0; i < MAXVIQPERBIN; i++) {
    out << "  iDataOffs[" << i << "]: " << _rvp8Hdr.Rx[i].iDataOff << endl;
    out << "  fBurstMag[" << i << "]: " << _rvp8Hdr.Rx[i].fBurstMag << endl;
    out << "  iBurstArg[" << i << "]: " << _rvp8Hdr.Rx[i].iBurstArg << endl;
    out << "  iWrapIQ[" << i << "]: " << _rvp8Hdr.Rx[i].iWrapIQ << endl;
  }

  double pulseTime =
    (double) _rvp8Hdr.iTimeUTC + _rvp8Hdr.iMSecUTC / 1000.0;
  out << "  pulseTime: " << pulseTime << endl;
  out << "  usTime: " << _rvp8Hdr.iMSecUTC / 1000.0 << endl;
  
  double az = (_rvp8Hdr.iAz / 65536.0) * 360.0;
  double el = (_rvp8Hdr.iEl / 65536.0) * 360.0;
  
  out << "  el: " << el << endl;
  out << "  az: " << az << endl;

}

/////////////////////////////////////////////////////////////////////////
// Memory management.
// This class uses the notion of clients to decide when to delete itself.
// When _nClients drops from 1 to 0, it will call delete on the this pointer.

int Pulse::addClient(const string &clientName)
  
{
  _nClients++;
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "Pulse add client, seqNum, nClients, az, client: "
	 << _seqNum << ", " << _nClients << ", " << _az << ", " << clientName << endl;
  }
  return _nClients;
}

int Pulse::removeClient(const string &clientName)

{
  _nClients--;
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "Pulse rem client, seqNum, nClients, az, client: "
	 << _seqNum << ", " << _nClients << ", " << _az << ", " << clientName << endl;
  }
  return _nClients;
}

///////////////////////////////////////////////////////////
// read in the headers
//
// Returns 0 on success, -1 on failure

int Pulse::_readHeaders(FILE *in, bool hasRocHdr)
{
  
  // read in ROC header
  
  if (hasRocHdr) {
    if (fread(&_rocHdr, sizeof(rocPulseHdr), 1, in) != 1) {
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    if (hasRocHdr) {
      cerr << "  Has ROC header" << endl;
    }
    cerr << "  ROC header version: " << _rocHdr.version << endl;
  }
    
  // if RCO header version is 1, or no ROC header, read in 
  // rvp8 pulse header V0, and convert. Otherwise read in
  // rvp8 pulse header

  if (hasRocHdr && _rocHdr.version > 1) {

    if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
      cerr << "Reading new RVP8 pulse header" << endl;
    }

    if (fread(&_rvp8Hdr, sizeof(_rvp8Hdr), 1, in) != 1) {
      return -1;
    }

  } else {

    if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
      cerr << "Reading old RVP8 pulse header, converting to new" << endl;
    }

    rvp8PulseHdr_v0 rvp8HdrV0;
    if (fread(&rvp8HdrV0, sizeof(rvp8HdrV0), 1, in) != 1) {
      return -1;
    }

    memset(&_rvp8Hdr, 0, sizeof(_rvp8Hdr));
    if (rvp8HdrV0.lValid) {
      _rvp8Hdr.iFlags = PHDRFLG_VALID;
    } else {
      _rvp8Hdr.iFlags = PHDRFLG_DATAGAP;
    }
    
    _rvp8Hdr.iMSecUTC = rvp8HdrV0.iMSecUTC;
    _rvp8Hdr.iTimeUTC = rvp8HdrV0.iTimeUTC;
    _rvp8Hdr.iBtime = rvp8HdrV0.iBtime;
    _rvp8Hdr.iSysTime = rvp8HdrV0.iSysTime;
    _rvp8Hdr.iPrevPRT = rvp8HdrV0.iPrevPRT;
    _rvp8Hdr.iNextPRT = rvp8HdrV0.iNextPRT;
    _rvp8Hdr.iSeqNum = rvp8HdrV0.iSeqNum;
    _rvp8Hdr.iAqMode = rvp8HdrV0.iAqMode;
    _rvp8Hdr.iAz = rvp8HdrV0.iAz;
    _rvp8Hdr.iEl = rvp8HdrV0.iEl;
    _rvp8Hdr.iNumVecs = rvp8HdrV0.iNumVecs;
    _rvp8Hdr.iMaxVecs = rvp8HdrV0.iMaxVecs;
    _rvp8Hdr.iVIQPerBin = rvp8HdrV0.iVIQPerBin;
    _rvp8Hdr.iTgBank = rvp8HdrV0.iTgBank;
    _rvp8Hdr.iTgWave = rvp8HdrV0.iTgWave;
    _rvp8Hdr.uiqPerm = rvp8HdrV0.uiqPerm;
    _rvp8Hdr.uiqOnce = rvp8HdrV0.uiqOnce;
    for (int ii = 0; ii < MAXVIQPERBIN; ii++) {
      _rvp8Hdr.Rx[ii].iDataOff = rvp8HdrV0.iDataOffs[ii];
      _rvp8Hdr.Rx[ii].fBurstMag = rvp8HdrV0.fBurstMags[ii];
      _rvp8Hdr.Rx[ii].iBurstArg = rvp8HdrV0.iBurstArgs[ii];
      _rvp8Hdr.Rx[ii].iWrapIQ = rvp8HdrV0.iWrapIQ;
    }

  }

  return 0;

}
  
/* ======================================================================
 * Convert a normalized floating "I" or "Q" value from the signal
 * processor's 16-bit packed format.  The floating values are in the
 * range -4.0 to +4.0, i.e., they are normalized so that full scale CW
 * input gives a magnitude of 1.0, while still leaving a factor of
 * four of additional amplitude headroom (12dB headroom power) to
 * account for FIR filter transients.
 *
 * From Sigmet lib.
 */

void Pulse::_vecFloatIQFromPackIQ
( volatile FLT4 fIQVals_a[], volatile const UINT2 iCodes_a[], SINT4 iCount_a )

{

  SINT4 iCount ; volatile const UINT2 *iCodes = iCodes_a ;
  volatile FLT4 *fIQVals = fIQVals_a ;

  for( iCount=0 ; iCount < iCount_a ; iCount++ ) {
    UINT2 iCode = *iCodes++ ; FLT4 fVal = 0.0 ;

    if( iCode ) {
      SINT4 iMan =  iCode        & 0x3FF ;
      SINT4 iExp = (iCode >> 11) & 0x01F ;

      if( iCode & 0x0400 ) iMan |= 0xFFFFF800 ;
      else                 iMan |= 0x00000400 ;

      fVal = ((FLT4)iMan) * ((FLT4)(UINT4)(1 << iExp)) / 1.09951163E12 ;
    }
    *fIQVals++ = fVal ;
  }
}


/* ------------------------------
 * Convert an array of packed floating to FLT4.
 */

void Pulse::_vecFloatIQFromPackIQ
( volatile FLT4 fIQVals_a[], volatile const UINT2 iCodes_a[],
  SINT4 iCount_a, UINT1 lHiSNR_a )
{
  SINT4 iCount ; volatile const UINT2 *iCodes = iCodes_a ;
  volatile FLT4 *fIQVals = fIQVals_a ;

  if( lHiSNR_a ) {
    /* High SNR packed format with 12-bit mantissa
     */
    for( iCount=0 ; iCount < iCount_a ; iCount++ ) {
      UINT2 iCode = *iCodes++ ; FLT4 fVal = 0.0 ;

      if( iCode & 0xF000 ) {
        SINT4 iMan =  iCode        & 0x7FF ;
        SINT4 iExp = (iCode >> 12) & 0x00F ;

        if( iCode & 0x0800 ) iMan |= 0xFFFFF000 ;
        else                 iMan |= 0x00000800 ;

        fVal = ((FLT4)iMan) * ((FLT4)(UINT4)(1 << iExp)) / 3.355443E7 ;
      }
      else {
        fVal = ( (FLT4)(((SINT4)iCode) << 20) ) / 1.759218E13 ;
      }
      *fIQVals++ = fVal ;
    }
  } else {
    /* Legacy packed format with 11-bit mantissa
     */
    for( iCount=0 ; iCount < iCount_a ; iCount++ ) {
      UINT2 iCode = *iCodes++ ; FLT4 fVal = 0.0 ;

      if( iCode ) {
        SINT4 iMan =  iCode        & 0x3FF ;
        SINT4 iExp = (iCode >> 11) & 0x01F ;

        if( iCode & 0x0400 ) iMan |= 0xFFFFF800 ;
        else                 iMan |= 0x00000400 ;

        fVal = ((FLT4)iMan) * ((FLT4)(UINT4)(1 << iExp)) / 1.099512E12 ;
      }
      *fIQVals++ = fVal ;
    }
  }
}

///////////////////////////////////////////////////////////////////
// subtract 2 complex numbers
//
// diff is (aa - bb)

void Pulse::_subtract(const Complex_t &aa, const Complex_t &bb,
		      Complex_t &result, double &angle)
  
{
  
  result.re = (aa.re * bb.re + aa.im * bb.im);
  result.im = (aa.im * bb.re - aa.re * bb.im);
  if (result.im != 0.0 || result.re != 0.0) {
    angle = atan2(result.im, result.re) * RAD_TO_DEG;
  } else {
    angle = 0.0;
  }
  
}

