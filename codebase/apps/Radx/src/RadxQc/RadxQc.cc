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
// RadxQc.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2016
//
///////////////////////////////////////////////////////////////
//
// RadxQc reads moments from Radx-supported format files, and
// performs Quality Control operations on the data. It
// optionally adds QC fields to the output, and optionally
// censors the input data based on the QC results.
//
///////////////////////////////////////////////////////////////

#include "RadxQc.hh"
#include "Thread.hh"
#include <algorithm>
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/TaArray.hh>
#include <dsserver/DsLdataInfo.hh>
#include <rapformats/WxObs.hh>
#include <Spdb/DsSpdb.hh>
#include <rapmath/trig.h>
#include <Radx/RadxVol.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxXml.hh>
using namespace std;

// Constructor

RadxQc::RadxQc(int argc, char **argv)
  
{

  OK = TRUE;
  _engine = NULL;

  // set programe name

  _progName = "RadxQc";
  
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

  if (_params.locate_rlan_interference) {
    if (!_params.NCP_available && !_params.WIDTH_available) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Problem with TDRP parameters." << endl;
      cerr << "  You have locate_rlan_interference set to TRUE" << endl;
      cerr << "  but neither WIDTH or NCP is avaiable." << endl;
      cerr << "  You need either NCP or WIDTH for RLAN location." << endl;
      OK = FALSE;
    }
  }

  // initialize compute object

  pthread_mutex_init(&_debugPrintMutex, NULL);
  
  if (_params.use_multiple_threads) {
    
    // set up compute thread pool
    
    for (int ii = 0; ii < _params.n_compute_threads; ii++) {
      
      ComputeThread *thread = new ComputeThread();
      thread->setApp(this);

      ComputeEngine *engine = new ComputeEngine(_params, ii);
      if (!engine->OK) {
        OK = FALSE;
      }
      thread->setComputeEngine(engine);

      pthread_t pth = 0;
      pthread_create(&pth, NULL, _computeInThread, thread);
      thread->setThreadId(pth);
      _availThreads.push_back(thread);

    }
    
  } else {

    // single threaded

    _engine = new ComputeEngine(_params, 0);
    if (!_engine->OK) {
      OK = FALSE;
    }

  }

  _printRunTime("Start ...");

}

//////////////////////////////////////
// destructor

RadxQc::~RadxQc()

