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
// TWolf2UF.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2010
//
///////////////////////////////////////////////////////////////
//
// TWolf2UF reads TWolf moments data in ASCII format and
// converts to UF.
//
////////////////////////////////////////////////////////////////

#include "TWolf2UF.hh"
#include "DateTime.hh"
#include <iostream>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <sys/stat.h>

using namespace std;

// Constructor

TWolf2UF::TWolf2UF(int argc, char **argv)
  
{

  isOK = true;
  _volNum = 0;
  _ufFile = NULL;
  
  // set programe name
  
  _progName = "TWolf2UF";

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // initialize UF record

  _rec.setInstrumentName(_args.instrumentName);
  _rec.setHorizBeamWidthDeg(_args.beamWidthDeg);
  _rec.setVertBeamWidthDeg(_args.beamWidthDeg);
  _rec.setPrf(_args.prf);
  _rec.setNoisePowerDbm(_args.noisePowerDbm);
  
}

// destructor

TWolf2UF::~TWolf2UF()

{

  _closeUfFile();

}

//////////////////////////////////////////////////
// Run

int TWolf2UF::Run()
{

  int iret = 0;

  // process files

  for (size_t ii = 0; ii < _args.inputFileList.size(); ii++) {
    if (_processFile(_args.inputFileList[ii])) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Process a file

int TWolf2UF::_processFile(const string &filePath)

{

  if (_args.debug) {
    cerr << "-----------------------------------" << endl;
    cerr << "Processing input file: " << filePath << endl;
  }

  // close previous UF file if needed
  
  _closeUfFile();

  // open input file

  FILE *in;
  if ((in = fopen(filePath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - TWolf2UF::_processFile" << endl;
    cerr << "  Cannot open file for reading: " << filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  _volNum++;
  _sweepNum = 0;
  _rayNum = 0;
  _fixedAngle = -9999;
  _prevFixedAngle = -9999;
  _fixedAngleChanging = false;

  // try setting start time from file name

  bool gotStartTime = false;
  _startTime.set(time_t(NULL));
  DateTime tmp;
  if (_getDataTime(filePath, tmp) == 0) {
    _startTime = tmp;
    _prevTime = _startTime;
    gotStartTime = true;
  }
  if (_args.debug) {
    cerr << "Data time: " << DateTime::strm(_startTime.utime()) << endl;
  }

  // read first records in file to decide if this is an RHI or not

  _isRhi = _isScanRhi(in);

  if (_args.debug) {
    if (_isRhi) {
      cerr << "  Scan is RHI" << endl;
    } else {
      cerr << "  Scan is PPI" << endl;
    }
  }

  // seek back to start of file

  fseek(in, 0L, SEEK_SET);

  // read in file
  
  static const int MaxLineLen = 20000;
  char line[MaxLineLen];

  while (!feof(in)) {
    
    if (fgets(line, MaxLineLen, in) == NULL) {
      break;
    }

    if(_args.verbose) {
      cerr << line;
    }

    // start time

    if (!gotStartTime) {
      int year, month, day, hour, min, sec;
      if (sscanf(line + 3, "%4d%2d%2d_%2d%2d%2d",
                 &year, &month, &day, &hour, &min, &sec) == 6) {
        _startTime.set(year, month, day, hour, min, sec);
        if (_args.debug) {
          cerr << "Start time: " << DateTime::strm(_startTime.utime()) << endl;
        }
        _prevTime = _startTime;
        gotStartTime = true;
      }
    }
    
    // process ray
    
    if (_processRay(line) == 0) {
      if (_writeRecord()) {
        _closeUfFile();
        fclose(in);
        return -1;
      }
    }
    
  } // if (!feof ...

  _closeUfFile();
  fclose(in);
  return 0;

}

//////////////////////////////////////////////////
// Process a ray
//
// Returns 0 on success, -1 on failure

int TWolf2UF::_processRay(const char *line)

{
  
  // read ray meta data

  int hour, min, sec;
  double lat, lon, elevM;
  double heading, track;
  double az, el;
  int nGates, nSamples;
  
  if (sscanf(line, "%2d%2d%2d%lg%lg%lg%lg%lg%lg%lg%d%d",
             &hour, &min, &sec,
             &lat, &lon, &elevM,
             &heading, &track, 
             &az, &el,
             &nGates, &nSamples) != 12) {
    return -1;
  }
  
  if (_args.verbose) {
    cerr << "hour, min, sec, lat, lon, elevM "
         << "heading, track, az, el, nGates, nSamples: "
         << hour << ", " << min << ", " << sec << ", "
         << lat << ", " << lon << ", " << elevM << ", "
         << heading << ", " << track << ", " 
         << az << ", " << el << ", "
         << nGates << ", " << nSamples << endl;
  }

  // compute ray time
  
  _rayTime.set(_prevTime.getYear(), _prevTime.getMonth(), _prevTime.getDay(),
               hour, min, sec);
  
  if (_rayTime.utime() < _prevTime.utime()) {
    // the day has changed
    _rayTime.set(_rayTime.utime() + 86400);
  }
  _prevTime = _rayTime;

  // angle and sweep num

  if (_isRhi) {
    _fixedAngle = az;
  } else {
    _fixedAngle = el;
  }
  
  if (_prevFixedAngle < -9990) {
    // initializing
    _prevFixedAngle = _fixedAngle;
  }

  double diffAngle = fabs(_fixedAngle - _prevFixedAngle);
  if (diffAngle > 0.025) {
    if (!_fixedAngleChanging) {
      _fixedAngleChanging = true;
    }
  } else {
    if (_fixedAngleChanging) {
      _sweepNum++;
      _fixedAngleChanging = false;
    }
  }

  _prevFixedAngle = _fixedAngle;

  // tokenize line
  
  vector<string> toks;
  _tokenize(line, " ", toks);
  
  // read the field data
  
  double *rangeArray = new double[nGates];
  double *htArray = new double[nGates];
  double *velArray = new double[nGates];
  double *snrArray = new double[nGates];
  double *widthArray = new double[nGates];
  
  int index = 10;
  double startRangeM = 0.0;
  double endRangeM = 0.0;

  for (int igate = 0; igate < nGates; igate++) {
    
    if (index + 5 > (int) toks.size()) {
      cerr << "ERROR - line not long enough" << endl;
      cerr << "  index, toks.size() " << index << ", " << toks.size() << endl;
      return -1;
    }
    double range, ht, htKm, vel, snr, width;
    if (sscanf(toks[index].c_str(), "%lg", &range) != 1) {
      cerr << "ERROR - cannot read range, tok: " << toks[index] << endl;
      return -1;
    }
    index++;
    if (sscanf(toks[index].c_str(), "%lg", &ht) != 1) {
      cerr << "ERROR - cannot read ht, tok: " << toks[index] << endl;
      return -1;
    }
    htKm = ht / 1000.0;
    index++;
    if (sscanf(toks[index].c_str(), "%lg", &vel) != 1) {
      cerr << "ERROR - cannot read vel, tok: " << toks[index] << endl;
      return -1;
    }
    index++;
    if (sscanf(toks[index].c_str(), "%lg", &snr) != 1) {
      cerr << "ERROR - cannot read snr, tok: " << toks[index] << endl;
      return -1;
    }
    index++;
    if (sscanf(toks[index].c_str(), "%lg", &width) != 1) {
      cerr << "ERROR - cannot read width, tok: " << toks[index] << endl;
      return -1;
    }
    index++;
    
    rangeArray[igate] = range;
    htArray[igate] = htKm;
    velArray[igate] = vel;
    snrArray[igate] = snr;
    widthArray[igate] = width;
    if (igate == 0) {
      startRangeM = range;
    } else {
      endRangeM = range;
    }
    
  } // igate
  
  double gateSpacingM = (endRangeM - startRangeM) / (nGates - 1.0);

  if (_args.verbose) {
    cerr << "--->> startRangeM, gateSpacingM: "
         << startRangeM << ", " << gateSpacingM << endl;
  }

  // load record

  _rec.loadRayData(_volNum, _sweepNum, _isRhi, _rayNum,
                   _rayTime, lat, lon, elevM,
                   el, az, nGates, nSamples,
                   startRangeM, gateSpacingM,
                   snrArray, velArray, widthArray, htArray);
  _rayNum++;
  
  return 0;

}

//////////////////////////////////////////////////
// Check if this file is an RHI
//
// Returns true if RHI, false if not

bool TWolf2UF::_isScanRhi(FILE *in)

{
  
  static const int MaxLineLen = 20000;
  char line[MaxLineLen];

  double prevEl = -9999;
  double sumDiff = 0.0;
  int count = 0;
  
  while (!feof(in)) {
    
    if (fgets(line, MaxLineLen, in) == NULL) {
      break;
    }

    int hour, min, sec;
    double lat, lon, elevM;
    double heading, track;
    double az, el;
    int nGates, nSamples;
    
    if (sscanf(line, "%2d%2d%2d%lg%lg%lg%lg%lg%lg%lg%d%d",
               &hour, &min, &sec,
               &lat, &lon, &elevM,
               &heading, &track, 
               &az, &el,
               &nGates, &nSamples) != 12) {
      continue;
    }
    
    if (prevEl < -9990) {

      prevEl = el;

    } else {

      double diff = fabs(el - prevEl);
      sumDiff += diff;
      count++;
      prevEl = el;
      
    }
      
    if (count > 20) {
      if (sumDiff > 2) {
        return true;
      } else {
        return false;
      }
    }

  } // while
  
  return false;

}

//////////////////////////////////////////////
// tokenize a string into a vector of strings
// given a spacer

void TWolf2UF::_tokenize(const string &str,
                         const string &spacer,
                         vector<string> &toks)
  
{
    
  toks.clear();
  size_t pos = 0;
  while (true) {
    size_t start = str.find_first_not_of(spacer, pos);
    size_t end = str.find_first_of(spacer, start);
    if (start == string::npos) {
      return;
    } else if (end == string::npos) {
      string tok;
      tok.assign(str, start, string::npos);
      toks.push_back(tok);
      return;
    } else {
      string tok;
      tok.assign(str, start, end - start);
      toks.push_back(tok);
    }
    pos = end;
  }
}

////////////////////////////////////////////////////////////
// compute and return uf file name

string TWolf2UF::_computeUfFileName(int volNum,
                                    string instrumentName,
                                    string scanType,
                                    int year, int month, int day,
                                    int hour, int min, int sec)
  
{

  // make sure instrument name is reasonable length
  
  if (instrumentName.size() > 8) {
    instrumentName.resize(8);
  }

  // replaces spaces in strings with underscores
  
  for (size_t ii = 0; ii < instrumentName.size(); ii++) {
    if (isspace(instrumentName[ii])) {
      instrumentName[ii] = '_';
    }
  }
  for (size_t ii = 0; ii < scanType.size(); ii++) {
    if (isspace(scanType[ii] == ' ')) {
      scanType[ii] = '_';
    }
  }
  
  char outName[BUFSIZ];
  sprintf(outName, "%04d%02d%02d_%02d%02d%02d_%s_%s_v%03d.uf",
          year, month, day, hour, min, sec,
          instrumentName.c_str(), scanType.c_str(), volNum);

  return outName;

}
  
////////////////////////////////////////////////////////////
// open UF file

int TWolf2UF::_openUfFile()
  
{

  _closeUfFile();

  // compute file path

  string scanType = "PPI";
  if (_isRhi) {
    scanType = "RHI";
  }

  string fileName = _computeUfFileName(_volNum,
                                       _args.instrumentName,
                                       scanType,
                                       _startTime.getYear(),
                                       _startTime.getMonth(),
                                       _startTime.getDay(),
                                       _startTime.getHour(),
                                       _startTime.getMin(),
                                       _startTime.getSec());

  char dayDir[2048];
  sprintf(dayDir, "%s/%.4d%.2d%.2d",
          _args.outputDir.c_str(),
          _startTime.getYear(),
          _startTime.getMonth(),
          _startTime.getDay());
  
  if (_makeDirRecurse(dayDir)) {
    int errNum = errno;
    cerr << "ERROR - TWolf2UF::_openUfFile" << endl;
    cerr << "  Cannot make output dir: " << dayDir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  _ufPath = dayDir;
  _ufPath += "/";
  _ufPath += fileName;
  
  if ((_ufFile = fopen(_ufPath.c_str(), "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - TWolf2UF::_openUfFile" << endl;
    cerr << "  Cannot open UF file for reading: " << _ufPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if (_args.debug) {
    cerr << "Writing to UF file: " << _ufPath << endl;
  }
  
  return 0;

}

////////////////////////////////////////////////////////////
// close UF file

void TWolf2UF::_closeUfFile()
  
{

  if (_ufFile) {
    fclose(_ufFile);
    _ufFile = NULL;
    if (_args.debug) {
      cerr << "Wrote UF file: " << _ufPath << endl;
      cerr << "-----------------------------------" << endl;
    }
  }

}

////////////////////////////////////////////////////////////
// write UF record

int TWolf2UF::_writeRecord()
  
{

  // check that file is open

  if (_ufFile == NULL) {
    if (_openUfFile()) {
      cerr << "ERROR - TWolf2UF::_writeRecord()" << endl;
      return -1;
    }
  }

  // write the record

  if (_rec.write(_ufFile)) {
    cerr << "ERROR - TWolf2UF::_writeRecord()" << endl;
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// Utility routine to create a directory.  If the directory
// already exists, does nothing.
//
// Returns -1 on error, 0 otherwise.

int TWolf2UF::_makeDir(const string &dir)
{
  
  // Status the directory to see if it already exists.
  
  struct stat stat_buf;
  if (stat(dir.c_str(), &stat_buf) == 0) {
    return 0;
  }
  
  // Directory doesn't exist, create it.
  
  if (mkdir(dir.c_str(),
	    S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
    // failed
    // check if dir has been made bu some other process
    // in the mean time, in which case return success
    if (stat(dir.c_str(), &stat_buf) == 0) {
      return 0;
    }
    return -1;
  }
  
  return 0;

}


/////////////////////////////////////////////////////////
// Utility routine to create a directory recursively.
// If the directory already exists, does nothing.
// Otherwise it recurses through the path, making all
// needed directories.
//
// Returns -1 on error, 0 otherwise.

int TWolf2UF::_makeDirRecurse(const string &dir)
{

  // Status the directory to see if it already exists.
  // '/' dir will always exist, so this stops the recursion
  // automatically.
  
  struct stat dir_stat;
  if (stat(dir.c_str(), &dir_stat) == 0) {
    return 0;
  }
  
  // create up dir - one up the directory tree -
  // by searching for the previous delim and removing it
  // from the string.
  // If no delim, try to make the directory non-recursively.
  
  size_t delimPos = dir.find_last_of("/");
  if (delimPos == string::npos) {
    return _makeDir(dir);
  } 

  string upDir(dir, 0, delimPos);
  
  // make the up dir
  
  if (_makeDirRecurse(upDir)) {
    return -1;
  }

  // make this dir
  
  if (_makeDir(dir)) {
    return -1;
  } else {
    return 0;
  }

}

/////////////////////////////////////////////////////////////////////
// Get the data time information from the given file path.
//
// The following formats are supported.
//    (* indicates any sequence of non-digits)
//    (? indicates a single non-digit)
//
//     */*mmddyy?hhmmss*
//     */*yyyymmdd?hhmmss*
//     */*yyyymmddhhmmss*
//
// Returns 0 on success, -1 on error.
// On success, sets data_time.

int TWolf2UF::_getDataTime(const string& file_path,
                           DateTime &data_time)
{

  int year, month, day, hour, min, sec;
  
  // find the last path delimiter in the path
  
  const char *start = NULL;
  start = strrchr(file_path.c_str(), '/');
  if (start == NULL) {
    start = file_path.c_str();
  } else {
    start++;
  }

  // find first digit

  while (start != '\0') {
    if (isdigit(*start)) {
      break;
    }
    start++;
  }

  // try *mmddyy_hhmmss*

  char cc;
  if (sscanf(start, "%2d%2d%2d%1c%2d%2d%2d",
             &month, &day, &year, &cc,
             &hour, &min, &sec) == 7) {
    if (!isdigit(cc)) {
      year += 2000;
      data_time.set(year, month, day, hour, min, sec);
      return 0;
    }
  }

  // try *mmddyy_vvvv_hhmmss*

  int volNum;
  char cc1;
  if (sscanf(start, "%2d%2d%2d%1c%4d%c%2d%2d%2d",
             &month, &day, &year, &cc,
             &volNum, &cc1,
             &hour, &min, &sec) == 9) {
    if (!isdigit(cc)) {
      year += 2000;
      data_time.set(year, month, day, hour, min, sec);
      _volNum = volNum;
      return 0;
    }
  }

  // try *yyyymmdd?hhmmss* (? is single non-digit)
  
  if (sscanf(start, "%4d%2d%2d%1c%2d%2d%2d",
             &year, &month, &day, &cc,
             &hour, &min, &sec) == 7) {
    if (!isdigit(cc)) {
      data_time.set(year, month, day, hour, min, sec);
      return 0;
    }
  }

  if (sscanf(start, "%4d%2d%2d%2d%2d%2d",
             &year, &month, &day,
             &hour, &min, &sec) == 6) {
    data_time.set(year, month, day, hour, min, sec);
    return 0;
  }
  
  // try *yyyymmddhhmm*
  
  if (sscanf(start, "%4d%2d%2d%2d%2d",
             &year, &month, &day,
             &hour, &min) == 5) {
    sec = 0;
    data_time.set(year, month, day, hour, min, sec);
    return 0;
  }

  // no luck

  return -1;

}



