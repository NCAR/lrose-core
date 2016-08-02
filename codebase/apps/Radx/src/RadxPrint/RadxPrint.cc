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
// RadxPrint.cc
//
// RadxPrint object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2010
//
///////////////////////////////////////////////////////////////

#include "RadxPrint.hh"
#include <Radx/RadxPath.hh>
#include <Radx/RadxVol.hh>
#include <Radx/DoradeRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxRay.hh>
#include <Mdv/GenericRadxFile.hh>
#include <sys/stat.h>
#include <cerrno>
using namespace std;

// Constructor

RadxPrint::RadxPrint(int argc, char **argv)
  
{

  OK = TRUE;

  // set programe name

  _progName = "RadxPrint";
  
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

  // parse the DateTime objects

  if (_params.specify_file_by_time) {
    
    if (_params.read_search_mode == Params::READ_RAYS_IN_INTERVAL) {

      _readStartTime.set(_params.read_start_time);
      if (_readStartTime.utime() == RadxTime::NEVER) {
        cerr << "ERROR: " << _progName << endl;
        cerr << "  Cannot parse read_start_time: "
             << "\"" << _params.read_start_time << "\"" << endl;
        cerr << "Problem with TDRP parameters." << endl;
        OK = FALSE;
      }
      
      _readEndTime.set(_params.read_end_time);
      if (_readEndTime.utime() == RadxTime::NEVER) {
        cerr << "ERROR: " << _progName << endl;
        cerr << "  Cannot parse read_end_time: "
             << "\"" << _params.read_end_time << "\"" << endl;
        cerr << "Problem with TDRP parameters." << endl;
        OK = FALSE;
      }
      
    } else {

      _readSearchTime.set(_params.read_search_time);
      if (_readSearchTime.utime() == RadxTime::NEVER) {
        cerr << "ERROR: " << _progName << endl;
        cerr << "  Cannot parse read_search_time: "
             << "\"" << _params.read_search_time << "\"" << endl;
        cerr << "Problem with TDRP parameters." << endl;
        OK = FALSE;
      }

    }

  } // if (_params.specify_file_by_time)

  return;

}

// destructor

RadxPrint::~RadxPrint()

{


}

//////////////////////////////////////////////////
// Run

int RadxPrint::Run()
{

  // special case - print DORADE format

  if (_params.print_mode == Params::PRINT_MODE_DORADE_FORMAT) {
    DoradeData::printAllFormats(stdout);
    return 0;
  }

  // search by time or path?

  if (_params.specify_file_by_time) {
    return _handleViaTime();
  } else {
    return _handleViaPath(_params.path);
  }

}

//////////////////////////////////////////////////
// handle file via specified path

int RadxPrint::_handleViaPath(const string &path)
{

  // does file exist?

  struct stat fileStat;
  if (stat(path.c_str(), &fileStat)) {
    int errNum = errno;
    cerr << "ERROR - RadxPrint::_handleViaPath" << endl;
    cerr << "  Cannot stat file: " << path << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // set up file

  GenericRadxFile file;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setDebug(true);
    file.setVerbose(true);
  }

  if (_params.debug) {
    cerr << "Working on file: " << path << endl;
  }

  // native print?

  if (_params.print_mode == Params::PRINT_MODE_NATIVE) {

    if (file.printNative(path, cout,
                         _params.print_rays,
                         _params.print_data)) {
      cerr << "ERROR - RadxPrint::_handleViaPath" << endl;
      cerr << "  Printing file natively: " << path << endl;
      cerr << file.getErrStr() << endl;
      return -1;
    }

    return 0;

  }

  // set up read

  _setupRead(file);
  
  if (_params.aggregate_sweep_files_on_read) {
    file.setReadAggregateSweeps(true);
  } else {
    file.setReadAggregateSweeps(false);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.printReadRequest(cerr);
  }

  // read into vol

  RadxVol vol;
  
  if (!_params.aggregate_all_files_on_read) {

    // read in file
    
    if (file.readFromPath(path, vol)) {
      cerr << "ERROR - RadxPrint::_handleViaPath" << endl;
      cerr << "  Printing file: " << path << endl;
      cerr << file.getErrStr() << endl;
      return -1;
    }
    _readPaths = file.getReadPaths();
    if (_params.debug) {
      for (size_t ii = 0; ii < _readPaths.size(); ii++) {
        cerr << "  ==>> read in file: " << _readPaths[ii] << endl;
      }
    }

  } else {

    // aggregate files on read
    
    _readPaths = _args.inputFileList;
    if (file.aggregateFromPaths(_readPaths, vol)) {
      cerr << "ERROR - RadxPrint::_handleViaPath" << endl;
      cerr << "Aggregating files on read" << endl;
      cerr << "  paths: " << endl;
      for (size_t ii = 0; ii < _readPaths.size(); ii++) {
        cerr << "         " << _readPaths[ii] << endl;
      }
      return -1;
    }
    if (_params.debug) {
      cerr << "Aggregating files on read" << endl;
      for (size_t ii = 0; ii < _readPaths.size(); ii++) {
        cerr << "==>> read in path: " << _readPaths[ii] << endl;
      }
    }

  }

  // set number of gates constant if requested

  if (_params.set_ngates_constant) {
    vol.setNGatesConstant();
  }

  // trim to 360s if requested

  if (_params.trim_surveillance_sweeps_to_360deg) {
    vol.trimSurveillanceSweepsTo360Deg();
  }

  // do print

  _printVol(vol);

  return 0;

}

