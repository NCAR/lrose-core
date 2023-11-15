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
// Era5File.cc
//
// ERA5 reanalysis NetCDF data produced by CISL
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2023
//
///////////////////////////////////////////////////////////////

#include "Era5File.hh"
#include <Ncxx/NcxxVar.hh>
#include <toolsa/Path.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <algorithm>
using namespace std;

//////////////
// Constructor

Era5File::Era5File(const Params &params) :
        _params(params)
  
{

  clear();

}

/////////////
// destructor

Era5File::~Era5File()

{
  _file.close();
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void Era5File::clear()
  
{

  clearErrStr();
  
  _timeDim.setNull();
  _lonDim.setNull();
  _latDim.setNull();
  _levelDim.setNull();
  
  _nTimesInFile = 0;
  _nPoints = 0;

  _dataSource.clear();
  _history.clear();
  _datasetUrl.clear();
  _datasetDoi.clear();

  // _timeVar.setNull();
  _dataTimes.clear();
  _iTimes.clear();

  _data.clear();
  
}

////////////////////////////////////////////////////////////
// Check if this is a Matt type file
// Returns true on success, false on failure

bool Era5File::isEra5File(const string &path)
  
{

  clear();
  
  // open file

  try {
    _file.open(path, NcxxFile::read);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Era5File::isEra5File");
    _addErrStr("  Cannot open file for reading: ", path);
    _addErrStr("  exception: ", e.what());
    return -1;
  }

  // read dimensions
  
  if (_readDimensions()) {
    _file.close();
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "DEBUG - not Era5 NetCDF reanalysis file" << endl;
      cerr << _errStr << endl;
    }
    return false;
  }

  // file has the correct dimensions, so it is a CfRadial file
  
  _file.close();
  return true;

}

#ifdef JUNK

////////////////////////////////////////////////
// get the date and time from a dorade file path
// returns 0 on success, -1 on failure

int Era5File::getTimeFromPath(const string &path, DateTime &rtime)

{

  Path rpath(path);
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

#endif

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int Era5File::readFromPath(const string &path)
  
{
  
  if (_params.debug) {
    cerr << "Reading file: " << path << endl;
  }

  string errStr("ERROR - Era5File::readFromPath");

  // init
  
  clear();
  _nTimesInFile = 0;
  _nPoints = 0;
  
  // open file

  try {
    _file.open(path, NcxxFile::read);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Era5File::readFromPath");
    _addErrStr("  Cannot open file for reading: ", path);
    _addErrStr("  exception: ", e.what());
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

#ifdef JUNK
  
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

  if (_params.mhayman_specify_fields) {
    if (_readFieldVariablesSpecified()) {
      _addErrStr(errStr);
      return -1;
    }
  } else {
    if (_readFieldVariablesAuto()) {
      _addErrStr(errStr);
      return -1;
    }
  }

  // Convert any pressure fields to hPa

  _convertPressureToHpa();

  // load the data into the read volume

  _loadReadVolume();

  // close file
  
  _file.close();

  // clean up

  _rays.clear();
  _dataTimes.clear();
  _dTimes.clear();

#endif
  

  return 0;

}

///////////////////////////////////
// read in the dimensions

int Era5File::_readDimensions()

{

  // read required dimensions

  _nTimesInFile = 0;
  _nPoints = 0;

  try {
    
    _timeDim = _file.getDim("time");
    _nTimesInFile = _timeDim.getSize();

    _levelDim = _file.getDim("level");
    _nLevels = _levelDim.getSize();
    
    _latDim = _file.getDim("latitude");
    _nLat = _latDim.getSize();
    
    _lonDim = _file.getDim("longitude");
    _nLon = _lonDim.getSize();
    
  } catch (NcxxException &e) {

    _addErrStr("ERROR - Era5File::_readDimensions");
    _addErrStr("  exception: ", e.what());
    return -1;

  }

  _nPoints = _nLat * _nLon;

  return 0;

}

///////////////////////////////////
// read the global attributes

int Era5File::_readGlobalAttributes()

{

  // data source
  
  _dataSource.clear();
  try {
    NcxxGroupAtt att = _file.getAtt("DATA_SOURCE");
    _dataSource = att.asString();
  } catch (NcxxException& e) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - no dataSource global attribute found" << endl;
    }
  }
  
  // history
  
  _history.clear();
  try {
    NcxxGroupAtt att = _file.getAtt("history");
    _history = att.asString();
  } catch (NcxxException& e) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - no history global attribute found" << endl;
    }
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    if (_history.size() > 0) {
      cerr << "history: " << _history << endl;
    }
    if (_dataSource.size() > 0) {
      cerr << "dataSource: " << _dataSource << endl;
    }
  }

  return 0;

}

///////////////////////////////////
// read the times

int Era5File::_readTimes()

