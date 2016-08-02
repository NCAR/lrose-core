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
////////////////////////////////////////////////////////////////////////
// DoeSounding2Spdb.cc
//
// DoeSounding2Spdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2011
//
///////////////////////////////////////////////////////////////
//
// DoeSounding2Spdb reads US DOE sounding data,
// converts them to sounding format and writes them
// to an SPDB data base.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <physics/physics.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/TaStr.hh>
#include <didss/DsInputPath.hh>
#include "DoeSounding2Spdb.hh"
using namespace std;

// Constructor

const double DoeSounding2Spdb::_missingDouble = -9999.0;
const float DoeSounding2Spdb::_missingFloat = -9999.0f;
const int DoeSounding2Spdb::_missingInt = -9999;

DoeSounding2Spdb::DoeSounding2Spdb(int argc, char **argv)

{

  isOK = true;
  _ncFile = NULL;
  _ncErr = NULL;

  // set programe name

  _progName = "DoeSounding2Spdb";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
    return;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

DoeSounding2Spdb::~DoeSounding2Spdb()

{

  // close input file if open

  _closeNc();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int DoeSounding2Spdb::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // file input object
  
  DsInputPath *input = NULL; // Set to NULL to get around compiler warnings.
  
  if (_params.mode == Params::FILELIST) {
    
    // FILELIST mode
    
    input = new DsInputPath(_progName,
			    _params.debug >= Params::DEBUG_VERBOSE,
			    _args.inputFileList);
    
  } else if (_params.mode == Params::ARCHIVE) {
    
    // archive mode - start time to end time
    
    input = new DsInputPath(_progName,
			    _params.debug >= Params::DEBUG_VERBOSE,
			    _params.input_dir,
			    _args.startTime, _args.endTime);
    
  } else if (_params.mode == Params::REALTIME) {
    
    // realtime mode - no latest_data_info file
    
    input = new DsInputPath(_progName,
			    _params.debug >= Params::DEBUG_VERBOSE,
			    _params.input_dir,
			    _params.max_realtime_valid_age,
			    PMU_auto_register,
			    _params.latest_data_info_avail,
			    true);
    
    if (_params.strict_subdir_check) {
      input->setStrictDirScan(true);
    }
    
    if (_params.file_name_check) {
      input->setSubString(_params.file_match_string);
    }
    
  }
  
  // loop through available files
  
  char *inputPath;
  while ((inputPath = input->next()) != NULL) {
    
    if (_processFile(inputPath)) {
      cerr << "WARNING - DoeSounding2Spdb::Run" << endl;
      cerr << "  Errors in processing file: " << inputPath << endl;
    }
    
  } // while
    
  return 0;

}

////////////////////
// process the file

int DoeSounding2Spdb::_processFile(const char *file_path)
  
{

  int iret = 0;

  if (_params.debug) {
    cerr << "Processing file: " << file_path << endl;
  }
  
  // registration

  char procmapString[BUFSIZ];
  Path path(file_path);
  sprintf(procmapString, "Processing file <%s>", path.getFile().c_str());
  PMU_force_register(procmapString);

  // Set the launch location

  _stationLat = _params.station_latitude;
  _stationLon = _params.station_longitude;
  _stationAlt = _params.station_altitude;

  // open the file

  if (_openNc(file_path)) {
    cerr << "ERROR - DoeSounding2Spdb::_processFile" << endl;
    cerr << "  Cannot open file: " << file_path << endl;
  }

  // read the dimensions and global attributes

  if (_readDimensions()) {
    cerr << "ERROR - DoeSounding2Spdb::_processFile" << endl;
    cerr << "  Reading dimensions, file: " << file_path << endl;
    _closeNc();
    return -1;
  }

  if (_readGlobalAttributes()) {
    cerr << "ERROR - DoeSounding2Spdb::_processFile" << endl;
    cerr << "  Reading global attributes, file: " << file_path << endl;
    _closeNc();
    return -1;
  }

  if (_readTime()) {
    cerr << "ERROR - DoeSounding2Spdb::_processFile" << endl;
    cerr << "  Reading time vector, file: " << file_path << endl;
    _closeNc();
    return -1;
  }

  if (_params.debug) {
    cerr << "  _nTimesInFile: " << _nTimesInFile << endl;
    cerr << "  _siteId: " << _siteId << endl;
    cerr << "  _facilityId: " << _facilityId << endl;
    cerr << "  _inputSource: " << _inputSource << endl;
    cerr << "  _soundingNumber: " << _soundingNumber << endl;
    cerr << "  _serialNumber: " << _serialNumber << endl;
  }

  // read in the time vector

  if (_readTime()) {
    cerr << "ERROR - DoeSounding2Spdb::_processFile" << endl;
    cerr << "  Reading times, file: " << file_path << endl;
    _closeNc();
    return -1;
  }

  // read in data

  if (_readField(_params.pressure_field_name, _pressure)) {
    return -1;
  }

  if (_readField(_params.pressure_field_name, _pressure)) {
    return -1;
  }

  if (_readField(_params.temp_dry_field_name, _tempDry)) {
    return -1;
  }

  if (_readField(_params.dewpoint_field_name, _dewPoint)) {
    return -1;
  }

  if (_readField(_params.wind_speed_field_name, _windSpeed)) {
    return -1;
  }

  if (_readField(_params.wind_dirn_field_name, _windDirn)) {
    return -1;
  }

  if (_readField(_params.rh_field_name, _rh)) {
    return -1;
  }

  if (_readField(_params.u_wind_field_name, _uWind)) {
    return -1;
  }

  if (_readField(_params.v_wind_field_name, _vWind)) {
    return -1;
  }

  if (_readField(_params.ascent_rate_field_name, _ascentRate)) {
    return -1;
  }

  if (_readField(_params.latitude_field_name, _latitude)) {
    return -1;
  }

  if (_readField(_params.longitude_field_name, _longitude)) {
    return -1;
  }

  if (_readField(_params.altitude_field_name, _altitude)) {
    return -1;
  }

  if (!_params.override_station_location) {
    if (_latitude[0] > -9990) {
      _stationLat = _latitude[0];
    }
    if (_longitude[0] > -9990) {
      _stationLon = _longitude[0];
    }
    if (_altitude[0] > -9990) {
      _stationAlt = _altitude[0];
    }
  }

  // create spdb output object
  
  SoundingPut sounding;
  sounding.init(_params.output_url, Sounding::SONDE_ID, "DOE-NetCDF" );
  sounding.setMissingValue(_missingDouble);

  sounding.setSiteId(0);
  sounding.setSiteName(_siteId);
  sounding.setSourceId(Sounding::SONDE_ID);

  sounding.setLocation(_stationLat, _stationLon, _stationAlt);

  // set the data - may override position

  sounding.set(_time[0], &_altitude, &_uWind, &_vWind, NULL,
               &_pressure, &_rh, &_tempDry, NULL);
  
  // set position

  sounding.setLocation(_stationLat, _stationLon, _stationAlt);

  // put the data

  time_t validTime = _time[0];
  time_t expireTime = validTime + _params.expire_seconds;
  int leadSecs = 0;

  if (sounding.writeSounding(validTime, expireTime, leadSecs)) {
    cerr << "ERROR - DoeSounding2Spdb::_processFile" << endl;
    cerr << "  Cannot put sounding data to: "
	 << _params.output_url << endl;
    iret = -1;
  }

  if (_params.debug) {
    cerr << "  Done with file: " << file_path << endl;
  }

  return iret;
   
}

//////////////////////////////////////
// open netcdf file for reading
// Returns 0 on success, -1 on failure

int DoeSounding2Spdb::_openNc(const string &path)
  
{

  _closeNc();
  _ncFile = new NcFile(path.c_str(), NcFile::ReadOnly);
  
  // Check that constructor succeeded
  
  if (!_ncFile || !_ncFile->is_valid()) {
    cerr << "ERROR - DoeSounding2Spdb::_openNc" << endl;
    cerr << "  Cannot open file for reading, path: " << path << endl;
    return -1;
  }
  
  // Change the error behavior of the netCDF C++ API by creating an
  // NcError object. Until it is destroyed, this NcError object will
  // ensure that the netCDF C++ API silently returns error codes
  // on any failure, and leaves any other error handling to the
  // calling program.

  if (_ncErr == NULL) {
    _ncErr = new NcError(NcError::silent_nonfatal);
  }

  return 0;

}

//////////////////////////////////////
// close netcdf file if open
// remove error object if it exists

void DoeSounding2Spdb::_closeNc()
  
{
  
  // close file if open, delete ncFile
  
  if (_ncFile) {
    _ncFile->close();
    delete _ncFile;
    _ncFile = NULL;
  }
  
  if (_ncErr) {
    delete _ncErr;
    _ncErr = NULL;
  }

}

///////////////////////////////////
// read in the dimensions

int DoeSounding2Spdb::_readDimensions()

{

  // read required dimensions

  _timeDim = _ncFile->get_dim("time");
  if (_timeDim == NULL) {
    cerr << "ERROR - DoeSounding2Spdb::_readDimensions" << endl;
    cerr << "  Cannot find 'time' dimension" << endl;
    return -1;
  }
  _nTimesInFile = _timeDim->size();

  return 0;

}

///////////////////////////////////
// read the global attributes

int DoeSounding2Spdb::_readGlobalAttributes()

{

  NcAtt *att;
  
  att = _ncFile->get_att("site_id");
  if (att != NULL) {
    _siteId = _asString(att);
    delete att;
  }
  
  att = _ncFile->get_att("facility_id");
  if (att != NULL) {
    _facilityId = _asString(att);
    delete att;
  }
  
  att = _ncFile->get_att("input_source");
  if (att != NULL) {
    _inputSource = _asString(att);
    delete att;
  }
  
  att = _ncFile->get_att("sounding_number");
  if (att != NULL) {
    _soundingNumber = _asString(att);
    delete att;
  }
  
  att = _ncFile->get_att("serial_number");
  if (att != NULL) {
    _serialNumber = _asString(att);
    delete att;
  }
  
  return 0;

}

///////////////////////////////////
// read the time vector

int DoeSounding2Spdb::_readTime()

{

  // get base time

  int ival;
  if (_readIntScalar("base_time", ival)) {
    cerr << "ERROR - DoeSounding2Spdb::_readTime" << endl;
    cerr << "  Cannot read 'base_time' variable" << endl;
    return -1;
  }
  _baseTime = (time_t) ival;

  // get time offset variable

  NcVar *timeOffsetVar = _ncFile->get_var("time_offset");
  if (timeOffsetVar == NULL) {
    cerr << "ERROR - DoeSounding2Spdb::_readTime" << endl;
    cerr << "  Cannot read 'time_offset' variable" << endl;
    return -1;
  }
  if (timeOffsetVar->get_dim(0) != _timeDim) {
    cerr << "ERROR - DoeSounding2Spdb::_readTime" << endl;
    cerr << "  time_offset var does not have dimension 'time'" << endl;
    return -1;
  }

  _time.clear();
  _dtime.clear();
  double *timeOffset = new double[timeOffsetVar->num_vals()];
  if (timeOffsetVar->get(timeOffset, timeOffsetVar->num_vals())) {
    double *dd = timeOffset;
    for (int ii = 0; ii < timeOffsetVar->num_vals(); ii++, dd++) {
      double dtime = *dd + _baseTime;
      _time.push_back((time_t) dtime);
      _dtime.push_back(dtime);
    }
  } else {
    delete[] timeOffset;
    cerr << "ERROR - DoeSounding2Spdb::_readTime" << endl;
    cerr << "  Cannot read time_offset var" << endl;
    return -1;
  }
  delete[] timeOffset;

  if (_time.size() < 1) {
    cerr << "ERROR - DoeSounding2Spdb::_readTime" << endl;
    cerr << "  No times in data" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t ii = 0; ii < _time.size(); ii++) {
      int msecs = (int) ((_dtime[ii] - _time[ii]) * 1000.0);
      fprintf(stderr, "ii, time: %d, %s.%.3d\n", (int) ii,
              DateTime::strm(_time[ii]).c_str(), msecs);
    }
  }

  return 0;

}

