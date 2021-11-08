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
// Pid2Grid.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2012
//
///////////////////////////////////////////////////////////////
//
// Pid2Grid reads moments from Radx-supported format files,
// interpolates onto a Cartesian grid, and writes out the
// results to Cartesian files in MDV or NetCDF.
//
///////////////////////////////////////////////////////////////

#include "Pid2Grid.hh"
#include "OutputMdv.hh"
#include "PolarCompute.hh"
#include "PolarThread.hh"
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/pjg.h>
#include <toolsa/mem.h>
#include <toolsa/toolsa_macros.h>
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

// initialize extra field names

string Pid2Grid::smoothedDbzFieldName = "DBZ_SMOOTHED";
string Pid2Grid::smoothedRhohvFieldName = "RHOHV_SMOOTHED";
string Pid2Grid::elevationFieldName = "ELEV";
string Pid2Grid::rangeFieldName = "RANGE";
string Pid2Grid::beamHtFieldName = "BEAM_HT";
string Pid2Grid::tempFieldName = "TEMPC";
string Pid2Grid::pidFieldName = "PID";
string Pid2Grid::pidInterestFieldName = "PID_INTEREST";
string Pid2Grid::mlFieldName = "MELTING_LAYER";
string Pid2Grid::mlExtendedFieldName = "ML_EXTENDED";
string Pid2Grid::convFlagFieldName = "CONV_FLAG";

// Constructor

Pid2Grid::Pid2Grid(int argc, char **argv)
  
{

  OK = TRUE;

  _cartInterp = NULL;

  // set programe name

  _progName = "Pid2Grid";
  ucopyright((char *) _progName.c_str());
  
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
  
  // if requested, print params for RATE then exit
  
  if (_args.printParamsRate) {
    _printParamsRate();
    exit(0);
  }
  
  // if requested, print params for PID then exit
  
  if (_args.printParamsPid) {
    _printParamsPid();
    exit(0);
  }
  
  // if requested, print params for KDP then exit
  
  if (_args.printParamsKdp) {
    _printParamsKdp();
    exit(0);
  }

  // read params for Ncar PID
  
  if (strstr(_params.PID_params_file_path, "use-defaults") == NULL) {
    // not using defaults
    if (_ncarPidParams.load(_params.PID_params_file_path,
                            NULL, true, _args.tdrpDebug)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Cannot read params file for NcarPid: "
           << _params.PID_params_file_path << endl;
      OK = FALSE;
      return;
    }
  }
  
  // read params for KdpFilt
  
  if (strstr(_params.KDP_params_file_path, "use-defaults") == NULL) {
    // not using defaults
    if (_kdpFiltParams.load(_params.KDP_params_file_path,
                            NULL, true, _args.tdrpDebug)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Cannot read params file for KdpFilt: "
           << _params.KDP_params_file_path << endl;
      OK = FALSE;
      return;
    }
  }

  // read in RATE params
  
  if (strstr(_params.RATE_params_file_path, "use-defaults") == NULL) {
    // not using defaults
    if (_precipRateParams.load(_params.RATE_params_file_path,
                               NULL, true, _args.tdrpDebug)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Cannot read params file for PrecipFilt: "
           << _params.RATE_params_file_path << endl;
      OK = FALSE;
      return;
    }
  }

  // initialize compute object
  
  pthread_mutex_init(&_debugPrintMutex, NULL);
  
  // set up compute thread pool
  
  for (int ii = 0; ii < _params.n_compute_threads; ii++) {
    PolarThread *thread =
      new PolarThread(this, _params, 
                      _kdpFiltParams,
                      _ncarPidParams,
                      _precipRateParams,
                      ii);
    if (!thread->OK) {
      delete thread;
      OK = FALSE;
      return;
    }
    _threadPool.addThreadToMain(thread);
    _computeThreads.push_back(thread->getPolarCompute());
  }

}

//////////////////////////////////////
// destructor

Pid2Grid::~Pid2Grid()

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

int Pid2Grid::Run()
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

int Pid2Grid::_runFilelist()
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

