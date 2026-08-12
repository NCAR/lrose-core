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
  _nPointsPlane = 0;
  _nPointsVol = 0;

  _dataSource.clear();
  _history.clear();
  _datasetUrl.clear();
  _datasetDoi.clear();

  _dataTimes.clear();
  _iTimes.clear();
  _timeUnitsMultiplierSecs = 3600;

  _fieldData.clear();
  _fieldNames.clear();
  _fields.clear();
  
}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// If timeIndex is negative, do not read field data.
// If timeIndex is 0 or positive, read for that time index.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int Era5File::readFromPath(const string &path,
                           int timeIndex)
  
{

  string errStr("ERROR - Era5File::readFromPath");

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Reading file: " << path << endl;
  }

  _pathInUse = path;
  _timeIndex = timeIndex;
  
  // init
  
  clear();
  _nTimesInFile = 0;
  _nPointsPlane = 0;
  _nPointsVol = 0;
  
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

  // read lat lon variables
  
  if (_readLatLon()) {
    _addErrStr(errStr);
    return -1;
  }

  // read level
  
  if (_readLevels()) {
    _addErrStr(errStr);
    return -1;
  }

  // discover field variables
  
  if (_readFieldsMetadata()) {
    _addErrStr(errStr);
    return -1;
  }

  // read field data if time index non-negative
  
  if (timeIndex >= 0) {
    if (_readField(timeIndex)) {
      _addErrStr(errStr);
      return -1;
    }
  }

  // print data in debug mode
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    printData(std::cerr);
  }
  
  // close file
  
  _file.close();
  
  return 0;

}

///////////////////////////////////
// read in the dimensions

int Era5File::_readDimensions()

