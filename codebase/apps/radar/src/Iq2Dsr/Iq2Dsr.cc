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
// Iq2Dsr.cc
//
// Iq2Dsr object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// Iq2Dsr reads IQ time-series data, computes the
// moments and writes the contents into a DsRadar FMQ.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <cassert>
#include <iostream>
#include <toolsa/pmu.h>
#include "EgmCorrection.hh"
#include "SpectraPrint.hh"
#include "Iq2Dsr.hh"
using namespace std;

////////////////////////////////////////////////////
// Constructor

Iq2Dsr::Iq2Dsr(int argc, char **argv)

{

  _fmq = NULL;
  _calib = NULL;

  _beamScanMode = -1;

  _beamVolNum = -1;
  _prevVolNum = -1;

  _beamSweepNum = -1;
  _prevSweepNum = -1;

  _currentScanMode = -1;
  _currentVolNum = -1;
  _currentSweepNum = -1;

  _endOfVolFlag = false;
  _endOfSweepFlag = false;

  _startOfSweepPending = false;
  _startOfVolPending = false;
  _endOfVolPending = false;

  _antennaTransition = false;

  _prevAntennaTransition = false;
  _inTransition = false;
  _prevAngle = 0.0;
  _motionDirn = 0.0;
  _nRaysInSweep = 0;

  _volMinEl = 180.0;
  _volMaxEl = -180.0;
  _nBeamsThisVol = 0;

  _prevPrtForEndOfVol = -1; 
  _prevPulseWidthForEndOfVol = -1;

  _prevPrtForParams = -1;
  _prevPrtForMoments = -1;
  _prevNGatesForParams = -1;
  _prevScanModeForParams = -1;
  
  _prevEl = -180;
  _nBeamsSinceParams = 0;

  _beamReader = NULL;
  _writeThread = NULL;
  
  pthread_mutex_init(&_debugPrintMutex, NULL);
  
  constructorOK = true;

  // set programe name

  _progName = "Iq2Dsr";
  ucopyright((char *) _progName.c_str());

  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    constructorOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    constructorOK = false;
    return;
  }

  if (_params.discard_beams_with_missing_pulses) {
    _params.check_for_missing_pulses = pTRUE;
  }
  
  // initalize calibration object, read in starting calibration

  _calib = new Calibration(_params);
  _calib->readCal(_params.startup_cal_file);
  if (!_calib->isCalAvailable()) {
    cerr << "ERROR - Iq2Dsr" << endl;
    cerr << "  Cannot read startup calibration, file: "
         << _params.startup_cal_file << endl;
    constructorOK = false;
    return;
  }
    
  // set up vector of moments manager objects

  for (int ii = 0; ii < _params.moments_params_n; ii++) {
    MomentsMgr *mgr = new MomentsMgr(_progName, _params);
    mgr->init(_params._moments_params[ii]);
    _momentsMgrArray.push_back(mgr);
  }
  if (_momentsMgrArray.size() < 1) {
    cerr << "ERROR: Iq2Dsr::Iq2Dsr." << endl;
    cerr << "  No algorithm geometry specified."; 
    cerr << "  The param moments_menuetry must have at least 1 entry."
         << endl;
    constructorOK = false;
    return;
  }

  // create the beam reader

  pthread_mutex_init(&_beamRecyclePoolMutex, NULL);
  _beamReader = new BeamReader(_progName, _params, _args,
                               _beamRecyclePool, _beamRecyclePoolMutex,
                               _momentsMgrArray);
  if (!_beamReader->constructorOK) {
    constructorOK = false;
  }

  // create the output queue
  
  _fmq = new OutputFmq(_progName, _params);
  if (!_fmq->constructorOK) {
    constructorOK = false;
    return;
  }

  // check for multi-threaded operation

  if (_params.use_multiple_threads) {

    // set up compute thread pool

    _threadPoolSize = _params.n_compute_threads;
    for (int ii = 0; ii < _threadPoolSize; ii++) {
      ComputeThread *thread = new ComputeThread;
      thread->setApp(this);
      pthread_t pth = 0;
      pthread_create(&pth, NULL, _computeMomentsInThread, thread);
      thread->setThreadId(pth);
      _threadPool.push_back(thread);
    }
    _threadPoolInsertPos = 0;
    _threadPoolRetrievePos = 0;

    // create write thread
    
    _writeThread = new WriteThread;
    _writeThread->setApp(this);
    pthread_t writeTh;
    pthread_create(&writeTh, NULL, _handleWrites, _writeThread);
    _writeThread->setThreadId(writeTh);

    // signal write thread to start up

    _writeThread->signalWorkToStart();

  }

  // create SpectraFile object

  SpectraPrint::Inst(_params);

  // initialize EGM correction for georef height, if needed

  if (_params.correct_altitude_for_egm) {
    EgmCorrection &egm = EgmCorrection::inst();
    egm.setParams(_params);
    if (egm.loadEgmData()) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Cannot read in EGM data" << endl;
      cerr << "  File: " << _params.egm_2008_geoid_file << endl;
      constructorOK = false;
      return;
    }
  }
  
  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // start time

  struct timeval tval;
  gettimeofday(&tval, NULL);
  _startTimeSecs = tval.tv_sec + tval.tv_usec / 1.0e6;
  _nGatesComputed = 0;

}