{

  _dataTimes.clear();
  _iTimes.clear();

  // read the time variable

  _timeVar = _file.getVar("time");
  if (_timeVar.isNull()) {
    _addErrStr("ERROR - Era5File::_readTimes");
    _addErrStr("  Cannot find 'time' variable");
    _addErrStr(_file.getErrStr());
    return -1;
  }
  if (_timeVar.getDimCount() < 1) {
    _addErrStr("ERROR - Era5File::_readTimes");
    _addErrStr("  time variable has no dimensions");
    return -1;
  }
  NcxxDim timeDim = _timeVar.getDim(0);
  if (timeDim != _timeDim) {
    _addErrStr("ERROR - Era5File::_readTimes");
    _addErrStr("  Time has incorrect dimension, name: ", timeDim.getName());
    return -1;
  }

  // get units attribute

  string units;
  try {
    NcxxVarAtt unitsAtt = _timeVar.getAtt("units");
    units = unitsAtt.asString();
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Era5File::_readTimes");
    _addErrStr("  Time has no units");
    return -1;
  }

  // parse the time units reference time

  int year, month, day, hour, min, sec;
  if (sscanf(units.c_str(),
             "hours since %4d-%2d-%2d %2d-%2d-%2d",
             &year, &month, &day, &hour, &min, &sec) != 6) {
    _addErrStr("ERROR - Era5File::_readTimes");
    _addErrStr("  Bad time units string: ", units);
  }
  _refTime.set(year, month, day, hour, min, sec);

  // read the time array
  
  _iTimes.resize(_nTimesInFile);
  try {
    _timeVar.getVal(_iTimes.data());
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Era5File::_readTimes");
    _addErrStr("  Cannot read times variable");
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  for (size_t ii = 0; ii < _nTimesInFile; ii++) {
    DateTime mtime = _refTime + _iTimes[ii] * 3600;
    _dataTimes.push_back(mtime);
  }

  return 0;

}

///////////////////////////////////
// read the lat/lon variables

int Era5File::_readLatLon()

{

  _lat.clear();
  _lon.clear();

  // latitude
  
  _latVar = _file.getVar("latitude");
  if (_latVar.isNull() || _latVar.numVals() < 1) {
    _addErrStr("ERROR - Era5File::_readLatLon");
    _addErrStr("  Cannot read latitude");
    _addErrStr(_file.getErrStr());
    return -1;
  }
  
  if (_latVar.getDimCount() != 1) {
    _addErrStr("ERROR - Era5File::_readLatLon");
    _addErrStr("  'latitude' is not 1-dimensional");
    return -1;
  }
    
  NcxxDim latDim = _latVar.getDim(0);
  if (latDim != _latDim) {
    _addErrStr("ERROR - Era5File::_readLatLon");
    _addErrStr("  latitude has incorrect dimension, name: ", _latVar.getName());
    return -1;
  }

  _lat.resize(_nLat);
  try {
    _latVar.getVal(_lat.data());
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Era5File::_readLatLon");
    _addErrStr("  getVal fails, cannot get latitude array, var name: ",
               _latVar.getName());
    return -1;
  }

  // longitude
  
  _lonVar = _file.getVar("longitude");
  if (_lonVar.isNull() || _lonVar.numVals() < 1) {
    _addErrStr("ERROR - Era5File::_readLatLon");
    _addErrStr("  Cannot read longitude");
    _addErrStr(_file.getErrStr());
    return -1;
  }
  
  if (_lonVar.getDimCount() != 1) {
    _addErrStr("ERROR - Era5File::_readLatLon");
    _addErrStr("  'longitude' is not 1-dimensional");
    return -1;
  }
    
  NcxxDim lonDim = _lonVar.getDim(0);
  if (lonDim != _lonDim) {
    _addErrStr("ERROR - Era5File::_readLatLon");
    _addErrStr("  longitude has incorrect dimension, name: ", _lonVar.getName());
    return -1;
  }

  _lon.resize(_nLon);
  try {
    _lonVar.getVal(_lon.data());
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Era5File::_readLatLon");
    _addErrStr("  getVal fails, cannot get longitude array, var name: ",
               _lonVar.getName());
    return -1;
  }

  return 0;

}

#ifdef JUNK

///////////////////////////////////
// read in ray variables

int Era5File::_readRayVariables()

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

  // _readRayVar("PITCH", _pitch);
  // if (_pitch.size() < _nTimesInFile) {
  //   _addErrStr("ERROR - PITCH variable required");
  //   iret = -1;
  // }

  _readRayVar("THDG", _heading);
  if (_heading.size() < _nTimesInFile) {
    _addErrStr("ERROR - THDG variable required");
    iret = -1;
  }

  // _readRayVar("PSXC", _pressure);
  // if (_pressure.size() < _nTimesInFile) {
  //   _addErrStr("ERROR - PSXC variable required");
  //   iret = -1;
  // }

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
    _addErrStr("ERROR - Era5File::_readRayVariables");
    return -1;
  }

  return 0;

}

///////////////////////////////////
// read a ray variable - double
// side effects: set var, vals

int Era5File::_readRayVar(const string &name, vector<double> &vals)
  
