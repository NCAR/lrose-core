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
// RadxMergeFields.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2010
//
///////////////////////////////////////////////////////////////
//
// Merges fields from multiple CfRadial files into a single file
//
////////////////////////////////////////////////////////////////

#include "RadxMergeFields.hh"
#include <Radx/RadxVol.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxRay.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DataFileNames.hh>
#include <toolsa/pmu.h>
using namespace std;

// Constructor

RadxMergeFields::RadxMergeFields(int argc, char **argv)
  
{

  OK = TRUE;

  // set programe name

  _progName = "RadxMergeFields";
  
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

  // process the params

  if (_checkParams()) {
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

}

// destructor

RadxMergeFields::~RadxMergeFields()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RadxMergeFields::Run()
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
// Process the params, set up field lists

int RadxMergeFields::_checkParams()
{

  int iret = 0;

  // get min and max index for source dirs
  
  int minIndex = 999999;
  int maxIndex = -999999;

  for (int ii = 0; ii < _params.input_datasets_n; ii++) {
    if (_params._input_datasets[ii].index < minIndex) {
      minIndex = _params._input_datasets[ii].index;
    }
    if (_params._input_datasets[ii].index > maxIndex) {
      maxIndex = _params._input_datasets[ii].index;
    }
  }
  
  // set the directory lists
  // check that the indices are consecutive
  
  for (int index = minIndex; index <= maxIndex; index++) {
    
    OutputGroup group;

    bool found = false;
    for (int ii = 0; ii < _params.input_datasets_n; ii++) {
      if (_params._input_datasets[ii].index == index) {
        const Params::input_dataset_t &dataset = _params._input_datasets[ii];
        group.dir = dataset.dir;
        group.fileTimeOffset = dataset.file_match_time_offset_sec;
        group.fileTimeTolerance = dataset.file_match_time_tolerance_sec;
        group.rayElevTolerance = dataset.ray_match_elevation_tolerance_deg;
        group.rayAzTolerance = dataset.ray_match_azimuth_tolerance_deg;
        group.rayTimeTolerance = dataset.ray_match_time_tolerance_sec;
        found = true;
        break;
      }
    }

    if (!found) {
      cerr << "ERROR - RadxMergeFields::_checkParams()" << endl;
      cerr << "  Source index missing: " << index << endl;
      cerr << "  Indices must be consecutive" << endl;
      cerr << "  See input_datasets parameter" << endl;
      iret = -1;
    }
    
    for (int ii = 0; ii < _params.output_fields_n; ii++) {
      if (_params._output_fields[ii].input_index == index) {
        group.fields.push_back(_params._output_fields[ii]);
      }
    }

    if (index == minIndex) {
      _primaryGroup = group;
    } else {
      _secondaryGroups.push_back(group);
    }

  } // index

  if (_primaryGroup.fields.size() < 1) {
    cerr << "ERROR - RadxMergeFields::_checkParams()" << endl;
    cerr << "  No fields specified for primary (lowest) index" << endl;
    cerr << "  At least 1 field must be specified for primary index" << endl;
    iret = -1;
  }
  
  return iret;

}

//////////////////////////////////////////////////
// Run in filelist mode

int RadxMergeFields::_runFilelist()
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

int RadxMergeFields::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_primaryGroup.dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (_params.aggregate_sweep_files_on_read) {
    tlist.setReadAggregateSweeps(true);
  }
  if (tlist.compile()) {
    cerr << "ERROR - RadxMergeFields::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: "
         << _primaryGroup.dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - RadxMergeFields::_runFilelist()" << endl;
    cerr << "  No files found, dir: "
         << _primaryGroup.dir << endl;
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
// Run in realtime mode

int RadxMergeFields::_runRealtime()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  // watch for new data to arrive

  LdataInfo ldata(_primaryGroup.dir,
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

int RadxMergeFields::_processFile(const string &primaryPath)
{

  if (_params.debug) {
    cerr << "INFO - RadxMergeFields::_processFile" << endl;
    cerr << "  Input path primary file: " << primaryPath << endl;
  }
  
  // read in primary file
  
  GenericRadxFile primaryFile;
  _setupPrimaryRead(primaryFile);
  RadxVol primaryVol;
  if (primaryFile.readFromPath(primaryPath, primaryVol)) {
    cerr << "ERROR - RadxMergeFields::_processFile" << endl;
    cerr << "  Cannot read in primary file: " << primaryPath << endl;
    cerr << primaryFile.getErrStr() << endl;
    return -1;
  }

  time_t primaryTime = primaryVol.getEndTimeSecs();
  bool dateOnly;
  if (DataFileNames::getDataTime(primaryPath, primaryTime, dateOnly)) {
    cerr << "ERROR - RadxMergeFields::_processFile" << endl;
    cerr << "  Cannot get time from file path: " << primaryPath << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Time for primary file: " << RadxTime::strm(primaryTime) << endl;
  }

  // Search for secondary files

  for (size_t igroup = 0; igroup < _secondaryGroups.size(); igroup++) {
    
    _activeGroup = _secondaryGroups[igroup];
    time_t searchTime = primaryTime + _activeGroup.fileTimeOffset;

    RadxTimeList tlist;
    tlist.setDir(_activeGroup.dir);
    tlist.setModeClosest(searchTime, _activeGroup.fileTimeTolerance);

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      tlist.printRequest(cerr);
    }
    
    if (tlist.compile()) {
      cerr << "ERROR - RadxMergeFields::_processFile()" << endl;
      cerr << "  Cannot compile secondary file time list" << endl;
      cerr << tlist.getErrStr() << endl;
      return -1;
    }
    const vector<string> &pathList = tlist.getPathList();
    if (pathList.size() < 1) {
      cerr << "WARNING - RadxMergeFields::_processFile()" << endl;
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
    _setupSecondaryRead(secondaryFile, _activeGroup);
    RadxVol secondaryVol;
    if (secondaryFile.readFromPath(secondaryPath, secondaryVol)) {
      cerr << "ERROR - RadxMergeFields::_processFile" << endl;
      cerr << "  Cannot read in secondary file: " << secondaryPath << endl;
      cerr << secondaryFile.getErrStr() << endl;
      return -1;
    }

    // merge the primary and seconday volumes, using the primary
    // volume to hold the merged data
    
    if (_mergeVol(primaryVol, secondaryVol, _activeGroup)) {
      cerr << "ERROR - RadxMergeFields::_processFile" << endl;
      cerr << "  Merge failed" << endl;
      cerr << "  Primary file: " << primaryPath << endl;
      cerr << "  Secondary file: " << secondaryPath << endl;
      return -1;
    }

  } // igroup

  // add combined fields if required

  if (_params.add_combined_fields) {
    _addCombinedFields(primaryVol);
  }
  
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
// set up read for primary data

void RadxMergeFields::_setupPrimaryRead(RadxFile &file)
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

  for (size_t ii = 0; ii < _primaryGroup.fields.size(); ii++) {
    file.addReadField(_primaryGroup.fields[ii].input_field_name);
  }

  if (_params.aggregate_sweep_files_on_read) {
    file.setReadAggregateSweeps(true);
  } else {
    file.setReadAggregateSweeps(false);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===== SETTING UP READ FOR PRIMARY FILES =====" << endl;
    file.printReadRequest(cerr);
    cerr << "=============================================" << endl;
  }
  
}

//////////////////////////////////////////////////
// set up read for secondary data

void RadxMergeFields::_setupSecondaryRead(RadxFile &file,
                                          const OutputGroup &group)
{
  
  if (_params.debug) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setVerbose(true);
  }

  if (_params.set_fixed_angle_limits) {
    file.setReadFixedAngleLimits(_params.lower_fixed_angle_limit,
                                 _params.upper_fixed_angle_limit);
  } else if (_params.set_sweep_num_limits) {
    file.setReadSweepNumLimits(_params.lower_sweep_num,
                               _params.upper_sweep_num);
  }

  for (size_t ii = 0; ii < group.fields.size(); ii++) {
    file.addReadField(group.fields[ii].input_field_name);
  }

  if (_params.aggregate_sweep_files_on_read) {
    file.setReadAggregateSweeps(true);
  } else {
    file.setReadAggregateSweeps(false);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===== SETTING UP READ FOR SECONDARY FILES =====" << endl;
    file.printReadRequest(cerr);
    cerr << "===============================================" << endl;
  }
  
}

//////////////////////////////////////////////////
// set up write

void RadxMergeFields::_setupWrite(RadxFile &file)
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

  if (_params.output_compressed) {
    file.setWriteCompressed(true);
    file.setCompressionLevel(_params.compression_level);
  } else {
    file.setWriteCompressed(false);
  }

  if (_params.output_native_byte_order) {
    file.setWriteNativeByteOrder(true);
  } else {
    file.setWriteNativeByteOrder(false);
  }

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
    case Params::OUTPUT_FORMAT_NEXRAD:
      file.setFileFormat(RadxFile::FILE_FORMAT_NEXRAD_AR2);
      break;
    case Params::OUTPUT_FORMAT_MDV_RADIAL:
      file.setFileFormat(RadxFile::FILE_FORMAT_MDV_RADIAL);
      break;
    default:
    case Params::OUTPUT_FORMAT_CFRADIAL:
      file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  }

  // set netcdf format - used for CfRadial

  switch (_params.netcdf_style) {
    case Params::NETCDF4_CLASSIC:
      file.setNcFormat(RadxFile::NETCDF4_CLASSIC);
      break;
    case Params::NC64BIT:
      file.setNcFormat(RadxFile::NETCDF_OFFSET_64BIT);
      break;
    case Params::NETCDF4:
      file.setNcFormat(RadxFile::NETCDF4);
      break;
    default:
      file.setNcFormat(RadxFile::NETCDF_CLASSIC);
  }

  if (_params.output_force_ngates_vary) {
    file.setWriteForceNgatesVary(true);
  }

}

