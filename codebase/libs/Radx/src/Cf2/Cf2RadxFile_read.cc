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
// Cf2RadxFile_read.cc
//
// Read methods for Cf2RadxFile object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2017
//
///////////////////////////////////////////////////////////////

#include <Radx/Cf2RadxFile.hh>
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

int Cf2RadxFile::readFromPath(const string &path,
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
    _addErrStr("ERROR - Cf2RadxFile::readFromPath");
    _addErrStr("  Loading sweep info");
    return -1;
  }

  // read from all paths

  for (size_t ii = 0; ii < paths.size(); ii++) {
    if (_readPath(paths[ii], ii)) {
      if (_debug) {
        cerr << "###########################################" << endl;
        cerr << "|||||||||||||||||||||||||||||||||||||||||||" << endl;
        cerr << _errStr << endl;
        cerr << "|||||||||||||||||||||||||||||||||||||||||||" << endl;
        cerr << "###########################################" << endl;
      }
      return -1;
    }
  }

  // load the data into the read volume

  _loadReadVolume();

  // remove transitions if applicable

  if (_readIgnoreTransitions) {
    _readVol->removeTransitionRays(_readTransitionNraysMargin);
  }

  // set format as read

  _fileFormat = FILE_FORMAT_CFRADIAL2;

  return 0;

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
// Returns 0 on success, -1 on failure

int Cf2RadxFile::_readPath(const string &path, size_t pathNum)
  
