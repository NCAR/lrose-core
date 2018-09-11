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
// ForayNcRadxFile_read.cc
//
// Read methods for ForayNcRadxFile object
//
// NetCDF file data for radar radial data in FORAY format
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2011
//
///////////////////////////////////////////////////////////////

#include <Radx/ForayNcRadxFile.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxPath.hh>
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

int ForayNcRadxFile::readFromPath(const string &path,
                                  RadxVol &vol)
  
{

  _initForRead(path, vol);

  // initialize udunits system if not already done
  
  // if (_udunitsInit()) {
  //   _addErrStr("ERROR - ForayNcRadxFile::readFromPath");
  //   _addErrStr("  Opening & initialize udunits system");
  //   return -1;
  // }

  // If the flag is set to aggregate sweeps into a volume on read,
  // create a vector of paths.
  // Otherwise load just original path into vector.

  vector<string> paths;

  if (_readAggregateSweeps) {
    int volNum = _getVolumePaths(path, paths);
    if (_debug) {
      cerr << "INFO - _readAggregatePaths" << endl;
      cerr << "  path: " << path << endl;
      cerr << "  volNum: " << volNum << endl;
    }
  } else {
    paths.push_back(path);
  }
    
  // get sweep information for full volume

  for (size_t ii = 0; ii < paths.size(); ii++) {
    if (_readSweepInfo(paths[ii])) {
      _addErrStr("ERROR - ForayNcRadxFile::readFromPath");
      return -1;
    }
  }

  // set sweep numbers which are valid on read

  if (_setSweepNums()) {
    return -1;
  }
  
  // read data from all paths
  
  for (size_t ii = 0; ii < paths.size(); ii++) {
    if (_readPath(paths[ii])) {
      return -1;
    }
  }

  // check we got some data
  
  if (_readVol->getRays().size() < 1) {
    _addErrStr("ERROR - ForayNcRadxFile::readFromPath");
    _addErrStr("  No valid rays found");
    return -1;
  }

  // finalize the read volume
  
  if (_finalizeReadVolume()) {
    return -1;
  }
  if (_debug) {
    _readVol->print(cerr);
  }
  
  // set the packing from the rays

  _readVol->setPackingFromRays();

  // set format as read

  _fileFormat = FILE_FORMAT_FORAY_NC;

  return 0;

}

////////////////////////////////////////////////////////////
// Read in sweep data using the specified path as a starting
// point. Aggregate the data from the sweeps into a single
// volume.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int ForayNcRadxFile::_readAggregatePaths(const string &path)
  
{
  
  // get the list of paths which make up this volume
  
  vector<string> paths;
  int volNum = _getVolumePaths(path, paths);
  if (_debug) {
    cerr << "INFO - _readAggregatePaths" << endl;
    cerr << "  path: " << path << endl;
    cerr << "  volNum: " << volNum << endl;
  }
  
  for (size_t ii = 0; ii < paths.size(); ii++) {
    if (_readPath(paths[ii])) {
      return -1;
    }
  }

  return 0;

}


////////////////////////////////////////////////////////////
// Read in data from specified path
// Returns 0 on success, -1 on failure

int ForayNcRadxFile::_readPath(const string &path)
  
{

  // clear tmp rays

  _rays.clear();

  // open file

  if (_file.openRead(path)) {
    _addErrStr("ERROR - ForayNcRadxFile::_readPath");
    _addErrStr(_file.getErrStr());
    return -1;
  }
  
  // read dimensions
  
  if (_readDimensions()) {
    _addErrStr("ERROR - ForayNcRadxFile::_readPath");
    return -1;
  }

  // read global attributes

  _readGlobalAttributes();

  // read in scalar variables

  if (_readScalarVariables()) {
    _addErrStr("ERROR - ForayNcRadxFile::_readPath");
    _addErrStr("  Cannot read scalar variables");
    _addErrStr("  The following variables are required:");
    _addErrStr("    base_time or volume_start_time");
    _addErrStr("    Fixed_Angle");
    _addErrStr("    Range_to_First_Cell, Cell_Spacing");
    _addErrStr("    Latitude, Longitude, Altitude");
    return -1;
  }

  // check sweep number

  if (_sweepNums.size() > 0) {
    bool reject = true;
    for (size_t ii = 0; ii < _sweepNums.size(); ii++) {
      if (_scanNumber == _sweepNums[ii]) {
        reject = false;
        break;
      }
    }
    if (reject) {
      if (_verbose) {
        cerr << "INFO - rejecting path: " << path << endl;
        if (_readFixedAngleLimitsSet) {
          cerr << "  Based on fixed angle: " << _Fixed_Angle << endl;
        } else {
          cerr << "  Based on sweep number: " << _scanNumber << endl;
        }
      }
      _file.close();
      return 0;
    }
  }

  // create the rays - one for each time
  
  if (_createRays()) {
    _addErrStr("ERROR - ForayNcRadxFile::_readPath");
    return -1;
  }

  // read in ray variables
  
  if (_readRayVariables()) {
    _addErrStr("ERROR - ForayNcRadxFile::_readPath");
    return -1;
  }
  
  // read field variable

  if (_readFieldVariables()) {
    _addErrStr("ERROR - ForayNcRadxFile::_readPath");
    return -1;
  }
  
  // close file

  _file.close();
  
  // add the rays to the read volume
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    _readVol->addRay(_rays[ii]);
  }
  
  // add to paths used on read

  _readPaths.push_back(path);

  return 0;

}

////////////////////////////////////////////////////////////
// Read in sweep info for path, append to file sweeps
// Returns 0 on success, -1 on failure

int ForayNcRadxFile::_readSweepInfo(const string &path)
  
{

  // open file

  if (_file.openRead(path)) {
    _addErrStr("ERROR - ForayNcRadxFile::_readSweepInfo");
    _addErrStr(_file.getErrStr());
    return -1;
  }
  
  // read dimensions
  
  if (_readDimensions()) {
    _addErrStr("ERROR - ForayNcRadxFile::_readSweepInfo");
    return -1;
  }

  // read global attributes

  _readGlobalAttributes();
  
  // read in scalar variables
  
  _readScalarVariables();

  // done with file

  _file.close();

  // create sweep

  RadxSweep *sweep = new RadxSweep();
  sweep->setFixedAngleDeg(_Fixed_Angle);
  sweep->setSweepNumber(_scanNumber);
  sweep->setVolumeNumber(_volumeNumber);
  sweep->setSweepMode(_sweepMode);
  _readVol->addSweepAsInFile(sweep); // makes a copy
  delete sweep;

  return 0;

}

