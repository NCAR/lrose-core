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
/////////////////////////////////////////////////////////////
// NcxxRadxFile_read.cc
//
// Read methods for NcxxRadxFile object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2017
//
///////////////////////////////////////////////////////////////

#include <Radx/NcxxRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
using namespace std;

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int NcxxRadxFile::readFromPath(const string &path,
                               RadxVol &vol)
  
{

  _initForRead(path, vol);

  // If the flag is set to aggregate sweeps into a volume on read,
  // create a vector of paths.  Otherwise load just original path into
  // vector.

  vector<string> paths;
  if (_readAggregateSweeps) {
    int volNum = _getVolumePaths(path, paths);
    if (_debug) {
      cerr << "INFO - _readAggregatePaths" << endl;
      cerr << "  specified path: " << path << endl;
      cerr << "  volNum: " << volNum << endl;
      cerr << "  Found paths:" << endl;
      for (size_t ii = 0; ii < paths.size(); ii++) {
        cerr << "    " << paths[ii] << endl;
      }
    }
  } else {
    paths.push_back(path);
  }

  // load sweep information from files

  if (_loadSweepInfo(paths)) {
    _addErrStr("ERROR - NcxxRadxFile::readFromPath");
    _addErrStr("  Loading sweep info");
    return -1;
  }

  // read from all paths

  for (size_t ii = 0; ii < paths.size(); ii++) {
    if (_readPath(paths[ii], ii)) {
      return -1;
    }
  }

  // load the data into the read volume

  _loadReadVolume();

  // remove transitions if applicable

  if (_readIgnoreTransitions) {
    _readVol->removeTransitionRays(_readTransitionNraysMargin);
  }

  // compute fixed angles if not found

  if (!_fixedAnglesFound) {
    _computeFixedAngles();
  }

  // set format as read

  _fileFormat = FILE_FORMAT_CFRADIAL;

  return 0;

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
// Returns 0 on success, -1 on failure

int NcxxRadxFile::_readPath(const string &path, size_t pathNum)
  
{

  if (_verbose) {
    cerr << "Reading file num, path: "
         << pathNum << ", " << path << endl;
  }

  string errStr("ERROR - NcxxRadxFile::readFromPath::_readPath");

  // clear tmp rays

  _nTimesInFile = 0;
  _raysFromFile.clear();
  _nRangeInFile = 0;

  // open file

  try {
    _file.open(path, NcxxFile::read);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxRadxFile::_readPath");
    _addErrStr("  Cannot open file for reading: ", path);
    _addErrStr("  exception: ", e.what());
    return -1;
  }

  // read dimensions
  
  if (_readDimensions()) {
    _addErrStr(errStr);
    return -1;
  }

  if (_nTimesInFile < 1) {
    _addErrStr("ERROR - NcxxRadxFile::_readPath");
    _addErrStr("  No times in file");
    return -1;
  }
  if (_nRangeInFile < 1) {
    _addErrStr("ERROR - NcxxRadxFile::_readPath");
    _addErrStr("  No ranges in file");
    return -1;
  }

  // read time variable now if that is all that is needed
  
  if (_readTimesOnly) {
    if (_readTimes(pathNum)) {
      _addErrStr(errStr);
      return -1;
    }
    return 0;
  }
  
  // check if georeferences and/or corrections are active

  _checkGeorefsActiveOnRead();
  _checkCorrectionsActiveOnRead();

  // for first path in aggregated list, read in non-varying values

  if (pathNum == 0) {

    // read global attributes
    
    if (_readGlobalAttributes()) {
      _addErrStr(errStr);
      return -1;
    }

    // read in scalar variables
    
    _readScalarVariables();
    
    // read frequency variable
    
    if (_readFrequencyVariable()) {
      _addErrStr(errStr);
      return -1;
    }

    // read in correction variables
    
    if (_correctionsActive) {
      _readCorrectionVariables();
    }

  }

  // read time variable
  
  if (_readTimes(pathNum)) {
    _addErrStr(errStr);
    return -1;
  }

  // read range variable
  // the range array size will be the max of the arrays found in
  // the files
  
  if (_readRangeVariable()) {
    _addErrStr(errStr);
    return -1;
  }
  
  // read position variables - lat/lon/alt
  
  if (_readPositionVariables()) {
    _addErrStr(errStr);
    return -1;
  }

  // read in sweep variables

  if (_readSweepVariables()) {
    _addErrStr(errStr);
    return -1;
  }

  // read in ray variables

  if (_readRayVariables()) {
    _addErrStr(errStr);
    return -1;
  }

  // read in georef variables

  if (_georefsActive) {
    if (_readGeorefVariables()) {
      _addErrStr(errStr);
      return -1;
    }
  }

  if (_readMetadataOnly) {

    // read field variables
    
    if (_readFieldVariables(true)) {
      _addErrStr(errStr);
      return -1;
    }
    
    if (_nGatesVary) {
      // set the packing so that the number of gates varies
      // this will ensure that a call to getNGatesVary() on
      // the volume will return true
      _readVol->addToPacking(1);
      _readVol->addToPacking(2);
    }

  } else {

    // create the rays to be read in, filling out the metadata
    
    if (_createRays(path)) {
      _addErrStr(errStr);
      return -1;
    }
    
    // read the ray ngates and offset vectors for each ray
    
    if (_readRayNgatesAndOffsets()) {
      _addErrStr(errStr);
      return -1;
    }
    
    // add field variables to file rays
    
    if (_readFieldVariables(false)) {
      _addErrStr(errStr);
      return -1;
    }

  }

  // read in calibration variables
  
  if (_readCalibrationVariables()) {
    _addErrStr(errStr);
    return -1;
  }

  // close file

  _file.close();

  // add file rays to main rays

  for (size_t ii = 0; ii < _raysFromFile.size(); ii++) {

    RadxRay *ray = _raysFromFile[ii];

    // check if we should keep this ray or discard it

    bool keep = true;
    if (_readRemoveRaysAllMissing) {
      if (ray->checkDataAllMissing()) {
        keep = false;
      }
    }

    // add to main vector if we are keeping it

    if (keep) {
      _raysVol.push_back(ray);
    } else {
      delete ray;
    }

  }

  // append to read paths

  _readPaths.push_back(path);

  // clean up

  _raysToRead.clear();
  _raysFromFile.clear();
  _clearGeorefVariables();
  _clearRayVariables();
  _dTimes.clear();

  return 0;

}

//////////////////////////////////////////////////////////
// get list of paths for the volume for the specified path
// returns the volume number

int NcxxRadxFile::_getVolumePaths(const string &path,
                                  vector<string> &paths)
  
{

  paths.clear();
  int volNum = -1;

  RadxPath rpath(path);
  string fileName = rpath.getFile();

  // find the volume number by searching for "_v"
  
  size_t vloc = fileName.find("_v");
  if (vloc == string::npos || vloc == 0) {
    // cannot find volume tag "_v"
    paths.push_back(path);
    return volNum;
  }

  // find trailing "_"

  size_t eloc = fileName.find("_", vloc + 2);
  if (eloc == string::npos || eloc == 0) {
    // cannot find trailing _
    paths.push_back(path);
    return volNum;
  }

  // is this a sweep file - i.e. not already a volume?
  
  size_t sloc = fileName.find("_s", vloc + 2);
  if (sloc == string::npos || sloc == 0) {
    // cannot find sweep tag "_s"
    paths.push_back(path);
    return volNum;
  }

  // set the vol str and numstr

  string volStr(fileName.substr(vloc, eloc - vloc + 1));
  string numStr(fileName.substr(vloc + 2, eloc - vloc - 2));

  // scan the volume number

  if (sscanf(numStr.c_str(), "%d", &volNum) != 1) {
    volNum = -1;
    return volNum;
  }

  // find all paths with this volume number in the same
  // directory as the specified path

  string dir = rpath.getDirectory();
  _addToPathList(dir, volStr, 0, 23, paths);
  
  RadxPath dpath(dir);
  string parentDir = dpath.getDirectory();

  // if time is close to start of day, search previous directory

  RadxTime rtime;
  if (getTimeFromPath(path, rtime)) {
    return volNum;
  }
  int rhour = rtime.getHour();

  if (rhour == 0) {
    RadxTime prevDate(rtime.utime() - RadxTime::RADX_SECS_IN_DAY);
    char prevDir[RadxPath::RADX_MAX_PATH_LEN];
    sprintf(prevDir, "%s%s%.4d%.2d%.2d",
            parentDir.c_str(), RadxPath::RADX_PATH_DELIM,
            prevDate.getYear(), prevDate.getMonth(), prevDate.getDay());
    _addToPathList(prevDir, volStr, 23, 23, paths);
  }

  // if time is close to end of day, search previous direectory

  if (rhour == 23) {
    RadxTime nextDate(rtime.utime() + RadxTime::RADX_SECS_IN_DAY);
    char nextDir[RadxPath::RADX_MAX_PATH_LEN];
    sprintf(nextDir, "%s%s%.4d%.2d%.2d",
            parentDir.c_str(), RadxPath::RADX_PATH_DELIM,
            nextDate.getYear(), nextDate.getMonth(), nextDate.getDay());
    _addToPathList(nextDir, volStr, 0, 0, paths);
  }

  // sort the path list

  sort(paths.begin(), paths.end());

  return volNum;

}

///////////////////////////////////////////////////////////
// add to the path list, given time constraints

void NcxxRadxFile::_addToPathList(const string &dir,
                                  const string &volStr,
                                  int minHour, int maxHour,
                                  vector<string> &paths)
  
{

  // find all paths with this volume number
  
  DIR *dirp;
  if((dirp = opendir(dir.c_str())) == NULL) {
    return;
  }
  
  struct dirent *dp;
  for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    string fileName(dp->d_name);

    // exclude dir entries which cannot be valid
    if (fileName.find("cfrad.") != 0) {
      continue;
    }
    if (fileName.find("IDL") != string::npos) {
      continue;
    }
    if (fileName.size() < 20) {
      continue;
    }

    if (fileName.find(volStr) == string::npos) {
      continue;
    }

    RadxTime rtime;
    if (getTimeFromPath(fileName, rtime)) {
      continue;
    }
    int hour = rtime.getHour();
    if (hour >= minHour && hour <= maxHour) {
      string filePath = dir;
      filePath += RadxPath::RADX_PATH_DELIM;
      filePath += fileName;
      paths.push_back(filePath);
    }

  } // dp

  closedir(dirp);

}

////////////////////////////////////////////////////////////
// Load up sweep information from files
// Returns 0 on success, -1 on failure

int NcxxRadxFile::_loadSweepInfo(const vector<string> &paths)
{

  // read in the sweep info

  for (size_t ii = 0; ii < paths.size(); ii++) {
    if (_appendSweepInfo(paths[ii])) {
      return -1;
    }
  }

  if (_verbose) {
    cerr << "====>> Sweeps as originally in files <<=======" << endl;
    for (size_t ii = 0; ii < _sweepsOrig.size(); ii++) {
      cerr << "sweep info path: " << _sweepsOrig[ii].path << endl;
      cerr << "  num: " << _sweepsOrig[ii].sweepNum << endl;
      cerr << "  angle: " << _sweepsOrig[ii].fixedAngle << endl;
      cerr << "  indexInFile: " << _sweepsOrig[ii].indexInFile << endl;
    }
    cerr << "==============================================" << endl;
  }
  
  // if no limits set, all sweeps are read

  if (!_readFixedAngleLimitsSet && !_readSweepNumLimitsSet) {
    _sweepsToRead = _sweepsOrig;
    return 0;
  }
  
  // find sweeps which lie within the fixedAngle limits

  _sweepsToRead.clear();
  for (size_t ii = 0; ii < _sweepsOrig.size(); ii++) {
    if (_readFixedAngleLimitsSet) {
      double angle = _sweepsOrig[ii].fixedAngle;
      if (angle > (_readMinFixedAngle - 0.01) && 
          angle < (_readMaxFixedAngle + 0.01)) {
        _sweepsToRead.push_back(_sweepsOrig[ii]);
      }
    } else if (_readSweepNumLimitsSet) {
      int sweepNum = _sweepsOrig[ii].sweepNum;
      if (sweepNum >= _readMinSweepNum &&
          sweepNum <= _readMaxSweepNum) {
        _sweepsToRead.push_back(_sweepsOrig[ii]);
      }
    }
  }

  // make sure we have at least one sweep number
  
  if (_sweepsToRead.size() == 0) {

    // strict checking?

    if (_readStrictAngleLimits) {
      _addErrStr("ERROR - NcxxRadxFile::_loadSweepInfo");
      _addErrStr("  No sweeps found within limits:");
      if (_readFixedAngleLimitsSet) {
        _addErrDbl("    min fixed angle: ", _readMinFixedAngle, "%g");
        _addErrDbl("    max fixed angle: ", _readMaxFixedAngle, "%g");
      } else if (_readSweepNumLimitsSet) {
        _addErrInt("    min sweep num: ", _readMinSweepNum);
        _addErrInt("    max sweep num: ", _readMaxSweepNum);
      }
      return -1;
    }

    int bestIndex = 0;
    if (_readFixedAngleLimitsSet) {
      double minDiff = 1.0e99;
      double meanAngle = (_readMinFixedAngle + _readMaxFixedAngle) / 2.0;
      if (_readMaxFixedAngle - _readMinFixedAngle < 0) {
        meanAngle -= 180.0;
      }
      if (meanAngle < 0) {
        meanAngle += 360.0;
      }
      for (size_t ii = 0; ii < _sweepsOrig.size(); ii++) {
        double angle = _sweepsOrig[ii].fixedAngle;
        double diff = fabs(angle - meanAngle);
        if (diff < minDiff) {
          minDiff = diff;
          bestIndex = ii;
        }
      }
    } else if (_readSweepNumLimitsSet) {
      double minDiff = 1.0e99;
      double meanNum = (_readMinSweepNum + _readMaxSweepNum) / 2.0;
      for (size_t ii = 0; ii < _sweepsOrig.size(); ii++) {
        int sweepNum = _sweepsOrig[ii].sweepNum;
        double diff = fabs(sweepNum - meanNum);
        if (diff < minDiff) {
          minDiff = diff;
          bestIndex = ii;
        }
      }
    }
    _sweepsToRead.push_back(_sweepsOrig[bestIndex]);
  }

  if (_verbose) {
    cerr << "====>> Sweeps to be read <<=======" << endl;
    for (size_t ii = 0; ii < _sweepsToRead.size(); ii++) {
      cerr << "sweep info path: " << _sweepsToRead[ii].path << endl;
      cerr << "  num: " << _sweepsToRead[ii].sweepNum << endl;
      cerr << "  angle: " << _sweepsToRead[ii].fixedAngle << endl;
      cerr << "  indexInFile: " << _sweepsToRead[ii].indexInFile << endl;
    }
    cerr << "=================================" << endl;
  }

  return 0;

}

