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
// RadxTimeStats.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2013
//
///////////////////////////////////////////////////////////////

#include "RadxTimeStats.hh"
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

RadxTimeStats::RadxTimeStats(int argc, char **argv)
  
{

  OK = TRUE;
  _nVols = 0;
  _nGates = 0;
  _allocNeeded = true;
  
  // set programe name

  _progName = "RadxTimeStats";
  
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

RadxTimeStats::~RadxTimeStats()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RadxTimeStats::Run()
{

  // set up angle list
  
  _initAngleList();
    
  // get the input paths based on mode
  
  vector<string> inputPaths;
  
  if (_params.mode == Params::ARCHIVE) {
    
    // get the files to be processed
    
    RadxTimeList tlist;
    tlist.setDir(_params.input_dir);
    tlist.setModeInterval(_args.startTime, _args.endTime);
    if (tlist.compile()) {
      LOG(ERROR) << "ERROR - RadxTimeStats::_runFilelist()";
      LOG(ERROR) << "  Cannot compile time list, dir: " << _params.input_dir;
      LOG(ERROR) << "  Start time: " << RadxTime::strm(_args.startTime);
      LOG(ERROR) << "  End time: " << RadxTime::strm(_args.endTime);
      LOG(ERROR) << tlist.getErrStr();
      return -1;
    }
    
    inputPaths = tlist.getPathList();
    if (inputPaths.size() < 1) {
      LOG(ERROR) << "ERROR - RadxTimeStats - ARCHIVE mode";
      LOG(ERROR) << "  No files found, dir: " << _params.input_dir;
      return -1;
    }
    
  } else {
    
    inputPaths = _args.inputFileList;

  }
    
  // loop through the input file list
  
  int iret = 0;
  for (size_t ii = 0; ii < inputPaths.size(); ii++) {
    if (_processFile(inputPaths[ii])) {
      iret = -1;
    }
  }
  return iret;

}

//////////////////////////////////////////////////
// Process a file
// Returns 0 on success, -1 on failure

int RadxTimeStats::_processFile(const string &filePath)
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
  
  // initialize the stats volume from the relevant scans in the file
  
  if (_initStatsVol()) {
    return -1;
  }
  
  // analyze this volume data set
  
  if (_analyzeVol()) {
    LOG(ERROR) << "ERROR - RadxTimeStats::_processFile()";
    LOG(ERROR) << "  Cannot process data in file: " << _readPath;
    return -1;
  }

  // write out the results

  if (_writeStatsVol()) {
    LOG(ERROR) << "ERROR - RadxTimeStats::_processFile()";
    LOG(ERROR) << "  Cannot write out clutter volume";
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// set up read

void RadxTimeStats::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  file.setReadIgnoreTransitions(true);
  file.setReadMaxRangeKm(_params.max_range_km);


  file.addReadField(_params.stats_field_name);
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }

}

//////////////////////////////////////////////////
// Read in a file
// Returns 0 on success, -1 on failure

