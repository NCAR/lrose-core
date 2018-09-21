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
//////////////////////////////////////////////////////////////////////////
// HcrVelCorrect.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2015
//
//////////////////////////////////////////////////////////////////////////
//
// HcrVelCorrect reads in HCR moments, computes the apparent velocity
// of the ground echo, filters the apparent velocity in time to remove
// spurious spikes, and then corrects the weather echo velocity using
// the filtered ground velocity as the correction to be applied.
//
//////////////////////////////////////////////////////////////////////////

#include "HcrVelCorrect.hh"
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxXml.hh>
#include <dsserver/DsLdataInfo.hh>
#include <Spdb/DsSpdb.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/pmu.h>
using namespace std;

// Constructor

HcrVelCorrect::HcrVelCorrect(int argc, char **argv)
  
{

  OK = TRUE;
  _firstInputFile = true;

  // set programe name

  _progName = "HcrVelCorrect";
  
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
  
  // set up the surface velocity object

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _surfVel.setDebug(true);
  } else if (_params.debug >= Params::DEBUG_EXTRA) {
    _surfVel.setVerbose(true);
  }

  _surfVel.setDbzFieldName(_params.dbz_field_name);
  _surfVel.setVelFieldName(_params.vel_field_name);

  _surfVel.setMinRangeToSurfaceKm(_params.min_range_to_surface_km);
  _surfVel.setMaxSurfaceHeightKm(_params.max_surface_height_km);
  _surfVel.setMinDbzForSurfaceEcho(_params.min_dbz_for_surface_echo);
  _surfVel.setNGatesForSurfaceEcho(_params.ngates_for_surface_echo);
  _surfVel.setMaxNadirErrorDeg(_params.max_nadir_error_for_surface_vel);

  // initialize the wave filtering

  _initWaveFilt();

  // initialize the FIR filtering

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _firFilt.setDebug(true);
  } else if (_params.debug >= Params::DEBUG_EXTRA) {
    _firFilt.setVerbose(true);
  }

  _firFilt.setSpikeFilterDifferenceThreshold
    (_params.spike_filter_difference_threshold);
  
  _firFilt.initFirFilters(_params.stage1_filter_n,
                          _params._stage1_filter,
                          _params.spike_filter_n,
                          _params._spike_filter,
                          _params.final_filter_n,
                          _params._final_filter);

  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

}

// destructor

HcrVelCorrect::~HcrVelCorrect()

