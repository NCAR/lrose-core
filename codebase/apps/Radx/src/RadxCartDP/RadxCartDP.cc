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
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace std;

// initialize geom field names

string RadxCartDP::elevationFieldName = "ELEV";
string RadxCartDP::rangeFieldName = "RANGE";
string RadxCartDP::beamHtFieldName = "BEAM_HT";
string RadxCartDP::tempFieldName = "TEMP_C";
string RadxCartDP::pidFieldName = "PID";
string RadxCartDP::pidInterestFieldName = "PID_INTEREST";
string RadxCartDP::rateZrFieldName = "RATE_ZR";
string RadxCartDP::rateHybridFieldName = "RATE_HYBRID";
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

  _radarInterp = nullptr;
  Mdvx::initStruct(_interpCoord);

  _pidField = nullptr;

  _rateZrField = nullptr;
  _rateHybridField = nullptr;

  _qpeZrField = nullptr;
  _qpeHybridField = nullptr;

  _extinctionField = nullptr;
  _terrainHtField = nullptr;
  _haveBeamBlock = false;
  
  _convStratAvailable = false;

  _initRadarFieldTypes();
  _initModelFieldTypes();
  
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
  
  if (_checkRadarFields()) {
    cerr << "ERROR - radar_fields params not valid" << endl;
    cerr << "There must be exactly 1 entry for each type" << endl;
    OK = false;
  }

  if (_checkModelFields()) {
    cerr << "ERROR - model_fields params not valid" << endl;
    cerr << "There must be exactly 1 entry for each type" << endl;
    OK = false;
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
  
  // if requested, print params for convective-stratiform then exit
  
  if (_args.printParamsConvStrat) {
    _printParamsConvStrat();
    exit(0);
  }

  // read params for KdpFilt
  
  if (strstr(_params.KDP_params_file_path, "use-defaults") == nullptr) {
    // not using defaults
    if (_kdpFiltParams.load(_params.KDP_params_file_path,
                            nullptr, true, _args.tdrpDebug)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Cannot read params file for KdpFilt: "
           << _params.KDP_params_file_path << endl;
      OK = false;
      return;
    }
  }

  // read params for Ncar PID

  if (strstr(_params.PID_params_file_path, "use-defaults") == nullptr) {
    // not using defaults
    if (_ncarPidParams.load(_params.PID_params_file_path,
                            nullptr, true, _args.tdrpDebug)) {
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
  
  if (strstr(_params.RATE_params_file_path, "use-defaults") == nullptr) {
    // not using defaults
    if (_precipRateParams.load(_params.RATE_params_file_path,
                               nullptr, true, _args.tdrpDebug)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Cannot read params file for PrecipFilt: "
           << _params.RATE_params_file_path << endl;
      OK = false;
      return;
    }
  }

  // read in Convective / Stratiform params
  
  if (_params.identify_conv_strat_partition) {
    if (strstr(_params.conv_strat_params_file_path, "use-defaults") == nullptr) {
      // not using defaults
      if (_convStratParams.load(_params.conv_strat_params_file_path,
                                nullptr, true, _args.tdrpDebug)) {
        cerr << "ERROR: " << _progName << endl;
        cerr << "Cannot read params file for Convective/Stratiform partition: "
             << _params.conv_strat_params_file_path << endl;
        OK = false;
        return;
      }
    }
  }

  // initialize compute object
  
  pthread_mutex_init(&_debugPrintMutex, nullptr);
  
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

  if (_radarInterp) {
    delete _radarInterp;
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

  // special case - create grid template
  
  if (_params.create_grid_template) {
    return _createGridTemplate();
  }
  
  // runtime mode
  
  if (_params.radar_input_mode == Params::ARCHIVE) {
    return _runArchive();
  } else if (_params.radar_input_mode == Params::FILELIST) {
    return _runFilelist();
  } else {
    return _runRealtime();
  }
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

int RadxCartDP::_processFile(const string &radarPath)
{

  PMU_auto_register("Processing file");

  // check file name

  if (!_fileNameValid(radarPath)) {
    // skip this file
    return 0;
  }

  // initialize
  
  _radarVol.clear();

  if (_params.debug) {
    cerr << "INFO - RadxCartDP::_processFile" << endl;
  }
  
  // read in file
  
  if (_readFile(radarPath)) {
    cerr << "ERROR - RadxCartDP::_processFile" << endl;
    cerr << "  Cannot read file: " << _radarVol.getPathInUse() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "SUCCESS - read in file: " << _radarVol.getPathInUse() << endl;
  }
  
  // initialize the projection and vlevels of the interp volume

  _initInterpGrid();

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

  // free up scalar rays

  _freeScalarRays();

  if (_params.write_debug_polar_output) {
    if (_writeDebugPolarOutput()) {
      cerr << "WARNING - cannot write debug files in polar coords" << endl;
    }
  }

  // set up interpolation fields
  
  _initInterp();
  
  // interpolate and write out
  
  _allocInterpToCart();
  if (_rhiMode) {
    if (_params.debug) {
      cerr << "  NOTE: data is in RHI mode" << endl;
    }
    _radarInterp->setRhiMode(true);
    _radarInterp->interpVol();
  } else {
    _radarInterp->setRhiMode(false);
    _radarInterp->interpVol();
  }

  // compute particle ID type
  
  int iret = 0;
  if (_computePid()) {
    cerr << "ERROR - RadxCartDP" << endl;
    cerr << "  Cannot compute PID" << endl;
    iret = -1;
  }
  
  // compute precip rates
  
  if (_computePrecip()) {
    cerr << "ERROR - RadxCartDP" << endl;
    cerr << "  Cannot compute precip rates" << endl;
    iret = -1;
  }

  // read in beam blockage if requested

  if (_params.read_beam_blockage) {
    if (!_haveBeamBlock) {
      if (_readBeamBlock() == 0) {
        _haveBeamBlock = true;
      } else {
        cerr << "ERROR - RadxCartDP" << endl;
        cerr << "  Cannot read in beam blockage" << endl;
        iret = -1;
      }
    }
  }

  // add QPE fields if we have beam blockage and terrain data

  if (_params.add_qpe_field) {
    if (_haveBeamBlock) {
      if (_computeQpe()) {
        cerr << "ERROR - RadxCartDP" << endl;
        cerr << "  Cannot add QPE field" << endl;
        iret = -1;
      }
    } else {
      // no beam blockage data, so this cannot be done
      cerr << "ERROR - RadxCartDP" << endl;
      cerr << "  Cannot add QPE field" << endl;
      cerr << "  For this we need beam blockage data, and it is not available." << endl;
      iret = -1;
    }
  }

  // compute convective stratiform split

  _convStratAvailable = false;
  if (_params.identify_conv_strat_partition) {
    // convective / stratiform split
    // _printRunTime("Cart interp - before strat/conv");
    if (_computeConvStrat() == 0) {
      _convStratAvailable = true;
    }
    // _printRunTime("Cart interp - after strat/conv");
  }
  
  // write out MDV file
  
  if (_writeOutputMdv()) {
    cerr << "ERROR - RadxCartDP" << endl;
    cerr << "  Cannot write output file" << endl;
    iret = -1;
  }

  // free up

  _radarInterp->freeMemory();
  if (_params.free_memory_between_files) {
    _radarVol.clear();
    _freeInterpRays();
  }
  
  return iret;

}

//////////////////////////////////////////////////
// Check file name is valid
// Returns true on success, false on failure

bool RadxCartDP::_fileNameValid(const string &radarPath)
{
  
  if (strlen(_params.radar_file_search_ext) > 0) {
    RadxPath rpath(radarPath);
    if (strcmp(rpath.getExt().c_str(), _params.radar_file_search_ext)) {
      if (_params.debug) {
        cerr << "WARNING - ignoring file: " << radarPath << endl;
        cerr << "  Does not have correct extension: "
             << _params.radar_file_search_ext << endl;
      }
      return false;
    }
  }
  
  if (strlen(_params.radar_file_search_substr) > 0) {
    RadxPath rpath(radarPath);
    if (rpath.getFile().find(_params.radar_file_search_substr) == string::npos) {
      if (_params.debug) {
        cerr << "WARNING - ignoring file: " << radarPath << endl;
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

int RadxCartDP::_readFile(const string &radarPath)
{

  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  if (inFile.readFromPath(radarPath, _radarVol)) {
    cerr << "ERROR - RadxCartDP::_readFile" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  
  // check we have at least 2 rays
  
  if (_radarVol.getNRays() < 2) {
    cerr << "ERROR - RadxCartDP::_readFile" << endl;
    cerr << "  Too few rays: " << _radarVol.getNRays() << endl;
    return -1;
  }
  
  // convert to fl32
  
  _radarVol.convertToFl32();

  // pad out the gates to the longest range
  
  _radarVol.setNGatesConstant();

  // set range geometry
  
  if (_params.override_gate_geometry) {
    // override gate geometry if requested
    _radarVol.setRangeGeom(_params.start_range_km, _params.gate_spacing_km);
  } else {
    // remap all rays to finest geom
    _radarVol.remapToFinestGeom();
  }
  
  //  check for rhi
  
  _rhiMode = _isRhi();

  // rename fields if requested

  for (int ii = 0; ii < _params.radar_fields_n; ii++) {
    string inName = _params._radar_fields[ii].input_name;
    string outName = _params._radar_fields[ii].output_name;
    if (inName.size() > 0 && inName != outName) {
      _radarVol.renameField(inName, outName);
    }
  }

  // override radar location if requested

  if (_params.override_radar_location) {
    _radarVol.overrideLocation(_params.radar_latitude_deg,
                               _params.radar_longitude_deg,
                               _params.radar_altitude_meters / 1000.0);
  }

  // override beam width if requested
  
  if (_params.override_beam_width) {
    _radarVol.setRadarBeamWidthDegH(_params.beam_width_deg_h);
    _radarVol.setRadarBeamWidthDegV(_params.beam_width_deg_v);
  }

  // override fixed angle if required

  if (_params.override_fixed_angle_with_mean_measured_angle) {
    _radarVol.computeFixedAnglesFromRays();
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  _startRangeKm: " << _radarVol.getStartRangeKm() << endl;
    cerr << "  _gateSpacingKm: " << _radarVol.getGateSpacingKm() << endl;
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

  _radarHtKm = _radarVol.getAltitudeKm();
  _wavelengthM = _radarVol.getWavelengthM();

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

  for (int ii = 0; ii < _params.radar_fields_n; ii++) {
    if (_params._radar_fields[ii].is_available) {
      file.addReadField(_params._radar_fields[ii].input_name);
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

  vector<RadxRay *> &rays = _radarVol.getRays();
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
    beamHt.setInstrumentHtKm(_radarVol.getAltitudeKm());
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
  
  _radarVol.setRayNumbersInOrder();

  // initialize pid

  _freeScalarRays();
  
  // loop through the input rays,
  // computing the pid fields

  const vector<RadxRay *> &inputRays = _radarVol.getRays();
  for (size_t iray = 0; iray < inputRays.size(); iray++) {

    // get a thread from the pool
    bool isDone = true;
    ScalarsThread *thread = 
      (ScalarsThread *) _scalarsThreadPool.getNextThread(true, isDone);
    if (thread == nullptr) {
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
    if (thread == nullptr) {
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
  if (scalarsRay == nullptr) {
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

  vector<RadxRay *> &readRays = _radarVol.getRays();
  
  if (readRays.size() != _scalarRays.size()) {
    cerr << "ERROR - RadxCartDP::_mergeScalarsIntoReadVol" << endl;
    cerr << "  readRays size: " << readRays.size() << endl;
    cerr << "  _scalarRays size: " << _scalarRays.size() << endl;
    return -1;
  }

  for (size_t ii = 0; ii < _scalarRays.size(); ii++) {

    RadxRay *scRay = _scalarRays[ii];
    if (scRay == nullptr) {
      cerr << "ERROR - null derived ray at index: " << ii << endl;
      return -1;
    }

    int rayNum = scRay->getRayNumber();
    if (rayNum < 0 || rayNum >= (int) readRays.size()) {
      cerr << "ERROR - bad ray number in derived ray: " << rayNum << endl;
      return -1;
    }

    RadxRay *readRay = readRays[rayNum];
    if (readRay == nullptr) {
      cerr << "ERROR - null read ray at rayNum: " << rayNum << endl;
      return -1;
    }

    const vector<RadxField *> &scFields = scRay->getFields();
    for (size_t jj = 0; jj < scFields.size(); jj++) {

      const RadxField *sc = scFields[jj];
      if (sc == nullptr) {
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
// Free up scalar rays

void RadxCartDP::_freeScalarRays()
{
  for (size_t ii = 0; ii < _scalarRays.size(); ii++) {
    delete _scalarRays[ii];
  }
  _scalarRays.clear();
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
  if (out.writeToDir(_radarVol, _params.debug_polar_output_dir, true, false)) {
    cerr << "ERROR - RadxCartDP::_writeDebugPolarOutput()" << endl;
    cerr << "  Cannot write debug polar file, path: "
         << out.getPathInUse() << endl;
    cerr << out.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "SUCCESS - wrote debug polar file, path: "
         << out.getPathInUse() << endl;
  }
  
  return 0;
  
}


//////////////////////////////////////////////////
// add angle fields

void RadxCartDP::_addAngleFields()
{
  
  // loop through rays
    
  vector<RadxRay *> &rays = _radarVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {
      
    RadxRay *ray = rays[iray];
    int nGates = ray->getNGates();
    vector<Radx::fl32> data32(nGates);
      
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
        data32[ii] = az;
      }
      azimuthField->setFieldFolds(0.0, 360.0);
      azimuthField->addDataFl32(nGates, data32.data());
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
        data32[ii] = el;
      }
      elevationField->addDataFl32(nGates, data32.data());
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
        data32[ii] = sinAz * cosEl;
      }
      alphaField->addDataFl32(nGates, data32.data());
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
        data32[ii] = cosAz * cosEl;
      }
      betaField->addDataFl32(nGates, data32.data());
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
        data32[ii] = sinEl;
      }
      gammaField->addDataFl32(nGates, data32.data());
      ray->addField(gammaField);
    }

  } // iray

}
  
//////////////////////////////////////////////////
// add range field

void RadxCartDP::_addRangeField()
{
  
  vector<RadxRay *> &rays = _radarVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    RadxRay *ray = rays[iray];
    int nGates = ray->getNGates();
    vector<Radx::fl32> data32(nGates);
    
    // range field
    
    RadxField *rangeField = new RadxField(_params.range_field_name, "km");
    rangeField->setLongName("range_from_radar");
    rangeField->setStandardName("slant_range");
    rangeField->setTypeFl32(-9999.0);
    double range = ray->getStartRangeKm();
    for (int ii = 0; ii < nGates; ii++) {
      data32[ii] = range;
      range += ray->getGateSpacingKm();
    }
    rangeField->addDataFl32(nGates, data32.data());
    ray->addField(rangeField);
  }

}
  
//////////////////////////////////////////////////
// add height field

void RadxCartDP::_addHeightField()
{
  
  // beamHeight

  BeamHeight beamHt;
  beamHt.setInstrumentHtKm(_radarVol.getAltitudeKm());
  if (_params.override_standard_pseudo_earth_radius) {
    beamHt.setPseudoRadiusRatio(_params.pseudo_earth_radius_ratio);
  }

  vector<RadxRay *> &rays = _radarVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    RadxRay *ray = rays[iray];
    int nGates = ray->getNGates();
    vector<Radx::fl32> data32(nGates);
    
    RadxField *heightField = new RadxField(_params.height_field_name, "km");
    heightField->setLongName("height_msl");
    heightField->setStandardName("height_msl");
    heightField->setTypeFl32(-9999.0);
    double elevDeg = ray->getElevationDeg();
    double range = ray->getStartRangeKm();
    for (int ii = 0; ii < nGates; ii++) {
      double ht = beamHt.computeHtKm(elevDeg, range);
      data32[ii] = ht;
      range += ray->getGateSpacingKm();
    }
    heightField->addDataFl32(nGates, data32.data());
    ray->addField(heightField);
  }
  
}

//////////////////////////////////////////////////
// add coverage field

void RadxCartDP::_addCoverageField()
{
  
  vector<RadxRay *> &rays = _radarVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    RadxRay *ray = rays[iray];
    int nGates = ray->getNGates();
    vector<Radx::fl32> data32(nGates);
    
    RadxField *covFld = new RadxField(_params.coverage_field_name, "");
    covFld->setLongName("radar_coverage_flag");
    covFld->setStandardName("radar_coverage_flag");
    covFld->setTypeFl32(-9999.0);
    for (int ii = 0; ii < nGates; ii++) {
      data32[ii] = 1.0;
    }
    covFld->addDataFl32(nGates, data32.data());
    covFld->setIsDiscrete(true);
    ray->addField(covFld);

  } // iray

}

//////////////////////////////////////////////////
// add time field

void RadxCartDP::_addTimeField()
{

  // start times

  time_t startTimeSecs = _radarVol.getStartTimeSecs();
  double startNanoSecs = _radarVol.getStartNanoSecs();

  // loop through rays

  vector<RadxRay *> &rays = _radarVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    RadxRay *ray = rays[iray];
    int nGates = ray->getNGates();
    vector<Radx::fl32> data32(nGates);

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
      data32[ii] = secsSinceStart;
    }
    timeFld->addDataFl32(nGates, data32.data());
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
  const vector<RadxRay *> &rays = _radarVol.getRays();
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

////////////////////////////////////////////////////////////////
// initialize the projection and vlevels of the interp volume

void RadxCartDP::_initInterpGrid()
{

  Mdvx::initStruct(_interpCoord);

  _interpCoord.nx = _params.grid_xy_geom.nx;
  _interpCoord.ny = _params.grid_xy_geom.ny;
  _interpCoord.nz = _params.grid_z_geom.nz;
  
  _interpNpointsPlane = _interpCoord.nx * _interpCoord.ny;
  _interpNpointsVol = _interpNpointsPlane * _interpCoord.nz;
  
  _interpCoord.dx = _params.grid_xy_geom.dx;
  _interpCoord.dy = _params.grid_xy_geom.dy;
  _interpCoord.dz = _params.grid_z_geom.dz;
  
  _interpCoord.minx = _params.grid_xy_geom.minx;
  _interpCoord.miny = _params.grid_xy_geom.miny;
  _interpCoord.minz = _params.grid_z_geom.minz;

  // init _interpVlevels
  
  if (_params.specify_individual_z_levels) {
    _interpVlevels.clear();
    for (int ii = 0; ii < _params.z_level_array_n; ii++) {
      _interpVlevels.push_back(_params._z_level_array[ii]);
    }
    _interpCoord.nz = _interpVlevels.size();
    _interpCoord.minz = _interpVlevels[0];
    if (_interpCoord.nz > 1) {
      _interpCoord.dz = _interpVlevels[1] - _interpVlevels[0];
    } else {
      _interpCoord.dz = 0.0;
    }
  } else {
    _interpVlevels.clear();
    for (int ii = 0; ii < _interpCoord.nz; ii++) {
      _interpVlevels.push_back(_interpCoord.minz + ii * _interpCoord.dz);
    }
  }

  // init _interpProj
  
  _interpCoord.proj_type = _params.grid_projection;
  if (_params.center_grid_on_radar) {
    _interpCoord.proj_origin_lat = _radarVol.getLatitudeDeg();
    _interpCoord.proj_origin_lon = _radarVol.getLongitudeDeg();
  } else {
    _interpCoord.proj_origin_lat = _params.grid_origin_lat;
    _interpCoord.proj_origin_lon = _params.grid_origin_lon;
  }
  
  switch (_params.grid_projection) {

    case Params::PROJ_LATLON:
      _interpCoord.proj_type = Mdvx::PROJ_LATLON;
      break;
      
    case Params::PROJ_LAMBERT_CONF:
      _interpCoord.proj_type = Mdvx::PROJ_LAMBERT_CONF;
      _interpCoord.proj_params.lc2.lat1 = _params.grid_lat1;
      _interpCoord.proj_params.lc2.lat2 = _params.grid_lat2;
      break;

    case Params::PROJ_MERCATOR:
      _interpCoord.proj_type = Mdvx::PROJ_MERCATOR;
      break;

    case Params::PROJ_POLAR_STEREO:
      _interpCoord.proj_type = Mdvx::PROJ_POLAR_STEREO;
      _interpCoord.proj_params.ps.tan_lon = _params.grid_tangent_lon;
      _interpCoord.proj_params.ps.central_scale = _params.grid_central_scale;
      if (_params.grid_pole_is_north) {
        _interpCoord.proj_params.ps.pole_type = Mdvx::POLE_NORTH;
      } else {
        _interpCoord.proj_params.ps.pole_type = Mdvx::POLE_SOUTH;
      }
      break;

    case Params::PROJ_FLAT:
      _interpCoord.proj_type = Mdvx::PROJ_FLAT;
      _interpCoord.proj_params.flat.rotation = _params.grid_rotation;
      break;

    case Params::PROJ_OBLIQUE_STEREO:
      _interpCoord.proj_type = Mdvx::PROJ_OBLIQUE_STEREO;
      _interpCoord.proj_params.os.tan_lat = _params.grid_tangent_lat;
      _interpCoord.proj_params.os.tan_lon = _params.grid_tangent_lon;
      _interpCoord.proj_params.os.central_scale = _params.grid_central_scale;
      break;

    case Params::PROJ_TRANS_MERCATOR:
      _interpCoord.proj_type = Mdvx::PROJ_TRANS_MERCATOR;
      _interpCoord.proj_params.tmerc.central_scale = _params.grid_central_scale;
      break;

    case Params::PROJ_ALBERS:
      _interpCoord.proj_type = Mdvx::PROJ_ALBERS;
      _interpCoord.proj_params.albers.lat1 = _params.grid_lat1;
      _interpCoord.proj_params.albers.lat2 = _params.grid_lat2;
      break;

    case Params::PROJ_LAMBERT_AZIM:
      _interpCoord.proj_type = Mdvx::PROJ_LAMBERT_AZIM;
      break;

  } // switch

  _interpProj.init(_interpCoord);

  // initialize lookup table

  _modelRemap.setTargetCoords(_interpProj, _interpVlevels);

}

//////////////////////////////////////////////////
// initialize interpolation fields

void RadxCartDP::_initInterp()

{

  // ensure interp rays are freed
  
  _freeInterpRays();
  
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
  
  vector<string> fieldNames = _radarVol.getUniqueFieldNameList();

  // find an example of each field, by search through the rays
  // use that field as the template

  for (size_t ifield = 0; ifield < fieldNames.size(); ifield++) {

    string radxName = fieldNames[ifield];
    Params::radar_field_t *fieldParams = getRadarField(radxName);

    vector<RadxRay *> &rays = _radarVol.getRays();
    for (size_t iray = 0; iray < rays.size(); iray++) {
      const RadxRay *ray = rays[iray];
      const RadxField *field = ray->getField(radxName);
      if (field != nullptr) {
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
        if (fieldParams && fieldParams->do_write) {
          interpField.writeToFile = true;
        }
        _interpFields.push_back(interpField);
        break;
      }
    }

  } // ifield

  // override fold limits from the parameters

  double nyquist = 0.0;
  vector<RadxRay *> &rays = _radarVol.getRays();
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
          _interpFields[ifld].foldRange = (_interpFields[ifld].foldLimitUpper -
                                           _interpFields[ifld].foldLimitLower);
          break;
        }
      } // ifld
    } // ii
  }

  // KDP field
  
  if (_params.output_KDP_field) {
    BaseInterp::Field KDPField;
    KDPField.radxName = kdpForPidFieldName;
    KDPField.outputName = _params.KDP_field_name;
    KDPField.longName = "specific_differential_phase";
    KDPField.standardName = "specific_differential_phase";
    KDPField.units = "deg/km";
    KDPField.isDiscrete = false;
    KDPField.writeToFile = true;
    _interpFields.push_back(KDPField);
  }

  // range field
  
  if (_params.output_range_field) {
    BaseInterp::Field rangeField;
    rangeField.radxName = _params.range_field_name;
    rangeField.outputName = _params.range_field_name;
    rangeField.longName = "slant_range";
    rangeField.standardName = "slant_range";
    rangeField.units = "km";
    rangeField.isDiscrete = false;
    rangeField.writeToFile = true;
    _interpFields.push_back(rangeField);
  }

  // height field
  
  if (_params.output_height_field) {
    BaseInterp::Field heightField;
    heightField.radxName = _params.height_field_name;
    heightField.outputName = _params.height_field_name;
    heightField.longName = "beam_height";
    heightField.standardName = "height";
    heightField.units = "km";
    heightField.isDiscrete = false;
    heightField.writeToFile = true;
    _interpFields.push_back(heightField);
  }

  // coverage field
  
  if (_params.output_coverage_field) {
    BaseInterp::Field coverageField;
    coverageField.radxName = _params.coverage_field_name;
    coverageField.outputName = _params.coverage_field_name;
    coverageField.longName = "radar_coverage";
    coverageField.standardName = "radar_coverage";
    coverageField.units = "";
    coverageField.isDiscrete = true;
    coverageField.writeToFile = true;
    _interpFields.push_back(coverageField);
  }

  // time field
  
  if (_params.output_time_field) {
    BaseInterp::Field timeField;
    timeField.radxName = _params.time_field_name;
    timeField.outputName = _params.time_field_name;
    timeField.longName = "time_since_start_of_volume";
    timeField.standardName = "time";
    timeField.units = "secs";
    timeField.writeToFile = true;
    _interpFields.push_back(timeField);
  }

}

////////////////////////////////////////////////////////////
// Allocate interpolation objects as needed

void RadxCartDP::_allocInterpToCart()
{
  if (_radarInterp == nullptr) {
    _radarInterp = new RadarInterp(_progName, _params, _radarVol,
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
  
  vector<RadxRay *> &rays = _radarVol.getRays();
  for (size_t isweep = 0; isweep < _radarVol.getNSweeps(); isweep++) {

    const RadxSweep *sweep = _radarVol.getSweeps()[isweep];

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
  
  vector<RadxRay *> &rays = _radarVol.getRays();
  
  for (size_t ifield = 0; ifield < _interpFields.size(); ifield++) {
    bool found = false;
    string fieldName = _interpFields[ifield].radxName;
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxRay *ray = rays[ii];
      const RadxField *fld = ray->getField(fieldName);
      if (fld != nullptr) {
        found = true;
        _interpFields[ifield].longName = fld->getLongName();
        _interpFields[ifield].standardName = fld->getStandardName();
        _interpFields[ifield].units = fld->getUnits();
        break;
      }
    }
    if (!found) {
      cerr << "WARNING - field not found: " << fieldName << endl;
      cerr << "  File: " << _radarVol.getPathInUse() << endl;
    }
  } // ifield

}

/////////////////////////////////////////////////////////
// read in model data
//
// Returns 0 on success, -1 on failure.

int RadxCartDP::_readModel()

{

  time_t radarTime = _radarVol.getStartTimeSecs();

  _modelRawMdvx.clearRead();
  _modelRawMdvx.setReadTime(Mdvx::READ_CLOSEST,
                            _params.model_input_url,
                            _params.model_search_margin_secs,
                            radarTime);

  for (int ii = 0; ii < _params.model_fields_n; ii++) {
    if (_params._model_fields[ii].is_available) {
      continue;
    }
    _modelRawMdvx.addReadField(_params._model_fields[ii].input_name);
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

  // compute the temperature profile from the model data

  if (_computeTempProfile()) {
    cerr << "ERROR - RadxCartDP::_readModel" << endl;
    cerr << "  Cannot compute temp profile, time: "
         << RadxTime::strm(_radarVol.getStartTimeSecs()) << endl;
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
    _modelInterpMdvx.getField(getModelInputName(Params::TEMP).c_str());
  if (tempFld == nullptr) {
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
    _modelInterpMdvx.addField(interpField);
  } // ifield
  
}

/////////////////////////////////////////////////////////
// read in beam blockage data
//
// Returns 0 on success, -1 on failure.

int RadxCartDP::_readBeamBlock()

{

  if (_params.debug) {
    cerr << "Reading in beam block file: "
         << _params.beam_block_input_file_path << endl;
  }
  
  // set up read
  
  _beamBlockMdvx.clearRead();
  _beamBlockMdvx.setReadPath(_params.beam_block_input_file_path);

  _beamBlockMdvx.addReadField(_params.beam_extinction_field_name);
  _beamBlockMdvx.addReadField(_params.terrain_ht_field_name);
  
  // perform read
  
  if (_beamBlockMdvx.readVolume()) {
    cerr << "ERROR - RadxCartDP::_readBeamBlock" << endl;
    cerr << "  Cannot read beam blockage data" << endl;
    cerr << "  File path: " << _params.beam_block_input_file_path << endl;
    cerr << _beamBlockMdvx.getErrStr() << endl;
    return -1;
  }

  // set field pointers

  _extinctionField = _beamBlockMdvx.getField(_params.beam_extinction_field_name);
  _terrainHtField = _beamBlockMdvx.getField(_params.terrain_ht_field_name);

  if (!_extinctionField) {
    cerr << "ERROR - RadxCartDP::_readBeamBlock" << endl;
    cerr << "  Cannot read beam extinction field: "
         << _params.beam_extinction_field_name << endl;
    cerr << "  File path: " << _params.beam_block_input_file_path << endl;
    return -1;
  }

  if (!_terrainHtField) {
    cerr << "ERROR - RadxCartDP::_readBeamBlock" << endl;
    cerr << "  Cannot read terrain height field: "
         << _params.terrain_ht_field_name << endl;
    cerr << "  File path: " << _params.beam_block_input_file_path << endl;
    return -1;
  }

  // check the beam block grid matches the interpolation grid
  
  MdvxProj bbProj(_beamBlockMdvx.getMasterHeader(),
                  _extinctionField->getFieldHeader());
  if (bbProj != _interpProj) {
    cerr << "ERROR - RadxCartDP::_readBeamBlock" << endl;
    cerr << "  BeamBlock grid does not match interp Cart grid" << endl;
    cerr << "==================================================" << endl;
    cerr << "================== Interp proj ===================" << endl;
    _interpProj.print(cerr);
    cerr << "==================================================" << endl;
    cerr << "================== BeamBlock proj ================" << endl;
    bbProj.print(cerr);
    cerr << "==================================================" << endl;
    return -1;
  }

  if (_extinctionField->getFieldHeader().nz != (si64) _interpVlevels.size()) {
    cerr << "ERROR - RadxCartDP::_readBeamBlock" << endl;
    cerr << "  BeamBlock grid nz does not match Cart grid nz" << endl;
    cerr << "  BeamBlock grid nz: "
         << _extinctionField->getFieldHeader().nz << endl;
    cerr << "  Cart grid nz: " << _interpVlevels.size() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "SUCCESS - got beam block file: "
         << _params.beam_block_input_file_path << endl;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// compute the PID field

int RadxCartDP::_computePid()
{

  if (_params.debug) {
    cerr << "Computing PID" << endl;
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

  string modelTempName = getModelInputName(Params::TEMP);
  MdvxField *tempFld = _modelInterpMdvx.getField(modelTempName.c_str());
  if (!tempFld) {
    cerr << "ERROR - _computePID" << endl;
    cerr << "  Model temp field missing: " << modelTempName << endl;
    return -1;
  }
  fl32 *tempArray = (fl32 *) tempFld->getVol();

  // compute PID for each point in the Cartesian volume
  
  _pidArray.resize(_interpNpointsVol);
  
  for (size_t ii = 0; ii < _interpNpointsVol; ii++) {
    
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
    
    _pidArray[ii] = pid;
    
  } // ii
  
  // create PID field filtered with a mode in 2D planes

  if (_params.debug) {
    cerr << "Applying mode filter to PID" << endl;
  }
  
  _pidFilt = _pidArray;
  int kernelSize = _params.PID_mode_filter_kernel_size;
  _modeFilterPid2D(_pidArray.data(),
                   _pidFilt.data(),
                   _radarInterp->getGridZLevels().size(),
                   _radarInterp->getGridNy(),
                   _radarInterp->getGridNx(),
                   kernelSize);
  
  if (_params.debug) {
    cerr << "DONE applying mode filter to PID" << endl;
  }
  
  // create MdvxField for pid
  
  Mdvx::field_header_t pidFhdr(tempFld->getFieldHeader());
  Mdvx::vlevel_header_t pidVhdr(tempFld->getVlevelHeader());
  
  pidFhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  pidFhdr.compression_type = Mdvx::COMPRESSION_NONE;
  pidFhdr.data_element_nbytes = sizeof(fl32);
  pidFhdr.volume_size = _interpNpointsVol * sizeof(fl32);
  
  _pidField = new MdvxField(pidFhdr, pidVhdr, _pidFilt.data());
  _pidField->setFieldName(pidFieldName);
  _pidField->setFieldNameLong("hydrometeor_particle_type");
  _pidField->setUnits("");
  _pidField->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_GZIP,
                         Mdvx::SCALING_SPECIFIED, 1.0, 0.0);
  
  if (_params.debug) {
    cerr << "DONE computing PID" << endl;
  }
  
  return 0;
  
}

//////////////////////////////////////////////////
// compute the Precip fields

int RadxCartDP::_computePrecip()
{
  
  if (_params.debug) {
    cerr << "Computing precip rates" << endl;
  }

  PrecipRate rate;
  rate.setFromParams(_precipRateParams);
  rate.setMissingVal(Radx::missingFl32);
  
  // get the fields we need for computing PID

  BaseInterp::Field *dbzFld = _getInterpField(dbzForPidFieldName);
  BaseInterp::Field *zdrFld = _getInterpField(zdrForPidFieldName);
  BaseInterp::Field *kdpFld = _getInterpField(kdpForPidFieldName);
  
  if (!dbzFld || !zdrFld || !kdpFld) {
    cerr << "ERROR - _computePrecip" << endl;
    cerr << "  One or more feature fields is missing" << endl;
    return -1;
  }

  vector<fl32> rateZrArray(_interpNpointsVol, Radx::missingFl32);
  vector<fl32> rateHybridArray(_interpNpointsVol, Radx::missingFl32);

  for (size_t ii = 0; ii < _interpNpointsVol; ii++) {
    
    double dbz = dbzFld->outputField[ii];
    double zdr = zdrFld->outputField[ii];
    double kdp = kdpFld->outputField[ii];
    int pid = _pidFilt[ii];
    
    double rateZr, rateZrSnow, rateZrMixed;
    double rateKdp, rateKdpZdr, rateZZdr;
  
    rate.computeBaseRates(dbz, zdr, kdp,
                          rateZr, rateZrSnow, rateZrMixed,
                          rateKdp, rateKdpZdr, rateZZdr);
    
    double rateHybrid, rateHidro, rateBringi;

    rate.computeHybrid(dbz, zdr, kdp,
                       rateZr, rateZrSnow, rateZrMixed,
                       rateKdp, rateKdpZdr, rateZZdr,
                       pid, rateHybrid, rateHidro, rateBringi);

    rateZrArray[ii] = rateZr;
    rateHybridArray[ii] = rateHybrid;
    
  } // ii
  
  // median filter for precip
  
  if (_params.debug) {
    cerr << "Applying median filter to precip" << endl;
  }
  
  _rateZrFilt = rateZrArray;
  _rateHybridFilt = rateHybridArray;
  
  int kernelSize = _params.RATE_median_filter_kernel_size;
  
  _medianFilter2D(rateZrArray.data(),
                  _rateZrFilt.data(),
                  _radarInterp->getGridZLevels().size(),
                  _radarInterp->getGridNy(),
                  _radarInterp->getGridNx(),
                  kernelSize, Radx::missingFl32, true);
  
  _medianFilter2D(rateHybridArray.data(),
                  _rateHybridFilt.data(),
                  _radarInterp->getGridZLevels().size(),
                  _radarInterp->getGridNy(),
                  _radarInterp->getGridNx(),
                  kernelSize, Radx::missingFl32, true);
 
  if (_params.debug) {
    cerr << "Done applying median filter to precip" << endl;
  }
  
  // create MdvxFields for precip
  
  Mdvx::field_header_t rateFhdr(_pidField->getFieldHeader());
  Mdvx::vlevel_header_t rateVhdr(_pidField->getVlevelHeader());
  
  rateFhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  rateFhdr.compression_type = Mdvx::COMPRESSION_NONE;
  rateFhdr.data_element_nbytes = sizeof(fl32);
  rateFhdr.volume_size = _interpNpointsVol * sizeof(fl32);

  _rateZrField = new MdvxField(rateFhdr, rateVhdr, _rateZrFilt.data());
  _rateZrField->setFieldName(rateZrFieldName);
  _rateZrField->setFieldNameLong("precip_rate_from_reflectivity");
  _rateZrField->setUnits("mm/hr");
  _rateZrField->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_GZIP,
                            Mdvx::SCALING_SPECIFIED, 1.0, 0.0);
  
  _rateHybridField = new MdvxField(rateFhdr, rateVhdr, _rateHybridFilt.data());
  _rateHybridField->setFieldName(rateHybridFieldName);
  _rateHybridField->setFieldNameLong
    ("precip_rate_hybrid_of_zh_zzdr_kdp_and_kdpzdr");
  _rateHybridField->setUnits("mm/hr");
  _rateHybridField->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_GZIP,
                                Mdvx::SCALING_SPECIFIED, 1.0, 0.0);
  
  if (_params.debug) {
    cerr << "DONE applying median filter to precip" << endl;
    cerr << "DONE computing precip rates" << endl;
  }
  
  return 0;
  
}

/////////////////////////////////////////////////////////
// compute QPE fields
// Returns 0 on success, -1 on failure.

int RadxCartDP::_computeQpe()

{

  if (_params.debug) {
    cerr << "Computing QPE fields" << endl;
  }
  
  // allocate 2D arrays
  
  _qpeZr.resize(_interpNpointsPlane, Radx::missingFl32);
  _qpeHybrid.resize(_interpNpointsPlane, Radx::missingFl32);
  
  // beam blockage and terrain ht arrays

  MdvxField extinctionField(*_extinctionField);
  extinctionField.convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  const fl32 *extinct = (const fl32*) extinctionField.getVol();

  MdvxField terrainHtField(*_terrainHtField);
  terrainHtField.convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  const fl32 *terrainHt = (const fl32*) terrainHtField.getVol();
  
  // beam ht field
  
  BaseInterp::Field *beamHtFld = _getInterpField(beamHtFieldName);
  const fl32 *beamHt = beamHtFld->outputField.data();

  // loop through the (y,x) domain

  size_t index2D = 0;
  for (int iy = 0; iy < _interpCoord.ny; iy++) {
    for (int ix = 0; ix < _interpCoord.nx; ix++, index2D++) {

      double tHtKm = terrainHt[index2D] / 1000.0;
      double minValidHtKm = tHtKm + _params.qpe_ht_margin_above_terrain_km;
      
      // start at the bottom of the column and move upwards
      
      size_t index3D = index2D;
      for (int iz = 0; iz < _interpCoord.nz; iz++, index3D += _interpNpointsPlane) {

        // move up if too close to terrain
        
        double zKm = _interpVlevels[iz];

        if (zKm < minValidHtKm) {
          continue;
        }
        
        // move up if we are below the lowest beam

        if (beamHt[index3D] == Radx::missingFl32) {
          continue;
        }

        // move up if beam is blocked in excess of threshold

        if (extinct[index3D] > _params.qpe_max_extinction_fraction) {
          continue;
        }

        // passes all tests, set QPE values

        _qpeZr[index2D] = _rateZrFilt[index3D];
        _qpeHybrid[index2D] = _rateHybridFilt[index3D];
        
        // done with this (x,y) location

        break;
        
      } // iz
      
    } // ix
  } // iy
  
  // create QPE Mdvx fields

  // field header
  
  Mdvx::field_header_t fhdr(_rateZrField->getFieldHeader());
  fhdr.nz = 1;
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.dz_constant = true;
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = 4;
  fhdr.volume_size = fhdr.nx * fhdr.ny * 1 * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  fhdr.grid_dz = 1.0;
  fhdr.grid_minz = 0;
  fhdr.scale = 1.0;
  fhdr.bias = 0.0;
  fhdr.bad_data_value = Radx::missingFl32;
  fhdr.missing_data_value = Radx::missingFl32;
  fhdr.min_value = 0;
  fhdr.max_value = 0;
  fhdr.min_value_orig_vol = 0;
  fhdr.max_value_orig_vol = 0;
  
  // vlevel header
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  vhdr.level[0] = 0;
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;

  // hybrid field
  
  _qpeHybridField = new MdvxField(fhdr, vhdr);
  _qpeHybridField->setFieldName(_params.qpe_hybrid_field_name);
  _qpeHybridField->setFieldNameLong(_rateZrField->getFieldNameLong());
  _qpeHybridField->setUnits(_rateZrField->getUnits());
  _qpeHybridField->setVolData(_qpeHybrid.data(),
                              fhdr.volume_size, Mdvx::ENCODING_FLOAT32);
  _qpeHybridField->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_GZIP);

  // Zr field

  _qpeZrField = new MdvxField(fhdr, vhdr);
  _qpeZrField->setFieldName(_params.qpe_zr_field_name);
  _qpeZrField->setFieldNameLong(_rateZrField->getFieldNameLong());
  _qpeZrField->setUnits(_rateZrField->getUnits());
  _qpeZrField->setVolData(_qpeZr.data(), fhdr.volume_size, Mdvx::ENCODING_FLOAT32);
  _qpeZrField->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_GZIP);

  if (_params.debug) {
    cerr << "DONE computing QPE fields" << endl;
  }
  
  return 0;

}

/////////////////////////////////////////////////////////
// compute Convective / Stratiform partition
// Returns 0 on success, -1 on failure.

int RadxCartDP::_computeConvStrat()

{

  if (_params.debug) {
    cerr << "Computing CONV-STRAT fields" << endl;
  }

  // set up parameters
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _convStrat.setVerbose(true);
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    _convStrat.setDebug(true);
  }

  _convStrat.setFromParams(_convStratParams);
  
  // set the grid in the ConvStratFinder object
  
  bool isLatLon = (_params.grid_projection == Params::PROJ_LATLON);
  _convStrat.setGrid(_interpCoord.nx, _interpCoord.ny,
                     _interpCoord.dx, _interpCoord.dy,
                     _interpCoord.minx, _interpCoord.miny,
                     _interpVlevels,
                     isLatLon);

  // set the terrain height array
  
  MdvxField terrainHtField(*_terrainHtField);
  terrainHtField.convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  const fl32 *terrainHt = (const fl32*) terrainHtField.getVol();
  fl32 htMiss = terrainHtField.getFieldHeader().missing_data_value;
  _convStrat.setTerrainHtField(terrainHt, htMiss);
  
  // set the temperature arrays
  
  MdvxField *tempFld =
    _modelInterpMdvx.getField(getModelInputName(Params::TEMP).c_str());
  if (tempFld == nullptr) {
    cerr << "ERROR - _computeConvStrat" << endl;
    cerr << "  Cannot find temp field in model: "
         << getModelInputName(Params::TEMP).c_str() << endl;
    return -1;
  }
  const fl32 *tempVol = (const fl32 *) tempFld->getVol();
  fl32 tempMiss = tempFld->getFieldHeader().missing_data_value;
  _convStrat.setTempBasedHts(tempVol, tempMiss);
  
  // get the dbz field for ConvStratFinder
  
  string dbzFieldName(getRadarInputName(Params::DBZ));
  BaseInterp::Field *dbzFld = _getInterpField(dbzFieldName);
  if (!dbzFld) {
    cerr << "ERROR - _computeConvStrat()" << endl;
    cerr << "  Cannot find dbz field: " << dbzFieldName << endl;
    cerr << "  conv/strat partition will not be computed" << endl;
    return -1;
  }
  fl32 *dbzVals = dbzFld->outputField.data();
  if (dbzVals == NULL) {
    cerr << "ERROR - _computeConvStrat()" << endl;
    cerr << "  Cannot find dbz data: " << dbzFieldName << endl;
    cerr << "  conv/strat partition will not be computed" << endl;
    return -1;
  }
  
  // compute the convective/stratiform partition
  
  if (_convStrat.computeEchoType(dbzVals, Radx::missingFl32)) {
    cerr << "ERROR - _computeConvStrat()" << endl;
    cerr << "  _convStrat.computePartition() failed" << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "DONE computing CONV-STRAT fields" << endl;
  }
  
  return 0;
  
}

////////////////////////////////////////
// add convective stratiform fields

void RadxCartDP::_addConvStratToOutput(OutputMdv &out)
  
{

  // vlevel for 2D fields
  
  vector<double> vlevel2D(1, 0.0);
  
  if (_params.conv_strat_write_partition) {

    // 3D
    
    out.createFieldAndAdd(_radarVol, _interpProj, _interpVlevels,
                          "EchoType3D",
                          "convective_stratiform_echo_type_3D",
                          "",
                          _convStrat.getMissingUi08(),
                          _convStrat.getEchoType3D());

    // 2D
    out.createFieldAndAdd(_radarVol, _interpProj, vlevel2D,
                          "EchoType2D",
                          "convective_stratiform_echo_type_2D",
                          "",
                          _convStrat.getMissingUi08(),
                          _convStrat.getEchoType2D());

  }
  
  if (_params.conv_strat_write_convectivity) {
    out.createFieldAndAdd(_radarVol, _interpProj, _interpVlevels,
                          "Convectivity3D",
                          "likelihood_of_convection_3D",
                          "",
                          _convStrat.getMissingFl32(),
                          _convStrat.getConvectivity3D());
  }

  if (_params.conv_strat_write_convective_dbz) {
    out.createFieldAndAdd(_radarVol, _interpProj, _interpVlevels,
                          "DbzConv",
                          "dbz_in_convection",
                          "dBZ",
                          _convStrat.getMissingFl32(),
                          _convStrat.getConvectiveDbz());
  }

  if (_params.conv_strat_write_echo_tops) {

    out.createFieldAndAdd(_radarVol, _interpProj, vlevel2D,
                          "ConvectiveTops",
                          "convective_tops",
                          "km",
                          _convStrat.getMissingFl32(),
                          _convStrat.getConvTopKm());

    char echoTopsLongName[1024];
    snprintf(echoTopsLongName, sizeof(echoTopsLongName),
             "echo_tops_at_dbz_%g", _convStrat.getDbzForEchoTops());
    
    out.createFieldAndAdd(_radarVol, _interpProj, vlevel2D,
                          "EchoTops",
                          echoTopsLongName,
                          "km",
                          _convStrat.getMissingFl32(),
                          _convStrat.getEchoTopKm());


  }

  if (_params.conv_strat_write_debug_fields) {

    // dbz texture
    
    out.createFieldAndAdd(_radarVol, _interpProj, _interpVlevels,
                          "DbzTexture3D",
                          "reflectivity_texture_3D",
                          "dBZ",
                          _convStrat.getMissingFl32(),
                          _convStrat.getTexture3D());

    // dbz col max
    
    out.createFieldAndAdd(_radarVol, _interpProj, vlevel2D,
                          "DbzComp",
                          "dbz_composite",
                          "dBZ",
                          _convStrat.getMissingFl32(),
                          _convStrat.getDbzColMax());

    // transition heights

    out.createFieldAndAdd(_radarVol, _interpProj, vlevel2D,
                          "ShallowHt",
                          "ht_of_transition_shallow_to_mid",
                          "km",
                          _convStrat.getMissingFl32(),
                          _convStrat.getShallowHtGrid());

    out.createFieldAndAdd(_radarVol, _interpProj, vlevel2D,
                          "DeepHt",
                          "ht_of_transition_mid_to_deep",
                          "km",
                          _convStrat.getMissingFl32(),
                          _convStrat.getDeepHtGrid());

  }
  
}

/////////////////////////////////////////////////////
// create grid template file
//
// We need to read in 1 radar file to ensure the grid
// location is correct.

int RadxCartDP::_createGridTemplate()
{

  if (_params.debug) {
    cerr << "INFO - creating grid template file" << endl;
  }

  // read in the last file in the radar data set
  
  RadxTimeList tlist;
  tlist.setDir(_params.radar_input_dir);
  tlist.setModeLast();
  if (tlist.compile()) {
    cerr << "ERROR - RadxCartDP::_createGridTemplate()" << endl;
    cerr << "  Cannot find radar files, dir: " << _params.radar_input_dir << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }
  
  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - RadxCartDP::_createGridTemplate()" << endl;
    cerr << "  No files found, dir: " << _params.radar_input_dir << endl;
    return -1;
  }
  
  // read in file

  _radarVol.clear();
  string radarPath = paths[0];
  if (_readFile(radarPath)) {
    cerr << "ERROR - RadxCartDP::_createGridTemplate()" << endl;
    cerr << "  Cannot read file: " << radarPath << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "SUCCESS - read in radar file: " << _radarVol.getPathInUse() << endl;
  }
  
  // initialize the projection and vlevels of the interp volume

  _initInterpGrid();

  // create the template object

  OutputMdv out(_progName, _params);
  out.setMasterHeader(_radarVol);

  size_t nptsVol = _interpCoord.nx * _interpCoord.ny * _interpCoord.nz;
  vector<ui08> vals(nptsVol, 1);
  fl32 missingVal = 0;
  
  out.createFieldAndAdd(_radarVol, _interpProj, _interpVlevels,
                        _params.template_3d_field_name,
                        "3D grid template for beam blockage",
                        "",
                        missingVal, vals.data());

  // write out file
  
  if (out.writeVol(_params.grid_template_dir)) {
    cerr << "ERROR - RadxCartDP::_createGridTemplate" << endl;
    cerr << "  Cannot write template file to dir: "
         << _params.grid_template_dir << endl;
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////
// write out MDV data

int RadxCartDP::_writeOutputMdv()
{

  if (_params.debug) {
    cerr << "  Writing output file and compressing ... " << endl;
  }

  // initialize
  
  OutputMdv out(_progName, _params);
  out.setMasterHeader(_radarVol);

  // add interp radar fields to output
  
  _radarInterp->addToOutputMdv(out);

  // add the model fields

  for (size_t ii = 0; ii < _modelInterpMdvx.getNFields(); ii++) {
    // check whether this should be written
    string fieldName = _modelInterpMdvx.getField(ii)->getFieldName();
    Params::model_field_t *modelField = getModelField(fieldName);
    if (modelField && modelField->do_write) {
      // make a copy of the field, since the mdvx object takes memory ownership
      MdvxField *ofld = new MdvxField(*_modelInterpMdvx.getField(ii));
      // rename
      ofld->setFieldName(modelField->output_name);
      // add to output object
      out.addField(ofld);
    }
  } // ii

  // add PID fields

  if (_pidField) {
    out.addField(_pidField);
    _pidField = nullptr; // memory handling passed to output mdv object
  }
  
  // add precip rate fields

  if (_rateZrField) {
    out.addField(_rateZrField);
    _rateZrField = nullptr; // memory handling passed to output mdv object
  }
  if (_rateHybridField) {
    out.addField(_rateHybridField);
    _rateHybridField = nullptr; // memory handling passed to output mdv object
  }

  // add QPE fields

  if (_qpeHybridField) {
    out.addField(_qpeHybridField);
    _qpeHybridField = nullptr; // memory handling passed to output mdv object
  }
  if (_qpeZrField) {
    out.addField(_qpeZrField);
    _qpeZrField = nullptr; // memory handling passed to output mdv object
  }

  // add beam blockage fields
  
  if (_haveBeamBlock) {
    if (_extinctionField) {
      // make copy since the Mdvx object takes ownership of the field
      MdvxField *eField = new MdvxField(*_extinctionField);
      eField->setFieldName(_params.beam_extinction_field_name);
      out.addField(eField);
    }
    if (_terrainHtField) {
      // make copy since the Mdvx object takes ownership of the field
      MdvxField *htField = new MdvxField(*_terrainHtField);
      htField->setFieldName(_params.terrain_ht_field_name);
      out.addField(htField);
    }
  }
  
  // add convective stratiform split to output
  
  if (_convStratAvailable) {
    _addConvStratToOutput(out);
  }

  // write out file
  
  if (out.writeVol(_params.output_dir)) {
    cerr << "ERROR - RadxCartDP::_writeOutputMdv" << endl;
    cerr << "  Cannot write file to output_dir: "
         << _params.output_dir << endl;
    return -1;
  }
  
  return 0;

}
  
//////////////////////////////////////////////////////////////////////////
// filter floating-point planes using the median in a 2D kernel

void RadxCartDP::_medianFilter2D(const fl32 *input,
                                 fl32 *output,
                                 size_t nz, size_t ny, size_t nx,
                                 int kernelSize,
                                 fl32 missingVal /* = Radx::missingFl32 */,
                                 bool copyEdges /* = true */)
{

  if (kernelSize < 1 || (kernelSize % 2) == 0) {
    throw std::runtime_error("_medianFilterFloatPlanes: kernelSize must be odd and >= 1");
  }

  const int radius = kernelSize / 2;
  const size_t nPoints = nz * ny * nx;

  // start with a straight copy so edges can remain unchanged if desired
  std::copy(input, input + nPoints, output);

  std::vector<fl32> vals;
  vals.reserve(kernelSize * kernelSize);

  for (size_t iz = 0; iz < nz; ++iz) {
    for (size_t iy = 0; iy < ny; ++iy) {
      for (size_t ix = 0; ix < nx; ++ix) {

        if (copyEdges) {
          if (iy < (size_t) radius || iy + radius >= ny ||
              ix < (size_t) radius || ix + radius >= nx) {
            continue;
          }
        }

        vals.clear();

        const int y0 = std::max<int>(0, (int) iy - radius);
        const int y1 = std::min<int>((int) ny - 1, (int) iy + radius);
        const int x0 = std::max<int>(0, (int) ix - radius);
        const int x1 = std::min<int>((int) nx - 1, (int) ix + radius);

        for (int jy = y0; jy <= y1; ++jy) {
          for (int jx = x0; jx <= x1; ++jx) {
            fl32 val = input[cartIndex(iz, jy, jx, ny, nx)];
            if (!std::isfinite(val) || val == missingVal) {
              continue;
            }
            vals.push_back(val);
          }
        }

        const size_t outIdx = cartIndex(iz, iy, ix, ny, nx);

        if (vals.empty()) {
          output[outIdx] = missingVal;
          continue;
        }

        const size_t midIndex = vals.size() / 2;
        std::nth_element(vals.begin(), vals.begin() + midIndex, vals.end());
        fl32 median = vals[midIndex];

        // Optional: for even number of samples, average the middle two.
        // For 3x3 and 5x5 kernels with full valid data this won't matter,
        // but it can matter near missing data.
        if ((vals.size() % 2) == 0) {
          std::nth_element(vals.begin(), vals.begin() + midIndex - 1, vals.end());
          median = (median + vals[midIndex - 1]) * 0.5f;
        }

        output[outIdx] = median;

      } // ix
    } // iy
  } // iz

}

//////////////////////////////////////////////////////////////////////////
// filter the PID planes using the mode in a kernel

void RadxCartDP::_modeFilterPid2D(const fl32 *input,
                                  fl32 *output,
                                  size_t nz, size_t ny, size_t nx,
                                  int kernelSize,
                                  fl32 missingVal /* = Radx::missingFl32 */,
                                  bool copyEdges /* = true */,
                                  bool preserveCenterOnTie /* = true */)
{

  if (kernelSize < 1 || (kernelSize % 2) == 0) {
    throw std::runtime_error("modeFilterPidPlanes: kernelSize must be odd and >= 1");
  }

  const int radius = kernelSize / 2;
  const size_t nPoints = nz * ny * nx;

  // Start with a straight copy so edges can remain unchanged if desired
  std::copy(input, input + nPoints, output);

  std::unordered_map<int, int> counts;
  counts.reserve(kernelSize * kernelSize);

  for (size_t iz = 0; iz < nz; ++iz) {
    for (size_t iy = 0; iy < ny; ++iy) {
      for (size_t ix = 0; ix < nx; ++ix) {

        if (copyEdges) {
          if (iy < (size_t) radius || iy + radius >= ny ||
              ix < (size_t) radius || ix + radius >= nx) {
            continue;
          }
        }
        
        counts.clear();
        
        const int y0 = std::max<int>(0, (int) iy - radius);
        const int y1 = std::min<int>((int) ny - 1, (int) iy + radius);
        const int x0 = std::max<int>(0, (int) ix - radius);
        const int x1 = std::min<int>((int) nx - 1, (int) ix + radius);
        
        for (int jy = y0; jy <= y1; ++jy) {
          for (int jx = x0; jx <= x1; ++jx) {
            fl32 val = input[cartIndex(iz, jy, jx, ny, nx)];
            if (!std::isfinite(val) || val == missingVal) {
              continue;
            }

            // PID classes are stored as float but represent integer labels
            int cls = static_cast<int>(std::lround(val));
            counts[cls]++;
          }
        }

        const size_t outIdx = cartIndex(iz, iy, ix, ny, nx);

        if (counts.empty()) {
          output[outIdx] = missingVal;
          continue;
        }

        int bestClass = 0;
        int bestCount = -1;
        bool tie = false;

        for (const auto &entry : counts) {
          if (entry.second > bestCount) {
            bestClass = entry.first;
            bestCount = entry.second;
            tie = false;
          } else if (entry.second == bestCount) {
            tie = true;
          }
        }

        if (tie && preserveCenterOnTie) {
          fl32 centerVal = input[outIdx];
          if (std::isfinite(centerVal) && centerVal != missingVal) {
            int centerClass = static_cast<int>(std::lround(centerVal));
            auto it = counts.find(centerClass);
            if (it != counts.end() && it->second == bestCount) {
              output[outIdx] = centerVal;
              continue;
            }
          }
        }

        output[outIdx] = static_cast<fl32>(bestClass);

      } // ix
    } // iy
  } // iz

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
  
  if (strstr(_params.RATE_params_file_path, "use-defaults") == nullptr) {
    // not using defaults
    if (_precipRateParams.load(_params.RATE_params_file_path,
                               nullptr, expandEnvVars, _args.tdrpDebug)) {
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
    cerr << "Reading PID params from file: "
         << _params.PID_params_file_path << endl;
  }

  // do we need to expand environment variables?

  bool expandEnvVars = false;
  if (_args.printParamsPidMode.find("expand") != string::npos) {
    expandEnvVars = true;
  }

  // read in PID params if applicable

  if (strstr(_params.PID_params_file_path, "use-defaults") == nullptr) {
    // not using defaults
    if (_ncarPidParams.load(_params.PID_params_file_path,
                            nullptr, expandEnvVars, _args.tdrpDebug)) {
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
    cerr << "Reading KDP params from file: "
         << _params.KDP_params_file_path << endl;
  }

  // do we need to expand environment variables?

  bool expandEnvVars = false;
  if (_args.printParamsKdpMode.find("expand") != string::npos) {
    expandEnvVars = true;
  }

  // read in KDP params if applicable

  if (strstr(_params.KDP_params_file_path, "use-defaults") == nullptr) {
    // not using defaults
    if (_kdpFiltParams.load(_params.KDP_params_file_path,
                            nullptr, expandEnvVars, _args.tdrpDebug)) {
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

////////////////////////////////////////////////////
// Print params for Convective/Stratiform partition

void RadxCartDP::_printParamsConvStrat()
{

  if (_params.debug) {
    cerr << "Reading Convective/Stratiform params from file: "
         << _params.conv_strat_params_file_path << endl;
  }

  // do we need to expand environment variables?
  
  bool expandEnvVars = false;
  if (_args.printParamsConvStratMode.find("expand") != string::npos) {
    expandEnvVars = true;
  }

  // read in ConvStrat params if applicable

  if (strstr(_params.conv_strat_params_file_path, "use-defaults") == nullptr) {
    // not using defaults
    if (_convStratParams.load(_params.conv_strat_params_file_path,
                              nullptr, expandEnvVars, _args.tdrpDebug)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Cannot read params file for ConvStrat: "
           << _params.conv_strat_params_file_path << endl;
      OK = false;
      return;
    }
  }
  
  // set print mode
  
  tdrp_print_mode_t printMode = PRINT_LONG;
  if (_args.printParamsConvStratMode.find("short") == 0) {
    printMode = PRINT_SHORT;
  } else if (_args.printParamsConvStratMode.find("norm") == 0) {
    printMode = PRINT_NORM;
  } else if (_args.printParamsConvStratMode.find("verbose") == 0) {
    printMode = PRINT_VERBOSE;
  }

  // do the print to stdout
  
  _convStratParams.print(stdout, printMode);

}

//////////////////////////////////////////////////////
// initialize radar field types array

void RadxCartDP::_initRadarFieldTypes()
{

  // initialize field types
  
  _radarFieldTypes.push_back(Params::DBZ);
  _radarFieldTypes.push_back(Params::VEL);
  _radarFieldTypes.push_back(Params::WIDTH);
  _radarFieldTypes.push_back(Params::SNR);
  _radarFieldTypes.push_back(Params::ZDR);
  _radarFieldTypes.push_back(Params::PHIDP);
  _radarFieldTypes.push_back(Params::RHOHV);
  _radarFieldTypes.push_back(Params::LDR);

}

//////////////////////////////////////////////////////
// check all radar fields are present in params file
// returns 0 on success, -1 on failure

int RadxCartDP::_checkRadarFields()
{
  int iret = 0;
  for (size_t jj = 0; jj < _radarFieldTypes.size(); jj++) {
    Params::radar_field_type_t rfType = _radarFieldTypes[jj];
    int count = 0;
    for (int ii = 0; ii < _params.radar_fields_n; ii++) {
      if (_params._radar_fields[ii].field_type == rfType) {
        count++;
      }
    } // ii
    if (count == 0) {
      cerr << "ERROR - missing radar field type: "
           << _radarFieldType2Str(_radarFieldTypes[jj]) << endl;
      iret = -1;
    } else if (count > 1) {
      cerr << "ERROR - duplicate radar field type: "
           << _radarFieldType2Str(_radarFieldTypes[jj]) << endl;
      iret = -1;
    }
  } // jj
  return iret;
}

//////////////////////////////////////////////////
// radar field type to string

string RadxCartDP::_radarFieldType2Str(Params::radar_field_type_t rftype)
{

  switch (rftype) {
    case Params::DBZ:
      return "DBZ";
    case Params::VEL:
      return "VEL";
    case Params::WIDTH:
      return "WIDTH";
    case Params::SNR:
      return "SNR";
    case Params::ZDR:
      return "ZDR";
    case Params::PHIDP:
      return "PHIDP";
    case Params::RHOHV:
      return "RHOHV";
    case Params::LDR:
      return "LDR";
    default:
      return "";
  }
  
}

//////////////////////////////////////////////////
// get radar field from type
// returns null on error
// NOTE: error cannot happen if _checkRadarFields() succeeded

Params::radar_field_t *RadxCartDP::getRadarField(Params::radar_field_type_t rftype)
{
  for (int ii = 0; ii < _params.radar_fields_n; ii++) {
    if (_params._radar_fields[ii].field_type == rftype) {
      return &_params._radar_fields[ii];
    }
  }
  // not found
  return nullptr;
}

//////////////////////////////////////////////////
// get radar field from name
// returns null on error
// NOTE: error cannot happen if _checkRadarFields() succeeded

Params::radar_field_t *RadxCartDP::getRadarField(const string &fieldName)
{
  for (int ii = 0; ii < _params.radar_fields_n; ii++) {
    if (fieldName == std::string(_params._radar_fields[ii].input_name)) {
      return &_params._radar_fields[ii];
    }
  }
  // not found
  return nullptr;
}

//////////////////////////////////////////////////
// get radar field names from type

string RadxCartDP::getRadarInputName(Params::radar_field_type_t rftype)
{
  for (int ii = 0; ii < _params.radar_fields_n; ii++) {
    if (_params._radar_fields[ii].field_type == rftype) {
      return _params._radar_fields[ii].input_name;
    }
  }
  // not found
  return "";
}

string RadxCartDP::getRadarOutputName(Params::radar_field_type_t rftype)
{
  for (int ii = 0; ii < _params.radar_fields_n; ii++) {
    if (_params._radar_fields[ii].field_type == rftype) {
      return _params._radar_fields[ii].output_name;
    }
  }
  // not found
  return "";
}

//////////////////////////////////////////////////
// get model field from type
// returns null on error
// NOTE: error cannot happen if _checkModelFields() succeeded

Params::model_field_t *RadxCartDP::getModelField(Params::model_field_type_t rftype)
{
  for (int ii = 0; ii < _params.model_fields_n; ii++) {
    if (_params._model_fields[ii].field_type == rftype) {
      return &_params._model_fields[ii];
    }
  }
  // not found
  return nullptr;
}

//////////////////////////////////////////////////
// get model field from name
// returns null on error
// NOTE: error cannot happen if _checkModelFields() succeeded

Params::model_field_t *RadxCartDP::getModelField(const string &fieldName)
{
  for (int ii = 0; ii < _params.model_fields_n; ii++) {
    if (fieldName == std::string(_params._model_fields[ii].input_name)) {
      return &_params._model_fields[ii];
    }
  }
  // not found
  return nullptr;
}

//////////////////////////////////////////////////////
// initialize model field types array

void RadxCartDP::_initModelFieldTypes()
{

  // initialize field types
  
  _modelFieldTypes.push_back(Params::TEMP);
  _modelFieldTypes.push_back(Params::RH);
  _modelFieldTypes.push_back(Params::UVEL);
  _modelFieldTypes.push_back(Params::VVEL);
  _modelFieldTypes.push_back(Params::WVEL);

}

//////////////////////////////////////////////////////
// check all model fields are present in params file
// returns 0 on success, -1 on failure

int RadxCartDP::_checkModelFields()
{
  int iret = 0;
  for (size_t jj = 0; jj < _modelFieldTypes.size(); jj++) {
    Params::model_field_type_t mftype = _modelFieldTypes[jj];
    int count = 0;
    for (int ii = 0; ii < _params.model_fields_n; ii++) {
      if (_params._model_fields[ii].field_type == mftype) {
        count++;
      }
    } // ii
    if (count == 0) {
      cerr << "ERROR - missing model field type: "
           << _modelFieldType2Str(_modelFieldTypes[jj]) << endl;
      iret = -1;
    } else if (count > 1) {
      cerr << "ERROR - duplicate model field type: "
           << _modelFieldType2Str(_modelFieldTypes[jj]) << endl;
      iret = -1;
    }
  } // jj
  return iret;
}

//////////////////////////////////////////////////
// model field type to string

string RadxCartDP::_modelFieldType2Str(Params::model_field_type_t mftype)
{

  switch (mftype) {
    case Params::TEMP:
      return "TEMP";
    case Params::RH:
      return "RH";
    case Params::UVEL:
      return "UVEL";
    case Params::VVEL:
      return "VVEL";
    case Params::WVEL:
      return "WVEL";
    default:
      return "";
  }
  
}

//////////////////////////////////////////////////
// get model field name from type

string RadxCartDP::getModelInputName(Params::model_field_type_t mftype)
{
  for (int ii = 0; ii < _params.model_fields_n; ii++) {
    if (_params._model_fields[ii].field_type == mftype) {
      return _params._model_fields[ii].input_name;
    }
  }
  // not found
  return "";
}

string RadxCartDP::getModelOutputName(Params::model_field_type_t mftype)
{
  for (int ii = 0; ii < _params.model_fields_n; ii++) {
    if (_params._model_fields[ii].field_type == mftype) {
      return _params._model_fields[ii].output_name;
    }
  }
  // not found
  return "";
}

//////////////////////////////////////////////////
// get model type from input name

Params::model_field_type_t RadxCartDP::getModelTypeFromInputName(const string name)
{
  for (int ii = 0; ii < _params.model_fields_n; ii++) {
    string inputName = _params._model_fields[ii].input_name;
    if (name == inputName) {
      return _params._model_fields[ii].field_type;
    }
  }
  // not found, assume temp
  return Params::MODEL_NOT_SET;
}

//////////////////////////////////////////////////
// get model type from output name

Params::model_field_type_t RadxCartDP::getModelTypeFromOutputName(const string name)
{
  for (int ii = 0; ii < _params.model_fields_n; ii++) {
    string outputName = _params._model_fields[ii].output_name;
    if (name == outputName) {
      return _params._model_fields[ii].field_type;
    }
  }
  // not found, assume temp
  return Params::MODEL_NOT_SET;
}

