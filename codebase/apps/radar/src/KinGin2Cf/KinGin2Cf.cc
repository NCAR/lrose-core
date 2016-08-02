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
///////////////////////////////////////////////////////////////
// KinGin2Cf.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2012
//
///////////////////////////////////////////////////////////////

#include "KinGin2Cf.hh"
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <didss/DataFileNames.hh>
#include <dsserver/DsLdataInfo.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/DoradeRadxFile.hh>
#include <Radx/ForayNcRadxFile.hh>
#include <Radx/NexradRadxFile.hh>
#include <Radx/UfRadxFile.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
using namespace std;

// Constructor

KinGin2Cf::KinGin2Cf(int argc, char **argv)
  
{

  OK = TRUE;
  _prevTime = -1;
  _volumeNumber = 0;
  _clear();

  // set programe name

  _progName = "KinGin2Cf";
  
  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = FALSE;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list, &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

  // check on overriding radar location

  if (_params.override_radar_location) {
    if (_params.radar_latitude_deg < -900 ||
        _params.radar_longitude_deg < -900 ||
        _params.radar_altitude_meters < -900) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Problem with command line or TDRP parameters." << endl;
      cerr << "  You have chosen to override radar location" << endl;
      cerr << "  You must override latitude, longitude and altitude" << endl;
      cerr << "  You must override all 3 values." << endl;
      OK = FALSE;
    }
  }

}

// destructor

KinGin2Cf::~KinGin2Cf()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int KinGin2Cf::Run()
{

  if (_params.mode == Params::ARCHIVE) {
    return _runArchive();
  } else if (_params.mode == Params::FILELIST) {
    return _runFilelist();
  } else {
    return _runRealtime();
  }
}

//////////////////////////////////////////////////
// Run in filelist mode

int KinGin2Cf::_runFilelist()
{

  // loop through the input file list

  int iret = 0;

  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {
    
    string inputPath = _args.inputFileList[ii];
    if (_processFile(inputPath)) {
      iret = -1;
    }
    
  }

  // write out last volume

  if (_writeVolume()) {
    cerr << "ERROR - KinGin2Cf::_runFileList()" << endl;
    cerr << "  Cannot write file" << endl;
    return -1;
  }

  return iret;

}

//////////////////////////////////////////////////
// clear out data

void KinGin2Cf::_clear()
{

  _sweepNumber = 0;
  _startTimeSecs = 0;
  _endTimeSecs = 0;
  _startNanoSecs = 0;
  _endNanoSecs = 0;

  _sweepStartTime.clear();
  _nRaysPerSweep.clear();
  
  _vol.clear();
  _vol.setInstrumentName(_params.radar_name);
  _vol.setSiteName(_params.site_name);

}


//////////////////////////////////////////////////
// Run in archive mode

