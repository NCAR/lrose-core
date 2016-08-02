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
// OpsInfo.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2006
//
///////////////////////////////////////////////////////////////
//
// Stores current ops info
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <cmath>
#include <dataport/port_types.h>
#include "OpsInfo.hh"
using namespace std;

////////////////////////////////////////////////////
// Constructor

OpsInfo::OpsInfo(const Params &params) :
        _params(params)

{

}

//////////////////////////////////////////////////////////////////
// destructor

OpsInfo::~OpsInfo()

{
  
}

///////////////////////////////////////////////////////////////
// read in pulse info
//
// Returns 0 on success, -1 on failure

int OpsInfo::read(FILE *in)

{

  if (_readPulseInfo(in)) {
    return -1;
  }

  _deriveFromPulseInfo();

  return 0;

}
  
///////////////////////////////////////////////////////////
// print the info
//

void OpsInfo::print(ostream &out) const
{

  _printPulseInfo(out);
  _printDerived(out);

}

///////////////////////////////////////////////////////////////
// read in pulse info
//
// Returns 0 on success, -1 on failure

int OpsInfo::_readPulseInfo(FILE *in)

{
  
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "--->> Reading Pulse Info <<-----" << endl;
  }
  
  // initialize to missing values
  
  _iVersion = -999;
  _iMajorMode = -999;
  _iPolarization = -999;
  _iPhaseModSeq = -999;
  _iSweep = -999;
  _iAuxNum = -999;
  _sTaskName = "";
  _sSiteName = "";
  _iAqMode = -999;
  _iUnfoldMode = -999;
  _iPWidthCode = -999;
  _fPWidthUSec = -999.9;
  _fDBzCalib = -999.9;
  _iSampleSize = -999;
  _iMeanAngleSync = -999;
  _iFlags = -999;
  _iPlaybackVersion = -999;
  _fSyClkMHz = -999.9;
  _fWavelengthCM = -999.9;
  _fSaturationDBM = -999.9;
  _fSaturationMult = -999.9;
  _fRangeMaskRes = -999.9;
  for (int ii = 0; ii < 512; ii++) {
    _iRangeMask[ii] = -999;
  }
  for (int ii = 0; ii < 2; ii++) {
    _fNoiseDBm[ii] = -999.9;
    _fNoiseStdvDB[ii] = -999.9;
  }
  _fNoiseRangeKM = -999.9;
  _fNoisePRFHz = -999.9;
  for (int ii = 0; ii < 2; ii++) {
    _iGparmLatchSts[ii] = -999;
  }
  for (int ii = 0; ii < 6; ii++) {
    _iGparmImmedSts[ii] = -999;
  }
  for (int ii = 0; ii < 4; ii++) {
    _iGparmDiagBits[ii] = -999;
  }
  _sVersionString = "";

  bool taskNameFound = false;
  bool siteNameFound = false;
  bool versionStringFound = false;
  bool rangeMaskFound = false;

  // read in

  bool inPulseInfo = false;
  char line[8192];

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
    
    if (!inPulseInfo && strstr(line, "rvp8PulseInfo start")) {
      inPulseInfo = true;
      continue;
    }
    
    if (strstr(line, "rvp8PulseInfo end")) {
      if (inPulseInfo) {
	return 0;
      } else {
	return -1;
      }
    }

    char name[80];
    int ival, jval[6];
    double dval, eval[2];

    if (_iVersion == -999 && sscanf(line, "iVersion=%d", &ival) == 1) {
      _iVersion = ival;
      continue;
    }
    if (_iMajorMode == -999 && sscanf(line, "iMajorMode=%d", &ival) == 1) {
      _iMajorMode = ival;
      continue;
    }
    if (_iPolarization == -999 &&
        sscanf(line, "iPolarization=%d", &ival) == 1) {
      _iPolarization = ival;
      continue;
    }
    if (_iPhaseModSeq == -999 &&
        sscanf(line, "iPhaseModSeq=%d", &ival) == 1) {
      _iPhaseModSeq = ival;
      continue;
    }
    if (_iSweep == -999 && sscanf(line, "taskID.iSweep=%d", &ival) == 1) {
      _iSweep = ival;
      continue;
    }
    if (_iAuxNum == -999 && sscanf(line, "taskID.iAuxNum=%d", &ival) == 1) {
      _iAuxNum = ival;
      continue;
    }
    if (!taskNameFound && strlen(line) < 80 &&
	sscanf(line, "taskID.sTaskName=%s", name) == 1) {
      _sTaskName = name;
      taskNameFound = true;
      continue;
    }
    if (!siteNameFound && strlen(line) < 80 &&
	sscanf(line, "sSiteName=%s", name) == 1) {
      _sSiteName = name;
    }
    if (_iAqMode == -999 && sscanf(line, "iAqMode=%d", &ival) == 1) {
      _iAqMode = ival;
      continue;
    }
    if (_iUnfoldMode == -999 && sscanf(line, "iUnfoldMode=%d", &ival) == 1) {
      _iUnfoldMode = ival;
      continue;
    }
    if (_iPWidthCode == -999 && sscanf(line, "iPWidthCode=%d", &ival) == 1) {
      _iPWidthCode = ival;
      continue;
    }
    if (_fPWidthUSec < -999 && sscanf(line, "fPWidthUSec=%lg", &dval) == 1) {
      _fPWidthUSec = dval;
      continue;
    }
    if (_fDBzCalib < -999 && sscanf(line, "fDBzCalib=%lg", &dval) == 1) {
      _fDBzCalib = dval;
      continue;
    }
    if (_iSampleSize == -999 && sscanf(line, "iSampleSize=%d", &ival) == 1) {
      _iSampleSize = ival;
      continue;
    }
    if (_iMeanAngleSync == -999 &&
        sscanf(line, "iMeanAngleSync=%d", &ival) == 1) {
      _iMeanAngleSync = ival;
      continue;
    }
    if (_iFlags == -999 && sscanf(line, "iFlags=%d", &ival) == 1) {
      _iFlags = ival;
      continue;
    }
    if (_iPlaybackVersion == -999 &&
        sscanf(line, "iPlaybackVersion=%d", &ival) == 1) {
      _iPlaybackVersion = ival;
      continue;
    }
    if (_fSyClkMHz < -999 && sscanf(line, "fSyClkMHz=%lg", &dval) == 1) {
      _fSyClkMHz = dval;
      continue;
    }
    if (_fWavelengthCM < -999 &&
        sscanf(line, "fWavelengthCM=%lg", &dval) == 1) {
      _fWavelengthCM = dval;
      continue;
    }
    if (_fSaturationDBM < -999 &&
        sscanf(line, "fSaturationDBM=%lg", &dval) == 1) {
      _fSaturationDBM = dval;
      _fSaturationMult = pow(10.0, dval / 10.0);
      continue;
    }
    if (_fRangeMaskRes < -999 &&
        sscanf(line, "fRangeMaskRes=%lg", &dval) == 1) {
      _fRangeMaskRes = dval;
      continue;
    }
    if (!rangeMaskFound && strstr(line, "iRangeMask=") != NULL) {
      char *toks = strtok(line, "= ");
      int count = 0;
      while (toks != NULL && count < 512) {
        toks = strtok(NULL, "= ");
        if (sscanf(toks, "%d", &ival) == 1) {
          _iRangeMask[count] = ival;
        }
        count++;
      }
      rangeMaskFound = true;
      continue;
    }

    if (_fNoiseDBm[0] < -999 &&
        sscanf(line, "fNoiseDBm=%lg %lg", &eval[0], &eval[1]) == 2) {
      _fNoiseDBm[0] = eval[0];
      _fNoiseDBm[1] = eval[1];
      continue;
    }

    if (_fNoiseStdvDB[0] < -999 &&
        sscanf(line, "fNoiseStdvDB=%lg %lg", &eval[0], &eval[1]) == 2) {
      _fNoiseStdvDB[0] = eval[0];
      _fNoiseStdvDB[1] = eval[1];
      continue;
    }

    if (_fNoiseRangeKM < -999 &&
        sscanf(line, "fNoiseRangeKM=%lg", &dval) == 1) {
      _fNoiseRangeKM = dval;
      continue;
    }

    if (_fNoisePRFHz < -999 && sscanf(line, "fNoisePRFHz=%lg", &dval) == 1) {
      _fNoisePRFHz = dval;
      continue;
    }
    
    if (_iGparmLatchSts[0] == -999 &&
        sscanf(line, "iGparmLatchSts=%d %d", &jval[0], &jval[1]) == 2) {
      _iGparmLatchSts[0] = jval[0];
      _iGparmLatchSts[1] = jval[1];
      continue;
    }

    if (_iGparmImmedSts[0] == -999 &&
        sscanf(line, "iGparmImmedSts=%d %d %d %d %d %d",
               &jval[0], &jval[1], &jval[2],
               &jval[3], &jval[4], &jval[5]) == 6) {
      _iGparmImmedSts[0] = jval[0];
      _iGparmImmedSts[1] = jval[1];
      _iGparmImmedSts[2] = jval[2];
      _iGparmImmedSts[3] = jval[3];
      _iGparmImmedSts[4] = jval[4];
      _iGparmImmedSts[5] = jval[5];
      continue;
    }

    if (_iGparmDiagBits[0] == -999 &&
        sscanf(line, "iGparmDiagBits=%d %d %d %d",
               &jval[0], &jval[1],
               &jval[2], &jval[3]) == 4) {
      _iGparmDiagBits[0] = jval[0];
      _iGparmDiagBits[1] = jval[1];
      _iGparmDiagBits[2] = jval[2];
      _iGparmDiagBits[3] = jval[3];
      continue;
    }

    if (!versionStringFound && strlen(line) < 80 &&
	sscanf(line, "sVersionString=%s", name) == 1) {
      _sVersionString = name;
      continue;
    }
    
  } // while

  return 0;

}