//////////////////////////////////////////////////////////////////
// destructor

Iq2Dsr::~Iq2Dsr()

{

  if (_params.debug) {
    cerr << "Entering Iq2Dsr destructor" << endl;
  }

  // unregister process

  PMU_auto_unregister();

  // free memory
  
  // if (_beamReader) {
  //   delete _beamReader;
  // }
  
  for (size_t ii = 0; ii < _momentsMgrArray.size(); ii++) {
    delete _momentsMgrArray[ii];
  }
  _momentsMgrArray.clear();
  
  if (_fmq) {
    delete _fmq;
  }

  if (_calib) {
    delete _calib;
  }
  
  for (size_t ii = 0; ii < _beamRecyclePool.size(); ii++) {
    delete _beamRecyclePool[ii];
  }

  pthread_mutex_destroy(&_beamRecyclePoolMutex);
  pthread_mutex_destroy(&_debugPrintMutex);

  if (_params.debug) {
    cerr << "Exiting Iq2Dsr destructor" << endl;
  }

}

void Iq2Dsr::_cleanUp()

{

  if (_params.debug) {
    cerr << "Entering _cleanUp" << endl;
  }

  _writeRemainingBeamsOnExit();

  // set threads to exit

  if (_writeThread) {
    // write thread will process all beams waiting in the
    // compute queue, and then exit
    _writeThread->setExitFlag(true);
  }
  
  for (size_t ii = 0; ii < _threadPool.size(); ii++) {
    _threadPool[ii]->setExitFlag(true);
    _threadPool[ii]->signalWorkToStart();
  }

  // end time

  struct timeval tval;
  gettimeofday(&tval, NULL);
  _endTimeSecs = tval.tv_sec + tval.tv_usec / 1.0e6;

  if (_params.debug) {
    double duration = _endTimeSecs - _startTimeSecs;
    double gatesPerSec = _nGatesComputed / duration;
    cerr << "Run stats:" << endl;
    cerr << "  Duration: " << duration << endl;
    cerr << "  N Gates: " << _nGatesComputed << endl;
    cerr << "  Gates per second: " << gatesPerSec << endl;
  }

  if (_params.debug) {
    cerr << "Exiting _cleanUp" << endl;
  }

}

//////////////////////////////////////////////////
// Run

int Iq2Dsr::Run ()
{

  int iret = 0;
  
  if (_params.use_multiple_threads) {
    
    iret = _runMultiThreaded();

  } else {
    
    iret = _runSingleThreaded();

  }

  _cleanUp();
  
  return iret;

}

//////////////////////////////////////////////////
// Run in single threaded mode