{

  if (_engine) {
    delete _engine;
  }

  // wait for active thread pool to complete

  for (size_t ii = 0; ii < _activeThreads.size(); ii++) {
    _activeThreads[ii]->waitForWorkToComplete();
  }

  // signal all threads to exit

  for (size_t ii = 0; ii < _availThreads.size(); ii++) {
    _availThreads[ii]->setExitFlag(true);
    _availThreads[ii]->signalWorkToStart();
  }
  for (size_t ii = 0; ii < _activeThreads.size(); ii++) {
    _activeThreads[ii]->setExitFlag(true);
    _activeThreads[ii]->signalWorkToStart();
  }

  // wait for all threads to exit
  
  for (size_t ii = 0; ii < _availThreads.size(); ii++) {
    _availThreads[ii]->waitForWorkToComplete();
  }
  for (size_t ii = 0; ii < _activeThreads.size(); ii++) {
    _activeThreads[ii]->waitForWorkToComplete();
  }

  // delete all threads
  
  for (size_t ii = 0; ii < _availThreads.size(); ii++) {
    delete _availThreads[ii];
  }
  for (size_t ii = 0; ii < _activeThreads.size(); ii++) {
    delete _activeThreads[ii];
  }

  pthread_mutex_destroy(&_debugPrintMutex);

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RadxQc::Run()
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

int RadxQc::_runFilelist()
{

  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init(_progName.c_str(),
                  _params.instance,
                  _params.procmap_register_interval);
    PMU_auto_register("Init filelist mode");
  }

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

int RadxQc::_runArchive()
{

  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init(_progName.c_str(),
                  _params.instance,
                  _params.procmap_register_interval);
    PMU_auto_register("Init archive mode");
  }

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (_params.aggregate_sweep_files_on_read) {
    tlist.setReadAggregateSweeps(true);
  }
  if (tlist.compile()) {
    cerr << "ERROR - RadxQc::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - RadxQc::_runFilelist()" << endl;
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

int RadxQc::_runRealtime()
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

int RadxQc::_processFile(const string &filePath)
{

  PMU_auto_register("Processing file");
  _printRunTime("Start of _processFile()");
  
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
    if (rpath.getFile().find(_params.input_file_search_substr) == string::npos) {
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
    cerr << "INFO - RadxQc::Run" << endl;
    cerr << "  Input path: " << filePath << endl;
  }
  
  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  RadxVol vol;
  if (inFile.readFromPath(filePath, vol)) {
    cerr << "ERROR - RadxQc::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  _readPaths = inFile.getReadPaths();
  _printRunTime("After file read");

  // override radar location if requested

  if (_params.override_radar_location) {
    vol.overrideLocation(_params.radar_latitude_deg,
                         _params.radar_longitude_deg,
                         _params.radar_altitude_meters / 1000.0);
  }
  if (_params.locate_rlan_interference) {
    vol.estimateSweepNyquistFromVel(_params.VEL_field_name);
  }

  // set radar properties

  _wavelengthM = vol.getWavelengthM();
  _radarHtKm = vol.getAltitudeKm();

  // set number of gates constant if requested

  if (_params.set_ngates_constant) {
    vol.setNGatesConstant();
  }

  // trim surveillance sweeps to 360 degrees if requested

  if (_params.trim_surveillance_sweeps_to_360deg) {
    vol.trimSurveillanceSweepsTo360Deg();
  }

  // retrieve temp profile from SPDB as appropriate

  _retrieveTempProfile(vol);

  // option to get site temperature
  
  if (_params.read_site_temp_from_spdb) {
    if (_retrieveSiteTempFromSpdb(vol,
                                  _siteTempC,
                                  _timeForSiteTemp) == 0) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> site tempC: " 
             << _siteTempC << " at " << RadxTime::strm(_timeForSiteTemp) << endl;
      }
    }
  }

  // prepare for identification of sea clutter

  if (_params.locate_sea_clutter) {

    _printRunTime("Start of sea clutter prep");

    // remap to a constant geometry
    vol.remapToPredomGeom();
    
    // compute the height at each gate
    if (_addHeightField(vol)) {
      cerr << "ERROR - RadxQc::Run" << endl;
      cerr << "  Cannot add MSL height field" << endl;
      return -1;
    }
    
    // prepare for dbz gradient by computing pseudo RHIs
   // compute dbz gradient
    if (_computeDbzGradient(vol)) {
      cerr << "ERROR - RadxQc::Run" << endl;
      cerr << "  Cannot compute for dbz gradient" << endl;
      return -1;
    }

    _printRunTime("End   of sea clutter prep");

  } // if (_params.locate_sea_clutter)

  // compute the derived fields
  
  _printRunTime("Before compute()");
  if (_compute(vol)) {
    cerr << "ERROR - RadxQc::Run" << endl;
    cerr << "  Cannot compute moments" << endl;
    return -1;
  }
  _printRunTime("After compute()");

  // write results to output file

  _printRunTime("Before file write");
  if (_params.write_output_volume) {
    if (_writeVol(vol)) {
      return -1;
    }
  }
  _printRunTime("After file write");

  return 0;

}

//////////////////////////////////////////////////
// encode fields for output

void RadxQc::_encodeFieldsForOutput(RadxVol &vol)
{

  vector<RadxRay *> &rays = vol.getRays();

  // encode the fields for output
  
  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {
    
    Params::output_field_t &ofld = _params._output_fields[ifield];
    string fieldName = ofld.name;
    
    for (size_t iray = 0; iray < rays.size(); iray++) {

      RadxRay *ray = rays[iray];
      
      RadxField *field = ray->getField(fieldName);
      if (field == NULL) {
        continue;
      }
      
      if (ofld.encoding == Params::OUTPUT_ENCODING_FL32) {
        field->convertToFl32();
      } else if (ofld.encoding == Params::OUTPUT_ENCODING_INT32) {
        if (ofld.scaling == Params::OUTPUT_SCALING_DYNAMIC) {
          field->convertToSi32();
        } else {
          field->convertToSi32(ofld.scale, ofld.offset);
        }
      } else if (ofld.encoding == Params::OUTPUT_ENCODING_INT16) {
        if (ofld.scaling == Params::OUTPUT_SCALING_DYNAMIC) {
          field->convertToSi16();
        } else {
          field->convertToSi16(ofld.scale, ofld.offset);
        }
        field->convertToSi16();
      } else if (ofld.encoding == Params::OUTPUT_ENCODING_INT08) {
        if (ofld.scaling == Params::OUTPUT_SCALING_DYNAMIC) {
          field->convertToSi08();
        } else {
          field->convertToSi08(ofld.scale, ofld.offset);
        }
      } 

    } // iray

  } // ifield


}

//////////////////////////////////////////////////
// set up read

void RadxQc::_setupRead(RadxFile &file)
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

  // field names

  file.addReadField(_params.DBZ_field_name);
  file.addReadField(_params.VEL_field_name);

  if (_params.WIDTH_available) {
    file.addReadField(_params.WIDTH_field_name);
  }

  if (_params.NCP_available) {
    file.addReadField(_params.NCP_field_name);
  }

  if (_params.SNR_available) {
    file.addReadField(_params.SNR_field_name);
  }


  if (_params.ZDR_available) {
    file.addReadField(_params.ZDR_field_name);
  }

  if (_params.PHIDP_available) {
    file.addReadField(_params.PHIDP_field_name);
  }
  if (_params.RHOHV_available) {
    file.addReadField(_params.RHOHV_field_name);
  }

  if (_params.LDR_available) {
    file.addReadField(_params.LDR_field_name);
  }

  bool applyInputCensoring = false;
  if (_params.write_censored_fields_to_output) {
    for (int ii = 0; ii < _params.censored_output_fields_n; ii++) {
      file.addReadField(_params._censored_output_fields[ii].input_name);
      if (_params._censored_output_fields[ii].apply_input_field_censoring) {
        applyInputCensoring = true;
      }
    }
  }
  if (applyInputCensoring) {
    for (int ii = 0; ii < _params.censoring_input_fields_n; ii++) {
      file.addReadField(_params._censoring_input_fields[ii].input_name);
    }
  }

  if (_params.remove_rays_with_antenna_transitions &&
      !_params.trim_surveillance_sweeps_to_360deg) {
    file.setReadIgnoreTransitions(true);
    file.setReadTransitionNraysMargin(_params.transition_nrays_margin);
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }
  
}

