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
// RawFile.cc
//
// UW Raw HSRL NetCDF data
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2017
//
///////////////////////////////////////////////////////////////

#include "RawFile.hh"
#include "MonField.hh"
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxCfactors.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
using namespace std;

//////////////
// Constructor

RawFile::RawFile(const Params &params) :
        _params(params)
  
{

  clear();

}

/////////////
// destructor

RawFile::~RawFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void RawFile::clear()
  
{

  clearErrStr();
  
  _file.close();

  _timeDim = NULL;
  _timeVecDim = NULL;
  _binCountDim = NULL;

  _nTimesInFile = 0;
  _timeVecSize = 0;
  _nBinsInFile = 0;

  _machType.clear();
  _hostName.clear();
  _userName.clear();
  _gitCommit.clear();
  _hsrlVersion = -9999;
  _dataAdded.clear();
  _sourceSoftware.clear();

  _timeVar = NULL;
  _dataTimes.clear();
  _dTimes.clear();
  
  _telescopeLockedVar = NULL;
  _telescopeDirectionVar = NULL;

  _latitudeVar = NULL;
  _longitudeVar = NULL;
  _altitudeVar = NULL;
  _headingVar = NULL;
  _gndSpeedVar = NULL;
  _vertVelVar = NULL;
  _pitchVar = NULL;
  _rollVar = NULL;

  _telescopeLocked.clear();
  _telescopeDirection.clear();

  _latitude.clear();
  _longitude.clear();
  _altitude.clear();
  _heading.clear();
  _gndSpeed.clear();
  _vertVel.clear();
  _pitch.clear();
  _roll.clear();

}

////////////////////////////////////////////////////////////
// Check if this is a CfRadial file
// Returns true on success, false on failure

bool RawFile::isRawHsrlFile(const string &path)
  
{

  clear();
  
  // open file

  if (_file.openRead(path)) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "DEBUG - not Raw HSRL file" << endl;
      cerr << _file.getErrStr() << endl;
    }
    return false;
  }

  // read dimensions
  
  if (_readDimensions()) {
    _file.close();
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "DEBUG - not Raw HSRL file" << endl;
      cerr << _errStr << endl;
    }
    return false;
  }

  // file has the correct dimensions, so it is a CfRadial file

  _file.close();
  return true;

}

////////////////////////////////////////////////
// get the date and time from a dorade file path
// returns 0 on success, -1 on failure

int RawFile::getTimeFromPath(const string &path, RadxTime &rtime)

{

  RadxPath rpath(path);
  const string &fileName = rpath.getFile();
  
  // find first digit in entry name - if no digits, return now

  const char *start = NULL;
  for (size_t ii = 0; ii < fileName.size(); ii++) {
    if (isdigit(fileName[ii])) {
      start = fileName.c_str() + ii;
      break;
    }
  }
  if (!start) return -1;
  const char *end = start + strlen(start);
  
  // iteratively try getting the date and time from the string
  // moving along by one character at a time
  
  while (start < end - 6) {
    int year, month, day, hour, min, sec;
    char sep;
    if (sscanf(start, "%4d%2d%2d%1c%2d%2d%2d",
               &year, &month, &day, &sep, &hour, &min, &sec) == 7) {
      if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
        return -1;
      }
      if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
        return -1;
      }
      rtime.set(year, month, day, hour, min, sec);
      return 0;
    }
    start++;
  }
  
  return -1;
  
}

////////////////////////////////////////////////////////////
/// get start and end times for data in file
/// returns 0 on success, -1 on failure

int RawFile::getStartAndEndTimes(const string &filePath,
                                 time_t &dataStartTime,
                                 time_t &dataEndTime)

