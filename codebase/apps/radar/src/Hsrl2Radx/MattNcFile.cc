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
// MattNcFile.cc
//
// HSRL NetCDF data produced by Matt Haymann's python code
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2018
//
///////////////////////////////////////////////////////////////

#include "MattNcFile.hh"
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
#include <Radx/RadxRemap.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Radx/RadxXml.hh>
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

MattNcFile::MattNcFile(const Params &params) :
        _params(params)
  
{

  clear();

}

/////////////
// destructor

MattNcFile::~MattNcFile()

{
  _file.close();
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void MattNcFile::clear()
  
{

  clearErrStr();
  
  _timeDim = NULL;
  _rangeDim = NULL;

  _nTimesInFile = 0;
  _nRangeInFile = 0;

  _history.clear();
  _project.clear();
  _statusXml.clear();

  _timeVar = NULL;
  _dataTimes.clear();
  _dTimes.clear();

  _rangeVar = NULL;
  _rangeKm.clear();
  _geom.setRangeGeom(0.0075, 154.2432);

  _clearRayVariables();

  _instrumentType = Radx::INSTRUMENT_TYPE_LIDAR;
  _platformType = Radx::PLATFORM_TYPE_AIRCRAFT;
  _primaryAxis = Radx::PRIMARY_AXIS_Y_PRIME;

  _rays.clear();
  
}

////////////////////////////////////////////////////////////
// Check if this is a Matt type file
// Returns true on success, false on failure

bool MattNcFile::isMattNcFile(const string &path)
  
{

  clear();
  
  // open file
  
  if (_file.openRead(path)) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "DEBUG - not NetCDF file" << endl;
      cerr << _file.getErrStr() << endl;
    }
    return false;
  }

  // read dimensions
  
  if (_readDimensions()) {
    _file.close();
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "DEBUG - not Matt-type HSRL file" << endl;
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

int MattNcFile::getTimeFromPath(const string &path, RadxTime &rtime)

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
  
  while (start < end - 14) {
    char cc;
    int year, month, day, hour, min;
    if (sscanf(start, "%4d%2d%2d%c%2d%2d",
               &year, &month, &day, &cc, &hour, &min) == 6) {
      if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
        return -1;
      }
      if (hour < 0 || hour > 23 || min < 0 || min > 59) {
        return -1;
      }
      rtime.set(year, month, day, hour, min, 0);
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

int MattNcFile::readFromPath(const string &path,
                             RadxVol &vol)
  
{
  
  if (_params.debug) {
    cerr << "Reading file: " << path << endl;
  }

  string errStr("ERROR - MattNcFile::readFromPath");

  _readVol = &vol;

  // get the start time from the file path

  getTimeFromPath(path, _startTime);

  // init
  
  clear();
  _nTimesInFile = 0;
  _nRangeInFile = 0;
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
  
  // read range variable
  
  if (_readRange()) {
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

int MattNcFile::_readDimensions()

{

  // read required dimensions
  
  if (_file.readDim("time", _timeDim) == 0) {
    _nTimesInFile = _timeDim->size();
  } else {
    _addErrStr("ERROR - MattNcFile::_readDimensions()");
    _addErrStr("  Cannot find 'time' dimension");
    return -1;
  }
  
  if (_nTimesInFile < 10) {
    _addErrStr("ERROR - MattNcFile::_readDimensions()");
    _addErrInt("  Too few times in file: ", _nTimesInFile);
    return -1;
  }

  if (_file.readDim("range", _rangeDim) == 0) {
    _nRangeInFile = _rangeDim->size();
  } else {
    _addErrStr("ERROR - MattNcFile::_readDimensions()");
    _addErrStr("  Cannot find 'range' dimension");
    return -1;
  }

  _nPoints = _nTimesInFile * _nRangeInFile;
  
  return 0;

}

///////////////////////////////////
// read the global attributes

int MattNcFile::_readGlobalAttributes()

{

  _history.clear();
  _project.clear();

  for (int ii = 0; ii < _file.getNc3File()->num_atts(); ii++) {
    
    Nc3Att* att = _file.getNc3File()->get_att(ii);
    
    if (att == NULL || !att->is_valid()) {
      continue;
    }

    if (att->values() == NULL || att->num_vals() == 0) {
      delete att;
      continue;
    }

    if (!strcmp(att->name(), "history")) {
      _history = Nc3xFile::asString(att);
    }

    if (!strcmp(att->name(), "project")) {
      _project = Nc3xFile::asString(att);
    }

    // Caller must delete attribute

    delete att;
    
  } // ii
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Global attr history: " << _history << endl;
    cerr << "Global attr project: " << _project << endl;
  }

  return 0;

}

///////////////////////////////////
// read the times

int MattNcFile::_readTimes()

{

  _dataTimes.clear();
  _dTimes.clear();

  // read the time variable

  _timeVar = _file.getNc3File()->get_var("time");
  if (_timeVar == NULL) {
    _addErrStr("ERROR - MattNcFile::_readTimes");
    _addErrStr("  Cannot find time variable");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  if (_timeVar->num_dims() != 1) {
    _addErrStr("ERROR - MattNcFile::_readTimes");
    _addErrStr("  time variable has no dimensions");
    return -1;
  }
  Nc3Dim *timeDim = _timeVar->get_dim(0);
  if (timeDim != _timeDim) {
    _addErrStr("ERROR - MattNcFile::_readTimes");
    _addErrStr("  time has incorrect dimensions");
    _addErrStr("  should be (time)");
    return -1;
  }
  
  // get units attribute
  
  Nc3Att* unitsAtt = _timeVar->get_att("units");
  if (unitsAtt == NULL) {
    _addErrStr("ERROR - MattNcFile::_readTimes");
    _addErrStr("  Time has no units");
    return -1;
  }
  string units = Nc3xFile::asString(unitsAtt);
  delete unitsAtt;

  // parse the time units reference time

  RadxTime stime(units);
  time_t refTimeSecs = stime.utime();

  
  // read in time 2D array

  float *timeData = new float[_nTimesInFile];
  if (!_timeVar->get(timeData, _nTimesInFile)) {
    _addErrStr("ERROR - MattNcFile::_readTimes");
    _addErrStr("  Cannot read time array");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    delete[] timeData;
    return -1;
  }

  for (size_t ii = 0; ii < _nTimesInFile; ii++) {
    double dt = timeData[ii];
    _dTimes.push_back(dt);
    RadxTime rayTime = refTimeSecs + dt;
    _dataTimes.push_back(rayTime);
  }
  
  delete[] timeData;
  return 0;

}

///////////////////////////////////
// read the range variable

int MattNcFile::_readRange()

{

  _rangeVar = _file.getNc3File()->get_var("range");
  if (_rangeVar == NULL || _rangeVar->num_vals() < 1) {
    _addErrStr("ERROR - MattNcFile::_readRange");
    _addErrStr("  Cannot read range");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  
  _rangeKm.clear();
  _nRangeInFile = _rangeDim->size();
  
  if (_rangeVar->num_dims() == 1) {
    
    // 1-dimensional - range dim
    
    Nc3Dim *rangeDim = _rangeVar->get_dim(0);
    if (rangeDim != _rangeDim) {
      _addErrStr("ERROR - NcfRadxFilem::_readRange");
      _addErrStr("  Range has incorrect dimension, name: ", rangeDim->name());
      return -1;
    }

    double *rangeMeters = new double[_nRangeInFile];
    if (_rangeVar->get(rangeMeters, _nRangeInFile)) {
      double *rr = rangeMeters;
      for (size_t ii = 0; ii < _nRangeInFile; ii++, rr++) {
        _rangeKm.push_back(*rr / 1000.0);
      }
    }
    delete[] rangeMeters;

  }
  
  // set the geometry from the range vector
  
  RadxRemap remap;
  if (remap.computeRangeLookup(_rangeKm)) {
    return -1;
  }
  _gateSpacingIsConstant = remap.getGateSpacingIsConstant();
  _geom.setRangeGeom(remap.getStartRangeKm(), remap.getGateSpacingKm());
  
  return 0;

}

///////////////////////////////////
// clear the ray variables

void MattNcFile::_clearRayVariables()

{

  _polAngle.clear();
  _telescopeDirection.clear();

  _lat.clear();
  _lon.clear();
  _alt.clear();
  _roll.clear();
  _pitch.clear();
  _heading.clear();
  _pressure.clear();
  _tas.clear();
  _temp.clear();

}

///////////////////////////////////
// read in ray variables

int MattNcFile::_readRayVariables()

{

  _clearRayVariables();
  int iret = 0;

  _readRayVar("polarization", _polAngle);
  if (_polAngle.size() < _nTimesInFile) {
    _addErrStr("ERROR - polarization variable required");
    iret = -1;
  }

  _readRayVar("TelescopeDirection", _telescopeDirection);
  if (_telescopeDirection.size() < _nTimesInFile) {
    _addErrStr("ERROR - TelescopeLocked variable required");
    iret = -1;
  }

  _readRayVar("GGLAT", _lat);
  if (_lat.size() < _nTimesInFile) {
    _addErrStr("ERROR - GGLAT variable required");
    iret = -1;
  }

  _readRayVar("GGLON", _lon);
  if (_lon.size() < _nTimesInFile) {
    _addErrStr("ERROR - GGLON variable required");
    iret = -1;
  }

  _readRayVar("GGALT", _alt);
  if (_alt.size() < _nTimesInFile) {
    _addErrStr("ERROR - GGALT variable required");
    iret = -1;
  }

  _readRayVar("ROLL", _roll);
  if (_roll.size() < _nTimesInFile) {
    _addErrStr("ERROR - ROLL variable required");
    iret = -1;
  }

  _readRayVar("PITCH", _pitch);
  if (_pitch.size() < _nTimesInFile) {
    _addErrStr("ERROR - PITCH variable required");
    iret = -1;
  }

  _readRayVar("THDG", _heading);
  if (_heading.size() < _nTimesInFile) {
    _addErrStr("ERROR - THDG variable required");
    iret = -1;
  }

  _readRayVar("PSXC", _pressure);
  if (_pressure.size() < _nTimesInFile) {
    _addErrStr("ERROR - PSXC variable required");
    iret = -1;
  }

  _readRayVar("TASX", _tas);
  if (_tas.size() < _nTimesInFile) {
    _addErrStr("ERROR - TASX variable required");
    iret = -1;
  }

  _readRayVar("ATX", _temp);
  if (_temp.size() < _nTimesInFile) {
    _addErrStr("ERROR - ATX variable required");
    iret = -1;
  }

  if (iret) {
    _addErrStr("ERROR - MattNcFile::_readRayVariables");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// read a ray variable - double
// side effects: set var, vals

int MattNcFile::_readRayVar(const string &name, vector<double> &vals)
  
{

  vals.clear();

  // get var
  
  Nc3Var *var = _getRayVar(name, true);
  if (var == NULL) {
    _addErrStr("ERROR - MattNcFile::_readRayVar");
    return -1;
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
    _addErrStr("ERROR - MattNcFile::_readRayVar");
    _addErrStr("  Cannot read variable: ", name);
    _addErrStr(_file.getNc3Error()->get_errmsg());
    iret = -1;
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// get a ray variable by name
// returns NULL on failure

Nc3Var* MattNcFile::_getRayVar(const string &name, bool required)

{

  // get var
  
  Nc3Var *var = _file.getNc3File()->get_var(name.c_str());
  if (var == NULL) {
    if (required) {
      _addErrStr("ERROR - MattNcFile::_getRayVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
    }
    return NULL;
  }

  // check time dimension
  
  if (var->num_dims() < 1) {
    if (required) {
      _addErrStr("ERROR - MattNcFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no dimensions");
    }
    return NULL;
  }
  Nc3Dim *timeDim = var->get_dim(0);
  if (timeDim->size() != _timeDim->size()) {
    if (required) {
      _addErrStr("ERROR - MattNcFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrInt("  variable has incorrect dimension, size: ", 
                 timeDim->size());
      _addErrInt("  should be: ", _timeDim->size());
    }
    return NULL;
  }

  return var;

}

///////////////////////////////////
// create the rays to be read in
// and set meta data

int MattNcFile::_createRays(const string &path)

{

  // compile a list of the rays to be read in
  
  _rays.clear();
  RadxCfactors corr;

  for (size_t ii = 0; ii < _nTimesInFile; ii++) {

    // new ray

    RadxRay *ray = new RadxRay;
    ray->copyRangeGeom(_geom);
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
      if (MattNcFile::readGeorefFromSpdb(_params.georef_data_spdb_url,
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
            if (isfinite(roll) && roll > -45.0 && roll < 45.0) {
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
            if (isfinite(roll) && roll > -45.0 && roll < 45.0) {
              ray->setElevationDeg(-94.0 - geo.getRoll());
            }
          }
        }

        ray->setGeoref(geo);

      } // if (MattNcFile::readGeorefFromSpdb ...

    } // if (_params.read_georef_data_from_aircraft_system)
    
    // other metadata - overloading
    
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

void MattNcFile::_loadReadVolume()
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

  _readVol->setTitle("NCAR EOL HSRL");
  _readVol->setSource("HSRL software");
  _readVol->setHistory(_history);
  _readVol->setInstitution("NCAR");
  _readVol->setReferences("");
  _readVol->setComment("");
  _readVol->setDriver("Hsrl2Radx");
  _readVol->setCreated(_startTime.getW3cStr());
  _readVol->setStatusXml(_statusXml);
  
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

  _readVol->copyRangeGeom(_geom);

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

int MattNcFile::_readFieldVariables()

{

  // initialize status xml

  _statusXml.clear();
  _statusXml += RadxXml::writeStartTag("FieldStatus", 0);
  bool gotStatus = false;

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
    Nc3Dim* rangeDim = var->get_dim(1);
    if (timeDim != _timeDim || rangeDim != _rangeDim) {
      continue;
    }

    // check the type
    Nc3Type ftype = var->type();
    if (ftype != nc3Double && ftype != nc3Byte) {
      // not a valid type for field data
      continue;
    }

    // set names, units, etc

    string name = var->name();
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "DEBUG - MattNcFile::_readFieldVariables" << endl;
      cerr << "  -->> adding field: " << name << endl;
    }

    string units;
    Nc3Att *unitsAtt = var->get_att("units");
    if (unitsAtt != NULL) {
      units = Nc3xFile::asString(unitsAtt);
      delete unitsAtt;
    }

    string description;
    Nc3Att *descAtt = var->get_att("description");
    if (descAtt != NULL) {
      description = Nc3xFile::asString(descAtt);
      delete descAtt;
    }

    string procStatus;
    Nc3Att *statusAtt = var->get_att("ProcessingStatus");
    if (statusAtt != NULL) {
      procStatus = Nc3xFile::asString(statusAtt);
      delete statusAtt;
    }

    // load in the data
    
    if (ftype == nc3Double) {
      if (_addFl64FieldToRays(var, name, units, description)) {
        _addErrStr("ERROR - MattNcFile::_readFieldVariables");
        _addErrStr("  cannot read field name: ", name);
        _addErrStr(_file.getNc3Error()->get_errmsg());
        return -1;
      }
    } else {
      if (_addSi08FieldToRays(var, name, units, description)) {
        _addErrStr("ERROR - MattNcFile::_readFieldVariables");
        _addErrStr("  cannot read field name: ", name);
        _addErrStr(_file.getNc3Error()->get_errmsg());
        return -1;
      }
    }

    // add processing status to statusXml, if appropriate

    if (procStatus.size() > 0) {
      _statusXml += RadxXml::writeStartTag("Field", 1);
      _statusXml += RadxXml::writeString("Name", 2, name);
      _statusXml += RadxXml::writeString("Description", 2, description);
      _statusXml += RadxXml::writeString("Status", 2, procStatus);
      _statusXml += RadxXml::writeEndTag("Field", 1);
      gotStatus = true;
    }

  } // ivar

  if (gotStatus) {
    _statusXml += RadxXml::writeEndTag("FieldStatus", 0);
  } else {
    _statusXml.clear();
  }

  return 0;

}

//////////////////////////////////////////////////////////////
// Add double field to rays
// The _rays array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int MattNcFile::_addFl64FieldToRays(Nc3Var* var,
                                    const string &name,
                                    const string &units,
                                    const string &description)
  
{

  // get data from array
  
  RadxArray<Radx::fl64> ddata_;
  Radx::fl64 *ddata = ddata_.alloc(_nPoints);
  int iret = !var->get(ddata, _nTimesInFile, _nRangeInFile);
  if (iret) {
    return -1;
  }

  // set name

  string outName(name);
  string standardName;
  // string longName;
  if (outName.find(_params.combined_hi_field_name) != string::npos) {
    outName = Names::CombinedHighCounts;
    standardName = Names::lidar_copolar_combined_backscatter_photon_count;
    // longName = "high_channel_combined_backscatter_photon_count";
  } else if (outName.find(_params.combined_lo_field_name) != string::npos) {
    outName = Names::CombinedLowCounts;
    standardName = Names::lidar_copolar_combined_backscatter_photon_count;
    // longName = "low_channel_combined_backscatter_photon_count";
  } else if (outName.find(_params.molecular_field_name) != string::npos) {
    outName = Names::MolecularCounts;
    standardName = Names::lidar_copolar_molecular_backscatter_photon_count;
    // longName = Names::lidar_copolar_molecular_backscatter_photon_count;
  } else if (outName.find(_params.cross_field_name) != string::npos) {
    outName = Names::CrossPolarCounts;
    standardName = Names::lidar_crosspolar_combined_backscatter_photon_count;
    // longName = Names::lidar_crosspolar_combined_backscatter_photon_count;
  }
  
  // loop through the rays
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {

    // get data for ray
    
    int startIndex = iray * _nRangeInFile;
    Radx::fl64 *dd = ddata + startIndex;

    RadxField *field =
      _rays[iray]->addField(outName, units, _nRangeInFile,
                            Radx::missingFl64,
                            dd,
                            true);
    
    // field->setLongName(longName);
    field->setStandardName(standardName);
    field->setLongName(description);
    field->copyRangeGeom(_geom);
    
  }
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add int8 field to rays
// The _rays array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int MattNcFile::_addSi08FieldToRays(Nc3Var* var,
                                    const string &name,
                                    const string &units,
                                    const string &description)
  
{

  // get data from array as bytes

  RadxArray<ncbyte> ndata_;
  ncbyte *ndata = ndata_.alloc(_nPoints);
  int iret = !var->get(ndata, _nTimesInFile, _nRangeInFile);
  if (iret) {
    return -1;
  }

  // convert to si08 

  RadxArray<Radx::si08> sdata_;
  Radx::si08 *sdata = sdata_.alloc(_nPoints);
  for (int ii = 0; ii < _nPoints; ii++) {
    sdata[ii] = (int) ndata[ii];
  }
  
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

    // get data for ray
    
    int startIndex = iray * _nRangeInFile;
    Radx::si08 *sd = sdata + startIndex;
    
    RadxField *field =
      _rays[iray]->addField(outName, units, _nRangeInFile,
                            Radx::missingSi08,
                            sd, 1.0, 0.0,
                            true);
    
    field->setLongName(longName);
    field->setStandardName(standardName);
    field->setComment(description);
    field->copyRangeGeom(_geom);
    
  }
  
  return 0;
  
}


///////////////////////////////////////////////
// add labelled integer value to error string,
// with optional following carriage return.

void MattNcFile::_addErrInt(string label, int iarg, bool cr)
{
  Radx::addErrInt(_errStr, label, iarg, cr);
}

///////////////////////////////////////////////
// add labelled double value to error string,
// with optional following carriage return.
// Default format is %g.

void MattNcFile::_addErrDbl(string label, double darg,
                            string format, bool cr)
  
{
  Radx::addErrDbl(_errStr, label, darg, format, cr);
}

////////////////////////////////////////
// add labelled string to error string
// with optional following carriage return.

void MattNcFile::_addErrStr(string label, string strarg, bool cr)

{
  Radx::addErrStr(_errStr, label, strarg, cr);
}

void MattNcFile::_clearRays()
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

void MattNcFile::computeRadarAngles(RadxGeoref &georef,
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

int MattNcFile::readGeorefFromSpdb(string georefUrl,
                                   time_t searchTime,
                                   int searchMarginSecs,
                                   bool debug,
                                   RadxGeoref &radxGeoref)
  
{

  DsSpdb spdb;
  
  if(spdb.getClosest(georefUrl, searchTime, searchMarginSecs)) {
    if (debug >= Params::DEBUG_VERBOSE) {
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


