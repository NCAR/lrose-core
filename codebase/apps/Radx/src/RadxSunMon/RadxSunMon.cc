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
// RadxSunMon.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2015
//
///////////////////////////////////////////////////////////////
//
// Searches for sun-spikes in radar rays, compute sun stats
// and saves to SPDB
//
////////////////////////////////////////////////////////////////

#include "RadxSunMon.hh"
#include <Radx/Radx.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxXml.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxArray.hh>
#include <radar/RadarCalib.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <Spdb/DsSpdb.hh>
#include <rapformats/WxObs.hh>
using namespace std;

// Constructor

RadxSunMon::RadxSunMon(int argc, char **argv)
  
{

  OK = TRUE;

  // set programe name

  _progName = "RadxSunMon";
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

  // check params

  if (_params.power_correction_curve_n < 5) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem with TDRP parameters." << endl;
    cerr << "  You must have at least 5 points in the power_correction_curve" << endl;
    OK = FALSE;
    return;
  }

  // initialize noise location

  if (_noiseInit()) {
    OK = FALSE;
    return;
  }

  // init stats

  _sumOffsetWeights = 0.0;
  _sumElOffset = 0.0;
  _sumAzOffset = 0.0;
  _meanElOffset = MomentsFields::missingDouble;
  _meanAzOffset = MomentsFields::missingDouble;

}

// destructor

RadxSunMon::~RadxSunMon()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RadxSunMon::Run()
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

