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
// Dsr2Radx.cc
//
// Dsr2Radx object
// Updated processing
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2018
//
///////////////////////////////////////////////////////////////
//
// Dsr2Radx reads an input radar FMQ, and saves out data files
// in formats supported by Radx.
//
///////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/TaArray.hh>
#include <toolsa/udatetime.h>
#include <toolsa/DateTime.hh>
#include <toolsa/toolsa_macros.h>
#include <dsserver/DsLdataInfo.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/DoradeRadxFile.hh>
#include <Radx/UfRadxFile.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxGeoref.hh>
#include <radar/IwrfMomReader.hh>

#include "Dsr2Radx.hh"
#include "Legacy.hh"

using namespace std;

const double Dsr2Radx::_smallVal = 0.0001;
const double Dsr2Radx::_pseudoDiam = 17066.0;

// Constructor

Dsr2Radx::Dsr2Radx(int argc, char **argv)

{

  isOK = true;
  _clearData();
  _nCheckPrint = 0;
  _nWarnCensorPrint = 0;

  _prevAzMoving = -999;
  _prevElevMoving = -999;

  _lutRadarAltitudeKm = -9999;

  _prevVolNum = -99999;
  _prevTiltNum = -1;
  _prevSweepNum = -1;
  _prevSweepMode = Radx::SWEEP_MODE_NOT_SET;
  _sweepNumOverride = 1;
  _sweepNumDecreasing = false;
  _endOfVol = false;
  _endOfVolTime = -1;
  _antenna = NULL;
  _outFile = NULL;
  _sweepMgr = NULL;
  _cachedRay = NULL;
  _prevRay = NULL;

  _sweepMode = Radx::SWEEP_MODE_NOT_SET;
  _scanMode = SCAN_MODE_UNKNOWN;
  _scanInfoFromHeaders = false;

  _isSolarScan = false;
  _reader = NULL;

  // set programe name

  _progName = "Dsr2Radx";
  ucopyright((char *) _progName.c_str());
  
  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // check params

  int nGeomOptions = 0;
  if (_params.convert_to_specified_output_gate_geometry) {
    nGeomOptions++;
  }
  if (_params.convert_to_predominant_gate_geometry) {
    nGeomOptions++;
  }
  if (_params.convert_to_finest_gate_geometry) {
    nGeomOptions++;
  }
  if (nGeomOptions > 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem with TDRP parameters" << endl;
    cerr << "  You can only set one of the following to true:" << endl;
    cerr << "    convert_to_specified_output_gate_geometry" << endl;
    cerr << "    convert_to_predominant_gate_geometry" << endl;
    cerr << "    convert_to_finest_gate_geometry" << endl;
    isOK = false;
  }

  int nSweepFilterOptions = 0;
  if (_params.filter_using_sweep_number) {
    nSweepFilterOptions++;
  }
  if (_params.filter_using_sweep_number_list) {
    nSweepFilterOptions++;
  }
  if (nSweepFilterOptions > 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem with TDRP parameters" << endl;
    cerr << "  You can only set one of the following to true:" << endl;
    cerr << "    filter_using_sweep_number" << endl;
    cerr << "    filter_using_sweep_number_list" << endl;
    isOK = false;
  }
  
  // override missing values

  if (_params.override_missing_metadata_values) {
    Radx::setMissingMetaDouble(_params.missing_metadata_double);
    Radx::setMissingMetaFloat(_params.missing_metadata_float);
    Radx::setMissingMetaInt(_params.missing_metadata_int);
    Radx::setMissingMetaChar(_params.missing_metadata_char);
  }
  if (_params.override_missing_field_values) {
    Radx::setMissingFl64(_params.missing_field_fl64);
    Radx::setMissingFl32(_params.missing_field_fl32);
    Radx::setMissingSi32(_params.missing_field_si32);
    Radx::setMissingSi16(_params.missing_field_si16);
    Radx::setMissingSi08(_params.missing_field_si08);
  }

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		_params.procmap_register_interval);

}

/////////////////////////////////////////////////////////
// destructor

Dsr2Radx::~Dsr2Radx()

{

  _clearData();

  if (_outFile) {
    delete _outFile;
  }

  if (_antenna != NULL) {
    delete _antenna;
  }

  if (_sweepMgr != NULL) {
    delete _sweepMgr;
  }

  for (int icalib = 0; icalib < (int) _calibs.size(); icalib++) {
    delete _calibs[icalib];
  }
  _calibs.clear();

  if (_reader) {
    delete _reader;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Dsr2Radx::Run ()
{
  
  // check for legacy processing

  // if (_params.use_legacy_processing) {
  //   Legacy *legacy = new Legacy(_progName, _params);
  //   int iret = legacy->Run();
  //   delete legacy;
  //   return iret;
  // }

  // normal processing
  // register with procmap
  
  PMU_auto_register("Run");

  // create antenna angle manager
  
  _antenna = new Antenna(_progName, _params);
  _endOfVolAutomatic =
    (_params.end_of_vol_decision == Params::AUTOMATIC);
  
  // create sweep mamager

  _sweepMgr = new SweepMgr(_progName, _params);

  // set up volume object

  if (_params.debug) {
    _vol.setDebug(true);
  }

  _vol.setTitle(_params.ncf_title);
  _vol.setInstitution(_params.ncf_institution);
  _vol.setReferences(_params.ncf_references);
  _vol.setSource(_params.ncf_source);
  _vol.setHistory(_params.ncf_history);
  _vol.setComment(_params.ncf_comment);

  // create reader for moments

  _reader = new IwrfMomReaderFmq(_params.input_fmq_url);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _reader->setDebug(IWRF_DEBUG_NORM);
  }
  if (_params.seek_to_end_of_input) {
    _reader->seekToEnd();
  } else {
    _reader->seekToStart();
  }

  // set to timeout if we want to write out volume
  // when data stops flowing

  if (_params.write_end_of_vol_when_data_stops) {
    _reader->setBlockingTimeout
      (_params.nsecs_no_data_for_end_of_vol * 1000);
  }

  // set up output file object

  _outFile = new RadxFile();
  _setupWrite();

  // loop

  int iret = 0;
  while (true) {
    iret = _run();
    return iret;
    if (_params.debug) {
      cerr << "Dsr2Radx::Run:" << endl;
      cerr << "  Trying to contact input server at url: "
	   << _params.input_fmq_url << endl;
    }
    umsleep(1000);
  }

  return iret;

}

//////////////////////////////////////////////////
// _run

int Dsr2Radx::_run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // clear all data from previously

  _clearData();

  // read beams from the queue and process them
  
  int iret = 0;

  while (true) {

    PMU_auto_register("Reading radar FMQ");
    
    // get the next ray
    
    RadxRay *ray = _reader->readNextRay();
    if (ray == NULL) {
      // read error - no data
      if (_params.write_end_of_vol_when_data_stops) {
        _processEndOfVol();
      }
      continue;
    } // if (ray == NULL) 
    _nRaysRead++;

    // process this ray
    
    _processRay(ray);

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      if (_nRaysRead % 2000 == 0) {
        const vector<RadxRcalib> &calibs = _reader->getRcalibs();
        if (calibs.size() > 0) {
          calibs[0].print(cerr);
        }
      }
    }
    
    // at the end of a volume, process volume, then reset
    
    if (_endOfVol) {
      _processEndOfVol();
    }
    
  } // while (true)
  
  return iret;

}

