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
// RadarTsInfo.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2007
//
///////////////////////////////////////////////////////////////
//
// Stores current radar ops info
//
////////////////////////////////////////////////////////////////
//
// Class has been deprecated.
// Use IwrfTsInfo instead.
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <cmath>
#include <toolsa/str.h>
#include <radar/RadarTsInfo.hh>
#include <toolsa/TaArray.hh>
#include <rapformats/DsRadarCalib.hh>
using namespace std;

////////////////////////////////////////////////////
// Constructor

RadarTsInfo::RadarTsInfo(RadarTsDebug_t debug) :
        _debug(debug)

{
  
  _derivedFromRvp8 = false;
  ts_ops_info_init(&_info);

}

//////////////////////////////////////////////////////////////////
// destructor

RadarTsInfo::~RadarTsInfo()

{
  
}

///////////////////////////////////////////////////////////
// set from a Ts pulse info buffer

int RadarTsInfo::setFromTsBuffer(const void *buf, int len)
  
{
  
  if (len < (int) sizeof(ts_ops_info_t)) {
    cerr << "ERROR - RadarTsInfo::setFromTsInfo" << endl;
    cerr << "  Buffer passed in too short, len: " << len << endl;
    cerr << "  Must be at least len: " << sizeof(ts_ops_info_t) << endl;
    return -1;
  }
  
  load(*((ts_ops_info_t *) buf));

  return 0;

}

///////////////////////////////////////////////////////////
// load value from TS ops info struct

void RadarTsInfo::load(const ts_ops_info_t &info)
  
{
  
  memcpy(&_info, &info, sizeof(ts_ops_info_t));
  
  if (_info.rvp8.iVersion != 9999) {
    _derivedFromRvp8 = true;
  } else {
    _derivedFromRvp8 = false;
  }

}
  
///////////////////////////////////////////////////////////
// set RVP8-specific fields

void RadarTsInfo::setRvp8Info(const ts_ops_info_t &info,
			      const ts_pulse_hdr_t &pulse)
  
{
  
  if (info.rvp8.iVersion != 9999) {
    return;
  }

  _derivedFromRvp8 = false;

  _info.rvp8.iTaskSweep = pulse.tiltNum;
  
  STRncopy(_info.rvp8.sSiteName, info.siteName, 32);
  _info.rvp8.fPWidthUSec = pulse.pulseWidth / 1000.0;
  _info.rvp8.fDBzCalib = info.calib.baseDbz1kmHc;
  _info.rvp8.fWavelengthCM = info.wavelengthCm;
  
  _info.rvp8.fRangeMaskRes = info.gateSpacingM;
  
  /* iRangeMask is bit mask of bins that have
   * actually been selected at the
   * above spacing. */
  
  memset(_info.rvp8.iRangeMask, 0, TS_GATE_MASK_LEN * sizeof(ui16));
  int one = 1;
  int count = 0;
  int nGates = pulse.nGates;
  for (int ii = 0; ii < TS_GATE_MASK_LEN; ii++) {
    for (int iBit = 0; iBit < 16; iBit++) {
      if (count >= nGates) {
	break;
      }
      int mask = one << iBit;
      _info.rvp8.iRangeMask[ii] |= mask;
      count++;
    }
    if (count >= nGates) {
      break;
    }
  }

  _info.rvp8.fNoiseDBm[0] = info.calib.noiseSourcePowerDbmH;
  _info.rvp8.fNoiseDBm[1] = info.calib.noiseSourcePowerDbmV;

}
  
///////////////////////////////////////////////////////////
// override radar name and site name

void RadarTsInfo::overrideRadarName(const string &radarName)
  
{
  STRncopy(_info.radarName, radarName.c_str(), TS_RADAR_NAME_LEN);
}

void RadarTsInfo::overrideSiteName(const string &siteName)
  
{
  STRncopy(_info.siteName, siteName.c_str(), TS_SITE_NAME_LEN);
}

///////////////////////////////////////////////////////////
// override radar location

void RadarTsInfo::overrideRadarLocation(double altitudeMeters,
                                        double latitudeDeg,
                                        double longitudeDeg)
  