{

  int iret = 0;
  
  _nTimesInFile = 0;
  _nLevels = 0;
  _nLat = 0;
  _nLon = 0;
  _nPointsPlane = 0;
  _nPointsVol = 0;

  try {

    // Time dimension: accept either "time" or "valid_time"

    _timeDim = _file.getDim(_params.time_name);
    if (_timeDim.isNull() && !strcmp(_params.time_name, "time")) {
      _timeDim = _file.getDim("valid_time");
    }
    if (_timeDim.isNull()) {
      _addErrStr("ERROR - Era5File::_readDimensions");
      _addErrStr("Cannot find time dimension named: ", _params.time_name);
      iret = -1;
    } else {
      _nTimesInFile = _timeDim.getSize();
    }
    
    // Vertical dimension: accept either "level" or "pressure_level"

    _levelDim = _file.getDim(_params.level_name);
    if (_levelDim.isNull() && !strcmp(_params.level_name, "level")) {
      _levelDim = _file.getDim("pressure_level");
    }
    if (_levelDim.isNull()) {
      _addErrStr("ERROR - Era5File::_readDimensions");
      _addErrStr("Cannot find level dimension named: ", _params.level_name);
      iret = -1;
    } else {
      _nLevels = _levelDim.getSize();
    }

    // Horizontal dimensions

    _latDim = _file.getDim(_params.latitude_name);
    if (_latDim.isNull()) {
      _addErrStr("ERROR - Era5File::_readDimensions");
      _addErrStr("Cannot find latitude dimension named: ", _params.latitude_name);
      iret = -1;
    } else {
      _nLat = _latDim.getSize();
    }

    _lonDim = _file.getDim(_params.longitude_name);
    if (_lonDim.isNull()) {
      _addErrStr("ERROR - Era5File::_readDimensions");
      _addErrStr("Cannot find longitude dimension named: ", _params.longitude_name);
      iret = -1;
    } else {
      _nLon = _lonDim.getSize();
    }

  } catch (const NcxxException &ee) {

    _addErrStr("ERROR - Era5File::_readDimensions");
    _addErrStr("  exception: ", ee.what());
    iret = -1;

  }

  if (iret == 0) {
    _nPointsPlane = _nLat * _nLon;
    _nPointsVol = _nPointsPlane * _nLevels;
  }

  return iret;
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
      cerr << "NOTE - no dataSource global attribute found" << endl;
      cerr << "This is not required" << endl;
    }
  }
  
  // history
  
  _history.clear();
  try {
    NcxxGroupAtt att = _file.getAtt("history");
    _history = att.asString();
  } catch (NcxxException& e) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "NOTE - no history global attribute found" << endl;
      cerr << "This is not required" << endl;
    }
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
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

  string timeName = _timeDim.getName();
  _timeVar = _file.getVar(timeName);
  if (_timeVar.isNull()) {
    _addErrStr("ERROR - Era5File::_readTimes");
    _addErrStr("  Cannot find time variable, name: ", timeName);
    _addErrStr(_file.getErrStr());
    return -1;
  }
  if (_timeVar.getDimCount() < 1) {
    _addErrStr("ERROR - Era5File::_readTimes");
    _addErrStr("  time variable has no dimensions, name: ", _timeVar.getName());
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
             "hours since %4d-%2d-%2d %2d:%2d:%2d",
             &year, &month, &day, &hour, &min, &sec) == 6) {
    _refTime.set(year, month, day, hour, min, sec);
  } else if (sscanf(units.c_str(),
                    "hours since %4d-%2d-%2d",
                    &year, &month, &day) == 3) {
    hour = min = sec = 0;
    _refTime.set(year, month, day, hour, min, sec);
    _timeUnitsMultiplierSecs = 3600;
  } else if (sscanf(units.c_str(),
                    "seconds since %4d-%2d-%2d %2d:%2d:%2d",
                    &year, &month, &day, &hour, &min, &sec) == 6) {
    _refTime.set(year, month, day, hour, min, sec);
    _timeUnitsMultiplierSecs = 1;
  } else if (sscanf(units.c_str(),
                    "seconds since %4d-%2d-%2d",
                    &year, &month, &day) == 3) {
    hour = min = sec = 0;
    _refTime.set(year, month, day, hour, min, sec);
    _timeUnitsMultiplierSecs = 1;
  } else {
    _addErrStr("ERROR - Era5File::_readTimes");
    _addErrStr("  Bad time units string: ", units);
    return -1;
  }

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
    DeltaTime delTime((long) (_iTimes[ii] * _timeUnitsMultiplierSecs));
    DateTime mtime = _refTime + delTime;
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
  
  _latVar = _file.getVar(_params.latitude_name);
  if (_latVar.isNull() || _latVar.numVals() < 1) {
    _addErrStr("ERROR - Era5File::_readLatLon");
    _addErrStr("  Cannot read latitude var, name: ", _params.latitude_name);
    _addErrStr(_file.getErrStr());
    return -1;
  }
  
  if (_latVar.getDimCount() != 1) {
    _addErrStr("ERROR - Era5File::_readLatLon");
    _addErrStr("  latitude is not 1-dimensional, var name: ", _latVar.getName());
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
  
  _lonVar = _file.getVar(_params.longitude_name);
  if (_lonVar.isNull() || _lonVar.numVals() < 1) {
    _addErrStr("ERROR - Era5File::_readLatLon");
    _addErrStr("  Cannot read longitude var, name: ", _params.longitude_name);
    _addErrStr(_file.getErrStr());
    return -1;
  }
  
  if (_lonVar.getDimCount() != 1) {
    _addErrStr("ERROR - Era5File::_readLatLon");
    _addErrStr("  longitude is not 1-dimensional, var name: ", _lonVar.getName());
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

///////////////////////////////////////
// read the levels

int Era5File::_readLevels()

{

  _levels.clear();
  
  string levelName = _levelDim.getName();
  _levelVar = _file.getVar(levelName);
  if (_levelVar.isNull() || _levelVar.numVals() < 1) {
    _addErrStr("ERROR - Era5File::_readLevel");
    _addErrStr("  Cannot read level var, name: ", _params.level_name);
    _addErrStr(_file.getErrStr());
    return -1;
  }
  
  if (_levelVar.getDimCount() != 1) {
    _addErrStr("ERROR - Era5File::_readLevel");
    _addErrStr("  level var is not 1-dimensional, name: ", _levelVar.getName());
    return -1;
  }
  
  NcxxDim levelDim = _levelVar.getDim(0);
  if (levelDim != _levelDim) {
    _addErrStr("ERROR - Era5File::_readLevel");
    _addErrStr("  level var, name: ", levelName);
    _addErrStr("  has incorrect dimension, name: ", _levelVar.getName());
    return -1;
  }
  
  _levels.resize(_nLevels);
  try {
    _levelVar.getVal(_levels.data());
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Era5File::_readLevel");
    _addErrStr("  getVal fails, cannot get level array, var name: ",
               _levelVar.getName());
    return -1;
  }

  return 0;

}

/////////////////////////////////////////
// read field metadata

int Era5File::_readFieldsMetadata()

{

  _fieldNames.clear();
  _fields.clear();
  _fieldName.clear();
  _longName.clear();
  _shortName.clear();
  _units.clear();
  _fillValue = -9999.0;
  _minValue = -1.0e33;
  _maxValue = 1.0e33;
  
  const multimap<string, NcxxVar> &vars = _file.getVars();

  for (multimap<string, NcxxVar>::const_iterator iter = vars.begin();
       iter != vars.end(); iter++) {
    
    NcxxVar var = iter->second;
    if (var.isNull() || !_isFieldVariable(var)) {
      continue;
    }

    field_t field;
    if (_readFieldMetadata(var.getName(), var, field)) {
      return -1;
    }
    
    _fieldNames.push_back(field.fieldName);
    _fields.push_back(field);
    
  } // iter

  if (_fields.size() == 0) {
    _addErrStr("ERROR - Era5File::_readFieldsMetadata");
    _addErrStr("  No 4-D float field variables found");
    return -1;
  }

  _fieldName = _fields[0].fieldName;
  _longName = _fields[0].longName;
  _shortName = _fields[0].shortName;
  _units = _fields[0].units;
  _fillValue = _fields[0].fillValue;
  _minValue = _fields[0].minValue;
  _maxValue = _fields[0].maxValue;
  _datasetUrl = _fields[0].datasetUrl;
  _datasetDoi = _fields[0].datasetDoi;

  return 0;

}

/////////////////////////////////////////
// read field for a specified time index

int Era5File::_readField(int timeIndex)

{

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    
    NcxxVar var = _file.getVar(_fields[ii].fieldName);
    if (var.isNull()) {
      _addErrStr("ERROR - Era5File::_readField");
      _addErrStr("  Cannot find field variable: ", _fields[ii].fieldName);
      return -1;
    }
    
    if (_readFieldVariable(var.getName(), timeIndex, var, _fields[ii])) {
      return -1;
    }
    
  } // ii

  _fieldData = _fields[0].data;

  return 0;

}

////////////////////////////////////////////
// check whether this is a data field variable

bool Era5File::_isFieldVariable(NcxxVar &var)
  
{

  // check the type
  
  NcxxType ftype = var.getType();
  if (ftype != ncxxFloat) {
    return false;
  }
  
  int numDims = var.getDimCount();
  // we need fields with 4 dimensions
  if (numDims != 4) {
    return false;
  }
  
  // check that we have the correct dimensions
  
  NcxxDim timeDim = var.getDim(0);
  if (timeDim != _timeDim) {
    return false;
  }
  
  NcxxDim levelDim = var.getDim(1);
  if (levelDim != _levelDim) {
    return false;
  }
  
  NcxxDim latDim = var.getDim(2);
  if (latDim != _latDim) {
    return false;
  }
  
  NcxxDim lonDim = var.getDim(3);
  if (lonDim != _lonDim) {
    return false;
  }

  return true;

}

////////////////////////////////////////////
// read field metadata

int Era5File::_readFieldMetadata(string fieldName,
                                 NcxxVar &var,
                                 field_t &field)
  
{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "DEBUG - Era5File::_readFieldMetadata" << endl;
    cerr << "  -->> adding field, input name: " << fieldName << endl;
  }

  field.fieldName = var.getName();
  
  // set names, units, etc from attributes

  try {
    NcxxVarAtt att = var.getAtt("long_name");
    att.getValues(field.longName);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Era5File::_readFieldMetadata");
    _addErrStr("  Var has no long_name: ", fieldName);
    _addErrStr("  ", e.whatStr());
    return -1;
  }

  try {
    NcxxVarAtt att = var.getAtt("short_name");
    att.getValues(field.shortName);
  } catch (NcxxException& e) {
    // _addErrStr("WARNING - Era5File::_readFieldMetadata");
    // _addErrStr("  Var has no short_name: ", fieldName);
    field.shortName = fieldName;
  }

  try {
    NcxxVarAtt att = var.getAtt("units");
    att.getValues(field.units);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Era5File::_readFieldMetadata");
    _addErrStr("  Var has no units: ", fieldName);
    _addErrStr("  ", e.whatStr());
    return -1;
  }

  try {
    NcxxVarAtt att = var.getAtt("_FillValue");
    att.getValues(&field.fillValue);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Era5File::_readFieldMetadata");
    _addErrStr("  Var has no _FillValue: ", fieldName);
    _addErrStr("  ", e.whatStr());
    return -1;
  }

  field.minValue = -1.0e33;
  try {
    NcxxVarAtt att = var.getAtt("minimum_value");
    att.getValues(&field.minValue);
  } catch (NcxxException& e) {
  }

  field.maxValue = 1.0e33;
  try {
    NcxxVarAtt att = var.getAtt("maximum_value");
    att.getValues(&field.maxValue);
  } catch (NcxxException& e) {
  }

  field.datasetUrl.clear();
  try {
    NcxxVarAtt att = var.getAtt("rda_dataset_url");
    att.getValues(field.datasetUrl);
  } catch (NcxxException& e) {
  }

  field.datasetDoi.clear();
  try {
    NcxxVarAtt att = var.getAtt("rda_dataset_doi");
    att.getValues(field.datasetDoi);
  } catch (NcxxException& e) {
  }

  return 0;

}

