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
// TaThreadPool.cc
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2017
//
///////////////////////////////////////////////////////////////
//
// Pool of TaThreads.
//
///////////////////////////////////////////////////////////////

#include <toolsa/TaThreadPool.hh>
#include <toolsa/TaThread.hh>
using namespace std;

/////////////////////////////////
// Constructor

TaThreadPool::TaThreadPool() 
{

  _debug = false;

  pthread_mutex_init(&_mainMutex, NULL);
  pthread_mutex_init(&_idleMutex, NULL);
  pthread_mutex_init(&_doneMutex, NULL);

  pthread_cond_init(&_idleCond, NULL);
  pthread_cond_init(&_doneCond, NULL);

}

///////////////
// destructor

TaThreadPool::~TaThreadPool()

{
  
  pthread_mutex_lock(&_mainMutex);
  for (size_t ii = 0; ii < _mainPool.size(); ii++) {
    delete _mainPool[ii];
  }
  pthread_mutex_unlock(&_mainMutex);
  
}

/////////////////////////////////////////////////////////////
// Add a thread to the pool
  
void TaThreadPool::addThreadToMain(TaThread *thread)

{

  pthread_mutex_lock(&_mainMutex);
  _mainPool.push_front(thread);
  pthread_mutex_unlock(&_mainMutex);

  pthread_mutex_lock(&_idleMutex);
  _idlePool.push_front(thread);
  pthread_mutex_unlock(&_idleMutex);

}

/////////////////////////////////////////////////////////////
// Get an idle thread, if available
  
TaThread *TaThreadPool::_getIdleThread(bool block)

{

  pthread_mutex_lock(&_idleMutex);

  // handle non-blocking behavior

  if (!block) {

    if (_idlePool.empty()) {
      pthread_mutex_unlock(&_idleMutex);
      return NULL;
    } else {
      TaThread *thread = _idlePool.back();
      _idlePool.pop_back();
      pthread_mutex_unlock(&_idleMutex);
      return thread;
    }
    
  }
  
  // handle blocking behavior

  if (!_idlePool.empty()) {
    TaThread *thread = _idlePool.back();
    _idlePool.pop_back();
    pthread_mutex_unlock(&_idleMutex);
    return thread;
  }

  while (_idlePool.empty()) {
    pthread_cond_wait(&_idleCond, &_idleMutex);
    if (!_idlePool.empty()) {
      TaThread *thread = _idlePool.back();
      _idlePool.pop_back();
      pthread_mutex_unlock(&_idleMutex);
      return thread;
    }
  }

  pthread_mutex_unlock(&_idleMutex);
  return NULL;

}

/////////////////////////////////////////////////////////////
// Get an done thread, if available
  
TaThread *TaThreadPool::_getDoneThread(bool block)

{

  pthread_mutex_lock(&_doneMutex);

  // handle non-blocking behavior

  if (!block) {

    if (_donePool.empty()) {
      pthread_mutex_unlock(&_doneMutex);
      return NULL;
    } else {
      TaThread *thread = _donePool.back();
      _donePool.pop_back();
      pthread_mutex_unlock(&_doneMutex);
      return thread;
    }
    
  }
  
  // handle blocking behavior

  if (!_donePool.empty()) {
    TaThread *thread = _donePool.back();
    _donePool.pop_back();
    pthread_mutex_unlock(&_doneMutex);
    return thread;
  }

  while (_donePool.empty()) {
    pthread_cond_wait(&_doneCond, &_doneMutex);
    if (!_donePool.empty()) {
      TaThread *thread = _donePool.back();
      _donePool.pop_back();
      pthread_mutex_unlock(&_doneMutex);
      return thread;
    }
  }

  pthread_mutex_unlock(&_doneMutex);
  return NULL;

}

#ifdef JUNK
  
/////////////////////////////////////////////////////////////
// Mutex handling for communication between caller and thread

// Parent signals thread to start work.
//
// On failure, throws runtime_error exception.

void TaThreadPool::signalRunToStart() 
{

  pthread_mutex_lock(&_startMutex);
  _startFlag = true;
  pthread_cond_signal(&_startCond);
  pthread_mutex_unlock(&_startMutex);
  
  // check that the thread has been created
  // if not create it.
  
  if (_thread == 0) {
    int iret = pthread_create(&_thread, NULL, _run, this);
    if (iret) {
      int errNum = errno;
      string errStr = "ERROR - TaThreadPool::signalRunToStart()\n";
      errStr += _threadName;
      errStr += "\n";
      errStr += "  Cannot create thread\n";
      errStr += "  ";
      errStr += strerror(errNum);
      throw runtime_error(errStr);
    }
    if (_threadDebug) {
      LockForScope lock;
      cerr << _threadName << ": thread start" << endl;
      cerr << "_thread: " << hex << _thread << dec << endl;
    }
  }

}

// Thread waits for parent to signal start

