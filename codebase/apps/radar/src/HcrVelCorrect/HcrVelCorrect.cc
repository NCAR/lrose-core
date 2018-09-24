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
// HcrVelCorrect.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2015
//
//////////////////////////////////////////////////////////////////////////
//
// HcrVelCorrect reads in HCR moments, computes the apparent velocity
// of the ground echo, filters the apparent velocity in time to remove
// spurious spikes, and then corrects the weather echo velocity using
// the filtered ground velocity as the correction to be applied.
//
//////////////////////////////////////////////////////////////////////////

#include "HcrVelCorrect.hh"
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxXml.hh>
#include <dsserver/DsLdataInfo.hh>
#include <Spdb/DsSpdb.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/pmu.h>
#include <algorithm>
using namespace std;

// Constructor

HcrVelCorrect::HcrVelCorrect(int argc, char **argv)
  
{

  OK = TRUE;

  // set programe name

  _progName = "HcrVelCorrect";
  
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
  
  // set up the surface velocity object

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _surfVel.setDebug(true);
  } else if (_params.debug >= Params::DEBUG_EXTRA) {
    _surfVel.setVerbose(true);
  }

  _surfVel.setDbzFieldName(_params.dbz_field_name);
  _surfVel.setVelFieldName(_params.vel_field_name);

  _surfVel.setMinRangeToSurfaceKm(_params.min_range_to_surface_km);
  _surfVel.setMaxSurfaceHeightKm(_params.max_surface_height_km);
  _surfVel.setMinDbzForSurfaceEcho(_params.min_dbz_for_surface_echo);
  _surfVel.setNGatesForSurfaceEcho(_params.ngates_for_surface_echo);
  _surfVel.setMaxNadirErrorDeg(_params.max_nadir_error_for_surface_vel);
  
  // initialize the wave filtering

  _initWaveFilt();

  // initialize the FIR filtering

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _firFilt.setDebug(true);
  } else if (_params.debug >= Params::DEBUG_EXTRA) {
    _firFilt.setVerbose(true);
  }

  _firFilt.setSpikeFilterDifferenceThreshold
    (_params.spike_filter_difference_threshold);
  
  _firFilt.initFirFilters(_params.stage1_filter_n,
                          _params._stage1_filter,
                          _params.spike_filter_n,
                          _params._spike_filter,
                          _params.final_filter_n,
                          _params._final_filter);

  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

}

// destructor

HcrVelCorrect::~HcrVelCorrect()

{

  // write out rays which have not yet been written
  // this only applies to the wave filter option
  
  if (_params.filter_type == Params::WAVE_FILTER) {
    while (_filtQueue.size() > 1) {
      FiltNode &oldest = _filtQueue[0];
      // not yet written out, do so now
      _addNodeRayToFiltVol(oldest);
      if (_params.write_surface_vel_results_to_spdb) {
        _writeFirFiltResultsToSpdb(oldest.ray);
      }
      RadxRay::deleteIfUnused(oldest.ray);
      _filtQueue.pop_front();
    }
  }

  // write out any pending data

  _writeFiltVol();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int HcrVelCorrect::Run()
{

  switch (_params.mode) {
    case Params::ARCHIVE:
      return _runArchive();
    case Params::REALTIME:
      return _runRealtime();
    case Params::FILELIST:
    default:
      return _runFilelist();
  } // switch

}

//////////////////////////////////////////////////
// Run in filelist mode

int HcrVelCorrect::_runFilelist()
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

int HcrVelCorrect::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - HcrVelCorrect::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - HcrVelCorrect::_runFilelist()" << endl;
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

int HcrVelCorrect::_runRealtime()
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
// Process a file
// Returns 0 on success, -1 on failure

int HcrVelCorrect::_processFile(const string &readPath)
{

  PMU_auto_register("Processing file");

  // check we have not already processed this file
  // in the file aggregation step
  
  for (size_t ii = 0; ii < _readPaths.size(); ii++) {
    RadxPath rpath(_readPaths[ii]);
    if (readPath.find(rpath.getFile()) != string::npos) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Skipping file: " << readPath << endl;
        cerr << "  Previously processed in aggregation step" << endl;
      }
      return 0;
    }
  }
  
  if (_params.debug) {
    cerr << "INFO - HcrVelCorrect::Run" << endl;
    cerr << "  Input path: " << readPath << endl;
  }
  
  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file

  if (inFile.readFromPath(readPath, _inVol)) {
    cerr << "ERROR - HcrVelCorrect::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  _readPaths = inFile.getReadPaths();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t ii = 0; ii < _readPaths.size(); ii++) {
      cerr << "  ==>> read in file: " << _readPaths[ii] << endl;
    }
  }

  // copy the meta data to the filtered vol

  _filtVol.copyMeta(_inVol);
  
  // save the end time, so we know when to write out file
  // associated with this data
  
  RadxTime fileEndTime(_inVol.getEndTimeSecs(), 
                       _inVol.getEndNanoSecs() / 1.0e9);
  _inputFileEndTime.push_back(fileEndTime);
  
  // process each ray in the volume
  
  int iret = 0;
  vector<RadxRay *> rays = _inVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {

    // create a copy of the ray
    // add a client to the copy to keep track of usage
    // the filter and the write volume will both try to delete
    // the ray if no longer used
    
    RadxRay *rayCopy = new RadxRay(*rays[iray]);
    rayCopy->addClient();
    
    // process the ray
    // computing vel and filtering

    if (_params.filter_type == Params::WAVE_FILTER) {
      if (_processRayWaveFilt(rayCopy)) {
        iret = -1;
      }
    } else {
      if (_processRayFirFilt(rayCopy)) {
        iret = -1;
      }
    } // if (_params.filter_type == Params::WAVE_FILTER) 
    
  } // iray
  
  return iret;

}

