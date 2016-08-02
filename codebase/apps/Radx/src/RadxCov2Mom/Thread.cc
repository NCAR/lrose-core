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
// Thread.cc
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2012
//
///////////////////////////////////////////////////////////////
//
// Handling compute threads
//
///////////////////////////////////////////////////////////////

#include "RadxCov2Mom.hh"
#include "Thread.hh"
#include "Moments.hh"
#include <cassert>

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

  _app = NULL;

  // moments object is created by parent,
  // and used by this object

  _moments = NULL;
  
}

ComputeThread::~ComputeThread()
{

}