//////////////////////////////////////////////////////////
// set sweep number limits
// assumes we have already read in sweep info

int ForayNcRadxFile::_setSweepNums()

{

  _sweepNums.clear();
  
  if (!_readFixedAngleLimitsSet && !_readSweepNumLimitsSet) {
    // no limits
    return 0;
  }
  
  // find sweep indexes which lie within the fixedAngle limits

  const vector<RadxSweep *> &sweeps = _readVol->getSweepsAsInFile();
  for (size_t ii = 0; ii < sweeps.size(); ii++) {
    int sweepNum = sweeps[ii]->getSweepNumber();
    if (_readSweepNumLimitsSet) {
      if (sweepNum >= _readMinSweepNum && sweepNum <= _readMaxSweepNum) {
        _sweepNums.push_back(sweepNum);
      }
    } else {
      double elev = sweeps[ii]->getFixedAngleDeg();
      if (elev > (_readMinFixedAngle - 0.01) &&
          elev < (_readMaxFixedAngle + 0.01)) {
        _sweepNums.push_back(sweepNum);
      }
    }
  }

  // do we have sweeps?

  if (_sweepNums.size() > 0) {
    sort(_sweepNums.begin(), _sweepNums.end());
    return 0;
  }

  // strict angle checking?

  if (_readStrictAngleLimits) {
    if (_readFixedAngleLimitsSet) {
      _addErrStr("ERROR - ForayNcRadxFile::_setSweepNums");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
    } else if (_readSweepNumLimitsSet) {
      _addErrStr("ERROR - ForayNcRadxFile::_setSweepNums");
      _addErrStr("  No data found within sweep num limits");
      _addErrInt("  min sweep num: ", _readMinSweepNum);
      _addErrInt("  max sweep num: ", _readMaxSweepNum);
    }
    return -1;
  }

  // make sure we have at least one sweep number

  int bestNum = 0;
  if (_readSweepNumLimitsSet) {
    double minDiff = 1.0e99;
    double meanNum = (_readMinSweepNum + _readMaxSweepNum) / 2.0;
    for (size_t ii = 0; ii < sweeps.size(); ii++) {
      int sweepNum = sweeps[ii]->getSweepNumber();
      double diff = fabs(sweepNum - meanNum);
      if (diff < minDiff) {
        minDiff = diff;
        bestNum = sweepNum;
      }
    }
  } else {
    double minDiff = 1.0e99;
    double meanAngle = (_readMinFixedAngle + _readMaxFixedAngle) / 2.0;
    if (_readMaxFixedAngle - _readMinFixedAngle < 0) {
      meanAngle -= 180.0;
    }
    if (meanAngle < 0) {
      meanAngle += 360.0;
    }
    for (size_t ii = 0; ii < sweeps.size(); ii++) {
      int sweepNum = sweeps[ii]->getSweepNumber();
      double elev = sweeps[ii]->getFixedAngleDeg();
      double diff = fabs(elev - meanAngle);
      if (diff < minDiff) {
        minDiff = diff;
        bestNum = sweepNum;
      }
    }
  }
  _sweepNums.push_back(bestNum);

  return 0;

}

//////////////////////////////////////////////////////////
// get list of paths for the volume for the specified path
// returns the volume number

int ForayNcRadxFile::_getVolumePaths(const string &path,
                                     vector<string> &paths)
  
{

  paths.clear();
  int volNum = -1;

  // find the volume number by searching for "_v"

  size_t vloc = path.find_last_of("v");
  if (vloc == string::npos || vloc == 0 ||
      vloc == (path.size() - 1) || path[vloc-1] != '_') {
    // cannot find volume tag "_v"
    paths.push_back(path);
    return volNum;
  }

  // scan the volume number

  string volStr = path.substr(vloc + 1);
  if (sscanf(volStr.c_str(), "%d", &volNum) != 1) {
    return -1;
  }

  // find all paths with this volume number in the same
  // directory as the specified path
  
  RadxPath rpath(path);
  string dir = rpath.getDirectory();
  _addToPathList(dir, volNum, 0, 23, paths);
  
  RadxPath dpath(dir);
  string parentDir = dpath.getDirectory();

  // if time is close to start of day, search previous direectory

  RadxTime rtime;
  if (getTimeFromPath(path, rtime)) {
    return -1;
  }
  int rhour = rtime.getHour();

  if (rhour == 0) {
    RadxTime prevDate(rtime.utime() - RadxTime::RADX_SECS_IN_DAY);
    char prevDir[RadxPath::RADX_MAX_PATH_LEN];
    sprintf(prevDir, "%s%s%.4d%.2d%.2d",
            parentDir.c_str(), RadxPath::RADX_PATH_DELIM,
            prevDate.getYear(), prevDate.getMonth(), prevDate.getDay());
    _addToPathList(prevDir, volNum, 23, 23, paths);
  }

  // if time is close to end of day, search previous direectory

  if (rhour == 23) {
    RadxTime nextDate(rtime.utime() + RadxTime::RADX_SECS_IN_DAY);
    char nextDir[RadxPath::RADX_MAX_PATH_LEN];
    sprintf(nextDir, "%s%s%.4d%.2d%.2d",
            parentDir.c_str(), RadxPath::RADX_PATH_DELIM,
            nextDate.getYear(), nextDate.getMonth(), nextDate.getDay());
    _addToPathList(nextDir, volNum, 0, 0, paths);
  }

  // sort the path list

  sort(paths.begin(), paths.end());

  return volNum;

}

///////////////////////////////////////////////////////////
// add to the path list, given time constraints

void ForayNcRadxFile::_addToPathList(const string &dir,
                                     int volNum,
                                     int minHour, int maxHour,
                                     vector<string> &paths)
  