//////////////////////////////////////////////////
// write out the volume

int RadxMergeFields::_writeVol(RadxVol &vol)
{

  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  if (_params.output_filename_mode == Params::SPECIFY_FILE_NAME) {

    string outPath = _params.output_dir;
    outPath += PATH_DELIM;
    outPath += _params.output_filename;

    // write to path
  
    if (outFile.writeToPath(vol, outPath)) {
      cerr << "ERROR - RadxMergeFields::_writeVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
      
  } else {

    // write to dir
  
    if (outFile.writeToDir(vol, _params.output_dir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - RadxMergeFields::_writeVol" << endl;
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
      cerr << "WARNING - RadxMergeFields::_writeVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// check geometry between 2 volumes
// Returns 0 on success, -1 on failure

int RadxMergeFields::_checkGeom(const RadxVol &primaryVol,
                                const RadxVol &secondaryVol)
  
{

  // check that geometry matches

  double diff = primaryVol.getStartRangeKm() - secondaryVol.getStartRangeKm();
  if (fabs(diff) > 0.001) {
    if (_params.debug) {
      cerr << "ERROR - RadxMergeFields::_checkGeom" << endl;
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
      cerr << "ERROR - RadxMergeFields::_checkGeom" << endl;
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

int RadxMergeFields::_mergeVol(RadxVol &primaryVol,
                               const RadxVol &secondaryVol,
                               const OutputGroup &group)

{

  // check that geometry matches

  if (_params.check_constant_geometry) {
    if (_checkGeom(primaryVol, secondaryVol)) {
      cerr << "ERROR - RadxMergeFields::_mergeVols" << endl;
      cerr << "  Volume geometries differ" << endl;
      return -1;
    }
  }

  // loop through all rays in primary vol

  const vector<RadxRay *> &pRays = primaryVol.getRays();
  int searchStart = 0;
  vector<RadxRay *> mergedRays;
  vector<RadxRay *> unusedRays;

  for (size_t ii = 0; ii < pRays.size(); ii++) {

    RadxRay *pRay = pRays[ii];
    double pTime = (double) pRay->getTimeSecs() + pRay->getNanoSecs() / 1.0e9;
    double pAz = pRay->getAzimuthDeg();   
    double pEl = pRay->getElevationDeg();

    int pMilli = pRay->getNanoSecs() / 1.0e6;
    char pMStr[16];
    sprintf(pMStr, "%.3d", pMilli);

    // find matching ray in secondary volume

    const vector<RadxRay *> &sRays = secondaryVol.getRays();
    bool found = false;
    for (size_t jj = searchStart; jj < sRays.size(); jj++) {
      
      RadxRay *sRay = sRays[jj];
      double sTime = (double) sRay->getTimeSecs() + sRay->getNanoSecs() / 1.0e9;
      double sAz = sRay->getAzimuthDeg();   
      double sEl = sRay->getElevationDeg();
      
      int sMilli = sRay->getNanoSecs() / 1.0e6;
      char sMStr[16];
      sprintf(sMStr, "%.3d", sMilli);

      double diffTime = fabs(pTime - sTime);
      double dAz = pAz - sAz;
      if (dAz < -180) {
        dAz += 360.0;
      } else if (dAz > 180) {
        dAz -= 360.0;
      }
      double diffAz = fabs(dAz);
      double diffEl = fabs(pEl - sEl);

      if (diffTime <= _activeGroup.rayTimeTolerance &&
          diffAz <= _activeGroup.rayAzTolerance &&
          diffEl <= _activeGroup.rayElevTolerance) {
        // same ray, merge the rays
        _mergeRay(*pRay, *sRay, group);
        mergedRays.push_back(pRay);
        found = true;
        if (_params.debug >= Params::DEBUG_EXTRA) {
          cerr << "Matched ray - time az el az2 el2 dEl dAz dTime: "
               << RadxTime::strm((time_t) pTime) << "." << pMStr << ", "
               << pAz << ", " << pEl << ", "
               << sAz << ", " << sEl << ", "
               << diffEl << ", "
               << diffAz << ", "
               << diffTime << endl;
        }
        break;
      }
      
    } // jj

    if (!found) {
      unusedRays.push_back(pRay);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "====>>> missed merge, time az el: "
             << RadxTime::strm((time_t) pTime) << "." << pMStr << ", "
             << pAz << ", " << pEl << endl;
      }
    }

  } // ii

  // clean up unused rays

  primaryVol.removeBadRays(mergedRays, unusedRays);

  return 0;

}

//////////////////////////////////////////////////////////////
// merge primary and seconday rays
//
// Returns 0 on success, -1 on failure

void RadxMergeFields::_mergeRay(RadxRay &primaryRay,
                                const RadxRay &secondaryRay,
                                const OutputGroup &group)
  
{

  // rename fields on primary ray

  for (size_t ifld = 0; ifld < primaryRay.getNFields(); ifld++) {
    RadxField *pField = primaryRay.getField(ifld);
    for (size_t ii = 0; ii < _primaryGroup.fields.size(); ii++) {
      string inputName = _primaryGroup.fields[ii].input_field_name;
      if (inputName == pField->getName()) {
        pField->setName(_primaryGroup.fields[ii].output_field_name);
        break;
      }
    } // ii
  } // ifld
  primaryRay.loadFieldNameMap();

  // compute lookup in case geometry differs

  RadxRemap remap;
  bool geomDiffers =
    remap.checkGeometryIsDifferent(secondaryRay.getStartRangeKm(),
                                   secondaryRay.getGateSpacingKm(),
                                   primaryRay.getStartRangeKm(),
                                   primaryRay.getGateSpacingKm());
  if (geomDiffers) {
    remap.prepareForInterp(secondaryRay.getNGates(),
                           secondaryRay.getStartRangeKm(),
                           secondaryRay.getGateSpacingKm(),
                           primaryRay.getStartRangeKm(),
                           primaryRay.getGateSpacingKm());
  }
  
  const vector<RadxField *> &sFields = secondaryRay.getFields();
  int nGatesPrimary = primaryRay.getNGates();

  for (size_t ifield = 0; ifield < sFields.size(); ifield++) {
    
    const RadxField *sField = sFields[ifield];

    // get output field name

    string outputName = sField->getName();
    Params::output_encoding_t outputEncoding = Params::ENCODING_INT16;
    for (size_t ii = 0; ii < group.fields.size(); ii++) {
      string inputName = group.fields[ii].input_field_name;
      if (inputName == outputName) {
        outputName = group.fields[ii].output_field_name;
        outputEncoding = group.fields[ii].output_encoding;
        break;
      }
    }

    // make a copy of the field

    RadxField *sCopy = new RadxField(*sField);

    // rename to output name
    
    sCopy->setName(outputName);

    // ensure geometry is correct, remap if needed
    
    if (geomDiffers) {
      sCopy->remapRayGeom(remap, true);
    }
    sCopy->setNGates(nGatesPrimary);
      
    // convert type

    switch (outputEncoding) {
      case Params::ENCODING_FLOAT32:
        sCopy->convertToFl32();
        break;
      case Params::ENCODING_INT32:
        sCopy->convertToSi32();
        break;
      case Params::ENCODING_INT08:
        sCopy->convertToSi08();
        break;
      case Params::ENCODING_INT16:
      default:
        sCopy->convertToSi16();
        break;
    } // switch

    // add to ray

    primaryRay.addField(sCopy);

  } // ifield

}

//////////////////////////////////////////////////////////////
// Add combined fields
//
// Returns 0 on success, -1 on failure

int RadxMergeFields::_addCombinedFields(RadxVol &vol)
  
{

  int iret = 0;

  for (int ii = 0; ii < _params.combined_fields_n; ii++) {

    const Params::combined_field_t &comb = _params._combined_fields[ii];
    if (_addCombinedField(vol, comb)) {
      iret = -1;
    }

  } // ii

  return iret;

}

//////////////////////////////////////////////////////////////
// Add combined fields
//
// Returns 0 on success, -1 on failure

int RadxMergeFields::_addCombinedField(RadxVol &vol,
                                       const Params::combined_field_t &comb)
  
{

  int iret = 0;
  double sum = 0.0;
  double count = 0.0;

  // loop through rays

  const vector<RadxRay *> &rays = vol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {

    // get the fields to be combined

    RadxRay *ray = rays[iray];
    RadxField *fld1 = ray->getField(comb.field_name_1);
    RadxField *fld2 = ray->getField(comb.field_name_2);
    if (fld1 == NULL || fld2 == NULL) {
      iret = -1;
      continue;
    }

    // copy the fields so we can convert to floats
    
    RadxField *copy1 = new RadxField(*fld1);
    RadxField *copy2 = new RadxField(*fld2);
    copy1->convertToFl32();
    copy2->convertToFl32();
    Radx::fl32 miss1 = copy1->getMissingFl32();
    Radx::fl32 miss2 = copy2->getMissingFl32();

    // create new field to hold combined data
    // base it on field 1

    RadxField *combf = new RadxField(*copy1);
    combf->setName(comb.combined_name);
    combf->setLongName(comb.long_name);
    Radx::fl32 missComb = combf->getMissingFl32();
    
    // compute bias if needed

    double meanBias = 0.0;
    double sumBias = 0.0;
    double nBias = 0.0;
    if(comb.combine_method == Params::COMBINE_UNBIASED_MEAN) {
      Radx::fl32 *vals1 = (Radx::fl32 *) copy1->getData();
      Radx::fl32 *vals2 = (Radx::fl32 *) copy2->getData();
      for (size_t ipt = 0; ipt < copy1->getNPoints();
           ipt++, vals1++, vals2++) {
        if (*vals1 != miss1 && *vals2 != miss2) {
          double bias = *vals1 - *vals2;
          sumBias += bias;
          nBias++;
        }
      }
      if (nBias > 0) {
        meanBias = sumBias / nBias;
      }
    }

    // compute combined values
    
    Radx::fl32 *vals1 = (Radx::fl32 *) copy1->getData();
    Radx::fl32 *vals2 = (Radx::fl32 *) copy2->getData();
    Radx::fl32 *valsComb = (Radx::fl32 *) combf->getData();
    
    bool requireBoth = comb.require_both;
    for (size_t ipt = 0; ipt < copy1->getNPoints();
         ipt++, vals1++, vals2++, valsComb++) {

      // check if we need both fields present

      *valsComb = missComb;
      if (requireBoth) {
        if (*vals1 == miss1 || *vals2 == miss2) {
          continue;
        }
      }
        
      // field 1 missing?

      if (*vals1 == miss1) {
        // only use field 2
        if (comb.combine_method == Params::COMBINE_UNBIASED_MEAN) {
          // adjust for bias
          *valsComb = *vals2 + meanBias;
        } else {
          *valsComb = *vals2;
        }
        continue;
      }
      
      // field 2 missing?

      if (*vals2 == miss2) {
        // only use field1
        *valsComb = *vals1;
        continue;
      }

      // combine fields
      
      if (comb.combine_method == Params::COMBINE_MEAN) {
        
        *valsComb = (*vals1 + *vals2) / 2.0;
        
      } else if (comb.combine_method == Params::COMBINE_UNBIASED_MEAN) {

        *valsComb = (*vals1 + *vals2 + meanBias) / 2.0;
        
      } else if (comb.combine_method == Params::COMBINE_GEOM_MEAN) {

        *valsComb = sqrt(*vals1 * *vals2);

      } else if (comb.combine_method == Params::COMBINE_MAX) {

        if (*vals1 > *vals2) {
          *valsComb = *vals1;
        } else {
          *valsComb = *vals2;
        }

      } else if (comb.combine_method == Params::COMBINE_MIN) {

        if (*vals1 < *vals2) {
          *valsComb = *vals1;
        } else {
          *valsComb = *vals2;
        }

      } else if (comb.combine_method == Params::COMBINE_SUM) {

        *valsComb = *vals1 + *vals2;
        
      } else if (comb.combine_method == Params::COMBINE_DIFF) {

        *valsComb = *vals1 - *vals2;
        
      } // if (comb.combine_method == Params::COMBINE_MEAN)

      sum += *valsComb;
      count += 1.0;

    } // ipt
    
    // convert type

    switch (comb.output_encoding) {
      case Params::ENCODING_FLOAT32:
        combf->convertToFl32();
        break;
      case Params::ENCODING_INT32:
        combf->convertToSi32();
        break;
      case Params::ENCODING_INT08:
        combf->convertToSi08();
        break;
      case Params::ENCODING_INT16:
      default:
        combf->convertToSi16();
        break;
    } // switch

    // add combined field to ray

    combf->computeMinAndMax();
    ray->addField(combf);

    // free up

    delete copy1;
    delete copy2;

  } // iray

  // print mean combined value if requested
  
  if (_params.print_mean_of_combined_fields) {
    if (count > 0) {
      double meanComb = sum / count;
      cout << "==========================================================" << endl;
      cout << "===>> Field " << comb.combined_name << ", mean value: " << meanComb << endl;
      cout << "==========================================================" << endl;
      cout << flush;
    }
  }

  return iret;

}

