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
// Also computes spectrum width corrected for aircraft motion.
//
//////////////////////////////////////////////////////////////////////////

#include "HcrVelCorrect.hh"
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxXml.hh>
#include <Radx/RadxCfactors.hh>
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

  // altitude correction

  if (_params.correct_altitude_for_egm) {
    if (_egm.readGeoid(_params.egm_2008_geoid_file)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Altitude correction for geoid." << endl;
      cerr << "  Problem reading geoid file: " 
           << _params.egm_2008_geoid_file << endl;
      OK = FALSE;
      return;
    }
  }

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
        _writeWaveFiltResultsToSpdb(oldest);
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
    // note that the rayCopy memory ownership will be transferred
    // to the filtering methods (_processRayWaveFilt or
    // _processRayFirFilt) so the memory does not need
    // to be freed here
    
    RadxRay *rayCopy = new RadxRay(*rays[iray]);
    rayCopy->addClient();
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "====>>>> waveFilt - reading ray at time: "
           << rayCopy->getRadxTime().asString(3) << endl;
    }
    
    // compute corrected spectrum width

    if (_params.add_corrected_spectrum_width_field) {
      _addCorrectedSpectrumWidth(rayCopy);
    }

    // correct altitude

    if (_params.correct_altitude_for_egm) {
      _correctAltitudeForGeoid(rayCopy);
    }

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

    if (_params.identify_progressive_depol) {
      if (_identProgressiveDepol(rayCopy)) {
        iret = -1;
      }
    }
    
  } // iray
  
  return iret;

}

//////////////////////////////////////////////////
// Process a ray, filtering with FIR filter

int HcrVelCorrect::_processRayFirFilt(RadxRay *ray)
  
{

  // get surface vel
  
  bool velIsValid = true;
  double velSurf, dbzSurf, rangeToSurf;
  if (_surfVel.computeSurfaceVel(ray,
                                 velSurf,
                                 dbzSurf,
                                 rangeToSurf)) {
    velSurf = 0.0;
    rangeToSurf = 0.0;
    dbzSurf = -9999.0;
    velIsValid = false;
  }

  // apply FIR filter to get filtered velocity

  if (_firFilt.filterRay(ray, velSurf, dbzSurf, rangeToSurf)) {
    return -1;
  }
  _filtRay = _firFilt.getFiltRay();
  if (velIsValid) {
    if (!_firFilt.velocityIsValid()) {
      velIsValid = false;
    }
  }

  if (velIsValid) {
    double velSurfFilt = _firFilt.getVelFilt();
    _correctVelForRay(_filtRay, velSurfFilt);
  } else {
    _copyVelForRay(_filtRay);
  }
  
  // write vol when done
  
  if ((_inputFileEndTime.size() > 0) &&
      (_filtRay->getRadxTime() > _inputFileEndTime[0])) {
    _writeFiltVol();
    _inputFileEndTime.pop_front();
  }
  
  // add to output vol
  
  _filtVol.addRay(_filtRay);
  if (_filtRay->getCfactors() != NULL) {
    _filtVol.setCfactors(*_filtRay->getCfactors());
  }
  
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

  // get surface vel
  
  double velSurf, dbzSurf, rangeToSurf;
  bool velIsValid = true;
  if (_surfVel.computeSurfaceVel(ray,
                                 velSurf,
                                 dbzSurf,
                                 rangeToSurf)) {
    velSurf = 0.0;
    rangeToSurf = 0.0;
    dbzSurf = -9999.0;
    velIsValid = false;
  }

  // apply wave filter to get filtered velocity

  bool filtIsValid = true;
  if (_applyWaveFilt(ray, velSurf, dbzSurf, rangeToSurf, velIsValid)) {
    filtIsValid = false;
  }
  
  for (size_t ii = 0; ii < _nodesPending.size(); ii++) {
    FiltNode *node = _nodesPending[ii];
    _filtRay = node->ray;
    if (filtIsValid) {
      _correctVelForRay(_filtRay, node->velWaveFilt);
    } else {
      _copyVelForRay(_filtRay);
    }
    // indicate that the corrected field has been added
    node->corrFieldAdded = true;
  }

  return 0;

}
  
///////////////////////////////////////////////////////////////////
// Initialize the wave filter

