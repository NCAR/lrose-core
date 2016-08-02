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

#define UGFNUM	(COUNT) (NAMINDX + 2)

ISEG u_seg[] = {
		{0,IDZ,UREGSEG}
		};
ISEG g_seg[] = {
		{0,IDZ,UREGSEG}
		};
ISEG ug_seg[] = {
		{0,IDZ + 1,UREGSEG},
		{IDZ + 1,IDZ,UREGSEG},
		{IDZ + 1,IDZ,UREGSEG},
		{0,IDZ,UREGSEG},
		};

IIDX u_idx[] = {
		{IDZ,0,0,0,0,1,u_seg}
		};
IIDX g_idx[] = {
		{IDZ,0,0,0,0,1,g_seg}
		};
IIDX ug_idx[] = {
		{2 * IDZ + 1,0,0,0,0,2,ug_seg},
		{2 * IDZ,0,0,0,0,2,ug_seg + 2}
		};

IFIL ctiug[3] = {
	{
	"FAIRCOM.FCS!UG",
	-1,
#ifdef ctSTRCH
	ctSIZE(FC_UG),
#else
	0,
#endif
	0,
	(SHARED | TRNLOG | ADMOPEN),
	2,
	0,
	(SHARED | TRNLOG | ADMOPEN),
	ug_idx
	},
	{
	"FAIRCOM.FCS!UG",
	-1,
#ifdef ctSTRCH
	ctSIZE(FC_UG),
#else
	0,
#endif
	0,
	(SHARED | TRNLOG | ADMOPEN),
	2,
	0,
	(SHARED | TRNLOG | ADMOPEN),
	ug_idx
	},
	{
	"FAIRCOM.FCS!UG",
	-1,
#ifdef ctSTRCH
	ctSIZE(FC_UG),
#else
	0,
#endif
	0,
	(SHARED | TRNLOG | ADMOPEN),
	2,
	0,
	(SHARED | TRNLOG | ADMOPEN),
	ug_idx
	}
};

IFIL ctigroup[3] = {
	{
	"FAIRCOM.FCS!GROUP",
	-1,
#ifdef ctSTRCH
	ctSIZE(FC_GROUP),
#else
	0,
#endif
	0,
	(SHARED | TRNLOG | ADMOPEN),
	1,
	0,
	(SHARED | TRNLOG | ADMOPEN),
	g_idx
	},
	{
	"FAIRCOM.FCS!GROUP",
	-1,
#ifdef ctSTRCH
	ctSIZE(FC_GROUP),
#else
	0,
#endif
	0,
	(SHARED | TRNLOG | ADMOPEN),
	1,
	0,
	(SHARED | TRNLOG | ADMOPEN),
	g_idx
	},
	{
	"FAIRCOM.FCS!GROUP",
	-1,
#ifdef ctSTRCH
	ctSIZE(FC_GROUP),
#else
	0,
#endif
	0,
	(SHARED | TRNLOG | ADMOPEN),
	1,
	0,
	(SHARED | TRNLOG | ADMOPEN),
	g_idx
	}
};

IFIL ctiuser[3] = {
	{
	"FAIRCOM.FCS!USER",
	-1,
#ifdef ctSTRCH
	ctSIZE(FC_USER),
#else
	0,
#endif
	0,
	(SHARED | TRNLOG | ADMOPEN),
	1,
	0,
	(SHARED | TRNLOG | ADMOPEN),
	u_idx
	},
	{
	"FAIRCOM.FCS!USER",
	-1,
#ifdef ctSTRCH
	ctSIZE(FC_USER),
#else
	0,
#endif
	0,
	(SHARED | TRNLOG | ADMOPEN),
	1,
	0,
	(SHARED | TRNLOG | ADMOPEN),
	u_idx
	},
	{
	"FAIRCOM.FCS!USER",
	-1,
#ifdef ctSTRCH
	ctSIZE(FC_USER),
#else
	0,
#endif
	0,
	(SHARED | TRNLOG | ADMOPEN),
	1,
	0,
	(SHARED | TRNLOG | ADMOPEN),
	u_idx
	}
};

