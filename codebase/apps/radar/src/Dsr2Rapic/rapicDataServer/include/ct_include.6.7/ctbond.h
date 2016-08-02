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

#ifndef ctBONDH
#define ctBONDH

#ifdef MULTITRD
typedef struct lqmsg {
	pUGRUP  lgptr;
	COUNT	lerrc;
	COUNT	lmode;
	UTEXT	lflvr;
	UTEXT	lalgn;
	TEXT	lusid[IDZ];
	TEXT	luswd[PWZ];
} LQMSG;
#ifdef ctSP
typedef LQMSG far   *	pLQMSG;
typedef char far *	pMTEXT;
#else
typedef LQMSG ctMEM *	pLQMSG;
typedef char ctMEM *	pMTEXT;
#endif

#define U_UID	(ctiuser.tfilno + 1)
#define G_GID	(ctigroup.tfilno + 1)
#define UG_UNM	(ctiug.tfilno + 1)
#endif

#endif /* ctBONDH */

/* end of ctbond.h */
