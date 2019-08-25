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
// AparTs2Moments.cc
//
// AparTs2Moments object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
///////////////////////////////////////////////////////////////
//
// AparTs2Moments reads IQ time-series data, computes the
// moments and writes the contents into a DsRadar FMQ.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <cassert>
#include <iostream>
#include <toolsa/pmu.h>
#include "AparTs2Moments.hh"
using namespace std;

////////////////////////////////////////////////////
// Constructor

AparTs2Moments::AparTs2Moments(int argc, char **argv)

{

  // initialize

  _fmq = NULL;
  _calib = NULL;

  _currentScanMode = -1;
  _currentVolNum = -1;
  _currentSweepNum = -1;

  _endOfSweepFlag = false;
  _endOfVolFlag = false;
  _nBeamsThisVol = 0;

  _prevPrtForParams = -1;
  _prevNGatesForParams = -1;
  _prevScanModeForParams = -1;
  
  _nBeamsSinceParams = 0;

  _beamReader = NULL;
  _writeThread = NULL;
  
  pthread_mutex_init(&_debugPrintMutex, NULL);
  
  constructorOK = true;

  // set programe name

  _progName = "AparTs2Moments";
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

  // initalize calibration object, read in starting calibration

  _calib = new Calibration(_params);
  _calib->readCal(_params.startup_cal_file);
  if (!_calib->isCalAvailable()) {
    cerr << "ERROR - AparTs2Moments" << endl;
    cerr << "  Cannot read startup calibration, file: "
         << _params.startup_cal_file << endl;
    constructorOK = false;
    return;
  }
    
  // create the beam reader
  
  pthread_mutex_init(&_beamRecyclePoolMutex, NULL);
  _beamReader = new BeamReader(_progName, _params, _args,
                               _beamRecyclePool,
                               _beamRecyclePoolMutex);
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

AparTs2Moments::~AparTs2Moments()

{

  if (_params.debug) {
    cerr << "Entering AparTs2Moments destructor" << endl;
  }

  // set write thread to exit

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    pthread_mutex_lock(&_debugPrintMutex);
    cerr << "==>> waiting for write thread to exit" << endl;
    pthread_mutex_unlock(&_debugPrintMutex);
  }

  if (_writeThread) {
    // write thread will process all beams waiting in the
    // compute queue, and then exit
    _writeThread->setExitFlag(true);
  }
  
  // set thread pool to exit

  for (size_t ii = 0; ii < _threadPool.size(); ii++) {
    _threadPool[ii]->setExitFlag(true);
    _threadPool[ii]->signalWorkToStart();
  }

  // wait for write thread to exit

  if (_writeThread) {
    _writeThread->waitForWorkToComplete();
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

  if (_beamReader) {
    delete _beamReader;
  }
  
  if (_fmq) {
    delete _fmq;
  }

  if (_calib) {
    delete _calib;
  }
  
  // unregister process

  PMU_auto_unregister();

  if (_params.debug) {
    cerr << "Exiting AparTs2Moments destructor" << endl;
  }

}

//////////////////////////////////////////////////
// Run

int AparTs2Moments::Run ()
{

  if (_params.use_multiple_threads) {
    return _runMultiThreaded();
  } else {
    return _runSingleThreaded();
  }

}

//////////////////////////////////////////////////
// Run in single threaded mode

int AparTs2Moments::_runSingleThreaded()
{

  int iret = 0;
  PMU_auto_register("Run single threaded");
  time_t latestBeamTime = 0;
  
  while (true) {

    PMU_auto_register("Getting next beam");

    // get next incoming beam
    
    Beam *beam = _beamReader->getNextBeam();
    if (beam == NULL) {
      break;
    }
    
    latestBeamTime = beam->getTimeSecs();
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

  _fmq->putEndOfTilt(_currentSweepNum, latestBeamTime);
  _fmq->putEndOfVolume(_currentVolNum, latestBeamTime);

  return iret;

}

//////////////////////////////////////////////////
// Run in multi-threaded mode

int AparTs2Moments::_runMultiThreaded()
{

  PMU_auto_register("Run multi-threaded");

  int iret = 0;
  time_t latestBeamTime = 0;
  
  while (true) {

    PMU_auto_register("Getting next beam");

    // get next incoming beam
    
    Beam *beam = _beamReader->getNextBeam();
    if (beam == NULL) {
      break;
    }
    
    _nGatesComputed += beam->getNGates();
    latestBeamTime = beam->getTimeSecs();
    
    // process the current beam
    
    if (_processBeamMultiThreaded(beam)) {
      iret = -1;
      break;
    }
    
    _nBeamsThisVol++;
    _nBeamsSinceParams++;

  } // while
  
  // put final end of sweep and volume flags

  _fmq->putEndOfTilt(_currentSweepNum, latestBeamTime);
  _fmq->putEndOfVolume(_currentVolNum, latestBeamTime);

  return iret;

}

///////////////////////////////////////
// process beam in single threaded mode

int AparTs2Moments::_processBeamSingleThreaded(Beam *beam)

{

  // get a new calibration, as appropriate

  _calib->loadCal(beam);

  // set the calibration data on the beam
  // this makes a copy of the DsRadarCalib object, so that
  // it will not change while the compute threads are running
  
  beam->setCalib(_calib->getAparTsCalib());

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
    cerr << "ERROR - AparTs2Moments::_processFile" << endl;
    cerr << "  Cannot write the beam data to output FMQ" << endl;
    return -1;
  }

  pthread_mutex_lock(&_beamRecyclePoolMutex);
  _beamRecyclePool.push_front(beam);
  pthread_mutex_unlock(&_beamRecyclePoolMutex);

  return 0;

}
  
///////////////////////////////////////
// process beam in multi-threaded mode

int AparTs2Moments::_processBeamMultiThreaded(Beam *beam)

{

  // get a new calibration, as appropriate
  
  _calib->loadCal(beam);

  // set the calibration data on the beam
  // this makes a copy of the DsRadarCalib object, so that
  // it will not change while the compute threads are running

  beam->setCalib(_calib->getAparTsCalib());

  // get an available thread from the thread pool

  ComputeThread *thread = _threadPool[_threadPoolInsertPos];
  _threadPoolInsertPos = (_threadPoolInsertPos + 1) % _threadPoolSize;
  thread->waitToBeAvailable();

  // set the beam on the thread, and start the computations

  thread->setBeam(beam);
  thread->signalWorkToStart();

  return 0;

}
  
/////////////////////////////////////////////////
// write the parameters and calibration
    
int AparTs2Moments::_writeParamsAndCalib(const Beam *beam)
  
{
  
  // put the params if needed

  if (_beamReader->isOpsInfoNew() ||
      _endOfSweepFlag ||
      _endOfVolFlag ||
      (_nBeamsSinceParams > _params.nbeams_for_params_and_calib) ||
      (fabs(beam->getPrt() - _prevPrtForParams) > 1.0) ||
      (beam->getNGatesOut() != _prevNGatesForParams) ||
      (beam->getScanMode() != _prevScanModeForParams)) {

    if (_params.debug >= Params::DEBUG_EXTRA) {
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
      cerr << "ERROR - AparTs2Moments::_computeMoments" << endl;
      cerr << "  Cannot write the params to the queue" << endl;
      return -1;
    }
    
    if (_fmq->writeCalib(*beam)) {
      cerr << "ERROR - AparTs2Moments::_computeMoments" << endl;
      cerr << "  Cannot write the calib info to the queue" << endl;
      return -1;
    }
    
    if (_fmq->writeStatusXml(*beam)) {
      cerr << "ERROR - AparTs2Moments::_computeMoments" << endl;
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

void *AparTs2Moments::_computeMomentsInThread(void *thread_data)
  
{
  
  // get thread data from args

  ComputeThread *compThread = (ComputeThread *) thread_data;
  AparTs2Moments *app = compThread->getApp();
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

    if (app->getParams().debug >= Params::DEBUG_VERBOSE) {
      pthread_mutex_t *debugPrintMutex = app->getDebugPrintMutex();
      pthread_mutex_lock(debugPrintMutex);
      cerr << "======>> starting beam compute" << endl;
      pthread_mutex_unlock(debugPrintMutex);
    }

    Beam *beam = compThread->getBeam();
    beam->computeMoments();
    
    if (app->getParams().debug >= Params::DEBUG_VERBOSE) {
      pthread_mutex_t *debugPrintMutex = app->getDebugPrintMutex();
      pthread_mutex_lock(debugPrintMutex);
      cerr << "======>> done with beam compute" << endl;
      pthread_mutex_unlock(debugPrintMutex);
    }

    // unlock done mutex
    
    compThread->signalParentWorkIsComplete();
    
  } // while

  return NULL;

}

///////////////////////////////////////////////////////////
// Thread function to handle writes

void *AparTs2Moments::_handleWrites(void *thread_data)
  
{
  
  // get thread data from args

  WriteThread *writeThread = (WriteThread *) thread_data;
  AparTs2Moments *app = writeThread->getApp();
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

int AparTs2Moments::writeBeams()
  
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
    
    // write the sweep and volume flags
    
    _handleSweepAndVolChange(beam);
    
    // write the radar params, field params and calibration
    
    _writeParamsAndCalib(beam);
    
    // write beam to FMQ

    if (_fmq->writeBeam(*beam)) {
      cerr << "ERROR - AparTs2Moments::_writeBeams" << endl;
      cerr << "  Cannot write the beam data to output FMQ" << endl;
      _writeThread->setReturnCode(-1);
    }

    // mark thread as available
    
    thread->markAsAvailable();

    // delete the beam
    
    pthread_mutex_lock(&_beamRecyclePoolMutex);
    _beamRecyclePool.push_front((Beam *) beam);
    pthread_mutex_unlock(&_beamRecyclePoolMutex);

  } // while

  return -1;

}

////////////////////////////////////////////////////////////////////////
// handle sweep and volume changes, writing flags as appropriate

void AparTs2Moments::_handleSweepAndVolChange(const Beam *beam)
  
{

  _endOfSweepFlag = false;
  if (_currentSweepNum == -1) {
    _currentSweepNum = beam->getSweepNum();
  } else if (beam->getSweepNum() != _currentSweepNum) {
    _fmq->putEndOfTilt(_currentSweepNum, beam->getTimeSecs());
    _endOfSweepFlag = true;
    _currentSweepNum = beam->getSweepNum();
  }

  _endOfVolFlag = false;
  if (_currentVolNum == -1) {
    _currentVolNum = beam->getVolNum();
  } else if (beam->getVolNum() != _currentVolNum) {
    _fmq->putEndOfVolume(_currentVolNum, beam->getTimeSecs());
    _endOfVolFlag = true;
    _nBeamsThisVol = 0;
    _currentVolNum = beam->getVolNum();
  }

}