{

  // close queues

  _inputFmq.closeMsgQueue();
  _outputFmq.closeMsgQueue();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int HcrVelCorrect::Run()
{

  switch (_params.mode) {
    case Params::FMQ:
      return _runFmq();
    case Params::ARCHIVE:
      return _runArchive();
    case Params::REALTIME:
      if (_params.latest_data_info_avail) {
        return _runRealtimeWithLdata();
      } else {
        return _runRealtimeNoLdata();
      }
    case Params::FILELIST:
    default:
      return _runFilelist();
  } // switch
}

//////////////////////////////////////////////////
// Run in filelist mode

int HcrVelCorrect::_runFilelist()
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

int HcrVelCorrect::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - HcrVelCorrect::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - HcrVelCorrect::_runFilelist()" << endl;
    cerr << "  No files found, dir: " << _params.input_dir << endl;
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

int HcrVelCorrect::_runRealtimeWithLdata()
{

  // watch for new data to arrive

  LdataInfo ldata(_params.input_dir,
                  _params.debug >= Params::DEBUG_VERBOSE);
  if (strlen(_params.search_ext) > 0) {
    ldata.setDataFileExt(_params.search_ext);
  }
  
  int iret = 0;
  int msecsWait = _params.wait_between_checks * 1000;
  while (true) {
    ldata.readBlocking(_params.max_realtime_data_age_secs,
                       msecsWait, PMU_auto_register);
    const string path = ldata.getDataPath();
    if (_processFile(path)) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode without latest data info

int HcrVelCorrect::_runRealtimeNoLdata()
{

  // Set up input path

  DsInputPath input(_progName,
		    _params.debug >= Params::DEBUG_VERBOSE,
		    _params.input_dir,
		    _params.max_realtime_data_age_secs,
		    PMU_auto_register,
		    _params.latest_data_info_avail,
		    false);

  input.setFileQuiescence(_params.file_quiescence);
  input.setSearchExt(_params.search_ext);
  input.setRecursion(_params.search_recursively);
  input.setMaxRecursionDepth(_params.max_recursion_depth);
  input.setMaxDirAge(_params.max_realtime_data_age_secs);

  int iret = 0;

  while(true) {

    // check for new data
    
    char *path = input.next(false);
    
    if (path == NULL) {
      
      // sleep a bit
      
      PMU_auto_register("Waiting for data");
      umsleep(_params.wait_between_checks * 1000);

    } else {

      // process the file

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

int HcrVelCorrect::_processFile(const string &readPath)
{

  PMU_auto_register("Processing file");

  // check we have not already processed this file
  // in the file aggregation step
  
  for (size_t ii = 0; ii < _readPaths.size(); ii++) {
    RadxPath rpath(_readPaths[ii]);
    if (readPath.find(rpath.getFile()) != string::npos) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Skipping file: " << readPath << endl;
        cerr << "  Previously processed in aggregation step" << endl;
      }
      return 0;
    }
  }
  
  if (_params.debug) {
    cerr << "INFO - HcrVelCorrect::Run" << endl;
    cerr << "  Input path: " << readPath << endl;
  }
  
  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file

  if (inFile.readFromPath(readPath, _inVol)) {
    cerr << "ERROR - HcrVelCorrect::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  _readPaths = inFile.getReadPaths();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t ii = 0; ii < _readPaths.size(); ii++) {
      cerr << "  ==>> read in file: " << _readPaths[ii] << endl;
    }
  }
  if (_firstInputFile) {
    _inEndTime.set(_inVol.getEndTimeSecs(), _inVol.getEndNanoSecs() / 1.0e9);
    _filtVol.copyMeta(_inVol);
    _firstInputFile = false;
  }
  
  // process each ray in the volume

  vector<RadxRay *> rays = _inVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {

    // create a copy of the ray
    // add a client to the copy to keep track of usage
    // the filter and the write volume will both try to delete
    // the ray if no longer used
    
    RadxRay *rayCopy = new RadxRay(*rays[iray]);
    rayCopy->addClient();

    // process the ray
    // computing vel and filtering
    
    if (_processRay(rayCopy) == 0) {

      RadxRay *filtRay = _firFilt.getFiltRay();
      RadxTime filtRayTime = filtRay->getRadxTime();
      
      // write vol when done
      
      if (filtRayTime > _inEndTime) {
        _writeFiltVol();
        _inEndTime.set(_inVol.getEndTimeSecs(),
                       _inVol.getEndNanoSecs() / 1.0e9);
      }
      
      // add to output vol
      
      _filtVol.addRay(filtRay);
      
      // write results to SPDB in XML if requested
      
      if (_params.write_surface_vel_results_to_spdb) {
        _writeResultsToSpdb(filtRay);
      }

    } // if (_processRay(rayCopy) == 0)

    RadxRay::deleteIfUnused(rayCopy);

  } // iray
  
  return 0;

}

//////////////////////////////////////////////////
// Process a ray

int HcrVelCorrect::_processRay(RadxRay *ray)
  
{

  // get surface vel
  
  double velSurf, dbzSurf, rangeToSurf;
  if (_surfVel.computeSurfaceVel(ray,
                                 velSurf,
                                 dbzSurf,
                                 rangeToSurf)) {
    velSurf = 0.0;
    rangeToSurf = 0.0;
    dbzSurf = -9999.0;
  }

  if (_params.filter_type == Params::WAVE_FILTER) {

    if (_applyWaveFilt(ray, velSurf, dbzSurf, rangeToSurf) == 0) {
      if (_velIsValid) {
        _correctVelForRay(_filtRay, _velFilt);
      } else {
        _copyVelForRay(_filtRay);
      }
      return 0;
    } else {
      return -1;
    }

  } else {

    if (_firFilt.filterRay(ray, velSurf, dbzSurf, rangeToSurf) == 0) {
      RadxRay *filtRay = _firFilt.getFiltRay();
      if (_firFilt.velocityIsValid()) {
        double velFilt = _firFilt.getVelFilt();
        _correctVelForRay(filtRay, velFilt);
      } else {
        _copyVelForRay(filtRay);
      }
      return 0;
    } else {
      return -1;
    }

  } // if (_params.filter_type == Params::WAVE_FILTER)
    
}
  
///////////////////////////////////////////////////////////////////
// Initialize the wave filter

void HcrVelCorrect::_initWaveFilt()

{

  _velIsValid = false;
  _velFilt = 0.0;
  _filtRay = NULL;

  // filter length
  
  _noiseFiltSecs = _params.noise_filter_length_secs;
  _waveFiltSecs = _params.wave_filter_length_secs;
  _filtSecs = max(_noiseFiltSecs, _waveFiltSecs);

}

///////////////////////////////////////////////////////////////////
// Apply the wave filter an incoming ray, filtering the surface vel.
// Returns 0 on success, -1 on failure.

int HcrVelCorrect::_applyWaveFilt(RadxRay *ray, 
                                  double velSurf,
                                  double dbzSurf,
                                  double rangeToSurf)

{

  // add node to queue

  FiltNode node;
  node.velSurf = velSurf;
  node.dbzSurf = dbzSurf;
  node.rangeToSurf = rangeToSurf;
  node.ray = ray;
  _filtNodes.push_front(node);

  // update the node stats
  // this will also write out any rays that are discarded
  // without having been written

  _updateNodeStats();

  return -1;

}

//////////////////////////////////////////////////
// update the node stats
// Side effects:
//   compute times
//   write out rays to be discarded that have not been written
//   determine if we have enough data for valid stats

void HcrVelCorrect::_updateNodeStats()
{

  // compute time span for filter
  
  FiltNode &youngest = _filtNodes[0];
  _filtStartTime = youngest.getTime();
  _filtEndTime = _filtStartTime + _filtSecs;

  // discard old nodes, writing out rays as appropriate

  while (_filtNodes.size() > 1) {
    size_t nNodes = _filtNodes.size();
    FiltNode &oldest = _filtNodes[nNodes - 1];
    if (oldest.getTime() > _filtEndTime) {
      if (!oldest.written) {
        // not yet written out, do so now
        _addRayToFiltVol(oldest.ray);
      }
      _filtNodes.pop_back();
    } else {
      break;
    }
  } // while

  size_t nNodes = _filtNodes.size();
  FiltNode &oldest = _filtNodes[nNodes - 1];
  _nodesEndTime = oldest.getTime();
  

}

//////////////////////////////////////////////////
// add a ray to the filtered volume
// write out the vol as needed

void HcrVelCorrect::_addRayToFiltVol(RadxRay *ray)
{
  
  if (ray->getRadxTime() > _inEndTime) {
    _writeFiltVol();
    _inEndTime.set(_inVol.getEndTimeSecs(),
                   _inVol.getEndNanoSecs() / 1.0e9);
  }
      
  // add to output vol
  
  _filtVol.addRay(ray);

}

//////////////////////////////////////////////////
// set up read

void HcrVelCorrect::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }

}

//////////////////////////////////////////////////
// convert all fields to specified output encoding

void HcrVelCorrect::_convertFieldsForOutput(RadxVol &vol)
{

  switch(_params.output_encoding) {
    case Params::OUTPUT_ENCODING_INT32:
      vol.convertToSi32();
      return;
    case Params::OUTPUT_ENCODING_INT16:
      vol.convertToSi16();
      return;
    case Params::OUTPUT_ENCODING_INT08:
      vol.convertToSi08();
      return;
    case Params::OUTPUT_ENCODING_FLOAT32:
    default:
      vol.convertToFl32();
      return;
  }

}

//////////////////////////////////////////////////
// write out the filtered volume

int HcrVelCorrect::_writeFiltVol()
{

  // set output encoding on all fields
  
  _convertFieldsForOutput(_filtVol);
  
  // set global attributes

  _setGlobalAttr(_filtVol);

  // set properties
  
  _filtVol.loadVolumeInfoFromRays();
  _filtVol.loadSweepInfoFromRays();

  // create output file
  
  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  if (_params.output_filename_mode == Params::SPECIFY_FILE_NAME) {
    
    string outPath = _params.output_dir;
    outPath += PATH_DELIM;
    outPath += _params.output_filename;

    // write to path
    
    if (outFile.writeToPath(_filtVol, outPath)) {
      cerr << "ERROR - HcrVelCorrect::_writeFiltVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
      
  } else {

    // write to dir
  
    if (outFile.writeToDir(_filtVol, _params.output_dir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - HcrVelCorrect::_writeFiltVol" << endl;
      cerr << "  Cannot write file to dir: " << _params.output_dir << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }

  }

  string outputPath = outFile.getPathInUse();
  
  // write latest data info file if requested 
  
  if (_params.write_latest_data_info) {
    DsLdataInfo ldata(_params.output_dir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    string relPath;
    RadxPath::stripDir(_params.output_dir, outputPath, relPath);
    ldata.setRelDataPath(relPath);
    ldata.setWriter(_progName);
    if (ldata.write(_filtVol.getEndTimeSecs())) {
      cerr << "WARNING - HcrVelCorrect::_writeFiltVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  // free up rays on vol

  _filtVol.clearRays();
  _filtVol.clearSweeps();
  _filtVol.clearRcalibs();
  _filtVol.clearCfactors();

  // copy the metadata from the current input volume

  _filtVol.copyMeta(_inVol);

  return 0;

}

//////////////////////////////////////////////////
// set up write

void HcrVelCorrect::_setupWrite(RadxFile &file)
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

  // set output format to CfRadial

  file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  file.setWriteCompressed(true);
  file.setCompressionLevel(_params.compression_level);

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

  if (strlen(_params.output_filename_prefix) > 0) {
    file.setWriteFileNamePrefix(_params.output_filename_prefix);
  }

  file.setWriteInstrNameInFileName(_params.include_instrument_name_in_file_name);

}

//////////////////////////////////////////////////
// set selected global attributes

void HcrVelCorrect::_setGlobalAttr(RadxVol &vol)
{

  vol.setDriver("HcrVelCorrect(NCAR)");
  
  string history(vol.getHistory());
  history += "\n";
  history += "Velocity filtering applied using HcrVelCorrect\n";
  vol.setHistory(history);
  
  RadxTime now(RadxTime::NOW);
  vol.setCreated(now.asString());

}

//////////////////////////////////////////////////
// Run in FMQ mode

int HcrVelCorrect::_runFmq()
{

  // Instantiate and initialize the input DsRadar queue and message
  
  if (_params.seek_to_end_of_input_fmq) {
    if (_inputFmq.init(_params.input_fmq_url, _progName.c_str(),
                       _params.debug >= Params::DEBUG_VERBOSE,
                       DsFmq::BLOCKING_READ_WRITE, DsFmq::END )) {
      fprintf(stderr, "ERROR - %s:Dsr2Radx::_run\n", _progName.c_str());
      fprintf(stderr, "Could not initialize radar queue '%s'\n",
	      _params.input_fmq_url);
      return -1;
    }
  } else {
    if (_inputFmq.init(_params.input_fmq_url, _progName.c_str(),
                       _params.debug >= Params::DEBUG_VERBOSE,
                       DsFmq::BLOCKING_READ_WRITE, DsFmq::START )) {
      fprintf(stderr, "ERROR - %s:Dsr2Radx::_run\n", _progName.c_str());
      fprintf(stderr, "Could not initialize radar queue '%s'\n",
	      _params.input_fmq_url);
      return -1;
    }
  }

  // create the output FMQ
  
  if (_outputFmq.init(_params.output_fmq_url,
                      _progName.c_str(),
                      _params.debug >= Params::DEBUG_VERBOSE,
                      DsFmq::READ_WRITE, DsFmq::END,
                      _params.output_fmq_compress,
                      _params.output_fmq_n_slots,
                      _params.output_fmq_buf_size)) {
    cerr << "WARNING - trying to create new fmq" << endl;
    if (_outputFmq.init(_params.output_fmq_url,
                        _progName.c_str(),
                        _params.debug >= Params::DEBUG_VERBOSE,
                        DsFmq::CREATE, DsFmq::START,
                        _params.output_fmq_compress,
                        _params.output_fmq_n_slots,
                        _params.output_fmq_buf_size)) {
      cerr << "ERROR - Radx2Dsr" << endl;
      cerr << "  Cannot open fmq, URL: " << _params.output_fmq_url << endl;
      return -1;
    }
  }
  
  if (_params.output_fmq_compress) {
    _outputFmq.setCompressionMethod(TA_COMPRESSION_GZIP);
  }
  
  if (_params.output_fmq_write_blocking) {
    _outputFmq.setBlockingWrite();
  }
  if (_params.output_fmq_data_mapper_report_interval > 0) {
    _outputFmq.setRegisterWithDmap
      (true, _params.output_fmq_data_mapper_report_interval);
  }

  // read messages from the queue and process them
  
  _nRaysRead = 0;
  _nRaysWritten = 0;
  _needWriteParams = false;
  RadxTime prevDwellRayTime;
  int iret = 0;
  while (true) {

    PMU_auto_register("Reading FMQ");

    // get a ray - this is created
    
    RadxRay *ray = _readFmqRay();
    if (ray == NULL) {
      umsleep(100);
      continue;
    }

    // process this ray

    if (_processRay(ray) == 0) {

      RadxRay *filtRay = _firFilt.getFiltRay();
      
      // write params if needed
      
      if (_needWriteParams) {
        if (_writeParams(filtRay)) {
          return -1; 
        }
        _needWriteParams = false;
      }
      
      // write out ray
      
      _writeRay(filtRay);

      // write results to SPDB in XML if requested

      if (_params.write_surface_vel_results_to_spdb) {
        _writeResultsToSpdb(filtRay);
      }

    } // if (processRay(ray) == 0)

    RadxRay::deleteIfUnused(ray);

  } // while (true)
  
  return iret;

}

////////////////////////////////////////////////////////////////////
// _readFmqRay()
//
// Read a ray from the FMQ, setting the flags about ray_data
// and _endOfVolume appropriately.
//
// Returns the ray on success, NULL on failure
// ray must be freed by caller.

RadxRay *HcrVelCorrect::_readFmqRay() 
  
{
  
  while (true) {

    PMU_auto_register("Reading radar queue");

    bool gotMsg = false;
    _inputContents = 0;
    if (_inputFmq.getDsMsg(_inputMsg, &_inputContents, &gotMsg)) {
      return NULL;
    }
    if (!gotMsg) {
      return NULL;
    }
    
    // set radar parameters if avaliable
    
    if (_inputContents & DsRadarMsg::RADAR_PARAMS) {
      _loadRadarParams();
      _needWriteParams = true;
    }
    
    // pass message through if not related to beam
    
    if (!(_inputContents & DsRadarMsg::RADAR_BEAM) &&
        !(_inputContents & DsRadarMsg::RADAR_PARAMS) &&
        !(_inputContents & DsRadarMsg::PLATFORM_GEOREF)) {
      if(_outputFmq.putDsMsg(_inputMsg, _inputContents)) {
        cerr << "ERROR - HcrVelCorrect::_runFmq()" << endl;
        cerr << "  Cannot copy message to output queue" << endl;
        cerr << "  URL: " << _params.output_fmq_url << endl;
        return NULL;
      }
    }

    // If we have radar and field params, and there is good ray data,
    
    if ((_inputContents & DsRadarMsg::RADAR_BEAM) && _inputMsg.allParamsSet()) {
        
      _nRaysRead++;
      
      // create ray from ray message
      
      RadxRay *ray = _createInputRay();
      ray->addClient();

      // debug print
      
      if (_params.debug) {
        if ((_nRaysRead > 0) && (_nRaysRead % 360 == 0)) {
          cerr << "==>>    read nRays, latest time, el, az: "
               << _nRaysRead << ", "
               << utimstr(ray->getTimeSecs()) << ", "
               << ray->getElevationDeg() << ", "
               << ray->getAzimuthDeg() << endl;
        }
      }
      
      // process that ray
      
      return ray;
      
    } // if (_inputContents ...

  } // while
  
  return NULL;

}

////////////////////////////////////////////////////////////////
// load radar params

void HcrVelCorrect::_loadRadarParams()

{
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "=========>> got RADAR_PARAMS" << endl;
  }

  _rparams = _inputMsg.getRadarParams();
  
  _filtVol.setInstrumentName(_rparams.radarName);
  _filtVol.setScanName(_rparams.scanTypeName);
  
  switch (_rparams.radarType) {
    case DS_RADAR_AIRBORNE_FORE_TYPE: {
      _filtVol.setPlatformType(Radx::PLATFORM_TYPE_FIXED);
      break;
    }
    case DS_RADAR_AIRBORNE_AFT_TYPE: {
      _filtVol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_FORE);
      break;
    }
    case DS_RADAR_AIRBORNE_TAIL_TYPE: {
      _filtVol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_TAIL);
      break;
    }
    case DS_RADAR_AIRBORNE_LOWER_TYPE: {
      _filtVol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_BELLY);
      break;
    }
    case DS_RADAR_AIRBORNE_UPPER_TYPE: {
      _filtVol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_ROOF);
      break;
    }
    case DS_RADAR_SHIPBORNE_TYPE: {
      _filtVol.setPlatformType(Radx::PLATFORM_TYPE_SHIP);
      break;
    }
    case DS_RADAR_VEHICLE_TYPE: {
      _filtVol.setPlatformType(Radx::PLATFORM_TYPE_VEHICLE);
      break;
    }
    case DS_RADAR_GROUND_TYPE:
    default:{
      _filtVol.setPlatformType(Radx::PLATFORM_TYPE_FIXED);
    }
  }
  
  _filtVol.setLocation(_rparams.latitude,
                        _rparams.longitude,
                        _rparams.altitude);

  _filtVol.addWavelengthCm(_rparams.wavelength);
  _filtVol.setRadarBeamWidthDegH(_rparams.horizBeamWidth);
  _filtVol.setRadarBeamWidthDegV(_rparams.vertBeamWidth);

}