////////////////////////////////////////////////////////////////////
// Process a ray
// Returns -1 on error

int Dsr2Radx::_processRay(RadxRay *ray) 
  
{
  
   
  PMU_auto_register("Processing ray");
  _endOfVol = false;
  
  // set platform parameters if avaliable

  if (_reader->getPlatformUpdated()) {
    _updatePlatform(ray);
  }

  // set calibration parameters if avaliable

  if (_reader->getRcalibUpdated()) {
    _updateRcalib();
  }
  
  // set status XML if available
  
  if (_reader->getStatusXmlUpdated()) {
    _vol.setStatusXml(_reader->getStatusXml());
  }

  // sweep numbers
  
  if (ray->getSweepNumber() < 0) {
    _sweepNumbersMissing = true;
  } else if (ray->getSweepNumber() < _prevTiltNum) {
    // tilt number decreased, so reset sweepNum
    _sweepNumDecreasing = true;
  }
  _prevTiltNum = ray->getSweepNumber();

  // adjust azimuth and elevation if required
  
  double elev = ray->getElevationDeg();
  if (_params.apply_elevation_offset) {
    elev += _params.elevation_offset;
    ray->setElevationDeg(Radx::conditionEl(elev));
  }

  double az = ray->getAzimuthDeg();
  if (_params.apply_azimuth_offset) {
    az += _params.azimuth_offset;
    ray->setAzimuthDeg(Radx::conditionAz(az));
  }

  // force sweep number change on azimuth transition?

  if (_params.force_sur_sweep_transitions_at_fixed_azimuth) {
    if ((_prevRay != NULL) &&
        (ray->getSweepMode() == Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE)) {
      double thisAz = ray->getAzimuthDeg();
      double prevAz = _prevRay->getAzimuthDeg();
      // check for end of sweep transition
      // this occurs at the same azimuth every time
      double transDeg = _params.sur_sweep_transitions_azimuth_deg;
      double deltaPrev = _computeDeltaAngle(transDeg, prevAz);
      double deltaThis = _computeDeltaAngle(transDeg, thisAz);
      if (deltaPrev > 0 && deltaThis <= 0) {
        if (_sweepNumDecreasing) {
          _sweepNumOverride = 1;
          _sweepNumDecreasing = false;
        } else {
          _sweepNumOverride++;
        }
        if (_params.debug) {
          cerr << "====>> Forcing sweep number change, az: "
               << thisAz << " <<====" << endl;
          cerr << "====>> New sweep number: "
               << _sweepNumOverride << " <<====" << endl;
        }
      }
    } // if (_prevRay != NULL ...
    ray->setSweepNumber(_sweepNumOverride);
  } // if (_params.force_sur_sweep_transitions_at_fixed_azimuth

  // range geometry

  int nGates = ray->getNGates();
  if (_params.remove_test_pulse) {
    nGates -= _params.ngates_test_pulse;
  }
  
  // optionally limit number of gates based on height
  // don't do this for solars
  
  if (_params.crop_above_max_height && !_isSolarScan) {
    double nGatesForMaxHt =
      _computeNgatesForMaxHt(elev,
                             _vol.getAltitudeKm(),
                             ray->getStartRangeKm(),
                             ray->getGateSpacingKm());
    if (nGatesForMaxHt < nGates) {
      nGates = (int) (nGatesForMaxHt + 0.5);
    }
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Computing nGates based on max ht, elev, nGates: "
           << elev << ", " << nGates << endl;
    }
  }
  
  if (ray->getSweepMode() == Radx::SWEEP_MODE_RHI ||
      ray->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
    double targetAz = ray->getFixedAngleDeg();
    if (_params.apply_azimuth_offset) {
      targetAz += _params.azimuth_offset;
    }
    ray->setFixedAngleDeg(Radx::conditionAz(targetAz));
  } else {
    double targetEl = ray->getFixedAngleDeg();
    if (_params.apply_elevation_offset) {
      targetEl += _params.elevation_offset;
    }
    ray->setFixedAngleDeg(Radx::conditionEl(targetEl));
  }

  // censor the fields in the ray if requested
  
  _censorInputRay(ray);
    
  // prepare the ray for output, by trimming out any unnecessary fields
  // and setting the names etc
  
  _prepareRayForOutput(ray);
    
  // debug print
  
  if (_params.debug) {
    int nPrintFreq = 90;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      nPrintFreq = 1;
    } else if (_params.debug >= Params::DEBUG_VERBOSE) {
      nPrintFreq = 10;
    }
    if ((_nRaysRead > 0) && (_nRaysRead % nPrintFreq == 0) &&
        (int) _vol.getNRays() != _nCheckPrint) {
      _nCheckPrint = (int) _vol.getNRays();
      fprintf(stderr,
              "  nRays, sweep, vol, latest time, el, az: "
              "%4d,%3d,%5d,%20s,%7.2f,%7.2f\n",
              _nRaysRead,
              ray->getSweepNumber(),
              ray->getVolumeNumber(),
              utimstr(ray->getTimeSecs()),
              ray->getElevationDeg(),
              ray->getAzimuthDeg());
    }
  }
    
  // end of vol condition

  if (_params.end_of_vol_decision == Params::ELAPSED_TIME) {
    
    if (_endOfVolTime < 0) {
      // initialize
      _computeEndOfVolTime(ray->getTimeSecs());
    } else if ((_endOfVolTime - ray->getTimeSecs()) >
               _params.nsecs_per_volume) {
      // we have gone back in time
      // maybe reprocessing old data
      if (_params.debug) {
        cerr << "Going back in time - reprocessing old data?" << endl;
      }
      _computeEndOfVolTime(ray->getTimeSecs());
    }
    if (ray->getTimeSecs() >= _endOfVolTime) {
      _endOfVol = true;
      _computeEndOfVolTime(ray->getTimeSecs() + 1);
    }
    
  } else if (_params.end_of_vol_decision == Params::EVERY_360_DEG) {
    
    if (_checkEndOfVol360(ray)) {
      _endOfVol = true;
    }
    
  } else if (_params.end_of_vol_decision == Params::CHANGE_IN_SWEEP_NUM) {
    
    int sweepNum = ray->getSweepNumber();
    if (sweepNum >= 0 && sweepNum != _prevSweepNum) {
      if (_prevSweepNum >= 0) {
        _endOfVol = true;
      }
      _prevSweepNum = sweepNum;
    }
    
  } else if (_params.end_of_vol_decision == Params::CHANGE_IN_SWEEP_MODE) {
    
    Radx::SweepMode_t sweepMode = ray->getSweepMode();
    if (sweepMode != Radx::SWEEP_MODE_NOT_SET && 
        sweepMode != _prevSweepMode) {
      if (_prevSweepMode != Radx::SWEEP_MODE_NOT_SET) {
        _endOfVol = true;
      }
      _prevSweepMode = sweepMode;
    }
    
  } else if (_params.end_of_vol_decision == Params::CHANGE_IN_VOL_NUM) {
    
    int volNum = ray->getVolumeNumber();
    if (volNum != -1 && volNum != _prevVolNum) {
      if (_prevVolNum != -99999 &&
          _params.end_of_vol_decision == Params::CHANGE_IN_VOL_NUM) {
        _endOfVol = true;
      }
      _prevVolNum = volNum;
    }
    
  } else if (_endOfVolAutomatic) {
    
    if (_antenna->addRay(ray)) {
      _endOfVol = true;
    }
    
  } // if (_params.end_of_vol_decision == Params::ELAPSED_TIME
    
  if ((int) _vol.getNRays() > _params.max_rays_in_vol) {
    _endOfVol = true;
  }

  // check in sweep mode?

  if (ray->getSweepMode() != _sweepMode &&
      _sweepMode != Radx::SWEEP_MODE_NOT_SET) {
    _endOfVol = true;
  }
  _sweepMode = ray->getSweepMode();

  // check for end-of-sweep or end-of-volume events
  
  const vector<RadxEvent> &events = _reader->getEvents();
  if (!_endOfVolAutomatic &&
      _params.end_of_vol_decision != Params::CHANGE_IN_VOL_NUM &&
      _params.end_of_vol_decision != Params::CHANGE_IN_SWEEP_NUM &&
      events.size() > 0) {
    
    for (size_t ievent = 0; ievent < events.size(); ievent++) {
      const RadxEvent &event = events[ievent];
      if (_params.end_of_vol_decision == Params::END_OF_VOL_FLAG &&
          event.getEndOfVolume()) {
        _endOfVol = true;
      } else if (_params.end_of_vol_decision == Params::LAST_SWEEP_IN_VOL &&
                 event.getEndOfSweep() &&
                 event.getSweepNumber() == _params.last_sweep_in_vol) {
        _endOfVol = true;
      }
    } // ievent
    
  } // if (!_endOfVolAutomatic
  
  // add the ray to the volume as appropriate
  // check for transitions

  if (ray != NULL) {

    bool ignoreRay = false;

    if (_params.clear_transition_flag_on_all_rays) {
      ray->setAntennaTransition(false);
    } else {
      if (_antenna->getInTransition()) {
        ray->setAntennaTransition(true);
        if (_params.filter_antenna_transitions) {
          ignoreRay = true;
        }
      }
    }

    if (ignoreRay) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Transition, ignoring, el, az: "
             << ray->getElevationDeg() << ", "
             << ray->getAzimuthDeg() << endl;
      }
      delete ray;
    } else {
      if (_cachedRay) {
        _addRayToVol(_cachedRay);
        _cachedRay = NULL;
      }
      if (_endOfVol) {
        if (_endOfVolAutomatic ||
            _params.end_of_vol_decision == Params::CHANGE_IN_VOL_NUM ||
            _params.end_of_vol_decision == Params::CHANGE_IN_SWEEP_NUM ||
            _params.end_of_vol_decision == Params::CHANGE_IN_SWEEP_MODE) {
          _cachedRay = ray;
        } else {
          _addRayToVol(ray);
        }
      } else {
        _addRayToVol(ray);
      }
    }
  }

  return 0;

}