///////////////////////////////////////////////////////////////
// derive quantities from pulse header

void OpsInfo::_deriveFromPulseInfo()

{

  // Find first, last, and total number of range bins in the mask
  // Based on SIGMET code in tsview.c

  int binCount = 0;
  int binStart = 0;
  int binEnd = 0;

  for (int ii = 0; ii < 512; ii++) {
    ui16 mask = _iRangeMask[ii];
    if (mask) {
      for (int iBit = 0; iBit < 16; iBit++) {
        if (1 & (mask >> iBit)) {
          int iBin = iBit + (16*ii);
          if (binCount == 0) {
            binStart = iBin;
          }
          binEnd = iBin;
          binCount++;
        }
      }
    }
  }
  
  // range computations

  _startRange = (binStart * _fRangeMaskRes) / 1000.0;
  _maxRange = (binEnd * _fRangeMaskRes) / 1000.0;
  _gateSpacing = (_maxRange - _startRange) / (binCount - 1.0);

  // polarization type

  if (_iPolarization == 0) {
    _polarizationType = "H";
  } else if (_iPolarization == 1) {
    _polarizationType = "V";
  } else if (_iPolarization == 2) {
    _polarizationType = "Dual_alt";
  } else if (_iPolarization == 3) {
    _polarizationType = "Dual_simul";
  }

}

