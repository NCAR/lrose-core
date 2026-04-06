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
// RadxCartDP.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2012
//
///////////////////////////////////////////////////////////////
//
// RadxCartDP reads moments from Radx-supported format files,
// interpolates onto a Cartesian grid, and writes out the
// results to Cartesian files in MDV or NetCDF.
//
///////////////////////////////////////////////////////////////

#include "RadxCartDP.hh"
#include "OutputMdv.hh"
#include "ScalarsCompute.hh"
#include "ScalarsThread.hh"
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <didss/LdataInfo.hh>
#include <radar/BeamHeight.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Mdv/GenericRadxFile.hh>
using namespace std;

// initialize geom field names

string RadxCartDP::elevationFieldName = "ELEV";
string RadxCartDP::rangeFieldName = "RANGE";
string RadxCartDP::beamHtFieldName = "BEAM_HT";
string RadxCartDP::tempFieldName = "TEMP_C";
string RadxCartDP::pidFieldName = "PID";
string RadxCartDP::pidInterestFieldName = "PID_INTEREST";
string RadxCartDP::mlFieldName = "MELTING_LAYER";
string RadxCartDP::mlExtendedFieldName = "ML_EXTENDED";
string RadxCartDP::convFlagFieldName = "CONV_FLAG";

string RadxCartDP::snrForPidFieldName = "SNR_FOR_PID";
string RadxCartDP::dbzForPidFieldName = "DBZ_FOR_PID";
string RadxCartDP::zdrForPidFieldName = "ZDR_FOR_PID";
string RadxCartDP::ldrForPidFieldName = "LDR_FOR_PID";
string RadxCartDP::rhohvForPidFieldName = "RHOHV_FOR_PID";
string RadxCartDP::phidpForPidFieldName = "PHIDP_FOR_PID";
string RadxCartDP::kdpForPidFieldName = "KDP_FOR_PID";
string RadxCartDP::zdrSdevForPidFieldName = "ZDR_SDEV_FOR_PID";
string RadxCartDP::phidpSdevForPidFieldName = "PHIDP_SDEV_FOR_PID";

// Constructor

RadxCartDP::RadxCartDP(int argc, char **argv)
  
{

  OK = true;

  _cartInterp = NULL;
  _pidField = NULL;
  
  // set programe name

  _progName = "RadxCartDP";
  ucopyright((char *) _progName.c_str());
  
  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = false;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list, &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = false;
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
      OK = false;
    }
  }
  
  // if requested, print params for KDP then exit
  
  if (_args.printParamsKdp) {
    _printParamsKdp();
    exit(0);
  }

  // if requested, print params for PID then exit
  
  if (_args.printParamsPid) {
    _printParamsPid();
    exit(0);
  }
  
  // if requested, print params for RATE then exit
  
  if (_args.printParamsRate) {
    _printParamsRate();
    exit(0);
  }
  
  // read params for KdpFilt
  
  if (strstr(_params.KDP_params_file_path, "use-defaults") == NULL) {
    // not using defaults
    if (_kdpFiltParams.load(_params.KDP_params_file_path,
                            NULL, true, _args.tdrpDebug)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Cannot read params file for KdpFilt: "
           << _params.KDP_params_file_path << endl;
      OK = false;
      return;
    }
  }

  // read params for Ncar PID

  if (strstr(_params.PID_params_file_path, "use-defaults") == NULL) {
    // not using defaults
    if (_ncarPidParams.load(_params.PID_params_file_path,
                            NULL, true, _args.tdrpDebug)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Cannot read params file for NcarPid: "
           << _params.PID_params_file_path << endl;
      OK = false;
      return;
    }
  }
  if (_pid.setFromParams(_ncarPidParams)) {
    OK = false;
  }

  // read in RATE params
  
  if (strstr(_params.RATE_params_file_path, "use-defaults") == NULL) {
    // not using defaults
    if (_precipRateParams.load(_params.RATE_params_file_path,
                               NULL, true, _args.tdrpDebug)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Cannot read params file for PrecipFilt: "
           << _params.RATE_params_file_path << endl;
      OK = false;
      return;
    }
  }

  // initialize compute object
  
  pthread_mutex_init(&_debugPrintMutex, NULL);
  
  // set up scalars compute thread pool
  
  for (int ii = 0; ii < _params.n_compute_threads; ii++) {
    ScalarsThread *thread =
      new ScalarsThread(this, _params, 
                        _kdpFiltParams,
                        _ncarPidParams,
                        ii);
    if (!thread->OK) {
      delete thread;
      OK = false;
      return;
    }
    _scalarsThreadPool.addThreadToMain(thread);
    _computeScalarsThreads.push_back(thread->getScalarsCompute());
  }

}

//////////////////////////////////////
// destructor

RadxCartDP::~RadxCartDP()