int Pid2Grid::_runArchive()
{

  // get start and end times

  time_t startTime = RadxTime::parseDateTime(_params.start_time);
  if (startTime == RadxTime::NEVER) {
    cerr << "ERROR - Pid2Grid::_runArchive()" << endl;
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
    cerr << "ERROR - Pid2Grid::_runArchive()" << endl;
    cerr << "  End time format incorrect: " << _params.end_time << endl;
    if (_args.endTimeSet) {
      cerr << "  Check command line" << endl;
    } else {
      cerr << "  Check params file: " << _paramsPath << endl;
    }
    return -1;
  }

  if (_params.debug) {
    cerr << "Running Pid2Grid in ARCHIVE mode" << endl;
    cerr << "  Input dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(endTime) << endl;
  }

  // get the files to be processed
  
  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(startTime, endTime);
  if (tlist.compile()) {
    cerr << "ERROR - Pid2Grid::_runArchive()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - Pid2Grid::_runArchive()" << endl;
    cerr << "  No files found, dir: " << _params.input_dir << endl;
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

int Pid2Grid::_runRealtime()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(),
                _params.instance,
                _params.procmap_register_interval);
  PMU_auto_register("Init realtime mode");

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

int Pid2Grid::_processFile(const string &filePath)
{

  PMU_auto_register("Processing file");

  // check file name
  
  if (strlen(_params.input_file_search_ext) > 0) {
    RadxPath rpath(filePath);
    if (strcmp(rpath.getExt().c_str(), _params.input_file_search_ext)) {
      if (_params.debug) {
        cerr << "WARNING - ignoring file: " << filePath << endl;
        cerr << "  Does not have correct extension: "
             << _params.input_file_search_ext << endl;
      }
      return 0;
    }
  }
  
  if (strlen(_params.input_file_search_substr) > 0) {
    RadxPath rpath(filePath);
    if (rpath.getFile().find(_params.input_file_search_substr)
        == string::npos) {
      if (_params.debug) {
        cerr << "WARNING - ignoring file: " << filePath << endl;
        cerr << "  Does not contain required substr: "
             << _params.input_file_search_substr << endl;
      }
      return 0;
    }
  }

  // check we have not already processed this file
  // in the file aggregation step

  RadxPath thisPath(filePath);
  for (size_t ipath = 0; ipath < _readPaths.size(); ipath++) {
    RadxPath rpath(_readPaths[ipath]);
    if (thisPath.getFile() == rpath.getFile()) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Skipping file: " << filePath << endl;
        cerr << "  Previously processed in aggregation step" << endl;
      }
      return 0;
    }
  }
  
  if (_params.debug) {
    cerr << "INFO - Pid2Grid::_processFile" << endl;
    cerr << "  Input file path: " << filePath << endl;
    cerr << "  Reading in file ..." << endl;
  }
  
  // ensure memory is freed up
  
  _readVol.clear();
  _freeInterpRays();

  // read in file
  
  if (_readFile(filePath)) {
    cerr << "ERROR - Pid2Grid::_processFile" << endl;
    return -1;
  }

  // check we have at least 2 rays
  
  if (_readVol.getNRays() < 2) {
    cerr << "ERROR - Pid2Grid::_processFile" << endl;
    cerr << "  Too few rays: " << _readVol.getNRays() << endl;
    return -1;
  }
  
  // make sure gate geom is constant

  _readVol.remapToFinestGeom();

  // read the temperature profile into the threads

  if (_computeThreads.size() > 0) {
    // thread 0
    PolarCompute *thread0 = _computeThreads[0];
    if (_params.debug) {
      cerr << "Thread #: " << 0 << endl;
      cerr << "  Loading temp profile for time: "
           << RadxTime::strm(_readVol.getStartTimeSecs()) << endl;
    }
    thread0->loadTempProfile(_readVol.getStartTimeSecs());
    // copy to other threads
    for (size_t ii = 1; ii < _computeThreads.size(); ii++) {
      _computeThreads[ii]->setTempProfile(thread0->getTempProfile());
    }
  }
  
  // add geometry and pid fields to the volume
  
  _addExtraFieldsToInput();

  // compute the derived fields
  
  if (_computeDerived()) {
    cerr << "ERROR - Pid2Grid::Run" << endl;
    cerr << "  Cannot compute derived fields" << endl;
    return -1;
  }

  // interpolate and write out
  
  if (_rhiMode) {
    if (_params.debug) {
      cerr << "  NOTE: data is in RHI mode" << endl;
    }
    _allocCartInterp();
    _cartInterp->setRhiMode(true);
    _cartInterp->interpVol();
  } else {
    _allocCartInterp();
    _cartInterp->setRhiMode(false);
    _cartInterp->interpVol();
  }

  // free up

  if (_params.free_memory_between_files) {
    _readVol.clear();
    _freeInterpRays();
  }
  
  return 0;

}

