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
#include "Names.hh"
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
#include <Spdb/DsSpdb.hh>
#include <rapformats/ac_georef.hh>
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

  _telescopeLocked.clear();
  _telescopeDirection.clear();

  _instrumentType = Radx::INSTRUMENT_TYPE_LIDAR;
  _platformType = Radx::PLATFORM_TYPE_AIRCRAFT;
  _primaryAxis = Radx::PRIMARY_AXIS_Y_PRIME;

  _rawGateSpacingKm = _params.raw_bin_spacing_km;
  _gateSpacingKm = _rawGateSpacingKm;
  _startRangeKm = _params.raw_bin_start_range_km;
  if (_params.combine_bins_on_read) {
    _gateSpacingKm *= _params.n_bins_per_gate;
    _startRangeKm += (_gateSpacingKm - _rawGateSpacingKm) / 2.0;
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

  _readVol = &vol;

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
  
  // add field variables to file rays
  
  if (_readFieldVariables()) {
    _addErrStr(errStr);
    return -1;
  }

  // load the data into the read volume

  _loadReadVolume();

  // close file
  
  _file.close();

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

  _nPoints = _nTimesInFile * _nBinsInFile;
  _nBinsPerGate = 1;
  if (_params.combine_bins_on_read) {
    _nBinsPerGate = _params.n_bins_per_gate;
  }
  _nGates = _nBinsInFile / _nBinsPerGate;
  
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

    cerr << "Global attr machType: " << _machType << endl;
    cerr << "Global attr hostName: " << _hostName << endl;
    cerr << "Global attr userName: " << _userName << endl;
    cerr << "Global attr gitCommit: " << _gitCommit << endl;
    cerr << "Global attr hsrlVersion: " << _hsrlVersion << endl;
    cerr << "Global attr sourceSoftware: " << _sourceSoftware << endl;
    
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

///////////////////////////////////
// clear the ray variables

void RawFile::_clearRayVariables()

{

  _telescopeLocked.clear();
  _telescopeDirection.clear();

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

///////////////////////////////////
// create the rays to be read in
// and set meta data

int RawFile::_createRays(const string &path)

{

  // compile a list of the rays to be read in
  
  _rays.clear();
  RadxCfactors corr;

  for (size_t ii = 0; ii < _nTimesInFile; ii++) {

    // new ray

    RadxRay *ray = new RadxRay;
    ray->setRangeGeom(_startRangeKm, _gateSpacingKm);
    ray->setTime(_dataTimes[ii]);

    // sweep info

    ray->setVolumeNumber(-9999);
    ray->setSweepNumber(0);
    ray->setSweepMode(Radx::SWEEP_MODE_POINTING);
    ray->setPrtMode(Radx::PRT_MODE_FIXED);
    ray->setTargetScanRateDegPerSec(0.0);
    ray->setIsIndexed(false);

    // georeference
    
    if (_telescopeDirection[ii] == 1) {

      // pointing up

      ray->setAzimuthDeg(0.0);
      ray->setElevationDeg(94.0);
      ray->setFixedAngleDeg(94.0);
      
    } else {
      
      // pointing down
      
      ray->setAzimuthDeg(0.0);
      ray->setElevationDeg(-94.0);
      ray->setFixedAngleDeg(-94.0);
      
    }
    
    if (_params.read_georef_data_from_aircraft_system) {

      RadxGeoref geo;
      if (RawFile::readGeorefFromSpdb(_params.georef_data_spdb_url,
                                      _dataTimes[ii].utime(),
                                      _params.georef_data_search_margin_secs,
                                      _params.debug >= Params::DEBUG_VERBOSE,
                                      geo) == 0) {
        if (_telescopeDirection[ii] == 1) {
          // pointing up
          geo.setRotation(-4.0);
          geo.setTilt(0.0);
          ray->setElevationDeg(94.0);
          if (_params.correct_elevation_angle_for_roll) {
            double roll = geo.getRoll();
            if (std::isfinite(roll) && roll > -45.0 && roll < 45.0) {
              ray->setElevationDeg(94.0 - roll);
            }
          }
        } else {
          // pointing down
          geo.setRotation(184.0);
          geo.setTilt(0.0);
          ray->setElevationDeg(-94.0);
          if (_params.correct_elevation_angle_for_roll) {
            double roll = geo.getRoll();
            if (std::isfinite(roll) && roll > -45.0 && roll < 45.0) {
              ray->setElevationDeg(-94.0 - geo.getRoll());
            }
          }
        }

        ray->setGeoref(geo);

        // compute az/el from geo
        
        // double azimuth, elevation;
        // RadxCfactors corr;
        // computeRadarAngles(geo, corr, azimuth, elevation);
        // ray->setAzimuthDeg(azimuth);
        // ray->setElevationDeg(elevation);
    
      } // if (RawFile::readGeorefFromSpdb ...

    } // if (_params.read_georef_data_from_aircraft_system)
    
    // other metadata - overloading
    
    ray->setMeasXmitPowerDbmH(_totalEnergy[ii]);
    ray->setEstimatedNoiseDbmHc(_polAngle[ii]);

    
    // hard coded 2000 as replacement for DATA_shot_count from raw file

    ray->setNSamples(2000);
    ray->setPrtSec(1.0 / 4000.0);
    
    // add to ray vector
    
    _rays.push_back(ray);

  } // ii

  return 0;

}

/////////////////////////////////////////////////////////
// load up the read volume with the data from this object

void RawFile::_loadReadVolume()
{

  _readVol->clear();

  _readVol->setOrigFormat("HSRL-RAW");
  _readVol->setVolumeNumber(-9999);
  _readVol->setInstrumentType(_instrumentType);
  _readVol->setInstrumentName("HSRL");
  _readVol->setSiteName("GV");
  _readVol->setPlatformType(_platformType);
  _readVol->setPrimaryAxis(_primaryAxis);
  
  _readVol->addFrequencyHz(Radx::LIGHT_SPEED / 538.0e-9);
  _readVol->addFrequencyHz(Radx::LIGHT_SPEED / 1064.0e-9);
  
  _readVol->setLidarConstant(-9999.0);
  _readVol->setLidarPulseEnergyJ(-9999.0);
  _readVol->setLidarPeakPowerW(-9999.0);
  _readVol->setLidarApertureDiamCm(-9999.0);
  _readVol->setLidarApertureEfficiency(-9999.0);
  _readVol->setLidarFieldOfViewMrad(-9999.0);
  _readVol->setLidarBeamDivergenceMrad(-9999.0);

  _readVol->setTitle("NCAR HSRL");
  _readVol->setSource("HSRL realtime software");
  _readVol->setHistory("Converted from RAW NetCDF files");
  _readVol->setInstitution("NCAR");
  _readVol->setReferences("University of Wisconsin");
  _readVol->setComment("");
  _readVol->setDriver("Hsrl2Radx");
  _readVol->setCreated(_dataAdded);
  _readVol->setStatusXml("");
  
  _readVol->setScanName("Vert");
  _readVol->setScanId(0);

  if (_rays.size() > 0) {
    const RadxGeoref *georef = _rays[0]->getGeoreference();
    if (georef) {
      _readVol->setLatitudeDeg(georef->getLatitude());
      _readVol->setLongitudeDeg(georef->getLongitude());
      _readVol->setAltitudeKm(georef->getAltitudeKmMsl());
    }
  }

  _readVol->setRangeGeom(_startRangeKm, _gateSpacingKm);

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    _readVol->addRay(_rays[ii]);
  }

  // memory responsibility has passed to the volume object, so clear
  // the vectors without deleting the objects to which they point

  _rays.clear();

  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();

  // load the volume information from the rays

  _readVol->loadVolumeInfoFromRays();
  
}

////////////////////////////////////////////
// read the field variables

int RawFile::_readFieldVariables()

{

  // loop through the variables, adding data fields as appropriate
  
  for (int ivar = 0; ivar < _file.getNc3File()->num_vars(); ivar++) {
    
    Nc3Var* var = _file.getNc3File()->get_var(ivar);
    if (var == NULL) {
      continue;
    }
    
    int numDims = var->num_dims();
    // we need fields with 2 dimensions
    if (numDims != 2) {
      continue;
    }

    // check that we have the correct dimensions
    Nc3Dim* timeDim = var->get_dim(0);
    Nc3Dim* bincountDim = var->get_dim(1);
    if (timeDim != _timeDim || bincountDim != _binCountDim) {
      continue;
    }
    
    // check the type
    Nc3Type ftype = var->type();
    if (ftype != nc3Int) {
      // not a valid type for field data
      continue;
    }

    // set names, units, etc

    string name = var->name();
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "DEBUG - RawFile::_readFieldVariables" << endl;
      cerr << "  -->> adding field: " << name << endl;
    }

    string longName;
    Nc3Att *longNameAtt = var->get_att("long_name");
    if (longNameAtt != NULL) {
      longName = Nc3xFile::asString(longNameAtt);
      delete longNameAtt;
    }
    
    string units = "counts";

    // load in the data

    if (_addCountFieldToRays(var, name, units)) {
      _addErrStr("ERROR - RawFile::_readFieldVariables");
      _addErrStr("  cannot read field name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }

  } // ivar

  return 0;

}

//////////////////////////////////////////////////////////////
// Add si32 field to rays
// The _rays array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int RawFile::_addCountFieldToRays(Nc3Var* var,
                                  const string &name,
                                  const string &units)
  
{

  // get int data from array
  
  RadxArray<Radx::si32> idata_;
  Radx::si32 *idata = idata_.alloc(_nPoints);
  int iret = !var->get(idata, _nTimesInFile, _nBinsInFile);
  if (iret) {
    return -1;
  }

  // set up float array

  RadxArray<Radx::fl32> fcounts_;
  Radx::fl32 *fcounts = fcounts_.alloc(_nGates);

  // set name

  string outName(name);
  string standardName;
  string longName;
  if (outName.find(_params.combined_hi_field_name) != string::npos) {
    outName = Names::CombinedHighCounts;
    standardName = Names::lidar_copolar_combined_backscatter_photon_count;
    longName = "high_channel_combined_backscatter_photon_count";
  } else if (outName.find(_params.combined_lo_field_name) != string::npos) {
    outName = Names::CombinedLowCounts;
    standardName = Names::lidar_copolar_combined_backscatter_photon_count;
    longName = "low_channel_combined_backscatter_photon_count";
  } else if (outName.find(_params.molecular_field_name) != string::npos) {
    outName = Names::MolecularCounts;
    standardName = Names::lidar_copolar_molecular_backscatter_photon_count;
    longName = Names::lidar_copolar_molecular_backscatter_photon_count;
  } else if (outName.find(_params.cross_field_name) != string::npos) {
    outName = Names::CrossPolarCounts;
    standardName = Names::lidar_crosspolar_combined_backscatter_photon_count;
    longName = Names::lidar_crosspolar_combined_backscatter_photon_count;
  }
  
  // loop through the rays
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {

    // get int counts for ray
    
    int startIndex = iray * _nBinsInFile;
    Radx::si32 *icounts = idata + startIndex;

    // sum counts per gate

    size_t ibin = 0;
    for (size_t igate = 0; igate < _nGates; igate++) {
      fcounts[igate] = 0.0;
      for (size_t ii = 0; ii < _nBinsPerGate; ii++, ibin++) {
        fcounts[igate] += icounts[ibin];
      }
      fcounts[igate] /= (double) _nBinsPerGate;
    }
    
    RadxField *field =
      _rays[iray]->addField(outName, units, _nGates,
                            Radx::missingFl32,
                            fcounts,
                            true);
    
    field->setLongName(longName);
    field->setStandardName(standardName);
    field->setRangeGeom(_startRangeKm, _gateSpacingKm);
    
  }
  
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

void RawFile::_clearRays()
{
  for (int ii = 0; ii < (int) _rays.size(); ii++) {
    delete _rays[ii];
  }
  _rays.clear();
}

///////////////////////////////////////////////////////////////////
// compute the true azimuth, elevation, etc. from platform
// parameters using Testud's equations with their different
// definitions of rotation angle, etc.
//
// see Wen-Chau Lee's paper
// "Mapping of the Airborne Doppler Radar Data"

void RawFile::computeRadarAngles(RadxGeoref &georef,
                                 RadxCfactors &corr,
                                 double &azimuthDeg,
                                 double &elevationDeg)
  
{
  
  double R = (georef.getRoll() + corr.getRollCorr()) * Radx::DegToRad;
  double P = (georef.getPitch() + corr.getPitchCorr()) * Radx::DegToRad;
  double H = (georef.getHeading() + corr.getHeadingCorr()) * Radx::DegToRad;
  double D = (georef.getDrift() + corr.getDriftCorr()) * Radx::DegToRad;
  double T = H + D;
  
  double sinP = sin(P);
  double cosP = cos(P);
  double sinD = sin(D);
  double cosD = cos(D);
  
  double theta_a = 
    (georef.getRotation() + corr.getRotationCorr()) * Radx::DegToRad;
  double tau_a =
    (georef.getTilt() + corr.getTiltCorr()) * Radx::DegToRad;
  double sin_tau_a = sin(tau_a);
  double cos_tau_a = cos(tau_a);
  double sin_theta_rc = sin(theta_a + R); /* roll corrected rotation angle */
  double cos_theta_rc = cos(theta_a + R); /* roll corrected rotation angle */
  
  double xsubt = (cos_theta_rc * sinD * cos_tau_a * sinP
                  + cosD * sin_theta_rc * cos_tau_a
                  -sinD * cosP * sin_tau_a);
  
  double ysubt = (-cos_theta_rc * cosD * cos_tau_a * sinP
                  + sinD * sin_theta_rc * cos_tau_a
                  + cosP * cosD * sin_tau_a);
  
  double zsubt = (cosP * cos_tau_a * cos_theta_rc
                  + sinP * sin_tau_a);
  
  double lambda_t = atan2(xsubt, ysubt);
  double azimuthRad = fmod(lambda_t + T, M_PI * 2.0);
  double elevationRad = asin(zsubt);
  
  elevationDeg = elevationRad * Radx::RadToDeg;
  azimuthDeg = azimuthRad * Radx::RadToDeg;
  if (azimuthDeg < 0) {
    azimuthDeg += 360.0;
  }
  
}

//////////////////////////////////////////////////////////////
// Read georeference from SPDB
// Returns 0 on success, -1 on error

int RawFile::readGeorefFromSpdb(string georefUrl,
                                time_t searchTime,
                                int searchMarginSecs,
                                bool debug,
                                RadxGeoref &radxGeoref)
  
{

  DsSpdb spdb;
  
  if(spdb.getClosest(georefUrl, searchTime, searchMarginSecs)) {
    if (debug) {
      cerr << "ERROR - Hsrl2Radx::_readGeorefFromSpdb" << endl;
      cerr << "  Cannot read georef data from URL: "
           << georefUrl << endl;
      cerr << spdb.getErrStr() << endl;
    }
    return -1;
  }

  if (spdb.getProdId() != SPDB_AC_GEOREF_ID) {
    cerr << "ERROR - Hsrl2Radx::_readGeorefFromSpdb" << endl;
    cerr << "  URL does not hold ac_georef data: "
         << georefUrl << endl;
    cerr << "  Product found: " << spdb.getProdLabel() << endl;
    return -1;
  }
  
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
  size_t nChunks = chunks.size();
  if (nChunks <= 0) {
    if (debug) {
      cerr << "ERROR - Hsrl2Radx::_readGeorefFromSpdb" << endl;
      cerr << "  Cannot read georef data from URL: "
           << georefUrl << endl;
      cerr << "  searchTime: " << RadxTime::strm(searchTime) << endl;
      cerr << "  searchMarginSecs: " << searchMarginSecs << endl;
      cerr << "  No chunks returned" << endl;
    }
    return -1;
  }
  
  const Spdb::chunk_t &chunk = chunks[0];
  if (chunk.len != sizeof(ac_georef_t)) {
    cerr << "ERROR - Hsrl2Radx::_readGeorefFromSpdb" << endl;
    cerr << "  Bad chunk length found: " << chunk.len << endl;
    cerr << "  Should be: " << sizeof(ac_georef_t) << endl;
    cerr << "  Ignoring chunk" << endl;
    return -1;
  }

  // decode chunk data
  
  ac_georef_t georef;
  memcpy(&georef, chunk.data, sizeof(georef));
  BE_to_ac_georef(&georef);

  radxGeoref.setTimeSecs(georef.time_secs_utc);
  radxGeoref.setNanoSecs(georef.time_nano_secs);
  radxGeoref.setLongitude(georef.longitude);
  radxGeoref.setLatitude(georef.latitude);
  radxGeoref.setAltitudeKmMsl(georef.altitude_msl_km);
  radxGeoref.setAltitudeKmAgl(georef.altitude_agl_km);
  radxGeoref.setEwVelocity(georef.ew_velocity_mps);
  radxGeoref.setNsVelocity(georef.ns_velocity_mps);
  radxGeoref.setVertVelocity(georef.vert_velocity_mps);
  radxGeoref.setHeading(georef.heading_deg);
  radxGeoref.setTrack(georef.track_deg);
  radxGeoref.setRoll(georef.roll_deg);
  radxGeoref.setPitch(georef.pitch_deg);
  radxGeoref.setDrift(georef.drift_angle_deg);
  radxGeoref.setEwWind(georef.ew_horiz_wind_mps);
  radxGeoref.setNsWind(georef.ns_horiz_wind_mps);
  radxGeoref.setVertWind(georef.vert_wind_mps);
  radxGeoref.setHeadingRate(georef.heading_rate_dps);
  radxGeoref.setPitchRate(georef.pitch_rate_dps);
  radxGeoref.setRollRate(georef.roll_rate_dps);
  radxGeoref.setDriveAngle1(georef.drive_angle_1_deg);
  radxGeoref.setDriveAngle2(georef.drive_angle_2_deg);

  return 0;

}