{

  vals.clear();

  // get var
  
  NcxxVar var;
  if (_getRayVar(var, name, true)) {
    _addErrStr("ERROR - Era5File::_readRayVar");
    return -1;
  }

  // load up data

  RadxArray<double> data_;
  double *data = data_.alloc(_nTimesInFile);
  try {
    var.getVal(data);
    for (size_t ii = 0; ii < _nTimesInFile; ii++) {
      vals.push_back(data[ii]);
    }
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Era5File::_readRayVar");
    _addErrStr("  Cannot read variable: ", name);
    _addErrStr(_file.getErrStr());
    return -1;
  }

  return 0;

}

///////////////////////////////////
// read a ray variable - float
// side effects: set var, vals

int Era5File::_readRayVar(const string &name, vector<float> &vals)
  
{

  vals.clear();

  // get var
  
  NcxxVar var;
  if (_getRayVar(var, name, true)) {
    _addErrStr("ERROR - Era5File::_readRayVar");
    return -1;
  }

  // load up data

  RadxArray<float> data_;
  float *data = data_.alloc(_nTimesInFile);
  try {
    var.getVal(data);
    for (size_t ii = 0; ii < _nTimesInFile; ii++) {
      vals.push_back(data[ii]);
    }
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Era5File::_readRayVar");
    _addErrStr("  Cannot read variable: ", name);
    _addErrStr(_file.getErrStr());
    return -1;
  }

  return 0;

}

///////////////////////////////////
// get a ray variable by name
// returns 0 on success, -1 on failure

int Era5File::_getRayVar(NcxxVar &var,
                           const string &name,
                           bool required)

{
  
  // get var
  
  var = _file.getVar(name);
  if (var.isNull()) {
    if (required) {
      _addErrStr("ERROR - Era5File::_getRayVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr(_file.getErrStr());
    }
    return -1;
  }

  // check time dimension
  
  if (var.getDimCount() < 1) {
    if (required) {
      _addErrStr("ERROR - Era5File::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no dimensions");
    }
    return -1;
  }

  NcxxDim timeDim = var.getDim(0);
  if (timeDim != _timeDim) {
    if (required) {
      _addErrStr("ERROR - Era5File::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has incorrect dimension, dim name: ", 
                 timeDim.getName());
      _addErrStr("  should be 'time'");
    }
    return -1;
  }

  return 0;

}

///////////////////////////////////
// create the rays to be read in
// and set meta data

int Era5File::_createRays(const string &path)

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
            float roll = geo.getRoll();
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
            float roll = geo.getRoll();
            if (std::isfinite(roll) && roll > -45.0 && roll < 45.0) {
              ray->setElevationDeg(-94.0 - geo.getRoll());
            }
          }
        }

        ray->setGeoref(geo);

      } // if (Era5File::readGeorefFromSpdb ...

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

void Era5File::_loadReadVolume()
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
// read the field variables automatically

int Era5File::_readFieldVariablesAuto()

{

  // initialize status xml

  _statusXml.clear();
  _statusXml += RadxXml::writeStartTag("FieldStatus", 0);
  bool gotStatus = false;

  // loop through the variables, adding data fields as appropriate
  
  const multimap<string, NcxxVar> &vars = _file.getVars();

  for (multimap<string, NcxxVar>::const_iterator iter = vars.begin();
       iter != vars.end(); iter++) {
    
    NcxxVar var = iter->second;
    if (var.isNull()) {
      continue;
    }
    
    _readFieldVariable(var.getName(), "", var, gotStatus);
    
  } // iter
  
  if (gotStatus) {
    _statusXml += RadxXml::writeEndTag("FieldStatus", 0);
  } else {
    _statusXml.clear();
  }

  return 0;

}

////////////////////////////////////////////
// read the field variables as specified in
// the parameter file

int Era5File::_readFieldVariablesSpecified()

{

  int iret = 0;

  // initialize status xml
  
  _statusXml.clear();
  _statusXml += RadxXml::writeStartTag("FieldStatus", 0);
  bool gotStatus = false;
  
  // loop through the variables as specified in the params file

  int nFields = _params.mhayman_fields_n;
  for (int ifield = 0; ifield < nFields; ifield++) {
  
    Params::mhayman_field_t &mfld = _params._mhayman_fields[ifield];
    NcxxVar var = _file.getVar(mfld.field_name);
    if (var.isNull()) {
      _addErrStr("ERROR - Era5File::_readFieldVariablesSpecified()");
      _addErrStr("  Cannot find specified field, name: ",
                 mfld.field_name);
      iret = -1;
      continue;
    }
    
    // check the type
    NcxxType ftype = var.getType();
    if (ftype != ncxxFloat && ftype != ncxxByte) {
      // not a valid type for field data
      _addErrStr("ERROR - Era5File::_readFieldVariablesSpecified()");
      _addErrStr("  Variable wrong type, name: ", mfld.field_name);
      _addErrStr("  Type must be float or byte");
      _addErrStr("  Type is: ", Ncxx::ncxxTypeToStr(ftype));
      iret = -1;
      continue;
    }

    // set output name
    
    string outputName(mfld.field_name);
    if (strlen(mfld.output_name) > 0) {
      outputName = mfld.output_name;
    }

    // check if we need to apply mask
    
    bool applyMask = false;
    if (mfld.apply_mask && (strlen(mfld.mask_name) > 0) && (ftype == ncxxFloat)) {
      applyMask = true;
    }

    // read in variable
    
    if (_readFieldVariable(var.getName(), "", var, gotStatus,
                           true, applyMask,
                           mfld.mask_name, 
                           mfld.mask_valid_value)) {
      _addErrStr("ERROR - Era5File::_readFieldVariablesSpecified()");
      iret = -1;
    }

  } // ivar

  if (gotStatus) {
    _statusXml += RadxXml::writeEndTag("FieldStatus", 0);
  } else {
    _statusXml.clear();
  }

  return iret;

}