int RadxSunMon::_runFilelist()
{
  
  int iret = 0;
  
  // loop through the input file list
  
  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {
    string inputPath = _args.inputFileList[ii];
    // read input moments file
    if (_readMoments(inputPath) == 0) {
      // process it
      if (_processVol()) {
        cerr << "ERROR - RadxSunMon::_runFileList" << endl;
        return -1;
      }
    } else {
      iret = -1;
    }
    _momentsVol.clear();
  }
  
  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int RadxSunMon::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.moments_input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - RadxSunMon::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.moments_input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - RadxSunMon::_runFilelist()" << endl;
    cerr << "  No files found, dir: " << _params.moments_input_dir << endl;
    return -1;
  }
  
  // loop through the input file list

  int iret = 0;
  for (size_t ii = 0; ii < paths.size(); ii++) {
    // read input file
    if (_readMoments(paths[ii]) == 0) {
      if (_processVol()) {
        cerr << "ERROR - RadxSunMon::_runArchive" << endl;
        return -1;
      }
    } else {
      iret = -1;
    }
    // free up
    _momentsVol.clear();
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode
// assumes latest_data_info is available

int RadxSunMon::_runRealtime()
{

  // init process mapper registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  // watch for new data to arrive

  LdataInfo ldata(_params.moments_input_dir,
                  _params.debug >= Params::DEBUG_VERBOSE);
  if (strlen(_params.search_ext) > 0) {
    ldata.setDataFileExt(_params.search_ext);
  }
  
  RadxVol vol;
  int iret = 0;
  int msecsWait = 1000;
  while (true) {
    ldata.readBlocking(_params.max_realtime_data_age_secs,
                       msecsWait, PMU_auto_register);
    const string path = ldata.getDataPath();
    // read input file
    if (_readMoments(path) == 0) {
      if (_processVol()) {
        cerr << "ERROR - RadxSunMon::_runRealtime" << endl;
        return -1;
      }
    } else {
      iret = -1;
    }
    // free up
    _momentsVol.clear();
  }

  return iret;

}

//////////////////////////////////////////////////
// Read in a moments file
// Returns 0 on success,  -1 on failure

int RadxSunMon::_readMoments(const string &readPath)
{
  
  PMU_auto_register("Reading moments");

  if (_params.debug) {
    cerr << "INFO - RadxSunMon::Run" << endl;
    cerr << "  Input path: " << readPath << endl;
  }
  
  // clear all data on volume object
  
  _momentsVol.clear();
  
  // set up read parameters

  GenericRadxFile inFile;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    inFile.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    inFile.setVerbose(true);
  }

  if (_params.estimate_dbmhc_from_dbz) {
    inFile.addReadField(_params.dbz_field_name);
  } else {
    inFile.addReadField(_params.dbmhc_field_name);
  }

  if (strlen(_params.dbmvc_field_name) > 0) {
    inFile.addReadField(_params.dbmvc_field_name);
  }
  if (strlen(_params.dbmhx_field_name) > 0) {
    inFile.addReadField(_params.dbmhx_field_name);
  }
  if (strlen(_params.dbmvx_field_name) > 0) {
    inFile.addReadField(_params.dbmvx_field_name);
  }

  inFile.addReadField(_params.vel_field_name);

  if (_params.estimate_ncp_from_spectrum_width) {
    inFile.addReadField(_params.width_field_name);
  } else {
    inFile.addReadField(_params.ncp_field_name);
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    inFile.printReadRequest(cerr);
  }

  // read in file
  
  if (inFile.readFromPath(readPath, _momentsVol)) {
    cerr << "ERROR - RadxSunMon::_readMoments" << endl;
    cerr << "  path: " << readPath << endl;
    cerr << inFile.getErrStr() << endl;
    cerr << "  Check that the following fields are in the moments data:" << endl;
    if (_params.estimate_dbmhc_from_dbz) {
      cerr << "  " << _params.dbz_field_name << endl;
    }
    if (strlen(_params.dbmhc_field_name) > 0) {
      cerr << "  " << _params.dbmhc_field_name << endl;
    }
    if (strlen(_params.dbmvc_field_name) > 0) {
      cerr << "  " << _params.dbmvc_field_name << endl;
    }
    if (strlen(_params.dbmhx_field_name) > 0) {
      cerr << "  " << _params.dbmhx_field_name << endl;
    }
    if (strlen(_params.dbmvx_field_name) > 0) {
      cerr << "  " << _params.dbmvx_field_name << endl;
    }
    return -1;
  }

  if (_momentsVol.getNRays() < 1) {
    cerr << "ERROR - RadxSunMon::_readMoments" << endl;
    cerr << "  path: " << readPath << endl;
    cerr << "  no rays found" << endl;
    return -1;
  }

  // override radar location if requested
  
  if (_params.override_radar_location) {
    if (_params.change_radar_latitude_sign) {
      _momentsVol.overrideLocation(_params.radar_latitude_deg * -1.0,
                                   _params.radar_longitude_deg,
                                   _params.radar_altitude_meters / 1000.0);
    } else {
      _momentsVol.overrideLocation(_params.radar_latitude_deg,
                                   _params.radar_longitude_deg,
                                   _params.radar_altitude_meters / 1000.0);
    }
  }
    
  // convert to floats

  _momentsVol.convertToFl32();
  _momentsPath = inFile.getPathInUse();

  if (_params.debug) {
    cerr << "Read moments file: " << _momentsPath << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// process a moments volume file

int RadxSunMon::_processVol()
{

  // calibration

  if (_momentsVol.getNRcalibs() > 0) {
    const vector<RadxRcalib *> rcalibs = _momentsVol.getRcalibs();
    RadarCalib::copyRadxToIwrf(*(rcalibs[rcalibs.size()-1]), _calib);
    // cerr << "11111111111111111111111111111111" << endl;
    // cerr << "111111111111 _momentsVol.getNRcalibs(): " << _momentsVol.getNRcalibs() << endl;
    // rcalibs[rcalibs.size()-1]->print(cerr);
    // cerr << "11111111111111111111111111111111" << endl;
  }

  _radarConstDb = _params.radar_constant_db;
  _atmosAtten.setAttenCrpl(_momentsVol.getWavelengthCm());

  // sun posn init

  _sunPosn.setLocation(_momentsVol.getLatitudeDeg(),
                       _momentsVol.getLongitudeDeg(),
                       _momentsVol.getAltitudeKm() * 1000.0);

  // option to get site temperature
  
  if (_params.read_site_temp_from_spdb) {
    if (_retrieveSiteTempFromSpdb(_momentsVol,
                                  _siteTempC,
                                  _timeForSiteTemp) == 0) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> site tempC: " 
             << _siteTempC << " at " << RadxTime::strm(_timeForSiteTemp) << endl;
      }
    }
  }

  // loop through all rays in vol
  
  bool haveXpolRatio = false;
  const vector<RadxRay *> &rays = _momentsVol.getRays();
  RadxTime sunTime(RadxTime::ZERO);
  
  for (size_t ii = 0; ii < rays.size(); ii++) {

    RadxRay *ray = rays[ii];
    _rayTime = ray->getRadxTime();

    // compute sun location every 10 secs
    
    if (_rayTime - sunTime > 10) {
      _sunPosn.computePosnNova(_rayTime.asDouble(), _elSun, _azSun);
      sunTime = _rayTime;
    }

    if (_elSun < _params.sun_analysis_min_elevation_deg) {
      // sun too low
      continue;
    }

    // init
    
    _nGates = ray->getNGates();
    _startRangeKm = ray->getStartRangeKm();
    _gateSpacingKm = ray->getGateSpacingKm();
    _radarHtKm = _momentsVol.getAltitudeKm();
    _azimuth = ray->getAzimuthDeg();
    _elevation = ray->getElevationDeg();
    _timeSecs = ray->getTimeSecs();
    _nanoSecs = ray->getNanoSecs();
    
    if (ii == 0) {
      _prevEl = _elevation;
      _prevAz = _azimuth;
    }

    // check relative to sun position
    // only process rays close to sun

    _elOffset = _elevation - _elSun;
    double dAz = _azimuth - _azSun;
    if (dAz < -180) {
      dAz += 360.0;
    } else if (dAz > 180) {
      dAz -= 360.0;
    }
    _azOffset = dAz * cos(_elevation * Radx::DegToRad);
    _angleOffset = sqrt(_elOffset * _elOffset + _azOffset * _azOffset);

    if (fabs(_elOffset) > _params.elevation_search_margin_deg ||
        fabs(_azOffset) > _params.azimuth_search_margin_deg) {
      if (_params.debug >= Params::DEBUG_EXTRA) {
        cerr << "==>> Ignoring ray, time, el, az: "
             << _rayTime.asString(3) << ", " << _elevation << ", " << _azimuth << endl;
        cerr << "  sun el, az: " << _elSun << ", " << _azSun << endl;
        cerr << "  _elOffset, _azOffset: " << _elOffset << ", " << _azOffset << endl;
      }
      continue;
    }

    if (_processRay(*ray) == 0) {

      // success

      // optionally compute cross-polar power ratio in clutter

      if (_params.compute_cross_pol_ratio_in_clutter) {
        if (!haveXpolRatio) {
          _computeXpolRatio();
          haveXpolRatio = true;
        }
        _zdrmDb = NAN;
        if (_S1S2 != MomentsFields::missingDouble) {
          _zdrmDb = -1.0 * (_S1S2 + _meanXpolRatioDb);
        } else if (_SS != MomentsFields::missingDouble) {
          _zdrmDb = -1.0 * (_SS + _meanXpolRatioDb);
        }
      }

      // write out
      
      if (_params.write_results_to_spdb) {
        if (_writeResultsToSpdb()) {
          cerr << "ERROR - RadxSunMon::_processVol" << endl;
          cerr << "  Cannot write results to spdb" << endl;
        }
      }

    }

    _prevEl = _elevation;
    _prevAz = _azimuth;

  } // ii

  return 0;

}

