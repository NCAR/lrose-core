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
// RadxMergeVols.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2016
//
///////////////////////////////////////////////////////////////
//
// Merges volumes from multiple CfRadial files into a single file
//
////////////////////////////////////////////////////////////////

#include "RadxMergeVols.hh"
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxRay.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DataFileNames.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/pmu.h>
using namespace std;

// Constructor

RadxMergeVols::RadxMergeVols(int argc, char **argv)
  
{

  OK = TRUE;

  // set programe name

  _progName = "RadxMergeVols";
  
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

  _serialStartIndex = -1;
  _serialThisIndex = -1;
  _firstFile = true;
  _volInProgress = false;

}

// destructor

RadxMergeVols::~RadxMergeVols()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RadxMergeVols::Run()
{

  if (_params.mode == Params::ARCHIVE) {
    return _runArchive();
  } else if (_params.mode == Params::FILELIST) {
    return _runFilelist();
  } else {
    if (_params.latest_data_info_avail) {
      return _runRealtimeWithLdata();
    } else {
      return _runRealtimeNoLdata();
    }
  }
}

//////////////////////////////////////////////////
// Run in filelist mode

int RadxMergeVols::_runFilelist()
{

  // loop through the input file list

  int iret = 0;

  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {

    string inputPath = _args.inputFileList[ii];
    if (_processFile(inputPath)) {
      iret = -1;
    }

  }

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int RadxMergeVols::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.primary_dataset_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (_params.aggregate_sweep_files_on_read) {
    tlist.setReadAggregateSweeps(true);
  }
  if (tlist.compile()) {
    cerr << "ERROR - RadxMergeVols::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: "
         << _params.primary_dataset_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - RadxMergeVols::_runFilelist()" << endl;
    cerr << "  No files found, dir: "
         << _params.primary_dataset_dir << endl;
    return -1;
  }
  
  // loop through the input file list
  
  int iret = 0;
  for (size_t ii = 0; ii < paths.size(); ii++) {
    if (_processFile(paths[ii])) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode with latest data info

int RadxMergeVols::_runRealtimeWithLdata()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  // watch for new data to arrive

  LdataInfo ldata(_params.primary_dataset_dir,
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
// Run in realtime mode without latest data info

int RadxMergeVols::_runRealtimeNoLdata()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);
  
  // Set up input path

  DsInputPath input(_progName,
		    _params.debug >= Params::DEBUG_VERBOSE,
		    _params.primary_dataset_dir,
		    _params.max_realtime_data_age_secs,
		    PMU_auto_register,
		    _params.latest_data_info_avail,
		    false);

  input.setFileQuiescence(_params.file_quiescence);
  input.setRecursion(_params.search_recursively);
  input.setMaxRecursionDepth(_params.max_recursion_depth);
  input.setMaxDirAge(_params.max_realtime_data_age_secs);

  int iret = 0;
  RadxVol vol;

  while(true) {

    // check for new data
    
    char *path = input.next(false);
    
    if (path == NULL) {
      
      // sleep a bit
      
      PMU_auto_register("Waiting for data");
      umsleep(_params.wait_between_checks * 1000);

    } else {
    
      // got a file

      if (_processFile(path)) {
        iret = -1;
      }

    }
    
  } // while

  return iret;

}

//////////////////////////////////////////////////
// Process a file
// Returns 0 on success, -1 on failure

int RadxMergeVols::_processFile(const string &primaryPath)
{

  if (_params.merge_method == Params::MERGE_PARALLEL) {
    return _processFileParallel(primaryPath);
  } else {
    return _processFileSerial(primaryPath);
  }

}
  
//////////////////////////////////////////////////
// Process a file using the parallel method
// Returns 0 on success, -1 on failure

int RadxMergeVols::_processFileParallel(const string &primaryPath)
{

  if (_params.debug) {
    cerr << "INFO - RadxMergeVols::_processFileParallel" << endl;
    cerr << "  Input path primary file: " << primaryPath << endl;
  }
  
  // read in primary file
  
  GenericRadxFile primaryFile;
  _setupRead(primaryFile);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===== SETTING UP READ FOR PRIMARY FILES =====" << endl;
    primaryFile.printReadRequest(cerr);
    cerr << "=============================================" << endl;
  }
  
  RadxVol primaryVol;
  if (primaryFile.readFromPath(primaryPath, primaryVol)) {
    cerr << "ERROR - RadxMergeVols::_processFileParallel" << endl;
    cerr << "  Cannot read in primary file: " << primaryPath << endl;
    cerr << primaryFile.getErrStr() << endl;
    return -1;
  }

  time_t primaryTime = primaryVol.getEndTimeSecs();
  bool dateOnly;
  if (DataFileNames::getDataTime(primaryPath, primaryTime, dateOnly)) {
    cerr << "ERROR - RadxMergeVols::_processFileParallel" << endl;
    cerr << "  Cannot get time from file path: " << primaryPath << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Time for primary file: " << RadxTime::strm(primaryTime) << endl;
  }

  // Search for secondary files
  
  for (int idata = 0; idata < _params.secondary_datasets_n; idata++) {

    const Params::secondary_dataset_t &secondary = _params._secondary_datasets[idata];
    
    time_t searchTime = primaryTime + secondary.file_match_time_offset_sec;
    
    RadxTimeList tlist;
    tlist.setDir(secondary.dir);
    if (secondary.find_mode == Params::FIND_CLOSEST) {
      tlist.setModeClosest(searchTime, secondary.file_match_time_tolerance_sec);
    } else if (secondary.find_mode == Params::FIND_FIRST_BEFORE) {
      tlist.setModeFirstBefore(searchTime, secondary.file_match_time_tolerance_sec);
    } else if (secondary.find_mode == Params::FIND_FIRST_AFTER) {
      tlist.setModeFirstAfter(searchTime, secondary.file_match_time_tolerance_sec);
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      tlist.printRequest(cerr);
    }
    
    if (tlist.compile()) {
      cerr << "ERROR - RadxMergeVols::_processFileParallel()" << endl;
      cerr << "  Cannot compile secondary file time list" << endl;
      cerr << tlist.getErrStr() << endl;
      return -1;
    }
    const vector<string> &pathList = tlist.getPathList();
    if (pathList.size() < 1) {
      cerr << "WARNING - RadxMergeVols::_processFileParallel()" << endl;
      cerr << "  No suitable secondary file found" << endl;
      cerr << "  Primary file: " << primaryPath << endl;
      return -1;
    }
    
    // read in secondary file, using first path in list
    
    string secondaryPath = pathList[0];
    if (_params.debug) {
      cerr << "Found secondary file: " << secondaryPath << endl;
    }
    GenericRadxFile secondaryFile;
    _setupRead(secondaryFile);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "===== SETTING UP READ FOR SECONDARY FILES =====" << endl;
      secondaryFile.printReadRequest(cerr);
      cerr << "===============================================" << endl;
    }
    RadxVol secondaryVol;
    if (secondaryFile.readFromPath(secondaryPath, secondaryVol)) {
      cerr << "ERROR - RadxMergeVols::_processFileParallel" << endl;
      cerr << "  Cannot read in secondary file: " << secondaryPath << endl;
      cerr << secondaryFile.getErrStr() << endl;
      return -1;
    }

    // merge the primary and seconday volumes, using the primary
    // volume to hold the merged data
    
    if (_mergeVol(primaryVol, secondaryVol)) {
      cerr << "ERROR - RadxMergeVols::_processFileParallel" << endl;
      cerr << "  Merge failed" << endl;
      cerr << "  Primary file: " << primaryPath << endl;
      cerr << "  Secondary file: " << secondaryPath << endl;
      return -1;
    }

  } // idata

  // finalize the volume

  primaryVol.setPackingFromRays();
  primaryVol.loadVolumeInfoFromRays();
  primaryVol.loadSweepInfoFromRays();
  primaryVol.remapToPredomGeom();
  
  // write out file

  if (_writeVol(primaryVol)) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// Process a file using the serial method