//////////////////////////////////////////////////
// Read in a RADX file
// Returns 0 on success, -1 on failure

int Pid2Grid::_readFile(const string &filePath)
{

  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  if (inFile.readFromPath(filePath, _readVol)) {
    cerr << "ERROR - Pid2Grid::_readFile" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  _readPaths = inFile.getReadPaths();
  
  // convert to fl32
  
  _readVol.convertToFl32();

  // pad out the gates to the longest range
  
  _readVol.setNGatesConstant();
  
  //  check for rhi
  
  _rhiMode = _isRhi();

  // rename fields if requested

  if (_params.rename_fields_on_input) {
    for (int ii = 0; ii < _params.renamed_fields_n; ii++) {
      string inputName = _params._renamed_fields[ii].input_name;
      string outputName = _params._renamed_fields[ii].output_name;
      _readVol.renameField(inputName, outputName);
    } // ii
  } // if (_params.rename_fields_on_input)

  // override radar location if requested

  if (_params.override_radar_location) {
    _readVol.overrideLocation(_params.radar_latitude_deg,
                              _params.radar_longitude_deg,
                              _params.radar_altitude_meters / 1000.0);
  }

  // override radar name and site name if requested
  
  if (_params.override_instrument_name) {
    _readVol.setInstrumentName(_params.instrument_name);
  }
  if (_params.override_site_name) {
    _readVol.setSiteName(_params.site_name);
  }
    
  // override beam width if requested
  
  if (_params.override_beam_width) {
    _readVol.setRadarBeamWidthDegH(_params.beam_width_deg_h);
    _readVol.setRadarBeamWidthDegV(_params.beam_width_deg_v);
  }

  // override gate geometry if requested
  
  if (_params.override_gate_geometry) {
    _readVol.setRangeGeom(_params.start_range_km, _params.gate_spacing_km);
  }

  // override fixed angle if required

  if (_params.override_fixed_angle_with_mean_measured_angle) {
    _readVol.computeFixedAnglesFromRays();
  }

  // reorder sweeps into ascending order if requested

  if (_params.reorder_sweeps_by_ascending_angle) {
    _readVol.reorderSweepsAscendingAngle();
  }

  // trim surveillance sweeps to 360 degrees if requested

  if (_params.trim_surveillance_sweeps_to_360deg) {
    _readVol.trimSurveillanceSweepsTo360Deg();
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  _startRangeKm: " << _readVol.getStartRangeKm() << endl;
    cerr << "  _gateSpacingKm: " << _readVol.getGateSpacingKm() << endl;
  }

  // add extra fields fields
  
  _addGeometryFields();
  _addTimeField();
  
  // set up interp fields
  
  _initInterpFields();
  
  // load up the input ray data vector

  _loadInterpRays();

  // check all fields are present
  // set standard names etc

  _checkFields(filePath);

  // set radar properties

  _radarHtKm = _readVol.getAltitudeKm();
  _wavelengthM = _readVol.getWavelengthM();

  return 0;

}

//////////////////////////////////////////////////
// set up read

void Pid2Grid::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }

  if (_params.remove_long_range_rays) {
    file.setReadRemoveLongRange(true);
  } else {
    file.setReadRemoveLongRange(false);
  }

  if (_params.remove_short_range_rays) {
    file.setReadRemoveShortRange(true);
  } else {
    file.setReadRemoveShortRange(false);
  }

  if (_params.compute_sweep_angles_from_vcp_tables) {
    file.setReadComputeSweepAnglesFromVcpTables(true);
  } else {
    file.setReadComputeSweepAnglesFromVcpTables(false);
  }

  if (_params.set_max_range) {
    file.setReadMaxRangeKm(_params.max_range_km);
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
// check all fields are present
// set standard names etc

void Pid2Grid::_checkFields(const string &filePath)
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
      cerr << "  File: " << filePath << endl;
    }
  } // ifield

}

