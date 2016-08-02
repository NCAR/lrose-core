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

#ifndef ctVRECH
#define ctVRECH

#define	VDEL_FLAG	0xfdfd
#define VACT_FLAG	0xfafa
#define	VNOD_FLAG	0xfbbf
#define VRES_FLAG	0xfefe
#define VLNK_FLAG	0xfcfc

#define SIZVPAD		ctSIZE(UCOUNT)
#define	SIZVHDR		(ctSIZE(UCOUNT) + 2 * ctSIZE(VRLEN))
#define SIZRHDR		(SIZVHDR + ctSIZE(LONG))
#define SIZSHDR		(SIZVHDR + 2 * ctSIZE(LONG))

#define	VDEL_OLD	0xfdfd
#define VACT_OLD	0xfafa
#define	VNOD_OLD	0xfbfb
#define SIZVHDR_OLD	6

#endif /* ctVRECH */

/* end of header */
