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
// Radx2Grid.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2012
//
///////////////////////////////////////////////////////////////
//
// Radx2Grid reads moments from Radx-supported format files,
// interpolates onto a Cartesian grid, and writes out the
// results to Cartesian files in MDV or NetCDF.
//
///////////////////////////////////////////////////////////////

#include "Radx2Grid.hh"
#include "OutputMdv.hh"
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

// Constructor

Radx2Grid::Radx2Grid(int argc, char **argv)
  
{

  OK = TRUE;

  _cartInterp = NULL;
  _ppiInterp = NULL;
  _polarInterp = NULL;
  _reorderInterp = NULL;
  _satInterp = NULL;
  _nWarnCensorPrint = 0;

  // set programe name

  _progName = "Radx2Grid";
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

  // volume number

  if (_params.override_volume_number ||
      _params.autoincrement_volume_number) {
    _volNum = _params.starting_volume_number;
  }

}

//////////////////////////////////////
// destructor

Radx2Grid::~Radx2Grid()

{

  // free up

  _freeInterpRays();
  if (_cartInterp) {
    delete _cartInterp;
  }
  if (_ppiInterp) {
    delete _ppiInterp;
  }
  if (_polarInterp) {
    delete _polarInterp;
  }
  if (_reorderInterp) {
    delete _reorderInterp;
  }
  if (_satInterp) {
    delete _satInterp;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Radx2Grid::Run()
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

int Radx2Grid::_runFilelist()
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

int Radx2Grid::_runArchive()
{

  // get start and end times

  time_t startTime = RadxTime::parseDateTime(_params.start_time);
  if (startTime == RadxTime::NEVER) {
    cerr << "ERROR - Radx2Grid::_runArchive()" << endl;
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
    cerr << "ERROR - Radx2Grid::_runArchive()" << endl;
    cerr << "  End time format incorrect: " << _params.end_time << endl;
    if (_args.endTimeSet) {
      cerr << "  Check command line" << endl;
    } else {
      cerr << "  Check params file: " << _paramsPath << endl;
    }
    return -1;
  }

  if (_params.debug) {
    cerr << "Running Radx2Grid in ARCHIVE mode" << endl;
    cerr << "  Input dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(endTime) << endl;
  }

  // get the files to be processed
  
  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(startTime, endTime);
  if (_params.aggregate_sweep_files_on_read) {
    tlist.setReadAggregateSweeps(true);
  }
  if (tlist.compile()) {
    cerr << "ERROR - Radx2Grid::_runArchive()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - Radx2Grid::_runArchive()" << endl;
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

int Radx2Grid::_runRealtime()
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

int Radx2Grid::_processFile(const string &filePath)
{

  PMU_auto_register("Processing file");

  // ensure memory is freed up
  
  _readVol.clear();
  _freeInterpRays();

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
    cerr << "INFO - Radx2Grid::_processFile" << endl;
    cerr << "  Input file path: " << filePath << endl;
    cerr << "  Reading in file ..." << endl;
  }
  
  // read in file
  
  if (_readFile(filePath)) {
    cerr << "ERROR - Radx2Grid::_processFile" << endl;
    return -1;
  }

  // check we have at least 2 rays

  if (_readVol.getNRays() < 2) {
    cerr << "ERROR - Radx2Grid::_processFile" << endl;
    cerr << "  Too few rays: " << _readVol.getNRays() << endl;
    return -1;
  }

  if (_params.check_number_of_sweeps) {
    if (_readVol.getNSweeps() < _params.min_number_of_sweeps) {
      cerr << "ERROR - Radx2Grid::_processFile" << endl;
      cerr << "  Too few sweeps: " << _readVol.getNSweeps() << endl;
      cerr << "  Min valid: " << _params.min_number_of_sweeps << endl;
      return -1;
    }
  }

  // make sure gate geom is constant

  _readVol.remapToFinestGeom();

  // interpolate and write out
  
  if (_rhiMode) {
    if (_params.debug) {
      cerr << "  NOTE: data is in RHI mode" << endl;
    }
    if (_params.interp_mode == Params::INTERP_MODE_CART_REORDER) {
      _allocReorderInterp();
      _reorderInterp->interpVol();
    } else {
      _allocCartInterp();
      _cartInterp->setRhiMode(true);
      _cartInterp->interpVol();
    }
  } else {
    if (_params.interp_mode == Params::INTERP_MODE_PPI) {
      _allocPpiInterp();
      _ppiInterp->interpVol();
    } else if (_params.interp_mode == Params::INTERP_MODE_POLAR) {
      _allocPolarInterp();
      _polarInterp->interpVol();
    } else if (_params.interp_mode == Params::INTERP_MODE_CART_REORDER) {

      _allocReorderInterp();
      _reorderInterp->interpVol();
    } else if (_params.interp_mode == Params::INTERP_MODE_CART_SAT) {
      _allocSatInterp();
      _satInterp->interpVol();
    } else {
      _allocCartInterp();
      _cartInterp->setRhiMode(false);
      _cartInterp->interpVol();
    }
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

int Radx2Grid::_readFile(const string &filePath)
{

  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  if (inFile.readFromPath(filePath, _readVol)) {
    cerr << "ERROR - Radx2Grid::_readFile" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  _readPaths = inFile.getReadPaths();
  
  // apply time offset

  if (_params.apply_time_offset) {
    _readVol.applyTimeOffsetSecs(_params.time_offset_secs);
  }

  // apply angle corrections as appropriate
  // side effect - forces el between -180 and 180
  
  _readVol.applyAzimuthOffset(_params.azimuth_correction_deg);
  _readVol.applyElevationOffset(_params.elevation_correction_deg);

  // pad out the gates to the longest range

  _readVol.setNGatesConstant();
  
  //  check for rhi
  
  _rhiMode = _isRhi();
  if (_params.interp_mode == Params::INTERP_MODE_CART_SAT) {
    _rhiMode = false;
  }

  // override radar location if requested

  if (_params.override_radar_location) {
    _readVol.overrideLocation(_params.radar_latitude_deg,
                              _params.radar_longitude_deg,
                              _params.radar_altitude_meters / 1000.0);
  }

  // volume number
  
  if (_params.override_volume_number ||
      _params.autoincrement_volume_number) {
    _readVol.setVolumeNumber(_volNum);
  }
  if (_params.autoincrement_volume_number) {
    _volNum++;
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
  
  if (_params.interp_mode == Params::INTERP_MODE_CART_SAT) {
    if (_params.sat_data_set_range_geom_from_fields) {
      _readVol.copyRangeGeomFromFieldsToRays();
    }
  }

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

  // set up and fields than need transforming

  _setupTransformFields();

  // add extra fields fields
  
  _addGeometryFields();
  _addTimeField();
  
  // for reorder, add in extra sweep at start and end
  // so that we can require boundedness

  if (_params.interp_mode == Params::INTERP_MODE_CART_REORDER) {
    _addBoundingSweeps();
  }
  
  // set up interp fields
  
  _initInterpFields();
  
  // load up the input ray data vector

  _loadInterpRays();

  // check all fields are present
  // set standard names etc

  _checkFields(filePath);

  return 0;

}

//////////////////////////////////////////////////
// set up read

void Radx2Grid::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }

  if (_params.aggregate_sweep_files_on_read) {
    file.setReadAggregateSweeps(true);
  } else {
    file.setReadAggregateSweeps(false);
  }

  if (_params.ignore_idle_scan_mode_on_read) {
    file.setReadIgnoreIdleMode(true);
  } else {
    file.setReadIgnoreIdleMode(false);
  }

  if (_params.remove_rays_with_antenna_transitions &&
      !_params.trim_surveillance_sweeps_to_360deg) {
    file.setReadIgnoreTransitions(true);
    file.setReadTransitionNraysMargin(_params.transition_nrays_margin);
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

  if (_params.interp_mode == Params::INTERP_MODE_POLAR ||
      _params.interp_mode == Params::INTERP_MODE_PPI) {
    if (_params.set_elevation_angle_limits) {
      file.setReadFixedAngleLimits(_params.lower_elevation_angle_limit,
                                   _params.upper_elevation_angle_limit);
      file.setReadStrictAngleLimits(true);
    }
  }

  if (_params.set_max_range) {
    file.setReadMaxRangeKm(_params.max_range_km);
  }

  if (_params.select_fields) {
    for (int ii = 0; ii < _params.selected_fields_n; ii++) {
      if (_params._selected_fields[ii].process_this_field) {
        file.addReadField(_params._selected_fields[ii].input_name);
      }
    }
    if (_params.apply_censoring) {
      for (int ii = 0; ii < _params.censoring_fields_n; ii++) {
        file.addReadField(_params._censoring_fields[ii].name);
      }
    }
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.printReadRequest(cerr);
  }
  
}

//////////////////////////////////////////////////
// check all fields are present
// set standard names etc

void Radx2Grid::_checkFields(const string &filePath)
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
// load up the input ray data vector

void Radx2Grid::_loadInterpRays()
{

  // loop through the rays in the read volume,
  // making some checks and then adding the rays
  // to the interp rays array as appropriate
  
  vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t isweep = 0; isweep < _readVol.getNSweeps(); isweep++) {

    const RadxSweep *sweep = _readVol.getSweeps()[isweep];

    for (size_t iray = sweep->getStartRayIndex();
         iray <= sweep->getEndRayIndex(); iray++) {

      const RadxRay *ray = rays[iray];

      // check elevation limits if required
      
      if (_params.interp_mode != Params::INTERP_MODE_POLAR &&
          _params.interp_mode != Params::INTERP_MODE_PPI) {
        if (_params.set_elevation_angle_limits) {
          double el = ray->getElevationDeg();
          if (el < _params.lower_elevation_angle_limit ||
              el > _params.upper_elevation_angle_limit) {
            continue;
          }
        }
      }

      // check azimith limits if required

      if (_params.set_azimuth_angle_limits) {
        double az = ray->getAzimuthDeg();
        double minAz = _params.lower_azimuth_angle_limit;
        double maxAz = _params.upper_azimuth_angle_limit;
        if (minAz <= maxAz) {
          // valid sector does not cross north
          if (az < minAz || az > maxAz) {
            continue;
          }
        } else {
          // valid sector does cross north
          if (az < minAz && az > maxAz) {
            continue;
          }
        }
      }

      // check fixed angle error?
      
      if (_params.check_fixed_angle_error) {
        double fixedAngle = ray->getFixedAngleDeg();
        double maxError = _params.max_fixed_angle_error;
        if (_rhiMode) {
          // RHI
          double error = fabs(fixedAngle - ray->getAzimuthDeg());
          if (error > 180) {
            error = fabs(error - 360.0);
          }
          if (error > maxError) {
            continue;
          }
        } else {
          // PPI
          double error = fabs(fixedAngle - ray->getElevationDeg());
          if (error > maxError) {
            continue;
          }
        }
      }

      // accept ray
      
      Interp::Ray *interpRay = 
        new Interp::Ray(rays[iray],
                        isweep,
                        _interpFields,
                        _params.use_fixed_angle_for_interpolation,
                        _params.use_fixed_angle_for_data_limits);
      if (_params.apply_censoring) {
        _censorInterpRay(interpRay);
      }
      _interpRays.push_back(interpRay);

    } // iray

  } // isweep


}
  
////////////////////////////////////////////////////////////////////
// censor an interp ray

void Radx2Grid::_censorInterpRay(Interp::Ray *interpRay)

{

  RadxRay *ray = interpRay->inputRay;

  if (!_params.apply_censoring) {
    return;
  }

  // initialize censoring flags to true to
  // turn censoring ON everywhere
  
  vector<int> censorFlag;
  size_t nGates = ray->getNGates();
  for (size_t igate = 0; igate < nGates; igate++) {
    censorFlag.push_back(1);
  }

  // check OR fields
  // if any of these have VALID data, we turn censoring OFF

  int orFieldCount = 0;

  for (int ifield = 0; ifield < _params.censoring_fields_n; ifield++) {

    const Params::censoring_field_t &cfld = _params._censoring_fields[ifield];
    if (cfld.combination_method != Params::LOGICAL_OR) {
      continue;
    }

    RadxField *field = ray->getField(cfld.name);
    if (field == NULL) {
      // field missing, do not censor
      if (_nWarnCensorPrint % 360 == 0) {
        cerr << "WARNING - censoring field missing: " << cfld.name << endl;
        cerr << "  Censoring will not be applied for this field." << endl;
      }
      _nWarnCensorPrint++;
      for (size_t igate = 0; igate < nGates; igate++) {
        censorFlag[igate] = 0;
      }
      continue;
    }
    
    orFieldCount++;
    
    double minValidVal = cfld.min_valid_value;
    double maxValidVal = cfld.max_valid_value;

    const Radx::fl32 *fdata = (const Radx::fl32 *) field->getData();
    for (size_t igate = 0; igate < nGates; igate++) {
      double val = fdata[igate];
      if (val >= minValidVal && val <= maxValidVal) {
        censorFlag[igate] = 0;
      }
    }
    
  } // ifield

  // if no OR fields were found, turn off ALL censoring at this stage

  if (orFieldCount == 0) {
    for (size_t igate = 0; igate < nGates; igate++) {
      censorFlag[igate] = 0;
    }
  }

  // check AND fields
  // if any of these have INVALID data, we turn censoring ON

  for (int ifield = 0; ifield < _params.censoring_fields_n; ifield++) {
    
    const Params::censoring_field_t &cfld = _params._censoring_fields[ifield];
    if (cfld.combination_method != Params::LOGICAL_AND) {
      continue;
    }

    RadxField *field = ray->getField(cfld.name);
    if (field == NULL) {
      continue;
    }
    
    double minValidVal = cfld.min_valid_value;
    double maxValidVal = cfld.max_valid_value;

    const Radx::fl32 *fdata = (const Radx::fl32 *) field->getData();
    for (size_t igate = 0; igate < nGates; igate++) {
      double val = fdata[igate];
      if (val < minValidVal || val > maxValidVal) {
        censorFlag[igate] = 1;
      }
    }
    
  } // ifield

  // check that uncensored runs meet the minimum length
  // those which do not are censored

  int minValidRun = _params.censoring_min_valid_run;
  if (minValidRun > 1) {
    int runLength = 0;
    bool doCheck = false;
    for (int igate = 0; igate < (int) nGates; igate++) {
      if (censorFlag[igate] == 0) {
        doCheck = false;
        runLength++;
      } else {
        doCheck = true;
      }
      // last gate?
      if (igate == (int) nGates - 1) doCheck = true;
      // check run length
      if (doCheck) {
        if (runLength < minValidRun) {
          // clear the run which is too short
          for (int jgate = igate - runLength; jgate < igate; jgate++) {
            censorFlag[jgate] = 1;
          } // jgate
        }
        runLength = 0;
      } // if (doCheck ...
    } // igate
  }

  // apply censoring by setting censored gates to missing for all fields

  vector<RadxField *> fields = ray->getFields();
  for (size_t ifield = 0; ifield < fields.size(); ifield++) {
    RadxField *field = fields[ifield];
    if (field->getLongName().find("diagnostic_field_") != string::npos) {
      // do not censor diagnostic fields
      continue;
    }
    Radx::fl32 *fdata = (Radx::fl32 *) field->getData();
    for (size_t igate = 0; igate < nGates; igate++) {
      if (censorFlag[igate] == 1) {
        fdata[igate] = Radx::missingFl32;
      }
    } // igate
  } // ifield
  
}

//////////////////////////////////////////////////
// add geometry fields

void Radx2Grid::_addGeometryFields()
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

void Radx2Grid::_addTimeField()
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

//////////////////////////////////////////////////
// set up the transform fields, as needed

void Radx2Grid::_setupTransformFields()
{

  if (!_params.transform_fields_for_interpolation) {
    return;
  }

  // loop through rays

  vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {

    RadxRay *ray = rays[iray];
    
    // loop through fields to be transformed

    for (int jfield = 0; jfield < _params.transform_fields_n; jfield++) {

      bool makeCopy = true;
      const Params::transform_field_t &transform = _params._transform_fields[jfield];
      if (strcmp(transform.input_name, transform.output_name) == 0) {
        makeCopy = false;
      }

      // find field on ray

      RadxField *rfield = ray->getField(transform.input_name);
      if (rfield == NULL) {
        continue;
      }

      // get working field
      
      RadxField *xfield = rfield;
      if (makeCopy) {
        // copy field
        xfield = new RadxField(*rfield);
        xfield->setName(transform.output_name);
      }

      // set units

      xfield->setUnits(transform.output_units);
      
      // transform

      xfield->convertToFl32();
      if (transform.transform == Params::TRANSFORM_DB_TO_LINEAR ||
          transform.transform == Params::TRANSFORM_DB_TO_LINEAR_AND_BACK) {
        xfield->transformDbToLinear();
      } else if (transform.transform == Params::TRANSFORM_LINEAR_TO_DB ||
                 transform.transform == Params::TRANSFORM_LINEAR_TO_DB_AND_BACK) {
        xfield->transformLinearToDb();
      }

      if (makeCopy) {
        // add to ray
        ray->addField(xfield);
      }

    } // jfield

  } // iray

}

/////////////////////////////////////////////////////
// check whether volume is predominantly in RHI mode

bool Radx2Grid::_isRhi()
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

////////////////////////////////////////////////////////////
// Add in sweeps to provide boundedness

void Radx2Grid::_addBoundingSweeps()
  
{

  if (_rhiMode) {
    return;
  }

  size_t nSweeps = _readVol.getNSweeps();
  if (nSweeps < 2) {
    return;
  }

  // find sweep number for lowest and highest angles

  int maxSweepNum = -99;
  int lowSweepNum = 0, highSweepNum = 0;
  double minFixedAngle = 360;
  double maxFixedAngle = -360;
  const vector<RadxSweep *> &sweeps = _readVol.getSweeps();
  for (size_t ii = 0; ii < sweeps.size(); ii++) {
    const RadxSweep *sweep = sweeps[ii];
    double fixedAngle = sweep->getFixedAngleDeg();
    int sweepNum = sweep->getSweepNumber();
    if (fixedAngle < minFixedAngle) {
      lowSweepNum = sweepNum;
      minFixedAngle = fixedAngle;
    }
    if (fixedAngle > maxFixedAngle) {
      highSweepNum = sweepNum;
      maxFixedAngle = fixedAngle;
    }
    if (sweepNum > maxSweepNum) {
      maxSweepNum = sweepNum;
    }
  }

  // loop through rays, adding rays below the lowest sweep

  double beamWidthDegV = _readVol.getRadarBeamWidthDegV();
  const vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    const RadxRay *ray = rays[ii];
    if (ray->getSweepNumber() == lowSweepNum) {
      // copy the ray
      RadxRay *copy = new RadxRay(*ray);
      // set the fixed angle half a beam width below
      copy->setFixedAngleDeg(ray->getFixedAngleDeg() - beamWidthDegV / 2.0);
      // set the elevation angle half a beam width below
      copy->setElevationDeg(ray->getElevationDeg() - beamWidthDegV / 2.0);
      // set the sweep number to 1 above the max
      copy->setSweepNumber(maxSweepNum + 1);
      // add to vol
      _readVol.addRay(copy);
    }
  } // ii

  // loop through rays, adding rays above the highest sweep

  for (size_t ii = 0; ii < rays.size(); ii++) {
    const RadxRay *ray = rays[ii];
    if (ray->getSweepNumber() == highSweepNum) {
      // copy the ray
      RadxRay *copy = new RadxRay(*ray);
      // set the fixed angle half a beam width above
      copy->setFixedAngleDeg(ray->getFixedAngleDeg() + beamWidthDegV / 2.0);
      // set the elevation angle half a beam width above
      copy->setElevationDeg(ray->getElevationDeg() + beamWidthDegV / 2.0);
      // set the sweep number to 2 above the max
      copy->setSweepNumber(maxSweepNum + 2);
      // add to vol
      _readVol.addRay(copy);
    }
  } // ii

  _readVol.loadSweepInfoFromRays();
  
}

//////////////////////////////////////////////////
// initialize the fields for interpolation

void Radx2Grid::_initInterpFields()
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
      string radxName = _params._folded_fields[ii].input_name;
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

  // override discrete flag from the parameters
  
  if (_params.set_discrete_fields) {
    for (int ii = 0; ii < _params.discrete_fields_n; ii++) {
      string radxName = _params._discrete_fields[ii].input_name;
      bool isDiscrete = _params._discrete_fields[ii].is_discrete;
      for (size_t ifld = 0; ifld < _interpFields.size(); ifld++) {
        if (_interpFields[ifld].radxName == radxName) {
          _interpFields[ifld].isDiscrete = isDiscrete;
          break;
        }
      } // ifld
    } // ii
  }

  // set bounded fields from the parameters (probably not in the radx data
  // anyway)
  
  if (_params.bound_fields) {
    for (int ii = 0; ii < _params.bounded_fields_n; ii++) {
      string radxName = _params._bounded_fields[ii].input_name;
      double v0 = _params._bounded_fields[ii].min_value;
      double v1 = _params._bounded_fields[ii].max_value;
      for (size_t ifld = 0; ifld < _interpFields.size(); ifld++) {
        if (_interpFields[ifld].radxName == radxName) {
          _interpFields[ifld].isBounded = true;
	  _interpFields[ifld].boundLimitLower = v0;
	  _interpFields[ifld].boundLimitUpper = v1;
          break;
        }
      } // ifld
    } // ii
  }

  // rename fields

  if (_params.rename_fields) {
    for (int ii = 0; ii < _params.renamed_fields_n; ii++) {
      string inputName = _params._renamed_fields[ii].input_name;
      string outputName = _params._renamed_fields[ii].output_name;
      for (size_t ifld = 0; ifld < _interpFields.size(); ifld++) {
        if (_interpFields[ifld].radxName == inputName) {
          _interpFields[ifld].outputName = outputName;
          break;
        }
      } // ifld
    } // ii
  } // if (_params.specify_field_names)

}

////////////////////////////////////////////////////////////
// Allocate interpolation objects as needed

void Radx2Grid::_allocCartInterp()
{
  if (_cartInterp == NULL) {
    _cartInterp = new CartInterp(_progName, _params, _readVol,
                                 _interpFields, _interpRays);
  }
}

void Radx2Grid::_allocPpiInterp()
{
  if (_ppiInterp == NULL) {
    _ppiInterp = new PpiInterp(_progName, _params, _readVol,
                               _interpFields, _interpRays);
  }
}

void Radx2Grid::_allocPolarInterp()
{
  if (_polarInterp == NULL) {
    _polarInterp = new PolarInterp(_progName, _params, _readVol,
                                   _interpFields, _interpRays);
  }
}

void Radx2Grid::_allocReorderInterp()
{
  if (_reorderInterp == NULL) {
    _reorderInterp = new ReorderInterp(_progName, _params, _readVol,
                                       _interpFields, _interpRays);
  }
}

void Radx2Grid::_allocSatInterp()
{
  if (_satInterp == NULL) {
    _satInterp = new SatInterp(_progName, _params, _readVol,
                               _interpFields, _interpRays);
  }
}

////////////////////////////////////////////////////////////
// Free up input rays

void Radx2Grid::_freeInterpRays()
  
{
  for (size_t ii = 0; ii < _interpRays.size(); ii++) {
    delete _interpRays[ii];
  }
  _interpRays.clear();
}

