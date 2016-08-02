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
// RadxCov2Mom.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2011
//
///////////////////////////////////////////////////////////////
//
// RadxCov2Mom reads covariances in Radx-supported format files,
// computes the moments and writes out the results to
// Radx-supported format files.
//
///////////////////////////////////////////////////////////////

#include "RadxCov2Mom.hh"
#include "Moments.hh"
#include "Thread.hh"
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/TaArray.hh>
#include <didss/LdataInfo.hh>
#include <rapmath/trig.h>
#include <dsserver/DsLdataInfo.hh>
#include <Spdb/DsSpdb.hh>
#include <rapformats/WxObs.hh>
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

RadxCov2Mom::RadxCov2Mom(int argc, char **argv)
  
{

  OK = TRUE;

  // set programe name

  _progName = "RadxCov2Mom";
  
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

  // read in calibration files

  if (_readCalFiles()) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem reading calibration files" << endl;
    OK = FALSE;
  }

  // initialize moments object

  pthread_mutex_init(&_debugPrintMutex, NULL);
  
  if (_params.use_multiple_threads) {

    // set up compute thread pool
    
    for (int ii = 0; ii < _params.n_compute_threads; ii++) {
      
      ComputeThread *thread = new ComputeThread();
      thread->setApp(this);

      Moments *moments = new Moments(_params);
      if (!moments->OK) {
        delete moments;
        OK = FALSE;
        return;
      }
      _moments.push_back(moments);
      thread->setMoments(moments);

      pthread_t pth = 0;
      pthread_create(&pth, NULL, _computeMomentsInThread, thread);
      thread->setThreadId(pth);
      _availThreads.push_back(thread);

    }
    
  } else {

    // single threaded

    Moments *moments = new Moments(_params);
    if (!moments->OK) {
      delete moments;
      OK = FALSE;
      return;
    }
    _moments.push_back(moments);

  }

}

// destructor

RadxCov2Mom::~RadxCov2Mom()

{

  for (size_t ii = 0; ii < _moments.size(); ii++) {
    delete _moments[ii];
  }
  _moments.clear();

  // set thread pool to exit

  for (size_t ii = 0; ii < _activeThreads.size(); ii++) {
    _activeThreads[ii]->waitForWorkToComplete();
  }

  for (size_t ii = 0; ii < _availThreads.size(); ii++) {
    _availThreads[ii]->setExitFlag(true);
    _availThreads[ii]->signalWorkToStart();
  }
  
  for (size_t ii = 0; ii < _activeThreads.size(); ii++) {
    _activeThreads[ii]->setExitFlag(true);
    _activeThreads[ii]->signalWorkToStart();
  }
  
  for (size_t ii = 0; ii < _availThreads.size(); ii++) {
    _availThreads[ii]->waitForWorkToComplete();
  }

  for (size_t ii = 0; ii < _activeThreads.size(); ii++) {
    _activeThreads[ii]->waitForWorkToComplete();
  }

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

int RadxCov2Mom::Run()
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

int RadxCov2Mom::_runFilelist()
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

int RadxCov2Mom::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (_params.aggregate_sweep_files_on_read) {
    tlist.setReadAggregateSweeps(true);
  }
  if (tlist.compile()) {
    cerr << "ERROR - RadxCov2Mom::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - RadxCov2Mom::_runFilelist()" << endl;
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

int RadxCov2Mom::_runRealtime()
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

int RadxCov2Mom::_processFile(const string &filePath)
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
    cerr << "INFO - RadxCov2Mom::Run" << endl;
    cerr << "  Input path: " << filePath << endl;
  }
  
  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  RadxVol vol;
  if (inFile.readFromPath(filePath, vol)) {
    cerr << "ERROR - RadxCov2Mom::Run" << endl;
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
  _wavelengthM = vol.getWavelengthM();
  _radarHtKm = vol.getAltitudeKm();

  // set number of gates constant if requested

  if (_params.set_ngates_constant) {
    vol.setNGatesConstant();
  }

  // optimize transitions in surveillance mode

  if (_params.optimize_surveillance_transitions) {
    vol.optimizeSurveillanceTransitions(_params.optimized_transitions_max_elev_error);
  }

  // trim surveillance sweeps to 360 degrees if requested

  if (_params.trim_surveillance_sweeps_to_360deg) {
    vol.trimSurveillanceSweepsTo360Deg();
  }

  // remove transitions if requested
  
  if (_params.remove_rays_with_antenna_transitions) {
    vol.removeTransitionRays(_params.transition_nrays_margin);
  }

  // enough rays in each sweep

  if (_params.check_min_rays_in_sweep) {
    vol.removeSweepsWithTooFewRays(_params.min_rays_in_sweep);
  }

  // optionally deduce the sweep scan mode from the antenna angles

  if (_params.compute_sweep_modes_from_ray_angles) {
    vol.setSweepScanModeFromRayAngles();
  }

  // optinally override the sweep fixed angles

  if (_params.compute_sweep_fixed_angles_from_rays) {
    vol.computeSweepFixedAnglesFromRays();
  }
  
  // convert all data to floats

  vol.convertToFl32();

  // initialize the calibrations for use in this volume

  _initCals(vol);

  // initialize noise stats

  _initNoiseStats();

  // compute the moments
  
  if (_computeMoments(vol)) {
    cerr << "ERROR - RadxCov2Mom::Run" << endl;
    cerr << "  Cannot compute moments" << endl;
    return -1;
  }

  // set encoding for output

  _encodeFieldsForOutput(vol);

  // merge in extra fields

  if (_params.add_merged_fields_to_output) {
    _addMergedFields(vol);
  }

  // remove censoring flag field if not needed in output file
  
  if (_params.censoring_mode != Params::CENSORING_NONE && 
      !_params.write_censor_flag_to_output) {
    vol.removeField(Moments::censorFlagFieldName);
  }

  // compute noise stats

  _computeNoiseStats();

  // write out file

  GenericRadxFile outFile;
  _setupWrite(outFile);
  if (_writeVol(vol)) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// encode fields for output

void RadxCov2Mom::_encodeFieldsForOutput(RadxVol &vol)
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

void RadxCov2Mom::_setupRead(RadxFile &file)
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

  if (_params.ignore_idle_scan_mode_on_read) {
    file.setReadIgnoreIdleMode(true);
  } else {
    file.setReadIgnoreIdleMode(false);
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }
  
}

//////////////////////////////////////////////////
// set up write

void RadxCov2Mom::_setupWrite(RadxFile &file)
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

  if (_params.output_force_ngates_vary) {
    file.setWriteForceNgatesVary(true);
  }

}