////////////////////////////////////////////
// read in a field variable

int Era5File::_readFieldVariable(string inputName,
                                   string outputName,
                                   NcxxVar &var,
                                   bool &gotStatus,
                                   bool required /* = false */,
                                   bool applyMask /* = false */,
                                   const string maskName /* = "" */,
                                   int maskValidValue /* = 0 */)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "DEBUG - Era5File::_readFieldVariable" << endl;
    cerr << "  -->> adding field, input name: " << inputName << endl;
    if (outputName.size() > 0) {
      cerr << "  -->>              output name: " << outputName << endl;
    }
  }
  
  // set output name to input name if missing

  if (outputName.size() == 0) {
    outputName = inputName;
  }

  // check the type

  NcxxType ftype = var.getType();
  if (ftype != ncxxFloat && ftype != ncxxByte) {
    if (!required) {
      return 0;
    }
    // not a valid type for field data
    _addErrStr("ERROR - Era5File::_readFieldVariable");
    _addErrStr("  Variable wrong type, name: ", inputName);
    _addErrStr("  Type must be float or byte");
    _addErrStr("  Type is: ", Ncxx::ncxxTypeToStr(ftype));
    return -1;
  }
  
  int numDims = var.getDimCount();
  // we need fields with 2 dimensions
  if (numDims != 2) {
    if (!required) {
      return 0;
    }
    // not a valid type for field data
    _addErrStr("ERROR - Era5File::_readFieldVariable");
    _addErrStr("  Variable should have 2 dimensions, name: ", inputName);
    _addErrInt("  Number of dimensions: ", numDims);
    return -1;
  }
  
  // check that we have the correct time dimension
  
  NcxxDim timeDim = var.getDim(0);
  if (timeDim != _timeDim) {
    if (!required) {
      return 0;
    }
    _addErrStr("ERROR - Era5File::_readFieldVariable()");
    _addErrStr("  Variable does not have time dimension: ",
               inputName);
    _addErrStr("  First dim is: ", timeDim.getName());
    return -1;
  }

  // check whether this variable uses the normal range dimension
  // if not, it is probably a raw field with a longer dimension
  
  NcxxDim dim1 = var.getDim(1);
  bool isRawField = false;
  if (dim1 != _rangeDim) {
    isRawField = true;
    // must be a float field
    if (ftype != ncxxFloat) {
      if (!required) {
        return 0;
      }
      _addErrStr("ERROR - Era5File::_readFieldVariable");
      _addErrStr("  Variable must be of type float: ", inputName);
      return -1;
    }
  }

  // set names, units, etc
  
  string units;
  try {
    NcxxVarAtt unitsAtt = var.getAtt("units");
    NcxxType utype = unitsAtt.getType();
    unitsAtt.getValues(units);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Era5File::_readFieldVariable");
    _addErrStr("  Var has no units: ", inputName);
    _addErrStr("  ", e.whatStr());
    return -1;
  }

  string description;
  string procStatus;
  try {
    NcxxVarAtt descAtt = var.getAtt("description");
    description = descAtt.asString();
    NcxxVarAtt statusAtt = var.getAtt("ProcessingStatus");
    procStatus = statusAtt.asString();
  } catch (NcxxException& e) {
    if (_params.debug) {
      cerr << "WARNING - getting attributes for field: " << inputName << endl;
      cerr << "  " << e.whatStr() << endl;
    }
  }

  if (isRawField) {
    
    vector<int> maskVals;
    if (applyMask) {
      // read in mask vals
      if (_readMaskVar(maskName, maskVals)) {
        _addErrStr("ERROR - Era5File::_readFieldVariable");
        _addErrStr("  cannot read mask for field: ", inputName);
        _addErrStr("  mask field name: ", maskName);
        return -1;
      }
    }

    if (_addRawFieldToRays(var, outputName, units, description,
                           applyMask, maskVals, maskValidValue)) {
      _addErrStr("ERROR - Era5File::_readFieldVariable");
      _addErrStr("  cannot read raw field name: ", inputName);
      _addErrStr(_file.getErrStr());
      return -1;
    }
    
  } else if (applyMask) {
    
    // read in mask vals
    
    vector<int> maskVals;
    if (_readMaskVar(maskName, maskVals)) {
      _addErrStr("ERROR - Era5File::_readFieldVariable");
      _addErrStr("  cannot read mask for field: ", inputName);
      _addErrStr("  mask field name: ", maskName);
      return -1;
    }

    // use mask
    
    if (_addMaskedFieldToRays(var, outputName, units, description,
                              maskVals, maskValidValue)) {
      _addErrStr("ERROR - Era5File::_readFieldVariable");
      _addErrStr("  cannot read field name: ", inputName);
      _addErrStr(_file.getErrStr());
      return -1;
    }

  } else {
    
    // no mask
    
    if (ftype == ncxxFloat) {
      if (_addFl32FieldToRays(var, outputName, units, description)) {
        _addErrStr("ERROR - Era5File::_readFieldVariable");
        _addErrStr("  cannot read field name: ", inputName);
        _addErrStr(_file.getErrStr());
        return -1;
      }
    } else {
      if (_addSi08FieldToRays(var, outputName, units, description)) {
        _addErrStr("ERROR - Era5File::_readFieldVariable");
        _addErrStr("  cannot read field name: ", inputName);
        _addErrStr(_file.getErrStr());
        return -1;
      }
    }
    
  }
      
  // add processing status to statusXml, if appropriate
  
  if (procStatus.size() > 0) {
    _statusXml += RadxXml::writeStartTag("Field", 1);
    _statusXml += RadxXml::writeString("Name", 2, outputName);
    _statusXml += RadxXml::writeString("Description", 2, description);
    _statusXml += RadxXml::writeString("Status", 2, procStatus);
    _statusXml += RadxXml::writeEndTag("Field", 1);
    gotStatus = true;
  }
  
  return 0;

}

