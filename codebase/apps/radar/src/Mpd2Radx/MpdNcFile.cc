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
// MpdNcFile.cc
//
// MPD NetCDF data produced by Mpd Haymann's python code
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2018
//
///////////////////////////////////////////////////////////////

#include "MpdNcFile.hh"
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxRemap.hh>
#include <Radx/RadxRangeGeom.hh>
#include <Radx/RadxXml.hh>
#include <Ncxx/NcxxVar.hh>
#include <Spdb/DsSpdb.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
#include <set>
using namespace std;

//////////////
// Constructor

MpdNcFile::MpdNcFile(const Params &params) :
        _params(params)
        
{
  
  clear();

}

/////////////
// destructor

MpdNcFile::~MpdNcFile()

{
  _file.close();
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void MpdNcFile::clear()
  
{

  clearErrStr();
  
  _timeDim.setNull();
  _rangeDim.setNull();

  _nTimesInFile = 0;
  _nRangeInFile = 0;

  _history.clear();
  _project.clear();
  _statusXml.clear();

  _dataTimes.clear();
  _dTimes.clear();

  _rangeKm.clear();
  _geom.setRangeGeom(0.0075, 154.2432);

  _clearRayVariables();

  _instrumentType = Radx::INSTRUMENT_TYPE_LIDAR;
  _platformType = Radx::PLATFORM_TYPE_FIXED;
  _primaryAxis = Radx::PRIMARY_AXIS_Z_PRIME;

  _rays.clear();
  
}

////////////////////////////////////////////////////////////
// Check if this is a Mpd type file
// Returns true on success, false on failure

bool MpdNcFile::isMpdNcFile(const string &path)
  
{

  clear();
  
  // open file

  try {
    _file.open(path, NcxxFile::read);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - MpdNcFile::isMpdNcFile");
    _addErrStr("  Cannot open file for reading: ", path);
    _addErrStr("  exception: ", e.what());
    return -1;
  }

  // read dimensions
  
  if (_readDimensions()) {
    _file.close();
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "DEBUG - not Mpd-type MPD file" << endl;
      cerr << _errStr << endl;
    }
    return false;
  }

  // file has the correct dimensions, so it is a CfRadial file
  
  _file.close();
  return true;

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int MpdNcFile::readFromPath(const string &path,
                            RadxVol &vol)
  
{
  
  if (_params.debug) {
    cerr << "Reading file: " << path << endl;
  }

  string errStr("ERROR - MpdNcFile::readFromPath");

  _readVol = &vol;
  _readVol->clear();

  // init
  
  clear();
  _nTimesInFile = 0;
  _nRangeInFile = 0;
  _rays.clear();

  // open file

  try {
    _file.open(path, NcxxFile::read);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - MpdNcFile::readFromPath");
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
  
  // read range variable
  
  if (_readRange()) {
    _addErrStr(errStr);
    return -1;
  }

  // read in ray qualifier fields

  if (_params.include_qualifier_fields) {
    if (_readRayQualifierFields()) {
      return -1;
    }
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

  if (_params.specify_output_fields) {
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

  return 0;

}

///////////////////////////////////
// read in the dimensions

int MpdNcFile::_readDimensions()

{

  // read required dimensions

  _nTimesInFile = 0;
  _nRangeInFile = 0;

  try {
    
    _timeDim = _file.getDim("time");
    _nTimesInFile = _timeDim.getSize();

    _rangeDim = _file.getDim("range");
    _nRangeInFile = _rangeDim.getSize();
    
  } catch (NcxxException &e) {

    _addErrStr("ERROR - MpdNcFile::_readDimensions");
    _addErrStr("  exception: ", e.what());
    return -1;

  }

  _nPoints = _nTimesInFile * _nRangeInFile;

  return 0;

}

///////////////////////////////////
// read the global attributes

int MpdNcFile::_readGlobalAttributes()

{
  
  const std::multimap<std::string, NcxxGroupAtt> attMap = _file.getAtts();
  for (auto ii = attMap.begin(); ii != attMap.end(); ii++) {

    string attName = ii->first;
    NcxxGroupAtt att = ii->second;

    if (att.getName() == "history") {

      _history = att.asString();

    } else if (att.getName() == "description") {

      _readVol->setComment(att.asString());

    } else if (att.getName() == "Project") {

      _project = att.asString();

    } else if (att.getName() == "latitude") {

      double latDeg;
      att.getValues(&latDeg);
      _readVol->setLatitudeDeg(latDeg);

    } else if (att.getName() == "longitude") {

      double lonDeg;
      att.getValues(&lonDeg);
      _readVol->setLongitudeDeg(lonDeg);

    } else if (att.getName() == "elevation") {

      double altM;
      att.getValues(&altM);
      _readVol->setAltitudeKm(altM / 1000.0);

    } else if (att.getName() == "MPD_Number") {

      int mpdNum;
      att.getValues(&mpdNum);
      char text[1024];
      snprintf(text, 1024, "MPD-number-%d", mpdNum);
      _readVol->setInstrumentName(text);

    } else {

      NcxxType attType = att.getType();
      if (attType == ncxxByte ||
          attType == ncxxShort ||
          attType == ncxxInt ||
          attType == ncxxUbyte ||
          attType == ncxxUshort ||
          attType == ncxxUint ||
          attType == ncxxInt64 ||
          attType == ncxxUint64) {
        // int attribute
        int ival;
        att.getValues(&ival);
        char text[1024];
        snprintf(text, 1024, "%d", ival);
        _readVol->addUserGlobAttr(att.getName(),
                                  RadxVol::UserGlobAttr::ATTR_INT,
                                  text);
      } else if (attType == ncxxFloat || attType == ncxxDouble) {
        // float attribute
        double dval;
        att.getValues(&dval);
        char text[1024];
        snprintf(text, 1024, "%g", dval);
        _readVol->addUserGlobAttr(att.getName(),
                                  RadxVol::UserGlobAttr::ATTR_DOUBLE,
                                  text);
      } else {
        // string attribute
        _readVol->addUserGlobAttr(att.getName(),
                                  RadxVol::UserGlobAttr::ATTR_STRING,
                                  att.asString());
      } // if (attType == ncxxByte ...

    } // if (att.getName() == "history") ...

  } // ii
  
  return 0;

}

///////////////////////////////////
// read the times

int MpdNcFile::_readTimes()

{

  _dataTimes.clear();
  _dTimes.clear();

  // read the time variable

  _timeVar = _file.getVar("time");
  if (_timeVar.isNull()) {
    _addErrStr("ERROR - MpdNcFile::_readTimes");
    _addErrStr("  Cannot find 'time' variable");
    _addErrStr(_file.getErrStr());
    return -1;
  }
  if (_timeVar.getDimCount() < 1) {
    _addErrStr("ERROR - MpdNcFile::_readTimes");
    _addErrStr("  time variable has no dimensions");
    return -1;
  }
  NcxxDim timeDim = _timeVar.getDim(0);
  if (timeDim != _timeDim) {
    _addErrStr("ERROR - MpdNcFile::_readTimes");
    _addErrStr("  Time has incorrect dimension, name: ", timeDim.getName());
    return -1;
  }

  // get units attribute

  string units;
  try {
    NcxxVarAtt unitsAtt = _timeVar.getAtt("units");
    units = unitsAtt.asString();
  } catch (NcxxException& e) {
    _addErrStr("ERROR - MpdNcFile::_readTimes");
    _addErrStr("  Time has no units");
    return -1;
  }

  // parse the time units reference time

  RadxTime stime(units);

  // read the time array
  
  RadxArray<double> dtimes_;
  double *dtimes = dtimes_.alloc(_nTimesInFile);
  try {
    _timeVar.getVal(dtimes);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - MpdNcFile::_readTimes");
    _addErrStr("  Cannot read times variable");
    _addErrStr("  exception: ", e.what());
    return -1;
  }
  for (size_t ii = 0; ii < _nTimesInFile; ii++) {
    float dt = dtimes[ii];
    _dTimes.push_back(dt);
    RadxTime rayTime(stime + dt);
    _dataTimes.push_back(rayTime);
  }

  return 0;

}

///////////////////////////////////
// read the range variable

int MpdNcFile::_readRange()

{

  _rangeVar = _file.getVar("range");
  if (_rangeVar.isNull() || _rangeVar.numVals() < 1) {
    _addErrStr("ERROR - MpdNcFile::_readRangeVariable");
    _addErrStr("  Cannot read range");
    _addErrStr(_file.getErrStr());
    return -1;
  }
  _rangeKm.clear();
  _nRangeInFile = _rangeDim.getSize();

  if (_rangeVar.getDimCount() != 1) {
    _addErrStr("ERROR - MpdNcFile::_readRangeVariable");
    _addErrStr("  'range' is not 1-dimensional");
    return -1;
  }
    
  NcxxDim rangeDim = _rangeVar.getDim(0);
  if (rangeDim != _rangeDim) {
    _addErrStr("ERROR - MpdNcFile::_readRangeVariable");
    _addErrStr("  Range has incorrect dimension, name: ", rangeDim.getName());
    return -1;
  }
  
  // get units

  string units = "m";
  try {
    NcxxVarAtt unitsAtt = _timeVar.getAtt("units");
    units = unitsAtt.asString();
  } catch (NcxxException& e) {
    // no range units
  }
  
  _rangeKm.clear();
  RadxArray<double> rangeMeters_;
  double *rangeMeters = rangeMeters_.alloc(_nRangeInFile);
  try {
    _rangeVar.getVal(rangeMeters);
    double *rr = rangeMeters;
    for (size_t ii = 0; ii < _nRangeInFile; ii++, rr++) {
      if (units == "km") {
        _rangeKm.push_back(*rr);
      } else {
        // meters
        _rangeKm.push_back(*rr / 1000.0);
      }
    }
  } catch (NcxxException& e) {
    _addErrStr("ERROR - MpdNcFile::_readRangeVariable");
    _addErrStr("  getVal fails, cannot get range data array, var name: ",
               rangeDim.getName());
    return -1;
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
// read in ray qualifier fields

int MpdNcFile::_readRayQualifierFields()

{

  // create a map for the required fields
  
  map<string, Params::mpd_field_t> qualFields;
  if (_params.specify_qualifier_fields) {
    for (int ii = 0; ii < _params.qualifier_fields_n; ii++) {
      qualFields[_params._qualifier_fields[ii].mpd_name] = _params._qualifier_fields[ii];
    }
  }
  
  const std::multimap<std::string, NcxxVar> &vars = _file.getVars();
  for (auto ii = vars.begin(); ii != vars.end(); ii++) {
    
    string varName = ii->first;
    const NcxxVar &var = ii->second;

    // check we have 1 dimension, time

    if (var.getDimCount() != 1) {
      continue;
    }
    if (var.getDim(0) != _timeDim) {
      continue;
    }

    // check if we need this field
    
    auto qualIndex = qualFields.find(varName);
    if (qualFields.size() == 0 || qualIndex != qualFields.end()) {
      
      // yes - add this field

      // set attributes
      
      string outputName = varName;
      string longName;
      string standardName;
      string ancillaryVariables;
      
      try {
        NcxxVarAtt longNameAtt = var.getAtt("descripion");
        longName = longNameAtt.asString();
      } catch (NcxxException& e) { }
      
      string units;
      try {
        NcxxVarAtt unitsAtt = var.getAtt("units");
        units = unitsAtt.asString();
      } catch (NcxxException& e) { }

      // try {
      //   NcxxVarAtt ancillaryVariablesAtt = var.getAtt("ancillary_variables");
      //   ancillaryVariables = ancillaryVariablesAtt.asString();
      // } catch (NcxxException& e) { }
      
      // override from params
      
      if (qualFields.size() != 0) {
        const Params::mpd_field_t &fieldParams = qualIndex->second;
        if (strlen(fieldParams.output_name) > 0) {
          outputName = fieldParams.output_name;
        }
        if (strlen(fieldParams.units) > 0) {
          units = fieldParams.units;
        }
        if (strlen(fieldParams.cf_standard_name) > 0) {
          standardName = fieldParams.cf_standard_name;
        }
      }
      
      NcxxType varType = var.getType();
      if (varType == ncxxDouble) {

        // double field

        if (_addFl64FieldToRays(var, outputName, units,
                                longName, standardName,
                                ancillaryVariables, true)) {
          _addErrStr("ERROR - MpdNcFile::_readRayQualifierFields");
          _addErrStr("  Cannot add field: ", outputName);
          return -1;
        }

      } else if (varType == ncxxFloat) {

        // float field

        if (_addFl32FieldToRays(var, outputName, units,
                                longName, standardName,
                                ancillaryVariables, true)) {
          _addErrStr("ERROR - MpdNcFile::_readRayQualifierFields");
          _addErrStr("  Cannot add field: ", outputName);
          return -1;
        }

      } else if (varType == ncxxByte ||
                 varType == ncxxShort ||
                 varType == ncxxInt ||
                 varType == ncxxUbyte ||
                 varType == ncxxUshort ||
                 varType == ncxxUint ||
                 varType == ncxxInt64 ||
                 varType == ncxxUint64) {
        
        // int field
        
        if (_addSi32FieldToRays(var, outputName, units,
                                longName, standardName, 
                                ancillaryVariables, true)) {
          _addErrStr("ERROR - MpdNcFile::_readRayQualifierFields");
          _addErrStr("  Cannot add field: ", outputName);
          return -1;
        }

      } // if (varType == ncxxDouble ...
      
    } // if (qualFields.size() == 0 ...

  } // ii

  return 0;

}

///////////////////////////////////
// clear the ray variables

void MpdNcFile::_clearRayVariables()

{

  _nSamples.clear();

  // _polAngle.clear();

  // _telescopeDirection.clear();

  // _lat.clear();
  // _lon.clear();
  // _alt.clear();
  // _roll.clear();
  // _pitch.clear();
  // _heading.clear();
  // _pressure.clear();
  // _tas.clear();
  // _temp.clear();

}

///////////////////////////////////
// read in ray variables

int MpdNcFile::_readRayVariables()

{

  _clearRayVariables();
  int iret = 0;

  if (strlen(_params.n_samples_field_name) > 0) {
    _readRayVar(_params.n_samples_field_name, _nSamples);
    if (_nSamples.size() < _nTimesInFile) {
      _addErrStr("WARNING - reading n_samples, field name: ", 
                 _params.n_samples_field_name);
      for (size_t ii = 0; ii < _nTimesInFile; ii++) {
        _nSamples.push_back(-9999.0);
      }
    }
  } else {
    for (size_t ii = 0; ii < _nTimesInFile; ii++) {
      _nSamples.push_back(-9999.0);
    }
  }

  // _readRayVar("TelescopeDirection", _telescopeDirection);
  // if (_telescopeDirection.size() < _nTimesInFile) {
  //   _addErrStr("ERROR - TelescopeLocked variable required");
  //   iret = -1;
  // }

  // _readRayVar("GGLAT", _lat);
  // if (_lat.size() < _nTimesInFile) {
  //   _addErrStr("ERROR - GGLAT variable required");
  //   iret = -1;
  // }

  // _readRayVar("GGLON", _lon);
  // if (_lon.size() < _nTimesInFile) {
  //   _addErrStr("ERROR - GGLON variable required");
  //   iret = -1;
  // }

  // _readRayVar("GGALT", _alt);
  // if (_alt.size() < _nTimesInFile) {
  //   _addErrStr("ERROR - GGALT variable required");
  //   iret = -1;
  // }

  // _readRayVar("ROLL", _roll);
  // if (_roll.size() < _nTimesInFile) {
  //   _addErrStr("ERROR - ROLL variable required");
  //   iret = -1;
  // }

  // _readRayVar("PITCH", _pitch);
  // if (_pitch.size() < _nTimesInFile) {
  //   _addErrStr("ERROR - PITCH variable required");
  //   iret = -1;
  // }

  // _readRayVar("THDG", _heading);
  // if (_heading.size() < _nTimesInFile) {
  //   _addErrStr("ERROR - THDG variable required");
  //   iret = -1;
  // }

  // _readRayVar("PSXC", _pressure);
  // if (_pressure.size() < _nTimesInFile) {
  //   _addErrStr("ERROR - PSXC variable required");
  //   iret = -1;
  // }

  // _readRayVar("TASX", _tas);
  // if (_tas.size() < _nTimesInFile) {
  //   _addErrStr("ERROR - TASX variable required");
  //   iret = -1;
  // }

  // _readRayVar("ATX", _temp);
  // if (_temp.size() < _nTimesInFile) {
  //   _addErrStr("ERROR - ATX variable required");
  //   iret = -1;
  // }

  if (iret) {
    _addErrStr("ERROR - MpdNcFile::_readRayVariables");
  }

  return iret;

}

///////////////////////////////////
// read a ray variable - double
// side effects: set var, vals

int MpdNcFile::_readRayVar(const string &name, vector<double> &vals)
  
{

  vals.clear();
  for (size_t ii = 0; ii < _nTimesInFile; ii++) {
    vals.push_back(-9999.0);
  }
  return 0;
  
  // get var
  
  NcxxVar var;
  if (_getRayVar(var, name, true)) {
    _addErrStr("ERROR - MpdNcFile::_readRayVar");
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
    _addErrStr("ERROR - MpdNcFile::_readRayVar");
    _addErrStr("  Cannot read variable: ", name);
    _addErrStr(_file.getErrStr());
    return -1;
  }

  return 0;

}

///////////////////////////////////
// read a ray variable - float
// side effects: set var, vals

int MpdNcFile::_readRayVar(const string &name, vector<float> &vals)
  
{

  vals.clear();
  for (size_t ii = 0; ii < _nTimesInFile; ii++) {
    vals.push_back((float) -9999.0);
  }
  return 0;

  // get var
  
  NcxxVar var;
  if (_getRayVar(var, name, true)) {
    _addErrStr("ERROR - MpdNcFile::_readRayVar");
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
    _addErrStr("ERROR - MpdNcFile::_readRayVar");
    _addErrStr("  Cannot read variable: ", name);
    _addErrStr(_file.getErrStr());
    return -1;
  }

  return 0;

}

///////////////////////////////////
// get a ray variable by name
// returns 0 on success, -1 on failure

int MpdNcFile::_getRayVar(NcxxVar &var,
                          const string &name,
                          bool required)

{
  
  // get var
  
  var = _file.getVar(name);
  if (var.isNull()) {
    if (required) {
      _addErrStr("ERROR - MpdNcFile::_getRayVar");
      _addErrStr("  Cannot read variable, name: ", name);
      _addErrStr(_file.getErrStr());
    }
    return -1;
  }

  // check time dimension
  
  if (var.getDimCount() < 1) {
    if (required) {
      _addErrStr("ERROR - MpdNcFile::_getRayVar");
      _addErrStr("  variable name: ", name);
      _addErrStr("  variable has no dimensions");
    }
    return -1;
  }

  NcxxDim timeDim = var.getDim(0);
  if (timeDim != _timeDim) {
    if (required) {
      _addErrStr("ERROR - MpdNcFile::_getRayVar");
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

int MpdNcFile::_createRays(const string &path)

{

  // compile a list of the rays to be read in
  
  _rays.clear();

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

    // pointing up
    
    ray->setAzimuthDeg(0.0);
    ray->setElevationDeg(90.0);
    ray->setFixedAngleDeg(90.0);
     
    if (_nSamples[ii] > 0) {
      ray->setNSamples((int) (_nSamples[ii] + 0.5));
    }
    
    // add to ray vector
    
    _rays.push_back(ray);

  } // ii

  return 0;

}

/////////////////////////////////////////////////////////
// load up the read volume with the data from this object

void MpdNcFile::_loadReadVolume()
{

  _readVol->setOrigFormat("MPD-RAW");
  _readVol->setVolumeNumber(-9999);
  _readVol->setInstrumentType(_instrumentType);
  _readVol->setInstrumentName("MPD");
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

  _readVol->setTitle("NCAR EOL MPD");
  _readVol->setSource("MPD software");
  _readVol->setHistory(_history);
  _readVol->setInstitution("NCAR");
  _readVol->setReferences("");
  _readVol->setDriver("Mpd2Radx");
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

/////////////////////////////////////////////////////////
// Convert any pressure fields to HPa

void MpdNcFile::_convertPressureToHpa()
{

  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RadxRay *ray = _rays[iray];
    vector<RadxField *> flds = ray->getFields();
    for (size_t ifield = 0; ifield < flds.size(); ifield++) {
      RadxField *field = flds[ifield];
      if (field->getUnits() == "Pa") {
        field->convertToFl64(); 
        Radx::fl64 *data = field->getDataFl64();
        for (size_t ii = 0; ii < field->getNPoints(); ii++) {
          data[ii] = data[ii] / 100.0;
        }
        field->setUnits("hPa");
      }
    }
  } // iray

}

////////////////////////////////////////////
// read the field variables automatically

int MpdNcFile::_readFieldVariablesAuto()

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
    
    _readFieldVariable(var.getName(), "", "", var, gotStatus);
    
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

int MpdNcFile::_readFieldVariablesSpecified()

{

  int iret = 0;

  // initialize status xml
  
  _statusXml.clear();
  _statusXml += RadxXml::writeStartTag("FieldStatus", 0);
  bool gotStatus = false;
  
  // loop through the variables as specified in the params file

  int nFields = _params.mpd_fields_n;
  for (int ifield = 0; ifield < nFields; ifield++) {
  
    Params::mpd_field_t &mfld = _params._mpd_fields[ifield];
    NcxxVar var = _file.getVar(mfld.mpd_name);
    if (var.isNull()) {
      _addErrStr("ERROR - MpdNcFile::_readFieldVariablesSpecified()");
      _addErrStr("  Cannot find specified field, name: ",
                 mfld.mpd_name);
      iret = -1;
      continue;
    }
    
    // check the type
    NcxxType ftype = var.getType();
    if (ftype != ncxxFloat && ftype != ncxxDouble) {
      // not a valid type for field data
      _addErrStr("ERROR - MpdNcFile::_readFieldVariablesSpecified()");
      _addErrStr("  Variable wrong type, name: ", mfld.mpd_name);
      _addErrStr("  Type must be float or double");
      _addErrStr("  Type is: ", Ncxx::ncxxTypeToStr(ftype));
      iret = -1;
      continue;
    }

    // set output name
    
    string outputName(mfld.mpd_name);
    if (strlen(mfld.output_name) > 0) {
      outputName = mfld.output_name;
    }
    string standardName(mfld.cf_standard_name);

    // read in variable
    
    if (_readFieldVariable(var.getName(), outputName, standardName, var, gotStatus)) {
      _addErrStr("ERROR - MpdNcFile::_readFieldVariablesSpecified()");
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

int MpdNcFile::_readFieldVariable(string inputName,
                                  string outputName,
                                  string standardName,
                                  NcxxVar &var,
                                  bool &gotStatus,
                                  bool required /* = false */)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "DEBUG - MpdNcFile::_readFieldVariable" << endl;
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
  if (ftype != ncxxFloat && ftype != ncxxDouble) {
    if (!required) {
      return 0;
    }
    // not a valid type for field data
    _addErrStr("ERROR - MpdNcFile::_readFieldVariable");
    _addErrStr("  Variable wrong type, name: ", inputName);
    _addErrStr("  Type must be float or double");
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
    _addErrStr("ERROR - MpdNcFile::_readFieldVariable");
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
    _addErrStr("ERROR - MpdNcFile::_readFieldVariable()");
    _addErrStr("  Variable does not have time dimension: ",
               inputName);
    _addErrStr("  First dim is: ", timeDim.getName());
    return -1;
  }

  // check whether this variable uses the normal range dimension
  // if not, it is probably a raw field with a longer dimension
  
  NcxxDim dim1 = var.getDim(1);
  if (dim1 != _rangeDim) {
    // must be a float field
    if (ftype != ncxxFloat) {
      if (!required) {
        return 0;
      }
      _addErrStr("ERROR - MpdNcFile::_readFieldVariable");
      _addErrStr("  Variable must be of type float: ", inputName);
      return -1;
    }
  }

  // attributes
  
  string units;
  try {
    NcxxVarAtt unitsAtt = var.getAtt("units");
    NcxxType utype = unitsAtt.getType();
    unitsAtt.getValues(units);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - MpdNcFile::_readFieldVariable");
    _addErrStr("  Var has no units: ", inputName);
    _addErrStr("  ", e.whatStr());
    return -1;
  }

  string description;
  try {
    NcxxVarAtt descAtt = var.getAtt("description");
    description = descAtt.asString();
  } catch (NcxxException& e) {
    if (_params.debug) {
      cerr << "WARNING - getting attributes for field: " << inputName << endl;
      cerr << "  " << e.whatStr() << endl;
    }
  }

  string ancillaryVariables;
  try {
    NcxxVarAtt descAtt = var.getAtt("ancillary_variables");
    ancillaryVariables = descAtt.asString();
  } catch (NcxxException& e) {
    if (_params.debug) {
      cerr << "WARNING - getting attributes for field: " << inputName << endl;
      cerr << "  " << e.whatStr() << endl;
    }
  }

  // no mask
  
  if (ftype == ncxxDouble) {
    if (_addFl32FieldToRays(var, outputName, units, description, standardName, 
                            ancillaryVariables, false)) {
      _addErrStr("ERROR - MpdNcFile::_readFieldVariable");
      _addErrStr("  cannot read field name: ", inputName);
      _addErrStr(_file.getErrStr());
      return -1;
    }
  } else if (ftype == ncxxFloat) {
    if (_addFl32FieldToRays(var, outputName, units, description, standardName, 
                            ancillaryVariables, false)) {
      _addErrStr("ERROR - MpdNcFile::_readFieldVariable");
      _addErrStr("  cannot read field name: ", inputName);
      _addErrStr(_file.getErrStr());
      return -1;
    }
  } else {
    if (_addSi08FieldToRays(var, outputName, units, description)) {
      _addErrStr("ERROR - MpdNcFile::_readFieldVariable");
      _addErrStr("  cannot read field name: ", inputName);
      _addErrStr(_file.getErrStr());
      return -1;
    }
  }
  
  return 0;

}

////////////////////////////////////////////
// get a mask field
// returns 0 on success, -1 on failure

int MpdNcFile::_readMaskVar(const string &maskFieldName,
                            vector<int> &maskVals)

{

  // init

  maskVals.clear();

  // get var

  NcxxVar var = _file.getVar(maskFieldName);
  if (var.isNull()) {
    _addErrStr("ERROR - MpdNcFile::_readMaskVar()");
    _addErrStr("  Cannot find mask field, name: ", maskFieldName);
    return -1;
  }
    
  // we need fields with 2 dimensions

  int numDims = var.getDimCount();
  if (numDims != 2) {
    _addErrStr("ERROR - MpdNcFile::_getMaskVar()");
    _addErrStr("  Bad mask field: ", maskFieldName);
    _addErrStr("  first dim is not time");
    return -1;
  }

  // check the type
  NcxxType ftype = var.getType();
  if (ftype != ncxxByte) {
    // not a valid type for field data
    // not a valid type for field data
    _addErrStr("ERROR - MpdNcFile::_getMaskVar()");
    _addErrStr("  Bad mask field: ", maskFieldName);
    _addErrStr("  Not an 8-bit byte field");
    return -1;
  }
  
  // check that we have the correct dimensions

  NcxxDim timeDim = var.getDim(0);
  if (timeDim != _timeDim) {
    _addErrStr("ERROR - MpdNcFile::_getMaskVar()");
    _addErrStr("  Bad mask field: ", maskFieldName);
    _addErrStr("  first dim is not time");
    return -1;
  }
  
  NcxxDim rangeDim = var.getDim(1);
  if (rangeDim != _rangeDim) {
    _addErrStr("ERROR - MpdNcFile::_getMaskVar()");
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
    _addErrStr("ERROR - MpdNcFile::_readFieldVariablesAuto()");
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

int MpdNcFile::_addFl64FieldToRays(const NcxxVar &var,
                                   const string &name,
                                   const string &units,
                                   const string &longName,
                                   const string &standardName,
                                   const string &ancillaryVariables,
                                   bool isQualifier)
  
{

  // get data from array
  
  size_t nData = _nPoints;
  if (isQualifier) {
    nData = _nTimesInFile;
  }
  RadxArray<Radx::fl64> ddata_;
  Radx::fl64 *ddata = ddata_.alloc(nData);
  try {
    var.getVal(ddata);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - MpdNcFile::_addFl64FieldToRays");
    _addErrStr("  getVal fails, cannot get range data array, var name: ",
               name);
    return -1;
  }
  
  // loop through the rays
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {

    // get data for ray
    
    int startIndex = iray * _nRangeInFile;
    int nGates = _nRangeInFile;
    if (isQualifier) {
      startIndex = iray;
      nGates = 1;
    }

    Radx::fl64 *dd = ddata + startIndex;
    RadxField *field =
      _rays[iray]->addField(name, units, nGates,
                            Radx::missingFl64,
                            dd, true, isQualifier);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);
    
  } // iray
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add float field to rays
// The _rays array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int MpdNcFile::_addFl32FieldToRays(const NcxxVar &var,
                                   const string &name,
                                   const string &units,
                                   const string &longName,
                                   const string &standardName,
                                   const string &ancillaryVariables,
                                   bool isQualifier)
  
{

  // get data from array
  
  size_t nData = _nPoints;
  if (isQualifier) {
    nData = _nTimesInFile;
  }
  RadxArray<Radx::fl32> fdata_;
  Radx::fl32 *fdata = fdata_.alloc(nData);
  try {
    var.getVal(fdata);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - MpdNcFile::_addFl32FieldToRays");
    _addErrStr("  getVal fails, cannot get range data array, var name: ",
               name);
    return -1;
  }
  
  // loop through the rays
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {

    // get data for ray
    
    int startIndex = iray * _nRangeInFile;
    int nGates = _nRangeInFile;
    if (isQualifier) {
      startIndex = iray;
      nGates = 1;
    }

    Radx::fl32 *ff = fdata + startIndex;
    RadxField *field =
      _rays[iray]->addField(name, units, nGates,
                            Radx::missingFl32,
                            ff, true, isQualifier);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);
    
  } // iray
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add int field to rays
// The _rays array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int MpdNcFile::_addSi32FieldToRays(const NcxxVar &var,
                                   const string &name,
                                   const string &units,
                                   const string &longName,
                                   const string &standardName,
                                   const string &ancillaryVariables,
                                   bool isQualifier)
  
{

  // get data from array
  
  size_t nData = _nPoints;
  if (isQualifier) {
    nData = _nTimesInFile;
  }
  RadxArray<Radx::si32> idata_;
  Radx::si32 *idata = idata_.alloc(nData);
  try {
    var.getVal(idata);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - MpdNcFile::_addSi32FieldToRays");
    _addErrStr("  getVal fails, cannot get range data array, var name: ",
               name);
    return -1;
  }
  
  // loop through the rays
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {

    // get data for ray
    
    int startIndex = iray * _nRangeInFile;
    int nGates = _nRangeInFile;
    if (isQualifier) {
      startIndex = iray;
      nGates = 1;
    }

    Radx::si32 *id = idata + startIndex;
    RadxField *field =
      _rays[iray]->addField(name, units, nGates,
                            Radx::missingSi32,
                            id, 1.0, 0.0,
                            true, isQualifier);
    
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);
    
  } // iray
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add raw field to rays
// raw fields have a different dimension
// Returns 0 on success, -1 on failure

int MpdNcFile::_addRawFieldToRays(NcxxVar &var,
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
    _addErrStr("ERROR - MpdNcFile::_addRawFieldToRays");
    _addErrStr("  Cannot find raw range for field name: ", name);
    _addErrStr("  Should be: ", dim1.getName());
    return -1;
  }

  NcxxDim rawRangeDim = rangeVar.getDim(0);
  int nRawRange = rawRangeDim.getSize();
  if (rawRangeDim.getSize() < _rangeDim.getSize()) {
    _addErrStr("ERROR - MpdNcFile::_addRawFieldToRays");
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
      _addErrStr("ERROR - MpdNcFile::_addRawFieldToRays");
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
      _addErrStr("ERROR - MpdNcFile::_addRawFieldToRays");
      _addErrStr("  Cannot read float range variable: ", rangeVar.getName());
      _addErrStr(_file.getErrStr());
      return -1;
    }
    
    for (int ii = 0; ii < nRawRange; ii++) {
      rawRange.push_back(range[ii]);
    }
    
  } else {
    
    _addErrStr("ERROR - MpdNcFile::_addRawFieldToRays");
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
      _addErrStr("ERROR - MpdNcFile::_addRawFieldToRays");
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
      _addErrStr("ERROR - MpdNcFile::_addRawFieldToRays");
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
    _addErrStr("ERROR - MpdNcFile::_addRawFieldToRays");
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

int MpdNcFile::_addMaskedFieldToRays(NcxxVar &var,
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
      _addErrStr("ERROR - MpdNcFile::_addMaskedFieldToRays");
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
      _addErrStr("ERROR - MpdNcFile::_addMaskedFieldToRays");
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
  
  // if (outName.find(_params.combined_hi_mpd_name) != string::npos) {
  //   outName = Names::CombinedHighCounts;
  //   standardName = Names::lidar_copolar_combined_backscatter_photon_count;
  // } else if (outName.find(_params.combined_lo_mpd_name) != string::npos) {
  //   outName = Names::CombinedLowCounts;
  //   standardName = Names::lidar_copolar_combined_backscatter_photon_count;
  // } else if (outName.find(_params.molecular_mpd_name) != string::npos) {
  //   outName = Names::MolecularCounts;
  //   standardName = Names::lidar_copolar_molecular_backscatter_photon_count;
  // } else if (outName.find(_params.cross_mpd_name) != string::npos) {
  //   outName = Names::CrossPolarCounts;
  //   standardName = Names::lidar_crosspolar_combined_backscatter_photon_count;
  // }
  
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

int MpdNcFile::_addSi08FieldToRays(NcxxVar &var,
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
    _addErrStr("ERROR - MpdNcFile::_addSi08FieldToRays");
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


///////////////////////////////////////////////
// add labelled integer value to error string,
// with optional following carriage return.

void MpdNcFile::_addErrInt(string label, int iarg, bool cr)
{
  Radx::addErrInt(_errStr, label, iarg, cr);
}

///////////////////////////////////////////////
// add labelled double value to error string,
// with optional following carriage return.
// Default format is %g.

void MpdNcFile::_addErrDbl(string label, double darg,
                           string format, bool cr)
  
{
  Radx::addErrDbl(_errStr, label, darg, format, cr);
}

////////////////////////////////////////
// add labelled string to error string
// with optional following carriage return.

void MpdNcFile::_addErrStr(string label, string strarg, bool cr)

{
  Radx::addErrStr(_errStr, label, strarg, cr);
}

void MpdNcFile::_clearRays()
{
  for (int ii = 0; ii < (int) _rays.size(); ii++) {
    delete _rays[ii];
  }
  _rays.clear();
}