{
  
  _info.altitudeM = altitudeMeters;
  _info.latitudeDeg = latitudeDeg;
  _info.longitudeDeg = longitudeDeg;

}

///////////////////////////////////////////////////////////
// override gate geometry

void RadarTsInfo::overrideGateGeometry(double startRangeMeters,
                                       double gateSpacingMeters)
  
{
  
  _info.startRangeM = startRangeMeters;
  _info.gateSpacingM = gateSpacingMeters; 

}

///////////////////////////////////////////////////////////
// override wavelength

void RadarTsInfo::overrideWavelength(double wavelengthCm)
  
{
  
  _info.wavelengthCm = wavelengthCm;

}

///////////////////////////////////////////////////////////////
// read in pulse info from RVP8 file
//
// Returns 0 on success, -1 on failure

int RadarTsInfo::readFromRvp8File(FILE *in)

{

  if (_readRvp8Info(in)) {
    return -1;
  }

  _deriveRangeFromRvp8Info();

  return 0;

}
  
/////////////////////////////
// write in tsarchive format

int RadarTsInfo::write2TsarchiveFile(FILE *out)

{

  if (out == NULL) {
    return -1;
  }

  fprintf(out, "rvp8PulseInfo start\n");
  fprintf(out, "iVersion=%d\n", _info.rvp8.iVersion);
  fprintf(out, "iMajorMode=%d\n", _info.rvp8.iMajorMode);
  fprintf(out, "iPolarization=%d\n", _info.rvp8.iPolarization);
  fprintf(out, "iPhaseModSeq=%d\n", _info.rvp8.iPhaseModSeq);
  fprintf(out, "taskID.iSweep=%d\n", _info.rvp8.iTaskSweep);
  fprintf(out, "taskID.iAuxNum=%d\n", _info.rvp8.iTaskAuxNum);
  fprintf(out, "taskID.sTaskName=%s\n", _info.rvp8.sTaskName);
  fprintf(out, "sSiteName=%s\n", _info.rvp8.sSiteName);
  fprintf(out, "iAqMode=%d\n", _info.rvp8.iAqMode);
  fprintf(out, "iUnfoldMode=%d\n", _info.rvp8.iUnfoldMode);
  fprintf(out, "iPWidthCode=%d\n", _info.rvp8.iPWidthCode);
  fprintf(out, "fPWidthUSec=%g\n", _info.rvp8.fPWidthUSec);
  fprintf(out, "fDBzCalib=%g\n", _info.rvp8.fDBzCalib);
  fprintf(out, "iSampleSize=%d\n", _info.rvp8.iSampleSize);
  fprintf(out, "iMeanAngleSync=%d\n", _info.rvp8.iMeanAngleSync);
  fprintf(out, "iFlags=%d\n", _info.rvp8.iFlags);
  fprintf(out, "iPlaybackVersion=%d\n", _info.rvp8.iPlaybackVersion);
  fprintf(out, "fSyClkMHz=%g\n", _info.rvp8.fSyClkMHz);
  fprintf(out, "fWavelengthCM=%g\n", _info.rvp8.fWavelengthCM);
  fprintf(out, "fSaturationDBM=%g\n", _info.rvp8.fSaturationDBM);
  fprintf(out, "fRangeMaskRes=%g\n", _info.rvp8.fRangeMaskRes);

  fprintf(out, "iRangeMask=");
  for (int ii = 0; ii < TS_GATE_MASK_LEN; ii++) {
    fprintf(out, "%d", _info.rvp8.iRangeMask[ii]);
    if (ii != TS_GATE_MASK_LEN - 1) {
      fprintf(out, " ");
    }
  }
  fprintf(out, "\n");
  
  fprintf(out, "fNoiseDBm=%g %g\n",
          _info.rvp8.fNoiseDBm[0],
          _info.rvp8.fNoiseDBm[1]);
  fprintf(out, "fNoiseStdvDB=%g %g\n",
          _info.rvp8.fNoiseStdvDB[0],
          _info.rvp8.fNoiseStdvDB[1]);
  
  fprintf(out, "fNoiseRangeKM=%g\n", _info.rvp8.fNoiseRangeKM);
  fprintf(out, "fNoisePRFHz=%g\n", _info.rvp8.fNoisePRFHz);

  fprintf(out, "iGparmLatchSts=%d %d\n",
          _info.rvp8.iGparmLatchSts[0],
          _info.rvp8.iGparmLatchSts[1]);

  fprintf(out, "iGparmImmedSts=%d %d %d %d %d %d\n",
          _info.rvp8.iGparmImmedSts[0],
          _info.rvp8.iGparmImmedSts[1],
          _info.rvp8.iGparmImmedSts[2],
          _info.rvp8.iGparmImmedSts[3],
          _info.rvp8.iGparmImmedSts[4],
          _info.rvp8.iGparmImmedSts[5]);

  fprintf(out, "iGparmDiagBits=%d %d %d %d\n",
          _info.rvp8.iGparmDiagBits[0],
          _info.rvp8.iGparmDiagBits[1],
          _info.rvp8.iGparmDiagBits[2],
          _info.rvp8.iGparmDiagBits[3]);

  fprintf(out, "sVersionString=%s\n", _info.rvp8.sVersionString);

  fprintf(out, "rvp8PulseInfo end\n");

  return 0;

}

