/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
///////////////////////////////////////////////////////////////
// ThreadTest.cc
//
// ThreadTest object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 1999
//
///////////////////////////////////////////////////////////////
//
// ThreadTest is a test shell for C++
//
///////////////////////////////////////////////////////////////

#include "ThreadTest.hh"
#include <sys/time.h>
#include <string>
#include <malloc.h>
#include <iostream>
#include <iomanip>
#include <toolsa/uusleep.h>
#include <toolsa/DateTime.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaThreadPool.hh>
#include <didss/DsInputPath.hh>
#include <didss/DataFileNames.hh>
using namespace std;

// Constructor

ThreadTest::ThreadTest(int argc, char **argv) :
        _args("ThreadTest")
        
{

  OK = TRUE;
  _countTotal = 0;
  _sumTotal = 0.0;
  
  // set programe name

  _progName = strdup("ThreadTest");

  // get command line args
  
  if (_args.parse(argc, (const char **) argv)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    OK = FALSE;
    return;
  }
  
  return;

}

// destructor

ThreadTest::~ThreadTest()

{
  _cleanUpThreads();
}

//////////////////////////////////////////////////
// Run

int ThreadTest::Run()
{

  // start time

  struct timeval tv;
  gettimeofday(&tv, NULL);
  double startTime = tv.tv_sec + tv.tv_usec / 1.0e6;

  // run threads depending on mode

  int iret = 0;
  if (_params.thread_mode == Params::THREADS_RUN_ONCE) {
    if (_runThreadsOnce()) {
      iret = -1;
    }
  } else {
    if (_runThreadPool()) {
      iret = -1;
    }
  }

  // clean up threads
  
  // _cleanUpThreads();

  // end time

  gettimeofday(&tv, NULL);
  double endTime = tv.tv_sec + tv.tv_usec / 1.0e6;

  // printout

  fprintf(stderr, "countTotal   : %15.10e\n", (double) _countTotal);
  fprintf(stderr, "sumTotal     : %15.10e\n", _sumTotal);
  cerr << "elapsed secs : " << endTime - startTime << endl;

  return iret;

}

//////////////////////////////////////////////////
// Run threads once

int ThreadTest::_runThreadsOnce()
{

  if (_params.debug) {
    cerr << "==>> runThreadsOnce <<==" << endl;
  }

  int nThreads = _params.n_threads;
  if (nThreads == 0) {
    return -1;
  }

  // create new threads

  for (int ii = 0; ii < nThreads; ii++) {
    MyThread *myThread = new MyThread(_params);
    _myThreads.push_back(myThread);
  } // ii

  // initialize thread states
  
  int nValsPerThread = (int) (_params.max_val / nThreads + 0.5);
  double startVal = 0;

  if (_params.debug) {
    cerr << "  nValsPerThread: " << nValsPerThread << endl;
  }

  for (int ii = 0; ii < nThreads; ii++) {

    double endVal = startVal + nValsPerThread - 1;
    if (endVal > _params.max_val - 1.0) {
      endVal = _params.max_val - 1.0;
    }

    _myThreads[ii]->setStartVal(startVal);
    _myThreads[ii]->setEndVal(endVal);

    startVal = endVal + 1.0;

  }

  // start threads
  
  for (int ii = 0; ii < nThreads; ii++) {
    _myThreads[ii]->signalRunToStart();
  }
  
  // wait for threads to complete

  for (int ii = 0; ii < nThreads; ii++) {
    _myThreads[ii]->waitForRunToComplete();
    _countTotal += _myThreads[ii]->getCount();
    _sumTotal += _myThreads[ii]->getSum();
  }
  
  return 0;

}

//////////////////////////////////////////////////
// Run using thread pool

int ThreadTest::_runThreadPool()
{
  
  if (_params.debug) {
    cerr << "==>> runThreadPool <<==" << endl;
  }

  int nThreads = _params.n_threads;
  if (nThreads == 0) {
    return -1;
  }

  // create new thread pool

  TaThreadPool pool;
  for (int ii = 0; ii < nThreads; ii++) {
    MyThread *myThread = new MyThread(_params);
    pool.addThreadToMain(myThread);
  } // ii

  // initialize thread states
  
  int nValsPerThread = _params.n_vals_per_thread;
  
  if (_params.debug) {
    cerr << "  nValsPerThread: " << nValsPerThread << endl;
  }

  double startVal = 0.0;
  pool.initForRun();
  
  while (!pool.checkAllDone()) {

    // get a thread from the pool

    bool isDone;
    TaThread *thread = pool.getNextThread(true, isDone);
    if (thread == NULL) {
      break;
    }
    MyThread *myThread = (MyThread *) thread;
    
    if (isDone) {
      
      // if it is a done thread, get results and return
      // thread to the pool
      
      _countTotal += myThread->getCount();
      _sumTotal += myThread->getSum();
      pool.addThreadToAvail(thread);

    } else {

      // set thread data and set running

      if (startVal < _params.max_val) {
        
        double endVal = startVal + nValsPerThread - 1;
        if (endVal > _params.max_val) {
          endVal = _params.max_val;
        }
        
        myThread->setStartVal(startVal);
        myThread->setEndVal(endVal);
        thread->signalRunToStart();
        
        startVal = endVal + 1.0;
        if (startVal > _params.max_val) {
          startVal = _params.max_val;
        }
        
      } else {
        
        pool.setReadyForDoneCheck();

      }

    }
  } // while

  cerr << "===>> countStarted: " << pool.getCountStarted() << endl;
  cerr << "===>> countDone: " << pool.getCountDone() << endl;

  return 0;

}


//////////////////////////////////////////////
// Manage compute threads for this xmitRcvMode

void ThreadTest::_cleanUpThreads()
  
{

  return;

  if (_myThreads.size() > 0) {
    
    // allow threads to exit by setting done flag
    
    for (int ii = 0; ii < (int) _myThreads.size(); ii++) {
      // set done flags so threads will exit
      _myThreads[ii]->setExitFlag(true);
      _myThreads[ii]->signalRunToStart();
    }
    
    // wait for threads to exit, clean up thread memory
    
    for (int ii = 0; ii < (int) _myThreads.size(); ii++) {
      _myThreads[ii]->waitForRunToComplete();
    }
    
    for (int ii = 0; ii < (int) _myThreads.size(); ii++) {
      delete _myThreads[ii];
    }
    
    _myThreads.clear();
    
  } // if (_myThreads.size() > 0)

}

