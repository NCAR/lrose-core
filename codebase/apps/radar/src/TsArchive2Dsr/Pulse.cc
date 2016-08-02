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
#include "OpsInfo.hh"
using namespace std;

// Constructor

Pulse::Pulse(const Params &params, double fSyClkMhz) :
        _params(params),
        _nClients(0),
        _fSyClkMhz(fSyClkMhz)
  
{
  _iq = NULL;
  _iq0 = NULL;
  _iq1 = NULL;
  _burstIq0 = NULL;
  _burstIq1 = NULL;
}

// destructor

Pulse::~Pulse()

{
  if (_iq) {
    delete[] _iq;
  }
}

///////////////////////////////////////////////////////////
// read pulse
//
// Returns 0 on success, -1 on failure

int Pulse::read(FILE *in, const OpsInfo &info)

{

  // read in header

  if (_readPulseHeader(in)) {
    return -1;
  }

  _deriveFromPulseHeader();

  if (_readPulseData(in, info)) {
    return -1;
  }

  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    print(cerr);
  }
  
  return 0;

}
  
/////////////////////////////////////////////////////////////////
// Check to see if horizontally polarized

bool Pulse::isHoriz() const

{

  if (!_params.invert_hv_flag) {
    if (_iPolarBits) {
      return true;
    } else {
      return false;
    }
  } else {
    if (_iPolarBits) {
      return false;
    } else {
      return true;
    }
  }

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

#ifdef JUNK
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "----> _iSeqNum: " << _iSeqNum << endl;
    cerr << "----> _time: " << DateTime::strn((time_t)_time) << endl;
    cerr << "----> _az: " << _az << endl;
    cerr << "----> _phaseDiff0: " << _phaseDiff[0] << endl;
    cerr << "----> _phaseDiffs:" << endl;
    for (size_t ii = 0; ii < _phaseDiffs.size(); ii++) {
      cerr << "------> Trip " << ii+1
	   << " phase diff: " << _phaseDiffs[ii] << endl;
    }
  }
#endif
  
#ifdef CHECK_PHASE_DIFF
  vector<Complex_t> burstIQ;
  for (int ii = 0; ii < maxTrips; ii++) {
    Complex_t iq;
    if (ii < qSize) {
      const fl32* fiq = pulseQueue[ii]->getIq();
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

void Pulse::print(ostream &out) const
{

  out << "==================== RVP8 Pulse header ==================" << endl;
  out << "  iVersion: " << (int) _iVersion << endl;
  out << "  iFlags: " << (int) _iFlags << endl;
  out << "  iMSecUTC: " << (int) _iMSecUTC << endl;
  out << "  iTimeUTC: " << (int) _iTimeUTC << endl;
  out << "  iBtimeAPI: " << (int) _iBtimeAPI << endl;
  out << "  iSysTime: " << (int) _iSysTime << endl;
  out << "  iPrevPRT: " << (int) _iPrevPRT << endl;
  out << "  iNextPRT: " << (int) _iNextPRT << endl;
  out << "  iSeqNum: " << (int) _iSeqNum << endl;
  out << "  iAqMode: " << (int) _iAqMode << endl;
  out << "  iAz: " << (int) _iAz << endl;
  out << "  iEl: " << (int) _iEl << endl;
  out << "  iNumVecs: " << _iNumVecs << endl;
  out << "  iMaxVecs: " << _iMaxVecs << endl;
  out << "  iVIQPerBin: " << (int) _iVIQPerBin << endl;
  out << "  iTgBank: " << (int) _iTgBank << endl;
  out << "  iTgWave: " << (int) _iTgWave << endl;
  out << "  uiqPerm.iLong[0]: " << (int) _uiqPerm[0] << endl;
  out << "  uiqPerm.iLong[1]: " << (int) _uiqPerm[1] << endl;
  out << "  uiqOnce.iLong[0]: " << (int) _uiqOnce[0] << endl;
  out << "  uiqOnce.iLong[1]: " << (int) _uiqOnce[1] << endl;
  out << "  RX[0].fBurstMag: " << (int) _fBurstMag[0] << endl;
  out << "  RX[0].iBurstArg: " << (int) _iBurstArg[0] << endl;
  out << "  RX[1].fBurstMag: " << (int) _fBurstMag[1] << endl;
  out << "  RX[1].iBurstArg: " << (int) _iBurstArg[1] << endl;
  out << "==================== Derived values ==================" << endl;
  out << "  nGates: " << _nGates << endl;
  out << "  nChannels: " << _nChannels << endl;
  out << "  seqNum: " << _iSeqNum << endl;
  out << "  time: " << DateTime::str(_time) << endl;
  out << "  ftime: " << _ftime << endl;
  out << "  prt: " << _prt << endl;
  out << "  prf: " << _prf << endl;
  out << "  el: " << _el << endl;
  out << "  az: " << _az << endl;
  out << "  phaseDiff[0]: " << _phaseDiff[0] << endl;
  out << "  phaseDiff[1]: " << _phaseDiff[1] << endl;

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
	 << _iSeqNum << ", " << _nClients << ", " << _az << ", " << clientName << endl;
  }
  return _nClients;
}

int Pulse::removeClient(const string &clientName)

{
  _nClients--;
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "Pulse rem client, seqNum, nClients, az, client: "
	 << _iSeqNum << ", " << _nClients << ", " << _az << ", " << clientName << endl;
  }
  return _nClients;
}

