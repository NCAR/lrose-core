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
/////////////////////////////////////////////////////////////
// Threads.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2007
//
///////////////////////////////////////////////////////////////
//
// Classes for dealing with threads
//
///////////////////////////////////////////////////////////////

#include "Iq2Dsr.hh"
#include "Threads.hh"
#include "MomentsMgr.hh"
#include <cassert>

pthread_mutex_t ComputeThread::_fftMutex = PTHREAD_MUTEX_INITIALIZER;

/////////////////////////////////
// Generic thread

Thread::Thread()

{

  pthread_mutex_init(&_startMutex, NULL);
  pthread_mutex_init(&_completeMutex, NULL);
  pthread_mutex_init(&_availMutex, NULL);
  pthread_mutex_init(&_exitMutex, NULL);

  pthread_cond_init(&_startCond, NULL);
  pthread_cond_init(&_completeCond, NULL);
  pthread_cond_init(&_availCond, NULL);

  _startFlag = false;
  _completeFlag = false;
  _availFlag = true;
  _exitFlag = false;

  _app = NULL;
  _returnCode = 0;

}

Thread::~Thread()

{

  pthread_mutex_destroy(&_startMutex);
  pthread_mutex_destroy(&_completeMutex);
  pthread_mutex_destroy(&_availMutex);

  pthread_cond_destroy(&_startCond);
  pthread_cond_destroy(&_completeCond);
  pthread_cond_destroy(&_availCond);

}

/////////////////////////////////////////////////////////////
// Mutex handling for communication between caller and thread

// Parent signals thread to start work

void Thread::signalWorkToStart() 
{
  pthread_mutex_lock(&_startMutex);
  _startFlag = true;
  pthread_cond_signal(&_startCond);
  pthread_mutex_unlock(&_startMutex);
}

// Thread waits for parent to signal start

void Thread::waitForStartSignal() 
{
  pthread_mutex_lock(&_startMutex);
  while (!_startFlag) {
    pthread_cond_wait(&_startCond, &_startMutex);
  }
  _startFlag = false;
  pthread_mutex_unlock(&_startMutex);
}

// Thread signals parent it is complete
 
void Thread::signalParentWorkIsComplete() 
{
  pthread_mutex_lock(&_completeMutex);
  _completeFlag = true;
  pthread_cond_signal(&_completeCond);
  pthread_mutex_unlock(&_completeMutex);
}

// Parent waits for thread to be complete

void Thread::waitForWorkToComplete() 
{
  pthread_mutex_lock(&_completeMutex);
  while (!_completeFlag) {
    pthread_cond_wait(&_completeCond, &_completeMutex);
  }
  _completeFlag = false;
  pthread_mutex_unlock(&_completeMutex);
}

// Mark thread as available
 
void Thread::markAsAvailable() 
{
  pthread_mutex_lock(&_availMutex);
  _availFlag = true;
  pthread_cond_signal(&_availCond);
  pthread_mutex_unlock(&_availMutex);
}

// Wait for thread to be available

void Thread::waitToBeAvailable() 
{
  pthread_mutex_lock(&_availMutex);
  while (!_availFlag) {
    pthread_cond_wait(&_availCond, &_availMutex);
  }
  _availFlag = false;
  pthread_mutex_unlock(&_availMutex);
}

// get flag indicating thread is available

bool Thread::getAvailFlag()
{
  pthread_mutex_lock(&_availMutex);
  bool flag = _availFlag;
  pthread_mutex_unlock(&_availMutex);
  return flag;
}

// set flag to tell thread to exit

void Thread::setExitFlag(bool val)
{
  pthread_mutex_lock(&_exitMutex);
  _exitFlag = val;
  pthread_mutex_unlock(&_exitMutex);
}

// get flag indicating thread should exit

bool Thread::getExitFlag()
{
  pthread_mutex_lock(&_exitMutex);
  bool flag = _exitFlag;
  pthread_mutex_unlock(&_exitMutex);
  return flag;
}

/////////////////////////////////
// Threads for computing moments

ComputeThread::ComputeThread() 

{

  startGate = 0;
  endGate = -1;

  nSamplesFft = 0;
  fft = NULL;
  fftHalf = NULL;

  nSamplesFftStag = 0;
  fftStagM = 0;
  fftStagN = 0;
  fftStag = NULL;

  nSamplesRegr = 0;
  nSamplesRegrStag = 0;
  regrStagM = 0;
  regrStagN = 0;

  _beam = NULL;
  _beamReadyForWrite = false;

}

ComputeThread::~ComputeThread()
{

  if (fft) {
    delete fft;
  }

  if (fftHalf) {
    delete fftHalf;
  }

}

