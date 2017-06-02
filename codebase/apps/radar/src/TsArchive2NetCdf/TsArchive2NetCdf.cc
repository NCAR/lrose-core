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
// TsArchive2NetCdf.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2005
//
///////////////////////////////////////////////////////////////
//
// TsArchive2NetCdf reads raw LIRP IQ files and converts them
// to NetDCF
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <Ncxx/Nc3File.hh>
#include "TsArchive2NetCdf.hh"
#include "InputPath.hh"

#define MAX_PATH_LEN 1024
#define PATH_DELIM '/'

using namespace std;

// Constructor

TsArchive2NetCdf::TsArchive2NetCdf(int argc, char **argv)
  
{

  _input = NULL;
  isOK = true;
  
  // set programe name
  
  _progName = "TsArchive2NetCdf";

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  return;
  
}

// destructor

TsArchive2NetCdf::~TsArchive2NetCdf()

{

  if (_input) {
    delete _input;
  }

}

//////////////////////////////////////////////////
// Run

int TsArchive2NetCdf::Run ()
{

  // initialize the data input object
  
  if (_args.realtime) {
    _input = new InputPath(_args.inDir, 0);
  } else {
    _input = new InputPath(_args.inputFileList);
  }
  if (_args.verbose) {
    _input->setDebug(TRUE);
  }

  int iret = 0;

  if (_args.debug) {
    cerr << "Running TsArchive2NetCdf - debug mode" << endl;
  }

  // loop until end of data
  
  char *inputPath;
  while ((inputPath = _input->next()) != NULL) {
    
    if (_processFile(inputPath)) {
      cerr << "ERROR = TsArchive2NetCdf::Run" << endl;
      cerr << "  Processing file: " << inputPath << endl;
      iret = -1;
    }
    
  }
  
  return iret;

}

///////////////////////////////
// process file

int TsArchive2NetCdf::_processFile(const char *input_path)

