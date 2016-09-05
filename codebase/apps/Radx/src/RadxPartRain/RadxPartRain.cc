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
// RadxPartRain.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2012
//
///////////////////////////////////////////////////////////////
//
// RadxPartRain reads moments from Radx-supported format files, 
// computes the PID and PRECIP rates and writes out the results 
// to Radx-supported format files
//
///////////////////////////////////////////////////////////////

#include "RadxPartRain.hh"
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
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxXml.hh>
using namespace std;

// Constructor

RadxPartRain::RadxPartRain(int argc, char **argv)
  
{

  OK = TRUE;
  _engine = NULL;

  // set programe name

  _progName = "RadxPartRain";
  
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

  // check parameters for consistency

  if (_params.estimate_zdr_bias_in_ice ||
      _params.estimate_zdr_bias_in_bragg) {
    if (_params.KDP_available) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  For estimating ZDR bias, you must set KDP_available to FALSE." << endl;
      cerr << "  This forces KDP to be recomputed from PHIDP." << endl;
      cerr << "  A recomputed KDP is required for estimating ZDR bias." << endl;
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

}

//////////////////////////////////////
// destructor

RadxPartRain::~RadxPartRain()

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

int RadxPartRain::Run()
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

int RadxPartRain::_runFilelist()
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

int RadxPartRain::_runArchive()
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
    cerr << "ERROR - RadxPartRain::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - RadxPartRain::_runFilelist()" << endl;
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

int RadxPartRain::_runRealtime()
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

int RadxPartRain::_processFile(const string &filePath)
{

  PMU_auto_register("Processing file");
  
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
    cerr << "INFO - RadxPartRain::Run" << endl;
    cerr << "  Input path: " << filePath << endl;
  }
  
  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  RadxVol vol;
  if (inFile.readFromPath(filePath, vol)) {
    cerr << "ERROR - RadxPartRain::Run" << endl;
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

  // initialize for ZDR bias and self consistency results

  _zdrInIceResults.clear();
  _zdrInBraggResults.clear();
  _zdrmInIceResults.clear();
  _zdrmInBraggResults.clear();
  _selfConResults.clear();

  // compute the derived fields
  
  if (_compute(vol)) {
    cerr << "ERROR - RadxPartRain::Run" << endl;
    cerr << "  Cannot compute moments" << endl;
    return -1;
  }

  // optionally compute ZDR bias and write to SPDB

  if (_params.estimate_zdr_bias_in_ice ||
      _params.estimate_zdr_bias_in_bragg) {
    _computeZdrBias(vol);
  }

  // optionally compute Z bias using self consistency and write results to SPDB

  if (_params.estimate_z_bias_using_self_consistency) {
    _computeSelfConZBias(vol);
  }

  // write results to output file

  if (_params.write_output_volume) {
    if (_writeVol(vol)) {
      return -1;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// encode fields for output

void RadxPartRain::_encodeFieldsForOutput(RadxVol &vol)
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

void RadxPartRain::_setupRead(RadxFile &file)
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

  if (_params.SNR_available) {
    file.addReadField(_params.SNR_field_name);
  }
  file.addReadField(_params.DBZ_field_name);
  file.addReadField(_params.ZDR_field_name);
  file.addReadField(_params.PHIDP_field_name);
  file.addReadField(_params.RHOHV_field_name);
  if (_params.LDR_available) {
    file.addReadField(_params.LDR_field_name);
  }
  if (_params.RHO_VXHX_available) {
    file.addReadField(_params.RHO_VXHX_field_name);
  }
  if (_params.KDP_available) {
    file.addReadField(_params.KDP_field_name);
  }
  if (_params.locate_rlan_interference) {
    file.addReadField(_params.VEL_field_name);
    if (_params.NCP_available) {
      file.addReadField(_params.NCP_field_name);
    }
    if (_params.WIDTH_available) {
      file.addReadField(_params.WIDTH_field_name);
    }
  }
  if (_params.copy_input_fields_to_output) {
    for (int ii = 0; ii < _params.copy_fields_n; ii++) {
      file.addReadField(_params._copy_fields[ii].input_name);
    }
  }
  if (_params.estimate_zdr_bias_in_ice ||
      _params.estimate_zdr_bias_in_bragg ||
      _params.estimate_z_bias_using_self_consistency) {
    file.addReadField(_params.VEL_field_name);
    file.addReadField(_params.ZDRM_field_name);
    file.addReadField(_params.RHOHV_NNC_field_name);
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

void RadxPartRain::_setupWrite(RadxFile &file)
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

int RadxPartRain::_writeVol(RadxVol &vol)
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
      cerr << "ERROR - RadxPartRain::_writeVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
      
  } else {

    // write to dir
  
    if (outFile.writeToDir(vol, _params.output_dir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - RadxPartRain::_writeVol" << endl;
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
    cerr << "WARNING - RadxPartRain::_writeVol" << endl;
    cerr << "  Cannot write latest data info file to dir: "
         << _params.output_dir << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// compute the moments for all rays in volume

int RadxPartRain::_compute(RadxVol &vol)
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

int RadxPartRain::_computeSingleThreaded(RadxVol &vol)
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

int RadxPartRain::_computeMultiThreaded(RadxVol &vol)
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

int RadxPartRain::_storeDerivedRay(ComputeThread *thread)

{
  
  RadxRay *derivedRay = thread->getMomRay();
  if (derivedRay == NULL) {
    _availThreads.push_back(thread);
    return -1;
  } else {
    // good return, add to results
    _derivedRays.push_back(derivedRay);
  }
  
  // load ZDR bias results

  const vector<double> &threadZdrInIceResults =
    thread->getComputeEngine()->getZdrInIceResults();
  for (size_t ii = 0; ii < threadZdrInIceResults.size(); ii++) {
    _zdrInIceResults.push_back(threadZdrInIceResults[ii]);
  }

  const vector<double> &threadZdrInBraggResults =
    thread->getComputeEngine()->getZdrInBraggResults();
  for (size_t ii = 0; ii < threadZdrInBraggResults.size(); ii++) {
    _zdrInBraggResults.push_back(threadZdrInBraggResults[ii]);
  }

  const vector<double> &threadZdrmInIceResults =
    thread->getComputeEngine()->getZdrmInIceResults();
  for (size_t ii = 0; ii < threadZdrmInIceResults.size(); ii++) {
    _zdrmInIceResults.push_back(threadZdrmInIceResults[ii]);
  }

  const vector<double> &threadZdrmInBraggResults =
    thread->getComputeEngine()->getZdrmInBraggResults();
  for (size_t ii = 0; ii < threadZdrmInBraggResults.size(); ii++) {
    _zdrmInBraggResults.push_back(threadZdrmInBraggResults[ii]);
  }

  // load self consistency results

  const vector<ComputeEngine::self_con_t> &threadSelfConResults =
    thread->getComputeEngine()->getSelfConResults();
  for (size_t ii = 0; ii < threadSelfConResults.size(); ii++) {
    _selfConResults.push_back(threadSelfConResults[ii]);
  }
  
  return 0;

}
      
///////////////////////////////////////////////////////////
// Thread function to compute moments

void *RadxPartRain::_computeInThread(void *thread_data)
  
{
  
  // get thread data from args

  ComputeThread *compThread = (ComputeThread *) thread_data;
  RadxPartRain *app = compThread->getApp();
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

void RadxPartRain::_findTransitions(vector<RadxRay *> &rays)

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
  
int RadxPartRain::_retrieveTempProfile(const RadxVol &vol)
  
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
  vector<NcarParticleId::TmpPoint> retrievedProfile;
  if (_tempProfile.getTempProfile(_params.sounding_spdb_url,
                                  vol.getStartTimeSecs(),
                                  retrievedTime,
                                  retrievedProfile)) {
    cerr << "ERROR - RadxPartRain::_tempProfileInit" << endl;
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

int RadxPartRain::_retrieveSiteTempFromSpdb(const RadxVol &vol,
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
    cerr << "WARNING - RadxPartRain::_retrieveSiteTempFromSpdb" << endl;
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
    cerr << "WARNING - RadxPartRain::_retrieveSiteTempFromSpdb" << endl;
    cerr << "  No suitable temp data from URL: " << _params.site_temp_spdb_url << endl;
    cerr << "  Search time: " << RadxTime::strm(searchTime) << endl;
    cerr << "  Search margin (secs): " << _params.site_temp_search_margin_secs << endl;
    return -1;
  }

  const Spdb::chunk_t &chunk = chunks[0];
  WxObs obs;
  if (obs.disassemble(chunk.data, chunk.len)) {
    cerr << "WARNING - RadxPartRain::_retrieveSiteTempFromSpdb" << endl;
    cerr << "  SPDB data is of incorrect type, prodLabel: " << spdb.getProdLabel() << endl;
    cerr << "  Should be station data type" << endl;
    cerr << "  URL: " << _params.site_temp_spdb_url << endl;
    return -1;
  }

  tempC = obs.getTempC();
  timeForTemp = obs.getObservationTime();

  return 0;

}

/////////////////////////////////////////////////////////////
// write zdr bias results to SPDB in XML

void RadxPartRain::_computeZdrBias(const RadxVol &vol)

{

  // compute the bias stats

  _zdrStatsIce.clear();
  _zdrStatsBragg.clear();

  _zdrmStatsIce.clear();
  _zdrmStatsBragg.clear();

  if ((int) _zdrInIceResults.size() > _params.zdr_bias_ice_min_npoints_valid) {
    _loadZdrResults(_zdrInIceResults,
                    _zdrStatsIce,
                    _params.zdr_bias_ice_percentiles_n,
                    _params._zdr_bias_ice_percentiles);
  }
  
  if ((int) _zdrmInIceResults.size() > _params.zdr_bias_ice_min_npoints_valid) {
    _loadZdrResults(_zdrmInIceResults,
                    _zdrmStatsIce,
                    _params.zdr_bias_ice_percentiles_n,
                    _params._zdr_bias_ice_percentiles);
  }
  
  if ((int) _zdrInBraggResults.size() > _params.zdr_bias_bragg_min_npoints_valid) {
    _loadZdrResults(_zdrInBraggResults,
                    _zdrStatsBragg,
                    _params.zdr_bias_bragg_percentiles_n,
                    _params._zdr_bias_bragg_percentiles);
  }
  
  if ((int) _zdrmInBraggResults.size() > _params.zdr_bias_bragg_min_npoints_valid) {
    _loadZdrResults(_zdrmInBraggResults,
                    _zdrmStatsBragg,
                    _params.zdr_bias_bragg_percentiles_n,
                    _params._zdr_bias_bragg_percentiles);
  }

  // write results to SPDB
  
  if (!_params.zdr_bias_write_results_to_spdb) {
    return;
  }

  RadxPath path(vol.getPathInUse());
  string xml;

  xml += RadxXml::writeStartTag("ZdrBias", 0);

  xml += RadxXml::writeString("file", 1, path.getFile());
  xml += RadxXml::writeBoolean("is_rhi", 1, vol.checkIsRhi());
  
  xml += RadxXml::writeInt("ZdrInIceNpts", 1, (int) _zdrStatsIce.count);
  xml += RadxXml::writeDouble("ZdrInIceMean", 1, _zdrStatsIce.mean);
  for (size_t ii = 0; ii < _zdrStatsIce.percentiles.size(); ii++) {
    double percent = _params._zdr_bias_ice_percentiles[ii];
    double val = _zdrStatsIce.percentiles[ii];
    char tag[1024];
    sprintf(tag, "ZdrInIcePerc%05.2f", percent);
    xml += RadxXml::writeDouble(tag, 1, val);
  }

  xml += RadxXml::writeInt("ZdrmInIceNpts", 1, (int) _zdrmStatsIce.count);
  xml += RadxXml::writeDouble("ZdrmInIceMean", 1, _zdrmStatsIce.mean);
  for (size_t ii = 0; ii < _zdrmStatsIce.percentiles.size(); ii++) {
    double percent = _params._zdr_bias_ice_percentiles[ii];
    double val = _zdrmStatsIce.percentiles[ii];
    char tag[1024];
    sprintf(tag, "ZdrmInIcePerc%05.2f", percent);
    xml += RadxXml::writeDouble(tag, 1, val);
  }

  xml += RadxXml::writeInt("ZdrInBraggNpts", 1, (int) _zdrStatsBragg.count);
  xml += RadxXml::writeDouble("ZdrInBraggMean", 1, _zdrStatsBragg.mean);
  for (size_t ii = 0; ii < _zdrStatsBragg.percentiles.size(); ii++) {
    double percent = _params._zdr_bias_bragg_percentiles[ii];
    double val = _zdrStatsBragg.percentiles[ii];
    char tag[1024];
    sprintf(tag, "ZdrInBraggPerc%05.2f", percent);
    xml += RadxXml::writeDouble(tag, 1, val);
  }

  xml += RadxXml::writeInt("ZdrmInBraggNpts", 1, (int) _zdrmStatsBragg.count);
  xml += RadxXml::writeDouble("ZdrmInBraggMean", 1, _zdrmStatsBragg.mean);
  for (size_t ii = 0; ii < _zdrmStatsBragg.percentiles.size(); ii++) {
    double percent = _params._zdr_bias_bragg_percentiles[ii];
    double val = _zdrmStatsBragg.percentiles[ii];
    char tag[1024];
    sprintf(tag, "ZdrmInBraggPerc%05.2f", percent);
    xml += RadxXml::writeDouble(tag, 1, val);
  }

  if (_params.read_site_temp_from_spdb) {
    xml += RadxXml::writeDouble("TempSite", 1, _siteTempC);
    RadxTime tempTime(_timeForSiteTemp);
    xml += RadxXml::writeString("TempTime", 1, tempTime.getW3cStr());
  }
  
  xml += RadxXml::writeEndTag("ZdrBias", 0);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Writing ZDR bias results to SPDB, url: "
         << _params.zdr_bias_spdb_output_url << endl;
  }

  DsSpdb spdb;
  time_t validTime = vol.getStartTimeSecs();
  spdb.addPutChunk(0, validTime, validTime, xml.size() + 1, xml.c_str());
  if (spdb.put(_params.zdr_bias_spdb_output_url,
               SPDB_XML_ID, SPDB_XML_LABEL)) {
    cerr << "ERROR - RadxPartRain::_computeZdrBias" << endl;
    cerr << spdb.getErrStr() << endl;
    return;
  }
  
  if (_params.debug) {
    cerr << "Wrote ZDR bias results to spdb, url: " 
         << _params.zdr_bias_spdb_output_url << endl;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=====================================" << endl;
    cerr << xml;
    cerr << "=====================================" << endl;
  }

}

//////////////////////////////////
// load stats for zdr results

void RadxPartRain::_loadZdrResults(vector<double> &results,
                                   ZdrStats &stats,
                                   int nPercentiles,
                                   double *percentiles)

{

  stats.clear();

  for (size_t ii = 0; ii < results.size(); ii++) {
    stats.sum += results[ii];
    stats.count++;
  }
  stats.mean = stats.sum / stats.count;

  // sort results vector
  
  sort(results.begin(), results.end());

  for (int ii = 0; ii < nPercentiles; ii++) {
    stats.percentiles.push_back(_computeZdrPerc(results, percentiles[ii]));
  }

}

/////////////////////////////////////////////////////////////
// compute a percentile value from the sorted bias data

double RadxPartRain::_computeZdrPerc(const vector<double> &zdrmResults,
                                     double percent)

{

  // get center of percentile location in the array

  double nVals = zdrmResults.size();
  int pos = (int) ((percent / 100.0) * nVals + 0.5);
  if (pos < 0) {
    pos = 0;
  } else if (pos > (int) zdrmResults.size() - 1) {
    pos = zdrmResults.size() - 1;
  }

  double percentileVal = zdrmResults[pos];

  // compute mean for 1% on either side

  int nMargin = (int) (nVals / 100.0 + 0.5);
  int istart = pos - nMargin;
  if (istart < 0) {
    istart = 0;
  }
  int iend = pos + nMargin;
  if (iend > (int) zdrmResults.size() - 1) {
    iend = zdrmResults.size() - 1;
  }

  double sum = 0.0;
  double count = 0.0;
  for (int ii = istart; ii <= iend; ii++) {
    sum += zdrmResults[ii];
    count++;
  }

  if (count > 0) {
    percentileVal = sum / count;
  }

  return percentileVal;

}

/////////////////////////////////////////////////////////////
// write zdr bias results to SPDB in XML

void RadxPartRain::_computeSelfConZBias(const RadxVol &vol)

{

  if (_selfConResults.size() < 1) {
    return;
  }

  // compute the mean bias
  
  double volBias = NAN;
  double sum = 0.0;
  for (size_t ii = 0; ii < _selfConResults.size(); ii++) {
    sum += _selfConResults[ii].dbzBias;
  }
  volBias = sum / (double) _selfConResults.size();

  if (_params.self_consistency_debug) {
    cerr << "Time, volBias: " << RadxTime::strm(vol.getStartTimeSecs()) << ", " << volBias << endl;
  }
  
  // prepare to write results to SPDB
  
  if (!_params.self_consistency_write_results_to_spdb) {
    return;
  }

  // add chunks

  DsSpdb spdb;
  RadxPath path(vol.getPathInUse());

  for (size_t ii = 0; ii < _selfConResults.size(); ii++) {

    const ComputeEngine::self_con_t &results = _selfConResults[ii];
    
    string xml;
    xml += RadxXml::writeStartTag("SelfConsistency", 0);

    xml += RadxXml::writeString("file", 1, path.getFile());
    xml += RadxXml::writeBoolean("is_rhi", 1, vol.checkIsRhi());
    xml += RadxXml::writeString("time", 1, results.rtime.asString(6));
    xml += RadxXml::writeInt("nResultsVol", 1, _selfConResults.size());
    xml += RadxXml::writeDouble("volBias", 1, volBias);
    xml += RadxXml::writeDouble("elevation", 1, results.elevation);
    xml += RadxXml::writeDouble("azimuth", 1, results.azimuth);
    xml += RadxXml::writeInt("runStart", 1, results.runStart);
    xml += RadxXml::writeInt("runEnd", 1, results.runEnd);
    xml += RadxXml::writeDouble("rangeStart", 1, results.rangeStart);
    xml += RadxXml::writeDouble("rangeEnd", 1, results.rangeEnd);
    xml += RadxXml::writeDouble("dbzCorrection", 1, results.dbzCorrection);
    xml += RadxXml::writeDouble("zdrCorrection", 1, results.zdrCorrection);
    xml += RadxXml::writeDouble("accumObs", 1, results.accumObs);
    xml += RadxXml::writeDouble("accumEst", 1, results.accumEst);
    xml += RadxXml::writeDouble("dbzBias", 1, results.dbzBias);
    xml += RadxXml::writeDouble("accumCorrelation", 1, results.accumCorrelation);

    if (_params.read_site_temp_from_spdb) {
      xml += RadxXml::writeDouble("TempSite", 1, _siteTempC);
      RadxTime tempTime(_timeForSiteTemp);
      xml += RadxXml::writeString("TempTime", 1, tempTime.getW3cStr());
    }
    
    xml += RadxXml::writeEndTag("SelfConsistency", 0);

    // set data types to force unique storage per run

    int dataType = (int) (results.rtime.getSubSec() * 1.0e9); // from nano secs
    int dataType2 = results.runStart;

    spdb.addPutChunk(dataType,
                     results.rtime.utime(),
                     results.rtime.utime(),
                     xml.size() + 1,
                     xml.c_str(),
                     dataType2);
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Writing self consistency results to SPDB, url: "
           << _params.self_consistency_spdb_output_url << endl;
      cerr << xml << endl;
    }
    
  } // ii

  // do the write

  if (spdb.put(_params.self_consistency_spdb_output_url,
               SPDB_XML_ID, SPDB_XML_LABEL)) {
    cerr << "ERROR - RadxPartRain::_computeSelfConZBias" << endl;
    cerr << spdb.getErrStr() << endl;
    return;
  }
  
  if (_params.debug) {
    cerr << "Wrote self consistency results to spdb, url: " 
         << _params.self_consistency_spdb_output_url << endl;
  }

}
  
