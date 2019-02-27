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
 * @file TaThreadDoubleQue.cc
 */

#include <toolsa/TaThreadDoubleQue.hh>
#include <toolsa/TaThread.hh>
#include <toolsa/LogStream.hh>
#include <unistd.h>
#include <stdlib.h>
#include <cassert>
#include <stdexcept>

//------------------------------------------------------------------
TaThreadDoubleQue::TaThreadDoubleQue() : TaThreadQue()
{
}

//------------------------------------------------------------------
TaThreadDoubleQue::~TaThreadDoubleQue()
{
  try
  {
    releaseThreads();
  }
  catch (std::runtime_error &r)
  {
    LOG(FATAL) << "Caught runtime error " << r.what();
    exit(-1);
  }
}

//------------------------------------------------------------------
void TaThreadDoubleQue::releaseThreads(void)
{
  // wait for active thread pool to complete
  for (size_t ii = 0; ii < _active.size(); ii++) {
    _active[ii]->waitForRunToComplete();
  }

  // signal all active and available threads to exit
  for (size_t ii = 0; ii < _available.size(); ii++) {
    _available[ii]->setExitFlag(true);
    _available[ii]->signalRunToStart();
  }
  for (size_t ii = 0; ii < _active.size(); ii++) {
    _active[ii]->setExitFlag(true);
    _active[ii]->signalRunToStart();
  }

  // wait for all active and available threads to exit
  for (size_t ii = 0; ii < _available.size(); ii++) {
    _available[ii]->waitForRunToComplete();
  }
  for (size_t ii = 0; ii < _active.size(); ii++) {
    _active[ii]->waitForRunToComplete();
  }

  // delete all active and available threads
  for (size_t ii = 0; ii < _available.size(); ii++) {
    delete _available[ii];
  }
  for (size_t ii = 0; ii < _active.size(); ii++) {
    delete _active[ii];
  }
  _available.clear();
  _active.clear();

  pthread_mutex_destroy(&_debugPrintMutex);
  pthread_mutex_destroy(&_inputOutputMutex);
}

//------------------------------------------------------------------
void TaThreadDoubleQue::storeThread(TaThread *thread)
{
  _available.push_back(thread);
}

//----------------------------------------------------------------
void TaThreadDoubleQue::activate(TaThread *t)
{
  t->signalRunToStart();
  _active.push_back(t);
}

//------------------------------------------------------------------
void TaThreadDoubleQue::waitForThreads(void)
{
  while (!_active.empty())
  {
    TaThread *thread = _active.front();
    _active.pop_front();
    _available.push_back(thread); 
    thread->waitForRunToComplete();
  }
}

//------------------------------------------------------------------
TaThread *TaThreadDoubleQue::_grabThread(const int i)
{
  TaThread *thread = NULL;
  if (!_available.empty())
  {
    // get thread from available pool
    thread = _available.front();
    _available.pop_front();
  }
  else
  {
    // get thread from active pool
    thread = _active.front();
    _active.pop_front();
    thread->waitForRunToComplete();
  }
  return thread;
}

