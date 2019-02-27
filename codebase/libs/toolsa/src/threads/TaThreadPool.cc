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

  pthread_mutex_init(&_mutex, NULL);
  pthread_cond_init(&_emptyCond, NULL);

  initForRun();

}

///////////////
// destructor

TaThreadPool::~TaThreadPool()

{
  
  pthread_mutex_lock(&_mutex);
  for (size_t ii = 0; ii < _mainPool.size(); ii++) {
    delete _mainPool[ii];
  }
  pthread_mutex_unlock(&_mutex);
  
}

/////////////////////////////////////////////////////////////
// Add a thread to the main pool
// this is used to initialize the pool.
  
void TaThreadPool::addThreadToMain(TaThread *thread)

{

  // lock

  pthread_mutex_lock(&_mutex);

  // let the thread know it is part of the pool

  thread->setPool(this);

  // add to the main list

  _mainPool.push_front(thread);

  // add to the avail list
  
  _availPool.push_front(thread);

  // unlock

  pthread_mutex_unlock(&_mutex);

}

/////////////////////////////////////////////////////////////
// Add a thread to the available pool.
// This is called after a done thread is handled.
  
void TaThreadPool::addThreadToAvail(TaThread *thread)

{

  pthread_mutex_lock(&_mutex);
  bool empty = _availPool.empty() && _donePool.empty();
  _availPool.push_front(thread);
  if (empty) {
    // indicate we are adding a thread to an empty pool
    pthread_cond_signal(&_emptyCond);
  }
  pthread_mutex_unlock(&_mutex);

}

/////////////////////////////////////////////////////////////
// Add a thread to the done pool.
// This is called after a thread completes execution.
  
void TaThreadPool::addThreadToDone(TaThread *thread)

{

  pthread_mutex_lock(&_mutex);
  bool empty = _availPool.empty() && _donePool.empty();
  _donePool.push_front(thread);
  _countDone++;
  if (empty) {
    // indicate we are adding a thread to an empty pool
    pthread_cond_signal(&_emptyCond);
  }
  pthread_mutex_unlock(&_mutex);

}

/////////////////////////////////////////////////////////////
// Initialize the pool for a run.
// Set counts to zero.
// The counts are used to determine when all of the
// threads is use are done.

void TaThreadPool::initForRun()
{

  pthread_mutex_lock(&_mutex);

  // set flags and counts

  _readyForDoneCheck = false;
  _countStarted = 0;
  _countDone = 0;

  // reinitialize the condition variable

  pthread_cond_destroy(&_emptyCond);
  pthread_cond_init(&_emptyCond, NULL);

  // ensure the queues are correct

  _availPool = _mainPool;
  _donePool.clear();

  pthread_mutex_unlock(&_mutex);
}

/////////////////////////////////////////////////////////////
// set the condition that we have completed starting new
// threads and are ready to collect remaining done threads

void TaThreadPool::setReadyForDoneCheck()
{
  pthread_mutex_lock(&_mutex);
  _readyForDoneCheck = true;
  pthread_mutex_unlock(&_mutex);
}

/////////////////////////////////////////////////////////////
// add to start count
// this is called by TaThread when starting

void TaThreadPool::incrementStartCount()
{
  pthread_mutex_lock(&_mutex);
  _countStarted++;
  pthread_mutex_unlock(&_mutex);
}

/////////////////////////////////////////////////////////////
// Check if all the threads are done.

bool TaThreadPool::checkAllDone()
{
  bool allDone = false;
  pthread_mutex_lock(&_mutex);
  if (_readyForDoneCheck) {
    if (_countDone == _countStarted &&
        _donePool.size() == 0) {
      allDone = true;
    }
  }
  pthread_mutex_unlock(&_mutex);
  return allDone;
}

/////////////////////////////////////////////////////////////
// Get next thread from the done or avail pool.
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
  TaThread *thread = NULL;

  thread = _getDoneThread();
  if (thread != NULL) {
    isDone = true;
    return thread;
  }

  // check if we have an available thread
  
  thread = _getAvailThread();
  if (thread != NULL) {
    return thread;
  }

  // in non-blocking mode, return now

  if (!block) {
    return NULL;
  }

  // wait for a thread to become available
  
  pthread_mutex_lock(&_mutex);

  while (true) {

    if (_availPool.empty() && _donePool.empty()) {
      pthread_cond_wait(&_emptyCond, &_mutex);
    }

    // return done thread if available

    if (!_donePool.empty()) {
      thread = _donePool.back();
      _donePool.pop_back();
      isDone = true;
      pthread_mutex_unlock(&_mutex);
      return thread;
    }
    
    // return available thread
    
    if (!_availPool.empty()) {
      thread = _availPool.back();
      _availPool.pop_back();
      pthread_mutex_unlock(&_mutex);
      return thread;
    }

  }

  pthread_mutex_unlock(&_mutex);
  return NULL;
}

/////////////////////////////////////////////////////////////
// Get next thread from the done pool.
// Blocks until a done thread is available,
// or all threads are done.
// Should only be called after setReadyForDoneCheck().

TaThread *TaThreadPool::getNextDoneThread()

{

  while (true) {
    
    // all done?
    if (checkAllDone()) {
      return NULL;
    }

    // check for done thread

    TaThread *thread = _getDoneThread();
    if (thread == NULL) {
    } else {
      return thread;
    }

    // sleep a bit
    TaThread::usecSleep(100);

  } // while

  return NULL;

}

/////////////////////////////////////////////////////////////
// Get a avail thread, without blocking
// returns NULL if no avail thread is ready
  
TaThread *TaThreadPool::_getAvailThread()

{
  
  pthread_mutex_lock(&_mutex);
  
  // never block
  
  if (_availPool.empty()) {
    pthread_mutex_unlock(&_mutex);
    return NULL;
  }

  
  TaThread *thread = _availPool.back();
  _availPool.pop_back();
  pthread_mutex_unlock(&_mutex);
  return thread;

}

/////////////////////////////////////////////////////////////
// Get a done thread, without blocking
// returns NULL if no done thread is ready
  
TaThread *TaThreadPool::_getDoneThread()

{
  
  pthread_mutex_lock(&_mutex);
  
  // never block
  
  if (_donePool.empty()) {
    pthread_mutex_unlock(&_mutex);
    return NULL;
  }

  
  TaThread *thread = _donePool.back();
  _donePool.pop_back();
  pthread_mutex_unlock(&_mutex);
  return thread;

}

