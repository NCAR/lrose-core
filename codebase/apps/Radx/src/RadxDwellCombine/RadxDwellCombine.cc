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
// RadxDwellCombine.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2014
//
//////////////////////////////////////////////////////////////////////////
//
// Combines multiple dwells from CfRadial files, writes out combined
// dwell files. The goal is to summarize dwells in pointing data - for
// example from vertically-pointing instruments. This can make displaying
// the data in a BSCAN quicker and more efficient.
//
//////////////////////////////////////////////////////////////////////////

#include "RadxDwellCombine.hh"
#include <Radx/RadxRay.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxStatusXml.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/pmu.h>
using namespace std;

// Constructor

RadxDwellCombine::RadxDwellCombine(int argc, char **argv)
  
{

  OK = TRUE;
  _nWarnCensorPrint = 0;
  _momReader = NULL;
  _outputFmq = NULL;

  // set programe name

  _progName = "RadxDwellCombine";
  
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

  // set dwell stats method

  _globalMethod = _getDwellStatsMethod(_params.dwell_stats_method);
  
  if (_params.set_stats_method_for_individual_fields) {
    for (int ii = 0; ii < _params.stats_method_fields_n; ii++) {
      const Params::stats_method_field_t &paramsMethod = 
        _params._stats_method_fields[ii];
      string fieldName = paramsMethod.field_name;
      RadxField::StatsMethod_t method =
        _getDwellStatsMethod(paramsMethod.stats_method);
      RadxField::NamedStatsMethod namedMethod(fieldName, method);
      _namedMethods.push_back(namedMethod);
    } // ii
  }

  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

}

// destructor

RadxDwellCombine::~RadxDwellCombine()

{

  if (_momReader) {
    delete _momReader;
  }

  if (_outputFmq) {
    delete _outputFmq;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RadxDwellCombine::Run()
{

  int iret = 0;

  switch (_params.mode) {
    case Params::FMQ:
      return _runFmq();
      break;
    case Params::ARCHIVE:
      iret = _runArchive();
      break;
    case Params::REALTIME:
      if (_params.latest_data_info_avail) {
        iret = _runRealtimeWithLdata();
      } else {
        iret = _runRealtimeNoLdata();
      }
      break;
    case Params::FILELIST:
    default:
      iret = _runFilelist();
  } // switch

  // if we are writing out on time boundaries, there
  // may be unwritten data, so write it now
  
  if (_params.write_output_files_on_time_boundaries) {
    if (_writeSplitVol()) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in FMQ mode

int RadxDwellCombine::_runFmq()
{

  // Open the input radar queue
  
  if (_openInputFmq()) {
    return -1;
  }

  // Open the output fmq

  if (_openOutputFmq()) {
    return -1;
  }
  
  // loop until read fails
  
  int iret = 0;
  _nRaysRead = 0;
  _nRaysWritten = 0;
  RadxTime prevDwellRayTime;

  while (true) {
    
    // read in next ray
    
    PMU_auto_register("Reading FMQ realtime");
    RadxRay *inputRay = _readFmqRay();
    if (inputRay == NULL) {
      return -1;
    }

    // add the ray to the dwell volume
    
    _dwellVol.addRay(inputRay);
      
    // combine rays if combined time exceeds specified dwell
    
    const vector<RadxRay *> &raysDwell = _dwellVol.getRays();
    size_t nRaysDwell = raysDwell.size();
    if (nRaysDwell > 1) {
      
      _dwellStartTime = raysDwell[0]->getRadxTime();
      _dwellEndTime = raysDwell[nRaysDwell-1]->getRadxTime();
      double dwellSecs = (_dwellEndTime - _dwellStartTime);
      dwellSecs *= ((double) nRaysDwell / (nRaysDwell - 1.0));
      
      if (dwellSecs >= _params.dwell_time_secs) {
        
        // dwell time exceeded
        // compute dwell ray and add to volume
        
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "INFO: _runFmq, using nrays: " << nRaysDwell << endl;
        }
        RadxRay *dwellRay =
          _dwellVol.computeFieldStats(_globalMethod,
                                      _namedMethods,
                                      _params.dwell_stats_max_fraction_missing);
        RadxTime dwellRayTime(dwellRay->getRadxTime());
        double deltaSecs = dwellRayTime - prevDwellRayTime;
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          if (deltaSecs < _params.dwell_time_secs * 0.8 ||
              deltaSecs > _params.dwell_time_secs * 1.2) {
            cerr << "===>> bad dwell time, nRaysDwell, dsecs: "
                 << dwellRay->getRadxTime().asString(3) << ", "
                 << nRaysDwell << ", "
                 << deltaSecs << endl;
          }
        }
        prevDwellRayTime = dwellRayTime;
        
        // create output message from combined ray
        
        RadxMsg msg;
        dwellRay->serialize(msg);
        if ((_params.debug >= Params::DEBUG_VERBOSE) ||
            (_params.debug && (_nRaysWritten % 1000 == 0))) {
          cerr << "Writing ray, time, el, az, rayNum: "
               << dwellRay->getRadxTime().asString(3) << ", "
               << dwellRay->getElevationDeg() << ", "
               << dwellRay->getAzimuthDeg() << ", "
               << _nRaysWritten << endl;
        }
        
        // write the message
        
        if (_outputFmq) {
          if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                                   msg.assembledMsg(), msg.lengthAssembled())) {
            cerr << "ERROR - _runFmq" << endl;
            cerr << "  Cannot write ray to output queue" << endl;
            iret = -1;
          }
        } // if (_outputFmq)
        _nRaysWritten++;
        
        // clean up
        
        RadxRay::deleteIfUnused(dwellRay);
        _dwellVol.clearRays();
        
      } // if (dwellSecs >= _params.dwell_time_secs)
      
    } // if (nRaysDwell > 1)
    
  } // while (true)
  
  return iret;

}

