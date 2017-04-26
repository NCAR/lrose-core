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
// Hsrl2Radx.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// June 2015
// 
// Brad Schoenrock, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// updated Mar 2017
//
///////////////////////////////////////////////////////////////

#include "Hsrl2Radx.hh"
#include <Radx/Radx.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <Radx/NcfRadxFile.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/pmu.h>
#include <radar/HsrlRawRay.hh>
#include "MslFile.hh"
#include "RawFile.hh"
#include "CalReader.hh"
#include "FullCals.hh"
#include "DerFieldCalcs.hh"
#include <math.h>  

#include <fstream>
#include <sstream>
#include <string>

using namespace std;

// Constructor

Hsrl2Radx::Hsrl2Radx(int argc, char **argv)
  
{

  OK = TRUE;

  // set programe name

  _progName = "Hsrl2Radx";
  
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

  if (!_params.read_georef_data_from_aircraft_system) {
    if (_params.instrument_latitude_deg < -900 ||
        _params.instrument_longitude_deg < -900 ||
        _params.instrument_altitude_meters < -900) {
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

Hsrl2Radx::~Hsrl2Radx()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Hsrl2Radx::Run()
{
  // reading in calibration files here ----- 
  
  _cals.setDeadTimeHi(_params.calvals_gvhsrl_path, 
                      _params.combined_hi_dead_time_name);
  _cals.setDeadTimeLo(_params.calvals_gvhsrl_path, 
                      _params.combined_lo_dead_time_name);
  _cals.setDeadTimeCross(_params.calvals_gvhsrl_path, 
                         _params.cross_pol_dead_time_name);
  _cals.setDeadTimeMol(_params.calvals_gvhsrl_path, 
                       _params.molecular_dead_time_name);
  _cals.setBinWidth(_params.calvals_gvhsrl_path, 
                    _params.bin_width_name); 
  _cals.setScanAdj(_params.calvals_gvhsrl_path, 
                   _params.scan_adjustment_name); 


  _cals.setBLCor(_params.baseline_calibration_path);
  _cals.setDiffDGeoCor(_params.diff_default_geofile_path);
  _cals.setGeoDefCor(_params.geofile_default_path);
  _cals.setAfPulCor(_params.afterpulse_default_path);
    
  //done reading calibration files. 

  if (_params.mode == Params::ARCHIVE) {
    return _runArchive();
  } else if (_params.mode == Params::FILELIST) {
    return _runFilelist();
  } else if (_params.mode == Params::REALTIME_FMQ) {
    return _runRealtimeFmq();
  } else if (_params.mode == Params::REALTIME_FILE) {
    if (_params.latest_data_info_avail) {
      return _runRealtimeWithLdata();
    } else {
      return _runRealtimeNoLdata();
    }
  }

  // will not reach here
  return -1;

}

//////////////////////////////////////////////////
// Run in filelist mode

int Hsrl2Radx::_runFilelist()
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

int Hsrl2Radx::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - Hsrl2Radx::_runArchive()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - Hsrl2Radx::_runArchive()" << endl;
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

int Hsrl2Radx::_runRealtimeWithLdata()
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

int Hsrl2Radx::_runRealtimeNoLdata()
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

  while(true) { // how does this loop end? --Brad

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
// Run in realtime mode,
// reads raw rays from an FMQ
// writes moments to and FMQ

int Hsrl2Radx::_runRealtimeFmq()
{
  
  // init process mapper registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  int iret = 0;

  while (true) {

    PMU_auto_register("Opening FMQ");
    
    if (_params.debug) {
      cerr << "  Opening Fmq: " << _params.input_fmq_path << endl;
    }
    
    _inputFmq.setHeartbeat(PMU_auto_register);
    int msecsSleepBlocking = 100;
    
    if (_inputFmq.initReadBlocking(_params.input_fmq_path,
                                   "Hsrl2Radx",
                                   _params.debug >= Params::DEBUG_EXTRA,
                                   Fmq::END,
                                   msecsSleepBlocking)) {
      cerr << "ERROR - Hsrl2Radx::_runRealtimeFmq" << endl;
      cerr << "  Cannot init FMQ for reading" << endl;
      cerr << "  Fmq: " << _params.input_fmq_path << endl;
      cerr << _inputFmq.getErrStr() << endl;
      umsleep(1000);
      iret = -1;
      continue;
    }
    
    // read data from the incoming FMQ and process it
    
    if (_readInputFmq()) {
      _inputFmq.closeMsgQueue();
      iret = -1;
    }
    
  } // while(true)

  return iret;

}

///////////////////////////////////////////////////
// read data from the incoming FMQ and process it

int Hsrl2Radx::_readInputFmq()

{

  // read data
  
  while (true) {
    
    PMU_auto_register("Reading data");
    
    // we need a new message
    // blocking read registers with Procmap while waiting
    
    if (_inputFmq.readMsgBlocking()) {
      cerr << "ERROR - Hsrl2Radx::_readInputFmq()" << endl;
      cerr << "  Cannot read message from FMQ" << endl;
      cerr << "  Fmq: " << _params.input_fmq_path << endl;
      return -1;
    }
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Got message from FMQ, len: " << _inputFmq.getMsgLen() << endl;
    }

    // deserialize the ray

    HsrlRawRay rawRay;
    rawRay.deserialize(_inputFmq.getMsg(), _inputFmq.getMsgLen());

    if (_params.debug >= Params::DEBUG_EXTRA) {
      rawRay.printTcpHdr(cerr);
      rawRay.printMetaData(cerr);
    }

    // convert raw ray into RadxRay
    
    RadxRay *radxRay = _convertRawToRadx(rawRay);

    // clean up

    delete radxRay;

  } // while

  return 0;

}

//////////////////////////////////////////////////
// Convert raw ray to Radx Ray

RadxRay *Hsrl2Radx::_convertRawToRadx(HsrlRawRay &rawRay)

{

  // create ray
  
  RadxRay *ray = new RadxRay;

  // range geom & ngates

  double gateSpacingKm = _params.raw_bin_spacing_km;
  double startRangeKm = _params.raw_bin_start_range_km;
  ray->setRangeGeom(startRangeKm, gateSpacingKm);
  ray->setNGates(rawRay.getNGates());

  // time
  
  time_t secs = rawRay.getTimeSecs();
  double nanoSecs = rawRay.getSubSecs() * 1.0e9;
  ray->setTime(secs, nanoSecs);
  
  // sweep info
  
  ray->setVolumeNumber(-9999);
  ray->setSweepNumber(0);
  ray->setSweepMode(Radx::SWEEP_MODE_POINTING);
  ray->setPrtMode(Radx::PRT_MODE_FIXED);
  ray->setTargetScanRateDegPerSec(0.0);
  ray->setIsIndexed(false);
  
  if (rawRay.getTelescopeDirn() == 1) {
      // pointing up
    ray->setAzimuthDeg(0.0);
    ray->setElevationDeg(90.0);
    ray->setFixedAngleDeg(90.0);
  } else {
    // pointing down
    ray->setAzimuthDeg(0.0);
    ray->setElevationDeg(-90.0);
    ray->setFixedAngleDeg(-90.0);
  }

#ifdef NOT_YET

  // TODO - read aircraft location etc from UDP
  // georeference
  
  if (!_params.read_georef_data_from_aircraft_system) {

    setLatitude(_latitude[ii]);
    setLongitude(_longitude[ii]);
    setAltitudeKmMsl(_altitude[ii] / 1000.0);

  } else {
    
    RadxGeoref geo;
    
    if (_telescopeDirection[ii] == 1) {
      
      // pointing up
      
      geo.setRotation(-4.0);
      geo.setTilt(0.0);
      
    } else {
      
      // pointing down
      
      geo.setRotation(184.0);
      geo.setTilt(0.0);
      
    }
    
    geo.setRoll(_roll[ii]);
    geo.setPitch(_pitch[ii]);
    geo.setHeading(_heading[ii]);
    geo.setDrift(0.0); // do not have drift in the file
    geo.setVertVelocity(_vertVel[ii]);
    
    geo.setLatitude(_latitude[ii]);
    geo.setLongitude(_longitude[ii]);
    geo.setAltitudeKmMsl(_altitude[ii] / 1000.0);
    
    double sinVal, cosVal;
    sincos(_heading[ii] * Radx::DegToRad, &sinVal, &cosVal);
    geo.setEwVelocity(_gndSpeed[ii] * sinVal);
    geo.setNsVelocity(_gndSpeed[ii] * cosVal);
    
    ray->setGeoref(geo);

  }
    
  // compute az/el from geo
  
  double azimuth, elevation;
  _computeRadarAngles(geo, corr, azimuth, elevation);
  ray->setAzimuthDeg(azimuth);
  ray->setElevationDeg(elevation);

#endif
  
  // other metadata - overloading
  
  ray->setMeasXmitPowerDbmH(rawRay.getTotalEnergy());
  ray->setEstimatedNoiseDbmHc(rawRay.getPolAngle());
  // hard coded 2000 as replacement for DATA_shot_count from raw file
  ray->setNSamples(2000);

  // add the fields
  
  _addRawFieldToRay(ray, startRangeKm, gateSpacingKm,
                    _params.combined_hi_field_name,
                    "counts",
                    "combined_hi_channel",
                    rawRay.getNGates(),
                    &rawRay.getCombinedHi()[0]);

  _addRawFieldToRay(ray, startRangeKm, gateSpacingKm,
                    _params.combined_lo_field_name,
                    "counts",
                    "combined_lo_channel",
                    rawRay.getNGates(),
                    &rawRay.getCombinedLo()[0]);

  _addRawFieldToRay(ray, startRangeKm, gateSpacingKm,
                    _params.molecular_field_name,
                    "counts",
                    "molecular_channel",
                    rawRay.getNGates(),
                    &rawRay.getMolecular()[0]);

  _addRawFieldToRay(ray, startRangeKm, gateSpacingKm,
                    _params.cross_field_name,
                    "counts",
                    "cross_channel",
                    rawRay.getNGates(),
                    &rawRay.getCross()[0]);


  // return

  return ray;

}

//////////////////////////////////////////////////////////////
// Add fl32 field to rays

void Hsrl2Radx::_addRawFieldToRay(RadxRay *ray,
                                  double startRangeKm,
                                  double gateSpacingKm,
                                  const string &name,
                                  const string &units,
                                  const string &longName,
                                  int nGates,
                                  const Radx::fl32 *fcounts)
  
{

  // create the field
  
  RadxField *field =
    ray->addField(name, units, nGates,
                  Radx::missingFl32,
                  fcounts,
                  true);
  
  field->setLongName(longName);
  field->setRangeGeom(startRangeKm, gateSpacingKm);
  
  // add db of same field
  
  RadxArray<Radx::fl32> dbcounts_;
  Radx::fl32 *dbcounts = dbcounts_.alloc(nGates);

  for (int igate = 0; igate < nGates; igate++) {
    if (fcounts[igate] > 0) {
      dbcounts[igate] = 10.0 * log10(fcounts[igate]);
    } else {
      dbcounts[igate] = Radx::missingFl32;
    }
  }

  string dbName = "db_";
  dbName += name;
  string dbUnits = "db_counts";
  string dbLongName = "db_";
  dbLongName += longName;
  
  RadxField *dbField =
    ray->addField(dbName, dbUnits, nGates,
                  Radx::missingFl32,
                  fcounts,
                  true);
  
  dbField->setLongName(dbLongName);
  dbField->setRangeGeom(startRangeKm, gateSpacingKm);
  
}

//////////////////////////////////////////////////
// Process a file
// Returns 0 on success, -1 on failure

int Hsrl2Radx::_processFile(const string &readPath)
{


  PMU_auto_register("Processing file");
  
  if (_params.debug) {
    cerr << "INFO - Hsrl2Radx::_processFile" << endl;
    cerr << "  Input path: " << readPath << endl;
  }

  // check if this is a CfRadial file

  NcfRadxFile file;
  if (file.isCfRadial(readPath)) {
    return _processUwCfRadialFile(readPath);
  } else {
    return _processUwRawFile(readPath);
  }

}

//////////////////////////////////////////////////
// Process a UofWisc pseudo CfRadial file
// Returns 0 on success, -1 on failure

int Hsrl2Radx::_processUwCfRadialFile(const string &readPath)
{

  PMU_auto_register("Processing UW CfRadial file");

  if (_params.debug) {
    cerr << "INFO - Hsrl2Radx::_processUwCfRadialFile" << endl;
    cerr << "  UW CfRadial file: " << readPath << endl;
  }

  MslFile inFile(_params);
  _setupRead(inFile);
  
  // read in file

  RadxVol vol;
  if (inFile.readFromPath(readPath, vol)) {
    cerr << "ERROR - Hsrl2Radx::Run" << endl;
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

  // convert to floats

  vol.convertToFl32();

  // override radar location if requested
  
  if (!_params.read_georef_data_from_aircraft_system) {
    vol.overrideLocation(_params.instrument_latitude_deg,
                         _params.instrument_longitude_deg,
                         _params.instrument_altitude_meters / 1000.0);
  }
    
  // override radar name and site name if requested
  
  if (_params.override_instrument_name) {
    vol.setInstrumentName(_params.instrument_name);
  }
  if (_params.override_site_name) {
    vol.setSiteName(_params.site_name);
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

  // override start range and/or gate spacing

  if (_params.override_start_range || _params.override_gate_spacing) {
    _overrideGateGeometry(vol);
  }

  // set the range relative to the instrument location

  if (_params.set_range_relative_to_instrument) {
    _setRangeRelToInstrument(inFile, vol);
  }
  
  // set vol values

  vol.loadSweepInfoFromRays();
  vol.loadVolumeInfoFromRays();

  // vol.printWithRayMetaData(cerr);

  // set field type, names, units etc
  
  _convertFields(vol);

  // set global attributes as needed

  _setGlobalAttr(vol);

  // write the file

  if (_writeVol(vol)) {
    cerr << "ERROR - Hsrl2Radx::_processFile" << endl;
    cerr << "  Cannot write volume to file" << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// set up read

void Hsrl2Radx::_setupRead(MslFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (!_params.write_other_fields_unchanged) {

    if (_params.set_output_fields) {
      for (int ii = 0; ii < _params.output_fields_n; ii++) {
        file.addReadField(_params._output_fields[ii].input_field_name);
      }
    }

  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }

}

//////////////////////////////////////////////////
// override the gate geometry

void Hsrl2Radx::_overrideGateGeometry(RadxVol &vol)

{

  vector<RadxRay *> &rays = vol.getRays();
  
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    RadxRay *ray = rays[iray];

    double startRange = ray->getStartRangeKm();
    double gateSpacing = ray->getGateSpacingKm();

    if (_params.override_start_range) {
      startRange = _params.start_range_km;
    }
  
    if (_params.override_gate_spacing) {
      gateSpacing = _params.gate_spacing_km;
    }

    ray->setRangeGeom(startRange, gateSpacing);

  } // iray

}

//////////////////////////////////////////////////
// set the range relative to the instrument location

void Hsrl2Radx::_setRangeRelToInstrument(MslFile &file,
                                         RadxVol &vol)

{

  // telescope direction - 0 is down, 1 is up

  const vector<int> &telescopeDirection = file.getTelescopeDirection();

  vector<RadxRay *> &rays = vol.getRays();
  
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    const RadxRay *ray = rays[iray];
    const RadxGeoref *georef = ray->getGeoreference();
    double altitudeKm = georef->getAltitudeKmMsl();

    int nGatesIn = ray->getNGates();
    double gateSpacingKm = ray->getGateSpacingKm();
    int altitudeInGates = (int) floor(altitudeKm / gateSpacingKm + 0.5);
    if (altitudeInGates > nGatesIn - 1) {
      altitudeInGates = nGatesIn - 1;
    }

    int telDirn = telescopeDirection[iray];
    bool isUp = (telDirn == _params.telescope_direction_is_up);

    int nGatesValid = nGatesIn;
    if (isUp) {
      nGatesValid = nGatesIn - altitudeInGates;
    } else {
      nGatesValid = altitudeInGates + 1;
    }
    if (nGatesValid < 0) {
      nGatesValid = 0;
    }
    if (nGatesValid > nGatesIn) {
      nGatesValid = nGatesIn;
    }

    vector<RadxField *> fields = ray->getFields();
    for (size_t ifield = 0; ifield < fields.size(); ifield++) {
      RadxField *field = fields[ifield];
      const Radx::fl32 *dataIn = field->getDataFl32();
      RadxArray<Radx::fl32> dataOut_;
      Radx::fl32 *dataOut = dataOut_.alloc(nGatesIn);
      for (int ii = 0; ii < nGatesIn; ii++) {
        dataOut[ii] = field->getMissingFl32();
      }
      if (isUp) {
        for (int ii = 0; ii < nGatesValid; ii++) {
          dataOut[ii] = dataIn[altitudeInGates + ii];
        }
      } else {
        for (int ii = 0; ii < nGatesValid; ii++) {
          dataOut[ii] = dataIn[altitudeInGates - ii];
        }
      }
      field->setDataFl32(nGatesIn, dataOut, true);
    } // ifield
    
  } // iray

}
  
//////////////////////////////////////////////////
// rename fields as required

void Hsrl2Radx::_convertFields(RadxVol &vol)
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
    
  }

}

//////////////////////////////////////////////////
// set up write

void Hsrl2Radx::_setupWrite(RadxFile &file)
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
  file.setCompressionLevel(_params.compression_level);

  // set output format

  file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);

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
  file.setWriteSubsecsInFileName(_params.include_subsecs_in_file_name);
  file.setWriteHyphenInDateTime(_params.use_hyphen_in_file_name_datetime_part);

}

//////////////////////////////////////////////////
// set selected global attributes

void Hsrl2Radx::_setGlobalAttr(RadxVol &vol)
{

  vol.setDriver("Hsrl2Radx(NCAR)");

  if (strlen(_params.version_override) > 0) {
    vol.setVersion(_params.version_override);
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

int Hsrl2Radx::_writeVol(RadxVol &vol)
{

  // output file

  NcfRadxFile outFile;
  _setupWrite(outFile);
  
  if (_params.output_filename_mode == Params::SPECIFY_FILE_NAME) {

    string outPath = _params.output_dir;
    outPath += PATH_DELIM;
    outPath += _params.output_filename;

    // write to path
  
    if (outFile.writeToPath(vol, outPath)) {
      cerr << "ERROR - Hsrl2Radx::_writeVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
      
  } else {

    // write to dir
  
    if (outFile.writeToDir(vol, _params.output_dir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - Hsrl2Radx::_writeVol" << endl;
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
      cerr << "WARNING - Hsrl2Radx::_writeVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// Process a UofWisc raw netcdf file
// Returns 0 on success, -1 on failure

int Hsrl2Radx::_processUwRawFile(const string &readPath)
{

  PMU_auto_register("Processing UW Raw file");
  
  if (_params.debug) {
    cerr << "INFO - Hsrl2Radx::_processUwRawFile" << endl;
    cerr << "  UW raw file: " << readPath << endl;
  }

  RawFile inFile(_params);
  
  // read in file
  RadxVol vol;
  if (inFile.readFromPath(readPath, vol)) {
    cerr << "ERROR - Hsrl2Radx::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  
  // override radar name and site name if requested
  if (_params.override_instrument_name) {
    vol.setInstrumentName(_params.instrument_name);
  }
  if (_params.override_site_name) {
    vol.setSiteName(_params.site_name);
  }
    
  // set global attributes as needed
  _setGlobalAttr(vol);
  
  // add in height, temperature and pressure fields
  vector<RadxRay *> rays = vol.getRays();
  for(size_t iray = 0; iray < rays.size(); iray++) {
    _addEnvFields(rays[iray]);
  }
  
  // add corrections and derived data outputs  
  // loop through the rays
  
  for(size_t iray = 0; iray < rays.size(); iray++) {
    _addDerivedFields(rays[iray]);
  }
    
  // write the file
  if (_writeVol(vol)) {
    cerr << "ERROR - Hsrl2Radx::_processFile" << endl;
    cerr << "  Cannot write volume to file" << endl;
    return -1;
  }

  return 0;

}


//////////////////////////////////////////////////
// Add in the height, pressure and temperature
// environmental fields

void Hsrl2Radx::_addEnvFields(RadxRay *ray)
{
  
  double altitudeKm = _params.instrument_altitude_meters;
  const RadxGeoref *geo = ray->getGeoreference();
  if (geo) {
    altitudeKm = geo->getAltitudeKmMsl();
  }
  
  double elevDeg = ray->getElevationDeg();
  size_t nGates = ray->getNGates();
  
  RadxArray<Radx::fl32> htKm_, tempK_, presHpa_;
  Radx::fl32 *htKm = htKm_.alloc(nGates);
  Radx::fl32 *tempK = tempK_.alloc(nGates);
  Radx::fl32 *presHpa = presHpa_.alloc(nGates);
  
  double sinEl = sin(elevDeg * Radx::DegToRad);
  double startRangeKm = ray->getStartRangeKm();
  double gateSpacingKm = ray->getGateSpacingKm();
  
  double rangeKm = startRangeKm;
  for (size_t igate = 0; igate < nGates; igate++, rangeKm += gateSpacingKm) {
    htKm[igate] = altitudeKm + rangeKm * sinEl;
    double htM = htKm[igate] * 1000.0;
    tempK[igate] = _stdAtmos.ht2temp(htM);
    presHpa[igate] = _stdAtmos.ht2pres(htM);
  }

  RadxField *htField =
    ray->addField("height", "km", nGates, Radx::missingFl32, htKm, true);
  htField->setLongName("height_MSL");
  htField->setRangeGeom(startRangeKm, gateSpacingKm);

  RadxField *tempField =
    ray->addField("temperature", "K", nGates, Radx::missingFl32, tempK, true);
  tempField->setLongName("temperaure_from_std_atmos");
  tempField->setRangeGeom(startRangeKm, gateSpacingKm);

  RadxField *presField =
    ray->addField("pressure", "HPa", nGates, Radx::missingFl32, presHpa, true);
  presField->setLongName("pressure_from_std_atmos");
  presField->setRangeGeom(startRangeKm, gateSpacingKm);
    
}

//////////////////////////////////////////////////
// Add in the HSRL moments

void Hsrl2Radx::_addDerivedFields(RadxRay *ray)
{

  double altitudeKm = _params.instrument_altitude_meters;
  const RadxGeoref *geo = ray->getGeoreference();
  if (geo) {
    altitudeKm = geo->getAltitudeKmMsl();
  }
  
  double elevDeg = ray->getElevationDeg();
  size_t nGates = ray->getNGates();
        
  // get more info from ray here
    
  double power = ray->getMeasXmitPowerDbmH();
  double shotCount = ray->getNSamples();
  
  RadxArray<Radx::fl32> htKm_, tempK_, presHpa_;
  Radx::fl32 *htKm = htKm_.alloc(nGates);
  Radx::fl32 *tempK = tempK_.alloc(nGates);
  Radx::fl32 *presHpa = presHpa_.alloc(nGates);
  
  double sinEl = sin(elevDeg * Radx::DegToRad);
  double startRangeKm = ray->getStartRangeKm();
  double gateSpacingKm = ray->getGateSpacingKm();
  
  double rangeKm = startRangeKm;
  for (size_t igate = 0; igate < nGates; igate++, rangeKm += gateSpacingKm) {
    htKm[igate] = altitudeKm + rangeKm * sinEl;
    double htM = htKm[igate] * 1000.0;
    tempK[igate] = _stdAtmos.ht2temp(htM);
    presHpa[igate] = _stdAtmos.ht2pres(htM);
  }
  
  //get raw data fields
  const RadxField *hiField = ray->getField(_params.combined_hi_field_name);
  const RadxField *loField = ray->getField(_params.combined_lo_field_name);
  const RadxField *crossField = ray->getField(_params.cross_field_name);
  const RadxField *molField = ray->getField(_params.molecular_field_name);

  //holds derived data fields
  RadxArray<Radx::fl32> volDepol_, backscatRatio_, partDepol_, 
    backscatCoeff_, extinction_;
  Radx::fl32 *volDepol = volDepol_.alloc(nGates);
  Radx::fl32 *backscatRatio = backscatRatio_.alloc(nGates);
  Radx::fl32 *partDepol = partDepol_.alloc(nGates);
  Radx::fl32 *backscatCoeff = backscatCoeff_.alloc(nGates);
  Radx::fl32 *extinction = extinction_.alloc(nGates);
    
  if(hiField != NULL && loField != NULL && molField != NULL && crossField != NULL)
  {
    const Radx::fl32 *hiData = hiField->getDataFl32();
    const Radx::fl32 *loData = loField->getDataFl32();
    const Radx::fl32 *crossData = crossField->getDataFl32();
    const Radx::fl32 *molData = molField->getDataFl32();
    vector< Radx::fl32> hiDataVec;
    vector< Radx::fl32> loDataVec;
    vector< Radx::fl32> crossDataVec;
    vector< Radx::fl32> molDataVec;
    vector< Radx::fl32> htKmVec;
    vector< Radx::fl32> tempKVec;
    vector< Radx::fl32> presHpaVec;
	
    for(unsigned int igate=0;igate<nGates;igate++)
    {
      hiDataVec.push_back(hiData[igate]);
      loDataVec.push_back(loData[igate]);
      crossDataVec.push_back(crossData[igate]);
      molDataVec.push_back(molData[igate]);
      htKmVec.push_back(htKm[igate]* 1000.0);
      tempKVec.push_back(tempK[igate]);
      presHpaVec.push_back(presHpa[igate]);
    }

	
    DerFieldCalcs data=DerFieldCalcs(_cals, hiDataVec, loDataVec, crossDataVec, 
                                     molDataVec, htKmVec, tempKVec, presHpaVec, 
                                     shotCount, power);
	
    vector<Radx::fl32> volDepolVec=data.get_volDepol();
    vector<Radx::fl32> backscatRatioVec=data.get_backscatRatio();
    vector<Radx::fl32> partDepolVec=data.get_partDepol();
    vector<Radx::fl32> backscatCoeffVec=data.get_backscatCoeff(); 
    vector<Radx::fl32> extinctionVec=data.get_extinction();
	
    for(unsigned int i=0;i<volDepolVec.size();i++)
    {
      volDepol[i]=volDepolVec.at(i);
      backscatRatio[i]=backscatRatioVec.at(i);
      partDepol[i]=partDepolVec.at(i);
      backscatCoeff[i]=backscatCoeffVec.at(i);
      if(i<volDepolVec.size()-1)
        extinction[i]=extinctionVec.at(i);

    }
  }

  RadxField *volDepolField =
    ray->addField("volDepol", "unit", nGates, Radx::missingFl32, volDepol, true);
  volDepolField->setLongName("volume_depolarization");
  volDepolField->setRangeGeom(startRangeKm, gateSpacingKm);
        
  RadxField *backscatRatioField =
    ray->addField("backscatRatio", "unit", nGates, Radx::missingFl32, backscatRatio, true);
  backscatRatioField->setLongName("backscatter_ratio");
  backscatRatioField->setRangeGeom(startRangeKm, gateSpacingKm);
    
  RadxField *partDepolField =
    ray->addField("partDepol", "unit", nGates, Radx::missingFl32, partDepol, true);
  partDepolField->setLongName("particle_depolarization");
  partDepolField->setRangeGeom(startRangeKm, gateSpacingKm);
    
  RadxField *backscatCoeffField =
    ray->addField("backscatCoeff", "unit", nGates, Radx::missingFl32, backscatCoeff, true);
  backscatCoeffField->setLongName("backscatter_coefficient");
  backscatCoeffField->setRangeGeom(startRangeKm, gateSpacingKm);
   
  RadxField *extinctionField =
    ray->addField("extinction", "unit", nGates, Radx::missingFl32, extinction, true);
  extinctionField->setLongName("extinction");
  extinctionField->setRangeGeom(startRangeKm, gateSpacingKm);
       
}