{
  
  // open file
  
  if (_file.openRead(filePath)) {
    cerr << "ERROR - RawFile::getStartAndEndTimes" << endl;
    cerr << _file.getErrStr() << endl;
    return -1;
  }
  
  // read dimensions
  
  if (_readDimensions()) {
    cerr << "ERROR - RawFile::getStartAndEndTimes" << endl;
    cerr << _errStr << endl;
    return -1;
  }
  
  // read time variable
  
  if (_readTimes()) {
    cerr << "ERROR - RawFile::getStartAndEndTimes" << endl;
    cerr << _errStr << endl;
    return -1;
  }
  if (_nTimesInFile < 1) {
    cerr << "ERROR - RawFile::getStartAndEndTimes" << endl;
    cerr << "  No times found in file: " << filePath << endl;
    return -1;
  }
  
  // close file
  
  _file.close();

  // set times
  
  dataStartTime = _dataTimes[0].utime();
  dataEndTime = _dataTimes[_nTimesInFile - 1].utime();

  return 0;

}

////////////////////////////////////////////////////////////
// Read in data from specified path
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int RawFile::readFromPath(const string &path)
  
{
  
  if (_params.debug) {
    cerr << "Reading file: " << path << endl;
  }

  string errStr("ERROR - RawFile::readFromPath");

  // clear tmp rays
  
  _nTimesInFile = 0;
  _nBinsInFile = 0;

  // open file

  if (_file.openRead(path)) {
    _addErrStr(_file.getErrStr());
    return -1;
  }
  
  // read dimensions
  
  if (_readDimensions()) {
    _addErrStr(errStr);
    return -1;
  }

  // read global attributes
  
  if (_readGlobalAttributes()) {
    _addErrStr(errStr);
    return -1;
  }

  // read time variable
  
  if (_readTimes()) {
    _addErrStr(errStr);
    return -1;
  }
  
  // close file
  
  _file.close();

  // clean up

  _dataTimes.clear();
  _dTimes.clear();

  return 0;

}

///////////////////////////////////
// read in the dimensions

int RawFile::_readDimensions()

{

  // read required dimensions
  
  if (_file.readDim("time", _timeDim) == 0) {
    _nTimesInFile = _timeDim->size();
  } else {
    _addErrStr("ERROR - RawFile::_readDimensions()");
    _addErrStr("  Cannot find 'time' dimension");
    return -1;
  }
  

  if (_file.readDim("time_vector", _timeVecDim) == 0) {
    _timeVecSize = _timeVecDim->size();
  } else {
    _addErrStr("ERROR - RawFile::_readDimensions()");
    _addErrStr("  Cannot find 'time_vector' dimension");
    return -1;
  }
  
  if (_file.readDim("bincount", _binCountDim) == 0) {
    _nBinsInFile = _binCountDim->size();
  } else {
    _addErrStr("ERROR - RawFile::_readDimensions()");
    _addErrStr("  Cannot find 'bincount' dimension");
    return -1;
  }

  _nPoints = _nTimesInFile * _nBinsInFile;
  _nBinsPerGate = 1;
  _nGates = _nBinsInFile / _nBinsPerGate;

  if (_params.debug) {
    cerr << "_nTimesInFile: " << _nTimesInFile << endl;
  }
  
  return 0;

}

///////////////////////////////////
// read the global attributes

int RawFile::_readGlobalAttributes()

