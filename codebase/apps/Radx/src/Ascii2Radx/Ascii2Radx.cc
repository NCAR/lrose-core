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
#include <toolsa/TaFile.hh>
#include <zlib.h>
using namespace std;

// Constructor

Ascii2Radx::Ascii2Radx(int argc, char **argv)
  
{

  OK = TRUE;
  _volNum = 0;

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

}

// destructor

Ascii2Radx::~Ascii2Radx()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Ascii2Radx::Run()
{

  if (_params.mode == Params::FILELIST) {
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
  
  // loop through the input file list
  
  RadxVol vol;
  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {
    _sweepNum = 0;
    string inputPath = _args.inputFileList[ii];
    // read input file
    int jret = _handleFile(inputPath, vol);
    if (jret == 0) {
      nGood++;
      if (_params.debug) {
        cerr << "  ====>> n good files so far: " << nGood << endl;
        cerr << "  ====>> n errors     so far: " << nError << endl;
        cerr << "  ====>> sum          so far: " << nGood + nError << endl;
      }
    } else {
      iret = -1;
      nError++;
      if (_params.debug) {
        cerr << "  ====>> n errors so far: " << nError << endl;
      }
    }
    // free up
    vol.clear();
  }

  if (_params.debug) {
    cerr << "Ascii2Radx done" << endl;
    cerr << "====>> n good files processed: " << nGood << endl;
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
    if (_handleFile(path, vol)) {
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
      
      if (_handleFile(path, vol)) {
        iret = -1;
      }

      // free up
      vol.clear();
  
    }

  } // while

  return iret;

}

//////////////////////////////////////////////////
// Handle an input file
// Returns 0 on success
//         1 if already read,
//         -1 on failure

int Ascii2Radx::_handleFile(const string &readPath,
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

  if (_params.print_ros2_to_stdout) {
    if (_printRos2ToFile(readPath, stdout)) {
      cerr << "ERROR - Ascii2Radx::_handleFile" << endl;
      cerr << "  printing ITALY ROS2 to stdout, path: " << readPath << endl;
      return -1;
    }
  } else if (_params.input_type == Params::ITALY_ROS2) {
    if (_handleItalyRos2(readPath, vol)) {
      cerr << "ERROR - Ascii2Radx::_handleFile" << endl;
      cerr << "  reading ITALY ROS2 compressed file: " << readPath << endl;
      return -1;
    }
  } else if (_params.input_type == Params::BUFR_ASCII) {
    if (_handleBufrAscii(readPath, vol)) {
      cerr << "ERROR - Ascii2Radx::_handleFile" << endl;
      cerr << "  reading BUFR ASCII file: " << readPath << endl;
      return -1;
    }
  } else if (_params.input_type == Params::ITALY_ASCII) {
    if (_handleItalyAscii(readPath, vol)) {
      cerr << "ERROR - Ascii2Radx::_handleFile" << endl;
      cerr << "  reading ITALY ASCII file: " << readPath << endl;
      return -1;
    }
  }

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

int Ascii2Radx::_handleBufrAscii(const string &readPath,
                                 RadxVol &vol)
{

  // open the file
  
  TaFile taFile;
  FILE *inFile = taFile.fopen(readPath, "r");
  if (inFile == NULL) {
    int errNum = errno;
    cerr << "ERROR - Ascii2Radx::_handleBufrAscii" << endl;
    cerr << "  path: " << readPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // read in the metadata
  
  if (_readBufrMetaData(inFile)) {
    cerr << "ERROR - Ascii2Radx::_handleBufrAscii" << endl;
    cerr << "  path: " << readPath << endl;
    cerr << "  cannot read metadata" << endl;
    return -1;
  }

  // read in the field data
  
  if (_readBufrFieldData(inFile)) {
    cerr << "ERROR - Ascii2Radx::_handleBufrAscii" << endl;
    cerr << "  path: " << readPath << endl;
    cerr << "  cannot read metadata" << endl;
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

    ray->addField(_params._output_fields[0].output_field_name,
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

  // write the volume out

  if (_writeVol(vol)) {
    cerr << "ERROR - Ascii2Radx::_handleBufrAscii" << endl;
    cerr << "  Cannot write volume to file" << endl;
    return -1;
  }

  // success

  return 0;

}


//////////////////////////////////////////////////
// Finalize the volume based on parameters
// Returns 0 on success, -1 on failure

void Ascii2Radx::_finalizeVol(RadxVol &vol)
  
{

  // initialize sweep limits

  vol.loadSweepInfoFromRays();

  // set meta data
  
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

  // remap geometry as applicable

  if (vol.gateGeomVariesByRay()) {
    vol.remapToPredomGeom();
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

  // sweep angles

  if (_params.debug) {
    cerr << "DEBUG - computing sweep fixed angles from ray data" << endl;
  }
  vol.computeFixedAnglesFromRays();

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
    
    string fname = ofld.output_field_name;
    string lname = ofld.long_name;
    string sname = ofld.standard_name;
    string units = ofld.units;
    
    Radx::DataType_t dtype = Radx::ASIS;
    switch(_params.output_encoding) {
      case Params::OUTPUT_ENCODING_FLOAT32:
        dtype = Radx::FL32;
        break;
      case Params::OUTPUT_ENCODING_INT08:
        dtype = Radx::SI08;
        break;
      case Params::OUTPUT_ENCODING_INT16:
      default:
        dtype = Radx::SI16;
        break;
    }
    
    vol.convertField(fname, dtype, 
                     fname, units, sname, lname);
    
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

  // set output format

  switch (_params.output_format) {
    case Params::OUTPUT_FORMAT_ODIM_HDF5:
      file.setFileFormat(RadxFile::FILE_FORMAT_ODIM_HDF5);
      break;
    case Params::OUTPUT_FORMAT_CFRADIAL2:
      file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL2);
    case Params::OUTPUT_FORMAT_CFRADIAL1:
    default:
      file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  }

  file.setWriteCompressed(true);
  file.setCompressionLevel(4);

  if (_params.cfradial_force_ngates_vary) {
    file.setWriteForceNgatesVary(true);
  }

  if (strlen(_params.output_filename_suffix) > 0) {
    file.setWriteFileNameSuffix(_params.output_filename_suffix);
  }

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

}

//////////////////////////////////////////////////
// write out the volume

int Ascii2Radx::_writeVol(RadxVol &vol)
{

  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);

  string outputDir = _params.output_dir;
    
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
  
    if (outFile.writeToDir(vol, outputDir, true, false)) {
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

int Ascii2Radx::_readBufrMetaData(FILE *inFile)
{

  _year = _month = _day = _hour = _min = _sec = 0;
  _readBufrMetaVariable(inFile, "Year", _year);
  _readBufrMetaVariable(inFile, "Month", _month);
  _readBufrMetaVariable(inFile, "Day", _day);
  _readBufrMetaVariable(inFile, "Hour", _hour);
  _readBufrMetaVariable(inFile, "Minute", _min);
  _readBufrMetaVariable(inFile, "Second", _sec);
  _volStartTime.set(_year, _month, _day, _hour, _min, _sec);

  _latitude = 0.0;
  _readBufrMetaVariable(inFile, "Latitude (high accuracy)", _latitude);

  _longitude = 0.0;
  _readBufrMetaVariable(inFile, "Longitude (high accuracy)", _longitude);

  _altitudeM = 0.0;
  _readBufrMetaVariable(inFile, "Height or altitude", _altitudeM);

  _antennaGain = Radx::missingFl64;
  _readBufrMetaVariable(inFile, "Maximum antenna gain", _antennaGain);

  _beamWidth = Radx::missingFl64;
  _readBufrMetaVariable(inFile, "3-dB beamwidth", _beamWidth);

  _antennaSpeedAz = Radx::missingFl64;
  _readBufrMetaVariable(inFile, "Antenna speed (azimuth)", _antennaSpeedAz);

  _antennaSpeedEl = Radx::missingFl64;
  _readBufrMetaVariable(inFile, "Antenna speed (elevation)", _antennaSpeedEl);

  _frequencyHz = Radx::missingFl64;
  _readBufrMetaVariable(inFile, "Mean frequency", _frequencyHz);

  _peakPowerWatts = Radx::missingFl64;
  _readBufrMetaVariable(inFile, "Peak power", _peakPowerWatts);

  _prf = Radx::missingFl64;
  _readBufrMetaVariable(inFile, "Pulse repetition frequency", _prf);

  _pulseWidthSec = Radx::missingFl64;
  _readBufrMetaVariable(inFile, "Pulse width", _pulseWidthSec);

  _rxBandWidthHz = Radx::missingFl64;
  _readBufrMetaVariable(inFile, "Intermediate frequency bandwidth", _rxBandWidthHz);

  _noiseLevelDbm = Radx::missingFl64;
  _readBufrMetaVariable(inFile, "Minimum detectable signal", _noiseLevelDbm);

  _dynamicRangeDb = Radx::missingFl64;
  _readBufrMetaVariable(inFile, "Dynamic range", _dynamicRangeDb);

  _gateSpacingM = 1000.0;
  _readBufrMetaVariable(inFile, "Longueur de la porte distance apres integration", _gateSpacingM);
  _startRangeM = _gateSpacingM / 2.0;

  _azimuthResDeg = 0.5;
  _readBufrMetaVariable(inFile, "Increment de l'azimut entre chaque tir de l'image polaire", _azimuthResDeg);

  _nSamples = 1;
  _readBufrMetaVariable(inFile, "Number of integrated pulses", _nSamples);

  _radarConstant = Radx::missingFl64;
  _readBufrMetaVariable(inFile, "Constante radar", _radarConstant);

  _elevDeg = 0.0;
  _readBufrMetaVariable(inFile, "Antenna elevation", _elevDeg);

  _startAz = 0.0;
  _readBufrMetaVariable(inFile, "Antenna beam azimuth", _startAz);
  _startAz += _azimuthResDeg / 2.0;

  _nGates = 0;
  _readBufrMetaVariable(inFile, "Number of pixels per row", _nGates);

  _nAz = 0;
  _readBufrMetaVariable(inFile, "Number of pixels per column", _nAz);

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

int Ascii2Radx::_readBufrFieldData(FILE *inFile)
{

  // read in number of total points (az * gates)
  // this also positions the file pointer to read the data

  _nPtsData = 0;
  if (_readBufrMetaVariable(inFile, 
                            "Facteur super elargi de repetition differe du descripteur",
                            _nPtsData)) {
    cerr << "ERROR - Ascii2Radx::_readBufrFieldData()" << endl;
    cerr << "  Cannot find variable for _nPtsData" << endl;
    return -1;
  }

  // read in data values

  _fieldData = _fieldData_.alloc(_nPtsData);
  for (int ii = 0; ii < _nPtsData; ii++) {
    double dval;
    if (_readBufrDataValue(inFile, "Pixel value", dval)) {
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

int Ascii2Radx::_readBufrMetaVariable(FILE *inFile,
                                      string varLabel, int &ival,
                                      string precedingLabel)
{
  double dval;
  if (_readBufrMetaVariable(inFile, varLabel, dval, precedingLabel)) {
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

int Ascii2Radx::_readBufrMetaVariable(FILE *inFile,
                                      string varLabel, double &dval,
                                      string precedingLabel)
{
  
  // rewind file

  rewind(inFile);

  // check for preceding label
  
  if (precedingLabel.size() > 0) {
    while (!feof(inFile)) {
      char line[1024];
      if (fgets(line, 1024, inFile) == NULL) {
        continue;
      }
      if (strstr(line, precedingLabel.c_str()) != NULL) {
        break;
      }
    }
  }

  // read in each line
  
  while (!feof(inFile)) {

    // get next line

    char line[1024];
    if (fgets(line, 1024, inFile) == NULL) {
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

int Ascii2Radx::_readBufrDataValue(FILE *inFile,
                                   string varLabel, double &dval)

{
  
  // read in each line
  
  while (!feof(inFile)) {

    // get next line

    char line[1024];
    if (fgets(line, 1024, inFile) == NULL) {
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

//////////////////////////////////////////////////
// Read in an ITALY ASCII file
// Returns 0 on success, -1 on failure

int Ascii2Radx::_handleItalyAscii(const string &readPath,
                                  RadxVol &vol)

{

  // open input file
  
  TaFile inFile;
  FILE *in = inFile.fopen(readPath, "r");
  if (in == NULL) {
    int errNum = errno;
    cerr << "ERROR - Ascii2Radx::_handleItalyAscii" << endl;
    cerr << "  Cannot open file, path: " << readPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // read in vol header

  char line[1000000];
  while (!feof(in)) {
    if (fgets(line, 1000000, in) == NULL) {
      int errNum = errno;
      cerr << "ERROR - Ascii2Radx::_handleItalyAscii" << endl;
      cerr << "  Cannot read volume header, path: " << readPath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
    if (strlen(line) > 6 && !strncmp(line, "VOLUME", 6)) {
      if (_readAsciiVolHeader(line)) {
        cerr << "ERROR - Ascii2Radx::_handleItalyAscii" << endl;
        cerr << "  Cannot decode volume header, path: " << readPath << endl;
        cerr << "  line: " << line << endl;
        return -1;
      }
      break;
    }
  }

  _frequencyHz = -9999.0;
  _prf = -9999.0;
  _pulseWidthSec = -9999.0;
  _beamWidth = 1.0;
  
  vol.setScanName("unknown");
  vol.setLatitudeDeg(_latitude);
  vol.setLongitudeDeg(_longitude);
  vol.setAltitudeKm(_altitudeM / 1000.0);
  
  // read in beams
  
  while (!feof(in)) {
    
    // read in beam header

    RadxTime beamTime;
    double el, az;
    size_t nGates;
    
    while (!feof(in)) {
      if (fgets(line, 1000000, in) == NULL) {
        // end of file
        break;
      }
      if (strlen(line) > 4 && !strncmp(line, "BEAM", 4)) {
        if (_readAsciiBeamHeader(line, beamTime, el, az, nGates)) {
          cerr << "WARNING - Ascii2Radx::_handleItalyAscii" << endl;
          cerr << "  Cannot decode beam header, path: " << readPath << endl;
          cerr << "  line: " << line << endl;
          break;
        }
        break;
      }
    }
    if (feof(in)) {
      continue;
    }

    // create a ray
    
    RadxRay *ray = new RadxRay;
    
    // set metadata

    ray->setScanName("unknown");
    // ray->setSweepNumber(0);
    ray->setCalibIndex(0);
    ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
    ray->setNyquistMps(_nyquist);
    ray->setRangeGeom(_startRangeM / 1000.0, _gateSpacingM / 1000.0);
    ray->setTime(beamTime);
    
    ray->setAzimuthDeg(az);
    ray->setElevationDeg(el);
    ray->setFixedAngleDeg(el);

    // read in fields
    
    while (!feof(in)) {
      if (fgets(line, 1000000, in) == NULL) {
        // end of file
        break;
      }
      if ((strlen(line) > nGates) && (line[1] == ':')) {
        if (_decodeAsciiBeamField(line, nGates, ray)) {
          cerr << "WARNING - Ascii2Radx::_handleItalyAscii" << endl;
          cerr << "  Cannot decode beam fields, path: " << readPath << endl;
          cerr << "  line: " << line << endl;
          break;
        }
      } else {
        break;
      }
    }
    
    // add ray to vol

    vol.addRay(ray);
    
  } // while (true)

  // set the metadata on the volume

  _finalizeVol(vol);

  // write the volume out

  if (_writeVol(vol)) {
    cerr << "ERROR - Ascii2Radx::_handleItalyRos2" << endl;
    cerr << "  Cannot write volume to file" << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// Read in volume header for an ITALY ASCII file
// Returns 0 on success, -1 on failure

int Ascii2Radx::_readAsciiVolHeader(const string &line)
{

  // tokenize the line, breaking at spaces

  vector<string> toks;
  TaStr::tokenize(line, " ", toks);
  if (toks.size() < 6) {
    return -1;
  }

  for (size_t ii = 0; ii < toks.size(); ii++) {

    string tok = toks[ii];
    
    if (tok.find("rad_lat=") == 0) {

      double val;
      if (sscanf(tok.c_str(), "rad_lat=%lg", &val) == 1) {
        _latitude = val;
      } else {
        cerr << "ERROR = _readAsciiVolHeader" << endl;
        cerr << "  no rad_lat in line: " << line << endl;
        return -1;
      }

    } else if (tok.find("rad_lon=") == 0) {

      double val;
      if (sscanf(tok.c_str(), "rad_lon=%lg", &val) == 1) {
        _longitude = val;
      } else {
        cerr << "ERROR = _readAsciiVolHeader" << endl;
        cerr << "  no rad_lon in line: " << line << endl;
        return -1;
      }

    } else if (tok.find("rad_alt=") == 0) {

      double val;
      if (sscanf(tok.c_str(), "rad_alt=%lg", &val) == 1) {
        _altitudeM = val;
      } else {
        cerr << "ERROR = _readAsciiVolHeader" << endl;
        cerr << "  no rad_alt in line: " << line << endl;
        return -1;
      }

    } else if (tok.find("range_bin=") == 0) {

      double val;
      if (sscanf(tok.c_str(), "range_bin=%lg", &val) == 1) {
        _gateSpacingM = val;
        _startRangeM = val / 2.0;
      } else {
        cerr << "ERROR = _readAsciiVolHeader" << endl;
        cerr << "  no range_bin in line: " << line << endl;
        return -1;
      }

    } else if (tok.find("nyquist_velocity=") == 0) {

      double val;
      if (sscanf(tok.c_str(), "nyquist_velocity=%lg", &val) == 1) {
        _nyquist = val;
      } else {
        cerr << "ERROR = _readAsciiVolHeader" << endl;
        cerr << "  no nyquist_velocity in line: " << line << endl;
        return -1;
      }

    } else if (tok.find("data_type=") == 0) {

      int val;
      if (sscanf(tok.c_str(), "data_type=%d", &val) == 1) {
        _dataType = val;
      } else {
        cerr << "ERROR = _readAsciiVolHeader" << endl;
        cerr << "  no data_type in line: " << line << endl;
        return -1;
      }
      
    }
    
  } // ii

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  _latitude: " << _latitude << endl;
    cerr << "  _longitude: " << _longitude << endl;
    cerr << "  _altitudeM: " << _altitudeM << endl;
    cerr << "  _startRangeM: " << _startRangeM << endl;
    cerr << "  _gateSpacingM: " << _gateSpacingM << endl;
    cerr << "  _nyquist: " << _nyquist << endl;
    cerr << "  _dataType: " << _dataType << endl;
  }
  
  return 0;
  
}

//////////////////////////////////////////////////
// Read in beam header for an ITALY ASCII file
// Returns 0 on success, -1 on failure

int Ascii2Radx::_readAsciiBeamHeader(const string &line,
                                     RadxTime &beamTime,
                                     double &el,
                                     double &az,
                                     size_t &nGates)
{

  // tokenize the line, breaking at spaces

  vector<string> toks;
  TaStr::tokenize(line, " ", toks);
  if (toks.size() < 5) {
    return -1;
  }

  for (size_t ii = 0; ii < toks.size(); ii++) {

    string tok = toks[ii];
    
    if (tok.find("t=") == 0) {

      double val;
      if (sscanf(tok.c_str(), "t=%lg", &val) == 1) {
        time_t secs = (time_t) val;
        double subSecs = val - secs;
        beamTime.set(secs, subSecs);
      } else {
        cerr << "ERROR = _readAsciiBeamHeader" << endl;
        cerr << "  cannot decode time, line: " << line << endl;
        return -1;
      }

    } else if (tok.find("el=") == 0) {
      
      double val;
      if (sscanf(tok.c_str(), "el=%lg", &val) == 1) {
        el = val;
      } else {
        cerr << "ERROR = _readAsciiBeamHeader" << endl;
        cerr << "  cannot decode elevation, line: " << line << endl;
        return -1;
      }

    } else if (tok.find("az=") == 0) {
      
      double val;
      if (sscanf(tok.c_str(), "az=%lg", &val) == 1) {
        az = val;
      } else {
        cerr << "ERROR = _readAsciiBeamHeader" << endl;
        cerr << "  cannot decode az, line: " << line << endl;
        return -1;
      }

    } else if (tok.find("n_bins=") == 0) {

      int val;
      if (sscanf(tok.c_str(), "n_bins=%d", &val) == 1) {
        nGates = val;
      } else {
        cerr << "ERROR = _readAsciiBeamHeader" << endl;
        cerr << "  cannot decode n_bins, line: " << line << endl;
        return -1;
      }
      
    }
    
  } // ii
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> adding beam <<==" << endl;
    cerr << "  beamTime: " << beamTime.asString(3) << endl;
    cerr << "  el: " << el << endl;
    cerr << "  az: " << az << endl;
    cerr << "  nGates: " << nGates << endl;
  }
  
  return 0;
  
}

//////////////////////////////////////////////////
// Decode beam field for an ITALY ASCII file
// Returns 0 on success, -1 on failure

int Ascii2Radx::_decodeAsciiBeamField(const string &line,
                                      size_t nGates,
                                      RadxRay *ray)
{

  // tokenize the line, breaking at spaces
  
  vector<string> toks;
  TaStr::tokenize(line, " ", toks);

  // check we have the field Id, followed by number of gates
  
  if (toks.size() != nGates + 1) {
    return -1;
  }

  // decode the values
  
  int fieldId = line[0];
  vector<double> vals;
  for (size_t ii = 1; ii < toks.size(); ii++) {
    string tok = toks[ii];
    double val = atof(tok.c_str());
    vals.push_back(val);
  }

  vector<Radx::fl32> fdata;
  _convertItalyAscii2Floats(fieldId, _dataType,
                            nGates, vals, fdata);
  
  // create the field

  RadxField *field = new RadxField;
  
  // set names and units

  _setItalyFieldNames(fieldId, field);

  // add the data
  
  field->setTypeFl32(-9999.0);
  field->addDataFl32(fdata.size(), fdata.data());
  ray->addField(field);

  return 0;
  
}

//////////////////////////////////////////////////
// Read in an ITALY ROS2 COMPRESSED file
// Returns 0 on success, -1 on failure

int Ascii2Radx::_handleItalyRos2(const string &readPath,
                                 RadxVol &vol)
{
  
  char *buf=NULL;
  char *beam=NULL;
  int n = 0;
  int dataType = 0;
  int sweep = -1;
  ros2_vol_hdr_t vh; 
  ros2_beam_hdr_t bh; 

  // open input file

  TaFile inFile;
  FILE *in = inFile.fopen(readPath, "r");
  if (in == NULL) {
    int errNum = errno;
    cerr << "ERROR - Ascii2Radx::_handleItalyRos2" << endl;
    cerr << "  Cannot open file, path: " << readPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // read in vol header

  if (fread(&vh, sizeof(vh), 1, in) != 1) {
    int errNum = errno;
    cerr << "ERROR - Ascii2Radx::_handleItalyRos2" << endl;
    cerr << "  Cannot read volume header, path: " << readPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if(strcmp(vh.signature, "ROS2_V")) {
    cerr << "ERROR - Ascii2Radx::_handleItalyRos2" << endl;
    cerr << "  Bad volume header, path: " << readPath << endl;
    cerr << "  First bytes should be: ROS_V" << endl;
    return -1;
  }

  _latitude = vh.rad_lat;
  _longitude = vh.rad_lon;
  _altitudeM = vh.rad_alt;
  _frequencyHz = vh.freq * 1.0e6;
  _prf = vh.PRF;
  _pulseWidthSec = vh.l_pulse / 1.0e9;
  _gateSpacingM = vh.l_bin;
  _startRangeM = _gateSpacingM / 2.0;
  _nyquist = vh.nyquist_v;
  _beamWidth = 1.0;
    
  vol.setScanName(vh.name);
  vol.setLatitudeDeg(_latitude);
  vol.setLongitudeDeg(_longitude);
  vol.setAltitudeKm(_altitudeM / 1000.0);
  if (_frequencyHz > 0) {
    vol.setFrequencyHz(_frequencyHz);
  }

  // read in beams
  
  while (true) {

    // read in header

    if (fread(&bh, sizeof(bh), 1, in) != 1){
      int errNum = errno;
      cerr << "ERROR - Ascii2Radx::_handleItalyRos2" << endl;
      cerr << "  Cannot read beam header, path: " << readPath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }

    if (bh.sweep < 0) {
      break;
    }
    
    // create a ray
    
    RadxRay *ray = new RadxRay;

    // set metadata

    ray->setScanName(vh.name);
    ray->setSweepNumber(bh.sweep);
    ray->setCalibIndex(0);
    ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
    ray->setNyquistMps(_nyquist);
    ray->setRangeGeom(_startRangeM / 1000.0, _gateSpacingM / 1000.0);
    
    double timeSecs = (time_t) bh.time;
    double subSecs = bh.time - timeSecs;
    RadxTime rayTime(timeSecs, subSecs);
    ray->setTime(rayTime);
    
    ray->setAzimuthDeg(bh.az);
    ray->setElevationDeg(bh.el);
    ray->setFixedAngleDeg(bh.el);

    ray->setNSamples(bh.n_pulses);
    ray->setPulseWidthUsec(_pulseWidthSec * 1.0e6);
    ray->setPrtSec(1.0 / vh.PRF);

    if (!dataType) {
      dataType= bh.data_type;
    }
          
    if (sweep != bh.sweep) {
      sweep = bh.sweep;
      beam = (char *) realloc(beam, bh.n_values * sizeof(Radx::fl32));
    }

    /* create buf for data */
          
    if (n < bh.beam_length) {
      n=bh.beam_length;
      buf=(char *) realloc(buf,n);
    }

    if (!fread(buf, bh.beam_length, 1, in)) {
      fprintf(stderr, "ERROR: cannot read ROS2 beam\n");
      break;
    }
    
    if (bh.compression) {
      if (!_ros2Uncompress((unsigned char*)buf,
                           bh.beam_length,
                           (unsigned char*)beam,
                           bh.n_values*sizeof(float))) {
        cerr << "ERROR - Ascii2Radx::_handleItalyRos2" << endl;
        cerr << "   cannot uncompress ROS2 beam\n" << endl;
        break;
      }
    } else {
      memcpy(beam, buf, bh.beam_length);
    }
          
    // add fields
	  
    if (vh.Z) {
      _addFieldToRay('Z', bh.data_type, vh.Z_pos, bh.n_bins, beam, ray);
    }
    if (vh.D) {
      _addFieldToRay('D', bh.data_type, vh.D_pos, bh.n_bins, beam, ray);
    }
    if (vh.P) {
      _addFieldToRay('P', bh.data_type, vh.P_pos, bh.n_bins, beam, ray);
    }
    if (vh.R) {
      _addFieldToRay('R', bh.data_type, vh.R_pos, bh.n_bins, beam, ray);
    }
    if (vh.L) {
      _addFieldToRay('L', bh.data_type, vh.L_pos, bh.n_bins, beam, ray);
    }
    if (vh.V) {
      _addFieldToRay('V', bh.data_type, vh.V_pos, bh.n_bins, beam, ray);
    }
    if (vh.S) {
      _addFieldToRay('S', bh.data_type, vh.S_pos, bh.n_bins, beam, ray);
    }

    // add ray to vol

    vol.addRay(ray);
    
  } // while (true)

  if (beam) free(beam);
  if (buf) free(buf);
  
  // set the metadata on the volume

  _finalizeVol(vol);

  // write the volume out

  if (_writeVol(vol)) {
    cerr << "ERROR - Ascii2Radx::_handleItalyRos2" << endl;
    cerr << "  Cannot write volume to file" << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// Read in an ITALY ROS2 COMPRESSED file
// and print to stdout in ITALY ASCII format.
// Returns 0 on success, -1 on failure

int Ascii2Radx::_printRos2ToFile(const string &readPath,
                                 FILE *out)
{

  char *buf=NULL,*beam=NULL,date[32];
  int n=0,sweep=-1,dataType=0;
  ros2_vol_hdr_t vh; 
  ros2_beam_hdr_t bh; 

  // open input file

  TaFile inFile;
  FILE *in = inFile.fopen(readPath, "r");
  if (in == NULL) {
    int errNum = errno;
    cerr << "ERROR - Ascii2Radx::_printRos2ToFile" << endl;
    cerr << "  Cannot open file, path: " << readPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // read in vol header

  if (fread(&vh, sizeof(vh), 1, in) != 1) {
    int errNum = errno;
    cerr << "ERROR - Ascii2Radx::_printRos2ToFile" << endl;
    cerr << "  Cannot read volume header, path: " << readPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if(strcmp(vh.signature, "ROS2_V")) {
    cerr << "ERROR - Ascii2Radx::_printRos2ToFile2" << endl;
    cerr << "  Bad volume header, path: " << readPath << endl;
    cerr << "  First bytes should be: ROS_V" << endl;
    return -1;
  }

  if (vh.Z) fprintf(out,"Z: REFLECTIVITY\n");			   
  if (vh.D) fprintf(out,"D: DIFFERENTIAL REFLECTIVITY\n");			   
  if (vh.P) fprintf(out,"P: DIFFERENTIAL PHASE SHIFT\n");				   
  if (vh.R) fprintf(out,"R: COEFFICIENT OF CORRELATION\n");				   
  if (vh.L) fprintf(out,"L: LINEAR DEPOLARIZATION RATIO\n");				   
  if (vh.V) fprintf(out,"V: DOPPLER VELOCITY\n");				   
  if (vh.S) fprintf(out,"S: SPREAD OF DOPPLER VELOCITY\n");				   
  // fprintf(out, "TIME: %s\n", RadxTime::strm(vh.date).c_str());

  time_t dtime = vh.date;
  strcpy(date, ctime(&dtime));
  date[strlen(date)-1]=0;
  fprintf(out,
          "\nVOLUME: time=%.ld (%s)   rad_lat=%.4f deg   rad_lon=%.4f deg   rad_alt=%.0f m"
          "   range_bin=%.1f m   nyquist_velocity=%.2f m/s",
          vh.date, date, vh.rad_lat, vh.rad_lon, vh.rad_alt, vh.l_bin, vh.nyquist_v); 

  // read in beams
  
  while (true) {

    // read in header

    if (fread(&bh, sizeof(bh), 1, in) != 1){
      int errNum = errno;
      cerr << "ERROR - Ascii2Radx::_printRos2ToFile" << endl;
      cerr << "  Cannot read beam header, path: " << readPath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }

    if (bh.sweep<0) {
      break;
    }
          
    if (!dataType) {
      dataType= bh.data_type;
      fprintf(out,"   dataType=%d\n", dataType);
    }
          
    fprintf(out,
            "\nBEAM: t=%.2lf   el=%.1f   az=%.1f  n_bins=%hd\n",
            bh.time,bh.el,bh.az,bh.n_bins); 

    if (sweep != bh.sweep) {
      sweep = bh.sweep;
      beam = (char *) realloc(beam, bh.n_values * sizeof(Radx::fl32));
    }

    /* ad inizio sweep il numero di range bin potrebbe variare */
          
    if (n < bh.beam_length) {
      n=bh.beam_length;
      buf=(char *) realloc(buf,n);
    }

    if (!fread(buf, bh.beam_length, 1, in)) {
      fprintf(stderr, "ERROR: cannot read ROS2 beam\n");
      break;
    }
    
    if (bh.compression) {
      if (!_ros2Uncompress((unsigned char*)buf,
                           bh.beam_length,
                           (unsigned char*)beam,
                           bh.n_values*sizeof(float))) {
        cerr << "ERROR - Ascii2Radx::_printRos2ToFile" << endl;
        cerr << "   cannot uncompress ROS2 beam\n" << endl;
        break;
      }
    } else {
      memcpy(beam, buf, bh.beam_length);
    }
          
    /* ora beam contiene i dati radar decompressi, che posso stampare  */   
	  
    if (vh.Z) {
      fprintf(out,"Z:");
      _ros2PrintValues(bh.data_type,vh.Z_pos,bh.n_bins,beam,out);
    }
    if (vh.D) {
      fprintf(out,"D:");
      _ros2PrintValues(bh.data_type,vh.D_pos,bh.n_bins,beam,out);
    }
    if (vh.P) {
      fprintf(out,"P:");
      _ros2PrintValues(bh.data_type,vh.P_pos,bh.n_bins,beam,out);
    }
    if (vh.R) {
      fprintf(out,"R:");
      _ros2PrintValues(bh.data_type,vh.R_pos,bh.n_bins,beam,out);
    }
    if (vh.L) {
      fprintf(out,"L:");
      _ros2PrintValues(bh.data_type,vh.L_pos,bh.n_bins,beam,out);
    }
    if (vh.V) {
      fprintf(out,"V:");
      _ros2PrintValues(bh.data_type,vh.V_pos,bh.n_bins,beam,out);
    }
    if (vh.S) {
      fprintf(out,"S:");
      _ros2PrintValues(bh.data_type,vh.S_pos,bh.n_bins,beam,out);
    }
    
  } // while (true)

  if (beam) free(beam);
  if (buf) free(buf);
  
  return 0;

}


//////////////////////////////////////////////////////////
// Uncompress a compressed file in ITALY compressed format

int Ascii2Radx::_ros2Uncompress(unsigned char *in, int n_in,
                                unsigned char *out, int n_out)
  
{
  /* tries to uncompress (zlib) the contents of of the memory area pointed to by in,
   * of size n_in.
   * The result is written to the memory area pointed to by *out,
   * of assigned size * n_out.
   * If the decompression is successful the function returns the value 1,
   * otherwise the input comes simply copied to the output and the value 0 is returned.
   */

  z_stream z;
  
  z.next_in =Z_NULL;
  z.avail_in=Z_NULL;
  z.zalloc  =Z_NULL;
  z.zfree   =Z_NULL;
  z.opaque  =Z_NULL;
  inflateInit(&z);
  
  z.next_in   = in;
  z.avail_in  = n_in;
  z.next_out  = out;
  z.avail_out = n_out;
  
  if (inflate(&z,Z_FINISH)==Z_STREAM_END) {
    inflateEnd(&z);
    return(1);
  } else {
    inflateEnd(&z);
    return(0);
  }
}

//////////////////////////////////////////////////////////
// Print out field values

void Ascii2Radx::_ros2PrintValues(int dataType,
                                  int position,
                                  int n_bins,
                                  char* beam,
                                  FILE* out)
  
{

  int offset = position * n_bins;
  
  switch (dataType)
  {
    case 1:
      /* unsigned char */
      for (int i=0;i<n_bins;i++) {
        fprintf(out," %03d" ,*((unsigned char*) beam+offset+i));
      }
      break;
    case 2:
      /* float */
      for (int i=0;i<n_bins;i++) {
        fprintf(out," %6.2f",*((float*) beam+offset+i));
      }
      break;
    case 3:
      /* ushort */
      for (int i=0;i<n_bins;i++) {
        fprintf(out," %05d" ,*((ushort*)beam+offset+i));
      }
      break;
    case 4:
      /* half */
      break;
  }           
  fprintf(out,"\n");
  return;
}

//////////////////////////////////////////////////////////
// Add a field to the ray

void Ascii2Radx::_addFieldToRay(int fieldId,
                                int dataType,
                                int position,
                                int n_bins,
                                char* beam,
                                RadxRay *ray)
  
{
  
  // create the field

  RadxField *field = new RadxField;

  // set names and units

  _setItalyFieldNames(fieldId, field);

  // convert the data to floats

  vector<Radx::fl32> fdata;
  _convertRos2ArrayToFloat(fieldId, dataType, position, n_bins,
                           beam, fdata);

  // add the data
  
  field->setTypeFl32(-9999.0);
  field->addDataFl32(fdata.size(), fdata.data());
  ray->addField(field);

}

//////////////////////////////////////////////////////////
// Convert a ros2 data value to a float vector

void Ascii2Radx::_convertRos2ArrayToFloat(int fieldId,
                                          int dataType,
                                          int position,
                                          int n_bins,
                                          char* beam,
                                          vector<Radx::fl32> &floats)

{

  int offset = position * n_bins;
  
  // init

  floats.clear();

  // get the scale and bias

  double scale = 1.0;
  double bias = 0.0;
  double range = 1.0;
  _computeItalyScaleAndBias(fieldId, dataType, scale, bias, range);
  
  switch (dataType) {
    case 1:
      /* unsigned char */
      for (int i = 0; i < n_bins; i++) {
        int ival = *((unsigned char*) beam + offset + i);
        double fval = -9999.0;
        if (ival != 0) {
          fval = (ival - 1.0) * scale + bias;
        }
        floats.push_back(fval);
      }
      break;
    case 2:
      /* float */
      for (int i = 0; i < n_bins; i++) {
        double fval = -9999.0;
        if (fval != 0.0) {
          fval = *((Radx::fl32*) beam + offset + i);
        }
        floats.push_back(fval);
      }
      break;
    case 3:
      /* ushort */
      for (int i = 0; i < n_bins; i++) {
        int ival = *((Radx::ui16*) beam + offset + i);
        double fval = -9999.0;
        if (ival != 0) {
          fval = (ival - 1.0) * scale + bias;
        }
        floats.push_back(fval);
      }
      break;
    default:
      /* half - treat as floats */
      for (int i = 0; i < n_bins; i++) {
        double fval = -9999.0;
        if (fval != 0.0) {
          fval = *((Radx::fl32*) beam + offset + i);
        }
        floats.push_back(fval);
      }
  }           

}

//////////////////////////////////////////////////////////
// Convert italy ascii field data to a float vector

void Ascii2Radx::_convertItalyAscii2Floats(int fieldId,
                                           int dataType,
                                           size_t nGates,
                                           const vector<double> &doublesIn,
                                           vector<Radx::fl32> &floatsOut)
  
{

  // init
  
  floatsOut.clear();

  // get the scale and bias

  double scale = 1.0;
  double bias = 0.0;
  double range = 1.0;
  _computeItalyScaleAndBias(fieldId, dataType, scale, bias, range);

  switch (dataType) {
    case 1:
      /* unsigned char */
      for (size_t i = 0; i < nGates; i++) {
        double ival = doublesIn[i];
        double fval = -9999.0;
        if (ival != 0.0) {
          fval = (ival - 1.0) * scale + bias;
        }
        floatsOut.push_back(fval);
      }
      break;
    case 2:
      /* float */
      for (size_t i = 0; i < nGates; i++) {
        double ival = doublesIn[i];
        double fval = -9999.0;
        if (ival != 0.0) {
          fval = ival;
        }
        floatsOut.push_back(fval);
      }
      break;
    case 3:
      /* ushort */
      for (size_t i = 0; i < nGates; i++) {
        double ival = doublesIn[i];
        double fval = -9999.0;
        if (ival != 0.0) {
          fval = (ival - 1.0) * scale + bias;
        }
        floatsOut.push_back(fval);
      }
      break;
    default:
      /* half - treat as floats */
      for (size_t i = 0; i < nGates; i++) {
        double ival = doublesIn[i];
        double fval = -9999.0;
        if (fval != 0.0) {
          fval = ival;
        }
        floatsOut.push_back(fval);
      }
  } // switch       

}

////////////////////////////////////////////////////////////
// set names and units based on field Id

void Ascii2Radx::_setItalyFieldNames(int fieldId, RadxField *field)

{

  switch (fieldId) {
    
    case 'Z':
      field->setName("DBZ");
      field->setUnits("dBZ");
      field->setLongName("reflectivity");
      field->setStandardName("equivalent_reflectivity_factor");
      break;
      
    case 'D':
      field->setName("ZDR");
      field->setUnits("dB");
      field->setLongName("differential_reflectivity");
      field->setStandardName("log_differential_reflectivity_hv");
      break;
      
    case 'P':
      field->setName("PHIDP");
      field->setUnits("deg");
      field->setLongName("differential_phase");
      field->setStandardName("differential_phase_hv");
      break;
      
    case 'R':
      field->setName("RHOHV");
      field->setUnits("");
      field->setLongName("cross_correlation_ratio");
      field->setStandardName("cross_correlation_ratio_hv");
      break;
      
    case 'L':
      field->setName("LDR");
      field->setUnits("dB");
      field->setLongName("linear_depolarization_ratio_hc_to_vx");
      field->setStandardName("log_linear_depolarization_ratio_h");
      break;
      
    case 'V':
      field->setName("VEL");
      field->setUnits("m/s");
      field->setLongName("doppler_velocity");
      field->setStandardName("radial_velocity_of_scatterers_away_from_instrument");
      field->setFieldFolds(-_nyquist, _nyquist);
      break;
      
    case 'S':
      field->setName("WIDTH");
      field->setUnits("m/s");
      field->setLongName("spectrum_width");
      field->setStandardName("doppler_spectrum_width");
      break;

  }

}

////////////////////////////////////////////////////////////
// compute the scale and bias for a field

void Ascii2Radx::_computeItalyScaleAndBias(int fieldId,
                                           int dataType,
                                           double &scale,
                                           double &bias,
                                           double &range)
  
{
  
  // set min and max vals in integer data
  
  int imin = 1;
  int imax = 255;
  if (dataType == 3) {
    // shorts
    imax = 65535;
  }
  range = (double) (imax - imin);
  
  // set scale and offset

  scale = 1.0;
  bias = 0.0;

  switch (fieldId) {
    
    case 'Z':
      scale = (96.0 - -31.5) / range;
      bias = -31.5;
      break;
      
    case 'D':
      scale = (7.9375 - -7.9375) / range;
      bias = -7.9375;
      break;
      
    case 'P':
      scale = (90.0 - -90.0) / range;
      bias = -90;
      break;
      
    case 'R':
      scale = (1.275 - -0.0048) / range;
      bias = -0.0048;
      break;
      
    case 'L':
      scale = (0.0 - -48.0) / range;
      bias = -48.0;
      break;
      
    case 'V':
      scale = (_nyquist - -_nyquist) / range;
      bias = -_nyquist;
      break;
      
    case 'S':
      scale = (_nyquist - 0.0) / range;
      bias = 0.0;
      break;
      
  } // switch

}

  
      