////////////////////////////////////////////////////////////////////
// add an input ray from an incoming message

RadxRay *HcrVelCorrect::_createInputRay()

{

  // input data

  const DsRadarBeam &rbeam = _inputMsg.getRadarBeam();
  const DsRadarParams &rparams = _inputMsg.getRadarParams();
  const vector<DsFieldParams *> &fparamsVec = _inputMsg.getFieldParams();

  // create new ray

  RadxRay *ray = new RadxRay;

  // set ray properties

  ray->setTime(rbeam.dataTime, rbeam.nanoSecs);
  ray->setVolumeNumber(rbeam.volumeNum);
  ray->setSweepNumber(rbeam.tiltNum);

  int scanMode = rparams.scanMode;
  if (rbeam.scanMode > 0) {
    scanMode = rbeam.scanMode;
  } else {
    scanMode = rparams.scanMode;
  }

  ray->setSweepMode(_getRadxSweepMode(scanMode));
  ray->setPolarizationMode(_getRadxPolarizationMode(rparams.polarization));
  ray->setPrtMode(_getRadxPrtMode(rparams.prfMode));
  ray->setFollowMode(_getRadxFollowMode(rparams.followMode));

  double elev = rbeam.elevation;
  if (elev > 180) {
    elev -= 360.0;
  }
  ray->setElevationDeg(elev);

  double az = rbeam.azimuth;
  if (az < 0) {
    az += 360.0;
  }
  ray->setAzimuthDeg(az);

  // range geometry

  int nGates = rparams.numGates;
  ray->setRangeGeom(rparams.startRange, rparams.gateSpacing);

  if (scanMode == DS_RADAR_RHI_MODE ||
      scanMode == DS_RADAR_EL_SURV_MODE) {
    ray->setFixedAngleDeg(rbeam.targetAz);
  } else {
    ray->setFixedAngleDeg(rbeam.targetElev);
  }

  ray->setIsIndexed(rbeam.beamIsIndexed);
  ray->setAngleResDeg(rbeam.angularResolution);
  ray->setAntennaTransition(rbeam.antennaTransition);
  ray->setNSamples(rparams.samplesPerBeam);
  
  ray->setPulseWidthUsec(rparams.pulseWidth);
  double prt = 1.0 / rparams.pulseRepFreq;
  ray->setPrtSec(prt);
  ray->setPrtRatio(1.0);
  ray->setNyquistMps(rparams.unambigVelocity);

  ray->setUnambigRangeKm(Radx::missingMetaDouble);
  ray->setUnambigRange();

  ray->setMeasXmitPowerDbmH(rbeam.measXmitPowerDbmH);
  ray->setMeasXmitPowerDbmV(rbeam.measXmitPowerDbmV);

  // platform georeference
  
  if (_inputContents & DsRadarMsg::PLATFORM_GEOREF) {
    const DsPlatformGeoref &platformGeoref = _inputMsg.getPlatformGeoref();
    const ds_iwrf_platform_georef_t &dsGeoref = platformGeoref.getGeoref();
    _georefs.push_back(platformGeoref);
    RadxGeoref georef;
    georef.setTimeSecs(dsGeoref.packet.time_secs_utc);
    georef.setNanoSecs(dsGeoref.packet.time_nano_secs);
    georef.setLongitude(dsGeoref.longitude);
    georef.setLatitude(dsGeoref.latitude);
    georef.setAltitudeKmMsl(dsGeoref.altitude_msl_km);
    georef.setAltitudeKmAgl(dsGeoref.altitude_agl_km);
    georef.setEwVelocity(dsGeoref.ew_velocity_mps);
    georef.setNsVelocity(dsGeoref.ns_velocity_mps);
    georef.setVertVelocity(dsGeoref.vert_velocity_mps);
    georef.setHeading(dsGeoref.heading_deg);
    georef.setRoll(dsGeoref.roll_deg);
    georef.setPitch(dsGeoref.pitch_deg);
    georef.setDrift(dsGeoref.drift_angle_deg);
    georef.setRotation(dsGeoref.rotation_angle_deg);
    georef.setTilt(dsGeoref.tilt_angle_deg);
    georef.setEwWind(dsGeoref.ew_horiz_wind_mps);
    georef.setNsWind(dsGeoref.ns_horiz_wind_mps);
    georef.setVertWind(dsGeoref.vert_wind_mps);
    georef.setHeadingRate(dsGeoref.heading_rate_dps);
    georef.setPitchRate(dsGeoref.pitch_rate_dps);
    georef.setDriveAngle1(dsGeoref.drive_angle_1_deg);
    georef.setDriveAngle2(dsGeoref.drive_angle_2_deg);
    ray->clearGeoref();
    ray->setGeoref(georef);
  }

  // load up fields

  int byteWidth = rbeam.byteWidth;
  
  for (size_t iparam = 0; iparam < fparamsVec.size(); iparam++) {

    // is this an output field or censoring field?

    const DsFieldParams &fparams = *fparamsVec[iparam];
    string fieldName = fparams.name;
    // if (_params.set_output_fields && !_isOutputField(fieldName)) {
    //   continue;
    // }

    // convert to floats
    
    Radx::fl32 *fdata = new Radx::fl32[nGates];

    if (byteWidth == sizeof(fl32)) {

      fl32 *inData = (fl32 *) rbeam.data() + iparam;
      fl32 inMissing = (fl32) fparams.missingDataValue;
      Radx::fl32 *outData = fdata;
      
      for (int igate = 0; igate < nGates;
           igate++, inData += fparamsVec.size(), outData++) {
        fl32 inVal = *inData;
        if (inVal == inMissing) {
          *outData = Radx::missingFl32;
        } else {
          *outData = *inData;
        }
      } // igate

    } else if (byteWidth == sizeof(ui16)) {

      ui16 *inData = (ui16 *) rbeam.data() + iparam;
      ui16 inMissing = (ui16) fparams.missingDataValue;
      double scale = fparams.scale;
      double bias = fparams.bias;
      Radx::fl32 *outData = fdata;
      
      for (int igate = 0; igate < nGates;
           igate++, inData += fparamsVec.size(), outData++) {
        ui16 inVal = *inData;
        if (inVal == inMissing) {
          *outData = Radx::missingFl32;
        } else {
          *outData = *inData * scale + bias;
        }
      } // igate

    } else {

      // byte width 1

      ui08 *inData = (ui08 *) rbeam.data() + iparam;
      ui08 inMissing = (ui08) fparams.missingDataValue;
      double scale = fparams.scale;
      double bias = fparams.bias;
      Radx::fl32 *outData = fdata;
      
      for (int igate = 0; igate < nGates;
           igate++, inData += fparamsVec.size(), outData++) {
        ui08 inVal = *inData;
        if (inVal == inMissing) {
          *outData = Radx::missingFl32;
        } else {
          *outData = *inData * scale + bias;
        }
      } // igate

    } // if (byteWidth == 4)

    RadxField *field = new RadxField(fparams.name, fparams.units);
    field->copyRangeGeom(*ray);
    field->setTypeFl32(Radx::missingFl32);
    field->addDataFl32(nGates, fdata);

    ray->addField(field);

    delete[] fdata;

  } // iparam

  return ray;

}

