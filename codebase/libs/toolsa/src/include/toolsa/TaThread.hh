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
// TaThread.h
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2013
//
///////////////////////////////////////////////////////////////
//
// Base class for threading operations.
//
// The intention is that you subclass from this class
// to do actual work.
//
// This class allows for optionally reusable threads.
// Subclass this class, and overide the run() method.
//
// Start the thread using signalToStart().
// Wait for it to complete using waitToComplete().
// To perform a traditional pthread join(), delete the object.
//
// You can call signalToStart() and waitToComplete()
// multiple times.
//
// Check if the thread is busy with getIsBusy().
//
// For a thread that runs continuously for a period of time,
// you can indicate that you want the thread to exit by calling
// setExitFlag() followed by waitToComplete(). For this to work
// the run method must check getExitFlag() routinely in its
// processing, and return if the flag is set - of course cleaning
// up appropriately before returning.
// 
///////////////////////////////////////////////////////////////

#ifndef TaThread_h
#define TaThread_h

#include <string>
#include <pthread.h>
class TaThreadPool;

////////////////////////////
// Generic thread base class

class TaThread {
public:
  
  TaThread();
  virtual ~TaThread();

  //////////////////////////////////////////////////////////////
  // Communication between parent and thread
  
  // Parent signals thread to start work
  
  void signalRunToStart();
  
  // Parent waits for work to complete
  
  void waitForRunToComplete();
  
  // Wait for thread to be available - i.e. not busy

  void waitUntilNotBusy();
  
  // get flag indicating thread is busy

  bool getIsBusy();
  
  // Set flag to tell thread to exit.
  
  void setExitFlag(bool state);

  // Get flag indicating thread should exit.
  // The user run() code, if long-lasting, should
  // frequently check this flag for 2 reasons: (a) to decide
  // whether the run() method should quit and return, and (b)
  // to allow the terminate() method to interrupt the run()
  // method.

  bool getExitFlag();
  
  // terminate
  // Cancels the thread and joins it to the parent.
  // More definite than the exit flag.
  // terminate depends on the user run() code checking
  // for the exit flag - see getExitFlag().
  
  void terminate();

  // Cancels the thread and joins it

  void cancel();

  // sleep in micro-seconds

  static void usecSleep(unsigned int usecs);

  // sleep in milli-seconds

  static void msecSleep(unsigned int msecs);

  // run method - this is where the work is actually done
  // the subclass should override this method with the
  // code to be run
  //
  // See getExitFlag() also - if the run method persists
  // for any length of time the getExitFlag() should be 
  // checked at relevant places in the code.
  
  virtual void run() = 0;

  // safe MUTEX
  // Initializes and destroys the mutex in the
  // constructor and destructor.
  
  class SafeMutex {
  public:
    inline SafeMutex() {
      pthread_mutex_init(&_pmutex, NULL);
    }
    inline ~SafeMutex() {
      pthread_mutex_destroy(&_pmutex);
    }
    inline pthread_mutex_t *getPthreadMutex() { 
      return &_pmutex;
    }
    inline void lock() {
      pthread_mutex_lock(&_pmutex);
    }
    inline void unlock() {
      pthread_mutex_unlock(&_pmutex);
    }
  private:
    pthread_mutex_t _pmutex;
    // prevent copy or assignment
    inline SafeMutex(const SafeMutex &rhs);
    inline SafeMutex & operator=(const SafeMutex &rhs);
  };

  // safe MUTEX locker - unlocks when it goes out of scope
  // Just use the constructor.
  // do not copy or assign
  //
  // By default this uses a debug mutex provided by TaThread.
  // Generally a user-supplied mutex is used.

  class LockForScope {
  public:
    inline LockForScope() {
      // use default mutex
      _mutex = &_defaultMutex;
      pthread_mutex_lock(_mutex->getPthreadMutex());
    }
    inline LockForScope(SafeMutex *mutex) {
      _mutex = mutex;
      pthread_mutex_lock(_mutex->getPthreadMutex());
    }
    inline ~LockForScope() {
      pthread_mutex_unlock(_mutex->getPthreadMutex());
    }
  private:
    SafeMutex *_mutex;
    // prevent copy or assignment
    inline LockForScope(const LockForScope &rhs);
    inline LockForScope & operator=(const LockForScope &rhs);
  };

  // debugging
  // set the name of the thread, and set the debug flag
  // if you want to see thread-specific acivity printed out

  void setThreadDebug(bool val) { _threadDebug = val; }
  bool getThreadDebug() const { return _threadDebug; }

  void setThreadName(const std::string &val) { _threadName = val; }
  const std::string &getThreadName() const { return _threadName; }

  // setting and getting an optional information pointer.
  void setThreadInfo(void *info) { _threadInfo = info; }
  void *getThreadInfo(void) { return _threadInfo; }

  // setting and getting an optional method pointer
  typedef void (* ThreadMethod_t)(void *);
  void setThreadMethod(ThreadMethod_t m) { _threadMethod = m; }
  ThreadMethod_t getThreadMethod(void) { return _threadMethod;}

  // setting and getting an optional context pointer
  void setThreadContext(void *context) { _context = context; }
  void *getThreadContext(void) { return _context; }

  // set the pool if this thread is part of a pool

  void setPool(TaThreadPool *pool) { _pool = pool; }

protected:

private:

  ThreadMethod_t _threadMethod;   // optional method pointer
  void *_threadInfo;              // optional info pointer
  void *_context;                 // optional context pointer


  bool _threadDebug;
  std::string _threadName;

  pthread_t _thread;

  pthread_mutex_t _startMutex;
  pthread_mutex_t _completeMutex;
  pthread_mutex_t _busyMutex;
  pthread_mutex_t _exitMutex;
  static SafeMutex _defaultMutex;

  pthread_cond_t _startCond;
  pthread_cond_t _completeCond;
  pthread_cond_t _busyCond;

  bool _startFlag;
  bool _completeFlag;
  bool _busyFlag;
  bool _exitFlag;

  TaThreadPool *_pool;

  // Thread waits for parent to signal start

  void _waitForStart();

  // Thread signals parent it is complete
  
  void _signalComplete();

  // Set busy flag

  void _setBusyFlag(bool state);

  // pthread entry point

  static void *_run(void *threadData);
  void *_run();

};

#endif