////////////////////////////////////////////
// get a mask field
// returns 0 on success, -1 on failure

int Era5File::_readMaskVar(const string &maskFieldName,
                             vector<int> &maskVals)

{

  // init

  maskVals.clear();

  // get var

  NcxxVar var = _file.getVar(maskFieldName);
  if (var.isNull()) {
    _addErrStr("ERROR - Era5File::_readMaskVar()");
    _addErrStr("  Cannot find mask field, name: ", maskFieldName);
    return -1;
  }
    
  // we need fields with 2 dimensions

  int numDims = var.getDimCount();
  if (numDims != 2) {
    _addErrStr("ERROR - Era5File::_getMaskVar()");
    _addErrStr("  Bad mask field: ", maskFieldName);
    _addErrStr("  first dim is not time");
    return -1;
  }

  // check the type
  NcxxType ftype = var.getType();
  if (ftype != ncxxByte) {
    // not a valid type for field data
    // not a valid type for field data
    _addErrStr("ERROR - Era5File::_getMaskVar()");
    _addErrStr("  Bad mask field: ", maskFieldName);
    _addErrStr("  Not an 8-bit byte field");
    return -1;
  }
  
  // check that we have the correct dimensions

  NcxxDim timeDim = var.getDim(0);
  if (timeDim != _timeDim) {
    _addErrStr("ERROR - Era5File::_getMaskVar()");
    _addErrStr("  Bad mask field: ", maskFieldName);
    _addErrStr("  first dim is not time");
    return -1;
  }
  
  NcxxDim rangeDim = var.getDim(1);
  if (rangeDim != _rangeDim) {
    _addErrStr("ERROR - Era5File::_getMaskVar()");
    _addErrStr("  Bad mask field: ", maskFieldName);
    _addErrStr("  second dim is not range");
    return -1;
  }
  
  // get data from array as bytes
  
  RadxArray<unsigned char> ndata_;
  unsigned char *ndata = ndata_.alloc(_nPoints);
  try {
    var.getVal(ndata);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Era5File::_readFieldVariablesAuto()");
    _addErrStr("  Time has no units");
    _addErrStr("  exception: ", e.what());
    return -1;
  }

  // load up vals
  
  for (int ii = 0; ii < _nPoints; ii++) {
    maskVals.push_back((int) ndata[ii]);
  }

  return 0;

}

//////////////////////////////////////////////////////////////
// Add double field to rays
// The _rays array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int Era5File::_addFl64FieldToRays(NcxxVar &var,
                                    const string &name,
                                    const string &units,
                                    const string &description)
  
