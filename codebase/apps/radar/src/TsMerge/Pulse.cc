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

Pulse::Pulse(const Params &params, double fSyClkMhz) :
        _params(params),
        _fSyClkMhz(fSyClkMhz)
  
{
  _iq = NULL;
  _packed = NULL;
}

// destructor

Pulse::~Pulse()

{
  if (_iq) {
    delete[] _iq;
  }
  if (_packed) {
    delete[] _packed;
  }
}

///////////////////////////////////////////////////////////
// read pulse
//
// Returns 0 on success, -1 on failure

int Pulse::read(FILE *in)

{

  // read in header

  if (_readPulseHeader(in)) {
    return -1;
  }

  _deriveFromPulseHeader();

  if (_readPulseData(in)) {
    return -1;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    print(cerr);
  }
  
  return 0;

}
  
/////////////////////////////////////////////////////////////////
// Check to see if horizontally polarized

bool Pulse::isHoriz() const

{

  if (iPolarBits) {
    return false;
  } else {
    return true;
  }

}

/////////////////////////////////////
// print

void Pulse::print(ostream &out) const
{

  out << "==================== RVP8 Pulse header ==================" << endl;
  out << "  iVersion: " << (int) iVersion << endl;
  out << "  iFlags: " << (int) iFlags << endl;
  out << "  iMSecUTC: " << (int) iMSecUTC << endl;
  out << "  iTimeUTC: " << (int) iTimeUTC << endl;
  out << "  iBtimeAPI: " << (int) iBtimeAPI << endl;
  out << "  iSysTime: " << (int) iSysTime << endl;
  out << "  iPrevPRT: " << (int) iPrevPRT << endl;
  out << "  iNextPRT: " << (int) iNextPRT << endl;
  out << "  iSeqNum: " << (int) iSeqNum << endl;
  out << "  iAqMode: " << (int) iAqMode << endl;
  out << "  iAz: " << (int) iAz << endl;
  out << "  iEl: " << (int) iEl << endl;
  out << "  iNumVecs: " << iNumVecs << endl;
  out << "  iMaxVecs: " << iMaxVecs << endl;
  out << "  iVIQPerBin: " << (int) iVIQPerBin << endl;
  out << "  iTgBank: " << (int) iTgBank << endl;
  out << "  iTgWave: " << (int) iTgWave << endl;
  out << "  uiqPerm.iLong[0]: " << (int) uiqPerm[0] << endl;
  out << "  uiqPerm.iLong[1]: " << (int) uiqPerm[1] << endl;
  out << "  uiqOnce.iLong[0]: " << (int) uiqOnce[0] << endl;
  out << "  uiqOnce.iLong[1]: " << (int) uiqOnce[1] << endl;
  out << "  RX[0].fBurstMag: " << (int) fBurstMag[0] << endl;
  out << "  RX[0].iBurstArg: " << (int) iBurstArg[0] << endl;
  out << "  RX[1].fBurstMag: " << (int) fBurstMag[1] << endl;
  out << "  RX[1].iBurstArg: " << (int) iBurstArg[1] << endl;
  out << "==================== Derived values ==================" << endl;
  out << "  nGates: " << _nGates << endl;
  out << "  nChannels: " << _nChannels << endl;
  out << "  seqNum: " << iSeqNum << endl;
  out << "  time: " << DateTime::str(_time) << endl;
  out << "  ftime: " << _ftime << endl;
  out << "  prt: " << _prt << endl;
  out << "  prf: " << _prf << endl;
  out << "  el: " << _el << endl;
  out << "  az: " << _az << endl;
  out << "  phaseDiff[0]: " << _phaseDiff[0] << endl;
  out << "  phaseDiff[1]: " << _phaseDiff[1] << endl;

}

///////////////////////////////////////////////////////////////
// read in pulse header
//
// Returns 0 on success, -1 on failure

int Pulse::_readPulseHeader(FILE *in)