//////////////////////////////////////////////////
// add geometry and pid fields to the volume

void Pid2Grid::_addExtraFieldsToInput()

{

  vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {

    RadxRay *ray = rays[iray];

    RadxField *smoothDbzFld = new RadxField(smoothedDbzFieldName, "dBZ");
    RadxField *smoothRhohvFld = new RadxField(smoothedRhohvFieldName, "");
    RadxField *elevFld = new RadxField(elevationFieldName, "deg");
    RadxField *rangeFld = new RadxField(rangeFieldName, "km");
    RadxField *beamHtFld = new RadxField(beamHtFieldName, "km");
    RadxField *tempFld = new RadxField(tempFieldName, "C");
    RadxField *pidFld = new RadxField(pidFieldName, "");
    RadxField *pidIntrFld = new RadxField(pidInterestFieldName, "");
    RadxField *mlFld = new RadxField(mlFieldName, "");

    size_t nGates = ray->getNGates();

    TaArray<Radx::fl32> elev_, rng_, ht_, temp_;
    Radx::fl32 *elev = elev_.alloc(nGates);
    Radx::fl32 *rng = rng_.alloc(nGates);
    Radx::fl32 *ht = ht_.alloc(nGates);
    Radx::fl32 *temp = temp_.alloc(nGates);
    
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
    const TempProfile &profile = _computeThreads[0]->getTempProfile();
    for (size_t igate = 0; igate < nGates; igate++, rangeKm += gateSpacingKm) {
      double htKm = beamHt.computeHtKm(elevationDeg, rangeKm);
      double tempC = profile.getTempForHtKm(htKm);
      elev[igate] = elevationDeg;
      rng[igate] = rangeKm;
      ht[igate] = htKm;
      temp[igate] = tempC;
    } // igate

    smoothDbzFld->setTypeFl32(-9999.0);
    smoothRhohvFld->setTypeFl32(-9999.0);
    elevFld->setTypeFl32(-9999.0);
    rangeFld->setTypeFl32(-9999.0);
    beamHtFld->setTypeFl32(-9999.0);
    tempFld->setTypeFl32(-9999.0);
    pidFld->setTypeSi32(-9999, 1.0, 0.0);
    pidIntrFld->setTypeFl32(-9999.0);
    mlFld->setTypeSi32(-9999, 1.0, 0.0);
    
    smoothDbzFld->addDataMissing(nGates);
    smoothRhohvFld->addDataMissing(nGates);
    elevFld->addDataFl32(nGates, elev);
    rangeFld->addDataFl32(nGates, rng);
    beamHtFld->addDataFl32(nGates, ht);
    tempFld->addDataFl32(nGates, temp);
    pidFld->addDataMissing(nGates);
    pidIntrFld->addDataMissing(nGates);
    mlFld->addDataMissing(nGates);

    ray->addField(smoothDbzFld);
    ray->addField(smoothRhohvFld);
    ray->addField(elevFld);
    ray->addField(rangeFld);
    ray->addField(beamHtFld);
    ray->addField(tempFld);
    ray->addField(pidFld);
    ray->addField(pidIntrFld);
    ray->addField(mlFld);

  } // iray

}

//////////////////////////////////////////////////
// add extra output fields

void Pid2Grid::_addExtraFieldsToOutput()

{

  // loop through rays

  vector<RadxRay *> &inputRays = _readVol.getRays();
  for (size_t iray = 0; iray < inputRays.size(); iray++) {
    
    RadxRay *inputRay = inputRays[iray];

    // match output ray to input ray based on time
    
    RadxRay *outputRay = NULL;
    double inTime = inputRay->getTimeDouble();
    for (size_t jj = 0; jj < _derivedRays.size(); jj++) {
      RadxRay *derivedRay = _derivedRays[jj];
      double outTime = derivedRay->getTimeDouble();
      if (fabs(inTime - outTime) < 0.001) {
        outputRay = derivedRay;
        break;
      }
    } // jj

    if (outputRay != NULL) {
      // make a copy of the input fields
      RadxField *mlFld = new RadxField(*inputRay->getField(mlFieldName));
      // add to output
      outputRay->addField(mlFld);
    }

  } // iray

}

/////////////////////////////////////////////////////
// compute the derived fields for all rays in volume