int NcxxRadxFile::_appendSweepInfo(const string &path)
{

  // open file

  try {
    _file.open(path, NcxxFile::read);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxRadxFile::readFromPath");
    _addErrStr("  Cannot open file for reading: ", path);
    _addErrStr("  exception: ", e.what());
    return -1;
  }

  // read dimensions
  
  if (_readDimensions()) {
    _addErrStr("ERROR - NcxxRadxFile::_appendSweepInfo");
    return -1;
  }

  if (_readSweepVariables()) {
    _addErrStr("ERROR - NcxxRadxFile::_appendSweepInfo");
    return -1;
  }
  
  // done with file

  _file.close();

  // add sweeps to list of file sweeps

  for (size_t ii = 0; ii < _sweepsInFile.size(); ii++) {
    RadxSweep *sweep = _sweepsInFile[ii];
    _readVol->addSweepAsInFile(sweep);
    SweepInfo info;
    info.path = path;
    info.sweepNum = sweep->getSweepNumber();
    info.fixedAngle = sweep->getFixedAngleDeg();
    info.indexInFile = ii;
    _sweepsOrig.push_back(info);
  }

  return 0;

}

/////////////////////////////////////////////////
// check if corrections are active on read

void NcxxRadxFile::_checkGeorefsActiveOnRead()
{

  _georefsActive = false;

  // get latitude variable

  _latitudeVar = _file.getVar(LATITUDE);
  if (_latitudeVar.isNull()) {
    return;
  }
  if (_latitudeVar.getDimCount() < 1) {
    return;
  }

  // if the latitude has dimension of time, then latitude is a 
  // vector and georefs are active
  
  NcxxDim timeDim = _latitudeVar.getDim(0);
  if (timeDim == _timeDim) {
    _georefsActive = true;
  }

}
  
/////////////////////////////////////////////////
// check if corrections are active on read

void NcxxRadxFile::_checkCorrectionsActiveOnRead()
{

  _correctionsActive = false;
  if (!_file.getVar(AZIMUTH_CORRECTION).isNull()) {
    _correctionsActive = true;
  }

}
  
///////////////////////////////////
// read in the dimensions

int NcxxRadxFile::_readDimensions()

{

  // read required dimensions

  _nTimesInFile = 0;
  _nRangeInFile = 0;

  try {
    
    _timeDim = _file.getDim(TIME);
    _nTimesInFile = _timeDim.getSize();

    _rangeDim = _file.getDim(RANGE);
    _nRangeInFile = _rangeDim.getSize();
    
    _nPointsDim = _file.getDim(N_POINTS);
    if (_nPointsDim.isNull()) {
      _nGatesVary = false;
      _nPoints = 0;
    } else {
      _nGatesVary = true;
      _nPoints = _nPointsDim.getSize();
    }

    _sweepDim = _file.getDim(SWEEP);

  } catch (NcxxException e) {

    _addErrStr("ERROR - NcxxRadxFile::readDimensions");
    _addErrStr("  exception: ", e.what());
    return -1;

  }

  // calibration dimension is optional

  try {
    _calDim = _file.getDim(R_CALIB);
  } catch (NcxxException e) {
    _calDim.setNull();
  }

  return 0;

}

///////////////////////////////////
// read the global attributes

int NcxxRadxFile::_readGlobalAttributes()

{

  // check for conventions
  
  try {
    NcxxGroupAtt att = _file.getAtt(CONVENTIONS);
    _conventions = att.asString();
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxRadxFile::_readGlobalAttributes");
    _addErrStr("  Cannot find conventions attribute");
    return -1;
  }
  if (_conventions.find(BaseConvention) == string::npos) {
    if (_conventions.find("CF") == string::npos &&
        _conventions.find("Radial") == string::npos) {
      _addErrStr("ERROR - NcxxRadxFile::_readGlobalAttributes");
      _addErrStr("  Invalid Conventions attribute: ", _conventions);
      return -1;
    }
  }

  // check for instrument name

  try {
    NcxxGroupAtt att = _file.getAtt(INSTRUMENT_NAME);
    _instrumentName = att.asString();
    if (_instrumentName.size() < 1) {
      _instrumentName = "unknown";
    }
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxRadxFile::_readGlobalAttributes");
    _addErrStr("  Cannot find instrument_name attribute");
    return -1;
  }

  // Loop through the global attributes, use the ones which make sense

  _origFormat = "CFRADIAL"; // default

  multimap<string,NcxxGroupAtt> atts = _file.getAtts();
  for (multimap<string,NcxxGroupAtt>::iterator ii = atts.begin();
       ii != atts.end(); ii++) {

    NcxxGroupAtt att = ii->second;
    
    if (att.isNull()) {
      continue;
    }
    if (att.getName().find(VERSION) != string::npos) {
      _version = att.asString();
    }
    if (att.getName().find(TITLE) != string::npos) {
      _title = att.asString();
    }
    if (att.getName().find(SOURCE) != string::npos) {
      _source = att.asString();
    }
    if (att.getName().find(HISTORY) != string::npos) {
      _history = att.asString();
    }
    if (att.getName().find(INSTITUTION) != string::npos) {
      _institution = att.asString();
    }
    if (att.getName().find(REFERENCES) != string::npos) {
      _references = att.asString();
    }
    if (att.getName().find(COMMENT) != string::npos) {
      _comment = att.asString();
    }
    if (att.getName().find(AUTHOR) != string::npos) {
      _author = att.asString();
    }
    if (att.getName().find(ORIGINAL_FORMAT) != string::npos) {
      _origFormat = att.asString();
    }
    if (att.getName().find(DRIVER) != string::npos) {
      _driver = att.asString();
    }
    if (att.getName().find(CREATED) != string::npos) {
      _created = att.asString();
    }
    if (att.getName().find(SITE_NAME) != string::npos) {
      _siteName = att.asString();
    }
    if (att.getName().find(SCAN_NAME) != string::npos) {
      _scanName = att.asString();
    }
    if (att.getName().find(SCAN_ID) != string::npos) {
      vector<int> scanIds;
      try {
        att.getValues(scanIds);
        _scanId = scanIds[0];
      } catch (NcxxException& e) {
      }
    }
    
    if (att.getName().find(RAY_TIMES_INCREASE) != string::npos) {
      string rayTimesIncrease = att.asString();
      if (rayTimesIncrease == "true") {
        _rayTimesIncrease = true;
      } else {
        _rayTimesIncrease = false;
      }
    }

  } // ii

  return 0;

}

///////////////////////////////////
// read the times

int NcxxRadxFile::_readTimes(int pathNum)