//////////////////////////////////////////////////
// write radar and field parameters

int HcrVelCorrect::_writeParams(const RadxRay *ray)

{

  // radar parameters
  
  DsRadarMsg msg;
  DsRadarParams &rparams = msg.getRadarParams();
  rparams = _rparams;
  rparams.numFields = ray->getNFields();
  
  // field parameters - all fields are fl32
  
  vector<DsFieldParams* > &fieldParams = msg.getFieldParams();
  
  for (int ifield = 0; ifield < (int) ray->getNFields(); ifield++) {
    
    const RadxField &fld = *ray->getFields()[ifield];
    double dsScale = 1.0, dsBias = 0.0;
    int dsMissing = (int) floor(fld.getMissingFl32() + 0.5);
    
    DsFieldParams *fParams = new DsFieldParams(fld.getName().c_str(),
                                               fld.getUnits().c_str(),
                                               dsScale, dsBias, sizeof(fl32),
                                               dsMissing);
    fieldParams.push_back(fParams);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fParams->print(cerr);
    }

  } // ifield

  // put params
  
  int content = DsRadarMsg::FIELD_PARAMS | DsRadarMsg::RADAR_PARAMS;
  if(_outputFmq.putDsMsg(msg, content)) {
    cerr << "ERROR - HcrVelCorrect::_writeParams()" << endl;
    cerr << "  Cannot write field params to FMQ" << endl;
    cerr << "  URL: " << _params.output_fmq_url << endl;
    return -1;
  }
        
  return 0;

}