int RadxTimeStats::_readFile(const string &filePath)
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
  
  LOG(DEBUG) << "INFO - RadxTimeStats::_readFile";
  LOG(DEBUG) << "  Input path: " << filePath;
  
  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  _readVol.clear();
  if (inFile.readFromPath(filePath, _readVol)) {
    LOG(ERROR) << "ERROR - RadxTimeStats::_readFile";
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

///////////////////////////////////////////
// process this data set

int RadxTimeStats::_analyzeVol()
  
{

  LOG(DEBUG) << "Processing data set ...";

  // add to DBZ sum and compute mean

  vector<RadxRay *> &rays = _statsVol.getRays();
  for (size_t iray = 0; iray < _nRaysStats; iray++) {
    RadxRay *ray = rays[iray];

    RadxField *inputFld = ray->getField(_params.stats_field_name);
    if (inputFld == NULL) {
      continue;
    }
    Radx::fl32 inputMiss = inputFld->getMissingFl32();
    Radx::fl32 *inputVals = inputFld->getDataFl32();
    
    Radx::fl32 *count = _count[iray];
    Radx::fl32 *sum = _sum[iray];
    Radx::fl32 *sqSum = _sqSum[iray];
    Radx::fl32 *mean = _mean[iray];
    Radx::fl32 *sdev = _sdev[iray];
    // Radx::fl32 *mode = _mode[iray];
    // Radx::fl32 *median = _median[iray];
    
    for (size_t igate = 0; igate < _nGates; igate++) {
      Radx::fl32 inputVal = inputVals[igate];
      if (inputVal != inputMiss) {
        sum[igate] += inputVal;
        sqSum[igate] += (inputVal * inputVal);
        count[igate] += 1.0;
      }
      double countVal = count[igate];
      if (countVal > 0) {
        mean[igate] = sum[igate] / countVal;
      }
      if (countVal > 1) {
        double sumVal = sum[igate];
        double sqSumVal = sqSum[igate];
        double varVal =
          (sqSumVal - (sumVal * sumVal) / countVal) / (countVal - 1.0);
        if (varVal > 0.0) {
          sdev[igate] = sqrt(varVal);
        } else {
          sdev[igate] = 0.0;
        }
      }
    } // igate

    // add output fields to ray

    RadxField *meanFldOut = new RadxField(*inputFld);
    meanFldOut->setName(_params.mean_field_name);
    meanFldOut->setDataFl32(_nGates, mean, true);
    
    RadxField *sdevFldOut = new RadxField(*inputFld);
    sdevFldOut->setName(_params.sdev_field_name);
    sdevFldOut->setDataFl32(_nGates, sdev, true);
    
    ray->addField(meanFldOut);
    ray->addField(sdevFldOut);
    
  } // iray

  // compute the clutter frequency histogram
  
  // for (size_t iray = 0; iray < _nRaysStats; iray++) {
  //   Radx::fl32 *clutFreq = _clutFreq[iray];
  //   for (size_t igate = 0; igate < _nGates; igate++) {
  //     _clutFreqHist.update(clutFreq[igate]);
  //   }
  // }
  // _clutFreqHist.computeVariance();
  
  // if (_params.debug >= Params::DEBUG_VERBOSE) {
  //   _clutFreqHist.print(stderr);
  //   // _clutFreqHist.printVariance(stderr);
  // }

#ifdef JUNK  
  // set the clutter flag

  double maxVariance;
  double clutFracThreshold = 0.0;
  if (_params.specify_clutter_frequency_threshold) {
    clutFracThreshold = _params.clutter_frequency_threshold;
  }
  
  LOG(DEBUG) << "  Freq histogram stats - maxVar, clutFracThreshold: "
             << maxVariance << ", " << clutFracThreshold;
  
  for (size_t iray = 0; iray < _nRaysStats; iray++) {
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
  
  for (size_t iray = 0; iray < _nRaysStats; iray++) {
    RadxRay *ray = rays[iray];
    RadxField *dbzFld = ray->getField(_params.stats_field_name);
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
#endif

  return 0;

}

//////////////////////////////////////////////////
// Initialize the angle list
// Returns 0 on success, -1 on failure

void RadxTimeStats::_initAngleList()
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

int RadxTimeStats::_checkGeom()
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
      LOG(ERROR) << "ERROR - RadxTimeStats::_checkGeom()";
      LOG(ERROR) << "  startRange changed: " << radxStartRange;
      LOG(ERROR) << "      previous value: " << _radxStartRange;
      iret = -1;
    }

    if (fabs(_radxGateSpacing - radxGateSpacing) > 0.0001) {
      LOG(ERROR) << "ERROR - RadxTimeStats::_checkGeom()";
      LOG(ERROR) << "  gateSpacing changed: " << radxGateSpacing;
      LOG(ERROR) << "      previous value: " << _radxGateSpacing;
      iret = -1;
    }
    
    if (fabs(_radarLatitude - radarLatitude) > 0.0001) {
      LOG(ERROR) << "ERROR - RadxTimeStats::_checkGeom()";
      LOG(ERROR) << "  radarLatitude changed: " << radarLatitude;
      LOG(ERROR) << "        previous value: " << _radarLatitude;
      iret = -1;
    }

    if (fabs(_radarLongitude - radarLongitude) > 0.0001) {
      LOG(ERROR) << "ERROR - RadxTimeStats::_checkGeom()";
      LOG(ERROR) << "  radarLongitude changed: " << radarLongitude;
      LOG(ERROR) << "        previous value: " << _radarLongitude;
      iret = -1;
    }
    
    if (fabs(_radarAltitude - radarAltitude) > 0.0001) {
      LOG(ERROR) << "ERROR - RadxTimeStats::_checkGeom()";
      LOG(ERROR) << "  radarAltitude changed: " << radarAltitude;
      LOG(ERROR) << "        previous value: " << _radarAltitude;
      iret = -1;
    }

    return iret;

  }

  return 0;
  
}