{

  // read the time variable

  _timeVar = _file.getVar(TIME);
  if (_timeVar.isNull()) {
    _addErrStr("ERROR - NcxxRadxFile::_readTimes");
    _addErrStr("  Cannot find time variable, name: ", TIME);
    _addErrStr(_file.getErrStr());
    return -1;
  }
  if (_timeVar.getDimCount() < 1) {
    _addErrStr("ERROR - NcxxRadxFile::_readTimes");
    _addErrStr("  time variable has no dimensions");
    return -1;
  }
  NcxxDim timeDim = _timeVar.getDim(0);
  if (timeDim != _timeDim) {
    _addErrStr("ERROR - NcxxRadxFile::_readTimes");
    _addErrStr("  Time has incorrect dimension, name: ", timeDim.getName());
    return -1;
  }

  // get units attribute

  try {
    NcxxVarAtt unitsAtt = _timeVar.getAtt(UNITS);
    string units = unitsAtt.asString();
    // parse the time units reference time
    RadxTime stime(units);
    _refTimeSecsFile = stime.utime();
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxRadxFile::_readTimes");
    _addErrStr("  Time has no units");
    return -1;
  }
  
  // set the time array
  
  RadxArray<double> dtimes_;
  double *dtimes = dtimes_.alloc(_nTimesInFile);
  try {
    _timeVar.getVal(dtimes);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxRadxFile::_readTimes");
    _addErrStr("  Cannot read times variable");
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  _dTimes.clear();
  for (size_t ii = 0; ii < _nTimesInFile; ii++) {
    _dTimes.push_back(dtimes[ii]);
  }

  double startTime = _dTimes[0];
  double endTime = _dTimes[_dTimes.size()-1];
  time_t startTimeSecs = _refTimeSecsFile + (int) startTime;
  time_t endTimeSecs = _refTimeSecsFile + (int) endTime;
  double startNanoSecs = (startTime - (int) startTime) * 1.0e9;
  double endNanoSecs = (endTime - (int) endTime) * 1.0e9;

  _readVol->setStartTime(startTimeSecs, startNanoSecs);
  _readVol->setEndTime(endTimeSecs, endNanoSecs);

  return 0;

}

///////////////////////////////////
// read the range variable

int NcxxRadxFile::_readRangeVariable()

{

  _rangeVar = _file.getVar(RANGE);
  if (_rangeVar.isNull() || _rangeVar.numVals() < 1) {
    _addErrStr("ERROR - NcxxRadxFile::_readRangeVariable");
    _addErrStr("  Cannot read range");
    _addErrStr(_file.getErrStr());
    return -1;
  }

  if (_rangeVar.numVals() <= (int) _rangeKm.size()) {
    // use data from previously-read sweep file
    return 0;
  }

  _rangeKm.clear();
  _nRangeInFile = _rangeDim.getSize();

  if (_rangeVar.getDimCount() == 1) {

    // 1-dimensional - range dim only
    // for gate geom that does not vary by ray

    NcxxDim rangeDim = _rangeVar.getDim(0);
    if (rangeDim != _rangeDim) {
      _addErrStr("ERROR - NcxxRadxFile::_readRangeVariable");
      _addErrStr("  Range has incorrect dimension, name: ", rangeDim.getName());
      return -1;
    }

    double *rangeMeters = new double[_nRangeInFile];
    try {
      _rangeVar.getVal(rangeMeters);
      double *rr = rangeMeters;
      for (size_t ii = 0; ii < _nRangeInFile; ii++, rr++) {
        _rangeKm.push_back(*rr / 1000.0);
      }
    } catch (NcxxException& e) {
      _addErrStr("ERROR - NcxxRadxFile::_readRangeVariable");
      _addErrStr("  getVal fails, cannot get range data array, var name: ", rangeDim.getName());
      return -1;
    }
    delete[] rangeMeters;

  } else {

    // 2-dimensional - (time dim x range dim)
    // for gate geom that does vary by ray
    
    // check that we have the correct dimensions
    NcxxDim timeDim = _rangeVar.getDim(0);
    NcxxDim rangeDim = _rangeVar.getDim(1);
    if (timeDim != _timeDim || rangeDim != _rangeDim) {
      _addErrStr("ERROR - NcxxRadxFile::_readRangeVariable");
      _addErrStr("  Range has incorrect dimensions");
      _addErrStr("  dim0, name: ", timeDim.getName());
      _addErrStr("  dim1, name: ", rangeDim.getName());
      return -1;
    }

    vector<size_t> dimSizes;
    dimSizes.push_back(_nTimesInFile);
    dimSizes.push_back(_nRangeInFile);
    double *rangeMeters = new double[_nTimesInFile * _nRangeInFile];
    try {
      _rangeVar.getVal(dimSizes, rangeMeters);
      double *rr = rangeMeters;
      for (size_t ii = 0; ii < _nRangeInFile; ii++, rr++) {
        _rangeKm.push_back(*rr / 1000.0);
      }
    } catch (NcxxException& e) {
    }
    delete[] rangeMeters;

  }
  
  // set the geometry from the range vector

  _remap.computeRangeLookup(_rangeKm);
  _gateSpacingIsConstant = _remap.getGateSpacingIsConstant();
  _geom.setRangeGeom(_remap.getStartRangeKm(), _remap.getGateSpacingKm());
  
  // get attributes and check for geometry

  double startRangeKm = Radx::missingMetaDouble;
  double gateSpacingKm = Radx::missingMetaDouble;

  map<string, NcxxVarAtt> atts = _rangeVar.getAtts();
  for (map<string, NcxxVarAtt>::iterator ii = atts.begin();
       ii != atts.end(); ii++) {
    
    NcxxVarAtt att = ii->second;
    
    if (att.isNull()) {
      continue;
    }
    
    if (att.getName().find(SPACING_IS_CONSTANT) != string::npos) {
      string spacingIsConstant = att.asString();
      if (spacingIsConstant == "true") {
        _gateSpacingIsConstant = true;
      } else {
        _gateSpacingIsConstant = false;
      }
    }

    if (att.getName().find(METERS_TO_CENTER_OF_FIRST_GATE) != string::npos) {
      if (att.getType() == ncxxFloat || att.getType() == ncxxDouble) {
        vector<double> vals;
        try {
          att.getValues(vals);
          startRangeKm = vals[0] / 1000.0;
        } catch (NcxxException& e) {
        }
      }
    }

    if (att.getName().find(METERS_BETWEEN_GATES) != string::npos) {
      if (att.getType() == ncxxFloat || att.getType() == ncxxDouble) {
        vector<double> vals;
        try {
          att.getValues(vals);
          gateSpacingKm = vals[0] / 1000.0;
        } catch (NcxxException& e) {
        }
      }
    }
    
  } // ii

  if (startRangeKm != Radx::missingMetaDouble &&
      gateSpacingKm != Radx::missingMetaDouble) {
    _geom.setRangeGeom(startRangeKm, gateSpacingKm);
  }

  return 0;

}

///////////////////////////////////
// read the scalar variables

int NcxxRadxFile::_readScalarVariables()

{

  try {

    _volumeNumberVar = 
      _file.readIntVar(VOLUME_NUMBER, 
                       _volumeNumber, Radx::missingMetaInt);
    
    _instrumentType = Radx::INSTRUMENT_TYPE_RADAR;
    
    try {
      string pstring;
      _instrumentTypeVar = _file.readCharStringVar(INSTRUMENT_TYPE, pstring);
      _instrumentType = Radx::instrumentTypeFromStr(pstring);
    } catch (NcxxException e) {
    }

    try {
      string pstring;
      _platformTypeVar = _file.readCharStringVar(PLATFORM_TYPE, pstring);
      _platformType = Radx::platformTypeFromStr(pstring);
    } catch (NcxxException e) {
    }

    try {
      string pstring;
      _primaryAxisVar = _file.readCharStringVar(PRIMARY_AXIS, pstring);
      _primaryAxis = Radx::primaryAxisFromStr(pstring);
    } catch (NcxxException e) {
    }
    
    if (!_file.getVar(STATUS_XML).isNull()) {
      try {
        string pstring;
        _statusXmlVar = _file.readCharStringVar(STATUS_XML, pstring);
        _statusXml = pstring;
      } catch (NcxxException e) {
      }
    }

    if (_instrumentType == Radx::INSTRUMENT_TYPE_RADAR) {
      
      _radarAntennaGainHVar =
        _file.readDoubleVar(RADAR_ANTENNA_GAIN_H, _radarAntennaGainDbH, false);
      _radarAntennaGainVVar =
        _file.readDoubleVar(RADAR_ANTENNA_GAIN_V, _radarAntennaGainDbV, false);
      _radarBeamWidthHVar =
        _file.readDoubleVar(RADAR_BEAM_WIDTH_H, _radarBeamWidthDegH, false);
      _radarBeamWidthVVar =
        _file.readDoubleVar(RADAR_BEAM_WIDTH_V, _radarBeamWidthDegV, false);
      _radarRxBandwidthVar =
        _file.readDoubleVar(RADAR_RX_BANDWIDTH, _radarRxBandwidthHz, false);
    
    } else {
      
      _lidarConstantVar = 
        _file.readDoubleVar(LIDAR_CONSTANT, _lidarConstant, false);
      _lidarPulseEnergyJVar = 
        _file.readDoubleVar(LIDAR_PULSE_ENERGY, _lidarPulseEnergyJ, false);
      _lidarPeakPowerWVar = 
        _file.readDoubleVar(LIDAR_PEAK_POWER, _lidarPeakPowerW, false);
      _lidarApertureDiamCmVar = 
        _file.readDoubleVar(LIDAR_APERTURE_DIAMETER,
                            _lidarApertureDiamCm, false);
      _lidarApertureEfficiencyVar = 
        _file.readDoubleVar(LIDAR_APERTURE_EFFICIENCY,
                            _lidarApertureEfficiency, false);
      _lidarFieldOfViewMradVar = 
        _file.readDoubleVar(LIDAR_FIELD_OF_VIEW,
                            _lidarFieldOfViewMrad, false);
      _lidarBeamDivergenceMradVar = 
        _file.readDoubleVar(LIDAR_BEAM_DIVERGENCE,
                          _lidarBeamDivergenceMrad, false);
      
    }

  } catch (NcxxException e) {
    _addErrStr("ERROR - NcxxRadxFile::_readScalarVariables");
    _addErrStr("  exception: ", e.what());
    return -1;
  } // try
  
  return 0;

}

///////////////////////////////////
// read the correction variables

int NcxxRadxFile::_readCorrectionVariables()

{
  
  _cfactors.clear();

  try {

    double val;

    _azimuthCorrVar = _file.readDoubleVar(AZIMUTH_CORRECTION, val, 0);
    _cfactors.setAzimuthCorr(val);
  
    _elevationCorrVar = _file.readDoubleVar(ELEVATION_CORRECTION, val, 0);
    _cfactors.setElevationCorr(val);

    _rangeCorrVar = _file.readDoubleVar(RANGE_CORRECTION, val, 0);
    _cfactors.setRangeCorr(val);

    _longitudeCorrVar = _file.readDoubleVar(LONGITUDE_CORRECTION, val, 0);
    _cfactors.setLongitudeCorr(val);
    
    _latitudeCorrVar = _file.readDoubleVar(LATITUDE_CORRECTION, val, 0);
    _cfactors.setLatitudeCorr(val);
    
    _pressureAltCorrVar =
      _file.readDoubleVar(PRESSURE_ALTITUDE_CORRECTION, val, 0);
    _cfactors.setPressureAltCorr(val);
    
    _altitudeCorrVar = _file.readDoubleVar(ALTITUDE_CORRECTION, val, 0);
    _cfactors.setAltitudeCorr(val);
    
    _ewVelCorrVar = 
      _file.readDoubleVar(EASTWARD_VELOCITY_CORRECTION, val, 0);
    _cfactors.setEwVelCorr(val);
    
    _nsVelCorrVar = 
      _file.readDoubleVar(NORTHWARD_VELOCITY_CORRECTION, val, 0);
    _cfactors.setNsVelCorr(val);

    _vertVelCorrVar = 
      _file.readDoubleVar(VERTICAL_VELOCITY_CORRECTION, val, 0);
    _cfactors.setVertVelCorr(val);

    _headingCorrVar = _file.readDoubleVar(HEADING_CORRECTION, val, 0);
    _cfactors.setHeadingCorr(val);
    
    _rollCorrVar = _file.readDoubleVar(ROLL_CORRECTION, val, 0);
    _cfactors.setRollCorr(val);
  
    _pitchCorrVar = _file.readDoubleVar(PITCH_CORRECTION, val, 0);
    _cfactors.setPitchCorr(val);
    
    _driftCorrVar = _file.readDoubleVar(DRIFT_CORRECTION, val, 0);
    _cfactors.setDriftCorr(val);

    _rotationCorrVar = _file.readDoubleVar(ROTATION_CORRECTION, val, 0);
    _cfactors.setRotationCorr(val);

    _tiltCorrVar = _file.readDoubleVar(TILT_CORRECTION, val, 0);
    _cfactors.setTiltCorr(val);

  } catch (NcxxException e) {
  }

  return 0;
  
}

///////////////////////////////////
// read the position variables

int NcxxRadxFile::_readPositionVariables()

{

  // time

  _georefTimeVar = _file.getVar(GEOREF_TIME);
  if (!_georefTimeVar.isNull()) {
    if (_georefTimeVar.numVals() < 1) {
      _addErrStr("ERROR - NcxxRadxFile::_readPositionVariables");
      _addErrStr("  Cannot read georef time");
      _addErrStr(_file.getErrStr());
      return -1;
    }
    if (_georefTimeVar.getType() != ncxxDouble) {
      _addErrStr("ERROR - NcxxRadxFile::_readPositionVariables");
      _addErrStr("  georef time is incorrect type: ", 
                 Ncxx::ncxxTypeToStr(_georefTimeVar.getType()));
      _addErrStr("  expecting type: double");
      return -1;
    }
  }

  // find latitude, longitude, altitude

  _latitudeVar = _file.getVar(LATITUDE);
  if (!_latitudeVar.isNull()) {
    if (_latitudeVar.numVals() < 1) {
      _addErrStr("ERROR - NcxxRadxFile::_readPositionVariables");
      _addErrStr("  Cannot read latitude");
      _addErrStr(_file.getErrStr());
      return -1;
    }
    if (_latitudeVar.getType() != ncxxDouble) {
      _addErrStr("ERROR - NcxxRadxFile::_readPositionVariables");
      _addErrStr("  latitude is incorrect type: ", 
                 Ncxx::ncTypeToStr(_latitudeVar.getType().getId()));
      _addErrStr("  expecting type: double");
      return -1;
    }
  } else {
    cerr << "WARNING - NcxxRadxFile::_readPositionVariables" << endl;
    cerr << "  No latitude variable" << endl;
    cerr << "  Setting latitude to 0" << endl;
  }

  _longitudeVar = _file.getVar(LONGITUDE);
  if (!_longitudeVar.isNull()) {
    if (_longitudeVar.numVals() < 1) {
      _addErrStr("ERROR - NcxxRadxFile::_readPositionVariables");
      _addErrStr("  Cannot read longitude");
      _addErrStr(_file.getErrStr());
      return -1;
    }
    if (_longitudeVar.getType() != ncxxDouble) {
      _addErrStr("ERROR - NcxxRadxFile::_readPositionVariables");
      _addErrStr("  longitude is incorrect type: ",
                 Ncxx::ncTypeToStr(_longitudeVar.getType().getId()));
      _addErrStr("  expecting type: double");
      return -1;
    }
  } else {
    cerr << "WARNING - NcxxRadxFile::_readPositionVariables" << endl;
    cerr << "  No longitude variable" << endl;
    cerr << "  Setting longitude to 0" << endl;
  }

  _altitudeVar = _file.getVar(ALTITUDE);
  if (!_altitudeVar.isNull()) {
    if (_altitudeVar.numVals() < 1) {
      _addErrStr("ERROR - NcxxRadxFile::_readPositionVariables");
      _addErrStr("  Cannot read altitude");
      _addErrStr(_file.getErrStr());
      return -1;
    }
    if (_altitudeVar.getType() != ncxxDouble) {
      _addErrStr("ERROR - NcxxRadxFile::_readPositionVariables");
      _addErrStr("  altitude is incorrect type: ",
                 Ncxx::ncTypeToStr(_altitudeVar.getType().getId()));
      _addErrStr("  expecting type: double");
      return -1;
    }
  } else {
    cerr << "WARNING - NcxxRadxFile::_readPositionVariables" << endl;
    cerr << "  No altitude variable" << endl;
    cerr << "  Setting altitude to 0" << endl;
  }

  _altitudeAglVar = _file.getVar(ALTITUDE_AGL);
  if (!_altitudeAglVar.isNull()) {
    if (_altitudeAglVar.numVals() < 1) {
      _addErrStr("WARNING - NcxxRadxFile::_readPositionVariables");
      _addErrStr("  Bad variable - altitudeAgl");
      _addErrStr(_file.getErrStr());
    }
    if (_altitudeAglVar.getType() != ncxxDouble) {
      _addErrStr("WARNING - NcxxRadxFile::_readPositionVariables");
      _addErrStr("  altitudeAgl is incorrect type: ",
                 Ncxx::ncTypeToStr(_altitudeAglVar.getType().getId()));
      _addErrStr("  expecting type: double");
    }
  }

  // set variables

  if (!_latitudeVar.isNull()) {
    double *data = new double[_latitudeVar.numVals()];
    try {
      _latitudeVar.getVal(data);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - NcxxRadxFile::_readPositionVariables");
      _addErrStr("  Cannot read var, name: ", _latitudeVar.getName());
      _addErrStr("  exception: ", e.what());
      return -1;
    }
    for (int ii = 0; ii < _latitudeVar.numVals(); ii++) {
      _latitude.push_back(data[ii]);
    }
    delete[] data;
  } else {
    _latitude.push_back(0.0);
  }

  if (!_longitudeVar.isNull()) {
    double *data = new double[_longitudeVar.numVals()];
    try {
      _longitudeVar.getVal(data);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - NcxxRadxFile::_readPositionVariables");
      _addErrStr("  Cannot read var, name", _longitudeVar.getName());
      _addErrStr("  exception: ", e.what());
      return -1;
    }
    for (int ii = 0; ii < _longitudeVar.numVals(); ii++) {
      _longitude.push_back(data[ii]);
    }
    delete[] data;
  } else {
    _longitude.push_back(0.0);
  }

  if (!_altitudeVar.isNull()) {
    double *data = new double[_altitudeVar.numVals()];
    try {
      _altitudeVar.getVal(data);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - NcxxRadxFile::_readPositionVariables");
      _addErrStr("  Cannot read var, name", _altitudeVar.getName());
      _addErrStr("  exception: ", e.what());
      return -1;
    }
    for (int ii = 0; ii < _altitudeVar.numVals(); ii++) {
      _altitude.push_back(data[ii]);
    }
    delete[] data;
  } else {
    _altitude.push_back(0.0);
  }

  if (!_altitudeAglVar.isNull()) {
    double *data = new double[_altitudeAglVar.numVals()];
    try {
      _altitudeAglVar.getVal(data);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - NcxxRadxFile::_readPositionVariables");
      _addErrStr("  Cannot read var, name", _altitudeAglVar.getName());
      _addErrStr("  exception: ", e.what());
      return -1;
    }
    for (int ii = 0; ii < _altitudeAglVar.numVals(); ii++) {
      _altitudeAgl.push_back(data[ii]);
    }
    delete[] data;
  } else {
    _altitudeAgl.push_back(0.0);
  }

  return 0;

}

///////////////////////////////////
// read the sweep meta-data

int NcxxRadxFile::_readSweepVariables()

{

  // create vector for the sweeps

  size_t nSweepsInFile = _sweepDim.getSize();

  // initialize
  
  vector<int> sweepNums, startRayIndexes, endRayIndexes;
  vector<double> fixedAngles, targetScanRates, rayAngleRes, intermedFreqHz;
  vector<string> sweepModes, polModes, prtModes, followModes, raysAreIndexed;

  int iret = 0;
  
  _readSweepVar(_sweepNumberVar, SWEEP_NUMBER, sweepNums);
  if (sweepNums.size() < nSweepsInFile) {
    iret = -1;
  }

  _readSweepVar(_sweepStartRayIndexVar, SWEEP_START_RAY_INDEX, startRayIndexes);
  if (startRayIndexes.size() < nSweepsInFile) {
    iret = -1;
  }

  _readSweepVar(_sweepEndRayIndexVar, SWEEP_END_RAY_INDEX, endRayIndexes);
  if (endRayIndexes.size() < nSweepsInFile) {
    iret = -1;
  }

  // _sweepFixedAngleVar = NULL;
  _readSweepVar(_sweepFixedAngleVar, FIXED_ANGLE, fixedAngles, false);
  if (_sweepFixedAngleVar.isNull()) {
    // try old string
    _readSweepVar(_sweepFixedAngleVar, "sweep_fixed_angle", fixedAngles, false);
  }
  if (_sweepFixedAngleVar.isNull()) {
    _fixedAnglesFound = false;
  } else {
    _fixedAnglesFound = true;
  }

  _readSweepVar(_targetScanRateVar, TARGET_SCAN_RATE, targetScanRates, false);

  _readSweepVar(_sweepModeVar, SWEEP_MODE, sweepModes);
  if (sweepModes.size() < nSweepsInFile) {
    iret = -1;
  }

  _readSweepVar(_polModeVar, POLARIZATION_MODE, polModes, false);
  _readSweepVar(_prtModeVar, PRT_MODE, prtModes, false);
  _readSweepVar(_sweepFollowModeVar, FOLLOW_MODE, followModes, false);
  
  _readSweepVar(_raysAreIndexedVar, RAYS_ARE_INDEXED, raysAreIndexed, false);
  _readSweepVar(_rayAngleResVar, RAY_ANGLE_RES, rayAngleRes, false);
  _readSweepVar(_intermedFreqHzVar, INTERMED_FREQ_HZ, intermedFreqHz, false);

  if (iret) {
    _addErrStr("ERROR - NcxxRadxFile::_readSweepVariables");
    return -1;
  }
  
  _sweepsInFile.clear();
  for (size_t ii = 0; ii < nSweepsInFile; ii++) {

    RadxSweep *sweep = new RadxSweep;
    sweep->setVolumeNumber(_volumeNumber);

    if (sweepNums.size() > ii) {
      sweep->setSweepNumber(sweepNums[ii]);
    }
    if (startRayIndexes.size() > ii) {
      sweep->setStartRayIndex(startRayIndexes[ii]);
    }
    if (endRayIndexes.size() > ii) {
      sweep->setEndRayIndex(endRayIndexes[ii]);
    }
    if (fixedAngles.size() > ii) {
      sweep->setFixedAngleDeg(fixedAngles[ii]);
    }
    if (targetScanRates.size() > ii) {
      sweep->setTargetScanRateDegPerSec(targetScanRates[ii]);
    }
    if (sweepModes.size() > ii) {
      sweep->setSweepMode(Radx::sweepModeFromStr(sweepModes[ii]));
    }
    if (polModes.size() > ii) {
      sweep->setPolarizationMode(Radx::polarizationModeFromStr(polModes[ii]));
    }
    if (prtModes.size() > ii) {
      sweep->setPrtMode(Radx::prtModeFromStr(prtModes[ii]));
    }
    if (followModes.size() > ii) {
      sweep->setFollowMode(Radx::followModeFromStr(followModes[ii]));
    }

    if (raysAreIndexed.size() > ii) {
      if (raysAreIndexed[ii] == "true") {
        sweep->setRaysAreIndexed(true);
      } else {
        sweep->setRaysAreIndexed(false);
      }
    }

    if (rayAngleRes.size() > ii) {
      sweep->setAngleResDeg(rayAngleRes[ii]);
    }

    if (intermedFreqHz.size() > ii) {
      sweep->setIntermedFreqHz(intermedFreqHz[ii]);
    }

    _sweepsInFile.push_back(sweep);
    _sweeps.push_back(sweep);

  } // ii

  return 0;

}

///////////////////////////////////
// clear the georeference vectors

void NcxxRadxFile::_clearGeorefVariables()

{

  _geoTime.clear();
  _geoLatitude.clear();
  _geoLongitude.clear();
  _geoAltitudeMsl.clear();
  _geoAltitudeAgl.clear();
  _geoEwVelocity.clear();
  _geoNsVelocity.clear();
  _geoVertVelocity.clear();
  _geoHeading.clear();
  _geoRoll.clear();
  _geoPitch.clear();
  _geoDrift.clear();
  _geoRotation.clear();
  _geoTilt.clear();
  _geoEwWind.clear();
  _geoNsWind.clear();
  _geoVertWind.clear();
  _geoHeadingRate.clear();
  _geoPitchRate.clear();
  _geoDriveAngle1.clear();
  _geoDriveAngle2.clear();

}

///////////////////////////////////
// read the georeference meta-data

int NcxxRadxFile::_readGeorefVariables()

{

  _clearGeorefVariables();
  int iret = 0;

  _readRayVar(_georefTimeVar, GEOREF_TIME, _geoTime);
  if (_geoTime.size() < _raysFromFile.size()) {
    // iret = -1;
  }

  _readRayVar(_latitudeVar, LATITUDE, _geoLatitude);
  if (_geoLatitude.size() < _raysFromFile.size()) {
    iret = -1;
  }

  _readRayVar(_longitudeVar, LONGITUDE, _geoLongitude);
  if (_geoLongitude.size() < _raysFromFile.size()) {
    iret = -1;
  }

  _readRayVar(_altitudeVar, ALTITUDE, _geoAltitudeMsl); // meters
  if (_geoAltitudeMsl.size() < _raysFromFile.size()) {
    iret = -1;
  }

  _readRayVar(_altitudeAglVar, ALTITUDE_AGL, _geoAltitudeAgl, false); // meters

  _readRayVar(EASTWARD_VELOCITY, _geoEwVelocity, false);
  _readRayVar(NORTHWARD_VELOCITY, _geoNsVelocity, false);
  _readRayVar(VERTICAL_VELOCITY, _geoVertVelocity, false);

  _readRayVar(HEADING, _geoHeading, false);
  _readRayVar(ROLL, _geoRoll, false);
  _readRayVar(PITCH, _geoPitch, false);

  _readRayVar(DRIFT, _geoDrift, false);
  _readRayVar(ROTATION, _geoRotation, false);
  _readRayVar(TILT, _geoTilt, false);

  _readRayVar(EASTWARD_WIND, _geoEwWind, false);
  _readRayVar(NORTHWARD_WIND, _geoNsWind, false);
  _readRayVar(VERTICAL_WIND, _geoVertWind, false);

  _readRayVar(HEADING_CHANGE_RATE, _geoHeadingRate, false);
  _readRayVar(PITCH_CHANGE_RATE, _geoPitchRate, false);
  _readRayVar(DRIVE_ANGLE_1, _geoDriveAngle1, false);
  _readRayVar(DRIVE_ANGLE_2, _geoDriveAngle2, false);

  if (iret) {
    _addErrStr("ERROR - NcxxRadxFile::_readGeorefVariables");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// clear the ray variables

void NcxxRadxFile::_clearRayVariables()

{

  _rayAzimuths.clear();
  _rayElevations.clear();
  _rayPulseWidths.clear();
  _rayPrts.clear();
  _rayPrtRatios.clear();
  _rayNyquists.clear();
  _rayUnambigRanges.clear();
  _rayAntennaTransitions.clear();
  _rayGeorefsApplied.clear();
  _rayNSamples.clear();
  _rayCalNum.clear();
  _rayXmitPowerH.clear();
  _rayXmitPowerV.clear();
  _rayScanRate.clear();
  _rayEstNoiseDbmHc.clear();
  _rayEstNoiseDbmVc.clear();
  _rayEstNoiseDbmHx.clear();
  _rayEstNoiseDbmVx.clear();

}

///////////////////////////////////
// read in ray variables

int NcxxRadxFile::_readRayVariables()

{

  _clearRayVariables();
  int iret = 0;

  _readRayVar(_azimuthVar, AZIMUTH, _rayAzimuths);
  if (_rayAzimuths.size() < _raysFromFile.size()) {
    _addErrStr("ERROR - azimuth variable required");
    iret = -1;
  }

  // HSRL?

  if (_readRayVar(_elevationVar, "telescope_roll_angle_offset",
                  _rayElevations, true) == 0) {
    // is HSRL
    for (size_t ii = 0; ii < _rayElevations.size(); ii++) {
      _rayElevations[ii] *= -1.0;
    }
    for (size_t ii = 0; ii < _rayAzimuths.size(); ii++) {
      _rayAzimuths[ii] = 0.0;
    }
  } else {
    // not HSRL
    clearErrStr();
    _readRayVar(_elevationVar, ELEVATION, _rayElevations);
  }
  if (_rayElevations.size() < _raysFromFile.size()) {
    _addErrStr("ERROR - elevation variable required");
    iret = -1;
  }

  _readRayVar(_pulseWidthVar, PULSE_WIDTH, _rayPulseWidths, false);
  _readRayVar(_prtVar, PRT, _rayPrts, false);
  _readRayVar(_prtRatioVar, PRT_RATIO, _rayPrtRatios, false);
  _readRayVar(_nyquistVar, NYQUIST_VELOCITY, _rayNyquists, false);
  _readRayVar(_unambigRangeVar, UNAMBIGUOUS_RANGE, _rayUnambigRanges, false);
  _readRayVar(_antennaTransitionVar, ANTENNA_TRANSITION, 
              _rayAntennaTransitions, false);
  _readRayVar(_georefsAppliedVar, GEOREFS_APPLIED,
              _rayGeorefsApplied, false);
  _readRayVar(_nSamplesVar, N_SAMPLES, _rayNSamples, false);
  _readRayVar(_calIndexVar, R_CALIB_INDEX, _rayCalNum, false);
  _readRayVar(_xmitPowerHVar, RADAR_MEASURED_TRANSMIT_POWER_H, 
              _rayXmitPowerH, false);
  _readRayVar(_xmitPowerVVar, RADAR_MEASURED_TRANSMIT_POWER_V, 
              _rayXmitPowerV, false);
  _readRayVar(_scanRateVar, SCAN_RATE, _rayScanRate, false);
  _readRayVar(_estNoiseDbmHcVar, RADAR_ESTIMATED_NOISE_DBM_HC,
              _rayEstNoiseDbmHc, false);
  _readRayVar(_estNoiseDbmVcVar, RADAR_ESTIMATED_NOISE_DBM_VC,
              _rayEstNoiseDbmVc, false);
  _readRayVar(_estNoiseDbmHxVar, RADAR_ESTIMATED_NOISE_DBM_HX,
              _rayEstNoiseDbmHx, false);
  _readRayVar(_estNoiseDbmVxVar, RADAR_ESTIMATED_NOISE_DBM_VX,
              _rayEstNoiseDbmVx, false);
  
  if (iret) {
    _addErrStr("ERROR - NcxxRadxFile::_readRayVariables");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// create the rays to be read in
// and set meta data

int NcxxRadxFile::_createRays(const string &path)

{

  // read in the ray gate geometry
  // or if not available, use global geometry
  
  _readRayGateGeom();

  // compile a list of the rays to be read in, using the list of
  // sweeps to read

  _raysToRead.clear();

  for (size_t isweep = 0; isweep < _sweepsToRead.size(); isweep++) {

    if (path != _sweepsToRead[isweep].path) {
      // the references sweep is not in this file
      continue;
    }

    RadxSweep *sweep = _sweepsInFile[_sweepsToRead[isweep].indexInFile];

    for (size_t ii = sweep->getStartRayIndex();
         ii <= sweep->getEndRayIndex(); ii++) {

      // add ray to list to be read

      RayInfo info;
      info.indexInFile = ii;
      info.sweep = sweep;
      _raysToRead.push_back(info);

    } // ii

  } // isweep

  // create the rays to be read
  
  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {

    size_t rayIndex = _raysToRead[ii].indexInFile;
    const RadxSweep *sweep = _raysToRead[ii].sweep;

    // new ray

    RadxRay *ray = new RadxRay;
    ray->setRangeGeom(_rayStartRange[rayIndex] / 1000.0,
                      _rayGateSpacing[rayIndex] / 1000.0);
    
    // set time
    
    double rayTimeDouble = _dTimes[rayIndex];
    time_t rayUtimeSecs = _refTimeSecsFile + (time_t) rayTimeDouble;
    double rayIntSecs;
    double rayFracSecs = modf(rayTimeDouble, &rayIntSecs);
    int rayNanoSecs = (int) (rayFracSecs * 1.0e9);

    if (rayIntSecs < 0 || rayNanoSecs < 0) {
      rayUtimeSecs -= 1;
      rayNanoSecs = 1000000000 + rayNanoSecs;
    }
      
    ray->setTime(rayUtimeSecs, rayNanoSecs);

    // sweep info

    ray->setSweepNumber(sweep->getSweepNumber());
    ray->setSweepMode(sweep->getSweepMode());
    ray->setPolarizationMode(sweep->getPolarizationMode());
    ray->setPrtMode(sweep->getPrtMode());
    ray->setFollowMode(sweep->getFollowMode());
    ray->setFixedAngleDeg(sweep->getFixedAngleDeg());
    ray->setTargetScanRateDegPerSec(sweep->getTargetScanRateDegPerSec());
    ray->setIsIndexed(sweep->getRaysAreIndexed());
    ray->setAngleResDeg(sweep->getAngleResDeg());

    if (_rayAzimuths.size() > rayIndex) {
      ray->setAzimuthDeg(_rayAzimuths[rayIndex]);
    }
    if (_rayElevations.size() > rayIndex) {
      ray->setElevationDeg(_rayElevations[rayIndex]);
    }
    if (_rayPulseWidths.size() > rayIndex) {
      ray->setPulseWidthUsec(_rayPulseWidths[rayIndex] * 1.0e6);
    }
    if (_rayPrts.size() > rayIndex) {
      ray->setPrtSec(_rayPrts[rayIndex]);
    }
    if (_rayPrtRatios.size() > rayIndex) {
      ray->setPrtRatio(_rayPrtRatios[rayIndex]);
    }
    if (_rayNyquists.size() > rayIndex) {
      ray->setNyquistMps(_rayNyquists[rayIndex]);
    }
    if (_rayUnambigRanges.size() > rayIndex) {
      if (_rayUnambigRanges[rayIndex] > 0) {
        ray->setUnambigRangeKm(_rayUnambigRanges[rayIndex] / 1000.0);
      }
    }
    if (_rayAntennaTransitions.size() > rayIndex) {
      ray->setAntennaTransition(_rayAntennaTransitions[rayIndex]);
    }
    if (_rayGeorefsApplied.size() > rayIndex) {
      ray->setGeorefApplied(_rayGeorefsApplied[rayIndex]);
    }
    if (_rayNSamples.size() > rayIndex) {
      ray->setNSamples(_rayNSamples[rayIndex]);
    }
    if (_rayCalNum.size() > rayIndex) {
      ray->setCalibIndex(_rayCalNum[rayIndex]);
    }
    if (_rayXmitPowerH.size() > rayIndex) {
      ray->setMeasXmitPowerDbmH(_rayXmitPowerH[rayIndex]);
    }
    if (_rayXmitPowerV.size() > rayIndex) {
      ray->setMeasXmitPowerDbmV(_rayXmitPowerV[rayIndex]);
    }
    if (_rayScanRate.size() > rayIndex) {
      ray->setTrueScanRateDegPerSec(_rayScanRate[rayIndex]);
    }
    if (_rayEstNoiseDbmHc.size() > rayIndex) {
      ray->setEstimatedNoiseDbmHc(_rayEstNoiseDbmHc[rayIndex]);
    }
    if (_rayEstNoiseDbmVc.size() > rayIndex) {
      ray->setEstimatedNoiseDbmVc(_rayEstNoiseDbmVc[rayIndex]);
    }
    if (_rayEstNoiseDbmHx.size() > rayIndex) {
      ray->setEstimatedNoiseDbmHx(_rayEstNoiseDbmHx[rayIndex]);
    }
    if (_rayEstNoiseDbmVx.size() > rayIndex) {
      ray->setEstimatedNoiseDbmVx(_rayEstNoiseDbmVx[rayIndex]);
    }

    if (_georefsActive) {

      RadxGeoref geo;
      
      if (_geoTime.size() > rayIndex) {
        double geoTime = _geoTime[rayIndex];
        int secs = (int) geoTime;
        int nanoSecs = (int) ((geoTime - secs) * 1.0e9 + 0.5);
        time_t tSecs = _readVol->getStartTimeSecs() + secs;
        geo.setTimeSecs(tSecs);
        geo.setNanoSecs(nanoSecs);
      }
      if (_geoLatitude.size() > rayIndex) {
        geo.setLatitude(_geoLatitude[rayIndex]);
      }
      if (_geoLongitude.size() > rayIndex) {
        geo.setLongitude(_geoLongitude[rayIndex]);
      }
      if (_geoAltitudeMsl.size() > rayIndex) {
        geo.setAltitudeKmMsl(_geoAltitudeMsl[rayIndex] / 1000.0);
      }
      if (_geoAltitudeAgl.size() > rayIndex) {
        geo.setAltitudeKmAgl(_geoAltitudeAgl[rayIndex] / 1000.0);
      }
      if (_geoEwVelocity.size() > rayIndex) {
        geo.setEwVelocity(_geoEwVelocity[rayIndex]);
      }
      if (_geoNsVelocity.size() > rayIndex) {
        geo.setNsVelocity(_geoNsVelocity[rayIndex]);
      }
      if (_geoVertVelocity.size() > rayIndex) {
        geo.setVertVelocity(_geoVertVelocity[rayIndex]);
      }
      if (_geoHeading.size() > rayIndex) {
        geo.setHeading(_geoHeading[rayIndex]);
      }
      if (_geoRoll.size() > rayIndex) {
        geo.setRoll(_geoRoll[rayIndex]);
      }
      if (_geoPitch.size() > rayIndex) {
        geo.setPitch(_geoPitch[rayIndex]);
      }
      if (_geoDrift.size() > rayIndex) {
        geo.setDrift(_geoDrift[rayIndex]);
      }
      if (_geoRotation.size() > rayIndex) {
        geo.setRotation(_geoRotation[rayIndex]);
      }
      if (_geoTilt.size() > rayIndex) {
        geo.setTilt(_geoTilt[rayIndex]);
      }
      if (_geoEwWind.size() > rayIndex) {
        geo.setEwWind(_geoEwWind[rayIndex]);
      }
      if (_geoNsWind.size() > rayIndex) {
        geo.setNsWind(_geoNsWind[rayIndex]);
      }
      if (_geoVertWind.size() > rayIndex) {
        geo.setVertWind(_geoVertWind[rayIndex]);
      }
      if (_geoHeadingRate.size() > rayIndex) {
        geo.setHeadingRate(_geoHeadingRate[rayIndex]);
      }
      if (_geoPitchRate.size() > rayIndex) {
        geo.setPitchRate(_geoPitchRate[rayIndex]);
      }
      if (_geoDriveAngle1.size() > rayIndex) {
        geo.setDriveAngle1(_geoDriveAngle1[rayIndex]);
      }
      if (_geoDriveAngle2.size() > rayIndex) {
        geo.setDriveAngle2(_geoDriveAngle2[rayIndex]);
      }
      
      ray->setGeoref(geo);

    } // if (_georefsActive) 
  
    // add to ray vector

    _raysFromFile.push_back(ray);

  } // ii

  // free up

  _rayStartRange.clear();
  _rayGateSpacing.clear();

  return 0;

}

///////////////////////////////////
// read the frequency variable

int NcxxRadxFile::_readFrequencyVariable()

{

  _frequency.clear();
  _frequencyVar = _file.getVar(FREQUENCY);
  if (_frequencyVar.isNull()) {
    return 0;
  }

  int nFreq = _frequencyVar.numVals();
  double *freq = new double[nFreq];
  try {
    _frequencyVar.getVal(freq);
    for (int ii = 0; ii < nFreq; ii++) {
      _frequency.push_back(freq[ii]);
    }
  } catch (NcxxException& e) {
  }
  delete[] freq;

  return 0;

}

/////////////////////////////////////
// read the geometry, if available
// if not available, use main geometry for all rays

void NcxxRadxFile::_readRayGateGeom()

{

  // clear

  _rayStartRange.clear();
  _rayGateSpacing.clear();
  _gateGeomVaries = false;

  // read start range and gate spacing if available

  bool rayGateGeomAvail = true;
  if (_readRayVar(_rayStartRangeVar, RAY_START_RANGE, _rayStartRange)) {
    rayGateGeomAvail = false;
  }
  if (_readRayVar(_rayGateSpacingVar, RAY_GATE_SPACING, _rayGateSpacing)) {
    rayGateGeomAvail = false;
  }
  
  if (rayGateGeomAvail) {
    // check if gate geometry varies between rays
    if (_nTimesInFile > 0) {
      for (size_t ii = 1; ii < _nTimesInFile; ii++) {
        if (_rayStartRange[ii] != _rayStartRange[0]) {
          _gateGeomVaries = true;
          break;
        }
        if (_rayGateSpacing[ii] != _rayGateSpacing[0]) {
          _gateGeomVaries = true;
          break;
        }
      }
      if (!_gateGeomVaries) {
        _geom.setRangeGeom(_rayStartRange[0] / 1000.0, _rayGateSpacing[0] / 1000.0);
      }
    }
  } else {
    for (size_t ii = 0; ii < _nTimesInFile; ii++) {
      // range geom not set on ray-by-ray basis
      // use global values instead
      _rayStartRange.push_back(_geom.getStartRangeKm() * 1000.0);
      _rayGateSpacing.push_back(_geom.getGateSpacingKm() * 1000.0);
    }
  }
  
}

/////////////////////////////////////
// read the ngates and offsets arrays

int NcxxRadxFile::_readRayNgatesAndOffsets()

{

  // clear

  _rayNGates.clear();
  _rayStartIndex.clear();

  // for constant number of gates, compute start indices
  
  if (!_nGatesVary) {
    _nPoints = 0;
    for (size_t ii = 0; ii < _nTimesInFile; ii++) {
      _rayNGates.push_back(_nRangeInFile);
      _rayStartIndex.push_back(_nPoints);
      _nPoints += _nRangeInFile;
    }
    return 0;
  }
  
  // non-constant nGates - read in arrays

  int iret = 0;

  if (_readRayVar(_rayNGatesVar, RAY_N_GATES, _rayNGates)) {
    _addErrStr("ERROR - NcxxRadxFile::_readRayNGatesAndOffsets");
    iret = -1;
  }
  
  if (_readRayVar(_rayStartIndexVar, RAY_START_INDEX, _rayStartIndex)) {
    _addErrStr("ERROR - NcxxRadxFile::_readRayNGatesAndOffsets");
    iret = -1;
  }

  return iret;

}

///////////////////////////////////
// read the calibration variables

int NcxxRadxFile::_readCalibrationVariables()

{

  if (_calDim.isNull()) {
    // no cal available
    return 0;
  }
  
  int iret = 0;
  for (size_t ii = 0; ii < _calDim.getSize(); ii++) {
    RadxRcalib *cal = new RadxRcalib;
    if (_readCal(*cal, ii)) {
      _addErrStr("ERROR - NcxxRadxFile::_readCalibrationVariables");
      _addErrStr("  calibration required, but error on read");
      iret = -1;
    }
    // check that this is not a duplicate
    bool alreadyAdded = false;
    for (size_t ii = 0; ii < _rCals.size(); ii++) {
      const RadxRcalib *rcal = _rCals[ii];
      if (fabs(rcal->getPulseWidthUsec()
               - cal->getPulseWidthUsec()) < 0.0001) {
        alreadyAdded = true;
      }
    }
    if (!alreadyAdded) {
      _rCals.push_back(cal);
    }
  } // ii

  return iret;

}
  
int NcxxRadxFile::_readCal(RadxRcalib &cal, int index)

{

  int iret = 0;
  double val;
  time_t ctime;

  // time

  if (_readCalTime(R_CALIB_TIME, 
                   _rCalTimeVar, index, ctime) == 0) {
    cal.setCalibTime(ctime);
  }

  // pulse width

  iret |= _readCalVar(R_CALIB_PULSE_WIDTH, 
                      _rCalPulseWidthVar, index, val, true);
  cal.setPulseWidthUsec(val * 1.0e6);

  // xmit power

  if (_readCalVar(R_CALIB_XMIT_POWER_H, 
                  _rCalXmitPowerHVar, index, val) == 0) {
    cal.setXmitPowerDbmH(val);
  }

  if (_readCalVar(R_CALIB_XMIT_POWER_V, 
                  _rCalXmitPowerVVar, index, val) == 0) {
    cal.setXmitPowerDbmV(val);
  }

  // waveguide loss

  if (_readCalVar(R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_H,
                  _rCalTwoWayWaveguideLossHVar, index, val) == 0) {
    cal.setTwoWayWaveguideLossDbH(val);
  }

  if (_readCalVar(R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_V,
                  _rCalTwoWayWaveguideLossVVar, index, val) == 0) {
    cal.setTwoWayWaveguideLossDbV(val);
  }

  // radome loss

  if (_readCalVar(R_CALIB_TWO_WAY_RADOME_LOSS_H,
                  _rCalTwoWayRadomeLossHVar, index, val) == 0) {
    cal.setTwoWayRadomeLossDbH(val);
  }

  if (_readCalVar(R_CALIB_TWO_WAY_RADOME_LOSS_V,
                  _rCalTwoWayRadomeLossVVar, index, val) == 0) {
    cal.setTwoWayRadomeLossDbV(val);
  }

  // receiver mismatch loss
  
  if (_readCalVar(R_CALIB_RECEIVER_MISMATCH_LOSS,
                  _rCalReceiverMismatchLossVar, index, val) == 0) {
    cal.setReceiverMismatchLossDb(val);
  }
  
  // k squared water

  if (_readCalVar(R_CALIB_K_SQUARED_WATER,
                  _rCalKSquaredWaterVar, index, val) == 0) {
    cal.setKSquaredWater(val);
  }
  
  // radar constant

  if (_readCalVar(R_CALIB_RADAR_CONSTANT_H, 
                  _rCalRadarConstHVar, index, val) == 0) {
    cal.setRadarConstantH(val);
  }

  if (_readCalVar(R_CALIB_RADAR_CONSTANT_V, 
                  _rCalRadarConstVVar, index, val) == 0) {
    cal.setRadarConstantV(val);
  }

  // antenna gain

  if (_readCalVar(R_CALIB_ANTENNA_GAIN_H, 
                  _rCalAntennaGainHVar, index, val) == 0) {
    cal.setAntennaGainDbH(val);
  }
  
  if (_readCalVar(R_CALIB_ANTENNA_GAIN_V, 
                  _rCalAntennaGainVVar, index, val) == 0) {
    cal.setAntennaGainDbV(val);
  }

  // noise dbm

  if (_readCalVar(R_CALIB_NOISE_HC, 
                  _rCalNoiseHcVar, index, val, true) == 0) {
    cal.setNoiseDbmHc(val);
  }

  if (_readCalVar(R_CALIB_NOISE_HX, 
                  _rCalNoiseHxVar, index, val) == 0) {
    cal.setNoiseDbmHx(val);
  }

  if (_readCalVar(R_CALIB_NOISE_VC, 
                  _rCalNoiseVcVar, index, val) == 0) {
    cal.setNoiseDbmVc(val);
  }

  if (_readCalVar(R_CALIB_NOISE_VX, 
                  _rCalNoiseVxVar, index, val) == 0) {
    cal.setNoiseDbmVx(val);
  }

  // i0 dbm

  if (_readCalVar(R_CALIB_I0_DBM_HC, 
                  _rCalI0HcVar, index, val, true) == 0) {
    cal.setI0DbmHc(val);
  }

  if (_readCalVar(R_CALIB_I0_DBM_HX, 
                  _rCalI0HxVar, index, val) == 0) {
    cal.setI0DbmHx(val);
  }

  if (_readCalVar(R_CALIB_I0_DBM_VC, 
                  _rCalI0VcVar, index, val) == 0) {
    cal.setI0DbmVc(val);
  }

  if (_readCalVar(R_CALIB_I0_DBM_VX, 
                  _rCalI0VxVar, index, val) == 0) {
    cal.setI0DbmVx(val);
  }

  // receiver gain
  
  if (_readCalVar(R_CALIB_RECEIVER_GAIN_HC, 
                  _rCalReceiverGainHcVar, index, val, true) == 0) {
    cal.setReceiverGainDbHc(val);
  }

  if (_readCalVar(R_CALIB_RECEIVER_GAIN_HX, 
                  _rCalReceiverGainHxVar, index, val) == 0) {
    cal.setReceiverGainDbHx(val);
  }

  if (_readCalVar(R_CALIB_RECEIVER_GAIN_VC, 
                  _rCalReceiverGainVcVar, index, val) == 0) {
    cal.setReceiverGainDbVc(val);
  }
  
  if (_readCalVar(R_CALIB_RECEIVER_GAIN_VX, 
                  _rCalReceiverGainVxVar, index, val) == 0) {
    cal.setReceiverGainDbVx(val);
  }

  // receiver slope

  if (_readCalVar(R_CALIB_RECEIVER_SLOPE_HC, 
                  _rCalReceiverSlopeHcVar, index, val) == 0) {
    cal.setReceiverSlopeDbHc(val);
  }

  if (_readCalVar(R_CALIB_RECEIVER_SLOPE_HX, 
                  _rCalReceiverSlopeHxVar, index, val) == 0) {
    cal.setReceiverSlopeDbHx(val);
  }

  if (_readCalVar(R_CALIB_RECEIVER_SLOPE_VC, 
                  _rCalReceiverSlopeVcVar, index, val) == 0) {
    cal.setReceiverSlopeDbVc(val);
  }

  if (_readCalVar(R_CALIB_RECEIVER_SLOPE_VX, 
                  _rCalReceiverSlopeVxVar, index, val) == 0) {
    cal.setReceiverSlopeDbVx(val);
  }

  // dynamic range

  if (_readCalVar(R_CALIB_DYNAMIC_RANGE_DB_HC, 
                  _rCalI0HcVar, index, val, true) == 0) {
    cal.setDynamicRangeDbHc(val);
  }

  if (_readCalVar(R_CALIB_DYNAMIC_RANGE_DB_HX, 
                  _rCalI0HxVar, index, val) == 0) {
    cal.setDynamicRangeDbHx(val);
  }

  if (_readCalVar(R_CALIB_DYNAMIC_RANGE_DB_VC, 
                  _rCalI0VcVar, index, val) == 0) {
    cal.setDynamicRangeDbVc(val);
  }

  if (_readCalVar(R_CALIB_DYNAMIC_RANGE_DB_VX, 
                  _rCalI0VxVar, index, val) == 0) {
    cal.setDynamicRangeDbVx(val);
  }

  // base dbz 1km

  if (_readCalVar(R_CALIB_BASE_DBZ_1KM_HC, 
                  _rCalBaseDbz1kmHcVar, index, val) == 0) {
    cal.setBaseDbz1kmHc(val);
  }

  if (_readCalVar(R_CALIB_BASE_DBZ_1KM_HX, 
                  _rCalBaseDbz1kmHxVar, index, val) == 0) {
    cal.setBaseDbz1kmHx(val);
  }

  if (_readCalVar(R_CALIB_BASE_DBZ_1KM_VC, 
                  _rCalBaseDbz1kmVcVar, index, val) == 0) {
    cal.setBaseDbz1kmVc(val);
  }

  if (_readCalVar(R_CALIB_BASE_DBZ_1KM_VX, 
                  _rCalBaseDbz1kmVxVar, index, val) == 0) {
    cal.setBaseDbz1kmVx(val);
  }

  // sun power

  if (_readCalVar(R_CALIB_SUN_POWER_HC, 
                  _rCalSunPowerHcVar, index, val) == 0) {
    cal.setSunPowerDbmHc(val);
  }

  if (_readCalVar(R_CALIB_SUN_POWER_HX, 
                  _rCalSunPowerHxVar, index, val) == 0) {
    cal.setSunPowerDbmHx(val);
  }

  if (_readCalVar(R_CALIB_SUN_POWER_VC, 
                  _rCalSunPowerVcVar, index, val) == 0) {
    cal.setSunPowerDbmVc(val);
  }

  if (_readCalVar(R_CALIB_SUN_POWER_VX, 
                  _rCalSunPowerVxVar, index, val) == 0) {
    cal.setSunPowerDbmVx(val);
  }

  // noise source power

  if (_readCalVar(R_CALIB_NOISE_SOURCE_POWER_H, 
                  _rCalNoiseSourcePowerHVar, index, val) == 0) {
    cal.setNoiseSourcePowerDbmH(val);
  }

  if (_readCalVar(R_CALIB_NOISE_SOURCE_POWER_V, 
                  _rCalNoiseSourcePowerVVar, index, val) == 0) {
    cal.setNoiseSourcePowerDbmV(val);
  }

  // power measurement loss

  if (_readCalVar(R_CALIB_POWER_MEASURE_LOSS_H, 
                  _rCalPowerMeasLossHVar, index, val) == 0) {
    cal.setPowerMeasLossDbH(val);
  }

  if (_readCalVar(R_CALIB_POWER_MEASURE_LOSS_V, 
                  _rCalPowerMeasLossVVar, index, val) == 0) {
    cal.setPowerMeasLossDbV(val);
  }

  // coupler loss

  if (_readCalVar(R_CALIB_COUPLER_FORWARD_LOSS_H, 
                  _rCalCouplerForwardLossHVar, index, val) == 0) {
    cal.setCouplerForwardLossDbH(val);
  }

  if (_readCalVar(R_CALIB_COUPLER_FORWARD_LOSS_V, 
                  _rCalCouplerForwardLossVVar, index, val) == 0) {
    cal.setCouplerForwardLossDbV(val);
  }

  // corrections

  if (_readCalVar(R_CALIB_DBZ_CORRECTION, 
                  _rCalDbzCorrectionVar, index, val) == 0) {
    cal.setDbzCorrection(val);
  }

  if (_readCalVar(R_CALIB_ZDR_CORRECTION, 
                  _rCalZdrCorrectionVar, index, val) == 0) {
    cal.setZdrCorrectionDb(val);
  }

  if (_readCalVar(R_CALIB_LDR_CORRECTION_H, 
                  _rCalLdrCorrectionHVar, index, val) == 0) {
    cal.setLdrCorrectionDbH(val);
  }

  if (_readCalVar(R_CALIB_LDR_CORRECTION_V, 
                  _rCalLdrCorrectionVVar, index, val) == 0) {
    cal.setLdrCorrectionDbV(val);
  }

  if (_readCalVar(R_CALIB_SYSTEM_PHIDP, 
                  _rCalSystemPhidpVar, index, val) == 0) {
    cal.setSystemPhidpDeg(val);
  }

  // test power

  if (_readCalVar(R_CALIB_TEST_POWER_H, 
                  _rCalTestPowerHVar, index, val) == 0) {
    cal.setTestPowerDbmH(val);
  }

  if (_readCalVar(R_CALIB_TEST_POWER_V, 
                  _rCalTestPowerVVar, index, val) == 0) {
    cal.setTestPowerDbmV(val);
  }

  return iret;

}

////////////////////////////////////////////
// read the field variables

int NcxxRadxFile::_readFieldVariables(bool metaOnly)

{

  // loop through the variables, adding data fields as appropriate
  
  const multimap<string, NcxxVar> &vars = _file.getVars();

  for (multimap<string, NcxxVar>::const_iterator iter = vars.begin();
       iter != vars.end(); iter++) {
    
    NcxxVar var = iter->second;
    if (var.isNull()) {
      continue;
    }
    string name = var.getName();
    
    int numDims = var.getDimCount();
    if (_nGatesVary) {
      // variable number of gates per ray
      // we need fields with 1 dimension
      if (numDims != 1) {
        continue;
      }
      const NcxxDim& nPointsDim = var.getDim(0);
      if (nPointsDim != _nPointsDim) {
        continue;
      }
    } else {
      // constant number of gates per ray
      // we need fields with 2 dimensions
      if (numDims != 2) {
        continue;
      }
      // check that we have the correct dimensions
      const NcxxDim &timeDim = var.getDim(0);
      const NcxxDim &rangeDim = var.getDim(1);
      if (timeDim != _timeDim || rangeDim != _rangeDim) {
        continue;
      }
    }
    
    // check the type
    NcxxType ftype = var.getType();
    if (ftype != ncxxDouble && ftype != ncxxFloat && ftype != ncxxInt &&
        ftype != ncxxShort && ftype != ncxxByte) {
      // not a valid type
      continue;
    }

    // check that we need this field
    
    if (!isFieldRequiredOnRead(name)) {
      if (_verbose) {
        cerr << "DEBUG - NcxxRadxFile::_readFieldVariables" << endl;
        cerr << "  -->> rejecting field: " << name << endl;
      }
      continue;
    }
    if (name == "range") {
      if (_verbose) {
        cerr << "DEBUG - NcxxRadxFile::_readFieldVariables" << endl;
        cerr << "  -->> ignoring dimension variable: " << name << endl;
      }
      continue;
    }

    if (_verbose) {
      cerr << "DEBUG - NcxxRadxFile::_readFieldVariables" << endl;
      cerr << "  -->> adding field: " << name << endl;
    }

    // set names, units, etc
    
    string standardName;
    try {
      NcxxVarAtt standardNameAtt = var.getAtt(STANDARD_NAME);
      if (!standardNameAtt.isNull()) {
        standardName = standardNameAtt.asString();
      }
    } catch (NcxxException& e) {
    }

    string longName;
    try {
      NcxxVarAtt longNameAtt = var.getAtt(LONG_NAME);
      longName = longNameAtt.asString();
    } catch (NcxxException& e) {
    }

    string units;
    try {
      NcxxVarAtt unitsAtt = var.getAtt(UNITS);
      units = unitsAtt.asString();
    } catch (NcxxException& e) {
    }

    string legendXml;
    try {
      NcxxVarAtt legendXmlAtt = var.getAtt(LEGEND_XML);
      if (!legendXmlAtt.isNull()) {
        legendXml = legendXmlAtt.asString();
      }
    } catch (NcxxException& e) {
    }

    string thresholdingXml;
    try {
      NcxxVarAtt thresholdingXmlAtt = var.getAtt(THRESHOLDING_XML);
      thresholdingXml = thresholdingXmlAtt.asString();
    } catch (NcxxException& e) {
    }
    
    float samplingRatio = Radx::missingMetaFloat;
    try {
      NcxxVarAtt samplingRatioAtt = var.getAtt(SAMPLING_RATIO);
      vector<float> vals;
      try {
        samplingRatioAtt.getValues(vals);
        samplingRatio = vals[0];
      } catch (NcxxException& e) {
      }
    } catch (NcxxException& e) {
    }

    // folding

    bool fieldFolds = false;
    float foldLimitLower = Radx::missingMetaFloat;
    float foldLimitUpper = Radx::missingMetaFloat;
    try {
      NcxxVarAtt fieldFoldsAtt = var.getAtt(FIELD_FOLDS);
      string fieldFoldsStr = fieldFoldsAtt.asString();
      if (fieldFoldsStr == "true"
          || fieldFoldsStr == "TRUE"
          || fieldFoldsStr == "True") {
        fieldFolds = true;
        try {
          NcxxVarAtt foldLimitLowerAtt = var.getAtt(FOLD_LIMIT_LOWER);
          vector<float> vals;
          try {
            foldLimitLowerAtt.getValues(vals);
            foldLimitLower = vals[0];
          } catch (NcxxException& e) {
          }
        } catch (NcxxException& e) {
        }
        try {
          NcxxVarAtt foldLimitUpperAtt = var.getAtt(FOLD_LIMIT_UPPER);
          vector<float> vals;
          try {
            foldLimitUpperAtt.getValues(vals);
            foldLimitUpper = vals[0];
          } catch (NcxxException& e) {
          }
        } catch (NcxxException& e) {
        }
      }
    } catch (NcxxException& e) {
    }

    // is this field discrete

    bool isDiscrete = false;
    try {
      NcxxVarAtt isDiscreteAtt = var.getAtt(IS_DISCRETE);
      string isDiscreteStr = isDiscreteAtt.asString();
      if (isDiscreteStr == "true"
          || isDiscreteStr == "TRUE"
          || isDiscreteStr == "True") {
        isDiscrete = true;
      }
    } catch (NcxxException& e) {
    }

    // get offset and scale

    double offset = 0.0;
    try {
      NcxxVarAtt offsetAtt = var.getAtt(ADD_OFFSET);
      vector<double> vals;
      try {
        offsetAtt.getValues(vals);
        offset = vals[0];
      } catch (NcxxException& e) {
      }
    } catch (NcxxException& e) {
    }

    double scale = 1.0;
    try {
      NcxxVarAtt scaleAtt = var.getAtt(SCALE_FACTOR);
      vector<double> vals;
      try {
        scaleAtt.getValues(vals);
        scale = vals[0];
      } catch (NcxxException& e) {
      }
    } catch (NcxxException& e) {
    }

    // if metadata only, don't read in fields
    
    if (metaOnly) {
      bool fieldAlreadyAdded = false;
      for (size_t ii = 0; ii < _readVol->getNFields(); ii++) {
        if (_readVol->getField(ii)->getName() == name) {
          fieldAlreadyAdded = true;
          break;
        }
      }
      if (!fieldAlreadyAdded) {
        RadxField *field = new RadxField(name, units);
        field->setLongName(longName);
        field->setStandardName(standardName);
        field->setSamplingRatio(samplingRatio);
        if (fieldFolds &&
            foldLimitLower != Radx::missingMetaFloat &&
            foldLimitUpper != Radx::missingMetaFloat) {
          field->setFieldFolds(foldLimitLower, foldLimitUpper);
        }
        if (isDiscrete) {
          field->setIsDiscrete(true);
        }
        if (legendXml.size() > 0) {
          field->setLegendXml(legendXml);
        }
        if (thresholdingXml.size() > 0) {
          field->setThresholdingXml(thresholdingXml);
        }
        _readVol->addField(field);
      }
      continue;
    }

    int iret = 0;
    
    switch (var.getType().getId()) {
      case NC_DOUBLE: {
        if (_addFl64FieldToRays(var, name, units, standardName, longName,
                                isDiscrete, fieldFolds,
                                foldLimitLower, foldLimitUpper)) {
          iret = -1;
        }
        break;
      }
      case NC_FLOAT: {
        if (_addFl32FieldToRays(var, name, units, standardName, longName,
                                isDiscrete, fieldFolds,
                                foldLimitLower, foldLimitUpper)) {
          iret = -1;
        }
        break;
      }
      case NC_INT: {
        if (_addSi32FieldToRays(var, name, units, standardName, longName,
                                scale, offset,
                                isDiscrete, fieldFolds,
                                foldLimitLower, foldLimitUpper)) {
          iret = -1;
        }
        break;
      }
      case NC_SHORT: {
        if (_addSi16FieldToRays(var, name, units, standardName, longName,
                                scale, offset,
                                isDiscrete, fieldFolds,
                                foldLimitLower, foldLimitUpper)) {
          iret = -1;
        }
        break;
      }
      case NC_BYTE: {
        if (_addSi08FieldToRays(var, name, units, standardName, longName,
                                scale, offset,
                                isDiscrete, fieldFolds,
                                foldLimitLower, foldLimitUpper)) {
          iret = -1;
        }
        break;
      }
      default: {
        iret = -1;
        // will not reach here because of earlier check on type
      }

    } // switch
    
    if (iret) {
      _addErrStr("ERROR - NcxxRadxFile::_readFieldVariables");
      _addErrStr("  cannot read field name: ", name);
      _addErrStr(_file.getErrStr());
      return -1;
    }

  } // ivar

  return 0;

}

///////////////////////////////////
// read a ray variable - double
// side effects: set var, vals

int NcxxRadxFile::_readRayVar(NcxxVar &var, const string &name,
                              vector<double> &vals, bool required)

{

  vals.clear();

  // get var

  if (_getRayVar(var, name, required)) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(Radx::missingMetaDouble);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - NcxxRadxFile::_readRayVar");
      return -1;
    }
  }

  // load up data

  int iret = 0;
  double *data = new double[_nTimesInFile];
  try {
    var.getVal(data);
    for (size_t ii = 0; ii < _nTimesInFile; ii++) {
      vals.push_back(data[ii]);
    }
  } catch (NcxxException& e) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(Radx::missingMetaDouble);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - NcxxRadxFile::_readRayVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getErrStr());
      iret = -1;
    }
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a ray variable - double
// side effects: set vals only

int NcxxRadxFile::_readRayVar(const string &name,
                              vector<double> &vals, bool required)
{
  NcxxVar var;
  return _readRayVar(var, name, vals, required);
}

///////////////////////////////////
// read a ray variable - integer
// side effects: set var, vals

int NcxxRadxFile::_readRayVar(NcxxVar &var, const string &name,
                              vector<int> &vals, bool required)

{

  vals.clear();

  // get var
  
  if (_getRayVar(var, name, required)) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(Radx::missingMetaInt);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - NcxxRadxFile::_readRayVar");
      return -1;
    }
  }

  // load up data

  int iret = 0;
  int *data = new int[_nTimesInFile];
  try {
    var.getVal(data);
    for (size_t ii = 0; ii < _nTimesInFile; ii++) {
      vals.push_back(data[ii]);
    }
  } catch (NcxxException& e) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(Radx::missingMetaInt);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - NcxxRadxFile::_readRayVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getErrStr());
      iret = -1;
    }
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a ray variable - integer
// side effects: set vals only

