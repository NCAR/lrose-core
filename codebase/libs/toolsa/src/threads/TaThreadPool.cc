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
#include <iostream>
using namespace std;

/////////////////////////////////
// Constructor

TaThreadPool::TaThreadPool() 
{

  _debug = false;

  pthread_mutex_init(&_mainMutex, NULL);
  pthread_mutex_init(&_availMutex, NULL);
  pthread_mutex_init(&_doneMutex, NULL);

  pthread_cond_init(&_availCond, NULL);
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
// Add a thread to the main pool
// this is used to initialize the pool.
  
void TaThreadPool::addThreadToMain(TaThread *thread)

{

  // let the thread know it is part of the pool

  thread->setPool(this);

  // add to the main list

  pthread_mutex_lock(&_mainMutex);
  _mainPool.push_front(thread);
  pthread_mutex_unlock(&_mainMutex);

  // add to the avail list

  pthread_mutex_lock(&_availMutex);
  _availPool.push_front(thread);
  pthread_mutex_unlock(&_availMutex);

}

/////////////////////////////////////////////////////////////
// Add a thread to the available pool.
// This is called after a done thread is handled.
  
void TaThreadPool::addThreadToAvail(TaThread *thread)

{

  pthread_mutex_lock(&_availMutex);
  _availPool.push_front(thread);
  if (_availPool.size() == 1) {
    // adding the first element
    pthread_cond_signal(&_availCond);
  }
  pthread_mutex_unlock(&_availMutex);

}

/////////////////////////////////////////////////////////////
// Add a thread to the done pool.
// This is called after a thread completes execution.
  
void TaThreadPool::addThreadToDone(TaThread *thread)

{

  pthread_mutex_lock(&_doneMutex);
  cerr << "XXXXXXXXXXXXXXXXXXXXX" << endl;
  _donePool.push_front(thread);
  pthread_mutex_unlock(&_doneMutex);

}

/////////////////////////////////////////////////////////////
// Get a thread from the done or avail pool.
// Preference is given to done threads.
// If block is true, blocks until a suitable thread is available.
// If block is false an no thread is available, returns NULL.
//
// If isDone is returned true, the thread came from the done pool.
//   Handle any return information from the done thread, and then
//   add it into the avail pool, using addThreadToAvail();
//
// If isDone is returned false, the thread came from the avail pool.
//   In this case, set the thread running.
//   It will add itself to the done pool when done,
//      using addThreadToDone().

TaThread *TaThreadPool::getNextThread(bool block, bool &isDone)

{

  // check if a done thread is available
  
  isDone = false;

  cerr << "ccccccccccccccccccccc" << endl;
  TaThread *doneThread = _getDoneThreadNoBlock();
  if (doneThread != NULL) {
    isDone = true;
    cerr << "dddddddddddddddddddddd" << endl;
    return doneThread;
  }

  // no done thread is ready, get an available thread

  cerr << "eeeeeeeeeeeeeeeeeeeeeeeee" << endl;
  TaThread *availThread = _getAvailThread(block);
  cerr << "fffffffffffffffffffffffff" << endl;
  return availThread;

}

/////////////////////////////////////////////////////////////
// Get an available thread
  
TaThread *TaThreadPool::_getAvailThread(bool block)

{

  // first try non-blocking

  cerr << "ggggggggggggggggggggggggg" << endl;
  TaThread *thread = _getAvailThreadNoBlock();
  if (thread != NULL) {
    cerr << "hhhhhhhhhhhhhhhhhhhhhhhhh" << endl;
    return thread;
  }

  // then try blocking

  cerr << "iiiiiiiiiiiiiiiiiiiiiiiii" << endl;
  thread = _getAvailThreadBlock();
  cerr << "jjjjjjjjjjjjjjjjjjjjjjjjj" << endl;
  return thread;

}

/////////////////////////////////////////////////////////////
// Get a avail thread, blocking until available
  
TaThread *TaThreadPool::_getAvailThreadBlock()

{
  
  pthread_mutex_lock(&_availMutex);

  if (!_availPool.empty()) {
    TaThread *thread = _availPool.back();
    _availPool.pop_back();
    pthread_mutex_unlock(&_availMutex);
    return thread;
  }

  while (_availPool.empty()) {
    pthread_cond_wait(&_availCond, &_availMutex);
    if (!_availPool.empty()) {
      TaThread *thread = _availPool.back();
      _availPool.pop_back();
      pthread_mutex_unlock(&_availMutex);
      return thread;
    }
  }

  pthread_mutex_unlock(&_availMutex);
  return NULL;

}

/////////////////////////////////////////////////////////////
// Get a avail thread, if available, without blocking
  
TaThread *TaThreadPool::_getAvailThreadNoBlock()

{
  
  pthread_mutex_lock(&_availMutex);
  
  // never block
  
  if (_availPool.empty()) {
    pthread_mutex_unlock(&_availMutex);
    return NULL;
  }

  
  TaThread *thread = _availPool.back();
  _availPool.pop_back();
  pthread_mutex_unlock(&_availMutex);
  return thread;

}

/////////////////////////////////////////////////////////////
// Get a done thread, if available, without blocking
  
TaThread *TaThreadPool::_getDoneThreadNoBlock()

{
  
  pthread_mutex_lock(&_doneMutex);
  
  // never block
  
  if (_donePool.empty()) {
    pthread_mutex_unlock(&_doneMutex);
    return NULL;
  }

  
  TaThread *thread = _donePool.back();
  _donePool.pop_back();
  pthread_mutex_unlock(&_doneMutex);
  return thread;

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