////////////////////////////////////////////////////////////////////
// Process an end-of-volume status

int Dsr2Radx::_processEndOfVol()
  
{
  
  if (_params.debug) {
    if (_params.end_of_vol_decision == Params::CHANGE_IN_SWEEP_NUM) {
      cerr << "=========== End of sweep ===============" << endl;
    } else {
      cerr << "=========== End of volume ==============" << endl;
    }
    cerr << "  nrays available:" << _vol.getNRays() << endl;
  }
  
  if (_processVol()) {
    _clearData();
    return -1;
  }
  
  _clearData();
  return 0;

}

////////////////////////////////////////////////////////////////
// process the volume

int Dsr2Radx::_processVol()
  
{

  int iret = 0;

  PMU_force_register("Processing volume");
  
  if (_params.debug) {
    cerr << "**** Start Dsr2Radx::_processVol() ****" << endl;
  }

  // load the current scan mode - PPI or RHI
  
  _loadCurrentScanMode();

  // set the sweep numbers in the input rays, if needed

  if (_sweepNumbersMissing || _params.find_sweep_numbers_using_histogram) {
    _sweepMgr->setSweepNumbers(_scanMode == SCAN_MODE_RHI, _vol.getRays());
  } else if (_scanMode != SCAN_MODE_RHI &&
             _params.end_of_vol_decision == Params::EVERY_360_DEG) {
    const vector<RadxRay *> &rays = _vol.getRays();
    for (size_t ii = 0; ii < rays.size(); ii++) {
      rays[ii]->setSweepNumber(0);
    }
  }
  
  // load up rays from ray data
  
  bool notEnoughRays = false;
  if (_scanMode == SCAN_MODE_RHI) {
    if ((int) _vol.getNRaysNonTransition() < _params.min_rays_per_rhi_vol) {
      notEnoughRays = true;
    }
  } else {
    if ((int) _vol.getNRaysNonTransition() < _params.min_rays_per_ppi_vol) {
      notEnoughRays = true;
    }
  }
  if (notEnoughRays) {
    cerr << "WARNING - Dsr2Radx::_processVol()" << endl;
    cerr << "  Too few rays: " << _vol.getNRays() << endl;
    cerr << "  skipping volume" << endl;
    _clearData();
    return 0;
  }

  // set the volume info from the rays

  _vol.loadVolumeInfoFromRays();
  
  // set the sweep numbers from the rays

  if (_params.increment_sweep_num_when_pol_mode_changes) {
    _vol.incrementSweepOnPolModeChange();
  }

  if (_params.increment_sweep_num_when_prt_mode_changes) {
    _vol.incrementSweepOnPrtModeChange();
  }

  bool isRhi = _vol.checkIsRhi();
  bool isSurveillance = false;
  if (!isRhi) {
    isSurveillance = _vol.checkIsSurveillance();
  }

  if (isRhi) {
    // RHI
    if (_params.adjust_rhi_sweep_limits_using_angles) {
      _vol.adjustSweepLimitsUsingAngles();
    } else {
      _vol.loadSweepInfoFromRays();
    }
  } else if (isSurveillance) {
    // SUR
    if (_params.adjust_sur_sweep_limits_using_angles) {
      _vol.optimizeSurveillanceTransitions(_params.adjust_sur_sweep_max_angle_error);
    } else {
      _vol.loadSweepInfoFromRays();
    }
    if (_params.trim_surveillance_sweeps_to_360deg) {
      _vol.trimSurveillanceSweepsTo360Deg();
    }
  } else {
    // sector
    if (_params.adjust_sector_sweep_limits_using_angles) {
      _vol.adjustSweepLimitsUsingAngles();
    } else {
      _vol.loadSweepInfoFromRays();
    }
  }

  if (isRhi) {
    if (_params.compute_rhi_fixed_angles_from_measured_azimuth) {
      _vol.computeFixedAnglesFromRays
        (true, _params.use_mean_to_compute_fixed_angles);
    }
  } else{
    if (_params.compute_ppi_fixed_angles_from_measured_elevation) {
      _vol.computeFixedAnglesFromRays
        (true, _params.use_mean_to_compute_fixed_angles);
    }
  }

  // set calibration indexes

  _vol.loadCalibIndexOnRays();

  // convert to common geometry

  if (_params.convert_to_specified_output_gate_geometry) {
    _vol.remapRangeGeom(_params.output_start_range_km,
                        _params.output_gate_spacing_km,
                        _params.interpolate_to_output_gate_geometry);
  } else if (_params.convert_to_predominant_gate_geometry) {
    _vol.remapToPredomGeom();
  } else if (_params.convert_to_finest_gate_geometry) {
    _vol.remapToFinestGeom();
  } else {
    _vol.filterOnPredomGeom();
  }

  // check for indexed rays

  _vol.checkForIndexedRays();

  // trim sweeps with too few rays

  size_t nraysVolBefore = _vol.getNRays();
  if (_params.check_min_rays_in_sweep) {
    _vol.removeSweepsWithTooFewRays(_params.min_rays_in_sweep);
  } else if (_params.check_min_rays_in_ppi_sweep && !isRhi) {
    _vol.removeSweepsWithTooFewRays(_params.min_rays_in_ppi_sweep);
  } else if (_params.check_min_rays_in_rhi_sweep && isRhi) {
    _vol.removeSweepsWithTooFewRays(_params.min_rays_in_rhi_sweep);
  }
  if (_params.debug) {
    if (nraysVolBefore != _vol.getNRays()) {
      cerr << "NOTE: removed sweeps with too few rays" << endl;
      cerr << "  nrays in vol before removal: "
           << nraysVolBefore << endl;
      cerr << "  nrays in vol after  removal: "
           << _vol.getNRays() << endl;
    }
  }

  // if requested, change some of the characteristics
  
  if (_params.override_instrument_type) {
    _vol.setInstrumentType((Radx::InstrumentType_t) _params.instrument_type);
  }
  if (_params.override_platform_type) {
    _vol.setPlatformType((Radx::PlatformType_t) _params.platform_type);
  }
  if (_params.override_primary_axis) {
    _vol.setPrimaryAxis((Radx::PrimaryAxis_t) _params.primary_axis);
    // if we change the primary axis, we need to reapply the georefs
    if (_params.apply_georeference_corrections) {
      _vol.applyGeorefs();
    }
  }

  // write the files

  if (_doWrite()) {
    iret = -1;
  }
  PMU_force_register("done writing");

  if (_params.debug) {
    cerr << "**** End Dsr2Radx::_processVol() ****" << endl;
  }

  return iret;

}

