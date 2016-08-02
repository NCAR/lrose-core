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

#ifndef ctATRDH
#define ctATRDH


#ifndef CTMUTEX
typedef struct ctmutex {
	LONG	flag;
	NINT	init;
} ctMUTEX;
#endif
typedef ctMUTEX ctMEM *	pctMUTEX;


#ifndef CTBLOCK
typedef struct ctblck {
	LONG	flag;
	NINT	init;
} ctBLOCK;
#endif
typedef ctBLOCK ctMEM *	pctBLOCK;


typedef struct ctsemap {
	NINT	sqid;
	NINT	scnt;
} ctSEMAP;
typedef ctSEMAP ctMEM * pctSEMAP;


#ifndef ctAppSTACKSIZE
#define ctAppSTACKSIZE	16384
#endif

#ifdef CTTHRDH
#define ctThrdCriticalIn	holdThreading();
#define ctThrdCriticalOut	freeThreading();
#endif

#ifdef PROTOTYPE
extern NINT mbThrdInit(NINT ,LONG );
extern NINT mbThrdTerm(VOID );
extern NINT mbThrdCreate(pctFUNC ,pVOID ,LONG );
extern VOID mbThrdExit(VOID );
extern NINT mbThrdSleep(LONG );
extern NINT mbThrdMutexGet(pctMUTEX );
extern NINT mbThrdMutexTry(pctMUTEX );
extern NINT mbThrdMutexRel(pctMUTEX );
extern NINT mbThrdMutexCls(pctMUTEX );
extern NINT mbThrdBlockGet(pctBLOCK ,LONG );
extern NINT mbThrdBlockWait(pctBLOCK ,LONG );
extern NINT mbThrdBlockRel(pctBLOCK );
extern NINT mbThrdBlockCls(pctBLOCK );
#else
extern NINT mbThrdInit();
extern NINT mbThrdTerm();
extern NINT mbThrdCreate();
extern VOID mbThrdExit();
extern NINT mbThrdSleep();
extern NINT mbThrdMutexGet();
extern NINT mbThrdMutexTry();
extern NINT mbThrdMutexRel();
extern NINT mbThrdMutexCls();
extern NINT mbThrdBlockGet();
extern NINT mbThrdBlockWait();
extern NINT mbThrdBlockRel();
extern NINT mbThrdBlockCls();
#endif

typedef struct thrdstub {
	pctFUNC	actfnc;
	pVOID	arglst;
	pVOID	usrvar;
	ctBLOCK	stbblk;
	NINT	errcod;
	NINT	refcon;
	} ThrdStub;
typedef ThrdStub * pThrdStub;

#endif /* ctATRDH */

/* end of ctatrd.h */