///////////////////////////////////
// read int scalar

int DoeSounding2Spdb::_readIntScalar(const string &name,
                                     int &val, bool required)
  
{
  
  NcVar* var = _ncFile->get_var(name.c_str());
  if (var == NULL) {
    if (!required) {
      val = _missingInt;
      return 0;
    } else {
      cerr << "ERROR - DoeSounding2Spdb::_readIntScalar" << endl;
      cerr << "  Cannot read variable, name: " << name << endl;
      cerr << _ncErr->get_errmsg() << endl;
      return -1;
    }
  }

  // check size
  
  if (var->num_vals() < 1) {
    cerr << "ERROR - DoeSounding2Spdb::_readIntScalar" << endl;
    cerr << "  variable name: " << name << endl;
    cerr << "  variable has no data" << endl;
    return -1;
  }

  val = var->as_int(0);
  
  return 0;
  
}

///////////////////////////////////
// read float scalar

int DoeSounding2Spdb::_readFloatScalar(const string &name,
                                       float &val, bool required)
  
{
  
  NcVar *var = _ncFile->get_var(name.c_str());
  if (var == NULL) {
    if (!required) {
      val = _missingFloat;
      return 0;
    } else {
      cerr << "ERROR - DoeSounding2Spdb::_readFloatScalar" << endl;
      cerr << "  Cannot read variable, name: " << name << endl;
      cerr << _ncErr->get_errmsg() << endl;
      return -1;
    }
  }

  // check size
  
  if (var->num_vals() < 1) {
    cerr << "ERROR - DoeSounding2Spdb::_readFloatScalar" << endl;
    cerr << "  variable name: " << name << endl;
    cerr << "  variable has no data" << endl;
    return -1;
  }

  val = var->as_float(0);
  
  return 0;
  
}