{

  _machType.clear();
  _hostName.clear();
  _userName.clear();
  _gitCommit.clear();
  _hsrlVersion = -9999;
  _sourceSoftware.clear();

  for (int ii = 0; ii < _file.getNc3File()->num_atts(); ii++) {
    
    Nc3Att* att = _file.getNc3File()->get_att(ii);
    
    if (att == NULL) {
      continue;
    }

    if (!strcmp(att->name(), "NCUTIL_Machtype")) {
      _machType = Nc3xFile::asString(att);
    }

    if (!strcmp(att->name(), "NCUTIL_Hostname")) {
      _hostName = Nc3xFile::asString(att);
    }

    if (!strcmp(att->name(), "NCUTIL_Username")) {
      _userName = Nc3xFile::asString(att);
    }

    if (!strcmp(att->name(), "NCUTIL_HSRL_GIT_COMMIT")) {
      _gitCommit = Nc3xFile::asString(att);
    }

    if (!strcmp(att->name(), "DATA_HSRLVersion")) {
      _hsrlVersion = att->as_int(0);
    }

    if (!strcmp(att->name(), "DATA_Added")) {
      _dataAdded = Nc3xFile::asString(att);
    }

    if (!strcmp(att->name(), "DATA_SourceSoftware")) {
      _sourceSoftware = Nc3xFile::asString(att);
    }

    // Caller must delete attribute

    delete att;
    
  } // ii

  if (_params.debug >= Params::DEBUG_VERBOSE) {

    cerr << "Gobal attr machType: " << _machType << endl;
    cerr << "Gobal attr hostName: " << _hostName << endl;
    cerr << "Gobal attr userName: " << _userName << endl;
    cerr << "Gobal attr gitCommit: " << _gitCommit << endl;
    cerr << "Gobal attr hsrlVersion: " << _hsrlVersion << endl;
    cerr << "Gobal attr sourceSoftware: " << _sourceSoftware << endl;
    
  }

  return 0;

}

///////////////////////////////////
// read the times

int RawFile::_readTimes()

{

  _dataTimes.clear();
  _dTimes.clear();

  // read the time variable

  _timeVar = _file.getNc3File()->get_var("DATA_time");
  if (_timeVar == NULL) {
    _addErrStr("ERROR - RawFile::_readTimes");
    _addErrStr("  Cannot find DATA_time variable");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  if (_timeVar->num_dims() < 2) {
    _addErrStr("ERROR - RawFile::_readTimes");
    _addErrStr("  time variable has no dimensions");
    return -1;
  }
  Nc3Dim *timeDim = _timeVar->get_dim(0);
  Nc3Dim *timeVecDim = _timeVar->get_dim(1);
  if (timeDim != _timeDim || timeVecDim != _timeVecDim) {
    _addErrStr("ERROR - RawFile::_readTimes");
    _addErrStr("  DATA_time has incorrect dimensions");
    _addErrStr("  Should be (time, time_vector)");
    return -1;
  }

  // read in time 2D array

  short *timeData = new short[_nTimesInFile * _timeVecSize];
  if (!_timeVar->get(timeData, _nTimesInFile, _timeVecSize)) {
    _addErrStr("ERROR - RawFile::_readTimes");
    _addErrStr("  Cannot read DATA_time 2D array");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    delete[] timeData;
    return -1;
  }

  for (size_t ii = 0; ii < _nTimesInFile; ii++) {
    short *tdat = timeData + ii * _timeVecSize;
    int year = tdat[0];
    int month = tdat[1];
    int day = tdat[2];
    int hour = tdat[3];
    int min = tdat[4];
    int sec = tdat[5];
    int milliSec = tdat[6];
    int microSec = tdat[7];
    double fracSec = milliSec / 1.0e3 + microSec / 1.0e6;
    RadxTime thisTime(year, month, day, hour, min, sec, fracSec);
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "  Ray time: " << thisTime.asString(6) << endl;
    }
    _dataTimes.push_back(thisTime);
    _dTimes.push_back(thisTime.asDouble());
  }
  
  delete[] timeData;
  return 0;

}


///////////////////////////////////////////////
// add labelled integer value to error string,
// with optional following carriage return.

void RawFile::_addErrInt(string label, int iarg, bool cr)
{
  Radx::addErrInt(_errStr, label, iarg, cr);
}

///////////////////////////////////////////////
// add labelled double value to error string,
// with optional following carriage return.
// Default format is %g.

void RawFile::_addErrDbl(string label, double darg,
                          string format, bool cr)
  
{
  Radx::addErrDbl(_errStr, label, darg, format, cr);
}

////////////////////////////////////////
// add labelled string to error string
// with optional following carriage return.