{

  // check whether this variable uses the range dimension
  // if not, it is probably a raw field with a longer dimension
  
  NcxxDim dim1 = var.getDim(1);
  bool usesRangeDim = true;
  if (dim1 != _rangeDim) {
    usesRangeDim = false;
  }
  if (!usesRangeDim) {
    // read in special range variable for this variable
    const multimap<string, NcxxVar> &vars = _file.getVars();
    for (multimap<string, NcxxVar>::const_iterator iter = vars.begin();
         iter != vars.end(); iter++) {
      NcxxVar rvar = iter->second;
      if (rvar.isNull()) {
        continue;
      }
      int numDims = rvar.getDimCount();
      // we need fields with 1 dimension
      if (numDims != 1) {
        continue;
      }
      // check that we have the correct dimensions
      if (rvar.getDim(0) != dim1) {
        continue;
      }
    }
  } // 

  // get data from array
  
  RadxArray<Radx::fl64> ddata_;
  Radx::fl64 *ddata = ddata_.alloc(_nPoints);
  try {
    var.getVal(ddata);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Era5File::_addFl64FieldToRays");
    _addErrStr("  getVal fails, cannot get range data array, var name: ",
               name);
    return -1;
  }

  // set name
  
  string outName(name);
  string standardName;

  if (outName.find(_params.combined_hi_field_name) != string::npos) {
    outName = Names::CombinedHighCounts;
    standardName = Names::lidar_copolar_combined_backscatter_photon_count;
  } else if (outName.find(_params.combined_lo_field_name) != string::npos) {
    outName = Names::CombinedLowCounts;
    standardName = Names::lidar_copolar_combined_backscatter_photon_count;
  } else if (outName.find(_params.molecular_field_name) != string::npos) {
    outName = Names::MolecularCounts;
    standardName = Names::lidar_copolar_molecular_backscatter_photon_count;
  } else if (outName.find(_params.cross_field_name) != string::npos) {
    outName = Names::CrossPolarCounts;
    standardName = Names::lidar_crosspolar_combined_backscatter_photon_count;
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
    
    field->setStandardName(standardName);
    field->setLongName(description);
    field->copyRangeGeom(_geom);
    
  } // iray
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add float field to rays
// The _rays array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int Era5File::_addFl32FieldToRays(NcxxVar &var,
                                    const string &name,
                                    const string &units,
                                    const string &description)
  