//////////////////////////////////////////////////
// Process a ray, filtering with FIR filter

int HcrVelCorrect::_processRayFirFilt(RadxRay *ray)
  
{

  // init

  _velIsValid = true;

  // get surface vel
  
  double velSurf, dbzSurf, rangeToSurf;
  if (_surfVel.computeSurfaceVel(ray,
                                 velSurf,
                                 dbzSurf,
                                 rangeToSurf)) {
    velSurf = 0.0;
    rangeToSurf = 0.0;
    dbzSurf = -9999.0;
    _velIsValid = false;
  }

  // apply FIR filter to get filtered velocity

  _velFilt = 0.0;
  if (_firFilt.filterRay(ray, velSurf, dbzSurf, rangeToSurf)) {
    return -1;
  }
  
  _filtRay = _firFilt.getFiltRay();
  if (_velIsValid) {
    _velIsValid = _firFilt.velocityIsValid();
  }
  if (_velIsValid) {
    _velFilt = _firFilt.getVelFilt();
    _correctVelForRay(_filtRay, _velFilt);
  } else {
    _copyVelForRay(_filtRay);
  }
  
  RadxTime filtRayTime = _filtRay->getRadxTime();
  
  // write vol when done
  
  if ((_inputFileEndTime.size() > 0) &&
      (filtRayTime > _inputFileEndTime[0])) {
    _writeFiltVol();
    _inputFileEndTime.pop_front();
  }
  
  // add to output vol
  
  _filtVol.addRay(_filtRay);
  
  // write results to SPDB in XML if requested
  
  if (_params.write_surface_vel_results_to_spdb) {
    _writeFirFiltResultsToSpdb(_filtRay);
  }

  return 0;

}
  
//////////////////////////////////////////////////
// Process a ray, filtering with wave filter

int HcrVelCorrect::_processRayWaveFilt(RadxRay *ray)
  
{

  // init
  
  _velIsValid = true;

  // get surface vel
  
  double velSurf, dbzSurf, rangeToSurf;
  if (_surfVel.computeSurfaceVel(ray,
                                 velSurf,
                                 dbzSurf,
                                 rangeToSurf)) {
    velSurf = 0.0;
    rangeToSurf = 0.0;
    dbzSurf = -9999.0;
    _velIsValid = false;
  }

  // apply wave filter to get filtered velocity

  _velFilt = 0.0;

  if (_applyWaveFilt(ray, velSurf, dbzSurf, rangeToSurf)) {
    return -1;
  }
  
  _filtRay = _waveNodeMid->ray;
  if (_velIsValid) {
    if (_params.wave_filter_type == Params::WAVE_MEAN) {
      _velFilt = _waveNodeMid->velWaveFiltMean;
    } else if (_params.wave_filter_type == Params::WAVE_MEDIAN) {
      _velFilt = _waveNodeMid->velWaveFiltMedian;
    } else {
      _velFilt = _waveNodeMid->velWaveFiltPoly;
    }
    _correctVelForRay(_filtRay, _velFilt);
    _waveNodeMid->velIsValid = true;
    _waveNodeMid->corrected = true;
  } else {
    _copyVelForRay(_filtRay);
    _waveNodeMid->velIsValid = false;
    _waveNodeMid->corrected = false;
  }
  
  RadxTime filtRayTime = _filtRay->getRadxTime();
  
  // write vol when done
  
  if ((_inputFileEndTime.size() > 0) &&
      (filtRayTime > _inputFileEndTime[0])) {
    _writeFiltVol();
    _inputFileEndTime.pop_front();
  }
  
  // add to output vol
  
  _filtVol.addRay(_filtRay);
  
  // write results to SPDB in XML if requested
  
  if (_params.write_surface_vel_results_to_spdb) {
    _writeWaveFiltResultsToSpdb(_filtRay);
  }
  
  return 0;

}
  