// Returns 0 on success, -1 on failure

int RadxMergeVols::_processFileSerial(const string &serialPath)
{

  if (_params.debug) {
    cerr << "INFO - RadxMergeVols::_processFileSerial" << endl;
    cerr << "  Input path serial file: " << serialPath << endl;
  }
  
  // read in file
  
  GenericRadxFile serialFile;
  _setupRead(serialFile);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===== SETTING UP READ FOR FILES =====" << endl;
    serialFile.printReadRequest(cerr);
    cerr << "=====================================" << endl;
  }
  
  RadxVol latestVol;
  if (serialFile.readFromPath(serialPath, latestVol)) {
    cerr << "ERROR - RadxMergeVols::_processFileSerial" << endl;
    cerr << "  Cannot read in serial file: " << serialPath << endl;
    cerr << serialFile.getErrStr() << endl;
    return -1;
  }

  // check if this vol is one of the request types
  
  int typeNum = -1;
  string latestTitle = latestVol.getTitle();
  string latestScanName = latestVol.getScanName();
  for (int itype = 0; itype < _params.serial_vol_types_n; itype++) {
    Params::serial_vol_type_t type = _params._serial_vol_types[itype];
    string searchTitle = type.vol_title;
    string searchScanName = type.scan_name;
    if (searchTitle.size() == 0 && searchScanName.size() == 0) {
      // none specified
      continue;
    }
    if (searchTitle.size() > 0) {
      if (searchTitle != latestTitle) {
        // wrong title
        continue;
      }
    }
    if (searchScanName.size() > 0) {
      if (searchScanName != latestScanName) {
        // wrong scanName
        continue;
      }
    }
    // success
    typeNum = itype;
    break;
  } // itype

  if (typeNum < 0) {
    // not wanted
    return -1;
  }

  if (typeNum == 0 && _volInProgress) {

    // indicates we have found the vol type that is not first in the list
    
    if (!_firstFile) {
      // write out current data
      if (_writeVol(_mergedVol)) {
        _mergedVol.clear();
        return -1;
      }
    }
    _firstFile = false;

    // initialize merged vol with the latest vol
    
    _mergedVol.clear();
    _mergedVol = latestVol;
    _mergedVol.setTitle(_params.serial_merge_vol_title);
    _mergedVol.setScanName(_params.serial_merge_scan_name);

    // clear secondary flag - we are starting a new vol

    _volInProgress = false;
    
    if (_params.debug) {
      cerr << "Init mergeVol from path: " << serialPath << endl;
      cerr << "  title, scanName: " << latestTitle << ", " << latestScanName << endl;
    }

  } else {
    
    if (typeNum != 0) {
      // indicates we have found the vol type that is not first in the list
      // so set flag to indicate we have moved past the first vol type
      _volInProgress = true;
    }
    
    // later vol type, copy the rays across
    
    size_t nSweepsSoFar = _mergedVol.getNSweeps();
    vector<RadxRay *> &latestRays = latestVol.getRays();
    for (size_t iray = 0; iray < latestRays.size(); iray++) {
      RadxRay *newRay = new RadxRay(*latestRays[iray]);
      int sweepNum = newRay->getSweepNumber() + nSweepsSoFar;
      newRay->setSweepNumber(sweepNum);
      _mergedVol.addRay(newRay);
    }
    _mergedVol.loadSweepInfoFromRays();
    _mergedVol.loadVolumeInfoFromRays();

    if (_params.debug) {
      cerr << "Adding vol to merge, path: " << serialPath << endl;
      cerr << "  title, scanName: " << latestTitle << ", " << latestScanName << endl;
    }

  }
  
  // if (typeNum == _params.serial_vol_types_n - 1) {
  //   // last type, write out
  //   if (_writeVol(_mergedVol)) {
  //     _mergedVol.clear();
  //     return -1;
  //   }
  //   _mergedVol.clear();
  // }

  return 0;

}