//////////////////////////////////////////////////
// set up write

void Dsr2Radx::_setupWrite()
{

  if (_params.debug) {
    _outFile->setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _outFile->setVerbose(true);
  }

  if (_params.output_compressed) {
    _outFile->setWriteCompressed(true);
    _outFile->setCompressionLevel(_params.output_compression_level);
  } else {
    _outFile->setWriteCompressed(false);
  }
  if (_params.output_native_byte_order) {
    _outFile->setWriteNativeByteOrder(true);
  } else {
    _outFile->setWriteNativeByteOrder(false);
  }

  switch (_params.netcdf_style) {
    case Params::NETCDF4:
      _outFile->setNcFormat(RadxFile::NETCDF4);
      break;
    case Params::OFFSET_64BIT:
      _outFile->setNcFormat(RadxFile::NETCDF_OFFSET_64BIT);
      break;
    case Params::NETCDF4_CLASSIC:
      _outFile->setNcFormat(RadxFile::NETCDF4_CLASSIC);
      break;
    case Params::CLASSIC:
    default:
      _outFile->setNcFormat(RadxFile::NETCDF_CLASSIC);
  }

  if (strlen(_params.output_filename_prefix) > 0) {
    _outFile->setWriteFileNamePrefix(_params.output_filename_prefix);
  }
  
  _outFile->setWriteInstrNameInFileName(_params.include_instrument_name_in_file_name);
  _outFile->setWriteSiteNameInFileName(_params.include_site_name_in_file_name);
  _outFile->setWriteSubsecsInFileName(_params.include_subsecs_in_file_name);
  _outFile->setWriteScanTypeInFileName(_params.include_scan_type_in_file_name);
  _outFile->setWriteScanNameInFileName(_params.include_scan_name_in_file_name);
  _outFile->setWriteVolNumInFileName(_params.include_vol_num_in_file_name);
  _outFile->setWriteFixedAngleInFileName(_params.include_mean_fixed_angle_in_file_name);
  _outFile->setWriteHyphenInDateTime(_params.use_hyphen_in_file_name_datetime_part);

}

