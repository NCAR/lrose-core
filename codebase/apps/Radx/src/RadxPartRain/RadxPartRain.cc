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
#include <cerrno>
#include <algorithm>
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/file_io.h>
#include <toolsa/TaFile.hh>
#include <toolsa/TaArray.hh>
#include <dsserver/DsLdataInfo.hh>
#include <rapformats/WxObs.hh>
#include <rapmath/stats.h>
#include <Spdb/DsSpdb.hh>
#include <rapmath/trig.h>
#include <Mdv/GenericRadxFile.hh>
#include <radar/BeamHeight.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxXml.hh>
using namespace std;

// initialize extra field names

string RadxPartRain::smoothedDbzFieldName = "smoothedDbz";
string RadxPartRain::smoothedRhohvFieldName = "smoothedRhohv";
string RadxPartRain::elevationFieldName = "elevation";
string RadxPartRain::rangeFieldName = "range";
string RadxPartRain::beamHtFieldName = "beam_height";
string RadxPartRain::tempFieldName = "temperature";
string RadxPartRain::pidFieldName = "pid";
string RadxPartRain::pidInterestFieldName = "pid_interest";
string RadxPartRain::mlFieldName = "melting_layer";
string RadxPartRain::mlExtendedFieldName = "ml_extended";
string RadxPartRain::convFlagFieldName = "conv_flag";

// Constructor

RadxPartRain::RadxPartRain(int argc, char **argv)
  
{

  OK = TRUE;
  _engineSingle = NULL;

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
      cerr << "  For estimating ZDR bias, you must set KDP_available to FALSE."
           << endl;
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
      ComputeThread *thread = new ComputeThread(this, _params, ii);
      if (!thread->OK) {
        delete thread;
        OK = FALSE;
        return;
      }
      _threadPool.addThreadToMain(thread);
    }

  } else {

    // single threaded
    
    _engineSingle = new ComputeEngine(_params, 0);
    if (!_engineSingle->OK) {
      OK = FALSE;
    }

  }

}

//////////////////////////////////////
// destructor

RadxPartRain::~RadxPartRain()

