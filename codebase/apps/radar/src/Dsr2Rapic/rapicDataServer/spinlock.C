/*

	spinlock.c

	Implementation of spinlock class

	Current implementation uses IRIX IPC spinlocks for
	ease of implementation

*/

// #include "rdr.h"
#include "spinlock.h"
#include <sys/errno.h>
#ifdef STDCPPHEADERS
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
using namespace std;
#else
#include <iostream.h>
#include <iomanip.h>
#include <stream.h>
#include <string.h>
#endif

int LockCount = 0;     
int RecordLockHolderPID = 0;
bool CheckHoldTime = true;
char spinlock_nolabel[] = "nolabel";
long long spinlock_lock_ops = 0;
long long spinlock_lock_wait_ops = 0;
long long spinlock_lock_fail_count = 0;
long long spinlock_rel_ops = 0;

bool spinlock_global_quit = false;

spinlock::spinlock(char *Label, int maxwaittime) {					
    lockwaitmax = maxwaittime;
    maxwaitingcount = waitingcount = 0;
    lock = 0;
    isLocked = false;
    //    has_timed_out = false;
    timeout_count = 0;
    quitting = false;
    AcquireOSLock();
    setLabel(Label);
#ifdef USE_ABIMUTEX
    ReleaseLockToOS = 0;
#else
    ReleaseLockToOS = 1;
#endif
    }

spinlock::spinlock(char *Label, float waitsecs) {					
    lockwaitmax = int(waitsecs * 100.);
    lock = 0;
    //    has_timed_out = false;
    timeout_count = 0;
    quitting = false;
    AcquireOSLock();
    setLabel(Label);
#ifdef USE_ABIMUTEX
    ReleaseLockToOS = 0;
#else
    ReleaseLockToOS = 1;
#endif
    strcpy(holderString, "free");
    }

void spinlock::setLabel(char *Label)
{
  if (Label)
    {
      strncpy(label, Label, 31);
    }
  else
    strncpy(label, spinlock_nolabel, 32);
}  

spinlock::~spinlock() {
	ReleaseLockToOS = 1;	// ensure lock will be released
	if (lock) 
	    ReleaseOSLock();
}	


bool spinlock::get_lock(const char *holderstring, int waittime, int quiet)
{
  if (waittime < 0) 
    waittime = lockwaitmax;
  bool result = get_lock(waittime); 
  if (result) // only record caller as holder if successful
    {
      if (holderstring)  
	strncpy(holderString, holderstring, 127);
    }
  else        // otherwise print error
    {
      cerr << "spinlock::get_lock Failed\nCaller=" << holderstring << 
	" WaitTime exceeded was " << float(waittime/100.0) <<
	"seconds\nHolder=" << holderString << "\n";
      if (CheckHoldTime)
	cerr << "\nHolder has had lock for " << float(elapsed_time()/100.0) << "seconds\n";
    }
  return result;
}

/*
 * wait time in 1/100ths of secs
*/