////////////////////////////////////////////////////////////////
// perform the write

int Dsr2Radx::_doWrite()
  
{

  PMU_force_register("_doWrite");
  
  int iret = 0;

  for (int idata = 0; idata < _params.output_data_set_n; idata++) {

    const Params::output_data_set_t &dset =
      _params._output_data_set[idata];
    
    string dataType = "nc";
    switch (dset.format) {
      case Params::OUTPUT_FORMAT_UF:
        _outFile->setFileFormat(RadxFile::FILE_FORMAT_UF);
        dataType = "uf";
        break;
      case Params::OUTPUT_FORMAT_FORAY:
        _outFile->setFileFormat(RadxFile::FILE_FORMAT_FORAY_NC);
        dataType = "foray";
        break;
      case Params::OUTPUT_FORMAT_DORADE:
        _outFile->setFileFormat(RadxFile::FILE_FORMAT_DORADE);
        dataType = "dorade";
        break;
      default:
        _outFile->setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
    }

    string outputDir = dset.output_dir;

    bool doWrite = true;
    if (_params.separate_output_dirs_by_scan_type) {
      outputDir += PATH_DELIM;
      if (_isSolarScan) {
        // special case
        outputDir += _params.sun_subdir;
        if (!_params.write_sun_files) {
          doWrite = false;
        }
      } else {
        switch (_scanMode) {
          case SCAN_MODE_RHI:
            outputDir += _params.rhi_subdir;
            if (!_params.write_rhi_files) {
              doWrite = false;
            }
            break;
          case SCAN_MODE_SECTOR:
            outputDir += _params.sector_subdir;
            if (!_params.write_sector_files) {
              doWrite = false;
            }
            break;
          case SCAN_MODE_VERT:
            outputDir += _params.vert_subdir;
            if (!_params.write_vert_files) {
              doWrite = false;
            }
            break;
          case SCAN_MODE_SUNSCAN:
          case SCAN_MODE_SUNSCAN_RHI:
            outputDir += _params.sun_subdir;
            if (!_params.write_sun_files) {
              doWrite = false;
            }
            break;
          case SCAN_MODE_PPI:
          default:
            outputDir += _params.surveillance_subdir;
            if (!_params.write_surveillance_files) {
              doWrite = false;
            }
        }
      }
    }
    if (!doWrite) {
      if (_params.debug) {
        cerr << "NOTE - Dsr2Radx::_doWrite(), nrays: " << _vol.getNRays() << endl;
        cerr << "  Skipping writing file for outputDir: " << outputDir << endl;
        switch (_scanMode) {
          case SCAN_MODE_RHI:
            cerr << "  parameter write_rhi_files is false" << endl;
            break;
          case SCAN_MODE_SECTOR:
            cerr << "  parameter write_sector_files is false" << endl;
            break;
          case SCAN_MODE_VERT:
            cerr << "  parameter write_vert_files is false" << endl;
            break;
          case SCAN_MODE_SUNSCAN:
          case SCAN_MODE_SUNSCAN_RHI:
            cerr << "  parameter write_sun_files is false" << endl;
            break;
          case SCAN_MODE_PPI:
          default:
            cerr << "  parameter write_surveillance_files is false" << endl;
        }
      }
      continue;
    }
    
    // check nrays in vol for volume-type formats

    // if (dset.format == Params::OUTPUT_FORMAT_UF ||
    //     dset.format == Params::OUTPUT_FORMAT_CFRADIAL) {
    //   if ((int) _vol.getNRays() < _params.min_rays_in_vol) {
    //     if (_params.debug) {
    //       cerr << "NOTE - Dsr2Radx::_doWrite(), nrays: " 
    //            << _vol.getNRays() << endl;
    //       cerr << "  dataType: " << dataType << endl;
    //       cerr << "  outputDir: " << outputDir << endl;
    //       cerr << "  too few rays, will not be saved" << endl;
    //     }
    //     continue;
    //   }
    // }

    // write out
    
    if (_outFile->writeToDir(_vol,
                             outputDir,
                             _params.append_day_dir_to_output_dir,
                             _params.append_year_dir_to_output_dir)) {

      cerr << "ERROR - Dsr2Radx::_doWrite()" << endl;
      cerr << _outFile->getErrStr() << endl;
      iret = -1;
      
    } else {

      // register the write with the DataMapper
      
      if (_params.write_individual_ldata_info) {
        for (size_t ipath = 0; ipath < _outFile->getWritePaths().size(); ipath++) {
          _writeLdataInfo(outputDir,
                          _outFile->getWritePaths()[ipath],
                          _outFile->getWriteDataTimes()[ipath],
                          dataType);
        }
      }
        
      // optionally register the master URL with the DataMapper
      
      if (_params.separate_output_dirs_by_scan_type &&
          _params.write_master_ldata_info) {
        for (size_t ipath = 0; ipath < _outFile->getWritePaths().size(); ipath++) {
          _writeLdataInfo(dset.output_dir,
                          _outFile->getWritePaths()[ipath],
                          _outFile->getWriteDataTimes()[ipath],
                          dataType);
        }
      }
      
    }

  } // idata

  return iret;

}

///////////////////////////////////////////////////
// write the master ldata info, as appropriate
// must call writeVol() successfully first
// Returns 0 on success, -1 on failure

int Dsr2Radx::_writeLdataInfo(const string &outputDir,
                              const string &outputPath,
                              time_t dataTime,
                              const string &dataType)
                                    

{

  LdataInfo *ldata = NULL;
  if (_params.register_with_data_mapper) {
    ldata = new DsLdataInfo;
  } else {
    ldata = new LdataInfo;
  }

  ldata->setDir(outputDir);
  ldata->setDataType(dataType);
        
  // compute relative data path
  
  string relPath;
  RadxPath::stripDir(ldata->getDataDirPath(), outputPath, relPath);
  ldata->setRelDataPath(relPath);

  RadxPath writePath(outputPath);
  ldata->setDataFileExt(writePath.getExt());
  ldata->setWriter("Legacy");
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    ldata->setDebug(true);
  }

  if (ldata->write(dataTime)) {
    cerr << "WARNING - Dsr2Radx::_writeLdataInfo" << endl;
    cerr << "  Cannot write LdataInfo to dir: "
         << outputDir << endl;
    delete ldata;
    return -1;
  }

  delete ldata;
  return 0;

}