{

  if (_engineSingle) {
    delete _engineSingle;
  }
  
  // mutex

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

  if (_params.debug) {
    cerr << "RadxPartRain::_runArchive" << endl;
    cerr << "  startTime: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  endTime: " << RadxTime::strm(_args.endTime) << endl;
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
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "n files found: " << paths.size() << endl;
    for (size_t ipath = 0; ipath < paths.size(); ipath++) {
      cerr << "    " << paths[ipath] << endl;
    }
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
    if (rpath.getFile().find(_params.input_file_search_substr)
        == string::npos) {
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
  
  if (inFile.readFromPath(filePath, _vol)) {
    cerr << "ERROR - RadxPartRain::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  _readPaths = inFile.getReadPaths();

  // convert to fl32
  
  _vol.convertToFl32();

  // override radar location if requested

  if (_params.override_radar_location) {
    _vol.overrideLocation(_params.radar_latitude_deg,
                         _params.radar_longitude_deg,
                         _params.radar_altitude_meters / 1000.0);
  }
  
  // set radar properties

  _wavelengthM = _vol.getWavelengthM();
  _radarHtKm = _vol.getAltitudeKm();

  // set number of gates constant if requested

  if (_params.set_ngates_constant) {
    _vol.setNGatesConstant();
  }

  // trim surveillance sweeps to 360 degrees

  _vol.trimSurveillanceSweepsTo360Deg();

  // retrieve temp profile from SPDB as appropriate

  _retrieveTempProfile();

  // option to get site temperature
  
  if (_params.read_site_temp_from_spdb) {
    if (_retrieveSiteTempFromSpdb(_siteTempC,
                                  _timeForSiteTemp) == 0) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> site tempC: " 
             << _siteTempC << " at " 
             << RadxTime::strm(_timeForSiteTemp) << endl;
      }
    }
  }

  // initialize for ZDR bias and self consistency results

  _zdrInIceElev.clear();
  _zdrInIceResults.clear();
  _zdrInBraggResults.clear();
  _zdrmInIceResults.clear();
  _zdrmInBraggResults.clear();
  _selfConResults.clear();

  // add geometry and pid fields to the volume
  
  _addExtraFieldsToInput();

  // compute the derived fields
  
  if (_compute()) {
    cerr << "ERROR - RadxPartRain::Run" << endl;
    cerr << "  Cannot compute moments" << endl;
    return -1;
  }

  // optionally compute ZDR bias and write to SPDB

  if (_params.estimate_zdr_bias_in_ice ||
      _params.estimate_zdr_bias_in_bragg) {
    _computeZdrBias();
  }

  // optionally compute Z bias using self consistency and write results to SPDB

  if (_params.estimate_z_bias_using_self_consistency) {
    _computeSelfConZBias();
  }

  // compute melting layer statistics

  if (_params.PID_locate_melting_layer) {
    _locateMeltingLayer();
  }
  
  // add extra fields to output
  
  _addExtraFieldsToOutput();

  // write results to output file

  if (_params.write_output_volume) {
    if (_writeVol()) {
      return -1;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// encode fields for output

void RadxPartRain::_encodeFieldsForOutput()
{

  vector<RadxRay *> &rays = _vol.getRays();

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

int RadxPartRain::_writeVol()
{

  // clear the input rays

  _vol.clearRays();
  
  // add the derived rays to the volume
  
  for (size_t iray = 0; iray < _derivedRays.size(); iray++) {
    _vol.addRay(_derivedRays[iray]);
  }
  
  _vol.sortRaysByNumber();
  _vol.loadVolumeInfoFromRays();
  _vol.loadSweepInfoFromRays();
  _vol.setPackingFromRays();

  // create output file object

  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  if (_params.output_filename_mode == Params::SPECIFY_FILE_NAME) {

    string outPath = _params.output_dir;
    outPath += PATH_DELIM;
    outPath += _params.output_filename;

    // write to path
  
    if (outFile.writeToPath(_vol, outPath)) {
      cerr << "ERROR - RadxPartRain::_writeVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
      
  } else {

    // write to dir
  
    if (outFile.writeToDir(_vol, _params.output_dir,
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
  if (ldata.write(_vol.getEndTimeSecs())) {
    cerr << "WARNING - RadxPartRain::_writeVol" << endl;
    cerr << "  Cannot write latest data info file to dir: "
         << _params.output_dir << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// add geometry and pid fields to the volume

void RadxPartRain::_addExtraFieldsToInput()

{

  vector<RadxRay *> &rays = _vol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {

    RadxRay *ray = rays[iray];

    RadxField *smoothDbzFld = new RadxField(smoothedDbzFieldName, "dBZ");
    RadxField *smoothRhohvFld = new RadxField(smoothedRhohvFieldName, "");
    RadxField *elevFld = new RadxField(elevationFieldName, "deg");
    RadxField *rangeFld = new RadxField(rangeFieldName, "km");
    RadxField *beamHtFld = new RadxField(beamHtFieldName, "km");
    RadxField *tempFld = new RadxField(tempFieldName, "C");
    RadxField *pidFld = new RadxField(pidFieldName, "");
    RadxField *pidIntrFld = new RadxField(pidInterestFieldName, "");
    RadxField *mlFld = new RadxField(mlFieldName, "");

    size_t nGates = ray->getNGates();

    TaArray<Radx::fl32> elev_, rng_, ht_, temp_;
    Radx::fl32 *elev = elev_.alloc(nGates);
    Radx::fl32 *rng = rng_.alloc(nGates);
    Radx::fl32 *ht = ht_.alloc(nGates);
    Radx::fl32 *temp = temp_.alloc(nGates);
    
    double startRangeKm = ray->getStartRangeKm();
    double gateSpacingKm = ray->getGateSpacingKm();

    BeamHeight beamHt;
    beamHt.setInstrumentHtKm(_vol.getAltitudeKm());
    if (_params.override_standard_pseudo_earth_radius) {
      beamHt.setPseudoRadiusRatio(_params.pseudo_earth_radius_ratio);
    }

    // loop through the gates
    
    double rangeKm = startRangeKm;
    double elevationDeg = ray->getElevationDeg();
    for (size_t igate = 0; igate < nGates; igate++, rangeKm += gateSpacingKm) {
      double htKm = beamHt.computeHtKm(elevationDeg, rangeKm);
      double tempC = _tempProfile.getTempForHtKm(htKm);
      elev[igate] = elevationDeg;
      rng[igate] = rangeKm;
      ht[igate] = htKm;
      temp[igate] = tempC;
    } // igate

    smoothDbzFld->setTypeFl32(-9999.0);
    smoothRhohvFld->setTypeFl32(-9999.0);
    elevFld->setTypeFl32(-9999.0);
    rangeFld->setTypeFl32(-9999.0);
    beamHtFld->setTypeFl32(-9999.0);
    tempFld->setTypeFl32(-9999.0);
    pidFld->setTypeSi32(-9999, 1.0, 0.0);
    pidIntrFld->setTypeFl32(-9999.0);
    mlFld->setTypeSi32(-9999, 1.0, 0.0);
    
    smoothDbzFld->addDataMissing(nGates);
    smoothRhohvFld->addDataMissing(nGates);
    elevFld->addDataFl32(nGates, elev);
    rangeFld->addDataFl32(nGates, rng);
    beamHtFld->addDataFl32(nGates, ht);
    tempFld->addDataFl32(nGates, temp);
    pidFld->addDataMissing(nGates);
    pidIntrFld->addDataMissing(nGates);
    mlFld->addDataMissing(nGates);

    ray->addField(smoothDbzFld);
    ray->addField(smoothRhohvFld);
    ray->addField(elevFld);
    ray->addField(rangeFld);
    ray->addField(beamHtFld);
    ray->addField(tempFld);
    ray->addField(pidFld);
    ray->addField(pidIntrFld);
    ray->addField(mlFld);

  } // iray

}

//////////////////////////////////////////////////
// add extra output fields

void RadxPartRain::_addExtraFieldsToOutput()

{

  // loop through rays

  vector<RadxRay *> &inputRays = _vol.getRays();
  for (size_t iray = 0; iray < inputRays.size(); iray++) {
    
    RadxRay *inputRay = inputRays[iray];

    // match output ray to input ray based on time
    
    RadxRay *outputRay = NULL;
    double inTime = inputRay->getTimeDouble();
    for (size_t jj = 0; jj < _derivedRays.size(); jj++) {
      RadxRay *derivedRay = _derivedRays[jj];
      double outTime = derivedRay->getTimeDouble();
      if (fabs(inTime - outTime) < 0.001) {
        outputRay = derivedRay;
        break;
      }
    } // jj

    if (outputRay != NULL) {
      // make a copy of the input fields
      RadxField *mlFld = new RadxField(*inputRay->getField(mlFieldName));
      // add to output
      outputRay->addField(mlFld);
    }

  } // iray

}

//////////////////////////////////////////////////
// compute the moments for all rays in volume

int RadxPartRain::_compute()
{

  // initialize the volume with ray numbers
  
  _vol.setRayNumbersInOrder();

  // initialize derived

  _derivedRays.clear();
  
  if (_params.use_multiple_threads) {
    if (_computeMultiThreaded()) {
      return -1;
    }
  } else {
    if (_computeSingleThreaded()) {
      return -1;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// compute the moments for all rays in volume
// single-threaded operation

int RadxPartRain::_computeSingleThreaded()
{

  // loop through the input rays,
  // computing the derived fields

  const vector<RadxRay *> &inputRays = _vol.getRays();
  for (size_t iray = 0; iray < inputRays.size(); iray++) {
    
    // get covariance ray
    
    RadxRay *inputRay = inputRays[iray];
    
    // compute moments
    
    RadxRay *derivedRay =
      _engineSingle->compute(inputRay, _radarHtKm, 
                             _wavelengthM, &_tempProfile);
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

int RadxPartRain::_computeMultiThreaded()
{

  // loop through the input rays,
  // computing the derived fields

  const vector<RadxRay *> &inputRays = _vol.getRays();
  for (size_t iray = 0; iray < inputRays.size(); iray++) {

    // get a thread from the pool
    bool isDone = true;
    ComputeThread *thread = 
      (ComputeThread *) _threadPool.getNextThread(true, isDone);
    if (thread == NULL) {
      break;
    }
    if (isDone) {
      // store the results computed by the thread
      _storeDerivedRay(thread);
      // return thread to the available pool
      _threadPool.addThreadToAvail(thread);
      // reduce iray by 1 since we did not actually process this ray
      // we only handled a previously started thread
      iray--;
    } else {
      // got a thread to use, set the input ray
      thread->setInputRay(inputRays[iray]);
      // set it running
      thread->signalRunToStart();
    }

  } // iray
    
  // collect remaining done threads

  _threadPool.setReadyForDoneCheck();
  while (!_threadPool.checkAllDone()) {
    ComputeThread *thread = (ComputeThread *) _threadPool.getNextDoneThread();
    if (thread == NULL) {
      break;
    } else {
      // store the results computed by the thread
      _storeDerivedRay(thread);
      // return thread to the available pool
      _threadPool.addThreadToAvail(thread);
    }
  } // while

  return 0;

}

///////////////////////////////////////////////////////////
// Store the derived ray

int RadxPartRain::_storeDerivedRay(ComputeThread *thread)

{
  
  RadxRay *derivedRay = thread->getDerivedRay();
  if (derivedRay != NULL) {
    // good return, add to results
    _derivedRays.push_back(derivedRay);
  }
  
  // load ZDR bias results

  const vector<double> &threadZdrInIceElev =
    thread->getComputeEngine()->getZdrInIceElev();
  for (size_t ii = 0; ii < threadZdrInIceElev.size(); ii++) {
    _zdrInIceElev.push_back(threadZdrInIceElev[ii]);
  }

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
      
//////////////////////////////////////
// initialize temperature profile
  
int RadxPartRain::_retrieveTempProfile()
  
{

  time_t retrievedTime = time(NULL);
  _tempProfile.clear();

  if (_params.use_soundings_from_spdb) {
    
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
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      _tempProfile.setDebug();
    }
    if (_params.debug >= Params::DEBUG_EXTRA) {
      _tempProfile.setVerbose();
    }
  
    if (_tempProfile.loadFromSpdb(_params.sounding_spdb_url,
                                  _vol.getStartTimeSecs(),
                                  retrievedTime)) {
      cerr << "ERROR - RadxPartRain::_tempProfileInit" << endl;
      cerr << "  Cannot retrive profile for time: "
           << RadxTime::strm(_vol.getStartTimeSecs()) << endl;
      cerr << "  url: " << _params.sounding_spdb_url << endl;
      cerr << "  station name: " << _params.sounding_location_name << endl;
      cerr << "  time margin secs: " << _params.sounding_search_time_margin_secs << endl;
      return -1;
    }

  } else {
    
    // get profile from PID file

    if (_tempProfile.loadFromPidThresholdsFile(_params.pid_thresholds_file_path)) {
      return -1;
    }

  }

  if (_params.debug) {
    cerr << "=====================================" << endl;
    cerr << "Got temp profile, URL: " << _params.sounding_spdb_url << endl;
    cerr << "Overriding temperature profile" << endl;
    cerr << "  vol time: " << RadxTime::strm(_vol.getStartTimeSecs()) << endl;
    cerr << "  retrievedTime: " << RadxTime::strm(retrievedTime) << endl;
    cerr << "  freezingLevel: " << _tempProfile.getFreezingLevel() << endl;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    const vector<TempProfile::PointVal> &retrievedProfile = _tempProfile.getProfile();
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

int RadxPartRain::_retrieveSiteTempFromSpdb(double &tempC,
                                            time_t &timeForTemp)
  
{

  // get surface data from SPDB

  DsSpdb spdb;
  si32 dataType = 0;
  if (strlen(_params.site_temp_station_name) > 0) {
    dataType =  Spdb::hash4CharsToInt32(_params.site_temp_station_name);
  }
  time_t searchTime = _vol.getStartTimeSecs();

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

void RadxPartRain::_computeZdrBias()

{

  // compute the bias stats

  _zdrStatsIce.clear();
  _zdrStatsBragg.clear();

  _zdrmStatsIce.clear();
  _zdrmStatsBragg.clear();

  if ((int) _zdrInIceResults.size() > _params.zdr_bias_ice_min_npoints_valid) {
    _loadZdrResults("ZdrInIce",
                    _zdrInIceResults,
                    _zdrStatsIce,
                    _params.zdr_bias_ice_percentiles_n,
                    _params._zdr_bias_ice_percentiles);
  }
  
  if ((int) _zdrmInIceResults.size() > _params.zdr_bias_ice_min_npoints_valid) {
    _loadZdrResults("ZdrmInIce",
                    _zdrmInIceResults,
                    _zdrmStatsIce,
                    _params.zdr_bias_ice_percentiles_n,
                    _params._zdr_bias_ice_percentiles);
  }
  
  if ((int) _zdrInBraggResults.size() > _params.zdr_bias_bragg_min_npoints_valid) {
    _loadZdrResults("ZdrInBragg",
                    _zdrInBraggResults,
                    _zdrStatsBragg,
                    _params.zdr_bias_bragg_percentiles_n,
                    _params._zdr_bias_bragg_percentiles);
  }
  
  if ((int) _zdrmInBraggResults.size() > _params.zdr_bias_bragg_min_npoints_valid) {
    _loadZdrResults("ZdrmInBragg",
                    _zdrmInBraggResults,
                    _zdrmStatsBragg,
                    _params.zdr_bias_bragg_percentiles_n,
                    _params._zdr_bias_bragg_percentiles);
  }

  if (_params.save_ice_zdr_to_file) {
    _saveZdrInIceToFile();
  }

  // write results to SPDB
  
  if (!_params.zdr_bias_write_results_to_spdb) {
    return;
  }

  RadxPath path(_vol.getPathInUse());
  string xml;

  xml += RadxXml::writeStartTag("ZdrBias", 0);

  xml += RadxXml::writeString("file", 1, path.getFile());
  xml += RadxXml::writeBoolean("is_rhi", 1, _vol.checkIsRhi());
  
  xml += RadxXml::writeInt("ZdrInIceNpts", 1, (int) _zdrStatsIce.count);
  xml += RadxXml::writeDouble("ZdrInIceMean", 1, _zdrStatsIce.mean);
  xml += RadxXml::writeDouble("ZdrInIceSdev", 1, _zdrStatsIce.sdev);
  xml += RadxXml::writeDouble("ZdrInIceSkewness", 1, _zdrStatsIce.skewness);
  xml += RadxXml::writeDouble("ZdrInIceKurtosis", 1, _zdrStatsIce.kurtosis);
  for (size_t ii = 0; ii < _zdrStatsIce.percentiles.size(); ii++) {
    double percent = _params._zdr_bias_ice_percentiles[ii];
    double val = _zdrStatsIce.percentiles[ii];
    char tag[1024];
    sprintf(tag, "ZdrInIcePerc%05.2f", percent);
    xml += RadxXml::writeDouble(tag, 1, val);
  }

  xml += RadxXml::writeInt("ZdrmInIceNpts", 1, (int) _zdrmStatsIce.count);
  xml += RadxXml::writeDouble("ZdrmInIceMean", 1, _zdrmStatsIce.mean);
  xml += RadxXml::writeDouble("ZdrmInIceSdev", 1, _zdrmStatsIce.sdev);
  xml += RadxXml::writeDouble("ZdrmInIceSkewness", 1, _zdrmStatsIce.skewness);
  xml += RadxXml::writeDouble("ZdrmInIceKurtosis", 1, _zdrmStatsIce.kurtosis);
  for (size_t ii = 0; ii < _zdrmStatsIce.percentiles.size(); ii++) {
    double percent = _params._zdr_bias_ice_percentiles[ii];
    double val = _zdrmStatsIce.percentiles[ii];
    char tag[1024];
    sprintf(tag, "ZdrmInIcePerc%05.2f", percent);
    xml += RadxXml::writeDouble(tag, 1, val);
  }

  xml += RadxXml::writeInt("ZdrInBraggNpts", 1, (int) _zdrStatsBragg.count);
  xml += RadxXml::writeDouble("ZdrInBraggMean", 1, _zdrStatsBragg.mean);
  xml += RadxXml::writeDouble("ZdrInBraggSdev", 1, _zdrStatsBragg.sdev);
  xml += RadxXml::writeDouble("ZdrInBraggSkewness", 1, _zdrStatsBragg.skewness);
  xml += RadxXml::writeDouble("ZdrInBraggKurtosis", 1, _zdrStatsBragg.kurtosis);
  for (size_t ii = 0; ii < _zdrStatsBragg.percentiles.size(); ii++) {
    double percent = _params._zdr_bias_bragg_percentiles[ii];
    double val = _zdrStatsBragg.percentiles[ii];
    char tag[1024];
    sprintf(tag, "ZdrInBraggPerc%05.2f", percent);
    xml += RadxXml::writeDouble(tag, 1, val);
  }

  xml += RadxXml::writeInt("ZdrmInBraggNpts", 1, (int) _zdrmStatsBragg.count);
  xml += RadxXml::writeDouble("ZdrmInBraggMean", 1, _zdrmStatsBragg.mean);
  xml += RadxXml::writeDouble("ZdrmInBraggSdev", 1, _zdrmStatsBragg.sdev);
  xml += RadxXml::writeDouble("ZdrmInBraggSkewness", 1, _zdrmStatsBragg.skewness);
  xml += RadxXml::writeDouble("ZdrmInBraggKurtosis", 1, _zdrmStatsBragg.kurtosis);
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
  time_t validTime = _vol.getStartTimeSecs();
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

void RadxPartRain::_loadZdrResults(string label,
                                   vector<double> &results,
                                   ZdrStats &stats,
                                   int nPercentiles,
                                   double *percentiles)

{

  // sort results vector
  
  sort(results.begin(), results.end());

  // compute stats

  stats.clear();

  double mean, sdev;
  STATS_normal_fit(results.size(), &results[0], &mean, &sdev);
  double skewness = STATS_normal_skewness(results.size(), &results[0], mean, sdev);
  double kurtosis = STATS_normal_kurtosis(results.size(), &results[0], mean, sdev);

  stats.count = results.size();
  stats.mean = mean;
  stats.sdev = sdev;
  stats.skewness = skewness;
  stats.kurtosis = kurtosis;

  for (int ii = 0; ii < nPercentiles; ii++) {
    stats.percentiles.push_back(_computeZdrPerc(results, percentiles[ii]));
  }

}

////////////////////////////////////////////
// save ZDR data in ice to files

void RadxPartRain::_saveZdrInIceToFile()

{

  // make output dir

  if (ta_makedir_recurse(_params.ice_zdr_save_dir)) {
    int errNum = errno;
    cerr << "ERROR - RadxPartRain::_saveZdrInIceToFile()" << endl;
    cerr << "  Cannot make dir: " << _params.ice_zdr_save_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return;
  }
  
  // compute file names

  RadxTime startTime(_vol.getStartTimeSecs());

  char appendPath[1024];
  sprintf(appendPath, "%s%sZdrInIce.txt", 
          _params.ice_zdr_save_dir, PATH_DELIM);

  char appendmPath[1024];
  sprintf(appendmPath, "%s%sZdrmInIce.txt", 
          _params.ice_zdr_save_dir, PATH_DELIM);
  
  char volPath[1024];
  sprintf(volPath, "%s%sZdrInIce_%.4d%.2d%.2d_%.2d%.2d%.2d.txt", 
          _params.ice_zdr_save_dir, PATH_DELIM,
          startTime.getYear(), startTime.getMonth(), startTime.getDay(),
          startTime.getHour(), startTime.getMin(), startTime.getSec());

  char volmPath[1024];
  sprintf(volmPath, "%s%sZdrmInIce_%.4d%.2d%.2d_%.2d%.2d%.2d.txt", 
          _params.ice_zdr_save_dir, PATH_DELIM,
          startTime.getYear(), startTime.getMonth(), startTime.getDay(),
          startTime.getHour(), startTime.getMin(), startTime.getSec());

  // open files

  TaFile append; // closes when goes out of scope
  FILE *appendFile;
  if ((appendFile = append.fopen(appendPath, "a")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - RadxPartRain::_saveZdrInIceToFile()" << endl;
    cerr << "  Cannot open zdr output file: " << appendPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return;
  }
  _writeHeaderZdrInIce(appendFile);
  
  TaFile appendm; // closes when goes out of scope
  FILE *appendmFile;
  if ((appendmFile = append.fopen(appendmPath, "a")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - RadxPartRain::_saveZdrInIceToFile()" << endl;
    cerr << "  Cannot open zdr output file: " << appendmPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return;
  }
  _writeHeaderZdrInIce(appendmFile);
  
  TaFile vol; // closes when goes out of scope
  FILE *volFile;
  if ((volFile = vol.fopen(volPath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - RadxPartRain::_saveZdrInIceToFile()" << endl;
    cerr << "  Cannot open zdr output file: " << volPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return;
  }
  _writeHeaderZdrInIce(volFile);
  
  TaFile volm; // closes when goes out of scope
  FILE *volmFile;
  if ((volmFile = vol.fopen(volmPath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - RadxPartRain::_saveZdrInIceToFile()" << endl;
    cerr << "  Cannot open zdr output file: " << volmPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return;
  }
  _writeHeaderZdrInIce(volmFile);
  
  // write to files

  for (size_t ii = 0; ii < _zdrInIceResults.size(); ii++) {
    fprintf(appendFile, "%.3f %.4f\n", _zdrInIceElev[ii], _zdrInIceResults[ii]);
    fprintf(volFile, "%.3f %.4f\n", _zdrInIceElev[ii], _zdrInIceResults[ii]);
  }

  for (size_t ii = 0; ii < _zdrmInIceResults.size(); ii++) {
    fprintf(appendmFile, "%.3f %.4f\n", _zdrInIceElev[ii], _zdrmInIceResults[ii]);
    fprintf(volmFile, "%.3f %.4f\n", _zdrInIceElev[ii], _zdrmInIceResults[ii]);
  }

  fflush(appendFile);
  fflush(appendmFile);
  fflush(volFile);
  fflush(volmFile);
  
}

/////////////////////////////////////////////////////////////
// write header to zdr in ice file
// if the file is zero length

void RadxPartRain::_writeHeaderZdrInIce(FILE *out)

{

  // is the file 0-length?

  struct stat fstat;
  bool fileIsNew = false;
  if (ta_fstat(fileno(out), &fstat) == 0) {
    if (fstat.st_size == 0) {
      fileIsNew = true;
    }
  }

  if (!fileIsNew) {
    // file is not new, no need to add header
    return;
  }

  fprintf(out, "# elev zdr\n");
  fprintf(out, "#============================================\n");
  fprintf(out, "# Table produced by RadxPartRain\n");
  fprintf(out, "#------------ Table column list -------------\n");
  fprintf(out, "#    col 000: elev\n");
  fprintf(out, "#    col 001: zdr\n");
  fprintf(out, "#--------------------------------------------\n");
  fprintf(out, "#============================================\n");
  
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

void RadxPartRain::_computeSelfConZBias()

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
    cerr << "Time, volBias: " << RadxTime::strm(_vol.getStartTimeSecs()) << ", " << volBias << endl;
  }
  
  // prepare to write results to SPDB
  
  if (!_params.self_consistency_write_results_to_spdb) {
    return;
  }

  // add chunks

  DsSpdb spdb;
  RadxPath path(_vol.getPathInUse());

  for (size_t ii = 0; ii < _selfConResults.size(); ii++) {

    const ComputeEngine::self_con_t &results = _selfConResults[ii];
    
    string xml;
    xml += RadxXml::writeStartTag("SelfConsistency", 0);

    xml += RadxXml::writeString("file", 1, path.getFile());
    xml += RadxXml::writeBoolean("is_rhi", 1, _vol.checkIsRhi());
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
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
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

/////////////////////////////////////////////////////////////
// Locate the melting layer
// Compute stats

void RadxPartRain::_locateMeltingLayer()

{

  /////////////////////////////////////////////////////////////
  // load pseudo RHIs

  _vol.loadPseudoRhis();
  _pseudoRhis = _vol.getPseudoRhis();

  /////////////////////////////////////////////////////////////
  // set the melting layer field at all gates at
  // which PID is WET_SNOW and fill in the gaps

  for (size_t iray = 0; iray < _vol.getRays().size(); iray++) {
    
    RadxRay *ray = _vol.getRays()[iray];

    size_t nGates = ray->getNGates();
    
    RadxField *pidField = ray->getField(pidFieldName);
    const Radx::si32 *pidVals = pidField->getDataSi32();
    
    RadxField *mlField = ray->getField(mlFieldName);
    Radx::si32 *mlVals = mlField->getDataSi32();
    
    for (size_t igate = 0; igate < nGates; igate++) {
      Radx::si32 pid = pidVals[igate];
      if (pid == NcarParticleId::WET_SNOW ||
          pid == NcarParticleId::GRAUPEL_RAIN) {
        mlVals[igate] = 1;
      } else if (pid > 0) {
        mlVals[igate] = 0;
      }
    } // igate
    
    // fill in gaps in melting layer field
    
    _applyInfillFilter(nGates, mlVals);
    
  } // iray

  /////////////////////////////////////////////////////////////
  // get vector of heights for all melting layer gates
  // so that we can compute stats on the melting layer
  
  vector<double> mlHts;
  vector<bool> rayHasMl;
  rayHasMl.resize(_vol.getRays().size());
  
  for (size_t iray = 0; iray < _vol.getRays().size(); iray++) {
    
    rayHasMl[iray] = false;
    
    // get ray
    
    RadxRay *ray = _vol.getRays()[iray];
    double elevation = ray->getElevationDeg();
    size_t nGates = ray->getNGates();
    
    if (elevation < 2.9 || elevation > 10.0) {
      // only process elevations between 4 and 10 deg elevation
      continue;
    }
    
    // get PID and height fields for this ray
    
    RadxField *mlField = ray->getField(mlFieldName);
    const Radx::si32 *mlVals = mlField->getDataSi32();

    RadxField *htField = ray->getField(beamHtFieldName);
    const Radx::fl32 *htVals = htField->getDataFl32();
    
    // loop through the gates loading up the ht array
    // for points in the melting layer
    // also find fist and last gates with wet snow
    
    bool hasMl = false;
    for (size_t igate = 0; igate < nGates; igate++) {
      if (mlVals[igate] > 0) {
        mlHts.push_back(htVals[igate]);
        hasMl = true;
      }
    } // igate
    
    if (hasMl) {
      rayHasMl[iray] = true;
    }

  } // iray

  if (mlHts.size() < 1) {
    cerr << "WARNING - RadxPartRain::_locateMeltingLayer" << endl;
    cerr << "  No melting layer found" << endl;
    cerr << "  i.e. no wet snow in PID" << endl;
    return;
  }

  /////////////////////////////////////////////////////////////
  // compute stats on the melting layer
  // sort the heights
  
  sort(mlHts.begin(), mlHts.end());

  // compute height-based properties
  
  int ipercBot = (int)
    ((_params.melting_layer_percentile_for_bottom_limit / 100.0) * 
     (double) mlHts.size());
  double htMlBot = mlHts[ipercBot];

  int ipercTop = (int)
    ((_params.melting_layer_percentile_for_top_limit / 100.0) * 
     (double) mlHts.size());
  double htMlTop = mlHts[ipercTop];

  double tempBot = _tempProfile.getTempForHtKm(htMlBot);
  double tempTop = _tempProfile.getTempForHtKm(htMlTop);

  // compute mean/median dbz immediately below and above layer

  vector<double> dbzBelow, dbzAbove;

  for (size_t iray = 0; iray < _vol.getRays().size(); iray++) {
    
    if (!rayHasMl[iray]) {
      continue;
    }

    RadxRay *ray = _vol.getRays()[iray];
    size_t nGates = ray->getNGates();

    // get DBZ field

    RadxField *dbzField = ray->getField(smoothedDbzFieldName);
    const Radx::fl32 *dbzVals = dbzField->getDataFl32();

    // get PID field and ht fields
    
    RadxField *pidField = ray->getField(pidFieldName);
    const Radx::si32 *pidVals = pidField->getDataSi32();
    
    RadxField *htField = ray->getField(beamHtFieldName);
    const Radx::fl32 *htVals = htField->getDataFl32();

    // find the gates for the bottom and top of the melting layer
    // as determined above

    int igateBot = 0;
    int igateTop = nGates - 1;
    
    for (size_t igate = 0; igate < nGates; igate++) {
      double gateHt = htVals[igate];
      if (igateBot == 0 && gateHt >= htMlBot) {
        igateBot = igate;
      }
      if (gateHt >= htMlTop) {
        igateTop = igate;
        break;
      }
    }

    int nGatesMeltingLayer = igateTop - igateBot + 1;

    // accumulate dbz values above and below the ML
    // so that we can compute the mean values for use
    // in precip estimation
    
    int countBelow = 0;
    for (int igate = igateBot; igate >= 0; igate--) {
      if (pidVals[igate] == NcarParticleId::LIGHT_RAIN ||
          pidVals[igate] == NcarParticleId::MODERATE_RAIN) {
        dbzBelow.push_back(dbzVals[igate]);
        countBelow++;
        if ((igateBot - igate) > nGatesMeltingLayer) {
          break;
        }
      }
    } // igate
    
    int countAbove = 0;
    for (int igate = igateTop; igate < (int) nGates; igate++) {
      if (pidVals[igate] == NcarParticleId::DRY_SNOW) {
        dbzAbove.push_back(dbzVals[igate]);
        countAbove++;
        if ((igate - igateTop) > nGatesMeltingLayer) {
          break;
        }
      }
    } // igate
    
  } // iray

  double meanDbzBelow = -9999.0;
  double medianDbzBelow = -9999.0;
  if (dbzBelow.size() > 0) {
    double sumDbzBelow = 0.0;
    for (size_t ii = 0; ii < dbzBelow.size(); ii++) {
      sumDbzBelow += dbzBelow[ii];
    }
    meanDbzBelow = sumDbzBelow / (double) dbzBelow.size();
    sort(dbzBelow.begin(), dbzBelow.end());
    medianDbzBelow = dbzBelow[dbzBelow.size() / 2];
  }

  double meanDbzAbove = -9999.0;
  double medianDbzAbove = -9999.0;
  double perc75DbzAbove = -9999.0;
  if (dbzAbove.size() > 0) {
    double sumDbzAbove = 0.0;
    for (size_t ii = 0; ii < dbzAbove.size(); ii++) {
      sumDbzAbove += dbzAbove[ii];
    }
    meanDbzAbove = sumDbzAbove / (double) dbzAbove.size();
    sort(dbzAbove.begin(), dbzAbove.end());
    medianDbzAbove = dbzAbove[dbzAbove.size() / 2];
    perc75DbzAbove = dbzAbove[dbzAbove.size() * 3 / 4];
  }

  
  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
  // debug print

  if (_params.debug) {
    cerr << "========== melting layer stats ============" << endl;
    cerr << "  ML perc for bot: "
         << _params.melting_layer_percentile_for_bottom_limit << endl;
    cerr << "  ML perc for top: "
         << _params.melting_layer_percentile_for_top_limit << endl;
    cerr << "  ML ht bot: " << htMlBot << endl;
    cerr << "  ML ht top: " << htMlTop << endl;
    cerr << "  ML temp bot: " << tempBot << endl;
    cerr << "  ML temp top: " << tempTop << endl;
    cerr << "  mean dBZ below: " << meanDbzBelow << endl;
    cerr << "  mean dBZ above: " << meanDbzAbove << endl;
    cerr << "  median dBZ below: " << medianDbzBelow << endl;
    cerr << "  median dBZ above: " << medianDbzAbove << endl;
    cerr << "  perc75 dBZ above: " << perc75DbzAbove << endl;
    cerr << "===========================================" << endl;
  }

  // create the convective flag field for each ray
  
  for (size_t iray = 0; iray < _vol.getRays().size(); iray++) {
    RadxRay *ray = _vol.getRays()[iray];
    size_t nGates = ray->getNGates();
    RadxField *convFld = new RadxField(convFlagFieldName, "");
    convFld->setTypeSi32(Radx::missingSi32, 1.0, 0.0);
    convFld->addDataMissing(nGates);
    ray->addField(convFld);
  }

  // identify convective regions from the vertical profile
  // and PID
  
  _checkForConvection(htMlTop);

  // create the extended ml flag field for each ray
  
  for (size_t iray = 0; iray < _vol.getRays().size(); iray++) {
    RadxRay *ray = _vol.getRays()[iray];
    size_t nGates = ray->getNGates();
    RadxField *mleFld = new RadxField(mlExtendedFieldName, "");
    mleFld->setTypeSi32(Radx::missingSi32, 1.0, 0.0);
    mleFld->addDataMissing(nGates);
    ray->addField(mleFld);
  }

  // expand the melting layer from the top of the layer to the
  // height at which the dbz has dropped to the 75th percentile
  // RHI mode
  
  if (_vol.checkIsRhi()) {
    _expandMlRhi(htMlTop, perc75DbzAbove);
  } else {
    _expandMlPpi(htMlTop, medianDbzAbove);
  }

  // fill in the gaps in the extended field

  for (size_t iray = 0; iray < _vol.getRays().size(); iray++) {
    RadxRay *ray = _vol.getRays()[iray];
    size_t nGates = ray->getNGates();
    RadxField *mleField = ray->getField(mlExtendedFieldName);
    if (mleField != NULL) {
      Radx::si32 *mleVals = mleField->getDataSi32();
      _applyInfillFilter(nGates, mleVals);
    }
  } // iray
  
  // load extra fields into derived ray
  
  for (size_t iray = 0; iray < _vol.getRays().size(); iray++) {
    RadxRay *ray = _vol.getRays()[iray];
    RadxRay *derivedRay = _derivedRays[iray];
    RadxField *mleFld = ray->getField(mlExtendedFieldName);
    RadxField *mleCopy = new RadxField(*mleFld);
    derivedRay->addField(mleCopy);
  }

  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
  // create dbz field flagged in the brightband

  for (size_t iray = 0; iray < _vol.getRays().size(); iray++) {
    
    RadxRay *ray = _vol.getRays()[iray];
    RadxRay *derivedRay = _derivedRays[iray];
    size_t nGates = ray->getNGates();
    
    // find DBZ field
    
    RadxField *dbzField = ray->getField(smoothedDbzFieldName);
    
    // get ML and ht fields
    
    RadxField *mlField = ray->getField(mlFieldName);
    const Radx::si32 *mlVals = mlField->getDataSi32();
    
    RadxField *mleField = ray->getField(mlExtendedFieldName);
    const Radx::si32 *mleVals = NULL;
    if (mleField) {
      mleVals = mleField->getDataSi32();
    }
    
    // make a copy of the DBZ field that we can amend in 
    // the brightband
    
    RadxField *dbzCorrField = new RadxField(*dbzField);
    dbzCorrField->setName("DBZ_CORR");
    TaArray<Radx::fl32> _dbzCorrVals_;
    Radx::fl32 *dbzCorrVals = _dbzCorrVals_.alloc(nGates);
    memcpy(dbzCorrVals, dbzCorrField->getDataFl32(), 
           nGates * sizeof(Radx::fl32));
    
    RadxField *dbzCorrField2 = new RadxField(*dbzField);
    dbzCorrField2->setName("DBZ_CORR2");
    TaArray<Radx::fl32> _dbzCorrVals2_;
    Radx::fl32 *dbzCorrVals2 = _dbzCorrVals2_.alloc(nGates);
    memcpy(dbzCorrVals2, dbzCorrField2->getDataFl32(), 
           nGates * sizeof(Radx::fl32));
    
    // look for melting layer, adjust reflectivity
    
    for (size_t igate = 0; igate < nGates; igate++) {
      if (mleVals && mleVals[igate] > 0) {
        dbzCorrVals2[igate] = 67.5;
      }
      if (mlVals[igate] > 0) {
        dbzCorrVals[igate] = 75.0;
        dbzCorrVals2[igate] = 75.0;
      }
    } // igate
    
    // set the corrected values in the field
    
    dbzCorrField->setDataFl32(nGates, dbzCorrVals, true);
    dbzCorrField2->setDataFl32(nGates, dbzCorrVals2, true);

    // add field to ray
    
    derivedRay->addField(dbzCorrField);
    derivedRay->addField(dbzCorrField2);
    
  } // iray

  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
  // write results to SPDB
  
  if (!_params. melting_layer_write_results_to_spdb) {
    return;
  }
  
  RadxPath path(_vol.getPathInUse());
  string xml;

  xml += RadxXml::writeStartTag("MeltingLayer", 0);

  xml += RadxXml::writeString("file", 1, path.getFile());
  xml += RadxXml::writeBoolean("is_rhi", 1, _vol.checkIsRhi());
  
  xml += RadxXml::writeDouble("percForBottom", 1, 
                              _params.melting_layer_percentile_for_bottom_limit);
  xml += RadxXml::writeDouble("percForTop", 1, 
                              _params.melting_layer_percentile_for_top_limit);

  xml += RadxXml::writeDouble("htMlBottom", 1, htMlBot);
  xml += RadxXml::writeDouble("htMlTop", 1, htMlTop);
  xml += RadxXml::writeDouble("tempBottom", 1, tempBot);
  xml += RadxXml::writeDouble("tempTop", 1, tempTop);
  xml += RadxXml::writeDouble("dBZMedianBelow", 1, medianDbzBelow);
  xml += RadxXml::writeDouble("dBZMedianAbove", 1, medianDbzAbove);
  xml += RadxXml::writeDouble("dBZMeanBelow", 1, meanDbzBelow);
  xml += RadxXml::writeDouble("dBZMeanAbove", 1, meanDbzAbove);
  xml += RadxXml::writeDouble("dBZPerc75Above", 1, perc75DbzAbove);
  
  xml += RadxXml::writeEndTag("MeltingLayer", 0);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Writing melting layer results to SPDB, url: "
         << _params.melting_layer_spdb_output_url << endl;
  }

  DsSpdb spdb;
  time_t validTime = _vol.getStartTimeSecs();
  spdb.addPutChunk(0, validTime, validTime, xml.size() + 1, xml.c_str());
  if (spdb.put(_params.melting_layer_spdb_output_url,
               SPDB_XML_ID, SPDB_XML_LABEL)) {
    cerr << "ERROR - RadxPartRain::_locateMeltingLayer" << endl;
    cerr << spdb.getErrStr() << endl;
    return;
  }
  
  if (_params.debug) {
    cerr << "Wrote melting layer results to spdb, url: " 
         << _params.melting_layer_spdb_output_url << endl;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=====================================" << endl;
    cerr << xml;
    cerr << "=====================================" << endl;
  }

}

/////////////////////////////////////////////////////////////
// identify convective regions from the vertical profile
// and PID

void RadxPartRain::_checkForConvection(double htMlTop)

{
  
  for (size_t irhi = 0; irhi < _pseudoRhis.size(); irhi++) {
    
    PseudoRhi *rhi = _pseudoRhis[irhi];
    const vector<RadxRay *> &rays = rhi->getRays();
    if (rays.size() < 2) {
      continue;
    }
    
    // load up 2D arrays for dbz, height and pid fields
    
    RadxArray2D<Radx::fl32> dbz2D, ht2D;
    RadxArray2D<Radx::si32> pid2D, conv2D;
    
    _vol.load2DFieldFromRays(rhi->getRays(), smoothedDbzFieldName,
                             dbz2D, Radx::missingFl32);
    _vol.load2DFieldFromRays(rhi->getRays(), beamHtFieldName,
                             ht2D, Radx::missingFl32);
    _vol.load2DFieldFromRays(rhi->getRays(), pidFieldName,
                             pid2D, Radx::missingSi32);
    _vol.load2DFieldFromRays(rhi->getRays(), convFlagFieldName,
                             conv2D, Radx::missingSi32);
    
    Radx::fl32 **dbzVals = dbz2D.dat2D();
    Radx::fl32 **htVals = ht2D.dat2D();
    Radx::si32 **pidVals = pid2D.dat2D();
    Radx::si32 **convVals = conv2D.dat2D();
    
    // array dimensions
    
    size_t nRays = dbz2D.sizeMajor();
    size_t maxNGates = dbz2D.sizeMinor();
    
    // check for convection
    
    for (size_t igate = 0; igate < maxNGates; igate++) {
      
      double isConv = false;
      for (size_t iray = 0; iray < nRays; iray++) {
        double dbz = dbzVals[iray][igate];
        double ht = htVals[iray][igate];
        int pid = pidVals[iray][igate];
        if (dbz > 50) {
          isConv = true;
        }
        if (ht > htMlTop * 1.25 && dbz > 45) {
          isConv = true;
        }
        if (ht > htMlTop * 1.5 && dbz > 35) {
          isConv = true;
        }
        if (pid == NcarParticleId::HEAVY_RAIN ||
            pid == NcarParticleId::HAIL ||
            pid == NcarParticleId::RAIN_HAIL_MIXTURE) {
          isConv = true;
        }
      } // iray

      if (isConv) {
        for (size_t iray = 0; iray < nRays; iray++) {
          double dbz = dbzVals[iray][igate];
          if (dbz > 0) {
            convVals[iray][igate] = 1;
          }
        }
      }
    }
      
    // load the convective flag field
    
    _vol.loadRaysFrom2DField(conv2D, rhi->getRays(),
                             convFlagFieldName, "",
                             Radx::missingSi32);

  } // irhi

  // fill in the gaps in the convective field
  // load convective field into derived ray
  
  for (size_t iray = 0; iray < _vol.getRays().size(); iray++) {
    RadxRay *ray = _vol.getRays()[iray];
    size_t nGates = ray->getNGates();
    RadxField *convFld = ray->getField(convFlagFieldName);
    Radx::si32 *convVals = convFld->getDataSi32();
    _applyInfillFilter(nGates, convVals);
    RadxRay *derivedRay = _derivedRays[iray];
    RadxField *convCopy = new RadxField(*convFld);
    derivedRay->addField(convCopy);
  }
  
}

/////////////////////////////////////////////////////////////
// expand the melting layer from the top of the layer to the
// height at which the dbz has dropped to the 75th percentile
// RHI mode

void RadxPartRain::_expandMlRhi(double htMlTop,
                                double dbzThreshold)

{

  // loop through through the RHIs

  for (size_t irhi = 0; irhi < _pseudoRhis.size(); irhi++) {

    PseudoRhi *rhi = _pseudoRhis[irhi];
    const vector<RadxRay *> &rays = rhi->getRays();
    if (rays.size() < 2) {
      continue;
    }
    
    // load up 2D arrays for ml, dbz and height fields

    // RadxArray2D<Radx::fl32> dbz2D;
    RadxArray2D<Radx::fl32> rhohv2D;
    RadxArray2D<Radx::fl32> range2D;
    RadxArray2D<Radx::si32> ml2D, mle2D, conv2D;
    
    // _vol.load2DFieldFromRays(rhi->getRays(), smoothedDbzFieldName,
    //                          dbz2D, Radx::missingFl32);
    _vol.load2DFieldFromRays(rhi->getRays(), smoothedRhohvFieldName,
                             rhohv2D, Radx::missingFl32);
    _vol.load2DFieldFromRays(rhi->getRays(), rangeFieldName,
                             range2D, Radx::missingFl32);
    _vol.load2DFieldFromRays(rhi->getRays(), mlFieldName,
                             ml2D, Radx::missingSi32);
    _vol.load2DFieldFromRays(rhi->getRays(), mlExtendedFieldName,
                             mle2D, Radx::missingSi32);
    _vol.load2DFieldFromRays(rhi->getRays(), convFlagFieldName,
                             conv2D, Radx::missingSi32);
    
    // Radx::fl32 **dbzVals = dbz2D.dat2D();
    Radx::fl32 **rhohvVals = rhohv2D.dat2D();
    Radx::fl32 **rangeVals = range2D.dat2D();
    Radx::si32 **mlVals = ml2D.dat2D();
    Radx::si32 **mleVals = mle2D.dat2D();
    Radx::si32 **convVals = conv2D.dat2D();

    // array dimensions
    
    size_t nRays = ml2D.sizeMajor();
    size_t maxNGates = ml2D.sizeMinor();
    
    // copy extended flag from ml flag
    
    memcpy(mle2D.dat1D(), ml2D.dat1D(), ml2D.size1D() * sizeof(Radx::si32));
    
    // loop through range gates

    for (size_t igate = 0; igate < maxNGates; igate++) {
      
      // find the mean ray index for the melting layer
      
      double sumRayIndex = 0.0;
      double rayCount = 0.0;
      for (size_t iray = 0; iray < nRays; iray++) {
        if (mlVals[iray][igate] > 0) {
          sumRayIndex += iray;
          rayCount++;
        }
      } // iray
      if (rayCount < 2) {
        // melting layer not well defined
        continue;
      }
      int meanRayIndex = (int) (sumRayIndex / rayCount + 0.5);

      // serch through the rays, starting at the middle of the
      // melting layer, and moving up,
      // until the reflectivity falls below a threshold, or
      // the rhohv exceeds a threshoold
      
      bool endFound = false;
      int indexEnd = -1;
      
      for (size_t iray = meanRayIndex; iray < nRays; iray++) {
        if (convVals[iray][igate] != 0) {
          break;
        }
        // if (dbzVals[iray][igate] < dbzThreshold) {
        //   endFound = true;
        //   indexEnd = iray;
        //   break;
        // }
        double rhohvThreshold =
          0.995 - rangeVals[iray][igate] *  0.000065;
        if (rhohvVals[iray][igate] < 0 || 
            rhohvVals[iray][igate] > rhohvThreshold) {
          endFound = true;
          indexEnd = iray;
          break;
        }
      } // iray

      // extend the ml index up to the end ray found above

      if (endFound) {
        for (int iray = meanRayIndex; iray <= indexEnd; iray++) {
          if (mlVals[iray][igate] == 0) {
            mleVals[iray][igate] = 1;
          }
        }
      }
      
    } // igate
    
    // load the extended ml data field and convective flag field
    
    _vol.loadRaysFrom2DField(mle2D, rhi->getRays(),
                             mlExtendedFieldName, "",
                             Radx::missingSi32);
    
  } // irhi

}

/////////////////////////////////////////////////////////////
// expand the melting layer from the top of the layer to the
// height at which the dbz has dropped to the 75th percentile
// PPI mode

void RadxPartRain::_expandMlPpi(double htMlTop,
                                double dbzThreshold)

{

  for (size_t iray = 0; iray < _vol.getRays().size(); iray++) {
    
    RadxRay *ray = _vol.getRays()[iray];
    size_t nGates = ray->getNGates();
    
    // Get fields
    
    // RadxField *dbzField = ray->getField(smoothedDbzFieldName);
    // const Radx::fl32 *dbzVals = dbzField->getDataFl32();
    
    RadxField *rhohvField = ray->getField(smoothedRhohvFieldName);
    const Radx::fl32 *rhohvVals = rhohvField->getDataFl32();
    
    RadxField *mlField = ray->getField(mlFieldName);
    const Radx::si32 *mlVals = mlField->getDataSi32();
    
    RadxField *mleField = ray->getField(mlExtendedFieldName);
    Radx::si32 *mleVals = mleField->getDataSi32();
    
    RadxField *convField = ray->getField(convFlagFieldName);
    Radx::si32 *convVals = convField->getDataSi32();
    
    RadxField *rangeField = ray->getField(rangeFieldName);
    const Radx::fl32 *rangeVals = rangeField->getDataFl32();

    // find the mean gate for the melting layer

    double sumGateIndex = 0.0;
    double gateCount = 0.0;
    for (size_t igate = 0; igate < nGates; igate++) {
      if (mlVals[igate] > 0) {
        sumGateIndex += igate;
        gateCount++;
      }
    } // igate
    if (gateCount < 10) {
      // melting layer not well defined
      continue;
    }
    int meanGateIndex = (int) (sumGateIndex / gateCount + 0.5);
    
    bool endFound = false;
    int indexEnd = -1;
    
    for (size_t igate = meanGateIndex; igate < nGates; igate++) {
      if (convVals[igate] != 0) {
        break;
      }
      // if (dbzVals[igate] < dbzThreshold) {
      //   indexEnd = igate;
      //   endFound = true;
      //   break;
      // }
      double rhohvThreshold = 0.997 - rangeVals[igate] *  0.00005;
      if (rhohvVals[igate] < 0 ||
          rhohvVals[igate] > rhohvThreshold) {
        indexEnd = igate;
        endFound = true;
        break;
      }
    } // iray
    
    if (endFound) {
      // extend the ml index up to where the dbz drops below threshold
      for (int igate = meanGateIndex + 1; igate <= indexEnd; igate++) {
        if (mlVals[igate] == 0) {
          mleVals[igate] = 1;
        }
      }
    }
    
  } // iray
    
}

/////////////////////////////////////////////////////////////
// apply in-fill filter to flag field

void RadxPartRain::_applyInfillFilter(int nGates,
                                      Radx::si32 *flag)

{

  // we apply the basic filter 3 times, and on the third time
  // we remove any short runs
  
  _applyInfillFilter(nGates, flag, false);
  _applyInfillFilter(nGates, flag, false);
  _applyInfillFilter(nGates, flag, true);

}

/////////////////////////////////////////////////////////////
// apply in-fill filter to flag field

void RadxPartRain::_applyInfillFilter(int nGates,
                                      Radx::si32 *flag,
                                      bool removeShortRuns)
  
{

  // compute the count of gates, within a range interval, that
  // have the flag set

  int halfLen = 10;
  int filtLen = halfLen * 2 + 1;
  if (filtLen >= nGates) {
    return;
  }

  // initialize
  
  vector<int> countSet;
  countSet.resize(nGates);
  for (int igate = 0; igate < nGates; igate++) {
    countSet[igate] = 0;
  }

  // load counts at beginning

  int count = 0;
  for (int igate = 0; igate < filtLen; igate++) {
    if (flag[igate] > 0) {
      count++;
      countSet[igate] = count;
    }
  }
 
  // load counts in main part of ray

  for (int igate = filtLen; igate < nGates; igate++) {
    if (flag[igate] > 0) {
      count++;
    }
    if (flag[igate - filtLen] > 0) {
      count--;
    }
    countSet[igate - halfLen] = count;
  }

  // load counts at end

  count = 0;
  for (int igate = nGates - 1; igate > nGates - halfLen - 1; igate--) {
    if (flag[igate] > 0) {
      count++;
      countSet[igate] = count;
    }
  }

  // set flag true if count exceeds half len at any point

  for (int igate = 0; igate < nGates; igate++) {
    if (countSet[igate] > halfLen) {
      flag[igate] = 1;
    }
  }

  // set flag false if count is below 5

  if (removeShortRuns) {
    for (int igate = 0; igate < nGates; igate++) {
      if (countSet[igate] < 5) {
        flag[igate] = 0;
      }
    }
  }

}

///////////////////////////////////////////////////////////////
// ComputeThread

// Constructor

RadxPartRain::ComputeThread::ComputeThread(RadxPartRain *obj,
                                           const Params &params,
                                           int threadNum) :
        _this(obj),
        _params(params),
        _threadNum(threadNum)
{

  OK = TRUE;
  _inputRay = NULL;
  _derivedRay = NULL;

  // create compute engine object
  
  _engine = new ComputeEngine(_params, _threadNum);
  if (_engine == NULL) {
    OK = FALSE;
    return;
  }
  if (!_engine->OK) {
    OK = FALSE;
    _engine = NULL;
    return;
  }

}  

// Destructor

RadxPartRain::ComputeThread::~ComputeThread()
{

  if (_engine != NULL) {
    delete _engine;
  }

}  

// run method

void RadxPartRain::ComputeThread::run()
{

  // check

  assert(_engine != NULL);
  assert(_inputRay != NULL);
  
  // Compute engine object will create the derived ray
  // The ownership of the ray is passed to the parent object
  // which adds it to the output volume.

  _derivedRay = _engine->compute(_inputRay,
                                 _this->_radarHtKm,
                                 _this->_wavelengthM,
                                 &_this->_tempProfile);

}