/////////////////////////////////////////////////////////////////////
// process ray
// returns 0 on success (found sun), -1 on failure (did not find sun)

int RadxSunMon::_processRay(const RadxRay &ray)
  
{

  // init
  
  _measuredDbmHc = MomentsFields::missingDouble;
  _measuredDbmVc = MomentsFields::missingDouble;
  _measuredDbmHx = MomentsFields::missingDouble;
  _measuredDbmVx = MomentsFields::missingDouble;

  _correctedDbmHc = MomentsFields::missingDouble;
  _correctedDbmVc = MomentsFields::missingDouble;
  _correctedDbmHx = MomentsFields::missingDouble;
  _correctedDbmVx = MomentsFields::missingDouble;

  _S1S2 = _SS = MomentsFields::missingDouble;

  _offSunPowerCorrectionDb = MomentsFields::missingDouble;
  _oneWayAtmosAttenDb = MomentsFields::missingDouble;
  _appliedPowerCorrectionDb = MomentsFields::missingDouble;
  _offsetWeight = MomentsFields::missingDouble;

  _noise.setRayProps(_nGates, _calib,
                     _timeSecs, _nanoSecs,
                     _elevation, _azimuth);
  
  // create and initialize moments array

  RadxArray<MomentsFields> mfields_;
  MomentsFields *mfields = mfields_.alloc(_nGates);
  for (size_t ii = 0; ii < _nGates; ii++) {
    mfields[ii].init();
  }

  // load up mfields

  // co-polar power h channel

  if (_params.estimate_dbmhc_from_dbz) {
    const RadxField *dbzFld = ray.getField(_params.dbz_field_name);
    if (dbzFld == NULL) {
      cerr << "ERROR - RadxSunMon::_processRay" << endl;
      cerr << "  dbz field missing: " << _params.dbz_field_name << endl;
      return -1;
    }
    const Radx::fl32 *dbzArray = dbzFld->getDataFl32();
    Radx::fl32 dbzMiss = dbzFld->getMissingFl32();
    double rangeKm = _startRangeKm;
    for (size_t ii = 0; ii < _nGates; ii++, rangeKm += _gateSpacingKm) {
      Radx::fl32 dbz = dbzArray[ii];
      if (dbz != dbzMiss) {
        mfields[ii].dbm_for_noise = _estimatePowerFromDbz(dbz, _elevation, rangeKm);
        mfields[ii].lag0_hc_db = mfields[ii].dbm_for_noise;
      }
    }
  } else {
    const RadxField *dbmhcFld = ray.getField(_params.dbmhc_field_name);
    if (dbmhcFld == NULL) {
      cerr << "ERROR - RadxSunMon::_processRay" << endl;
      cerr << "  dbmhc field missing: " << _params.dbmhc_field_name << endl;
      return -1;
    }
    const Radx::fl32 *dbmhcArray = dbmhcFld->getDataFl32();
    Radx::fl32 dbmhcMiss = dbmhcFld->getMissingFl32();
    for (size_t ii = 0; ii < _nGates; ii++) {
      Radx::fl32 dbmhc = dbmhcArray[ii];
      if (dbmhc != dbmhcMiss) {
        mfields[ii].dbm_for_noise = dbmhc;
        mfields[ii].lag0_hc_db = dbmhc;
      }
    }
  }

  // co-polar power v channel

  const RadxField *dbmvcFld = ray.getField(_params.dbmvc_field_name);
  if (dbmvcFld != NULL) {
    const Radx::fl32 *dbmvcArray = dbmvcFld->getDataFl32();
    Radx::fl32 dbmvcMiss = dbmvcFld->getMissingFl32();
    for (size_t ii = 0; ii < _nGates; ii++) {
      Radx::fl32 dbmvc = dbmvcArray[ii];
      if (dbmvc != dbmvcMiss) {
        mfields[ii].lag0_vc_db = dbmvc;
      }
    }
  }

  // cross-polar power h channel

  const RadxField *dbmhxFld = ray.getField(_params.dbmhx_field_name);
  if (dbmhxFld != NULL) {
    const Radx::fl32 *dbmhxArray = dbmhxFld->getDataFl32();
    Radx::fl32 dbmhxMiss = dbmhxFld->getMissingFl32();
    for (size_t ii = 0; ii < _nGates; ii++) {
      Radx::fl32 dbmhx = dbmhxArray[ii];
      if (dbmhx != dbmhxMiss) {
        mfields[ii].lag0_hx_db = dbmhx;
      }
    }
  }

  // cross-polar power v channel

  const RadxField *dbmvxFld = ray.getField(_params.dbmvx_field_name);
  if (dbmvxFld != NULL) {
    const Radx::fl32 *dbmvxArray = dbmvxFld->getDataFl32();
    Radx::fl32 dbmvxMiss = dbmvxFld->getMissingFl32();
    for (size_t ii = 0; ii < _nGates; ii++) {
      Radx::fl32 dbmvx = dbmvxArray[ii];
      if (dbmvx != dbmvxMiss) {
        mfields[ii].lag0_vx_db = dbmvx;
      }
    }
  }

  // velocity
  
  const RadxField *velFld = ray.getField(_params.vel_field_name);
  if (velFld == NULL) {
    cerr << "ERROR - RadxSunMon::_processRay" << endl;
    cerr << "  vel field missing: " << _params.vel_field_name << endl;
    return -1;
  }
  const Radx::fl32 *velArray = velFld->getDataFl32();
  Radx::fl32 velMiss = velFld->getMissingFl32();
  for (size_t ii = 0; ii < _nGates; ii++) {
    Radx::fl32 vel = velArray[ii];
    if (vel != velMiss) {
      mfields[ii].phase_for_noise = _getPhaseFromVel(vel);
      mfields[ii].vel = vel;
    }
  }

  // ncp
  
  if (_params.estimate_ncp_from_spectrum_width) {
    const RadxField *widthFld = ray.getField(_params.width_field_name);
    if (widthFld == NULL) {
      cerr << "ERROR - RadxSunMon::_processRay" << endl;
      cerr << "  width field missing: " << _params.width_field_name << endl;
      return -1;
    }
    const Radx::fl32 *widthArray = widthFld->getDataFl32();
    Radx::fl32 widthMiss = widthFld->getMissingFl32();
    for (size_t ii = 0; ii < _nGates; ii++) {
      Radx::fl32 width = widthArray[ii];
      if (width != widthMiss) {
        mfields[ii].ncp = _estimateNcpFromWidth(width);
        mfields[ii].width = width;
      }
    }
  } else {
    const RadxField *ncpFld = ray.getField(_params.ncp_field_name);
    if (ncpFld == NULL) {
      cerr << "ERROR - RadxSunMon::_processRay" << endl;
      cerr << "  ncp field missing: " << _params.ncp_field_name << endl;
      return -1;
    }
    const Radx::fl32 *ncpArray = ncpFld->getDataFl32();
    Radx::fl32 ncpMiss = ncpFld->getMissingFl32();
    for (size_t ii = 0; ii < _nGates; ii++) {
      Radx::fl32 ncp = ncpArray[ii];
      if (ncp != ncpMiss) {
        mfields[ii].ncp = ncp;
      }
    }
  }

  // locate the noise gates

  _noise.locate(mfields);
  const vector<bool> &noiseFlag = _noise.getNoiseFlag();
  int nNoiseGates = 0;
  for (size_t ii = 0; ii < noiseFlag.size(); ii++) {
    if (noiseFlag[ii]) {
      nNoiseGates++;
    }
  }
  if (nNoiseGates < _params.noise_min_ngates_for_ray_median) {
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "==>> Ignoring ray, el, az: "
           << _elevation << ", " << _azimuth << endl;
      cerr << "  Too few noise gates: " << nNoiseGates << endl;
    }
    return -1;
  }

  // compute the mean powers in the noise gates

  double sumDbmHc = 0.0;
  double sumDbmVc = 0.0;
  double sumDbmHx = 0.0;
  double sumDbmVx = 0.0;

  double countDbmHc = 0.0;
  double countDbmVc = 0.0;
  double countDbmHx = 0.0;
  double countDbmVx = 0.0;

  for (size_t ii = 0; ii < noiseFlag.size(); ii++) {

    if (!noiseFlag[ii]) {
      continue;
    }

    const MomentsFields &mfield = mfields[ii];

    if (mfield.lag0_hc_db != MomentsFields::missingDouble) {
      sumDbmHc += mfield.lag0_hc_db;
      countDbmHc++;
    }

    if (mfield.lag0_vc_db != MomentsFields::missingDouble) {
      sumDbmVc += mfield.lag0_vc_db;
      countDbmVc++;
    }

    if (mfield.lag0_hx_db != MomentsFields::missingDouble) {
      sumDbmHx += mfield.lag0_hx_db;
      countDbmHx++;
    }

    if (mfield.lag0_vx_db != MomentsFields::missingDouble) {
      sumDbmVx += mfield.lag0_vx_db;
      countDbmVx++;
    }

  }
  
  if (countDbmHc > 0) {
    _measuredDbmHc = (sumDbmHc / countDbmHc) + _calib.getReceiverGainDbHc();
  }
  if (countDbmVc > 0) {
    _measuredDbmVc = (sumDbmVc / countDbmVc) + _calib.getReceiverGainDbVc();
  }
  if (countDbmHx > 0) {
    _measuredDbmHx = (sumDbmHx / countDbmHx) + _calib.getReceiverGainDbHx();
  }
  if (countDbmVx > 0) {
    _measuredDbmVx = (sumDbmVx / countDbmVx) + _calib.getReceiverGainDbVx();
  }

  // compute mean power correction over the dwell

  _offSunPowerCorrectionDb = _computeDwellPowerCorrection();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> Adjusting powers for angle offset" << endl;
    cerr << "  Off-sun power correction (dB): " << _offSunPowerCorrectionDb << endl;
  }
  _appliedPowerCorrectionDb = _offSunPowerCorrectionDb;
  
  // we can also account for attenuation
  // we assume a 1000km path
  
  double sunPathKm = 1000.0;
  _oneWayAtmosAttenDb = _atmosAtten.getAtten(_elSun, sunPathKm) / 2.0;
  
  if (_params.correct_measured_powers_for_atmos_atten) {
    _appliedPowerCorrectionDb += _oneWayAtmosAttenDb;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "==>> Adjusting powers for atmos atten, elev: " << _elevation << endl;
      cerr << "     One way atmos atten (dB): " << _oneWayAtmosAttenDb << endl;
    }
  }

  if (_angleOffset <= _params.max_angle_offset_for_power_estimation) {
    if (_measuredDbmHc != MomentsFields::missingDouble) {
      _correctedDbmHc = _measuredDbmHc + _appliedPowerCorrectionDb;
    }
    if (_measuredDbmVc != MomentsFields::missingDouble) {
      _correctedDbmVc = _measuredDbmVc + _appliedPowerCorrectionDb;
    }
    if (_measuredDbmHx != MomentsFields::missingDouble) {
      _correctedDbmHx = _measuredDbmHx + _appliedPowerCorrectionDb;
    }
    if (_measuredDbmVx != MomentsFields::missingDouble) {
      _correctedDbmVx = _measuredDbmVx + _appliedPowerCorrectionDb;
    }
    if (_measuredDbmHc != MomentsFields::missingDouble &&
        _measuredDbmVc != MomentsFields::missingDouble &&
        _measuredDbmHx != MomentsFields::missingDouble &&
        _measuredDbmVx != MomentsFields::missingDouble) {
      _S1S2 = (_measuredDbmVc - _measuredDbmHc) + (_measuredDbmVx - _measuredDbmHx);
    } else if (_measuredDbmHc != MomentsFields::missingDouble &&
               _measuredDbmVc != MomentsFields::missingDouble) {
      _SS = 2.0 * (_measuredDbmVc - _measuredDbmHc);
    }
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "======>> Found sun spike <<======" << endl;
    cerr << "  time, ray el, az: "
         << _rayTime.asString(3) << ", " << _elevation << ", " << _azimuth << endl;
    cerr << "  sun el, az: " << _elSun << ", " << _azimuth << endl;
    cerr << "  _elOffset, _azOffset, _angleOffset: "
         << _elOffset << ", " << _azOffset << ", " << _angleOffset << endl;
    cerr << "  _oneWayAtmosAttenDb: " << _oneWayAtmosAttenDb << endl;
    cerr << "  _offSunPowerCorrectionDb: " << _offSunPowerCorrectionDb << endl;
    cerr << "  _appliedPowerCorrectionDb: " << _appliedPowerCorrectionDb << endl;
    if (_measuredDbmHc != MomentsFields::missingDouble) {
      cerr << "  measured, corrected DbmHc: "
           << _measuredDbmHc << ", " << _correctedDbmHc << endl;
    }
    if (_measuredDbmVc != MomentsFields::missingDouble) {
      cerr << "  measured, corrected DbmVc: "
           << _measuredDbmVc << ", " << _correctedDbmVc << endl;
    }
    if (_measuredDbmHx != MomentsFields::missingDouble) {
      cerr << "  measured, corrected DbmHx: "
           << _measuredDbmHx << ", " << _correctedDbmHx << endl;
    }
    if (_measuredDbmVx != MomentsFields::missingDouble) {
      cerr << "  measured, corrected DbmVx: "
           << _measuredDbmVx << ", " << _correctedDbmVx << endl;
    }
  }

  
  if (_angleOffset <= _params.max_angle_offset_for_pointing_estimation) {
    if (_correctedDbmHc != MomentsFields::missingDouble) {
      _offsetWeight = 1.0 / fabs(_params.theoretical_max_dbmhc - _correctedDbmHc);
      _sumOffsetWeights += _offsetWeight;
      _sumElOffset += _elOffset * _offsetWeight;
      _sumAzOffset = _azOffset * _offsetWeight;
      _meanElOffset = _sumElOffset / _sumOffsetWeights;
      _meanAzOffset = _sumAzOffset / _sumOffsetWeights;
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "  Offset weight: " << _offsetWeight << endl;
        cerr << "  Running mean el offset: " << _meanElOffset << endl;
        cerr << "  Running mean az offset: " << _meanAzOffset << endl;
      }
    }
  }

  if (_angleOffset > _params.max_angle_offset_for_power_estimation &&
      _angleOffset > _params.max_angle_offset_for_pointing_estimation) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "NOTE - angle offset exceeds limit for power and pointing estimation" << endl;
      cerr << "  Ignoring this ray" << endl;
      return -1;
    }
  }

  return 0;

}

