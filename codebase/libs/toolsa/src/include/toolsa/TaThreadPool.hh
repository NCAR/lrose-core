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
// The thread pool is intended for applications which need to
// create a pool of threads, and keep them around for use,
// thus avoiding the overhead of creating and destroying threads.
//
// The steps for use are as follows:
//
// (1) Create the pool object.
// (2) Load it up with TaThread objects, using addThreadToMain().
//     This will add the threads to the main pool, which stays
//     at a fixed size, once the threads have been added.
//     The main pool is used for freeing up the thread objects
//     later in the destructor.
//     At that point the pool is ready to use.
// (3) For any given processing run, use the following sequence:
//     (a) call initForRun() to initialize counts and flags.
//     (b) loop calling getNextThread().
//     (c) If isDone is true, process the thread's
//         return data and then add it back into the pool
//         using addThreadToAvail().
//     (d) If isDone is false, set the thread properties
//         and start the thread running by calling signalRunToStart().
//  (4) When all threads have been activated, call
//      setReadyForDoneCheck() so that you can start the collection
//      of remaining done threads.
//  (5) Keep looping until checkAllDone() returns true.
//
// The following is some example code that shows this pattern of use.
// This code is checked in under libs/tools/src/thread/test.
//
// int ThreadTest::runThreadPool()
// {
//  
//   int nThreads = _params.n_threads;
//
//   // create new thread pool
//
//   TaThreadPool pool;
//   for (int ii = 0; ii < nThreads; ii++) {
//     MyThread *myThread = new MyThread(_params);
//     pool.addThreadToMain(myThread);
//   } // ii
//
//   // initialize thread states
//  
//   int nValsPerThread = _params.n_vals_per_thread;
//  
//   if (_params.debug) {
//     cerr << "  nValsPerThread: " << nValsPerThread << endl;
//   }
//
//   double startVal = 0.0;
//   pool.initForRun();
//  
//   while (!pool.checkAllDone()) {
//
//     // get a thread from the pool
//
//     bool isDone;
//     TaThread *thread = pool.getNextThread(true, isDone);
//     if (thread == NULL) {
//       break;
//     }
//     MyThread *myThread = (MyThread *) thread;
//    
//     if (isDone) {
//      
//       // if it is a done thread, get results and return
//       // thread to the pool
//      
//       _countTotal += myThread->getCount();
//       _sumTotal += myThread->getSum();
//       pool.addThreadToAvail(thread);
//
//     } else {
//
//       // set thread data and set running
//
//       if (startVal < _params.max_val) {
//        
//         double endVal = startVal + nValsPerThread - 1;
//         if (endVal > _params.max_val) {
//           endVal = _params.max_val;
//         }
//        
//         myThread->setStartVal(startVal);
//         myThread->setEndVal(endVal);
//         thread->signalRunToStart();
//        
//         startVal = endVal + 1.0;
//         if (startVal > _params.max_val) {
//           startVal = _params.max_val;
//         }
//        
//       } else {
//        
//         pool.setReadyForDoneCheck();
//
//       }
//
//     }
//   } // while
//
//   cerr << "===>> countStarted: " << pool.getCountStarted() << endl;
//   cerr << "===>> countDone: " << pool.getCountDone() << endl;
//
//   return 0;
//
// }
//
///////////////////////////////////////////////////////////////

#ifndef TaThreadPool_h
#define TaThreadPool_h

#include <string>
#include <deque>
#include <pthread.h>
class TaThread;

using namespace std;

////////////////////////////
// Generic thread base class

class TaThreadPool {

public:
  
  TaThreadPool();
  virtual ~TaThreadPool();
  
  // Add a thread to the main pool
  // this is used to initialize the pool.
  
  void addThreadToMain(TaThread *thread);
  
  // Add a thread to the avail pool
  // This is called after a done thread is handled.

  void addThreadToAvail(TaThread *thread);
  
  // Add a thread to the done pool
  // This is normally called by the thread itself, once it is done.
  
  void addThreadToDone(TaThread *thread);
  
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
  
  TaThread *getNextThread(bool block, bool &isDone);

  // Initialize the pool for a run.
  // Set counts to zero.
  // The counts are used to determine when all of the
  // threads is use are done.

  void initForRun();

  // add to start count
  // this is called by TaThread when starting

  void incrementStartCount();

  // set the condition that we have completed starting new
  // threads and are ready to collect remaining done threads

  void setReadyForDoneCheck();

  // Get next thread from the done pool.
  // Blocks until a done thread is available,
  // or all threads are done.
  // Should only be called after setReadyForDoneCheck().
  
  TaThread *getNextDoneThread();
    
  // check if all threads are done
  // all threads are done if _countDone == _countStarted

  bool checkAllDone();
  int getCountStarted() const { return _countStarted; }
  int getCountDone() const { return _countDone; }

  // debugging
  
  void setDebug(bool val) { _debug = val; }
  bool getDebug() const { return _debug; }
  size_t getMainPoolSize() const { return _mainPool.size(); }
  size_t getAvailPoolSize() const { return _availPool.size(); }
  size_t getDonePoolSize() const { return _donePool.size(); }

protected:

private:
  
  bool _debug;

  pthread_mutex_t _mutex;    // general mutex
  pthread_cond_t _emptyCond; // condition variable for empty pool

  deque<TaThread *> _mainPool;  // all of the threads
  deque<TaThread *> _availPool; // threads available for use
  deque<TaThread *> _donePool;  // threads done but not yet dealt with

  bool _readyForDoneCheck;
  int _countStarted;
  int _countDone;
  
  TaThread *_getAvailThread();
  TaThread *_getDoneThread();

};

#endif