//////////////////////////////////////////////////
// write out the volume

int RadxCov2Mom::_writeVol(RadxVol &vol)
{

  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);

  // add to status XML

  string statusXml = vol.getStatusXml();
  statusXml += "\n";
  string procXml = _loadProcXml();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Adding to status XML" << endl;
    cerr << procXml<< endl;
  }
  statusXml += procXml;
  vol.setStatusXml(statusXml);
  
  if (_params.output_filename_mode == Params::SPECIFY_FILE_NAME) {

    string outPath = _params.output_dir;
    outPath += PATH_DELIM;
    outPath += _params.output_filename;

    // write to path
  
    if (outFile.writeToPath(vol, outPath)) {
      cerr << "ERROR - RadxCov2Mom::_writeVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
      
  } else {

    // write to dir
  
    if (outFile.writeToDir(vol, _params.output_dir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - RadxCov2Mom::_writeVol" << endl;
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
      cerr << "WARNING - RadxCov2Mom::_writeVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  // write status and cal to SPDB in XML

  if (_params.write_status_to_spdb) {
    
    string spdbXml;
    for (size_t ii = 0; ii < _calibs.size(); ii++) {
      if (_calibs[ii].used) {
        string calXml;
        _calibs[ii].radxCal.convert2Xml(calXml);
        spdbXml += calXml;
      }
    }
    spdbXml += statusXml;

    if (_writeStatusXmlToSpdb(vol, spdbXml)) {
      cerr << "WARNING - RadxCov2Mom::_writeVol" << endl;
      cerr << "  Cannot write status XML to SPDB" << endl;
    }

  }

  return 0;

}

/////////////////////////////////////////////////////
// create xml string to record processing details

string RadxCov2Mom::_loadProcXml()
{

  string xml;
  
  xml += RadxXml::writeStartTag("RadxCov2Mom", 0);
  
  // ZDR temperature correction
  
  if (_params.use_temperature_based_zdr_correction) {
    xml += RadxXml::writeStartTag("TempBasedZdrCorrection", 1);
    xml += RadxXml::writeDouble("zdrTempCorrDb", 2, _zdrTempCorrDb);
    xml += RadxXml::writeDouble("siteTempC", 2, _siteTempC);
    RadxTime tempTime(_timeForSiteTemp);
    xml += RadxXml::writeString("timeForSiteTemp", 2, tempTime.getW3cStr());
    xml += RadxXml::writeDouble("correctionSlope", 2,
                                _params.temp_based_zdr_correction_slope);
    xml += RadxXml::writeDouble("correctionIntercept", 2,
                                _params.temp_based_zdr_correction_intercept);
    xml += RadxXml::writeEndTag("TempBasedZdrCorrection", 1);
  } else if (_params.override_cal_zdr_correction) {
    xml += RadxXml::writeDouble("zdrCorrectionDb", 1, _params.zdr_correction_db);
  }

  // other corrections

  if (_params.override_cal_dbz_correction) {
    xml += RadxXml::writeDouble("dbzCorrection", 1, _params.dbz_correction);
  }

  if (_params.override_cal_system_phidp) {
    xml += RadxXml::writeDouble("systemPhidpDeg", 1, _params.system_phidp_deg);
  }

  if (_params.override_cal_ldr_correction) {
    xml += RadxXml::writeDouble("ldrCorrectionDbH", 1, _params.ldr_correction_db_h);
    xml += RadxXml::writeDouble("ldrCorrectionDbV", 1, _params.ldr_correction_db_v);
  }
    
  if (_measXmitPowerDbmH >= _params.min_valid_measured_xmit_power_dbm &&
      _measXmitPowerDbmH <= _params.max_valid_measured_xmit_power_dbm) {
    xml += RadxXml::writeDouble("measXmitPowerDbmH", 1, _measXmitPowerDbmH);
  }

  if (_measXmitPowerDbmV >= _params.min_valid_measured_xmit_power_dbm &&
      _measXmitPowerDbmV <= _params.max_valid_measured_xmit_power_dbm) {
    xml += RadxXml::writeDouble("measXmitPowerDbmV", 1, _measXmitPowerDbmV);
  }

  xml += RadxXml::writeDouble("measXmitPowerCorrDb", 1,
                              _params.measured_xmit_power_correction_db);
  
  if (_nNoiseDbmHc > 0) {
    xml += RadxXml::writeDouble("meanNoiseDbmHc", 1, _meanNoiseDbmHc);
  } else {
    xml += RadxXml::writeString("meanNoiseDbmHc", 1, "nan");
  }
  if (_nNoiseDbmVc > 0) {
    xml += RadxXml::writeDouble("meanNoiseDbmVc", 1, _meanNoiseDbmVc);
  } else {
    xml += RadxXml::writeString("meanNoiseDbmVc", 1, "nan");
  }
  if (_nNoiseDbmHx > 0) {
    xml += RadxXml::writeDouble("meanNoiseDbmHx", 1, _meanNoiseDbmHx);
  } else {
    xml += RadxXml::writeString("meanNoiseDbmHx", 1, "nan");
  }
  if (_nNoiseDbmVx > 0) {
    xml += RadxXml::writeDouble("meanNoiseDbmVx", 1, _meanNoiseDbmVx);
  } else {
    xml += RadxXml::writeString("meanNoiseDbmVx", 1, "nan");
  }
  
  xml += RadxXml::writeEndTag("RadxCov2Mom", 0);
  
  return xml;

}

/////////////////////////////////////////////////////
// read in the calibration files, set up cal objects

int RadxCov2Mom::_readCalFiles()
{

  // check that we have at least one calibration file

  if (_params.cal_files_n < 1) {
    cerr << "ERROR - RadxCov2Mom::_readCalFiles()" << endl;
    cerr << "  No calibration files specified" << endl;
    cerr << "  You must specify at least 1 calibration file" << endl;
    cerr << "  See parameter 'cal_files'" << endl;
    return -1;
  }

  for (int icalib = 0; icalib < _params.cal_files_n; icalib++) {
    
    double pulseWidthUs = _params._cal_files[icalib].pulse_width_us;
    string calPath = _params._cal_files[icalib].cal_file_path;

    IwrfCalib iwrfCal;
    string errStr;
    if (iwrfCal.readFromXmlFile(calPath, errStr)) {
      cerr << "ERROR - RadxCov2Mom::_readCalFiles" << endl;
      cerr << "  Cannot decode cal file: " << calPath << endl;
      cerr << errStr;
      return -1;
    }
    
    Calib calib;
    calib.pulseWidthUs = pulseWidthUs;
    calib.referenceCal = iwrfCal;
    calib.workingCal = iwrfCal;
    iwrfCal.copyToRadxRcalib(calib.radxCal);
    calib.used = false;

    _calibs.push_back(calib);
    
  } // icalib

  return 0;

}

/////////////////////////////////////////////////////
// initialize cals for use

void RadxCov2Mom::_initCals(const RadxVol &vol)
{

  // determine measured xmit power

  _loadMeasuredXmitPower(vol);
  
  // check for temp based zdr correction
  
  bool useTempBasedZdrCorr = false;
  if (_params.use_temperature_based_zdr_correction) {
    if (_loadZdrTempCorrection(vol.getStartTimeSecs()) == 0) {
      useTempBasedZdrCorr = true;
    }
  }

  // loop through cals

  for (int ii = 0; ii < _params.cal_files_n; ii++) {

    // get objects for this pulse width

    Calib &calib = _calibs[ii];
    IwrfCalib workingCal = calib.referenceCal;
    
    // set system phidp

    if (_params.override_cal_system_phidp) {
      workingCal.setSystemPhidpDeg(_params.system_phidp_deg);
    }

    // set dbz correction
    
    if (_params.override_cal_dbz_correction) {
      workingCal.setDbzCorrection(_params.dbz_correction);
    }

    // set zdr correction

    if (_params.override_cal_zdr_correction) {
      workingCal.setZdrCorrectionDb(_params.zdr_correction_db);
    }

    if (useTempBasedZdrCorr) {
      // override zdr correction with temp-based value
      workingCal.setZdrCorrectionDb(_zdrTempCorrDb);
    }

    // set ldr correction

    if (_params.override_cal_ldr_correction) {
      workingCal.setLdrCorrectionDbH(_params.ldr_correction_db_h);
      workingCal.setLdrCorrectionDbV(_params.ldr_correction_db_v);
    }

    // adjust for transmit power?

    if (_params.adjust_calibration_for_measured_xmit_power) {
      workingCal.adjustRadarConst(workingCal.getPulseWidthUs(),
                                  _measXmitPowerDbmH,
                                  _measXmitPowerDbmV);
    }
    
    // save working cal and radx cal

    calib.workingCal = workingCal;
    workingCal.copyToRadxRcalib(calib.radxCal);
    
    // initialize used flag to false

    calib.used = false;

    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "==========================================" << endl;
      cerr << "reference cal: " << endl;
      calib.referenceCal.print(cerr);
      cerr << "==========================================" << endl;
      cerr << "working cal: " << endl;
      calib.workingCal.print(cerr);
      cerr << "==========================================" << endl;
      cerr << "radx cal: " << endl;
      calib.radxCal.print(cerr);
      cerr << "==========================================" << endl;
    }
    
  } // ii

}

/////////////////////////////////////////////////////
// load up zdr correction based on temperature
// returns 0 on success, -1 on failure

int RadxCov2Mom::_loadZdrTempCorrection(time_t volTime)
{

  // init

  _zdrTempCorrDb = 0.0;
  _siteTempC = NAN;
  _timeForSiteTemp = 0;

  // get surface data from SPDB
  
  DsSpdb spdb;
  si32 dataType = 0;
  if (strlen(_params.site_temp_station_name) > 0) {
    dataType =  Spdb::hash4CharsToInt32(_params.site_temp_station_name);
  }
  time_t searchTime = volTime;
  
  if (spdb.getClosest(_params.site_temp_spdb_url,
                      searchTime,
                      _params.site_temp_search_margin_secs,
                      dataType)) {
    cerr << "WARNING - RadxCov2Mom::_getZdrTempCorrection()" << endl;
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
    cerr << "WARNING - RadxCov2Mom::_getZdrTempCorrection()" << endl;
    cerr << "  No suitable temp data from URL: " << _params.site_temp_spdb_url << endl;
    cerr << "  Search time: " << RadxTime::strm(searchTime) << endl;
    cerr << "  Search margin (secs): " << _params.site_temp_search_margin_secs << endl;
    return -1;
  }

  const Spdb::chunk_t &chunk = chunks[0];
  WxObs obs;
  if (obs.disassemble(chunk.data, chunk.len)) {
    cerr << "WARNING - RadxCov2Mom::_getZdrTempCorrection()" << endl;
    cerr << "  SPDB data is of incorrect type, prodLabel: "
         << spdb.getProdLabel() << endl;
    cerr << "  Should be station data type" << endl;
    cerr << "  URL: " << _params.site_temp_spdb_url << endl;
    return -1;
  }

  _siteTempC = obs.getTempC();
  _timeForSiteTemp = obs.getObservationTime();
  _zdrTempCorrDb = (_params.temp_based_zdr_correction_slope * _siteTempC +
                    _params.temp_based_zdr_correction_intercept);

  if (_params.debug) {
    cerr << "Setting temp-based zdr correction" << endl;
    cerr << "  siteTempC: " << _siteTempC << endl;
    cerr << "  correctionSlope: " << _params.temp_based_zdr_correction_slope << endl;
    cerr << "  correctionIntercept: " << _params.temp_based_zdr_correction_intercept << endl;
    cerr << "  timeForSiteTemp: " << RadxTime::strm(_timeForSiteTemp) << endl;
    cerr << "  zdrTempCorrDb: " << _zdrTempCorrDb << endl;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// get the best calibration for the specified pulse width

const IwrfCalib &RadxCov2Mom::_getBestCal(double pulseWidthUs)

{

  double minDiff = 1.0e99;

  size_t iiBest = 0;
  for (size_t ii = 0; ii < _calibs.size(); ii++) {
    double diff = fabs(pulseWidthUs - _calibs[ii].pulseWidthUs);
    if (diff < minDiff) {
      minDiff = diff;
      iiBest = ii;
    }
  }
  
  _calibs[iiBest].used = true;
  return _calibs[iiBest].workingCal;

}

//////////////////////////////////////////////////
// compute the moments for all rays in volume

int RadxCov2Mom::_computeMoments(RadxVol &vol)
{

  // compute the moments
  
  const vector<RadxRay *> &covRays = vol.getRays();
  vector <RadxRay *> momRays;
  
  if (_params.use_multiple_threads) {
    if (_computeMomentsMultiThreaded(vol, covRays, momRays)) {
      return -1;
    }
  } else {
    if (_computeMomentsSingleThreaded(vol, covRays, momRays)) {
      return -1;
    }
  }

  // add echo fields to moments rays if appropriate
  
  if (_params.add_echo_fields_to_output) {
    _addEchoFields(covRays, momRays);
  }
    
  // clear the covariance rays
  
  vol.clearRays();
  
  // add the moments rays to the volume
  
  for (size_t iray = 0; iray < momRays.size(); iray++) {
    vol.addRay(momRays[iray]);
  }
  
  // load up calibrations for output vol

  vol.clearRcalibs();
  for (size_t ii = 0; ii < _calibs.size(); ii++) {
    if (_calibs[ii].used) {
      RadxRcalib *rcal = new RadxRcalib(_calibs[ii].radxCal);
      vol.addCalib(rcal);
    }
  }

  // set the max range for all rays on the volume

  if (_params.set_max_range) {
    vol.setMaxRangeKm(_params.max_range_km);
  }

  vol.loadVolumeInfoFromRays();
  vol.loadSweepInfoFromRays();
  vol.setPackingFromRays();

  return 0;

}

//////////////////////////////////////////////////
// compute the moments for all rays in volume

int RadxCov2Mom::_computeMomentsSingleThreaded(RadxVol &vol,
                                               const vector<RadxRay *> &covRays,
                                               vector <RadxRay *> &momRays)
{

  // loop through the rays, computing the moments

  for (size_t iray = 0; iray < covRays.size(); iray++) {

    // get covariance ray

    const RadxRay *covRay = covRays[iray];

    // // create moments ray
    
    // RadxRay *momRay = new RadxRay;
    // momRay->copyMetaData(*covRay);
    
    // get best calibration for this ray

    const IwrfCalib &calib = _getBestCal(covRay->getPulseWidthUsec());
    
    // compute moments
    
    RadxRay *momRay = 
      _moments[0]->compute(covRay, calib,
                           _measXmitPowerDbmH, _measXmitPowerDbmV,
                           _wavelengthM, _radarHtKm);
    if (momRay == NULL) {
      cerr << "ERROR - _computeMoments" << endl;
      return -1;
    }

    // add to vector

    momRays.push_back(momRay);

    // sum up for noise

    _addToNoiseStats(*_moments[0]);

  } // iray

  return 0;
  
}

//////////////////////////////////////////////////
// compute the moments for all rays in volume

int RadxCov2Mom::_computeMomentsMultiThreaded(RadxVol &vol,
                                              const vector<RadxRay *> &covRays,
                                              vector <RadxRay *> &momRays)
{

  // loop through the rays, computing the moments

  for (size_t iray = 0; iray < covRays.size(); iray++) {

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

      // sum up for noise
      
      _addToNoiseStats(*thread->getMoments());

      // store ray
      
      RadxRay *momRay = thread->getMomRay();
      if (momRay == NULL) {
        cerr << "ERROR - _computeMomentsMultiThreaded" << endl;
        _availThreads.push_back(thread);
        return -1;
      } else {
        // good return, add to results
        momRays.push_back(momRay);
      }
      
    }
    
    // get new covariance ray
    
    const RadxRay *covRay = covRays[iray];

    // get best calibration for this ray

    const IwrfCalib &calib = _getBestCal(covRay->getPulseWidthUsec());
    
    // set thread going to compute moments
    
    thread->setCovRay(covRay);
    thread->setCalib(&calib);
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
    
    RadxRay *momRay = thread->getMomRay();
    if (momRay == NULL) {
      cerr << "ERROR - _computeMomentsMultiThreaded" << endl;
      return -1;
    } else {
      // good return, add to results
      momRays.push_back(momRay);
    }

    // sum up for noise

    _addToNoiseStats(*thread->getMoments());

  }
  
  return 0;

}

///////////////////////////////////////////////////////////
// Thread function to compute moments

void *RadxCov2Mom::_computeMomentsInThread(void *thread_data)
  
{
  
  // get thread data from args

  ComputeThread *compThread = (ComputeThread *) thread_data;
  RadxCov2Mom *app = compThread->getApp();
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
      cerr << "======>> starting beam compute" << endl;
      pthread_mutex_unlock(debugPrintMutex);
    }

    Moments *moments = compThread->getMoments();
    const RadxRay *covRay = compThread->getCovRay();
    const IwrfCalib *calib = compThread->getCalib();
    RadxRay *momRay = 
      moments->compute(covRay, *calib,
                       app->_measXmitPowerDbmH, app->_measXmitPowerDbmV,
                       app->_wavelengthM, app->_radarHtKm);
    compThread->setMomRay(momRay);
    
    if (app->getParams().debug >= Params::DEBUG_EXTRA) {
      pthread_mutex_t *debugPrintMutex = app->getDebugPrintMutex();
      pthread_mutex_lock(debugPrintMutex);
      cerr << "======>> done with moments compute" << endl;
      pthread_mutex_unlock(debugPrintMutex);
    }

    // unlock done mutex
    
    compThread->signalParentWorkIsComplete();
    
  } // while

  return NULL;

}

///////////////////////////////////////////////////////////////////
// add echo fields from covariance data to the output moments data

int RadxCov2Mom::_addEchoFields(const vector<RadxRay *> &covRays,
                                vector <RadxRay *> &momRays)

{

  if (covRays.size() != momRays.size()) {
    cerr << "ERROR - RadxCov2Mom::_addEchoFields" << endl;
    cerr << "  Number of moments rays incorrect: " << momRays.size() << endl;
    cerr << "  Should be: " << covRays.size() << endl;
    return -1;
  }

  for (size_t iray = 0; iray < covRays.size(); iray++) {

    const RadxRay *covRay = covRays[iray];
    RadxRay *momRay = momRays[iray];

    // get censor flag as required
    
    const RadxField *cFld = NULL;
    if (_params.censoring_mode != Params::CENSORING_NONE) {
      cFld = momRay->getField(Moments::censorFlagFieldName);
    }
    
    for (int ii = 0; ii < _params.echo_fields_n; ii++) {
      
      const Params::echo_field_t &efld = _params._echo_fields[ii];
      const RadxField *inFld = covRay->getField(efld.input_field_name);
      if (inFld == NULL) {
        continue; // field not found
      }

      // copy input field to output field

      RadxField *outFld = new RadxField(*inFld);

      // censor if appropriate
      
      if (cFld != NULL) {
        size_t nGates = covRay->getNGates();
        if (momRay->getNGates() < nGates) {
          nGates = momRay->getNGates();
        }
        const Radx::si08 *cFlag = cFld->getDataSi08();
        Radx::fl32 *oData = (Radx::fl32 *) outFld->getDataFl32();
        Radx::fl32 oMiss = outFld->getMissingFl32();
        for (size_t igate = 0; igate < nGates; igate++) {
          if (cFlag[igate] != 0) {
            oData[igate] = oMiss;
          }
        }
      }

      // convert type

      Radx::DataType_t outType = Radx::SI16;
      if (efld.encoding == Params::OUTPUT_ENCODING_INT08) {
        outType = Radx::SI08;
      } else if (efld.encoding == Params::OUTPUT_ENCODING_INT32) {
        outType = Radx::SI32;
      } else if (efld.encoding == Params::OUTPUT_ENCODING_FL32) {
        outType = Radx::FL32;
      }
      if (efld.output_scaling == Params::OUTPUT_SCALING_SPECIFIED) {
        outFld->convert(outType,
                        efld.output_scale,
                        efld.output_offset,
                        efld.output_field_name,
                        efld.output_units,
                        efld.standard_name,
                        efld.long_name);
      } else {
        outFld->convert(outType,
                        efld.output_field_name,
                        efld.output_units,
                        efld.standard_name,
                        efld.long_name);
      }

      // add to ray

      momRay->addField(outFld);

    } // ii

  } // iray

  return 0;

}

///////////////////////////////////////////////////////////////////
// load the measured xmit power for each channel as appropriate

void RadxCov2Mom::_loadMeasuredXmitPower(const RadxVol &vol)

{

  _measXmitPowerDbmH = NAN;
  _measXmitPowerDbmV = NAN;

  double sumH = 0.0;
  double sumV = 0.0;
  int countH = 0;
  int countV = 0;

  for (size_t iray = 0; iray < vol.getNRays(); iray++) {

    const RadxRay *ray = vol.getRays()[iray];

    double powerDbmH = ray->getMeasXmitPowerDbmH();
    double powerDbmV = ray->getMeasXmitPowerDbmV();

    if (powerDbmH >= _params.min_valid_measured_xmit_power_dbm &&
        powerDbmH <= _params.max_valid_measured_xmit_power_dbm) {
      sumH += powerDbmH;
      countH++;
    }

    if (powerDbmV >= _params.min_valid_measured_xmit_power_dbm &&
        powerDbmV <= _params.max_valid_measured_xmit_power_dbm) {
      sumV += powerDbmV;
      countV++;
    }

  } // iray

  if (countH > 0) {
    _measXmitPowerDbmH = sumH / countH + _params.measured_xmit_power_correction_db;
  }
  if (countV > 0) {
    _measXmitPowerDbmV = sumV / countV + _params.measured_xmit_power_correction_db;
  }

  if (_params.swap_measured_xmit_power_channels) {
    double tmp = _measXmitPowerDbmH;
    _measXmitPowerDbmH = _measXmitPowerDbmV;
    _measXmitPowerDbmV = tmp;
  }

  if (_params.debug) {
    cerr << "====>> Computing mean measured transmit power from all rays in volume" << endl;
    cerr << "       measXmitPowerDbmH: " << _measXmitPowerDbmH << endl;
    cerr << "       measXmitPowerDbmV: " << _measXmitPowerDbmV << endl;
  }

}
//////////////////////////////////////////////////
// Add merged fields to output vol

int RadxCov2Mom::_addMergedFields(RadxVol &primaryVol)
{

  // Search for file from which to merge data
  
  time_t searchTime = primaryVol.getStartTimeSecs();
  RadxTimeList tlist;
  tlist.setDir(_params.merge_input_dir);
  tlist.setModeClosest(searchTime, _params.merge_file_time_tolerance_sec);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    tlist.printRequest(cerr);
  }
  
  if (tlist.compile()) {
    cerr << "ERROR - RadxCov2Mom::_addMergedFields()" << endl;
    cerr << "  Cannot compile merge file time list" << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }
  const vector<string> &pathList = tlist.getPathList();
  if (pathList.size() < 1) {
    cerr << "ERROR - RadxCov2Mom::_addMergedFields()" << endl;
    cerr << "  No suitable merge file found" << endl;
    cerr << "  Primary file: " << primaryVol.getPathInUse() << endl;
    return -1;
  }
  
  // read in merge file, using first path in list
  
  string mergePath = pathList[0];
  if (_params.debug) {
    cerr << "Found merge file: " << mergePath << endl;
  }
  GenericRadxFile mergeFile;
  _setupMergeRead(mergeFile);
  RadxVol mergeVol;
  if (mergeFile.readFromPath(mergePath, mergeVol)) {
    cerr << "ERROR - RadxCov2Mom::_addMergedFields()" << endl;
    cerr << "  Cannot read in merge file: " << mergePath << endl;
    cerr << mergeFile.getErrStr() << endl;
    return -1;
  }
  
  // merge the primary and merge volumes, using the primary
  // volume to hold the merged data
  
  if (_mergeVol(primaryVol, mergeVol)) {
    cerr << "ERROR - RadxCov2Mom::_addMergedFields()" << endl;
    cerr << "  Merge failed" << endl;
    cerr << "  Primary file: " << primaryVol.getPathInUse() << endl;
    cerr << "  Merge file: " << mergePath << endl;
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// set up read for merge data

void RadxCov2Mom::_setupMergeRead(RadxFile &mergeFile)
  
{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mergeFile.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    mergeFile.setVerbose(true);
  }
  
  for (int ii = 0; ii < _params.merge_fields_n; ii++) {
    mergeFile.addReadField(_params._merge_fields[ii].input_field_name);
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===== SETTING UP READ FOR MERGE FILES =====" << endl;
    mergeFile.printReadRequest(cerr);
    cerr << "===============================================" << endl;
  }

}

//////////////////////////////////////////////////
// Perform the merge
// Returns 0 on success, -1 on failure

int RadxCov2Mom::_mergeVol(const RadxVol &primaryVol, RadxVol &secondaryVol)
  
{
  if (primaryVol.checkIsRhi()) {
    return _mergeVolRhi(primaryVol, secondaryVol);
  } else {
    return _mergeVolPpi(primaryVol, secondaryVol);
  }
}

//////////////////////////////////////////////////
// Perform the merge on PPI-type volume
// Returns 0 on success, -1 on failure

int RadxCov2Mom::_mergeVolPpi(const RadxVol &primaryVol, RadxVol &secondaryVol)

{

  // loop through all rays in primary vol

  const vector<RadxRay *> &pRays = primaryVol.getRays();

  for (size_t ii = 0; ii < pRays.size(); ii++) {

    RadxRay *pRay = pRays[ii];
    RadxTime pTime(pRay->getTimeSecs(), pRay->getNanoSecs() / 1.0e9);
    double pAz = pRay->getAzimuthDeg();   
    double pEl = pRay->getElevationDeg();
    if (_params.merge_ray_check_fixed_angle) {
      pEl = pRay->getFixedAngleDeg();
    }

    // find matching ray in secondary volume

    const vector<RadxRay *> &sRays = secondaryVol.getRays();
    for (size_t jj = 0; jj < sRays.size(); jj++) {
      
      RadxRay *sRay = sRays[jj];
      RadxTime sTime(sRay->getTimeSecs(), sRay->getNanoSecs() / 1.0e9);
      double sAz = sRay->getAzimuthDeg();   
      double sEl = sRay->getElevationDeg();
      if (_params.merge_ray_check_fixed_angle) {
        sEl = sRay->getFixedAngleDeg();
      }
      double diffTime = fabs(pTime - sTime);
      double dAz = pAz - sAz;
      if (dAz < -180) {
        dAz += 360.0;
      } else if (dAz > 180) {
        dAz -= 360.0;
      }
      double diffAz = fabs(dAz);
      double diffEl = fabs(pEl - sEl);
      
      if (_params.debug >= Params::DEBUG_EXTRA) {
        if (diffAz < 2.5 && diffEl < 2.5) {
          cerr << "==>> mergeVolPpi: diffAz, diffEl, aAz, sEl, pAz, pEl: "
               << diffAz << ", " 
               << diffEl << ", "
               << sAz << ", " << sEl << ", " 
               << pAz << ", " << pEl << endl;
        }
      }

      if (diffAz <= _params.merge_ray_azimuth_tolerance_deg &&
          diffEl <= _params.merge_ray_elevation_tolerance_deg) {

        // angles match, merge the rays
        _mergeRay(*pRay, *sRay);

        if (_params.debug >= Params::DEBUG_EXTRA) {
          cerr << "Matched ray - time az el az2 el2 dEl dAz dTime: "
               << pTime.asString(6) << ", "
               << pAz << ", " << pEl << ", "
               << sAz << ", " << sEl << ", "
               << diffEl << ", "
               << diffAz << ", "
               << diffTime << endl;
        }
        break;
      }
      
    } // jj

  } // ii
  
  return 0;

}

//////////////////////////////////////////////////
// Perform the merge on RHI-type volume
// Returns 0 on success, -1 on failure

int RadxCov2Mom::_mergeVolRhi(const RadxVol &primaryVol, RadxVol &secondaryVol)

{

  // loop through all rays in primary vol

  const vector<RadxRay *> &pRays = primaryVol.getRays();

  for (size_t ii = 0; ii < pRays.size(); ii++) {

    RadxRay *pRay = pRays[ii];
    RadxTime pTime(pRay->getTimeSecs(), pRay->getNanoSecs() / 1.0e9);
    double pAz = pRay->getAzimuthDeg();   
    if (_params.merge_ray_check_fixed_angle) {
     pAz = pRay->getFixedAngleDeg();   
    }
    double pEl = pRay->getElevationDeg();
    
    // find matching ray in secondary volume

    const vector<RadxRay *> &sRays = secondaryVol.getRays();
    for (size_t jj = 0; jj < sRays.size(); jj++) {
      
      RadxRay *sRay = sRays[jj];
      RadxTime sTime(sRay->getTimeSecs(), sRay->getNanoSecs() / 1.0e9);
      double sAz = sRay->getAzimuthDeg();   
      if (_params.merge_ray_check_fixed_angle) {
        sAz = sRay->getFixedAngleDeg();
      }
      double sEl = sRay->getElevationDeg();
      double diffTime = fabs(pTime - sTime);
      double dAz = pAz - sAz;
      if (dAz < -180) {
        dAz += 360.0;
      } else if (dAz > 180) {
        dAz -= 360.0;
      }
      double diffAz = fabs(dAz);
      double diffEl = fabs(pEl - sEl);

      if (_params.debug >= Params::DEBUG_EXTRA) {
        if (diffAz < 2.5 && diffEl < 2.5) {
          cerr << "==>> mergeVolRhi: diffAz, diffEl, aAz, sEl, pAz, pEl: "
               << diffAz << ", " 
               << diffEl << ", "
               << sAz << ", " << sEl << ", " 
               << pAz << ", " << pEl << endl;
        }
      }

      if (diffAz <= _params.merge_ray_azimuth_tolerance_deg &&
          diffEl <= _params.merge_ray_elevation_tolerance_deg) {

        // angles match, merge the rays
        _mergeRay(*pRay, *sRay);

        if (_params.debug >= Params::DEBUG_EXTRA) {
          cerr << "Matched ray - time az el az2 el2 dEl dAz dTime: "
               << pTime.asString(6) << ", "
               << pAz << ", " << pEl << ", "
               << sAz << ", " << sEl << ", "
               << diffEl << ", "
               << diffAz << ", "
               << diffTime << endl;
        }
        break;
      }
      
    } // jj

  } // ii
  
  return 0;

}

//////////////////////////////////////////////////////////////
// merge primary and seconday rays
//
// Returns 0 on success, -1 on failure

void RadxCov2Mom::_mergeRay(RadxRay &primaryRay,
                            const RadxRay &secondaryRay)
  
{

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
    string longName = sField->getLongName();
    string standardName = sField->getStandardName();
    string units = sField->getUnits();
    Params::output_encoding_t outputEncoding = Params::OUTPUT_ENCODING_INT16;
    for (int ii = 0; ii < _params.merge_fields_n; ii++) {
      string inputName = _params._merge_fields[ii].input_field_name;
      if (inputName == outputName) {
        if (strlen(_params._merge_fields[ii].output_field_name) > 0) {
          outputName = _params._merge_fields[ii].output_field_name;
        }
        if (strlen(_params._merge_fields[ii].long_name) > 0) {
          longName = _params._merge_fields[ii].long_name;
        }
        if (strlen(_params._merge_fields[ii].standard_name) > 0) {
          standardName = _params._merge_fields[ii].standard_name;
        }
        if (strlen(_params._merge_fields[ii].output_units) > 0) {
          units = _params._merge_fields[ii].output_units;
        }
        outputEncoding = _params._merge_fields[ii].encoding;
        break;
      }
    }

    // make a copy of the field

    RadxField *sCopy = new RadxField(*sField);

    // set name and units
    
    sCopy->setName(outputName);
    sCopy->setUnits(units);
    sCopy->setLongName(longName);
    sCopy->setStandardName(standardName);

    // ensure geometry is correct, remap if needed
    
    if (geomDiffers) {
      sCopy->remapRayGeom(remap, true);
    }
    sCopy->setNGates(nGatesPrimary);
      
    // convert type

    switch (outputEncoding) {
      case Params::OUTPUT_ENCODING_FL32:
        sCopy->convertToFl32();
        break;
      case Params::OUTPUT_ENCODING_INT32:
        sCopy->convertToSi32();
        break;
      case Params::OUTPUT_ENCODING_INT08:
        sCopy->convertToSi08();
        break;
      case Params::OUTPUT_ENCODING_INT16:
      default:
        sCopy->convertToSi16();
        break;
    } // switch

    // add to ray

    primaryRay.addField(sCopy);

  } // ifield
  
  primaryRay.loadFieldNameMap();

}