///////////////////////////////////////////////////////////
// print the pulse header
//

void OpsInfo::_printPulseInfo(ostream &out) const
{

  out << "==================== Pulse info ==================" << endl;

  out << "  _iVersion: " << _iVersion << endl;
  out << "  _iMajorMode: " << _iMajorMode << endl;
  out << "  _iPolarization: " << _iPolarization << endl;
  out << "  _iPhaseModSeq: " << _iPhaseModSeq << endl;
  out << "  _iSweep: " << _iSweep << endl;
  out << "  _iAuxNum: " << _iAuxNum << endl;
  out << "  _sTaskName: " << _sTaskName << endl;
  out << "  _sSiteName: " << _sSiteName << endl;
  out << "  _iAqMode: " << _iAqMode << endl;
  out << "  _iUnfoldMode: " << _iUnfoldMode << endl;
  out << "  _iPWidthCode: " << _iPWidthCode << endl;
  out << "  _fPWidthUSec: " << _fPWidthUSec << endl;
  out << "  _fDBzCalib: " << _fDBzCalib << endl;
  out << "  _iSampleSize: " << _iSampleSize << endl;
  out << "  _iMeanAngleSync: " << _iMeanAngleSync << endl;
  out << "  _iFlags: " << _iFlags << endl;
  out << "  _iPlaybackVersion: " << _iPlaybackVersion << endl;
  out << "  _fSyClkMHz: " << _fSyClkMHz << endl;
  out << "  _fWavelengthCM: " << _fWavelengthCM << endl;
  out << "  _fSaturationDBM: " << _fSaturationDBM << endl;
  out << "  _fRangeMaskRes: " << _fRangeMaskRes << endl;
  out << "  _iRangeMask: ";

  for (int ii = 0; ii < 512; ii++) {
    out << _iRangeMask[ii];
    if (ii != 511) {
      out << " ";
    }
  }
  out << endl;

  out << "  _fNoiseDBm: "
      << _fNoiseDBm[0] << ", "
      << _fNoiseDBm[1] << endl;

  out << "  _fNoiseStdvDB: "
      << _fNoiseStdvDB[0] << ", "
      << _fNoiseStdvDB[1] << endl;

  out << "  _fNoiseRangeKM: " << _fNoiseRangeKM << endl;
  out << "  _fNoisePRFHz: " << _fNoisePRFHz << endl;

  out << "  _iGparmLatchSts: "
      << _iGparmLatchSts[0] << ", "
      << _iGparmLatchSts[1] << endl;

  out << "  _iGparmImmedSts: "
      << _iGparmImmedSts[0] << ", "
      << _iGparmImmedSts[1] << ", "
      << _iGparmImmedSts[2] << ", "
      << _iGparmImmedSts[3] << ", "
      << _iGparmImmedSts[4] << ", "
      << _iGparmImmedSts[5] << endl;

  out << "  _iGparmDiagBits: "
      << _iGparmDiagBits[0] << ", "
      << _iGparmDiagBits[1] << ", "
      << _iGparmDiagBits[2] << ", "
      << _iGparmDiagBits[3] << endl;

  out << "  _sVersionString: " << _sVersionString << endl;

}

///////////////////////////////////////////////////////////
// print the derived info
//

void OpsInfo::_printDerived(ostream &out) const
{
  
  out << "==================== Derived info ==================" << endl;

  out << "  _startRange: " << _startRange << endl;
  out << "  _maxRange: " << _maxRange << endl;
  out << "  _gateSpacing: " << _gateSpacing << endl;
  out << "  _polarizationType: " << _polarizationType << endl;

}


