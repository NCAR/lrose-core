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
// ** 2) Redistributions of source code must retain the above conypyright      
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
// RadxClutter.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2022
//
///////////////////////////////////////////////////////////////

#include "RadxClutter.hh"
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxComplex.hh>
#include <Mdv/GenericRadxFile.hh>
#include <dsserver/DsLdataInfo.hh>
#include <toolsa/pmu.h>
#include <toolsa/LogStream.hh>
#include <toolsa/LogStreamInit.hh>
#include <algorithm>

using namespace std;

// Constructor

RadxClutter::RadxClutter(int argc, char **argv)
  
{

  OK = TRUE;
  _nVols = 0;
  _nGates = 0;
  _allocNeeded = true;
  
  // set programe name

  _progName = "RadxClutter";
  
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

  // init process mapper registration

  if (_params.mode == Params::REALTIME) {
    PMU_auto_init( _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

  // initialize logging
  LogStreamInit::init(false, false, true, true);
  LOG_STREAM_DISABLE(LogStream::WARNING);
  LOG_STREAM_DISABLE(LogStream::DEBUG);
  LOG_STREAM_DISABLE(LogStream::DEBUG_VERBOSE);
  LOG_STREAM_DISABLE(LogStream::DEBUG_EXTRA);
  if (_params.debug >= Params::DEBUG_EXTRA) {
    LOG_STREAM_ENABLE(LogStream::DEBUG_EXTRA);
    LOG_STREAM_ENABLE(LogStream::DEBUG_VERBOSE);
    LOG_STREAM_ENABLE(LogStream::DEBUG);
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    LOG_STREAM_ENABLE(LogStream::DEBUG_VERBOSE);
    LOG_STREAM_ENABLE(LogStream::DEBUG);
  } else if (_params.debug) {
    LOG_STREAM_ENABLE(LogStream::DEBUG);
    LOG_STREAM_ENABLE(LogStream::WARNING);
  }

}

// destructor

RadxClutter::~RadxClutter()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RadxClutter::Run()
{

  // initialize
  
  if (_params.action == Params::ANALYZE_CLUTTER) {

    // ANALYZE_CLUTTER
    // set up angle list
    
    _initAngleList();
    
  } else {

    // CLUTTER_REMOVAL
    // read in the clutter volume
    
    if (_readClutterFile(_params.clutter_stats_path)) {
      return -1;
    }

  } // if (_params.action == Params::ANALYZE_CLUTTER)


  
  // run based on mode
  
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

int RadxClutter::_runFilelist()
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

int RadxClutter::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    LOG(ERROR) << "ERROR - RadxClutter::_runFilelist()";
    LOG(ERROR) << "  Cannot compile time list, dir: " << _params.input_dir;
    LOG(ERROR) << "  Start time: " << RadxTime::strm(_args.startTime);
    LOG(ERROR) << "  End time: " << RadxTime::strm(_args.endTime);
    LOG(ERROR) << tlist.getErrStr();
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    LOG(ERROR) << "ERROR - RadxClutter::_runFilelist()";
    LOG(ERROR) << "  No files found, dir: " << _params.input_dir;
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
// Run in realtime mode

int RadxClutter::_runRealtime()
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

int RadxClutter::_processFile(const string &filePath)
{

  // read in the file

  _readPath = filePath;
  if (_readFile(_readPath)) {
    return -1;
  }

  // check for substr
  
  string subStr = _params.file_name_substr;
  if (subStr.size() > 0) {
    RadxPath rpath(_readPath);
    string fileName = rpath.getFile();
    if (fileName.find(subStr) == string::npos) {
      LOG(DEBUG) << "Looking for substr: " << subStr;
      LOG(DEBUG) << "No substr found, ignoring file: " << _readPath;
      return 0;
    }
  }
  
  // check if this is an RHI
  
  _isRhi = _readVol.checkIsRhi();
  if (_params.scan_mode == Params::PPI) {
    if (_isRhi) {
      LOG(ERROR) << "Scan mode is not PPI, ignoring file: " << _readPath;
      return -1;
    }
  } else {
    if (!_isRhi) {
      LOG(ERROR) << "Scan mode is not RHI, ignoring file: " << _readPath;
      return -1;
    }
  }

  // check the geometry has not changed

  if (_checkGeom()) {
    return -1;
  }
  _nVols++;
  
  if (_params.action == Params::ANALYZE_CLUTTER) {
    return _performAnalysis();
  } else {
    return _performFiltering();
  }

}

//////////////////////////////////////////////////
// Run analysis on this file
// Returns 0 on success, -1 on failure

int RadxClutter::_performAnalysis()
{

  // initialize the clutter volume from the relevant scans in the file

  if (_initClutterVol()) {
    return -1;
  }

  // initialize the histogram for the clutter frequency
  
  _clutFreqHist.init(0.0, 0.02, 1.0);

  // analyze this data set
  
  if (_analyzeClutter()) {
    LOG(ERROR) << "ERROR - RadxClutter::_performAnalysis()";
    LOG(ERROR) << "  Cannot process data in file: " << _readPath;
    return -1;
  }

  // write out the results

  if (_writeClutterVol()) {
    LOG(ERROR) << "ERROR - RadxClutter::_performAnalysis()";
    LOG(ERROR) << "  Cannot write out clutter volume";
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// Remove clutter from a file
// Returns 0 on success, -1 on failure

int RadxClutter::_performFiltering()
{

  // analyze this data set
  
  if (_filterClutter()) {
    LOG(ERROR) << "ERROR - RadxClutter::_performFiltering";
    LOG(ERROR) << "  Cannot process data in file: " << _readPath;
    return -1;
  }

  // write out the results

  if (_writeFiltVol()) {
    LOG(ERROR) << "ERROR - RadxClutter::Run";
    LOG(ERROR) << "  Cannot write out filtered volume";
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// set up read

void RadxClutter::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  file.setReadIgnoreTransitions(true);
  file.setReadMaxRangeKm(_params.max_range_km);


  if (_params.action == Params::ANALYZE_CLUTTER) {
    file.addReadField(_params.dbz_field_name);
    if (_params.use_vel_field) {
      file.addReadField(_params.vel_field_name);
    }
  } else {
    if (_params.specify_output_fields) {
      file.addReadField(_params.dbz_field_name);
      for (int ii = 0; ii < _params.output_fields_n; ii++) {
        file.addReadField(_params._output_fields[ii]);
      }
    }
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }

}

//////////////////////////////////////////////////
// Read in a file
// Returns 0 on success, -1 on failure

int RadxClutter::_readFile(const string &filePath)
{

  // check we have not already processed this file
  // in the file aggregation step
  
  RadxPath thisPath(filePath);
  for (size_t ii = 0; ii < _readPaths.size(); ii++) {
    RadxPath rpath(_readPaths[ii]);
    if (thisPath.getFile() == rpath.getFile()) {
      LOG(DEBUG_VERBOSE) << "Skipping file: " << filePath;
      LOG(DEBUG_VERBOSE) << "  Previously processed in aggregation step";
      return 0;
    }
  }
  
  LOG(DEBUG) << "INFO - RadxClutter::_readFile";
  LOG(DEBUG) << "  Input path: " << filePath;
  
  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  _readVol.clear();
  if (inFile.readFromPath(filePath, _readVol)) {
    LOG(ERROR) << "ERROR - RadxClutter::_readFile";
    LOG(ERROR) << inFile.getErrStr();
    return -1;
  }
  _readPaths = inFile.getReadPaths();

  // convert to floats
  
  _readVol.convertToFl32();
  
  // set number of gates constant
  
  _readVol.setNGatesConstant();

  if (_readVol.getNRays() < 3) {
    return -1;
  } else {
    return 0;
  }
  
}

//////////////////////////////////////////////////
// Read in the process clutter file
// Returns 0 on success, -1 on failure

int RadxClutter::_readClutterFile(const string &clutterPath)
{

  GenericRadxFile inFile;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    inFile.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    inFile.setVerbose(true);
    inFile.printReadRequest(cerr);
  }
  
  // read in file
  
  _clutterVol.clear();
  if (inFile.readFromPath(clutterPath, _clutterVol)) {
    LOG(ERROR) << "ERROR - RadxClutter::_readClutterFile";
    LOG(ERROR) << inFile.getErrStr();
    return -1;
  }
  
  // convert to floats
  
  _clutterVol.convertToFl32();
  
  // set number of gates constant
  
  _readVol.setNGatesConstant();

  LOG(DEBUG) << "INFO - RadxClutter::_readClutterFile";
  LOG(DEBUG) << "  Read in clutter path: " << clutterPath;
  
  if (_clutterVol.getNRays() < 3) {
    return -1;
  } else {
    return 0;
  }
  
}

///////////////////////////////////////////
// process this data set

int RadxClutter::_analyzeClutter()
  
{

  LOG(DEBUG) << "Processing data set ...";

  // add to DBZ sum and compute mean

  vector<RadxRay *> &rays = _clutterVol.getRays();
  for (size_t iray = 0; iray < _nRaysClutter; iray++) {
    RadxRay *ray = rays[iray];

    RadxField *dbzFld = ray->getField(_params.dbz_field_name);
    if (dbzFld == NULL) {
      continue;
    }
    Radx::fl32 dbzMiss = dbzFld->getMissingFl32();
    Radx::fl32 *dbzVals = dbzFld->getDataFl32();
    
    RadxField *velFld = ray->getField(_params.vel_field_name);
    Radx::fl32 velMiss = Radx::missingFl32;
    if (velFld != NULL) {
      velMiss = velFld->getMissingFl32();
    }
    Radx::fl32 *velVals = NULL;
    if (velFld) {
      velVals = velFld->getDataFl32();
    }
    
    Radx::fl32 *dbzCount = _dbzCount[iray];
    Radx::fl32 *dbzSum = _dbzSum[iray];
    Radx::fl32 *dbzSqSum = _dbzSqSum[iray];
    Radx::fl32 *dbzMean = _dbzMean[iray];
    Radx::fl32 *dbzSdev = _dbzSdev[iray];
    Radx::fl32 *clutSum = _clutSum[iray];
    Radx::fl32 *clutFreq = _clutFreq[iray];
    
    for (size_t igate = 0; igate < _nGates; igate++) {
      Radx::fl32 dbzVal = dbzVals[igate];
      if (dbzVal != dbzMiss) {
        dbzSum[igate] += dbzVal;
        dbzSqSum[igate] += (dbzVal * dbzVal);
        dbzCount[igate] += 1.0;
        bool isClutter = false;
        if (dbzVal >= _params.clutter_dbz_threshold) {
          isClutter = true; // dbz exceeds threshold - could be clutter
        }
        if (velVals) {
          Radx::fl32 velVal = velVals[igate];
          if (velVal != velMiss && fabs(velVal) > _params.max_abs_vel) {
            isClutter = false; // vel exceeds threshold - not clutter
          }
        }
        if (isClutter) {
          clutSum[igate] += 1.0;
        }
      }
      double count = dbzCount[igate];
      if (count > 0) {
        dbzMean[igate] = dbzSum[igate] / count;
        clutFreq[igate] = clutSum[igate] / count;
      }
      if (count > 1) {
        double sum = dbzSum[igate];
        double sumSq = dbzSqSum[igate];
        double var =
          (sumSq - (sum * sum) / count) / (count - 1.0);
        if (var > 0.0) {
          dbzSdev[igate] = sqrt(var);
        } else {
          dbzSdev[igate] = 0.0;
        }
      }
    } // igate

    // add output fields to ray

    RadxField *dbzMeanFldOut = new RadxField(*dbzFld);
    dbzMeanFldOut->setName(_params.dbz_mean_field_name);
    dbzMeanFldOut->setDataFl32(_nGates, dbzMean, true);
    
    RadxField *dbzSdevFldOut = new RadxField(*dbzFld);
    dbzSdevFldOut->setName(_params.dbz_sdev_field_name);
    dbzSdevFldOut->setDataFl32(_nGates, dbzSdev, true);
    
    RadxField *clutFreqFldOut = new RadxField(*dbzFld);
    clutFreqFldOut->setName(_params.clut_freq_field_name);
    clutFreqFldOut->setLongName("ClutterTimeFraction");
    clutFreqFldOut->setUnits("");
    clutFreqFldOut->setDataFl32(_nGates, clutFreq, true);

    ray->addField(dbzMeanFldOut);
    ray->addField(dbzSdevFldOut);
    ray->addField(clutFreqFldOut);
    
  } // iray

  // compute the clutter frequency histogram
  
  for (size_t iray = 0; iray < _nRaysClutter; iray++) {
    Radx::fl32 *clutFreq = _clutFreq[iray];
    for (size_t igate = 0; igate < _nGates; igate++) {
      _clutFreqHist.update(clutFreq[igate]);
    }
  }
  _clutFreqHist.computeVariance();
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _clutFreqHist.print(stderr);
    _clutFreqHist.printVariance(stderr);
  }

  // set the clutter flag

  double maxVariance;
  double clutFracThreshold = _clutFreqHist.getFreqForMaxVar(maxVariance);
  if (_params.specify_clutter_frequency_threshold) {
    clutFracThreshold = _params.clutter_frequency_threshold;
  }
  
  LOG(DEBUG) << "  Freq histogram stats - maxVar, clutFracThreshold: "
             << maxVariance << ", " << clutFracThreshold;
  
  for (size_t iray = 0; iray < _nRaysClutter; iray++) {
    Radx::fl32 *clutFreq = _clutFreq[iray];
    Radx::fl32 *clutFlag = _clutFlag[iray];
    for (size_t igate = 0; igate < _nGates; igate++) {
      if (clutFreq[igate] > clutFracThreshold) {
        clutFlag[igate] = 1.0;
      } else {
        clutFlag[igate] = 0.0;
      }
    } // igate
  } // iray

  // add the clutter flag to the output
  
  for (size_t iray = 0; iray < _nRaysClutter; iray++) {
    RadxRay *ray = rays[iray];
    RadxField *dbzFld = ray->getField(_params.dbz_field_name);
    if (dbzFld == NULL) {
      continue;
    }
    Radx::fl32 *clutFlag = _clutFlag[iray];
    RadxField *clutFlagFldOut = new RadxField(*dbzFld);
    clutFlagFldOut->setName(_params.clut_flag_field_name);
    clutFlagFldOut->setLongName("ClutterFlag");
    clutFlagFldOut->setUnits("");
    clutFlagFldOut->setDataFl32(_nGates, clutFlag, true);
    ray->addField(clutFlagFldOut);
  } // iray

  return 0;

}

//////////////////////////////////////////////////
// Initialize the angle list
// Returns 0 on success, -1 on failure

void RadxClutter::_initAngleList()
{

  _fixedAngles.clear();
  _scanAngles.clear();
  
  for (int ii = 0; ii < _params.sweep_fixed_angles_n; ii++) {
    _fixedAngles.push_back(_params._sweep_fixed_angles[ii]);
  }
  sort(_fixedAngles.begin(), _fixedAngles.end());

  if (_params.scan_mode == Params::PPI) {
    
    // PPI clutter scan - compute azimuths
    
    double sectorDelta = _params.last_ray_angle - _params.first_ray_angle;
    if (_params.first_ray_angle > _params.last_ray_angle) {
      sectorDelta += 360.0;
    }

    double az = _params.first_ray_angle;
    double sumDelta = 0.0;
    while (sumDelta < sectorDelta) {
      _scanAngles.push_back(az);
      az = RadxComplex::computeSumDeg(az, _params.delta_ray_angle);
      if (az < 0) {
        az += 360.0;
      }
      sumDelta += fabs(_params.delta_ray_angle);
    } // while (sumDelta < sectorDelta)
    
  } else {

    // RHI clutter scan - compute RHI elevations
    
    double sectorDelta = _params.last_ray_angle - _params.first_ray_angle;

    double el = _params.first_ray_angle;
    double sumDelta = 0.0;
    while (sumDelta < sectorDelta) {
      _scanAngles.push_back(el);
      el = RadxComplex::computeSumDeg(el, _params.delta_ray_angle);
      sumDelta += fabs(_params.delta_ray_angle);
    } // while (sumDelta < sectorDelta)

  } // if (_params.scan_mode == Params::PPI)
  
}

/////////////////////////////////////////////////////////////
// check ray geometry in volume

int RadxClutter::_checkGeom()
{
  
  const vector<RadxRay *> &rays = _readVol.getRays();
  if (rays.size() < 1) {
    return -1;
  }
  
  if (_nVols == 0) {

    // initialize geom
    
    _nGates = _readVol.getMaxNGates();
    _radxStartRange = _readVol.getStartRangeKm();
    _radxGateSpacing = _readVol.getGateSpacingKm();
    
    _radarLatitude = _readVol.getLatitudeDeg();
    _radarLongitude = _readVol.getLongitudeDeg();
    _radarAltitude = _readVol.getAltitudeKm();
    
  } else {

    // check geom

    size_t nGates = _readVol.getMaxNGates();
    if (nGates != _nGates) {
      _readVol.setNGates(_nGates);
    }

    double radxStartRange = _readVol.getStartRangeKm();
    double radxGateSpacing = _readVol.getGateSpacingKm();
    
    double radarLatitude = _readVol.getLatitudeDeg();
    double radarLongitude = _readVol.getLongitudeDeg();
    double radarAltitude = _readVol.getAltitudeKm();

    int iret = 0;

    if (fabs(_radxStartRange - radxStartRange) > 0.0001) {
      LOG(ERROR) << "ERROR - RadxClutter::_checkGeom()";
      LOG(ERROR) << "  startRange changed: " << radxStartRange;
      LOG(ERROR) << "      previous value: " << _radxStartRange;
      iret = -1;
    }

    if (fabs(_radxGateSpacing - radxGateSpacing) > 0.0001) {
      LOG(ERROR) << "ERROR - RadxClutter::_checkGeom()";
      LOG(ERROR) << "  gateSpacing changed: " << radxGateSpacing;
      LOG(ERROR) << "      previous value: " << _radxGateSpacing;
      iret = -1;
    }
    
    if (fabs(_radarLatitude - radarLatitude) > 0.0001) {
      LOG(ERROR) << "ERROR - RadxClutter::_checkGeom()";
      LOG(ERROR) << "  radarLatitude changed: " << radarLatitude;
      LOG(ERROR) << "        previous value: " << _radarLatitude;
      iret = -1;
    }

    if (fabs(_radarLongitude - radarLongitude) > 0.0001) {
      LOG(ERROR) << "ERROR - RadxClutter::_checkGeom()";
      LOG(ERROR) << "  radarLongitude changed: " << radarLongitude;
      LOG(ERROR) << "        previous value: " << _radarLongitude;
      iret = -1;
    }
    
    if (fabs(_radarAltitude - radarAltitude) > 0.0001) {
      LOG(ERROR) << "ERROR - RadxClutter::_checkGeom()";
      LOG(ERROR) << "  radarAltitude changed: " << radarAltitude;
      LOG(ERROR) << "        previous value: " << _radarAltitude;
      iret = -1;
    }

    return iret;

  }

  return 0;
  
}

/////////////////////////////////////////////////////////////
// initialize the clutter volume from the selected angles
// this is not optimized for speed, but is simple and
// performance is not limiting here.

int RadxClutter::_initClutterVol()
{

  const vector<RadxRay *> &rays = _readVol.getRays();
  if (rays.size() < 1) {
    return -1;
  }

  // initialize the clutter volume with the read volume,
  // and then clear out the rays

  _clutterVol.clear();
  _clutterVol = _readVol;
  _clutterVol.clearRays();

  // create empty ray with all gates missing
  
  RadxRay emptyRay(*rays[0]);
  RadxField *dbzFld = emptyRay.getField(_params.dbz_field_name);
  if (dbzFld == NULL) {
    LOG(ERROR) << "DBZ field name not found in ray: " << _params.dbz_field_name;
    return -1;
  }
  dbzFld->setGatesToMissing(0, dbzFld->getNPoints() - 1);
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "==>> looking for rays that match clutter volume <<==" << endl;
  }
  
  // loop through the sweeps and scan angles

  for (size_t isweep = 0; isweep < _fixedAngles.size(); isweep++) {
    for (size_t iang = 0; iang < _scanAngles.size(); iang++) {

      // get the clutter angle
      
      double clutEl = _fixedAngles[isweep];
      double clutAz = _scanAngles[iang];
      if (_isRhi) {
        clutEl = _scanAngles[iang];
        clutAz = _fixedAngles[isweep];
      }

      // find the closest ray in the measured volume
      
      double minAngDist = 1.0e6;
      double matchAz = -9999;
      double matchEl = -9999;
      size_t matchRayIndex = 0;
      RadxRay *matchRay = NULL;
      bool matchFound = false;
      
      for (size_t ii = 0; ii < rays.size(); ii++) {
        RadxRay *ray = rays[ii];
        double rayEl = ray->getElevationDeg();
        double rayAz = ray->getAzimuthDeg();
        double dEl = fabs(rayEl - clutEl);
        double dAz = fabs(rayAz - clutAz);
        if (dEl > _params.elev_tolerance_deg ||
            dAz > _params.az_tolerance_deg) {
          continue;
        }
        double angDist = sqrt(dEl * dEl + dAz * dAz);
        if (angDist < minAngDist) {
          minAngDist = angDist;
          matchEl = rayEl;
          matchAz = rayAz;
          matchRay = ray;
          matchRayIndex = ii;
          matchFound = true;
        }
      } // ii

      // create the clutter ray
      
      RadxRay *clutRay = NULL;
      if (matchFound) {
        clutRay = new RadxRay(*matchRay);
        if (_params.debug >= Params::DEBUG_EXTRA) {
          fprintf(stderr,
                  "====>> isweep, iang, index, rayEl, matchEl, "
                  "rayAz, matchAz, dEl, dAz: "
                  "%3ld %3ld %5ld %7.3f %7.3f %7.3f %7.3f %7.3f %7.3f\n",
                  isweep, iang, matchRayIndex,
                  clutEl, matchEl, clutAz, matchAz,
                  fabs(clutEl - matchEl),
                  fabs(clutAz - matchAz));
        }
      } else {
        clutRay = new RadxRay(emptyRay);
        if (_params.debug >= Params::DEBUG_EXTRA) {
          fprintf(stderr,
                  "====>> isweep, iang, index, rayEl, matchEl, "
                  "rayAz, matchAz, dEl, dAz: "
                  "%3ld %3ld %5ld %7.3f %7.3f %7.3f %7.3f %7.3f %7.3f\n",
                  isweep, iang, matchRayIndex,
                  clutEl, matchEl, clutAz, matchAz,
                  fabs(clutEl - matchEl),
                  fabs(clutAz - matchAz));
        }
      }
      clutRay->setElevationDeg(clutEl);
      clutRay->setAzimuthDeg(clutAz);
      if (_isRhi) {
        clutRay->setFixedAngleDeg(clutAz);
      } else {
        clutRay->setFixedAngleDeg(clutEl);
      }
      clutRay->clearEventFlags();
      clutRay->setSweepNumber(isweep);
      
      // add to the clutter vol

      _clutterVol.addRay(clutRay);

    } // iang
  } // isweep

  _clutterVol.loadSweepInfoFromRays();
  _clutterVol.loadVolumeInfoFromRays();
  _nRaysClutter = _clutterVol.getNRays();
  
  // allocate

  if (_allocNeeded) {
    
    // first time, allocate arrays for analysis
    
    _dbzSum = _dbzSumArray.alloc(_nRaysClutter, _nGates);
    _dbzSqSum = _dbzSqSumArray.alloc(_nRaysClutter, _nGates);
    _dbzCount = _dbzCountArray.alloc(_nRaysClutter, _nGates);
    _dbzMean = _dbzMeanArray.alloc(_nRaysClutter, _nGates);
    _dbzSdev = _dbzSdevArray.alloc(_nRaysClutter, _nGates);
    _clutSum = _clutSumArray.alloc(_nRaysClutter, _nGates);
    _clutFreq = _clutFreqArray.alloc(_nRaysClutter, _nGates);
    _clutFlag = _clutFlagArray.alloc(_nRaysClutter, _nGates);

    // initialize to 0
    
    Radx::fl32 *dbzSum1D = _dbzSumArray.dat1D();
    memset(dbzSum1D, 0, _nRaysClutter * _nGates * sizeof(Radx::fl32));
    Radx::fl32 *dbzSqSum1D = _dbzSqSumArray.dat1D();
    memset(dbzSqSum1D, 0, _nRaysClutter * _nGates * sizeof(Radx::fl32));
    Radx::fl32 *dbzCount1D = _dbzCountArray.dat1D();
    memset(dbzCount1D, 0, _nRaysClutter * _nGates * sizeof(Radx::fl32));
    Radx::fl32 *dbzMean1D = _dbzMeanArray.dat1D();
    memset(dbzMean1D, 0, _nRaysClutter * _nGates * sizeof(Radx::fl32));
    Radx::fl32 *dbzSdev1D = _dbzSdevArray.dat1D();
    memset(dbzSdev1D, 0, _nRaysClutter * _nGates * sizeof(Radx::fl32));

    Radx::fl32 *clutSum1D = _clutSumArray.dat1D();
    memset(clutSum1D, 0, _nRaysClutter * _nGates * sizeof(Radx::fl32));
    Radx::fl32 *clutFreq1D = _clutFreqArray.dat1D();
    memset(clutFreq1D, 0, _nRaysClutter * _nGates * sizeof(Radx::fl32));
    Radx::fl32 *clutFlag1D = _clutFlagArray.dat1D();
    memset(clutFlag1D, 0, _nRaysClutter * _nGates * sizeof(Radx::fl32));

    _allocNeeded = false;

  }

  return 0;
  
}

//////////////////////////////////////////////////
// set up write

void RadxClutter::_setupWrite(RadxFile &file)
{

  if (_params.debug) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_AND_END_TIMES);

  file.setWriteCompressed(true);
  file.setCompressionLevel(4);

}

//////////////////////////////////////
// Write the clutter volume

int RadxClutter::_writeClutterVol()

{

  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  string outputDir = _params.clutter_stats_output_dir;
  
  // write to dir
  
  if (outFile.writeToDir(_clutterVol, outputDir, true, false)) {
    LOG(ERROR) << "ERROR - RadxConvert::_writeClutterVol";
    LOG(ERROR) << "  Cannot write file to dir: " << outputDir;
    LOG(ERROR) << outFile.getErrStr();
    return -1;
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
    ldata.setDataFileExt("nc");
    ldata.setDataType("netCDF");
    
    string fileName;
    RadxPath::stripDir(_params.clutter_stats_output_dir, outputPath, fileName);
    ldata.setRelDataPath(fileName);
    
    ldata.setIsFcast(false);
    ldata.write(_readVol.getStartTimeSecs());
    
    LOG(DEBUG) << "RadxClutter::_writeClutterVol(): Data written to "
               << outputPath;
    
  }

  return 0;

}

///////////////////////////////////////////
// get matching ray from clutter volume

RadxRay *RadxClutter::_getClutRay(RadxRay *ray)
{

  const vector<RadxRay *> &clutRays = _clutterVol.getRays();
  if (clutRays.size() < 1) {
    return NULL;
  }

  // get the el/az for this ray
  
  double el = ray->getElevationDeg();
  double az = ray->getAzimuthDeg();

  // find the closest ray
  
  double minAngDist = 1.0e6;
  RadxRay *matchRay = NULL;
  
  for (size_t iray = 0; iray < clutRays.size(); iray++) {
    RadxRay *clutRay = clutRays[iray];
    double clutEl = clutRay->getElevationDeg();
    double clutAz = clutRay->getAzimuthDeg();
    double dEl = fabs(el - clutEl);
    double dAz = fabs(az - clutAz);
    if (dEl > _params.elev_tolerance_deg ||
        dAz > _params.az_tolerance_deg) {
      continue;
    }
    double angDist = sqrt(dEl * dEl + dAz * dAz);
    if (angDist < minAngDist) {
      minAngDist = angDist;
      matchRay = clutRay;
    }
  } // ii
  
  return matchRay;
  
}

///////////////////////////////////////////
// remove clutter from the volume

int RadxClutter::_filterClutter()
  
{

  LOG(DEBUG) << "Filtering clutter from volume ..." << _readPath;
  
  // copy the read volume
  
  _filtVol = _readVol;
  
  // loop through the rays in the read volume
  
  const vector<RadxRay *> &rays = _filtVol.getRays();
  if (rays.size() < 1) {
    return -1;
  }

  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    RadxRay *ray = rays[iray];
    RadxRay *clutRay = _getClutRay(ray);
    if (clutRay != NULL) {
      _filterRay(ray, clutRay);
    }

  } // iray
  
  return 0;
  
}

//////////////////////////////////////
// Filter a ray based on a clutter ray

void RadxClutter::_filterRay(RadxRay *ray, const RadxRay *clutRay)

{

  // get the dbz field
  
  RadxField *dbzFld = ray->getField(_params.dbz_field_name);
  if (dbzFld == NULL) {
    return;
  }
  Radx::fl32 dbzMiss = dbzFld->getMissingFl32();
  Radx::fl32 *dbzVals = dbzFld->getDataFl32();

  // get the clutter stats
  
  const RadxField *dbzMeanFld = clutRay->getField(_params.dbz_mean_field_name);
  if (dbzMeanFld == NULL) {
    return;
  }
  Radx::fl32 dbzMeanMiss = dbzMeanFld->getMissingFl32();
  const Radx::fl32 *dbzMeanVals = dbzMeanFld->getDataFl32();

  const RadxField *dbzSdevFld = clutRay->getField(_params.dbz_sdev_field_name);
  if (dbzSdevFld == NULL) {
    return;
  }
  Radx::fl32 dbzSdevMiss = dbzSdevFld->getMissingFl32();
  const Radx::fl32 *dbzSdevVals = dbzSdevFld->getDataFl32();

  const RadxField *clutFreqFld = clutRay->getField(_params.clut_freq_field_name);
  if (clutFreqFld == NULL) {
    return;
  }
  Radx::fl32 clutFreqMiss = clutFreqFld->getMissingFl32();
  const Radx::fl32 *clutFreqVals = clutFreqFld->getDataFl32();

  const RadxField *clutFlagFld = clutRay->getField(_params.clut_flag_field_name);
  if (clutFlagFld == NULL) {
    return;
  }
  Radx::fl32 clutFlagMiss = clutFlagFld->getMissingFl32();
  const Radx::fl32 *clutFlagVals = clutFlagFld->getDataFl32();

  // create the filt dbz field
  
  RadxField *filtFld = new RadxField(*dbzFld);
  Radx::fl32 filtMiss = filtFld->getMissingFl32();
  filtFld->setName(_params.dbz_filt_field_name);
  filtFld->setLongName("Filtered-reflectivity");
  Radx::fl32 *filtVals = filtFld->getDataFl32();

  for (size_t ii = 0; ii < _nGates; ii++) {
    
    filtVals[ii] = filtMiss;
    
    Radx::fl32 dbz = dbzVals[ii];
    if (dbz == dbzMiss) {
      continue;
    }

    Radx::fl32 dbzMean = dbzMeanVals[ii];
    if (dbzMean == dbzMeanMiss) {
      continue;
    }

    Radx::fl32 dbzSdev = dbzSdevVals[ii];
    if (dbzSdev == dbzSdevMiss) {
      continue;
    }

    // check if we have clutter
    
    Radx::fl32 clutFlag = clutFlagVals[ii];
    if (clutFlag == clutFlagMiss) {
      continue;
    }

    if (_params.specify_filter_frequency_threshold) {
      Radx::fl32 clutFreq = clutFreqVals[ii];
      if (clutFreq == clutFreqMiss) {
        continue;
      }
      if (clutFreq < _params.filter_frequency_threshold) {
        clutFlag = false;
      } else {
        clutFlag = true;
      }
    }

    // compute filtered value
    
    double clutThreshold = dbzMean + dbzSdev * _params.n_sdev_for_clut_threshold;
    filtVals[ii] = dbz;
    if (clutFlag > 0) {
      if (dbz < clutThreshold) {
        filtVals[ii] = _params.min_dbz_filt;
      }
    }

  } // ii
  
  ray->addField(filtFld);

}


//////////////////////////////////////
// Write the clutter-removed volume

int RadxClutter::_writeFiltVol()

{

  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  string outputDir = _params.filt_output_dir;
  
  // write to dir
  
  if (outFile.writeToDir(_filtVol, outputDir, true, false)) {
    LOG(ERROR) << "ERROR - RadxConvert::_writeClutterRemovedVol";
    LOG(ERROR) << "  Cannot write file to dir: " << outputDir;
    LOG(ERROR) << outFile.getErrStr();
    return -1;
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
    ldata.setDataFileExt("nc");
    ldata.setDataType("netCDF");
    
    string fileName;
    RadxPath::stripDir(_params.clutter_stats_output_dir, outputPath, fileName);
    ldata.setRelDataPath(fileName);
    
    ldata.setIsFcast(false);
    ldata.write(_readVol.getStartTimeSecs());
    
    LOG(DEBUG) << "RadxClutter::_writeClutterRemovedVol(): Data written to "
               << outputPath;
    
  }

  return 0;

}