{

  // free up

  _freeInterpRays();

  if (_cartInterp) {
    delete _cartInterp;
  }

  // mutex

  pthread_mutex_destroy(&_debugPrintMutex);

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RadxCartDP::Run()
{

  if (_params.radar_input_mode == Params::ARCHIVE) {
    return _runArchive();
  } else if (_params.radar_input_mode == Params::FILELIST) {
    return _runFilelist();
  } else {
    return _runRealtime();
  }
}

//////////////////////////////////////////////////
// get radar field name from type

string RadxCartDP::getRadarInputName(Params::radar_field_type_t ftype)
{
  for (int ii = 0; ii < _params.radar_field_names_n; ii++) {
    if (_params._radar_field_names[ii].field_type == ftype) {
      return _params._radar_field_names[ii].input_name;
    }
  }
  // not found
  return "";
}

string RadxCartDP::getRadarOutputName(Params::radar_field_type_t ftype)
{
  for (int ii = 0; ii < _params.radar_field_names_n; ii++) {
    if (_params._radar_field_names[ii].field_type == ftype) {
      return _params._radar_field_names[ii].output_name;
    }
  }
  // not found
  return "";
}

//////////////////////////////////////////////////
// get model field name from type

string RadxCartDP::getModelInputName(Params::model_field_type_t ftype)
{
  for (int ii = 0; ii < _params.model_field_names_n; ii++) {
    if (_params._model_field_names[ii].field_type == ftype) {
      return _params._model_field_names[ii].input_name;
    }
  }
  // not found
  return "";
}

string RadxCartDP::getModelOutputName(Params::model_field_type_t ftype)
{
  for (int ii = 0; ii < _params.model_field_names_n; ii++) {
    if (_params._model_field_names[ii].field_type == ftype) {
      return _params._model_field_names[ii].output_name;
    }
  }
  // not found
  return "";
}

//////////////////////////////////////////////////
// get model type from input name

Params::model_field_type_t RadxCartDP::getModelTypeFromInputName(const string name)
{
  for (int ii = 0; ii < _params.model_field_names_n; ii++) {
    string inputName = _params._model_field_names[ii].input_name;
    if (name == inputName) {
      return _params._model_field_names[ii].field_type;
    }
  }
  // not found, assume temp
  return Params::NOT_SET;
}

//////////////////////////////////////////////////
// get model type from output name

Params::model_field_type_t RadxCartDP::getModelTypeFromOutputName(const string name)
{
  for (int ii = 0; ii < _params.model_field_names_n; ii++) {
    string outputName = _params._model_field_names[ii].output_name;
    if (name == outputName) {
      return _params._model_field_names[ii].field_type;
    }
  }
  // not found, assume temp
  return Params::NOT_SET;
}

//////////////////////////////////////////////////
// get beam block field name from type

string RadxCartDP::getBeamBlockInputName(Params::bblock_field_type_t ftype)
{
  for (int ii = 0; ii < _params.bblock_field_names_n; ii++) {
    if (_params._bblock_field_names[ii].field_type == ftype) {
      return _params._bblock_field_names[ii].input_name;
    }
  }
  // not found
  return "";
}

string RadxCartDP::getBeamBlockOutputName(Params::bblock_field_type_t ftype)
{
  for (int ii = 0; ii < _params.bblock_field_names_n; ii++) {
    if (_params._bblock_field_names[ii].field_type == ftype) {
      return _params._bblock_field_names[ii].output_name;
    }
  }
  // not found
  return "";
}

//////////////////////////////////////////////////
// Run in filelist mode

int RadxCartDP::_runFilelist()
{

  // loop through the input file list

  int iret = 0;

  for (int ifile = 0; ifile < (int) _args.inputFileList.size(); ifile++) {

    string inputPath = _args.inputFileList[ifile];
    if (_processFile(inputPath)) {
      iret = -1;
    }

  }

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int RadxCartDP::_runArchive()
{

  // get start and end times

  time_t startTime = RadxTime::parseDateTime(_params.start_time);
  if (startTime == RadxTime::NEVER) {
    cerr << "ERROR - RadxCartDP::_runArchive()" << endl;
    cerr << "  Start time format incorrect: " << _params.start_time << endl;
    if (_args.startTimeSet) {
      cerr << "  Check command line" << endl;
    } else {
      cerr << "  Check params file: " << _paramsPath << endl;
    }
    return -1;
  }

  time_t endTime = RadxTime::parseDateTime(_params.end_time);
  if (endTime == RadxTime::NEVER) {
    cerr << "ERROR - RadxCartDP::_runArchive()" << endl;
    cerr << "  End time format incorrect: " << _params.end_time << endl;
    if (_args.endTimeSet) {
      cerr << "  Check command line" << endl;
    } else {
      cerr << "  Check params file: " << _paramsPath << endl;
    }
    return -1;
  }

  if (_params.debug) {
    cerr << "Running RadxCartDP in ARCHIVE mode" << endl;
    cerr << "  Input dir: " << _params.radar_input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(endTime) << endl;
  }

  // get the files to be processed
  
  RadxTimeList tlist;
  tlist.setDir(_params.radar_input_dir);
  tlist.setModeInterval(startTime, endTime);
  if (tlist.compile()) {
    cerr << "ERROR - RadxCartDP::_runArchive()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.radar_input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - RadxCartDP::_runArchive()" << endl;
    cerr << "  No files found, dir: " << _params.radar_input_dir << endl;
    return -1;
  }
  
  // loop through the input file list
  
  int iret = 0;
  for (size_t ipath = 0; ipath < paths.size(); ipath++) {
    if (_processFile(paths[ipath])) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode

int RadxCartDP::_runRealtime()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(),
                _params.instance,
                _params.procmap_register_interval);
  PMU_auto_register("Init realtime mode");

  // watch for new data to arrive

  LdataInfo ldata(_params.radar_input_dir,
                  _params.debug >= Params::DEBUG_VERBOSE);
  
  int iret = 0;

  while (true) {
    ldata.readBlocking(_params.radar_max_realtime_data_age_secs,
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

int RadxCartDP::_processFile(const string &filePath)
{

  PMU_auto_register("Processing file");

  // check file name

  if (!_fileNameValid(filePath)) {
    // skip this file
    return 0;
  }

  // initialize
  
  _readVol.clear();
  _freeInterpRays();

  if (_params.debug) {
    cerr << "INFO - RadxCartDP::_processFile" << endl;
  }
  
  // read in file
  
  if (_readFile(filePath)) {
    cerr << "ERROR - RadxCartDP::_processFile" << endl;
    cerr << "  Cannot read file: " << _readVol.getPathInUse() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "SUCCESS - read in file: " << _readVol.getPathInUse() << endl;
  }
  
  // initialize the projection and vlevels of the target volume

  _initTargetGrid();

  // read the temperature profile from the model

  if (_readModel()) {
    cerr << "ERROR - RadxCartDP::_processFile" << endl;
    cerr << "  Cannot read model environment data" << endl;
    return -1;
  }

  // add ray geometry fields to the input volume
  
  _addRayGeomFieldsToInput();
  
  // compute the scalar (kdp, pid, precip rate) fields

  if (_computeScalars()) {
    cerr << "ERROR - RadxCartDP::Run" << endl;
    cerr << "  Cannot compute scalar pid feature fields" << endl;
    return -1;
  }

  // merge the scalars into the read volume

  if (_mergeScalarsIntoReadVol()) {
    cerr << "ERROR - RadxCartDP::Run" << endl;
    cerr << "  Cannot merge scalars into read vol" << endl;
    return -1;
  }

  if (_params.write_debug_polar_output) {
    if (_writeDebugPolarOutput()) {
      cerr << "WARNING - cannot write debug files in polar coords" << endl;
    }
  }

  // set up interpolation fields
  
  _initInterp();
  
  // interpolate and write out
  
  if (_rhiMode) {
    if (_params.debug) {
      cerr << "  NOTE: data is in RHI mode" << endl;
    }
    _allocInterpToCart();
    _cartInterp->setRhiMode(true);
    _cartInterp->interpVol();
  } else {
    _allocInterpToCart();
    _cartInterp->setRhiMode(false);
    _cartInterp->interpVol();
  }

  // compute particle ID type
  
  int iret = 0;
  if (_computePid()) {
    cerr << "ERROR - RadxCartDP" << endl;
    cerr << "  Cannot compute PID" << endl;
    iret = -1;
  }
  
  // write out MDV file
  
  if (_writeOutputMdv()) {
    cerr << "ERROR - RadxCartDP" << endl;
    cerr << "  Cannot write output file" << endl;
    iret = -1;
  }

  // free up

  _cartInterp->freeMemory();
  if (_params.free_memory_between_files) {
    _readVol.clear();
    _freeInterpRays();
  }
  
  return iret;

}

//////////////////////////////////////////////////
// Check file name is valid
// Returns true on success, false on failure

bool RadxCartDP::_fileNameValid(const string &filePath)
{
  
  if (strlen(_params.radar_file_search_ext) > 0) {
    RadxPath rpath(filePath);
    if (strcmp(rpath.getExt().c_str(), _params.radar_file_search_ext)) {
      if (_params.debug) {
        cerr << "WARNING - ignoring file: " << filePath << endl;
        cerr << "  Does not have correct extension: "
             << _params.radar_file_search_ext << endl;
      }
      return false;
    }
  }
  
  if (strlen(_params.radar_file_search_substr) > 0) {
    RadxPath rpath(filePath);
    if (rpath.getFile().find(_params.radar_file_search_substr) == string::npos) {
      if (_params.debug) {
        cerr << "WARNING - ignoring file: " << filePath << endl;
        cerr << "  Does not contain required substr: "
             << _params.radar_file_search_substr << endl;
      }
      return false;
    }
  }

  return true;

}

//////////////////////////////////////////////////
// Read in a RADX file
// Returns 0 on success, -1 on failure

int RadxCartDP::_readFile(const string &filePath)
{

  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  if (inFile.readFromPath(filePath, _readVol)) {
    cerr << "ERROR - RadxCartDP::_readFile" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  
  // check we have at least 2 rays
  
  if (_readVol.getNRays() < 2) {
    cerr << "ERROR - RadxCartDP::_readFile" << endl;
    cerr << "  Too few rays: " << _readVol.getNRays() << endl;
    return -1;
  }
  
  // convert to fl32
  
  _readVol.convertToFl32();

  // pad out the gates to the longest range
  
  _readVol.setNGatesConstant();

  // set range geometry
  
  if (_params.override_gate_geometry) {
    // override gate geometry if requested
    _readVol.setRangeGeom(_params.start_range_km, _params.gate_spacing_km);
  } else {
    // remap all rays to finest geom
    _readVol.remapToFinestGeom();
  }
  
  //  check for rhi
  
  _rhiMode = _isRhi();

  // rename fields if requested

  for (int ii = 0; ii < _params.radar_field_names_n; ii++) {
    string inName = _params._radar_field_names[ii].input_name;
    string outName = _params._radar_field_names[ii].output_name;
    if (inName.size() > 0 && inName != outName) {
      _readVol.renameField(inName, outName);
    }
  }

  // override radar location if requested

  if (_params.override_radar_location) {
    _readVol.overrideLocation(_params.radar_latitude_deg,
                              _params.radar_longitude_deg,
                              _params.radar_altitude_meters / 1000.0);
  }

  // override beam width if requested
  
  if (_params.override_beam_width) {
    _readVol.setRadarBeamWidthDegH(_params.beam_width_deg_h);
    _readVol.setRadarBeamWidthDegV(_params.beam_width_deg_v);
  }

  // override fixed angle if required

  if (_params.override_fixed_angle_with_mean_measured_angle) {
    _readVol.computeFixedAnglesFromRays();
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  _startRangeKm: " << _readVol.getStartRangeKm() << endl;
    cerr << "  _gateSpacingKm: " << _readVol.getGateSpacingKm() << endl;
  }

  // add extra debug fields if requested
  
  if (_params.output_angle_fields) {
    _addAngleFields();
  }
  if (_params.output_range_field) {
    _addRangeField();
  }
  if (_params.output_height_field) {
    _addHeightField();
  }
  if (_params.output_coverage_field) {
    _addCoverageField();
  }
  if (_params.output_time_field) {
    _addTimeField();
  }
  
  // set radar properties

  _radarHtKm = _readVol.getAltitudeKm();
  _wavelengthM = _readVol.getWavelengthM();

  return 0;

}

//////////////////////////////////////////////////
// set up read

void RadxCartDP::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }

  if (_params.set_max_range) {
    file.setReadMaxRangeKm(_params.max_range_km);
  }

  for (int ii = 0; ii < _params.radar_field_names_n; ii++) {
    if (strlen(_params._radar_field_names[ii].input_name) > 0) {
      file.addReadField(_params._radar_field_names[ii].input_name);
    }
  }
  
  if (_params.copy_selected_input_fields_to_output) {
    for (int ii = 0; ii < _params.copy_fields_n; ii++) {
      file.addReadField(_params._copy_fields[ii].field_name);
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.printReadRequest(cerr);
  }
  
}

//////////////////////////////////////////////////
// add ray geometry and pid fields to the volume

void RadxCartDP::_addRayGeomFieldsToInput()

{

  vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {

    RadxRay *ray = rays[iray];
    size_t nGates = ray->getNGates();
    
    RadxField *elevFld = new RadxField(elevationFieldName, "deg");
    RadxField *rangeFld = new RadxField(rangeFieldName, "km");
    RadxField *beamHtFld = new RadxField(beamHtFieldName, "km");
    
    vector<Radx::fl32> elev(nGates), rng(nGates), ht(nGates);
    
    double startRangeKm = ray->getStartRangeKm();
    double gateSpacingKm = ray->getGateSpacingKm();
    
    BeamHeight beamHt;
    beamHt.setInstrumentHtKm(_readVol.getAltitudeKm());
    if (_params.override_standard_pseudo_earth_radius) {
      beamHt.setPseudoRadiusRatio(_params.pseudo_earth_radius_ratio);
    }

    // loop through the gates
    
    double rangeKm = startRangeKm;
    double elevationDeg = ray->getElevationDeg();
    for (size_t igate = 0; igate < nGates; igate++, rangeKm += gateSpacingKm) {
      double htKm = beamHt.computeHtKm(elevationDeg, rangeKm);
      elev[igate] = elevationDeg;
      rng[igate] = rangeKm;
      ht[igate] = htKm;
    } // igate
    
    elevFld->setTypeFl32(-9999.0);
    rangeFld->setTypeFl32(-9999.0);
    beamHtFld->setTypeFl32(-9999.0);
    
    elevFld->addDataFl32(nGates, elev.data());
    rangeFld->addDataFl32(nGates, rng.data());
    beamHtFld->addDataFl32(nGates, ht.data());
    
    ray->addField(elevFld);
    ray->addField(rangeFld);
    ray->addField(beamHtFld);

  } // iray

}

/////////////////////////////////////////////////////
// compute the pid fields for all rays in volume

int RadxCartDP::_computeScalars()
{

  // initialize the volume with ray numbers
  
  _readVol.setRayNumbersInOrder();

  // initialize pid

  _scalarRays.clear();
  
  // loop through the input rays,
  // computing the pid fields

  const vector<RadxRay *> &inputRays = _readVol.getRays();
  for (size_t iray = 0; iray < inputRays.size(); iray++) {

    // get a thread from the pool
    bool isDone = true;
    ScalarsThread *thread = 
      (ScalarsThread *) _scalarsThreadPool.getNextThread(true, isDone);
    if (thread == NULL) {
      break;
    }
    if (isDone) {
      // store the results computed by the thread
      if (_storeScalarsRay(thread)) {
        cerr << "ERROR - RadxRate::_compute()" << endl;
        cerr << "  Cannot compute for ray num: " << iray << endl;
        break;
      }
      // return thread to the available pool
      _scalarsThreadPool.addThreadToAvail(thread);
      // reduce iray by 1 since we did not actually process this ray
      // we only handled a previously started thread
      iray--;
    } else {
      // got a thread to use, set the input ray
      thread->setInputRay(inputRays[iray]);
      // set it running
      thread->signalRunToStart();
    }

  } // iray
    
  // collect remaining done threads

  _scalarsThreadPool.setReadyForDoneCheck();
  while (!_scalarsThreadPool.checkAllDone()) {
    ScalarsThread *thread = (ScalarsThread *) _scalarsThreadPool.getNextDoneThread();
    if (thread == NULL) {
      break;
    } else {
      // store the results computed by the thread
      _storeScalarsRay(thread);
      // return thread to the available pool
      _scalarsThreadPool.addThreadToAvail(thread);
    }
  } // while

  return 0;

}

///////////////////////////////////////////////////////////
// Store the scalars ray

int RadxCartDP::_storeScalarsRay(ScalarsThread *thread)
  
{
  
  RadxRay *scalarsRay = thread->getOutputRay();
  if (scalarsRay == NULL) {
    // error
    return -1;
  }

  // good return, add to results
  _scalarRays.push_back(scalarsRay);
  
  return 0;

}
      
////////////////////////////////////////////////////////////////
// merge the scalars into the read volume

int RadxCartDP::_mergeScalarsIntoReadVol()
{

  vector<RadxRay *> &readRays = _readVol.getRays();
  
  if (readRays.size() != _scalarRays.size()) {
    cerr << "ERROR - RadxCartDP::_mergeScalarsIntoReadVol" << endl;
    cerr << "  readRays size: " << readRays.size() << endl;
    cerr << "  _scalarRays size: " << _scalarRays.size() << endl;
    return -1;
  }

  for (size_t ii = 0; ii < _scalarRays.size(); ii++) {

    RadxRay *scRay = _scalarRays[ii];
    if (scRay == NULL) {
      cerr << "ERROR - null derived ray at index: " << ii << endl;
      return -1;
    }

    int rayNum = scRay->getRayNumber();
    if (rayNum < 0 || rayNum >= (int) readRays.size()) {
      cerr << "ERROR - bad ray number in derived ray: " << rayNum << endl;
      return -1;
    }

    RadxRay *readRay = readRays[rayNum];
    if (readRay == NULL) {
      cerr << "ERROR - null read ray at rayNum: " << rayNum << endl;
      return -1;
    }

    const vector<RadxField *> &scFields = scRay->getFields();
    for (size_t jj = 0; jj < scFields.size(); jj++) {

      const RadxField *sc = scFields[jj];
      if (sc == NULL) {
        continue;
      }

      // remove any placeholder / previous version of same field
      if (readRay->getField(sc->getName())) {
        readRay->removeField(sc->getName());
      }

      // deep copy field into read volume ray
      RadxField *copy = new RadxField(*sc);
      readRay->addField(copy);

    } // jj

  } // ii

  return 0;
}

//////////////////////////////////////////////////
// Write polar volume for debugging
// Input file + derived fields in polar coords.

int RadxCartDP::_writeDebugPolarOutput()
{

  if (_params.debug) {
    cerr << "INFO - writing debug polar data in Cfradial" << endl;
  }
  
  RadxFile out;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    out.setDebug(true);
  }
  if (out.writeToDir(_readVol, _params.debug_polar_output_dir, true, false)) {
    cerr << "ERROR - RadxCartDP::_writeDebugPolarOutput()" << endl;
    cerr << "  Cannot write debug polar file, path: " << out.getPathInUse() << endl;
    cerr << out.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "SUCCESS - wrote debug polar file, path: " << out.getPathInUse() << endl;
  }
  
  return 0;
  
}


//////////////////////////////////////////////////
// add angle fields

void RadxCartDP::_addAngleFields()
{
  
  // loop through rays
    
  vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {
      
    RadxRay *ray = rays[iray];
    int nGates = ray->getNGates();
    TaArray<Radx::fl32> data_;
    Radx::fl32 *data = data_.alloc(nGates);
      
    double az = ray->getAzimuthDeg();
    double el = ray->getElevationDeg();
    double cosAz = cos(az * Radx::DegToRad);
    double cosEl = cos(el * Radx::DegToRad);
    double sinAz = sin(az * Radx::DegToRad);
    double sinEl = sin(el * Radx::DegToRad);
      
    // add azimuth field
    if (strlen(_params.angle_fields.azimuth_field_name) > 0) {
      RadxField *azimuthField = 
        new RadxField(_params.angle_fields.azimuth_field_name, "deg");
      azimuthField->setLongName("azimuth_angle");
      azimuthField->setStandardName("sensor_to_target_azimuth_angle");
      azimuthField->setTypeFl32(-9999.0);
      for (int ii = 0; ii < nGates; ii++) {
        data[ii] = az;
      }
      azimuthField->setFieldFolds(0.0, 360.0);
      azimuthField->addDataFl32(nGates, data);
      ray->addField(azimuthField);
    }
      
    // add elevation field
    if (strlen(_params.angle_fields.elevation_field_name) > 0) {
      RadxField *elevationField = 
        new RadxField(_params.angle_fields.elevation_field_name, "deg");
      elevationField->setLongName("elevation_angle");
      elevationField->setStandardName("sensor_to_target_elevation_angle");
      elevationField->setTypeFl32(-9999.0);
      for (int ii = 0; ii < nGates; ii++) {
        data[ii] = el;
      }
      elevationField->addDataFl32(nGates, data);
      ray->addField(elevationField);
    }
      
    // add alpha field - sin(az) * cos(el)
    if (strlen(_params.angle_fields.alpha_field_name) > 0) {
      RadxField *alphaField = 
        new RadxField(_params.angle_fields.alpha_field_name, "");
      alphaField->setLongName("alpha_for_doppler_analysis");
      alphaField->setStandardName("alpha_for_doppler_analysis");
      alphaField->setTypeFl32(-9999.0);
      for (int ii = 0; ii < nGates; ii++) {
        data[ii] = sinAz * cosEl;
      }
      alphaField->addDataFl32(nGates, data);
      ray->addField(alphaField);
    }
      
    // add beta field - cos(az) * cos(el)
    if (strlen(_params.angle_fields.beta_field_name) > 0) {
      RadxField *betaField = 
        new RadxField(_params.angle_fields.beta_field_name, "");
      betaField->setLongName("beta_for_doppler_analysis");
      betaField->setStandardName("beta_for_doppler_analysis");
      betaField->setTypeFl32(-9999.0);
      for (int ii = 0; ii < nGates; ii++) {
        data[ii] = cosAz * cosEl;
      }
      betaField->addDataFl32(nGates, data);
      ray->addField(betaField);
    }
      
    // add gamma field - sin(el)
    if (strlen(_params.angle_fields.gamma_field_name) > 0) {
      RadxField *gammaField = 
        new RadxField(_params.angle_fields.gamma_field_name, "");
      gammaField->setLongName("gamma_for_doppler_analysis");
      gammaField->setStandardName("gamma_for_doppler_analysis");
      gammaField->setTypeFl32(-9999.0);
      for (int ii = 0; ii < nGates; ii++) {
        data[ii] = sinEl;
      }
      gammaField->addDataFl32(nGates, data);
      ray->addField(gammaField);
    }

  } // iray

}
  
//////////////////////////////////////////////////
// add range field

void RadxCartDP::_addRangeField()
{
  
  vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    RadxRay *ray = rays[iray];
    int nGates = ray->getNGates();
    TaArray<Radx::fl32> data_;
    Radx::fl32 *data = data_.alloc(nGates);
    
    // range field
    
    RadxField *rangeField = new RadxField(_params.range_field_name, "km");
    rangeField->setLongName("range_from_radar");
    rangeField->setStandardName("slant_range");
    rangeField->setTypeFl32(-9999.0);
    double range = ray->getStartRangeKm();
    for (int ii = 0; ii < nGates; ii++) {
      data[ii] = range;
      range += ray->getGateSpacingKm();
    }
    rangeField->addDataFl32(nGates, data);
    ray->addField(rangeField);
  }

}
  
//////////////////////////////////////////////////
// add height field

void RadxCartDP::_addHeightField()
{
  
  // beamHeight

  BeamHeight beamHt;
  beamHt.setInstrumentHtKm(_readVol.getAltitudeKm());
  if (_params.override_standard_pseudo_earth_radius) {
    beamHt.setPseudoRadiusRatio(_params.pseudo_earth_radius_ratio);
  }

  vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    RadxRay *ray = rays[iray];
    int nGates = ray->getNGates();
    TaArray<Radx::fl32> data_;
    Radx::fl32 *data = data_.alloc(nGates);
      
    RadxField *heightField = new RadxField(_params.height_field_name, "km");
    heightField->setLongName("height_msl");
    heightField->setStandardName("height_msl");
    heightField->setTypeFl32(-9999.0);
    double elevDeg = ray->getElevationDeg();
    double range = ray->getStartRangeKm();
    for (int ii = 0; ii < nGates; ii++) {
      double ht = beamHt.computeHtKm(elevDeg, range);
      data[ii] = ht;
      range += ray->getGateSpacingKm();
    }
    heightField->addDataFl32(nGates, data);
    ray->addField(heightField);
  }
  
}

//////////////////////////////////////////////////
// add coverage field

void RadxCartDP::_addCoverageField()
{
  
  vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    RadxRay *ray = rays[iray];
    int nGates = ray->getNGates();
    TaArray<Radx::fl32> data_;
    Radx::fl32 *data = data_.alloc(nGates);
    
    RadxField *covFld = new RadxField(_params.coverage_field_name, "");
    covFld->setLongName("radar_coverage_flag");
    covFld->setStandardName("radar_coverage_flag");
    covFld->setTypeFl32(-9999.0);
    for (int ii = 0; ii < nGates; ii++) {
      data[ii] = 1.0;
    }
    covFld->addDataFl32(nGates, data);
    covFld->setIsDiscrete(true);
    ray->addField(covFld);

  } // iray

}

//////////////////////////////////////////////////
// add time field

void RadxCartDP::_addTimeField()
{

  // start times

  time_t startTimeSecs = _readVol.getStartTimeSecs();
  double startNanoSecs = _readVol.getStartNanoSecs();

  // loop through rays

  vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    RadxRay *ray = rays[iray];
    int nGates = ray->getNGates();
    TaArray<Radx::fl32> data_;
    Radx::fl32 *data = data_.alloc(nGates);

    // time field

    RadxField *timeFld = new RadxField(_params.time_field_name, "seconds");
    timeFld->setLongName("time_since_volume_start");
    timeFld->setStandardName("time_since_volume_start");
    timeFld->setTypeFl32(-9999.0);
    time_t timeSecs = ray->getTimeSecs();
    double nanoSecs = ray->getNanoSecs();
    double secsSinceStart = 
      (double) (timeSecs - startTimeSecs) - ((nanoSecs - startNanoSecs) * 1.0e-9);
    for (int ii = 0; ii < nGates; ii++) {
      data[ii] = secsSinceStart;
    }
    timeFld->addDataFl32(nGates, data);
    if (!_params.interp_time_field) {
      timeFld->setIsDiscrete(true);
    }
    ray->addField(timeFld);

  } // iray

}