{
  
  if (_args.debug) {
    cerr << "Processing file: " << input_path << endl;
  }
  
  // open file
  
  FILE *in;
  if ((in = fopen(input_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - TsArchive2NetCdf::_processFile" << endl;
    cerr << "  File: " << input_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // read in pulse header
  
  if (_readPulseInfo(in)) {
    cerr << "ERROR - TsArchive2NetCdf::_processFile" << endl;
    cerr << "  Cannot read pulse info" << endl;
    cerr << "  File: " << input_path << endl;
    fclose(in);
    return -1;
  }
  
  if (_args.verbose2) {
    _printPulseInfo(cerr);
  }
  
  // read in data, load up vectors
  
  vector<iq_t> iqVec;
  vector<float> elVec, azVec;
  vector<double> timeVec;
  vector<int> prtVec;
  vector<float> modCodeVec;
  
  _nGatesSave = -1;
  _nGatesFirst = -1;
  _startTime = 0;
  _startAz = 0.0;
  double sumEl = 0.0;

  while (!feof(in)) {
    
    // read in pulse header

    if (_readPulseHeader(in)) {
      continue;
    }
    
    // derive quantities from pulse header

    _deriveFromPulseHeader();

    if (_args.verbose2) {
      _printPulseHeader(cerr);
    }
    
    if (!(_headerFlags & PHDRFLG_VALID)) {
      continue;
    }

    // set number of gates to be saved out, based on whether saveSecond
    // is true or false

    if (_nGatesFirst < 0) {
      _nGatesFirst = _nGates;
    }
    if (_args.saveSecond) {
      if (_nGatesSave < 0 && _nGates != _nGatesFirst) {
	_nGatesSave = _nGates;
	if (_args.debug) {
	  cerr << "--> INFO: only saving pulses with nGates: "
	       << _nGatesSave << endl;
	}
      }
    } else {
      if (_nGatesSave < 0) {
	_nGatesSave = _nGates;
	if (_args.debug) {
	  cerr << "--> INFO: only saving pulses with nGates: "
	       << _nGatesSave << endl;
	}
      }
    }

    if (_startTime == 0) {
      _startTime = _pulseTimeSecs;
      _startAz = _az;
    }
    sumEl += _el;
    
    // read in iq data
    
    int nVals = _nGates * 2;
    UINT2 packed[nVals];
    if (fread(packed, sizeof(UINT2), nVals, in) != nVals) {
      continue;
    }
    
    // convert packed data to floats
    
    FLT4 unpacked[nVals];
    bool highSnrPack = true;
    _vecFloatIQFromPackIQ(unpacked, packed, nVals, (UINT1) highSnrPack);
    
    // load up data

    if (_nGates == _nGatesSave) {

      timeVec.push_back(_pulseTime);
      prtVec.push_back(_iprt);
      elVec.push_back(_el);
      azVec.push_back(_az);
      modCodeVec.push_back((float)_phaseDiff);
      
      FLT4 *iqp = unpacked;
      for (int igate = 0; igate < _nGates; igate++) {
	iq_t iq;
	iq.i = *iqp;
	iqp++;
	iq.q = *iqp;
	iqp++;
	iqVec.push_back(iq);
      } // igate

      if (_args.verbose) {
	cerr << "Using pulse, time, _iprt, _el, _az, _nGates: "
	     << _pulseTime << " "
	     << _iprt << " "
	     << _el << " "
	     << _az << " "
	     << _nGates << endl;
      }
    
    } else {

      if (_args.verbose) {
	cerr << "Skipping pulse, time, _iprt, _el, _az, _nGates: "
	     << _pulseTime << " "
	     << _iprt << " "
	     << _el << " "
	     << _az << " "
	     << _nGates << endl;
      }

    }

  } // while

  fclose(in);
  
  _elevation = sumEl / elVec.size();

  // compute output and tmp paths
  
  string outName;
  const char *lastDelim = strrchr(input_path, PATH_DELIM);
  if (lastDelim) {
    outName = lastDelim + 1;
  } else {
    outName = input_path;
  }
  if (_args.saveSecond) {
    outName += ".second";
  }
  outName += ".nc";
  string outPath = _args.outDir;
  outPath += PATH_DELIM;
  outPath += outName;
  string tmpPath = outPath;
  tmpPath += ".tmp";
  
  if (_args.debug) {
    cerr << "Output file path: " << outPath << endl;
  }

  // write out tmp file
  
  if (_writeTmpFile(tmpPath, iqVec, elVec, azVec,
		    prtVec, timeVec, modCodeVec)) {
    cerr << "ERROR - TsArchive2NetCdf::_processFile" << endl;
    cerr << "  Cannot write tmp file: " << tmpPath << endl;
    return -1;
  }
  
  if (_args.debug) {
    cerr << "  Wrote tmp file: " << tmpPath << endl;
  }

  // move the tmp file to final name
  
  if (rename(tmpPath.c_str(), outPath.c_str())) {
    int errNum = errno;
    cerr << "ERROR - TsArchive2NetCdf::_processFile" << endl;
    cerr << "  Cannot rename file: " << tmpPath << endl;
    cerr << "             to file: " << outPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if (_args.debug) {
    cerr << "  Renamed file: " << tmpPath << endl;
    cerr << "       to file: " << outPath << endl;
  }

  return 0;

}

///////////////////////////////////////////////////////////////
// read in pulse info
//
// Returns 0 on success, -1 on failure

int TsArchive2NetCdf::_readPulseInfo(FILE *in)

{

  if (_args.verbose2) {
    cerr << "--->> Reading Pulse Info <<-----" << endl;
  }

  bool inPulseInfo = false;
  char line[BUFSIZ];

  while (!feof(in)) {

    if (fgets(line, BUFSIZ, in) == NULL) {
      return -1;
    }
    
    if (_args.verbose2) {
      if (strlen(line) < 1024) {
  	cerr << line;
      }
    }

    if (strstr(line, "rvp8PulseInfo start")) {
      inPulseInfo = true;
    }

    if (strstr(line, "rvp8PulseInfo end")) {
      if (inPulseInfo) {
	return 0;
      } else {
	return -1;
      }
    }

    // look for strings

    char name[_maxNameLen];

    if (strlen(line) < _maxNameLen &&
	sscanf(line, "sVersionString=%s", name) == 1) {
      _versionStr = name;
    }
    
    if (strlen(line) < _maxNameLen &&
	sscanf(line, "taskID.sTaskName=%s", name) == 1) {
      _taskName = name;
    }
    
    if (strlen(line) < _maxNameLen &&
	sscanf(line, "sSiteName=%s", name) == 1) {
      _siteName = name;
    }

    // look for integers

    int ival;
    if (sscanf(line, "iVersion=%d", &ival) == 1) {
      _infoVersionNum = ival;
    }
    if (sscanf(line, "iFlags=%d", &ival) == 1) {
      _infoFlags = ival;
    }
    if (sscanf(line, "iSampleSize=%d", &ival) == 1) {
      _sampleSize = ival;
    }
    if (sscanf(line, "iPolarization=%d", &ival) == 1) {
      _polarization = ival;
    }
    if (sscanf(line, "iMajorMode=%d", &ival) == 1) {
      _majorMode = ival;
    }
    if (sscanf(line, "taskID.iSweep=%d", &ival) == 1) {
      _sweepNum = ival;
    }

    // look for floats

    double dval;
    if (sscanf(line, "fPWidthUSec=%lg", &dval) == 1) {
      _pulseWidthUsec = dval;
    }
    if (sscanf(line, "fDBzCalib=%lg", &dval) == 1) {
      _dbzCalib = dval;
    }
    if (sscanf(line, "fSyClkMHz=%lg", &dval) == 1) {
      _syClkMhz = dval;
    }
    if (sscanf(line, "fWavelengthCM=%lg", &dval) == 1) {
      _wavelengthCm = dval;
    }
    if (sscanf(line, "fRangeMaskRes=%lg", &dval) == 1) {
      _rangeMaskRes = dval;
    }
    if (sscanf(line, "fSaturationDBM=%lg", &dval) == 1) {
      _saturationDbm = dval;
    }
    if (sscanf(line, "fNoiseDBm=%lg", &dval) == 1) {
      _noiseDbm = dval;
    }
    if (sscanf(line, "fNoiseRangeKM=%lg", &dval) == 1) {
      _noiseRangeKm = dval;
    }
    if (sscanf(line, "fNoisePRFHz=%lg", &dval) == 1) {
      _noisePrf = dval;
    }

  } // while

}

  
///////////////////////////////////////////////////////////////
// read in pulse header
//
// Returns 0 on success, -1 on failure

int TsArchive2NetCdf::_readPulseHeader(FILE *in)

{

  if (_args.verbose2) {
    cerr << "--->> Reading Pulse Header <<-----" << endl;
  }

  bool inPulseHeader = false;
  char line[BUFSIZ];

  while (!feof(in)) {

    if (fgets(line, BUFSIZ, in) == NULL) {
      return -1;
    }

    if (_args.verbose2) {
      if (strlen(line) < 1024) {
  	cerr << line;
      }
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

    int ival;
    if (sscanf(line, "iVersion=%d", &ival) == 1) {
      _headerVersionNum = ival;
    }
    if (sscanf(line, "iFlags=%d", &ival) == 1) {
      _headerFlags = ival;
    }
    if (sscanf(line, "iMSecUTC=%d", &ival) == 1) {
      _msecUTC = ival;
    }
    if (sscanf(line, "iTimeUTC=%d", &ival) == 1) {
      _timeUTC = ival;
    }
    if (sscanf(line, "iBtimeAPI=%d", &ival) == 1) {
      _btimeAPI = ival;
    }
    if (sscanf(line, "iSysTime=%d", &ival) == 1) {
      _sysTime = ival;
    }
    if (sscanf(line, "iPrevPRT=%d", &ival) == 1) {
      _prevPRT = ival;
    }
    if (sscanf(line, "iNextPRT=%d", &ival) == 1) {
      _nextPRT = ival;
    }
    if (sscanf(line, "iSeqNum=%d", &ival) == 1) {
      _seqNum = ival;
    }
    if (sscanf(line, "iAqMode=%d", &ival) == 1) {
      _aqMode = ival;
    }
    if (sscanf(line, "iPolarBits=%d", &ival) == 1) {
      _polarBits = ival;
    }
    if (sscanf(line, "iTxPhase=%d", &ival) == 1) {
      _txPhase = ival;
    }
    if (sscanf(line, "iAz=%d", &ival) == 1) {
      _iaz = ival;
    }
    if (sscanf(line, "iEl=%d", &ival) == 1) {
      _iel = ival;
    }
    if (sscanf(line, "iNumVecs=%d", &ival) == 1) {
      _numVecs = ival;
    }
    if (sscanf(line, "iMaxVecs=%d", &ival) == 1) {
      _maxVecs = ival;
    }
    if (sscanf(line, "iVIQPerBin=%d", &ival) == 1) {
      _vIQPerBin = ival;
    }
    if (sscanf(line, "iTgBank=%d", &ival) == 1) {
      _tgBank = ival;
    }
    if (sscanf(line, "iTgWave=%d", &ival) == 1) {
      _tgWave = ival;
    }

    // look for floats
    
    double dval;
    if (sscanf(line, "RX[0].fBurstMag=%lg", &dval) == 1) {
      _burstMag0 = dval;
    }
    if (sscanf(line, "RX[0].fBurstArg=%lg", &dval) == 1) {
      _burstArg0 = dval;
    }
    if (sscanf(line, "RX[1].fBurstMag=%lg", &dval) == 1) {
      _burstMag1 = dval;
    }
    if (sscanf(line, "RX[1].fBurstArg=%lg", &dval) == 1) {
      _burstArg1 = dval;
    }

  } // while

}

///////////////////////////////////////////////////////////////
// derive quantities from pulse header

void TsArchive2NetCdf::_deriveFromPulseHeader()

{

  _pulseTimeSecs = _timeUTC;
  _pulseTime = (double) _timeUTC + _msecUTC / 1000.0;
  _nGates = _numVecs; 
  _az = (_iaz / 65535.0) * 360.0;
  _el = (_iel / 65535.0) * 360.0;

  if (_nextPRT == 0) {
    _prt = 1000.0;
  } else {
    _prt = _nextPRT / _syClkMhz;
  }
  _iprt = (int) (_prt + 0.5);

  _phaseDiff = (_burstArg0 / 65536.0) * 360.0;

}

  
///////////////////////////////////////////////////////////
// print the pulse header
//

void TsArchive2NetCdf::_printPulseInfo(ostream &out) const
{

  out << "==================== Pulse info ==================" << endl;
  out << "  _versionStr: " << _versionStr << endl;
  out << "  _taskName: " << _taskName << endl;
  out << "  _siteName: " << _siteName << endl;
  out << "  _infoVersionNum: " << _infoVersionNum << endl;
  out << "  _infoFlags: " << _infoFlags << endl;
  out << "  _sampleSize: " << _sampleSize << endl;
  out << "  _polarization: " << _polarization << endl;
  out << "  _majorMode: " << _majorMode << endl;
  out << "  _sweepNum: " << _sweepNum << endl;
  out << "  _pulseWidthUsec: " << _pulseWidthUsec << endl;
  out << "  _dbzCalib: " << _dbzCalib << endl;
  out << "  _syClkMhz: " << _syClkMhz << endl;
  out << "  _wavelengthCm: " << _wavelengthCm << endl;
  out << "  _rangeMaskRes: " << _rangeMaskRes << endl;
  out << "  _saturationDbm: " << _saturationDbm << endl;
  out << "  _noiseDbm: " << _noiseDbm << endl;
  out << "  _noiseRangeKm: " << _noiseRangeKm << endl;
  out << "  _noisePrf: " << _noisePrf << endl;

}

///////////////////////////////////////////////////////////
// print the pulse header
//

void TsArchive2NetCdf::_printPulseHeader(ostream &out) const
{

  out << "==================== Pulse header ==================" << endl;
  out << "  _headerVersionNum: " << _headerVersionNum << endl;
  out << "  _headerFlags: " << _headerFlags << endl;
  out << "  _msecUTC: " << _msecUTC << endl;
  out << "  _timeUTC: " << _timeUTC << endl;
  out << "  _btimeAPI: " << _btimeAPI << endl;
  out << "  _sysTime: " << _sysTime << endl;
  out << "  _prevPRT: " << _prevPRT << endl;
  out << "  _nextPRT: " << _nextPRT << endl;
  out << "  _seqNum: " << _seqNum << endl;
  out << "  _aqMode: " << _aqMode << endl;
  out << "  _polarBits: " << _polarBits << endl;
  out << "  _txPhase: " << _txPhase << endl;
  out << "  _iaz: " << _iaz << endl;
  out << "  _iel: " << _iel << endl;
  out << "  _numVecs: " << _numVecs << endl;
  out << "  _maxVecs: " << _maxVecs << endl;
  out << "  _vIQPerBin: " << _vIQPerBin << endl;
  out << "  _tgBank: " << _tgBank << endl;
  out << "  _tgWave: " << _tgWave << endl;
  out << "  _burstMag0: " << _burstMag0 << endl;
  out << "  _burstArg0: " << _burstArg0 << endl;
  out << "  _burstMag1: " << _burstMag1 << endl;
  out << "  _burstArg1: " << _burstArg1 << endl;

  // derived

  out << "============= Derived from pulse header ==================" << endl;
  out << "  _pulseTimeSecs: " << _pulseTimeSecs << endl;
  out << "  _pulseTime: " << _pulseTime << endl;
  out << "  _nGates: " << _nGates << endl;
  out << "  _el: " << _el << endl;
  out << "  _az: " << _az << endl;
  out << "  _prt: " << _prt << endl;
  out << "  _iprt: " << _iprt << endl;
  out << "  _phaseDiff: " << _phaseDiff << endl;

}

////////////////////////////////////////
// write out the netDCF file to tmp name
//
// Returns 0 on success, -1 on failure

int TsArchive2NetCdf::_writeTmpFile(const string &tmpPath,
				    const vector<iq_t> &iqVec,
				    const vector<float> &elVec,
				    const vector<float> &azVec,
				    const vector<int> &prtVec,
				    const vector<double> &timeVec,
				    const vector<float> &modCodeVec)
  
{

  // ensure output directory exists
  
  if (_makedir_recurse(_args.outDir.c_str())) {
    cerr << "ERROR - TsArchive2NetCdf::_writeTmpFile" << endl;
    cerr << "  Cannot make output dir: " << _args.outDir << endl;
    cerr << "  " << strerror(errno) << endl;
    return -1;
  }

  ////////////////////////
  // create Nc3File object
  
  Nc3Error err(Nc3Error::verbose_nonfatal);
  
  Nc3File out(tmpPath.c_str(), Nc3File::Replace);
  if (!out.is_valid()) {
    cerr << "ERROR - TsArchive2NetCdf::_writeTmpFile" << endl;
    cerr << "  Cannot create file: " << tmpPath << endl;
    return -1;
  }
  int iret = 0;

  /////////////////////
  // global attributes
  
  int nTimes = iqVec.size() / _nGatesSave;
  int startingSample = 0;
  int endingSample = startingSample + nTimes - 1;
  int startGate = 0;
  int endGate = startGate + _nGatesSave - 1;
  
  char desc[1024];
  sprintf(desc,
	  "Radar time series reformatted by TsArchive2NetCdf\n"
	  "Starting Sample =%d, Ending Sample =%d, "
	  "Start Gate= %d, End Gate = %d\n"
	  "Azimuth = %.2f, Elevation = %.2f\n",
	  startingSample, endingSample, startGate, endGate,
	  _startAz, _elevation);
  out.add_att("Description", desc);
  out.add_att("FirstGate", startGate);
  out.add_att("LastGate", endGate);

  //////////////////
  // add dimensions
  
  Nc3Dim *gatesDim = out.add_dim("gates", _nGatesSave);
  //int gatesId = gatesDim->id();
  
  Nc3Dim *frtimeDim = out.add_dim("frtime");
  //int frtimeId = frtimeDim->id();

  /////////////////////////////////
  // add vars and their attributes
  
  // Time variable

  Nc3Var *timeVar = out.add_var("Time", nc3Double, frtimeDim);
  timeVar->add_att("long_name", "Date/Time value");
  timeVar->add_att("units", "days since 0000-01-01");
  timeVar->add_att("_FillValue", 0.0);
  {
    double times[nTimes];
    for (int jj = 0; jj < nTimes; jj++) {
      times[jj] = timeVec[jj] / 86400.0;
    }
    long edge = nTimes;
    timeVar->put(times, &edge);
  }

  // Elevation variable
  
  Nc3Var *elVar = out.add_var("Elevation", nc3Float, frtimeDim);
  elVar->add_att("long_name", "Antenna Elevation");
  elVar->add_att("units", "degrees");
  {
    float validRange[2] = {0.0f, 360.0f};
    elVar->add_att("valid_range", 2, validRange);
  }
  elVar->add_att("_FillValue", (float) 0.0);
  {
    float elevations[nTimes];
    for (int jj = 0; jj < nTimes; jj++) {
      elevations[jj] = elVec[jj];
    }
    long edge = nTimes;
    elVar->put(elevations, &edge);
  }

  // Azimuth variable

  Nc3Var *azVar = out.add_var("Azimuth", nc3Float, frtimeDim);
  azVar->add_att("long_name", "Antenna Azimuth");
  azVar->add_att("units", "degrees");
  {
    float validRange[2] = {0.0f, 360.0f};
    azVar->add_att("valid_range", 2, validRange);
  }
  azVar->add_att("_FillValue", (float) 0.0);
  {
    float azimuths[nTimes];
    for (int jj = 0; jj < nTimes; jj++) {
      azimuths[jj] = azVec[jj];
    }
    long edge = nTimes;
    azVar->put(azimuths, &edge);
  }

  // modulation code variable

  Nc3Var *modCodeVar = out.add_var("ModCode", nc3Float, frtimeDim);
  modCodeVar->add_att("long_name", "Modulation code, pulse-to-pulse");
  modCodeVar->add_att("units", "degrees");
  {
    float validRange[2] = {0.0f, 360.0f};
    modCodeVar->add_att("valid_range", 2, validRange);
  }
  modCodeVar->add_att("_FillValue", (float) 0.0);
  {
    float modCodes[nTimes];
    for (int jj = 0; jj < nTimes; jj++) {
      modCodes[jj] = modCodeVec[jj];
    }
    long edge = nTimes;
    modCodeVar->put(modCodes, &edge);
  }

  // PRT variable

  Nc3Var *prtVar = out.add_var("Prt", nc3Int, frtimeDim);
  prtVar->add_att("long_name", "Pulse Repetition Time");
  prtVar->add_att("units", "microseconds");
  prtVar->add_att("valid_range", 1000000);
  {
    int prts[nTimes];
    for (int jj = 0; jj < nTimes; jj++) {
      prts[jj] = prtVec[jj];
    }
    long edge = nTimes;
    prtVar->put(prts, &edge);
  }
  
  // SampleNum variable

  Nc3Var *sampleNumVar = out.add_var("SampleNum", nc3Int, frtimeDim);
  sampleNumVar->add_att("long_name", "Sample Number");
  sampleNumVar->add_att("units", "Counter");
  sampleNumVar->add_att("valid_range", 100000000);
  {
    int sampleNums[nTimes];
    for (int jj = 0; jj < nTimes; jj++) {
      sampleNums[jj] = jj;
    }
    long edge = nTimes;
    sampleNumVar->put(sampleNums, &edge);
  }

  // I variable
  
  Nc3Var *iVar = out.add_var("I", nc3Float, frtimeDim, gatesDim);
  iVar->add_att("long_name", "In-phase time series variable");
  iVar->add_att("units", "scaled A/D counts");
  {
    float validRange[2] = {-1.e+20f, 1.e+20f};
    iVar->add_att("valid_range", 2, validRange);
  }
  iVar->add_att("_FillValue", (float) 0.0);
  {
    float *idata = new float[iqVec.size()];
    for (size_t jj = 0; jj < iqVec.size(); jj++) {
      idata[jj] = iqVec[jj].i;
    }
    long edges[2];
    edges[0] = nTimes;
    edges[1] = _nGatesSave;
    iVar->put(idata, edges);
    delete[] idata;
  }

  // Q variable

  Nc3Var *qVar = out.add_var("Q", nc3Float, frtimeDim, gatesDim);
  qVar->add_att("long_name", "Quadruture time series variable");
  qVar->add_att("units", "scaled A/D counts");
  {
    float validRange[2] = {-1.e+20f, 1.e+20f};
    qVar->add_att("valid_range", 2, validRange);
  }
  qVar->add_att("_FillValue", (float) 0.0);
  {
    float *qdata = new float[iqVec.size()];
    for (size_t jj = 0; jj < iqVec.size(); jj++) {
      qdata[jj] = iqVec[jj].q;
    }
    long edges[2];
    edges[0] = nTimes;
    edges[1] = _nGatesSave;
    qVar->put(qdata, edges);
    delete[] qdata;
  }

  return iret;

}

//////////////////////////////////////////////////////////
// _makedir()
//
// Utility routine to create a directory.  If the directory
// already exists, does nothing.
//
// Returns -1 on error, 0 otherwise.

int TsArchive2NetCdf::_makedir(const char *path)
{
  
  struct stat stat_buf;
  
  // Status the directory to see if it already exists.

  if (stat(path, &stat_buf) == 0) {
    return(0);
  }
  
  // Directory doesn't exist, create it.
  
  if (mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
    return(-1);
  
  return(0);

}

///////////////////////////////////////////////////////////
// _makedir_recurse()
//
// Utility routine to create a directory recursively.
// If the directory already exists, does nothing.
// Otherwise it recurses through the path, making all
// needed directories.
//
// Returns -1 on error, 0 otherwise.

int TsArchive2NetCdf::_makedir_recurse(const char *path)
{

  char up_dir[MAX_PATH_LEN];
  char *last_delim;
  struct stat dir_stat;
  
  // Stat the directory to see if it already exists.
  // '/' dir will always exist, so this stops the recursion
  // automatically.
  
  if (stat(path, &dir_stat) == 0) {
    return(0);
  }
  
  // create up dir - one up the directory tree -
  // by searching for the previous delim and removing it
  // from the string.
  // If no delim, try to make the directory non-recursively.
  
  strncpy(up_dir, path, MAX_PATH_LEN);
  last_delim = strrchr(up_dir, PATH_DELIM);
  if (!last_delim) {
    return _makedir(up_dir);
  }
  *last_delim = '\0';
  
  // make the up dir
  
  if (_makedir_recurse(up_dir)) {
    return (-1);
  }

  // make this dir

  if (_makedir(path)) {
    return -1;
  } else {
    return 0;
  }
  
}

/* ======================================================================
 * Convert a normalized floating "I" or "Q" value from the signal
 * processor's 16-bit packed format.  The floating values are in the
 * range -4.0 to +4.0, i.e., they are normalized so that full scale CW
 * input gives a magnitude of 1.0, while still leaving a factor of
 * four of additional amplitude headroom (12dB headroom power) to
 * account for FIR filter transients.
 */

void TsArchive2NetCdf::_vecFloatIQFromPackIQ
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

void TsArchive2NetCdf::_vecFloatIQFromPackIQ
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