{

  // find all paths with this volume number
  
  DIR *dirp;
  if((dirp = opendir(dir.c_str())) == NULL) {
    return;
  }
  
  char volText[64];
  sprintf(volText, "_v%03d", volNum);

  struct dirent *dp;
  for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {

    string fileName(dp->d_name);

    // exclude dir entries which cannot be valid
    if (fileName.find("swp") == string::npos) {
      continue;
    }
    if (fileName.size() < 20) {
      continue;
    }

    RadxTime rtime;
    if (getTimeFromPath(fileName, rtime) == 0) {
      int hour = rtime.getHour();
      if (hour >= minHour && hour <= maxHour) {
        // find file names which have this volume number
        if (fileName.find(volText) != string::npos) {
          string filePath = dir;
          filePath += RadxPath::RADX_PATH_DELIM;
          filePath += fileName;
          paths.push_back(filePath);
        }
      }
    }

  } // dp

  closedir(dirp);

}

///////////////////////////////////
// read in the dimensions

int ForayNcRadxFile::_readDimensions()

{

  // read required dimensions

  int iret = 0;
  iret |= _file.readDim("Time", _TimeDim);
  iret |= _file.readDim("maxCells", _maxCellsDim);
  if (_file.readDim("numSystems", _numSystemsDim) != 0) {
    iret |= _file.readDim("maxSystems", _numSystemsDim);
  }

  if (iret) {
    _addErrStr("ERROR - ForayNcRadxFile::_readDimensions");
    return -1;
  }

  // derived quantites
  
  _nGates = _maxCellsDim->size();
  
  return 0;

}

///////////////////////////////////
// read the global attributes

void ForayNcRadxFile::_readGlobalAttributes()

{

  // Loop through the global attributes, use the ones which make sense
  
  _instrumentType = Radx::INSTRUMENT_TYPE_RADAR;
  _platformType = Radx::PLATFORM_TYPE_FIXED;
  _sweepMode = Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
  
  for (int ii = 0; ii < _file.getNc3File()->num_atts(); ii++) {
    
    Nc3Att* att = _file.getNc3File()->get_att(ii);
    
    if (att == NULL) {
      continue;
    }
    
    if (!strcmp(att->name(), "Conventions")) {
      _conventions = _file.asString(att);
    }
    
    if (!strcmp(att->name(), "Instrument_Name")) {
      _instrumentName = _file.asString(att);
    }
    if (_instrumentName.size() < 1) {
      _instrumentName = "unknown";
    }

    if (!strcmp(att->name(), "Project_Name")) {
      _siteName = _file.asString(att);
    }

    if (!strcmp(att->name(), "Instrument_Type")) {
      _instrumentTypeStr = _file.asString(att);
      if (_instrumentTypeStr == "Ground") {
        _platformType = Radx::PLATFORM_TYPE_FIXED;
      } else {
        _platformType = Radx::PLATFORM_TYPE_AIRCRAFT_TAIL;
      }
    }

    if (!strcmp(att->name(), "Scan_Mode")) {
      _scanModeStr = _file.asString(att);
      if (_scanModeStr == "SUR") {
        _sweepMode = Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
      } else if (_scanModeStr == "PPI") {
        _sweepMode = Radx::SWEEP_MODE_SECTOR;
      } else if (_scanModeStr == "RHI") {
        _sweepMode = Radx::SWEEP_MODE_RHI;
      } else if (_scanModeStr == "IDL") {
        _sweepMode = Radx::SWEEP_MODE_IDLE;
      }
    }

    if (!strcmp(att->name(), "Project_Name")) {
      _projectName = _file.asString(att);
    }
    
    if (!strcmp(att->name(), "Producer_Name")) {
      _institution = _file.asString(att);
    }

    if (!strcmp(att->name(), "Volume_Number")) {
      _volumeNumber = att->as_int(0);
    }
    
    if (!strcmp(att->name(), "Scan_Number")) {
      _scanNumber = att->as_int(0);
    }
    
    if (!strcmp(att->name(), "Num_Samples")) {
      _nSamples = att->as_int(0);
    }
    
    if (!strcmp(att->name(), "Software")) {
      _source = _file.asString(att);
    }

    _comment = "Converted from FORAY NC file";
    
    // Caller must delete attribute

    delete att;
    
  } // ii

  // NDTA data is written out unsigned

  if (_source.find("NCARTurbDetectAlg") != string::npos) {
    _dataIsUnsigned = true;
  }

}

///////////////////////////////////
// read the times, create the rays

int ForayNcRadxFile::_createRays()