//////////////////////////////////////////////////
// write ray

int HcrVelCorrect::_writeRay(const RadxRay *ray)
  
{

  // write params if needed

  int nGates = ray->getNGates();
  const vector<RadxField *> &fields = ray->getFields();
  int nFields = ray->getNFields();
  int nPoints = nGates * nFields;
  
  // meta-data
  
  DsRadarMsg msg;
  DsRadarBeam &beam = msg.getRadarBeam();

  beam.dataTime = ray->getTimeSecs();
  beam.nanoSecs = (int) (ray->getNanoSecs() + 0.5);
  beam.referenceTime = 0;

  beam.byteWidth = sizeof(fl32); // fl32

  beam.volumeNum = ray->getVolumeNumber();
  beam.tiltNum = ray->getSweepNumber();
  Radx::SweepMode_t sweepMode = ray->getSweepMode();
  beam.scanMode = _getDsScanMode(sweepMode);
  beam.antennaTransition = ray->getAntennaTransition();
  
  beam.azimuth = ray->getAzimuthDeg();
  beam.elevation = ray->getElevationDeg();
  
  if (sweepMode == Radx::SWEEP_MODE_RHI ||
      sweepMode == Radx::SWEEP_MODE_MANUAL_RHI) {
    beam.targetAz = ray->getFixedAngleDeg();
  } else {
    beam.targetElev = ray->getFixedAngleDeg();
  }

  beam.beamIsIndexed = ray->getIsIndexed();
  beam.angularResolution = ray->getAngleResDeg();
  beam.nSamples = ray->getNSamples();

  beam.measXmitPowerDbmH = ray->getMeasXmitPowerDbmH();
  beam.measXmitPowerDbmV = ray->getMeasXmitPowerDbmV();

  // 4-byte floats
  
  fl32 *data = new fl32[nPoints];
  for (int ifield = 0; ifield < nFields; ifield++) {
    fl32 *dd = data + ifield;
    const RadxField *fld = fields[ifield];
    const Radx::fl32 *fd = (Radx::fl32 *) fld->getData();
    if (fd == NULL) {
      cerr << "ERROR - Radx2Dsr::_writeBeam" << endl;
      cerr << "  NULL data pointer, field name, elev, az: "
           << fld->getName() << ", "
           << ray->getElevationDeg() << ", "
           << ray->getAzimuthDeg() << endl;
      delete[] data;
      return -1;
    }
    for (int igate = 0; igate < nGates; igate++, dd += nFields, fd++) {
      *dd = *fd;
    }
  }
  beam.loadData(data, nPoints * sizeof(fl32), sizeof(fl32));
  delete[] data;
    
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    beam.print(cerr);
  }
  
  // add georeference if applicable

  int contents = (int) DsRadarMsg::RADAR_BEAM;
  if (_georefs.size() > 0) {
    DsPlatformGeoref &georef = msg.getPlatformGeoref();
    // use mid georef
    georef = _georefs[_georefs.size() / 2];
    contents |= DsRadarMsg::PLATFORM_GEOREF;
  }
    
  // put beam
  
  if(_outputFmq.putDsMsg(msg, contents)) {
    cerr << "ERROR - HcrVelCorrect::_writeBeam()" << endl;
    cerr << "  Cannot write beam to FMQ" << endl;
    cerr << "  URL: " << _params.output_fmq_url << endl;
    return -1;
  }

  // debug print
  
  _nRaysWritten++;
  if (_params.debug) {
    if (_nRaysWritten % 100 == 0) {
      cerr << "====>> wrote nRays, latest time, el, az: "
           << _nRaysWritten << ", "
           << utimstr(ray->getTimeSecs()) << ", "
           << ray->getElevationDeg() << ", "
           << ray->getAzimuthDeg() << endl;
    }
  }
  
  return 0;

}

