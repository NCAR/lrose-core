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
// RadxDiffVol.cc
//
// RadxDiffVol object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2013
//
///////////////////////////////////////////////////////////////

#include "RadxDiffVol.hh"
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxPath.hh>
#include <Mdv/GenericRadxFile.hh>
#include <toolsa/file_io.h>
#include <sys/stat.h>
#include <cerrno>
#include <cmath>
using namespace std;

// Constructor

RadxDiffVol::RadxDiffVol(int argc, char **argv)
{

  OK = TRUE;
  _out = &cerr;

  // set programe name

  _progName = "RadxDiffVol";
  
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
  
  _readSearchTime = 0;

  if (_params.search_mode == Params::SEARCH_BY_TIME) {
    _readSearchTime =
      RadxTime::parseDateTime(_params.read_search_time);
    if (_readSearchTime == RadxTime::NEVER) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Cannot parse read_search_time: "
           << "\"" << _params.read_search_time << "\"" << endl;
      cerr << "Problem with TDRP parameters." << endl;
      OK = FALSE;
    }
  }
    
  return;

}

// destructor

RadxDiffVol::~RadxDiffVol()

{
  if (_outFile.is_open()) {
    _outFile.close();
  }
}

//////////////////////////////////////////////////
// Run

int RadxDiffVol::Run()
{

  // set the output device

  if (!strcmp(_params.output_file_path, "stdout")) {
    _out = &cout;
  } else if (!strcmp(_params.output_file_path, "stderr")) {
    _out = &cerr;
  } else {
    RadxPath opath(_params.output_file_path);
    if (ta_makedir_recurse(opath.getDirectory().c_str())) {
      int errNum = errno;
      cerr << "ERROR - RadxDiffVol::Run" << endl;
      cerr << "  Cannot create output dir: " << opath.getDirectory() << endl;
      cerr << strerror(errNum) << endl;
      return -1;
    }
    try {
      if (_params.append_to_output) {
        _outFile.open(_params.output_file_path, ios::out|ios::app);
      } else {
        _outFile.open(_params.output_file_path, ios::out);
      }
    }
    catch (std::ofstream::failure &e) {
      cerr << "ERROR - RadxDiffVol::Run" << endl;
      cerr << "Exception opening file: " << _params.output_file_path << endl;
      cerr << e.what() << endl;
      return -1;
    }
    _out = &_outFile;
  }

  // search by time or path?

  int iret = 0;
  if (_params.search_mode == Params::SEARCH_BY_TIME) {
    iret = _handleViaTime();
  } else {
    iret = _handleViaPaths(_params.file1_path, _params.file2_path);
  }

  if (iret) {
    *_out << "RadxDiffVol - FAILURE" << endl;
    return 1;
  } else {
    cerr << "RadxDiffVol - SUCCESS" << endl;
    return 0;
  }

}

//////////////////////////////////////////////////
// handle file via specified paths