///////////////////////////////////////////////////////////////
// read in pulse header
//
// Returns 0 on success, -1 on failure

int Pulse::_readPulseHeader(FILE *in)

{
  
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "--->> Reading Pulse Header <<-----" << endl;
  }

  char line[BUFSIZ];

  // initialize to missing values

  _iVersion = -999;
  _iFlags = -999;
  _iMSecUTC = -999;
  _iTimeUTC = -999;
  _iBtimeAPI = -999;
  _iSysTime = -999;
  _iPrevPRT = -999;
  _iNextPRT = -999;
  _iSeqNum = -999;
  _iAqMode = -999;
  _iPolarBits = -999;
  _iTxPhase = -999;
  _iAz = -999;
  _iEl = -999;
  _iNumVecs = -999;
  _iMaxVecs = -999;
  _iVIQPerBin = -999;
  _iTgBank = -999;
  _iTgWave = -999;

  _uiqPerm[0] = -999;
  _uiqPerm[1] = -999;
  _uiqOnce[0] = -999;
  _uiqOnce[1] = -999;
  
  _fBurstMag[0] = -999.9;
  _iBurstArg[0] = -999;
  _fBurstMag[1] = -999.9;
  _iBurstArg[1] = -999;

  // read to the pulse header
  
  if (OpsInfo::findNextStr(in, "PulseHdr start")) {
    return -1;
  }
    
  while (!feof(in)) {

    if (fgets(line, BUFSIZ, in) == NULL) {
      return -1;
    }
    
    if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
      char shortLine[80];
      memcpy(shortLine, line, 79);
      shortLine[79] = '\0';
      cerr << shortLine;
    }

    if (strstr(line, "rvp8PulseHdr end")) {
      return 0;
    }

    // look for integers

    int ival, jval;
    if (_iVersion == -999 &&
        sscanf(line, "iVersion=%d", &ival) == 1) {
      _iVersion = ival;
      continue;
    }
    if (_iFlags == -999 &&
        sscanf(line, "iFlags=%d", &ival) == 1) {
      _iFlags = ival;
      continue;
    }
    if (_iMSecUTC == -999 &&
        sscanf(line, "iMSecUTC=%d", &ival) == 1) {
      _iMSecUTC = ival;
      continue;
    }
    if (_iTimeUTC == -999 &&
        sscanf(line, "iTimeUTC=%d", &ival) == 1) {
      _iTimeUTC = ival;
      continue;
    }
    if (_iBtimeAPI == -999 &&
        sscanf(line, "iBtimeAPI=%d", &ival) == 1) {
      _iBtimeAPI = ival;
      continue;
    }
    if (_iSysTime == -999 &&
        sscanf(line, "iSysTime=%d", &ival) == 1) {
      _iSysTime = ival;
      continue;
    }
    if (_iPrevPRT == -999 &&
        sscanf(line, "iPrevPRT=%d", &ival) == 1) {
      _iPrevPRT = ival;
      continue;
    }
    if (_iNextPRT == -999 &&
        sscanf(line, "iNextPRT=%d", &ival) == 1) {
      _iNextPRT = ival;
      continue;
    }
    if (_iSeqNum == -999 &&
        sscanf(line, "iSeqNum=%d", &ival) == 1) {
      _iSeqNum = ival;
      continue;
    }
    if (_iAqMode == -999 &&
        sscanf(line, "iAqMode=%d", &ival) == 1) {
      _iAqMode = ival;
      continue;
    }
    if (_iPolarBits == -999 &&
        sscanf(line, "iPolarBits=%d", &ival) == 1) {
      _iPolarBits = ival;
      continue;
    }
    if (_iTxPhase == -999 &&
        sscanf(line, "iTxPhase=%d", &ival) == 1) {
      _iTxPhase = ival;
      continue;
    }
    if (_iAz == -999 &&
        sscanf(line, "iAz=%d", &ival) == 1) {
      _iAz = ival;
      continue;
    }
    if (_iEl == -999 &&
        sscanf(line, "iEl=%d", &ival) == 1) {
      _iEl = ival;
      continue;
    }
    if (_iNumVecs == -999 &&
        sscanf(line, "iNumVecs=%d", &ival) == 1) {
      _iNumVecs = ival;
      continue;
    }
    if (_iMaxVecs == -999 &&
        sscanf(line, "iMaxVecs=%d", &ival) == 1) {
      _iMaxVecs = ival;
      continue;
    }
    if (_iVIQPerBin == -999 &&
        sscanf(line, "iVIQPerBin=%d", &ival) == 1) {
      _iVIQPerBin = ival;
      continue;
    }
    if (_iTgBank == -999 &&
        sscanf(line, "iTgBank=%d", &ival) == 1) {
      _iTgBank = ival;
      continue;
    }
    if (_iTgWave == -999 &&
        sscanf(line, "iTgWave=%d", &ival) == 1) {
      _iTgWave = ival;
      continue;
    }

    if (_uiqPerm[0] == -999 &&
        sscanf(line, "uiqPerm.iLong=%d %d", &ival, &jval) == 2) {
      _uiqPerm[0] = ival;
      _uiqPerm[1] = jval;
      continue;
    }
    
    if (_uiqOnce[0] == -999 &&
        sscanf(line, "uiqOnce.iLong=%d %d", &ival, &jval) == 2) {
      _uiqOnce[0] = ival;
      _uiqOnce[1] = jval;
      continue;
    }
    
    // look for floats
    
    double dval;
    if (_fBurstMag[0] < -999 &&
        sscanf(line, "RX[0].fBurstMag=%lg", &dval) == 1) {
      _fBurstMag[0] = dval;
      continue;
    }
    if (_iBurstArg[0] == -999 &&
        sscanf(line, "RX[0].iBurstArg=%d", &ival) == 1) {
      _iBurstArg[0] = ival;
      continue;
    }
    if (_fBurstMag[1] < -999 &&
        sscanf(line, "RX[1].fBurstMag=%lg", &dval) == 1) {
      _fBurstMag[1] = dval;
      continue;
    }
    if (_iBurstArg[1] == -999 &&
        sscanf(line, "RX[1].iBurstArg=%d", &ival) == 1) {
      _iBurstArg[1] = ival;
      continue;
    }

  } // while
  
  return 0;

}