///////////////////////////////////////////////////////////////////
// Initialize the wave filter

void HcrVelCorrect::_initWaveFilt()

{

  // init

  _nNoiseNodes = 0;

  _velIsValid = false;
  _filtRay = NULL;
  _velFilt = 0.0;

  _waveIndexStart = 0;
  _waveIndexMid = 0;
  _waveIndexEnd = 0;
  _waveNodeMid = NULL;

  _noiseIndexStart = 0;
  _noiseIndexMid = 0;
  _noiseIndexEnd = 0;
  _noiseNodeMid = NULL;

  // filter length
  
  _noiseFiltSecs = _params.noise_filter_length_secs;
  _waveFiltSecs = _params.wave_filter_length_secs;
  _totalFiltSecs = _noiseFiltSecs + _waveFiltSecs;

  _poly.setOrder(_params.wave_polynomial_order);

}

///////////////////////////////////////////////////////////////////
// Apply the wave filter an incoming ray, filtering the surface vel.
// Returns 0 on success, -1 on failure.

int HcrVelCorrect::_applyWaveFilt(RadxRay *ray,
                                  double velSurf,
                                  double dbzSurf,
                                  double rangeToSurf)

{

  // add new node to queue
  // oldest nodes are at the front, youngest nodes are at the back
  // so time increases from front to back
  // initialize all velocities with surface vel

  FiltNode node;
  node.dbzSurf = dbzSurf;
  node.rangeToSurf = rangeToSurf;
  node.ray = ray;
  node.velSurf = velSurf;
  node.velNoiseFilt = velSurf;
  node.velNoiseFiltMean = velSurf;
  node.velNoiseFiltMedian = velSurf;
  node.velWaveFiltMean = velSurf;
  node.velWaveFiltMedian = velSurf;
  node.velWaveFiltPoly = velSurf;
  _filtQueue.push_back(node);

  // Set the time limits for the filters
  // this will also write out any rays that are discarded
  // without having been written

  if (_setFilterLimits()) {
    return -1;
  }

  if ((int) _filtQueue.size() < _params.wave_filter_min_n_rays) {
    _velIsValid = false;
    return 0;
  }

  // run the noise filter first

  _runNoiseFilter();

  // then run the wave filter

  _runWaveFilter();

  return 0;

}

//////////////////////////////////////////////////
// Set the time limits for the filters
// Side effects:
//   compute times
//   write out rays to be discarded that have not been written
//   determine if we have enough data for valid stats