/////////////////////////////////////////////////////
// check whether volume is predominantly in RHI mode

bool RadxCartDP::_isRhi()
{
  
  // check to see if we are in RHI mode, set flag accordingly
  
  int nRaysRhi = 0;
  const vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    if (rays[ii]->getSweepMode() == Radx::SWEEP_MODE_RHI ||
        rays[ii]->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
      nRaysRhi++;
    }
  }
  double fractionRhi = (double) nRaysRhi / (double) rays.size();
  if (fractionRhi > 0.5) {
    return true;
  } else {
    return false;
  }

}

/////////////////////////////////////////////////////////
// initialize the projection and vlevels of the target volume

void RadxCartDP::_initTargetGrid()
{

  Mdvx::coord_t coord;

  coord.nx = _params.grid_xy_geom.nx;
  coord.ny = _params.grid_xy_geom.ny;
  coord.nz = _params.grid_z_geom.nz;
  
  _targetNpoints = coord.nx * coord.ny* coord.nz;

  coord.dx = _params.grid_xy_geom.dx;
  coord.dy = _params.grid_xy_geom.dy;
  coord.dz = _params.grid_z_geom.dz;
  
  coord.minx = _params.grid_xy_geom.minx;
  coord.miny = _params.grid_xy_geom.miny;
  coord.minz = _params.grid_z_geom.minz;

  // init _targetVlevels
  
  if (_params.specify_individual_z_levels) {
    _targetVlevels.clear();
    for (int ii = 0; ii < _params.z_level_array_n; ii++) {
      _targetVlevels.push_back(_params._z_level_array[ii]);
    }
    coord.nz = _targetVlevels.size();
    coord.minz = _targetVlevels[0];
    if (coord.nz > 1) {
      coord.dz = _targetVlevels[1] - _targetVlevels[0];
    } else {
      coord.dz = 0.0;
    }
  } else {
    _targetVlevels.clear();
    for (int ii = 0; ii < coord.nz; ii++) {
      _targetVlevels.push_back(coord.minz + ii * coord.dz);
    }
  }

  // init _targetProj
  
  coord.proj_type = _params.grid_projection;
  if (_params.center_grid_on_radar) {
    coord.proj_origin_lat = _readVol.getLatitudeDeg();
    coord.proj_origin_lon = _readVol.getLongitudeDeg();
  } else {
    coord.proj_origin_lat = _params.grid_origin_lat;
    coord.proj_origin_lon = _params.grid_origin_lon;
  }
  
  switch (_params.grid_projection) {

    case Params::PROJ_LATLON:
      coord.proj_type = Mdvx::PROJ_LATLON;
      break;
      
    case Params::PROJ_LAMBERT_CONF:
      coord.proj_type = Mdvx::PROJ_LAMBERT_CONF;
      coord.proj_params.lc2.lat1 = _params.grid_lat1;
      coord.proj_params.lc2.lat2 = _params.grid_lat2;
      break;

    case Params::PROJ_MERCATOR:
      coord.proj_type = Mdvx::PROJ_MERCATOR;
      break;

    case Params::PROJ_POLAR_STEREO:
      coord.proj_type = Mdvx::PROJ_POLAR_STEREO;
      coord.proj_params.ps.tan_lon = _params.grid_tangent_lon;
      coord.proj_params.ps.central_scale = _params.grid_central_scale;
      if (_params.grid_pole_is_north) {
        coord.proj_params.ps.pole_type = Mdvx::POLE_NORTH;
      } else {
        coord.proj_params.ps.pole_type = Mdvx::POLE_SOUTH;
      }
      break;

    case Params::PROJ_FLAT:
      coord.proj_type = Mdvx::PROJ_FLAT;
      coord.proj_params.flat.rotation = _params.grid_rotation;
      break;

    case Params::PROJ_OBLIQUE_STEREO:
      coord.proj_type = Mdvx::PROJ_OBLIQUE_STEREO;
      coord.proj_params.os.tan_lat = _params.grid_tangent_lat;
      coord.proj_params.os.tan_lon = _params.grid_tangent_lon;
      coord.proj_params.os.central_scale = _params.grid_central_scale;
      break;

    case Params::PROJ_TRANS_MERCATOR:
      coord.proj_type = Mdvx::PROJ_TRANS_MERCATOR;
      coord.proj_params.tmerc.central_scale = _params.grid_central_scale;
      break;

    case Params::PROJ_ALBERS:
      coord.proj_type = Mdvx::PROJ_ALBERS;
      coord.proj_params.albers.lat1 = _params.grid_lat1;
      coord.proj_params.albers.lat2 = _params.grid_lat2;
      break;

    case Params::PROJ_LAMBERT_AZIM:
      coord.proj_type = Mdvx::PROJ_LAMBERT_AZIM;
      break;

  } // switch

  _targetProj.init(coord);

  // initialize lookup table

  _modelRemap.setTargetCoords(_targetProj, _targetVlevels);

}