///////////////////////////////////////////////////////////////
// derive quantities from pulse header

void Pulse::_deriveFromPulseHeader()

{
  
  _time = _iTimeUTC;
  _ftime = (double) _iTimeUTC + _iMSecUTC / 1000.0;
  _nGates = _iNumVecs - 1; // first gate holds the burst
  _nChannels = _iVIQPerBin;
  _az = (_iAz / 65535.0) * 360.0;
  _el = (_iEl / 65535.0) * 360.0;

  if (_iNextPRT == 0) {
    _prt = 0.001;
  } else {
    _prt = ((double) _iNextPRT / _fSyClkMhz) / 1.0e6;
  }
  _prf = 1.0 / _prt;

  _phaseDiff[0] = (_iBurstArg[0] / 65536.0) * 360.0;
  _phaseDiff[1] = (_iBurstArg[1] / 65536.0) * 360.0;

}

///////////////////////////////////////////////////////////////
// read in pulse data
//
// Should follow _readPulseHeader and _derivedFromPulseHeader()
// immediately
//
// Returns 0 on success, -1 on failure

int Pulse::_readPulseData(FILE *in, const OpsInfo &info)
  
{
  
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "--->> Reading Pulse Data <<-----" << endl;
  }

  // read in packed data

  _nIQ = _iNumVecs * _iVIQPerBin * 2;
  ui16 *packed = new ui16[_nIQ];

  int nRead = (int) fread(packed, sizeof(ui16), _nIQ, in);
  if (nRead != _nIQ) {
    cerr << "ERROR - Pulse::_readPulseData" << endl;
    cerr << "  Cannot fread on pulse data" << endl;
    cerr << "  Expecting nIQ: " << _nIQ << endl;
    cerr << "  Got nRead: " << nRead << endl;
    if (feof(in)) {
      cerr << "  At end of file" << endl;
    }
    delete[] packed;
    return -1;
  }

  // unpack the data

  if (_iq) {
    delete[] _iq;
  }
  _iq = new fl32[_nIQ];
  _vecFloatIQFromPackIQ(_iq, packed, _nIQ);

  _burstIq0 = _iq;
  _iq0 = _burstIq0 + 2; // burst is in first gate
  
  if (_iVIQPerBin > 0) {
    _burstIq1 = _iq +_iNumVecs * 2; // channel 1 after channel 0
    _iq1 = _burstIq1 + 2; // burst is in first gate
  }

  // adjust the IQ values for the saturation characteristics
  // apply the square root of the multiplier, since power is
  // I squared plus Q squared
  
  double mult = sqrt(info.getSaturationMult());
  for (int ii = 0; ii < _nIQ; ii++) {
    _iq[ii] *= mult;
  }

  // clean up

  delete[] packed;

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
( volatile fl32 fIQVals_a[], volatile const ui16 iCodes_a[],
  si32 iCount_a)
{

  si32 iCount ; volatile const ui16 *iCodes = iCodes_a ;
  volatile fl32 *fIQVals = fIQVals_a ;

  /* High SNR packed format with 12-bit mantissa
   */
  for( iCount=0 ; iCount < iCount_a ; iCount++ ) {
    ui16 iCode = *iCodes++ ; fl32 fVal = 0.0 ;
    
    if( iCode & 0xF000 ) {
      si32 iMan =  iCode        & 0x7FF ;
      si32 iExp = (iCode >> 12) & 0x00F ;
      
      if( iCode & 0x0800 ) iMan |= 0xFFFFF000 ;
      else                 iMan |= 0x00000800 ;
      
      fVal = ((fl32)iMan) * ((fl32)(ui32)(1 << iExp)) / 3.355443E7 ;
    }
    else {
      fVal = ( (fl32)(((si32)iCode) << 20) ) / 1.759218E13 ;
    }
    *fIQVals++ = fVal ;
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