//////////////////////////////////////////////////
// Open input fmq

int RadxDwellCombine::_openInputFmq()
{

  // Instantiate and initialize the input radar queues

  if (_params.debug) {
    cerr << "DEBUG - opening input fmq for moments: "
         << _params.input_fmq_url << endl;
  }
  
  _momReader = new IwrfMomReaderFmq(_params.input_fmq_url);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _momReader->setDebug(IWRF_DEBUG_NORM);
  }

  if (_params.seek_to_end_of_input_fmq) {
    _momReader->seekToEnd();
  } else {
    _momReader->seekToStart();
  }

  return 0;

}

//////////////////////////////////////////////////
// Open output fmq

int RadxDwellCombine::_openOutputFmq()
{

  // create the output FMQ
  
  _outputFmq = new DsFmq;

  if (_outputFmq->init(_params.output_fmq_url,
                       _progName.c_str(),
                       _params.debug >= Params::DEBUG_VERBOSE,
                       DsFmq::READ_WRITE, DsFmq::END,
                       _params.output_fmq_compress,
                       _params.output_fmq_n_slots,
                       _params.output_fmq_buf_size)) {
    cerr << "ERROR - " << _progName << "::_openFmqs" << endl;
    cerr << "  Cannot open output fmq, URL: " << _params.output_fmq_url << endl;
    return -1;
  }
  if (_params.output_fmq_compress) {
    _outputFmq->setCompressionMethod(TA_COMPRESSION_GZIP);
  }
  if (_params.output_fmq_write_blocking) {
    _outputFmq->setBlockingWrite();
  }
  if (_params.output_fmq_data_mapper_report_interval > 0) {
    _outputFmq->setRegisterWithDmap(true, _params.output_fmq_data_mapper_report_interval);
  }
  // _outputFmq->setSingleWriter();

  return 0;

}

/////////////////////////////////////////////////////////////////
// Read a ray in FMQ mode
// Creates ray, must be freed by caller.