bool spinlock::get_lock(int waittime,int quiet) {
//    lockdebug_t debug_t;
    int attempts;
    int firstfail = 1;
    int nesteddetected  = 0;
    int lockresult = 0;
    THREADID_T this_pid = 0;
#ifdef THREAD_SPROC
#ifndef USE_ABIMUTEX
    lockdebug_t ldb;
#endif    
#endif

    // initial check on whether OS lock does exist
    if (!(lock && uselocks)) 
	return true;

    if (RecordLockHolderPID) 
      this_pid = GETPID();
    waitingcount++;
    if (waitingcount > maxwaitingcount) {
      maxwaitingcount = waitingcount;
      if (maxwaitingcount > 1)
	fprintf(stderr, "spinlock::get_lock (%s) Held by %s - maxwaitingcount = %d\n", 
		label, holderString, maxwaitingcount);
    }
    if (waittime < 0) 
	attempts = lockwaitmax;
    else 
	attempts = waittime;

// added uselocks to while loop since it is conceiveably possible that the OS lock could be
// destroyed whilst we are waiting on the lock.....
//
// loop until we acquire the lock, timeout, detect a nesting
// if we don't need locks any more, uselocks will be set false and we will drop out
#ifdef THREAD_SPROC
#ifdef USE_ABIMUTEX
    while (uselocks && attempts && !nesteddetected && 
	(acquire_lock(lock) != 0)) {
#else
    while (uselocks && attempts && !nesteddetected && 
	((lockresult = uscsetlock(lock,1)) == 0)) {
#endif
#elif defined WIN32
DWORD temp;
    while (uselocks && attempts && !nesteddetected && 
	    ((temp = WaitForSingleObject(lock, 0)) != 0)) {
#elif defined THREAD_PTHREAD
int temp;
    while (uselocks && attempts && ((temp = pthread_mutex_trylock(lock)) != 0)
	    && !nesteddetected) {
      // DOCUMENTATION SUGGEST THE EDEADLK ERROR INDICATES
      // that the lock is already held by this thread, i.e. nested (recursive)
      // This tries to handle nested locking via the DEADLK status
      if (temp == EDEADLK) {	// NESTED - this mutex is already held by this thread
	nested = nesteddetected = 1;	// set nested lock flag
	nestcount++;
	// fprintf(stderr,"spinlock::get_lock - Nested pthread lock detected pid=%d Incremented depth=%d\n",
	//    GETPID(), nestcount);
	cerr << "spinlock::get_lock(" << label << ")- Nested pthread lock detected pid=" << GETPID() <<
	  "Incremented depth=" << nestcount << "\n";
	firstfail = 0;			// held by same pid ie nested get_lock call
      }
      else 
	if (temp != EBUSY) attempts = 0;	// should only be busy, else something wrong
#endif

// If RecordLockHolderPID turned ON check for nested calls
// to get_lock
// WIN32 can handle nested locks as long as the number of get lock calls == num of rel lock calls
#ifndef WIN32
      if (firstfail && RecordLockHolderPID) {	// check whether lock is
	firstfail = 0;			// held by same pid ie nested get_lock call
	if (this_pid == holder_pid) {	
	  nested = nesteddetected = 1;	// set nested lock flag
	  nestcount++;
	  // fprintf(stderr,"spinlock::get_lock - Nested. pid=%d Incremented depth=%d\n",
	  //	holder_pid, nestcount);
	  cerr << "spinlock::get_lock(" << label << ") - Nested. pid=" << holder_pid << 
	    " Incremented depth=" << nestcount << "\n";
	}
	if (holder_pid == 0) {
#ifdef THREAD_SPROC
#ifndef USE_ABIMUTEX
	  usctllock(lock, CL_DEBUGFETCH, &ldb);
	  fprintf(stderr,"spinlock::get_lock(%s) - Set lock failed (pid=%d)"
		  " but holder_pid=0!! lockdebug_pid=%d\n", 
		  label, this_pid, ldb.ld_owner_pid);
	  holder_pid = ldb.ld_owner_pid;
#else
	  //fprintf(stderr,"spinlock::get_lock - Set lock failed (pid=%d)"
	  //	" but holder_pid=0!!\n", this_pid);
	  cerr << "spinlock::get_lock(" << label << ") - Set lock failed (pid=" << this_pid 
	       << ") but holder_pid=0!! " << "\n";
#endif
#endif // THREAD_SPROC
	  attempts = 4;	// holder_pid not defined!, limit further attempts to 4
	}
      }
#endif
	// sleep off this time round
      if ((attempts > 0) && !DebugMode) // infinitely patient in
	attempts--;			// if attempts < 0, no timeout	// DEBUGMODE
      if (attempts > lockwaitmax) {	// error condition, 
	fprintf(stderr,"spinlock::get_lock(%s) - ERROR attempts count out of range, resetting\n", label);
	attempts = lockwaitmax;
      }
      spinlock_lock_wait_ops++;
      if (!nesteddetected) {
	if (attempts > 0) sec_delay(0.01);
	if (attempts < 0) sec_delay(0);
      }
    }
    
    if (attempts > 0) 
      {
	spinlock_lock_ops++;
	if(RecordLockHolderPID)
	  holder_pid = this_pid;
	if (CheckHoldTime) 
#ifndef WIN32
#ifdef sgi
	  gettimeofday(&lockedtime);
#else
          gettimeofday(&lockedtime,0);
#endif
#else
	  lockedtime = GetTickCount();
#endif
      }
    else if (attempts == 0)
      {
	spinlock_lock_fail_count++;
	if (!quiet) 
	  {
	    //	usctllock(lock, CL_DEBUGFETCH, &debug_t);
	    //	    has_timed_out = true;
	    timeout_count++;
	    // fprintf(stderr,"spinlock::get_lock - TIMED OUT. pid=%d Lock held by pid=%d\n", 
	    //	this_pid, holder_pid);
	    cerr << "spinlock::get_lock(" << label << ") - TIMED OUT. pid=" << this_pid << " Lock held by pid=" << holder_pid << "-" << holderString << " Processes waiting for this lock=" << waitingcount << "\n";
	  }
      }
    if (lockresult < 0) {
      fprintf(stderr,"spinlock::get_lock(%s) - Set lock failed with error\n", label);
      perror("");
      attempts = 0;
    }
    waitingcount--;

    if (quitting) // quitting uses "uselocks" flag to exit get_lock immediately 
      {
	attempts = 0; // ensure attempts cleared so fail status returned	
      }

    if (attempts != 0)
      {
	strcpy(holderString, "Anonymous holder"); // overwrite previous holderstr
	// if called by get_lock above it will insert actuall holderstr
	// isLocked = true;   // seems to cause deadlock
      }

    return attempts != 0;  
}


/*
 * Release an OS lock back to the OS 
 * - typically used when we know no more data will be written
 */
bool spinlock::ReleaseOSLock(int waittime,int quiet) {
//	int attempts;

  if ((lock == NULL) || !uselocks)
    return true;

  if (!get_lock(waittime)) // it is only safe to change 'uselocks' when we do actually hold the lock
    // a proven race condition exists if another thread holds a lock when
    // uselocks is set false which results in the following loop being stuck permanently
    // since rel_lock will return immediately without releasing the currently held OS lock
    {
      if (!get_lock(lockwaitmax*2))
	{
	  fprintf(stderr,"spinlock::ReleaseOSLock(%s) - Failed to get_lock, unable to delete lock\n",
		  label);
	  return true;
	}
    }
  uselocks = false;   // no other threads can seize the lock from now on
  rel_lock();         /// do administrative lock release, but won't unlock because uselocks is false
  
  if (!ReleaseLockToOS) {
    return true;	// don't release lock to OS, false in low overhead mutex systems
	}

#ifdef THREAD_SPROC				    // lock is free, give it back to OS
#ifdef USE_ABIMUTEX
  release_lock(lock);
  delete(lock);
#else
    usfreelock(lock,arena);
#endif
#elif defined THREAD_PTHREAD
    int result = 0;
    if ((result = pthread_mutex_trylock(lock)) == EBUSY) // uselocks = false above will have stopped rel_lock from working
      result = pthread_mutex_unlock(lock);               // unlock it here
    if ((result = pthread_mutex_destroy(lock)) == 0)
      delete lock;
    else
      {
	//	char errstring[128] = "";
	//	strerror_r(result, errstring, 128);
	fprintf(stderr,"spinlock::ReleaseOSLock(%s) - pthread_mutex_destroy Failed - errno=%d\n",
		label, result);
      }
#elif defined WIN32
    CloseHandle(lock);
#endif
    lock = 0;
    holder_pid = 0;
    LockCount--;
    return true;;  
}
	
void spinlock::AcquireOSLock() {
    if(lock) return;
#ifdef THREAD_SPROC
#ifdef USE_ABIMUTEX
    lock = new(abilock_t);
    init_lock(lock);
#else
    if (!(arena && (lock = usnewlock(arena)))) 
	fprintf(stderr,"spinlock::spinlock(%s) - Failed to assign lock. Locks assigned = %d."
		" pid=%d\n", label, LockCount, GETPID());
    else LockCount++;
#endif // USE_ABIMUTEX
#elif defined THREAD_PTHREAD
    int temp;
    lock = new pthread_mutex_t;
    if ((temp = pthread_mutex_init(lock, NULL)) == 0)
	LockCount++;
    else lock = 0;
#elif defined WIN32
    if ((lock = CreateMutex(NULL, FALSE, NULL)) == NULL)
	    fprintf(stderr, "spinlock::spinlock(%s) - Failed to assign lock. Locks assigned = %d. pid=%d\n", 
		    label, LockCount, GETPID());
    else LockCount++;
#endif

    holder_pid = 0;
#ifndef WIN32
    nestcount = 0;
    nested = 0;
#endif
    waitingcount = 0;
    if(lock) 
    uselocks = true;
}

bool spinlock::HasOSLock() {
    return (lock != 0);
}

void spinlock::rel_lock() {
    //    lockdebug_t debug_t;
    int	elapsedtime = 0;
    THREADID_T this_pid = 0;
    
    if (!(lock && uselocks)) 
      {
	if (isLocked)
	  {
	    strcpy(holderString, "Released");
	    isLocked = false;
	  }
	return;
      }
    /*
    if (!isLocked)  // only try to unlock a locked lock
      return;
    */
    if (RecordLockHolderPID) {
	this_pid = GETPID();
	if(this_pid == holder_pid) {
#ifndef WIN32
	    if (nestcount) {	// if still nested, don't actually unlock, just decrement depth
		nestcount--;
		fprintf(stderr,"spinlock::rel_lock(%s) - Nested. Decremented depth = %d\n", label, nestcount);
		if (!nestcount) isLocked = false;
		return;
		}    
#endif
	    }
	else {
	    if (holder_pid == 0)
//  		fprintf(stderr,"spinlock::rel_lock - rel lock called but holder_pid=0!! "
//  			"pid=%d\n", this_pid);
	      cerr << "spinlock::rel_lock(" << label << ") - rel lock called but holder_pid=0!! pid=" << this_pid << "\n";
	    else 
//  		fprintf(stderr,"spinlock::rel_lock - Wrong process(%d) tried to unlock. "
//  			"Held by %d\n", this_pid, holder_pid);
	      cerr << "spinlock::rel_lock(" << label << ") - Wrong process(" << this_pid << ") tried to unlock. Held by " << 
		holder_pid << "\n";
	    isLocked = false;
	    return;
	    }
	}
#ifdef THREAD_SPROC
#ifdef USE_ABIMUTEX
    if (stat_lock(lock) == LOCKED) 
	release_lock(lock);
#else    
    if (ustestlock(lock)) 
	usunsetlock(lock);
#endif
    else {
    //	usctllock(lock, CL_DEBUGFETCH, &debug_t);
	fprintf(stderr,"spinlock::rel_lock called when not locked. pid=%d Lock held by pid=%d Holder=%s\n",
		label, this_pid, holder_pid, holderString);
    }
#elif defined THREAD_PTHREAD
    int temp;
    if ((temp = pthread_mutex_unlock(lock)) != 0) {
//  	fprintf(stderr,"spinlock::rel_lock Failed. pid=%d Lock held by pid=%d\n", 
//  		this_pid, holder_pid);
      cerr << "spinlock::rel_lock(" << label << ") Failed. pid=" << this_pid << " Lock held by pid=" << 
	holder_pid << " Holder=" << holderString << "\n";
	perror(0);
	}
#elif defined WIN32
    if (!ReleaseMutex(lock)) {
	fprintf(stderr, "spinlock::rel_lock(%s) Failed. pid=%d\n", label, GETPID());
	}
#endif
    if (CheckHoldTime) {
	if ((lockwaitmax > 0) && isLocked &&
	    (elapsedtime = elapsed_time()) > lockwaitmax) {
//  	    fprintf(stderr,"spinlock::rel_lock - Process %d held lock for %1.3f secs Max=%1.3f\n",
//  		    holder_pid, elapsedtime/1000000.0, lockwaitmax/100.0);
	  cerr.setf(ios::fixed, ios::floatfield);
	  cerr << "spinlock::rel_lock(" << label << ") - ***WARNING*** Process " << 
	    holder_pid << " Holder=" << holderString << " held lock for " << 
	    setprecision(3) << 
	    elapsedtime/100.0 << "secs Max=" <<
	    lockwaitmax/100.0 << endl;
	    }
	}
    holder_pid = 0;
#ifndef WIN32
    if (nested && (nestcount == 0)) {
	fprintf(stderr,"spinlock::rel_lock(%s) - Nested release complete\n", label);
	nested = 0;
	}
#endif
    isLocked = false;
    strcpy(holderString, "Released");
    spinlock_rel_ops++;
}

int spinlock::elapsed_time() {	// time since lock set in 100ths of secs
#ifdef WIN32
	DWORD localtimeval;
  if (!CheckHoldTime) 
    return 0;
  localtimeval = GetTickCount();
  return (localtimeval - lockedtime) / 10;
#else
	timeval localtimeval;
  if (!CheckHoldTime) return 0;
#ifdef sgi
  gettimeofday(&localtimeval);
#else
  gettimeofday(&localtimeval,0);
#endif
  return ((localtimeval.tv_sec - lockedtime.tv_sec) * 100) + 
	  ((localtimeval.tv_usec - lockedtime.tv_usec) / 10000);
#endif
}

void spinlock::setQuitting()
{ 
  quitting = true; 
  uselocks = false;
  cerr << "spinlock::setQuitting(" << label << ") - Force any get_lock calls to fail immediately\n";
  
} // quitting - force immediate get_lock timeouts

 spinl_freelist::spinl_freelist() {
    lock = new spinlock("spinl_freelist", 10);
    count = 0;
    freelocks = 0;
}

spinl_freelist::~spinl_freelist() {
    spinlock *next;
    lock->get_lock();
    while (freelocks) {
	next = freelocks->next;
	delete freelocks;
	freelocks = next;
	count--;
	}
    lock->rel_lock();
    delete lock;
    lock = 0;
}

void spinl_freelist::add_lock(spinlock *freelock) {
    lock->get_lock();
    freelock->next = freelocks;
    freelocks = freelock;
    count++;
    lock->rel_lock();
}

spinlock* spinl_freelist::get_lock(char *Label, int maxwaittime) {
    spinlock *Ret;
    lock->get_lock();
    if (freelocks) {
	Ret = freelocks;
	freelocks = freelocks->next;
	count--;
	Ret->lockwaitmax = maxwaittime;
	Ret->next = 0;
	Ret->setLabel(Label);
	lock->rel_lock();
	return Ret;
	}
    else {
	lock->rel_lock();
	return new spinlock(Label, maxwaittime);
	}
}
