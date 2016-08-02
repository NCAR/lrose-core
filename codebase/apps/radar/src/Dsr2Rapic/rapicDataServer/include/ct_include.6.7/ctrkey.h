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

#ifndef ctRKEYH
#define ctRKEYH

typedef struct {
	pTEXT   rsymb;	/* ptr to key symbolic name			*/
	pTEXT   radr;	/* ptr to key buffer				*/
	COUNT	rtype;
	UCOUNT	rlen;
	} RKEY;
typedef RKEY ctMEM *	pRKEY;

typedef struct {
	pTEXT   fldnam[3]; /* symbolic names from DATOBJ array		*/
	} RDAT;
typedef RDAT ctMEM *	pRDAT;

#endif /* ctRKEYH */

/* end of ctrkey.h */