//////////////////////////////////////////////////
// initialize interpolation fields

void RadxCartDP::_initInterp()

{

  // set up interp fields
  
  _initInterpFields();
  
  // load up the input ray data vector

  _loadInterpRays();

  // check all fields are present
  // set standard names etc

  _checkInterpFields();

}

//////////////////////////////////////////////////
// initialize the fields for interpolation

void RadxCartDP::_initInterpFields()
{
  
  _interpFields.clear();
  
  // get field name list
  
  vector<string> fieldNames = _readVol.getUniqueFieldNameList();

  // find an example of each field, by search through the rays
  // use that field as the template

  for (size_t ifield = 0; ifield < fieldNames.size(); ifield++) {

    string radxName = fieldNames[ifield];

    vector<RadxRay *> &rays = _readVol.getRays();
    for (size_t iray = 0; iray < rays.size(); iray++) {
      const RadxRay *ray = rays[iray];
      const RadxField *field = ray->getField(radxName);
      if (field != NULL) {
        BaseInterp::Field interpField;
        interpField.radxName = field->getName();
        interpField.outputName = field->getName();
        interpField.longName = field->getLongName();
        interpField.standardName = field->getStandardName();
        interpField.units = field->getUnits();
        interpField.inputDataType = field->getDataType();
        interpField.inputScale = field->getScale();
        interpField.inputOffset = field->getOffset();
        if (field->getFieldFolds()) {
          interpField.fieldFolds = true;
          interpField.foldLimitLower = field->getFoldLimitLower();
          interpField.foldLimitUpper = field->getFoldLimitUpper();
          interpField.foldRange =
            interpField.foldLimitUpper - interpField.foldLimitLower;
        }
        if (field->getIsDiscrete()) {
          interpField.isDiscrete = true;
        }
        _interpFields.push_back(interpField);
        break;
      }
    }

  } // ifield

  // override fold limits from the parameters

  double nyquist = 0.0;
  vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {
    const RadxRay *ray = rays[iray];
    if (fabs(ray->getNyquistMps() - Radx::missingFl64) > 1.0e-6) {
      nyquist = ray->getNyquistMps();
      break;
    }
  }

  if (_params.set_fold_limits) {
    for (int ii = 0; ii < _params.folded_fields_n; ii++) {
      string radxName = _params._folded_fields[ii].field_name;
      bool fieldFolds = _params._folded_fields[ii].field_folds;
      bool useNyquist = _params._folded_fields[ii].use_global_nyquist;
      double foldLimitLower = _params._folded_fields[ii].fold_limit_lower;
      double foldLimitUpper = _params._folded_fields[ii].fold_limit_upper;
      for (size_t ifld = 0; ifld < _interpFields.size(); ifld++) {
        if (_interpFields[ifld].radxName == radxName) {
          _interpFields[ifld].fieldFolds = fieldFolds;
          if (useNyquist && nyquist > 0) {
            _interpFields[ifld].foldLimitLower = -nyquist;
            _interpFields[ifld].foldLimitUpper = nyquist;
          } else {
            _interpFields[ifld].foldLimitLower = foldLimitLower;
            _interpFields[ifld].foldLimitUpper = foldLimitUpper;
          }
          _interpFields[ifld].foldRange =
            _interpFields[ifld].foldLimitUpper - _interpFields[ifld].foldLimitLower;
          break;
        }
      } // ifld
    } // ii
  }

}