void TaThreadPool::_waitForStart() 
{
  pthread_mutex_lock(&_startMutex);
  while (!_startFlag) {
    pthread_cond_wait(&_startCond, &_startMutex);
  }
  _startFlag = false;
  pthread_mutex_unlock(&_startMutex);
}

// Thread signals parent it is complete
 
void TaThreadPool::_signalComplete() 
{
  pthread_mutex_lock(&_completeMutex);
  _completeFlag = true;
  pthread_cond_signal(&_completeCond);
  pthread_mutex_unlock(&_completeMutex);
}

// Parent waits for thread to be complete

void TaThreadPool::waitForRunToComplete() 
{
  pthread_mutex_lock(&_completeMutex);
  while (!_completeFlag) {
    pthread_cond_wait(&_completeCond, &_completeMutex);
  }
  _completeFlag = false;
  pthread_mutex_unlock(&_completeMutex);
}

// Mark thread as busy
 
void TaThreadPool::_setBusyFlag(bool state) 
{
  pthread_mutex_lock(&_busyMutex);
  _busyFlag = state;
  pthread_mutex_unlock(&_busyMutex);
}

// Wait for thread to be available - i.e. not busy

void TaThreadPool::waitUntilNotBusy() 
{
  pthread_mutex_lock(&_busyMutex);
  while (_busyFlag) {
    pthread_cond_wait(&_busyCond, &_busyMutex);
  }
  _busyFlag = false;
  pthread_mutex_unlock(&_busyMutex);
}

// get flag indicating thread is busy

bool TaThreadPool::getIsBusy()
{
  pthread_mutex_lock(&_busyMutex);
  bool flag = _busyFlag;
  pthread_mutex_unlock(&_busyMutex);
  return flag;
}

// set flag to tell thread to exit

void TaThreadPool::setExitFlag(bool val)
{
  pthread_mutex_lock(&_exitMutex);
  _exitFlag = val;
  pthread_mutex_unlock(&_exitMutex);
}

// get flag indicating thread should exit

bool TaThreadPool::getExitFlag()
{
  pthread_mutex_lock(&_exitMutex);
  bool flag = _exitFlag;
  pthread_mutex_unlock(&_exitMutex);
  pthread_testcancel();
  return flag;
}

// static run method - entry point for thread

void *TaThreadPool::_run(void *threadData)
{

  // get thread data from args

  TaThreadPool *thread = (TaThreadPool *) threadData;
  assert(thread);
  return thread->_run();

}

// _run method on class

void *TaThreadPool::_run()

{

  while (true) {
    
    // wait for main to unlock start mutex on this thread
    
    _waitForStart();
    
    // if exit flag is set, exit now
    
    if (getExitFlag()) {
      _signalComplete();
      pthread_exit(0);
    }

    // set flag to indicate we are busy

     _setBusyFlag(true);

    // call run method
    // this is overloaded by the subclass
    
    run();
    
    // clear busy flag
    
    _setBusyFlag(false);
    
    // if exit flag is set, exit now
    
    if (getExitFlag()) {
      _signalComplete();
      pthread_exit(0);
    }

    // unlock done mutex
    
    _signalComplete();
    
  } // while

  return NULL;

}

//////////////////////////////////
// terminate
// (a) Set exit flag
// (b) Cancels the thread and joins it

void TaThreadPool::terminate()

{

  if (_thread == 0) {
    // no thread started yet
    return;
  }

  if (_threadDebug) {
    LockForScope lock;
    cerr << _threadName << ": thread terminate start" << endl;
    cerr << "_thread: " << hex << _thread << dec << endl;
  }
  
  // set the exit flag in case run method is active
  // setExitFlag(true); // DON'T USE
  // _signalComplete(); // DON'T USE

  // cancel the thread

  cancel();

  if (_threadDebug) {
    LockForScope lock;
    cerr << _threadName << ": thread terminate done" << endl;
    cerr << "_thread: " << hex << _thread << dec << endl;
  }
  
}

//////////////////////////////////
// Cancels the thread and joins it

void TaThreadPool::cancel()

{

  if (_thread == 0) {
    return;
  }

  // cancel the thread
  
  pthread_cancel(_thread);
  
  // join to parent
  
  pthread_join(_thread, NULL);
  
  // set to 0
  
  _thread = 0;
  
}
  
////////////////////////////////////////////////
// sleep in micro-seconds
//

void TaThreadPool::usecSleep(unsigned int usecs)

{
  
  struct timeval sleep_time;
  fd_set read_value;
  
  sleep_time.tv_sec = usecs / 1000000;
  sleep_time.tv_usec = usecs % 1000000;

  memset (&read_value, 0, sizeof(fd_set));

  select(30, &read_value, 0, 0, &sleep_time);

}

////////////////////////////////////////////////
// sleep in milli-seconds
//

void TaThreadPool::msecSleep(unsigned int msecs)

{
  TaThreadPool::usecSleep(msecs * 1000);
}

#endif
