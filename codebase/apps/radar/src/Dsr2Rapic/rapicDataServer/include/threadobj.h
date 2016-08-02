/*
 * threadobj.h
 * 
 * ThreadObj class definition
 * Intended as an abstract class, sub-class for O/S specific 
 * implementation
 *
 */

#ifndef __THREAD_OBJ_H
#define __THREAD_OBJ_H

#include <sys/select.h>  //added  SD 3/6/00
#include <sys/types.h>
#include <unistd.h>
#include "bool.h" 
#include "rpEventTimer.h"

#ifdef sgi
#undef THREAD_SPROC
#define THREAD_PTHREAD
#else 
#define THREAD_PTHREAD
#undef THREAD_SPROC
#endif

#ifdef THREAD_PTHREAD
#include <pthread.h>
typedef pthread_t THREADID_T;
typedef pid_t PID_T;
#define DLY_TCK 1000000
#endif

#ifdef THREAD_SPROC
#include <sys/types.h>
#include <sys/prctl.h>
#include <signal.h>
#include <limits.h>
#define DLY_TCK CLK_TCK
typedef pid_t THREADID_T;
typedef pid_t PID_T;
#endif

void sec_delay(float delaycount);   // delaycount in seconds
void delay(int delaycount);	    // delaycount is (DLY_TCK ticks per sec) 
bool appClosing();                  // app should implement this
                                    // thread's will use this to set
                                    // stopFlag

class ThreadObj {
protected:
    bool stopFlag;	// if set will cause runLoop to exit
    bool threadRunning;
    time_t workProcTime; // time workProc last entered
    time_t workProcTimeout; // reasonable limit for time to wait before it looks as though the thread is dead
    char threadName[256];
    bool inRunLoop;
    bool enabled;
    bool quiet;
    float loopDelay;	// no of secs for runLoop to delay after each workProc call
    bool doParentCheck; // if true, check parent and exit if parent dies
    THREADID_T	thread_id, parent_id;
    PID_T thread_pid, parent_pid;
    
    rpEventTimer workProcExecTimer,  // work proc exec timer, time in work proc
      workProcLoopTimer;              // time btwn work proc loop runs 
#ifdef THREAD_PTHREAD
    static void * 
#endif
#ifdef THREAD_SPROC
    static void
#endif
      threadEntry(void *thisThreadObj);
    virtual void threadInit(); // perform any init tasks before runloop starts
    virtual void threadExit(); // allow thread stopped tidy up
      //calls workProc, delays loopDelay until topFlag detected
    virtual void runLoop();
    
public:
    
    ThreadObj(float lp_dly = 1.0);
    virtual ~ThreadObj();
    
    // launch the thread, then enter runLoop
    virtual bool startThread();
    
    // exits runLoop, drops out of thread
    // maxWait in seconds
    virtual bool stopThread(int maxWait = 30);
    
    // setr flag to stop thread and return
    virtual void setStopThreadFlag();
    
    // perform the work done by this thread, may be called directly
    // if not using threadObj in true threaded mode
    virtual void workProc();
    
    // usual implementation will check the parent thread, and set stopFlag if parent is dead
    void setParentCheck(bool state = true) { doParentCheck = state; };
    bool getParentCheck() { return doParentCheck; };
    virtual void checkParent();

    void setLoopDelay(float dly = 1.0) { loopDelay = dly; };
    float getLoopDelay() { return loopDelay; };

    void setQuiet(bool state = true) { quiet = state; };
    bool getQuiet() { return quiet; };

    bool isThreadRunning() { return threadRunning; };

    void setEnabled(bool state = true) { enabled = state; };
    bool getEnabled() { return enabled; };

    virtual int	threadID();
    virtual char *threadLabel();

    virtual bool threadTimedOut();

    virtual int sendKill(int sig); // send signal to this thread
    virtual int sendKill(THREADID_T thr_id, int sig); // send signal to specified thread

};

#endif