RadxRay *RadxDwellCombine::_readFmqRay()
{

  // read next ray
  
  RadxRay *ray = _momReader->readNextRay();
  if (ray == NULL) {
    return NULL;
  }
  _nRaysRead++;
  
  // check for platform update
  
  if (_momReader->getPlatformUpdated()) {
    
    RadxPlatform platform = _momReader->getPlatform();
    _platform = platform;
    _setPlatformMetadata(_platform);
    _wavelengthM = _platform.getWavelengthM();
    if (_wavelengthM < 0) {
      _wavelengthM = 0.003176;
    }
    _prt = ray->getPrtSec();
    _nyquist = ((_wavelengthM / _prt) / 4.0);
    if (_params.fixed_location_mode) {
      _platform.setLatitudeDeg(_params.fixed_radar_location.latitudeDeg);
      _platform.setLongitudeDeg(_params.fixed_radar_location.longitudeDeg);
      _platform.setAltitudeKm(_params.fixed_radar_location.altitudeKm);
    }
    
    // create message
    RadxMsg msg;
    platform.serialize(msg);
    // write the platform to the output queue
    if (_outputFmq) {
      if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                               msg.assembledMsg(), msg.lengthAssembled())) {
        cerr << "ERROR - _readRay" << endl;
        cerr << "  Cannot copy platform metadata to output queue" << endl;
      }
    }

  } // if (_momReaderShort->getPlatformUpdated())

  // check for calibration update
  
  if (_momReader->getRcalibUpdated()) {
    const vector<RadxRcalib> &calibs = _momReader->getRcalibs();
    _calibs = calibs;
    for (size_t ii = 0; ii < calibs.size(); ii++) {
      // create message
      RadxRcalib calib = calibs[ii];
      RadxMsg msg;
      calib.serialize(msg);
      // write to output queue
      if (_outputFmq) {
        if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                                 msg.assembledMsg(), msg.lengthAssembled())) {
          cerr << "ERROR - _readRay" << endl;
          cerr << "  Cannot copy calib to output queue" << endl;
        }
      }
    } // ii
  }

  // check for status xml update
  
  if (_momReader->getStatusXmlUpdated()) {
    const string statusXml = _momReader->getStatusXml();
    _statusXml = statusXml;
    // create RadxStatusXml object
    RadxStatusXml status;
    status.setXmlStr(statusXml);
    // create message
    RadxMsg msg;
    status.serialize(msg);
    // write to output queue
    if (_outputFmq) {
      if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                               msg.assembledMsg(), msg.lengthAssembled())) {
        cerr << "ERROR - _readRay" << endl;
        cerr << "  Cannot copy status xml to output queue" << endl;
      }
    }
  }
  
  // update events
  
  _events = _momReader->getEvents();
  for (size_t ii = 0; ii < _events.size(); ii++) {
    RadxEvent event = _events[ii];
    RadxMsg msg;
    event.serialize(msg);
    // write to output queue
    if (_outputFmq) {
      if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                               msg.assembledMsg(), msg.lengthAssembled())) {
        cerr << "ERROR - _readRay" << endl;
        cerr << "  Cannot copy event to output queue" << endl;
      }
    }
  } // ii

  // override location as required
  
  if (_params.fixed_location_mode) {
    RadxGeoref *georef = ray->getGeoreference();
    if (georef != NULL) {
      georef->setLatitude(_params.fixed_radar_location.latitudeDeg);
      georef->setLongitude(_params.fixed_radar_location.longitudeDeg);
      georef->setAltitudeKmMsl(_params.fixed_radar_location.altitudeKm);
      georef->setEwVelocity(0.0);
      georef->setNsVelocity(0.0);
      georef->setVertVelocity(0.0);
      georef->setHeading(0.0);
      georef->setTrack(0.0);
      georef->setEwWind(0.0);
      georef->setNsWind(0.0);
      georef->setVertWind(0.0);
    }
  }
  
  return ray;

}

//////////////////////////////////////////////////
// Run in filelist mode

int RadxDwellCombine::_runFilelist()
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

int RadxDwellCombine::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - RadxDwellCombine::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - RadxDwellCombine::_runFilelist()" << endl;
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

int RadxDwellCombine::_runRealtimeWithLdata()
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

int RadxDwellCombine::_runRealtimeNoLdata()
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

