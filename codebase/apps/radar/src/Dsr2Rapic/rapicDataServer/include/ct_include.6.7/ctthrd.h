/*
 *      OFFICIAL NOTIFICATION: the following CONFIDENTIAL and PROPRIETARY 
 * 	property legend shall not be removed from this source code module 
 * 	for any reason.
 *
 *	This program is the CONFIDENTIAL and PROPRIETARY property 
 *	of FairCom(R) Corporation. Any unauthorized use, reproduction or
 *	transfer of this computer program is strictly prohibited.
 *
 *      Copyright (c) 1984 - 1997 FairCom Corporation.
 *	This is an unpublished work, and is subject to limited distribution and
 *	restricted disclosure only. ALL RIGHTS RESERVED.
 *
 *			RESTRICTED RIGHTS LEGEND
 *	Use, duplication, or disclosure by the Government is subject to
 *	restrictions set forth in subparagraph (c)(1)(ii) of the Rights in
 * 	Technical Data and Computer Software clause at DFARS 252.227-7013.
 *	FairCom Corporation, 4006 West Broadway, Columbia, MO 65203.
 *
 *	c-tree PLUS(tm)	Version 6.7
 *			Release A2
 *			August 1, 1997
 */

#ifndef CTTHRDH
#define CTTHRDH

#ifdef ctEnvVar
ctEnvVar
#endif

#define	uchar(x) ((unsigned char)(x))

typedef unsigned short twoBytes;
typedef unsigned long fourBytes;
typedef unsigned char byte;


#define NoError (0)
#ifndef NO
#define NO	(0)
#define YES	(~NO)
#define OFF	(0)
#define ON	(~OFF)
#endif
#define ifnot(x) if (!(x))

/* Define null object pointer in case stdio.h isn't included */
/* #ifndef	NULL   WA */
#ifdef NULL
#undef NULL
#endif
#ifndef NULL
/* General purpose NULL pointer */
#define NULL (void *)0
#endif



#ifdef OneByteWait	/* this should be defined for netbios use */
#define WAITONTYPE byte
#define WAITONVALUE (0xff)
#else
#define WAITONTYPE COUNT
#define WAITONVALUE (1)
#endif

typedef struct Thread_ {
	struct Thread_	*nextthread;	/* Ptr to next Thread */
	struct Thread_	*prevthread;	/* Ptr to previous Thread */
	void 		*blocked;	/* defined code/Ptr to queue if blked */
	WAITONTYPE	*waiton;	/* Ptr to word event flag:1 if waiting*/
	THRD_BUF	 environs;	/* register save area */
#ifdef ctThrdSave
	THRD_BUF	 savethrd;	/* save starting context */
	LONG		 savetype;	/* save thread type: NewUser, ... */
#endif
	LONG		 refcon;	/* user reference constant */
	LONG		 priority;	/* relative importance:0==most */
	LONG		 frequency;	/* ticks between activations */
	ULONG		 lastcalled;	/* ticks at last deactivation */
	ULONG		 nextcall;	/* min tickcount for activation */
#ifdef ctThrdPad
	ULONG		 thdpad[ctThrdPad];
#endif
	ULONG		 waktime;	/* actual clock before reactivation */
	ULONG		 maxwait;	/* max tickcount before reactivation */
	LONG		 stack;		/* topmost byte of stack space	*/
}	Thread_;

typedef struct Threads_ {
	Thread_	*current;
	ULONG	defaultStack;
	Thread_	RealWorld;
	LONG	ThdCount;
	LONG	ThdHold;
	ULONG	ThdTime;
	ULONG	ThdSwitchTime;
#ifdef __MAC__
	byte	*SaveHeapEnd;
#endif
}Threads_;

typedef struct Flag_ {
	struct Thread_	*wait;		/* nonzero if a Thread waits on this q */
	} Flag_;

/*********/
#ifndef ctSRVR
#define CTMUTEX
typedef struct ctmutex {
	long int        flag;
	short int       ownr;
	short int       padg;
} ctMUTEX;
#define CTBLOCK
typedef struct ctblck {
	long int        flag;
	short int       ownr;
	short int       padg;
} ctBLOCK;
#endif /* ~ctSRVR */
/*********/