int NcxxRadxFile::_readRayVar(const string &name,
                              vector<int> &vals, bool required)
{
  NcxxVar var;
  return _readRayVar(var, name, vals, required);
}

///////////////////////////////////////////
// read a ray variable - boolean
// side effects: set var, vals

int NcxxRadxFile::_readRayVar(NcxxVar &var, const string &name,
                              vector<bool> &vals, bool required)
  
{
  
  vals.clear();
  
  // get var
  
  if (_getRayVar(var, name, required)) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(false);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - NcxxRadxFile::_readRayVar");
      return -1;
    }
  }

  // load up data

  int iret = 0;
  int *data = new int[_nTimesInFile];
  try {
    var.getVal(data);
    for (size_t ii = 0; ii < _nTimesInFile; ii++) {
      if (data[ii] == 0) {
        vals.push_back(false);
      } else {
        vals.push_back(true);
      }
    }
  } catch (NcxxException& e) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(false);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - NcxxRadxFile::_readRayVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getErrStr());
      iret = -1;
    }
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a ray variable - boolean
// side effects: set vals only

int NcxxRadxFile::_readRayVar(const string &name,
                              vector<bool> &vals, bool required)
{
  NcxxVar var;
  return _readRayVar(var, name, vals, required);
}

///////////////////////////////////
// get a ray variable by name
// returns 0 on success, -1 on failure