int RadxDwellCombine::_processFile(const string &readPath)
{

  PMU_auto_register("Processing file");

  // check we have not already processed this file
  // in the file aggregation step
  
  RadxPath thisPath(readPath);
  for (size_t ii = 0; ii < _readPaths.size(); ii++) {
    RadxPath rpath(_readPaths[ii]);
    if (thisPath.getFile() == rpath.getFile()) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Skipping file: " << readPath << endl;
        cerr << "  Previously processed in aggregation step" << endl;
      }
      return 0;
    }
  }
  
  if (_params.debug) {
    cerr << "INFO - RadxDwellCombine::Run" << endl;
    cerr << "  Input path: " << readPath << endl;
  }
  
  // if we are reading gematronik files in realtime mode, we need to wait
  // for all fields to be written before proceeding
  
  if (_params.mode == Params::REALTIME && _params.gematronik_realtime_mode) {
    if (_params.debug) {
      cerr << "Waiting for all Gematronik fields, sleeping for secs: "
           << _params.gematronik_realtime_wait_secs << endl;
    }
    for (int ii = 0; ii < _params.gematronik_realtime_wait_secs; ii++) {
      PMU_auto_register("Waiting for Gematronik files");
      umsleep(1000);
    }
    PMU_force_register("Got Gematronik files");
  }
  
  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file

  RadxVol vol;
  if (inFile.readFromPath(readPath, vol)) {
    cerr << "ERROR - RadxDwellCombine::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  _readPaths = inFile.getReadPaths();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t ii = 0; ii < _readPaths.size(); ii++) {
      cerr << "  ==>> read in file: " << _readPaths[ii] << endl;
    }
  }

  // remove unwanted fields
  
  if (_params.exclude_specified_fields) {
    for (int ii = 0; ii < _params.excluded_fields_n; ii++) {
      if (_params.debug) {
        cerr << "Removing field name: " << _params._excluded_fields[ii] << endl;
      }
      vol.removeField(_params._excluded_fields[ii]);
    }
  }

  // linear transform on fields as required

  if (_params.apply_linear_transforms) {
    _applyLinearTransform(vol);
  }

  // add field folding attribute if needed

  if (_params.set_field_folds_attribute) {
    _setFieldFoldsAttribute(vol);
  }

  // combine the dwells

  if (_params.center_dwell_on_time) {
    _combineDwellsCentered(vol);
  } else {
    _combineDwells(vol);
  }

  // censor as needed

  if (_params.apply_censoring) {
    if (_params.debug) {
      cerr << "DEBUG - applying censoring" << endl;
    }
    _censorFields(vol);
  }

  // set field type, names, units etc
  
  _convertFields(vol);

  if (_params.set_output_encoding_for_all_fields) {
    _convertAllFields(vol);
  }

  // set global attributes

  _setGlobalAttr(vol);

  // override platform metadata
  
  if (_params.override_platform_type || _params.override_primary_axis) {
    RadxPlatform platform = vol.getPlatform();
    _setPlatformMetadata(platform);
    vol.setPlatform(platform);
  }

  // write the file

  if (_writeVol(vol)) {
    cerr << "ERROR - RadxDwellCombine::_processFile" << endl;
    cerr << "  Cannot write volume to file" << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// set up read

void RadxDwellCombine::_setupRead(RadxFile &file)
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
// set up write

void RadxDwellCombine::_setupWrite(RadxFile &file)
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
  file.setCompressionLevel(4);

  // set output format

  switch (_params.output_format) {
    case Params::OUTPUT_FORMAT_CFRADIAL2:
      file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL2);
      break;
    default:
    case Params::OUTPUT_FORMAT_CFRADIAL:
      file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  }

  if (strlen(_params.output_filename_prefix) > 0) {
    file.setWriteFileNamePrefix(_params.output_filename_prefix);
  }

  file.setWriteInstrNameInFileName(_params.include_instrument_name_in_file_name);
  file.setWriteSiteNameInFileName(_params.include_site_name_in_file_name);
  file.setWriteSubsecsInFileName(_params.include_subsecs_in_file_name);
  file.setWriteScanTypeInFileName(_params.include_scan_type_in_file_name);
  file.setWriteVolNumInFileName(_params.include_vol_num_in_file_name);
  file.setWriteHyphenInDateTime(_params.use_hyphen_in_file_name_datetime_part);

}

//////////////////////////////////////////////////
// set selected global attributes

void RadxDwellCombine::_setGlobalAttr(RadxVol &vol)
{

  vol.setDriver("RadxDwellCombine(NCAR)");

  if (strlen(_params.radar_name_override) > 0) {
    vol.setInstrumentName(_params.radar_name_override);
  }

  if (strlen(_params.site_name_override) > 0) {
    vol.setSiteName(_params.site_name_override);
  }

  if (strlen(_params.title_override) > 0) {
    vol.setTitle(_params.title_override);
  }

  if (strlen(_params.institution_override) > 0) {
    vol.setInstitution(_params.institution_override);
  }

  if (strlen(_params.references_override) > 0) {
    vol.setReferences(_params.references_override);
  }

  if (strlen(_params.source_override) > 0) {
    vol.setSource(_params.source_override);
  }

  if (strlen(_params.history_override) > 0) {
    vol.setHistory(_params.history_override);
  } else {
    string history(vol.getHistory());
    history += "\n";
    history += "Dwells combined using RadxDwellCombine\n";
    vol.setHistory(history);
  }
  
  if (strlen(_params.author_override) > 0) {
    vol.setAuthor(_params.author_override);
  }

  if (strlen(_params.comment_override) > 0) {
    vol.setComment(_params.comment_override);
  }

  RadxTime now(RadxTime::NOW);
  vol.setCreated(now.asString());

}