int HcrVelCorrect::_setFilterLimits()
{

  if (_filtQueue.size() < 1) {
    return -1;
  }

  // set up noise filter time limits
  
  _noiseTimeEnd = _filtQueue[_filtQueue.size()-1].getTime();
  _noiseTimeStart = _noiseTimeEnd - _noiseFiltSecs;
  
  // compute start time for wave filter
  
  _waveTimeEnd = _noiseTimeStart;
  _wavePeriodStart = _waveTimeEnd - _waveFiltSecs;
  
  // discard nodes older than _wavePeriodStart,
  // writing out rays from discarded nodes
  
  while (_filtQueue.size() > 1) {
    FiltNode &oldest = _filtQueue[0];
    if (oldest.getTime() < _wavePeriodStart) {
      // not yet written out, do so now
      _addNodeRayToFiltVol(oldest);
      RadxRay::deleteIfUnused(oldest.ray);
      _filtQueue.pop_front();
    } else {
      break;
    }
  } // while

  // set up noise filter index limits
  
  _noiseIndexStart = _filtQueue.size() - 1;
  _noiseIndexEnd = _filtQueue.size() - 1;
  _nNoiseNodes = 0;
  bool noiseFound = false;
  for (size_t ii = 0; ii < _filtQueue.size(); ii++) {
    RadxTime nodeTime = _filtQueue[ii].getTime();
    if (nodeTime >= _noiseTimeStart && nodeTime <= _noiseTimeEnd) {
      _nNoiseNodes++;
      if (!noiseFound) {
        _noiseIndexStart = ii;
        noiseFound = true;
      }
      _noiseIndexEnd = ii;
    }
  }
  _noiseIndexMid = (_noiseIndexStart + _noiseIndexEnd) / 2;
  _noiseNodeMid = &_filtQueue[_noiseIndexMid];
  
  if (_noiseIndexStart < 2) {
    return -1;
  }

  // compute wave filter index limits

  FiltNode &oldest = _filtQueue[0];
  _waveTimeStart = oldest.getTime();
  _waveIndexStart = 0;
  _waveIndexEnd = _noiseIndexStart - 1;
  _waveTimeEnd = _filtQueue[_waveIndexEnd].getTime();
  
  // find the mid node closest to the mean time

  double waveSecs = _waveTimeEnd - _waveTimeStart;
  _waveTimeMean = _waveTimeEnd - waveSecs / 2.0;
  double minTimeDiff = 1.0e99;
  for (size_t ii = 0; ii < _filtQueue.size(); ii++) {
    RadxTime nodeTime = _filtQueue[ii].getTime();
    double timeDiff = fabs(_filtQueue[ii].getTime() - _waveTimeMean);
    if (timeDiff < minTimeDiff) {
      _waveIndexMid = ii;
      _waveNodeMid = &_filtQueue[_waveIndexMid];
      minTimeDiff = timeDiff;
    }
  }
  _waveTimeMid = _waveNodeMid->getTime();
  
  return 0;
  
}

//////////////////////////////////////////////////
// run the noise filter

void HcrVelCorrect::_runNoiseFilter()
{

  // get vector of surface velocities
  
  vector<double> velSurf;
  for (size_t ii = _noiseIndexStart; ii <= _noiseIndexEnd; ii++) {
    velSurf.push_back(_filtQueue[ii].velSurf);
  }
  
  // compute the mean

  double sum = 0.0;
  double count = 0.0;
  for (size_t ii = 0; ii < velSurf.size(); ii++) {
    sum += velSurf[ii];
    count++;
  }

  if (count < 1) {
    _velIsValid = false;
    return;
  }

  double mean = sum / count;

  // compute the median
  
  sort(velSurf.begin(), velSurf.end());
  int indexHalf = velSurf.size() / 2;
  double median = velSurf[indexHalf];

  // set the filtered value on all younger nodes

  for (size_t ii = _noiseIndexMid; ii <= _noiseIndexEnd; ii++) {
    _filtQueue[ii].velNoiseFiltMean = mean;
    _filtQueue[ii].velNoiseFiltMedian = median;
    _filtQueue[ii].velNoiseFilt = median;
  }

}

//////////////////////////////////////////////////
// run the wave filter

void HcrVelCorrect::_runWaveFilter()
{

  // get vector of surface velocities

  vector<double> velNoiseFilt;
  vector<double> dtime;
  RadxTime startTime = _filtQueue[0].getTime();
  for (size_t ii = _waveIndexStart; ii <= _waveIndexEnd; ii++) {
    velNoiseFilt.push_back(_filtQueue[ii].velNoiseFilt);
    double deltaTime = _filtQueue[ii].getTime() - startTime;
    dtime.push_back(deltaTime);
  }
  
  // compute the mean
  
  double sum = 0.0;
  double count = 0.0;
  for (size_t ii = 0; ii < velNoiseFilt.size(); ii++) {
    sum += velNoiseFilt[ii];
    count++;
  }

  if (count < 1) {
    _velIsValid = false;
    return;
  }

  double mean = sum / count;
  _waveNodeMid->velWaveFiltMean = mean;
  for (size_t ii = _waveIndexMid; ii <= _waveIndexEnd; ii++) {
    _filtQueue[ii].velWaveFiltMean = mean;
  }

  // perform the polynomial fit

  _poly.clear();
  _poly.setValues(dtime, velNoiseFilt);
  if (_poly.performFit() == 0) {
    for (size_t ii = _waveIndexMid; ii <= _waveIndexEnd; ii++) {
      _filtQueue[ii].velWaveFiltPoly = _poly.getYEst(ii);
    }
  }

  // compute the median

  sort(velNoiseFilt.begin(), velNoiseFilt.end());
  int indexHalf = velNoiseFilt.size() / 2;
  double median = velNoiseFilt[indexHalf];
  for (size_t ii = _waveIndexMid; ii <= _waveIndexEnd; ii++) {
    _filtQueue[ii].velWaveFiltMedian = median;
  }

}