{

  if (_verbose) {
    cerr << "Reading file num, path: "
         << pathNum << ", " << path << endl;
  }
  _pathInUse = path;
  
  string errStr("ERROR - Cf2RadxFile::readFromPath::_readPath");

  // clear tmp rays

  _raysFromFile.clear();

  // open file

  try {
    _file.open(path, NcxxFile::read);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Cf2RadxFile::_readPath");
    _addErrStr("  Cannot open file for reading: ", path);
    _addErrStr("  exception: ", e.what());
    return -1;
  }

  // read dimensions
  
  try {
    _readRootDimensions();
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Cf2RadxFile::_readPath");
    _addErrStr("  Cannot read dimensions, path: ", path);
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  
  // read in sweep variables
  
  try {
    _readSweepsMetaAsInFile();
  } catch (NcxxException e) {
    _addErrStr("ERROR - Cf2RadxFile::_readPath()");
    _addErrStr("  path: ", path);
    _addErrStr("  exception: ", e.what());
    if (_debug) {
      cerr << "====>> ERROR - _readPath <<====" << endl;
      cerr << _errStr << endl;
      cerr << "===============================" << endl;
    }
    return -1;
  }

  // read time variable now if that is all that is needed
  
  if (_readTimesOnly) {
    try {
      _readTimes();
    } catch (NcxxException e) {
      return -1;
    }
    return 0;
  }
  
  // for first path in aggregated list, read in non-varying values

  if (pathNum == 0) {

    // read global attributes
    
    try {
      _readGlobalAttributes();
    } catch (NcxxException e) {
      _addErrStr("ERROR - Cf2RadxFile::_readPath()");
      _addErrStr("  reading global attributes, path: ", path);
      _addErrStr("  exception: ", e.what());
      return -1;
    }

    // read in scalar variables
    
    try {
      _readRootScalarVariables();
    } catch (NcxxException e) {
      _addErrStr("ERROR - Cf2RadxFile::_readPath()");
      _addErrStr("  reading scalar variables, path: ", path);
      _addErrStr("  exception: ", e.what());
      return -1;
    }
    
    // read in instrument paramaters
    
    if (_instrumentType == Radx::INSTRUMENT_TYPE_RADAR) {
      _readRadarParameters();
    } else {
      _readLidarParameters();
    }
    
    // read in correction variables, if available
    
    _readGeorefCorrections();

    // read location variables - lat/lon/alt
    
    _readLocation();

  }

  // read in calibration variables
  
  if (_instrumentType == Radx::INSTRUMENT_TYPE_RADAR) {
    _readRadarCalibration();
  } else {
    _readLidarCalibration();
  }

  // read time variable
  
  try {
    _readTimes();
  } catch (NcxxException e) {
    return -1;
  }

  // read the sweeps
  
  try {
    _readSweeps();
  } catch (NcxxException e) {
    _addErrStr("ERROR - Cf2RadxFile::_readPath()");
    _addErrStr("  reading sweeps and their fields, path: ", path);
    _addErrStr("  exception: ", e.what());
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

  _raysFromFile.clear();
  _clearGeorefVariables();
  _clearRayVariables();
  _rayTimes.clear();

  return 0;

}

//////////////////////////////////////////////////////////
// get list of paths for the volume for the specified path
// returns the volume number

int Cf2RadxFile::_getVolumePaths(const string &path,
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

void Cf2RadxFile::_addToPathList(const string &dir,
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

int Cf2RadxFile::_loadSweepInfo(const vector<string> &paths)
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
      _addErrStr("ERROR - Cf2RadxFile::_loadSweepInfo");
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

int Cf2RadxFile::_appendSweepInfo(const string &path)
{

  // open file

  try {
    _file.open(path, NcxxFile::read);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Cf2RadxFile::_appendSweepInfo");
    _addErrStr("  Cannot open file for reading: ", path);
    _addErrStr("  exception: ", e.what());
    return -1;
  }

  // read dimensions
  
  try {
    _readRootDimensions();
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Cf2RadxFile::_appendSweepInfo");
    _addErrStr("  Cannot read dimensions");
    return -1;
  }

  try {
    _readSweepsMetaAsInFile();
  } catch (NcxxException e) {
    _addErrStr("ERROR - Cf2RadxFile::_appendSweepInfo");
    _addErrStr("  path: ", path);
    _addErrStr(e.what());
    if (_debug) {
      cerr << "====>> ERROR - _readPath <<====" << endl;
      cerr << _errStr << endl;
      cerr << "===============================" << endl;
    }
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

///////////////////////////////////
// read in the dimensions
// throws exception on error

void Cf2RadxFile::_readRootDimensions()

{

  // sweep dimension

  _sweepDim.setNull();
  try {
    _sweepDim = _file.getDim(SWEEP);
  } catch (NcxxException e) {
    NcxxErrStr err;
    err.addErrStr("ERROR - Cf2RadxFile::readRootDimensions");
    err.addErrStr("  Cannot find sweep dimension");
    err.addErrStr("  exception: ", e.what());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  // frequency dimension is optional

  _frequencyDim = _file.getDim(FREQUENCY);

}

///////////////////////////////////
// read the global attributes
// throws exception on error

void Cf2RadxFile::_readGlobalAttributes()

{

  // check for conventions
  
  try {
    NcxxGroupAtt att = _file.getAtt(CONVENTIONS);
    _conventions = att.asString();
  } catch (NcxxException& e) {
    NcxxErrStr err;
    err.addErrStr("ERROR - Cf2RadxFile::_readGlobalAttributes");
    err.addErrStr("  Cannot find conventions attribute");
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  if (_conventions.find(BaseConvention) == string::npos) {
    if (_conventions.find("CF") == string::npos) {
      NcxxErrStr err;
      err.addErrStr("ERROR - Cf2RadxFile::_readGlobalAttributes");
      err.addErrStr("  Invalid Conventions attribute: ", _conventions);
      err.addErrStr("  Should be 'CF...'");
      throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
    }
  }

  // check for conventions
  
  try {
    NcxxGroupAtt att = _file.getAtt(SUB_CONVENTIONS);
    _subconventions = att.asString();
  } catch (NcxxException& e) {
    NcxxErrStr err;
    err.addErrStr("ERROR - Cf2RadxFile::_readGlobalAttributes");
    err.addErrStr("  Cannot find subconventions attribute");
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  if (_subconventions.find(BaseConvention) == string::npos) {
    NcxxErrStr err;
    err.addErrStr("ERROR - Cf2RadxFile::_readGlobalAttributes");
    err.addErrStr("  Invalid sub_conventions attribute: ", _subconventions);
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  // check for version
  
  try {
    NcxxGroupAtt att = _file.getAtt(VERSION);
    _version = att.asString();
  } catch (NcxxException& e) {
    NcxxErrStr err;
    err.addErrStr("ERROR - Cf2RadxFile::_readGlobalAttributes");
    err.addErrStr("  Cannot find version attribute");
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  if (_version.size() < 1 ||
      _version.find("CF-Radial-2") == string::npos) {
    NcxxErrStr err;
    err.addErrStr("ERROR - Cf2RadxFile::_readGlobalAttributes");
    err.addErrStr("  Invalid version: ", _version);
    err.addErrStr("  Should be 2.x");
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  // Loop through the global attributes, use the ones which make sense

  _origFormat = "CFRADIAL2"; // default
  _instrumentName = "unknown";

  multimap<string,NcxxGroupAtt> atts = _file.getAtts();
  for (multimap<string,NcxxGroupAtt>::iterator ii = atts.begin();
       ii != atts.end(); ii++) {

    NcxxGroupAtt att = ii->second;
    
    if (att.isNull()) {
      continue;
    } else if (att.getName().find(INSTRUMENT_NAME) != string::npos) {
      _instrumentName = att.asString();
    } else if (att.getName().find(TITLE) != string::npos) {
      _title = att.asString();
    } else if (att.getName().find(SOURCE) != string::npos) {
      _source = att.asString();
    } else if (att.getName().find(HISTORY) != string::npos) {
      _history = att.asString();
    } else if (att.getName().find(INSTITUTION) != string::npos) {
      _institution = att.asString();
    } else if (att.getName().find(REFERENCES) != string::npos) {
      _references = att.asString();
    } else if (att.getName().find(COMMENT) != string::npos) {
      _comment = att.asString();
    } else if (att.getName().find(AUTHOR) != string::npos) {
      _author = att.asString();
    } else if (att.getName().find(ORIGINAL_FORMAT) != string::npos) {
      _origFormat = att.asString();
    } else if (att.getName().find(DRIVER) != string::npos) {
      _driver = att.asString();
    } else if (att.getName().find(CREATED) != string::npos) {
      _created = att.asString();
    } else if (att.getName().find(SITE_NAME) != string::npos) {
      _siteName = att.asString();
    } else if (att.getName().find(SCAN_NAME) != string::npos) {
      _scanName = att.asString();
    } else if (att.getName().find(SCAN_ID) != string::npos) {
      vector<int> scanIds;
      try {
        att.getValues(scanIds);
        _scanId = scanIds[0];
      } catch (NcxxException& e) {
      }
    } else if (att.getName().find(RAY_TIMES_INCREASE) != string::npos) {
      string rayTimesIncrease = att.asString();
      if (rayTimesIncrease == "true") {
        _rayTimesIncrease = true;
      } else {
        _rayTimesIncrease = false;
      }
    }

  } // ii

}

/////////////////////////////////////////////
// read the times
// assumes sweeps have been previously read
// loads up _rayTimes vector
// throws exception on error

void Cf2RadxFile::_readTimes()
  
{

  _rayTimes.clear();
  for (size_t isweep = 0; isweep < _sweepsInFile.size(); isweep++) {
    try {
      _readSweepTimes(_sweepGroups[isweep], _rayTimes);
    } catch (NcxxException& e) {
      NcxxErrStr err;
      err.addErrStr("ERROR - Cf2RadxFile::_readTimes");
      throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
    }
  }

}

///////////////////////////////////
// read the times in a sweep group
// loads up times vector
// throws exception on error

void Cf2RadxFile::_readSweepTimes(NcxxGroup &group,
                                  vector<double> &times)

{

  NcxxDim timeDim = group.getDim(TIME);
  
  // read the time variable

  NcxxVar timeVar = group.getVar(TIME);
  if (timeVar.isNull()) {
    NcxxErrStr err;
    err.addErrStr("ERROR - Cf2RadxFile::_readSweepTimes");
    err.addErrStr("  Cannot find time variable, name: ", TIME);
    err.addErrStr("  group: ", group.getName());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  
  if (timeVar.getDimCount() < 1) {
    NcxxErrStr err;
    err.addErrStr("ERROR - Cf2RadxFile::_readSweepTimes");
    err.addErrStr("  time variable has no dimensions");
    err.addErrStr("  group: ", group.getName());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  NcxxDim varTimeDim = timeVar.getDim(0);
  if (varTimeDim != timeDim) {
    NcxxErrStr err;
    err.addErrStr("ERROR - Cf2RadxFile::_readSweepTimes");
    err.addErrStr("  Time has incorrect dimension, name: ", varTimeDim.getName());
    err.addErrStr("  group: ", group.getName());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  
  // get units attribute
  
  try {
    NcxxVarAtt unitsAtt = timeVar.getAtt(UNITS);
    string units = unitsAtt.asString();
    // parse the time units reference time
    RadxTime stime(units);
    _refTimeSecsFile = stime.utime();
  } catch (NcxxException& e) {
    NcxxErrStr err;
    err.addErrStr("ERROR - Cf2RadxFile::_readSweepTimes");
    err.addErrStr("  Time has no units");
    err.addErrStr("  group: ", group.getName());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  
  // set the time array
  
  size_t nTimes = varTimeDim.getSize();
  RadxArray<double> dtimes_;
  double *dtimes = dtimes_.alloc(nTimes);
  try {
    timeVar.getVal(dtimes);
  } catch (NcxxException& e) {
    NcxxErrStr err;
    err.addErrStr("ERROR - Cf2RadxFile::_readSweepTimes");
    err.addErrStr("  Cannot read times variable");
    err.addErrStr("  exception: ", e.what());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  for (size_t ii = 0; ii < nTimes; ii++) {
    times.push_back(dtimes[ii]);
  }

}

////////////////////////////////////////////////////
// read the sweep meta-data as it exists in the file
// loads up _sweepsInFile vector
// throws exception on error

void Cf2RadxFile::_readSweepsMetaAsInFile()

{

  _sweepGroups.clear();
  _sweepGroupNames.clear();
  _sweepsInFile.clear();
  
  size_t nSweepsInFile = _sweepDim.getSize();
  if (nSweepsInFile < 1) {
    NcxxErrStr err;
    err.addErrStr("ERROR - Cf2RadxFile::_readSweepsMetaAsInFile");
    err.addErrStr("  No sweeps found");
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  if (_debug) {
    cerr << "=====>> nSweeps: " << nSweepsInFile << endl;
  }
  
  // get the sweep group names - at root level
  {
    try {
      _read1DVar(_file, _sweepDim, SWEEP_GROUP_NAME, _sweepGroupNames);
    } catch (NcxxException e) {
      NcxxErrStr err;
      err.addErrStr("ERROR - Cf2RadxFile::_readSweepsMetaAsInFile");
      err.addErrStr("  Cannot read var, name", SWEEP_GROUP_NAME);
      err.addErrStr("  exception: ", e.what());
      throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
    }
  }

  // read each sweep group, accumulating sweeps as in file

  size_t startRayIndex = 0;
  for (size_t isweep = 0; isweep < _sweepGroupNames.size(); isweep++) {
    
    if (_debug) {
      cerr << "======>>> reading sweepGroupName: " 
           << _sweepGroupNames[isweep] << endl;
    }
    
    NcxxGroup group = _file.getGroup(_sweepGroupNames[isweep]);
    if (group.isNull()) {
      NcxxErrStr err;
      err.addErrStr("ERROR - Cf2RadxFile::_readSweepsMetaAsInFile");
      err.addErrStr("  Cannot read sweep group, name", _sweepGroupNames[isweep]);
      throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
    }

    RadxSweep *sweep = new RadxSweep;
    sweep->setStartRayIndex(startRayIndex);
    _readSweepMeta(group, sweep);
    _sweepGroups.push_back(group);
    _sweepsInFile.push_back(sweep);
    _sweeps.push_back(sweep);
    startRayIndex = sweep->getEndRayIndex() + 1;
    
  } // isweep

}

///////////////////////////////////
// read the sweep meta-data
// throws exception on error

void Cf2RadxFile::_readSweepMeta(NcxxGroup &group,
                                 RadxSweep *sweep)
  
{
  
  // volume number

  sweep->setVolumeNumber(_volumeNumber);

  // sweep number
  {
    NcxxVar var = group.getVar(SWEEP_NUMBER);
    if (!var.isNull()) {
      size_t len = var.getDimCount();
      if (len != 0) {
        NcxxErrStr err;
        err.addErrStr("ERROR - Cf2RadxFile::_readSweepMeta");
        err.addErrStr("  Reading var: ", SWEEP_NUMBER);
        err.addErrInt("  Bad dimCount, should be 0, found: ", len);
        throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
      }
      int val;
      var.getVal(&val);
      sweep->setSweepNumber(val);
    }
  }

  // fixed angle
  _fixedAngleFound = false;
  try {
    NcxxVar var = group.getVar(SWEEP_FIXED_ANGLE);
    if (!var.isNull()) {
      float val;
      var.getVal(&val);
      sweep->setFixedAngleDeg(val);
      _fixedAngleFound = true;
    }
  } catch (NcxxException e) {
  }
  
  // target scan rate
  {
    NcxxVar var = group.getVar(TARGET_SCAN_RATE);
    if (!var.isNull()) {
      size_t len = var.getDimCount();
      if (len != 0) {
        NcxxErrStr err;
        err.addErrStr("ERROR - Cf2RadxFile::_readSweepMeta");
        err.addErrStr("  Reading var: ", TARGET_SCAN_RATE);
        err.addErrInt("  Bad dimCount, should be 0, found: ", len);
        throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
      }
      float val;
      var.getVal(&val);
      sweep->setTargetScanRateDegPerSec(val);
    }
  }
  
  // sweep mode
  {
    NcxxVar var = group.getVar(SWEEP_MODE);
    if (!var.isNull()) {
      size_t len = var.getDimCount();
      if (len != 0) {
        NcxxErrStr err;
        err.addErrStr("ERROR - Cf2RadxFile::_readSweepMeta");
        err.addErrStr("  Reading var: ", SWEEP_MODE);
        err.addErrInt("  Bad dimCount, should be 0, found: ", len);
        throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
      }
      char *val;
      var.getVal(&val);
      sweep->setSweepMode(Radx::sweepModeFromStr(val));
    }
  }

  // polarization mode
  {
    NcxxVar var = group.getVar(POLARIZATION_MODE);
    if (!var.isNull()) {
      size_t len = var.getDimCount();
      if (len != 0) {
        NcxxErrStr err;
        err.addErrStr("ERROR - Cf2RadxFile::_readSweepMeta");
        err.addErrStr("  Reading var: ", POLARIZATION_MODE);
        err.addErrInt("  Bad dimCount, should be 0, found: ", len);
        throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
      }
      char *val;
      var.getVal(&val);
      sweep->setPolarizationMode(Radx::polarizationModeFromStr(val));
    }
  }

  // prt mode
  {
    NcxxVar var = group.getVar(PRT_MODE);
    if (!var.isNull()) {
      size_t len = var.getDimCount();
      if (len != 0) {
        NcxxErrStr err;
        err.addErrStr("ERROR - Cf2RadxFile::_readSweepMeta");
        err.addErrStr("  Reading var: ", PRT_MODE);
        err.addErrInt("  Bad dimCount, should be 0, found: ", len);
        throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
      }
      char *val;
      var.getVal(&val);
      sweep->setPrtMode(Radx::prtModeFromStr(val));
    }
  }

  // follow mode
  {
    NcxxVar var = group.getVar(FOLLOW_MODE);
    if (!var.isNull()) {
      size_t len = var.getDimCount();
      if (len != 0) {
        NcxxErrStr err;
        err.addErrStr("ERROR - Cf2RadxFile::_readSweepMeta");
        err.addErrStr("  Reading var: ", FOLLOW_MODE);
        err.addErrInt("  Bad dimCount, should be 0, found: ", len);
        throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
      }
      char *val;
      var.getVal(&val);
      sweep->setFollowMode(Radx::followModeFromStr(val));
    }
  }

  // indexed rays?
  {
    NcxxVar var = group.getVar(RAYS_ARE_INDEXED);
    if (!var.isNull()) {
      size_t len = var.getDimCount();
      if (len != 0) {
        NcxxErrStr err;
        err.addErrStr("ERROR - Cf2RadxFile::_readSweepMeta");
        err.addErrStr("  Reading var: ", RAYS_ARE_INDEXED);
        err.addErrInt("  Bad dimCount, should be 0, found: ", len);
        throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
      }
      char *val;
      var.getVal(&val);
      string sval(val);
      if (sval == "true") {
        sweep->setRaysAreIndexed(true);
      } else {
        sweep->setRaysAreIndexed(false);
      }
    }
  }

  if (sweep->getRaysAreIndexed()) {
    NcxxVar var = group.getVar(RAY_ANGLE_RESOLUTION);
    if (!var.isNull()) {
      size_t len = var.getDimCount();
      if (var.isNull() || len != 0) {
        NcxxErrStr err;
        err.addErrStr("ERROR - Cf2RadxFile::_readSweepMeta");
        err.addErrStr("  Reading var: ", RAY_ANGLE_RESOLUTION);
        err.addErrInt("  Bad dimCount, should be 0, found: ", len);
        throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
      }
      float val;
      var.getVal(&val);
      sweep->setAngleResDeg(val);
    }
  }

  // intermediate frequency
  {
    NcxxVar var = group.getVar(INTERMED_FREQ_HZ);
    if (!var.isNull()) {
      size_t len = var.getDimCount();
      if (var.isNull() || len != 0) {
        NcxxErrStr err;
        err.addErrStr("ERROR - Cf2RadxFile::_readSweepMeta");
        err.addErrStr("  Reading var: ", INTERMED_FREQ_HZ);
        err.addErrInt("  Bad dimCount, should be 0, found: ", len);
        throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
      }
      float val;
      var.getVal(&val);
      sweep->setIntermedFreqHz(val);
    }
  }

  // ray index counts

  NcxxDim timeDim = group.getDim(TIME);
  size_t nRays = timeDim.getSize();
  sweep->setEndRayIndex(sweep->getStartRayIndex() + nRays - 1);

  if (_debug) {
    sweep->print(cerr);
  }
    
}

//////////////////////////////////////////////////////////////
// read the root level scalar variables
// throws exception on read

  void Cf2RadxFile::_readRootScalarVariables()
  
{

  try {
    _file.readIntVar(VOLUME_NUMBER, _volumeNumber, Radx::missingMetaInt);
  } catch (NcxxException e) {
    _volumeNumber = 0;
  }

  try {
    string pstring;
    _file.readScalarStringVar(INSTRUMENT_TYPE, pstring);
    _instrumentType = Radx::instrumentTypeFromStr(pstring);
  } catch (NcxxException e) {
    _instrumentType = Radx::INSTRUMENT_TYPE_RADAR;
  }

  try {
    string pstring;
    _file.readScalarStringVar(PLATFORM_TYPE, pstring);
    _platformType = Radx::platformTypeFromStr(pstring);
  } catch (NcxxException e) {
    _platformType = Radx::PLATFORM_TYPE_FIXED;
  }
  
  try {
    string pstring;
    _file.readScalarStringVar(PRIMARY_AXIS, pstring);
    _primaryAxis = Radx::primaryAxisFromStr(pstring);
  } catch (NcxxException e) {
    _primaryAxis = Radx::PRIMARY_AXIS_Z;
  }
  
  try {
    string pstring;
    _file.readScalarStringVar(STATUS_XML, pstring);
    _statusXml = pstring;
  } catch (NcxxException e) {
    _statusXml.clear();
  }
  
}

//////////////////////////////////////////////////////////////
// read the radar parameters

void Cf2RadxFile::_readRadarParameters()
  
{

  if (_instrumentType != Radx::INSTRUMENT_TYPE_RADAR) {
    return;
  }

  NcxxGroup group = _file.getGroup(RADAR_PARAMETERS);
  if (!group.isNull()) {
    group.readDoubleVar(RADAR_ANTENNA_GAIN_H, _radarAntennaGainDbH, false);
    group.readDoubleVar(RADAR_ANTENNA_GAIN_V, _radarAntennaGainDbV, false);
    group.readDoubleVar(RADAR_BEAM_WIDTH_H, _radarBeamWidthDegH, false);
    group.readDoubleVar(RADAR_BEAM_WIDTH_V, _radarBeamWidthDegV, false);
    group.readDoubleVar(RADAR_RX_BANDWIDTH, _radarRxBandwidthHz, false);
    _readFrequency(group);
  }

}

//////////////////////////////////////////////////////////////
// read the lidar parameters

void Cf2RadxFile::_readLidarParameters()
  
{

  if (_instrumentType != Radx::INSTRUMENT_TYPE_LIDAR) {
    return;
  }

  NcxxGroup group = _file.getGroup(LIDAR_PARAMETERS);
  if (!group.isNull()) {

    group.readDoubleVar(LIDAR_CONSTANT, _lidarConstant, false);
    group.readDoubleVar(LIDAR_PULSE_ENERGY, _lidarPulseEnergyJ, false);
    group.readDoubleVar(LIDAR_PEAK_POWER, _lidarPeakPowerW, false);
    group.readDoubleVar(LIDAR_APERTURE_DIAMETER, _lidarApertureDiamCm, false);
    group.readDoubleVar(LIDAR_APERTURE_EFFICIENCY, _lidarApertureEfficiency, false);
    group.readDoubleVar(LIDAR_FIELD_OF_VIEW, _lidarFieldOfViewMrad, false);
    group.readDoubleVar(LIDAR_BEAM_DIVERGENCE, _lidarBeamDivergenceMrad, false);
    
    _readFrequency(group);
    
  }

}

///////////////////////////////////
// read the frequency variable
// throws exception on error

void Cf2RadxFile::_readFrequency(NcxxGroup &group)

{
  
  _frequency.clear();
  NcxxVar var = group.getVar(FREQUENCY);
  if (var.isNull()) {
    return;
  }

  int nFreq = var.numVals();
  RadxArray<double> freq_;
  double *freq = freq_.alloc(nFreq);
  var.getVal(freq);
  for (int ii = 0; ii < nFreq; ii++) {
    _frequency.push_back(freq[ii]);
  }
  
}

 /////////////////////////////////////////////////////////
 // read the georeference corrections

 void Cf2RadxFile::_readGeorefCorrections()

 {

   _cfactors.clear();

   NcxxGroup group = _file.getGroup(GEOREF_CORRECTION);
   if (group.isNull()) {
     _correctionsActive = false;
     return;
   }

   _correctionsActive = true;
   double val;
   
   group.readDoubleVar(AZIMUTH_CORRECTION, val, 0);
   _cfactors.setAzimuthCorr(val);
   
   group.readDoubleVar(ELEVATION_CORRECTION, val, 0);
   _cfactors.setElevationCorr(val);
   
   group.readDoubleVar(RANGE_CORRECTION, val, 0);
   _cfactors.setRangeCorr(val);
   
   group.readDoubleVar(LONGITUDE_CORRECTION, val, 0);
   _cfactors.setLongitudeCorr(val);
   
   group.readDoubleVar(LATITUDE_CORRECTION, val, 0);
   _cfactors.setLatitudeCorr(val);
   
   group.readDoubleVar(PRESSURE_ALTITUDE_CORRECTION, val, 0);
   _cfactors.setPressureAltCorr(val);
   
   group.readDoubleVar(ALTITUDE_CORRECTION, val, 0);
   _cfactors.setAltitudeCorr(val);
   
   group.readDoubleVar(EASTWARD_VELOCITY_CORRECTION, val, 0);
   _cfactors.setEwVelCorr(val);
   
   group.readDoubleVar(NORTHWARD_VELOCITY_CORRECTION, val, 0);
   _cfactors.setNsVelCorr(val);
   
   group.readDoubleVar(VERTICAL_VELOCITY_CORRECTION, val, 0);
   _cfactors.setVertVelCorr(val);
   
   group.readDoubleVar(HEADING_CORRECTION, val, 0);
   _cfactors.setHeadingCorr(val);
   
   group.readDoubleVar(ROLL_CORRECTION, val, 0);
   _cfactors.setRollCorr(val);
   
   group.readDoubleVar(PITCH_CORRECTION, val, 0);
   _cfactors.setPitchCorr(val);
   
   group.readDoubleVar(DRIFT_CORRECTION, val, 0);
   _cfactors.setDriftCorr(val);
   
   group.readDoubleVar(ROTATION_CORRECTION, val, 0);
   _cfactors.setRotationCorr(val);
   
   group.readDoubleVar(TILT_CORRECTION, val, 0);
   _cfactors.setTiltCorr(val);
   
 }

 ///////////////////////////////////
 // read the location

 void Cf2RadxFile::_readLocation()

 {

   // latitude

   try {
     _file.readDoubleVar(LATITUDE, _latitude, Radx::missingFl64);
   } catch (NcxxException e) {
     _latitude = 0.0;
     cerr << "WARNING - Cf2RadxFile::_readLocation" << endl;
     cerr << "  No latitude variable" << endl;
     cerr << "  Setting latitude to 0" << endl;
   }

   // longitude

   try {
     _file.readDoubleVar(LONGITUDE, _longitude, Radx::missingFl64);
   } catch (NcxxException e) {
     _longitude = 0.0;
     cerr << "WARNING - Cf2RadxFile::_readLocation" << endl;
     cerr << "  No longitude variable" << endl;
     cerr << "  Setting longitude to 0" << endl;
   }

   // altitude

   try {
     _file.readDoubleVar(ALTITUDE, _altitudeM, Radx::missingFl64);
   } catch (NcxxException e) {
     _altitudeM = 0.0;
     cerr << "WARNING - Cf2RadxFile::_readLocation" << endl;
     cerr << "  No altitude variable" << endl;
     cerr << "  Setting altitude to 0" << endl;
   }

   // altitude AGL

   try {
     _file.readDoubleVar(ALTITUDE_AGL, _altitudeAglM, Radx::missingFl64);
   } catch (NcxxException e) {
     _altitudeAglM = 0.0;
   }

 }

 ////////////////////////////////////////
 // read the radar calibrations

 void Cf2RadxFile::_readRadarCalibration()

 {

   NcxxGroup group = _file.getGroup(RADAR_CALIBRATION);
   if (group.isNull()) {
     return;
   }

   NcxxDim dim = group.getDim(R_CALIB);
   size_t nCalib = dim.getSize();
   
   for (size_t ical = 0; ical < nCalib; ical++) {
     RadxRcalib *cal = new RadxRcalib;
     try {
       _readRcal(group, dim, *cal, ical);
     } catch (NcxxException& e) {
       NcxxErrStr err;
       cerr << "WARNING - Cf2RadxFile::_readCalibrationVariables" << endl;
       cerr << "  calibration found, but error on read" << endl;
       cerr << "  ical: " << ical << endl;
       cerr << e.what() << endl;
       continue;
     }
     // there is generally one cal per pulse width
     // check that this is not a duplicate
     bool alreadyAdded = false;
     for (size_t ii = 0; ii < _rCals.size(); ii++) {
       const RadxRcalib *rcal = _rCals[ii];
       if (fabs(rcal->getPulseWidthUsec() -
                cal->getPulseWidthUsec()) < 0.0001) {
         alreadyAdded = true;
       }
     }
     if (!alreadyAdded) {
       _rCals.push_back(cal);
     }
   } // ical
   
 }

 ////////////////////////////////////////
 // read a specific cal
 // throws exception on error

 void Cf2RadxFile::_readRcal(NcxxGroup &group,
                             NcxxDim &dim,
                             RadxRcalib &cal,
                             size_t index)

 {

   // must have time, pulse width and receiver gain for HC channel

   try {
     time_t ctime;
     _readCalTime(group, dim, CALIBRATION_TIME, index, ctime);
     cal.setCalibTime(ctime);
   } catch (NcxxException e) {
     NcxxErrStr err;
     err.addErrStr("ERROR - Cf2RadxFile::_readRcal");
     err.addErrStr("  Cannot read cal time");
     err.addErrStr(e.what());
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }


   double width;
   try {
     _readCalVar(group, dim, PULSE_WIDTH, index, width, true);
     if (width > 0) {
       cal.setPulseWidthUsec(width * 1.0e6);
     }
   } catch (NcxxException e) {
     NcxxErrStr err;
     err.addErrStr("ERROR - Cf2RadxFile::_readRcal");
     err.addErrStr("  Cannot read pulse width");
     err.addErrDbl("  pulse_width: ", width, "%g");
     err.addErrStr(e.what());
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }

   double gain;
   try {
     _readCalVar(group, dim, RECEIVER_GAIN_HC, index, gain, true);
     if (gain > -9000) {
       cal.setReceiverGainDbHc(gain);
     }
   } catch (NcxxException e) {
     NcxxErrStr err;
     err.addErrStr("ERROR - Cf2RadxFile::_readRcal");
     err.addErrStr("  Cannot read receiver gain");
     err.addErrDbl("  receiver_gain_hc: ", gain, "%g");
     err.addErrStr(e.what());
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }

   // other values are not required

   try {

     double val;

     _readCalVar(group, dim, XMIT_POWER_H, index, val);
     cal.setXmitPowerDbmH(val);

     _readCalVar(group, dim, XMIT_POWER_V, index, val);
     cal.setXmitPowerDbmV(val);

     _readCalVar(group, dim, TWO_WAY_WAVEGUIDE_LOSS_H, index, val);
     cal.setTwoWayWaveguideLossDbH(val);

     _readCalVar(group, dim, TWO_WAY_WAVEGUIDE_LOSS_V, index, val);
     cal.setTwoWayWaveguideLossDbV(val);

     _readCalVar(group, dim, TWO_WAY_RADOME_LOSS_H, index, val);
     cal.setTwoWayRadomeLossDbH(val);

     _readCalVar(group, dim, TWO_WAY_RADOME_LOSS_V, index, val);
     cal.setTwoWayRadomeLossDbV(val);

     _readCalVar(group, dim, RECEIVER_MISMATCH_LOSS, index, val);
     cal.setReceiverMismatchLossDb(val);

     _readCalVar(group, dim, RADAR_CONSTANT_H, index, val);
     cal.setRadarConstantH(val);

     _readCalVar(group, dim, RADAR_CONSTANT_V, index, val);
     cal.setRadarConstantV(val);

     _readCalVar(group, dim, ANTENNA_GAIN_H, index, val);
     cal.setAntennaGainDbH(val);

     _readCalVar(group, dim, ANTENNA_GAIN_V, index, val);
     cal.setAntennaGainDbV(val);

     _readCalVar(group, dim, NOISE_HC, index, val, true);
     cal.setNoiseDbmHc(val);

     _readCalVar(group, dim, NOISE_HX, index, val);
     cal.setNoiseDbmHx(val);

     _readCalVar(group, dim, NOISE_VC, index, val);
     cal.setNoiseDbmVc(val);

     _readCalVar(group, dim, NOISE_VX, index, val);
     cal.setNoiseDbmVx(val);

     _readCalVar(group, dim, RECEIVER_GAIN_HX, index, val);
     cal.setReceiverGainDbHx(val);

     _readCalVar(group, dim, RECEIVER_GAIN_VC, index, val);
     cal.setReceiverGainDbVc(val);

     _readCalVar(group, dim, RECEIVER_GAIN_VX, index, val);
     cal.setReceiverGainDbVx(val);

     _readCalVar(group, dim, BASE_DBZ_1KM_HC, index, val);
     cal.setBaseDbz1kmHc(val);

     _readCalVar(group, dim, BASE_DBZ_1KM_HX, index, val);
     cal.setBaseDbz1kmHx(val);

     _readCalVar(group, dim, BASE_DBZ_1KM_VC, index, val);
     cal.setBaseDbz1kmVc(val);

     _readCalVar(group, dim, BASE_DBZ_1KM_VX, index, val);
     cal.setBaseDbz1kmVx(val);

     _readCalVar(group, dim, SUN_POWER_HC, index, val);
     cal.setSunPowerDbmHc(val);

     _readCalVar(group, dim, SUN_POWER_HX, index, val);
     cal.setSunPowerDbmHx(val);

     _readCalVar(group, dim, SUN_POWER_VC, index, val);
     cal.setSunPowerDbmVc(val);

     _readCalVar(group, dim, SUN_POWER_VX, index, val);
     cal.setSunPowerDbmVx(val);

     _readCalVar(group, dim, NOISE_SOURCE_POWER_H, index, val);
     cal.setNoiseSourcePowerDbmH(val);

     _readCalVar(group, dim, NOISE_SOURCE_POWER_V, index, val);
     cal.setNoiseSourcePowerDbmV(val);

     _readCalVar(group, dim, POWER_MEASURE_LOSS_H, index, val);
     cal.setPowerMeasLossDbH(val);

     _readCalVar(group, dim, POWER_MEASURE_LOSS_V, index, val);
     cal.setPowerMeasLossDbV(val);

     _readCalVar(group, dim, COUPLER_FORWARD_LOSS_H, index, val);
     cal.setCouplerForwardLossDbH(val);

     _readCalVar(group, dim, COUPLER_FORWARD_LOSS_V, index, val);
     cal.setCouplerForwardLossDbV(val);

     _readCalVar(group, dim, DBZ_CORRECTION, index, val);
     cal.setDbzCorrection(val);

     _readCalVar(group, dim, ZDR_CORRECTION, index, val);
     cal.setZdrCorrectionDb(val);

     _readCalVar(group, dim, LDR_CORRECTION_H, index, val);
     cal.setLdrCorrectionDbH(val);

     _readCalVar(group, dim, LDR_CORRECTION_V, index, val);
     cal.setLdrCorrectionDbV(val);

     _readCalVar(group, dim, SYSTEM_PHIDP, index, val);
     cal.setSystemPhidpDeg(val);

     _readCalVar(group, dim, TEST_POWER_H, index, val);
     cal.setTestPowerDbmH(val);

     _readCalVar(group, dim, TEST_POWER_V, index, val);
     cal.setTestPowerDbmV(val);

     _readCalVar(group, dim, RECEIVER_SLOPE_HC, index, val);
     cal.setReceiverSlopeDbHc(val);

     _readCalVar(group, dim, RECEIVER_SLOPE_HX, index, val);
     cal.setReceiverSlopeDbHx(val);

     _readCalVar(group, dim, RECEIVER_SLOPE_VC, index, val);
     cal.setReceiverSlopeDbVc(val);

     _readCalVar(group, dim, RECEIVER_SLOPE_VX, index, val);
     cal.setReceiverSlopeDbVx(val);

   } catch (NcxxException e) {
   }

 }

 ///////////////////////////////////
 // get calibration time
 // throws exception on error

 void Cf2RadxFile::_readCalTime(NcxxGroup &group,
                                NcxxDim &dim,
                                const string &name,
                                size_t index,
                                time_t &val)

 {

   // read the time strings

   vector<string> timeStrings;
   _read1DVar(group, dim, name, timeStrings);

   if (index > timeStrings.size() - 1) {
     NcxxErrStr err;
     err.addErrStr("ERROR - Cf2RadxFile::_readCalTime");
     err.addErrStr("  index value exceeds dim size");
     err.addErrInt("  index: ", index);
     err.addErrInt("  dim: ", timeStrings.size());
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }

   // scan the string to get the time

   const char *timeStr = timeStrings[index].c_str();
   int year, month, day, hour, min, sec;
   if (sscanf(timeStr, "%4d-%2d-%2dT%2d:%2d:%2dZ",
              &year, &month, &day, &hour, &min, &sec) != 6) {
     NcxxErrStr err;
     err.addErrStr("ERROR - Cf2RadxFile::_readCalTime");
     err.addErrStr("  Cannot parse cal time string: ", timeStr);
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }
   RadxTime ctime(year, month, day, hour, min, sec);
   val = ctime.utime();

 }

 ///////////////////////////////////
 // get calibration variable
 // throws exception on error

 NcxxVar Cf2RadxFile::_readCalVar(NcxxGroup &group,
                                  NcxxDim &dim,
                                  const string &name,
                                  size_t index, 
                                  double &val,
                                  bool required)

 {

   val = Radx::missingMetaDouble;

   NcxxVar var = group.getVar(name);

   if (var.isNull()) {
     if (!required) {
       return var;
     } else {
       NcxxErrStr err;
       err.addErrStr("ERROR - Cf2RadxFile::_readCalVar");
       err.addErrStr("  cal variable name: ", name);
       err.addErrStr("  group name: ", group.getName());
       err.addErrStr("  Cannot read calibration variable");
       throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
     }
   }

   if (var.numVals() < (int) index - 1) {
     NcxxErrStr err;
     err.addErrStr("ERROR - Cf2RadxFile::_readCalVar");
     err.addErrStr("  requested index too high");
     err.addErrStr("  cal variable name: ", name);
     err.addErrInt("  requested index: ", index);
     err.addErrInt("  n cals available: ", var.numVals());
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }

   // read value

   vector<size_t> indices;
   indices.push_back(index);
   double data;
   var.getVal(indices, &data);
   val = data;

   return var;

 }

 ////////////////////////////////////////
 // read the lidar calibrations
 // throws exception on error

 void Cf2RadxFile::_readLidarCalibration()

 {

 }

 ////////////////////////////////////////
 // read the sweeps with their fields
 // throws exception on error

 void Cf2RadxFile::_readSweeps()

 {

   // loop through the sweeps in this file

   for (size_t isweep = 0; isweep < _sweepGroups.size(); isweep++) {

     // check that this sweep is required

     bool processThisSweep = false;
     for (size_t ii = 0; ii < _sweepsToRead.size(); ii++) {
       if (_pathInUse == _sweepsToRead[ii].path &&
           isweep == _sweepsToRead[ii].indexInFile) {
         processThisSweep = true;
         break;
       }
     }
     if (!processThisSweep) {
       continue;
     }

     _sweepGroup = _sweepGroups[isweep];
     _readSweep(_sweepsInFile[isweep]);

     // add sweeps to main array

     for (size_t ii = 0; ii < _sweepRays.size(); ii++) {
       _raysFromFile.push_back(_sweepRays[ii]);
     }

   } // isweep

 }

 ////////////////////////////////////////
 // read a sweep with its fields
 // throws exception on error

 void Cf2RadxFile::_readSweep(RadxSweep *sweep)

 {

   NcxxErrStr err;
   err.addErrStr("Cf2RadxFile::_readSweep()");
   err.addErrStr("  Processing sweep group: ", _sweepGroup.getName());

   // get dimensions

   try {
     _timeDimSweep = _sweepGroup.getDim(TIME);
   } catch (NcxxException e) {
     err.addErrStr("  ERROR - no time dimension");
     err.addErrStr("  exception: ", e.what());
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }

   try {
     _rangeDimSweep = _sweepGroup.getDim(RANGE);
   } catch (NcxxException e) {
     err.addErrStr("  ERROR - no range dimension");
     err.addErrStr("  exception: ", e.what());
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }

   // read in times

   _sweepTimes.clear();
   try {
     _readSweepTimes(_sweepGroup, _sweepTimes);
   } catch (NcxxException e) {
     err.addErrStr("  ERROR reading times array");
     err.addErrStr("  exception: ", e.what());
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }

   // read range table

   _sweepRangeKm.clear();
   try {
     _readSweepRange(_sweepGroup, _rangeDimSweep, _sweepRangeKm);
   } catch (NcxxException e) {
     err.addErrStr("  ERROR reading range table");
     err.addErrStr("  exception: ", e.what());
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }

   // read in ray variables

   try {
     _readRayVariables();
   } catch (NcxxException e) {
     err.addErrStr("  ERROR reading ray variables");
     err.addErrStr("  exception: ", e.what());
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }

   // read in georef variables if active

   try {
     _readGeorefVariables();
   } catch (NcxxException e) {
   }

   if (_readMetadataOnly) {

     // read field variables with only metadata
     // no actual field data

     _readFieldVariables(true);

   } else {

     // create the rays to be read in, filling out the metadata

     _createSweepRays(sweep);

     // add field variables to the rays

     _readFieldVariables(false);

   }

   // compute fixed angles if not found
   
   if (!_fixedAngleFound) {
     _computeFixedAngle(sweep);
   }

 }

 ////////////////////////////////////////
 // read the range variable for a sweep

 void Cf2RadxFile::_readSweepRange(NcxxGroup &group, NcxxDim &dim,
                                   vector<double> rangeKm)

 {

   NcxxVar rangeVar = group.getVar(RANGE);
   if (rangeVar.isNull() || rangeVar.numVals() < 1) {
     NcxxErrStr err;
     err.addErrStr("ERROR - Cf2RadxFile::_readSweepRange");
     err.addErrStr("  group: ", group.getName());
     err.addErrStr("  Cannot find range variable");
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }

   rangeKm.clear();
   _nRangeInSweep = dim.getSize();

   if (rangeVar.getDimCount() != 1) {
     NcxxErrStr err;
     err.addErrStr("ERROR - Cf2RadxFile::_readSweepRange");
     err.addErrStr("  group: ", group.getName());
     err.addErrInt("  range nDims = ", rangeVar.getDimCount());
     err.addErrStr("  should be 1");
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }

   RadxArray<double> rangeMeters_;
   double *rangeMeters = rangeMeters_.alloc(_nRangeInSweep);
   try {
     rangeVar.getVal(rangeMeters);
     double *rr = rangeMeters;
     for (size_t ii = 0; ii < _nRangeInSweep; ii++, rr++) {
       rangeKm.push_back(*rr / 1000.0);
     }
   } catch (NcxxException& e) {
     NcxxErrStr err;
     err.addErrStr("ERROR - Cf2RadxFile::_readSweepRange");
     err.addErrStr("  Cannot read range data");
     err.addErrStr("  exception: ", e.what());
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }

   // set the geometry from the range vector

   _remapSweep.computeRangeLookup(rangeKm);
   _gateSpacingIsConstant = _remapSweep.getGateSpacingIsConstant();
   _geomSweep.setRangeGeom(_remapSweep.getStartRangeKm(), _remapSweep.getGateSpacingKm());

   // get attributes and check for geometry

   double startRangeKm = Radx::missingMetaDouble;
   double gateSpacingKm = Radx::missingMetaDouble;

   map<string, NcxxVarAtt> atts = rangeVar.getAtts();
   for (map<string, NcxxVarAtt>::iterator ii = atts.begin();
        ii != atts.end(); ii++) {

     NcxxVarAtt att = ii->second;

     if (att.isNull()) {
       continue;
     }

     if (att.getName().find(METERS_TO_CENTER_OF_FIRST_GATE) != string::npos) {
       vector<double> vals;
       try {
         att.getValues(vals);
         startRangeKm = vals[0] / 1000.0;
       } catch (NcxxException& e) {
       }
     }

     if (att.getName().find(METERS_BETWEEN_GATES) != string::npos) {
       vector<double> vals;
       try {
         att.getValues(vals);
         gateSpacingKm = vals[0] / 1000.0;
       } catch (NcxxException& e) {
       }
     }

   } // ii

   if (startRangeKm != Radx::missingMetaDouble &&
       gateSpacingKm != Radx::missingMetaDouble) {
     _geomSweep.setRangeGeom(startRangeKm, gateSpacingKm);
   }

 }

 ///////////////////////////////////
 // clear the ray variables

 void Cf2RadxFile::_clearRayVariables()

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
 // throws exception on error

 void Cf2RadxFile::_readRayVariables()

 {

   _clearRayVariables();

   // azimuth

   try {
     _readRayVar(_sweepGroup, _timeDimSweep, AZIMUTH, _rayAzimuths, true);
   } catch (NcxxException& e) {
     NcxxErrStr err;
     err.addErrStr("ERROR - Cf2RadxFile::_readRayVariables");
     err.addErrStr("  Ray must have azimuth");
     err.addErrStr("  exception: ", e.what());
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }

   try {
     _readRayVar(_sweepGroup, _timeDimSweep, ELEVATION, _rayElevations, true);
   } catch (NcxxException& e) {
     NcxxErrStr err;
     err.addErrStr("ERROR - Cf2RadxFile::_readRayVariables");
     err.addErrStr("  Ray must have elevation");
     err.addErrStr("  exception: ", e.what());
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }

   _readRayVar(_sweepGroup, _timeDimSweep, PULSE_WIDTH, _rayPulseWidths, false);
   _readRayVar(_sweepGroup, _timeDimSweep, PRT, _rayPrts, false);
   _readRayVar(_sweepGroup, _timeDimSweep, PRT_RATIO, _rayPrtRatios, false);
   _readRayVar(_sweepGroup, _timeDimSweep, NYQUIST_VELOCITY, _rayNyquists, false);
   _readRayVar(_sweepGroup, _timeDimSweep, UNAMBIGUOUS_RANGE, _rayUnambigRanges, false);
   _readRayVar(_sweepGroup, _timeDimSweep, ANTENNA_TRANSITION, 
               _rayAntennaTransitions, false);
   _readRayVar(_sweepGroup, _timeDimSweep, GEOREFS_APPLIED,
               _rayGeorefsApplied, false);
   _readRayVar(_sweepGroup, _timeDimSweep, N_SAMPLES, _rayNSamples, false);
   _readRayVar(_sweepGroup, _timeDimSweep, R_CALIB_INDEX, _rayCalNum, false);

   // monitoring

   NcxxGroup monGroup = _sweepGroup.getGroup(MONITORING);
   if (!monGroup.isNull()) {
  
     _readRayVar(monGroup, _timeDimSweep, SCAN_RATE, _rayScanRate, false);
     _readRayVar(monGroup, _timeDimSweep, RADAR_ESTIMATED_NOISE_DBM_HC,
                 _rayEstNoiseDbmHc, false);
     _readRayVar(monGroup, _timeDimSweep, RADAR_ESTIMATED_NOISE_DBM_VC,
                 _rayEstNoiseDbmVc, false);
     _readRayVar(monGroup, _timeDimSweep, RADAR_ESTIMATED_NOISE_DBM_HX,
                 _rayEstNoiseDbmHx, false);
     _readRayVar(monGroup, _timeDimSweep, RADAR_ESTIMATED_NOISE_DBM_VX,
                 _rayEstNoiseDbmVx, false);

     _readRayVar(monGroup, _timeDimSweep, RADAR_MEASURED_TRANSMIT_POWER_H, 
                 _rayXmitPowerH, false);
     _readRayVar(monGroup, _timeDimSweep, RADAR_MEASURED_TRANSMIT_POWER_V, 
                 _rayXmitPowerV, false);

   }

 }

 ///////////////////////////////////
 // clear the georeference vectors

 void Cf2RadxFile::_clearGeorefVariables()

 {

   _geoTime.clear();
   _geoUnitNum.clear();
   _geoUnitId.clear();
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
 // throws exception on error

 void Cf2RadxFile::_readGeorefVariables()

 {

   // clear them

   _clearGeorefVariables();
   _georefsActive = false;

   // do we have a georef group in this sweep?

   NcxxGroup georefGroup = _sweepGroup.getGroup(GEOREFERENCE);
   if (georefGroup.isNull()) {
     // no georef group
     return;
   }

   try {
     _readRayVar(georefGroup, _timeDimSweep, GEOREF_TIME, _geoTime);
   } catch (NcxxException& e) {
     NcxxErrStr err;
     err.addErrStr("ERROR - Cf2RadxFile::_readGeorefVariables");
     err.addErrStr("  Georef must have time");
     err.addErrStr("  exception: ", e.what());
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }

   try {
     _readRayVar(georefGroup, _timeDimSweep, LATITUDE, _geoLatitude);
   } catch (NcxxException& e) {
     NcxxErrStr err;
     err.addErrStr("ERROR - Cf2RadxFile::_readGeorefVariables");
     err.addErrStr("  Georef must have latitude");
     err.addErrStr("  exception: ", e.what());
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }

   try {
     _readRayVar(georefGroup, _timeDimSweep, LONGITUDE, _geoLongitude);
   } catch (NcxxException& e) {
     NcxxErrStr err;
     err.addErrStr("ERROR - Cf2RadxFile::_readGeorefVariables");
     err.addErrStr("  Georef must have longitude");
     err.addErrStr("  exception: ", e.what());
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }

   try {
     _readRayVar(georefGroup, _timeDimSweep, ALTITUDE, _geoAltitudeMsl);
   } catch (NcxxException& e) {
     NcxxErrStr err;
     err.addErrStr("ERROR - Cf2RadxFile::_readGeorefVariables");
     err.addErrStr("  Georef must have altitude");
     err.addErrStr("  exception: ", e.what());
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
   }

   _readRayVar(georefGroup, _timeDimSweep, GEOREF_UNIT_NUM, _geoUnitNum, false);
   _readRayVar(georefGroup, _timeDimSweep, GEOREF_UNIT_ID, _geoUnitId, false);

   _readRayVar(georefGroup, _timeDimSweep, ALTITUDE_AGL, _geoAltitudeAgl, false); // meters

   _readRayVar(georefGroup, _timeDimSweep, EASTWARD_VELOCITY, _geoEwVelocity, false);
   _readRayVar(georefGroup, _timeDimSweep, NORTHWARD_VELOCITY, _geoNsVelocity, false);
   _readRayVar(georefGroup, _timeDimSweep, VERTICAL_VELOCITY, _geoVertVelocity, false);

   _readRayVar(georefGroup, _timeDimSweep, HEADING, _geoHeading, false);
   _readRayVar(georefGroup, _timeDimSweep, ROLL, _geoRoll, false);
   _readRayVar(georefGroup, _timeDimSweep, PITCH, _geoPitch, false);

   _readRayVar(georefGroup, _timeDimSweep, DRIFT, _geoDrift, false);
   _readRayVar(georefGroup, _timeDimSweep, ROTATION, _geoRotation, false);
   _readRayVar(georefGroup, _timeDimSweep, TILT, _geoTilt, false);

   _readRayVar(georefGroup, _timeDimSweep, EASTWARD_WIND, _geoEwWind, false);
   _readRayVar(georefGroup, _timeDimSweep, NORTHWARD_WIND, _geoNsWind, false);
   _readRayVar(georefGroup, _timeDimSweep, VERTICAL_WIND, _geoVertWind, false);

   _readRayVar(georefGroup, _timeDimSweep, HEADING_CHANGE_RATE, _geoHeadingRate, false);
   _readRayVar(georefGroup, _timeDimSweep, PITCH_CHANGE_RATE, _geoPitchRate, false);
   _readRayVar(georefGroup, _timeDimSweep, DRIVE_ANGLE_1, _geoDriveAngle1, false);
   _readRayVar(georefGroup, _timeDimSweep, DRIVE_ANGLE_2, _geoDriveAngle2, false);

   _georefsActive = true;

 }

 ///////////////////////////////////
 // create the rays to be read in
 // for a sweep, and set meta data
 // throws exception on error

 void Cf2RadxFile::_createSweepRays(const RadxSweep *sweep)

 {

   _sweepRays.clear();

   for (size_t iray = 0; iray < _timeDimSweep.getSize(); iray++) {

     // new ray

     RadxRay *ray = new RadxRay;
     ray->copyRangeGeom(_geomSweep);

     // set time

     double rayTimeDouble = _sweepTimes[iray];
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

     if (_rayAzimuths.size() > iray) {
       ray->setAzimuthDeg(_rayAzimuths[iray]);
     }
     if (_rayElevations.size() > iray) {
       ray->setElevationDeg(_rayElevations[iray]);
     }
     if (_rayPulseWidths.size() > iray) {
       ray->setPulseWidthUsec(_rayPulseWidths[iray] * 1.0e6);
     }
     if (_rayPrts.size() > iray) {
       ray->setPrtSec(_rayPrts[iray]);
     }
     if (_rayPrtRatios.size() > iray) {
       ray->setPrtRatio(_rayPrtRatios[iray]);
     }
     if (_rayNyquists.size() > iray) {
       ray->setNyquistMps(_rayNyquists[iray]);
     }
     if (_rayUnambigRanges.size() > iray) {
       if (_rayUnambigRanges[iray] > 0) {
         ray->setUnambigRangeKm(_rayUnambigRanges[iray] / 1000.0);
       }
     }
     if (_rayAntennaTransitions.size() > iray) {
       ray->setAntennaTransition(_rayAntennaTransitions[iray]);
     }
     if (_georefsActive && _rayGeorefsApplied.size() > iray) {
       ray->setGeorefApplied(_rayGeorefsApplied[iray]);
     }
     if (_rayNSamples.size() > iray) {
       ray->setNSamples(_rayNSamples[iray]);
     }
     if (_rayCalNum.size() > iray) {
       ray->setCalibIndex(_rayCalNum[iray]);
     }
     if (_rayXmitPowerH.size() > iray) {
       ray->setMeasXmitPowerDbmH(_rayXmitPowerH[iray]);
     }
     if (_rayXmitPowerV.size() > iray) {
       ray->setMeasXmitPowerDbmV(_rayXmitPowerV[iray]);
     }
     if (_rayScanRate.size() > iray) {
       ray->setTrueScanRateDegPerSec(_rayScanRate[iray]);
     }
     if (_rayEstNoiseDbmHc.size() > iray) {
       ray->setEstimatedNoiseDbmHc(_rayEstNoiseDbmHc[iray]);
     }
     if (_rayEstNoiseDbmVc.size() > iray) {
       ray->setEstimatedNoiseDbmVc(_rayEstNoiseDbmVc[iray]);
     }
     if (_rayEstNoiseDbmHx.size() > iray) {
       ray->setEstimatedNoiseDbmHx(_rayEstNoiseDbmHx[iray]);
     }
     if (_rayEstNoiseDbmVx.size() > iray) {
       ray->setEstimatedNoiseDbmVx(_rayEstNoiseDbmVx[iray]);
     }

     if (_georefsActive) {

       RadxGeoref geo;

       if (_geoTime.size() > iray) {
         double geoTime = _geoTime[iray];
         int secs = (int) geoTime;
         int nanoSecs = (int) ((geoTime - secs) * 1.0e9 + 0.5);
         time_t tSecs = _readVol->getStartTimeSecs() + secs;
         geo.setTimeSecs(tSecs);
         geo.setNanoSecs(nanoSecs);
       }
       if (_geoUnitNum.size() > iray) {
         geo.setUnitNum(_geoUnitNum[iray]);
       }
       if (_geoUnitId.size() > iray) {
         geo.setUnitId(_geoUnitId[iray]);
       }
       if (_geoLatitude.size() > iray) {
         geo.setLatitude(_geoLatitude[iray]);
       }
       if (_geoLongitude.size() > iray) {
         geo.setLongitude(_geoLongitude[iray]);
       }
       if (_geoAltitudeMsl.size() > iray) {
         geo.setAltitudeKmMsl(_geoAltitudeMsl[iray] / 1000.0);
       }
       if (_geoAltitudeAgl.size() > iray) {
         geo.setAltitudeKmAgl(_geoAltitudeAgl[iray] / 1000.0);
       }
       if (_geoEwVelocity.size() > iray) {
         geo.setEwVelocity(_geoEwVelocity[iray]);
       }
       if (_geoNsVelocity.size() > iray) {
         geo.setNsVelocity(_geoNsVelocity[iray]);
       }
       if (_geoVertVelocity.size() > iray) {
         geo.setVertVelocity(_geoVertVelocity[iray]);
       }
       if (_geoHeading.size() > iray) {
         geo.setHeading(_geoHeading[iray]);
       }
       if (_geoRoll.size() > iray) {
         geo.setRoll(_geoRoll[iray]);
       }
       if (_geoPitch.size() > iray) {
         geo.setPitch(_geoPitch[iray]);
       }
       if (_geoDrift.size() > iray) {
         geo.setDrift(_geoDrift[iray]);
       }
       if (_geoRotation.size() > iray) {
         geo.setRotation(_geoRotation[iray]);
       }
       if (_geoTilt.size() > iray) {
         geo.setTilt(_geoTilt[iray]);
       }
       if (_geoEwWind.size() > iray) {
         geo.setEwWind(_geoEwWind[iray]);
       }
       if (_geoNsWind.size() > iray) {
         geo.setNsWind(_geoNsWind[iray]);
       }
       if (_geoVertWind.size() > iray) {
         geo.setVertWind(_geoVertWind[iray]);
       }
       if (_geoHeadingRate.size() > iray) {
         geo.setHeadingRate(_geoHeadingRate[iray]);
       }
       if (_geoPitchRate.size() > iray) {
         geo.setPitchRate(_geoPitchRate[iray]);
       }
       if (_geoDriveAngle1.size() > iray) {
         geo.setDriveAngle1(_geoDriveAngle1[iray]);
       }
       if (_geoDriveAngle2.size() > iray) {
         geo.setDriveAngle2(_geoDriveAngle2[iray]);
       }

       ray->setGeoref(geo);

     } // if (_georefsActive) 

     // add to ray vector

     _sweepRays.push_back(ray);

   } // ii

 }

 ////////////////////////////////////////////
 // read the field variables
 // throws exception on error

 void Cf2RadxFile::_readFieldVariables(bool metaOnly)

 {

   // loop through the variables, adding data fields as appropriate

   const multimap<string, NcxxVar> &vars = _sweepGroup.getVars();

   for (multimap<string, NcxxVar>::const_iterator iter = vars.begin();
        iter != vars.end(); iter++) {

     NcxxVar var = iter->second;
     if (var.isNull()) {
       continue;
     }
     string name = var.getName();
     int numDims = var.getDimCount();
     if (numDims != 2) {
       continue;
     }
     // check that we have the correct dimensions
     const NcxxDim &timeDim = var.getDim(0);
     const NcxxDim &rangeDim = var.getDim(1);
     if (timeDim != _timeDimSweep || rangeDim != _rangeDimSweep) {
       continue;
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
         cerr << "DEBUG - Cf2RadxFile::_readFieldVariables" << endl;
         cerr << "  -->> rejecting field: " << name << endl;
       }
       continue;
     }
     if (name == "range") {
       if (_verbose) {
         cerr << "DEBUG - Cf2RadxFile::_readFieldVariables" << endl;
         cerr << "  -->> ignoring dimension variable: " << name << endl;
       }
       continue;
     }

     if (_verbose) {
       cerr << "DEBUG - Cf2RadxFile::_readFieldVariables" << endl;
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

     string fieldComment;
     try {
       NcxxVarAtt fieldCommentAtt = var.getAtt(COMMENT);
       fieldComment = fieldCommentAtt.asString();
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
         if (fieldComment.size() > 0) {
           field->setComment(fieldComment);
         }
         _readVol->addField(field);
       }
       continue;
     }

     try {

       switch (var.getType().getId()) {
         case NC_DOUBLE:
           _addFl64FieldToRays(var, name, units, standardName, longName,
                               isDiscrete, fieldFolds,
                               foldLimitLower, foldLimitUpper);
           break;
         case NC_FLOAT:
           _addFl32FieldToRays(var, name, units, standardName, longName,
                               isDiscrete, fieldFolds,
                               foldLimitLower, foldLimitUpper);
           break;
         case NC_INT:
           _addSi32FieldToRays(var, name, units, standardName, longName,
                               scale, offset,
                               isDiscrete, fieldFolds,
                               foldLimitLower, foldLimitUpper);
           break;
         case NC_SHORT:
           _addSi16FieldToRays(var, name, units, standardName, longName,
                               scale, offset,
                               isDiscrete, fieldFolds,
                               foldLimitLower, foldLimitUpper);
           break;
         case NC_BYTE:
           _addSi08FieldToRays(var, name, units, standardName, longName,
                               scale, offset,
                               isDiscrete, fieldFolds,
                               foldLimitLower, foldLimitUpper);
           break;
         default: {} // will not reach here because of earlier check on type

       } // switch

     } catch (NcxxException& e) {

       NcxxErrStr err;
       err.addErrStr("ERROR - Cf2RadxFile::_readFieldVariables");
       err.addErrStr("  exception: ", e.what());
       throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));

     }

   } // ivar

 }

 ////////////////////////////////////////////////////////
 // read a ray variable from a sweep - double
 // side effect: sets vals
 // returns var used
 // throws exception on failure

 NcxxVar Cf2RadxFile::_readRayVar(NcxxGroup &group,
                                  NcxxDim &dim,
                                  const string &name,
                                  vector<double> &vals, 
                                  bool required)

 {

   vals.clear();
   size_t nTimes = dim.getSize();

   // get var

   NcxxVar var = group.getVar(name);
   if (var.isNull()) {
     if (!required) {
       for (size_t ii = 0; ii < nTimes; ii++) {
         vals.push_back(Radx::missingMetaDouble);
       }
       clearErrStr();
       return var;
     } else {
       NcxxErrStr err;
       err.addErrStr("ERROR - Cf2RadxFile::_readRayVar");
       err.addErrStr("  Cannot find var, name: ", name);
       err.addErrStr("  group name: ", group.getName());
       throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
     }
   }

   // load up data

   RadxArray<double> data_;
   double *data = data_.alloc(nTimes);
   try {
     var.getVal(data);
     for (size_t ii = 0; ii < nTimes; ii++) {
       vals.push_back(data[ii]);
     }
   } catch (NcxxException& e) {
     if (!required) {
       for (size_t ii = 0; ii < nTimes; ii++) {
         vals.push_back(Radx::missingMetaDouble);
       }
       clearErrStr();
     } else {
       NcxxErrStr err;
       err.addErrStr("ERROR - Cf2RadxFile::_readRayVar");
       err.addErrStr("  Cannot read variable: ", name);
       err.addErrStr("  exception: ", e.what());
       throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
     }
   }

   return var;

 }

 ///////////////////////////////////
 // read a ray variable - integer
 // side effect: sets vals
 // returns var used
 // throws exception on failure

 NcxxVar Cf2RadxFile::_readRayVar(NcxxGroup &group,
                                  NcxxDim &dim,
                                  const string &name,
                                  vector<int> &vals,
                                  bool required)

 {

   vals.clear();
   size_t nTimes = dim.getSize();

   // get var

   NcxxVar var = group.getVar(name);
   if (var.isNull()) {
     if (!required) {
       for (size_t ii = 0; ii < nTimes; ii++) {
         vals.push_back(Radx::missingMetaDouble);
       }
       clearErrStr();
       return var;
     } else {
       NcxxErrStr err;
       err.addErrStr("ERROR - Cf2RadxFile::_readRayVar");
       err.addErrStr("  Cannot find var, name: ", name);
       err.addErrStr("  group name: ", group.getName());
       throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
     }
   }

   // load up data

   RadxArray<int> data_;
   int *data = data_.alloc(nTimes);
   try {
     var.getVal(data);
     for (size_t ii = 0; ii < nTimes; ii++) {
       vals.push_back(data[ii]);
     }
   } catch (NcxxException& e) {
     if (!required) {
       for (size_t ii = 0; ii < nTimes; ii++) {
         vals.push_back(Radx::missingMetaInt);
       }
       clearErrStr();
     } else {
       NcxxErrStr err;
       err.addErrStr("ERROR - Cf2RadxFile::_readRayVar");
       err.addErrStr("  Cannot read variable: ", name);
       err.addErrStr("  exception: ", e.what());
       throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
     }
   }

   return var;

 }

 ///////////////////////////////////
 // read a ray variable - long integer
 // side effect: sets vals
 // returns var used
 // throws exception on failure

 NcxxVar Cf2RadxFile::_readRayVar(NcxxGroup &group,
                                  NcxxDim &dim,
                                  const string &name,
                                  vector<Radx::si64> &vals,
                                  bool required)

 {

   vals.clear();
   size_t nTimes = dim.getSize();

   // get var

   NcxxVar var = group.getVar(name);
   if (var.isNull()) {
     if (!required) {
       for (size_t ii = 0; ii < nTimes; ii++) {
         vals.push_back(Radx::missingMetaDouble);
       }
       clearErrStr();
       return var;
     } else {
       NcxxErrStr err;
       err.addErrStr("ERROR - Cf2RadxFile::_readRayVar");
       err.addErrStr("  Cannot find var, name: ", name);
       err.addErrStr("  group name: ", group.getName());
       throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
     }
   }

   // load up data

   RadxArray<Radx::si64> data_;
   Radx::si64 *data = data_.alloc(nTimes);
   try {
     var.getVal(data);
     for (size_t ii = 0; ii < nTimes; ii++) {
       vals.push_back(data[ii]);
     }
   } catch (NcxxException& e) {
     if (!required) {
       for (size_t ii = 0; ii < nTimes; ii++) {
         vals.push_back(Radx::missingMetaInt);
       }
       clearErrStr();
     } else {
       NcxxErrStr err;
       err.addErrStr("ERROR - Cf2RadxFile::_readRayVar");
       err.addErrStr("  Cannot read variable: ", name);
       err.addErrStr("  exception: ", e.what());
       throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
     }
   }

   return var;

 }

 ///////////////////////////////////////////
 // read a ray variable - boolean
 // side effect: sets vals
 // returns var used
 // throws exception on failure

 NcxxVar Cf2RadxFile::_readRayVar(NcxxGroup &group,
                                  NcxxDim &dim,
                                  const string &name,
                                  vector<bool> &vals,
                                  bool required)

 {

   vals.clear();
   size_t nTimes = dim.getSize();

   // get var

   NcxxVar var = group.getVar(name);
   if (var.isNull()) {
     if (!required) {
       for (size_t ii = 0; ii < nTimes; ii++) {
         vals.push_back(Radx::missingMetaDouble);
       }
       clearErrStr();
       return var;
     } else {
       NcxxErrStr err;
       err.addErrStr("ERROR - Cf2RadxFile::_readRayVar");
       err.addErrStr("  Cannot find var, name: ", name);
       err.addErrStr("  group name: ", group.getName());
       throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
     }
   }

   // load up data

   RadxArray<int> data_;
   int *data = data_.alloc(nTimes);
   try {
     var.getVal(data);
     for (size_t ii = 0; ii < nTimes; ii++) {
       if (data[ii] == 0) {
         vals.push_back(false);
       } else {
         vals.push_back(true);
       }
     }
   } catch (NcxxException& e) {
     if (!required) {
       for (size_t ii = 0; ii < nTimes; ii++) {
         vals.push_back(false);
       }
       clearErrStr();
     } else {
       NcxxErrStr err;
       err.addErrStr("ERROR - Cf2RadxFile::_readRayVar");
       err.addErrStr("  Cannot read variable: ", name);
       err.addErrStr("  exception: ", e.what());
       throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
     }
   }

   return var;

 }


 //////////////////////////////////////////////////////////////
 // Add fl64 fields to _sweepRays
 // The _sweepRays array has previously been set up by _createSweepRays()
 // Throws exception on error

 void Cf2RadxFile::_addFl64FieldToRays(NcxxVar &var,
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

   size_t nTimes = _timeDimSweep.getSize();
   size_t nGates = _rangeDimSweep.getSize();
   size_t nVals = nTimes * nGates;

   RadxArray<Radx::fl64> data_;
   Radx::fl64 *data = data_.alloc(nVals);

   try {
     var.getVal(data);
   } catch (NcxxException& e) {
     NcxxErrStr err;
     err.addErrStr("ERROR - Cf2RadxFile::_addFl64FieldToRays");
     err.addErrStr("  Cannot read fl64 variable: ", name);
     err.addErrStr("  exception: ", e.what());
     throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
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

   for (size_t ii = 0; ii < nVals; ii++) {
     if (!isfinite(data[ii])) {
       data[ii] = missingVal;
     }
   }

   // load field on rays

   size_t startIndex = 0;
   for (size_t iray = 0; iray < _sweepRays.size();
        iray++, startIndex += nGates) {

     RadxField *field =
       _sweepRays[iray]->addField(name, units, nGates,
                                  missingVal,
                                  data + startIndex,
                                  true);

     field->setStandardName(standardName);
     field->setLongName(longName);
     field->copyRangeGeom(_geomSweep);

     if (fieldFolds &&
         foldLimitLower != Radx::missingMetaFloat &&
         foldLimitUpper != Radx::missingMetaFloat) {
       field->setFieldFolds(foldLimitLower, foldLimitUpper);
     }
     if (isDiscrete) {
       field->setIsDiscrete(true);
     }

   }

 }

 //////////////////////////////////////////////////////////////
 // Add fl32 fields to _sweepRays
 // The _sweepRays array has previously been set up by _createSweepRays()
// Throws exception on error

void Cf2RadxFile::_addFl32FieldToRays(NcxxVar &var,
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
  
  size_t nTimes = _timeDimSweep.getSize();
  size_t nGates = _rangeDimSweep.getSize();
  size_t nVals = nTimes * nGates;

  RadxArray<Radx::fl32> data_;
  Radx::fl32 *data = data_.alloc(nVals);
  
  try {
    var.getVal(data);
  } catch (NcxxException& e) {
    NcxxErrStr err;
    err.addErrStr("ERROR - Cf2RadxFile::_addFl32FieldToRays");
    err.addErrStr("  Cannot read fl32 variable: ", name);
    err.addErrStr("  exception: ", e.what());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  
  // set missing value
  
  Radx::fl32 missingVal = Radx::missingFl32;
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
  
  for (size_t ii = 0; ii < nVals; ii++) {
    if (!isfinite(data[ii])) {
      data[ii] = missingVal;
    }
  }
  
  // load field on rays

  size_t startIndex = 0;
  for (size_t iray = 0; iray < _sweepRays.size();
       iray++, startIndex += nGates) {
    
    RadxField *field =
      _sweepRays[iray]->addField(name, units, nGates,
                                 missingVal,
                                 data + startIndex,
                                 true);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geomSweep);
    
    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
}

//////////////////////////////////////////////////////////////
// Add si32 fields to _sweepRays
// The _sweepRays array has previously been set up by _createSweepRays()
// Throws exception on error

void Cf2RadxFile::_addSi32FieldToRays(NcxxVar &var,
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
  
  size_t nTimes = _timeDimSweep.getSize();
  size_t nGates = _rangeDimSweep.getSize();
  size_t nVals = nTimes * nGates;

  RadxArray<Radx::si32> data_;
  Radx::si32 *data = data_.alloc(nVals);
  
  try {
    var.getVal(data);
  } catch (NcxxException& e) {
    NcxxErrStr err;
    err.addErrStr("ERROR - Cf2RadxFile::_addSi32FieldToRays");
    err.addErrStr("  Cannot read si32 variable: ", name);
    err.addErrStr("  exception: ", e.what());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  
  // set missing value
  
  Radx::si32 missingVal = Radx::missingSi32;
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

  // load field on rays

  size_t startIndex = 0;
  for (size_t iray = 0; iray < _sweepRays.size();
       iray++, startIndex += nGates) {
    
    RadxField *field =
      _sweepRays[iray]->addField(name, units, nGates,
                                 missingVal,
                                 data + startIndex,
                                 scale, offset,
                                 true);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geomSweep);
    
    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
}

//////////////////////////////////////////////////////////////
// Add si16 fields to _sweepRays
// The _sweepRays array has previously been set up by _createSweepRays()
// Throws exception on error

void Cf2RadxFile::_addSi16FieldToRays(NcxxVar &var,
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
  
  size_t nTimes = _timeDimSweep.getSize();
  size_t nGates = _rangeDimSweep.getSize();
  size_t nVals = nTimes * nGates;

  RadxArray<Radx::si16> data_;
  Radx::si16 *data = data_.alloc(nVals);
  
  try {
    var.getVal(data);
  } catch (NcxxException& e) {
    NcxxErrStr err;
    err.addErrStr("ERROR - Cf2RadxFile::_addSi16FieldToRays");
    err.addErrStr("  Cannot read si16 variable: ", name);
    err.addErrStr("  exception: ", e.what());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  
  // set missing value
  
  Radx::si16 missingVal = Radx::missingSi16;
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

  // load field on rays

  size_t startIndex = 0;
  for (size_t iray = 0; iray < _sweepRays.size();
       iray++, startIndex += nGates) {
    
    RadxField *field =
      _sweepRays[iray]->addField(name, units, nGates,
                                 missingVal,
                                 data + startIndex,
                                 scale, offset,
                                 true);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geomSweep);
    
    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
}

//////////////////////////////////////////////////////////////
// Add si08 fields to _sweepRays
// The _sweepRays array has previously been set up by _createSweepRays()
// Throws exception on error

void Cf2RadxFile::_addSi08FieldToRays(NcxxVar &var,
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
  
  size_t nTimes = _timeDimSweep.getSize();
  size_t nGates = _rangeDimSweep.getSize();
  size_t nVals = nTimes * nGates;

  RadxArray<Radx::si08> data_;
  Radx::si08 *data = data_.alloc(nVals);
  
  try {
    var.getVal(data);
  } catch (NcxxException& e) {
    NcxxErrStr err;
    err.addErrStr("ERROR - Cf2RadxFile::_addSi08FieldToRays");
    err.addErrStr("  Cannot read si08 variable: ", name);
    err.addErrStr("  exception: ", e.what());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  
  // set missing value
  
  Radx::si08 missingVal = Radx::missingSi08;
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

  // load field on rays

  size_t startIndex = 0;
  for (size_t iray = 0; iray < _sweepRays.size();
       iray++, startIndex += nGates) {
    
    RadxField *field =
      _sweepRays[iray]->addField(name, units, nGates,
                                 missingVal,
                                 data + startIndex,
                                 scale, offset,
                                 true);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geomSweep);
    
    if (fieldFolds &&
        foldLimitLower != Radx::missingMetaFloat &&
        foldLimitUpper != Radx::missingMetaFloat) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

  }
  
}

/////////////////////////////////////////////////////////
// load up the read volume with the data from this object

void Cf2RadxFile::_loadReadVolume()
  
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

  _readVol->setLatitudeDeg(_latitude);
  _readVol->setLongitudeDeg(_longitude);
  _readVol->setAltitudeKm(_altitudeM / 1000.0);
  _readVol->setSensorHtAglM(_altitudeAglM);

  _readVol->copyRangeGeom(_geomSweep);

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

void Cf2RadxFile::_computeFixedAngle(RadxSweep *sweep)
  
{
  
  double sumElev = 0.0;
  double count = 0.0;
  
  for (size_t iray = 0; iray < _sweepRays.size(); iray++) {
    const RadxRay *ray = _sweepRays[iray];
    sumElev += ray->getElevationDeg();
    count++;
  }

  double meanElev = sumElev / count;
  double fixedAngle = ((int) (meanElev * 100.0 + 0.5)) / 100.0;
  
  sweep->setFixedAngleDeg(fixedAngle);

  for (size_t iray = 0; iray < _sweepRays.size(); iray++) {
    RadxRay *ray = _sweepRays[iray];
    ray->setFixedAngleDeg(fixedAngle);
  }
      
}

///////////////////////////////////
// read a sweep variable - double
// throws exception on error

NcxxVar Cf2RadxFile::_read1DVar(NcxxGroup &group,
                                NcxxDim &dim,
                                const string &name,
                                vector<double> &vals,
                                bool required)
  
{
  
  vals.clear();

  // get var
  
  size_t nVals = dim.getSize();
  NcxxVar var = group.getVar(name);
  if (var.isNull()) {
    if (!required) {
      for (size_t ii = 0; ii < nVals; ii++) {
        vals.push_back(Radx::missingMetaDouble);
      }
      clearErrStr();
      return var;
    } else {
      NcxxErrStr err;
      err.addErrStr("ERROR - Cf2RadxFile::_read1DVar(double *)");
      err.addErrStr("  var missing, name: ", name);
      err.addErrStr("  group: ", group.getName());
      throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
    }
  }

  // load up data

  RadxArray<double> data_;
  double *data = data_.alloc(nVals);
  try {
    var.getVal(data);
    for (size_t ii = 0; ii < nVals; ii++) {
      vals.push_back(data[ii]);
    }
  } catch (NcxxException& e) {
    if (!required) {
      for (size_t ii = 0; ii < nVals; ii++) {
        vals.push_back(Radx::missingMetaDouble);
      }
      clearErrStr();
      return var;
    } else {
      NcxxErrStr err;
      err.addErrStr("ERROR - Cf2RadxFile::_read1DVar(double *)");
      err.addErrStr("  Cannot read variable: ", name);
      err.addErrStr("  exception: ", e.what());
      throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
    }
  }

  return var;

}

///////////////////////////////////
// read a sweep variable - integer
// throws exception on error

NcxxVar Cf2RadxFile::_read1DVar(NcxxGroup &group,
                                NcxxDim &dim,
                                const string &name,
                                vector<int> &vals,
                                bool required)

{

  vals.clear();

  // get var

  size_t nVals = dim.getSize();
  NcxxVar var = group.getVar(name);
  if (var.isNull()) {
    if (!required) {
      for (size_t ii = 0; ii < nVals; ii++) {
        vals.push_back(Radx::missingMetaInt);
      }
      clearErrStr();
      return var;
    } else {
      NcxxErrStr err;
      err.addErrStr("ERROR - Cf2RadxFile::_read1DVar(int *)");
      err.addErrStr("  var missing, name: ", name);
      err.addErrStr("  group: ", group.getName());
      throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
    }
  }

  // load up data

  RadxArray<int> data_;
  int *data = data_.alloc(nVals);
  try {
    var.getVal(data);
    for (size_t ii = 0; ii < nVals; ii++) {
      vals.push_back(data[ii]);
    }
  } catch (NcxxException& e) {
    if (!required) {
      for (size_t ii = 0; ii < nVals; ii++) {
        vals.push_back(Radx::missingMetaInt);
      }
      clearErrStr();
      return var;
    } else {
      NcxxErrStr err;
      err.addErrStr("ERROR - Cf2RadxFile::_read1DVar");
      err.addErrStr("  Cannot read variable: ", name);
      err.addErrStr("  exception: ", e.what());
      throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
    }
  }

  return var;

}

///////////////////////////////////
// read a sweep variable - string

NcxxVar Cf2RadxFile::_read1DVar(NcxxGroup &group,
                                NcxxDim &dim,
                                const string &name,
                                vector<string> &vals,
                                bool required)

{
  
  vals.clear();

  // get var

  size_t nVals = dim.getSize();
  NcxxVar var = group.getVar(name);
  if (var.isNull()) {
    if (!required) {
      for (size_t ii = 0; ii < nVals; ii++) {
        vals.push_back("");
      }
      clearErrStr();
      return var;
    } else {
      NcxxErrStr err;
      err.addErrStr("ERROR - Cf2RadxFile::_read1DVar(int *)");
      err.addErrStr("  var missing, name: ", name);
      err.addErrStr("  group: ", group.getName());
      throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
    }
  }

  // load up data

  RadxArray<char *> data_;
  char **data = data_.alloc(nVals);
  try {
    var.getVal(data);
    for (size_t ii = 0; ii < nVals; ii++) {
      vals.push_back(data[ii]);
    }
  } catch (NcxxException& e) {
    if (!required) {
      for (size_t ii = 0; ii < nVals; ii++) {
        vals.push_back("");
      }
      clearErrStr();
      return var;
    } else {
      NcxxErrStr err;
      err.addErrStr("ERROR - Cf2RadxFile::_read1DVar");
      err.addErrStr("  Cannot read variable: ", name);
      err.addErrStr("  exception: ", e.what());
      throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
    }
  }

  return var;

}