//////////////////////////////////////
// initialize noise computations
  
int RadxSunMon::_noiseInit()
  
{

  // initialize noise location
  
  _noise.setNGatesKernel(_params.noise_ngates_kernel);
  _noise.setComputeRayMedian(_params.noise_min_ngates_for_ray_median);

  // interest maps for for noise
  
  vector<InterestMap::ImPoint> pts;
  if (_convertInterestParamsToVector
      ("phase_change_error_for_noise",
       _params._phase_change_error_for_noise_interest_map,
       _params.phase_change_error_for_noise_interest_map_n,
       pts)) {
    return -1;
  }
  _noise.setInterestMapPhaseChangeErrorForNoise
    (pts, _params.phase_change_error_for_noise_interest_weight);

  if (_convertInterestParamsToVector
      ("dbm_sdev_for_noise",
       _params._dbm_sdev_for_noise_interest_map,
       _params.dbm_sdev_for_noise_interest_map_n,
       pts)) {
    return -1;
  }
  _noise.setInterestMapDbmSdevForNoise
    (pts, _params.dbm_sdev_for_noise_interest_weight);

  if (_convertInterestParamsToVector
      ("ncp_mean_sdev_for_noise",
       _params._ncp_mean_for_noise_interest_map,
       _params.ncp_mean_for_noise_interest_map_n,
       pts)) {
    return -1;
  }
  _noise.setInterestMapNcpMeanForNoise
    (pts, _params.ncp_mean_for_noise_interest_weight);

  _noise.setInterestThresholdForNoise
    (_params.interest_threshold_for_noise);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _noise.printParams(cerr);
  }

  return 0;

}