{

  // read the time variable

  Nc3Var *timeVar = _file.getNc3File()->get_var(TIME_OFFSET);
  if (timeVar == NULL) {
    _addErrStr("ERROR - ForayNcRadxFile::_createRays");
    _addErrStr("  Cannot find time variable, name: ", TIME_OFFSET);
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  if (timeVar->num_dims() < 1) {
    _addErrStr("ERROR - ForayNcRadxFile::_createRays");
    _addErrStr("  time variable has no dimensions");
    return -1;
  }
  Nc3Dim *timeDim = timeVar->get_dim(0);
  if (timeDim != _TimeDim) {
    _addErrStr("ERROR - ForayNcRadxFile::_createRays");
    _addErrStr("  Time has incorrect dimension, name: ", timeDim->name());
    return -1;
  }
  int nTimes = timeVar->num_vals();

  // get units attribute
  
  Nc3Att* unitsAtt = timeVar->get_att(UNITS);
  if (unitsAtt == NULL) {
    _addErrStr("ERROR - ForayNcRadxFile::_createRays");
    _addErrStr("  Time has no units");
    return -1;
  }
  string units = _file.asString(unitsAtt);
  delete unitsAtt;

  // create rays, one for each time

  _rays.clear();
  double *dtimes = new double[nTimes];
  if (timeVar->get(dtimes, nTimes)) {
    double *dd = dtimes;
    for (int ii = 0; ii < nTimes; ii++, dd++) {
      double rayTimeDouble = (double) _base_time + *dd;
      time_t rayUtimeSecs = (time_t) rayTimeDouble;
      double rayFracSecs = rayTimeDouble - rayUtimeSecs;
      int rayNanoSecs = (int) (rayFracSecs * 1.0e9);
      RadxRay *ray = new RadxRay;
      ray->setTime(rayUtimeSecs, rayNanoSecs);
      if (_startTimeSecs == 0 && _endTimeSecs == 0) {
        _startTimeSecs = rayUtimeSecs;
        _startNanoSecs = rayNanoSecs;
      }
      _endTimeSecs = rayUtimeSecs;
      _endNanoSecs = rayNanoSecs;
      ray->copyRangeGeom(_geom);
      _rays.push_back(ray);
    } // ii
  }
  delete[] dtimes;

  return 0;

}

///////////////////////////////////
// read the scalar variables
//
// Only cause failure on required items

int ForayNcRadxFile::_readScalarVariables()

{
  
  int iret = 0;

  if (_file.readIntVal("base_time", _base_time, 0, true)) {
    iret |= _file.readIntVal("volume_start_time", _base_time, 0, true);
  }

  _file.readIntVal("Cell_Spacing_Method", _Cell_Spacing_Method, 0, false);
  _file.readIntVal("calibration_data_present", _calibration_data_present, 0, false);

  _Fixed_Angle = -9999.0;
  _file.readDoubleVal("Fixed_Angle",
                      _Fixed_Angle, Radx::missingMetaDouble, true);
  if (_Fixed_Angle < -9990) {
    cerr << "WARNING - ForayNcRadxFile::_readScalarVariables()" << endl;
    cerr << "  Variable 'Fixed_Angle' not found'" << endl;
    cerr << "  Fixed angle will be computed from the data" << endl;
  }

  iret |= _file.readDoubleVal("Range_to_First_Cell",
                              _Range_to_First_Cell, Radx::missingMetaDouble, true);
  if (_file.readDoubleVal("Cell_Spacing",
                          _Cell_Spacing, Radx::missingMetaDouble, true)) {
    iret |= _readCellDistance();
  }
  
  _file.readDoubleVal("Nyquist_Velocity",
                      _Nyquist_Velocity, Radx::missingMetaDouble, false);
  _file.readDoubleVal("Unambiguous_Range",
                      _Unambiguous_Range, Radx::missingMetaDouble, false);

  iret |= _file.readDoubleVal("Latitude", _Latitude, Radx::missingMetaDouble, true);
  iret |= _file.readDoubleVal("Longitude", _Longitude, Radx::missingMetaDouble, true);
  iret |= _file.readDoubleVal("Altitude", _Altitude, Radx::missingMetaDouble, true);
  _file.readDoubleVal("Radar_Constant",
                      _Radar_Constant, Radx::missingMetaDouble, true);
  
  _file.readDoubleVal("Wavelength", _Wavelength, Radx::missingMetaDouble, false);
  _file.readDoubleVal("PRF", _PRF, Radx::missingMetaDouble, false);

  _file.readDoubleVal("rcvr_gain", _rcvr_gain, Radx::missingMetaDouble, false);
  _file.readDoubleVal("ant_gain", _ant_gain, Radx::missingMetaDouble, false);
  _file.readDoubleVal("sys_gain", _sys_gain, Radx::missingMetaDouble, false);
  _file.readDoubleVal("bm_width", _bm_width, Radx::missingMetaDouble, false);
  _file.readDoubleVal("pulse_width", _pulse_width, Radx::missingMetaDouble, false);
  _file.readDoubleVal("band_width", _band_width, Radx::missingMetaDouble, false);
  _file.readDoubleVal("peak_pwr", _peak_pwr, Radx::missingMetaDouble, false);
  _file.readDoubleVal("xmtr_pwr", _xmtr_pwr, Radx::missingMetaDouble, false);
  _file.readDoubleVal("noise_pwr", _noise_pwr, Radx::missingMetaDouble, false);
  _file.readDoubleVal("tst_pls_pwr", _tst_pls_pwr, Radx::missingMetaDouble, false);
  _file.readDoubleVal("tst_pls_rng0", _tst_pls_rng0, Radx::missingMetaDouble, false);
  _file.readDoubleVal("tst_pls_rng1", _tst_pls_rng1, Radx::missingMetaDouble, false);

  if (_calibration_data_present) {

    _file.readDoubleVal("ant_gain_h_db",
                        _ant_gain_h_db, Radx::missingMetaDouble, false);
    _file.readDoubleVal("ant_gain_v_db",
                        _ant_gain_v_db, Radx::missingMetaDouble, false);
    _file.readDoubleVal("xmit_power_h_dbm",
                        _xmit_power_h_dbm, Radx::missingMetaDouble, false);
    _file.readDoubleVal("xmit_power_v_dbm",
                        _xmit_power_v_dbm, Radx::missingMetaDouble, false);
    _file.readDoubleVal("two_way_waveguide_loss_h_db",
                        _two_way_waveguide_loss_h_db, Radx::missingMetaDouble, false);
    _file.readDoubleVal("two_way_waveguide_loss_v_db",
                        _two_way_waveguide_loss_v_db, Radx::missingMetaDouble, false);
    _file.readDoubleVal("two_way_radome_loss_h_db",
                        _two_way_radome_loss_h_db, Radx::missingMetaDouble, false);
    _file.readDoubleVal("two_way_radome_loss_v_db",
                        _two_way_radome_loss_v_db, Radx::missingMetaDouble, false);
    _file.readDoubleVal("receiver_mismatch_loss_db",
                        _receiver_mismatch_loss_db, Radx::missingMetaDouble, false);
    _file.readDoubleVal("radar_constant_h",
                        _radar_constant_h, Radx::missingMetaDouble, false);
    _file.readDoubleVal("radar_constant_v",
                        _radar_constant_v, Radx::missingMetaDouble, false);
    _file.readDoubleVal("noise_hc_dbm",
                        _noise_hc_dbm, Radx::missingMetaDouble, false);
    _file.readDoubleVal("noise_vc_dbm",
                        _noise_vc_dbm, Radx::missingMetaDouble, false);
    _file.readDoubleVal("noise_hx_dbm",
                        _noise_hx_dbm, Radx::missingMetaDouble, false);
    _file.readDoubleVal("noise_vx_dbm",
                        _noise_vx_dbm, Radx::missingMetaDouble, false);
    _file.readDoubleVal("receiver_gain_hc_db",
                        _receiver_gain_hc_db, Radx::missingMetaDouble, false);
    _file.readDoubleVal("receiver_gain_vc_db",
                        _receiver_gain_vc_db, Radx::missingMetaDouble, false);
    _file.readDoubleVal("receiver_gain_hx_db",
                        _receiver_gain_hx_db, Radx::missingMetaDouble, false);
    _file.readDoubleVal("receiver_gain_vx_db",
                        _receiver_gain_vx_db, Radx::missingMetaDouble, false);
    _file.readDoubleVal("base_1km_hc_dbz",
                        _base_1km_hc_dbz, Radx::missingMetaDouble, false);
    _file.readDoubleVal("base_1km_vc_dbz",
                        _base_1km_vc_dbz, Radx::missingMetaDouble, false);
    _file.readDoubleVal("base_1km_hx_dbz",
                        _base_1km_hx_dbz, Radx::missingMetaDouble, false);
    _file.readDoubleVal("base_1km_vx_dbz",
                        _base_1km_vx_dbz, Radx::missingMetaDouble, false);
    _file.readDoubleVal("sun_power_hc_dbm",
                        _sun_power_hc_dbm, Radx::missingMetaDouble, false);
    _file.readDoubleVal("sun_power_vc_dbm",
                        _sun_power_vc_dbm, Radx::missingMetaDouble, false);
    _file.readDoubleVal("sun_power_hx_dbm",
                        _sun_power_hx_dbm, Radx::missingMetaDouble, false);
    _file.readDoubleVal("sun_power_vx_dbm",
                        _sun_power_vx_dbm, Radx::missingMetaDouble, false);
    _file.readDoubleVal("noise_source_power_h_dbm",
                        _noise_source_power_h_dbm, Radx::missingMetaDouble, false);
    _file.readDoubleVal("noise_source_power_v_dbm",
                        _noise_source_power_v_dbm, Radx::missingMetaDouble, false);
    _file.readDoubleVal("power_measure_loss_h_db",
                        _power_measure_loss_h_db, Radx::missingMetaDouble, false);
    _file.readDoubleVal("power_measure_loss_v_db",
                        _power_measure_loss_v_db, Radx::missingMetaDouble, false);
    _file.readDoubleVal("coupler_forward_loss_h_db",
                        _coupler_forward_loss_h_db, Radx::missingMetaDouble, false);
    _file.readDoubleVal("coupler_forward_loss_v_db",
                        _coupler_forward_loss_v_db, Radx::missingMetaDouble, false);
    _file.readDoubleVal("zdr_correction_db",
                        _zdr_correction_db, Radx::missingMetaDouble, false);
    _file.readDoubleVal("ldr_correction_h_db",
                        _ldr_correction_h_db, Radx::missingMetaDouble, false);
    _file.readDoubleVal("ldr_correction_v_db",
                        _ldr_correction_v_db, Radx::missingMetaDouble, false);
    _file.readDoubleVal("system_phidp_deg",
                        _system_phidp_deg, Radx::missingMetaDouble, false);
    
    _rCal.init();
    _rCal.setPulseWidthUsec(_pulse_width * 1.0e6);
    _rCal.setXmitPowerDbmH(_xmit_power_h_dbm);
    _rCal.setXmitPowerDbmV(_xmit_power_v_dbm);
    _rCal.setTwoWayWaveguideLossDbH(_two_way_waveguide_loss_h_db);
    _rCal.setTwoWayWaveguideLossDbV(_two_way_waveguide_loss_v_db);
    _rCal.setTwoWayRadomeLossDbH(_two_way_radome_loss_h_db);
    _rCal.setTwoWayRadomeLossDbV(_two_way_radome_loss_v_db);
    _rCal.setReceiverMismatchLossDb(_receiver_mismatch_loss_db);
    _rCal.setRadarConstantH(_radar_constant_h);
    _rCal.setRadarConstantV(_radar_constant_v);
    _rCal.setAntennaGainDbH(_ant_gain_h_db);
    _rCal.setAntennaGainDbV(_ant_gain_v_db);
    _rCal.setNoiseDbmHc(_noise_hc_dbm);
    _rCal.setNoiseDbmHx(_noise_hx_dbm);
    _rCal.setNoiseDbmVc(_noise_vc_dbm);
    _rCal.setNoiseDbmVx(_noise_vx_dbm);
    _rCal.setReceiverGainDbHc(_receiver_gain_hc_db);
    _rCal.setReceiverGainDbHx(_receiver_gain_hx_db);
    _rCal.setReceiverGainDbVc(_receiver_gain_vc_db);
    _rCal.setReceiverGainDbVx(_receiver_gain_vx_db);
    _rCal.setBaseDbz1kmHc(_base_1km_hc_dbz);
    _rCal.setBaseDbz1kmHx(_base_1km_hx_dbz);
    _rCal.setBaseDbz1kmVc(_base_1km_vc_dbz);
    _rCal.setBaseDbz1kmVx(_base_1km_vx_dbz);
    _rCal.setSunPowerDbmHc(_sun_power_hc_dbm);
    _rCal.setSunPowerDbmHx(_sun_power_hx_dbm);
    _rCal.setSunPowerDbmVc(_sun_power_vc_dbm);
    _rCal.setSunPowerDbmVx(_sun_power_vx_dbm);
    _rCal.setNoiseSourcePowerDbmH(_noise_source_power_h_dbm);
    _rCal.setNoiseSourcePowerDbmV(_noise_source_power_v_dbm);
    _rCal.setPowerMeasLossDbH(_power_measure_loss_h_db);
    _rCal.setPowerMeasLossDbV(_power_measure_loss_v_db);
    _rCal.setCouplerForwardLossDbH(_coupler_forward_loss_h_db);
    _rCal.setCouplerForwardLossDbV(_coupler_forward_loss_v_db);
    _rCal.setZdrCorrectionDb(_zdr_correction_db);
    _rCal.setLdrCorrectionDbH(_ldr_correction_h_db);
    _rCal.setLdrCorrectionDbV(_ldr_correction_v_db);
    _rCal.setSystemPhidpDeg(_system_phidp_deg);
    
  }

  if (iret) {
    _addErrStr("ERROR - ForayNcRadxFile::_readScalarVariables");
    return -1;
  }

  // compute derived quantities

  _gateSpacingIsConstant = true;
  _startRangeKm = _Range_to_First_Cell / 1000.0;
  _gateSpacingKm = _Cell_Spacing / 1000.0;
  _geom.setRangeGeom(_startRangeKm, _gateSpacingKm);

  return 0;

}

//////////////////////////////////////////////////
// read the Cell_Distance_Vector variable
// This is used if Cell_Spacing is missing

int ForayNcRadxFile::_readCellDistance()

{

  Nc3Var *distVar = _file.getNc3File()->get_var("Cell_Distance_Vector");
  if (distVar == NULL) {
    _addErrStr("ERROR - ForayNcRadxFile::_readCellDistanceVar");
    _addErrStr("  Cannot find Cell_Distance_Vector variable");
    _addErrStr("          nor Cell_Spacing variable");
    return -1;
  }

  // check time dimension
  
  if (distVar->num_dims() < 1) {
    _addErrStr("ERROR - ForayNcRadxFile::_readCellDistanceVar");
    _addErrStr("  variable Cell_Distance_Vector has no dimensions");
    return -1;
  }
  Nc3Dim *dim = distVar->get_dim(0);
  if (dim != _maxCellsDim) {
    _addErrStr("ERROR - ForayNcRadxFile::_readCellDistanceVar");
    _addErrStr("  variable Cell_Distance_Vector");
    _addErrStr("  has incorrect dimension, dim name: ", dim->name());
    _addErrStr("  should be maxCells");
    return -1;
  }

  if (distVar->get(&_Cell_Spacing, 1)) {
    return 0;
  } else {
    _addErrStr("ERROR - ForayNcRadxFile::_readCellDistanceVar");
    _addErrStr("  Cannot read variable Cell_Distance_Vector");
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }

}

///////////////////////////////////
// read the ray meta-data

int ForayNcRadxFile::_readRayVariables()

{

  // initialize

  vector<double> azimuths, elevations;
  vector<double> time_offsets, clip_ranges;

  int iret = 0;

  _readRayVar("Azimuth", azimuths);
  if (azimuths.size() < _rays.size()) {
    _addErrStr("ERROR - azimuth variable required");
    iret = -1;
  }

  _readRayVar("Elevation", elevations);
  if (elevations.size() < _rays.size()) {
    _addErrStr("ERROR - elevation variable required");
    iret = -1;
  }

  _readRayVar("time_offset", time_offsets);
  if (time_offsets.size() < _rays.size()) {
    _addErrStr("ERROR - time_offset variable required");
    iret = -1;
  }

  _readRayVar("clip_range", clip_ranges);

  if (iret) {
    _addErrStr("ERROR - ForayNcRadxFile::_readRayVariables");
    return -1;
  }

  for (int ii = 0; ii < (int) _rays.size(); ii++) {

    RadxRay *ray = _rays[ii];

    ray->setVolumeNumber(_volumeNumber);
    ray->setSweepNumber(_scanNumber);
    ray->setCalibIndex(0);
    ray->setSweepMode(_sweepMode);
    if (_instrumentName.find("SPOL") != string::npos) {
      ray->setPolarizationMode(Radx::POL_MODE_HV_ALT);
    }
    ray->setPrtMode(Radx::PRT_MODE_FIXED);
    ray->setFollowMode(Radx::FOLLOW_MODE_NONE);

    double timeOffsetSecs = time_offsets[ii];
    double rayTime = _base_time + timeOffsetSecs;
    time_t raySecs = (time_t) rayTime;
    double rayNanoSecs = (rayTime - raySecs) * 1.0e9;
    ray->setTime(raySecs, rayNanoSecs);

    if ((int) azimuths.size() > ii) {
      ray->setAzimuthDeg(azimuths[ii]);
    }
    
    if ((int) elevations.size() > ii) {
      ray->setElevationDeg(elevations[ii]);
    }
    
    ray->setFixedAngleDeg(_Fixed_Angle);
    ray->setPulseWidthUsec(_pulse_width * 1.0e6);
    ray->setPrtSec(1.0 / _PRF);
    ray->setNyquistMps(_Nyquist_Velocity);
    ray->setUnambigRangeKm(_Unambiguous_Range / 1000.0);
    ray->setAntennaTransition(false);
    ray->setNSamples(_nSamples);
    ray->setCalibIndex(0);
    ray->setMeasXmitPowerDbmH(_xmit_power_h_dbm);
    ray->setMeasXmitPowerDbmV(_xmit_power_v_dbm);
    ray->setRangeGeom(_startRangeKm, _gateSpacingKm);

  }

  return 0;

}

///////////////////////////////////
// read a ray variable - double

int ForayNcRadxFile::_readRayVar(const string &name, vector<double> &vals)

{

  vals.clear();

  // get var

  Nc3Var* var = _getRayVar(name);
  if (var == NULL) {
    _addErrStr("ERROR - ForayNcRadxFile::_readRayVar");
    return -1;
  }

  // load up data

  int nRays = _rays.size();
  double *data = new double[nRays];
  double *dd = data;
  if (var->get(data, nRays)) {
    for (int ii = 0; ii < nRays; ii++, dd++) {
      vals.push_back(*dd);
    }
  } else {
    _addErrStr("ERROR - ForayNcRadxFile::_readRayVar");
    _addErrStr("  Cannot read variable: ", name);
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  delete[] data;

  return 0;

}

///////////////////////////////////
// read a ray variable - integer

int ForayNcRadxFile::_readRayVar(const string &name, vector<int> &vals)

{

  vals.clear();

  // get var

  Nc3Var* var = _getRayVar(name);
  if (var == NULL) {
    _addErrStr("ERROR - ForayNcRadxFile::_readRayVar");
    return -1;
  }

  // load up data

  int nRays = _rays.size();
  int *data = new int[nRays];
  int *dd = data;
  if (var->get(data, nRays)) {
    for (int ii = 0; ii < nRays; ii++, dd++) {
      vals.push_back(*dd);
    }
  } else {
    _addErrStr("ERROR - ForayNcRadxFile::_readRayVar");
    _addErrStr("  Cannot read variable: ", name);
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return -1;
  }
  delete[] data;

  return 0;

}

///////////////////////////////////
// get a ray variable by name
// returns NULL on failure

Nc3Var* ForayNcRadxFile::_getRayVar(const string &name)

{

  // get var

  Nc3Var *var = _file.getNc3File()->get_var(name.c_str());
  if (var == NULL) {
    _addErrStr("ERROR - ForayNcRadxFile::_getRayVar");
    _addErrStr("  Cannot read variable, name: ", name);
    _addErrStr(_file.getNc3Error()->get_errmsg());
    return NULL;
  }

  // check time dimension
  
  if (var->num_dims() < 1) {
    _addErrStr("ERROR - ForayNcRadxFile::_getRayVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has no dimensions");
    return NULL;
  }
  Nc3Dim *timeDim = var->get_dim(0);
  if (timeDim != _TimeDim) {
    _addErrStr("ERROR - ForayNcRadxFile::_getRayVar");
    _addErrStr("  variable name: ", name);
    _addErrStr("  variable has incorrect dimension, dim name: ", timeDim->name());
    _addErrStr("  should be: ", TIME);
    return NULL;
  }

  return var;

}

////////////////////////////////////////////
// read the field variables

int ForayNcRadxFile::_readFieldVariables()

{

  // loop through the variables, adding data fields as appropriate
  
  for (int ivar = 0; ivar < _file.getNc3File()->num_vars(); ivar++) {
    
    Nc3Var* var = _file.getNc3File()->get_var(ivar);
    if (var == NULL) {
      continue;
    }
    
    // we need fields with 2 dimensions
    
    int numDims = var->num_dims();
    if (numDims != 2) {
      continue;
    }
    
    // check that we have the correct dimensions
    
    Nc3Dim* timeDim = var->get_dim(0);
    Nc3Dim* maxCellsDim = var->get_dim(1);
    if (timeDim != _TimeDim || maxCellsDim != _maxCellsDim) {
      continue;
    }
    
    // check the type
    Nc3Type ftype = var->type();
    if (ftype != nc3Double && ftype != nc3Float && ftype != nc3Int &&
        ftype != nc3Short && ftype != nc3Byte) {
      // not a valid type
      continue;
    }

    // check that we need this field
    
    string fieldName = var->name();
    if (!isFieldRequiredOnRead(fieldName)) {
      if (_verbose) {
        cerr << "DEBUG - ForayNcRadxFile::_readFieldVariables" << endl;
        cerr << "  -->> rejecting field: " << fieldName << endl;
      }
      continue;
    }
    if (_debug) {
      cerr << "DEBUG - ForayNcRadxFile::_readFieldVariables" << endl;
      cerr << "  -->> adding field: " << fieldName << endl;
    }

    // set names, units, etc
    
    string name = var->name();
    
    string standardName;
    Nc3Att *standardNameAtt = var->get_att(STANDARD_NAME);
    if (standardNameAtt != NULL) {
      standardName = _file.asString(standardNameAtt);
      delete standardNameAtt;
    }
    
    string longName;
    Nc3Att *longNameAtt = var->get_att(LONG_NAME);
    if (longNameAtt != NULL) {
      longName = _file.asString(longNameAtt);
      delete longNameAtt;
    }

    string units;
    Nc3Att *unitsAtt = var->get_att(UNITS);
    if (unitsAtt != NULL) {
      units = _file.asString(unitsAtt);
      delete unitsAtt;
    }

    // get missing val, offset and scale

    Nc3Att *missingValueAtt = var->get_att(MISSING_VALUE);

    double offset = 0.0;
    Nc3Att *offsetAtt = var->get_att(ADD_OFFSET);
    if (offsetAtt != NULL) {
      offset = offsetAtt->as_double(0);
      delete offsetAtt;
    }

    double scale = 1.0;
    Nc3Att *scaleAtt = var->get_att(SCALE_FACTOR);
    if (scaleAtt != NULL) {
      scale = scaleAtt->as_double(0);
      delete scaleAtt;
    }

    int nPoints = _rays.size() * _nGates;
    int iret = 0;
    
    switch (var->type()) {
      case nc3Double: {
        if (_addFl64FieldToRays(var, nPoints,
                                name, units, standardName, longName)) {
          iret = -1;
        }
        break;
      }
      case nc3Float: {
        if (_addFl32FieldToRays(var, nPoints,
                                name, units, standardName, longName)) {
          iret = -1;
        }
        break;
      }
      case nc3Int: {
        if (_addSi32FieldToRays(var, nPoints,
                                name, units, standardName, longName,
                                scale, offset)) {
          iret = -1;
        }
        break;
      }
      case nc3Short: {
        if (_addSi16FieldToRays(var, nPoints,
                                name, units, standardName, longName,
                                scale, offset)) {
          iret = -1;
        }
        break;
      }
      case nc3Byte: {
        if (_addSi08FieldToRays(var, nPoints,
                                name, units, standardName, longName,
                                scale, offset)) {
          iret = -1;
        }
        break;
      }
      default: {
        iret = -1;
        // will not reach here because of earlier check on type
      }

    } // switch
    
    if (missingValueAtt != NULL) {
      delete missingValueAtt;
    }
    
    if (iret) {
      _addErrStr("ERROR - ForayNcRadxFile::_readFieldVariables");
      _addErrStr("  cannot read field name: ", name);
      _addErrStr(_file.getNc3Error()->get_errmsg());
      return -1;
    }

  } // ivar

  return 0;

}

//////////////////////////////////////////////////////////////
// Add fl64 fields to _rays
// The _rays array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int ForayNcRadxFile::_addFl64FieldToRays(Nc3Var* var, int nPoints,
                                         const string &name,
                                         const string &units,
                                         const string &standardName,
                                         const string &longName)
  
{

  Radx::fl64 *data = new Radx::fl64[nPoints];

  if (!var->get(data, _rays.size(), _nGates)) {
    delete[] data;
    return -1;
  }

  Radx::fl64 missingVal = Radx::missingFl64;
  Nc3Att *missingValueAtt = var->get_att(MISSING_VALUE);
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  }
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    RadxField *field =
      _rays[ii]->addField(name, units, _nGates,
                          missingVal,
                          data + ii * _nGates,
                          true);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);
  }
  
  delete[] data;
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add fl32 fields to _rays
// The _rays array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int ForayNcRadxFile::_addFl32FieldToRays(Nc3Var* var, int nPoints,
                                         const string &name,
                                         const string &units,
                                         const string &standardName,
                                         const string &longName)
  