////////////////////////////////////////////////////////////
// Allocate interpolation objects as needed

void RadxCartDP::_allocInterpToCart()
{
  if (_cartInterp == NULL) {
    _cartInterp = new CartInterp(_progName, _params, _readVol,
                                 _interpFields, _interpRays);
  }
}

////////////////////////////////////////////////////////////
// Free up input rays

void RadxCartDP::_freeInterpRays()
  
{
  for (size_t ii = 0; ii < _interpRays.size(); ii++) {
    delete _interpRays[ii];
  }
  _interpRays.clear();
}

//////////////////////////////////////////////////
// load up the input ray data vector

void RadxCartDP::_loadInterpRays()
{

  // loop through the rays in the read volume,
  // making some checks and then adding the rays
  // to the interp rays array as appropriate
  
  vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t isweep = 0; isweep < _readVol.getNSweeps(); isweep++) {

    const RadxSweep *sweep = _readVol.getSweeps()[isweep];

    for (size_t iray = sweep->getStartRayIndex();
         iray <= sweep->getEndRayIndex(); iray++) {

      // accept ray
      
      BaseInterp::Ray *interpRay = 
        new BaseInterp::Ray(rays[iray],
                            isweep,
                            _interpFields,
                            _params.use_fixed_angle_for_interpolation,
                            _params.use_fixed_angle_for_data_limits);
      _interpRays.push_back(interpRay);
      
    } // iray

  } // isweep


}
  
