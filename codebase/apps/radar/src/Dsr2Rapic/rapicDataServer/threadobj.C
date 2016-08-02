/*
 * Implementation of the ThreadObj class
 * 
 */

#include "threadobj.h"
#include <stdio.h>
#include <errno.h>
#include "log.h"
#include <string.h>
#include <sched.h>
#include <signal.h>
#ifdef STDCPPHEADERS
#include <iostream>
using namespace std;
#else
#include <iostream.h>
#endif

#ifdef THREAD_PTHREAD
// usleep count must be < 1000000, break usleep calls down
// to 500,000 usec chunks
void sec_delay(float secdelay)
{
  long delaycount = long(secdelay * DLY_TCK);

  while (delaycount >= 500000)
    {
      usleep(500000);
      delaycount -= 500000;
    }
  usleep(delaycount);
}

void delay(int delaycount)
{
  while (delaycount >= 500000)
    {
      usleep(500000);
      delaycount -= 500000;
    }
  usleep(delaycount);
}

#endif

#ifdef THREAD_SPROC
void sec_delay(float secdelay)
{
  long delaycount = secdelay*DLY_TCK;
  sginap(delaycount);
}

void delay(int delaycount)
{
  sginap(delaycount);
}

#endif

ThreadObj::ThreadObj(float lp_dly)
{
  stopFlag = false;
  threadRunning = false;
  inRunLoop = false;
  loopDelay = lp_dly;		// default is loop every second 
  doParentCheck = true;
  strcpy(threadName, "ThreadObj");
  thread_id = parent_id = 0;
  thread_pid = parent_pid = 0;
  enabled = true;
  quiet = false;
  workProcTime = 0;
  workProcTimeout = 60;    // default is 1 minutes
}

ThreadObj::~ThreadObj()
{
  stopThread();
  if (threadRunning)
    cerr << "ThreadObj::~ThreadObj Error - destructor called when thread still running\n";
}

// launch the thread, then enter runLoop                               
bool ThreadObj::startThread()
{
  bool OK = false;
  if (thread_id > 0) {
    if (!quiet) 
      //	  fprintf(stderr,"ThreadObj::startThread (%s) ERROR - thread_id = %d\n", 
      //	threadName, thread_id);
      cerr << "ThreadObj::startThread (" << threadName << ")  ERROR - thread_id = " << thread_id << "\n";
    return true;
  }
#ifdef THREAD_SPROC
  if ((thread_id = sproc(threadEntry,PR_SALL,this)) < 0) {
    if (!quiet) 
      // fprintf(stderr,"ThreadObj::startThread (%s) ERROR Thread call failed - %s\n",
      //	threadName, strerror(errno));
      cerr << "ThreadObj::startThread (" << threadName << ")  ERROR Thread call failed - " << strerror(errno) << "\n";
    thread_id = parent_id = 0;
    OK = false;
  }
  else {
    if (!quiet) 
      // fprintf(stderr,"ThreadObj::startThread (%s) Succeeded: sproc thread_id=%d\n",
      //	threadName, thread_id);
      cerr << "ThreadObj::startThread (" << threadName << ")  Succeeded: sproc thread_id=" << thread_id << "\n";
    OK = true;
  }
#endif
#ifdef THREAD_PTHREAD
  parent_id = pthread_self();
  if (!quiet) 
    // fprintf(stderr,"ThreadObj::startThread (%s) from parent thread_id# %d\n", 
    //	threadName, parent_id);
    cerr << "ThreadObj::startThread (" << threadName << ") from parent thread_id= " << parent_id << "\n";
  if (pthread_create(&thread_id, NULL, &threadEntry,this) != 0) {
    // fprintf(stderr,"ThreadObj::startThread (%s) ERROR pthread_create call failed - %s\n",
    //	threadName, strerror(errno));
    cerr << "ThreadObj::startThread (" << threadName << ") ERROR pthread_create call failed - " << strerror(errno) << "\n";
    thread_id = parent_id = 0;
    OK = false;
  }
  else {
    int 	schedpolicy;
    sched_param schedparm;
    pthread_getschedparam(thread_id, &schedpolicy, &schedparm);
    if (!quiet) 
      // fprintf(stderr,"ThreadObj::startThread (%s) schedpolicy=%d\n",
      // 	threadName, schedpolicy);
      cerr << "ThreadObj::startThread (" << threadName << ") schedpolicy=" << schedpolicy << "\n";
    if (!quiet) 
      // fprintf(stderr,"ThreadObj::startThread (%s) Succeeded: pthread thread_id=%d\n", 
      //	threadName, thread_id);
      cerr << "ThreadObj::startThread (" << threadName << ") Succeeded: pthread thread_id=" << thread_id << "\n";
    OK = true;
  }
#endif
  return OK;
}

