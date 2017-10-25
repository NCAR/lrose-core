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

  _nTimesInFile = 0;
  _timeVecSize = 0;

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
  
}

////////////////////////////////////////////////////////////
// Open file
// Returns true on success, false on failure

int RawFile::openFile(const string &path)
  
{
  
  _file.close();

  // open file
  
  if (_file.openRead(path)) {
    cerr << "ERROR - RawFile::openFile()" << endl;
    cerr << "  Cannot open file: " << path << endl;
    cerr << _file.getErrStr() << endl;
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// Close file

void RawFile::closeFile()
  
{
  _file.close();
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
// Opens specified path
// Reads times
// Remains open, ready for use
// Returns 0 on success, -1 on failure

int RawFile::openAndReadTimes(const string &path)
  
{
  
  if (_params.debug) {
    cerr << "Open file: " << path << endl;
  }
  
  // clear tmp rays
  
  _nTimesInFile = 0;
  
  // open file

  if (_file.openRead(path)) {
    cerr << "ERROR - RawFile::openAndReadTimes()" << endl;
    cerr << "  Cannot open file: " << path << endl;
    cerr << _file.getErrStr() << endl;
    return -1;
  }
  
  // read dimensions
  
  if (_readDimensions()) {
    cerr << "ERROR - RawFile::openAndReadTimes()" << endl;
    cerr << "  Cannot read dimensions: " << path << endl;
    cerr << _file.getErrStr() << endl;
    return -1;
  }

  // read global attributes
  
  if (_readGlobalAttributes()) {
    cerr << "ERROR - RawFile::openAndReadTimes()" << endl;
    cerr << "  Cannot read global attributes: " << path << endl;
    cerr << _file.getErrStr() << endl;
    return -1;
  }

  // read time variable
  
  if (_readTimes()) {
    cerr << "ERROR - RawFile::openAndReadTimes()" << endl;
    cerr << "  Cannot read times: " << path << endl;
    cerr << _file.getErrStr() << endl;
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////
// get the index for a given time
// returns -1 on error

int RawFile::getTimeIndex(time_t timeVal)
{

  if (_dataTimes.size() < 1) {
    return -1;
  }

  // get closest time

  RadxTime vtime(timeVal);
  double minDiff = 1.0e99;
  int minIndex = -1;
  for (size_t ii = 0; ii < _dataTimes.size(); ii++) {
    double diff = fabs(vtime - _dataTimes[ii]);
    if (diff < minDiff) {
      minDiff = diff;
      minIndex = ii;
    }
  }

  return minIndex;

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

////////////////////////////////////////
// append to monitoring stats
// returns 0 on success, -1 on failure

int RawFile::appendMonStats(MonField &monField,
                            int startTimeIndex,
                            int endTimeIndex)
{
  
  int iret = 0;
  if (monField.getQualifier().size() == 0) {

    vector<double> dvals;
    string longName, units;
    if (_readRayVar2Doubles(monField.getName(), dvals, longName, units) == 0) {
      for (int itime = startTimeIndex; itime <= endTimeIndex; itime++) {
        double dval = dvals[itime];
        if (dval >= monField.getMinValidValue() &&
            dval <= monField.getMaxValidValue()) {
          monField.addValue(dvals[itime]);
          monField.setLongName(longName);
          monField.setUnits(units);
        }
      }
    }

  }

  return iret;

}


////////////////////////////////////////
// read a ray variable, put into doubles
// side effects: set var, vals

int RawFile::_readRayVar2Doubles(const string &name,
                                 vector<double> &dvals,
                                 string &longName,
                                 string &units)
{
  
  Nc3Var *var = _getRayVar(name);
  
  if (var == NULL) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - RawFile::_readRayVar2Doubles" << endl;
      cerr << "  Cannot find var: " << name << endl;
    }
    return -1;
  }

  int iret = 0;
  Nc3Type varType = var->type();
  switch (varType) {
    case nc3Double: {
      if (_readRayVar(var, name, dvals)) {
        iret = -1;
      }
      break;
    }
    case nc3Float: {
      vector<float> fvals;
      if (_readRayVar(var, name, fvals)) {
        iret = -1;
      } else {
        for (size_t ii = 0; ii < fvals.size(); ii++) {
          dvals.push_back(fvals[ii]);
        }
      }
      break;
    }
    case nc3Int: {
      vector<int> ivals;
      if (_readRayVar(var, name, ivals)) {
        iret = -1;
      } else {
        for (size_t ii = 0; ii < ivals.size(); ii++) {
          dvals.push_back(ivals[ii]);
        }
      }
      break;
    }
    case nc3Short: {
      vector<short> svals;
      if (_readRayVar(var, name, svals)) {
        iret = -1;
      } else {
        for (size_t ii = 0; ii < svals.size(); ii++) {
          dvals.push_back(svals[ii]);
        }
      }
      break;
    }
    default: {
      cerr << "ERROR - RawFile::_readRayVar2Doubles" << endl;
      cerr << "  Bad type for var: " << name << endl;
      cerr << "  type: " << varType << endl;
      return -1;
    }
  } // switch

  if (iret == 0) {
    Nc3Att *longNameAtt = var->get_att("long_name");
    if (longNameAtt) {
      string sval = Nc3xFile::asString(longNameAtt);
      if (sval.size() > 0) {
        longName = sval;
      }
      delete longNameAtt;
    } else {
      Nc3Att *descAtt = var->get_att("description");
      if (descAtt) {
        string sval = Nc3xFile::asString(descAtt);
        if (sval.size() > 0) {
          longName = sval;
        }
        delete descAtt;
      }
    }
    Nc3Att *unitsAtt = var->get_att("units");
    if (unitsAtt) {
      string sval = Nc3xFile::asString(unitsAtt);
      if (sval.size() > 0) {
        units = sval;
      }
      delete unitsAtt;
    }
  }

  return iret;
  
}

///////////////////////////////////
// read a ray variable - double
// side effects: set var, vals

int RawFile::_readRayVar(Nc3Var* &var, const string &name,
                         vector<double> &vals)
  
{

  vals.clear();

  // get var
  
  var = _getRayVar(name);
  if (var == NULL) {
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
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - RawFile::_readRayVar" << endl;
      cerr << "  Cannot read variable: " << name << endl;
      cerr << _file.getNc3Error()->get_errmsg() << endl;
    }
    iret = -1;
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a ray variable - float
// side effects: set var, vals

int RawFile::_readRayVar(Nc3Var* &var, const string &name,
                         vector<float> &vals)
  
{

  vals.clear();

  // get var

  var = _getRayVar(name);
  if (var == NULL) {
    return -1;
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
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - RawFile::_readRayVar" << endl;
      cerr << "  Cannot read variable: " << name << endl;
      cerr << _file.getNc3Error()->get_errmsg() << endl;
    }
    iret = -1;
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a ray variable - integer
// side effects: set var, vals

int RawFile::_readRayVar(Nc3Var* &var, const string &name,
                         vector<int> &vals)
  
{

  vals.clear();

  // get var
  
  var = _getRayVar(name);
  if (var == NULL) {
    return -1;
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
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - RawFile::_readRayVar" << endl;
      cerr << "  Cannot read variable: " << name << endl;
      cerr << _file.getNc3Error()->get_errmsg() << endl;
    }
    iret = -1;
  }
  delete[] data;
  return iret;

}

///////////////////////////////////
// read a ray variable - short
// side effects: set var, vals

int RawFile::_readRayVar(Nc3Var* &var, const string &name,
                         vector<short> &vals)
  
{

  vals.clear();

  // get var
  
  var = _getRayVar(name);
  if (var == NULL) {
    return -1;
  }

  // load up data

  short *data = new short[_nTimesInFile];
  short *dd = data;
  int iret = 0;
  if (var->get(data, _nTimesInFile)) {
    for (size_t ii = 0; ii < _nTimesInFile; ii++, dd++) {
      vals.push_back(*dd);
    }
  } else {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - RawFile::_readRayVar" << endl;
      cerr << "  Cannot read variable: " << name << endl;
      cerr << _file.getNc3Error()->get_errmsg() << endl;
    }
    iret = -1;
  }
  delete[] data;
  return iret;

}

///////////////////////////////////////////
// read a ray variable - boolean
// side effects: set var, vals

int RawFile::_readRayVar(Nc3Var* &var, const string &name,
                         vector<bool> &vals)
  
{
  
  vals.clear();
  
  // get var
  
  var = _getRayVar(name);
  if (var == NULL) {
    return -1;
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
// get a ray variable by name
// returns NULL on failure

Nc3Var* RawFile::_getRayVar(const string &name)

{

  // get var
  
  Nc3Var *var = _file.getNc3File()->get_var(name.c_str());
  if (var == NULL) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - RawFile::_getRayVar" << endl;
      cerr << "  Cannot read variable, name: " << name << endl;
      cerr << _file.getNc3Error()->get_errmsg() << endl;
    }
    return NULL;
  }

  // check time dimension
  
  if (var->num_dims() < 1) {
    cerr << "ERROR - RawFile::_getRayVar" << endl;
    cerr << "  variable name: " << name << endl;
    cerr << "  variable has no dimensions" << endl;
    return NULL;
  }
  Nc3Dim *timeDim = var->get_dim(0);
  if (timeDim != _timeDim) {
    cerr << "ERROR - RawFile::_getRayVar" << endl;
    cerr << "  variable name: " << name << endl;
    cerr << "  variable has no dimensions" << endl;
    cerr << "  variable has incorrect dimension, dim name: "
         << timeDim->name() << endl;
    cerr << "  should be: " << "time" << endl;
    return NULL;
  }

  return var;

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