int RadxDiffVol::_handleViaPaths(const string &path1,
                              const string &path2)
{

  _path1 = path1;
  _path2 = path2;
  
  if (_readFile(_path1, _vol1, true)) {
    *_out << "ERROR - RadxDiffVol::_handleViaPaths" << endl;
    return -1;
  }

  if (_readFile(_path2, _vol2, false)) {
    *_out << "ERROR - RadxDiffVol::_handleViaPaths" << endl;
    return -1;
  }
  
  // convert to floats for easy comparison

  _vol1.convertToFl32();
  _vol2.convertToFl32();

  if (_params.set_ngates_constant) {
    _vol1.setNGatesConstant();
    _vol2.setNGatesConstant();
  }

  if (_params.trim_surveillance_sweeps_to_360deg) {
    _vol1.trimSurveillanceSweepsTo360Deg();
    _vol2.trimSurveillanceSweepsTo360Deg();
  }

  // match range geometry if required

  if (_params.match_range_geometry) {
    _vol1.setNGatesConstant();
    _vol2.setNGatesConstant();
    _vol2.remapRangeGeom(_vol1.getStartRangeKm(), 
                         _vol1.getGateSpacingKm());
    _vol2.setNGates(_vol1.getMaxNGates());
  }

  // perform differencing

  if (_performDiff()) {
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////
// Handle search via specified time and search mode

int RadxDiffVol::_handleViaTime()
{

  string path1;
  if (_getPathForTime(_params.file1_dir, path1)) {
    *_out << "ERROR - RadxDiffVol::_handleViaTime()" << endl;
    return -1;
  }

  string path2;
  if (_getPathForTime(_params.file2_dir, path1)) {
    *_out << "ERROR - RadxDiffVol::_handleViaTime()" << endl;
    return -1;
  }

  if (_handleViaPaths(path1, path2)) {
    *_out << "ERROR - RadxDiffVol::_handleViaPaths" << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// read in file

int RadxDiffVol::_readFile(const string &path,
                        RadxVol &vol,
                        bool isFile1)

{
  
  // does file exist?

  struct stat fileStat;
  if (stat(path.c_str(), &fileStat)) {
    int errNum = errno;
    *_out << "ERROR - RadxDiffVol::_readFile" << endl;
    *_out << "  Cannot stat file: " << path << endl;
    *_out << strerror(errNum) << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Working on file: " << path << endl;
  }

  // set up read

  GenericRadxFile file;
  _setupRead(file, isFile1);

  // read in file
  
  if (file.readFromPath(path, vol)) {
    *_out << "ERROR - RadxDiffVol::_readFile" << endl;
    *_out << "  Reading file: " << path << endl;
    *_out << file.getErrStr() << endl;
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////////////////
// Handle search via specified time and search mode

int RadxDiffVol::_getPathForTime(const string &dir,
                              string &path)

{

  // Search for specified path
  
  RadxTimeList tlist;
  tlist.setDir(dir);
  switch (_params.time_mode) {
    case Params::READ_CLOSEST:
      tlist.setModeClosest(_readSearchTime,
                           _params.read_search_margin);
      break;
    case Params::READ_FIRST_BEFORE:
      tlist.setModeFirstBefore(_readSearchTime,
                               _params.read_search_margin);
      break;
    case Params::READ_FIRST_AFTER:
      tlist.setModeFirstAfter(_readSearchTime,
                              _params.read_search_margin);
      break;
    case Params::READ_LATEST:
    default:
      tlist.setModeLast();
  }
  
  if (_params.aggregate_sweep_files_on_read) {
    tlist.setReadAggregateSweeps(true);
  }

  if (tlist.compile()) {
    *_out << "ERROR - RadxDiffVol::_getPathForTime()" << endl;
    *_out << "  Cannot get file path via time list" << endl;
    *_out << "  Dir: " << dir << endl;
    *_out << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &pathList = tlist.getPathList();

  if (pathList.size() < 1) {
    *_out << "ERROR - RadxDiffVol::_getPathForTime()" << endl;
    *_out << "  No files found" << endl;
    return -1;
  }

  // use first path
  
  if (_params.debug) {
    cerr << "Getting path for dir: " << dir << endl;
    cerr << "  N paths found: " << pathList.size() << endl;
    for (size_t ii = 0; ii < pathList.size(); ii++) {
      cerr << "    " << pathList[ii] << endl;
    }
    cerr << "  Using path: " << pathList[0] << endl;
  }

  path = pathList[0];
  return 0;

}

//////////////////////////////////////////////////
// set up read

void RadxDiffVol::_setupRead(RadxFile &file, bool isFile1)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.read_set_fixed_angle_limits) {
    file.setReadFixedAngleLimits(_params.read_lower_fixed_angle,
                                 _params.read_upper_fixed_angle);
  } else if (_params.read_set_sweep_num_limits) {
    file.setReadSweepNumLimits(_params.read_lower_sweep_num,
                               _params.read_upper_sweep_num);
  }

  if (_params.specify_field_names) {
    for (int ii = 0; ii < _params.field_names_n; ii++) {
      if (isFile1) {
        file.addReadField(_params._field_names[ii].file1_field_name);
      } else {
        file.addReadField(_params._field_names[ii].file2_field_name);
      }
    }
  }

  if (_params.aggregate_sweep_files_on_read) {
    file.setReadAggregateSweeps(true);
  }
  
  if (_params.keep_long_range_rays) {
    file.setReadRemoveLongRange(false);
  } else {
    file.setReadRemoveLongRange(true);
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

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    if (isFile1) {
      cerr << "============ read request for file1" << endl;
    } else {
      cerr << "============ read request for file2" << endl;
    }
    file.printReadRequest(cerr);
  }
  
}

//////////////////////////////////////////////////
// perform the difference

int RadxDiffVol::_performDiff()
{

  _totalPoints = 0.0;
  _totalErrors = 0.0;

  int iret = 0;

  if (_params.debug) {
    cerr << "Performing diff on Radx-supported files" << endl;
    cerr << "  path1: " << _path1 << endl;
    cerr << "  path2: " << _path2 << endl;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "===================================" << endl;
    cerr << "File 1: " << _path1 << endl;
    _vol1.print(cerr);
    cerr << "===================================" << endl;
    cerr << "File 2: " << _path2 << endl;
    _vol2.print(cerr);
    cerr << "===================================" << endl;
  }

  if (_diffVolMetaData()) {
    iret = -1;
  }

  if (_params.check_sweeps) {
    if (_diffSweeps()) {
      iret = -1;
    }
  }

  if (_params.check_rays) {
    if (_diffRays()) {
      iret = -1;
    }
  }

  double percentBad = (_totalErrors / _totalPoints) * 100.0;
  if (percentBad > _params.min_percent_error_for_summary_report) {
    *_out << "Data value mismatch found" << endl;
    *_out << "  n total points: " << _totalPoints << endl;
    *_out << "  n bad   points: " << _totalErrors << endl;
    *_out << "  percent bad: " << percentBad << endl;
    iret = -1;
  }

  if (iret) {
    *_out << "Performing diff on Radx-supported files" << endl;
    *_out << "  path1: " << _path1 << endl;
    *_out << "  path2: " << _path2 << endl;
  }

  return iret;

}

//////////////////////////////////////////////////
// perform the difference on the volume metadata

int RadxDiffVol::_diffVolMetaData()
{

  int iret = 0;

  // volume number

  if (_params.check_volume_number) {
    if (_vol1.getVolumeNumber() != _vol2.getVolumeNumber()) {
      *_out << "Difference in volume number:" << endl;
      *_out << "  file1 volumeNumber: " << _vol1.getVolumeNumber() << endl;
      *_out << "  file2 volumeNumber: " << _vol2.getVolumeNumber() << endl;
      iret = -1;
    }
  }

  // time
  
  if (_vol1.getStartTimeSecs() != _vol2.getStartTimeSecs()) {
    *_out << "Difference in start time secs:" << endl;
    *_out << "  file1 startTimeSecs: "
          << RadxTime::strm(_vol1.getStartTimeSecs()) << endl;
    *_out << "  file2 startTimeSecs: "
          << RadxTime::strm(_vol2.getStartTimeSecs()) << endl;
    iret = -1;
  }
  
  if (fabs((double) _vol1.getStartNanoSecs() -
           (double) _vol2.getStartNanoSecs()) > 100) {
    *_out << "Difference in start time secs:" << endl;
    *_out << "  file1 startNanoSecs: " << (int) _vol1.getStartNanoSecs() << endl;
    *_out << "  file2 startNanoSecs: " << (int) _vol2.getStartNanoSecs() << endl;
    iret = -1;
  }

  if (_vol1.getEndTimeSecs() != _vol2.getEndTimeSecs()) {
    *_out << "Difference in end time secs:" << endl;
    *_out << "  file1 endTimeSecs: "
          << RadxTime::strm(_vol1.getEndTimeSecs()) << endl;
    *_out << "  file2 endTimeSecs: "
          << RadxTime::strm(_vol2.getEndTimeSecs()) << endl;
    iret = -1;
  }

  if (fabs((double) _vol1.getEndNanoSecs() -
           (double) _vol2.getEndNanoSecs()) > 100) {
    *_out << "Difference in end time secs:" << endl;
    *_out << "  file1 endNanoSecs: " << _vol1.getEndNanoSecs() << endl;
    *_out << "  file2 endNanoSecs: " << _vol2.getEndNanoSecs() << endl;
    iret = -1;
  }

  // rays

  if (_vol1.getNRays() != _vol2.getNRays()) {
    *_out << "Difference in number of rays:" << endl;
    *_out << "  file1 nRays: " << _vol1.getNRays() << endl;
    *_out << "  file2 nRays: " << _vol2.getNRays() << endl;
    iret = -1;
  }

  // fields

  vector<string> fields1 = _vol1.getUniqueFieldNameList();
  vector<string> fields2 = _vol2.getUniqueFieldNameList();

  if (fields1.size() != fields2.size()) {
    *_out << "Difference in number of fields:" << endl;
    *_out << "  file1 nFields: " << fields1.size() << endl;
    *_out << "  file2 nFields: " << fields2.size() << endl;
    iret = -1;
  } else {
    if (_params.check_field_names) {
      for (size_t ii = 0; ii < fields1.size(); ii++) {
        if (fields1[ii] != fields2[ii]) {
          *_out << "Difference in field names" << endl;
          *_out << "  file1 field[" << ii << "].name: " << fields1[ii] << endl;
          *_out << "  file2 field[" << ii << "].name: " << fields2[ii] << endl;
          iret = -1;
        }
      } // ii
    } 
  }

  return iret;

}


//////////////////////////////////////////////////
// perform the difference on the sweep details

int RadxDiffVol::_diffSweeps()
{

  int iret = 0;

  // sweeps

  if (_vol1.getNSweeps() != _vol2.getNSweeps()) {
    *_out << "Difference in volume NSweeps:" << endl;
    *_out << "  file1 NSweeps: " << _vol1.getNSweeps() << endl;
    *_out << "  file2 NSweeps: " << _vol2.getNSweeps() << endl;
    return -1;
  }

  const vector<RadxSweep *> &sweeps1 = _vol1.getSweeps();
  const vector<RadxSweep *> &sweeps2 = _vol2.getSweeps();

  for (size_t isweep = 0; isweep < sweeps1.size(); isweep++) {
    
    const RadxSweep *sweep1 = sweeps1[isweep];
    const RadxSweep *sweep2 = sweeps2[isweep];

    if (_diffSweeps(isweep, sweep1, sweep2)) {
      iret = -1;
    }

  } // isweep

  return iret;

}

//////////////////////////////////////////////////
// perform the difference on the sweep details

int RadxDiffVol::_diffSweeps(int isweep,
                          const RadxSweep *sweep1,
                          const RadxSweep *sweep2)
{

  int iret = 0;

  if (_params.check_sweep_numbers) {
    if (sweep1->getSweepNumber() != sweep2->getSweepNumber()) {
      *_out << "Difference in SweepNumber for sweep index: " << isweep << endl;
      *_out << "  sweep1 SweepNumber: " << sweep1->getSweepNumber() << endl;
      *_out << "  sweep2 SweepNumber: " << sweep2->getSweepNumber() << endl;
      iret = -1;
    }
  }
  
  if (sweep1->getNRays() != sweep2->getNRays()) {
    *_out << "Difference in NRays for sweep index: " << isweep << endl;
    *_out << "  sweep1 NRays: " << sweep1->getNRays() << endl;
    *_out << "  sweep2 NRays: " << sweep2->getNRays() << endl;
    iret = -1;
  }
  
  if (sweep1->getStartRayIndex() != sweep2->getStartRayIndex()) {
    *_out << "Difference in StartRayIndex for sweep index: " << isweep << endl;
    *_out << "  sweep1 StartRayIndex: " << sweep1->getStartRayIndex() << endl;
    *_out << "  sweep2 StartRayIndex: " << sweep2->getStartRayIndex() << endl;
    iret = -1;
  }
  
  if (sweep1->getEndRayIndex() != sweep2->getEndRayIndex()) {
    *_out << "Difference in EndRayIndex for sweep index: " << isweep << endl;
    *_out << "  sweep1 EndRayIndex: " << sweep1->getEndRayIndex() << endl;
    *_out << "  sweep2 EndRayIndex: " << sweep2->getEndRayIndex() << endl;
    iret = -1;
  }
  
  double angleDiff = fabs(sweep1->getFixedAngleDeg() -
                          sweep2->getFixedAngleDeg());
  if (angleDiff > 0.01) {
    *_out << "Difference in FixedAngleDeg for sweep index: " << isweep << endl;
    *_out << "  sweep1 FixedAngleDeg: " << sweep1->getFixedAngleDeg() << endl;
    *_out << "  sweep2 FixedAngleDeg: " << sweep2->getFixedAngleDeg() << endl;
    iret = -1;
  }
  
  if (sweep1->getRaysAreIndexed() != sweep2->getRaysAreIndexed()) {
    *_out << "Difference in RaysAreIndexed for sweep index: " << isweep << endl;
    *_out << "  sweep1 RaysAreIndexed: "
          << (char *) (sweep1->getRaysAreIndexed()?"Y":"N") << endl;
    *_out << "  sweep2 RaysAreIndexed: "
          << (char *) (sweep2->getRaysAreIndexed()?"Y":"N") << endl;
    iret = -1;
  }
  
  if (sweep1->getRaysAreIndexed()) {
    double resDiff = fabs(sweep1->getAngleResDeg() -
                          sweep2->getAngleResDeg());
    if (resDiff > 0.01) {
      *_out << "Difference in AngleResDeg for sweep index: " << isweep << endl;
      *_out << "  sweep1 AngleResDeg: " << sweep1->getAngleResDeg() << endl;
      *_out << "  sweep2 AngleResDeg: " << sweep2->getAngleResDeg() << endl;
      iret = -1;
    }
  }
  
  if (sweep1->getSweepMode() != sweep2->getSweepMode()) {
    *_out << "Difference in SweepMode for sweep index: " << isweep << endl;
    *_out << "  sweep1 SweepMode: "
          << Radx::sweepModeToStr(sweep1->getSweepMode()) << endl;
    *_out << "  sweep2 SweepMode: "
          << Radx::sweepModeToStr(sweep2->getSweepMode()) << endl;
    iret = -1;
  }

  if (iret) {
    *_out << "ERROR for sweep index: " << isweep << endl;
    *_out << "  sweep1 FixedAngleDeg: " << sweep1->getFixedAngleDeg() << endl;
    *_out << "  sweep2 FixedAngleDeg: " << sweep2->getFixedAngleDeg() << endl;
  }
  
  return iret;

}

//////////////////////////////////////////////////
// perform the difference on the ray details

int RadxDiffVol::_diffRays()
{

  int iret = 0;

  // rays

  if (_vol1.getNRays() != _vol2.getNRays()) {
    *_out << "Difference in volume NRays:" << endl;
    *_out << "  file1 NRays: " << _vol1.getNRays() << endl;
    *_out << "  file2 NRays: " << _vol2.getNRays() << endl;
    return -1;
  }

  const vector<RadxRay *> &rays1 = _vol1.getRays();
  const vector<RadxRay *> &rays2 = _vol2.getRays();
  
  for (size_t iray = 0; iray < rays1.size(); iray++) {
    
    const RadxRay *ray1 = rays1[iray];
    const RadxRay *ray2 = rays2[iray];

    if (_diffRays(iray, ray1, ray2)) {
      iret = -1;
    }

  } // iray

  return iret;

}

//////////////////////////////////////////////////
// perform the difference on the ray details

int RadxDiffVol::_diffRays(int iray,
                        const RadxRay *ray1,
                        const RadxRay *ray2)

{

  int iret = 0;
  
  if (_params.check_sweeps && _params.check_sweep_numbers) {
    if (ray1->getSweepNumber() != ray2->getSweepNumber()) {
      *_out << "Difference in SweepNumber for ray index: " << iray << endl;
      *_out << "  ray1 SweepNumber: " << ray1->getSweepNumber() << endl;
      *_out << "  ray2 SweepNumber: " << ray2->getSweepNumber() << endl;
      iret = -1;
    }
  }
  
  // time

  double maxTimeDiffSecs = _params.time_max_diff_for_match;
  double timeDiff = fabs(ray1->getTimeDouble() - ray2->getTimeDouble());
  if (timeDiff > maxTimeDiffSecs) {
    *_out << "Difference in Time for ray index: " << iray << endl;
    *_out << "  ray1 TimeDouble: " << ray1->getTimeDouble() << endl;
    *_out << "  ray2 TimeDouble: " << ray2->getTimeDouble() << endl;
    *_out << "  ray1 TimeSecs: "
          << RadxTime::strm(ray1->getTimeSecs()) << endl;
    *_out << "  ray2 TimeSecs: "
          << RadxTime::strm(ray1->getTimeSecs()) << endl;
    *_out << "  ray1 NanoSecs: " << (int) ray1->getNanoSecs() << endl;
    *_out << "  ray2 NanoSecs: " << (int) ray2->getNanoSecs() << endl;
    iret = -1;
  }

  // angles

  double azDiff = fabs(ray1->getAzimuthDeg() - ray2->getAzimuthDeg());
  if (azDiff > _params.angle_max_diff_for_match) {
    *_out << "Difference in AzimuthDeg for ray index: " << iray << endl;
    *_out << "  ray1 AzimuthDeg: " << ray1->getAzimuthDeg() << endl;
    *_out << "  ray2 AzimuthDeg: " << ray2->getAzimuthDeg() << endl;
    iret = -1;
  }
  
  double elDiff = fabs(ray1->getElevationDeg() - ray2->getElevationDeg());
  if (elDiff > _params.angle_max_diff_for_match) {
    *_out << "Difference in ElevationDeg for ray index: " << iray << endl;
    *_out << "  ray1 ElevationDeg: " << ray1->getElevationDeg() << endl;
    *_out << "  ray2 ElevationDeg: " << ray2->getElevationDeg() << endl;
    iret = -1;
  }
  
  if (ray1->getAntennaTransition() != ray2->getAntennaTransition()) {
    *_out << "Difference in AntennaTransition for ray index: " << iray << endl;
    *_out << "  ray1 AntennaTransition: "
          << (char *) (ray1->getAntennaTransition()?"Y":"N") << endl;
    *_out << "  ray2 AntennaTransition: "
          << (char *) (ray2->getAntennaTransition()?"Y":"N") << endl;
    iret = -1;
  }
  
  double nyDiff = fabs(ray1->getNyquistMps() - ray2->getNyquistMps());
  if (nyDiff > _params.nyquist_max_diff_for_match) {
    *_out << "Difference in NyquistMps for ray index: " << iray << endl;
    *_out << "  ray1 NyquistMps: " << ray1->getNyquistMps() << endl;
    *_out << "  ray2 NyquistMps: " << ray2->getNyquistMps() << endl;
    iret = -1;
  }
  
  if (ray1->getNGates() != ray2->getNGates()) {
    *_out << "Difference in NGates for ray index: " << iray << endl;
    *_out << "  ray1 NGates: " << ray1->getNGates() << endl;
    *_out << "  ray2 NGates: " << ray2->getNGates() << endl;
    iret = -1;
  }
  
  if (_params.check_fields) {
    if (_diffFields(iray, ray1, ray2)) {
      iret = -1;
    }
  } // if (_params.check_fields)
  
  if (iret) {
    *_out << "ERROR for ray index: " << iray << endl;
    *_out << "  ray1 el, az: "
          << ray1->getElevationDeg() << ", "
          << ray1->getAzimuthDeg() << endl;
    *_out << "  ray2 el, az: "
          << ray2->getElevationDeg() << ", "
          << ray2->getAzimuthDeg() << endl;
  }
  
  return iret;

}

//////////////////////////////////////////////////
// perform the difference on the fields in a ray

int RadxDiffVol::_diffFields(int iray,
                          const RadxRay *ray1,
                          const RadxRay *ray2)

{
  
  int iret = 0;
  
  if (_params.check_number_of_fields &&
      ray1->getFields().size() != ray2->getFields().size()) {
    
    *_out << "Difference in NFields for ray index: " << iray << endl;
    *_out << "  ray1 NFields: " << ray1->getFields().size() << endl;
    *_out << "  ray2 NFields: " << ray2->getFields().size() << endl;
    iret = -1;
    
  } else if (_params.check_fields) {
    
    size_t nFieldsCheck = ray1->getFields().size();
    if (nFieldsCheck > ray2->getFields().size()) {
      nFieldsCheck = ray2->getFields().size();
    }
    for (size_t ifield = 0; ifield < nFieldsCheck; ifield++) {
      
      const RadxField *field1 = ray1->getFields()[ifield];
      const RadxField *field2 = ray2->getFields()[ifield];
      bool doCheck = true;
      if (_params.check_field_names) {
        field2 = ray2->getField(field1->getName());
        if (field2 == NULL) {
          *_out << "Difference in field names for ray index: " << iray << endl;
          *_out << "                            field index: " << ifield << endl;
          *_out << "  field1 name: " << field1->getName() << endl;
          field2 = ray2->getFields()[ifield];
          if (field2) {
            *_out << "  field2 name: " << field2->getName() << endl;
          }
          iret = -1;
          doCheck = false;
        }
      }
      
      if (doCheck) {
        if (_diffFields(iray, ifield, field1, field2)) {
          iret = -1;
        }
      }
      
    } // ifield
    
  }

  return iret;

}

///////////////////////////////////////////////////////
// diff fields for given ray and field index

int RadxDiffVol::_diffFields(int iray,
                          int ifield,
                          const RadxField *field1,
                          const RadxField *field2)

{

  int iret = 0;

  if (_params.check_field_units) {
    string units1 = field1->getUnits();
    string units2 = field2->getUnits();
    if (units1 != units2) {
      *_out << "Difference in units for field index: " << ifield << endl;
      *_out << "  field1 units: " << units1 << endl;
      *_out << "  field2 units: " << units2 << endl;
      iret = -1;
    }
  }

  if (field1->getNPoints() != field2->getNPoints()) {
    *_out << "Difference in NPoints for field index: " << ifield << endl;
    *_out << "  field1 NPoints: " << field1->getNPoints() << endl;
    *_out << "  field2 NPoints: " << field2->getNPoints() << endl;
    iret = -1;
  }

  if (iret || !_params.check_field_data) {
    return iret;
  }

  Radx::fl32 missing1 = field1->getMissingFl32();
  Radx::fl32 missing2 = field2->getMissingFl32();

  const Radx::fl32 *data1 = field1->getDataFl32();
  const Radx::fl32 *data2 = field2->getDataFl32();

  for (size_t ii = 0; ii < field1->getNPoints(); ii++) {

    _totalPoints++;
    bool error = false;
    if (data1[ii] == missing1 || data2[ii] == missing2) {
      if (data1[ii] != missing1 || data2[ii] != missing2) {
        error = true;
      }
    } else {
      double diff = fabs(data1[ii] - data2[ii]);
      if (diff > _params.field_data_max_diff_for_match) {
        error = true;
      }
    }

    if (error) {
      if (_params.report_all_field_data_diffs) {
        *_out << "  Mismatch in data vals, field, ipoint, data1, data2: "
              << field1->getName() << ", "
              << ii << ", "
              << data1[ii] << ", "
              << data2[ii] << endl;
      }
      _totalErrors++;
    }

  } // ii

  return iret;

}