///////////////////////////////////////////////////////////////
// read in pulse info from RVP8 file
//
// Returns 0 on success, -1 on failure

int RadarTsInfo::_readRvp8Info(FILE *in)

{

  _derivedFromRvp8 = true;

  // initialize to missing values
  
  memset(&_info, 0, sizeof(_info));
  ds_radar_calib_init(&_info.calib); 

  _info.rvp8.iVersion = 999;
  _info.rvp8.iMajorMode = 999;
  _info.rvp8.iPolarization = 999;
  _info.rvp8.iPhaseModSeq = 999;
  _info.rvp8.iTaskSweep = 999;

  _info.rvp8.iTaskAuxNum = 999;
  _info.rvp8.sTaskName[0] = '\0';
  _info.rvp8.sSiteName[0] = '\0';
  _info.rvp8.iAqMode = 999;
  _info.rvp8.iUnfoldMode = 999;
  _info.rvp8.iPWidthCode = 999;
  _info.rvp8.fPWidthUSec = -999.9;
  _info.rvp8.fDBzCalib = -999.9;
  _info.rvp8.iSampleSize = 999;
  _info.rvp8.iMeanAngleSync = 999;
  _info.rvp8.iFlags = 999;
  _info.rvp8.iPlaybackVersion = 999;
  _info.rvp8.fSyClkMHz = -999.9;
  _info.rvp8.fWavelengthCM = -999.9;
  _info.rvp8.fSaturationDBM = -999.9;
  _info.rvp8.fRangeMaskRes = -999.9;
  for (int ii = 0; ii < TS_GATE_MASK_LEN; ii++) {
    _info.rvp8.iRangeMask[ii] = 0;
  }
  for (int ii = 0; ii < 2; ii++) {
    _info.rvp8.fNoiseDBm[ii] = -999.9;
    _info.rvp8.fNoiseStdvDB[ii] = -999.9;
  }
  _info.rvp8.fNoiseRangeKM = -999.9;
  _info.rvp8.fNoisePRFHz = -999.9;
  for (int ii = 0; ii < 2; ii++) {
    _info.rvp8.iGparmLatchSts[ii] = 999;
  }
  for (int ii = 0; ii < 6; ii++) {
    _info.rvp8.iGparmImmedSts[ii] = 999;
  }
  for (int ii = 0; ii < 4; ii++) {
    _info.rvp8.iGparmDiagBits[ii] = 999;
  }
  _info.rvp8.sVersionString[0] = '\0';

  bool taskNameFound = false;
  bool siteNameFound = false;
  bool versionStringFound = false;
  bool rangeMaskFound = false;
  bool iVersionFound = false;
  bool iMajorModeFound = false;
  bool iPolarizationFound = false;
  bool iPhaseModSeqFound = false;
  bool iTaskSweepFound = false;
  bool iTaskAuxNumFound = false;
  bool iAqModeFound = false;
  bool iUnfoldModeFound = false;
  bool iPWidthCodeFound = false;
  bool fPWidthUSecFound = false;
  bool fDBzCalibFound = false;
  bool iSampleSizeFound = false;
  bool iMeanAngleSyncFound = false;
  bool iFlagsFound = false;
  bool iPlaybackVersionFound = false;
  bool fSyClkMHzFound = false;
  bool fWavelengthCMFound = false;
  bool fSaturationDBMFound = false;
  bool fRangeMaskResFound = false;
  bool fNoiseDBmFound = false;
  bool fNoiseStdvDBFound = false;
  bool fNoiseRangeKMFound = false;
  bool fNoisePRFHzFound = false;
  bool iGparmLatchStsFound = false;
  bool iGparmImmedStsFound = false;
  bool iGparmDiagBitsFound = false;

  // find the start of the info header
  
  if (findNextStr(in, "PulseInfo start")) {
    return -1;
  }
    
  // read in info header

  char line[8192];

  while (!feof(in)) {

    if (fgets(line, BUFSIZ, in) == NULL) {
      return -1;
    }
    
    if (false) {
      char shortLine[80];
      memcpy(shortLine, line, 79);
      shortLine[79] = '\0';
      cerr << shortLine;
    }
    
    if (strstr(line, "rvp8PulseInfo end")) {
      return 0;
    }

    char name[80];
    int ival, jval[6];
    double dval, eval[2];


    if (!iVersionFound && sscanf(line, "iVersion=%d", &ival) == 1) {
      _info.rvp8.iVersion = ival;
      iVersionFound = true;
      continue;
    }

    if (!iMajorModeFound && sscanf(line, "iMajorMode=%d", &ival) == 1) {
      _info.rvp8.iMajorMode = ival;
      iMajorModeFound = true;
      continue;
    }

    if (!iPolarizationFound && sscanf(line, "iPolarization=%d", &ival) == 1) {
      _info.rvp8.iPolarization = ival;
      iPolarizationFound = true;
      continue;
    }

    if (!iPhaseModSeqFound && sscanf(line, "iPhaseModSeq=%d", &ival) == 1) {
      _info.rvp8.iPhaseModSeq = ival;
      iPhaseModSeqFound = true;
      continue;
    }

    if (!iTaskSweepFound && sscanf(line, "taskID.iSweep=%d", &ival) == 1) {
      _info.rvp8.iTaskSweep = ival;
      iTaskSweepFound = true;
      continue;
    }

    if (!iTaskAuxNumFound && sscanf(line, "taskID.iAuxNum=%d", &ival) == 1) {
      _info.rvp8.iTaskAuxNum = ival;
      iTaskAuxNumFound = true;
      continue;
    }
    if (!taskNameFound && strlen(line) < 80 &&
	sscanf(line, "taskID.sTaskName=%s", name) == 1) {
      STRncopy(_info.rvp8.sTaskName, name, sizeof(_info.rvp8.sTaskName));
      taskNameFound = true;
      continue;
    }
    if (!siteNameFound && strlen(line) < 80 &&
	sscanf(line, "sSiteName=%s", name) == 1) {
      STRncopy(_info.rvp8.sSiteName, name, sizeof(_info.rvp8.sTaskName));
      siteNameFound = true;
    }

    if (!iAqModeFound && sscanf(line, "iAqMode=%d", &ival) == 1) {
      _info.rvp8.iAqMode = ival;
      iAqModeFound = true;
      continue;
    }

    if (!iUnfoldModeFound && sscanf(line, "iUnfoldMode=%d", &ival) == 1) {
      _info.rvp8.iUnfoldMode = ival;
      iUnfoldModeFound = true;
      continue;
    }

    if (!iPWidthCodeFound && sscanf(line, "iPWidthCode=%d", &ival) == 1) {
      _info.rvp8.iPWidthCode = ival;
      iPWidthCodeFound = true;
      continue;
    }

    if (!fPWidthUSecFound && sscanf(line, "fPWidthUSec=%lg", &dval) == 1) {
      _info.rvp8.fPWidthUSec = dval;
      fPWidthUSecFound = true;
      continue;
    }

    if (!fDBzCalibFound && sscanf(line, "fDBzCalib=%lg", &dval) == 1) {
      _info.rvp8.fDBzCalib = dval;
      fDBzCalibFound = true;
      continue;
    }

    if (!iSampleSizeFound && sscanf(line, "iSampleSize=%d", &ival) == 1) {
      _info.rvp8.iSampleSize = ival;
      iSampleSizeFound = true;
      continue;
    }

    if (!iMeanAngleSyncFound && sscanf(line, "iMeanAngleSync=%d", &ival) == 1) {
      _info.rvp8.iMeanAngleSync = ival;
      iMeanAngleSyncFound = true;
      continue;
    }

    if (!iFlagsFound && sscanf(line, "iFlags=%d", &ival) == 1) {
      _info.rvp8.iFlags = ival;
      iFlagsFound = true;
      continue;
    }

    if (!iPlaybackVersionFound && sscanf(line, "iPlaybackVersion=%d", &ival) == 1) {
      _info.rvp8.iPlaybackVersion = ival;
      iPlaybackVersionFound = true;
      continue;
    }

    if (!fSyClkMHzFound && sscanf(line, "fSyClkMHz=%lg", &dval) == 1) {
      _info.rvp8.fSyClkMHz = dval;
      fSyClkMHzFound = true;
      continue;
    }

    if (!fWavelengthCMFound && sscanf(line, "fWavelengthCM=%lg", &dval) == 1) {
      _info.rvp8.fWavelengthCM = dval;
      _info.wavelengthCm = dval;
      fWavelengthCMFound = true;
      continue;
    }

    if (!fSaturationDBMFound && sscanf(line, "fSaturationDBM=%lg", &dval) == 1) {
      _info.rvp8.fSaturationDBM = dval;
      fSaturationDBMFound = true;
      continue;
    }
    
    if (!fRangeMaskResFound && sscanf(line, "fRangeMaskRes=%lg", &dval) == 1) {
      _info.rvp8.fRangeMaskRes = dval;
      fRangeMaskResFound = true;
      continue;
    }

    if (!rangeMaskFound && strstr(line, "iRangeMask=") != NULL) {
      char *toks = strtok(line, "= ");
      int count = 0;
      while (toks != NULL && count < TS_GATE_MASK_LEN) {
        toks = strtok(NULL, "= ");
        if (toks != NULL) {
          if (sscanf(toks, "%d", &ival) == 1) {
            _info.rvp8.iRangeMask[count] = ival;
          }
          count++;
        }
      }
      rangeMaskFound = true;
      continue;
    }


    if (!fNoiseDBmFound &&
        sscanf(line, "fNoiseDBm=%lg %lg", &eval[0], &eval[1]) == 2) {
      _info.rvp8.fNoiseDBm[0] = eval[0];
      _info.rvp8.fNoiseDBm[1] = eval[1];
      fNoiseDBmFound = true;
      continue;
    }


    if (!fNoiseStdvDBFound &&
        sscanf(line, "fNoiseStdvDB=%lg %lg", &eval[0], &eval[1]) == 2) {
      _info.rvp8.fNoiseStdvDB[0] = eval[0];
      _info.rvp8.fNoiseStdvDB[1] = eval[1];
      fNoiseStdvDBFound = true;
      continue;
    }


    if (!fNoiseRangeKMFound && sscanf(line, "fNoiseRangeKM=%lg", &dval) == 1) {
      _info.rvp8.fNoiseRangeKM = dval;
      fNoiseRangeKMFound = true;
      continue;
    }


    if (!fNoisePRFHzFound && sscanf(line, "fNoisePRFHz=%lg", &dval) == 1) {
      _info.rvp8.fNoisePRFHz = dval;
      fNoisePRFHzFound = true;
      continue;
    }
    

    if (!iGparmLatchStsFound &&
        sscanf(line, "iGparmLatchSts=%d %d", &jval[0], &jval[1]) == 2) {
      _info.rvp8.iGparmLatchSts[0] = jval[0];
      _info.rvp8.iGparmLatchSts[1] = jval[1];
      iGparmLatchStsFound = true;
      continue;
    }


    if (!iGparmImmedStsFound &&
        sscanf(line, "iGparmImmedSts=%d %d %d %d %d %d",
               &jval[0], &jval[1], &jval[2],
               &jval[3], &jval[4], &jval[5]) == 6) {
      _info.rvp8.iGparmImmedSts[0] = jval[0];
      _info.rvp8.iGparmImmedSts[1] = jval[1];
      _info.rvp8.iGparmImmedSts[2] = jval[2];
      _info.rvp8.iGparmImmedSts[3] = jval[3];
      _info.rvp8.iGparmImmedSts[4] = jval[4];
      _info.rvp8.iGparmImmedSts[5] = jval[5];
      iGparmImmedStsFound = true;
      continue;
    }


    if (!iGparmDiagBitsFound &&
        sscanf(line, "iGparmDiagBits=%d %d %d %d",
               &jval[0], &jval[1],
               &jval[2], &jval[3]) == 4) {
      _info.rvp8.iGparmDiagBits[0] = jval[0];
      _info.rvp8.iGparmDiagBits[1] = jval[1];
      _info.rvp8.iGparmDiagBits[2] = jval[2];
      _info.rvp8.iGparmDiagBits[3] = jval[3];
      iGparmDiagBitsFound = true;
      continue;
    }

    if (!versionStringFound && strlen(line) < 80 &&
	sscanf(line, "sVersionString=%s", name) == 1) {
      STRncopy(_info.rvp8.sVersionString, name, sizeof(_info.rvp8.sVersionString));
      continue;
    }
    
  } // while

  return 0;

}