//////////////////////////////////////////////////
// add a ray to the filtered volume
// write out the vol as needed

void HcrVelCorrect::_addNodeRayToFiltVol(FiltNode &node)
{
  
  // write out if latest time is past input file time
  
  RadxTime rayTime = node.ray->getRadxTime();
  if ((_inputFileEndTime.size() > 0) &&
      (rayTime > _inputFileEndTime[0])) {
    _writeFiltVol();
    _inputFileEndTime.pop_front();
  }

  // if not yet corrected, copy velocity in ray fields
  
  if (!node.corrected) {
    _copyVelForRay(node.ray);
  }
  
  // add to output vol
  
  _filtVol.addRay(node.ray);
  
}

//////////////////////////////////////////////////
// set up read

void HcrVelCorrect::_setupRead(RadxFile &file)
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
// convert all fields to specified output encoding

void HcrVelCorrect::_convertFieldsForOutput(RadxVol &vol)
{

  switch(_params.output_encoding) {
    case Params::OUTPUT_ENCODING_INT32:
      vol.convertToSi32();
      return;
    case Params::OUTPUT_ENCODING_INT16:
      vol.convertToSi16();
      return;
    case Params::OUTPUT_ENCODING_INT08:
      vol.convertToSi08();
      return;
    case Params::OUTPUT_ENCODING_FLOAT32:
    default:
      vol.convertToFl32();
      return;
  }

}

//////////////////////////////////////////////////
// write out the filtered volume