int KinGin2Cf::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setModeValid(_params.input_dir, _args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - KinGin2Cf::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - KinGin2Cf::_runFilelist()" << endl;
    cerr << "  No files found, dir: " << _params.input_dir << endl;
    return -1;
  }
  
  // loop through the input file list
  
  int iret = 0;
  for (size_t ii = 0; ii < paths.size(); ii++) {
    if (_processFile(paths[ii])) {
      iret = -1;
    }
  }

  // write out last volume
  
  if (_writeVolume()) {
    cerr << "ERROR - KinGin2Cf::_runFileList()" << endl;
    cerr << "  Cannot write file" << endl;
    return -1;
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode

int KinGin2Cf::_runRealtime()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  // watch for new data to arrive

  LdataInfo ldata(_params.input_dir,
                  _params.debug >= Params::DEBUG_VERBOSE);
  
  int iret = 0;

  while (true) {
    ldata.readBlocking(_params.max_realtime_data_age_secs,
                       1000, PMU_auto_register);
    
    const string path = ldata.getDataPath();
    if (_processFile(path)) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Process a file
// Returns 0 on success, -1 on failure

int KinGin2Cf::_processFile(const string &filePath)
{

  // get time for file

  Path path(filePath);
  string fileName(path.getFile());
  const char *start = fileName.c_str();
  RadxTime volStartTime;
  bool timeFound = false;
  while (*start != '\0') {
    int year, month, day, hour, min, sec;
    if (sscanf(start, "%4d%2d%2d_%2d%2d%2d",
               &year, &month, &day, &hour, &min, &sec) == 6) {
      volStartTime.set(year, month, day, hour, min, sec);
      timeFound = true;
      break;
    }
    start++;
  }

  if (!timeFound) {
    cerr << "ERROR - KinGin2Cf::_processFile()" << endl;
    cerr << "  Cannot compute time from file name: " << fileName << endl;
    return -1;
  }

  // Write out previous data if time has changed, or if we
  // are not aggregating sweeps
  
  if (volStartTime.utime() != _prevTime) {
    if (_writeVolume()) {
      cerr << "ERROR - KinGin2Cf::_processFile()" << endl;
      cerr << "  Cannot write file" << endl;
      return -1;
    }
    _prevTime = volStartTime.utime();
  }

  if (_params.debug) {
    cerr << "Processing file: " << filePath << endl;
    cerr << "  time from path: " << RadxTime::strm(volStartTime.utime()) << endl;
  }

  // read in the file
  
  if (_readFile(filePath)) {
    cerr << "ERROR - KinGin2Cf::_processFile()" << endl;
    cerr << "  Cannot read file: " << filePath << endl;
    return -1;
  }

  // increment sweep number

  _sweepNumber++;

  return 0;

}

//////////////////////////////////////////////////
// read in file

int KinGin2Cf::_readFile(const string &filePath)
{

  if (_openRead(filePath)) {
    cerr << "ERROR - KinGin2Cf::_readFile" << endl;
    return -1;
  }

  // read dimensions
  
  if (_readDimensions()) {
    cerr << "ERROR - KinGin2Cf::_readFile" << endl;
    return -1;
  }

  // read global attributes

  _readGlobalAttributes();

  // read scalar variables

  if (_readScalarVariables()) {
    cerr << "ERROR - KinGin2Cf::_readFile" << endl;
    return -1;
  }

  // read range

  if (_readRangeVariable()) {
    cerr << "ERROR - KinGin2Cf::_readFile" << endl;
    return -1;
  }
  
  // read ray variables

  if (_readRayVariables()) {
    cerr << "ERROR - KinGin2Cf::_readFile" << endl;
    return -1;
  }

  // read fields

  if (_readFieldVariables()) {
    cerr << "ERROR - KinGin2Cf::_readFile" << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////
// open netcdf file for reading
// Returns 0 on success, -1 on failure

int KinGin2Cf::_openRead(const string &path)
  
{

  _close();
  _file = new NcFile(path.c_str(), NcFile::ReadOnly);
  
  // Check that constructor succeeded
  
  if (!_file || !_file->is_valid()) {
    cerr << "ERROR - KinGin2Cf::_openRead" << endl;
    cerr << "  Cannot open file for reading, path: " << path << endl;
    return -1;
  }
  
  // Change the error behavior of the netCDF C++ API by creating an
  // NcError object. Until it is destroyed, this NcError object will
  // ensure that the netCDF C++ API silently returns error codes
  // on any failure, and leaves any other error handling to the
  // calling program.

  if (_err == NULL) {
    _err = new NcError(NcError::silent_nonfatal);
  }

  return 0;

}

//////////////////////////////////////
// close netcdf file if open
// remove error object if it exists

void KinGin2Cf::_close()
  
{
  
  // close file if open, delete ncFile
  
  if (_file) {
    _file->close();
    delete _file;
    _file = NULL;
  }
  
  if (_err) {
    delete _err;
    _err = NULL;
  }

}

///////////////////////////////////
// read in the dimensions

int KinGin2Cf::_readDimensions()

{

  // read required dimensions

  int iret = 0;
  iret |= _readDim("BINS", _binsDim);
  iret |= _readDim("RAYS", _raysDim);
  
  if (iret) {
    cerr << "ERROR - KinGin2Cf::_readDimensions" << endl;
    return -1;
  }

  _nBins = _binsDim->size();
  _nRaysSweep = _raysDim->size();

  return 0;

}

///////////////////////////////////////////
// read a dimension

int KinGin2Cf::_readDim(const string &name, NcDim* &dim)

{
  dim = _file->get_dim(name.c_str());
  if (dim == NULL) {
    cerr << "ERROR - KinGin2Cf::_readDim" << endl;
    cerr << "  Cannot find dimension, name: " << name << endl;
    cerr << _err->get_errmsg() << endl;
    return -1;
  }
  return 0;
}

///////////////////////////////////
// read the global attributes

void KinGin2Cf::_readGlobalAttributes()

{

  for (int ii = 0; ii < _file->num_atts(); ii++) {
    
    NcAtt* att = _file->get_att(ii);
    
    if (att == NULL) {
      continue;
    }
    
    if (!strcmp(att->name(), "Convention")) {
      _convention = _asString(att);
    }
    
    if (!strcmp(att->name(), "BinNumber")) {
      _binNumber = att->as_int(0);
    }

    if (!strcmp(att->name(), "RayNumber")) {
      _rayNumber = att->as_int(0);
    }
    
    // Caller must delete attribute
    
    delete att;
    
  } // ii

}

///////////////////////////////////////////
// get string representation of component

string KinGin2Cf::_asString(const NcTypedComponent *component,
                            int index /* = 0 */)
  
{
  
  const char* strc = component->as_string(index);
  string strs(strc);
  delete[] strc;
  return strs;

}

///////////////////////////////////
// read the scalar variables
//
// Only cause failure on required items

int KinGin2Cf::_readScalarVariables()
  
{
  
  int iret = 0;

  _readIntVar("PDP_COR_ON", _PDP_COR_ON, false);

  if (_params.override_radar_location) {
    _Radar_lat = _params.radar_latitude_deg;
    _Radar_lon = _params.radar_longitude_deg;
    _Radar_altitude = _params.radar_altitude_meters;
  } else {
    iret |= _readDoubleVar("Radar_lat", _Radar_lat, true);
    iret |= _readDoubleVar("Radar_lon", _Radar_lon, true);
    iret |= _readDoubleVar("Radar_altitude", _Radar_altitude, true);
  }
    
  _readDoubleVar("FREQ", _FREQ, false);
  _readDoubleVar("PRF_HIGH", _PRF_HIGH, false);
  _readDoubleVar("PRF_LOW", _PRF_LOW, false);
  _readDoubleVar("NOISE_H", _NOISE_H, false);
  _readDoubleVar("NOISE_V", _NOISE_V, false);
  _readDoubleVar("NYQVEL", _NYQVEL, false);
  _readDoubleVar("A_CONST_AH", _A_CONST_AH, false);
  _readDoubleVar("B_CONST_AH", _B_CONST_AH, false);
  _readDoubleVar("A_CONST_ADP", _A_CONST_ADP, false);
  _readDoubleVar("B_CONST_ADP", _B_CONST_ADP, false);
  
  iret |= _readDoubleVar("SCANTIME", _SCANTIME, true);

  if (iret) {
    cerr << "ERROR - KinGin2Cf::_readScalarVariables" << endl;
    return -1;
  }

  int year = (int) (_SCANTIME / 1.0e10 + 0.0001);
  double fac = year * 1.0e10;
  int month = (int) ((_SCANTIME - fac) / 1.0e8 + 0.0001);
  fac += month * 1.0e8;
  int day = (int) ((_SCANTIME - fac) / 1.0e6 + 0.0001);
  fac += day * 1.0e6;
  int hour = (int) ((_SCANTIME - fac) / 1.0e4 + 0.0001);
  fac += hour * 1.0e4;
  int min = (int) ((_SCANTIME - fac) / 1.0e2 + 0.0001);
  fac += min * 1.0e2;
  int sec = (int) ((_SCANTIME - fac) + 0.0001);

  _scanTime.set(year, month, day, hour, min, sec);
  if (_params.debug) {
    cerr << "  _scanTime from data: " << DateTime::strm(_scanTime.utime()) << endl;
  }
  _sweepStartTime.push_back(_scanTime.utime());

  return 0;

}

///////////////////////////////////
// read int variable

int KinGin2Cf::_readIntVar(const string &name, int &val, bool required)
  
{
  
  NcVar*var = _file->get_var(name.c_str());
  if (var == NULL) {
    if (required) {
      cerr << "ERROR - KinGin2Cf::_readIntVar" << endl;
      cerr << "  Cannot read variable, name: " << name << endl;
      cerr << _err->get_errmsg() << endl;
    }
    return -1;
  }
  
  // check size
  
  if (var->num_vals() < 1) {
    if (required) {
      cerr << "ERROR - KinGin2Cf::_readIntVar" << endl;
      cerr << "  variable name: " << name << endl;
      cerr << "  variable has no data" << endl;
    }
    return -1;
  }

  val = var->as_int(0);
  
  return 0;
  
}

///////////////////////////////////
// read double variable

int KinGin2Cf::_readDoubleVar(const string &name, double &val, bool required)

{
  
  NcVar* var = _file->get_var(name.c_str());
  if (var == NULL) {
    if (required) {
      cerr << "ERROR - KinGin2Cf::_readDoubleVar" << endl;
      cerr << "  Cannot read variable, name: " << name << endl;
      cerr << _err->get_errmsg() << endl;
    }
    return -1;
  }

  // check size
  
  if (var->num_vals() < 1) {
    if (required) {
      cerr << "ERROR - KinGin2Cf::_readDoubleVar" << endl;
      cerr << "  variable name: " << name << endl;
      cerr << "  variable has no data" << endl;
    }
    return -1;
  }

  val = var->as_double(0);
  
  return 0;
  
}

///////////////////////////////////
// read the range variable

int KinGin2Cf::_readRangeVariable()

{

  // initialize

  vector<double> ranges;

  // get var
  
  NcVar *var = _file->get_var("RANGE");
  if (var == NULL) {
    cerr << "ERROR - KinGin2Cf::_getRangeVariable" << endl;
    cerr << "  Cannot read RANGE variable" << endl;
    cerr << _err->get_errmsg() << endl;
    return -1;
  }

  // check dimension
  
  if (var->num_dims() < 1) {
    cerr << "ERROR - KinGin2Cf::_getRangeVariable" << endl;
    cerr << "  RANGE variable has no dimensions" << endl;
    return -1;
  }
  NcDim *rangeDim = var->get_dim(0);
  if (rangeDim != _binsDim) {
    cerr << "ERROR - KinGin2Cf::_getRangeVariable" << endl;
    cerr << "  RANGE variable has incorrect dimension" << endl;
    cerr << "  dim name: " << rangeDim->name() << endl;
    cerr << "  should be: BINS" << endl;
    return -1;
  }

  // load up data
  
  float *data = new float[_nBins];
  float *dd = data;
  _ranges.clear();
  if (var->get(data, _nBins)) {
    for (int ii = 0; ii < _nBins; ii++, dd++) {
      _ranges.push_back(*dd);
    }
  } else {
    cerr << "ERROR - KinGin2Cf::_readRangeVariable" << endl;
    cerr << "  Cannot read RANGE variable" << endl;
    cerr << _err->get_errmsg() << endl;
    delete[] data;
    return -1;
  }
  delete[] data;

  _startRangeKm = _ranges[0] / 1000.0;
  _gateSpacingKm =
    ((_ranges[_nBins-1] - _ranges[0]) / (_nBins - 1.0)) / 1000.0;
  
  _geom.setStartRangeKm(_startRangeKm);
  _geom.setGateSpacingKm(_gateSpacingKm);

  return 0;

}

///////////////////////////////////
// read the ray meta-data

int KinGin2Cf::_readRayVariables()

{

  // initialize

  vector<double> azimuths, elevations;
  vector<double> time_offsets;

  int iret = 0;

  _readRayVar("Azimuth", azimuths);
  if ((int) azimuths.size() < _nRaysSweep) {
    cerr << "ERROR - Azimuth variable required" << endl;
    iret = -1;
  }

  _readRayVar("Elevation", elevations);
  if ((int) elevations.size() < _nRaysSweep) {
    cerr << "ERROR - Elevation variable required" << endl;
    iret = -1;
  }
  
  
  // set time offsets to = 100 deg/sec
  // (NOTE - should later be in data file)
  
  for (int ii = 0; ii < _nRaysSweep; ii++) {
    time_offsets.push_back(ii * 0.01);
  }
  
  if (iret) {
    cerr << "ERROR - KinGin2Cf::_readRayVariables" << endl;
    return -1;
  }

  _rays.clear();
  for (int ii = 0; ii <  _raysDim->size(); ii++) {

    double rayTimeDouble = (double) _scanTime.utime() + time_offsets[ii];
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

    ray->setVolumeNumber(_volumeNumber);
    ray->setSweepNumber(_sweepNumber);
    ray->setCalibIndex(0);
    ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
    ray->setPolarizationMode(Radx::POL_MODE_HV_SIM);
    ray->setPrtMode(Radx::PRT_MODE_FIXED);
    ray->setFollowMode(Radx::FOLLOW_MODE_NONE);

    if ((int) azimuths.size() > ii) {
      ray->setAzimuthDeg(azimuths[ii]);
    }
    
    if ((int) elevations.size() > ii) {
      ray->setElevationDeg(elevations[ii]);
    }
    
    ray->setFixedAngleDeg(ray->getElevationDeg());
    ray->setPulseWidthUsec(_params.pulse_width_usec);
    ray->setPrtSec(1.0 / _PRF_HIGH);
    ray->setPrtRatio(_PRF_HIGH / _PRF_LOW);
    ray->setNyquistMps(_NYQVEL);
    double unambigRangeKm = (Radx::LIGHT_SPEED / (2.0 * _PRF_HIGH)) / 1000.0; 
    ray->setUnambigRangeKm(unambigRangeKm);
    ray->setAntennaTransition(false);
    ray->setNSamples(_params.n_samples);
    ray->setCalibIndex(0);
    ray->setMeasXmitPowerDbmH(-9999.0);
    ray->setMeasXmitPowerDbmV(-9999.0);
    ray->setRangeGeom(_startRangeKm, _gateSpacingKm);

    _rays.push_back(ray);  // for use by readRayFields()
    _vol.addRay(ray); // _vol will free ray memory
    
  } // ii

  _nRaysPerSweep.push_back(_rays.size());

  return 0;

}

///////////////////////////////////
// read a ray variable - double

int KinGin2Cf::_readRayVar(const string &name, vector<double> &vals)
  
{

  vals.clear();

  // get var

  NcVar* var = _getRayVar(name);
  if (var == NULL) {
    cerr << "ERROR - KinGin2Cf::_readRayVar" << endl;
    return -1;
  }

  // load up data
  
  double *data = new double[_nRaysSweep];
  double *dd = data;
  if (var->get(data, _nRaysSweep)) {
    for (int ii = 0; ii < _nRaysSweep; ii++, dd++) {
      vals.push_back(*dd);
    }
  } else {
    cerr << "ERROR - KinGin2Cf::_readRayVar" << endl;
    cerr << "  Cannot read variable: " << name << endl;
    cerr << _err->get_errmsg() << endl;
    return -1;
  }
  delete[] data;

  return 0;

}

///////////////////////////////////
// get a ray variable by name
// returns NULL on failure

NcVar* KinGin2Cf::_getRayVar(const string &name)

{

  // get var

  NcVar *var = _file->get_var(name.c_str());
  if (var == NULL) {
    cerr << "ERROR - KinGin2Cf::_getRayVar" << endl;
    cerr << "  Cannot read variable, name: " << name << endl;
    cerr << _err->get_errmsg() << endl;
    return NULL;
  }

  // check rays dimension
  
  if (var->num_dims() < 1) {
    cerr << "ERROR - KinGin2Cf::_getRayVar" << endl;
    cerr << "  variable name: " << name << endl;
    cerr << "  variable has no dimensions" << endl;
    return NULL;
  }

  NcDim *raysDim = var->get_dim(0);
  if (raysDim != _raysDim) {
    cerr << "ERROR - KinGin2Cf::_getRayVar" << endl;
    cerr << "  variable name: " << name << endl;
    cerr << "  variable has incorrect dimension, dim name: "
         << raysDim->name() << endl;
    cerr << "  should be: RAYS" << endl;
    return NULL;
  }

  return var;

}

////////////////////////////////////////////
// read the field variables

int KinGin2Cf::_readFieldVariables()
  
{
  
  int iret = 0;

  // loop through the variables, adding data fields as appropriate
  
  for (int ifld = 0; ifld < _params.output_fields_n; ifld++) {
    
    const Params::output_field_t &field = _params._output_fields[ifld];
    string inputFieldName = field.input_field_name;
    NcVar* var = _file->get_var(inputFieldName.c_str());
    if (var == NULL) {
      continue;
    }
    
    // we need fields with 2 dimensions
    
    int numDims = var->num_dims();
    if (numDims != 2) {
      continue;
    }
    
    // check that we have the correct dimensions
    
    NcDim* raysDim = var->get_dim(0);
    NcDim* binsDim = var->get_dim(1);
    if (raysDim != _raysDim || binsDim != _binsDim) {
      continue;
    }
    
    // check the type
    NcType ftype = var->type();
    if (ftype != ncFloat) {
      // not a valid type
      cerr << "WARNING - field: " << inputFieldName << endl;
      cerr << "  Not a float type field - ignoring" << endl;
      continue;
    }

    string outputFieldName = field.output_field_name;
    string units = field.output_units;
    string longName = field.long_name;
    string standardName = field.standard_name;
    Params::output_encoding_t encoding = field.encoding;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "DEBUG - KinGin2Cf::_readFieldVariables" << endl;
      cerr << "  -->> adding input field: " << inputFieldName << endl;
      cerr << "  -->>       output field: " << outputFieldName << endl;
    }
    
    if (_addFl32FieldToRays(var, outputFieldName, 
                            units, standardName, longName, encoding)) {
      cerr << "ERROR - KinGin2Cf::_readFieldVariables" << endl;
      iret = -1;
    }
    
  } // ifld

  return iret;

}

/////////////////////////////////////////////////////////////////////
// Add fl32 fields to _rays.
// The _rays array has previously been set up by _readRayVariables().
// Returns 0 on success, -1 on failure

int KinGin2Cf::_addFl32FieldToRays(NcVar* var,
                                   const string &name,
                                   const string &units,
                                   const string &standardName,
                                   const string &longName,
                                   Params::output_encoding_t encoding)
  
{
  
  int nPoints = _nRaysSweep * _nBins;
  Radx::fl32 *data = new Radx::fl32[nPoints];

  if (!var->get(data, _nRaysSweep, _nBins)) {
    delete[] data;
    cerr << "ERROR - KinGin2Cf::_addFl32FieldToRays" << endl;
    cerr << "  cannot read field name: " << var->name() << endl;
    cerr << _err->get_errmsg() << endl;
    return -1;
  }

  Radx::fl32 missingVal = -1000.0f;
  NcAtt *missingValueAtt = var->get_att("missing_value");
  if (missingValueAtt != NULL) {
    missingVal = missingValueAtt->as_float(0);
    delete missingValueAtt;
  }
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    RadxField *field =
      _rays[ii]->addField(name, units, _nBins,
                          missingVal,
                          data + ii * _nBins,
                          true);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->copyRangeGeom(_geom);
    switch(encoding) {
      case Params::OUTPUT_ENCODING_FLOAT32:
        field->convertToFl32();
        break;
      case Params::OUTPUT_ENCODING_INT32:
        field->convertToSi32();
        break;
      case Params::OUTPUT_ENCODING_INT08:
        field->convertToSi08();
        break;
      case Params::OUTPUT_ENCODING_INT16:
      default:
        field->convertToSi16();
    }
  }
  
  delete[] data;
  return 0;
  
}

//////////////////////////////////////////////////
// write out volume

int KinGin2Cf::_writeVolume()
{

  if (_vol.getNRays() == 0) {
    _vol.clear();
    return 0;
  }

  // set the ray times by interpolation / extrapolation

  _setRayTimes();

  // compute fixed angles

  _computeFixedAngles();

  // radar location

  _vol.setLatitudeDeg(_Radar_lat);
  _vol.setLongitudeDeg(_Radar_lon);
  _vol.setAltitudeKm(_Radar_altitude / 1000.0);

  // metadata

  _vol.addFrequencyHz(_FREQ * 1.0e6);
  RadxRcalib *cal = new RadxRcalib;
  cal->setNoiseDbmHc(_NOISE_H);
  cal->setNoiseDbmVc(_NOISE_V);

  _vol.loadVolumeInfoFromRays();
  _vol.loadSweepInfoFromRays();

  _vol.setStartTime(_startTimeSecs, _startNanoSecs);
  _vol.setEndTime(_endTimeSecs, _endNanoSecs);

  // set number of gates constant if requested

  if (_params.set_ngates_constant) {
    _vol.setNGatesConstant();
  }

  // write out file

  RadxFile *outFile;

  switch (_params.output_format) {
    
    case Params::OUTPUT_FORMAT_UF: {
      
      UfRadxFile *ufFile = new UfRadxFile();
      outFile = ufFile;
      _setupWrite(*outFile);
      
    } break; // end UF
      
    case Params::OUTPUT_FORMAT_DORADE: {

      DoradeRadxFile *doradeFile = new DoradeRadxFile();
      outFile = doradeFile;
      _setupWrite(*outFile);
      
    } break; // end DORADE
      
    case Params::OUTPUT_FORMAT_FORAY: {

      ForayNcRadxFile *forayFile = new ForayNcRadxFile();
      outFile = forayFile;
      _setupWrite(*outFile);
      
    } break; // end FORAY
      
    case Params::OUTPUT_FORMAT_NEXRAD: {

      NexradRadxFile *nexradFile = new NexradRadxFile();
      outFile = nexradFile;
      _setupWrite(*outFile);
      
    } break; // end NEXRAD
      
    default:
    case Params::OUTPUT_FORMAT_CFRADIAL: {
      
      NcfRadxFile *ncfFile = new NcfRadxFile();
      outFile = ncfFile;
      _setupWrite(*outFile);
      switch (_params.netcdf_style) {
        case Params::NETCDF4_CLASSIC:
          ncfFile->setNcFormat(RadxFile::NETCDF4_CLASSIC);
          break;
        case Params::NC64BIT:
          ncfFile->setNcFormat(RadxFile::NETCDF_OFFSET_64BIT);
          break;
        case Params::NETCDF4:
          ncfFile->setNcFormat(RadxFile::NETCDF4);
          break;
        default:
          ncfFile->setNcFormat(RadxFile::NETCDF_CLASSIC);
      }
    } // end CFRADIAL

  } // switch

  if (outFile->writeToDir(_vol, _params.output_dir,
                          _params.append_day_dir_to_output_dir,
                          _params.append_year_dir_to_output_dir)) {
    cerr << "ERROR - KinGin2Cf::_processFile" << endl;
    cerr << "  Cannot write file to dir: " << _params.output_dir << endl;
    cerr << outFile->getErrStr() << endl;
    delete outFile;
    _vol.clear();
    return -1;
  }
  string outputPath = outFile->getPathInUse();
  delete outFile;

  // in realtime mode, write latest data info file

  if (_params.mode == Params::REALTIME) {
    DsLdataInfo ldata(_params.output_dir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    RadxPath rpath(outputPath);
    ldata.setRelDataPath(rpath.getFile());
    ldata.setWriter(_progName);
    if (ldata.write(_vol.getEndTimeSecs())) {
      cerr << "WARNING - KinGin2Cf::_processFile" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  _clear();
  return 0;

}

//////////////////////////////////////////////////
// set up write

void KinGin2Cf::_setupWrite(RadxFile &file)
{

  if (_params.debug) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.compute_output_path_using_end_time) {
    file.setWriteUsingEndTime(true);
  } else {
    file.setWriteUsingEndTime(false);
  }

  if (_params.output_compressed) {
    file.setWriteCompressed(true);
    file.setCompressionLevel(_params.compression_level);
  } else {
    file.setWriteCompressed(false);
  }

  if (_params.output_native_byte_order) {
    file.setWriteNativeByteOrder(true);
  } else {
    file.setWriteNativeByteOrder(false);
  }

  if (_params.output_format == Params::OUTPUT_FORMAT_DORADE) {
    file.setFileFormat(RadxFile::FILE_FORMAT_DORADE);
  } else if (_params.output_format == Params::OUTPUT_FORMAT_NEXRAD) {
    file.setFileFormat(RadxFile::FILE_FORMAT_NEXRAD_AR2);
  } else if (_params.output_format == Params::OUTPUT_FORMAT_UF) {
    file.setFileFormat(RadxFile::FILE_FORMAT_UF);
  } else {
    // assume CfRadial
    file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  }

  if (_params.write_individual_sweeps) {
    file.setWriteIndividualSweeps(true);
  }

}

//////////////////////////////////////////////////
// set up the ray times

void KinGin2Cf::_setRayTimes()
{

  if (_sweepStartTime.size() < 2) {
    return;
  }
  
  size_t count = 0;
  double rayRate = 0;
  
  for (size_t ii = 0; ii < _sweepStartTime.size(); ii++) {
    
    // compute rate for sweep - except for last sweep
    
    if (ii < _sweepStartTime.size() - 1) {
      double deltaSecs = _sweepStartTime[ii+1] - _sweepStartTime[ii];
      rayRate = deltaSecs / _nRaysPerSweep[ii];
    }
    
    // set times on rays for sweep
    // for last sweep use rate from previous sweep

    double rayTimeDouble = _sweepStartTime[ii];
    for (int jj = 0; jj < _nRaysPerSweep[ii]; jj++) {
      
      time_t rayUtimeSecs = (time_t) rayTimeDouble;
      double rayFracSecs = rayTimeDouble - rayUtimeSecs;
      int rayNanoSecs = (int) (rayFracSecs * 1.0e9);
      
      RadxRay *ray = _vol.getRays()[count];
      ray->setTime(rayUtimeSecs, rayNanoSecs);
      
      rayTimeDouble += rayRate;
      count++;

    }

  } // ii

}

//////////////////////////////////////////////////
// compute the fixed angle for sweeps by computing
// the mean angle

void KinGin2Cf::_computeFixedAngles()
{

  int count = 0;
  
  for (size_t ii = 0; ii < _sweepStartTime.size(); ii++) {

    double sumAngle = 0.0;
    
    for (int jj = 0; jj < _nRaysPerSweep[ii]; jj++) {
      RadxRay *ray = _vol.getRays()[count + jj];
      sumAngle += ray->getElevationDeg();
    }
    
    double meanAngle = sumAngle / _nRaysPerSweep[ii];
    meanAngle = ((int) (meanAngle * 100.0 + 0.5)) / 100.0;
    
    for (int jj = 0; jj < _nRaysPerSweep[ii]; jj++) {
      RadxRay *ray = _vol.getRays()[count + jj];
      ray->setFixedAngleDeg(meanAngle);
    }
    
    count += _nRaysPerSweep[ii];

  } // ii

}