void RawFile::_addErrStr(string label, string strarg, bool cr)

{
  Radx::addErrStr(_errStr, label, strarg, cr);
}

#ifdef JUNK

///////////////////////////////////
// read in ray variables

int RawFile::_readRayVariables()

{

  _clearRayVariables();
  int iret = 0;

  _readRayVar(_telescopeLockedVar, "TelescopeLocked", _telescopeLocked);
  if (_telescopeLocked.size() < _nTimesInFile) {
    _addErrStr("ERROR - TelescopeLocked variable required");
    iret = -1;
  }

  _readRayVar(_telescopeDirectionVar, "TelescopeDirection", _telescopeDirection);
  if (_telescopeDirection.size() < _nTimesInFile) {
    _addErrStr("ERROR - TelescopeDirection variable required");
    iret = -1;
  }

  _readRayVar(_latitudeVar, "iwg1_Lat", _latitude);
  if (_latitude.size() < _nTimesInFile) {
    _addErrStr("ERROR - iwg1_Lat variable required");
    iret = -1;
  }

  _readRayVar(_longitudeVar, "iwg1_Lon", _longitude);
  if (_longitude.size() < _nTimesInFile) {
    _addErrStr("ERROR - iwg1_Lon variable required");
    iret = -1;
  }

  _readRayVar(_altitudeVar, "iwg1_GPS_MSL_Alt", _altitude);
  if (_altitude.size() < _nTimesInFile) {
    _addErrStr("ERROR - iwg1_GPS_MSL_Alt variable required");
    iret = -1;
  }

  _readRayVar(_altitudeVar, "iwg1_True_Hdg", _heading);
  if (_heading.size() < _nTimesInFile) {
    _addErrStr("ERROR - iwg1_True_Hdg variable required");
    iret = -1;
  }

  _readRayVar(_gndSpeedVar, "iwg1_Grnd_Spd", _gndSpeed, false);
  _readRayVar(_vertVelVar, "iwg1_Vert_Velocity", _vertVel, false);

  _readRayVar(_pitchVar, "iwg1_Pitch", _pitch);
  if (_pitch.size() < _nTimesInFile) {
    _addErrStr("ERROR - iwg1_Pitch variable required");
    iret = -1;
  }

  _readRayVar(_rollVar, "iwg1_Roll", _roll);
  if (_roll.size() < _nTimesInFile) {
    _addErrStr("ERROR - iwg1_Roll variable required");
    iret = -1;
  }

  _readRayVar(_pollAngleVar, "polarization", _polAngle);
  _readRayVar(_totalEnergyVar, "total_energy", _totalEnergy);

  if (iret) {
    _addErrStr("ERROR - RawFile::_readRayVariables");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// read a ray variable - double
// side effects: set var, vals

int RawFile::_readRayVar(Nc3Var* &var, const string &name,
                         vector<double> &vals, bool required)
  
{

  vals.clear();

  // get var

  var = _getRayVar(name, required);
  if (var == NULL) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(Radx::missingMetaDouble);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - RawFile::_readRayVar");
      return -1;
    }
  }

  // load up data

  double *data = new double[_nTimesInFile];
  double *dd = data;
  int iret = 0;
  if (var->get(data, _nTimesInFile)) {
    for (size_t ii = 0; ii < _nTimesInFile; ii++, dd++) {
      vals.push_back(*dd);
    }
  } else {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(Radx::missingMetaDouble);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - RawFile::_readRayVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      iret = -1;
    }
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a ray variable - float
// side effects: set var, vals

int RawFile::_readRayVar(Nc3Var* &var, const string &name,
                         vector<float> &vals, bool required)
  
{

  vals.clear();

  // get var

  var = _getRayVar(name, required);
  if (var == NULL) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(Radx::missingMetaFloat);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - RawFile::_readRayVar");
      return -1;
    }
  }

  // load up data

  float *data = new float[_nTimesInFile];
  float *dd = data;
  int iret = 0;
  if (var->get(data, _nTimesInFile)) {
    for (size_t ii = 0; ii < _nTimesInFile; ii++, dd++) {
      vals.push_back(*dd);
    }
  } else {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(Radx::missingMetaFloat);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - RawFile::_readRayVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      iret = -1;
    }
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a ray variable - double
// side effects: set vals only