void HcrVelCorrect::_initWaveFilt()

{

  // init

  _filtRay = NULL;

  _waveIndexStart = 0;
  _waveIndexMid = 0;
  _waveIndexEnd = 0;
  _waveNodeMid = NULL;

  _noiseIndexStart = 0;
  _noiseIndexMid = 0;
  _noiseIndexEnd = 0;

  // filter length
  
  _noiseFiltSecs = _params.noise_filter_length_secs;
  _waveFiltSecs = _params.wave_filter_length_secs;

  // polynomial filter order

  _poly.setOrder(_params.wave_filter_polynomial_order);

}

///////////////////////////////////////////////////////////////////
// Apply the wave filter an incoming ray, filtering the surface vel.
// Returns 0 on success, -1 on failure.

int HcrVelCorrect::_applyWaveFilt(RadxRay *ray,
                                  double velSurf,
                                  double dbzSurf,
                                  double rangeToSurf,
                                  bool velIsValid)

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
  node.velWaveFilt = velSurf;
  // node.velIsValid = velIsValid;
  _filtQueue.push_back(node);

  // Set the time limits for the filters
  // this will also write out any rays that are discarded
  // without having been written

  if (_setWaveFilterLimits()) {
    return -1;
  }

  if ((int) _filtQueue.size() < 3) {
    return -1;
  }

  // run the noise filter first

  _runNoiseFilter();
  
  // then run the wave filter
  
  _runWaveFilter();
  
  return 0;

}

//////////////////////////////////////////////////
// Set the time limits for the wave filters
// Side effects:
//   compute times
//   write out rays to be discarded that have not been written
//   determine if we have enough data for valid stats

