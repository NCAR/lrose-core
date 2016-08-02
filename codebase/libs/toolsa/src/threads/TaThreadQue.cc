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
 * @file TaThreadQue.cc
 */

#include <toolsa/TaThreadQue.hh>
#include <toolsa/TaThread.hh>
#include <toolsa/TaThreadLog.hh>
#include <toolsa/LogStream.hh>
#include <unistd.h>
#include <cstdio>

//------------------------------------------------------------------
TaThreadQue::TaThreadQue()
{
  _numThreads = 0;
  pthread_mutex_init(&_debugPrintMutex, NULL);
  pthread_mutex_init(&_inputOutputMutex, NULL);
}

//------------------------------------------------------------------
TaThreadQue::~TaThreadQue()
{
  pthread_mutex_destroy(&_debugPrintMutex);
  pthread_mutex_destroy(&_inputOutputMutex);
}

//----------------------------------------------------------------
void TaThreadQue::activate(TaThread *t)
{
  t->signalRunToStart();
}

//------------------------------------------------------------------
void TaThreadQue::init(const int num_threads, const bool debug)
{
  if (debug)
  {
    LOG_STREAM_ENABLE_CUSTOM_TYPE(TaThreadLog::name());
  }
  else
  {
    LOG_STREAM_DISABLE_CUSTOM_TYPE(TaThreadLog::name());
  }
  LOGC(TaThreadLog::name()) << "Setting " << num_threads << " threads";
  _numThreads = num_threads;
  if (num_threads < 2)
  {
    return;
  }
  for (int i=0; i<num_threads; ++i)
  {
    // call clone to get the thread pointer of the correct derived class
    TaThread *thread = clone(i);
    thread->setThreadDebug(debug);
    storeThread(thread);
  }
}

//----------------------------------------------------------------
void TaThreadQue::reinit(const int numThread, const bool debug)
{
  releaseThreads();
  init(numThread, debug);
}

//------------------------------------------------------------------
void TaThreadQue::thread(int index, void *info)
{
  if (_numThreads > 1)
  {
    // get a thread from the que
    TaThread *thread = _grabThread(index);
    if (thread == NULL)
    {
      LOG(ERROR) << "no threads in que";
      return;
    }
    // put the input info pointer into the thread
    thread->setThreadInfo(info);

    // start it up
    activate(thread);
  }
  else
  {
    // clone a thread just to get the correct run method
    TaThread *thread = clone(index);

    // put in info
    thread->setThreadInfo(info);

    // run
    thread->run();

    // delete this object.. Is this a problem since it is cast to
    // the base class?  
    delete thread;
  }
}

//------------------------------------------------------------------
void TaThreadQue::lockForIO(void)
{
  if (_numThreads > 1)
  {
    pthread_mutex_lock(&_inputOutputMutex);
  }
}

//------------------------------------------------------------------
void TaThreadQue::unlockAfterIO(void)
{
  if (_numThreads > 1)
  {
    pthread_mutex_unlock(&_inputOutputMutex);
  }
}

//------------------------------------------------------------------
void TaThreadQue::setContext(TaThread *t)
{
  t->setThreadContext(this);
}