///////////////////////////////////
// read double scalar

int DoeSounding2Spdb::_readDoubleScalar(const string &name,
                                        double &val, bool required)
  
{
  
  NcVar *var = _ncFile->get_var(name.c_str());
  if (var == NULL) {
    if (!required) {
      val = _missingDouble;
      return 0;
    } else {
      cerr << "ERROR - DoeSounding2Spdb::_readDoubleScalar" << endl;
      cerr << "  Cannot read variable, name: " << name << endl;
      cerr << _ncErr->get_errmsg() << endl;
      return -1;
    }
  }

  // check size
  
  if (var->num_vals() < 1) {
    cerr << "ERROR - DoeSounding2Spdb::_readDoubleScalar" << endl;
    cerr << "  variable name: " << name << endl;
    cerr << "  variable has no data" << endl;
    return -1;
  }

  val = var->as_double(0);
  
  return 0;
  
}

/////////////////////////////////////////////////////
// read a field

int DoeSounding2Spdb::_readField(const string &name,
                                 vector<double> &val)
{
  
  if (name.size() > 0) {
    if (_readVector(name, _timeDim, val)) {
      cerr << "ERROR - DoeSounding2Spdb::_readField" << endl;
      cerr << "  Cannot read in field: " << name << endl;
      _fillVectorMissing(_timeDim, val);
      return -1;
    }
  } else {
    // field not specified, fill with missing
    _fillVectorMissing(_timeDim, val);
  }

  return 0;

}
  