int Iq2Dsr::_runSingleThreaded()
{

  int iret = 0;
  PMU_auto_register("Run single threaded");
  Beam *latestBeam = NULL;
  
  while (true) {
    
    PMU_auto_register("Getting next beam");

    // get next incoming beam
    
    Beam *beam = _beamReader->getNextBeam();
    if (beam == NULL) {
      break;
    }

    latestBeam = beam;
    _nGatesComputed += beam->getNGates();
    
    // process the current beam
    
    if (_processBeamSingleThreaded(beam)) {
      iret = -1;
      break;
    }
    
    _nBeamsThisVol++;
    _nBeamsSinceParams++;
    
  } // while
  
  // put final end of sweep and volume flags

  if (latestBeam != NULL) {
    _fmq->putEndOfTilt(_currentSweepNum, *latestBeam);
    _fmq->putEndOfVolume(_currentVolNum, *latestBeam);
  }

  return iret;

}

// #define TESTING
#ifdef TESTING

//////////////////////////////////////////////////
// Run in multi-threaded mode

int Iq2Dsr::_runMultiThreaded()
{

  PMU_auto_register("Run multi-threaded");

  int iret = 0;
  Beam *latestBeam = NULL;
  
  Beam *beams[30];
  for (int ii = 0; ii < 30; ii++) {
    beams[ii] = _beamReader->getNextBeam();
  }

  for (int ii = 0; ii < 10000; ii++) {

    // get next incoming beam
    
    Beam *beam = beams[ii % 30];
    if (beam == NULL) {
      continue;
    }
    
    _nGatesComputed += beam->getNGates();
    latestBeam = beam;
    
    // process the current beam
    
    if (_processBeamMultiThreaded(beam)) {
      iret = -1;
      break;
    }
    
    _nBeamsThisVol++;
    _nBeamsSinceParams++;

  } // while
  
  // put final end of sweep and volume flags

  if (latestBeam != NULL) {
    _fmq->putEndOfTilt(_currentSweepNum, latestBeam);
    _fmq->putEndOfVolume(_currentVolNum, latestBeam);
  }
  
  return iret;

}

#else

//////////////////////////////////////////////////
// Run in multi-threaded mode

int Iq2Dsr::_runMultiThreaded()
{

  PMU_auto_register("Run multi-threaded");

  int iret = 0;
  Beam *latestBeam = NULL;
  
  while (true) {

    PMU_auto_register("Getting next beam");

    // get next incoming beam
    
    Beam *beam = _beamReader->getNextBeam();
    if (beam == NULL) {
      break;
    }
    
    _nGatesComputed += beam->getNGates();
    latestBeam = beam;
    
    // process the current beam
    
    if (_processBeamMultiThreaded(beam)) {
      iret = -1;
      break;
    }
    
    _nBeamsThisVol++;
    _nBeamsSinceParams++;

  } // while
  
  // put final end of sweep and volume flags

  if (latestBeam != NULL) {
    _fmq->putEndOfTilt(_currentSweepNum, *latestBeam);
    _fmq->putEndOfVolume(_currentVolNum, *latestBeam);
  }

  return iret;

}

#endif

///////////////////////////////////////
// process beam in single threaded mode

int Iq2Dsr::_processBeamSingleThreaded(Beam *beam)

{

  // get a new calibration, as appropriate

  _calib->loadCal(beam);

  // set the calibration data on the beam
  // this makes a copy of the DsRadarCalib object, so that
  // it will not change while the compute threads are running
  
  beam->setCalib(_calib->getIwrfCalib());

  // compute the moments for this beam
  
  beam->computeMoments();

  // write the sweep and volume flags

  _handleSweepAndVolChange(beam);
  
  // write the radar params, field params and calibration
  
  _writeParamsAndCalib(beam);

  // write out beam

  beam->setVolNum(_currentVolNum);
  beam->setSweepNum(_currentSweepNum);
  if (_fmq->writeBeam(*beam)) {
    cerr << "ERROR - Iq2Dsr::_processFile" << endl;
    cerr << "  Cannot write the beam data to output FMQ" << endl;
    return -1;
  }

#ifdef TESTING
#else  

  pthread_mutex_lock(&_beamRecyclePoolMutex);
  _beamRecyclePool.push_front(beam);
  pthread_mutex_unlock(&_beamRecyclePoolMutex);

#endif

  return 0;

}
  