////////////////////////////////////////////////////////////////////////
// Convert interest map points to vector
//
// Returns 0 on success, -1 on failure

int RadxSunMon::_convertInterestParamsToVector(const string &label,
                                               const Params::interest_map_point_t *map,
                                               int nPoints,
                                               vector<InterestMap::ImPoint> &pts)

{
  
  pts.clear();
  
  double prevVal = -1.0e99;
  for (int ii = 0; ii < nPoints; ii++) {
    if (map[ii].value <= prevVal) {
      cerr << "ERROR - RadxSunMon::_convertInterestParamsToVector" << endl;
      cerr << "  Map label: " << label << endl;
      cerr << "  Map values must increase monotonically" << endl;
      return -1;
    }
    InterestMap::ImPoint pt(map[ii].value, map[ii].interest);
    pts.push_back(pt);
    prevVal = map[ii].value;
  } // ii
  
  return 0;

}

////////////////////////////////////////////////////////////////////////
// Get phase from velocity

double RadxSunMon::_getPhaseFromVel(double vel)

{
  double phase = (vel / _nyquist) * 180.0;
  return phase;
}
////////////////////////////////////////////////////////////////////////
// Estimate ncp from spectrum width and nyquist

double RadxSunMon::_estimateNcpFromWidth(double width)