void ThreadObj::setStopThreadFlag()
{
  stopFlag = true;
}

// exits runLoop, drops out of thread                                  
bool ThreadObj::stopThread(int maxWait)
{
  if (inRunLoop)
    {
      stopFlag = true;
      while (inRunLoop && maxWait)
	{
	  sec_delay(1);
	  maxWait--;
	}
    }
  threadExit();
  return (maxWait > 0);
}

//calls workProc, delays loopDelay until topFlag detected
void ThreadObj::runLoop()
{
  threadInit();
  inRunLoop = true;
  thread_pid = getpid();
  parent_pid = getppid();
  while (!stopFlag)
    {
      if (workProcLoopTimer.started())
	workProcLoopTimer.stop();
      workProcExecTimer.start();
      workProcLoopTimer.start();
      workProc();
      workProcExecTimer.stop();      
      if (doParentCheck)
	checkParent();
      if (!stopFlag) 
	sec_delay(loopDelay);
    }
  inRunLoop = false;
  threadExit();   // if not tidied up, do it now
}

// perform the work required by this thread, may be called directly    
// if not using threadObj in true threaded mode
// ALL CHILD CLASSES SHOULD CALL THIS BASE METHOD
void ThreadObj::workProc()
{
  workProcTime = time(0);  // all workProc implementations SHOULD CALL THIS
  if (appClosing())
    stopFlag = true;
  /*
   * Actual work performed by the thread lives in here
   * Called repeatedly by run-loop.
   * 
   * workProc should honor the enabled state
   * how this is done depends on the sub-class specifics
   * e.g. if enabled state changes, there may be related 
   * responses required
   * 
   * e.g. communications may need to be disconnected etc.
   */
}

#ifdef THREAD_PTHREAD
void * 
#endif
#ifdef THREAD_SPROC
void
#endif
ThreadObj::threadEntry(void *thisThreadObj) {
#ifdef THREAD_SPROC
  ((ThreadObj *)thisThreadObj)->parent_id = getppid();
#endif
#ifdef THREAD_PTHREAD
  ((ThreadObj *)thisThreadObj)->thread_id = pthread_self();
#endif
  ((ThreadObj *)thisThreadObj)->threadRunning = true;
  ((ThreadObj *)thisThreadObj)->runLoop();
#ifdef THREAD_PTHREAD
  return NULL;
#endif
}
						 
void ThreadObj::threadInit()
{
  // allows child classes to perform any init tasks
  // before runloop is started
}


void ThreadObj::threadExit()
{
  /*
   * Allow tidy up on exit of thread
   * NOTE: this is not the destructor,  a thread could be stopped
   * but the instance still exist and be restarted later on.
   */
  if (!threadRunning)
    return;
  if (!quiet) 
    cerr << "ThreadObj::threadExit (" << threadName << ") - EXITING thread_id=" << thread_id << "\n";
  threadRunning = false;
  thread_id = parent_id = 0;
}

void ThreadObj::checkParent()
{
  if (sendKill(parent_id, 0) < 0) {
    if (!quiet) 
      cerr << "ThreadObj::checkParent(" << threadName << "#" << thread_id << "), Parent process (#" << 
	parent_id << ") not responding,  terminating\n";
    stopFlag = true;
  }
}

int ThreadObj::threadID()
{
  return thread_id;
}

char *ThreadObj::threadLabel()
{
  return threadName;
}

int ThreadObj::sendKill(int sig) // send signal to this thread
{
  return sendKill(thread_id, sig);
}

int ThreadObj::sendKill(THREADID_T thr_id, int sig) // send signal to specified thread
{
#ifdef THREAD_SPROC
  return kill(thr_id, sig);
#endif
#ifdef THREAD_PTHREAD
  return pthread_kill(thr_id, sig);
#endif
}

bool ThreadObj::threadTimedOut()
{
  bool timedout = false;

  if (!workProcTime)
    return false;
  else
    {
      timedout = (time(0) - workProcTime) > workProcTimeout;
      if (timedout)
	{
	  sec_delay(0.5);
	  timedout = (time(0) - workProcTime) > workProcTimeout; 
	  // double confirm just in case workProcTime was being changed
	}
      if (timedout)
	{
	  cerr << "ThreadObj::threadTimedOut - " << threadName << workProcTimeout << 
	    " workProc TIMEOUT EXCEEDED\n";
	}
      return timedout;
    }
} 
