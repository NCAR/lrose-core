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

#ifndef ctSEMA
#define ctSEMA

#ifdef ctDBGSEM 
#define SNAMELEN	4
#endif

typedef struct semaphore {
	LONG	flag;
	COUNT	ownr;
	COUNT	padg;
#ifdef ctDBGSEM 
	TEXT	snam[SNAMELEN];    /* debugging */
#endif
} SEMA;
typedef SEMA ctMEM *	pSEMA;

typedef struct stkstk {
	struct
	stkstk ctMEM
		*sp;	/* link list  */
	LONG	 st;	/* used last  */
	LONG	 sv;	/* long parm  */
	UINT	 sz;	/* stack size */
} STKSTK;
typedef STKSTK ctMEM *	pSTKSTK;
#define STKADJ	 ctSIZE(STKSTK)
#define STKIDL	 10L


#endif /* ctSEMA */

/* end of ctsema.h */