//////////////////////////////////////////////////
// write out the volume

int RadxDwellCombine::_writeVol(RadxVol &vol)
{

  // are we writing files on time boundaries
  
  if (_params.write_output_files_on_time_boundaries) {
    return _writeVolOnTimeBoundary(vol);
  }

  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  if (_params.output_filename_mode == Params::SPECIFY_FILE_NAME) {

    string outPath = _params.output_dir;
    outPath += PATH_DELIM;
    outPath += _params.output_filename;

    // write to path
  
    if (outFile.writeToPath(vol, outPath)) {
      cerr << "ERROR - RadxDwellCombine::_writeVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
      
  } else {

    // write to dir
  
    if (outFile.writeToDir(vol, _params.output_dir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - RadxDwellCombine::_writeVol" << endl;
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
    if (ldata.write(vol.getEndTimeSecs())) {
      cerr << "WARNING - RadxDwellCombine::_writeVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// write out the data splitting on time

int RadxDwellCombine::_writeVolOnTimeBoundary(RadxVol &vol)
{
  
  // check for time gap

  RadxTime newVolStart = vol.getStartRadxTime();
  RadxTime splitVolEnd = _splitVol.getEndRadxTime();
  double gapSecs = newVolStart - splitVolEnd;
  if (gapSecs > _params.output_file_time_interval_secs * 2) {
    if (_params.debug) {
      cerr << "==>> Found time gap between volumes" << endl;
      cerr << "  splitVolEnd: " << splitVolEnd.asString(3) << endl;
      cerr << "  newVolStart: " << newVolStart.asString(3) << endl;
    }
    _writeSplitVol();
    _setNextEndOfVolTime(newVolStart);
    // clear out rays from previous file
    _dwellVol.clearRays();
    // clear any rays before the new vol start
    // these could have been introduced during the merge
  }

  // add rays to the output vol

  _splitVol.copyMeta(vol);
  vector<RadxRay *> &volRays = vol.getRays();
  for (size_t ii = 0; ii < volRays.size(); ii++) {
    RadxRay *ray = volRays[ii];
    if (ray->getRadxTime() > _nextEndOfVolTime) {
      if (_writeSplitVol()) {
        return -1;
      }
    }
    RadxRay *splitRay = new RadxRay(*ray);
    _splitVol.addRay(splitRay);
  } // ii

  return 0;

}

//////////////////////////////////////////////////
// write out the split volume

int RadxDwellCombine::_writeSplitVol()
{

  // sanity check

  if (_splitVol.getNRays() < 1) {
    return 0;
  }

  // load the sweep information from the rays
  
  _splitVol.loadSweepInfoFromRays();

  // load the volume information from the rays
  
  _splitVol.loadVolumeInfoFromRays();
  
  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  // write out

  if (outFile.writeToDir(_splitVol, _params.output_dir,
                         _params.append_day_dir_to_output_dir,
                         _params.append_year_dir_to_output_dir)) {
    cerr << "ERROR - RadxDwellCombine::_writeSplitVol" << endl;
    cerr << "  Cannot write file to dir: " << _params.output_dir << endl;
    cerr << outFile.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote file: " << outFile.getPathInUse() << endl;
    cerr << "  StartTime: " << _splitVol.getStartRadxTime().asString(3) << endl;
    cerr << "  EndTime  : " << _splitVol.getEndRadxTime().asString(3) << endl;
  }

  // write latest data info file if requested 
  
  if (_params.write_latest_data_info) {
    string outputPath = outFile.getPathInUse();
    DsLdataInfo ldata(_params.output_dir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    string relPath;
    RadxPath::stripDir(_params.output_dir, outputPath, relPath);
    ldata.setRelDataPath(relPath);
    ldata.setWriter(_progName);
    if (ldata.write(_splitVol.getEndTimeSecs())) {
      cerr << "WARNING - RadxDwellCombine::_writeSplitVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  // update next end of vol time

  RadxTime nextVolStart(_splitVol.getEndTimeSecs() + 1);
  _setNextEndOfVolTime(nextVolStart);

  // clear

  _splitVol.clearRays();

  return 0;

}

//////////////////////////////////////////////////
// Compute next end of vol time

void RadxDwellCombine::_setNextEndOfVolTime(RadxTime &refTime)
{
  _nextEndOfVolTime.set
    (((refTime.utime() / _params.output_file_time_interval_secs) + 1) *
     _params.output_file_time_interval_secs);
  if (_params.debug) {
    cerr << "==>> Next end of vol time: " << _nextEndOfVolTime.asString(3) << endl;
  }
}

//////////////////////////////////////////////////
// apply linear transform to fields as required

void RadxDwellCombine::_applyLinearTransform(RadxVol &vol)
{

  for (int ii = 0; ii < _params.transform_fields_n; ii++) {
    const Params::transform_field_t &tfld = _params._transform_fields[ii];
    string iname = tfld.input_field_name;
    double scale = tfld.transform_scale;
    double offset = tfld.transform_offset;
    vol.applyLinearTransform(iname, scale, offset);
  } // ii

}

////////////////////////////////////////////////////////
// set the field folds attribute on selected fields

void RadxDwellCombine::_setFieldFoldsAttribute(RadxVol &vol)
{

  for (int ii = 0; ii < _params.field_folds_n; ii++) {
    
    const Params::field_folds_t &fld = _params._field_folds[ii];

    vol.setFieldFolds(fld.field_name,
                      fld.use_nyquist,
                      fld.fold_limit_lower,
                      fld.fold_limit_upper);

  } // ii
  
}

//////////////////////////////////////////////////
// rename fields as required

void RadxDwellCombine::_convertFields(RadxVol &vol)
{

  if (!_params.set_output_fields) {
    return;
  }

  for (int ii = 0; ii < _params.output_fields_n; ii++) {

    const Params::output_field_t &ofld = _params._output_fields[ii];
    
    string iname = ofld.input_field_name;
    string oname = ofld.output_field_name;
    string lname = ofld.long_name;
    string sname = ofld.standard_name;
    string ounits = ofld.output_units;
    
    Radx::DataType_t dtype = Radx::ASIS;
    switch(ofld.encoding) {
      case Params::OUTPUT_ENCODING_FLOAT32:
        dtype = Radx::FL32;
        break;
      case Params::OUTPUT_ENCODING_INT32:
        dtype = Radx::SI32;
        break;
      case Params::OUTPUT_ENCODING_INT16:
        dtype = Radx::SI16;
        break;
      case Params::OUTPUT_ENCODING_INT08:
        dtype = Radx::SI08;
        break;
      case Params::OUTPUT_ENCODING_ASIS:
        dtype = Radx::ASIS;
      default: {}
    }

    if (ofld.output_scaling == Params::SCALING_DYNAMIC) {
      vol.convertField(iname, dtype, 
                       oname, ounits, sname, lname);
    } else {
      vol.convertField(iname, dtype, 
                       ofld.output_scale, ofld.output_offset,
                       oname, ounits, sname, lname);
    }
    
  }

}

//////////////////////////////////////////////////
// convert all fields to specified output encoding

void RadxDwellCombine::_convertAllFields(RadxVol &vol)
{

  switch(_params.output_encoding) {
    case Params::OUTPUT_ENCODING_FLOAT32:
      vol.convertToFl32();
      return;
    case Params::OUTPUT_ENCODING_INT32:
      vol.convertToSi32();
      return;
    case Params::OUTPUT_ENCODING_INT16:
      vol.convertToSi16();
      return;
    case Params::OUTPUT_ENCODING_INT08:
      vol.convertToSi08();
      return;
    case Params::OUTPUT_ENCODING_ASIS:
    default:
      return;
  }

}

/////////////////////////////////////////////////////////////////////////
// Combine the dwells in this volume

int RadxDwellCombine::_combineDwells(RadxVol &vol)

{
  
  if (_params.debug) {
    cerr << "INFO - combineDwells: nrays left from previous file: "
         << _dwellVol.getNRays() << endl;
  }

  // create a volume for stats
  
  vector<RadxRay *> combRays;
  
  const vector<RadxRay *> &fileRays = vol.getRays();
  for (size_t iray = 0; iray < fileRays.size(); iray++) {
    
    // add rays to stats vol
    
    RadxRay *ray = new RadxRay(*fileRays[iray]);
    if (_dwellVol.getNRays() == 0) {
      _dwellStartTime = ray->getRadxTime();
    }
    _dwellVol.addRay(ray);
    int nRaysDwell = _dwellVol.getNRays();
    _dwellEndTime = ray->getRadxTime();
    double dwellSecs = (_dwellEndTime - _dwellStartTime);
    if (nRaysDwell > 1) {
      dwellSecs *= ((double) nRaysDwell / (nRaysDwell - 1.0));
    }
    
    // dwell time exceeded, so compute dwell ray and add to volume
    
    if (dwellSecs >= _params.dwell_time_secs) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "INFO: _combineDwells, using nrays: " << nRaysDwell << endl;
      }
      RadxRay *dwellRay =
        _dwellVol.computeFieldStats(_globalMethod,
                                    _namedMethods,
                                    _params.dwell_stats_max_fraction_missing);
      if (dwellRay->getRadxTime() >= vol.getStartRadxTime() - 60) {
        combRays.push_back(dwellRay);
      } else {
        RadxRay::deleteIfUnused(dwellRay);
      }
      // clear out stats vol
      _dwellVol.clearRays();
    }
      
  } // iray

  // move combination rays into volume

  vol.clearRays();
  for (size_t ii = 0; ii < combRays.size(); ii++) {
    vol.addRay(combRays[ii]);
  }
  
  // compute volume metadata

  vol.loadSweepInfoFromRays();

  return 0;

}

/////////////////////////////////////////////////////////////////////////
// Combine the dwells centered on time

int RadxDwellCombine::_combineDwellsCentered(RadxVol &vol)

{
  
  if (_params.debug) {
    cerr << "INFO - combineDwellsCentered: nrays left from previous file: "
         << _dwellVol.getNRays() << endl;
  }

  // create a volume for combined dwells
  
  vector<RadxRay *> combRays;
  
  const vector<RadxRay *> &fileRays = vol.getRays();
  for (size_t iray = 0; iray < fileRays.size(); iray++) {

    RadxRay *ray = new RadxRay(*fileRays[iray]);
    _latestRayTime = ray->getRadxTime();
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "==>> got new ray, latestRayTime: " << _latestRayTime.asString(3) << endl;
    }

    // at the start of reading a volume, we always need 
    // at least 1 ray in the dwell

    if (_dwellVol.getNRays() == 0) {
      _dwellVol.addRay(ray);
      continue;
    }
    
    // set dwell time limits if we have just 1 ray in the dwell so far
    
    if (_dwellVol.getNRays() == 1) {

      _dwellStartTime = _dwellVol.getRays()[0]->getRadxTime();

      RadxTime volStartTime(vol.getStartTimeSecs());
      double dsecs = _latestRayTime - volStartTime;
      double roundedSecs =
        ((int) (dsecs / _params.dwell_time_secs) + 1.0) * _params.dwell_time_secs;
      _dwellMidTime = volStartTime + roundedSecs;
      _dwellEndTime = _dwellMidTime + _params.dwell_time_secs / 2.0;

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> starting new dwell <<==" << endl;
        cerr << "  _dwellStartTime: " << _dwellStartTime.asString(3) << endl;
        cerr << "  _dwellMidTime: " << _dwellMidTime.asString(3) << endl;
        cerr << "  _dwellEndTime: " << _dwellEndTime.asString(3) << endl;
      }
        
    }
    
    // dwell time exceeded, so compute dwell ray stats
    // and add results to volume
    
    if (_latestRayTime > _dwellEndTime) {

      // beyond end of dwell, process this one
      
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "INFO: _combineDwellsCentered, using nrays: "
             << _dwellVol.getNRays() << endl;
        const vector<RadxRay *> &dwellRays = _dwellVol.getRays();
        for (size_t jray = 0; jray < dwellRays.size(); jray++) {
          const RadxRay *dray = dwellRays[jray];
          cerr << "INFO: using ray at time: "
               << dray->getRadxTime().asString(3) << endl;
        }
      } // debug

      // compute ray for dwell
      
      RadxRay *dwellRay =
        _dwellVol.computeFieldStats(_globalMethod, _namedMethods,
                                    _params.dwell_stats_max_fraction_missing);

      // add it to the combination

      if (dwellRay) {
        if (dwellRay->getRadxTime() >= vol.getStartRadxTime() - 60) {
          dwellRay->setTime(_dwellMidTime);
          combRays.push_back(dwellRay);
        } else {
          RadxRay::deleteIfUnused(dwellRay);
        }
      }

      // clear out stats vol

      _dwellVol.clearRays();

    } // if (_latestRayTime > _dwellEndTime) {
      
    // add the latest ray to the next dwell vol
    
    _dwellVol.addRay(ray);
    
  } // iray

  // move combination rays into volume
  
  vol.clearRays();
  for (size_t ii = 0; ii < combRays.size(); ii++) {
    vol.addRay(combRays[ii]);
  }
  
  // compute volume metadata

  vol.loadSweepInfoFromRays();

  return 0;

}

////////////////////////////////////////////////////////
// set dwell stats method from params

RadxField::StatsMethod_t
  RadxDwellCombine::_getDwellStatsMethod(Params::dwell_stats_method_t method)
  
{

  switch (method) {

    case Params::DWELL_STATS_MEAN:
      return RadxField::STATS_METHOD_MEAN;
      break;
    case Params::DWELL_STATS_MEDIAN:
      return RadxField::STATS_METHOD_MEDIAN;
      break;
    case Params::DWELL_STATS_DISCRETE_MODE:
      return RadxField::STATS_METHOD_DISCRETE_MODE;
      break;
    case Params::DWELL_STATS_MAXIMUM:
      return RadxField::STATS_METHOD_MAXIMUM;
      break;
    case Params::DWELL_STATS_MINIMUM:
      return RadxField::STATS_METHOD_MINIMUM;
      break;
    case Params::DWELL_STATS_MIDDLE:
    default:
      return RadxField::STATS_METHOD_MIDDLE;

  }

}


////////////////////////////////////////////////////////////////////
// censor fields in vol

void RadxDwellCombine::_censorFields(RadxVol &vol)

{

  vector<RadxRay *> &rays = vol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    _censorRay(rays[ii]);
  }
  

}

////////////////////////////////////////////////////////////////////
// censor fields in a ray

void RadxDwellCombine::_censorRay(RadxRay *ray)

{

  if (!_params.apply_censoring) {
    return;
  }

  // convert fields to floats

  vector<Radx::DataType_t> fieldTypes;
  vector<RadxField *> fields = ray->getFields();
  for (size_t ii = 0; ii < fields.size(); ii++) {
    RadxField *field = fields[ii];
    Radx::DataType_t dtype = field->getDataType();
    fieldTypes.push_back(dtype);
    field->convertToFl32();
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
  // except those specified to not be censored

  if (_params.specify_non_censored_fields) {

    // only censor fields that are not excluded

    for (size_t ifield = 0; ifield < fields.size(); ifield++) {
      RadxField *field = fields[ifield];
      bool applyCensoring = true;
      for (int ii = 0; ii < _params.non_censored_fields_n; ii++) {
        string fieldToAvoid = _params._non_censored_fields[ii];
        if (field->getName() == fieldToAvoid) {
          applyCensoring = false;
          break;
        }
      }
      if (applyCensoring) {
        Radx::fl32 *fdata = (Radx::fl32 *) field->getData();
        for (size_t igate = 0; igate < nGates; igate++) {
          if (censorFlag[igate] == 1) {
            fdata[igate] = Radx::missingFl32;
          }
        } // igate
      } else {
        if (_params.debug >= Params::DEBUG_EXTRA) {
          cerr << "Not censoring field: " << field->getName() << endl;
        }
      }
    } // ifield

  } else {

    // censor all fields

    for (size_t ifield = 0; ifield < fields.size(); ifield++) {
      RadxField *field = fields[ifield];
      Radx::fl32 *fdata = (Radx::fl32 *) field->getData();
      for (size_t igate = 0; igate < nGates; igate++) {
        if (censorFlag[igate] == 1) {
          fdata[igate] = Radx::missingFl32;
        }
      } // igate
    } // ifield

  }
    
  // convert back to original types
  
  for (size_t ii = 0; ii < fields.size(); ii++) {
    RadxField *field = fields[ii];
    field->convertToType(fieldTypes[ii]);
  }

}

////////////////////////////////////////////////////////
// modify platform metadata

void RadxDwellCombine::_setPlatformMetadata(RadxPlatform &platform)
  
{

  if (_params.override_platform_type) {
    platform.setPlatformType((Radx::PlatformType_t) _params.platform_type);
  }

  if (_params.override_primary_axis) {
    platform.setPrimaryAxis((Radx::PrimaryAxis_t) _params.primary_axis);
  }

}