////////////////////////////////////////////
// read in a field variable

int Era5File::_readFieldVariable(string fieldName,
                                 int timeIndex,
                                 NcxxVar &var,
                                 field_t &field)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "DEBUG - Era5File::_readFieldVariable" << endl;
    cerr << "  -->> adding field, input name: " << fieldName << endl;
    cerr << "  -->> timeIndex: " << timeIndex << endl;
  }
  
  if (!_isFieldVariable(var)) {
    return -1;
  }

  // set starting location in each dimension

  vector<size_t> start;
  start.push_back(timeIndex);
  start.push_back(0);
  start.push_back(0);
  start.push_back(0);

  // set count in each dimension

  vector<size_t> count;
  count.push_back(1);
  count.push_back(_nLevels);
  count.push_back(_nLat);
  count.push_back(_nLon);

  // get the data
  
  field.data.resize(_nPointsVol);
  try {
    var.getVal(start, count, field.data.data());
  } catch (NcxxException& e) {
    _addErrStr("ERROR - Era5File::_readFieldVariable");
    _addErrStr("  getVal fails, var name: ",
               var.getName());
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////
// print data details after read

void Era5File::printData(ostream &out)
{

  out << "========================================================" << endl;
  
  out << "ERA5 file: " << _pathInUse << endl;
  out << "  _nTimesInFile: " << _nTimesInFile << endl;
  out << "  _nLevels: " << _nLevels << endl;
  out << "  _nLat: " << _nLat << endl;
  out << "  _nLon: " << _nLon << endl;
  out << "  _nPointsPlane: " << _nPointsPlane << endl;
  out << "  _nPointsVol: " << _nPointsVol << endl;
  
  out << "  _refTime: " << _refTime.asString() << endl;
  for (size_t ii = 0; ii < _iTimes.size(); ii++) {
    out << "    ii, itime, dataTime: "
        << ii << ", "
        << _iTimes[ii] << ", "
        << _dataTimes[ii].asString() << endl;
  }
  
  out << "=========>> lats: ";
  for (size_t ii = 0; ii < _lat.size(); ii++) {
    out << _lat[ii] << ", ";
  }
  out << endl;
  
  out << "=========>> lons: ";
  for (size_t ii = 0; ii < _lon.size(); ii++) {
    out << _lon[ii] << ", ";
  }
  out << endl;
  
  if (_timeIndex >= 0) {
    out << "=============== field ===============" << endl;
    out << "  time: " << _dataTimes[_timeIndex].asString() << endl;
    out << "  fieldName: " << _fieldName << endl;
    out << "  longName: " << _longName << endl;
    out << "  shortName: " << _shortName << endl;
    out << "  units: " << _units << endl;
    out << "  fillValue: " << _fillValue << endl;
    out << "  minValue: " << _minValue << endl;
    out << "  maxValue: " << _maxValue << endl;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      int nPrint = _fieldData.size();
      if (nPrint > 500) {
        nPrint = 500;
      }
      out << "=========>> printing first " << nPrint << " data entries" << endl;
      for (int ii = 0; ii < nPrint; ii++) {
        out << _fieldData[ii];
        if (ii != nPrint - 1) {
          out << ", ";
        }
      }
      out << endl;
    }
    out << "=============== field ===============" << endl;
  }
  
  out << "========================================================" << endl;
  
}

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