///////////////////////////////////////
// process beam in multi-threaded mode

int Iq2Dsr::_processBeamMultiThreaded(Beam *beam)

{

  // get a new calibration, as appropriate
  
  _calib->loadCal(beam);

  // set the calibration data on the beam
  // this makes a copy of the DsRadarCalib object, so that
  // it will not change while the compute threads are running

  beam->setCalib(_calib->getIwrfCalib());

  // get an available thread from the thread pool

  ComputeThread *thread = _threadPool[_threadPoolInsertPos];
  _threadPoolInsertPos = (_threadPoolInsertPos + 1) % _threadPoolSize;
  thread->waitToBeAvailable();

  // set the beam on the thread, and start the computations

  thread->setBeam(beam);
  thread->setBeamReadyForWrite(false);
  thread->signalWorkToStart();

  return 0;

}
  
/////////////////////////////////////////////////
// write the parameters and calibration
    
int Iq2Dsr::_writeParamsAndCalib(const Beam *beam)
  
{
  
  // put the params if needed

  if (_beamReader->isOpsInfoNew() ||
      _endOfSweepFlag ||
      _endOfVolFlag ||
      (_nBeamsSinceParams > _params.nbeams_for_params_and_calib) ||
      (fabs(beam->getPrt() - _prevPrtForParams) > 1.0) ||
      (beam->getNGatesOut() != _prevNGatesForParams) ||
      (beam->getScanMode() != _prevScanModeForParams)) {

    if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
      cerr << "Writing params and calibs, for following reasons:" << endl;
      if (_beamReader->isOpsInfoNew()) cerr << "==>> ops info is new" << endl;
      if(_endOfSweepFlag) cerr << "==>> end of sweep" << endl;
      if (_endOfVolFlag) cerr << "==>> end of volume" << endl;
      if (_nBeamsSinceParams > _params.nbeams_for_params_and_calib) {
	cerr << "==>> _nBeamsSinceParams: " << _nBeamsSinceParams << endl;
      }
      if (fabs(beam->getPrt() - _prevPrtForParams) > 1.0) {
	cerr << "==>> prt has changed to: " << beam->getPrt() << endl;
      }
      if (beam->getNGatesOut() != _prevNGatesForParams) {
	cerr << "==>> ngates out has changed to: " << beam->getNGatesOut() << endl;
      }
      if (beam->getScanMode() != _prevScanModeForParams) {
	cerr << "==>> scan mode has changed to: " << beam->getScanMode() << endl;
      }
    }
    
    if (_fmq->writeParams(*beam)) {
      cerr << "ERROR - Iq2Dsr::_computeMoments" << endl;
      cerr << "  Cannot write the params to the queue" << endl;
      return -1;
    }
    
    if (_fmq->writeCalib(*beam)) {
      cerr << "ERROR - Iq2Dsr::_computeMoments" << endl;
      cerr << "  Cannot write the calib info to the queue" << endl;
      return -1;
    }
    
    if (_fmq->writeStatusXml(*beam)) {
      cerr << "ERROR - Iq2Dsr::_computeMoments" << endl;
      cerr << "  Cannot write the status XML to the queue" << endl;
      return -1;
    }
    
    _prevPrtForParams = beam->getPrt();
    _prevNGatesForParams = beam->getNGatesOut();
    _prevScanModeForParams = beam->getScanMode();
    _nBeamsSinceParams = 0;
    
  }

  return 0;

}

///////////////////////////////////////////////////////////
// Thread function to compute moments

void *Iq2Dsr::_computeMomentsInThread(void *thread_data)
  