{
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "--->> Reading Pulse Header <<-----" << endl;
  }

  bool inPulseHeader = false;
  char line[BUFSIZ];

  // initialize to missing values

  iVersion = -999;
  iFlags = -999;
  iMSecUTC = -999;
  iTimeUTC = -999;
  iBtimeAPI = -999;
  iSysTime = -999;
  iPrevPRT = -999;
  iNextPRT = -999;
  iSeqNum = -999;
  iAqMode = -999;
  iPolarBits = -999;
  iTxPhase = -999;
  iAz = -999;
  iEl = -999;
  iNumVecs = -999;
  iMaxVecs = -999;
  iVIQPerBin = -999;
  iTgBank = -999;
  iTgWave = -999;

  uiqPerm[0] = -999;
  uiqPerm[1] = -999;
  uiqOnce[0] = -999;
  uiqOnce[1] = -999;
  
  fBurstMag[0] = -999.9;
  iBurstArg[0] = -999;
  fBurstMag[1] = -999.9;
  iBurstArg[1] = -999;

  while (!feof(in)) {

    if (fgets(line, BUFSIZ, in) == NULL) {
      return -1;
    }
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      char shortLine[80];
      memcpy(shortLine, line, 79);
      shortLine[79] = '\0';
      cerr << shortLine;
    }
    
    if (strstr(line, "rvp8PulseHdr start")) {
      inPulseHeader = true;
    }

    if (strstr(line, "rvp8PulseHdr end")) {
      if (inPulseHeader) {
	return 0;
      } else {
	return -1;
      }
    }

    // look for integers

    int ival, jval;
    if (iVersion == -999 &&
        sscanf(line, "iVersion=%d", &ival) == 1) {
      iVersion = ival;
      continue;
    }
    if (iFlags == -999 &&
        sscanf(line, "iFlags=%d", &ival) == 1) {
      iFlags = ival;
      continue;
    }
    if (iMSecUTC == -999 &&
        sscanf(line, "iMSecUTC=%d", &ival) == 1) {
      iMSecUTC = ival;
      continue;
    }
    if (iTimeUTC == -999 &&
        sscanf(line, "iTimeUTC=%d", &ival) == 1) {
      iTimeUTC = ival;
      continue;
    }
    if (iBtimeAPI == -999 &&
        sscanf(line, "iBtimeAPI=%d", &ival) == 1) {
      iBtimeAPI = ival;
      continue;
    }
    if (iSysTime == -999 &&
        sscanf(line, "iSysTime=%d", &ival) == 1) {
      iSysTime = ival;
      continue;
    }
    if (iPrevPRT == -999 &&
        sscanf(line, "iPrevPRT=%d", &ival) == 1) {
      iPrevPRT = ival;
      continue;
    }
    if (iNextPRT == -999 &&
        sscanf(line, "iNextPRT=%d", &ival) == 1) {
      iNextPRT = ival;
      continue;
    }
    if (iSeqNum == -999 &&
        sscanf(line, "iSeqNum=%d", &ival) == 1) {
      iSeqNum = ival;
      continue;
    }
    if (iAqMode == -999 &&
        sscanf(line, "iAqMode=%d", &ival) == 1) {
      iAqMode = ival;
      continue;
    }
    if (iPolarBits == -999 &&
        sscanf(line, "iPolarBits=%d", &ival) == 1) {
      iPolarBits = ival;
      continue;
    }
    if (iTxPhase == -999 &&
        sscanf(line, "iTxPhase=%d", &ival) == 1) {
      iTxPhase = ival;
      continue;
    }
    if (iAz == -999 &&
        sscanf(line, "iAz=%d", &ival) == 1) {
      iAz = ival;
      continue;
    }
    if (iEl == -999 &&
        sscanf(line, "iEl=%d", &ival) == 1) {
      iEl = ival;
      continue;
    }
    if (iNumVecs == -999 &&
        sscanf(line, "iNumVecs=%d", &ival) == 1) {
      iNumVecs = ival;
      continue;
    }
    if (iMaxVecs == -999 &&
        sscanf(line, "iMaxVecs=%d", &ival) == 1) {
      iMaxVecs = ival;
      continue;
    }
    if (iVIQPerBin == -999 &&
        sscanf(line, "iVIQPerBin=%d", &ival) == 1) {
      iVIQPerBin = ival;
      continue;
    }
    if (iTgBank == -999 &&
        sscanf(line, "iTgBank=%d", &ival) == 1) {
      iTgBank = ival;
      continue;
    }
    if (iTgWave == -999 &&
        sscanf(line, "iTgWave=%d", &ival) == 1) {
      iTgWave = ival;
      continue;
    }

    if (uiqPerm[0] == -999 &&
        sscanf(line, "uiqPerm.iLong=%d %d", &ival, &jval) == 2) {
      uiqPerm[0] = ival;
      uiqPerm[1] = jval;
      continue;
    }
    
    if (uiqOnce[0] == -999 &&
        sscanf(line, "uiqOnce.iLong=%d %d", &ival, &jval) == 2) {
      uiqOnce[0] = ival;
      uiqOnce[1] = jval;
      continue;
    }
    
    // look for floats
    
    double dval;
    if (fBurstMag[0] < -999 &&
        sscanf(line, "RX[0].fBurstMag=%lg", &dval) == 1) {
      fBurstMag[0] = dval;
      continue;
    }
    if (iBurstArg[0] == -999 &&
        sscanf(line, "RX[0].iBurstArg=%d", &ival) == 1) {
      iBurstArg[0] = ival;
      continue;
    }
    if (fBurstMag[1] < -999 &&
        sscanf(line, "RX[1].fBurstMag=%lg", &dval) == 1) {
      fBurstMag[1] = dval;
      continue;
    }
    if (iBurstArg[1] == -999 &&
        sscanf(line, "RX[1].iBurstArg=%d", &ival) == 1) {
      iBurstArg[1] = ival;
      continue;
    }

  } // while
  
  return 0;

}

///////////////////////////////////////////////////////////////
// derive quantities from pulse header

void Pulse::_deriveFromPulseHeader()