//////////////////////////////////////////////////
// check all fields are present
// set standard names etc

void RadxCartDP::_checkInterpFields()
{
  
  vector<RadxRay *> &rays = _readVol.getRays();
  
  for (size_t ifield = 0; ifield < _interpFields.size(); ifield++) {
    bool found = false;
    string fieldName = _interpFields[ifield].radxName;
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxRay *ray = rays[ii];
      const RadxField *fld = ray->getField(fieldName);
      if (fld != NULL) {
        found = true;
        _interpFields[ifield].longName = fld->getLongName();
        _interpFields[ifield].standardName = fld->getStandardName();
        _interpFields[ifield].units = fld->getUnits();
        break;
      }
    }
    if (!found) {
      cerr << "WARNING - field not found: " << fieldName << endl;
      cerr << "  File: " << _readVol.getPathInUse() << endl;
    }
  } // ifield

}

/////////////////////////////////////////////////////////
// read in model data
//
// Returns 0 on success, -1 on failure.

int RadxCartDP::_readModel()

{

  time_t radarTime = _readVol.getStartTimeSecs();

  _modelRawMdvx.clearRead();
  _modelRawMdvx.setReadTime(Mdvx::READ_CLOSEST,
                         _params.model_input_url,
                         _params.model_search_margin_secs,
                         radarTime);

  for (int ii = 0; ii < _params.model_field_names_n; ii++) {
    if (strlen(_params._model_field_names[ii].input_name) == 0) {
      continue;
    }
    _modelRawMdvx.addReadField(_params._model_field_names[ii].input_name);
  }
  
  if (_modelRawMdvx.readVolume()) {
    cerr << "ERROR - RadxCartDP::_readModel" << endl;
    cerr << "  Cannot read model data" << endl;
    cerr << "  URL: " << _params.model_input_url << endl;
    cerr << "  Search time: " << DateTime::strm(radarTime) << endl;
    cerr << "  Search margin (secs): " << _params.model_search_margin_secs << endl;
    cerr << _modelRawMdvx.getErrStr() << endl;
    return -1;
  }

  // interpolate the model data onto the output Cartesian grid

  _interpModelToOutputGrid();

  // rename fields as appropriate

  for (size_t ifld = 0; ifld < _modelInterpMdvx.getNFields(); ifld++) {
    MdvxField *fld = _modelInterpMdvx.getField(ifld);
    string inputName = fld->getFieldName();
    Params::model_field_type_t mtype = getModelTypeFromInputName(inputName);
    string outputName = getModelOutputName(mtype);
    fld->setFieldName(outputName);
  }

  // debug write
  
  if (_params.write_interpolated_model_data) {
    if (_params.debug) {
      cerr << "Writing interpolated model data file" << endl;
    }
    if (_modelInterpMdvx.writeToDir(_params.interpolated_model_output_url)) {
      cerr << "WARNING - error writing model data" << endl;
      cerr << _modelInterpMdvx.getErrStr() << endl;
    }
    if (_params.debug) {
      cerr << "Wrote interpolated model data, path: "
           << _modelInterpMdvx.getPathInUse() << endl;
    }
  }
  
  // compute the temperature profile from the model data

  if (_computeTempProfile()) {
    cerr << "ERROR - RadxCartDP::_readModel" << endl;
    cerr << "  Cannot compute temp profile, time: "
         << RadxTime::strm(_readVol.getStartTimeSecs()) << endl;
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// compute the temperatude profile from the interpolated model

int RadxCartDP::_computeTempProfile()
{

  _tempProfile.clear();

  MdvxField *tempFld =
    _modelInterpMdvx.getField(getModelOutputName(Params::TEMP).c_str());
  if (tempFld == NULL) {
    cerr << "ERROR - RadxCartDP::_computeTempProfile" << endl;
    cerr << "  Cannot find temp field in model, time: "
         << RadxTime::strm(_modelInterpMdvx.getValidTime()) << endl;
    return -1;
  }

  const Mdvx::field_header_t &fhdr = tempFld->getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr = tempFld->getVlevelHeader();
  fl32 *tempVol = (fl32 *) tempFld->getVol();
  fl32 miss = fhdr.missing_data_value;
  size_t nPtsPlane = fhdr.ny * fhdr.nx;

  for (int iz = 0; iz < fhdr.nz; iz++) {
    double htKm = vhdr.level[iz];
    fl32 *tmpPtr = tempVol + iz * nPtsPlane;
    double sum = 0.0, nn = 0.0;
    for (size_t ii = 0; ii < nPtsPlane; ii++, tmpPtr++) {
      if (*tmpPtr != miss) {
        sum += *tmpPtr;
        nn++;
      }
    } // ii
    if (nn > 0) {
      double meanTemp = sum / nn;
      TempProfile::PointVal val(htKm, meanTemp);
      _tempProfile.addPoint(val);
    }
    
  } // iz

  if (_tempProfile.getProfile().size() < 2) {
    cerr << "ERROR - RadxCartDP::_computeTempProfile" << endl;
    cerr << "  Not enough temp data for valid profile" << endl;
    cerr << "  Cannot find temp field in model, time: "
         << RadxTime::strm(_modelInterpMdvx.getValidTime()) << endl;
    return -1;
  }
  
  return 0;

}

/////////////////////////////////////////////////////////
// interpolate the model data onto the output grid

void RadxCartDP::_interpModelToOutputGrid()
{

  _modelInterpMdvx.clear();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _modelInterpMdvx.setDebug(true);
  }
  _modelInterpMdvx.setMasterHeader(_modelRawMdvx.getMasterHeader());
  
  for (size_t ifield = 0; ifield < _modelRawMdvx.getNFields(); ifield++) {
    MdvxField *rawFld = _modelRawMdvx.getField(ifield);
    MdvxField *interpField = _modelRemap.interpField(*rawFld);
    string rawName = rawFld->getFieldName();
    //if (strlen(_params.model_field_names[ifield].
    _modelInterpMdvx.addField(interpField);
    
  } // ifield
  
}

/////////////////////////////////////////////////////
// write out MDV data

int RadxCartDP::_writeOutputMdv()
{

  if (_params.debug) {
    cerr << "  Writing output file ... " << endl;
  }

  // initialize
  
  OutputMdv out(_progName, _params);
  out.setMasterHeader(_readVol);

  // fill with interpolated fields
  
  _cartInterp->fillOutputMdv(out);

  // add the model fields

  for (size_t ii = 0; ii < _modelInterpMdvx.getNFields(); ii++) {
    // make a copy of the field, since the mdvx object takes memory ownership
    MdvxField *field = new MdvxField(*_modelInterpMdvx.getField(ii));
    // add to output object
    out.addField(field);
  } // ii

  // add PID field

  if (_pidField) {
    out.addField(_pidField);
    _pidField = nullptr; // memory handling passed to output mdv object
  }
  
  // write out file
  
  if (out.writeVol()) {
    cerr << "ERROR - RadxCartDP::_writeOutputMdv" << endl;
    cerr << "  Cannot write file to output_dir: "
         << _params.output_dir << endl;
    return -1;
  }
  
  return 0;

}
  
//////////////////////////////////////////////////
// compute the PID field

int RadxCartDP::_computePid()
{

  if (_params.debug) {
    cerr << "Computing Cartesian PID" << endl;
  }
  
  // get the fields we need for computing PID

  BaseInterp::Field *snrFld = _getInterpField(snrForPidFieldName);
  BaseInterp::Field *dbzFld = _getInterpField(dbzForPidFieldName);
  BaseInterp::Field *zdrFld = _getInterpField(zdrForPidFieldName);
  BaseInterp::Field *ldrFld = _getInterpField(ldrForPidFieldName);
  BaseInterp::Field *rhohvFld = _getInterpField(rhohvForPidFieldName);
  BaseInterp::Field *kdpFld = _getInterpField(kdpForPidFieldName);
  BaseInterp::Field *zdrSdevFld = _getInterpField(zdrSdevForPidFieldName);
  BaseInterp::Field *phidpSdevFld = _getInterpField(phidpSdevForPidFieldName);
  
  if (!snrFld || !dbzFld || !zdrFld || !rhohvFld ||
      !kdpFld || !zdrSdevFld || !phidpSdevFld) {
    cerr << "ERROR - _computePID" << endl;
    cerr << "  One or more feature fields is missing" << endl;
    return -1;
  }

  string modelTempName = getModelOutputName(Params::TEMP);
  MdvxField *tempFld = _modelInterpMdvx.getField(modelTempName.c_str());
  if (!tempFld) {
    cerr << "ERROR - _computePID" << endl;
    cerr << "  Model temp field missing: " << modelTempName << endl;
    return -1;
  }
  fl32 *tempArray = (fl32 *) tempFld->getVol();

  // compute PID for each point in the Cartesian volume
  
  vector<fl32> pidArray(_targetNpoints);
  
  for (size_t ii = 0; ii < _targetNpoints; ii++) {
    
    double snr = snrFld->outputField[ii];
    double dbz = dbzFld->outputField[ii];
    double zdr = zdrFld->outputField[ii];
    double ldr = -9999.0;
    if (ldrFld) {
      ldr = ldrFld->outputField[ii];
    }
    double rhohv = rhohvFld->outputField[ii];
    double kdp = kdpFld->outputField[ii];
    double zdrSdev = zdrSdevFld->outputField[ii];
    double phidpSdev = phidpSdevFld->outputField[ii];
    double temp = tempArray[ii];

    int pid = 0, pid2 = 0;
    double interest = 0.0, interest2 = 0.0, confidence = 0.0;
    
    _pid.computePid(snr, dbz, temp, zdr, kdp, ldr, rhohv, zdrSdev, phidpSdev,
                    pid, interest, pid2, interest2, confidence);
    
    pidArray[ii] = pid;
    
  } // ii
  
  // create MdvxField for pid
  
  Mdvx::field_header_t tempFhdr(tempFld->getFieldHeader());
  Mdvx::vlevel_header_t tempVhdr(tempFld->getVlevelHeader());
  
  tempFhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  tempFhdr.compression_type = Mdvx::COMPRESSION_NONE;
  tempFhdr.data_element_nbytes = sizeof(fl32);
  tempFhdr.volume_size = _targetNpoints * sizeof(fl32);
  
  _pidField = new MdvxField(tempFhdr, tempVhdr, pidArray.data());
  _pidField->setFieldName(pidFieldName);
  _pidField->setFieldNameLong("hydrometeor_particle_type");
  _pidField->setUnits("");
  _pidField->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_GZIP,
                         Mdvx::SCALING_SPECIFIED, 1.0, 0.0);
  
  if (_params.debug) {
    cerr << "DONE computing Cartesian PID" << endl;
  }
  
  return 0;
  
}


