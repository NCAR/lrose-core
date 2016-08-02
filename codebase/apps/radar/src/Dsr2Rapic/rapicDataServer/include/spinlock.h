/*
	The spinlock class uses the IRIX spinlocks to implement
	interprocess synchronisation on sgi

	Uses simple_lock services on aix
*/

#ifndef	__SPINLOCK_H
#define __SPINLOCK_H

#ifdef sgi
#include <sys/bsd_types.h>
#endif

#include "bool.h"

#include "threadobj.h"

extern long long spinlock_lock_ops;
extern long long spinlock_lock_wait_ops;
extern long long spinlock_lock_fail_count;
extern long long spinlock_rel_ops;
extern bool spinlock_global_quit;

#ifdef THREAD_SPROC
#define USE_ABIMUTEX
#define _SGI_MP_SOURCE
// #define false 0
// #define true 1
#ifdef USE_ABIMUTEX
#include <abi_mutex.h>
typedef abilock_t* LOCKVAR;
#else
#include <ulocks.h>
typedef ulock_t LOCKVAR;
extern usptr_t *arena;
#endif
#include <sys/types.h>
#include <sys/select.h>  //SD add 21/12/99
#include <unistd.h>
// #include <sys/prctl.h>
#define GETPID() getpid()
#endif

#ifdef THREAD_PTHREAD
#include <time.h>
#ifdef aix
#include <sys/m_param.h>
#endif
typedef pthread_mutex_t* LOCKVAR;
#define GETPID() pthread_self()
#endif

#ifdef WIN32
#ifndef _WINDOWS_
#include "windows.h"
#endif
typedef HANDLE LOCKVAR;
#define delay Sleep
#define DLY_TCK 1000
#define THREADID_T DWORD
#define GETPID() GetCurrentThreadId()
#include <sys/time.h>
#endif


#include <stdio.h>
#include <sys/time.h>

extern bool DebugMode;
 
class spinlock {
  friend class spinl_freelist;
  LOCKVAR	lock;
  spinlock *next;
  THREADID_T	holder_pid;
#ifndef WIN32
  timeval lockedtime;  // time that this was locked, for debugging
  int	nestcount;		// track depth of lock nesting
  int	nested;			// true if lock was nested	
#else
  DWORD	lockedtime;    // WIN32 locks can nest OK.
#endif
  bool uselocks;       // if false do not use OS locks
  //  bool has_timed_out;  // set flag if lock has ever timed out
  uint  timeout_count;  // number of times lock has timed out
  bool isLocked;       // set while lock is locked
  int	ReleaseLockToOS;	// if true allow lock to be released prior to destructor call
  bool quitting;
 public:
  char label[32];
  char holderString[128];
  int  lockwaitmax;		// max no of 0.01secs
  int  waitingcount;		// count of processes waiting for lock
  int  maxwaitingcount;		// count of max processes waiting for lock
  bool get_lock(int waittime=-1,int quiet=0);	// blocking call, wait for lock
  bool get_lock(const char *holderstring, int waittime=-1, int quiet=0);	// blocking call, wait for lock
  //	int wait_lock(int waittime=-1,int quiet=0);
  bool ReleaseOSLock(int waittime=-1,int quiet=0);
  void AcquireOSLock();
  bool HasOSLock();	// true if lock != 0
  void rel_lock();	// release lock
  int	elapsed_time();	// time since lock set
  bool	hasTimedOut() { return timeout_count != 0; };
  int	timedOutCount() { return int(timeout_count); };
  void  setQuitting(); // quitting - force immediate get_lock timeouts
  spinlock(char *Label, int maxwaittime = -1); // wait time in 0.01 secs
  spinlock(char *Label, float waitsecs);
  ~spinlock();
  void setLabel(char *Label);
};

/* maintain a freelist of freed locks */
class spinl_freelist {
  spinlock *freelocks, *lock;
  int count;
 public:
  spinl_freelist();
  ~spinl_freelist();
  void add_lock(spinlock *freelock);
  spinlock *get_lock(char *Label, int maxwaittime = -1);
};

#endif	/* __SPINLOCK_H */