// create and destroy FFT objects in a thread safe manner

RadarFft *ComputeThread::createFft(size_t nSamples)

{
  pthread_mutex_lock(&_fftMutex);
  RadarFft *fft = new RadarFft(nSamples);
  pthread_mutex_unlock(&_fftMutex);
  return fft;
}

void ComputeThread::destroyFft(RadarFft *fft)

{
  pthread_mutex_lock(&_fftMutex);
  delete fft;
  pthread_mutex_unlock(&_fftMutex);
}

// initialize the FFTs

void ComputeThread::initFfts(size_t nSamples)
{
  
  if (nSamplesFft == nSamples) {
    return;
  }

  // delete old objects
  
  if (fft) {
    delete fft;
  }
  if (fftHalf) {
    delete fftHalf;
  }
  
  // create new objects for correct size
  
  fft = new RadarFft(nSamples);
  fftHalf = new RadarFft(nSamples / 2);
  nSamplesFft = nSamples;

}
  
// initialize the FFTs - staggered mode

void ComputeThread::initFftsStag(size_t nSamples, int stagM, int stagN)
{
  
  if (nSamplesFftStag == nSamples &&
      fftStagM == stagM &&
      fftStagN == stagN) {
    return;
  }

  initFfts(nSamples);

  // delete old object
  
  if (fftStag) {
    delete fftStag;
  }
  
  // create new object for correct size

  int nStaggered =
    RadarMoments::computeNExpandedStagPrt(nSamples, stagM, stagN);
  fftStag = new RadarFft(nStaggered);
  
  nSamplesFftStag = nSamples;
  fftStagM = stagM;
  fftStagN = stagN;
  
}

////////////////////////////////////
// initialize the regression filter

void ComputeThread::initRegr(size_t nSamples)
{

  if (nSamplesRegr == nSamples) {
    return;
  }

  // set up objects for correct size

  assert(_app);
  const Params &params = _app->getParams();
  double wavelengthM = _app->getWavelengthM();

  regr.setup(nSamples,
             params.regression_filter_determine_order_from_cnr,
             params.regression_filter_specified_polynomial_order,
             params.regression_filter_clutter_width_factor,
             params.regression_filter_cnr_exponent,
             wavelengthM);
  
  regrHalf.setup(nSamples / 2,
                 params.regression_filter_determine_order_from_cnr,
                 params.regression_filter_specified_polynomial_order,
                 params.regression_filter_clutter_width_factor,
                 params.regression_filter_cnr_exponent,
                 wavelengthM);
  
  nSamplesRegr = nSamples;
  
}

///////////////////////////////////////////////////////////////////
// initialize the regression filter from another regression object

void ComputeThread::initRegr(const ForsytheRegrFilter &master,
                             const ForsytheRegrFilter &masterHalf)
{
  
  if (nSamplesRegr == master.getNSamples()) {
    return;
  }
    
  // set up objects for correct size
  
  regr = master;
  regrHalf = masterHalf;
  nSamplesRegr = master.getNSamples();
  
}

////////////////////////////////////////////////////
// initialize the regression filter - staggered PRT

void ComputeThread::initRegrStag(size_t nSamples, int stagM, int stagN)
{
  
  if (nSamplesRegrStag == nSamples &&
      regrStagM == stagM &&
      regrStagN == stagN) {
    return;
  }
    
  // set up objects for correct size
  
  assert(_app);
  const Params &params = _app->getParams();
  double wavelengthM = _app->getWavelengthM();
  
  regrStag.setupStaggered(nSamples, stagM, stagN,
                          params.regression_filter_determine_order_from_cnr,
                          params.regression_filter_specified_polynomial_order,
                          params.regression_filter_clutter_width_factor,
                          params.regression_filter_cnr_exponent,
                          wavelengthM);
  
  nSamplesRegrStag = nSamples;
  regrStagM = stagM;
  regrStagN = stagN;
  
}

///////////////////////////////////////////////////////////////////
// initialize the regression filter from another regression object

void ComputeThread::initRegrStag(const ForsytheRegrFilter &master)
{
  
  if (nSamplesRegrStag == master.getNSamples() &&
      regrStagM == master.getStaggeredM() &&
      regrStagN == master.getStaggeredN()) {
    return;
  }
    
  // set up objects for correct size
  
  regrStag = master;
  nSamplesRegrStag = master.getNSamples();
  regrStagM = master.getStaggeredM();
  regrStagN = master.getStaggeredN();
  
}

//////////////////////////////////
// Thread for writing output FMQ

WriteThread::WriteThread()
{

  _volNum = 0;
  _tiltNum = 0;

}

WriteThread::~WriteThread()
{

}

