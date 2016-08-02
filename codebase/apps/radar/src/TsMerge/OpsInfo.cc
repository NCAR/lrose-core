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
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "--->> Reading Pulse Info <<-----" << endl;
  }
  
  // initialize to missing values
  
  iVersion = -999;
  iMajorMode = -999;
  iPolarization = -999;
  iPhaseModSeq = -999;
  iSweep = -999;
  iAuxNum = -999;
  sTaskName = "";
  sSiteName = "";
  iAqMode = -999;
  iUnfoldMode = -999;
  iPWidthCode = -999;
  fPWidthUSec = -999.9;
  fDBzCalib = -999.9;
  iSampleSize = -999;
  iMeanAngleSync = -999;
  iFlags = -999;
  iPlaybackVersion = -999;
  fSyClkMHz = -999.9;
  fWavelengthCM = -999.9;
  fSaturationDBM = -999.9;
  fSaturationMult = -999.9;
  fRangeMaskRes = -999.9;
  for (int ii = 0; ii < 512; ii++) {
    iRangeMask[ii] = -999;
  }
  for (int ii = 0; ii < 2; ii++) {
    fNoiseDBm[ii] = -999.9;
    fNoiseStdvDB[ii] = -999.9;
  }
  fNoiseRangeKM = -999.9;
  fNoisePRFHz = -999.9;
  for (int ii = 0; ii < 2; ii++) {
    iGparmLatchSts[ii] = -999;
  }
  for (int ii = 0; ii < 6; ii++) {
    iGparmImmedSts[ii] = -999;
  }
  for (int ii = 0; ii < 4; ii++) {
    iGparmDiagBits[ii] = -999;
  }
  sVersionString = "";

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
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
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

    if (iVersion == -999 && sscanf(line, "iVersion=%d", &ival) == 1) {
      iVersion = ival;
      continue;
    }
    if (iMajorMode == -999 && sscanf(line, "iMajorMode=%d", &ival) == 1) {
      iMajorMode = ival;
      continue;
    }
    if (iPolarization == -999 &&
        sscanf(line, "iPolarization=%d", &ival) == 1) {
      iPolarization = ival;
      continue;
    }
    if (iPhaseModSeq == -999 &&
        sscanf(line, "iPhaseModSeq=%d", &ival) == 1) {
      iPhaseModSeq = ival;
      continue;
    }
    if (iSweep == -999 && sscanf(line, "taskID.iSweep=%d", &ival) == 1) {
      iSweep = ival;
      continue;
    }
    if (iAuxNum == -999 && sscanf(line, "taskID.iAuxNum=%d", &ival) == 1) {
      iAuxNum = ival;
      continue;
    }
    if (!taskNameFound && strlen(line) < 80 &&
	sscanf(line, "taskID.sTaskName=%s", name) == 1) {
      sTaskName = name;
      taskNameFound = true;
      continue;
    }
    if (!siteNameFound && strlen(line) < 80 &&
	sscanf(line, "sSiteName=%s", name) == 1) {
      sSiteName = name;
    }
    if (iAqMode == -999 && sscanf(line, "iAqMode=%d", &ival) == 1) {
      iAqMode = ival;
      continue;
    }
    if (iUnfoldMode == -999 && sscanf(line, "iUnfoldMode=%d", &ival) == 1) {
      iUnfoldMode = ival;
      continue;
    }
    if (iPWidthCode == -999 && sscanf(line, "iPWidthCode=%d", &ival) == 1) {
      iPWidthCode = ival;
      continue;
    }
    if (fPWidthUSec < -999 && sscanf(line, "fPWidthUSec=%lg", &dval) == 1) {
      fPWidthUSec = dval;
      continue;
    }
    if (fDBzCalib < -999 && sscanf(line, "fDBzCalib=%lg", &dval) == 1) {
      fDBzCalib = dval;
      continue;
    }
    if (iSampleSize == -999 && sscanf(line, "iSampleSize=%d", &ival) == 1) {
      iSampleSize = ival;
      continue;
    }
    if (iMeanAngleSync == -999 &&
        sscanf(line, "iMeanAngleSync=%d", &ival) == 1) {
      iMeanAngleSync = ival;
      continue;
    }
    if (iFlags == -999 && sscanf(line, "iFlags=%d", &ival) == 1) {
      iFlags = ival;
      continue;
    }
    if (iPlaybackVersion == -999 &&
        sscanf(line, "iPlaybackVersion=%d", &ival) == 1) {
      iPlaybackVersion = ival;
      continue;
    }
    if (fSyClkMHz < -999 && sscanf(line, "fSyClkMHz=%lg", &dval) == 1) {
      fSyClkMHz = dval;
      continue;
    }
    if (fWavelengthCM < -999 &&
        sscanf(line, "fWavelengthCM=%lg", &dval) == 1) {
      fWavelengthCM = dval;
      continue;
    }
    if (fSaturationDBM < -999 &&
        sscanf(line, "fSaturationDBM=%lg", &dval) == 1) {
      fSaturationDBM = dval;
      fSaturationMult = pow(10.0, dval / 10.0);
      continue;
    }
    if (fRangeMaskRes < -999 &&
        sscanf(line, "fRangeMaskRes=%lg", &dval) == 1) {
      fRangeMaskRes = dval;
      continue;
    }
    if (!rangeMaskFound && strstr(line, "iRangeMask=") != NULL) {
      char *toks = strtok(line, "= ");
      int count = 0;
      while (toks != NULL && count < 512) {
        toks = strtok(NULL, "= ");
        if (toks != NULL) {
          if (sscanf(toks, "%d", &ival) == 1) {
            iRangeMask[count] = ival;
          }
          count++;
        }
      }
      rangeMaskFound = true;
      continue;
    }

    if (fNoiseDBm[0] < -999 &&
        sscanf(line, "fNoiseDBm=%lg %lg", &eval[0], &eval[1]) == 2) {
      fNoiseDBm[0] = eval[0];
      fNoiseDBm[1] = eval[1];
      continue;
    }

    if (fNoiseStdvDB[0] < -999 &&
        sscanf(line, "fNoiseStdvDB=%lg %lg", &eval[0], &eval[1]) == 2) {
      fNoiseStdvDB[0] = eval[0];
      fNoiseStdvDB[1] = eval[1];
      continue;
    }

    if (fNoiseRangeKM < -999 &&
        sscanf(line, "fNoiseRangeKM=%lg", &dval) == 1) {
      fNoiseRangeKM = dval;
      continue;
    }

    if (fNoisePRFHz < -999 && sscanf(line, "fNoisePRFHz=%lg", &dval) == 1) {
      fNoisePRFHz = dval;
      continue;
    }
    
    if (iGparmLatchSts[0] == -999 &&
        sscanf(line, "iGparmLatchSts=%d %d", &jval[0], &jval[1]) == 2) {
      iGparmLatchSts[0] = jval[0];
      iGparmLatchSts[1] = jval[1];
      continue;
    }

    if (iGparmImmedSts[0] == -999 &&
        sscanf(line, "iGparmImmedSts=%d %d %d %d %d %d",
               &jval[0], &jval[1], &jval[2],
               &jval[3], &jval[4], &jval[5]) == 6) {
      iGparmImmedSts[0] = jval[0];
      iGparmImmedSts[1] = jval[1];
      iGparmImmedSts[2] = jval[2];
      iGparmImmedSts[3] = jval[3];
      iGparmImmedSts[4] = jval[4];
      iGparmImmedSts[5] = jval[5];
      continue;
    }

    if (iGparmDiagBits[0] == -999 &&
        sscanf(line, "iGparmDiagBits=%d %d %d %d",
               &jval[0], &jval[1],
               &jval[2], &jval[3]) == 4) {
      iGparmDiagBits[0] = jval[0];
      iGparmDiagBits[1] = jval[1];
      iGparmDiagBits[2] = jval[2];
      iGparmDiagBits[3] = jval[3];
      continue;
    }

    if (!versionStringFound && strlen(line) < 80 &&
	sscanf(line, "sVersionString=%s", name) == 1) {
      sVersionString = name;
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
    ui16 mask = iRangeMask[ii];
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

  startRange = (binStart * fRangeMaskRes) / 1000.0;
  maxRange = (binEnd * fRangeMaskRes) / 1000.0;
  gateSpacing = (maxRange - startRange) / (binCount - 1.0);

}

