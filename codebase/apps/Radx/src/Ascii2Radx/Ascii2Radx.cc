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
// Ascii2Radx.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2016
//
///////////////////////////////////////////////////////////////

#include "Ascii2Radx.hh"
#include <Radx/Radx.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/TaStr.hh>
using namespace std;

// Constructor

Ascii2Radx::Ascii2Radx(int argc, char **argv)
  
{

  OK = TRUE;
  _volNum = 0;
  _inFile = NULL;

  // set programe name

  _progName = "Ascii2Radx";
  ucopyright((char *) _progName.c_str());
  
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

  // check we have an output field

  if (_params.output_fields_n < 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem with TDRP parameters." << endl;
    cerr << "  You must specify at least 1 output field" << endl;
    cerr << "  See output_fields[] parameter" << endl;
    OK = FALSE;
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

}

// destructor

Ascii2Radx::~Ascii2Radx()

{

  if (_inFile != NULL) {
    fclose(_inFile);
    _inFile = NULL;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Ascii2Radx::Run()
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

int Ascii2Radx::_runFilelist()
{
  
  int iret = 0;

  if (_params.debug) {
    cerr << "Running Ascii2Radx" << endl;
    cerr << "  n input files: " << _args.inputFileList.size() << endl;
  }

  int nGood = 0;
  int nError = 0;
  
  if (!_params.aggregate_all_files_on_read) {

    // loop through the input file list
    
    RadxVol vol;
    for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {
      _sweepNum = 0;
      string inputPath = _args.inputFileList[ii];
      // read input file
      int jret = _readFile(inputPath, vol);
      if (jret == 0) {
        // finalize the volume
        _finalizeVol(vol);
        // write the volume out
        if (_writeVol(vol)) {
          cerr << "ERROR - Ascii2Radx::_runFileList" << endl;
          cerr << "  Cannot write volume to file" << endl;
          iret = -1;
          nError++;
          if (_params.debug) {
            cerr << "  ====>> n errors so far: " << nError << endl;
          }
        } else {
          nGood++;
          if (_params.debug) {
            cerr << "  ====>> n good files so far: " << nGood << endl;
            cerr << "  ====>> n errors     so far: " << nError << endl;
            cerr << "  ====>> sum          so far: " << nGood + nError << endl;
          }
        }
      } else if (jret < 0) {
        iret = -1;
        nError++;
        if (_params.debug) {
          cerr << "  ====>> n errors so far: " << nError << endl;
        }
      }
      // free up
      vol.clear();
    }

  } else {
    
    // aggregate the files into a single volume on read
    
    _sweepNum = 0;
    RadxVol vol;
    GenericRadxFile inFile;
    vector<string> paths = _args.inputFileList;
    if (inFile.aggregateFromPaths(paths, vol)) {
      cerr << "ERROR - Ascii2Radx::_runFileList" << endl;
      cerr << "  paths: " << endl;
      for (size_t ii = 0; ii < paths.size(); ii++) {
        cerr << "         " << paths[ii] << endl;
      }
      return -1;
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      for (size_t ii = 0; ii < paths.size(); ii++) {
        cerr << "==>> read in file: " << paths[ii] << endl;
      }
    }
    
    // finalize the volume
    
    _finalizeVol(vol);
    
    // write the volume out
    if (_writeVol(vol)) {
      cerr << "ERROR - Ascii2Radx::_runFileList" << endl;
      cerr << "  Cannot write aggregated volume to file" << endl;
      iret = -1;
    }

    nGood++;
    
  } // if (!_params.aggregate_all_files_on_read) {

  if (_params.debug) {
    cerr << "Ascii2Radx done" << endl;
    cerr << "====>> n good files processed: " << nGood << endl;
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int Ascii2Radx::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (_params.aggregate_sweep_files_on_read) {
    tlist.setReadAggregateSweeps(true);
  }
  if (tlist.compile()) {
    cerr << "ERROR - Ascii2Radx::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - Ascii2Radx::_runFilelist()" << endl;
    cerr << "  No files found, dir: " << _params.input_dir << endl;
    return -1;
  }
  
  // loop through the input file list

  RadxVol vol;
  int iret = 0;
  for (size_t ii = 0; ii < paths.size(); ii++) {
    // read input file
    int jret = _readFile(paths[ii], vol);
    if (jret == 0) {
      // finalize the volume
      _finalizeVol(vol);
      // write the volume out
      if (_writeVol(vol)) {
        cerr << "ERROR - Ascii2Radx::_runArchive" << endl;
        cerr << "  Cannot write volume to file" << endl;
        return -1;
      }
    } else if (jret < 0) {
      iret = -1;
    }
    // free up
    vol.clear();
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode with latest data info

int Ascii2Radx::_runRealtimeWithLdata()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  // watch for new data to arrive

  LdataInfo ldata(_params.input_dir,
                  _params.debug >= Params::DEBUG_VERBOSE);
  if (strlen(_params.search_ext) > 0) {
    ldata.setDataFileExt(_params.search_ext);
  }

  RadxVol vol;
  int iret = 0;
  int msecsWait = _params.wait_between_checks * 1000;
  while (true) {
    ldata.readBlocking(_params.max_realtime_data_age_secs,
                       msecsWait, PMU_auto_register);
    const string path = ldata.getDataPath();
    // read input file
    int jret = _readFile(path, vol);
    if (jret == 0) {
      // finalize the volume
      _finalizeVol(vol);
      // write the volume out
      if (_writeVol(vol)) {
        cerr << "ERROR - Ascii2Radx::_runRealtimeWithLdata" << endl;
        cerr << "  Cannot write volume to file" << endl;
        return -1;
      }
    } else if (jret < 0) {
      iret = -1;
    }
    // free up
    vol.clear();
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode without latest data info

int Ascii2Radx::_runRealtimeNoLdata()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);
  
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
  RadxVol vol;

  while(true) {

    // check for new data
    
    char *path = input.next(false);
    
    if (path == NULL) {
      
      // sleep a bit
      
      PMU_auto_register("Waiting for data");
      umsleep(_params.wait_between_checks * 1000);

    } else {

      // read the input file
      
      int jret = _readFile(path, vol);
      if (jret == 0) {
        // finalize the volume
        _finalizeVol(vol);
        // write the volume out
        if (_writeVol(vol)) {
          cerr << "ERROR - Ascii2Radx::_runRealtimeNoLdata" << endl;
          cerr << "  Cannot write volume to file" << endl;
          return -1;
        }
      } else if (jret < 0) {
        iret = -1;
      }

      // free up
      vol.clear();
  
    }

  } // while

  return iret;

}

//////////////////////////////////////////////////
// Read in a file
// accounting for special cases such as gematronik
// Returns 0 on success
//         1 if already read,
//         -1 on failure

int Ascii2Radx::_readFile(const string &readPath,
                          RadxVol &vol)
{

  PMU_auto_register("Processing file");

  // clear all data on volume object

  vol.clear();

  // check we have not already processed this file
  // in the file aggregation step
  
  RadxPath thisPath(readPath);
  for (size_t ii = 0; ii < _readPaths.size(); ii++) {
    RadxPath listPath(_readPaths[ii]);
    if (thisPath.getFile() == listPath.getFile()) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Skipping file: " << readPath << endl;
        cerr << "  Previously processed in aggregation step" << endl;
      }
      return 1;
    }
  }
  
  if (_params.debug) {
    cerr << "INFO - Ascii2Radx::Run" << endl;
    cerr << "  Input path: " << readPath << endl;
  }

  // open the file

  if ((_inFile = fopen(readPath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Ascii2Radx::_readFile" << endl;
    cerr << "  path: " << readPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if (_params.input_type == Params::BUFR_ASCII) {
    if (_readBufrAscii(readPath, vol)) {
      cerr << "ERROR - Ascii2Radx::_readFile" << endl;
      cerr << "  path: " << readPath << endl;
      fclose(_inFile);
      return -1;
    }
  }

  // close the file
  
  fclose(_inFile);

  // append the read path

  _readPaths.push_back(readPath);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t ii = 0; ii < _readPaths.size(); ii++) {
      cerr << "  ==>> read in file: " << _readPaths[ii] << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// Read in a BUFR ASCII file
// Returns 0 on success, -1 on failure

int Ascii2Radx::_readBufrAscii(const string &readPath,
                               RadxVol &vol)
{

  // read in the metadata
  
  if (_readBufrMetaData()) {
    cerr << "ERROR - Ascii2Radx::_readFile" << endl;
    cerr << "  path: " << readPath << endl;
    cerr << "  cannot read metadata" << endl;
    fclose(_inFile);
    return -1;
  }

  // read in the field data
  
  if (_readBufrFieldData()) {
    cerr << "ERROR - Ascii2Radx::_readFile" << endl;
    cerr << "  path: " << readPath << endl;
    cerr << "  cannot read metadata" << endl;
    fclose(_inFile);
    return -1;
  }

  // create the rays

  double deltaAz = 0.0;
  double az = _startAz;
  for (int iray = 0; iray < _nAz; iray++) {

    // create a ray
    
    RadxRay *ray = new RadxRay;

    // set metadata

    ray->setVolumeNumber(_volNum);
    ray->setSweepNumber(_sweepNum);
    ray->setCalibIndex(0);
    ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);

    RadxTime rayTime = _volStartTime + deltaAz / _antennaSpeedAz;
    deltaAz += _azimuthResDeg;
    ray->setTime(rayTime);

    ray->setAzimuthDeg(az);
    az += _azimuthResDeg;
    if (az > 360.0) {
      az -= 360.0;
    } else if (az < 0) {
      az += 360.0;
    }
    
    ray->setElevationDeg(_elevDeg);
    ray->setFixedAngleDeg(_elevDeg);

    ray->setTrueScanRateDegPerSec(_antennaSpeedAz);
    ray->setTargetScanRateDegPerSec(_antennaSpeedAz);
    ray->setIsIndexed(true);
    ray->setAngleResDeg(_azimuthResDeg);

    ray->setNSamples(_nSamples);
    ray->setPulseWidthUsec(_pulseWidthSec * 1.0e6);
    ray->setPrtSec(1.0 / _prf);

    ray->setEstimatedNoiseDbmHc(_noiseLevelDbm);
    ray->setEstimatedNoiseDbmVc(_noiseLevelDbm);

    // add field

    ray->addField(_params._output_fields[0].field_name,
                  _params._output_fields[0].units,
                  _nGates, Radx::missingFl64,
                  _fieldData + iray * _nGates, true);

    ray->setRangeGeom(_startRangeM / 1000.0, _gateSpacingM / 1000.0);

    ray->convertToSi16();

    // add to volume

    vol.addRay(ray);

  } // iray

  // set the metadata on the volume

  _finalizeVol(vol);

  return 0;

}


//////////////////////////////////////////////////
// Finalize the volume based on parameters
// Returns 0 on success, -1 on failure

void Ascii2Radx::_finalizeVol(RadxVol &vol)
  
{

  vol.setVolumeNumber(_volNum);

  vol.setStartTime(_volStartTime.utime(), _volStartTime.getSubSec());

  vol.setLatitudeDeg(_latitude);
  vol.setLongitudeDeg(_longitude);
  vol.setAltitudeKm(_altitudeM / 1000.0);

  vol.setFrequencyHz(_frequencyHz);
  vol.setRadarBeamWidthDegH(_beamWidth);
  vol.setRadarBeamWidthDegV(_beamWidth);
  vol.setRadarAntennaGainDbH(_antennaGain);
  vol.setRadarAntennaGainDbV(_antennaGain);
  vol.setRadarReceiverBandwidthMhz(_rxBandWidthHz / 1.0e6);

  if (_params.set_max_range) {
    vol.setMaxRangeKm(_params.max_range_km);
  }

  // override start range and/or gate spacing

  if (_params.override_start_range || _params.override_gate_spacing) {
    vol.remapRangeGeom(_params.start_range_km, _params.gate_spacing_km);
  }

  // remap geometry as applicable

  if (_params.remap_to_predominant_range_geometry) {
    vol.remapToPredomGeom();
  }
  if (_params.remap_to_finest_range_geometry) {
    vol.remapToFinestGeom();
  }

  // override radar location if requested
  
  if (_params.override_radar_location) {
    vol.overrideLocation(_params.radar_latitude_deg,
                         _params.radar_longitude_deg,
                         _params.radar_altitude_meters / 1000.0);
  }
    
  // override radar name and site name if requested
  
  if (_params.override_instrument_name) {
    vol.setInstrumentName(_params.instrument_name);
  }
  if (_params.override_site_name) {
    vol.setSiteName(_params.site_name);
  }
    
  // apply time offset

  if (_params.apply_time_offset) {
    if (_params.debug) {
      cerr << "NOTE: applying time offset (secs): " 
           << _params.time_offset_secs << endl;
    }
    vol.applyTimeOffsetSecs(_params.time_offset_secs);
  }

  // apply angle offsets

  if (_params.apply_azimuth_offset) {
    if (_params.debug) {
      cerr << "NOTE: applying azimuth offset (deg): " 
           << _params.azimuth_offset << endl;
    }
    vol.applyAzimuthOffset(_params.azimuth_offset);
  }
  if (_params.apply_elevation_offset) {
    if (_params.debug) {
      cerr << "NOTE: applying elevation offset (deg): " 
           << _params.elevation_offset << endl;
    }
    vol.applyElevationOffset(_params.elevation_offset);
  }

  // sweep angles

  if (_params.debug) {
    cerr << "DEBUG - computing sweep fixed angles from ray data" << endl;
  }
  vol.computeFixedAnglesFromRays();

  // sweep limits
  
  if (_params.adjust_sweep_limits_using_angles) {
    if (_params.debug) {
      cerr << "DEBUG - adjusting sweep limits using angles" << endl;
    }
    vol.adjustSweepLimitsUsingAngles();
  }

  if (_params.set_fixed_angle_limits) {
    vol.constrainByFixedAngle(_params.lower_fixed_angle_limit,
                              _params.upper_fixed_angle_limit);
  } else if (_params.set_sweep_num_limits) {
    vol.constrainBySweepNum(_params.lower_sweep_num,
                            _params.upper_sweep_num);
  }

  // set number of gates constant if requested

  if (_params.set_ngates_constant) {
    if (_params.debug) {
      cerr << "DEBUG - setting nGates constant" << endl;
    }
    vol.setNGatesConstant();
  }

  // optimize transitions in surveillance mode

  if (_params.optimize_surveillance_transitions) {
    vol.optimizeSurveillanceTransitions(_params.optimized_transitions_max_elev_error);
  }

  // trim to 360s if requested

  if (_params.trim_surveillance_sweeps_to_360deg) {
    if (_params.debug) {
      cerr << "DEBUG - trimming surveillance sweeps to 360 deg" << endl;
    }
    vol.trimSurveillanceSweepsTo360Deg();
  }

  // set field type, names, units etc
  
  _convertFields(vol);

  // load sweep and/or volumen info from rays

  vol.loadSweepInfoFromRays();
  vol.loadVolumeInfoFromRays();

  // volume number
  
  vol.setVolumeNumber(_volNum);
  _volNum++;

  // set global attributes

  _setGlobalAttr(vol);

}

//////////////////////////////////////////////////
// rename fields as required

void Ascii2Radx::_convertFields(RadxVol &vol)
{

  for (int ii = 0; ii < _params.output_fields_n; ii++) {

    const Params::output_field_t &ofld = _params._output_fields[ii];
    
    string fname = ofld.field_name;
    string lname = ofld.long_name;
    string sname = ofld.standard_name;
    string units = ofld.units;
    
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
      vol.convertField(fname, dtype, 
                       fname, units, sname, lname);
    } else {
      vol.convertField(fname, dtype, 
                       ofld.output_scale, ofld.output_offset,
                       fname, units, sname, lname);
    }
    
  }

}

//////////////////////////////////////////////////
// set up write

void Ascii2Radx::_setupWrite(RadxFile &file)
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
    case Params::OUTPUT_FORMAT_NSSL_MRD:
      file.setFileFormat(RadxFile::FILE_FORMAT_NSSL_MRD);
      break;
    case Params::OUTPUT_FORMAT_ODIM_HDF5:
      file.setFileFormat(RadxFile::FILE_FORMAT_ODIM_HDF5);
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

  if (strlen(_params.output_filename_prefix) > 0) {
    file.setWriteFileNamePrefix(_params.output_filename_prefix);
  }
  if (strlen(_params.output_filename_suffix) > 0) {
    file.setWriteFileNameSuffix(_params.output_filename_suffix);
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

void Ascii2Radx::_setGlobalAttr(RadxVol &vol)
{

  vol.setDriver("Ascii2Radx(NCAR)");

  if (strlen(_params.version) > 0) {
    vol.setVersion(_params.version);
  }

  if (strlen(_params.title) > 0) {
    vol.setTitle(_params.title);
  }

  if (strlen(_params.institution) > 0) {
    vol.setInstitution(_params.institution);
  }

  if (strlen(_params.references) > 0) {
    vol.setReferences(_params.references);
  }

  if (strlen(_params.source) > 0) {
    vol.setSource(_params.source);
  }

  if (strlen(_params.history) > 0) {
    vol.setHistory(_params.history);
  }

  if (strlen(_params.author) > 0) {
    vol.setAuthor(_params.author);
  }

  if (strlen(_params.comment) > 0) {
    vol.setComment(_params.comment);
  }

  RadxTime now(RadxTime::NOW);
  vol.setCreated(now.asString());

  if (_params.add_user_specified_global_attributes) {
    for (int ii = 0; ii < _params.user_defined_global_attributes_n; ii++) {
      Params::attr_t attr = _params._user_defined_global_attributes[ii];
      RadxVol::UserGlobAttr::attr_type_t attrType = 
        RadxVol::UserGlobAttr::ATTR_STRING;
      switch (attr.attrType) {
        case Params::ATTR_STRING:
          attrType = RadxVol::UserGlobAttr::ATTR_STRING;
          break;
        case Params::ATTR_INT:
          attrType = RadxVol::UserGlobAttr::ATTR_INT;
          break;
        case Params::ATTR_DOUBLE:
          attrType = RadxVol::UserGlobAttr::ATTR_DOUBLE;
          break;
        case Params::ATTR_INT_ARRAY:
          attrType = RadxVol::UserGlobAttr::ATTR_INT_ARRAY;
          break;
        case Params::ATTR_DOUBLE_ARRAY:
          attrType = RadxVol::UserGlobAttr::ATTR_DOUBLE_ARRAY;
          break;
      } // switch
      vol.addUserGlobAttr(attr.name, attrType, attr.val);
    } // ii
  } // if (_params.add_user_specified_global_attributes) {

}

//////////////////////////////////////////////////
// write out the volume

int Ascii2Radx::_writeVol(RadxVol &vol)
{

  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);

  string outputDir = _params.output_dir;

  if (_params.separate_output_dirs_by_scan_type) {
    outputDir += PATH_DELIM;
    Radx::SweepMode_t sweepMode = vol.getPredomSweepMode();
    switch (sweepMode) {
      case Radx::SWEEP_MODE_RHI:
        outputDir += _params.rhi_subdir;
        break;
      case Radx::SWEEP_MODE_SECTOR:
        outputDir += _params.sector_subdir;
        break;
      case Radx::SWEEP_MODE_VERTICAL_POINTING:
        outputDir += _params.vert_subdir;
        break;
      case Radx::SWEEP_MODE_SUNSCAN:
      case Radx::SWEEP_MODE_SUNSCAN_RHI:
        outputDir += _params.sun_subdir;
        break;
      default:
        outputDir += _params.surveillance_subdir;
    }
  }
    
  if (_params.output_filename_mode == Params::SPECIFY_FILE_NAME) {
    
    string outPath = outputDir;
    outPath += PATH_DELIM;
    outPath += _params.output_filename;

    // write to path
  
    if (outFile.writeToPath(vol, outPath)) {
      cerr << "ERROR - Ascii2Radx::_writeVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
    
  } else {

    // write to dir
  
    if (outFile.writeToDir(vol, outputDir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - Ascii2Radx::_writeVol" << endl;
      cerr << "  Cannot write file to dir: " << outputDir << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }

  }

  string outputPath = outFile.getPathInUse();

  // write latest data info file if requested 
  
  if (_params.write_latest_data_info) {
    DsLdataInfo ldata(outputDir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    string relPath;
    RadxPath::stripDir(outputDir, outputPath, relPath);
    ldata.setRelDataPath(relPath);
    ldata.setWriter(_progName);
    if (ldata.write(vol.getEndTimeSecs())) {
      cerr << "WARNING - Ascii2Radx::_writeVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << outputDir << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// read in the BUFR metadata

int Ascii2Radx::_readBufrMetaData()
{

  _year = _month = _day = _hour = _min = _sec = 0;
  _readBufrMetaVariable("Year", _year);
  _readBufrMetaVariable("Month", _month);
  _readBufrMetaVariable("Day", _day);
  _readBufrMetaVariable("Hour", _hour);
  _readBufrMetaVariable("Minute", _min);
  _readBufrMetaVariable("Second", _sec);
  _volStartTime.set(_year, _month, _day, _hour, _min, _sec);

  _latitude = 0.0;
  _readBufrMetaVariable("Latitude (high accuracy)", _latitude);
  if (_params.change_radar_latitude_sign) {
    _latitude *= -1.0;
  }

  _longitude = 0.0;
  _readBufrMetaVariable("Longitude (high accuracy)", _longitude);

  _altitudeM = 0.0;
  _readBufrMetaVariable("Height or altitude", _altitudeM);

  _antennaGain = Radx::missingFl64;
  _readBufrMetaVariable("Maximum antenna gain", _antennaGain);

  _beamWidth = Radx::missingFl64;
  _readBufrMetaVariable("3-dB beamwidth", _beamWidth);

  _antennaSpeedAz = Radx::missingFl64;
  _readBufrMetaVariable("Antenna speed (azimuth)", _antennaSpeedAz);

  _antennaSpeedEl = Radx::missingFl64;
  _readBufrMetaVariable("Antenna speed (elevation)", _antennaSpeedEl);

  _frequencyHz = Radx::missingFl64;
  _readBufrMetaVariable("Mean frequency", _frequencyHz);

  _peakPowerWatts = Radx::missingFl64;
  _readBufrMetaVariable("Peak power", _peakPowerWatts);

  _prf = Radx::missingFl64;
  _readBufrMetaVariable("Pulse repetition frequency", _prf);

  _pulseWidthSec = Radx::missingFl64;
  _readBufrMetaVariable("Pulse width", _pulseWidthSec);

  _rxBandWidthHz = Radx::missingFl64;
  _readBufrMetaVariable("Intermediate frequency bandwidth", _rxBandWidthHz);

  _noiseLevelDbm = Radx::missingFl64;
  _readBufrMetaVariable("Minimum detectable signal", _noiseLevelDbm);

  _dynamicRangeDb = Radx::missingFl64;
  _readBufrMetaVariable("Dynamic range", _dynamicRangeDb);

  _gateSpacingM = 1000.0;
  _readBufrMetaVariable("Longueur de la porte distance apres integration", _gateSpacingM);
  _startRangeM = _gateSpacingM / 2.0;

  _azimuthResDeg = 0.5;
  _readBufrMetaVariable("Increment de l'azimut entre chaque tir de l'image polaire", _azimuthResDeg);

  _nSamples = 1;
  _readBufrMetaVariable("Number of integrated pulses", _nSamples);

  _radarConstant = Radx::missingFl64;
  _readBufrMetaVariable("Constante radar", _radarConstant);

  _elevDeg = 0.0;
  _readBufrMetaVariable("Antenna elevation", _elevDeg);

  _startAz = 0.0;
  _readBufrMetaVariable("Antenna beam azimuth", _startAz);
  _startAz += _azimuthResDeg / 2.0;

  _nGates = 0;
  _readBufrMetaVariable("Number of pixels per row", _nGates);

  _nAz = 0;
  _readBufrMetaVariable("Number of pixels per column", _nAz);

  if(_params.debug) {

    cerr << "  _volStartTime: " << _volStartTime.asString(3) << endl;
    cerr << "  _latitude: " << _latitude << endl;
    cerr << "  _longitude: " << _longitude << endl;
    cerr << "  _altitudeM: " << _altitudeM << endl;
    cerr << "  _antennaGain: " << _antennaGain << endl;
    cerr << "  _beamWidth: " << _beamWidth << endl;
    cerr << "  _antennaSpeedAz: " << _antennaSpeedAz << endl;
    cerr << "  _antennaSpeedEl: " << _antennaSpeedEl << endl;
    cerr << "  _frequencyHz: " << _frequencyHz << endl;
    cerr << "  _peakPowerWatts: " << _peakPowerWatts << endl;
    cerr << "  _prf: " << _prf << endl;
    cerr << "  _pulseWidthSec: " << _pulseWidthSec << endl;
    cerr << "  _noiseLevelDbm: " << _noiseLevelDbm << endl;
    cerr << "  _dynamicRangeDb: " << _dynamicRangeDb << endl;
    cerr << "  _gateSpacingM: " << _gateSpacingM << endl;
    cerr << "  _startRangeM: " << _startRangeM << endl;
    cerr << "  _azimuthResDeg: " << _azimuthResDeg << endl;
    cerr << "  _radarConstant: " << _radarConstant << endl;
    cerr << "  _elevDeg: " << _elevDeg << endl;
    cerr << "  _startAz: " << _startAz << endl;
    cerr << "  _nSamples: " << _nSamples << endl;
    cerr << "  _nGates: " << _nGates << endl;
    cerr << "  _nAz: " << _nAz << endl;

  }

  return 0;

}

//////////////////////////////////////////////////
// read in the BUFR field data

int Ascii2Radx::_readBufrFieldData()
{

  // read in number of total points (az * gates)
  // this also positions the file pointer to read the data

  _nPtsData = 0;
  if (_readBufrMetaVariable("Facteur super elargi de repetition differe du descripteur",
                            _nPtsData)) {
    cerr << "ERROR - Ascii2Radx::_readBufrFieldData()" << endl;
    cerr << "  Cannot find variable for _nPtsData" << endl;
    return -1;
  }

  // read in data values

  _fieldData = _fieldData_.alloc(_nPtsData);
  for (int ii = 0; ii < _nPtsData; ii++) {
    double dval;
    if (_readBufrDataValue("Pixel value", dval)) {
      cerr << "ERROR - Ascii2Radx::_readBufrFieldData()" << endl;
      cerr << "  Cannot read data value, ii: " << ii << endl;
      return -1;
    }
    _fieldData[ii] = dval;
  }

  return 0;

}
//////////////////////////////////////////////////
// read in the metadata variable - integer
// if precedingLabel is not zero len, we first look for that
// and then read in the variable
// returns 0 on success, -1 on failure

int Ascii2Radx::_readBufrMetaVariable(string varLabel, int &ival,
                                      string precedingLabel)
{
  double dval;
  if (_readBufrMetaVariable(varLabel, dval, precedingLabel)) {
    return -1;
  }
  ival = (int) floor(dval + 0.5);
  return 0;
}
  
//////////////////////////////////////////////////
// read in the metadata variable
// if precedingLabel is not zero len, we first look for that
// and then read in the variable
// returns 0 on success, -1 on failure

int Ascii2Radx::_readBufrMetaVariable(string varLabel, double &dval,
                                      string precedingLabel)
{
  
  // rewind file

  rewind(_inFile);

  // check for preceding label

  if (precedingLabel.size() > 0) {
    while (!feof(_inFile)) {
      char line[1024];
      if (fgets(line, 1024, _inFile) == NULL) {
        continue;
      }
      if (strstr(line, precedingLabel.c_str()) != NULL) {
        break;
      }
    }
  }

  // read in each line
  
  while (!feof(_inFile)) {

    // get next line

    char line[1024];
    if (fgets(line, 1024, _inFile) == NULL) {
      continue;
    }

    // check if this line has required label
    
    if (strstr(line, varLabel.c_str()) == NULL) {
      continue;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Decoding metadata variable, label: " << varLabel << endl;
      cerr << line;
    }
      
    // tokenize the line

    vector<string> toks;
    TaStr::tokenize(line, " ", toks);
    if (toks.size() < 1) {
      cerr << "ERROR - Ascii2Radx::_readBufrMetaVariable" << endl;
      cerr << "  Bad string for label: " << varLabel << endl;
      cerr << "  line: " << line;
      return -1;
    }

    // find first token with '.' - i.e. floating point number

    for (size_t itok = 0; itok < toks.size(); itok++) {

      string &valStr = toks[itok];
      if (valStr.find(".") == string::npos) {
        continue;
      }

      if (sscanf(valStr.c_str(), "%lg", &dval) == 1) {
        return 0;
      }
      
      cerr << "ERROR - Ascii2Radx::_readBufrMetaVariable" << endl;
      cerr << "  Bad string for label: " << varLabel << endl;
      cerr << "  line: " << line;
      cerr << "  valStr: " << valStr << endl;
      return -1;
    
    } // itok
    
  } // while

  return -1;

}

//////////////////////////////////////////////////
// read in data value
// returns 0 on success, -1 on failure

int Ascii2Radx::_readBufrDataValue(string varLabel, double &dval)

{
  
  // read in each line
  
  while (!feof(_inFile)) {

    // get next line

    char line[1024];
    if (fgets(line, 1024, _inFile) == NULL) {
      continue;
    }

    // check if this line has required label
    
    if (strstr(line, varLabel.c_str()) == NULL) {
      continue;
    }

    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Decoding data value, label: " << varLabel << endl;
      cerr << line;
    }
      
    // tokenize the line

    vector<string> toks;
    TaStr::tokenize(line, " ", toks);
    if (toks.size() < 1) {
      cerr << "ERROR - Ascii2Radx::_readBufrDataValue" << endl;
      cerr << "  Bad string for label: " << varLabel << endl;
      cerr << "  line: " << line;
      return -1;
    }
    
    // find first token with '.' - i.e. floating point number
    
    for (size_t itok = 0; itok < toks.size(); itok++) {

      string &valStr = toks[itok];
      if (valStr.find(".") == string::npos) {
        continue;
      }
      
      if (sscanf(valStr.c_str(), "%lg", &dval) == 1) {
        return 0;
      }
      
      cerr << "ERROR - Ascii2Radx::_readBufrDataValue" << endl;
      cerr << "  Bad string for label: " << varLabel << endl;
      cerr << "  line: " << line;
      cerr << "  valStr: " << valStr << endl;
      return -1;
      
    } // itok
    
  } // while

  return -1;

}