{
  
  _time = iTimeUTC;
  _ftime = (double) iTimeUTC + iMSecUTC / 1000.0;
  _nGates = iNumVecs - 1; // first gate holds the burst
  _nChannels = iVIQPerBin;
  _az = (iAz / 65535.0) * 360.0;
  _el = (iEl / 65535.0) * 360.0;

  if (iNextPRT == 0) {
    _prt = 0.001;
  } else {
    _prt = ((double) iNextPRT / _fSyClkMhz) / 1.0e6;
  }
  _prf = 1.0 / _prt;

  _phaseDiff[0] = (iBurstArg[0] / 65536.0) * 360.0;
  _phaseDiff[1] = (iBurstArg[1] / 65536.0) * 360.0;

}

///////////////////////////////////////////////////////////////
// read in pulse data
//
// Should follow _readPulseHeader and _derivedFromPulseHeader()
// immediately
//
// Returns 0 on success, -1 on failure

int Pulse::_readPulseData(FILE *in)
  
{
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "--->> Reading Pulse Data <<-----" << endl;
  }

  // read in packed data

  _nIQ = iNumVecs * iVIQPerBin * 2;
  _packed = new ui16[_nIQ];

  int nRead = (int) fread(_packed, sizeof(ui16), _nIQ, in);
  if (nRead != _nIQ) {
    cerr << "ERROR - Pulse::_readPulseData" << endl;
    cerr << "  Cannot fread on pulse data" << endl;
    cerr << "  Expecting nIQ: " << _nIQ << endl;
    cerr << "  Got nRead: " << nRead << endl;
    if (feof(in)) {
      cerr << "  At end of file" << endl;
    }
    return -1;
  }

  // unpack the data

  if (_iq) {
    delete[] _iq;
  }
  _iq = new fl32[_nIQ];
  _vecFloatIQFromPackIQ(_iq, _packed, _nIQ);
  // vecFloatIQFromPackIQ_(_iq, _packed, _nIQ, 0);

  return 0;

}

///////////////////////////////////////////////////////////////
// reduce the IQ values by a given DB value

void Pulse::reduceIq(double reductionDb)
  
{
  
  double reductionFactor = 1.0 / sqrt(pow(10.0, reductionDb / 10.0));

  for (int ii = 0; ii < _nIQ; ii++) {
    _iq[ii] = _iq[ii] * reductionFactor;
  }

}

///////////////////////////////////////////////////////////////
// merge IQ data from another pulse into this one by
// adding the IQ data
//
// returns 0 on success, -1 on failure

int Pulse::addIq(const fl32 *iq, int niq,
                 double reductionDb)
  
{
  
  if (niq != _nIQ) {
    cerr << "ERROR - Pulse::addIq" << endl;
    cerr << "  nIQ does not match" << endl;
    cerr << "  Expecting nIQ: " << _nIQ << endl;
    cerr << "  Passed in nIQ: " << niq << endl;
    return -1;
  }
  
  double reductionFactor = 1.0 / sqrt(pow(10.0, reductionDb / 10.0));

  for (int ii = 0; ii < niq; ii++) {
    _iq[ii] += (iq[ii] * reductionFactor);
  }

  return 0;

}

///////////////////////////////////////////////////////////////
// pack the float IQ data into int16

void Pulse::packIq()
  
{
  
  if (_packed == NULL) {
    _packed = new ui16[_nIQ];
  }

  // vecPackIQFromFloatIQ_(_packed, _iq, _nIQ, 0);
  _vecPackIQFromFloatIQ(_packed, _iq, _nIQ);

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

/* ======================================================================
 *
 * Convert an array of FLT4's to packed floating.
 */

void Pulse::_vecPackIQFromFloatIQ
  ( volatile ui16 iCodes_a[], volatile const fl32 fIQVals_a[],
    si32 iCount_a)
{

  si32 iCount ; volatile const fl32 *fIQVals = fIQVals_a ;
  volatile ui16 *iCodes = iCodes_a ;

  for( iCount=0 ; iCount < iCount_a ; iCount++ ) {
    ui16 iCode ; fl32 fIQVal = *fIQVals++ ;
    
    if     (  fIQVal >=  4.0 ) iCode = 0xF7FF ;
    else if(  fIQVal <= -4.0 ) iCode = 0xF800 ;
    else if( (fIQVal >  -1.221299E-4) &&
             (fIQVal <   1.220703E-4) ) {
      si32 iUnderFlow = NINT( 1.677721E7 * fIQVal ) ;
      iCode = 0xFFF & MAX( -2048, MIN( 2047, iUnderFlow ) ) ;
    } else {

      int iSign, iExp, iMan ;
      iMan = NINT( 4096.0 * frexp( fIQVal, &iExp ) ) ;
      iExp += 13 ; iSign = (fIQVal < 0.0) ;
      
      if( iMan ==  4096 ) iExp++ ; /* Fix up mantissa overflows */
      if( iMan == -2048 ) iExp-- ; /*   by adjusting the exponent. */
      
      iCode = (iExp << 12) | (iSign << 11) | (0x7FF & iMan) ;
    }
    *iCodes++ = iCode ;
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

