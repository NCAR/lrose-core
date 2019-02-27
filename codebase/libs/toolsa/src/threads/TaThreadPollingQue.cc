// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// Copyright, University Corporation for Atmospheric Research (UCAR) 2009-2017. 
// The Government's right to use this data and/or software is restricted per 
// the terms of Cooperative Agreement between UCAR and the National  Science 
// Foundation, to government use only which includes the nonexclusive, 
// nontransferable, irrevocable, royalty-free license to exercise or have 
// exercised for or on behalf of the U.S. Government throughout the world. 
// All other rights are reserved. 
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
#include <stdlib.h>
#include <stdexcept>

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
  try
  {
    releaseThreads();
  }
  catch (std::runtime_error &r)
  {
    LOG(ERROR) << "Caught Runtime error" << r.what();
    exit(-1);
  }
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