////////////////////////////////////////////////////
// Handle search via specified time and search mode

int RadxPrint::_handleViaTime()
{

  GenericRadxFile file;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setDebug(true);
    file.setVerbose(true);
  }

  // set up read

  _setupRead(file);

  if (_params.aggregate_sweep_files_on_read) {
    file.setReadAggregateSweeps(true);
  } else {
    file.setReadAggregateSweeps(false);
  }

  RadxField::StatsMethod_t dwellStatsMethod;
  switch (_params.read_dwell_stats) {
    case Params::DWELL_STATS_MEAN:
      dwellStatsMethod = RadxField::STATS_METHOD_MEAN;
      break;
    case Params::DWELL_STATS_MEDIAN:
      dwellStatsMethod = RadxField::STATS_METHOD_MEDIAN;
      break;
    case Params::DWELL_STATS_MAXIMUM:
      dwellStatsMethod = RadxField::STATS_METHOD_MAXIMUM;
      break;
    case Params::DWELL_STATS_MINIMUM:
      dwellStatsMethod = RadxField::STATS_METHOD_MINIMUM;
      break;
    case Params::DWELL_STATS_MIDDLE:
    default:
      dwellStatsMethod = RadxField::STATS_METHOD_MIDDLE;
      break;
  }
  
  switch (_params.read_search_mode) {
    case Params::READ_RAYS_IN_INTERVAL:
      file.setReadRaysInInterval(_readStartTime,
                                 _readEndTime,
                                 _params.read_dwell_secs,
                                 dwellStatsMethod);
      break;
    case Params::READ_CLOSEST:
      file.setReadModeClosest(_readSearchTime.utime(),
                              _params.read_search_margin);
      break;
    case Params::READ_FIRST_BEFORE:
      file.setReadModeFirstBefore(_readSearchTime.utime(),
                                  _params.read_search_margin);
      break;
    case Params::READ_FIRST_AFTER:
      file.setReadModeFirstAfter(_readSearchTime.utime(),
                                 _params.read_search_margin);
      break;
    case Params::READ_LATEST:
    default:
      file.setReadModeLast();
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.printReadRequest(cerr);
  }
  
  // perform the read
  
  RadxVol vol;
  if (file.readFromDir(_params.dir, vol)) {
    cerr << "ERROR - RadxPrint::_handleViaTime" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }

  _readPaths = file.getReadPaths();
  if (_params.debug) {
    for (size_t ii = 0; ii < _readPaths.size(); ii++) {
      cerr << "  ==>> read in file: " << _readPaths[ii] << endl;
    }
  }

  // set number of gates constant if requested

  if (_params.set_ngates_constant) {
    vol.setNGatesConstant();
  }

  // trim to 360s if requested

  if (_params.trim_surveillance_sweeps_to_360deg) {
    vol.trimSurveillanceSweepsTo360Deg();
  }

  // do print

  _printVol(vol);

  return 0;

}

//////////////////////////////////////////////////
// set up read