//////////////////////////////////////////////////
// get interp field for given name
// returns null on error

BaseInterp::Field *RadxCartDP::_getInterpField(const string &name)
{
  for (size_t ii = 0; ii < _interpFields.size(); ii++) {
    if (_interpFields[ii].outputName == name) {
      return &_interpFields[ii];
    }
  }
  // not found - LDR is optional
  if (name != ldrForPidFieldName) {
    cerr << "ERROR - cannot find interp field, name: " << name << endl;
  }
  return nullptr;
}

//////////////////////////////////////////////////
// Print params for RATE

void RadxCartDP::_printParamsRate()
{

  if (_params.debug) {
    cerr << "Reading RATE params from file: "
         << _params.RATE_params_file_path << endl;
  }
  
  // do we need to expand environment variables?

  bool expandEnvVars = false;
  if (_args.printParamsRateMode.find("expand") != string::npos) {
    expandEnvVars = true;
  }

  // read in RATE params if applicable
  
  if (strstr(_params.RATE_params_file_path, "use-defaults") == NULL) {
    // not using defaults
    if (_precipRateParams.load(_params.RATE_params_file_path,
                               NULL, expandEnvVars, _args.tdrpDebug)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Cannot read params file for PrecipFilt: "
           << _params.RATE_params_file_path << endl;
      OK = false;
      return;
    }
  }

  // set print mode

  tdrp_print_mode_t printMode = PRINT_LONG;
  if (_args.printParamsRateMode.find("short") == 0) {
    printMode = PRINT_SHORT;
  } else if (_args.printParamsRateMode.find("norm") == 0) {
    printMode = PRINT_NORM;
  } else if (_args.printParamsRateMode.find("verbose") == 0) {
    printMode = PRINT_VERBOSE;
  }

  // do the print to stdout
  
  _precipRateParams.print(stdout, printMode);

}