int HcrVelCorrect::_setWaveFilterLimits()
{

  if (_filtQueue.size() < 1) {
    return -1;
  }

  // set up noise filter time limits
  
  RadxTime noiseTimeEnd = _filtQueue[_filtQueue.size()-1].getTime();
  RadxTime noiseTimeStart = noiseTimeEnd - _noiseFiltSecs;
  
  // compute start time for wave filter
  
  RadxTime waveTimeEnd = noiseTimeStart;
  RadxTime wavePeriodStart = waveTimeEnd - _waveFiltSecs;
  
  // discard nodes older than _wavePeriodStart,
  // writing out rays from discarded nodes
  
  while (_filtQueue.size() > 1) {
    FiltNode &oldest = _filtQueue[0];
    if (oldest.getTime() < wavePeriodStart) {
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
  for (ssize_t ii = _filtQueue.size() - 1; ii >= 0; ii--) {
    RadxTime nodeTime = _filtQueue[ii].getTime();
    if (nodeTime < noiseTimeStart) {
      break;
    }
    _noiseIndexStart = ii;
  }
  _noiseIndexMid = (_noiseIndexStart + _noiseIndexEnd) / 2;
  
  // compute wave filter index limits
  
  RadxTime waveTimeStart = _filtQueue[0].getTime();
  _waveIndexStart = 0;
  _waveIndexEnd = _noiseIndexStart;
  waveTimeEnd = _filtQueue[_waveIndexEnd].getTime();
  
  // find the mid node closest to the mean time
  
  double waveSecs = waveTimeEnd - waveTimeStart;
  RadxTime waveTimeMean = waveTimeEnd - waveSecs / 2.0;
  double minTimeDiff = 1.0e99;
  for (size_t ii = 0; ii < _filtQueue.size(); ii++) {
    RadxTime nodeTime = _filtQueue[ii].getTime();
    double timeDiff = fabs(_filtQueue[ii].getTime() - waveTimeMean);
    if (timeDiff < minTimeDiff) {
      _waveIndexMid = ii;
      _waveNodeMid = &_filtQueue[_waveIndexMid];
      minTimeDiff = timeDiff;
    }
  }

  // load up queue of the pending nodes
  // i.e. those for which the corrected field
  // has not yet been added

  _nodesPending.clear();
  for (ssize_t ii = _waveIndexMid; ii >= 0; ii--) {
    if (!_filtQueue[ii].corrFieldAdded) {
      _nodesPending.push_front(&_filtQueue[ii]);
    }
  } // ii
  
  return 0;
  
}

//////////////////////////////////////////////////
// run the noise filter

int HcrVelCorrect::_runNoiseFilter()
{

  // get vector of surface velocities
  
  vector<double> velSurf;
  for (size_t ii = _noiseIndexStart; ii <= _noiseIndexEnd; ii++) {
    if (!std::isnan(_filtQueue[ii].velSurf)) {
      velSurf.push_back(_filtQueue[ii].velSurf);
    }
  }
  if (velSurf.size() < 1) {
    return -1;
  }
  
  // compute the median
  
  sort(velSurf.begin(), velSurf.end());
  int indexHalf = velSurf.size() / 2;
  double median = velSurf[indexHalf];
  
  // set the filtered value on all younger nodes
  
  for (size_t ii = _noiseIndexMid; ii <= _noiseIndexEnd; ii++) {
    _filtQueue[ii].velNoiseFilt = median;
  }

  // set the filtered value on older nodes that have not been set
  
  for (size_t ii = _noiseIndexMid; ii <= _noiseIndexEnd; ii++) {
    if (std::isnan(_filtQueue[ii].velNoiseFilt)) {
      _filtQueue[ii].velNoiseFilt = median;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// run the wave filter

int HcrVelCorrect::_runWaveFilter()
{

  // get vector of surface velocities

  vector<double> velNoiseFilt;
  vector<double> dtimeValid, dtimeFull;
  RadxTime startTime = _filtQueue[0].getTime();
  for (size_t ii = _waveIndexStart; ii <= _waveIndexEnd; ii++) {
    double deltaTime = _filtQueue[ii].getTime() - startTime;
    dtimeFull.push_back(deltaTime);
    if (!std::isnan(_filtQueue[ii].velSurf)) {
      velNoiseFilt.push_back(_filtQueue[ii].velNoiseFilt);
      dtimeValid.push_back(deltaTime);
    }
  }
  
  if (velNoiseFilt.size() < 1) {
    return -1;
  }
  
  // perform the polynomial fit
  
  _poly.clear();
  _poly.setValues(dtimeValid, velNoiseFilt);
  if (_poly.performFit() == 0) {
    // success
    // set the polynomial values on all younger nodes
    for (size_t ii = _waveIndexMid; ii <= _waveIndexEnd; ii++) {
      _filtQueue[ii].velWaveFilt = _poly.getYEst(dtimeFull[ii]);
    }
    // set the polynomial values on all older nodes that 
    // have not yet had the correction field added
    for (size_t ii = 0; ii < _waveIndexMid; ii++) {
      if (!_filtQueue[ii].corrFieldAdded) {
        _filtQueue[ii].velWaveFilt = _poly.getYEst(dtimeFull[ii]);
      }
    }

  }

  return 0;

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

  // add to output vol
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "====>>>> waveFilt - adding ray to output vol, time: "
         << rayTime.asString(3) << endl;
  }

  _filtVol.addRay(node.ray);
  if (_filtRay->getCfactors() != NULL) {
    _filtVol.setCfactors(*_filtRay->getCfactors());
  }
  
  // write vel filtering results to spdb

  if (_params.write_surface_vel_results_to_spdb) {
    _writeWaveFiltResultsToSpdb(node);
  }

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
  velField->setLongName("doppler_velocity_corrected_for_vertical_motion");
  velField->setComment("This field is computed by correcting the raw measured "
                       "velocity for the vertical motion of the aircraft.");

  // create the corrected field
  
  RadxField *corrField = new RadxField(_params.corrected_vel_field_name,
                                       velField->getUnits());
  corrField->copyMetaData(*velField);
  corrField->setName(_params.corrected_vel_field_name);
  corrField->setLongName("doppler_velocity_corrected_using_surface_measurement");
  corrField->setComment("This field is computed by correcting the velocity "
                        "using the measured velocity of the surface echo.");

  // correct the values
  
  const Radx::fl32 *vel = velField->getDataFl32();
  Radx::fl32 miss = velField->getMissingFl32();
  RadxArray<Radx::fl32> corrected_;
  Radx::fl32 *corrected = corrected_.alloc(velField->getNPoints());
  for (size_t ii = 0; ii < velField->getNPoints(); ii++) {
    if (vel[ii] != miss) {
      corrected[ii] = vel[ii] - surfFilt;
    } else {
      corrected[ii] = miss;
    }
  }

  // set data for field
  
  corrField->setDataFl32(velField->getNPoints(), corrected, true);
  
  // add field to ray

  ray->addField(corrField);

  // optionally add in the delta velocity field
  
  if (_params.add_delta_vel_field) {
    _addDeltaField(ray, -surfFilt);
  }

}

//////////////////////////////////////////////////
// copy velocity across for ray

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
  velField->setLongName("doppler_velocity_corrected_for_vertical_motion");
  velField->setComment("This field is computed by correcting the raw measured "
                       "velocity for the vertical motion of the aircraft.");
  
  // create the field to be copied
  
  RadxField *copyField =
    new RadxField(_params.corrected_vel_field_name,
                  velField->getUnits());
  copyField->copyMetaData(*velField);
  copyField->setName(_params.corrected_vel_field_name);
  copyField->setLongName("doppler_velocity_corrected_using_surface_measurement");
  copyField->setComment("This field is computed by correcting the velocity "
                        "using the measured velocity of the surface echo.");
  
  // copy the values
  
  const Radx::fl32 *vel = velField->getDataFl32();
  RadxArray<Radx::fl32> copy_;
  Radx::fl32 *copy = copy_.alloc(velField->getNPoints());
  for (size_t ii = 0; ii < velField->getNPoints(); ii++) {
    copy[ii] = vel[ii];
  }

  // set data for field

  copyField->setDataFl32(velField->getNPoints(), copy, true);

  // add field to ray

  ray->addField(copyField);

  // optionally add in the delta velocity field
  
  if (_params.add_delta_vel_field) {
    _addDeltaField(ray, 0.0);
  }

}


//////////////////////////////////////////////////
// add delta field to ray

void HcrVelCorrect::_addDeltaField(RadxRay *ray, double deltaVel)

{

  RadxField *velField = ray->getField(_params.vel_field_name);
  if (velField == NULL) {
    // no vel field, nothing to do
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - no vel field found: " << _params.vel_field_name << endl;
    }
    return;
  }

  RadxField *deltaField = new RadxField(_params.delta_vel_field_name,
                                        velField->getUnits());
  deltaField->copyMetaData(*velField);
  deltaField->setName(_params.delta_vel_field_name);
  deltaField->setLongName("velocity_delta_from_surface_measurement");
  char comment[2048];
  snprintf(comment, 2048,
           "This is the correction applied to the %s field "
           "to produce the %s field",
           _params.vel_field_name, _params.corrected_vel_field_name);
  deltaField->setComment(comment);
  
  const Radx::fl32 *vel = velField->getDataFl32();
  Radx::fl32 miss = velField->getMissingFl32();
  RadxArray<Radx::fl32> delta_;
  Radx::fl32 *delta = delta_.alloc(velField->getNPoints());
  for (size_t ii = 0; ii < velField->getNPoints(); ii++) {
    if (vel[ii] != miss) {
      delta[ii] = deltaVel;
    } else {
      delta[ii] = miss;
    }
  }
  
  // set data for field
  
  deltaField->setDataFl32(velField->getNPoints(), delta, true);
  
  // add field to ray
  
  ray->addField(deltaField);
  
}

//////////////////////////////////////////////////
// compute and add in the corrected spectrum
// width field

int HcrVelCorrect::_addCorrectedSpectrumWidth(RadxRay *ray)

{
  
  // get the spectrum width field

  const RadxField *widthField = ray->getField(_params.width_field_name);

  // get the aircraft speed

  const RadxGeoref *georef = ray->getGeoreference();
  if (georef == NULL) {
    return -1;
  }
  double ewVel = georef->getEwVelocity();
  double nsVel = georef->getNsVelocity();
  double speed = sqrt(ewVel * ewVel + nsVel * nsVel);

  // compute the delta correction

  double elev = ray->getElevationDeg();
  double sinElev = sin(elev * DEG_TO_RAD);
  double delta =
    fabs(0.3 * speed * sinElev *
         (_params.width_correction_beamwidth_deg * DEG_TO_RAD));
  
  // create a copy of this field

  RadxField *corrWidth = new RadxField(*widthField);
  
  // compute the corrected width for each gate
  
  corrWidth->convertToFl32();
  Radx::fl32 miss = corrWidth->getMissingFl32();
  Radx::fl32 *ww = corrWidth->getDataFl32();
  for (size_t ii = 0; ii < corrWidth->getNPoints(); ii++) {
    if (ww[ii] != miss) {
      double xx = ww[ii] * ww[ii] - delta * delta;
      if (xx < 0.01) {
        xx = 0.01;
      }
      ww[ii] = sqrt(xx);
    }
  }

  // set the name
  
  corrWidth->setName(_params.corrected_width_field_name);
  corrWidth->setLongName("doppler_spectrum_width_corrected_for_aircraft_motion");
  corrWidth->setComment("This field is computed by correcting the raw measured "
                        "spectrum width for the horizontal motion of the aircraft.");

  // add to the ray

  ray->addField(corrWidth);
  
  return 0;

}
  
//////////////////////////////////////////////////
// correct GPS altitude for geoid

void HcrVelCorrect::_correctAltitudeForGeoid(RadxRay *ray)

{
  
  // get the georeference

  RadxGeoref *georef = ray->getGeoreference();
  if (georef == NULL) {
    // nothing we can do
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - HcrVelCorrect::_correctAltitudeForGeoid()" << endl;
      cerr << "No georeference available, cannot fix altitude" << endl;
    }
    return;
  }

  // get correction factors if they are there
  
  const RadxCfactors *cfac_ = ray->getCfactors();
  RadxCfactors cfac;
  if (cfac_ != NULL) {
    cfac = *cfac_;
  }

  // get the geoid delta for the location

  double geoidM = _egm.getInterpGeoidM(georef->getLatitude(),
                                       georef->getLongitude());

  // the altitude correction has the opposite sign, since it
  // is added to the measured altitude

  double altCorrM = geoidM * -1.0;
  cfac.setAltitudeCorr(altCorrM);
  ray->setCfactors(cfac);
  
  double altKmMsl = georef->getAltitudeKmMsl() + altCorrM / 1000.0;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr,
            "==>> correctAltitudeForGeoid: "
            "lat, lon, geoidM, corrM, altBefore, altAfter: "
            "%8.4f %8.4f %6.2f %6.2f %8.4f %8.4f\n",
            georef->getLatitude(),
            georef->getLongitude(),
            geoidM,
            altCorrM,
            georef->getAltitudeKmMsl(),
            altKmMsl);
  }
  
  georef->setAltitudeKmMsl(altKmMsl);

}