/////////////////////////////////////////////
// convert from DsrRadar enums to Radx enums

Radx::SweepMode_t HcrVelCorrect::_getRadxSweepMode(int dsrScanMode)

{

  switch (dsrScanMode) {
    case DS_RADAR_SECTOR_MODE:
    case DS_RADAR_FOLLOW_VEHICLE_MODE:
      return Radx::SWEEP_MODE_SECTOR;
    case DS_RADAR_COPLANE_MODE:
      return Radx::SWEEP_MODE_COPLANE;
    case DS_RADAR_RHI_MODE:
      return Radx::SWEEP_MODE_RHI;
    case DS_RADAR_VERTICAL_POINTING_MODE:
      return Radx::SWEEP_MODE_VERTICAL_POINTING;
    case DS_RADAR_MANUAL_MODE:
      return Radx::SWEEP_MODE_POINTING;
    case DS_RADAR_SURVEILLANCE_MODE:
      return Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
    case DS_RADAR_EL_SURV_MODE:
      return Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE;
    case DS_RADAR_SUNSCAN_MODE:
      return Radx::SWEEP_MODE_SUNSCAN;
    case DS_RADAR_SUNSCAN_RHI_MODE:
      return Radx::SWEEP_MODE_SUNSCAN_RHI;
    case DS_RADAR_POINTING_MODE:
      return Radx::SWEEP_MODE_POINTING;
    default:
      return Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
  }
}