#define DeadLock  (~0)
#define NoWait   (0)
#define ShortWait (9)
#define WaitForever (~0)
#define posted(F) (nbrInQ((F)))
#define killFlag(F)  (killQ(F))
#define milliThreshHold		999L


#ifdef PROTOTYPE
extern	NINT	waitWhile(COUNT *,LONG );
extern	NINT	waitForFlag(Flag_ *,LONG );
extern	NINT	getFlag(Flag_ *,LONG );
extern	VOID	releaseFlag(Flag_ *);
extern	VOID	initFlag(Flag_ *);
extern	VOID	deleteFlag(Flag_ *);
#else
extern	NINT	waitWhile();
extern	NINT	waitForFlag();
extern	NINT	getFlag();
extern	VOID	releaseFlag();
extern	VOID	initFlag();
extern	VOID	deleteFlag();
#endif

#define	THREAD_ACTIVE	((VOID *)0L)	/*	Thread ready to run	*/
#define	THREAD_SLEEPING ((VOID *)1L)	/*	thread not ready to run	*/
#define THREAD_DEAD	((VOID *)(-1L))	/*	thread no longer valid	*/
#ifdef ctThrdApp
#define BASETHREAD 0
#else
#define BASETHREAD (1L)
#endif
#define LOWESTPRIORITY (0x7fffffff)

#ifndef DefaultStackSize
#define DefaultStackSize (8192)
#endif

#ifndef getThreadCtl
extern Threads_ *THRDGBLS;
#define getThreadCtl() ((Threads_ *)(THRDGBLS))
#endif

#define CURTHREAD (((Threads_ *)(THRDGBLS))->current)

#ifdef PROTOTYPE
extern VOID threadWake(VOID *q);
extern	ULONG	getThdTime(VOID);
extern	VOID	*threadInit(VOID *term);
extern	Thread_	*threadOk(Thread_ *t);
extern	Thread_	*myThread(VOID);
extern	LONG	nbrThreads(VOID);
extern	VOID	*threadStatus(Thread_ *t);
extern	VOID	endThread(VOID);
extern	VOID	killThread(Thread_ *t);
extern	VOID	abortThreading(VOID);
extern	VOID	sleepThread(Thread_ *t,LONG max);
extern	VOID	wakeThread(Thread_ *t);
extern	VOID	setPriority(Thread_ *t,LONG p);
extern	VOID	setFrequency(Thread_ *t,LONG f);
extern	VOID	setThreadTime(Thread_ *t,LONG w);
extern	ULONG	getThreadTime(VOID);
extern	VOID	holdThreading(VOID);
extern	VOID	freeThreading(VOID);
#ifndef __LOCALMEMORY__
extern	byte	*memory(LONG nbr,LONG size);
extern	VOID	memoryfree(byte *p);
#endif
extern	LONG	threadID(VOID);
extern	Thread_	*makeThread(ProcPtr thread,LONG size,LONG refcon,LONG priority,LONG freq,LONG narg,VOID *arg0);
extern	VOID	DEFER(VOID);
#else
extern	ULONG	getThdTime();
extern	VOID	*threadInit();
extern	Thread_	*threadOk();
extern	Thread_	*myThread();
extern	LONG	nbrThreads();
extern	VOID	*threadStatus();
extern	VOID	endThread();
extern	VOID	killThread();
extern	VOID	abortThreading();
extern	VOID	sleepThread();
extern	VOID	wakeThread();
extern	VOID	setPriority();
extern	VOID	setFrequency();
extern	VOID	setThreadTime();
extern	ULONG	getThreadTime();
extern	VOID	holdThreading();
extern	VOID	freeThreading();
#ifndef memory
extern	byte	*memory();
extern	VOID	memoryfree();
#endif
extern	LONG	threadID();
extern	Thread_	*makeThread();
extern	VOID	DEFER();
extern VOID threadWake();
#endif

#endif /* CTTHRDH */

/* end of ctthrd.h */
