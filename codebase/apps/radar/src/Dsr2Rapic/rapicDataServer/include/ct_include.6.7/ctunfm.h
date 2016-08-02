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

#ifdef 	ctCTAPPX_C /* if included by ctappx.c */
#ifdef  ctLOCLIB
#ifdef  UNIFRMAT
#ifndef ctUNFMH
#define extUNFM
#endif /* ifndef ctUNFMH */
#endif /* ifdef UNIFRMAT */
#endif /* ifdef ctLOCLIB */
#endif /* ifdef ctCTAPPX_C */

#ifdef ctCTUNF2_C
#ifdef UNIFRMAT
#ifndef ctUNFMH
#define extUNFM
#endif /* ifndef ctUNFMH */
#endif /* ifdef UNIFRMAT */
#endif /* ifdef ctCTUNF2_C */

#ifdef extUNFM
#define ctUNFMH
extern ConvMap cthdrcmap;
extern ConvBlk cthdrcfld[];
extern ConvMap ctrescmap;
extern ConvBlk ctrescfld[];
extern ConvMap ctnodcmap;
extern ConvBlk ctnodcfld[];
extern ConvMap ctvarcmap;
extern ConvBlk ctvarcfld[];
extern ConvMap ctsupcmap;
extern ConvBlk ctsupcfld[];
extern ConvMap ctmapcmap;
extern ConvBlk ctmapcfld[];
extern ConvMap ctblkcmap;
extern ConvBlk ctblkcfld[];
extern ConvMap ctdelcmap;
extern ConvBlk ctdelcfld[];
extern ConvMap ctsctcmap;
extern ConvBlk ctsctcfld[];
extern ConvMap ctseqcmap;
extern ConvBlk ctseqcfld[];
extern ConvMap ctfdfcmap;
extern ConvBlk ctfdfcfld[];
extern ConvMap ctfilcmap;
extern ConvBlk ctfilcfld[];
extern ConvMap ctidxcmap;
extern ConvBlk ctidxcfld[];
extern ConvMap ctfilcmap2;
extern ConvBlk ctfilcfld2[];
extern ConvMap ctsegcmap;
extern ConvBlk ctsegcfld[];
extern ConvMap ctcihcmap;
extern ConvBlk ctcihcfld[];
extern ConvMap ctcibcmap;
extern ConvBlk ctcibcfld[];
#endif
/***************************************************************************/

/***************************************************************************/
#ifndef ctUNFMH
#define ctUNFMH

ConvMap cthdrcmap   = { 0,1,0,0,0,46,7 };
ConvBlk cthdrcfld[] = {
			{4,CT_INT4,9},
			{2,CT_INT2,7},
			{1,CT_CHAR,7},
			{2,CT_INT2,3},
			{4,CT_INT4,4},
			{2,CT_INT2,3},
			{4,CT_INT4,6}
		};

ConvMap ctrescmap   = { 0,1,0,0,0,8,2 };
ConvBlk ctrescfld[] = {
			{2,CT_INT2U,0},
			{4,CT_INT4,6}
		};

ConvMap ctnodcmap   = { 0,1,0,0,0,8,3 };
ConvBlk ctnodcfld[] = {
			{4,CT_INT4,1},
			{2,CT_INT2,3},
			{1,CT_CHAR,1}
		};

ConvMap ctvarcmap   = { 0,1,0,0,0,3,2 };
ConvBlk ctvarcfld[] = {
			{2,CT_INT2U,0},
			{4,CT_INT4,1}
		};

ConvMap ctsupcmap   = { 0,1,0,0,0,5,3 };
ConvBlk ctsupcfld[] = {
			{2,CT_INT2U,0},
			{4,CT_INT4,1},
			{4,CT_INT4,1}
		};

ConvMap ctmapcmap   = { 0,1,0,0,0,7,2 };
ConvBlk ctmapcfld[] = {
			{1,CT_CHARU,3},
			{4,CT_INT4,2}
		};

ConvMap ctblkcmap   = { 0,1,0,0,0,3,2 };
ConvBlk ctblkcfld[] = {
			{2,CT_INT2U,0},
			{1,CT_CHARU,1}
		};

ConvMap ctdelcmap   = { 0,1,0,0,0,2,2 };
ConvBlk ctdelcfld[] = {
			{1,CT_CHARU,0},
			{4,CT_INT4,0}
		};

ConvMap ctsctcmap   = { 0,1,0,0,0,3,2 };
ConvBlk ctsctcfld[] = {
			{FCRNAM_LEN,CT_FSTRING,0},
			{4,CT_INT4U,1}
		};

ConvMap ctseqcmap   = { 0,1,0,0,0,256,1 };
ConvBlk ctseqcfld[] = {
			{2,CT_INT2,255}
		};

ConvMap ctfdfcmap   = { 0,1,0,0,0,99,66};
ConvBlk ctfdfcfld[] = {
			{FCRNAM_LEN,CT_FSTRING,0},
			{4,CT_INT4U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1},
			{4,CT_INT4U,0},
			{2,CT_INT2U,1}
		};

ConvMap ctfilcmap   = { 0,1,0,0,0,8,2};
ConvBlk ctfilcfld[] = {
			{4,CT_INT4U,0},
			{2,CT_INT2,6}
		};

ConvMap ctidxcmap   = { 0,1,0,0,0,6,1};
ConvBlk ctidxcfld[] = {
			{2,CT_INT2,5}
		};

ConvMap ctfilcmap2   = { 0,1,0,0,0,12,4};
ConvBlk ctfilcfld2[] = {
			{4,CT_INT4U,0},
			{2,CT_INT2,6},
			{4,CT_INT4U,2},
			{2,CT_INT2,0}
		};

ConvMap ctsegcmap   = { 0,1,0,0,0,3,1};
ConvBlk ctsegcfld[] = {
			{2,CT_INT2,2}
		};

ConvMap ctcihcmap   = { 0,1,0,0,0,2,1 };
ConvBlk ctcihcfld[] = {
			{2,CT_INT2,1}
		};

ConvMap ctcibcmap   = { 0,1,0,0,0,3,2 };
ConvBlk ctcibcfld[] = {
			{2,CT_INT2,1},
			{4,CT_INT4,0}
		};

#endif /* ifndef ctUNFMH */
/**************************************************************************/