//////////////////////////////////////////////////
// write wave filter results to SPDB in XML

void HcrVelCorrect::_writeWaveFiltResultsToSpdb(FiltNode &node)
  
{

  // check if we have a good velocity
  
  if (std::isnan(node.velSurf)) {
    return;
  }
  
  // get the node for this ray

  const RadxRay *ray = node.ray;

  // form XML string

  string xml;
  xml += RadxXml::writeStartTag("HcrVelCorr", 0);

  xml += RadxXml::writeDouble("VelSurf", 1,
                              node.velSurf);
  xml += RadxXml::writeDouble("DbzSurf", 1,
                              node.dbzSurf);
  xml += RadxXml::writeDouble("RangeToSurf",
                              1, node.rangeToSurf);
  
  xml += RadxXml::writeDouble("VelNoiseFilt", 1,
                              node.velNoiseFilt);
  xml += RadxXml::writeDouble("VelWaveFilt", 1,
                              node.velWaveFilt);
  
  double velCorr = node.velSurf - node.velWaveFilt;
  xml += RadxXml::writeDouble("VelCorr", 1, velCorr);
  
  const RadxGeoref *georef = ray->getGeoreference();
  if (georef != NULL) {
    xml += RadxXml::writeDouble("Altitude", 1, georef->getAltitudeKmMsl());
    xml += RadxXml::writeDouble("VertVel", 1, georef->getVertVelocity());
    xml += RadxXml::writeDouble("Roll", 1, georef->getRoll());
    xml += RadxXml::writeDouble("Pitch", 1, georef->getPitch());
    xml += RadxXml::writeDouble("Rotation", 1, georef->getRotation());
    xml += RadxXml::writeDouble("Tilt", 1, georef->getTilt());
    xml += RadxXml::writeDouble("Elevation", 1, ray->getElevationDeg());
    xml += RadxXml::writeDouble("DriveAngle1", 1, georef->getDriveAngle1());
    xml += RadxXml::writeDouble("DriveAngle2", 1, georef->getDriveAngle2());
  }
  
  xml += RadxXml::writeEndTag("HcrVelCorr", 0);
  
  // write to SPDB

  DsSpdb spdb;
  time_t validTime = ray->getTimeSecs();
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
  double velCorr = _firFilt.getVelMeasured() - _firFilt.getVelFilt();
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

//////////////////////////////////////////////////
// identify progressive depolarization in ray

int HcrVelCorrect::_identProgressiveDepol(RadxRay *ray)
  
{

  // get LDR field

  RadxField *ldrField = ray->getField(_params.ldr_field_name);
  if (ldrField == NULL) {
    // no ldr field, nothing to do
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - identProgressiveDepol()" << endl;
      cerr << "  No LDR field found: " << _params.ldr_field_name << endl;
    }
    return -1;
  }
  ldrField->convertToFl32();
  Radx::fl32 ldrMiss = ldrField->getMissingFl32();
  Radx::fl32 *ldr = ldrField->getDataFl32();
  size_t nGates = ldrField->getNPoints();
  double startRange = ldrField->getStartRangeKm();
  double gateSpacing = ldrField->getGateSpacingKm();

  // get DBZ field

  RadxField *dbzField = ray->getField(_params.dbz_field_name);
  if (dbzField == NULL) {
    // no dbz field, nothing to do
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - identProgressiveDepol()" << endl;
      cerr << "  No DBZ field found: " << _params.dbz_field_name << endl;
    }
    return -1;
  }
  dbzField->convertToFl32();
  Radx::fl32 dbzMiss = dbzField->getMissingFl32();
  Radx::fl32 *dbz = dbzField->getDataFl32();

  // create output fields, add them to the ray

  RadxField *ldrFiltField = new RadxField(*ldrField);
  ldrFiltField->setName(_params.ldr_filt_field_name);
  ldrFiltField->setLongName("LDR_filtered_using_polynomial");
  ldrFiltField->setGatesToMissing(0, nGates - 1);
  Radx::fl32 *ldrFilt = ldrFiltField->getDataFl32();
  ray->addField(ldrFiltField);

  RadxField *ldrGradField = new RadxField(*ldrField);
  ldrGradField->setName(_params.ldr_gradient_field_name);
  ldrGradField->setLongName("LDR_gradient_with_range");
  ldrGradField->setGatesToMissing(0, nGates - 1);
  Radx::fl32 *ldrGrad = ldrGradField->getDataFl32();
  ray->addField(ldrGradField);

  RadxField *dbzCorrField = new RadxField(*dbzField);
  dbzCorrField->setName(_params.dbz_corrected_field_name);
  dbzCorrField->setLongName("DBZ_corrected_for_LDR");
  Radx::fl32 *dbzCorr = dbzCorrField->getDataFl32();
  ray->addField(dbzCorrField);

  // perform the polynomial fit for filtering
  
  PolyFit poly;
  poly.setOrder(_params.ldr_filter_polynomial_order);

  for (size_t ii = 0; ii < nGates; ii++) {
    if (ldr[ii] != ldrMiss) {
      double range = startRange + ii * gateSpacing;
      poly.addValue(range, ldr[ii]);
    }
  }
  
  if (poly.performFit()) {
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "WARNING - cannot fit polynomial to LDR field" << endl;
      cerr << "Ignoring ray" << endl;
      return -1;
    }
  }
  
  // success
  // set the filtered field and compute the gradient

  for (size_t ii = 0; ii < nGates; ii++) {
    double range = startRange + ii * gateSpacing;
    ldrFilt[ii] = poly.getYEst(range);
    if (ii > 0) {
      double deltaLdr = ldrFilt[ii] - ldrFilt[ii - 1];
      ldrGrad[ii] = deltaLdr / gateSpacing;
    }
    if (dbz[ii] != dbzMiss) {
      double powerLost = pow(10.0, ldrFilt[ii] / 10.0);
      double dbzLinear = pow(10.0, dbz[ii] / 10.0);
      double dbzCorrLinear = dbzLinear + powerLost;
      dbzCorr[ii] = log10(dbzCorrLinear * 10.0);
    }
  }

  return 0;

}
  
