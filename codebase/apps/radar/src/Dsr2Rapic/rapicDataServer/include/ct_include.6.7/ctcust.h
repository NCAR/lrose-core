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

#ifndef ctCUSTH
#define ctCUSTH

#define ctCUSTvarlen	0x00000001	/* function requires output length */
#define dbMaxFiles	290
#define dbColSpace	2048

typedef struct offlen {
	UCOUNT	offset;
	UCOUNT	length;
	} OFFLEN;
typedef OFFLEN ctMEM * pOFFLEN;

typedef struct clicol {
	UCOUNT	offset;
	UCOUNT	length;
	TEXT	column[18];
	} CLICOL;
typedef CLICOL ctMEM * pCLICOL;

typedef struct opninf {
	UCOUNT	reclen;
	COUNT	numcol;
	COUNT	blbfno;
	COUNT	lclcol;
	UCOUNT	resrv1;
	UCOUNT	resrv2;
	CLICOL	clicol[1];
	} OPNINF;
typedef OPNINF ctMEM * pOPNINF;

typedef struct opc1 {
	LONG	cfn;
	LONG	chn;
	COUNT	cs1;
	COUNT	cs2;
	LONG	cl3;
	TEXT	cvr[FNZ];
	} OPC1;
typedef OPC1 ctMEM * pOPC1;

typedef struct opc2 {
	LONG	cfn;
	LONG	chn;
	COUNT	cs1;
	COUNT	cs2;
	LONG	cl3;
	VRLEN	cil;
	VRLEN	col;
	TEXT	cvr[8];
	} OPC2;
typedef OPC2 ctMEM * pOPC2;

extern char  custom_msg[];
extern char *custom_fnm[];

#endif /* ctCUSTH */

/* end of header */