///////////////////////////////////////////////////////////////
// derive quantities from RVP8 info

void RadarTsInfo::_deriveRangeFromRvp8Info()

{

  // Find first, last, and total number of range bins in the mask
  // Based on SIGMET code in tsview.c

  int binCount = 0;
  int binStart = 0;
  int binEnd = 0;

  for (int ii = 0; ii < TS_GATE_MASK_LEN; ii++) {
    ui16 mask = _info.rvp8.iRangeMask[ii];
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
  
  double startRangeM = (binStart * _info.rvp8.fRangeMaskRes);
  double maxRangeM = (binEnd * _info.rvp8.fRangeMaskRes);
  double gateSpacingM = (maxRangeM - _info.startRangeM) / (binCount - 1.0);
  // pulse centered on PRT boundary, and first gate holds burst
  startRangeM += gateSpacingM; 
  
  _info.startRangeM = startRangeM;
  _info.gateSpacingM = gateSpacingM;

}

///////////////////////////////////////////////////////////
// print

void RadarTsInfo::print(ostream &out) const
{

  out << "==================== Pulse info ==================" << endl;

  out << "altitudeM:" << _info.altitudeM << endl;
  out << "latitudeDeg:" << _info.latitudeDeg << endl;
  out << "longitudeDeg:" << _info.longitudeDeg << endl;
  out << "startRangeM:" << _info.startRangeM << endl;
  out << "gateSpacingM:" << _info.gateSpacingM << endl;
  out << "scanMode:" << _info.scanMode << endl;
  out << "xmitRcvMode:" << _info.xmitRcvMode << endl;
  out << "prfMode:" << _info.prfMode << endl;
  out << "xmitPhaseMode:" << _info.xmitPhaseMode << endl;
  out << "wavelengthCm:" << _info.wavelengthCm << endl;
  out << "beamWidthDegH:" << _info.beamWidthDegH << endl;
  out << "beamWidthDegV:" << _info.beamWidthDegV << endl;
  out << "radarName:" << _info.radarName << endl;
  out << "siteName:" << _info.siteName << endl;
  
  // Calibration

  DsRadarCalib calib;
  calib.set(_info.calib);
  calib.print(out);

  // RVP8

  out << "-------------------- RVP8 section ------------------" << endl;

  out << "iVersion: " << _info.rvp8.iVersion << endl;
  out << "iMajorMode: " << _info.rvp8.iMajorMode << endl;
  out << "iPolarization: " << _info.rvp8.iPolarization << endl;
  out << "iPhaseModSeq: " << _info.rvp8.iPhaseModSeq << endl;
  out << "iTaskSweep: " << _info.rvp8.iTaskSweep << endl;
  out << "iTaskAuxNum: " << _info.rvp8.iTaskAuxNum << endl;
  out << "sTaskName: " << _info.rvp8.sTaskName << endl;
  out << "sSiteName: " << _info.rvp8.sSiteName << endl;
  out << "iAqMode: " << _info.rvp8.iAqMode << endl;
  out << "iUnfoldMode: " << _info.rvp8.iUnfoldMode << endl;
  out << "iPWidthCode: " << _info.rvp8.iPWidthCode << endl;
  out << "fPWidthUSec: " << _info.rvp8.fPWidthUSec << endl;
  out << "fDBzCalib: " << _info.rvp8.fDBzCalib << endl;
  out << "iSampleSize: " << _info.rvp8.iSampleSize << endl;
  out << "iMeanAngleSync: " << _info.rvp8.iMeanAngleSync << endl;
  out << "iFlags: " << _info.rvp8.iFlags << endl;
  out << "iPlaybackVersion: " << _info.rvp8.iPlaybackVersion << endl;
  out << "fSyClkMHz: " << _info.rvp8.fSyClkMHz << endl;
  out << "fWavelengthCM: " << _info.rvp8.fWavelengthCM << endl;
  out << "fSaturationDBM: " << _info.rvp8.fSaturationDBM << endl;
  out << "fRangeMaskRes: " << _info.rvp8.fRangeMaskRes << endl;
  out << "iRangeMask: ";

  for (int ii = 0; ii < TS_GATE_MASK_LEN; ii++) {
    out << _info.rvp8.iRangeMask[ii];
    if (ii != 511) {
      out << " ";
    }
  }
  out << endl;

  out << "fNoiseDBm: "
      << _info.rvp8.fNoiseDBm[0] << ", "
      << _info.rvp8.fNoiseDBm[1] << endl;

  out << "fNoiseStdvDB: "
      << _info.rvp8.fNoiseStdvDB[0] << ", "
      << _info.rvp8.fNoiseStdvDB[1] << endl;

  out << "fNoiseRangeKM: " << _info.rvp8.fNoiseRangeKM << endl;
  out << "fNoisePRFHz: " << _info.rvp8.fNoisePRFHz << endl;

  out << "iGparmLatchSts: "
      << _info.rvp8.iGparmLatchSts[0] << ", "
      << _info.rvp8.iGparmLatchSts[1] << endl;

  out << "iGparmImmedSts: "
      << _info.rvp8.iGparmImmedSts[0] << ", "
      << _info.rvp8.iGparmImmedSts[1] << ", "
      << _info.rvp8.iGparmImmedSts[2] << ", "
      << _info.rvp8.iGparmImmedSts[3] << ", "
      << _info.rvp8.iGparmImmedSts[4] << ", "
      << _info.rvp8.iGparmImmedSts[5] << endl;

  out << "iGparmDiagBits: "
      << _info.rvp8.iGparmDiagBits[0] << ", "
      << _info.rvp8.iGparmDiagBits[1] << ", "
      << _info.rvp8.iGparmDiagBits[2] << ", "
      << _info.rvp8.iGparmDiagBits[3] << endl;

  out << "sVersionString: " << _info.rvp8.sVersionString << endl;

}


///////////////////////////////////////////////////////////////
// find search string in data
//
// Returns 0 on succes, -1 on failure (EOF)

int RadarTsInfo::findNextStr(FILE *in,
                             const char *searchStr)
  
{
  
  // const char *searchStr = "rvp8PulseHdr start";

  int searchLen = strlen(searchStr);

  // create buffer for incoming characters
  
  TaArray<char> cbuf_;
  char *cbuf = cbuf_.alloc(searchLen);
  memset(cbuf, 0, searchLen);
  
  while (!feof(in)) {
  
    // read in a character

    int cc = fgetc(in);
    if (cc == EOF) {
      return -1;
    }
    
    // move the characters along in the buffer

    memmove(cbuf + 1, cbuf, searchLen - 1);

    // store the latest character

    cbuf[0] = cc;
    
    // test for the search string

    bool match = true;
    for (int ii = 0, jj = searchLen - 1; ii < searchLen; ii++, jj--) {
      if (searchStr[ii] != cbuf[jj]) {
        match = false;
        break;
      }
    }

    if (match) {
      return 0;
    }

  } // while

  return -1;

}