{
  
  // get thread data from args

  ComputeThread *compThread = (ComputeThread *) thread_data;
  Iq2Dsr *app = compThread->getApp();
  assert(app);

  while (true) {

    // wait for main to unlock start mutex on this thread
    
    compThread->waitForStartSignal();
    
    // if exit flag is set, app is done, exit now
    
    if (compThread->getExitFlag()) {
      if (app->getParams().debug >= Params::DEBUG_VERBOSE) {
        pthread_mutex_t *debugPrintMutex = app->getDebugPrintMutex();
        pthread_mutex_lock(debugPrintMutex);
        cerr << "====>> compute thread exiting" << endl;
        pthread_mutex_unlock(debugPrintMutex);
      }
      compThread->signalParentWorkIsComplete();
      return NULL;
    }
    
    // compute moments

    Beam *beam = compThread->getBeam();
    if (beam != NULL) {

      if (app->getParams().debug >= Params::DEBUG_VERBOSE) {
        pthread_mutex_t *debugPrintMutex = app->getDebugPrintMutex();
        pthread_mutex_lock(debugPrintMutex);
        cerr << "======>> starting beam compute, el, az: "
             << beam->getEl() << ", " << beam->getAz() << endl;
        pthread_mutex_unlock(debugPrintMutex);
      }

      // compute moments for this beam
      beam->computeMoments();
      compThread->setBeamReadyForWrite(true);
      if (app->getParams().debug >= Params::DEBUG_VERBOSE) {
        pthread_mutex_t *debugPrintMutex = app->getDebugPrintMutex();
        pthread_mutex_lock(debugPrintMutex);
        cerr << "======>> done with beam compute, el, az"
             << beam->getEl() << ", " << beam->getAz() << endl;
        pthread_mutex_unlock(debugPrintMutex);
      }
    }
      
    // unlock done mutex
    
    compThread->signalParentWorkIsComplete();
    
  } // while

  return NULL;

}

///////////////////////////////////////////////////////////
// Thread function to handle writes

void *Iq2Dsr::_handleWrites(void *thread_data)
  
{
  
  // get thread data from args

  WriteThread *writeThread = (WriteThread *) thread_data;
  Iq2Dsr *app = writeThread->getApp();
  assert(app);

  // wait for main to unlock start mutex on this thread
    
  writeThread->waitForStartSignal();

  // call method to check for new data and write it

  int iret = app->writeBeams();
  writeThread->setReturnCode(iret);
  
  if (app->getParams().debug >= Params::DEBUG_VERBOSE) {
    pthread_mutex_t *debugPrintMutex = app->getDebugPrintMutex();
    pthread_mutex_lock(debugPrintMutex);
    cerr << "==>> write thread exited" << endl;
    pthread_mutex_unlock(debugPrintMutex);
  }
    
  // unlock done mutex
  
  writeThread->signalParentWorkIsComplete();
  
  return NULL;

}

///////////////////////////////////////////////////////////
// Thread function to write out beams to FMQ

int Iq2Dsr::writeBeams()
  
{
  
  while (true) {

    if (_writeThread->getExitFlag()) {
      // exit flag is set, app is done, return now
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        pthread_mutex_lock(&_debugPrintMutex);
        cerr << "==>> write thread got exit flag" << endl;
        pthread_mutex_unlock(&_debugPrintMutex);
      }
      return 0;
    }

    // retrieve next thread
    
    ComputeThread *thread = _threadPool[_threadPoolRetrievePos];
    
    // confirm the done status of the thread

    thread->waitForWorkToComplete();

    // is exit flag set on thread
    
    if (thread->getExitFlag()) {
      return 0;
    }
    
    // advance retrieve position

    _threadPoolRetrievePos = (_threadPoolRetrievePos + 1) % _threadPoolSize;
    
    // get the beam from the thread
    
    Beam *beam = thread->getBeam();
    
    if (beam != NULL && thread->getBeamReadyForWrite()) {

      // write the sweep and volume flags
      
      _handleSweepAndVolChange(beam);
      
      // write the radar params, field params and calibration
      
      _writeParamsAndCalib(beam);
      
      // write beam to FMQ
      
      beam->setVolNum(_currentVolNum);
      beam->setSweepNum(_currentSweepNum);
      if (_fmq->writeBeam(*beam)) {
        cerr << "ERROR - Iq2Dsr::_writeBeams" << endl;
        cerr << "  Cannot write the beam data to output FMQ" << endl;
        _writeThread->setReturnCode(-1);
      }
      
      // clear out beam pointer
      // clear ready for write flag
      
      thread->setBeam(NULL);
      thread->setBeamReadyForWrite(false);
      
    }
    
    // mark thread as available

    thread->markAsAvailable();

    // delete the beam
    
#ifdef TESTING
#else  

  pthread_mutex_lock(&_beamRecyclePoolMutex);
  _beamRecyclePool.push_front((Beam *) beam);
  pthread_mutex_unlock(&_beamRecyclePoolMutex);

#endif

  } // while

  return -1;

}