{
  double ncp = exp(-1.0 * pow((4.0 * width / _nyquist), 2.0));
  return ncp;
}

////////////////////////////////////////////////////////////////////////
// Estimate power from dbz and range

double RadxSunMon::_estimatePowerFromDbz(double dbz, double elev, double rangeKm)

{
  double powerDbm = dbz - 20.0 * log(rangeKm) + _radarConstDb;
  if (_params.use_atmos_atten_in_dbmhc_estimation) {
    powerDbm += _atmosAtten.getAtten(elev, rangeKm);
  }
  return powerDbm;
}

////////////////////////////////////////////////////////////////////////
// Compute mean dwell power correction based on el and az error

double RadxSunMon::_computeDwellPowerCorrection()

{

  // integrate over a dwell
  // using movement from prev beam to this one
  
  double dEl = _elevation - _prevEl;
  double dAz = _azimuth - _prevAz;
  if (dAz < -180.0) {
    dAz += 360.0;
  } else if (dAz > 180.0) {
    dAz -= 360.0;
  }
  if (fabs(dEl) > 10.0 || fabs(dAz) > 10.0) {
    // invalid dwell
    return 0.0;
  }
  
  // we integrate over half a dwell before the current location
  // and half a dwell after

  double sum = 0.0;
  int nn = 100;
  double dnn = (double) nn;
  double dpos = dnn / 2.0;
  for (int ii = 0; ii < nn; ii++, dpos++) {

    double el = _prevEl + (dpos * dEl) / dnn;
    double az = _prevAz + (dpos * dAz) / dnn;
    
    double offsetEl = el - _elSun;
    double offsetAz = az - _azSun;
    if (offsetAz < -180) {
      offsetAz += 360.0;
    } else if (offsetAz > 180) {
      offsetAz -= 360.0;
    }
    offsetAz *= cos(_elevation * Radx::DegToRad);
    double offsetAng = sqrt(offsetEl * offsetEl + offsetAz * offsetAz);
    double reductDb = _getPowerCorrection(offsetAng);

    sum += pow(10.0, reductDb / 10.0);

  }

  double mean = sum / dnn;
  double meanDb = 10.0 * log10(mean);
  return meanDb;

}

////////////////////////////////////////////////////////////////////////
// Compute power correction based on el and az error

double RadxSunMon::_getPowerCorrection(double angleOffset)

{

  int nCurve = _params.power_correction_curve_n;

  double powerCorrection = 0.0;
  if (angleOffset <=
      _params._power_correction_curve[0].angular_offset_deg) {
    powerCorrection = _params._power_correction_curve[0].power_correction_db;
  } else if (angleOffset >=
             _params._power_correction_curve[nCurve-1].angular_offset_deg) {
    powerCorrection = _params._power_correction_curve[nCurve-1].power_correction_db;
  } else {
    for (int ii = 1; ii < nCurve; ii++) {
      if (angleOffset >= _params._power_correction_curve[ii-1].angular_offset_deg &&
          angleOffset <= _params._power_correction_curve[ii].angular_offset_deg) {
        double frac =
          (angleOffset - _params._power_correction_curve[ii-1].angular_offset_deg) /
          (_params._power_correction_curve[ii].angular_offset_deg -
           _params._power_correction_curve[ii-1].angular_offset_deg);
        powerCorrection = _params._power_correction_curve[ii-1].power_correction_db +
          frac * (_params._power_correction_curve[ii].power_correction_db -
                  _params._power_correction_curve[ii-1].power_correction_db);
      }
    }
  }

  return powerCorrection;

}