Radx::PolarizationMode_t HcrVelCorrect::_getRadxPolarizationMode(int dsrPolMode)

{
  switch (dsrPolMode) {
    case DS_POLARIZATION_HORIZ_TYPE:
      return Radx::POL_MODE_HORIZONTAL;
    case DS_POLARIZATION_VERT_TYPE:
      return Radx::POL_MODE_VERTICAL;
    case DS_POLARIZATION_DUAL_TYPE:
      return Radx::POL_MODE_HV_SIM;
    case DS_POLARIZATION_DUAL_HV_ALT:
      return Radx::POL_MODE_HV_ALT;
    case DS_POLARIZATION_DUAL_HV_SIM:
      return Radx::POL_MODE_HV_SIM;
    case DS_POLARIZATION_RIGHT_CIRC_TYPE:
    case DS_POLARIZATION_LEFT_CIRC_TYPE:
      return Radx::POL_MODE_CIRCULAR;
    case DS_POLARIZATION_ELLIPTICAL_TYPE:
    default:
      return Radx::POL_MODE_HORIZONTAL;
  }
}

Radx::FollowMode_t HcrVelCorrect::_getRadxFollowMode(int dsrMode)

{
  switch (dsrMode) {
    case DS_RADAR_FOLLOW_MODE_SUN:
      return Radx::FOLLOW_MODE_SUN;
    case DS_RADAR_FOLLOW_MODE_VEHICLE:
      return Radx::FOLLOW_MODE_VEHICLE;
    case DS_RADAR_FOLLOW_MODE_AIRCRAFT:
      return Radx::FOLLOW_MODE_AIRCRAFT;
    case DS_RADAR_FOLLOW_MODE_TARGET:
      return Radx::FOLLOW_MODE_TARGET;
    case DS_RADAR_FOLLOW_MODE_MANUAL:
      return Radx::FOLLOW_MODE_MANUAL;
    default:
      return Radx::FOLLOW_MODE_NONE;
  }
}

Radx::PrtMode_t HcrVelCorrect::_getRadxPrtMode(int dsrMode)

{
  switch (dsrMode) {
    case DS_RADAR_PRF_MODE_FIXED:
      return Radx::PRT_MODE_FIXED;
    case DS_RADAR_PRF_MODE_STAGGERED_2_3:
    case DS_RADAR_PRF_MODE_STAGGERED_3_4:
    case DS_RADAR_PRF_MODE_STAGGERED_4_5:
      return Radx::PRT_MODE_STAGGERED;
    default:
      return Radx::PRT_MODE_FIXED;
  }
}

//////////////////////////////////////////////////
// get Dsr enums from Radx enums

int HcrVelCorrect::_getDsScanMode(Radx::SweepMode_t mode)