///////////////////////////////////////////////////////////
// Write out remaining beams on exit

int Iq2Dsr::_writeRemainingBeamsOnExit()
  
{

  if (!_params.use_multiple_threads) {
    return 0;
  }
    
  int iret = 0;
  int startRetrievePos = _threadPoolRetrievePos;
  int endRetrievePos = (startRetrievePos - 1) % _threadPoolSize;
  
  int retrievePos = startRetrievePos;
  while (retrievePos != endRetrievePos) {
    
    // get thread pointer
    
    ComputeThread *thread = _threadPool[retrievePos];
    
    // advance retrieve position

    retrievePos = (retrievePos + 1) % _threadPoolSize;
    
    // get the beam from the thread
    
    Beam *beam = thread->getBeam();
    
    if (beam != NULL) {

      // check beam is ready

      if (!thread->getBeamReadyForWrite()) {
        umsleep(500);
      }

      if (thread->getBeamReadyForWrite()) {
        
        // write the sweep and volume flags
        
        _handleSweepAndVolChange(beam);
        
        // write the radar params, field params and calibration
        
        _writeParamsAndCalib(beam);
        
        // write beam to FMQ
        
        beam->setVolNum(_currentVolNum);
        beam->setSweepNum(_currentSweepNum);
        if (_fmq->writeBeam(*beam)) {
          cerr << "ERROR - Iq2Dsr::_writeBeams" << endl;
          cerr << "  Cannot write the beam data to output FMQ" << endl;
          iret = -1;
        }

      } // if (thread->getBeamReadyForWrite())
        
      // clear out beam pointer and ready for write flag
      
      thread->setBeam(NULL);
      thread->setBeamReadyForWrite(false);
      
    } // if (beam != NULL)

  } // while (_threadPoolRetrievePos != endRetrievePos) {

  return iret;

}

////////////////////////////////////////////////////////////////////////
// handle sweep and volume changes, writing flags as appropriate

void Iq2Dsr::_handleSweepAndVolChange(const Beam *beam)
  