////////////////////////////////////////////////////////////////////////
// Compute the cross-polar power ratio in clutter
// Returns 0 on success, -1 on failure

int RadxSunMon::_computeXpolRatio()

{

  // initialize stats for xpol ratio
  
  _nXpolRatio = 0;
  _sumXpolRatioDb = 0.0;
  _meanXpolRatioDb = NAN;
  
  double receiverGainDbHx = 0.0;
  double receiverGainDbVx = 0.0;
  if (_momentsVol.getNRcalibs() > 0) {
    const RadxRcalib *calib = _momentsVol.getRcalibs()[0];
    receiverGainDbHx = calib->getReceiverGainDbHx();
    receiverGainDbVx = calib->getReceiverGainDbVx();
  }

  // loop through all rays in moments vol
  
  const vector<RadxRay *> &rays = _momentsVol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    
    RadxRay *ray = rays[ii];
    double el = ray->getElevationDeg();

    if (el < _params.clutter_min_elevation_deg ||
        el > _params.clutter_max_elevation_deg) {
      continue;
    }

    // process this ray

    if (_processRayForXpolRatio(ray)) {
      cerr << "ERROR - RadxSunMon::_computeXpolRatio()" << endl;
      cerr << "  Cannot compute cross-pol power ratio" << endl;
    }
    
  } // ii

  // compute the stats

  if (_nXpolRatio > 0) {
    _meanXpolRatioDb = _sumXpolRatioDb / (double) _nXpolRatio;
    // cerr << "1111111111111 _meanXpolRatioDb: " << _meanXpolRatioDb << endl;
    // cerr << "1111111111111 _receiverGainDbHx: " << receiverGainDbHx << endl;
    // cerr << "1111111111111 _receiverGainDbVx: " << receiverGainDbVx << endl;
    // cerr << "2222222222222 gainHx: " << _calib.getReceiverGainDbHx() << endl;
    // cerr << "2222222222222 gainVx: " << _calib.getReceiverGainDbVx() << endl;
    _meanXpolRatioDb += (receiverGainDbHx - receiverGainDbVx);
  }
  if (_params.debug) {
    cerr << "========== XPOL RATIO STATS =========" << endl;
    cerr << "  _nXpolRatio: " << _nXpolRatio << endl;
    cerr << "  _meanXpolRatioDb: " << _meanXpolRatioDb << endl;
    cerr << "  _zdrmDb: " << _zdrmDb << endl;
    cerr << "=====================================" << endl;
  }

  return 0;

}

////////////////////////////////////////////////////////////////////////
// Process ray for cross-polar ratio
// Returns 0 on success, -1 on failure

int RadxSunMon::_processRayForXpolRatio(RadxRay *ray)

{

  // x-polar power h channel
  
  const RadxField *dbmhxFld = ray->getField(_params.dbmhx_field_name);
  if (dbmhxFld == NULL) {
    cerr << "ERROR - RadxSunMon::_processRayForXpolRatio()" << endl;
    cerr << "  No DBMHX field available: " << _params.dbmhx_field_name << endl;
    return -1;
  }
  const Radx::fl32 *dbmhxArray = dbmhxFld->getDataFl32();
  Radx::fl32 dbmhxMiss = dbmhxFld->getMissingFl32();

  // x-polar power v channel
  
  const RadxField *dbmvxFld = ray->getField(_params.dbmvx_field_name);
  if (dbmvxFld == NULL) {
    cerr << "ERROR - RadxSunMon::_processRayForXpolRatio()" << endl;
    cerr << "  No DBMVX field available: " << _params.dbmvx_field_name << endl;
    return -1;
  }
  const Radx::fl32 *dbmvxArray = dbmvxFld->getDataFl32();
  Radx::fl32 dbmvxMiss = dbmvxFld->getMissingFl32();

  // loop through gates

  double range = _startRangeKm;
  for (size_t igate = 0; igate < _nGates; igate++, range += _gateSpacingKm) {

    if (range < _params.clutter_min_range_km ||
        range > _params.clutter_max_range_km) {
      continue;
    }

    Radx::fl32 dbmhx = dbmhxArray[igate];
    if (dbmhx == dbmhxMiss ||
        dbmhx < _params.clutter_min_power_dbm ||
        dbmhx > _params.clutter_max_power_dbm) {
      continue;
    }
        
    Radx::fl32 dbmvx = dbmvxArray[igate];
    if (dbmvx == dbmvxMiss ||
        dbmvx < _params.clutter_min_power_dbm ||
        dbmvx > _params.clutter_max_power_dbm) {
      continue;
    }
    
    _sumXpolRatioDb += (dbmhx - dbmvx);
    _nXpolRatio++;
    
  } // igate

  return 0;

}

//////////////////////////////////////////////////
// retrieve site temp from SPDB for volume time

int RadxSunMon::_retrieveSiteTempFromSpdb(const RadxVol &vol,
                                          double &tempC,
                                          time_t &timeForTemp)
  