int HcrVelCorrect::_writeFiltVol()
{

  // set output encoding on all fields
  
  _convertFieldsForOutput(_filtVol);
  
  // set global attributes

  _setGlobalAttr(_filtVol);

  // set properties
  
  _filtVol.loadVolumeInfoFromRays();
  _filtVol.loadSweepInfoFromRays();

  // create output file
  
  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  if (_params.output_filename_mode == Params::SPECIFY_FILE_NAME) {
    
    string outPath = _params.output_dir;
    outPath += PATH_DELIM;
    outPath += _params.output_filename;

    // write to path
    
    if (outFile.writeToPath(_filtVol, outPath)) {
      cerr << "ERROR - HcrVelCorrect::_writeFiltVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
      
  } else {

    // write to dir
  
    if (outFile.writeToDir(_filtVol, _params.output_dir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - HcrVelCorrect::_writeFiltVol" << endl;
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
    if (ldata.write(_filtVol.getEndTimeSecs())) {
      cerr << "WARNING - HcrVelCorrect::_writeFiltVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  // free up rays on vol

  _filtVol.clearRays();
  _filtVol.clearSweeps();
  _filtVol.clearRcalibs();
  _filtVol.clearCfactors();

  // copy the metadata from the current input volume

  _filtVol.copyMeta(_inVol);

  return 0;

}

//////////////////////////////////////////////////
// set up write

void HcrVelCorrect::_setupWrite(RadxFile &file)
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

  // set output format to CfRadial

  file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  file.setWriteCompressed(true);
  file.setCompressionLevel(_params.compression_level);

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

}

//////////////////////////////////////////////////
// set selected global attributes

void HcrVelCorrect::_setGlobalAttr(RadxVol &vol)
{

  vol.setDriver("HcrVelCorrect(NCAR)");
  
  string history(vol.getHistory());
  history += "\n";
  history += "Velocity filtering applied using HcrVelCorrect\n";
  vol.setHistory(history);
  
  RadxTime now(RadxTime::NOW);
  vol.setCreated(now.asString());

}

//////////////////////////////////////////////////
// correct velocity on ray

void HcrVelCorrect::_correctVelForRay(RadxRay *ray, double surfFilt)

{

  RadxField *velField = ray->getField(_params.vel_field_name);
  if (velField == NULL) {
    // no vel field, nothing to do
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - no vel field found: " << _params.vel_field_name << endl;
    }
    return;
  }

  // create the corrected field
  
  RadxField *correctedField = new RadxField(_params.corrected_vel_field_name,
                                            velField->getUnits());
  correctedField->copyMetaData(*velField);
  correctedField->setName(_params.corrected_vel_field_name);

  // correct the values
  
  const Radx::fl32 *vel = velField->getDataFl32();
  Radx::fl32 miss = velField->getMissingFl32();
  Radx::fl32 *corrected = new Radx::fl32[velField->getNPoints()];
  for (size_t ii = 0; ii < velField->getNPoints(); ii++) {
    if (vel[ii] != miss) {
      corrected[ii] = vel[ii] - surfFilt;
    } else {
      corrected[ii] = miss;
    }
  }

  // set data for field

  correctedField->setDataFl32(velField->getNPoints(), corrected, true);
  delete[] corrected;

  // add field to ray

  ray->addField(correctedField);

}

//////////////////////////////////////////////////
// copy velocity across for yay

void HcrVelCorrect::_copyVelForRay(RadxRay *ray)
  
{
  
  RadxField *velField = ray->getField(_params.vel_field_name);
  if (velField == NULL) {
    // no vel field, nothing to do
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - no vel field found: "
           << _params.vel_field_name << endl;
    }
    return;
  }

  // create the field to be copied
  
  RadxField *copyField = new RadxField(_params.corrected_vel_field_name,
                                       velField->getUnits());
  copyField->copyMetaData(*velField);
  copyField->setName(_params.corrected_vel_field_name);
  
  // copy the values
  
  const Radx::fl32 *vel = velField->getDataFl32();
  Radx::fl32 *copy = new Radx::fl32[velField->getNPoints()];
  for (size_t ii = 0; ii < velField->getNPoints(); ii++) {
    copy[ii] = vel[ii];
  }

  // set data for field

  copyField->setDataFl32(velField->getNPoints(), copy, true);
  delete[] copy;

  // add field to ray

  ray->addField(copyField);

}


//////////////////////////////////////////////////
// write wave filter results to SPDB in XML

void HcrVelCorrect::_writeWaveFiltResultsToSpdb(const RadxRay *filtRay)
  
{

  // check if we have a good velocity
  
  if (_waveNodeMid == NULL || !_waveNodeMid->velIsValid) {
    return;
  }
  
  // form XML string

  string xml;
  xml += RadxXml::writeStartTag("HcrVelCorr", 0);

  xml += RadxXml::writeDouble("VelSurf", 1,
                              _waveNodeMid->velSurf);
  xml += RadxXml::writeDouble("DbzSurf", 1,
                              _waveNodeMid->dbzSurf);
  xml += RadxXml::writeDouble("RangeToSurf",
                              1, _waveNodeMid->rangeToSurf);
  
  xml += RadxXml::writeDouble("VelNoiseFilt", 1,
                              _waveNodeMid->velNoiseFilt);
  xml += RadxXml::writeDouble("VelNoiseFiltMean", 1,
                              _waveNodeMid->velNoiseFiltMean);
  xml += RadxXml::writeDouble("VelNoiseFiltMedian", 1,
                              _waveNodeMid->velNoiseFiltMedian);
  xml += RadxXml::writeDouble("VelWaveFiltMean", 1,
                              _waveNodeMid->velWaveFiltMean);
  xml += RadxXml::writeDouble("VelWaveFiltMedian", 1,
                              _waveNodeMid->velWaveFiltMedian);
  xml += RadxXml::writeDouble("VelWaveFiltPoly", 1,
                              _waveNodeMid->velWaveFiltPoly);
  
  double velCorr = _waveNodeMid->velSurf - _velFilt;
  xml += RadxXml::writeDouble("VelCorr", 1, velCorr);
  
  const RadxGeoref *georef = filtRay->getGeoreference();
  if (georef != NULL) {
    xml += RadxXml::writeDouble("Altitude", 1, georef->getAltitudeKmMsl());
    xml += RadxXml::writeDouble("VertVel", 1, georef->getVertVelocity());
    xml += RadxXml::writeDouble("Roll", 1, georef->getRoll());
    xml += RadxXml::writeDouble("Pitch", 1, georef->getPitch());
    xml += RadxXml::writeDouble("Rotation", 1, georef->getRotation());
    xml += RadxXml::writeDouble("Tilt", 1, georef->getTilt());
    xml += RadxXml::writeDouble("Elevation", 1, filtRay->getElevationDeg());
    xml += RadxXml::writeDouble("DriveAngle1", 1, georef->getDriveAngle1());
    xml += RadxXml::writeDouble("DriveAngle2", 1, georef->getDriveAngle2());
  }
  
  xml += RadxXml::writeEndTag("HcrVelCorr", 0);
  
  // write to SPDB

  DsSpdb spdb;
  time_t validTime = filtRay->getTimeSecs();
  spdb.addPutChunk(0, validTime, validTime, xml.size() + 1, xml.c_str());
  if (spdb.put(_params.surface_vel_results_spdb_output_url,
               SPDB_XML_ID, SPDB_XML_LABEL)) {
    cerr << "ERROR - HcrVelCorrect::_writeWaveFiltResultsToSpdb" << endl;
    cerr << spdb.getErrStr() << endl;
    return;
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Wrote HCR wave filt vel correction results to spdb, url: " 
         << _params.surface_vel_results_spdb_output_url << endl;
    cerr << "=====================================" << endl;
    cerr << xml;
    cerr << "=====================================" << endl;
  }
 
}

//////////////////////////////////////////////////
// write FIR filt results to SPDB in XML

void HcrVelCorrect::_writeFirFiltResultsToSpdb(const RadxRay *filtRay)
  
{

  // check if we have a good velocity
  
  if (!_firFilt.velocityIsValid()) {
    return;
  }
  
  // form XML string

  string xml;
  xml += RadxXml::writeStartTag("HcrVelCorr", 0);

  xml += RadxXml::writeDouble("VelSurf", 1,
                              _firFilt.getVelMeasured());
  xml += RadxXml::writeDouble("DbzSurf", 1,
                              _firFilt.getDbzSurf());
  xml += RadxXml::writeDouble("RangeToSurf", 1,
                              _firFilt.getRangeToSurface());
  xml += RadxXml::writeDouble("VelStage1", 1,
                              _firFilt.getVelStage1());
  xml += RadxXml::writeDouble("VelSpike", 1,
                              _firFilt.getVelSpike());
  xml += RadxXml::writeDouble("VelCond", 1,
                              _firFilt.getVelCond());
  xml += RadxXml::writeDouble("VelFilt", 1,
                              _firFilt.getVelFilt());
  double velCorr = _firFilt.getVelMeasured() - _velFilt;
  xml += RadxXml::writeDouble("VelCorr", 1, velCorr);
  
  const RadxGeoref *georef = filtRay->getGeoreference();
  if (georef != NULL) {
    xml += RadxXml::writeDouble("Altitude", 1, georef->getAltitudeKmMsl());
    xml += RadxXml::writeDouble("VertVel", 1, georef->getVertVelocity());
    xml += RadxXml::writeDouble("Roll", 1, georef->getRoll());
    xml += RadxXml::writeDouble("Pitch", 1, georef->getPitch());
    xml += RadxXml::writeDouble("Rotation", 1, georef->getRotation());
    xml += RadxXml::writeDouble("Tilt", 1, georef->getTilt());
    xml += RadxXml::writeDouble("DriveAngle1", 1, georef->getDriveAngle1());
    xml += RadxXml::writeDouble("DriveAngle2", 1, georef->getDriveAngle2());
  }

  xml += RadxXml::writeEndTag("HcrVelCorr", 0);

  // write to SPDB
  
  DsSpdb spdb;
  time_t validTime = filtRay->getTimeSecs();
  spdb.addPutChunk(0, validTime, validTime, xml.size() + 1, xml.c_str());
  if (spdb.put(_params.surface_vel_results_spdb_output_url,
               SPDB_XML_ID, SPDB_XML_LABEL)) {
    cerr << "ERROR - HcrVelCorrect::_writeFirFiltResultsToSpdb" << endl;
    cerr << spdb.getErrStr() << endl;
    return;
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Wrote HCR FIR filtering vel correction results to spdb, url: " 
         << _params.surface_vel_results_spdb_output_url << endl;
    cerr << "=====================================" << endl;
    cerr << xml;
    cerr << "=====================================" << endl;
  }
 
}