int Pid2Grid::_computeDerived()
{

  // initialize the volume with ray numbers
  
  _readVol.setRayNumbersInOrder();

  // initialize derived

  _derivedRays.clear();
  
  // loop through the input rays,
  // computing the derived fields

  const vector<RadxRay *> &inputRays = _readVol.getRays();
  for (size_t iray = 0; iray < inputRays.size(); iray++) {

    // get a thread from the pool
    bool isDone = true;
    PolarThread *thread = 
      (PolarThread *) _threadPool.getNextThread(true, isDone);
    if (thread == NULL) {
      break;
    }
    if (isDone) {
      // store the results computed by the thread
      if (_storeDerivedRay(thread)) {
        cerr << "ERROR - RadxRate::_compute()" << endl;
        cerr << "  Cannot compute for ray num: " << iray << endl;
        break;
      }
      // return thread to the available pool
      _threadPool.addThreadToAvail(thread);
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

  _threadPool.setReadyForDoneCheck();
  while (!_threadPool.checkAllDone()) {
    PolarThread *thread = (PolarThread *) _threadPool.getNextDoneThread();
    if (thread == NULL) {
      break;
    } else {
      // store the results computed by the thread
      _storeDerivedRay(thread);
      // return thread to the available pool
      _threadPool.addThreadToAvail(thread);
    }
  } // while

  return 0;

}

///////////////////////////////////////////////////////////
// Store the derived ray

int Pid2Grid::_storeDerivedRay(PolarThread *thread)

{
  
  RadxRay *derivedRay = thread->getOutputRay();
  if (derivedRay == NULL) {
    // error
    return -1;
  }

  // good return, add to results
  _derivedRays.push_back(derivedRay);
  
  return 0;

}
      
//////////////////////////////////////////////////
// load up the input ray data vector

void Pid2Grid::_loadInterpRays()
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
      
      Interp::Ray *interpRay = 
        new Interp::Ray(rays[iray],
                        isweep,
                        _interpFields,
                        _params.use_fixed_angle_for_interpolation,
                        _params.use_fixed_angle_for_data_limits);
      _interpRays.push_back(interpRay);

    } // iray

  } // isweep


}
  
//////////////////////////////////////////////////
// add geometry fields

void Pid2Grid::_addGeometryFields()
{
  
  if (_params.output_angle_fields) {

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

  } // if(_params.output_angle_fields)
  
  // beamHeight

  BeamHeight beamHt;
  beamHt.setInstrumentHtKm(_readVol.getAltitudeKm());
  if (_params.override_standard_pseudo_earth_radius) {
    beamHt.setPseudoRadiusRatio(_params.pseudo_earth_radius_ratio);
  }

  // other geom fields
  
  vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    RadxRay *ray = rays[iray];
    int nGates = ray->getNGates();
    TaArray<Radx::fl32> data_;
    Radx::fl32 *data = data_.alloc(nGates);
      
    // range field
    
    if (_params.output_range_field) {
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
    
    // height field
    
    if (_params.output_height_field) {
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

    // coverage field
    if (_params.output_coverage_field) {
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
    }
    
  } // iray

}

//////////////////////////////////////////////////
// add time field

void Pid2Grid::_addTimeField()
{

  if (!_params.output_time_field) {
    return;
  }
  
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

bool Pid2Grid::_isRhi()
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

//////////////////////////////////////////////////
// initialize the fields for interpolation

void Pid2Grid::_initInterpFields()
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
        Interp::Field interpField;
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

void Pid2Grid::_allocCartInterp()
{
  if (_cartInterp == NULL) {
    _cartInterp = new CartInterp(_progName, _params, _readVol,
                                 _interpFields, _interpRays);
  }
}

////////////////////////////////////////////////////////////
// Free up input rays

void Pid2Grid::_freeInterpRays()
  
{
  for (size_t ii = 0; ii < _interpRays.size(); ii++) {
    delete _interpRays[ii];
  }
  _interpRays.clear();
}

//////////////////////////////////////////////////
// Print params for RATE

void Pid2Grid::_printParamsRate()
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
      OK = FALSE;
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

void Pid2Grid::_printParamsPid()
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
      OK = FALSE;
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

void Pid2Grid::_printParamsKdp()
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
      OK = FALSE;
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