{

  // check whether this variable uses the range dimension
  // if not, it is probably a raw field with a longer dimension
  
  NcxxDim dim1 = var.getDim(1);
  bool usesRangeDim = true;
  if (dim1 != _rangeDim) {
    usesRangeDim = false;
  }
  if (!usesRangeDim) {
    // read in special range variable for this variable
    const multimap<string, NcxxVar> &vars = _file.getVars();
    for (multimap<string, NcxxVar>::const_iterator iter = vars.begin();
         iter != vars.end(); iter++) {
      NcxxVar rvar = iter->second;
      if (rvar.isNull()) {
        continue;
      }
      int numDims = rvar.getDimCount();
      // we need fields with 1 dimension
      if (numDims != 1) {
        continue;
      }
      // check that we have the correct dimensions
      if (rvar.getDim(0) != dim1) {
        continue;
      }
    }
  } // 

  // get data from array
  
  RadxArray<Radx::fl32> ddata_;
  Radx::fl32 *ddata = ddata_.alloc(_nPoints);
  try {
    var.getVal(ddata);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Era5File::_addFl32FieldToRays");
    _addErrStr("  getVal fails, cannot get range data array, var name: ",
               name);
    return -1;
  }

  // set name
  
  string outName(name);
  string standardName;

  if (outName.find(_params.combined_hi_field_name) != string::npos) {
    outName = Names::CombinedHighCounts;
    standardName = Names::lidar_copolar_combined_backscatter_photon_count;
  } else if (outName.find(_params.combined_lo_field_name) != string::npos) {
    outName = Names::CombinedLowCounts;
    standardName = Names::lidar_copolar_combined_backscatter_photon_count;
  } else if (outName.find(_params.molecular_field_name) != string::npos) {
    outName = Names::MolecularCounts;
    standardName = Names::lidar_copolar_molecular_backscatter_photon_count;
  } else if (outName.find(_params.cross_field_name) != string::npos) {
    outName = Names::CrossPolarCounts;
    standardName = Names::lidar_crosspolar_combined_backscatter_photon_count;
  }
  
  // loop through the rays
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {

    // get data for ray
    
    int startIndex = iray * _nRangeInFile;
    Radx::fl32 *dd = ddata + startIndex;

    RadxField *field =
      _rays[iray]->addField(outName, units, _nRangeInFile,
                            Radx::missingFl32,
                            dd,
                            true);
    
    field->setStandardName(standardName);
    field->setLongName(description);
    field->copyRangeGeom(_geom);
    
  } // iray
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add raw field to rays
// raw fields have a different dimension
// Returns 0 on success, -1 on failure

int Era5File::_addRawFieldToRays(NcxxVar &var,
                                   const string &name,
                                   const string &units,
                                   const string &description,
                                   bool applyMask,
                                   const vector<int> &maskVals,
                                   int maskValidValue)
  
{
  
  // get the range dimension for this variable
  // this should differ from the main range dimension
  
  NcxxDim dim1 = var.getDim(1);
  NcxxVar rangeVar;
  // read in special range variable for this variable
  const multimap<string, NcxxVar> &vars = _file.getVars();
  for (multimap<string, NcxxVar>::const_iterator iter = vars.begin();
       iter != vars.end(); iter++) {
    NcxxVar rvar = iter->second;
    if (rvar.isNull()) {
      continue;
    }
    int numDims = rvar.getDimCount();
    // range field has only 1 dimension
    if (numDims != 1) {
      continue;
    }
    // check that we have the correct dimensions
    if (rvar.getDim(0) == dim1) {
      rangeVar = rvar;
      break;
    }
  } // iter
  if (rangeVar.isNull()) {
    _addErrStr("ERROR - Era5File::_addRawFieldToRays");
    _addErrStr("  Cannot find raw range for field name: ", name);
    _addErrStr("  Should be: ", dim1.getName());
    return -1;
  }

  NcxxDim rawRangeDim = rangeVar.getDim(0);
  int nRawRange = rawRangeDim.getSize();
  if (rawRangeDim.getSize() < _rangeDim.getSize()) {
    _addErrStr("ERROR - Era5File::_addRawFieldToRays");
    _addErrInt("  Raw range field too short, size: ",
               rawRangeDim.getSize());
    _addErrInt("  Should at least equal range dim, size: ",
               _rangeDim.getSize());
    return -1;
  }
  
  // load up range data
  
  vector<float> rawRange;
  NcxxType rtype = rangeVar.getType();
  if (rtype == ncxxFloat) {
    
    RadxArray<Radx::fl32> range_;
    Radx::fl32 *range = range_.alloc(nRawRange);
    try {
      rangeVar.getVal(range);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - Era5File::_addRawFieldToRays");
      _addErrStr("  Cannot read float range variable: ", rangeVar.getName());
      _addErrStr(_file.getErrStr());
      return -1;
    }
    
    for (int ii = 0; ii < nRawRange; ii++) {
      rawRange.push_back(range[ii]);
    }
  
  } else if (rtype == ncxxDouble) {

    RadxArray<Radx::fl64> range_;
    Radx::fl64 *range = range_.alloc(nRawRange);
    try {
      rangeVar.getVal(range);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - Era5File::_addRawFieldToRays");
      _addErrStr("  Cannot read float range variable: ", rangeVar.getName());
      _addErrStr(_file.getErrStr());
      return -1;
    }
    
    for (int ii = 0; ii < nRawRange; ii++) {
      rawRange.push_back(range[ii]);
    }
    
  } else {
    
    _addErrStr("ERROR - Era5File::_addRawFieldToRays");
    _addErrStr("  Bad type for range variable: ", rangeVar.getName());
    _addErrStr("  Should be float or float");
    return -1;

  }

  // get field data
  
  int nPoints = _nTimesInFile * nRawRange;
  RadxArray<Radx::fl64> ddata_;
  Radx::fl64 *ddata = ddata_.alloc(nPoints);
  
  NcxxType ftype = var.getType();
  if (rtype == ncxxFloat) {

    // read in as floats

    RadxArray<Radx::fl32> fdata_;
    Radx::fl32 *fdata = fdata_.alloc(nPoints);
    try {
      var.getVal(fdata);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - Era5File::_addRawFieldToRays");
      _addErrStr("  Cannot read float data for var: ", name);
      _addErrStr(_file.getErrStr());
      return -1;
    }
    
    // copy floats to doubles

    for (int ii = 0; ii < nPoints; ii++) {
      ddata[ii] = fdata[ii];
    }
  
  } else if (rtype == ncxxDouble) {

    try {
      var.getVal(ddata);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - Era5File::_addRawFieldToRays");
      _addErrStr("  Cannot read double data for var: ", name);
      _addErrStr(_file.getErrStr());
      return -1;
    }

  }
    
  // compute the indices of the range array relative to the main range array

  float startRangeInFile = _rangeKm[0] * 1000.0;
  int rawRangeStartIndex = -1;
  for (int ii = 0; ii < nRawRange; ii++) {
    if (fabs(startRangeInFile - rawRange[ii]) < 0.1) {
      rawRangeStartIndex = ii;
      break;
    }
  }
  if (rawRangeStartIndex < 0) {
    _addErrStr("ERROR - Era5File::_addRawFieldToRays");
    _addErrStr("  Cannot match raw range to file range array");
    return -1;
  }
  int rawRangeEndIndex = rawRangeStartIndex + _nRangeInFile;
  if (rawRangeEndIndex > nRawRange) {
    rawRangeEndIndex = nRawRange;
  }
  int nCopy = rawRangeEndIndex - rawRangeStartIndex;

  // apply mask?
  
  if (applyMask && ((int) maskVals.size() == _nPoints)) {
    for (size_t iray = 0; iray < _rays.size(); iray++) {
      int dataStartIndex = iray * nRawRange + rawRangeStartIndex;
      int maskStartIndex = iray * _nRangeInFile;
      for (int ii = 0; ii < nCopy; ii++) {
        if (maskVals[maskStartIndex + ii] != maskValidValue) {
          ddata[dataStartIndex + ii] = Radx::missingFl64;
        }
      } // ii
    } // iray
  } // if (applyMask && (maskValues.size() == _nPoints))

  // loop through the rays, copying in the raw data fields
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {

    // get data for ray
    
    int startIndex = iray * nRawRange + rawRangeStartIndex;
    Radx::fl64 *dd = ddata + startIndex;
    
    RadxField *field =
      _rays[iray]->addField(name, units, nCopy,
                            Radx::missingFl64,
                            dd,
                            true);
    
    field->setLongName(description);
    field->copyRangeGeom(_geom);
    
  } // iray
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add masked field to rays
// The _rays array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int Era5File::_addMaskedFieldToRays(NcxxVar &var,
                                      const string &name,
                                      const string &units,
                                      const string &description,
                                      vector<int> &maskVals,
                                      int maskValidValue)
  
{

  // get data from array
  
  RadxArray<Radx::fl64> ddata_;
  Radx::fl64 *ddata = ddata_.alloc(_nPoints);

  NcxxType ftype = var.getType();
  if (ftype == ncxxFloat) {
    
    // read in as floats
    
    RadxArray<Radx::fl32> fdata_;
    Radx::fl32 *fdata = fdata_.alloc(_nPoints);
    try {
      var.getVal(fdata);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - Era5File::_addMaskedFieldToRays");
      _addErrStr("  Cannot read float data for var: ", name);
      _addErrStr(_file.getErrStr());
      return -1;
    }
    
    // copy floats to doubles

    for (int ii = 0; ii < _nPoints; ii++) {
      ddata[ii] = fdata[ii];
    }
  
  } else if (ftype == ncxxDouble) {

    try {
      var.getVal(ddata);
    } catch (NcxxException& e) {
      _addErrStr("ERROR - Era5File::_addMaskedFieldToRays");
      _addErrStr("  Cannot read double data for var: ", name);
      _addErrStr(_file.getErrStr());
      return -1;
    }

  }

  // apply mask
  
  for (int ii = 0; ii < _nPoints; ii++) {
    if (ii > (int) maskVals.size() - 1) {
      break;
    }
    if (maskVals[ii] != maskValidValue) {
      ddata[ii] = Radx::missingFl64;
    }
  }

  // set name

  string outName(name);
  string standardName;
  
  if (outName.find(_params.combined_hi_field_name) != string::npos) {
    outName = Names::CombinedHighCounts;
    standardName = Names::lidar_copolar_combined_backscatter_photon_count;
  } else if (outName.find(_params.combined_lo_field_name) != string::npos) {
    outName = Names::CombinedLowCounts;
    standardName = Names::lidar_copolar_combined_backscatter_photon_count;
  } else if (outName.find(_params.molecular_field_name) != string::npos) {
    outName = Names::MolecularCounts;
    standardName = Names::lidar_copolar_molecular_backscatter_photon_count;
  } else if (outName.find(_params.cross_field_name) != string::npos) {
    outName = Names::CrossPolarCounts;
    standardName = Names::lidar_crosspolar_combined_backscatter_photon_count;
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
    
    field->setStandardName(standardName);
    field->setLongName(description);
    field->copyRangeGeom(_geom);
    
  } // iray
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add int8 field to rays - this will be a mask
// The _rays array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int Era5File::_addSi08FieldToRays(NcxxVar &var,
                                    const string &name,
                                    const string &units,
                                    const string &description)
  
{

  // get data from array as bytes

  RadxArray<unsigned char> ndata_;
  unsigned char *ndata = ndata_.alloc(_nPoints);
  
  try {
    var.getVal(ndata);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Era5File::_addSi08FieldToRays");
    _addErrStr("  Cannot read float data for var: ", name);
    _addErrStr(_file.getErrStr());
    return -1;
  }
  
  // convert to si08 

  RadxArray<Radx::si08> sdata_;
  Radx::si08 *sdata = sdata_.alloc(_nPoints);
  for (int ii = 0; ii < _nPoints; ii++) {
    sdata[ii] = (int) ndata[ii];
  }
  
  // loop through the rays
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {

    // get data for ray
    
    int startIndex = iray * _nRangeInFile;
    Radx::si08 *sd = sdata + startIndex;
    
    RadxField *field =
      _rays[iray]->addField(name, units, _nRangeInFile,
                            Radx::missingSi08,
                            sd, 1.0, 0.0,
                            true);
    
    field->setComment(description);
    field->copyRangeGeom(_geom);
    
  }
  
  return 0;
  
}

#endif

///////////////////////////////////////////////
// add labelled integer value to error string,
// with optional following carriage return.

void Era5File::_addErrInt(string label, int iarg, bool cr)
{
  _errStr += label;
  char str[32];
  sprintf(str, "%d", iarg);
  _errStr += str;
  if (cr) {
    _errStr += "\n";
  }
}

///////////////////////////////////////////////
// add labelled double value to error string,
// with optional following carriage return.
// Default format is %g.

void Era5File::_addErrDbl(string label, double darg,
                            string format, bool cr)
  
{
  _errStr += label;
  char str[128];
  sprintf(str, format.c_str(), darg);
  _errStr += str;
  if (cr) {
    _errStr += "\n";
  }
}

////////////////////////////////////////
// add labelled string to error string
// with optional following carriage return.

void Era5File::_addErrStr(string label, string strarg, bool cr)

{
  _errStr += label;
  _errStr += strarg;
  if (cr) {
    _errStr += "\n";
  }
}