/////////////////////////////////////////////////////
// read data and fill vector

int DoeSounding2Spdb::_readVector(const string &name,
                                  NcDim *dim,
                                  vector<double> &val)
  
{

  NcVar *var = _ncFile->get_var(name.c_str());
  if (var == NULL) {
    cerr << "ERROR - DoeSounding2Spdb::_readFloatVector" << endl;
    cerr << "  Cannot read variable: " << name << endl;
    return -1;
  }
  if (var->get_dim(0) != dim) {
    cerr << "ERROR - DoeSounding2Spdb::_readFloatVector" << endl;
    cerr << "  variable: " << name << endl;
    cerr << "  incorrect dimension found: " << var->get_dim(0)->name() << endl;
    cerr << "  incorrect dimension, should be: " << dim->name() << endl;
    return -1;
  }
  
  val.clear();

  if (var->type() == NC_FLOAT) {

    float *fvals = new float[var->num_vals()];
    if (var->get(fvals, var->num_vals())) {
      float *ff = fvals;
      for (int ii = 0; ii < var->num_vals(); ii++, ff++) {
        val.push_back((double) *ff);
    }
    } else {
      delete[] fvals;
      cerr << "ERROR - DoeSounding2Spdb::_readVector" << endl;
      cerr << "  Cannot read var: " << name << endl;
      return -1;
    }
    delete[] fvals;

  } else if (var->type() == NC_DOUBLE) {

    double *dvals = new double[var->num_vals()];
    if (var->get(dvals, var->num_vals())) {
      double *dd = dvals;
      for (int ii = 0; ii < var->num_vals(); ii++, dd++) {
        val.push_back((double) *dd);
    }
    } else {
      delete[] dvals;
      cerr << "ERROR - DoeSounding2Spdb::_readVector" << endl;
      cerr << "  Cannot read var: " << name << endl;
      return -1;
    }
    delete[] dvals;

  } else if (var->type() == NC_INT) {

    int *ivals = new int[var->num_vals()];
    if (var->get(ivals, var->num_vals())) {
      int *iv = ivals;
      for (int ii = 0; ii < var->num_vals(); ii++, iv++) {
        val.push_back((int) *iv);
    }
    } else {
      delete[] ivals;
      cerr << "ERROR - DoeSounding2Spdb::_readVector" << endl;
      cerr << "  Cannot read var: " << name << endl;
      return -1;
    }
    delete[] ivals;

  }

  return 0;

}

/////////////////////////////////////////////////////
// fill float vector with missing

void DoeSounding2Spdb::_fillVectorMissing(NcDim *dim,
                                          vector<double> &val)

{
  
  val.clear();
  for (long ii = 0; ii < dim->size(); ii++) {
    val.push_back(_missingDouble);
  }

}

///////////////////////////////////////////
// get string representation of component

string DoeSounding2Spdb::_asString(const NcTypedComponent *component,
                                   int index /* = 0 */)
  
{
  
  const char* strc = component->as_string(index);
  string strs(strc);
  delete[] strc;
  return strs;

}