int RawFile::_readRayVar(const string &name,
                         vector<double> &vals, bool required)
{
  Nc3Var *var;
  return _readRayVar(var, name, vals, required);
}

///////////////////////////////////
// read a ray variable - integer
// side effects: set var, vals

int RawFile::_readRayVar(Nc3Var* &var, const string &name,
                         vector<int> &vals, bool required)
  
{

  vals.clear();

  // get var
  
  var = _getRayVar(name, required);
  if (var == NULL) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(Radx::missingMetaInt);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - RawFile::_readRayVar");
      return -1;
    }
  }

  // load up data

  int *data = new int[_nTimesInFile];
  int *dd = data;
  int iret = 0;
  if (var->get(data, _nTimesInFile)) {
    for (size_t ii = 0; ii < _nTimesInFile; ii++, dd++) {
      vals.push_back(*dd);
    }
  } else {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(Radx::missingMetaInt);
      }
      clearErrStr();
    } else {
      _addErrStr("ERROR - RawFile::_readRayVar");
      _addErrStr("  Cannot read variable: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      iret = -1;
    }
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a ray variable - integer
// side effects: set vals only

int RawFile::_readRayVar(const string &name,
                         vector<int> &vals, bool required)
{
  Nc3Var *var;
  return _readRayVar(var, name, vals, required);
}

///////////////////////////////////////////
// read a ray variable - boolean
// side effects: set var, vals

int RawFile::_readRayVar(Nc3Var* &var, const string &name,
                         vector<bool> &vals, bool required)
  
{
  
  vals.clear();
  
  // get var
  
  var = _getRayVar(name, false);
  if (var == NULL) {
    if (!required) {
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        vals.push_back(false);
      }
      clearErrStr();
      return 0;
    } else {
      _addErrStr("ERROR - RawFile::_readRayVar");
      return -1;
    }
  }

  // load up data
  
  int *data = new int[_nTimesInFile];
  int *dd = data;
  int iret = 0;
  if (var->get(data, _nTimesInFile)) {
    for (size_t ii = 0; ii < _nTimesInFile; ii++, dd++) {
      if (*dd == 0) {
        vals.push_back(false);
      } else {
        vals.push_back(true);
      }
    }
  } else {
    for (size_t ii = 0; ii < _nTimesInFile; ii++) {
      vals.push_back(false);
    }
    clearErrStr();
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a ray variable - boolean
// side effects: set vals only

int RawFile::_readRayVar(const string &name,
                         vector<bool> &vals, bool required)
{
  Nc3Var *var;
  return _readRayVar(var, name, vals, required);
}

///////////////////////////////////
// get a ray variable by name
// returns NULL on failure

Nc3Var* RawFile::_getRayVar(const string &name, bool required)

{

  // get var
  
  Nc3Var *var = _file.getNc3File()->get_var(name.c_str());
  if (var == NULL) {
    if (required) {
      _addErrStr("ERROR - RawFile::_getRayVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
    }
    return NULL;
  }

  // check time dimension
  
  if (var->num_dims() < 1) {
    if (required) {
      _addErrStr("ERROR - RawFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no dimensions");
    }
    return NULL;
  }
  Nc3Dim *timeDim = var->get_dim(0);
  if (timeDim != _timeDim) {
    if (required) {
      _addErrStr("ERROR - RawFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has incorrect dimension, dim name: ", 
                 timeDim->name());
      _addErrStr("  should be: ", "time");
    }
    return NULL;
  }

  return var;

}

#endif