{
  switch (mode) {
    case Radx::SWEEP_MODE_SECTOR:
      return DS_RADAR_SECTOR_MODE;
    case Radx::SWEEP_MODE_COPLANE:
      return DS_RADAR_COPLANE_MODE;
    case Radx::SWEEP_MODE_RHI:
      return DS_RADAR_RHI_MODE;
    case Radx::SWEEP_MODE_VERTICAL_POINTING:
      return DS_RADAR_VERTICAL_POINTING_MODE;
    case Radx::SWEEP_MODE_IDLE:
      return DS_RADAR_IDLE_MODE;
    case Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE:
      return DS_RADAR_SURVEILLANCE_MODE;
    case Radx::SWEEP_MODE_SUNSCAN:
      return DS_RADAR_SUNSCAN_MODE;
    case Radx::SWEEP_MODE_POINTING:
      return DS_RADAR_POINTING_MODE;
    case Radx::SWEEP_MODE_MANUAL_PPI:
      return DS_RADAR_MANUAL_MODE;
    case Radx::SWEEP_MODE_MANUAL_RHI:
      return DS_RADAR_MANUAL_MODE;
    case Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE:
    default:
      return DS_RADAR_SURVEILLANCE_MODE;
  }
}

//////////////////////////////////////////////////
// correct velocity on ray

void HcrVelCorrect::_correctVelForRay(RadxRay *ray, double surfFilt)

{

  RadxField *velField = ray->getField(_params.vel_field_name);
  if (velField == NULL) {
    // no vel field, nothing to do
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - no vel field found: " << _params.vel_field_name << endl;
    }
    return;
  }

  // create the corrected field
  
  RadxField *correctedField = new RadxField(_params.corrected_vel_field_name,
                                            velField->getUnits());
  correctedField->copyMetaData(*velField);
  correctedField->setName(_params.corrected_vel_field_name);

  // correct the values
  
  const Radx::fl32 *vel = velField->getDataFl32();
  Radx::fl32 miss = velField->getMissingFl32();
  Radx::fl32 *corrected = new Radx::fl32[velField->getNPoints()];
  for (size_t ii = 0; ii < velField->getNPoints(); ii++) {
    if (vel[ii] != miss) {
      corrected[ii] = vel[ii] - surfFilt;
    } else {
      corrected[ii] = miss;
    }
  }

  // set data for field

  correctedField->setDataFl32(velField->getNPoints(), corrected, true);
  delete[] corrected;

  // add field to ray

  ray->addField(correctedField);

}

//////////////////////////////////////////////////
// copy velocity across for yay

void HcrVelCorrect::_copyVelForRay(RadxRay *ray)
  
{
  
  RadxField *velField = ray->getField(_params.vel_field_name);
  if (velField == NULL) {
    // no vel field, nothing to do
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - no vel field found: "
           << _params.vel_field_name << endl;
    }
    return;
  }

  // create the field to be copied
  
  RadxField *copyField = new RadxField(_params.corrected_vel_field_name,
                                       velField->getUnits());
  copyField->copyMetaData(*velField);
  copyField->setName(_params.corrected_vel_field_name);
  
  // copy the values
  
  const Radx::fl32 *vel = velField->getDataFl32();
  Radx::fl32 *copy = new Radx::fl32[velField->getNPoints()];
  for (size_t ii = 0; ii < velField->getNPoints(); ii++) {
    copy[ii] = vel[ii];
  }

  // set data for field

  copyField->setDataFl32(velField->getNPoints(), copy, true);
  delete[] copy;

  // add field to ray

  ray->addField(copyField);

}


//////////////////////////////////////////////////
// write results to SPDB in XML

void HcrVelCorrect::_writeResultsToSpdb(const RadxRay *filtRay)
  
{

  // check if we have a good velocity
  
  if (!_firFilt.velocityIsValid()) {
    return;
  }

  // form XML string

  string xml;
  xml += RadxXml::writeStartTag("HcrVelCorr", 0);
  xml += RadxXml::writeDouble("VelMeas", 1, _firFilt.getVelMeasured());
  xml += RadxXml::writeDouble("VelStage1", 1, _firFilt.getVelStage1());
  xml += RadxXml::writeDouble("VelSpike", 1, _firFilt.getVelSpike());
  xml += RadxXml::writeDouble("VelCond", 1, _firFilt.getVelCond());
  xml += RadxXml::writeDouble("VelFilt", 1, _firFilt.getVelFilt());
  xml += RadxXml::writeDouble("VelCorr", 1, _firFilt.getVelMeasured() - _firFilt.getVelFilt());
  xml += RadxXml::writeDouble("Range", 1, _firFilt.getRangeToSurface());
  xml += RadxXml::writeDouble("DbzMax", 1, _firFilt.getDbzSurf());
  xml += RadxXml::writeEndTag("HcrVelCorr", 0);

  // write to SPDB

  DsSpdb spdb;
  time_t validTime = filtRay->getTimeSecs();
  spdb.addPutChunk(0, validTime, validTime, xml.size() + 1, xml.c_str());
  if (spdb.put(_params.surface_vel_results_spdb_output_url,
               SPDB_XML_ID, SPDB_XML_LABEL)) {
    cerr << "ERROR - HcrVelCorrect::_writeResultsToSpdb" << endl;
    cerr << spdb.getErrStr() << endl;
    return;
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Wrote HCR vel correction results to spdb, url: " 
         << _params.surface_vel_results_spdb_output_url << endl;
    cerr << "=====================================" << endl;
    cerr << xml;
    cerr << "=====================================" << endl;
  }
 
}

