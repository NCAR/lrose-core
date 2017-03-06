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
// TaThreadPool.h
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2017
//
///////////////////////////////////////////////////////////////
//
// Class for thread pool.
//
///////////////////////////////////////////////////////////////

#ifndef TaThreadPool_h
#define TaThreadPool_h

#include <string>
#include <pthread.h>
class TaThread;

////////////////////////////
// Generic thread base class

class TaThreadPool {

public:
  
  TaThreadPool();
  virtual ~TaThreadPool();
  
  // Add a thread to the pool
  
  void addThread(TaThread *thread);
  
  // Get a thread from the pool
  // If block is true, blocks until thread is available.
  // If block is false an no thread is available, returns NULL.
  // This removes a thread from the available deque and
  // adds it to the busy deque.
  
  TaThread *getAvailThread(bool block);
  
  // Return a thread to the available deque
  
  void returnToAvail(TaThread *thread);
  
  // debugging
  
  void setDebug(bool val) { _debug = val; }
  bool getDebug() const { return _debug; }

protected:

private:
  
  bool _debug;

  pthread_mutex_t _poolMutex;
  pthread_mutex_t _availMutex;
  pthread_cond_t _availCond;

  vector<TaThread *> _pool;
  deque<TaThread *> _avail;
  
};

#endif