/////////////////////////////////////////////////////////////
// initialize the stats volume from the selected angles
// this is not optimized for speed, but is simple and
// performance is not limiting here.

int RadxTimeStats::_initStatsVol()
{

  const vector<RadxRay *> &rays = _readVol.getRays();
  if (rays.size() < 1) {
    return -1;
  }

  // initialize the clutter volume with the read volume,
  // and then clear out the rays

  _statsVol.clear();
  _statsVol = _readVol;
  _statsVol.clearRays();

  // create empty ray with all gates missing
  
  RadxRay emptyRay(*rays[0]);
  RadxField *dbzFld = emptyRay.getField(_params.stats_field_name);
  if (dbzFld == NULL) {
    LOG(ERROR) << "Input field name not found in ray: " << _params.stats_field_name;
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

      _statsVol.addRay(clutRay);

    } // iang
  } // isweep

  _statsVol.loadSweepInfoFromRays();
  _statsVol.loadVolumeInfoFromRays();
  _nRaysStats = _statsVol.getNRays();
  
  // allocate

  if (_allocNeeded) {
    
    // first time, allocate arrays for analysis
    
    _sum = _sumArray.alloc(_nRaysStats, _nGates);
    _sqSum = _sqSumArray.alloc(_nRaysStats, _nGates);
    _count = _countArray.alloc(_nRaysStats, _nGates);
    _mean = _meanArray.alloc(_nRaysStats, _nGates);
    _sdev = _sdevArray.alloc(_nRaysStats, _nGates);
    _mode = _modeArray.alloc(_nRaysStats, _nGates);
    _median = _medianArray.alloc(_nRaysStats, _nGates);
    _histo = _histoArray.alloc(_nRaysStats, _nGates);

    // initialize to 0
    
    Radx::fl32 *sum1D = _sumArray.dat1D();
    memset(sum1D, 0, _nRaysStats * _nGates * sizeof(Radx::fl32));
    Radx::fl32 *sqSum1D = _sqSumArray.dat1D();
    memset(sqSum1D, 0, _nRaysStats * _nGates * sizeof(Radx::fl32));
    Radx::fl32 *count1D = _countArray.dat1D();
    memset(count1D, 0, _nRaysStats * _nGates * sizeof(Radx::fl32));
    Radx::fl32 *mean1D = _meanArray.dat1D();
    memset(mean1D, 0, _nRaysStats * _nGates * sizeof(Radx::fl32));
    Radx::fl32 *sdev1D = _sdevArray.dat1D();
    memset(sdev1D, 0, _nRaysStats * _nGates * sizeof(Radx::fl32));
    Radx::fl32 *mode1D = _modeArray.dat1D();
    memset(mode1D, 0, _nRaysStats * _nGates * sizeof(Radx::fl32));
    Radx::fl32 *median1D = _medianArray.dat1D();
    memset(median1D, 0, _nRaysStats * _nGates * sizeof(Radx::fl32));

    double maxVal = _params.max_expected_value;
    double minVal = _params.min_expected_value;
    double range = maxVal - minVal;
    double nbins = 50.0;
    double deltaVal = range / nbins;
    for (size_t iray = 0; iray < _nRaysStats; iray++) {
      for (size_t igate = 0; igate < _nGates; igate++) {
        Histo &hist = _histo[iray][igate];
        hist.init(minVal, deltaVal, maxVal);
      } // igate
    } // iray

    _allocNeeded = false;

  }

  return 0;
  
}

//////////////////////////////////////////////////
// set up write

void RadxTimeStats::_setupWrite(RadxFile &file)
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
// Write the stats volume

int RadxTimeStats::_writeStatsVol()

{

  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  string outputDir = _params.output_dir;
  
  // write to dir
  
  if (outFile.writeToDir(_statsVol, outputDir, true, false)) {
    LOG(ERROR) << "ERROR - RadxConvert::_writeStatsVol";
    LOG(ERROR) << "  Cannot write file to dir: " << outputDir;
    LOG(ERROR) << outFile.getErrStr();
    return -1;
  }

  return 0;

}

///////////////////////////////////////////
// get matching ray from stats volume

RadxRay *RadxTimeStats::_getStatsRay(RadxRay *ray)
{

  const vector<RadxRay *> &clutRays = _statsVol.getRays();
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