////////////////////////////////////////////////////////////////
// update platform params

void Dsr2Radx::_updatePlatform(RadxRay *ray)

{

  const RadxPlatform &platform = _reader->getPlatform();
  _vol.setPlatform(platform);

  if (_params.override_radar_name) {
    _vol.setInstrumentName(_params.radar_name);
  }

  if(strlen(_params.site_name) > 0) {
    _vol.setSiteName(_params.site_name);
  }

  // radar location
  
  if (_params.override_radar_location) {
    _vol.overrideLocation(_params.radar_location.latitudeDeg,
                          _params.radar_location.longitudeDeg,
                          _params.radar_location.altitudeKm);
  }

}

////////////////////////////////////////////////////////////////
// update calibrations

void Dsr2Radx::_updateRcalib()

{

  const vector<RadxRcalib> &rcalibs = _reader->getRcalibs();
  _vol.clearRcalibs();
  for (size_t ii = 0; ii < rcalibs.size(); ii++) {
    RadxRcalib *calib = new RadxRcalib(rcalibs[ii]);
    _vol.addCalib(calib);
  }

}

/////////////////////////////////////////////////////////////////////////////
// compute difference between 2 angles: (a1 - a2)

double Dsr2Radx::_computeDeltaAngle(double a1, double a2)
{
  double diff = a1 - a2;
  if (diff < -180.0) {
    diff += 360.0;
  } else if (diff > 180.0) {
    diff -= 360.0;
  }
  return diff;
}

////////////////////////////////////////////////////////////////////
// censor an input ray

void Dsr2Radx::_censorInputRay(RadxRay *ray)

{

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
    Radx::fl32 fmiss = field->getMissingFl32();
    Radx::fl32 *fdata = (Radx::fl32 *) field->getData();
    for (size_t igate = 0; igate < nGates; igate++) {
      if (censorFlag[igate] == 1) {
        fdata[igate] = fmiss;
      }
    } // igate
  } // ifield
  
}

////////////////////////////////////////////////////////////////////
// prepare a ray for output, by applying the following procedures:
//
//  (a) ensure only output fields remain
//  (b) set standard name etc
//  (c) convert to output type

void Dsr2Radx::_prepareRayForOutput(RadxRay *ray)
  
{

  // remove fields which are not in the output field list

  vector<string> wantedNames;
  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {
    string dsrFieldName = _params._output_fields[ifield].dsr_name;
    wantedNames.push_back(dsrFieldName);
  } // ifield
  ray->trimToWantedFields(wantedNames);

  // set output names, pack data

  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {

    Params::output_field_t &ofld = _params._output_fields[ifield];
    string dsrFieldName = ofld.dsr_name;

    RadxField *field = ray->getField(dsrFieldName);
    if (field == NULL) {
      continue;
    }

    if (strlen(ofld.output_name) > 0) {
      field->setName(ofld.output_name);
    }
    if (strlen(ofld.long_name) > 0) {
      field->setLongName(ofld.long_name);
    }
    if (strlen(ofld.standard_name) > 0) {
      field->setStandardName(ofld.standard_name);
    }
    if (strlen(ofld.output_units) > 0) {
      field->setUnits(ofld.output_units);
    }

    if (ofld.output_encoding == Params::OUTPUT_SHORT) {
      field->convertToSi16();
    } else if (ofld.output_encoding == Params::OUTPUT_BYTE) {
      field->convertToSi08();
    } 

  } // ifield

  // load the field name map on the ray, because the field
  // names have changed

  ray->loadFieldNameMap();
  
}

////////////////////////////////////////////////////////////////////
// Do we need this field?

bool Dsr2Radx::_isCensoringField(const string &name)

{

  if (!_params.apply_censoring) {
    return false;
  }

  for (int ifield = 0; ifield < _params.censoring_fields_n; ifield++) {
    string censoringFieldName = _params._censoring_fields[ifield].name;
    if (name == censoringFieldName) {
      return true;
    }
  }

  return false;

}

bool Dsr2Radx::_isOutputField(const string &name)

{

  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {
    string dsrFieldName = _params._output_fields[ifield].dsr_name;
    if (name == dsrFieldName) {
      return true;
    }
  }
  
  return false;

}

////////////////////////////////////////////////////////////////////
// Add a ray, checking to see if we should
//
// Otherwise delete it

void Dsr2Radx::_addRayToVol(RadxRay *ray)

{
  if (_acceptRay(ray)) {
    _vol.addRay(ray);
    _prevRay = ray;
  } else {
    delete ray;
  }
}

////////////////////////////////////////////////////////////////////
// Check whether to accept ray

bool Dsr2Radx::_acceptRay(const RadxRay *ray)

{

  // check gate geometry

  if (_params.filter_using_gate_spacing) {
    double diff = fabs(_params.specified_gate_spacing - ray->getGateSpacingKm());
    if (diff > _smallVal) {
      return false;
    }
  }
  
  if (_params.filter_using_start_range) {
    double diff = fabs(_params.specified_start_range - ray->getStartRangeKm());
    if (diff > _smallVal) {
      return false;
    }
  }

  // check elevation

  if (_params.filter_using_elev) {
    double elev = ray->getElevationDeg();
    if (elev < _params.specified_min_elev ||
        elev > _params.specified_max_elev) {
      return false;
    }
  }
  
  // check sweep number

  if (_params.filter_using_sweep_number) {
    int sweepNum = ray->getSweepNumber();
    if (sweepNum < _params.specified_min_sweep_number ||
        sweepNum > _params.specified_max_sweep_number) {
      return false;
    }
  }
  
  if (_params.filter_using_sweep_number_list) {
    int sweepNum = ray->getSweepNumber();
    bool inList = false;
    for (int ii = 0; ii < _params.specified_sweep_number_list_n; ii++) {
      if (sweepNum == _params._specified_sweep_number_list[ii]) {
        inList = true;
      }
    }
    if (!inList) {
      return false;
    }
  }
  
  // check for scan in idle mode
  
  if (_params.filter_when_scan_idle) {
    if (ray->getSweepMode() == Radx::SWEEP_MODE_IDLE) {
      return false;
    }
  }

  // check for scan in pointing mode
  
  if (_params.filter_when_scan_pointing) {
    if (ray->getSweepMode() == Radx::SWEEP_MODE_POINTING) {
      return false;
    }
  }

  // check for antenna transitions
  
  if (_params.filter_antenna_transitions) {
    if (ray->getAntennaTransition()) {
      return false;
    }
  }

  // check for movement if required
  
  if (_params.filter_antenna_stationary) {
    if (!_isAntennaMoving(ray)) {
      if (_params.debug >= Params::DEBUG_EXTRA) {
	cerr << "WARNING - antenna not moving, will ignore ray" << endl;
	cerr << "  el, az: "
	     << ray->getElevationDeg() << ", "
	     << ray->getAzimuthDeg() << endl;
      }
      return false;
    }
  }

  return true;

}