{

  // initialize end of sweep and volume flags

  if (_params.use_volume_info_from_time_series) {
    _endOfVolFlag = beam->getEndOfVolFlag();
    if (_endOfVolFlag) {
      _endOfVolPending = true;
    }
  } else {
    _endOfVolFlag = false;
  }

  if (_params.use_sweep_info_from_time_series) {
    _endOfSweepFlag = beam->getEndOfSweepFlag();
  } else {
    _endOfSweepFlag = false;
  }

  // scan mode change
  
  _beamScanMode = beam->getScanMode();
  if (_currentScanMode != _beamScanMode) {
    _fmq->putNewScanType(_beamScanMode, *beam);
    _currentScanMode = _beamScanMode;
    if (_params.set_end_of_sweep_when_antenna_changes_direction) {
      _endOfVolPending = true;
    }
    if (_params.debug) {
      cerr << "Scan mode change to: "
           << iwrf_scan_mode_to_str(_currentScanMode) << endl;
    }
  }

  if (!_params.use_volume_info_from_time_series) {
    // have to deduce the end of volume condition
    _deduceEndOfVol(beam);
    return;
  }
  
  // set vol and sweep num from beam

  _prevVolNum = _beamVolNum;
  _beamVolNum = beam->getVolNum();
  if (_prevVolNum != _beamVolNum) {
    _endOfVolFlag = true;
    _endOfVolPending = true;
  }

  _prevSweepNum = _beamSweepNum;
  _beamSweepNum = beam->getSweepNum();
  _antennaTransition = beam->getAntennaTransition();

  // initialize first time through

  if (_currentVolNum < 0) {
    _currentVolNum = _beamVolNum;
  }
  if (_currentScanMode < 0) {
    _currentScanMode = _beamScanMode;
  }
  if (_currentSweepNum < 0) {
    _currentSweepNum = _beamSweepNum;
  }
  
  // if requested, use procedure to find dirn reversal to
  // trigger end of sweep

  if (_params.set_end_of_sweep_when_antenna_changes_direction) {
    if (_beamScanMode == IWRF_SCAN_MODE_RHI ||
        _beamScanMode == IWRF_SCAN_MODE_IDLE ||
        _beamScanMode == IWRF_SCAN_MODE_SECTOR) {
      _changeSweepOnDirectionChange(beam);
      return;
    }
  }

  if (_currentSweepNum != _beamSweepNum) {
    _endOfSweepFlag = true;
  }
  
  // set pending flags
  
  if (_endOfVolFlag) {
    _startOfVolPending = true;
  }

  if (_endOfSweepFlag) {
    _startOfSweepPending = true;
  }

  // end of sweep?

  if (_endOfSweepFlag) {
    _fmq->putEndOfTilt(_currentSweepNum, *beam);
    if (_params.debug) {
      cerr << "End of sweep num: " << _currentSweepNum << endl;
    }
    _currentSweepNum = _beamSweepNum;
  }

  // end of vol?
  
  if (_endOfVolFlag) {
    _putEndOfVol(beam);
  }
  
  // start of vol?
  
  if (_startOfVolPending) {
    if (!_params.delay_tilt_start_msg_during_ant_trans ||
        !_antennaTransition) {
      _fmq->putStartOfVolume(_currentVolNum, *beam);
      if (_params.debug) {
        cerr << "Start of vol num: " << _currentVolNum << endl;
      }
      _startOfVolPending = false;
    }
  }

  // start of sweep?
  
  if (_startOfSweepPending) {
    if (!_params.delay_tilt_start_msg_during_ant_trans ||
        !_antennaTransition) {
      _fmq->putStartOfTilt(_currentSweepNum, *beam);
      if (_params.debug) {
        cerr << "Start of sweep num: " << _currentSweepNum << endl;
      }
      _startOfSweepPending = false;
    }
  }

}

////////////////////////////////////////////////////////////////////////
// Put end of volume

void Iq2Dsr::_putEndOfVol(const Beam *beam)
  
{

  _fmq->putEndOfVolume(_currentVolNum, *beam);
  if (_params.debug) {
    cerr << "End of vol num: " << _currentVolNum << endl;
  }
  _currentVolNum = _beamVolNum;
  _nBeamsThisVol = 0;

}

////////////////////////////////////////////////////////////////////////
// Deduce end of vol condition

void Iq2Dsr::_deduceEndOfVol(const Beam *beam)
  
{
  
  // set tilt number to missing

  _endOfVolFlag = false;
  _currentSweepNum = -1;
  
  // set elev stats
  
  if (beam->getEl() < _prevEl && beam->getEl() < _volMinEl) {
    _volMinEl = beam->getEl();
  }
  if (beam->getEl() > _prevEl && beam->getEl() > _volMaxEl) {
    _volMaxEl = beam->getEl();
  }
  
  // guess at end of vol condition
  
  if (_params.set_end_of_vol_from_elev_angle) {
    if (_nBeamsThisVol >= _params.min_beams_per_vol) {
      if (_params.vol_starts_at_bottom) {
        double deltaEl = _volMaxEl - beam->getEl();
        if (deltaEl > _params.elev_change_for_end_of_vol) {
          _endOfVolFlag = true;
        }
      } else {
        double deltaEl = beam->getEl() - _volMinEl;
        if (deltaEl > _params.elev_change_for_end_of_vol) {
          _endOfVolFlag = true;
        }
      }
    }
  }

  if (_params.set_end_of_vol_on_prf_change) {
    if (fabs(beam->getPrt() - _prevPrtForEndOfVol) > 1.0e-5) {
      _endOfVolFlag = true;
      _prevPrtForEndOfVol = beam->getPrt();
    }
  }
  
  if (_params.set_end_of_vol_on_pulse_width_change) {
    if (fabs(beam->getPulseWidth() - _prevPulseWidthForEndOfVol) > 1.0e-5) {
      _endOfVolFlag = true;
      _prevPulseWidthForEndOfVol = beam->getPulseWidth();
    }
  }
  
  if (_endOfVolFlag) {
    
    _fmq->putEndOfVolume(_currentVolNum, *beam);
    _currentVolNum++;
    _fmq->putStartOfVolume(_currentVolNum, *beam);
    
    _volMinEl = 180.0;
    _volMaxEl = -180.0;
    _nBeamsThisVol = 0;
    
  }
  
  _prevEl = beam->getEl();
  
}