{

  Radx::fl32 *data = new Radx::fl32[nPoints];

  if (!var->get(data, _rays.size(), _nGates)) {
    delete[] data;
    return -1;
  }

  Radx::fl32 missingVal = Radx::missingFl32;
  Nc3Att *missingValueAtt = var->get_att(MISSING_VALUE);
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_double(0);
    delete missingValueAtt;
  }
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    RadxField *field =
      _rays[ii]->addField(name, units, _nGates,
                          missingVal,
                          data + ii * _nGates,
                          true);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);
  }
  
  delete[] data;
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add si32 fields to _rays
// The _rays array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int ForayNcRadxFile::_addSi32FieldToRays(Nc3Var* var, int nPoints,
                                         const string &name,
                                         const string &units,
                                         const string &standardName,
                                         const string &longName,
                                         double scale, double offset)
  
{

  Radx::si32 *data = new Radx::si32[nPoints];

  if (!var->get(data, _rays.size(), _nGates)) {
    delete[] data;
    return -1;
  }

  Radx::si32 missingVal = Radx::missingSi32;
  Nc3Att *missingValueAtt = var->get_att(MISSING_VALUE);
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_int(0);
    delete missingValueAtt;
  }
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    RadxField *field =
      _rays[ii]->addField(name, units, _nGates,
                          missingVal,
                          data + ii * _nGates,
                          scale, offset,
                          true);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);
  }
  
  delete[] data;
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add si16 fields to _rays
// The _rays array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int ForayNcRadxFile::_addSi16FieldToRays(Nc3Var* var, int nPoints,
                                         const string &name,
                                         const string &units,
                                         const string &standardName,
                                         const string &longName,
                                         double scale, double offset)
  