//////////////////////////////////////////////////
// Print params for PID

void RadxCartDP::_printParamsPid()
{

  if (_params.debug) {
    cerr << "Reading PID params from file: " << _params.PID_params_file_path << endl;
  }

  // do we need to expand environment variables?

  bool expandEnvVars = false;
  if (_args.printParamsPidMode.find("expand") != string::npos) {
    expandEnvVars = true;
  }

  // read in PID params if applicable

  if (strstr(_params.PID_params_file_path, "use-defaults") == NULL) {
    // not using defaults
    if (_ncarPidParams.load(_params.PID_params_file_path,
                            NULL, expandEnvVars, _args.tdrpDebug)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Cannot read params file for PidFilt: "
           << _params.PID_params_file_path << endl;
      OK = false;
      return;
    }
  }

  // set print mode

  tdrp_print_mode_t printMode = PRINT_LONG;
  if (_args.printParamsPidMode.find("short") == 0) {
    printMode = PRINT_SHORT;
  } else if (_args.printParamsPidMode.find("norm") == 0) {
    printMode = PRINT_NORM;
  } else if (_args.printParamsPidMode.find("verbose") == 0) {
    printMode = PRINT_VERBOSE;
  }

  // do the print to stdout

  _ncarPidParams.print(stdout, printMode);

}

//////////////////////////////////////////////////
// Print params for KDP

void RadxCartDP::_printParamsKdp()
{

  if (_params.debug) {
    cerr << "Reading KDP params from file: " << _params.KDP_params_file_path << endl;
  }

  // do we need to expand environment variables?

  bool expandEnvVars = false;
  if (_args.printParamsKdpMode.find("expand") != string::npos) {
    expandEnvVars = true;
  }

  // read in KDP params if applicable

  if (strstr(_params.KDP_params_file_path, "use-defaults") == NULL) {
    // not using defaults
    if (_kdpFiltParams.load(_params.KDP_params_file_path,
                            NULL, expandEnvVars, _args.tdrpDebug)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Cannot read params file for KdpFilt: "
           << _params.KDP_params_file_path << endl;
      OK = false;
      return;
    }
  }

  // set print mode

  tdrp_print_mode_t printMode = PRINT_LONG;
  if (_args.printParamsKdpMode.find("short") == 0) {
    printMode = PRINT_SHORT;
  } else if (_args.printParamsKdpMode.find("norm") == 0) {
    printMode = PRINT_NORM;
  } else if (_args.printParamsKdpMode.find("verbose") == 0) {
    printMode = PRINT_VERBOSE;
  }

  // do the print to stdout

  _kdpFiltParams.print(stdout, printMode);

}