//////////////////////////////////////////////////
// set up write

void RadxQc::_setupWrite(RadxFile &file)
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

  if (_params.write_individual_sweeps) {
    file.setWriteIndividualSweeps(true);
  }

  file.setWriteForceNgatesVary(true);

}

//////////////////////////////////////////////////
// write out the volume

int RadxQc::_writeVol(RadxVol &vol)
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
      cerr << "ERROR - RadxQc::_writeVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
      
  } else {

    // write to dir
  
    if (outFile.writeToDir(vol, _params.output_dir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - RadxQc::_writeVol" << endl;
      cerr << "  Cannot write file to dir: " << _params.output_dir << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }

  }

  string outputPath = outFile.getPathInUse();

  // write latest data info file
  
  DsLdataInfo ldata(_params.output_dir);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    ldata.setDebug(true);
  }
  string relPath;
  RadxPath::stripDir(_params.output_dir, outputPath, relPath);
  ldata.setRelDataPath(relPath);
  ldata.setWriter(_progName);
  if (ldata.write(vol.getEndTimeSecs())) {
    cerr << "WARNING - RadxQc::_writeVol" << endl;
    cerr << "  Cannot write latest data info file to dir: "
         << _params.output_dir << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// compute the moments for all rays in volume

int RadxQc::_compute(RadxVol &vol)
{

  _derivedRays.clear();

  if (_params.use_multiple_threads) {
    if (_computeMultiThreaded(vol)) {
      return -1;
    }
  } else {
    if (_computeSingleThreaded(vol)) {
      return -1;
    }
  }

  // clear the covariance rays

  vol.clearRays();

  // add the moments rays to the volume
  
  if (_params.remove_rays_with_antenna_transitions) {

    // find the transitions

    _findTransitions(_derivedRays);
    
    // load up the rays on the vol,
    // deleting those which are transitions
    
    for (size_t iray = 0; iray < _derivedRays.size(); iray++) {
      if (_transitionFlags[iray]) {
        delete _derivedRays[iray];
      } else {
        _derivedRays[iray]->setAntennaTransition(false);
        vol.addRay(_derivedRays[iray]);
      }
    }
    
  } else {

    // add all of the moments rays
    
    for (size_t iray = 0; iray < _derivedRays.size(); iray++) {
      vol.addRay(_derivedRays[iray]);
    }

  }
  
  vol.loadVolumeInfoFromRays();
  vol.loadSweepInfoFromRays();
  vol.setPackingFromRays();

  return 0;

}

//////////////////////////////////////////////////
// compute the moments for all rays in volume
// single-threaded operation

int RadxQc::_computeSingleThreaded(RadxVol &vol)
{

  // loop through the input rays,
  // computing the derived fields

  const vector<RadxRay *> &inputRays = vol.getRays();
  for (size_t iray = 0; iray < inputRays.size(); iray++) {
    
    // get covariance ray
    
    RadxRay *inputRay = inputRays[iray];
    
    // compute moments
    
    RadxRay *derivedRay = _engine->compute(inputRay, _radarHtKm, _wavelengthM, &_tempProfile);
    if (derivedRay == NULL) {
      cerr << "ERROR - _compute" << endl;
      return -1;
    }
    
    // add to vector
    
    _derivedRays.push_back(derivedRay);

  } // iray

  return 0;

}

//////////////////////////////////////////////////
// compute the moments for all rays in volume
// using multiple threads

int RadxQc::_computeMultiThreaded(RadxVol &vol)
{

  // loop through the input rays,
  // computing the derived fields

  const vector<RadxRay *> &inputRays = vol.getRays();
  for (size_t iray = 0; iray < inputRays.size(); iray++) {

    // is a thread available, if not wait for one
    
    ComputeThread *thread = NULL;
    if (_availThreads.size() > 0) {

      // get thread from available pool
      
      thread = _availThreads.front();
      _availThreads.pop_front();

    } else {

      // get thread from active pool

      thread = _activeThreads.front();
      _activeThreads.pop_front();

      // wait for moments computations to complete

      thread->waitForWorkToComplete();

      // store ray
      
      if (_storeDerivedRay(thread)) {
        cerr << "ERROR - _computeMultiThreaded" << endl;
        return -1;
      }

    }
    
    // get new covariance ray
    
    RadxRay *inputRay = inputRays[iray];
    
    // set thread going to compute moments
    
    thread->setCovRay(inputRay);
    thread->setWavelengthM(vol.getWavelengthM());
    thread->signalWorkToStart();

    // push onto active pool
    
    _activeThreads.push_back(thread);
    
  } // iray
  
  // wait for all active threads to complete
  
  while (_activeThreads.size() > 0) {
    
    ComputeThread *thread = _activeThreads.front();
    _activeThreads.pop_front();
    _availThreads.push_back(thread);

    // wait for moments computations to complete
    
    thread->waitForWorkToComplete();

    // store ray

    if (_storeDerivedRay(thread)) {
      cerr << "ERROR - _computeMultiThreaded" << endl;
      return -1;
    }

  }

  return 0;

}

///////////////////////////////////////////////////////////
// Store the derived ray

int RadxQc::_storeDerivedRay(ComputeThread *thread)

{
  
  RadxRay *derivedRay = thread->getMomRay();
  if (derivedRay == NULL) {
    _availThreads.push_back(thread);
    return -1;
  } else {
    // good return, add to results
    _derivedRays.push_back(derivedRay);
  }
  
  return 0;

}
      
///////////////////////////////////////////////////////////
// Thread function to compute moments

void *RadxQc::_computeInThread(void *thread_data)
  
{
  
  // get thread data from args

  ComputeThread *compThread = (ComputeThread *) thread_data;
  RadxQc *app = compThread->getApp();
  assert(app);

  while (true) {

    // wait for main to unlock start mutex on this thread
    
    compThread->waitForStartSignal();
    
    // if exit flag is set, app is done, exit now
    
    if (compThread->getExitFlag()) {
      if (app->getParams().debug >= Params::DEBUG_EXTRA) {
        pthread_mutex_t *debugPrintMutex = app->getDebugPrintMutex();
        pthread_mutex_lock(debugPrintMutex);
        cerr << "====>> compute thread exiting" << endl;
        pthread_mutex_unlock(debugPrintMutex);
      }
      compThread->signalParentWorkIsComplete();
      return NULL;
    }
    
    // compute moments

    if (app->getParams().debug >= Params::DEBUG_EXTRA) {
      pthread_mutex_t *debugPrintMutex = app->getDebugPrintMutex();
      pthread_mutex_lock(debugPrintMutex);
      cerr << "======>> starting compute" << endl;
      pthread_mutex_unlock(debugPrintMutex);
    }

    ComputeEngine *engine = compThread->getComputeEngine();
    RadxRay *inputRay = compThread->getCovRay();
    RadxRay *derivedRay = engine->compute(inputRay,
                                          app->_radarHtKm,
                                          app->_wavelengthM,
                                          &app->_tempProfile);
    compThread->setMomRay(derivedRay);
    
    if (app->getParams().debug >= Params::DEBUG_EXTRA) {
      pthread_mutex_t *debugPrintMutex = app->getDebugPrintMutex();
      pthread_mutex_lock(debugPrintMutex);
      cerr << "======>> done with compute" << endl;
      pthread_mutex_unlock(debugPrintMutex);
    }

    // unlock done mutex
    
    compThread->signalParentWorkIsComplete();
    
  } // while

  return NULL;

}

////////////////////////////////////////////////////////////
// Find the transitions in the rays

void RadxQc::_findTransitions(vector<RadxRay *> &rays)

{

  _transitionFlags.clear();

  for (size_t iray = 0; iray < rays.size(); iray++) {
    _transitionFlags.push_back(rays[iray]->getAntennaTransition());
  } // iray

  // widen good data by the specified margin

  int margin = _params.transition_nrays_margin;
  if (margin > 0) {
    
    // widen at start of transitions
    
    for (int ii = _transitionFlags.size() - 1; ii > 0; ii--) {
      if (_transitionFlags[ii] && !_transitionFlags[ii-1]) {
        for (int jj = ii; jj < ii + margin; jj++) {
          if (jj < (int) _transitionFlags.size()) {
            _transitionFlags[jj] = false;
          }
        }
      }
    } // ii
    
    // widen at end of transitions
    
    for (int ii = 1; ii < (int) _transitionFlags.size(); ii++) {
      if (_transitionFlags[ii-1] && !_transitionFlags[ii]) {
        for (int jj = ii - margin; jj < ii; jj++) {
          if (jj >= 0) {
            _transitionFlags[jj] = false;
          }
        }
      }
    } // ii
    
  }

}

//////////////////////////////////////
// initialize temperature profile
  
int RadxQc::_retrieveTempProfile(const RadxVol &vol)
  
{

  if (!_params.use_soundings_from_spdb) {
    _tempProfile.clear();
    return 0;
  }

  _tempProfile.setSoundingLocationName
    (_params.sounding_location_name);
  _tempProfile.setSoundingSearchTimeMarginSecs
    (_params.sounding_search_time_margin_secs);
  
  _tempProfile.setCheckPressureRange
    (_params.sounding_check_pressure_range);
  _tempProfile.setSoundingRequiredMinPressureHpa
    (_params.sounding_required_pressure_range_hpa.min_val);
  _tempProfile.setSoundingRequiredMaxPressureHpa
    (_params.sounding_required_pressure_range_hpa.max_val);
  
  _tempProfile.setCheckHeightRange
    (_params.sounding_check_height_range);
  _tempProfile.setSoundingRequiredMinHeightM
    (_params.sounding_required_height_range_m.min_val);
  _tempProfile.setSoundingRequiredMaxHeightM
    (_params.sounding_required_height_range_m.max_val);
  
  _tempProfile.setCheckPressureMonotonicallyDecreasing
    (_params.sounding_check_pressure_monotonically_decreasing);

  _tempProfile.setHeightCorrectionKm
    (_params.sounding_height_correction_km);

  if (_params.sounding_use_wet_bulb_temp) {
    _tempProfile.setUseWetBulbTemp(true);
  }
  
  time_t retrievedTime;
  vector<TempProfile::PointVal> retrievedProfile;
  if (_tempProfile.getTempProfile(_params.sounding_spdb_url,
                                  vol.getStartTimeSecs(),
                                  retrievedTime,
                                  retrievedProfile)) {
    cerr << "ERROR - RadxQc::_tempProfileInit" << endl;
    cerr << "  Cannot retrive profile for time: "
         << RadxTime::strm(vol.getStartTimeSecs()) << endl;
    cerr << "  url: " << _params.sounding_spdb_url << endl;
    cerr << "  station name: " << _params.sounding_location_name << endl;
    cerr << "  time margin secs: " << _params.sounding_search_time_margin_secs << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "=====================================" << endl;
    cerr << "Got temp profile, URL: " << _params.sounding_spdb_url << endl;
    cerr << "Overriding temperature profile" << endl;
    cerr << "  vol time: " << RadxTime::strm(vol.getStartTimeSecs()) << endl;
    cerr << "  retrievedTime: " << RadxTime::strm(retrievedTime) << endl;
    cerr << "  freezingLevel: " << _tempProfile.getFreezingLevel() << endl;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=====================================" << endl;
    cerr << "Temp  profile" << endl;
    int nLevels = (int) retrievedProfile.size();
    int nPrint = 50;
    int printInterval = nLevels / nPrint;
    if (nLevels < nPrint) {
      printInterval = 1;
    }
    for (size_t ii = 0; ii < retrievedProfile.size(); ii++) {
      bool doPrint = false;
      if (ii % printInterval == 0) {
        doPrint = true;
      }
      if (ii < retrievedProfile.size() - 1) {
        if (retrievedProfile[ii].tmpC * retrievedProfile[ii+1].tmpC <= 0) {
          doPrint = true;
        }
      }
      if (ii > 0) {
        if (retrievedProfile[ii-1].tmpC * retrievedProfile[ii].tmpC <= 0) {
          doPrint = true;
        }
      }
      if (doPrint) {
        cerr << "  ilevel, press(Hpa), alt(km), temp(C): " << ii << ", "
             << retrievedProfile[ii].pressHpa << ", "
             << retrievedProfile[ii].htKm << ", "
             << retrievedProfile[ii].tmpC << endl;
      }
    }
    cerr << "=====================================" << endl;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// retrieve site temp from SPDB for volume time

int RadxQc::_retrieveSiteTempFromSpdb(const RadxVol &vol,
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
    cerr << "WARNING - RadxQc::_retrieveSiteTempFromSpdb" << endl;
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
    cerr << "WARNING - RadxQc::_retrieveSiteTempFromSpdb" << endl;
    cerr << "  No suitable temp data from URL: " << _params.site_temp_spdb_url << endl;
    cerr << "  Search time: " << RadxTime::strm(searchTime) << endl;
    cerr << "  Search margin (secs): " << _params.site_temp_search_margin_secs << endl;
    return -1;
  }

  const Spdb::chunk_t &chunk = chunks[0];
  WxObs obs;
  if (obs.disassemble(chunk.data, chunk.len)) {
    cerr << "WARNING - RadxQc::_retrieveSiteTempFromSpdb" << endl;
    cerr << "  SPDB data is of incorrect type, prodLabel: " << spdb.getProdLabel() << endl;
    cerr << "  Should be station data type" << endl;
    cerr << "  URL: " << _params.site_temp_spdb_url << endl;
    return -1;
  }

  tempC = obs.getTempC();
  timeForTemp = obs.getObservationTime();

  return 0;

}

//////////////////////////////////////////////////////////////
// for sea clutter, add field with MSL height at each gate

int RadxQc::_addHeightField(RadxVol &vol)
  
{

  // initialize the beam height class

  if (_params.override_standard_pseudo_earth_radius) {
    _beamHt.setPseudoRadiusRatio(_params.pseudo_earth_radius_ratio);
  }
  _beamHt.setInstrumentHtKm(vol.getAltitudeKm());

  double minElev, maxElev, minRange, maxRange;
  vol.computeGeomLimitsFromRays(minElev, maxElev, minRange, maxRange);
  
  double elevRes = 0.01;
  minElev = ((int) (minElev / elevRes)) * elevRes;
  int nElev = (int) ((maxElev - minElev) / elevRes) + 1;

  double startRangeKm, gateSpacingKm;
  vol.getPredomRayGeom(startRangeKm, gateSpacingKm);
  double rangeRes = gateSpacingKm;
  minRange = startRangeKm;
  int nRange = (int) ((maxRange - minRange) / rangeRes) + 1;

  _beamHt.initHtCache(nElev, minElev, elevRes,
                      nRange, minRange, rangeRes);
  // #endif

  const vector<RadxRay *> rays = vol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {
    RadxRay &ray = *rays[iray];
    double elev = ray.getElevationDeg();
    double startRange = ray.getStartRangeKm();
    double gateSpacing = ray.getGateSpacingKm();
    double range = startRange;
    fl32 *htKm = new fl32[ray.getNGates()];
    for (size_t igate = 0; igate < ray.getNGates(); igate++, range += gateSpacing) {
      htKm[igate] = _beamHt.computeHtKm(elev, range);
    }
    RadxField *htField = new RadxField(_params.ray_height_field_name, "km");
    htField->setLongName("height_of_ray_msl");
    htField->setStandardName("height_above_reference_ellipsoid");
    htField->setMissingFl32(Radx::missingFl32);
    htField->addDataFl32(ray.getNGates(), htKm);
    ray.addField(htField);
    delete[] htKm;
  }

  return 0;

}

//////////////////////////////////////////////////////////////////
// for sea clutter, compute the reflectivity gradient
// for the whole volume

int RadxQc::_computeDbzGradient(RadxVol &vol)
  
{

  // reorder the sweeps in ascending order
  
  vol.reorderSweepsAscendingAngle();

  // need at leat 2 sweeps
  
  const vector<RadxSweep *> &sweeps = vol.getSweeps();
  if (sweeps.size() < 2) {
    return -1;
  }
  vector<RadxRay *> &rays = vol.getRays();
  
  // loop through the sweeps from lowest to highest
  // except for last one
  
  for (size_t isweep = 0; isweep < sweeps.size() - 1; isweep++) {
    
    const RadxSweep *lowerSweep = sweeps[isweep];
    const RadxSweep *upperSweep = sweeps[isweep + 1];

    size_t lowerSweepStartRayIndex = lowerSweep->getStartRayIndex();
    size_t lowerSweepEndRayIndex = lowerSweep->getEndRayIndex();

    size_t upperSweepStartRayIndex = upperSweep->getStartRayIndex();
    size_t upperSweepEndRayIndex = upperSweep->getEndRayIndex();

    // compute mean delta azimuth for low sweep
    // from this, compute azimuth margin for finding RHI rays
    
    double prevAz = rays[lowerSweepStartRayIndex]->getAzimuthDeg();
    double sumDeltaAz = 0.0;
    double count = 0.0;
    for (size_t iray = lowerSweepStartRayIndex + 1; iray <= lowerSweepEndRayIndex; iray++) {
      double az = rays[iray]->getAzimuthDeg();
      double deltaAz = fabs(az - prevAz);
      if (deltaAz > 180.0) {
        deltaAz = fabs(deltaAz - 360.0);
      }
      sumDeltaAz += deltaAz;
      count++;
      prevAz = az;
    } // iray
    double meanDeltaAz = sumDeltaAz / count;
    double azMargin = meanDeltaAz * 2.5;
    
    // loop through rays in lower sweep
    
    for (size_t lray = lowerSweepStartRayIndex; 
         lray <= lowerSweepEndRayIndex; lray++) {

      RadxRay *lowerRay = rays[lray];
      double lowerAz = lowerRay->getAzimuthDeg();

      // find the matching ray in the upper sweep
      // this is the ray with the closest azimuth

      double minDiff = 9999;
      RadxRay *upperMatch = NULL;
      
      for (size_t uray = upperSweepStartRayIndex; 
           uray <= upperSweepEndRayIndex; uray++) {
        
        RadxRay *upperRay = rays[uray];
        double upperAz = upperRay->getAzimuthDeg();
        
        double azDiff = fabs(lowerAz - upperAz);
        if (azDiff > 180.0) {
          azDiff = fabs(azDiff - 360.0);
        }
        if (azDiff < minDiff) {
          minDiff = azDiff;
          // check for valid azimuth difference
          if (minDiff <= azMargin) {
            upperMatch = upperRay;
          }
        }
        
      } // uray

      if (upperMatch != NULL) {
        if (_computeDbzGradient(*lowerRay, *upperMatch)) {
          // cannot find expected fields
          return -1;
        }
      }
      
      if (isweep == sweeps.size() - 2) {
        // for upper sweeps, copy the gradient field from
        // the next from top ray to the top ray
        _copyDbzGradient(*lowerRay, *upperMatch);
      }
  
    } // lray

  } // isweep
  
  return 0;

}

//////////////////////////////////////////////////////////////////
// compute the reflectivity gradient for a pair of rays in an RHI

int RadxQc::_computeDbzGradient(RadxRay &lowerRay, RadxRay &upperRay)
  
{

  // get the fields

  RadxField *dbzFieldLower = lowerRay.getField(_params.DBZ_field_name);
  if (dbzFieldLower == NULL) {
    cerr << "ERROR - RadxQc::_computeDbzGradient()" << endl;
    cerr << "  Cannot find DBZ field for lower ray: " << _params.DBZ_field_name << endl;
    cerr << "  Elevation: " << lowerRay.getElevationDeg() << endl;
    cerr << "  Azimuth: " << lowerRay.getAzimuthDeg() << endl;
    return -1;
  }

  RadxField *dbzFieldUpper = upperRay.getField(_params.DBZ_field_name);
  if (dbzFieldUpper == NULL) {
    cerr << "ERROR - RadxQc::_computeDbzGradient()" << endl;
    cerr << "  Cannot find DBZ field for upper ray: " << _params.DBZ_field_name << endl;
    cerr << "  Elevation: " << upperRay.getElevationDeg() << endl;
    cerr << "  Azimuth: " << upperRay.getAzimuthDeg() << endl;
    return -1;
  }

  RadxField *htFieldLower = lowerRay.getField(_params.ray_height_field_name);
  if (htFieldLower == NULL) {
    cerr << "ERROR - RadxQc::_computeDbzGradient()" << endl;
    cerr << "  Cannot find height field for lower ray: " << _params.ray_height_field_name << endl;
    cerr << "  Elevation: " << lowerRay.getElevationDeg() << endl;
    cerr << "  Azimuth: " << lowerRay.getAzimuthDeg() << endl;
    return -1;
  }

  RadxField *htFieldUpper = upperRay.getField(_params.ray_height_field_name);
  if (htFieldUpper == NULL) {
    cerr << "ERROR - RadxQc::_computeDbzGradient()" << endl;
    cerr << "  Cannot find height field for upper ray: " << _params.ray_height_field_name << endl;
    cerr << "  Elevation: " << upperRay.getElevationDeg() << endl;
    cerr << "  Azimuth: " << upperRay.getAzimuthDeg() << endl;
    return -1;
  }
  
  dbzFieldLower->convertToFl32();
  const Radx::fl32 *dbzLower = dbzFieldLower->getDataFl32();
  Radx::fl32 dbzLowerMissing = dbzFieldLower->getMissingFl32();

  dbzFieldUpper->convertToFl32();
  const Radx::fl32 *dbzUpper = dbzFieldUpper->getDataFl32();
  Radx::fl32 dbzUpperMissing = dbzFieldUpper->getMissingFl32();

  htFieldLower->convertToFl32();
  // const Radx::fl32 *htLower = htFieldLower->getDataFl32();

  htFieldUpper->convertToFl32();
  // const Radx::fl32 *htUpper = htFieldUpper->getDataFl32();

  // compute the elevation angle cosines, so that we can correct for range

  double cosLower = cos(lowerRay.getElevationDeg() * DEG_TO_RAD);
  double cosUpper = cos(upperRay.getElevationDeg() * DEG_TO_RAD);
  double deltaElev = upperRay.getElevationDeg() - lowerRay.getElevationDeg();

  // loop through the range gates in the lower ray

  double lowerStartRange = lowerRay.getStartRangeKm();
  double upperStartRange = upperRay.getStartRangeKm();

  double lowerGateSpacing = lowerRay.getGateSpacingKm();
  double upperGateSpacing = upperRay.getGateSpacingKm();

  double range = lowerStartRange;
  
  // create array for gradient

  fl32 *dbzGrad = new fl32[lowerRay.getNGates()];

  // loop through gates, computing the gradient
  
  for (size_t igate = 0; igate < lowerRay.getNGates(); igate++, range += lowerGateSpacing) {
    
    double gndRange = range * cosLower;
    double rangeUpper = gndRange / cosUpper; 

    int igateUpper = (int) ((rangeUpper - upperStartRange) / upperGateSpacing + 0.5);
    if (igateUpper < 0) {
      igateUpper = 0;
    }
    if (igateUpper > (int) upperRay.getNGates() - 1) {
      igateUpper = upperRay.getNGates() - 1;
    }
    
    double noiseDbz = _params.noise_dbz_at_100km + 20.0 * (log10(range) - 2.0);
    double dbzU = dbzUpper[igateUpper];
    if (dbzU == dbzUpperMissing) {
      dbzU = noiseDbz;
    }
    double dbzL = dbzLower[igate];
    if (dbzL == dbzLowerMissing) {
      dbzL = noiseDbz;
    }

    double dbzDiff = dbzU - dbzL;
    dbzGrad[igate] = dbzDiff / deltaElev;
    
  } // igate

  // create and add field
  
  RadxField *gradField = new RadxField(_params.dbz_elevation_gradient_field_name, "dB/deg");
  gradField->setLongName("gradient_of_dbz_with_elevation");
  gradField->setStandardName("gradient_of_dbz_with_elevation");
  gradField->setMissingFl32(Radx::missingFl32);
  gradField->addDataFl32(lowerRay.getNGates(), dbzGrad);
  lowerRay.addField(gradField);
  delete[] dbzGrad;

  return 0;

}

//////////////////////////////////////////////////////////////////
// copy the gradient field from the lower ray to upper ray

void RadxQc::_copyDbzGradient(const RadxRay &lowerRay, RadxRay &upperRay)
  
{

  const RadxField *gradFieldLower = lowerRay.getField(_params.dbz_elevation_gradient_field_name);
  if (gradFieldLower != NULL) {
    RadxField *gradFieldUpper = new RadxField(*gradFieldLower);
    gradFieldUpper->setNGates(upperRay.getNGates());
    upperRay.addField(gradFieldUpper);
  }

}

///////////////////////////////////////////////////////////////////
// Print the elapsed run time since the previous call, in seconds.

void RadxQc::_printRunTime(const string& str)
{
  if (!_params.debug) {
    return;
  }
  struct timeval tvb;
  gettimeofday(&tvb, NULL);
  double deltaSec = tvb.tv_sec - _timeA.tv_sec
    + 1.e-6 * (tvb.tv_usec - _timeA.tv_usec);
  cerr << "TIMING, task: " << str << ", secs used: " << deltaSec << endl;
  _timeA.tv_sec = tvb.tv_sec;
  _timeA.tv_usec = tvb.tv_usec;
}