{

  Radx::si16 *data = new Radx::si16[nPoints];

  if (!var->get(data, _rays.size(), _nGates)) {
    delete[] data;
    return -1;
  }
  
  Radx::si16 missingVal = Radx::missingSi16;
  Nc3Att *missingValueAtt = var->get_att(MISSING_VALUE);
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_short(0);
    delete missingValueAtt;
  }
  
  if (_dataIsUnsigned) {
    // convert from unsigned to signed if needed
    // NTDA data is written as unsigned values
    Radx::ui16 *uval = (Radx::ui16 *) data;
    Radx::si16 *sval = (Radx::si16 *) data;
    for (int ii = 0; ii < nPoints; ii++, uval++, sval++) {
      int ival = *uval - 32768;
      *sval = ival;
    }
    offset += scale * 32768.0;
    missingVal -= 32768;
  }
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    RadxField *field =
      _rays[ii]->addField(name, units, _nGates,
                          missingVal,
                          data + ii * _nGates,
                          scale, offset,
                          true);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);
  }
  
  delete[] data;
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add si08 fields to _rays
// The _rays array has previously been set up by _createRays()
// Returns 0 on success, -1 on failure

int ForayNcRadxFile::_addSi08FieldToRays(Nc3Var* var, int nPoints,
                                         const string &name,
                                         const string &units,
                                         const string &standardName,
                                         const string &longName,
                                         double scale, double offset)
  