////////////////////////////////////////////////////////////////
// is antenna moving

bool Dsr2Radx::_isAntennaMoving(const RadxRay *ray)

{

  double elev = ray->getElevationDeg();
  double az = ray->getAzimuthDeg();
  double deltaElev = fabs(elev - _prevElevMoving);
  double deltaAz = fabs(az - _prevAzMoving);
  bool isMoving;
  if (deltaElev > _params.min_angle_change_for_moving_antenna ||
      deltaAz > _params.min_angle_change_for_moving_antenna) {
    isMoving = true;
  } else {
    isMoving = false;
  }

  _prevElevMoving = elev;
  _prevAzMoving = az;

  return isMoving;

}

////////////////////////////////////////////////////////////////
// load the current scan mode
//
// returns 0 on success, -1 on failure

int Dsr2Radx::_loadCurrentScanMode()
  
{
  
  if (_vol.getNRays() == 0) {
    _scanMode = SCAN_MODE_UNKNOWN;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_NORM) {
    cerr << "loadCurrentScanMode()" << endl;
  }

  if (_params.use_input_scan_mode) {
    
    if (_findScanModeFromHeaders() == 0) {
      _scanInfoFromHeaders = true;
      _endOfVolAutomatic =
	(_params.end_of_vol_decision == Params::AUTOMATIC);
    } else {
      _findScanModeFromAntenna();
      _scanInfoFromHeaders = false;
      _endOfVolAutomatic = true;
    }

  } else {

    _findScanModeFromAntenna();
    _scanInfoFromHeaders = false;
    _endOfVolAutomatic =
      (_params.end_of_vol_decision == Params::AUTOMATIC);

  }

  // special case - vertical scan

  if (_isVertScan()) {
    _scanMode = SCAN_MODE_VERT;
  }

  if (_params.debug >= Params::DEBUG_NORM) {
    if (_scanMode == SCAN_MODE_RHI) {
      cerr << "  currentScanMode: RHI" << endl;
    } else if (_scanMode == SCAN_MODE_PPI) {
      cerr << "  currentScanMode: PPI" << endl;
    } else if (_scanMode == SCAN_MODE_SECTOR) {
      cerr << "  currentScanMode: SECTOR" << endl;
    } else if (_scanMode == SCAN_MODE_SURVEILLANCE) {
      cerr << "  currentScanMode: SURVEILLANCE" << endl;
    } else if (_scanMode == SCAN_MODE_VERT) {
      cerr << "  currentScanMode: VERT" << endl;
    } else {
      cerr << "  currentScanMode: UNKNOWN" << endl;
    }      
  }

  // set scan name
  // is this a solar?

  const vector<RadxRay *> rays = _vol.getRays();
  if (rays.size() > 0) {
    size_t nRays = rays.size();
    size_t midIndex = nRays / 2;
    RadxRay *midRay = rays[midIndex];
    string solarScanName(_params.solar_scan_name);
    string scanName(midRay->getScanName());
    if (scanName == solarScanName) {
      _isSolarScan = true;
    } else {
      _isSolarScan = false;
    }
    _vol.setScanName(scanName);
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// find the current scan mode from the data headers
//
// returns 0 on success, -1 on failure

int Dsr2Radx::_findScanModeFromHeaders()
  
{
  
  // find predominant scan mode
  
  double countSur = 0;
  double countSec = 0;
  double countRhi = 0;
  double countSun = 0;
  double countSunRhi = 0;
  double countTotal = 0;

  for (size_t iray = 0; iray < _vol.getNRays(); iray++) {
    RadxRay *ray = _vol.getRays()[iray];
    Radx::SweepMode_t sweepMode = ray->getSweepMode();
    if (sweepMode == Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE ||
	sweepMode == Radx::SWEEP_MODE_VERTICAL_POINTING) {
      countSur++;
    } else if (sweepMode == Radx::SWEEP_MODE_SECTOR) {
      countSec++;
    } else if (sweepMode == Radx::SWEEP_MODE_RHI) {
      countRhi++;
    } else if (sweepMode == Radx::SWEEP_MODE_SUNSCAN) {
      countSun++;
    } else if (sweepMode == Radx::SWEEP_MODE_SUNSCAN_RHI) {
      countSunRhi++;
    }
    countTotal++;
  }

  double fractionGood =
    (countSur + countSec + countRhi + countSun + countSunRhi) / countTotal;
  if (fractionGood < 0.8) {
    return -1;
  }
  
  if (countRhi > (countSur + countSec)) {
    _scanMode = SCAN_MODE_RHI;
  } else if (countSec > countSur) {
    _scanMode = SCAN_MODE_SECTOR;
  } else if (countSun > countSur) {
    _scanMode = SCAN_MODE_SUNSCAN;
  } else if (countSunRhi > countSur) {
    _scanMode = SCAN_MODE_SUNSCAN_RHI;
  } else {
    _scanMode = SCAN_MODE_SURVEILLANCE;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "findScanModeFromHeaders() ...." << endl;
    cerr << "  countSur: " << countSur << endl;
    cerr << "  countSec: " << countSec << endl;
    cerr << "  countRhi: " << countRhi << endl;
    cerr << "  countSun: " << countSun << endl;
  }
  
  return 0;

}

////////////////////////////////////////////////////////////////
// find the current scan mode from antenna angles

void Dsr2Radx::_findScanModeFromAntenna()
  
{
  
  // find which slews more - azimuth or elevation
  
  double azTravel = 0.0;
  double elTravel = 0.0;

  const vector<RadxRay *> rays = _vol.getRays();
  RadxRay *ray = rays[0];
  double prevEl = ray->getElevationDeg();
  double prevAz = ray->getAzimuthDeg();
  
  for (size_t iray = 1; iray < rays.size(); iray++) {
    RadxRay *ray = rays[iray];
    double el = ray->getElevationDeg();
    double az = ray->getAzimuthDeg();
    double elDiff = el - prevEl;
    if (elDiff > 180) {
      elDiff -= 360.0;
    } else if (elDiff < -180) {
      elDiff += 360.0;
    }
    double azDiff = az - prevAz;
    if (azDiff > 180) {
      azDiff -= 360.0;
    } else if (azDiff < -180) {
      azDiff += 360.0;
    }
    elTravel += fabs(elDiff);
    azTravel += fabs(azDiff);
    prevEl = el;
    prevAz = az;
  }
  
  // at this point we can only determine whether it is an
  // RHI or not. In PpiMgr, we can determine whether it is a
  // sector or surveillance
  
  if (elTravel > azTravel) {
    _scanMode = SCAN_MODE_RHI;
  } else {
    _scanMode = SCAN_MODE_PPI; // generic non-rhi mode
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "findScanModeFromAntenna() ...." << endl;
    cerr << "  elTravel: " << elTravel << endl;
    cerr << "  azTravel: " << azTravel << endl;
    if (_scanMode == SCAN_MODE_RHI) {
      cerr << "    RHI mode" << endl;
    } else {
      cerr << "    PPI mode" << endl;
    }
  }

}

////////////////////////////////////////////////////////////////
// check if this is a vertically-pointing scan

bool Dsr2Radx::_isVertScan()
  
{

  // check if this is even an option

  if (!_params.separate_output_dirs_by_scan_type) {
    return false;
  }

  // count number of beams above min elevation
  
  double countAbove = 0.0;
  double countTotal = 0.0;

  const vector<RadxRay *> rays = _vol.getRays();
  for (size_t iray = 1; iray < rays.size(); iray++) {
    RadxRay *ray = rays[iray];
    double el = ray->getElevationDeg();
    if (el > _params.min_elevation_for_vert_files) {
      countAbove++;
    }
    countTotal++;
  }

  double fractionAbove = countAbove / countTotal;

  if (fractionAbove > _params.min_vert_fraction_for_vert_files) {
    return true;
  }

  return false;

}

/////////////////////////////////////////////////////////////////
// clear methods

void Dsr2Radx::_clearData()

{
  _vol.clear();
  _sweepNumbersMissing = false;
  _nRaysRead = 0;
  _prevRay = NULL;
}

////////////////////////////////////////////////////
// compute the number of gates given the max height

int Dsr2Radx::_computeNgatesForMaxHt(double elev,
                                     double radarAltitudeKm,
                                     double startRangeKm,
                                     double gateSpacingKm)

{

  // make sure lookup table is up to date

  _computeMaxRangeLut(radarAltitudeKm);

  // condition elevation angle

  if (elev > 90.0) {
    elev = 180.0 - elev;
  }

  // negative elevation angle?
  
  if (elev <= 0) {
    // no limit to number of gates
    return 1000000;
  }

  // get range from lookup table

  int ielev = (int) elev;
  if (ielev > 89) {
    ielev = 89;
  } else if (ielev < 0) {
    ielev = 0;
  }

  double maxRangeKm = _maxRangeLut[ielev];

  // compute number of gates

  int maxGates = (int) ((maxRangeKm - startRangeKm) / gateSpacingKm + 1.0);

  return maxGates;

}

/////////////////////////////////////////////////////////////////
// compute lookup table for max range at various elevation angles
	
void Dsr2Radx::_computeMaxRangeLut(double radarAltitudeKm)

{

  if (fabs(_lutRadarAltitudeKm - radarAltitudeKm) < 0.001) {
    // previously computed
    return;
  }

  // fill lookup table

  double maxHtKm = _params.max_height_km_msl;

  for (int ielev = 0; ielev < 90; ielev++) {
    
    double elev = ielev + 0.01;
    double sinElev = sin(elev * DEG_TO_RAD);

    for (double rangeKm = 1.0; rangeKm < 2000; rangeKm++) {
      double htKm = (radarAltitudeKm +
                     (rangeKm * sinElev) +
                     ((rangeKm * rangeKm) / _pseudoDiam)); 
      _maxRangeLut[ielev] = rangeKm;
      if (htKm > maxHtKm) {
        break;
      }
    } // rangeKm

  } // ielev

  _lutRadarAltitudeKm = radarAltitudeKm;

}

/////////////////////////////////////////////////////////////////
// compute time for end of volume
	
void Dsr2Radx::_computeEndOfVolTime(time_t beamTime)

{

  int secsPerDay = 86400;
  int jDay = beamTime / secsPerDay;
  int secInDay = beamTime % secsPerDay;
  int volInDay = secInDay / _params.nsecs_per_volume;
  int endOfVolSec = (volInDay + 1) * _params.nsecs_per_volume;
  if (endOfVolSec > secsPerDay) {
    endOfVolSec = secsPerDay;
  }
  _endOfVolTime = jDay * secsPerDay + endOfVolSec;
  
  if (_params.debug) {
    cerr << "==>> end_of_vol_decision : ELAPSED_TIME <<==" << endl;
    cerr << "==>> Next end of vol time: " <<
      RadxTime::strm(_endOfVolTime) << endl;
  }

}

/////////////////////////////////////////////////////////////////
// check for end of vol in 360_deg mode
// we check for az passing az_for_end_of_vol_360
	
bool Dsr2Radx::_checkEndOfVol360(RadxRay *ray)

{
  
  if (_prevRay == NULL) {
    // no previous ray in this volume
    return false;
  }
  
  // get azimuths for this ray and the previous one

  double az = ray->getAzimuthDeg();
  double prevAz = _prevRay->getAzimuthDeg();

  // compute az diffs between these and the az at which the vol changes
  
  double azDiff =
    Radx::computeAngleDiff(az, _params.az_for_end_of_vol_360);
  double prevAzDiff =
    Radx::computeAngleDiff(prevAz, _params.az_for_end_of_vol_360);

  // if exact, then we are at the target az

  if (azDiff == 0.0 || prevAzDiff == 0.0) {
    return true;
  }

  // if diff is too large, we are not close to the target
  
  if (fabs(azDiff) > 10.0 || fabs(prevAzDiff) > 10.0) {
    return false;
  }

  // if the sign changed from one diff to the next
  // we have crossed the target az

  if (azDiff * prevAzDiff < 0.0) {
    return true;
  }

  // failure

  return false;

}