//////////////////////////////////////////////////
// Initialize for noise stats

void RadxCov2Mom::_initNoiseStats()

{

  _nNoiseDbmHc = 0;
  _nNoiseDbmVc = 0;
  _nNoiseDbmHx = 0;
  _nNoiseDbmVx = 0;

  _sumNoiseDbmHc = 0.0;
  _sumNoiseDbmVc = 0.0;
  _sumNoiseDbmHx = 0.0;
  _sumNoiseDbmVx = 0.0;

  _meanNoiseDbmHc = NAN;
  _meanNoiseDbmVc = NAN;
  _meanNoiseDbmHx = NAN;
  _meanNoiseDbmVx = NAN;

}

//////////////////////////////////////////////////
// Add to noise stats for ray

void RadxCov2Mom::_addToNoiseStats(Moments &mom)

{

  if (!_params.compute_vol_noise_stats) {
    return;
  }

  double elev = mom.getElevationDeg();
  if (elev < _params.vol_noise_stats_min_elev_deg ||
      elev > _params.vol_noise_stats_max_elev_deg) {
    return;
  }

  double noiseDbmHc = mom.getMedianNoiseDbmHc();
  if (noiseDbmHc > -9000) {
    _nNoiseDbmHc++;
    _sumNoiseDbmHc += noiseDbmHc;
  }

  double noiseDbmVc = mom.getMedianNoiseDbmVc();
  if (noiseDbmVc > -9000) {
    _nNoiseDbmVc++;
    _sumNoiseDbmVc += noiseDbmVc;
  }

  double noiseDbmHx = mom.getMedianNoiseDbmHx();
  if (noiseDbmHx > -9000) {
    _nNoiseDbmHx++;
    _sumNoiseDbmHx += noiseDbmHx;
  }

  double noiseDbmVx = mom.getMedianNoiseDbmVx();
  if (noiseDbmVx > -9000) {
    _nNoiseDbmVx++;
    _sumNoiseDbmVx += noiseDbmVx;
  }

}

