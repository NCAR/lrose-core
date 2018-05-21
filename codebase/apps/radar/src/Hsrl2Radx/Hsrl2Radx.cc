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
#include "Names.hh"
#include <Radx/Radx.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxSweep.hh>
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
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <radar/HsrlRawRay.hh>
#include <Fmq/DsFmq.hh>

#include <cmath>  
#include <fstream>
#include <sstream>
#include <string>

#include "MslFile.hh"
#include "RawFile.hh"
#include "CalReader.hh"
#include "FullCals.hh"
#include "DerFieldCalcs.hh"
#include "OutputFmq.hh"

 using namespace std;

 // Constructor

 Hsrl2Radx::Hsrl2Radx(int argc, char **argv)

 {

   OK = TRUE;

   _calcs = NULL;
   _calcsFilt = NULL;

   _modelTempField = NULL;
   _modelPresField = NULL;
   _modelTempData = NULL;
   _modelPresData = NULL;

   _nBinsInRay = 0;
   _nBinsPerGate = 0;
   _nGates = 0;

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
       cerr << "  You do not have aircraft data position information" << endl;
       cerr << "  Therefore you must set latitude, longitude and altitude" << endl;
       cerr << "  You must set all 3 values." << endl;
       OK = FALSE;
     }
   }

 }

 // destructor

 Hsrl2Radx::~Hsrl2Radx()

 {

   if (_calcs) {
     delete _calcs;
   }

   if (_calcsFilt) {
     delete _calcsFilt;
   }

   // unregister process

   PMU_auto_unregister();

 }

 //////////////////////////////////////////////////
 // Run

 int Hsrl2Radx::Run()
 {

   // read in calibration files

   if (_readCals()) {
     return -1;
   }

   // create objects to calculate derived fields

   _calcs = new DerFieldCalcs(_params, _cals);
   _calcsFilt = new DerFieldCalcs(_params, _cals);

   // now run

   int iret = 0;
   if (_params.mode == Params::ARCHIVE) {
     iret = _runArchive();
   } else if (_params.mode == Params::FILELIST) {
     iret = _runFilelist();
   } else if (_params.mode == Params::REALTIME_FMQ) {
     iret = _runRealtimeFmq();
   } else if (_params.mode == Params::REALTIME_FILE) {
     if (_params.latest_data_info_avail) {
       iret = _runRealtimeWithLdata();
     } else {
       iret = _runRealtimeNoLdata();
     }
   }

   return iret;

 }

 //////////////////////////////////////////////////
 // Read in cals

 int Hsrl2Radx::_readCals()
 {

   _cals.readDeadTimeHi(_params.calvals_gvhsrl_path, 
                        _params.combined_hi_dead_time_name);

   _cals.readDeadTimeLo(_params.calvals_gvhsrl_path, 
                        _params.combined_lo_dead_time_name);

   _cals.readDeadTimeCross(_params.calvals_gvhsrl_path, 
                           _params.cross_pol_dead_time_name);

   _cals.readDeadTimeMol(_params.calvals_gvhsrl_path, 
                         _params.molecular_dead_time_name);

   _cals.readBinWidth(_params.calvals_gvhsrl_path, 
                      _params.bin_width_name); 

   _cals.readScanAdj(_params.calvals_gvhsrl_path, 
                     _params.scan_adjustment_name); 

   _cals.readMolGain(_params.calvals_gvhsrl_path, 
                     _params.molecular_gain_name);

   _cals.readBaselineCor(_params.baseline_calibration_path);
   _cals.readDiffGeoCor(_params.diff_default_geofile_path);
   _cals.readGeoCor(_params.geofile_default_path);
   _cals.readAfterPulseCor(_params.afterpulse_default_path);

   return 0;

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

   // create output queue

   OutputFmq outputFmq(_progName, _params);
   if (!outputFmq.constructorOK) {
     cerr << "ERROR - Hsrl2Radx::_runRealtimeFmq" << endl;
     cerr << "  Cannot init FMQ for writing" << endl;
     cerr << "  Fmq: " << _params.output_fmq_url << endl;
     return -1;
   }

   // create input queue

   int iret = 0;
   DsFmq inputFmq;

   // loop

   while (true) {

     PMU_auto_register("Opening input FMQ");

     if (_params.debug) {
       cerr << "  Opening Fmq: " << _params.input_fmq_path << endl;
     }

     inputFmq.setHeartbeat(PMU_auto_register);
     int msecsSleepBlocking = 100;

     if (inputFmq.initReadBlocking(_params.input_fmq_path,
                                   "Hsrl2Radx",
                                   _params.debug >= Params::DEBUG_EXTRA,
                                   Fmq::END,
                                   msecsSleepBlocking)) {
       cerr << "ERROR - Hsrl2Radx::_runRealtimeFmq" << endl;
       cerr << "  Cannot init FMQ for reading" << endl;
       cerr << "  Fmq: " << _params.input_fmq_path << endl;
       cerr << inputFmq.getErrStr() << endl;
       umsleep(1000);
       iret = -1;
       continue;
     }

     // read data from the incoming FMQ and process it

     if (_readFmq(inputFmq, outputFmq)) {
       inputFmq.closeMsgQueue();
       iret = -1;
     }

   } // while(true)

   return iret;

 }

 ///////////////////////////////////////////////////
 // read data from the incoming FMQ and process it,
 // writing to the output FMQ

 int Hsrl2Radx::_readFmq(DsFmq &inputFmq,
                         OutputFmq &outputFmq)

 {

   // read data from the input queue
   // convert it and write it to the output queue

   int64_t rayCount = 0;

   while (true) {

     PMU_auto_register("Reading data");

     // we need a new message
     // blocking read registers with Procmap while waiting

     if (inputFmq.readMsgBlocking()) {
       cerr << "ERROR - Hsrl2Radx::_readInputFmq()" << endl;
       cerr << "  Cannot read message from FMQ" << endl;
       cerr << "  Fmq: " << _params.input_fmq_path << endl;
       return -1;
     }

     if (_params.debug >= Params::DEBUG_VERBOSE) {
       cerr << "Got message from FMQ, len: " << inputFmq.getMsgLen() << endl;
     }

     // deserialize the raw ray

     HsrlRawRay rawRay;
     rawRay.deserialize(inputFmq.getMsg(), inputFmq.getMsgLen());

     if (_params.debug >= Params::DEBUG_EXTRA) {
       rawRay.printTcpHdr(cerr);
       rawRay.printMetaData(cerr);
     }

     // convert incoming raw ray into RadxRay

     RadxRay *radxRay = _convertRawToRadx(rawRay);

     // add filtered count fields

     // _addFilteredCountsToRay(radxRay);

     // add environment fields

     _addEnvFields(radxRay);

     // add moments

     _addDerivedMoments(radxRay);
     // _addFilteredMoments(radxRay);

     // set 0 counts to missing

     // _setZeroCountsToMissing(radxRay);

     // write params to FMQ every n rays

     if (rayCount % _params.nrays_for_params == 0) {
       outputFmq.writeParams(radxRay);
     }

     // write ray to the output FMQ

     outputFmq.writeRay(radxRay);

     // clean up

     delete radxRay;
     rayCount++;

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

   _nBinsInRay = rawRay.getNGates();
   _nBinsPerGate = 1;
   if (_params.combine_bins_on_read) {
     _nBinsPerGate = _params.n_bins_per_gate;
   }
   _nGates = _nBinsInRay / _nBinsPerGate;
   ray->setNGates(_nGates);

   double rawGateSpacingKm = _params.raw_bin_spacing_km;
   _gateSpacingKm = rawGateSpacingKm;
   _startRangeKm = _params.raw_bin_start_range_km;

   if (_params.combine_bins_on_read) {
     _gateSpacingKm *= _nBinsPerGate;
     _startRangeKm += (_gateSpacingKm - rawGateSpacingKm) / 2.0;
   }

   ray->setRangeGeom(_startRangeKm, _gateSpacingKm);

   // time

   time_t secs = rawRay.getTimeSecs();
   double nanoSecs = rawRay.getSubSecs() * 1.0e9;
   ray->setTime(secs, nanoSecs);

   // sweep info

   ray->setVolumeNumber(-1);
   ray->setSweepNumber(-1);
   ray->setSweepMode(Radx::SWEEP_MODE_POINTING);
   ray->setPrtMode(Radx::PRT_MODE_FIXED);
   ray->setTargetScanRateDegPerSec(0.0);
   ray->setIsIndexed(false);

   if (rawRay.getTelescopeDirn() == 1) {
       // pointing up
     ray->setAzimuthDeg(0.0);
     ray->setElevationDeg(94.0);
     ray->setFixedAngleDeg(94.0);
   } else {
     // pointing down
     ray->setAzimuthDeg(0.0);
     ray->setElevationDeg(-94.0);
     ray->setFixedAngleDeg(-94.0);
   }

   // read aircraft georeference from SPDB

   if (_params.read_georef_data_from_aircraft_system) {

     RadxGeoref geo;
     if (RawFile::readGeorefFromSpdb(_params.georef_data_spdb_url,
                                     secs,
                                     _params.georef_data_search_margin_secs,
                                     _params.debug >= Params::DEBUG_VERBOSE,
                                     geo) == 0) {
       if (rawRay.getTelescopeDirn() == 1) {
         // pointing up
         geo.setRotation(-4.0);
         geo.setTilt(0.0);
         if (_params.correct_elevation_angle_for_roll) {
           ray->setElevationDeg(94.0 - geo.getRoll());
         } else {
           ray->setElevationDeg(94.0);
         }
       } else {
         // pointing down
         geo.setRotation(184.0);
         geo.setTilt(0.0);
         if (_params.correct_elevation_angle_for_roll) {
           ray->setElevationDeg(-94.0 - geo.getRoll());
         } else {
           ray->setElevationDeg(-94.0);
         }
       }
       ray->setGeoref(geo);

       // compute az/el from geo

       // double azimuth, elevation;
       // RadxCfactors corr;
       // RawFile::computeRadarAngles(geo, corr, azimuth, elevation);
       // ray->setAzimuthDeg(azimuth);
       // ray->setElevationDeg(elevation);

     } // if (RawFile::readGeorefFromSpdb ...

   } // if (_params.read_georef_data_from_aircraft_system)

   // other metadata - overloading

   ray->setMeasXmitPowerDbmH(rawRay.getTotalEnergy());
   ray->setEstimatedNoiseDbmHc(rawRay.getPolAngle());
   // hard coded 2000 as replacement for DATA_shot_count from raw file
   ray->setNSamples(2000);

   // add the raw fields

   _addRawFieldToRay(ray,
                     Names::CombinedHighCounts,
                     "counts",
                     Names::lidar_copolar_combined_backscatter_photon_count,
                     &rawRay.getCombinedHi()[0]);

   _addRawFieldToRay(ray,
                     Names::CombinedLowCounts,
                     "counts",
                     Names::lidar_copolar_combined_backscatter_photon_count,
                     &rawRay.getCombinedLo()[0]);

   _addRawFieldToRay(ray,
                     Names::MolecularCounts,
                     "counts",
                     Names::lidar_copolar_molecular_backscatter_photon_count,
                     &rawRay.getMolecular()[0]);

   _addRawFieldToRay(ray,
                     Names::CrossPolarCounts,
                     "counts",
                     Names::lidar_crosspolar_combined_backscatter_photon_count,
                     &rawRay.getCross()[0]);


   // return

   return ray;

 }

 //////////////////////////////////////////////////////////////
 // Add fl32 field to rays

 void Hsrl2Radx::_addRawFieldToRay(RadxRay *ray,
                                   const string &name,
                                   const string &units,
                                   const string &standardName,
                                   const Radx::fl32 *rcounts)

 {

   RadxArray<Radx::fl32> fcounts_;
   Radx::fl32 *fcounts = fcounts_.alloc(_nGates);

   // sum counts per gate

   size_t ibin = 0;
   for (int igate = 0; igate < _nGates; igate++) {
     fcounts[igate] = 0.0;
     for (int ii = 0; ii < _nBinsPerGate; ii++, ibin++) {
       fcounts[igate] += rcounts[ibin];
     }
     fcounts[igate] /= (double) _nBinsPerGate;
   }

   // create the field

   RadxField *field =
     ray->addField(name, units, _nGates,
                   Radx::missingFl32,
                   fcounts,
                   true);

   field->setStandardName(standardName);
   field->setLongName(standardName);
   field->setRangeGeom(_startRangeKm, _gateSpacingKm);

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
   vol.setPrimaryAxis(Radx::PRIMARY_AXIS_Y_PRIME);

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
   file.setWriteLdataInfo(true);

   // set output format

   file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
   file.setNcFormat(RadxFile::NETCDF4);

   if (strlen(_params.output_filename_prefix) > 0) {
     file.setWriteFileNamePrefix(_params.output_filename_prefix);
   }

   file.setWriteInstrNameInFileName
     (_params.include_instrument_name_in_file_name);

   file.setWriteSiteNameInFileName(false);
   file.setWriteScanTypeInFileName(false);
   file.setWriteVolNumInFileName(false);

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
   vector<RadxRay *> rays = vol.getRays();

   // set global attributes as needed

   _setGlobalAttr(vol);

   // add filtered count fields

   // for(size_t iray = 0; iray < rays.size(); iray++) {
   //   _addFilteredCountsToRay(rays[iray]);
   // }

   // add in height, temperature and pressure fields

   for(size_t iray = 0; iray < rays.size(); iray++) {
     _addEnvFields(rays[iray]);
   }

   // add corrections and derived data outputs  
   // loop through the rays

   for(size_t iray = 0; iray < rays.size(); iray++) {
     _addDerivedMoments(rays[iray]);
     // _addFilteredMoments(rays[iray]);
     // set 0 counts to missing
     // _setZeroCountsToMissing(rays[iray]);
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

   double altitudeKm = _params.instrument_altitude_meters / 1000.0;
   const RadxGeoref *geo = ray->getGeoreference();
   if (geo) {
     altitudeKm = geo->getAltitudeKmMsl();
   }

   double elevDeg = ray->getElevationDeg();
   size_t nGates = ray->getNGates();

   RadxArray<Radx::fl32> htMeters_, tempK_, presHpa_;
   Radx::fl32 *htMeters = htMeters_.alloc(nGates);
   Radx::fl32 *tempK = tempK_.alloc(nGates);
   Radx::fl32 *presHpa = presHpa_.alloc(nGates);

   double sinEl = sin(elevDeg * Radx::DegToRad);
   double startRangeKm = ray->getStartRangeKm();
   double gateSpacingKm = ray->getGateSpacingKm();

   double rangeKm = startRangeKm;
   for (size_t igate = 0; igate < nGates; igate++, rangeKm += gateSpacingKm) {
     double htm = (altitudeKm + rangeKm * sinEl) * 1000.0;
     htMeters[igate] = htm;
     tempK[igate] = _stdAtmos.ht2temp(htm);
     presHpa[igate] = _stdAtmos.ht2pres(htm);
   }

   if (_params.read_temp_and_pressure_profile_from_model_files) {
     if (_getModelData(ray->getTimeSecs()) == 0) {
       _setProfileFromModel(ray, htMeters, tempK, presHpa);
     }
   }

   RadxField *htField =
     ray->addField(Names::Height, "m", nGates, Radx::missingFl32, htMeters, true);
   htField->setStandardName(Names::height_above_mean_sea_level);
   htField->setLongName(Names::height_above_mean_sea_level);
   htField->setRangeGeom(startRangeKm, gateSpacingKm);

   RadxField *tempField =
     ray->addField(Names::Temperature, "K", nGates, Radx::missingFl32, tempK, true);
   tempField->setStandardName(Names::air_temperature);
   tempField->setLongName(Names::air_temperature);
   tempField->setRangeGeom(startRangeKm, gateSpacingKm);

   RadxField *presField =
     ray->addField(Names::Pressure, "HPa", nGates, Radx::missingFl32, presHpa, true);
   presField->setStandardName(Names::air_pressure);
   presField->setLongName(Names::air_pressure);
   presField->setRangeGeom(startRangeKm, gateSpacingKm);

 }

 //////////////////////////////////////////////////
 // Add in the derived HSRL moments

 void Hsrl2Radx::_addDerivedMoments(RadxRay *ray)
 {

   // ray info

   time_t rayTime = ray->getTimeSecs();
   size_t nGates = ray->getNGates();
   double startRangeKm = ray->getStartRangeKm();
   double gateSpacingKm = ray->getGateSpacingKm();

   double power = ray->getMeasXmitPowerDbmH();
   double shotCount = ray->getNSamples();

   // set the cals for this time

   _cals.setTime(rayTime);

   // environmental fields

   RadxField *htField = ray->getField(Names::Height);
   RadxField *tempField = ray->getField(Names::Temperature);
   RadxField *presField = ray->getField(Names::Pressure);
   assert(htField != NULL);
   assert(tempField != NULL);
   assert(presField != NULL);

   // get raw data fields

   const RadxField *hiField = ray->getField(Names::CombinedHighCounts);
   assert(hiField != NULL);

   const RadxField *loField = ray->getField(Names::CombinedLowCounts);
   assert(loField != NULL);

   const RadxField *molField = ray->getField(Names::MolecularCounts);
   assert(molField != NULL);

   const RadxField *crossField = ray->getField(Names::CrossPolarCounts);
   assert(crossField != NULL);

   // calculate the derived fields

   _calcs->computeDerived(nGates,
                          startRangeKm, gateSpacingKm,
                          hiField->getDataFl32(),
                          loField->getDataFl32(),
                          crossField->getDataFl32(),
                          molField->getDataFl32(),
                          htField->getDataFl32(),
                          tempField->getDataFl32(),
                          presField->getDataFl32(), 
                          shotCount,
                          power);

   // load fields with the results

   // combined high rate

   RadxField *hiRate =
     ray->addField(Names::CombinedHighRate, "s-1", nGates, Radx::missingFl32, 
                   _calcs->getHiRate().data(), true);
   hiRate->setStandardName(Names::lidar_copolar_combined_backscatter_photon_rate);
   hiRate->setLongName(Names::lidar_copolar_combined_backscatter_photon_rate);
   hiRate->setRangeGeom(startRangeKm, gateSpacingKm);

   // RadxField *hiRateF =
   //   ray->addField(Names::CombinedHighRate_F, "s-1", nGates, Radx::missingFl32, 
   //                 _calcs->getHiRateF().data(), true);
   // hiRate->setStandardName(Names::lidar_copolar_combined_backscatter_photon_rate);
   // hiRate->setLongName(Names::lidar_copolar_combined_backscatter_photon_rate_filtered);
   // hiRateF->setRangeGeom(startRangeKm, gateSpacingKm);

   // combined low rate

   RadxField *loRate =
     ray->addField(Names::CombinedLowRate, "s-1", nGates, Radx::missingFl32, 
                   _calcs->getLoRate().data(), true);
   loRate->setStandardName(Names::lidar_copolar_combined_backscatter_photon_rate);
   loRate->setLongName(Names::lidar_copolar_combined_backscatter_photon_rate);
   loRate->setRangeGeom(startRangeKm, gateSpacingKm);

   // RadxField *loRateF =
   //   ray->addField(Names::CombinedLowRate_F, "s-1", nGates, Radx::missingFl32, 
   //                 _calcs->getLoRateF().data(), true);
   // loRate->setStandardName(Names::lidar_copolar_combined_backscatter_photon_rate);
   // loRate->setLongName(Names::lidar_copolar_combined_backscatter_photon_rate_filtered);
   // loRateF->setRangeGeom(startRangeKm, gateSpacingKm);

   // molecular rate

   RadxField *molRate =
     ray->addField(Names::MolecularRate, "s-1", nGates, Radx::missingFl32, 
                   _calcs->getMolRate().data(), true);
   molRate->setStandardName(Names::lidar_copolar_molecular_backscatter_photon_rate);
   molRate->setLongName(Names::lidar_copolar_molecular_backscatter_photon_rate);
   molRate->setRangeGeom(startRangeKm, gateSpacingKm);

   // RadxField *molRateF =
   //   ray->addField(Names::MolecularRate_F, "s-1", nGates, Radx::missingFl32, 
   //                 _calcs->getMolRateF().data(), true);
   // molRate->setStandardName(Names::lidar_copolar_molecular_backscatter_photon_rate);
   // molRate->setLongName(Names::lidar_copolar_molecular_backscatter_photon_rate_filtered);
   // molRateF->setRangeGeom(startRangeKm, gateSpacingKm);

   // cross rate

   RadxField *crossRate =
     ray->addField(Names::CrossPolarRate, "s-1", nGates, Radx::missingFl32, 
                   _calcs->getCrossRate().data(), true);
   crossRate->setStandardName(Names::lidar_crosspolar_combined_backscatter_photon_rate);
   crossRate->setLongName(Names::lidar_crosspolar_combined_backscatter_photon_rate);
   crossRate->setRangeGeom(startRangeKm, gateSpacingKm);

   // RadxField *crossRateF =
   //   ray->addField(Names::CrossPolarRate_F, "s-1", nGates, Radx::missingFl32, 
   //                 _calcs->getCrossRateF().data(), true);
   // crossRate->setStandardName(Names::lidar_crosspolar_combined_backscatter_photon_rate);
   // crossRate->setLongName(Names::lidar_crosspolar_combined_backscatter_photon_rate_filtered);
   // crossRateF->setRangeGeom(startRangeKm, gateSpacingKm);

   // beta m sonde

   RadxField *betaMSonde =
     ray->addField(Names::BetaMSonde, "", nGates, Radx::missingFl32, 
                   _calcs->getBetaMSonde().data(), true);
   betaMSonde->setStandardName(Names::lidar_beta_m_sonde);
   betaMSonde->setLongName(Names::lidar_beta_m_sonde);
   betaMSonde->setRangeGeom(startRangeKm, gateSpacingKm);

   // vol depol ratio

   RadxField *volDepol =
     ray->addField(Names::VolumeDepolRatio, "", nGates, Radx::missingFl32, 
                   _calcs->getVolDepol().data(), true);
   volDepol->setStandardName(Names::lidar_volume_depolarization_ratio);
   volDepol->setLongName(Names::lidar_volume_depolarization_ratio);
   volDepol->setRangeGeom(startRangeKm, gateSpacingKm);

   // RadxField *volDepolF =
   //   ray->addField(Names::VolumeDepolRatio_F, "", nGates, Radx::missingFl32, 
   //                 _calcs->getVolDepolF().data(), true);
   // volDepolF->setStandardName(Names::lidar_volume_depolarization_ratio);
   // volDepolF->setLongName(Names::lidar_volume_depolarization_ratio_filtered);
   // volDepolF->setRangeGeom(startRangeKm, gateSpacingKm);

   // particle depol ratio

   RadxField *partDepol =
     ray->addField(Names::ParticleDepolRatio, "", nGates, Radx::missingFl32,
                   _calcs->getPartDepol().data(), true);
   partDepol->setLongName(Names::lidar_particle_depolarization_ratio);
   partDepol->setStandardName(Names::lidar_particle_depolarization_ratio);
   partDepol->setRangeGeom(startRangeKm, gateSpacingKm);

   // RadxField *partDepolF =
   //   ray->addField(Names::ParticleDepolRatio_F, "", nGates, Radx::missingFl32,
   //                 _calcs->getPartDepolF().data(), true);
   // partDepolF->setLongName(Names::lidar_particle_depolarization_ratio_filtered);
   // partDepolF->setStandardName(Names::lidar_particle_depolarization_ratio);
   // partDepolF->setRangeGeom(startRangeKm, gateSpacingKm);

   // backscatter ratio

   RadxField *backscatRatio =
     ray->addField(Names::BackScatterRatio, "", nGates, Radx::missingFl32, 
                   _calcs->getBackscatRatio().data(), true);
   backscatRatio->setStandardName(Names::lidar_backscatter_ratio);
   backscatRatio->setLongName(Names::lidar_backscatter_ratio);
   backscatRatio->setRangeGeom(startRangeKm, gateSpacingKm);

   // RadxField *backscatRatioF =
   //   ray->addField(Names::BackScatterRatio_F, "", nGates, Radx::missingFl32, 
   //                 _calcs->getBackscatRatioF().data(), true);
   // backscatRatioF->setStandardName(Names::lidar_backscatter_ratio);
   // backscatRatioF->setLongName(Names::lidar_backscatter_ratio_filtered);
   // backscatRatioF->setRangeGeom(startRangeKm, gateSpacingKm);

   // backscatter coefficient

   RadxField *backscatCoeff =
     ray->addField(Names::BackScatterCoeff, "m-1.sr-1", nGates, Radx::missingFl32,
                   _calcs->getBackscatCoeff().data(), true);
   backscatCoeff->setStandardName(Names::lidar_backscatter_coefficient);
   backscatCoeff->setLongName(Names::lidar_backscatter_coefficient);
   backscatCoeff->setRangeGeom(startRangeKm, gateSpacingKm);

   // RadxField *backscatCoeffF =
   //   ray->addField(Names::BackScatterCoeff_F, "m-1.sr-1", nGates, Radx::missingFl32,
   //                 _calcs->getBackscatCoeffF().data(), true);
   // backscatCoeffF->setStandardName(Names::lidar_backscatter_coefficient);
   // backscatCoeffF->setLongName(Names::lidar_backscatter_coefficient_filtered);
   // backscatCoeffF->setRangeGeom(startRangeKm, gateSpacingKm);

   // extinction coeff

   RadxField *extinction =
     ray->addField(Names::ExtinctionCoeff, "m-1", nGates, Radx::missingFl32, 
                   _calcs->getExtinctionCoeff().data(), true);
   extinction->setStandardName(Names::lidar_extinction_coefficient);
   extinction->setLongName(Names::lidar_extinction_coefficient);
   extinction->setRangeGeom(startRangeKm, gateSpacingKm);

   // RadxField *extinctionF =
   //   ray->addField(Names::ExtinctionCoeff_F, "m-1", nGates, Radx::missingFl32, 
   //                 _calcs->getExtinctionCoeffF().data(), true);
   // extinctionF->setStandardName(Names::lidar_extinction_coefficient);
   // extinctionF->setLongName(Names::lidar_extinction_coefficient_filtered);
   // extinctionF->setRangeGeom(startRangeKm, gateSpacingKm);

   // optical depth

   RadxField *optDepth =
     ray->addField(Names::OpticalDepth, "", nGates, Radx::missingFl32, 
                   _calcs->getOpticalDepth().data(), true);
   optDepth->setStandardName(Names::lidar_optical_depth);
   optDepth->setLongName(Names::lidar_optical_depth);
   optDepth->setRangeGeom(startRangeKm, gateSpacingKm);

   // RadxField *optDepthF =
   //   ray->addField(Names::OpticalDepth_F, "", nGates, Radx::missingFl32, 
   //                 _calcs->getOpticalDepthF().data(), true);
   // optDepthF->setStandardName(Names::lidar_optical_depth);
   // optDepthF->setLongName(Names::lidar_optical_depth_filtered);
   // optDepthF->setRangeGeom(startRangeKm, gateSpacingKm);

 }

 ///////////////////////////////////////////////////////
 // read in temp and pressure profile from model

 int Hsrl2Radx::_getModelData(time_t rayTime)

 {

   // check if the previously read file will suffice

   time_t prevValidTime = _mdvx.getValidTime();
   double tdiff = fabs((double) prevValidTime - (double) rayTime);
   if (tdiff <= _params.model_profile_search_margin_secs) {
     if (_params.debug >= Params::DEBUG_EXTRA) {
       cerr << "Reusing previously read MDV file for profile" << endl;
       cerr << "  file: " << _mdvx.getPathInUse() << endl;
     }
     return 0;
   }

   // set up the read

   _mdvx.clear();
   _mdvx.setReadTime(Mdvx::READ_CLOSEST,
                     _params.model_profile_mdv_data_url,
                     _params.model_profile_search_margin_secs,
                     rayTime);
   _mdvx.addReadField(_params.model_temperature_field_name);
   _mdvx.addReadField(_params.model_pressure_field_name);
   _mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
   _mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);

   // perform the read

   if (_mdvx.readVolume()) {
     cerr << "ERROR - Hsrl2Radx::_getModelData()" << endl;
     cerr << "  Cannot read model data for time: "
          << RadxTime::strm(rayTime) << endl;
     cerr << _mdvx.getErrStr() << endl;
     return -1;
   }

   if (_params.debug) {
     cerr << "====>> read model data for temp and pressure profiles" << endl;
     cerr << "  ray time: "
          << RadxTime::strm(rayTime) << endl;
     cerr << "  model valid time: " 
          << RadxTime::strm(_mdvx.getValidTime()) << endl;
     cerr << "  file: " << _mdvx.getPathInUse() << endl;
     cerr << "=====================================================" << endl;
   }

   // get fields

   _modelTempField = _mdvx.getField(_params.model_temperature_field_name);
   if (_modelTempField == NULL) {
     cerr << "ERROR - Hsrl2Radx::_getModelData()" << endl;
     cerr << "  No temperaure field found" << endl;
     return -1;
   }
   _modelTempFhdr = _modelTempField->getFieldHeader();
   _modelTempVhdr = _modelTempField->getVlevelHeader();
   _modelTempData = (fl32 *) _modelTempField->getVol();

   _modelPresField = _mdvx.getField(_params.model_pressure_field_name);
   if (_modelPresField == NULL) {
     cerr << "ERROR - Hsrl2Radx::_getModelData()" << endl;
     cerr << "  No pressure field found" << endl;
     return -1;
   }
   _modelPresData = (fl32 *) _modelPresField->getVol();

   // set projection

   _modelProj.init(_modelTempFhdr);

   return 0;

 }

 ///////////////////////////////////////////////////////
 // set the profile from the model data

 int Hsrl2Radx::_setProfileFromModel(RadxRay *ray,
                                     Radx::fl32 *htMeters,
                                     Radx::fl32 *tempK,
                                     Radx::fl32 *presHpa)

 {

   // get lat and lon

   double lat = 0.0, lon = 0.0;
   const RadxGeoref *geo = ray->getGeoreference();
   if (geo == NULL) {
     lat = _params.instrument_latitude_deg;
     lon = _params.instrument_longitude_deg;
   } else {
     lat = geo->getLatitude();
     lon = geo->getLongitude();
   }

   // get grid location index

   int xIndex, yIndex;
   if (_modelProj.latlon2xyIndex(lat, lon, xIndex, yIndex, true)) {
     if (_params.debug >= Params::DEBUG_VERBOSE) {
       cerr << "ERROR - Hsrl2Radx::_setProfileFromModel()" << endl;
       cerr << "  lat,lon outside model domain: " << lat << ", " << lon << endl;
     }
     return -1;
   }
   int nx = _modelTempFhdr.nx;
   int ny = _modelTempFhdr.ny;
   int nz = _modelTempFhdr.nz;
   int planeOffset = yIndex * nx + xIndex;
   int nPointsPlane = ny * nx;

   if (_params.debug >= Params::DEBUG_VERBOSE) {
     cerr << "lat, lon, xIndex, yIndex: "
          << lat << ", "
          << lon << ", "
          << xIndex << ", "
          << yIndex << endl;
   }

   // load up temp and pressure column arrays from model

   RadxArray<Radx::fl32> modelTempK_, modelPresHpa_;
   Radx::fl32 *modelTempK = modelTempK_.alloc(nz);
   Radx::fl32 *modelPresHpa = modelPresHpa_.alloc(nz);

   for (int iz = 0; iz < nz; iz++) {
     long int dataOffset = iz * nPointsPlane + planeOffset;
     if (_params.temperature_profile_units == Params::DEGREES_CELCIUS) {
       modelTempK[iz] = TEMP_C_TO_K(_modelTempData[dataOffset]);
     } else {
       modelTempK[iz] = _modelTempData[dataOffset];
     }
     if (_params.pressure_profile_units == Params::PA) {
       modelPresHpa[iz] = _modelPresData[dataOffset] / 100.0;
     } else {
       modelPresHpa[iz] = _modelPresData[dataOffset];
     }
     // cerr << "11111111 iz, ht, temp0, temp, pres0, pres: " << iz << ", "
     //      << vhdr.level[iz] << ", "
     //      << tempData[iz * nPointsPlane] << ", "
     //      << modelTempK[iz] << ", "
     //      << presData[iz * nPointsPlane] << ", "
     //      << modelPresHpa[iz] << endl;
   } // iz

   // interpolate temp and pressure from model onto ray

   size_t nGates = ray->getNGates();

   if (ray->getElevationDeg() > 0) {

     // pointing up

     int startIz = 1;
     for (size_t igate = 0; igate < nGates; igate++) {
       double htKm = htMeters[igate] / 1000.0;
       if (htKm <= _modelTempVhdr.level[0]) {
         tempK[igate] = modelTempK[0];
         presHpa[igate] = modelPresHpa[0];
       } else if (htKm >= _modelTempVhdr.level[nz - 1]) {
         tempK[igate] = modelTempK[nz - 1];
         presHpa[igate] = modelPresHpa[nz - 1];
       } else {
         for (int iz = startIz; iz < nz; iz++) {
           if (htKm > _modelTempVhdr.level[iz - 1] &&
               htKm <= _modelTempVhdr.level[iz]) {
             double wt1 =
               (htKm - _modelTempVhdr.level[iz - 1]) / 
               (_modelTempVhdr.level[iz] - _modelTempVhdr.level[iz - 1]);
             double wt0 = 1.0 - wt1;
             tempK[igate] = modelTempK[iz - 1] * wt0 + modelTempK[iz] * wt1;
             presHpa[igate] = modelPresHpa[iz - 1] * wt0 + modelPresHpa[iz] * wt1;
             startIz = iz;
             break;
           } // if (htKm > ...
         } // iz
       } // if (htKm <= ...
     } // igate

   } else {

     // pointing down

     int startIz = nz - 1;
     for (size_t igate = 0; igate < nGates; igate++) {
       double htKm = htMeters[igate] / 1000.0;
       if (htKm <= _modelTempVhdr.level[0]) {
         tempK[igate] = modelTempK[0];
         presHpa[igate] = modelPresHpa[0];
       } else if (htKm >= _modelTempVhdr.level[nz - 1]) {
         tempK[igate] = modelTempK[nz - 1];
         presHpa[igate] = modelPresHpa[nz - 1];
       } else {
         for (int iz = startIz; iz > 0; iz--) {
           if (htKm > _modelTempVhdr.level[iz - 1] &&
               htKm <= _modelTempVhdr.level[iz]) {
             double wt1 =
               (htKm - _modelTempVhdr.level[iz - 1]) / 
               (_modelTempVhdr.level[iz] - _modelTempVhdr.level[iz - 1]);
             double wt0 = 1.0 - wt1;
             tempK[igate] = modelTempK[iz - 1] * wt0 + modelTempK[iz] * wt1;
             presHpa[igate] = modelPresHpa[iz - 1] * wt0 + modelPresHpa[iz] * wt1;
             startIz = iz;
             break;
           } // if (htKm > ...
         } // iz
       } // if (htKm <= ...
     } // igate

   } // if (ray->getElevationDeg() > 0) 
     
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Add filtered counts to ray

// void Hsrl2Radx::_addFilteredCountsToRay(RadxRay *ray)
  
// {

//   _addFilteredCountsToRay(ray, 
//                           Names::CombinedHighCounts,
//                           Names::CombinedHighCounts_F);
  
//   _addFilteredCountsToRay(ray, 
//                           Names::CombinedLowCounts,
//                           Names::CombinedLowCounts_F);
  
//   _addFilteredCountsToRay(ray, 
//                           Names::MolecularCounts,
//                           Names::MolecularCounts_F);
  
//   _addFilteredCountsToRay(ray, 
//                           Names::CrossPolarCounts,
//                           Names::CrossPolarCounts_F);

// }
    
//////////////////////////////////////////////////////////////
// Add filtered counts to ray

// void Hsrl2Radx::_addFilteredCountsToRay(RadxRay *ray,
//                                         const string &name,
//                                         const string &filteredName)
  
// {

//   // get the field

//   RadxField *raw = ray->getField(name);
//   assert(raw);

//   // make a copy

//   RadxField *copy = new RadxField(*raw);

//   // get the count data
  
//   Radx::fl32 *fcounts = copy->getDataFl32();
  
//   // censor counts as required
  
//   if (_params.rate_censoring_threshold > 0) {
//     for (size_t ii = 0; ii < ray->getNGates(); ii++) {
//       if (fcounts[ii] < _params.rate_censoring_threshold) {
//         fcounts[ii] = 0;
//       }
//     }
//   }

//   // despeckle
  
//   if (_params.apply_speckle_filter) {
//     _applyZeroSpeckleFilter(_nGates, _params.speckle_filter_len, fcounts);
//   }

//   // change the name
  
//   copy->setName(filteredName);

//   // add the modified field

//   ray->addField(copy);

// }

//////////////////////////////////////////////////////////////
// Set zero counts to missing

// void Hsrl2Radx::_setZeroCountsToMissing(RadxRay *ray)
// {
//   _setZeroCountsToMissing(ray, Names::CombinedHighCounts);
//   _setZeroCountsToMissing(ray, Names::CombinedHighCounts_F);
//   _setZeroCountsToMissing(ray, Names::CombinedLowCounts);
//   _setZeroCountsToMissing(ray, Names::CombinedLowCounts_F);
//   _setZeroCountsToMissing(ray, Names::MolecularCounts);
//   _setZeroCountsToMissing(ray, Names::MolecularCounts_F);
//   _setZeroCountsToMissing(ray, Names::CrossPolarCounts);
//   _setZeroCountsToMissing(ray, Names::CrossPolarCounts_F);
// }

// void Hsrl2Radx::_setZeroCountsToMissing(RadxRay *ray,
//                                         const string &name)
  
// {

//   // get the field
  
//   RadxField *fld = ray->getField(name);
//   assert(fld);

//   // get the count data
  
//   Radx::fl32 *fcounts = fld->getDataFl32();
  
//   // censor counts as required
  
//   for (size_t ii = 0; ii < ray->getNGates(); ii++) {
//     if (fcounts[ii] <= 0.0) {
//       fcounts[ii] = Radx::missingFl32;
//     }
//   }

// }

//////////////////////////////////////////////////
// Add in the filtered derived HSRL moments

// void Hsrl2Radx::_addFilteredMoments(RadxRay *ray)
// {

//   // ray info
  
//   time_t rayTime = ray->getTimeSecs();
//   size_t nGates = ray->getNGates();
//   double startRangeKm = ray->getStartRangeKm();
//   double gateSpacingKm = ray->getGateSpacingKm();

//   double power = ray->getMeasXmitPowerDbmH();
//   double shotCount = ray->getNSamples();

//   // set the cals for this time

//   _cals.setTime(rayTime);

//   // environmental fields

//   RadxField *htField = ray->getField(Names::Height);
//   RadxField *tempField = ray->getField(Names::Temperature);
//   RadxField *presField = ray->getField(Names::Pressure);
//   assert(htField != NULL);
//   assert(tempField != NULL);
//   assert(presField != NULL);

//   // get count fields

//   const RadxField *hiField = ray->getField(Names::CombinedHighCounts_F);
//   assert(hiField != NULL);

//   const RadxField *loField = ray->getField(Names::CombinedLowCounts_F);
//   assert(loField != NULL);

//   const RadxField *molField = ray->getField(Names::MolecularCounts_F);
//   assert(molField != NULL);

//   const RadxField *crossField = ray->getField(Names::CrossPolarCounts_F);
//   assert(crossField != NULL);

//   // calculate the derived fields
  
//   cerr << "MMMMMMMMMMMMMMMMMMMMMMMMm _calcsFilt" << endl;

//   _calcsFilt->computeDerived(nGates,
//                              startRangeKm, gateSpacingKm,
//                              hiField->getDataFl32(),
//                              loField->getDataFl32(),
//                              crossField->getDataFl32(),
//                              molField->getDataFl32(),
//                              htField->getDataFl32(),
//                              tempField->getDataFl32(),
//                              presField->getDataFl32(), 
//                              shotCount,
//                              power);

//   cerr << "NNNNNNNNNNNNNNNNNNNNNNNNN _calcsFilt" << endl;

//   // apply speckle filter on results

//   if (_params.apply_speckle_filter) {
//     _applyMissingSpeckleFilter(nGates, _params.speckle_filter_len, 
//                                _calcsFilt->getVolDepol().data());
//     _applyMissingSpeckleFilter(nGates, _params.speckle_filter_len,
//                                _calcsFilt->getPartDepol().data());
//     _applyMissingSpeckleFilter(nGates, _params.speckle_filter_len,
//                                _calcsFilt->getBackscatRatio().data());
//     _applyMissingSpeckleFilter(nGates, _params.speckle_filter_len,
//                                _calcsFilt->getBackscatCoeff().data());
//     _applyMissingSpeckleFilter(nGates, _params.speckle_filter_len,
//                                _calcsFilt->getExtinctionCoeff().data());
//     _applyMissingSpeckleFilter(nGates, _params.speckle_filter_len,
//                                _calcsFilt->getOpticalDepth().data());
//   }

//   // load fields with the results

//   RadxField *volDepolField =
//     ray->addField(Names::VolumeDepolRatio_F, "", nGates, Radx::missingFl32, 
//                   _calcsFilt->getVolDepol().data(), true);
//   volDepolField->setStandardName(Names::lidar_volume_depolarization_ratio);
//   volDepolField->setLongName(Names::lidar_volume_depolarization_ratio);
//   volDepolField->setRangeGeom(startRangeKm, gateSpacingKm);
        
//   RadxField *backscatRatioField =
//     ray->addField(Names::BackScatterRatio_F, "", nGates, Radx::missingFl32, 
//                   _calcsFilt->getBackscatRatio().data(), true);
//   backscatRatioField->setStandardName(Names::lidar_backscatter_ratio);
//   backscatRatioField->setLongName(Names::lidar_backscatter_ratio);
//   backscatRatioField->setRangeGeom(startRangeKm, gateSpacingKm);
    
//   RadxField *partDepolField =
//     ray->addField(Names::ParticleDepolRatio_F, "", nGates, Radx::missingFl32,
//                   _calcsFilt->getPartDepol().data(), true);
//   partDepolField->setLongName(Names::lidar_particle_depolarization_ratio);
//   partDepolField->setStandardName(Names::lidar_particle_depolarization_ratio);
//   partDepolField->setRangeGeom(startRangeKm, gateSpacingKm);
    
//   RadxField *backscatCoeffField =
//     ray->addField(Names::BackScatterCoeff_F, "m-1.sr-1", nGates, Radx::missingFl32,
//                   _calcsFilt->getBackscatCoeff().data(), true);
//   backscatCoeffField->setStandardName(Names::lidar_backscatter_coefficient);
//   backscatCoeffField->setLongName(Names::lidar_backscatter_coefficient);
//   backscatCoeffField->setRangeGeom(startRangeKm, gateSpacingKm);
   
//   RadxField *extinctionField =
//     ray->addField(Names::ExtinctionCoeff_F, "m-1", nGates, Radx::missingFl32, 
//                   _calcsFilt->getExtinctionCoeff().data(), true);
//   extinctionField->setStandardName(Names::lidar_extinction_coefficient);
//   extinctionField->setLongName(Names::lidar_extinction_coefficient);
//   extinctionField->setRangeGeom(startRangeKm, gateSpacingKm);

//   RadxField *optDepthField =
//     ray->addField(Names::OpticalDepth_F, "", nGates, Radx::missingFl32, 
//                   _calcsFilt->getOpticalDepth().data(), true);
//   optDepthField->setStandardName(Names::lidar_optical_depth);
//   optDepthField->setLongName(Names::lidar_optical_depth);
//   optDepthField->setRangeGeom(startRangeKm, gateSpacingKm);
       
// }

///////////////////////////////////////////////////////
// run speckle filter for a given length
// checks for missing data
//
// minRunLen: length of run being tested for

// void Hsrl2Radx::_applyMissingSpeckleFilter(int nGates,
//                                            int minRunLen,
//                                            Radx::fl32 *data)
  
// {

//   int count = 0;
//   // loop through all gates
//   for (int ii = 0; ii < nGates; ii++) {
//     // check for non-missing
//     if (data[ii] != Radx::missingFl32) {
//       // set, so count up length of run
//       count++;
//     } else {
//       // not set, end of run
//       if (count <= minRunLen) {
//         // run too short, indicates possible speckle
//         for (int jj = ii - count; jj < ii; jj++) {
//           // remove speckle gates
//           data[jj] = Radx::missingFl32;
//         }
//       }
//       count = 0;
//     }
//   } // ii

// }

// ///////////////////////////////////////////////////////
// // run speckle filter for a given length
// // checks for zero data
// //
// // minRunLen: length of run being tested for

// void Hsrl2Radx::_applyZeroSpeckleFilter(int nGates,
//                                            int minRunLen,
//                                            Radx::fl32 *data)
  
// {

//   int count = 0;
//   // loop through all gates
//   for (int ii = 0; ii < nGates; ii++) {
//     // check for non-zero
//     if (data[ii] != 0.0) {
//       // set, so count up length of run
//       count++;
//     } else {
//       // not set, end of run
//       if (count <= minRunLen) {
//         // run too short, indicates possible speckle
//         for (int jj = ii - count; jj < ii; jj++) {
//           // remove speckle gates
//           data[jj] = 0.0;
//         }
//       }
//       count = 0;
//     }
//   } // ii

// }

