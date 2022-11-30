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
// RadxClutMon.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2015
//
///////////////////////////////////////////////////////////////
//
// Monitors clutter, to check radar calibration over time
//
////////////////////////////////////////////////////////////////

#include "RadxClutMon.hh"
#include <Radx/Radx.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxXml.hh>
#include <Radx/RadxRcalib.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <Spdb/DsSpdb.hh>
using namespace std;

// Constructor

RadxClutMon::RadxClutMon(int argc, char **argv)
  
{

  OK = TRUE;

  // set programe name

  _progName = "RadxClutMon";
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

}

// destructor

RadxClutMon::~RadxClutMon()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RadxClutMon::Run()
{

  // read in clutter map

  if (_readClutterMap()) {
    cerr << "ERROR - RadxClutMon::Run()" << endl;
    cerr << "  Cannot read in clutter map file" << endl;
    return -1;
  }

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

int RadxClutMon::_runFilelist()
{
  
  int iret = 0;
  
  // loop through the input file list
  
  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {
    string inputPath = _args.inputFileList[ii];
    // read input moments file
    if (_readMoments(inputPath) == 0) {
      // process it
      if (_processVol()) {
        cerr << "ERROR - RadxClutMon::_runFileList" << endl;
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

int RadxClutMon::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.moments_input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - RadxClutMon::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.moments_input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - RadxClutMon::_runFilelist()" << endl;
    cerr << "  No files found, dir: " << _params.moments_input_dir << endl;
    return -1;
  }
  
  // loop through the input file list

  int iret = 0;
  for (size_t ii = 0; ii < paths.size(); ii++) {
    // read input file
    if (_readMoments(paths[ii]) == 0) {
      if (_processVol()) {
        cerr << "ERROR - RadxClutMon::_runArchive" << endl;
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

int RadxClutMon::_runRealtime()
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
        cerr << "ERROR - RadxClutMon::_runRealtime" << endl;
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

int RadxClutMon::_readMoments(const string &readPath)
{
  
  PMU_auto_register("Reading moments");

  if (_params.debug) {
    cerr << "INFO - RadxClutMon::Run" << endl;
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

  if (_params.set_fixed_angle_limits) {
    inFile.setReadFixedAngleLimits(_params.lower_fixed_angle_limit,
                                 _params.upper_fixed_angle_limit);
    if (_params.lower_fixed_angle_limit == _params.upper_fixed_angle_limit) {
      // relax strict angle checking since only a single angle is specified
      // which means the user wants the closest angle
      inFile.setReadStrictAngleLimits(false);
    }
  }

  if (!_params.apply_strict_angle_limits) {
    inFile.setReadStrictAngleLimits(false);
  }

  if (_params.remove_rays_with_antenna_transitions &&
      !_params.trim_surveillance_sweeps_to_360deg) {
    inFile.setReadIgnoreTransitions(true);
    inFile.setReadTransitionNraysMargin(0);
  }
  
  inFile.addReadField(_params.dbz_field_name);
  inFile.addReadField(_params.clutter_power_field_name);
  if (strlen(_params.vel_field_name) > 0) {
    inFile.addReadField(_params.vel_field_name);
  }
  if (strlen(_params.cpa_field_name) > 0) {
    inFile.addReadField(_params.cpa_field_name);
  }
  if (strlen(_params.dbmhc_field_name) > 0) {
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


  if (_params.debug >= Params::DEBUG_EXTRA) {
    inFile.printReadRequest(cerr);
  }

  // read in file
  
  if (inFile.readFromPath(readPath, _momentsVol)) {
    cerr << "ERROR - RadxClutMon::_readMoments" << endl;
    cerr << "  path: " << readPath << endl;
    cerr << inFile.getErrStr() << endl;
    cerr << "NOTE: the clutter output field name from RadxPersistentClutter" << endl;
    cerr << "      must also exist in the moments data file" << endl;
    cerr << "      Ensure that this field is in the moments data: "
         << _params.clutter_power_field_name << endl;
    cerr << "Also check that the following fields are in the moments data:" << endl;
    cerr << "  " << _params.dbz_field_name << endl;
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

  // convert to floats

  _momentsVol.convertToFl32();

  // remap range geometry if neededs

  if (_checkGeom(_clutMapVol, _momentsVol)) {
    _momentsVol.remapRangeGeom(_clutMapVol.getStartRangeKm(),
                               _clutMapVol.getGateSpacingKm());
  }
  
  _momentsPath = inFile.getPathInUse();
  if (_params.debug) {
    cerr << "Read moments file: " << _momentsPath << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// process a moments volume file

int RadxClutMon::_processVol()
{

  // initialize stats

  _initForStats();

  // loop through all rays in clutMap vol
  
  const vector<RadxRay *> &cRays = _clutMapVol.getRays();
  for (size_t ii = 0; ii < cRays.size(); ii++) {
    
    RadxRay *cRay = cRays[ii];
    double cAz = cRay->getAzimuthDeg();   
    double cEl = cRay->getElevationDeg();

    // check azimuth if requested

    if (_params.set_azimuth_limits) {
      if (!_azWithinLimits(cAz)) {
        continue;
      }
    }
    
    // find matching ray in moments volume
    
    const vector<RadxRay *> &mRays = _momentsVol.getRays();
    for (size_t jj = 0; jj < mRays.size(); jj++) {
      
      RadxRay *mRay = mRays[jj];
      double mAz = mRay->getAzimuthDeg();   
      double mEl = mRay->getElevationDeg();
      
      double dAz = cAz - mAz;
      if (dAz < -180) {
        dAz += 360.0;
      } else if (dAz > 180) {
        dAz -= 360.0;
      }
      double diffAz = fabs(dAz);
      double diffEl = fabs(cEl - mEl);
      
      if (diffAz <= _params.ray_azimuth_tolerance_deg &&
          diffEl <= _params.ray_elevation_tolerance_deg) {
        // rays match in position, process ray
        _processRay(*cRay, *mRay);
        if (_params.debug >= Params::DEBUG_EXTRA) {
          cerr << "Matched ray - caz cel maz mel dEl dAz: "
               << cAz << ", " << cEl << ", "
               << mAz << ", " << mEl << ", "
               << diffEl << ", "
               << diffAz << endl;
        }
        break;
      }
      
    } // jj

  } // ii

  // compute the stats

  _computeStats();
  if (_params.debug) {
    _printStats(cerr);
  }

  // write out

  if (_params.write_results_to_spdb) {
    if (_writeStatsToSpdb()) {
      cerr << "ERROR - RadxClutMon::_processVol" << endl;
      cerr << "  Cannot write stats to spdb" << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////////////////////////
// check az is within limits

bool RadxClutMon::_azWithinLimits(double az)

{

  double minAz = _params.lower_azimuth_limit;
  double maxAz = _params.upper_azimuth_limit;

  if (minAz < maxAz) {

    if (az >= minAz && az <= maxAz) {
      return true;
    } else {
      return false;
    }

  } else {

    if (az > maxAz && az < minAz) {
      return false;
    } else {
      return true;
    }

  }
  
}

//////////////////////////////////////////////////////////////
// process ray pair

int RadxClutMon::_processRay(const RadxRay &clutMapRay,
                             const RadxRay &momentsRay)
  
{

  // set number of gates to minimum
  
  size_t nGates = clutMapRay.getNGates();
  if (nGates > momentsRay.getNGates()) {
    nGates = momentsRay.getNGates();
  }

  // get field pointers for clutter data

  const RadxField *clutPowerFld =
    clutMapRay.getField(_params.clutter_power_field_name);
  if (clutPowerFld == NULL) {
    cerr << "ERROR - RadxClutMon::_processRay" << endl;
    cerr << "  Clut power field missing: " << _params.clutter_power_field_name << endl;
    return -1;
  }
  const Radx::fl32 *clutPowerArray = clutPowerFld->getDataFl32();
  Radx::fl32 clutPowerMiss = clutPowerFld->getMissingFl32();
  
  const RadxField *clutTimeFracFld =
    clutMapRay.getField(_params.clutter_time_fraction_field_name);
  if (clutTimeFracFld == NULL) {
    cerr << "ERROR - RadxClutMon::_processRay" << endl;
    cerr << "  Clut time fraction field missing: " 
         << _params.clutter_time_fraction_field_name << endl;
    return -1;
  }
  const Radx::fl32 *clutTimeFracArray = clutTimeFracFld->getDataFl32();
  Radx::fl32 clutTimeFracMiss = clutTimeFracFld->getMissingFl32();

  // get field pointers for moments data

  const RadxField *clutMomentsFld =
    momentsRay.getField(_params.clutter_power_field_name);
  if (clutMomentsFld == NULL) {
    cerr << "ERROR - RadxClutMon::_processRay" << endl;
    cerr << "  Clut moments field missing: " << _params.clutter_power_field_name << endl;
    cerr << "  The clutter output field name from RadxPersistentClutter" << endl;
    cerr << "      must also exist in the moments data file" << endl;
    return -1;
  }
  const Radx::fl32 *clutMomentsArray = clutMomentsFld->getDataFl32();
  Radx::fl32 clutMomentsMiss = clutMomentsFld->getMissingFl32();
  
  const RadxField *dbzFld = momentsRay.getField(_params.dbz_field_name);
  if (dbzFld == NULL) {
    cerr << "ERROR - RadxClutMon::_processRay" << endl;
    cerr << "  DBZ field missing: " << _params.dbz_field_name << endl;
    return -1;
  }
  const Radx::fl32 *dbzArray = dbzFld->getDataFl32();
  Radx::fl32 dbzMiss = dbzFld->getMissingFl32();

  const RadxField *dbmhcFld = momentsRay.getField(_params.dbmhc_field_name);
  const Radx::fl32 *dbmhcArray = NULL;
  Radx::fl32 dbmhcMiss = Radx::missingFl32;
  if (dbmhcFld != NULL) {
    dbmhcArray = dbmhcFld->getDataFl32();
    dbmhcMiss = dbmhcFld->getMissingFl32();
  }

  const RadxField *dbmvcFld = momentsRay.getField(_params.dbmvc_field_name);
  const Radx::fl32 *dbmvcArray = NULL;
  Radx::fl32 dbmvcMiss = Radx::missingFl32;
  if (dbmvcFld != NULL) {
    dbmvcArray = dbmvcFld->getDataFl32();
    dbmvcMiss = dbmvcFld->getMissingFl32();
  }

  const RadxField *dbmhxFld = momentsRay.getField(_params.dbmhx_field_name);
  const Radx::fl32 *dbmhxArray = NULL;
  Radx::fl32 dbmhxMiss = Radx::missingFl32;
  if (dbmhxFld != NULL) {
    dbmhxArray = dbmhxFld->getDataFl32();
    dbmhxMiss = dbmhxFld->getMissingFl32();
  }

  const RadxField *dbmvxFld = momentsRay.getField(_params.dbmvx_field_name);
  const Radx::fl32 *dbmvxArray = NULL;
  Radx::fl32 dbmvxMiss = Radx::missingFl32;
  if (dbmvxFld != NULL) {
    dbmvxArray = dbmvxFld->getDataFl32();
    dbmvxMiss = dbmvxFld->getMissingFl32();
  }

  const RadxField *velFld = momentsRay.getField(_params.vel_field_name);
  const Radx::fl32 *velArray = NULL;
  Radx::fl32 velMiss = Radx::missingFl32;
  if (velFld != NULL) {
    velArray = velFld->getDataFl32();
    velMiss = velFld->getMissingFl32();
  }

  const RadxField *cpaFld = momentsRay.getField(_params.cpa_field_name);
  const Radx::fl32 *cpaArray = NULL;
  Radx::fl32 cpaMiss = Radx::missingFl32;
  if (cpaFld != NULL) {
    cpaArray = cpaFld->getDataFl32();
    cpaMiss = cpaFld->getMissingFl32();
  }

  // loop though the gates

  double range = _momentsVol.getStartRangeKm();
  for (size_t igate = 0; igate < nGates; 
       igate++, range += _momentsVol.getGateSpacingKm()) {

    if (range < _params.clutter_min_range_km) {
      continue;
    }
    if (range > _params.clutter_max_range_km) {
      break;
    }

    // check for time fraction

    Radx::fl32 clutTimeFrac = clutTimeFracArray[igate];
    if (clutTimeFrac == clutTimeFracMiss ||
        clutTimeFrac < _params.clutter_min_time_fraction) {
      continue;
    }
    
    // get clutter power, categorize
    
    Radx::fl32 clutPower = clutPowerArray[igate];
    if (clutPower == clutPowerMiss) {
      continue;
    }
    bool isStrong = false;
    bool isWeak = false;
    if (clutPower >= _params.strong_clutter_min_power_dbm &&
        clutPower <= _params.strong_clutter_max_power_dbm) {
      isStrong = true;
    } else if (clutPower >= _params.weak_clutter_min_power_dbm &&
               clutPower <= _params.weak_clutter_max_power_dbm) {
      isWeak = true;
    }
    if (!isStrong && !isWeak) {
      continue;
    }

    // check vel and CPA

    Radx::fl32 vel = velMiss;
    if (velArray) vel = velArray[igate];
    if (vel != velMiss) {
      if (fabs(vel) > _params.clutter_max_abs_vel) {
        continue;
      }
    }

    Radx::fl32 cpa = cpaMiss;
    if (cpaArray) cpa = cpaArray[igate];
    if (cpa != cpaMiss) {
      if (cpa < _params.clutter_min_cpa) {
        continue;
      }
    }


    // get gate fields

    Radx::fl32 dbz = dbzMiss;
    if (dbzArray) dbz = dbzArray[igate];
    
    Radx::fl32 dbmhc = dbmhcMiss;
    if (dbmhcArray) dbmhc = dbmhcArray[igate];
    
    Radx::fl32 dbmvc = dbmvcMiss;
    if (dbmvcArray) dbmvc = dbmvcArray[igate];
    
    Radx::fl32 dbmhx = dbmhxMiss;
    if (dbmhxArray) dbmhx = dbmhxArray[igate];
    
    Radx::fl32 dbmvx = dbmvxMiss;
    if (dbmvxArray) dbmvx = dbmvxArray[igate];

    Radx::fl32 clutMoments = clutMomentsMiss;
    if (clutMomentsArray) clutMoments = clutMomentsArray[igate];

    // accumulate stats for stong clutter gates

    if (isStrong) {

      _nGatesStrong++;

      if (dbz != dbzMiss) {
        _nDbzStrong++;
        _sumDbzStrong += dbz;
      }

      if (dbmhc != dbmhcMiss) {
        _nDbmhcStrong++;
        _sumDbmhcStrong += dbmhc;
      }

      if (dbmvc != dbmvcMiss) {
        _nDbmvcStrong++;
        _sumDbmvcStrong += dbmvc;
      }

      if (dbmhx != dbmhxMiss) {
        _nDbmhxStrong++;
        _sumDbmhxStrong += dbmhx;
      }

      if (dbmvx != dbmvxMiss) {
        _nDbmvxStrong++;
        _sumDbmvxStrong += dbmvx;
      }

      if (dbmhc != dbmhcMiss && dbmvc != dbmvcMiss) {
        double zdr = dbmhc - dbmvc;
        _nZdrStrong++;
        _sumZdrStrong += zdr;
      }

      if (dbmhx != dbmhxMiss && dbmvx != dbmvxMiss) {
        double xpolr = dbmhx - dbmvx;
        _nXpolrStrong++;
        _sumXpolrStrong += xpolr;
      }

    }

    // accumulate stats for weak clutter gates

    if (isWeak) {

      _nGatesWeak++;

      if (dbz != dbzMiss) {
        _nDbzWeak++;
        _sumDbzWeak += dbz;
      }

      if (dbmhc != dbmhcMiss) {
        _nDbmhcWeak++;
        _sumDbmhcWeak += dbmhc;
      }

      if (dbmvc != dbmvcMiss) {
        _nDbmvcWeak++;
        _sumDbmvcWeak += dbmvc;
      }

      if (dbmhx != dbmhxMiss) {
        _nDbmhxWeak++;
        _sumDbmhxWeak += dbmhx;
      }

      if (dbmvx != dbmvxMiss) {
        _nDbmvxWeak++;
        _sumDbmvxWeak += dbmvx;
      }

      if (dbmhc != dbmhcMiss && dbmvc != dbmvcMiss) {
        double zdr = dbmhc - dbmvc;
        _nZdrWeak++;
        _sumZdrWeak += zdr;
      }

      if (dbmhx != dbmhxMiss && dbmvx != dbmvxMiss) {
        double xpolr = dbmhx - dbmvx;
        _nXpolrWeak++;
        _sumXpolrWeak += xpolr;
      }

      if (clutPower != clutPowerMiss &&
          clutMoments != clutMomentsMiss) {
        double deltaPower = clutMoments - clutPower;
        if (deltaPower > _params.power_margin_for_wx_contamination) {
          _nWxWeak++;
        }
      }

    }

  } // igate

  return 0;

}

//////////////////////////////////////////////////
// Read in clutter map

int RadxClutMon::_readClutterMap()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.clutter_map_input_dir);
  RadxTime searchTime(_params.clutter_map_search_time.year,
                      _params.clutter_map_search_time.month,
                      _params.clutter_map_search_time.day,
                      _params.clutter_map_search_time.hour,
                      _params.clutter_map_search_time.min,
                      _params.clutter_map_search_time.sec);
  tlist.setModeClosest(searchTime,
                       _params.clutter_map_search_margin_secs);
  
  if (tlist.compile()) {
    cerr << "ERROR - RadxClutMon::_readClutterMap()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.clutter_map_input_dir << endl;
    cerr << "  Search time: " << RadxTime::strm(searchTime.utime()) << endl;
    cerr << "  Search margin secs: " << _params.clutter_map_search_margin_secs << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - RadxClutMon::_readClutterMap()" << endl;
    cerr << "  No suitable files found, dir: " << _params.clutter_map_input_dir << endl;
    cerr << "  Search time: " << RadxTime::strm(searchTime.utime()) << endl;
    cerr << "  Search margin secs: " << _params.clutter_map_search_margin_secs << endl;
    return -1;
  }
  string clutMapPath = paths[0];

  GenericRadxFile inFile;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    inFile.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    inFile.setVerbose(true);
  }
  
  if (_params.set_fixed_angle_limits) {
    inFile.setReadFixedAngleLimits(_params.lower_fixed_angle_limit,
                                   _params.upper_fixed_angle_limit);
    if (_params.lower_fixed_angle_limit == _params.upper_fixed_angle_limit) {
      inFile.setReadStrictAngleLimits(false);
    }
  }
  
  if (!_params.apply_strict_angle_limits) {
    inFile.setReadStrictAngleLimits(false);
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    inFile.printReadRequest(cerr);
  }

  inFile.addReadField(_params.clutter_power_field_name);
  inFile.addReadField(_params.clutter_time_fraction_field_name);
  
  // perform the read

  _clutMapVol.clear();
  if (inFile.readFromPath(clutMapPath, _clutMapVol)) {
    cerr << "ERROR - RadxClutMon::_readClutterMap" << endl;
    cerr << "  path: " << clutMapPath << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }

  // convert to floats

  _clutMapVol.convertToFl32();

  _clutMapPath = inFile.getPathInUse();
  if (_params.debug) {
    cerr << "Read clutter map file: " << _clutMapPath << endl;
  }

  return 0;
  
}

//////////////////////////////////////////////////////
// check geometry between clutter and moments volumes
// Returns 0 if geom matches, -1 if not

int RadxClutMon::_checkGeom(const RadxVol &clutMapVol,
                            const RadxVol &momentsVol)
  
{

  // check that geometry matches

  double diff = clutMapVol.getStartRangeKm() - momentsVol.getStartRangeKm();
  if (fabs(diff) > 0.001) {
    if (_params.debug) {
      cerr << "ERROR - RadxClutMon::_checkGeom" << endl;
      cerr << "  Volumes have different start range" << endl;
      cerr << "  start range clutter: "
           << clutMapVol.getStartRangeKm() << endl;
      cerr << "  start range moments: "
           << momentsVol.getStartRangeKm() << endl;
    }
    return -1;
  }

  diff = clutMapVol.getGateSpacingKm() - momentsVol.getGateSpacingKm();
  if (fabs(diff) > 0.001) {
    if (_params.debug) {
      cerr << "ERROR - RadxClutMon::_checkGeom" << endl;
      cerr << "  Volumes have different gate spacing" << endl;
      cerr << "  gate spacing clutter: "
           << clutMapVol.getGateSpacingKm() << endl;
      cerr << "  gate spacing moments: "
           << momentsVol.getGateSpacingKm() << endl;
    }
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////////
// initialize for stats

void RadxClutMon::_initForStats()
  
{

  _nDbzStrong = _nDbzWeak = 0;
  _nDbmhcStrong = _nDbmhcWeak = 0;
  _nDbmvcStrong = _nDbmvcWeak = 0;
  _nDbmhxStrong = _nDbmhxWeak = 0;
  _nDbmvxStrong = _nDbmvxWeak = 0;
  _nZdrStrong = _nZdrWeak = 0;
  _nXpolrStrong = _nXpolrWeak = 0;

  _sumDbzStrong = _sumDbzWeak = 0.0;
  _sumDbmhcStrong = _sumDbmhcWeak = 0.0;
  _sumDbmvcStrong = _sumDbmvcWeak = 0.0;
  _sumDbmhxStrong = _sumDbmhxWeak = 0.0;
  _sumDbmvxStrong = _sumDbmvxWeak = 0.0;
  _sumZdrStrong = _sumZdrWeak = 0.0;
  _sumXpolrStrong = _sumXpolrWeak = 0.0;

  _meanDbzStrong = _meanDbzWeak = NAN;
  _meanDbmhcStrong = _meanDbmhcWeak = NAN;
  _meanDbmvcStrong = _meanDbmvcWeak = NAN;
  _meanDbmhxStrong = _meanDbmhxWeak = NAN;
  _meanDbmvxStrong = _meanDbmvxWeak = NAN;
  _meanZdrStrong = _meanZdrWeak = NAN;
  _meanXpolrStrong = _meanXpolrWeak = NAN;

  _nGatesStrong = 0;
  _nGatesWeak = 0;
  _nWxWeak = 0;
  _fractionWxWeak = NAN;

  _xmitPowerDbmH = _xmitPowerDbmV = _xmitPowerDbmBoth = NAN;

  if (_params.monitor_transmit_power) {
    string statusXml = _momentsVol.getStatusXml();
    double val;
    if (RadxXml::readDouble(statusXml, _params.xmit_power_key_both, val) == 0) {
      _xmitPowerDbmBoth = val;   
    }
    if (RadxXml::readDouble(statusXml, _params.xmit_power_key_h, val) == 0) {
      _xmitPowerDbmH = val;   
    }
    if (RadxXml::readDouble(statusXml, _params.xmit_power_key_v, val) == 0) {
      _xmitPowerDbmV = val;   
    }
  }

}

//////////////////////////////////////////////////////
// compute stats

void RadxClutMon::_computeStats()
  
{

  // adjust powers for calibrated receiver gains

  double receiverGainDbHc = 0.0;
  double receiverGainDbVc = 0.0;
  double receiverGainDbHx = 0.0;
  double receiverGainDbVx = 0.0;
  if (_momentsVol.getNRcalibs() > 0) {
    const RadxRcalib *calib = _momentsVol.getRcalibs()[0];
    receiverGainDbHc = calib->getReceiverGainDbHc();
    receiverGainDbVc = calib->getReceiverGainDbVc();
    receiverGainDbHx = calib->getReceiverGainDbHx();
    receiverGainDbVx = calib->getReceiverGainDbVx();
  }
  
  if (_nDbzStrong > 0) {
    _meanDbzStrong = _sumDbzStrong / (double) _nDbzStrong;
  }
  if (_nDbzWeak > 0) {
    _meanDbzWeak = _sumDbzWeak / (double) _nDbzWeak;
  }

  if (_nDbmhcStrong > 0) {
    _meanDbmhcStrong = _sumDbmhcStrong / (double) _nDbmhcStrong;
    _meanDbmhcStrong += receiverGainDbHc;
  }
  if (_nDbmhcWeak > 0) {
    _meanDbmhcWeak = _sumDbmhcWeak / (double) _nDbmhcWeak;
    _meanDbmhcWeak += receiverGainDbHc;
  }

  if (_nDbmvcStrong > 0) {
    _meanDbmvcStrong = _sumDbmvcStrong / (double) _nDbmvcStrong;
    _meanDbmvcStrong += receiverGainDbVc;
  }
  if (_nDbmvcWeak > 0) {
    _meanDbmvcWeak = _sumDbmvcWeak / (double) _nDbmvcWeak;
    _meanDbmvcWeak += receiverGainDbVc;
  }

  if (_nDbmhxStrong > 0) {
    _meanDbmhxStrong = _sumDbmhxStrong / (double) _nDbmhxStrong;
    _meanDbmhxStrong += receiverGainDbHx;
  }
  if (_nDbmhxWeak > 0) {
    _meanDbmhxWeak = _sumDbmhxWeak / (double) _nDbmhxWeak;
    _meanDbmhxWeak += receiverGainDbHx;
  }

  if (_nDbmvxStrong > 0) {
    _meanDbmvxStrong = _sumDbmvxStrong / (double) _nDbmvxStrong;
    _meanDbmvxStrong += receiverGainDbVx;
  }
  if (_nDbmvxWeak > 0) {
    _meanDbmvxWeak = _sumDbmvxWeak / (double) _nDbmvxWeak;
    _meanDbmvxWeak += receiverGainDbVx;
  }

  if (_nZdrStrong > 0) {
    _meanZdrStrong = _sumZdrStrong / (double) _nZdrStrong;
    _meanZdrStrong += (receiverGainDbHc - receiverGainDbVc);
  }
  if (_nZdrWeak > 0) {
    _meanZdrWeak = _sumZdrWeak / (double) _nZdrWeak;
    _meanZdrWeak += (receiverGainDbHc - receiverGainDbVc);
  }

  if (_nXpolrStrong > 0) {
    _meanXpolrStrong = _sumXpolrStrong / (double) _nXpolrStrong;
    _meanXpolrStrong += (receiverGainDbHx - receiverGainDbVx);
  }
  if (_nXpolrWeak > 0) {
    _meanXpolrWeak = _sumXpolrWeak / (double) _nXpolrWeak;
    _meanXpolrWeak += (receiverGainDbHx - receiverGainDbVx);
  }

  _fractionWxWeak = (double) _nWxWeak / _nGatesWeak;

}


//////////////////////////////////////////////////////
// print stats

void RadxClutMon::_printStats(ostream &out)
  
{

  out << "============= clutter stats ==============" << endl;
  
  out << "  ClutMap file " << _clutMapPath << endl;
  out << "  Moments file " << _momentsPath << endl;
  out << "  Time: " << RadxTime::strm(_momentsVol.getStartTimeSecs()) << endl;

  out << "------------------------------------------" << endl;

  out << "  Stats in strong clutter:" << endl;
  out << "   nDbz, meanDbz: " << _nDbzStrong << ", " << _meanDbzStrong << endl;
  out << "   nDbmhc, meanDbmhc: " << _nDbmhcStrong << ", " << _meanDbmhcStrong << endl;
  out << "   nDbmvc, meanDbmvc: " << _nDbmvcStrong << ", " << _meanDbmvcStrong << endl;
  out << "   nDbmhx, meanDbmhx: " << _nDbmhxStrong << ", " << _meanDbmhxStrong << endl;
  out << "   nDbmvx, meanDbmvx: " << _nDbmvxStrong << ", " << _meanDbmvxStrong << endl;
  out << "   nZdr, meanZdr: " << _nZdrStrong << ", " << _meanZdrStrong << endl;
  out << "   nXpolr, meanXpolr: " << _nXpolrStrong << ", " << _meanXpolrStrong << endl;
  
  out << "------------------------------------------" << endl;

  out << "  Stats in weak clutter:" << endl;
  out << "   nDbz, meanDbz: " << _nDbzWeak << ", " << _meanDbzWeak << endl;
  out << "   nDbmhc, meanDbmhc: " << _nDbmhcWeak << ", " << _meanDbmhcWeak << endl;
  out << "   nDbmvc, meanDbmvc: " << _nDbmvcWeak << ", " << _meanDbmvcWeak << endl;
  out << "   nDbmhx, meanDbmhx: " << _nDbmhxWeak << ", " << _meanDbmhxWeak << endl;
  out << "   nDbmvx, meanDbmvx: " << _nDbmvxWeak << ", " << _meanDbmvxWeak << endl;
  out << "   nZdr, meanZdr: " << _nZdrWeak << ", " << _meanZdrWeak << endl;
  out << "   nXpolr, meanXpolr: " << _nXpolrWeak << ", " << _meanXpolrWeak << endl;

  out << "  nGatesStrong: " << _nGatesStrong << endl;
  out << "  nGatesWeak: " << _nGatesWeak << endl;
  out << "  nWxWeak: " << _nWxWeak << endl;
  out << "  fractionWxWeak: " << _fractionWxWeak << endl;

  if (_fractionWxWeak > _params.min_fraction_for_wx_contamination) {
    out << "   weather contamination: TRUE" << endl;
  } else {
    out << "   weather contamination: FALSE" << endl;
  }
  
  if (_params.monitor_transmit_power) {
    out << "    xmitPowerDbmBoth: " << _xmitPowerDbmBoth << endl;
    out << "    xmitPowerDbmH: " << _xmitPowerDbmH << endl;
    out << "    xmitPowerDbmV: " << _xmitPowerDbmV << endl;
  }

  out << "==========================================" << endl;

}

//////////////////////////////////////////////////////
// write stats to SPDB

int RadxClutMon::_writeStatsToSpdb()
  
{

  // check we have data

  if (_nDbzStrong < 10) {
    return 0;
  }

  string xml;

  xml += RadxXml::writeStartTag("ClutterStats", 0);
  
  RadxPath clutMapPath(_clutMapPath);
  xml += RadxXml::writeString("ClutMapFile", 1, clutMapPath.getFile());

  RadxPath momentsPath(_momentsPath);
  xml += RadxXml::writeString("MomentsFile", 1, momentsPath.getFile());

  RadxTime startTime(_momentsVol.getStartTimeSecs());
  xml += RadxXml::writeString("Time", 1, startTime.getW3cStr());

  xml += RadxXml::writeInt("nDbzStrong", 1, _nDbzStrong);
  xml += RadxXml::writeDouble("meanDbzStrong", 1, _meanDbzStrong);
  
  xml += RadxXml::writeInt("nDbmhcStrong", 1, _nDbmhcStrong);
  xml += RadxXml::writeDouble("meanDbmhcStrong", 1, _meanDbmhcStrong);
  
  xml += RadxXml::writeInt("nDbmvcStrong", 1, _nDbmvcStrong);
  xml += RadxXml::writeDouble("meanDbmvcStrong", 1, _meanDbmvcStrong);
  
  xml += RadxXml::writeInt("nDbmhxStrong", 1, _nDbmhxStrong);
  xml += RadxXml::writeDouble("meanDbmhxStrong", 1, _meanDbmhxStrong);
  
  xml += RadxXml::writeInt("nDbmvxStrong", 1, _nDbmvxStrong);
  xml += RadxXml::writeDouble("meanDbmvxStrong", 1, _meanDbmvxStrong);
  
  xml += RadxXml::writeInt("nZdrStrong", 1, _nZdrStrong);
  xml += RadxXml::writeDouble("meanZdrStrong", 1, _meanZdrStrong);
  
  xml += RadxXml::writeInt("nXpolrStrong", 1, _nXpolrStrong);
  xml += RadxXml::writeDouble("meanXpolrStrong", 1, _meanXpolrStrong);
  
  xml += RadxXml::writeInt("nDbzWeak", 1, _nDbzWeak);
  xml += RadxXml::writeDouble("meanDbzWeak", 1, _meanDbzWeak);
  
  xml += RadxXml::writeInt("nDbmhcWeak", 1, _nDbmhcWeak);
  xml += RadxXml::writeDouble("meanDbmhcWeak", 1, _meanDbmhcWeak);
  
  xml += RadxXml::writeInt("nDbmvcWeak", 1, _nDbmvcWeak);
  xml += RadxXml::writeDouble("meanDbmvcWeak", 1, _meanDbmvcWeak);
  
  xml += RadxXml::writeInt("nDbmhxWeak", 1, _nDbmhxWeak);
  xml += RadxXml::writeDouble("meanDbmhxWeak", 1, _meanDbmhxWeak);
  
  xml += RadxXml::writeInt("nDbmvxWeak", 1, _nDbmvxWeak);
  xml += RadxXml::writeDouble("meanDbmvxWeak", 1, _meanDbmvxWeak);
  
  xml += RadxXml::writeInt("nZdrWeak", 1, _nZdrWeak);
  xml += RadxXml::writeDouble("meanZdrWeak", 1, _meanZdrWeak);
  
  xml += RadxXml::writeInt("nXpolrWeak", 1, _nXpolrWeak);
  xml += RadxXml::writeDouble("meanXpolrWeak", 1, _meanXpolrWeak);
  
  xml += RadxXml::writeInt("nGatesStrong", 1, _nGatesStrong);
  xml += RadxXml::writeInt("nGatesWeak", 1, _nGatesWeak);
  xml += RadxXml::writeInt("nWxWeak", 1, _nWxWeak);
  xml += RadxXml::writeDouble("fractionWxWeak", 1, _fractionWxWeak);

  if (_fractionWxWeak > _params.min_fraction_for_wx_contamination) {
    xml += RadxXml::writeBoolean("weatherContamination", 1, true);
  } else {
    xml += RadxXml::writeBoolean("weatherContamination", 1, false);
  }

  if (_params.monitor_transmit_power) {
    xml += RadxXml::writeDouble("XmitPowerDbmBoth", 1, _xmitPowerDbmBoth);
    xml += RadxXml::writeDouble("XmitPowerDbmH", 1, _xmitPowerDbmH);
    xml += RadxXml::writeDouble("XmitPowerDbmV", 1, _xmitPowerDbmV);
  }

  xml += RadxXml::writeEndTag("ClutterStats", 0);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Writing XML results to SPDB:" << endl;
    cerr << xml << endl;
  }

  DsSpdb spdb;
  time_t validTime = startTime.utime();
  spdb.addPutChunk(0, validTime, validTime, xml.size() + 1, xml.c_str());
  if (spdb.put(_params.spdb_output_url,
               SPDB_XML_ID, SPDB_XML_LABEL)) {
    cerr << "ERROR - RadxClutMon::_writeStatsToSpdb" << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Wrote to spdb, url: " << _params.spdb_output_url << endl;
  }

  return 0;

}


