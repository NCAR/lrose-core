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
// Feb 2017
//
///////////////////////////////////////////////////////////////

#include "RawFile.hh"
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
  _gndSpeedVar = NULL;
  _vertVelVar = NULL;
  _pitchVar = NULL;
  _rollVar = NULL;

  _telescopeLocked.clear();
  _telescopeDirection.clear();
  _rotation.clear();
  _tilt.clear();

  _latitude.clear();
  _longitude.clear();
  _altitude.clear();
  _gndSpeed.clear();
  _vertVel.clear();
  _pitch.clear();
  _roll.clear();

  _instrumentType = Radx::INSTRUMENT_TYPE_LIDAR;
  _platformType = Radx::PLATFORM_TYPE_AIRCRAFT;
  _primaryAxis = Radx::PRIMARY_AXIS_Y_PRIME;

  _rawGateSpacingM = 3.75;
  _gateSpacingKm = _rawGateSpacingM * cos(4.0 * Radx::DegToRad);
  _startRangeKm = _gateSpacingKm / 2.0;
  if (_params.combine_gates_on_read) {
    _gateSpacingKm *= _params.n_gates_to_combine;
    _startRangeKm = _gateSpacingKm / 2.0;
  }

  _rays.clear();
  
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
    if (sscanf(start, "%4d%2d%2d_%2d%2d%2d",
               &year, &month, &day, &hour, &min, &sec) == 6) {
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
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int RawFile::readFromPath(const string &path,
                          RadxVol &vol)
  
{
  
  if (_params.debug) {
    cerr << "Reading file: " << path << endl;
  }

  string errStr("ERROR - RawFile::readFromPath");

  // clear tmp rays
  
  _nTimesInFile = 0;
  _nBinsInFile = 0;
  _rays.clear();

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
  
  // read in ray metadata variables
  
  if (_readRayVariables()) {
    _addErrStr(errStr);
    return -1;
  }
  
  // create the rays, filling out the metadata
  
  if (_createRays(path)) {
    _addErrStr(errStr);
    return -1;
  }
  
  // load the data into the read volume

  // _loadReadVolume();

  // close file

  _file.close();

#ifdef JUNK
  
  // check if georeferences and/or corrections are active

  _checkGeorefsActiveOnRead();
  _checkCorrectionsActiveOnRead();

  // for first path in aggregated list, read in non-varying values

  if (pathNum == 0) {

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

  // read in georef variables

  if (_georefsActive) {
    if (_readGeorefVariables()) {
      _addErrStr(errStr);
      return -1;
    }
  }

  // set the ray pointing angles

  _setPointingAngles();

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

#endif

  // clean up

  _rays.clear();
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

  for (int ii = 0; ii < _file.getNcFile()->num_atts(); ii++) {
    
    NcAtt* att = _file.getNcFile()->get_att(ii);
    
    if (att == NULL) {
      continue;
    }

    if (!strcmp(att->name(), "NCUTIL_Machtype")) {
      _machType = NetcdfClassic::asString(att);
    }

    if (!strcmp(att->name(), "NCUTIL_Hostname")) {
      _hostName = NetcdfClassic::asString(att);
    }

    if (!strcmp(att->name(), "NCUTIL_Username")) {
      _userName = NetcdfClassic::asString(att);
    }

    if (!strcmp(att->name(), "NCUTIL_HSRL_GIT_COMMIT")) {
      _gitCommit = NetcdfClassic::asString(att);
    }

    if (!strcmp(att->name(), "DATA_HSRLVersion")) {
      _hsrlVersion = att->as_int(0);
    }

    if (!strcmp(att->name(), "DATA_Added")) {
      _dataAdded = NetcdfClassic::asString(att);
    }

    if (!strcmp(att->name(), "DATA_SourceSoftware")) {
      _sourceSoftware = NetcdfClassic::asString(att);
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

  _timeVar = _file.getNcFile()->get_var("DATA_time");
  if (_timeVar == NULL) {
    _addErrStr("ERROR - RawFile::_readTimes");
    _addErrStr("  Cannot find DATA_time variable");
    _addErrStr(_file.getNcError()->get_errmsg());
    return -1;
  }
  if (_timeVar->num_dims() < 2) {
    _addErrStr("ERROR - RawFile::_readTimes");
    _addErrStr("  time variable has no dimensions");
    return -1;
  }
  NcDim *timeDim = _timeVar->get_dim(0);
  NcDim *timeVecDim = _timeVar->get_dim(1);
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
    _addErrStr(_file.getNcError()->get_errmsg());
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

  return 0;

}

///////////////////////////////////
// clear the ray variables

void RawFile::_clearRayVariables()

{

  _telescopeLocked.clear();
  _telescopeDirection.clear();
  _rotation.clear();
  _tilt.clear();
  _latitude.clear();
  _longitude.clear();
  _altitude.clear();
  _gndSpeed.clear();
  _vertVel.clear();
  _pitch.clear();
  _roll.clear();

}

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

  if (iret) {
    _addErrStr("ERROR - RawFile::_readRayVariables");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// read a ray variable - double
// side effects: set var, vals

int RawFile::_readRayVar(NcVar* &var, const string &name,
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
      _addErrStr(_file.getNcError()->get_errmsg());
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
  NcVar *var;
  return _readRayVar(var, name, vals, required);
}

///////////////////////////////////
// read a ray variable - integer
// side effects: set var, vals

int RawFile::_readRayVar(NcVar* &var, const string &name,
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
      _addErrStr(_file.getNcError()->get_errmsg());
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
  NcVar *var;
  return _readRayVar(var, name, vals, required);
}

///////////////////////////////////////////
// read a ray variable - boolean
// side effects: set var, vals

int RawFile::_readRayVar(NcVar* &var, const string &name,
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
  NcVar *var;
  return _readRayVar(var, name, vals, required);
}

///////////////////////////////////
// get a ray variable by name
// returns NULL on failure

NcVar* RawFile::_getRayVar(const string &name, bool required)

{

  // get var
  
  NcVar *var = _file.getNcFile()->get_var(name.c_str());
  if (var == NULL) {
    if (required) {
      _addErrStr("ERROR - RawFile::_getRayVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr(_file.getNcError()->get_errmsg());
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
  NcDim *timeDim = var->get_dim(0);
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

///////////////////////////////////
// create the rays to be read in
// and set meta data

int RawFile::_createRays(const string &path)

{

  // compile a list of the rays to be read in
  
  _rays.clear();

  for (size_t ii = 0; ii < _nTimesInFile; ii++) {

    // new ray

    RadxRay *ray = new RadxRay;
    ray->setRangeGeom(_startRangeKm, _gateSpacingKm);
    ray->setTime(_dataTimes[ii]);
    
    // sweep info

    ray->setSweepNumber(0);
    ray->setSweepMode(Radx::SWEEP_MODE_POINTING);
    ray->setPrtMode(Radx::PRT_MODE_FIXED);
    ray->setTargetScanRateDegPerSec(0.0);
    ray->setIsIndexed(false);

    // georeference
    
    RadxGeoref geo;

    if (_telescopeDirection[ii] == 1) {

      // pointing up

      geo.setRotation(-4.0);
      geo.setTilt(0.0);
      ray->setAzimuthDeg(0.0);
      ray->setElevationDeg(90.0);
      ray->setFixedAngleDeg(90.0);
      
    } else {

      // pointing down

      geo.setRotation(184.0);
      geo.setTilt(0.0);
      ray->setAzimuthDeg(0.0);
      ray->setElevationDeg(-90.0);
      ray->setFixedAngleDeg(-90.0);
      
    }

    geo.setRoll(_roll[ii]);
    geo.setPitch(_pitch[ii]);

    geo.setLatitude(_latitude[ii]);
    geo.setLongitude(_longitude[ii]);
    geo.setAltitudeKmMsl(_altitude[ii]);

    geo.setVertVelocity(_vertVel[ii]);
    
    ray->setGeoref(geo);
    
    // add to ray vector
    
    _rays.push_back(ray);

  } // ii

  return 0;

}

/////////////////////////////////////////////////////////
// load up the read volume with the data from this object

void RawFile::_loadReadVolume()
  
{

  _readVol.clear();

  _readVol.setOrigFormat("HSRL-RAW");
  _readVol.setVolumeNumber(-9999);
  _readVol.setInstrumentType(_instrumentType);
  _readVol.setInstrumentName("HSRL");
  _readVol.setSiteName("GV");
  _readVol.setPlatformType(_platformType);
  _readVol.setPrimaryAxis(_primaryAxis);
  for (int ii = 0; ii < (int) _rays.size(); ii++) {
    _rays[ii]->setVolumeNumber(-9999);
  }
  
  _readVol.addFrequencyHz(Radx::LIGHT_SPEED / 538.0e-6);
  _readVol.addFrequencyHz(Radx::LIGHT_SPEED / 1064.0e-6);
  
  _readVol.setLidarConstant(-9999.0);
  _readVol.setLidarPulseEnergyJ(-9999.0);
  _readVol.setLidarPeakPowerW(-9999.0);
  _readVol.setLidarApertureDiamCm(-9999.0);
  _readVol.setLidarApertureEfficiency(-9999.0);
  _readVol.setLidarFieldOfViewMrad(-9999.0);
  _readVol.setLidarBeamDivergenceMrad(-9999.0);

  _readVol.setTitle("NCAR HSRL");
  _readVol.setSource("HSRL realtime software");
  _readVol.setHistory("Converted from RAW NetCDF files");
  _readVol.setInstitution("NCAR");
  _readVol.setReferences("University of Wisconsin");
  _readVol.setComment("");
  _readVol.setDriver("Hsrl2Radx");
  _readVol.setCreated(_dataAdded);
  _readVol.setStatusXml("");
  
  _readVol.setScanName("Vert");
  _readVol.setScanId(0);

  if (_latitude.size() > 0) {
    for (size_t ii = 0; ii < _latitude.size(); ii++) {
      if (_latitude[ii] > -9990) {
        _readVol.setLatitudeDeg(_latitude[ii]);
        break;
      }
    }
  }
  if (_longitude.size() > 0) {
    for (size_t ii = 0; ii < _longitude.size(); ii++) {
      if (_longitude[ii] > -9990) {
        _readVol.setLongitudeDeg(_longitude[ii]);
        break;
      }
    }
  }
  if (_altitude.size() > 0) {
    for (size_t ii = 0; ii < _altitude.size(); ii++) {
      if (_altitude[ii] > -9990) {
        _readVol.setAltitudeKm(_altitude[ii] / 1000.0);
        break;
      }
    }
  }

  _readVol.setRangeGeom(_startRangeKm, _gateSpacingKm);

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    _readVol.addRay(_rays[ii]);
  }

  // memory responsibility has passed to the volume object, so clear
  // the vectors without deleting the objects to which they point

  _rays.clear();
  // _fields.clear();

  // apply goeref info

  _readVol.applyGeorefs();

  // load the sweep information from the rays

  _readVol.loadSweepInfoFromRays();
  
  // load the volume information from the rays

  _readVol.loadVolumeInfoFromRays();
  
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

void RawFile::_clearRays()
{
  for (int ii = 0; ii < (int) _rays.size(); ii++) {
    delete _rays[ii];
  }
  _rays.clear();
}

#ifdef JUNK

///////////////////////////////////
// read the scalar variables

int RawFile::_readScalarVariables()

{
  
  int iret = 0;

  iret |= _file.readIntVar(_volumeNumberVar, VOLUME_NUMBER, 
                           _volumeNumber, Radx::missingMetaInt);

  string pstring;
  _instrumentType = Radx::INSTRUMENT_TYPE_RADAR;
  if (_file.readStringVar(_instrumentTypeVar, INSTRUMENT_TYPE, pstring) == 0) {
    _instrumentType = Radx::instrumentTypeFromStr(pstring);
  }

  if (_file.readStringVar(_platformTypeVar, PLATFORM_TYPE, pstring) == 0) {
    _platformType = Radx::platformTypeFromStr(pstring);
  }

  if (_file.readStringVar(_primaryAxisVar, PRIMARY_AXIS, pstring) == 0) {
    _primaryAxis = Radx::primaryAxisFromStr(pstring);
  }

  if (_file.getNcFile()->get_var(STATUS_XML) != NULL) {
    if (_file.readStringVar(_statusXmlVar, STATUS_XML, pstring) == 0) {
      _statusXml = pstring;
    }
  }

  NcVar *var;
  if (_file.readStringVar(var, TIME_COVERAGE_START, pstring) == 0) {
    RadxTime stime(pstring);
    _timeCoverageStart = stime.utime();
  }
  if (_file.readStringVar(var, TIME_COVERAGE_END, pstring) == 0) {
    RadxTime stime(pstring);
    _timeCoverageEnd = stime.utime();
  }

  if (_instrumentType == Radx::INSTRUMENT_TYPE_RADAR) {

    _file.readDoubleVar(_radarAntennaGainHVar,
                        RADAR_ANTENNA_GAIN_H, _radarAntennaGainDbH, false);
    _file.readDoubleVar(_radarAntennaGainVVar,
                        RADAR_ANTENNA_GAIN_V, _radarAntennaGainDbV, false);
    _file.readDoubleVar(_radarBeamWidthHVar,
                        RADAR_BEAM_WIDTH_H, _radarBeamWidthDegH, false);
    _file.readDoubleVar(_radarBeamWidthVVar,
                        RADAR_BEAM_WIDTH_V, _radarBeamWidthDegV, false);
    _file.readDoubleVar(_radarRxBandwidthVar,
                        RADAR_RX_BANDWIDTH, _radarRxBandwidthHz, false);
  } else {

    _file.readDoubleVar(_lidarConstantVar, 
                        LIDAR_CONSTANT, _lidarConstant, false);
    _file.readDoubleVar(_lidarPulseEnergyJVar, 
                        LIDAR_PULSE_ENERGY, _lidarPulseEnergyJ, false);
    _file.readDoubleVar(_lidarPeakPowerWVar, 
                        LIDAR_PEAK_POWER, _lidarPeakPowerW, false);
    _file.readDoubleVar(_lidarApertureDiamCmVar, 
                        LIDAR_APERTURE_DIAMETER,
                        _lidarApertureDiamCm, false);
    _file.readDoubleVar(_lidarApertureEfficiencyVar, 
                        LIDAR_APERTURE_EFFICIENCY,
                        _lidarApertureEfficiency, false);
    _file.readDoubleVar(_lidarFieldOfViewMradVar, 
                        LIDAR_FIELD_OF_VIEW,
                        _lidarFieldOfViewMrad, false);
    _file.readDoubleVar(_lidarBeamDivergenceMradVar, 
                        LIDAR_BEAM_DIVERGENCE,
                        _lidarBeamDivergenceMrad, false);
    
  }

  if (iret) {
    _addErrStr("ERROR - RawFile::_readScalarVariables");
    return -1;
  } else {
    return 0;
  }

}

///////////////////////////////////
// read the position variables

int RawFile::_readPositionVariables()

{

  // time

  _georefTimeVar = _file.getNcFile()->get_var(GEOREF_TIME);
  if (_georefTimeVar != NULL) {
    if (_georefTimeVar->num_vals() < 1) {
      _addErrStr("ERROR - RawFile::_readPositionVariables");
      _addErrStr("  Cannot read georef time");
      _addErrStr(_file.getNcError()->get_errmsg());
      return -1;
    }
    if (_latitudeVar->type() != ncDouble) {
      _addErrStr("ERROR - RawFile::_readPositionVariables");
      _addErrStr("  georef time is incorrect type: ", 
                 NetcdfClassic::ncTypeToStr(_georefTimeVar->type()));
      _addErrStr("  expecting type: double");
      return -1;
    }
  }

  // find latitude, longitude, altitude

  _latitudeVar = _file.getNcFile()->get_var(LATITUDE);
  if (_latitudeVar != NULL) {
    if (_latitudeVar->num_vals() < 1) {
      _addErrStr("ERROR - RawFile::_readPositionVariables");
      _addErrStr("  Cannot read latitude");
      _addErrStr(_file.getNcError()->get_errmsg());
      return -1;
    }
    if (_latitudeVar->type() != ncDouble) {
      _addErrStr("ERROR - RawFile::_readPositionVariables");
      _addErrStr("  latitude is incorrect type: ", 
                 NetcdfClassic::ncTypeToStr(_latitudeVar->type()));
      _addErrStr("  expecting type: double");
      return -1;
    }
  } else {
    cerr << "WARNING - RawFile::_readPositionVariables" << endl;
    cerr << "  No latitude variable" << endl;
    cerr << "  Setting latitude to 0" << endl;
  }

  _longitudeVar = _file.getNcFile()->get_var(LONGITUDE);
  if (_longitudeVar != NULL) {
    if (_longitudeVar->num_vals() < 1) {
      _addErrStr("ERROR - RawFile::_readPositionVariables");
      _addErrStr("  Cannot read longitude");
      _addErrStr(_file.getNcError()->get_errmsg());
      return -1;
    }
    if (_longitudeVar->type() != ncDouble) {
      _addErrStr("ERROR - RawFile::_readPositionVariables");
      _addErrStr("  longitude is incorrect type: ",
                 NetcdfClassic::ncTypeToStr(_longitudeVar->type()));
      _addErrStr("  expecting type: double");
      return -1;
    }
  } else {
    cerr << "WARNING - RawFile::_readPositionVariables" << endl;
    cerr << "  No longitude variable" << endl;
    cerr << "  Setting longitude to 0" << endl;
  }

  _altitudeVar = _file.getNcFile()->get_var(ALTITUDE);
  if (_altitudeVar != NULL) {
    if (_altitudeVar->num_vals() < 1) {
      _addErrStr("ERROR - RawFile::_readPositionVariables");
      _addErrStr("  Cannot read altitude");
      _addErrStr(_file.getNcError()->get_errmsg());
      return -1;
    }
    if (_altitudeVar->type() != ncDouble) {
      _addErrStr("ERROR - RawFile::_readPositionVariables");
      _addErrStr("  altitude is incorrect type: ",
                 NetcdfClassic::ncTypeToStr(_altitudeVar->type()));
      _addErrStr("  expecting type: double");
      return -1;
    }
  } else {
    cerr << "WARNING - RawFile::_readPositionVariables" << endl;
    cerr << "  No altitude variable" << endl;
    cerr << "  Setting altitude to 0" << endl;
  }

  _altitudeAglVar = _file.getNcFile()->get_var(ALTITUDE_AGL);
  if (_altitudeAglVar != NULL) {
    if (_altitudeAglVar->num_vals() < 1) {
      _addErrStr("WARNING - RawFile::_readPositionVariables");
      _addErrStr("  Bad variable - altitudeAgl");
      _addErrStr(_file.getNcError()->get_errmsg());
    }
    if (_altitudeAglVar->type() != ncDouble) {
      _addErrStr("WARNING - RawFile::_readPositionVariables");
      _addErrStr("  altitudeAgl is incorrect type: ",
                 NetcdfClassic::ncTypeToStr(_altitudeAglVar->type()));
      _addErrStr("  expecting type: double");
    }
  }

  // set variables

  if (_latitudeVar != NULL) {
    double *data = new double[_latitudeVar->num_vals()];
    if (_latitudeVar->get(data, _latitudeVar->num_vals())) {
      double *dd = data;
      for (int ii = 0; ii < _latitudeVar->num_vals(); ii++, dd++) {
        _latitude.push_back(*dd);
      }
    }
    delete[] data;
  } else {
    _latitude.push_back(0.0);
  }

  if (_longitudeVar != NULL) {
    double *data = new double[_longitudeVar->num_vals()];
    if (_longitudeVar->get(data, _longitudeVar->num_vals())) {
      double *dd = data;
      for (int ii = 0; ii < _longitudeVar->num_vals(); ii++, dd++) {
        _longitude.push_back(*dd);
      }
    }
    delete[] data;
  } else {
    _longitude.push_back(0.0);
  }

  if (_altitudeVar != NULL) {
    double *data = new double[_altitudeVar->num_vals()];
    if (_altitudeVar->get(data, _altitudeVar->num_vals())) {
      double *dd = data;
      for (int ii = 0; ii < _altitudeVar->num_vals(); ii++, dd++) {
        _altitude.push_back(*dd);
      }
    }
    delete[] data;
  } else {
    _altitude.push_back(0.0);
  }

  if (_altitudeAglVar != NULL) {
    double *data = new double[_altitudeAglVar->num_vals()];
    if (_altitudeAglVar->get(data, _altitudeAglVar->num_vals())) {
      double *dd = data;
      for (int ii = 0; ii < _altitudeAglVar->num_vals(); ii++, dd++) {
        _altitudeAgl.push_back(*dd);
      }
    }
    delete[] data;
  }

  return 0;

}

///////////////////////////////////
// clear the georeference vectors

void RawFile::_clearGeorefVariables()

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

/////////////////////////////////////////////
// set pointing angles from other variables

void RawFile::_setPointingAngles()

{

  // set elevation and azimuth
  
  _rayElevations.resize(_nTimesInFile);
  _rayAzimuths.resize(_nTimesInFile);
  _geoRotation.resize(_nTimesInFile);
  
  for (size_t ii = 0; ii < _nTimesInFile; ii++) {
    if (_telescopeDirection[ii] == _params.telescope_direction_is_up) {
      // _rayElevations[ii] = 94.0;
      // _rayElevations[ii] = 86.0;
      _rayElevations[ii] = 90.0;
      _geoRotation[ii] = 360;
    } else {
      // _rayElevations[ii] = -94.0;
      // _rayElevations[ii] = -86.0;
      _rayElevations[ii] = -90.0;
      _geoRotation[ii] = 180;
    }
    _rayAzimuths[ii] = 0.0;
    
    // if (_geoRoll.size() > ii && _telescopeRollAngleOffset.size() > ii) {
    //   if (isfinite(_geoRoll[ii]) && isfinite(_telescopeRollAngleOffset[ii])) {
    //     _rayElevations[ii] -= _geoRoll[ii];
    //     // _rayElevations[ii] = - _geoRoll[ii] + (90.0 - _telescopeRollAngleOffset[ii]);
    //     cerr << "111111 roll, offset, elev: " << ", "
    //          << _geoRoll[ii] << ", "
    //          << _telescopeRollAngleOffset[ii] << ", "
    //          << _rayElevations[ii] << endl;
    //   }
    // }

    // if (_params.debug >= Params::DEBUG_EXTRA) {
    //   cerr << "---> Telescope details - ii, offset, dirn, locked, roll, el, rot: "
    //        << ii << ", "
    //        << _telescopeRollAngleOffset[ii] << ", "
    //        << _telescopeDirection[ii] << ", "
    //        << _telescopeLocked[ii] << ", "
    //        << _geoRoll[ii] << ", "
    //        << _rayElevations[ii] << ", "
    //        << _geoRotation[ii] << endl;
    // }
  }

}

///////////////////////////////////
// read the calibration variables

int RawFile::_readCalibrationVariables()

{

  if (_calDim == NULL) {
    // no cal available
    return 0;
  }
  
  int iret = 0;
  for (int ii = 0; ii < _calDim->size(); ii++) {
    RadxRcalib *cal = new RadxRcalib;
    if (_readCal(*cal, ii)) {
      _addErrStr("ERROR - RawFile::_readCalibrationVariables");
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
  
int RawFile::_readCal(RadxRcalib &cal, int index)

{

  int iret = 0;
  double val;
  time_t ctime;

  iret |= _readCalTime(R_CALIB_TIME, 
                       _rCalTimeVar, index, ctime);
  cal.setCalibTime(ctime);

  iret |= _readCalVar(R_CALIB_PULSE_WIDTH, 
                      _rCalPulseWidthVar, index, val, true);
  cal.setPulseWidthUsec(val * 1.0e6);

  iret |= _readCalVar(R_CALIB_XMIT_POWER_H, 
                      _rCalXmitPowerHVar, index, val);
  cal.setXmitPowerDbmH(val);

  iret |= _readCalVar(R_CALIB_XMIT_POWER_V, 
                      _rCalXmitPowerVVar, index, val);
  cal.setXmitPowerDbmV(val);

  iret |= _readCalVar(R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_H,
                      _rCalTwoWayWaveguideLossHVar, index, val);
  cal.setTwoWayWaveguideLossDbH(val);

  iret |= _readCalVar(R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_V,
                      _rCalTwoWayWaveguideLossVVar, index, val);
  cal.setTwoWayWaveguideLossDbV(val);

  iret |= _readCalVar(R_CALIB_TWO_WAY_RADOME_LOSS_H,
                      _rCalTwoWayRadomeLossHVar, index, val);
  cal.setTwoWayRadomeLossDbH(val);

  iret |= _readCalVar(R_CALIB_TWO_WAY_RADOME_LOSS_V,
                      _rCalTwoWayRadomeLossVVar, index, val);
  cal.setTwoWayRadomeLossDbV(val);

  iret |= _readCalVar(R_CALIB_RECEIVER_MISMATCH_LOSS,
                      _rCalReceiverMismatchLossVar, index, val);
  cal.setReceiverMismatchLossDb(val);

  iret |= _readCalVar(R_CALIB_RADAR_CONSTANT_H, 
                      _rCalRadarConstHVar, index, val);
  cal.setRadarConstantH(val);

  iret |= _readCalVar(R_CALIB_RADAR_CONSTANT_V, 
                      _rCalRadarConstVVar, index, val);
  cal.setRadarConstantV(val);

  iret |= _readCalVar(R_CALIB_ANTENNA_GAIN_H, 
                      _rCalAntennaGainHVar, index, val);
  cal.setAntennaGainDbH(val);
  
  iret |= _readCalVar(R_CALIB_ANTENNA_GAIN_V, 
                      _rCalAntennaGainVVar, index, val);
  cal.setAntennaGainDbV(val);

  iret |= _readCalVar(R_CALIB_NOISE_HC, 
                      _rCalNoiseHcVar, index, val, true);
  cal.setNoiseDbmHc(val);

  iret |= _readCalVar(R_CALIB_NOISE_HX, 
                      _rCalNoiseHxVar, index, val);
  cal.setNoiseDbmHx(val);

  iret |= _readCalVar(R_CALIB_NOISE_VC, 
                      _rCalNoiseVcVar, index, val);
  cal.setNoiseDbmVc(val);

  iret |= _readCalVar(R_CALIB_NOISE_VX, 
                      _rCalNoiseVxVar, index, val);
  cal.setNoiseDbmVx(val);

  iret |= _readCalVar(R_CALIB_RECEIVER_GAIN_HC, 
                      _rCalReceiverGainHcVar, index, val, true);
  cal.setReceiverGainDbHc(val);

  iret |= _readCalVar(R_CALIB_RECEIVER_GAIN_HX, 
                      _rCalReceiverGainHxVar, index, val);
  cal.setReceiverGainDbHx(val);

  iret |= _readCalVar(R_CALIB_RECEIVER_GAIN_VC, 
                      _rCalReceiverGainVcVar, index, val);
  cal.setReceiverGainDbVc(val);

  iret |= _readCalVar(R_CALIB_RECEIVER_GAIN_VX, 
                      _rCalReceiverGainVxVar, index, val);
  cal.setReceiverGainDbVx(val);

  iret |= _readCalVar(R_CALIB_BASE_DBZ_1KM_HC, 
                      _rCalBaseDbz1kmHcVar, index, val);
  cal.setBaseDbz1kmHc(val);

  iret |= _readCalVar(R_CALIB_BASE_DBZ_1KM_HX, 
                      _rCalBaseDbz1kmHxVar, index, val);
  cal.setBaseDbz1kmHx(val);

  iret |= _readCalVar(R_CALIB_BASE_DBZ_1KM_VC, 
                      _rCalBaseDbz1kmVcVar, index, val);
  cal.setBaseDbz1kmVc(val);

  iret |= _readCalVar(R_CALIB_BASE_DBZ_1KM_VX, 
                      _rCalBaseDbz1kmVxVar, index, val);
  cal.setBaseDbz1kmVx(val);

  iret |= _readCalVar(R_CALIB_SUN_POWER_HC, 
                      _rCalSunPowerHcVar, index, val);
  cal.setSunPowerDbmHc(val);

  iret |= _readCalVar(R_CALIB_SUN_POWER_HX, 
                      _rCalSunPowerHxVar, index, val);
  cal.setSunPowerDbmHx(val);

  iret |= _readCalVar(R_CALIB_SUN_POWER_VC, 
                      _rCalSunPowerVcVar, index, val);
  cal.setSunPowerDbmVc(val);

  iret |= _readCalVar(R_CALIB_SUN_POWER_VX, 
                      _rCalSunPowerVxVar, index, val);
  cal.setSunPowerDbmVx(val);

  iret |= _readCalVar(R_CALIB_NOISE_SOURCE_POWER_H, 
                      _rCalNoiseSourcePowerHVar, index, val);
  cal.setNoiseSourcePowerDbmH(val);

  iret |= _readCalVar(R_CALIB_NOISE_SOURCE_POWER_V, 
                      _rCalNoiseSourcePowerVVar, index, val);
  cal.setNoiseSourcePowerDbmV(val);

  iret |= _readCalVar(R_CALIB_POWER_MEASURE_LOSS_H, 
                      _rCalPowerMeasLossHVar, index, val);
  cal.setPowerMeasLossDbH(val);

  iret |= _readCalVar(R_CALIB_POWER_MEASURE_LOSS_V, 
                      _rCalPowerMeasLossVVar, index, val);
  cal.setPowerMeasLossDbV(val);

  iret |= _readCalVar(R_CALIB_COUPLER_FORWARD_LOSS_H, 
                      _rCalCouplerForwardLossHVar, index, val);
  cal.setCouplerForwardLossDbH(val);

  iret |= _readCalVar(R_CALIB_COUPLER_FORWARD_LOSS_V, 
                      _rCalCouplerForwardLossVVar, index, val);
  cal.setCouplerForwardLossDbV(val);

  iret |= _readCalVar(R_CALIB_ZDR_CORRECTION, 
                      _rCalZdrCorrectionVar, index, val);
  cal.setZdrCorrectionDb(val);

  iret |= _readCalVar(R_CALIB_LDR_CORRECTION_H, 
                      _rCalLdrCorrectionHVar, index, val);
  cal.setLdrCorrectionDbH(val);

  iret |= _readCalVar(R_CALIB_LDR_CORRECTION_V, 
                      _rCalLdrCorrectionVVar, index, val);
  cal.setLdrCorrectionDbV(val);

  iret |= _readCalVar(R_CALIB_SYSTEM_PHIDP, 
                      _rCalSystemPhidpVar, index, val);
  cal.setSystemPhidpDeg(val);

  iret |= _readCalVar(R_CALIB_TEST_POWER_H, 
                      _rCalTestPowerHVar, index, val);
  cal.setTestPowerDbmH(val);

  iret |= _readCalVar(R_CALIB_TEST_POWER_V, 
                      _rCalTestPowerVVar, index, val);
  cal.setTestPowerDbmV(val);

  iret |= _readCalVar(R_CALIB_RECEIVER_SLOPE_HC, 
                      _rCalReceiverSlopeHcVar, index, val);
  cal.setReceiverSlopeDbHc(val);

  iret |= _readCalVar(R_CALIB_RECEIVER_SLOPE_HX, 
                      _rCalReceiverSlopeHxVar, index, val);
  cal.setReceiverSlopeDbHx(val);

  iret |= _readCalVar(R_CALIB_RECEIVER_SLOPE_VC, 
                      _rCalReceiverSlopeVcVar, index, val);
  cal.setReceiverSlopeDbVc(val);

  iret |= _readCalVar(R_CALIB_RECEIVER_SLOPE_VX, 
                      _rCalReceiverSlopeVxVar, index, val);
  cal.setReceiverSlopeDbVx(val);

  return iret;

}

////////////////////////////////////////////
// read the field variables

int RawFile::_readFieldVariables(bool metaOnly)

{

  // loop through the variables, adding data fields as appropriate
  
  for (int ivar = 0; ivar < _file.getNcFile()->num_vars(); ivar++) {
    
    NcVar* var = _file.getNcFile()->get_var(ivar);
    if (var == NULL) {
      continue;
    }
    
    int numDims = var->num_dims();
    if (_nGatesVary) {
      // variable number of gates per ray
      // we need fields with 1 dimension
      if (numDims != 1) {
        continue;
      }
      NcDim* nPointsDim = var->get_dim(0);
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
      NcDim* timeDim = var->get_dim(0);
      NcDim* rangeDim = var->get_dim(1);
      if (timeDim != _timeDim || rangeDim != _rangeDim) {
        continue;
      }
    }
    
    // check the type
    NcType ftype = var->type();
    if (ftype != ncDouble && ftype != ncFloat && ftype != ncInt &&
        ftype != ncShort && ftype != ncByte) {
      // not a valid type
      continue;
    }

    // check that we need this field

    string fieldName = var->name();
    if (!isFieldRequiredOnRead(fieldName)) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "DEBUG - RawFile::_readFieldVariables" << endl;
        cerr << "  -->> rejecting field: " << fieldName << endl;
      }
      continue;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "DEBUG - RawFile::_readFieldVariables" << endl;
      cerr << "  -->> adding field: " << fieldName << endl;
    }

    // set names, units, etc
    
    string name = var->name();

    string standardName;
    NcAtt *standardNameAtt = var->get_att(STANDARD_NAME);
    if (standardNameAtt != NULL) {
      standardName = NetcdfClassic::asString(standardNameAtt);
      delete standardNameAtt;
    }
    
    string longName;
    NcAtt *longNameAtt = var->get_att(LONG_NAME);
    if (longNameAtt != NULL) {
      longName = NetcdfClassic::asString(longNameAtt);
      delete longNameAtt;
    }

    string units;
    NcAtt *unitsAtt = var->get_att(UNITS);
    if (unitsAtt != NULL) {
      units = NetcdfClassic::asString(unitsAtt);
      delete unitsAtt;
    }

    string legendXml;
    NcAtt *legendXmlAtt = var->get_att(LEGEND_XML);
    if (legendXmlAtt != NULL) {
      legendXml = NetcdfClassic::asString(legendXmlAtt);
      delete legendXmlAtt;
    }

    string thresholdingXml;
    NcAtt *thresholdingXmlAtt = var->get_att(THRESHOLDING_XML);
    if (thresholdingXmlAtt != NULL) {
      thresholdingXml = NetcdfClassic::asString(thresholdingXmlAtt);
      delete thresholdingXmlAtt;
    }

    float samplingRatio = Radx::missingMetaFloat;
    NcAtt *samplingRatioAtt = var->get_att(SAMPLING_RATIO);
    if (samplingRatioAtt != NULL) {
      samplingRatio = samplingRatioAtt->as_float(0);
      delete samplingRatioAtt;
    }

    // folding

    bool fieldFolds = false;
    float foldLimitLower = Radx::missingMetaFloat;
    float foldLimitUpper = Radx::missingMetaFloat;
    NcAtt *fieldFoldsAtt = var->get_att(FIELD_FOLDS);
    if (fieldFoldsAtt != NULL) {
      string fieldFoldsStr = NetcdfClassic::asString(fieldFoldsAtt);
      if (fieldFoldsStr == "true"
          || fieldFoldsStr == "TRUE"
          || fieldFoldsStr == "True") {
        fieldFolds = true;
        NcAtt *foldLimitLowerAtt = var->get_att(FOLD_LIMIT_LOWER);
        if (foldLimitLowerAtt != NULL) {
          foldLimitLower = foldLimitLowerAtt->as_float(0);
          delete foldLimitLowerAtt;
        }
        NcAtt *foldLimitUpperAtt = var->get_att(FOLD_LIMIT_UPPER);
        if (foldLimitUpperAtt != NULL) {
          foldLimitUpper = foldLimitUpperAtt->as_float(0);
          delete foldLimitUpperAtt;
        }
      }
      delete fieldFoldsAtt;
    }

    // is this field discrete

    bool isDiscrete = false;
    NcAtt *isDiscreteAtt = var->get_att(IS_DISCRETE);
    if (isDiscreteAtt != NULL) {
      string isDiscreteStr = NetcdfClassic::asString(isDiscreteAtt);
      if (isDiscreteStr == "true"
          || isDiscreteStr == "TRUE"
          || isDiscreteStr == "True") {
        isDiscrete = true;
      }
      delete isDiscreteAtt;
    }

    // get offset and scale

    double offset = 0.0;
    NcAtt *offsetAtt = var->get_att(ADD_OFFSET);
    if (offsetAtt != NULL) {
      offset = offsetAtt->as_double(0);
      delete offsetAtt;
    }

    double scale = 1.0;
    NcAtt *scaleAtt = var->get_att(SCALE_FACTOR);
    if (scaleAtt != NULL) {
      scale = scaleAtt->as_double(0);
      delete scaleAtt;
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
    
    switch (var->type()) {
      case ncDouble: {
        if (_addFl64FieldToRays(var, name, units, standardName, longName,
                                isDiscrete, fieldFolds,
                                foldLimitLower, foldLimitUpper)) {
          iret = -1;
        }
        break;
      }
      case ncFloat: {
        if (_addFl32FieldToRays(var, name, units, standardName, longName,
                                isDiscrete, fieldFolds,
                                foldLimitLower, foldLimitUpper)) {
          iret = -1;
        }
        break;
      }
      case ncInt: {
        if (_addSi32FieldToRays(var, name, units, standardName, longName,
                                scale, offset,
                                isDiscrete, fieldFolds,
                                foldLimitLower, foldLimitUpper)) {
          iret = -1;
        }
        break;
      }
      case ncShort: {
        if (_addSi16FieldToRays(var, name, units, standardName, longName,
                                scale, offset,
                                isDiscrete, fieldFolds,
                                foldLimitLower, foldLimitUpper)) {
          iret = -1;
        }
        break;
      }
      case ncByte: {
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
      _addErrStr("ERROR - RawFile::_readFieldVariables");
      _addErrStr("  cannot read field name: ", name);
      _addErrStr(_file.getNcError()->get_errmsg());
      return -1;
    }

  } // ivar

  return 0;

}

///////////////////////////////////
// get calibration variable
// returns -1 on failure

int RawFile::_readCalVar(const string &name, NcVar* &var,
                             int index, double &val, bool required)
  
{

  val = Radx::missingMetaDouble;
  var = _file.getNcFile()->get_var(name.c_str());

  if (var == NULL) {
    if (!required) {
      return 0;
    } else {
      _addErrStr("ERROR - RawFile::_readCalVar");
      _addErrStr("  cal variable name: ", name);
      _addErrStr("  Cannot read calibration variable");
      _addErrStr(_file.getNcError()->get_errmsg());
      return -1;
    }
  }

  if (var->num_vals() < index-1) {
    _addErrStr("ERROR - RawFile::_readCalVar");
    _addErrStr("  requested index too high");
    _addErrStr("  cal variable name: ", name);
    _addErrInt("  requested index: ", index);
    _addErrInt("  n cals available: ", var->num_vals());
    return -1;
  }

  val = var->as_double(index);

  return 0;

}

//////////////////////////////////////////////////////////////
// Add fl64 fields to _raysFromFile
// The _raysFromFile array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int RawFile::_addFl64FieldToRays(NcVar* var,
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

  Radx::fl64 *data = new Radx::fl64[_nPoints];
  int iret = 0;
  if (_nGatesVary) {
    iret = !var->get(data, _nPoints);
  } else {
    iret = !var->get(data, _nTimesInFile, _nRangeInFile);
  }
  if (iret) {
    delete[] data;
    return -1;
  }

  // set missing value

  Radx::fl64 missingVal = Radx::missingFl64;
  NcAtt *missingValueAtt = var->get_att(MISSING_VALUE);
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  } else {
    missingValueAtt = var->get_att(FILL_VALUE);
    if (missingValueAtt != NULL) {
      missingVal = missingValueAtt->as_double(0);
      delete missingValueAtt;
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
      cerr << "WARNING - RawFile::_addSi16FieldToRays" << endl;
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

int RawFile::_addFl32FieldToRays(NcVar* var,
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

  Radx::fl32 *data = new Radx::fl32[_nPoints];
  int iret = 0;
  if (_nGatesVary) {
    iret = !var->get(data, _nPoints);
  } else {
    iret = !var->get(data, _nTimesInFile, _nRangeInFile);
  }
  if (iret) {
    delete[] data;
    return -1;
  }

  // set missing value

  Radx::fl32 missingVal = Radx::missingFl32;
  NcAtt *missingValueAtt = var->get_att(MISSING_VALUE);
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  } else {
    missingValueAtt = var->get_att(FILL_VALUE);
    if (missingValueAtt != NULL) {
      missingVal = missingValueAtt->as_double(0);
      delete missingValueAtt;
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
      cerr << "WARNING - RawFile::_addSi16FieldToRays" << endl;
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

int RawFile::_addSi32FieldToRays(NcVar* var,
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

  Radx::si32 *data = new Radx::si32[_nPoints];
  int iret = 0;
  if (_nGatesVary) {
    iret = !var->get(data, _nPoints);
  } else {
    iret = !var->get(data, _nTimesInFile, _nRangeInFile);
  }
  if (iret) {
    delete[] data;
    return -1;
  }

  // set missing value

  Radx::si32 missingVal = Radx::missingSi32;
  NcAtt *missingValueAtt = var->get_att(MISSING_VALUE);
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_int(0);
    delete missingValueAtt;
  } else {
    missingValueAtt = var->get_att(FILL_VALUE);
    if (missingValueAtt != NULL) {
      missingVal = missingValueAtt->as_int(0);
      delete missingValueAtt;
    }
  }
  
  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    size_t rayIndex = _raysToRead[ii].indexInFile;

    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - RawFile::_addSi16FieldToRays" << endl;
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

int RawFile::_addSi16FieldToRays(NcVar* var,
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

  Radx::si16 *data = new Radx::si16[_nPoints];
  int iret = 0;
  if (_nGatesVary) {
    iret = !var->get(data, _nPoints);
  } else {
    iret = !var->get(data, _nTimesInFile, _nRangeInFile);
  }
  if (iret) {
    delete[] data;
    return -1;
  }

  // set missing value

  Radx::si16 missingVal = Radx::missingSi16;
  NcAtt *missingValueAtt = var->get_att(MISSING_VALUE);
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_short(0);
    delete missingValueAtt;
  } else {
    missingValueAtt = var->get_att(FILL_VALUE);
    if (missingValueAtt != NULL) {
      missingVal = missingValueAtt->as_short(0);
      delete missingValueAtt;
    }
  }
  
  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    size_t rayIndex = _raysToRead[ii].indexInFile;

    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - RawFile::_addSi16FieldToRays" << endl;
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

int RawFile::_addSi08FieldToRays(NcVar* var,
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

  Radx::si08 *data = new Radx::si08[_nPoints];
  int iret = 0;
  if (_nGatesVary) {
    iret = !var->get((ncbyte *) data, _nPoints);
  } else {
    iret = !var->get((ncbyte *) data, _nTimesInFile, _nRangeInFile);
  }
  if (iret) {
    delete[] data;
    return -1;
  }

  // set missing value

  Radx::si08 missingVal = Radx::missingSi08;
  NcAtt *missingValueAtt = var->get_att(MISSING_VALUE);
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_ncbyte(0);
    delete missingValueAtt;
  } else {
    missingValueAtt = var->get_att(FILL_VALUE);
    if (missingValueAtt != NULL) {
      missingVal = missingValueAtt->as_ncbyte(0);
      delete missingValueAtt;
    }
  }
  
  // load field on rays

  for (size_t ii = 0; ii < _raysToRead.size(); ii++) {
    
    size_t rayIndex = _raysToRead[ii].indexInFile;

    if (rayIndex > _nTimesInFile - 1) {
      cerr << "WARNING - RawFile::_addSi16FieldToRays" << endl;
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

/////////////////////////////////////////////////////////////
// Compute the fixed angles by averaging the elevation angles
// on the sweeps

void RawFile::_computeFixedAngles()
  
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

#endif