///////////////////////////////////////////////////////////
// print the pulse header
//

void OpsInfo::_printPulseInfo(ostream &out) const
{

  out << "==================== Pulse info ==================" << endl;

  out << "  iVersion: " << iVersion << endl;
  out << "  iMajorMode: " << iMajorMode << endl;
  out << "  iPolarization: " << iPolarization << endl;
  out << "  iPhaseModSeq: " << iPhaseModSeq << endl;
  out << "  iSweep: " << iSweep << endl;
  out << "  iAuxNum: " << iAuxNum << endl;
  out << "  sTaskName: " << sTaskName << endl;
  out << "  sSiteName: " << sSiteName << endl;
  out << "  iAqMode: " << iAqMode << endl;
  out << "  iUnfoldMode: " << iUnfoldMode << endl;
  out << "  iPWidthCode: " << iPWidthCode << endl;
  out << "  fPWidthUSec: " << fPWidthUSec << endl;
  out << "  fDBzCalib: " << fDBzCalib << endl;
  out << "  iSampleSize: " << iSampleSize << endl;
  out << "  iMeanAngleSync: " << iMeanAngleSync << endl;
  out << "  iFlags: " << iFlags << endl;
  out << "  iPlaybackVersion: " << iPlaybackVersion << endl;
  out << "  fSyClkMHz: " << fSyClkMHz << endl;
  out << "  fWavelengthCM: " << fWavelengthCM << endl;
  out << "  fSaturationDBM: " << fSaturationDBM << endl;
  out << "  fRangeMaskRes: " << fRangeMaskRes << endl;
  out << "  iRangeMask: ";

  for (int ii = 0; ii < 512; ii++) {
    out << iRangeMask[ii];
    if (ii != 511) {
      out << " ";
    }
  }
  out << endl;

  out << "  fNoiseDBm: "
      << fNoiseDBm[0] << ", "
      << fNoiseDBm[1] << endl;

  out << "  fNoiseStdvDB: "
      << fNoiseStdvDB[0] << ", "
      << fNoiseStdvDB[1] << endl;

  out << "  fNoiseRangeKM: " << fNoiseRangeKM << endl;
  out << "  fNoisePRFHz: " << fNoisePRFHz << endl;

  out << "  iGparmLatchSts: "
      << iGparmLatchSts[0] << ", "
      << iGparmLatchSts[1] << endl;

  out << "  iGparmImmedSts: "
      << iGparmImmedSts[0] << ", "
      << iGparmImmedSts[1] << ", "
      << iGparmImmedSts[2] << ", "
      << iGparmImmedSts[3] << ", "
      << iGparmImmedSts[4] << ", "
      << iGparmImmedSts[5] << endl;

  out << "  iGparmDiagBits: "
      << iGparmDiagBits[0] << ", "
      << iGparmDiagBits[1] << ", "
      << iGparmDiagBits[2] << ", "
      << iGparmDiagBits[3] << endl;

  out << "  sVersionString: " << sVersionString << endl;

}

///////////////////////////////////////////////////////////
// print the derived info
//

void OpsInfo::_printDerived(ostream &out) const
{
  
  out << "==================== Derived info ==================" << endl;

  out << "  startRange: " << startRange << endl;
  out << "  maxRange: " << maxRange << endl;
  out << "  gateSpacing: " << gateSpacing << endl;

}