//////////////////////////////////////////////////
// set up file read

void RadxMergeVols::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.set_fixed_angle_limits) {
    file.setReadFixedAngleLimits(_params.lower_fixed_angle_limit,
                                 _params.upper_fixed_angle_limit);
  } else if (_params.set_sweep_num_limits) {
    file.setReadSweepNumLimits(_params.lower_sweep_num,
                               _params.upper_sweep_num);
  }

  if (_params.aggregate_sweep_files_on_read) {
    file.setReadAggregateSweeps(true);
  } else {
    file.setReadAggregateSweeps(false);
  }

}

//////////////////////////////////////////////////
// set up write

void RadxMergeVols::_setupWrite(RadxFile &file)
{

  if (_params.debug) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.output_filename_mode == Params::START_TIME_ONLY) {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_TIME_ONLY);
  } else if (_params.output_filename_mode == Params::END_TIME_ONLY) {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_END_TIME_ONLY);
  } else {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_AND_END_TIMES);
  }

  file.setWriteCompressed(true);
  file.setCompressionLevel(_params.compression_level);
  file.setWriteNativeByteOrder(false);

  // set output format

  switch (_params.output_format) {
    case Params::OUTPUT_FORMAT_UF:
      file.setFileFormat(RadxFile::FILE_FORMAT_UF);
      break;
    case Params::OUTPUT_FORMAT_DORADE:
      file.setFileFormat(RadxFile::FILE_FORMAT_DORADE);
      break;
    case Params::OUTPUT_FORMAT_FORAY:
      file.setFileFormat(RadxFile::FILE_FORMAT_FORAY_NC);
      break;
    case Params::OUTPUT_FORMAT_MDV_RADIAL:
      file.setFileFormat(RadxFile::FILE_FORMAT_MDV_RADIAL);
      break;
    default:
    case Params::OUTPUT_FORMAT_CFRADIAL:
      file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  }

  // set netcdf format - used for CfRadial

  file.setNcFormat(RadxFile::NETCDF4);
  
  if (_params.output_force_ngates_vary) {
    file.setWriteForceNgatesVary(true);
  }

}