////////////////////////////////////////////////////////////////////////
// delay sweep change until antenna changes direction

void Iq2Dsr::_changeSweepOnDirectionChange(const Beam *beam)
  
{
  
  // no transitions in this mode, we change sweep number instanteously

  _antennaTransition = false;
  _nRaysInSweep++;
  
  // compute angle change

  double angle;
  if (_beamScanMode == IWRF_SCAN_MODE_RHI) {
    angle = beam->getEl();
  } else {
    angle = beam->getAz();
  }
  double deltaAngle = angle - _prevAngle;
  if (deltaAngle > 180) {
    deltaAngle -= 360.0;
  } else if (deltaAngle < -180) {
    deltaAngle += 360.0;
  }
  if (fabs(deltaAngle) < _params.required_delta_angle_for_antenna_direction_change) {
    return;
  }
  _prevAngle = angle;

  // check for dirn change

  bool dirnChange = false;
  if (_motionDirn * deltaAngle < 0) {
    dirnChange = true;
  }

  if (deltaAngle > 0) {
    _motionDirn = 1.0;
  } else {
    _motionDirn = -1.0;
  }
  
  // do nothing if number of rays is too small
  
  if (_nRaysInSweep < _params.min_rays_in_sweep_for_antenna_direction_change) {
    return;
  }
  
  // do nothing if the direction of motion has not changed
  
  if (!dirnChange && !_endOfVolFlag) {
    return;
  }
  
  // set flags on change
  
  _endOfSweepFlag = true;
  _fmq->putEndOfTilt(_currentSweepNum, *beam);

  if (dirnChange && _params.debug) {
    cerr << "Dirn change, end of sweep num: " << _currentSweepNum << endl;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "    nrays, el, az, angle, prevAngle, deltaAngle, _motionDirn, dirnChange: "
           << _nRaysInSweep << ", "
           << beam->getEl() << ", " << beam->getAz() << ", "
           << angle << ", "
           << _prevAngle << ", "
           << deltaAngle << ", "
           << _motionDirn << ", "
           << dirnChange << endl;
    }
  }
  _nRaysInSweep = 0;

  // increment sweep number

  if (_endOfVolPending) {
    _endOfVolFlag = true;
    _endOfVolPending = false;
  }

  if (!_endOfVolFlag) {
    _currentSweepNum++;
  }

  // end of vol?
  
  bool maxSweepReached = false;
  if (_currentSweepNum >
      _params.max_sweeps_in_vol_for_antenna_direction_change) {
    _endOfVolFlag = true;
    maxSweepReached = true;
  }
  
  if (_endOfVolFlag) {
    _fmq->putEndOfVolume(_currentVolNum, *beam);
    if (_params.debug) {
      cerr << "End of vol num: " << _currentVolNum << endl;
    }
    if (maxSweepReached) {
      _currentVolNum++;
    } else {
      _currentVolNum = _beamVolNum;
    }
    _fmq->putStartOfVolume(_currentVolNum, *beam);
    if (_params.debug) {
      cerr << "Start of vol num: " << _currentVolNum << endl;
    }
    _nBeamsThisVol = 0;
    _currentSweepNum = 0;
    _endOfVolFlag = false;
  }
  
  // start of sweep
  
  _fmq->putStartOfTilt(_currentSweepNum, *beam);
  if (_params.debug) {
    cerr << "Start of sweep num: " << _currentSweepNum << endl;
  }

}