{

  // get surface data from SPDB

  DsSpdb spdb;
  si32 dataType = 0;
  if (strlen(_params.site_temp_station_name) > 0) {
    dataType =  Spdb::hash4CharsToInt32(_params.site_temp_station_name);
  }
  time_t searchTime = vol.getStartTimeSecs();

  if (spdb.getClosest(_params.site_temp_spdb_url,
                      searchTime,
                      _params.site_temp_search_margin_secs,
                      dataType)) {
    cerr << "WARNING - RadxSunMon::_retrieveSiteTempFromSpdb" << endl;
    cerr << "  Cannot get temperature from URL: " << _params.site_temp_spdb_url << endl;
    cerr << "  Station name: " << _params.site_temp_station_name << endl;
    cerr << "  Search time: " << RadxTime::strm(searchTime) << endl;
    cerr << "  Search margin (secs): " << _params.site_temp_search_margin_secs << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
  
  // got chunks
  
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
  if (chunks.size() < 1) {
    cerr << "WARNING - RadxSunMon::_retrieveSiteTempFromSpdb" << endl;
    cerr << "  No suitable temp data from URL: " << _params.site_temp_spdb_url << endl;
    cerr << "  Search time: " << RadxTime::strm(searchTime) << endl;
    cerr << "  Search margin (secs): " << _params.site_temp_search_margin_secs << endl;
    return -1;
  }

  const Spdb::chunk_t &chunk = chunks[0];
  WxObs obs;
  if (obs.disassemble(chunk.data, chunk.len)) {
    cerr << "WARNING - RadxSunMon::_retrieveSiteTempFromSpdb" << endl;
    cerr << "  SPDB data is of incorrect type, prodLabel: " << spdb.getProdLabel() << endl;
    cerr << "  Should be station data type" << endl;
    cerr << "  URL: " << _params.site_temp_spdb_url << endl;
    return -1;
  }

  tempC = obs.getTempC();
  timeForTemp = obs.getObservationTime();

  return 0;

}

//////////////////////////////////////////////////////
// write results to SPDB

int RadxSunMon::_writeResultsToSpdb()
  
{

  string xml;

  xml += RadxXml::writeStartTag("RadxSunMon", 0);
  
  RadxPath momentsPath(_momentsPath);
  xml += RadxXml::writeString("MomentsFile", 1, momentsPath.getFile());

  xml += RadxXml::writeString("Time", 1, _rayTime.getW3cStr());
  
  xml += RadxXml::writeDouble("elRay", 1, _elevation);
  xml += RadxXml::writeDouble("azRay", 1, _azimuth);

  xml += RadxXml::writeDouble("elSun", 1, _elSun);
  xml += RadxXml::writeDouble("azSun", 1, _azSun);

  xml += RadxXml::writeDouble("elOffset", 1, _elOffset);
  xml += RadxXml::writeDouble("azOffset", 1, _azOffset);
  xml += RadxXml::writeDouble("angleOffset", 1, _angleOffset);

  xml += RadxXml::writeDouble("meanElOffset", 1, _meanElOffset);
  xml += RadxXml::writeDouble("meanAzOffset", 1, _meanAzOffset);
  
  xml += RadxXml::writeDouble("measuredDbmHc", 1, _measuredDbmHc);
  xml += RadxXml::writeDouble("measuredDbmVc", 1, _measuredDbmVc);
  xml += RadxXml::writeDouble("measuredDbmHx", 1, _measuredDbmHx);
  xml += RadxXml::writeDouble("measuredDbmVx", 1, _measuredDbmVx);
  
  xml += RadxXml::writeDouble("correctedDbmHc", 1, _correctedDbmHc);
  xml += RadxXml::writeDouble("correctedDbmVc", 1, _correctedDbmVc);
  xml += RadxXml::writeDouble("correctedDbmHx", 1, _correctedDbmHx);
  xml += RadxXml::writeDouble("correctedDbmVx", 1, _correctedDbmVx);

  xml += RadxXml::writeDouble("oneWayAtmosAttenDb", 1, _oneWayAtmosAttenDb);
  xml += RadxXml::writeDouble("offSunPowerCorrectionDb", 1, _offSunPowerCorrectionDb);
  xml += RadxXml::writeDouble("appliedPowerCorrectionDb", 1, _appliedPowerCorrectionDb);

  xml += RadxXml::writeDouble("SS", 1, _SS);
  xml += RadxXml::writeDouble("S1S2", 1, _S1S2);

  if (_params.compute_cross_pol_ratio_in_clutter) {
    xml += RadxXml::writeDouble("XpolRatioDb", 1, _meanXpolRatioDb);
    xml += RadxXml::writeDouble("zdrmDb", 1, _zdrmDb);
  }

  if (_params.read_site_temp_from_spdb) {
    xml += TaXml::writeDouble("TempSite", 1, _siteTempC);
    RadxTime tempTime(_timeForSiteTemp);
    xml += RadxXml::writeString("TempTime", 1, tempTime.getW3cStr());
  }

  xml += RadxXml::writeEndTag("RadxSunMon", 0);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Writing XML results to SPDB:" << endl;
    cerr << xml << endl;
  }

  DsSpdb spdb;
  time_t validTime = _rayTime.utime();
  int dataType = (int) (_rayTime.getSubSec() * 1000000.0);
  spdb.addPutChunk(dataType, validTime, validTime, xml.size() + 1, xml.c_str());
  if (spdb.put(_params.spdb_output_url,
               SPDB_XML_ID, SPDB_XML_LABEL)) {
    cerr << "ERROR - RadxSunMon::_writeResultsToSpdb" << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Wrote to spdb, url: " << _params.spdb_output_url << endl;
  }

  return 0;

}