//////////////////////////////////////////////////
// Compute noise stats

void RadxCov2Mom::_computeNoiseStats()

{
  
  if (_nNoiseDbmHc > 0) {
    _meanNoiseDbmHc = _sumNoiseDbmHc / (double) _nNoiseDbmHc;
    if (_params.debug) {
      cerr << "==>> _nNoiseDbmHc, _meanNoiseDbmHc: "
           << _nNoiseDbmHc << ", " <<  _meanNoiseDbmHc << endl;
    }
  }

  if (_nNoiseDbmVc > 0) {
    _meanNoiseDbmVc = _sumNoiseDbmVc / (double) _nNoiseDbmVc;
    if (_params.debug) {
      cerr << "==>> _nNoiseDbmVc, _meanNoiseDbmVc: "
           << _nNoiseDbmVc << ", " <<  _meanNoiseDbmVc << endl;
    }
  }

  if (_nNoiseDbmHx > 0) {
    _meanNoiseDbmHx = _sumNoiseDbmHx / (double) _nNoiseDbmHx;
    if (_params.debug) {
      cerr << "==>> _nNoiseDbmHx, _meanNoiseDbmHx: "
           << _nNoiseDbmHx << ", " <<  _meanNoiseDbmHx << endl;
    }
  }

  if (_nNoiseDbmVx > 0) {
    _meanNoiseDbmVx = _sumNoiseDbmVx / (double) _nNoiseDbmVx;
    if (_params.debug) {
      cerr << "==>> _nNoiseDbmVx, _meanNoiseDbmVx: "
           << _nNoiseDbmVx << ", " <<  _meanNoiseDbmVx << endl;
    }
  }

}

//////////////////////////////////////////////////
// Write status XML to spdb

int RadxCov2Mom::_writeStatusXmlToSpdb(const RadxVol &vol,
                                       const string &xml)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Writing stats XML to SPDB:" << endl;
    cerr << xml << endl;
  }

  DsSpdb spdb;
  time_t validTime = vol.getStartTimeSecs();
  spdb.addPutChunk(0, validTime, validTime, xml.size() + 1, xml.c_str());
  if (spdb.put(_params.status_spdb_output_url,
               SPDB_XML_ID, SPDB_XML_LABEL)) {
    cerr << "ERROR - RadxCov2Mom::_writeStatusXmlToSpdb" << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Wrote to spdb, url: " << _params.status_spdb_output_url << endl;
  }

  return 0;


}