int NcxxRadxFile::_getRayVar(NcxxVar &var, const string &name, bool required)

{

  // get var
  
  var = _file.getVar(name);
  if (var.isNull()) {
    if (required) {
      _addErrStr("ERROR - NcxxRadxFile::_getRayVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr(_file.getErrStr());
    }
    return -1;
  }

  // check time dimension
  
  if (var.getDimCount() < 1) {
    if (required) {
      _addErrStr("ERROR - NcxxRadxFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no dimensions");
    }
    return -1;
  }
  NcxxDim timeDim = var.getDim(0);
  if (timeDim != _timeDim) {
    if (required) {
      _addErrStr("ERROR - NcxxRadxFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has incorrect dimension, dim name: ", 
                 timeDim.getName());
      _addErrStr("  should be: ", TIME);
    }
    return -1;
  }

  return 0;

}

///////////////////////////////////
// read a sweep variable - double

int NcxxRadxFile::_readSweepVar(NcxxVar &var, const string &name,
                                vector<double> &vals, bool required)

{

  vals.clear();

  // get var

  size_t nSweeps = _sweepDim.getSize();
  if (_getSweepVar(var, name)) {
    if (!required) {
      for (size_t ii = 0; ii < nSweeps; ii++) {
        vals.push_back(Radx::missingMetaDouble);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - NcxxRadxFile::_readSweepVar");
      return -1;
    }
  }

  // load up data

  int iret = 0;
  double *data = new double[nSweeps];
  try {
    var.getVal(data);
    for (size_t ii = 0; ii < nSweeps; ii++) {
      vals.push_back(data[ii]);
    }
  } catch (NcxxException& e) {
    if (!required) {
      for (size_t ii = 0; ii < nSweeps; ii++) {
        vals.push_back(Radx::missingMetaDouble);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - NcxxRadxFile::_readSweepVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getErrStr());
      iret = -1;
    }
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a sweep variable - integer

int NcxxRadxFile::_readSweepVar(NcxxVar &var, const string &name,
                                vector<int> &vals, bool required)

{

  vals.clear();

  // get var

  size_t nSweeps = _sweepDim.getSize();
  if (_getSweepVar(var, name)) {
    if (!required) {
      for (size_t ii = 0; ii < nSweeps; ii++) {
        vals.push_back(Radx::missingMetaInt);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - NcxxRadxFile::_readSweepVar");
      return -1;
    }
  }

  // load up data

  int iret = 0;
  int *data = new int[nSweeps];
  try {
    var.getVal(data);
    for (size_t ii = 0; ii < nSweeps; ii++) {
      vals.push_back(data[ii]);
    }
  } catch (NcxxException& e) {
    if (!required) {
      for (size_t ii = 0; ii < nSweeps; ii++) {
        vals.push_back(Radx::missingMetaInt);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - NcxxRadxFile::_readSweepVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getErrStr());
      iret = -1;
    }
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a sweep variable - string

int NcxxRadxFile::_readSweepVar(NcxxVar &var, const string &name,
                                vector<string> &vals, bool required)

{

  // get var
  
  size_t nSweeps = _sweepDim.getSize();
  var = _file.getVar(name);
  if (var.isNull()) {
    if (!required) {
      for (size_t ii = 0; ii < nSweeps; ii++) {
        vals.push_back("");
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - NcxxRadxFile::_readSweepVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr(_file.getErrStr());
      return -1;
    }
  }

  // check sweep dimension
  
  if (var.getDimCount() < 2) {
    _addErrStr("ERROR - NcxxRadxFile::_readSweepVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has fewer than 2 dimensions");
    return -1;
  }
  NcxxDim sweepDim = var.getDim(0);
  if (sweepDim != _sweepDim) {
    _addErrStr("ERROR - NcxxRadxFile::_readSweepVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has incorrect first dimension, dim name: ",
               sweepDim.getName());
    _addErrStr("  should be: ", SWEEP);
    return -1;
  }
  NcxxDim stringLenDim = var.getDim(1);
  if (stringLenDim.isNull()) {
    _addErrStr("ERROR - NcxxRadxFile::_readSweepVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has NULL second dimension");
    _addErrStr("  should be a string length dimension");
    return -1;
  }

  int ntype = var.getType().getId();
  if (ntype != NC_CHAR) {
    _addErrStr("ERROR - NcxxRadxFile::_readSweepVar");
    _addErrStr("  Incorrect variable type");
    _addErrStr("  Expecting char");
    _addErrStr("  Found: ", Ncxx::ncTypeToStr(ntype));
    return -1;
  }

  // load up data

  size_t stringLen = stringLenDim.getSize();
  size_t nChars = nSweeps * stringLen;
  char *cvalues = new char[nChars];
  try {
    var.getVal(cvalues);
    // replace white space with nulls
    for (size_t ii = 0; ii < nChars; ii++) {
      if (isspace(cvalues[ii])) {
        cvalues[ii] = '\0';
      }
    }
    // ensure null termination
    char *cv = cvalues;
    char *cval = new char[stringLen+1];
    for (size_t ii = 0; ii < nSweeps; ii++, cv += stringLen) {
      memcpy(cval, cv, stringLen);
      cval[stringLen] = '\0';
      vals.push_back(string(cval));
    }
    delete[] cval;
  } catch (NcxxException& e) {
    if (!required) {
      clearErrStr();
    } else {
      _addErrStr("ERROR - NcxxRadxFile::_readSweepVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getErrStr());
      return -1;
    }
  }
  delete[] cvalues;

  return 0;

}

///////////////////////////////////
// get a sweep variable
// returns 0 on success, -1 on failure

int NcxxRadxFile::_getSweepVar(NcxxVar &var, const string &name)

{
  
  // get var
  
  var = _file.getVar(name);
  if (var.isNull()) {
    _addErrStr("ERROR - NcxxRadxFile::_getSweepVar");
    _addErrStr("  Cannot read variable, name: ", name);
    _addErrStr(_file.getErrStr());
    return -1;
  }

  // check sweep dimension
  
  if (var.getDimCount() < 1) {
    _addErrStr("ERROR - NcxxRadxFile::_getSweepVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has no dimensions");
    return -1;
  }
  NcxxDim sweepDim = var.getDim(0);
  if (sweepDim != _sweepDim) {
    _addErrStr("ERROR - NcxxRadxFile::_getSweepVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has incorrect dimension, dim name: ",
               sweepDim.getName());
    _addErrStr("  should be: ", SWEEP);
    return -1;
  }

  return 0;

}

///////////////////////////////////
// get calibration time
// returns -1 on failure

int NcxxRadxFile::_readCalTime(const string &name, NcxxVar &var,
                               int index, time_t &val)

{

  var = _file.getVar(name);

  if (var.isNull()) {
    _addErrStr("ERROR - NcxxRadxFile::_readCalTime");
    _addErrStr("  cal variable name: ", name);
    _addErrStr("  Cannot read calibration time");
    _addErrStr(_file.getErrStr());
    return -1;
  }

  // check cal dimension

  if (var.getDimCount() < 2) {
    _addErrStr("ERROR - NcxxRadxFile::_readCalTime");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has fewer than 2 dimensions");
    return -1;
  }

  NcxxDim rCalDim = var.getDim(0);
  if (rCalDim != _calDim) {
    _addErrStr("ERROR - NcxxRadxFile::_readCalTime");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has incorrect first dimension, dim name: ", 
               rCalDim.getName());
    _addErrStr("  should be: ", R_CALIB);
    return -1;
  }

  NcxxDim stringLenDim = var.getDim(1);
  if (stringLenDim.isNull()) {
    _addErrStr("ERROR - NcxxRadxFile::_readCalTime");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has NULL second dimension");
    _addErrStr("  should be a string length dimension");
    return -1;
  }
  
  int ntype = var.getType().getId();
  if (ntype != NC_CHAR) {
    _addErrStr("ERROR - NcxxRadxFile::_readCalTime");
    _addErrStr("  Incorrect variable type");
    _addErrStr("  Expecting char");
    _addErrStr("  Found: ", Ncxx::ncTypeToStr(ntype));
    return -1;
  }

  // load up data
  
  size_t nCals = _calDim.getSize();
  if (index > (int) nCals - 1) {
    _addErrStr("ERROR - NcxxRadxFile::_readCalTime");
    _addErrStr("  requested index too high");
    _addErrStr("  cal variable name: ", name);
    _addErrInt("  requested index: ", index);
    _addErrInt("  n cals available: ", nCals);
    return -1;
  }

  size_t stringLen = stringLenDim.getSize();
  size_t nChars = nCals * stringLen;
  char *cvalues = new char[nChars];
  vector<string> times;
  try {
    var.getVal(cvalues);
    char *cv = cvalues;
    char *cval = new char[stringLen+1];
    for (size_t ii = 0; ii < nCals; ii++, cv += stringLen) {
      // ensure null termination
      memcpy(cval, cv, stringLen);
      cval[stringLen] = '\0';
      times.push_back(string(cval));
      cv[stringLen-1] = '\0';
    }
    delete[] cval;
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxRadxFile::_readCalTime");
    _addErrStr("  Cannot read variable: ", name);
    _addErrStr(_file.getErrStr());
    delete[] cvalues;
    return -1;
  }
  delete[] cvalues;
  
  const char *timeStr = times[index].c_str();
  int year, month, day, hour, min, sec;
  if (sscanf(timeStr, "%4d-%2d-%2dT%2d:%2d:%2dZ",
             &year, &month, &day, &hour, &min, &sec) != 6) {
    _addErrStr("ERROR - NcxxRadxFile::_readCalTime");
    _addErrStr("  Cannot parse cal time string: ", timeStr);
    return -1;
  }
  RadxTime ctime(year, month, day, hour, min, sec);
  val = ctime.utime();

  return 0;

}

///////////////////////////////////
// get calibration variable
// returns -1 on failure

int NcxxRadxFile::_readCalVar(const string &name, NcxxVar &var,
                              int index, double &val, bool required)
  
{

  val = Radx::missingMetaDouble;
  var = _file.getVar(name);

  if (var.isNull()) {
    if (!required) {
      return 0;
    } else {
      _addErrStr("ERROR - NcxxRadxFile::_readCalVar");
      _addErrStr("  cal variable name: ", name);
      _addErrStr("  Cannot read calibration variable");
      _addErrStr(_file.getErrStr());
      return -1;
    }
  }

  if (var.numVals() < index-1) {
    _addErrStr("ERROR - NcxxRadxFile::_readCalVar");
    _addErrStr("  requested index too high");
    _addErrStr("  cal variable name: ", name);
    _addErrInt("  requested index: ", index);
    _addErrInt("  n cals available: ", var.numVals());
    return -1;
  }

  // read value

  vector<size_t> indices;
  indices.push_back(index);
  double data;
  int iret = 0;
  try {
    var.getVal(indices, &data);
    val = data;
  } catch (NcxxException& e) {
    _addErrStr("ERROR - NcxxRadxFile::_readCalVarVar");
    _addErrStr("  Cannot read variable: ", name);
    _addErrStr(_file.getErrStr());
    iret = -1;
  }

  return iret;

}

//////////////////////////////////////////////////////////////
// Add fl64 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int NcxxRadxFile::_addFl64FieldToRays(NcxxVar &var,
                                      const string &name,
                                      const string &units,
                                      const string &standardName,
                                      const string &longName,
                                      bool isDiscrete,
                                      bool fieldFolds,
                                      float foldLimitLower,
                                      float foldLimitUpper)
  
{

  // get data from array

  size_t nVals = _nTimesInFile * _nRangeInFile;
  if (_nGatesVary) {
    nVals = _nPoints;
  }
  Radx::fl64 *data = new Radx::fl64[nVals];

  try {
    var.getVal(data);
  } catch (NcxxException& e) {
    delete[] data;
    return -1;
  }
  
  // set missing value
  
  Radx::fl64 missingVal = Radx::missingFl64;
  try {
    NcxxVarAtt missingValueAtt = var.getAtt(MISSING_VALUE);
    vector<double> vals;
    try {
      missingValueAtt.getValues(vals);
      missingVal = vals[0];
    } catch (NcxxException& e) {
    }
  } catch (NcxxException& e) {
    try {
      NcxxVarAtt missingValueAtt = var.getAtt(FILL_VALUE);
      vector<double> vals;
      try {
        missingValueAtt.getValues(vals);
        missingVal = vals[0];
      } catch (NcxxException& e) {
      }
    } catch (NcxxException& e) {
    }
  }

  // reset nans to missing
  
  for (int ii = 0; ii < _nPoints; ii++) {
    if (!isfinite(data[ii])) {
      data[ii] = missingVal;
    }
  }

  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    size_t rayIndex = _raysToRead[ii].indexInFile;

    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - NcxxRadxFile::_addFl64FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimesInFile << endl;
      cerr << "  skipping ...." << endl;
      continue;
    }
    
    int nGates = _nRangeInFile;
    int startIndex = rayIndex * _nRangeInFile;
    if (_nGatesVary) {
      nGates = _rayNGates[rayIndex];
      startIndex = _rayStartIndex[rayIndex];
    }
    
    RadxField *field =
      _raysFromFile[ii]->addField(name, units, nGates,
                                  missingVal,
                                  data + startIndex,
                                  true);

    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);
    
    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
  delete[] data;
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add fl32 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int NcxxRadxFile::_addFl32FieldToRays(NcxxVar &var,
                                      const string &name,
                                      const string &units,
                                      const string &standardName,
                                      const string &longName,
                                      bool isDiscrete,
                                      bool fieldFolds,
                                      float foldLimitLower,
                                      float foldLimitUpper)
  
{

  // get data from array

  size_t nVals = _nTimesInFile * _nRangeInFile;
  if (_nGatesVary) {
    nVals = _nPoints;
  }
  Radx::fl32 *data = new Radx::fl32[nVals];

  try {
    var.getVal(data);
  } catch (NcxxException& e) {
    delete[] data;
    return -1;
  }
  
  // set missing value
  
  Radx::fl32 missingVal = Radx::missingFl32;
  try {
    NcxxVarAtt missingValueAtt = var.getAtt(MISSING_VALUE);
    vector<float> vals;
    try {
      missingValueAtt.getValues(vals);
      missingVal = vals[0];
    } catch (NcxxException& e) {
    }
  } catch (NcxxException& e) {
    try {
      NcxxVarAtt missingValueAtt = var.getAtt(FILL_VALUE);
      vector<float> vals;
      try {
        missingValueAtt.getValues(vals);
        missingVal = vals[0];
      } catch (NcxxException& e) {
      }
    } catch (NcxxException& e) {
    }
  }

  // reset nans to missing
  
  for (int ii = 0; ii < _nPoints; ii++) {
    if (!isfinite(data[ii])) {
      data[ii] = missingVal;
    }
  }

  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    size_t rayIndex = _raysToRead[ii].indexInFile;

    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - NcxxRadxFile::_addFl32FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimesInFile << endl;
      cerr << "  skipping ...." << endl;
      continue;
    }
    
    int nGates = _nRangeInFile;
    int startIndex = rayIndex * _nRangeInFile;
    if (_nGatesVary) {
      nGates = _rayNGates[rayIndex];
      startIndex = _rayStartIndex[rayIndex];
    }

    RadxField *field =
      _raysFromFile[ii]->addField(name, units, nGates,
                                  missingVal,
                                  data + startIndex,
                                  true);

    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);

    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
  delete[] data;
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add si32 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int NcxxRadxFile::_addSi32FieldToRays(NcxxVar &var,
                                      const string &name,
                                      const string &units,
                                      const string &standardName,
                                      const string &longName,
                                      double scale, double offset,
                                      bool isDiscrete,
                                      bool fieldFolds,
                                      float foldLimitLower,
                                      float foldLimitUpper)
  
{

  // get data from array

  size_t nVals = _nTimesInFile * _nRangeInFile;
  if (_nGatesVary) {
    nVals = _nPoints;
  }
  Radx::si32 *data = new Radx::si32[nVals];
  
  try {
    var.getVal(data);
  } catch (NcxxException& e) {
    delete[] data;
    return -1;
  }
  
  // set missing value
  
  Radx::si32 missingVal = Radx::missingSi32;
  try {
    NcxxVarAtt missingValueAtt = var.getAtt(MISSING_VALUE);
    vector<int> vals;
    try {
      missingValueAtt.getValues(vals);
      missingVal = vals[0];
    } catch (NcxxException& e) {
    }
  } catch (NcxxException& e) {
    try {
      NcxxVarAtt missingValueAtt = var.getAtt(FILL_VALUE);
      vector<int> vals;
      try {
        missingValueAtt.getValues(vals);
        missingVal = vals[0];
      } catch (NcxxException& e) {
      }
    } catch (NcxxException& e) {
    }
  }

  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    size_t rayIndex = _raysToRead[ii].indexInFile;

    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - NcxxRadxFile::_addSi32FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimesInFile << endl;
      cerr << "  skipping ...." << endl;
      continue;
    }
    
    int nGates = _nRangeInFile;
    int startIndex = rayIndex * _nRangeInFile;
    if (_nGatesVary) {
      nGates = _rayNGates[rayIndex];
      startIndex = _rayStartIndex[rayIndex];
    }
    
    RadxField *field =
      _raysFromFile[ii]->addField(name, units, nGates,
                                  missingVal,
                                  data + startIndex,
                                  scale, offset,
                                  true);

    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);

    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
  delete[] data;
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add si16 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int NcxxRadxFile::_addSi16FieldToRays(NcxxVar &var,
                                      const string &name,
                                      const string &units,
                                      const string &standardName,
                                      const string &longName,
                                      double scale, double offset,
                                      bool isDiscrete,
                                      bool fieldFolds,
                                      float foldLimitLower,
                                      float foldLimitUpper)
  
{

  // get data from array

  size_t nVals = _nTimesInFile * _nRangeInFile;
  if (_nGatesVary) {
    nVals = _nPoints;
  }
  Radx::si16 *data = new Radx::si16[nVals];
  
  try {
    var.getVal(data);
  } catch (NcxxException& e) {
    delete[] data;
    return -1;
  }
  
  // set missing value
  
  Radx::si16 missingVal = Radx::missingSi16;
  try {
    NcxxVarAtt missingValueAtt = var.getAtt(MISSING_VALUE);
    vector<short> vals;
    try {
      missingValueAtt.getValues(vals);
      missingVal = vals[0];
    } catch (NcxxException& e) {
    }
  } catch (NcxxException& e) {
    try {
      NcxxVarAtt missingValueAtt = var.getAtt(FILL_VALUE);
      vector<short> vals;
      try {
        missingValueAtt.getValues(vals);
        missingVal = vals[0];
      } catch (NcxxException& e) {
      }
    } catch (NcxxException& e) {
    }
  }

  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    size_t rayIndex = _raysToRead[ii].indexInFile;

    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - NcxxRadxFile::_addSi16FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimesInFile << endl;
      cerr << "  skipping ...." << endl;
      continue;
    }
    
    int nGates = _nRangeInFile;
    int startIndex = rayIndex * _nRangeInFile;
    if (_nGatesVary) {
      nGates = _rayNGates[rayIndex];
      startIndex = _rayStartIndex[rayIndex];
    }
    
    RadxField *field =
      _raysFromFile[ii]->addField(name, units, nGates,
                                  missingVal,
                                  data + startIndex,
                                  scale, offset,
                                  true);

    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);

    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
  delete[] data;
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add si08 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int NcxxRadxFile::_addSi08FieldToRays(NcxxVar &var,
                                      const string &name,
                                      const string &units,
                                      const string &standardName,
                                      const string &longName,
                                      double scale, double offset,
                                      bool isDiscrete,
                                      bool fieldFolds,
                                      float foldLimitLower,
                                      float foldLimitUpper)
  
{

  // get data from array

  size_t nVals = _nTimesInFile * _nRangeInFile;
  if (_nGatesVary) {
    nVals = _nPoints;
  }
  Radx::si08 *data = new Radx::si08[nVals];
  
  try {
    var.getVal(data);
  } catch (NcxxException& e) {
    delete[] data;
    return -1;
  }
  
  // set missing value
  
  Radx::si08 missingVal = Radx::missingSi08;
  try {
    NcxxVarAtt missingValueAtt = var.getAtt(MISSING_VALUE);
    vector<char> vals;
    try {
      missingValueAtt.getValues(vals);
      missingVal = vals[0];
    } catch (NcxxException& e) {
    }
  } catch (NcxxException& e) {
    try {
      NcxxVarAtt missingValueAtt = var.getAtt(FILL_VALUE);
      vector<char> vals;
      try {
        missingValueAtt.getValues(vals);
        missingVal = vals[0];
      } catch (NcxxException& e) {
      }
    } catch (NcxxException& e) {
    }
  }

  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    size_t rayIndex = _raysToRead[ii].indexInFile;

    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - NcxxRadxFile::_addSi08FieldToRays" << endl;
      cerr << "  Trying to access ray beyond data" << endl;
      cerr << "  Trying to read ray index: " << rayIndex << endl;
      cerr << "  nTimesInFile: " << _nTimesInFile << endl;
      cerr << "  skipping ...." << endl;
      continue;
    }
    
    int nGates = _nRangeInFile;
    int startIndex = rayIndex * _nRangeInFile;
    if (_nGatesVary) {
      nGates = _rayNGates[rayIndex];
      startIndex = _rayStartIndex[rayIndex];
    }
    
    RadxField *field =
      _raysFromFile[ii]->addField(name, units, nGates,
                                  missingVal,
                                  data + startIndex,
                                  scale, offset,
                                  true);

    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);

    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
  delete[] data;
  return 0;
  
}

/////////////////////////////////////////////////////////
// load up the read volume with the data from this object

void NcxxRadxFile::_loadReadVolume()
  
{

  _readVol->setOrigFormat("CFRADIAL");
  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setInstrumentType(_instrumentType);
  _readVol->setPlatformType(_platformType);
  _readVol->setPrimaryAxis(_primaryAxis);
  for (int ii = 0; ii < (int) _raysVol.size(); ii++) {
    _raysVol[ii]->setVolumeNumber(_volumeNumber);
  }

  for (size_t ii = 0; ii < _frequency.size(); ii++) {
    _readVol->addFrequencyHz(_frequency[ii]);
  }

  _readVol->setRadarAntennaGainDbH(_radarAntennaGainDbH);
  _readVol->setRadarAntennaGainDbV(_radarAntennaGainDbV);
  _readVol->setRadarBeamWidthDegH(_radarBeamWidthDegH);
  _readVol->setRadarBeamWidthDegV(_radarBeamWidthDegV);
  if (_radarRxBandwidthHz > 0) {
    _readVol->setRadarReceiverBandwidthMhz(_radarRxBandwidthHz / 1.0e6);
  } else {
    _readVol->setRadarReceiverBandwidthMhz(_radarRxBandwidthHz); // missing
  }

  _readVol->setVersion(_version);
  _readVol->setTitle(_title);
  _readVol->setSource(_source);
  _readVol->setHistory(_history);
  _readVol->setInstitution(_institution);
  _readVol->setReferences(_references);
  _readVol->setComment(_comment);
  _readVol->setOrigFormat(_origFormat);
  _readVol->setDriver(_driver);
  _readVol->setCreated(_created);
  _readVol->setStatusXml(_statusXml);
  _readVol->setSiteName(_siteName);
  _readVol->setScanName(_scanName);
  _readVol->setScanId(_scanId);
  _readVol->setInstrumentName(_instrumentName);

  if (_latitude.size() > 0) {
    for (size_t ii = 0; ii < _latitude.size(); ii++) {
      if (_latitude[ii] > -9990) {
        _readVol->setLatitudeDeg(_latitude[ii]);
        break;
      }
    }
  }
  if (_longitude.size() > 0) {
    for (size_t ii = 0; ii < _longitude.size(); ii++) {
      if (_longitude[ii] > -9990) {
        _readVol->setLongitudeDeg(_longitude[ii]);
        break;
      }
    }
  }
  if (_altitude.size() > 0) {
    for (size_t ii = 0; ii < _altitude.size(); ii++) {
      if (_altitude[ii] > -9990) {
        _readVol->setAltitudeKm(_altitude[ii] / 1000.0);
        break;
      }
    }
  }
  if (_altitudeAgl.size() > 0) {
    for (size_t ii = 0; ii < _altitudeAgl.size(); ii++) {
      if (_altitudeAgl[ii] > -9990) {
        _readVol->setSensorHtAglM(_altitudeAgl[ii]);
        break;
      }
    }
  }

  _readVol->copyRangeGeom(_geom);

  if (_correctionsActive) {
    _readVol->setCfactors(_cfactors);
  }

  for (size_t ii = 0; ii < _raysVol.size(); ii++) {
    _readVol->addRay(_raysVol[ii]);
  }

  for (size_t ii = 0; ii < _rCals.size(); ii++) {
    _readVol->addCalib(_rCals[ii]);
  }

  if (_readSetMaxRange) {
    _readVol->setMaxRangeKm(_readMaxRangeKm);
  }
  
  // memory responsibility has passed to the volume object, so clear
  // the vectors without deleting the objects to which they point

  _raysVol.clear();
  _rCals.clear();
  _fields.clear();

  // apply goeref info if applicable

  if (_readApplyGeorefs) {
    _readVol->applyGeorefs();
  }

  // set volume geometry to the predominant

  double predomStartRange, predomGateSpacing;
  _readVol->getPredomRayGeom(predomStartRange, predomGateSpacing);
  _readVol->setRangeGeom(predomStartRange, predomGateSpacing);
  
  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();
  
  // load the volume information from the rays

  _readVol->loadVolumeInfoFromRays();
  
  // check for indexed rays, set info on rays

  _readVol->checkForIndexedRays();

}

/////////////////////////////////////////////////////////////
// Compute the fixed angles by averaging the elevation angles
// on the sweeps

void NcxxRadxFile::_computeFixedAngles()
  
{

  for (size_t isweep = 0; isweep < _readVol->getNSweeps(); isweep++) {

    RadxSweep &sweep = *_readVol->getSweeps()[isweep];

    double sumElev = 0.0;
    double count = 0.0;

    for (size_t iray = sweep.getStartRayIndex();
         iray <= sweep.getEndRayIndex(); iray++) {
      const RadxRay &ray = *_readVol->getRays()[iray];
      sumElev += ray.getElevationDeg();
      count++;
    }

    double meanElev = sumElev / count;
    double fixedAngle = ((int) (meanElev * 100.0 + 0.5)) / 100.0;

    sweep.setFixedAngleDeg(fixedAngle);
      
    for (size_t iray = sweep.getStartRayIndex();
         iray <= sweep.getEndRayIndex(); iray++) {
      RadxRay &ray = *_readVol->getRays()[iray];
      ray.setFixedAngleDeg(fixedAngle);
    }

  } // isweep

  _readVol->loadFixedAnglesFromSweepsToRays();

}

