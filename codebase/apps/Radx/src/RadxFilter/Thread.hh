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
// Thread.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2013
//
///////////////////////////////////////////////////////////////
//
// Handling compute threads
//
///////////////////////////////////////////////////////////////

#ifndef Threads_hh
#define Threads_hh

#include <pthread.h>
class ComputeEngine;
class RadxFilter;
class RadxRay;

using namespace std;

////////////////////////////
// Generic thread base class

class Thread {
public:
  
  Thread();
  virtual ~Thread();

  // thread details

  inline void setThreadId(pthread_t val) { _thread = val; }
  
  // return code
  
  inline void setReturnCode(int val) { _returnCode = val; }
  inline int getReturnCode() const { return _returnCode; }

  //////////////////////////////////////////////////////////////
  // Mutex handling for communication between caller and thread
  
  // Parent signals thread to start work
  
  void signalWorkToStart();
  
  // Thread waits for parent to signal start

  void waitForStartSignal();

  // Thread signals parent it is complete

  void signalParentWorkIsComplete();

  // Parent waits for thread to be complete

  void waitForWorkToComplete();

  // Mark thread as available

  void markAsAvailable();

  // Wait for thread to be available

  void waitToBeAvailable();
  
  // get flag indicating thread is available

  bool getAvailFlag();

  // set flag to tell thread to exit

  void setExitFlag(bool val);

  // get flag indicating thread should exit

  bool getExitFlag();

protected:

  pthread_t _thread;

  pthread_mutex_t _startMutex;
  pthread_mutex_t _completeMutex;
  pthread_mutex_t _availMutex;
  pthread_mutex_t _exitMutex;

  pthread_cond_t _startCond;
  pthread_cond_t _completeCond;
  pthread_cond_t _availCond;

  bool _startFlag;
  bool _completeFlag;
  bool _availFlag;
  bool _exitFlag;

  // return code
  
  int _returnCode;

private:

};

//////////////////////////////
// Moments computation thread

class ComputeThread : public Thread 

{

public:

  ComputeThread();
  virtual ~ComputeThread();

  // application context

  inline void setApp(RadxFilter *val) { _app = val; }
  inline RadxFilter *getApp() const { return _app; }

  // moments computation context

  inline void setComputeEngine(ComputeEngine *val) { _engine = val; }
  inline ComputeEngine *getComputeEngine() const { return _engine; }
  
  inline void setInputRay(RadxRay *val) { _inputRay = val; }
  inline RadxRay *getInputRay() { return _inputRay; }

  inline void setOutputRay(RadxRay *val) { _outputRay = val; }
  inline RadxRay *getOutputRay() const { return _outputRay; }

private:

  // application context

  RadxFilter *_app;
  ComputeEngine *_engine;

  // ray context

  RadxRay *_inputRay;
  RadxRay *_outputRay;

};

#endif