//////////////////////////////////////////////////
// write out the volume

int RadxMergeVols::_writeVol(RadxVol &vol)
{

  // ensure radar name is only alphanumeric

  string radarName = vol.getInstrumentName();
  for (size_t ii = 0; ii < radarName.size(); ii++) {
    if (!isalnum(radarName[ii])) {
      radarName[ii] = '_';
    }
  }
  vol.setInstrumentName(radarName);

  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  if (_params.output_filename_mode == Params::SPECIFY_FILE_NAME) {

    string outPath = _params.output_dir;
    outPath += PATH_DELIM;
    outPath += _params.output_filename;

    // write to path
  
    if (outFile.writeToPath(vol, outPath)) {
      cerr << "ERROR - RadxMergeVols::_writeVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
      
  } else {

    // write to dir
  
    if (outFile.writeToDir(vol, _params.output_dir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - RadxMergeVols::_writeVol" << endl;
      cerr << "  Cannot write file to dir: " << _params.output_dir << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }

  }

  string outputPath = outFile.getPathInUse();

  // write latest data info file if requested 
  
  if (_params.mode == Params::REALTIME) {
    DsLdataInfo ldata(_params.output_dir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    string relPath;
    RadxPath::stripDir(_params.output_dir, outputPath, relPath);
    ldata.setRelDataPath(relPath);
    ldata.setWriter(_progName);
    if (ldata.write(vol.getEndTimeSecs())) {
      cerr << "WARNING - RadxMergeVols::_writeVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// check geometry between 2 volumes
// Returns 0 on success, -1 on failure

int RadxMergeVols::_checkGeom(const RadxVol &primaryVol,
                              const RadxVol &secondaryVol)
  
{

  // check that geometry matches

  double diff = primaryVol.getStartRangeKm() - secondaryVol.getStartRangeKm();
  if (fabs(diff) > 0.001) {
    if (_params.debug) {
      cerr << "ERROR - RadxMergeVols::_checkGeom" << endl;
      cerr << "  Volumes have different start range" << endl;
      cerr << "  start range primary: "
           << primaryVol.getStartRangeKm() << endl;
      cerr << "  start range secondary: "
           << secondaryVol.getStartRangeKm() << endl;
    }
    return -1;
  }

  diff = primaryVol.getGateSpacingKm() - secondaryVol.getGateSpacingKm();
  if (fabs(diff) > 0.001) {
    if (_params.debug) {
      cerr << "ERROR - RadxMergeVols::_checkGeom" << endl;
      cerr << "  Volumes have different gate spacing" << endl;
      cerr << "  gate spacing primary: "
           << primaryVol.getGateSpacingKm() << endl;
      cerr << "  gate spacing secondary: "
           << secondaryVol.getGateSpacingKm() << endl;
    }
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////////////////
// merge the primary and seconday volumes, using the primary
// volume to hold the merged data
//
// Returns 0 on success, -1 on failure

int RadxMergeVols::_mergeVol(RadxVol &primaryVol,
                             const RadxVol &secondaryVol)

{

  // check that geometry matches

  if (_params.check_constant_geometry) {
    if (_checkGeom(primaryVol, secondaryVol)) {
      cerr << "ERROR - RadxMergeVols::_mergeVols" << endl;
      cerr << "  Volume geometries differ" << endl;
      return -1;
    }
  }

  // add secondary rays to primary vol

  int maxSweepNum = 0;
  const vector<RadxRay *> &pRays = primaryVol.getRays();
  for (size_t iray = 0; iray < pRays.size(); iray++) {
    const RadxRay &pRay = *pRays[iray];
    if (pRay.getSweepNumber() > maxSweepNum) {
      maxSweepNum = pRay.getSweepNumber();
    }
  } // iray

  const vector<RadxRay *> &sRays = secondaryVol.getRays();
  for (size_t iray = 0; iray < sRays.size(); iray++) {
    RadxRay *copyRay = new RadxRay(*sRays[iray]);
    int sweepNum = copyRay->getSweepNumber() + maxSweepNum + 1;
    copyRay->setSweepNumber(sweepNum);
    primaryVol.addRay(copyRay);
  } // iray

  return 0;

}