{

  Radx::si08 *data = new Radx::si08[nPoints];

  if (!var->get((ncbyte *) data, _rays.size(), _nGates)) {
    delete[] data;
    return -1;
  }

  Radx::si08 missingVal = Radx::missingSi08;
  Nc3Att *missingValueAtt = var->get_att(MISSING_VALUE);
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_ncbyte(0);
    delete missingValueAtt;
  }
  
  if (_dataIsUnsigned) {
    // convert from unsigned to signed if needed
    // NTDA data is written as unsigned values
    Radx::ui08 *uval = (Radx::ui08 *) data;
    Radx::si08 *sval = (Radx::si08 *) data;
    for (int ii = 0; ii < nPoints; ii++, uval++, sval++) {
      int ival = *uval - 128;
      *sval = ival;
    }
    offset += scale * 128.0;
    missingVal -= 128;
  }
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    RadxField *field =
      _rays[ii]->addField(name, units, _nGates,
                          missingVal,
                          data + ii * _nGates,
                          scale, offset,
                          true);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);
  }
  
  delete[] data;
  return 0;

}

/////////////////////////////////////////////////////////////
// finalize the read volume

int ForayNcRadxFile::_finalizeReadVolume()
  
{

  _readVol->setOrigFormat("FORAY");

  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setInstrumentType(_instrumentType);
  _readVol->setPlatformType(_platformType);
  
  double freq = Radx::LIGHT_SPEED / _Wavelength;
  _readVol->addFrequencyHz(freq);

  _readVol->setRadarAntennaGainDbH(_ant_gain_h_db);
  _readVol->setRadarAntennaGainDbV(_ant_gain_v_db);
  _readVol->setRadarBeamWidthDegH(_bm_width);
  _readVol->setRadarBeamWidthDegV(_bm_width);
  
  _readVol->setStartTime(_startTimeSecs, _startNanoSecs);
  _readVol->setEndTime(_endTimeSecs, _endNanoSecs);

  _readVol->setTitle(_title);
  _readVol->setSource(_source);
  _readVol->setHistory(_history);
  _readVol->setInstitution(_institution);
  _readVol->setReferences(_references);
  _readVol->setComment(_comment);
  _readVol->setSiteName(_siteName);
  _readVol->setScanName(_scanModeStr);
  _readVol->setInstrumentName(_instrumentName);

  _readVol->setLatitudeDeg(_Latitude);
  _readVol->setLongitudeDeg(_Longitude);
  _readVol->setAltitudeKm(_Altitude / 1000.0);
  
  RadxRcalib *cal = new RadxRcalib(_rCal);
  _readVol->addCalib(cal);

  if (_Fixed_Angle < -9990) {
    _readVol->computeFixedAnglesFromRays();
  }

  // set max range

  if (_readSetMaxRange) {
    _readVol->setMaxRangeKm(_readMaxRangeKm);
  }
  
  // remove rays with all missing data, if requested

  if (_readRemoveRaysAllMissing) {
    _readVol->removeRaysWithDataAllMissing();
  }

  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();
  
  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - ForayNcRadxFile::_finalizeReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                      _readStrictAngleLimits)) {
      _addErrStr("ERROR - ForayNcRadxFile::_finalizeReadVolume");
      _addErrStr("  No data found within sweep num limits");
      _addErrInt("  min sweep num: ", _readMinSweepNum);
      _addErrInt("  max sweep num: ", _readMaxSweepNum);
      return -1;
    }
  }

  // check for indexed rays, set info on rays
  
  _readVol->checkForIndexedRays();
  
  // load up volume info from rays

  _readVol->loadVolumeInfoFromRays();

  return 0;

}