void RadxPrint::_setupRead(RadxFile &file)
{

  if (_params.read_set_fixed_angle_limits) {
    file.setReadFixedAngleLimits(_params.read_lower_fixed_angle,
                                 _params.read_upper_fixed_angle);
    if (_params.read_lower_fixed_angle == _params.read_upper_fixed_angle) {
      // relax strict angle checking since only a single angle is specified
      // which means the user wants the closest angle
      file.setReadStrictAngleLimits(false);
    }
  } else if (_params.read_set_sweep_num_limits) {
    file.setReadSweepNumLimits(_params.read_lower_sweep_num,
                               _params.read_upper_sweep_num);
  }

  if (!_params.read_apply_strict_angle_limits) {
    file.setReadStrictAngleLimits(false);
  }

  if (_params.read_set_field_names) {
    for (int i = 0; i < _params.read_field_names_n; i++) {
      file.addReadField(_params._read_field_names[i]);
    }
  }

  if (_params.ignore_antenna_transitions) {
    file.setReadIgnoreTransitions(true);
  } else {
    file.setReadIgnoreTransitions(false);
  }

  if (_params.read_meta_data_only) {
    file.setReadMetadataOnly(true);
  }

  if (_params.remove_rays_with_all_data_missing) {
    file.setReadRemoveRaysAllMissing(true);
  } else {
    file.setReadRemoveRaysAllMissing(false);
  }

  if (_params.preserve_sweeps) {
    file.setReadPreserveSweeps(true);
  } else {
    file.setReadPreserveSweeps(false);
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

  if (_params.set_max_range) {
    file.setReadMaxRangeKm(_params.max_range_km);
  }

  if (_params.change_radar_latitude_sign) {
    file.setChangeLatitudeSignOnRead(true);
  }

  if (_params.apply_georeference_corrections) {
    file.setApplyGeorefsOnRead(true);
  }

  if (_params.remove_rays_with_antenna_transitions &&
      !_params.trim_surveillance_sweeps_to_360deg) {
    file.setReadIgnoreTransitions(true);
  }
  
  if (_params.read_set_radar_num) {
    file.setRadarNumOnRead(_params.read_radar_num);
  }

}

//////////////////////////////////////////////////
// perform print

void RadxPrint::_printVol(RadxVol &vol)
{

  if (_params.load_volume_fields_from_rays) {
    vol.loadFieldsFromRays();
  }
  
  if (_params.print_ray_table) {
    _printRayTable(cout, vol);
  } else if (_params.print_data) {
    vol.printWithFieldData(cout);
  } else if (_params.print_rays) {
    vol.printWithRayMetaData(cout);
  } else if (_params.print_ray_summary) {
    vol.printRaySummary(cout);
  } else {
    vol.print(cout);
  }

}

//////////////////////////////////////////////////
// print out ray table

void RadxPrint::_printRayTable(ostream &out, const RadxVol &vol)
{

  const vector<RadxRay *> rays = vol.getRays();

  for (size_t ii = 0; ii < rays.size(); ii++) {

    const RadxRay &ray = *rays[ii];

    RadxTime rtime(ray.getTimeSecs());

    cout << rtime.getYear() << " "
         << rtime.getMonth() << " "
         << rtime.getDay() << " "
         << rtime.getHour() << " "
         << rtime.getMin() << " "
         << rtime.getSec() << " "
         << ray.getNanoSecs() * 1.0e-9 << " "
         << ray.getFixedAngleDeg() << " "
         << ray.getElevationDeg() << " "
         << ray.getAzimuthDeg() << " "
         << ray.getAntennaTransition() << " "
         << ray.getNGates() << " "
         << ray.getStartRangeKm() << " "
         << ray.getGateSpacingKm() << " "
         << ray.getSweepNumber() << " "
         << ray.getPrtSec() << " "
         << ray.getNyquistMps() << " "
         << ray.getUnambigRangeKm() << " "
         << ray.getMeasXmitPowerDbmH() << " "
         << ray.getMeasXmitPowerDbmV() << " "
         << ray.getEstimatedNoiseDbmHc() << " "
         << ray.getEstimatedNoiseDbmVc() << " "
         << ray.getEstimatedNoiseDbmHx() << " "
         << ray.getEstimatedNoiseDbmVx() << " "
         << ray.getEventFlagsSet() << " "
         << ray.getStartOfSweepFlag() << " "
         << ray.getEndOfSweepFlag() << " "
         << ray.getStartOfVolumeFlag() << " "
         << ray.getEndOfVolumeFlag() << endl;

  } // ii

}

