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
/**
 * @file TaThreadPollingQue.cc
 */

#include <toolsa/TaThreadPollingQue.hh>
#include <toolsa/TaThread.hh>
#include <toolsa/TaThreadLog.hh>
#include <toolsa/LogStream.hh>
#include <unistd.h>
#include <cstdio>

//------------------------------------------------------------------
TaThreadPollingQue::TaThreadPollingQue()
{
  _complete = false;
  _thread_index = -1;
  _waiting = false;
  pthread_mutex_init(&_pollingMutex, NULL);
  pthread_mutex_init(&_waitingMutex, NULL);
  pthread_cond_init(&_pollingCond, NULL);
  pthread_cond_init(&_waitingCond, NULL);
}

//------------------------------------------------------------------
TaThreadPollingQue::~TaThreadPollingQue()
{
  releaseThreads();
  pthread_mutex_destroy(&_pollingMutex);
  pthread_mutex_destroy(&_waitingMutex);
  pthread_cond_destroy(&_pollingCond);
  pthread_cond_destroy(&_waitingCond);
}

//------------------------------------------------------------------
void TaThreadPollingQue::releaseThreads(void)
{
  // signal all threads to exit
  for (size_t ii = 0; ii < _thread.size(); ii++) {
    _thread[ii]->setExitFlag(true);
    _thread[ii]->signalRunToStart();
  }

  // wait for all threads to exit
  for (size_t ii = 0; ii < _thread.size(); ii++) {
    _thread[ii]->waitForRunToComplete();
  }

  // delete all threads
  for (size_t ii = 0; ii < _thread.size(); ii++) {
    delete _thread[ii];
  }
  _thread.clear();
}

//------------------------------------------------------------------
void TaThreadPollingQue::storeThread(TaThread *thread)
{
  _thread.push_back(thread);
}

//------------------------------------------------------------------
bool TaThreadPollingQue::waitForAnyOneThread(void *info,
					     void copy(void*, void *))
{
  LOGC(TaThreadLog::name()) << "Waiting";
  TaThread *thread = _grabCompletedThread();
  if (thread == NULL)
  {
    // no threads to wait for
    LOGC(TaThreadLog::name()) << "grabCompletedThread returned NULL";
    return false;
  }
  LOGC(TaThreadLog::name()) << "grabCompletedThread returned with info";
  copy(thread->getThreadInfo(), info);
  
  // re-activate this thread
  thread->signalRunToStart();

  return true;
}

//------------------------------------------------------------------
void TaThreadPollingQue::signalFromThreadWhenComplete(int index)
{
  pthread_mutex_lock(&_pollingMutex);
  _complete = true;
  _thread_index = index;
  pthread_cond_signal(&_pollingCond);
  pthread_mutex_unlock(&_pollingMutex);
}

//------------------------------------------------------------------
void TaThreadPollingQue::waitForQueToWait(void)
{
  pthread_mutex_lock(&_waitingMutex);
  while (!_waiting) {
    pthread_cond_wait(&_waitingCond, &_waitingMutex);
  }
  _waiting = false;
  pthread_mutex_unlock(&_waitingMutex);
}

//------------------------------------------------------------------
TaThread *TaThreadPollingQue::_grabThread(const int index)
{
  return _thread[index];
}

//------------------------------------------------------------------
TaThread *TaThreadPollingQue::_grabCompletedThread(void)
{
  TaThread *thread = NULL;
  if (_thread.empty())
  {
    LOG(ERROR) << "No polling threads";
    return thread;
  }

  // figure out which thread is complete and return that
  int index;
  _waitForThreadToComplete(index);
  return _thread[index];
}

//------------------------------------------------------------------
void TaThreadPollingQue::_waitForThreadToComplete(int &index)
{
  _signalThreadQueIsWaiting();

  pthread_mutex_lock(&_pollingMutex);
  while (!_complete) {
    pthread_cond_wait(&_pollingCond, &_pollingMutex);
  }
  _complete = false;
  index = _thread_index;
  pthread_mutex_unlock(&_pollingMutex);
}

//------------------------------------------------------------------
void TaThreadPollingQue::_signalThreadQueIsWaiting(void)
{
  pthread_mutex_lock(&_waitingMutex);
  _waiting = true;
  pthread_cond_signal(&_waitingCond);
  pthread_mutex_unlock(&_waitingMutex);
}
