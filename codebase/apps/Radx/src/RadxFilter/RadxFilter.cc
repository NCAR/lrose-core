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
// RadxFilter.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
// June 2013
//
///////////////////////////////////////////////////////////////
//
// RadxFilter applies a filter to data in a Radx-supported file
// and writes out the filtered result.
//
///////////////////////////////////////////////////////////////

#include "RadxFilter.hh"
#include "ComputeEngine.hh"
#include "Thread.hh"
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/TaArray.hh>
#include <didss/LdataInfo.hh>
#include <rapmath/trig.h>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
using namespace std;

// Constructor

RadxFilter::RadxFilter(int argc, char **argv)
  
{

  OK = TRUE;
  _engine = NULL;

  // set programe name

  _progName = "RadxFilter";
  
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

  // initialize compute object

  pthread_mutex_init(&_debugPrintMutex, NULL);
  
  if (_params.use_multiple_threads) {
    
    // set up compute thread pool
    
    for (int ii = 0; ii < _params.n_compute_threads; ii++) {
      
      ComputeThread *thread = new ComputeThread();
      thread->setApp(this);

      ComputeEngine *engine = new ComputeEngine(_params);
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

    _engine = new ComputeEngine(_params);
    if (!_engine->OK) {
      OK = FALSE;
    }

  }

}

//////////////////////////////////////
// destructor

RadxFilter::~RadxFilter()

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

int RadxFilter::Run()
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

int RadxFilter::_runFilelist()
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

int RadxFilter::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (_params.aggregate_sweep_files_on_read) {
    tlist.setReadAggregateSweeps(true);
  }
  if (tlist.compile()) {
    cerr << "ERROR - RadxFilter::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - RadxFilter::_runFilelist()" << endl;
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

int RadxFilter::_runRealtime()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);

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

int RadxFilter::_processFile(const string &filePath)
{

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
    cerr << "INFO - RadxFilter::Run" << endl;
    cerr << "  Input path: " << filePath << endl;
  }
  
  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  RadxVol vol;
  if (inFile.readFromPath(filePath, vol)) {
    cerr << "ERROR - RadxFilter::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  _readPaths = inFile.getReadPaths();

  // override radar location if requested
  
  if (_params.override_radar_location) {
    vol.overrideLocation(_params.radar_latitude_deg,
                         _params.radar_longitude_deg,
                         _params.radar_altitude_meters / 1000.0);
  }

  // set number of gates constant if requested

  if (_params.set_ngates_constant) {
    vol.setNGatesConstant();
  }

  // trim surveillance sweeps to 360 degrees if requested

  if (_params.trim_surveillance_sweeps_to_360deg) {
    vol.trimSurveillanceSweepsTo360Deg();
  }

  // rename field if required
  
  if (_params.rename_fields) {
    for (int ii = 0; ii < _params.renamed_fields_n; ii++) {
      string inputName = _params._renamed_fields[ii].input_name;
      string outputName = _params._renamed_fields[ii].output_name;
      vol.renameField(inputName, outputName);
    } // ii
  } // if (_params.specify_field_names)
  

  // apply the filtering
  
  if (_compute(vol)) {
    cerr << "ERROR - RadxFilter::Run" << endl;
    cerr << "  Cannot compute moments" << endl;
    return -1;
  }

  // write out file

  GenericRadxFile outFile;
  _setupWrite(outFile);
  if (outFile.writeToDir(vol, _params.output_dir,
                         _params.append_day_dir_to_output_dir,
                         _params.append_year_dir_to_output_dir)) {
    cerr << "ERROR - RadxFilter::_processFile" << endl;
    cerr << "  Cannot write file to dir: " << _params.output_dir << endl;
    cerr << outFile.getErrStr() << endl;
    return -1;
  }
  string outputPath = outFile.getPathInUse();

  // in realtime mode, write latest data info file

  if (_params.mode == Params::REALTIME) {
    LdataInfo ldata(_params.output_dir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    RadxPath rpath(outputPath);
    ldata.setRelDataPath(rpath.getFile());
    ldata.setWriter(_progName);
    if (ldata.write(vol.getEndTimeSecs())) {
      cerr << "WARNING - RadxFilter::_processFile" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// set up read

void RadxFilter::_setupRead(RadxFile &file)
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

  if (_params.remove_rays_with_antenna_transitions &&
      !_params.trim_surveillance_sweeps_to_360deg) {
    file.setReadIgnoreTransitions(true);
    file.setReadTransitionNraysMargin(_params.transition_nrays_margin);
  }
  
  if (_params.select_fields) {
    for (int ii = 0; ii < _params.selected_fields_n; ii++) {
      if (_params._selected_fields[ii].process_this_field) {
        file.addReadField(_params._selected_fields[ii].input_name);
      }
    }
    if (_params.rename_fields) {
      for (int ii = 0; ii < _params.renamed_fields_n; ii++) {
        file.addReadField(_params._renamed_fields[ii].input_name);
      }
    }
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }
  
}

//////////////////////////////////////////////////
// set up write

void RadxFilter::_setupWrite(RadxFile &file)
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
// compute the moments for all rays in volume

int RadxFilter::_compute(RadxVol &vol)
{

  _outputRays.clear();

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

    _findTransitions(_outputRays);
    
    // load up the rays on the vol,
    // deleting those which are transitions
    
    for (size_t iray = 0; iray < _outputRays.size(); iray++) {
      if (_transitionFlags[iray]) {
        delete _outputRays[iray];
      } else {
        _outputRays[iray]->setAntennaTransition(false);
        vol.addRay(_outputRays[iray]);
      }
    }
    
  } else {

    // add all of the moments rays
    
    for (size_t iray = 0; iray < _outputRays.size(); iray++) {
      vol.addRay(_outputRays[iray]);
    }

  }
  
  vol.loadVolumeInfoFromRays();
  vol.loadSweepInfoFromRays();
  vol.setPackingFromRays();

  return 0;

}

//////////////////////////////////////////////////
// compute the moments for all rays in volume

int RadxFilter::_computeSingleThreaded(RadxVol &vol)
{

  // loop through the input rays,
  // computing the output fields

  const vector<RadxRay *> &inputRays = vol.getRays();
  for (size_t iray = 0; iray < inputRays.size(); iray++) {
    
    // get covariance ray
    
    RadxRay *inputRay = inputRays[iray];
    
    // compute moments
    
    RadxRay *outputRay = _engine->compute(inputRay);
    if (outputRay == NULL) {
      cerr << "ERROR - _compute" << endl;
      return -1;
    }
    
    // add to vector

    _outputRays.push_back(outputRay);
    
  } // iray

  return 0;

}

//////////////////////////////////////////////////
// compute the moments for all rays in volume

int RadxFilter::_computeMultiThreaded(RadxVol &vol)
{

  // loop through the input rays,
  // computing the output fields

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
      
      RadxRay *outputRay = thread->getOutputRay();
      if (outputRay == NULL) {
        cerr << "ERROR - _computeMultiThreaded" << endl;
        _availThreads.push_back(thread);
        return -1;
      } else {
        // good return, add to results
        _outputRays.push_back(outputRay);
      }
      
    }
    
    // get new covariance ray
    
    RadxRay *inputRay = inputRays[iray];
    
    // set thread going to compute moments
    
    thread->setInputRay(inputRay);
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
    
    RadxRay *outputRay = thread->getOutputRay();
    if (outputRay == NULL) {
      cerr << "ERROR - _computeMultiThreaded" << endl;
      return -1;
    } else {
      // good return, add to results
      _outputRays.push_back(outputRay);
    }

  }

  return 0;

}

///////////////////////////////////////////////////////////
// Thread function to compute moments

void *RadxFilter::_computeInThread(void *thread_data)
  
{
  
  // get thread data from args

  ComputeThread *compThread = (ComputeThread *) thread_data;
  RadxFilter *app = compThread->getApp();
  assert(app);

  while (true) {

    // wait for main to unlock start mutex on this thread
    
    compThread->waitForStartSignal();
    
    // if exit flag is set, app is done, exit now
    
    if (compThread->getExitFlag()) {
      if (app->getParams().debug >= Params::DEBUG_VERBOSE) {
        pthread_mutex_t *debugPrintMutex = app->getDebugPrintMutex();
        pthread_mutex_lock(debugPrintMutex);
        cerr << "====>> compute thread exiting" << endl;
        pthread_mutex_unlock(debugPrintMutex);
      }
      compThread->signalParentWorkIsComplete();
      return NULL;
    }
    
    // compute moments

    if (app->getParams().debug >= Params::DEBUG_VERBOSE) {
      pthread_mutex_t *debugPrintMutex = app->getDebugPrintMutex();
      pthread_mutex_lock(debugPrintMutex);
      cerr << "======>> starting compute" << endl;
      pthread_mutex_unlock(debugPrintMutex);
    }

    ComputeEngine *engine = compThread->getComputeEngine();
    RadxRay *inputRay = compThread->getInputRay();
    RadxRay *outputRay = engine->compute(inputRay);
    compThread->setOutputRay(outputRay);
    
    if (app->getParams().debug >= Params::DEBUG_VERBOSE) {
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

void RadxFilter::_findTransitions(vector<RadxRay *> &rays)

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
