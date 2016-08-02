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

#ifndef CTISAMH
#define CTISAMH

#define ISADD		1
#define ISDEL		2
#define ISRWT		3

#ifdef MULTITRD

#ifdef RTREE
#define usrkey(i)	ctis[sOWNR]->s1[i].s1usrkey
#define usrdat(i)	ctis[sOWNR]->s1[i].s1usrdat
#define eusrkey(i)	       pis1->s1usrkey
#define eusrdat(i)	       pis1->s1usrdat
#define srtknm		ctis[sOWNR]->ssrtknm
#define srtdat		ctis[sOWNR]->ssrtdat
#define tmpdat		ctis[sOWNR]->stmpdat
#endif

#define seqbuf		ctis[sOWNR]->sseqbuf
#define seqold		ctis[sOWNR]->sseqold
#define seqlen		ctis[sOWNR]->sseqlen
#define seqkey		ctis[sOWNR]->sseqkey
#define seqnum		ctis[sOWNR]->sseqnum
#define maxqset		ctis[sOWNR]->smaxqset
#define ct_fndval	ctis[sOWNR]->sct_fndval
#define ct_nwrcfg	ctis[sOWNR]->sct_nwrcfg
#define ct_vfsg		ctis[sOWNR]->sct_vfsg
#define ct_ismlk	ctis[sOWNR]->sct_ismlk
#define ct_savset	ctis[sOWNR]->sct_savset
#define ct_dtmap(i)	ctis[sOWNR]->s1[i].s1ct_dtmap
#define ct_rvmap(i)	ctis[sOWNR]->s1[i].s1ct_rvmap
#ifdef ctCONDIDX
#define ct_rlkey(i)	ctis[sOWNR]->s1[i].s1ct_rlkey
#endif
#define ct_nlkey(i)	ctis[sOWNR]->s1[i].s1ct_nlkey
#define ct_nlchr(i)	ctis[sOWNR]->s1[i].s1ct_nlchr
#define ct_vfin(i)	ctis[sOWNR]->s1[i].s1ct_vfin
#define ct_sgpos(i,j)	ctis[sOWNR]->s2[i][j].s2ct_sgpos
#define ct_sglen(i,j)	ctis[sOWNR]->s2[i][j].s2ct_sglen
#define ct_sgmod(i,j)	ctis[sOWNR]->s2[i][j].s2ct_sgmod
#define ct_kymap(i,j)	ctis[sOWNR]->sct_kymap[i][j]

#define ect_dtmap(i)		pis1->s1ct_dtmap
#define ect_rvmap(i)		pis1->s1ct_rvmap
#ifdef ctCONDIDX
#define ect_rlkey(i)		pis1->s1ct_rlkey
#endif
#define ect_nlkey(i)		pis1->s1ct_nlkey
#define ect_nlchr(i)		pis1->s1ct_nlchr
#define ect_vfin(i)		pis1->s1ct_vfin
#define ect_sgpos(i,j)		pis2->s2ct_sgpos
#define ect_sglen(i,j)		pis2->s2ct_sglen
#define ect_sgmod(i,j)		pis2->s2ct_sgmod
#define ect_kymap(i,j)		pis->sct_kymap[i][j]

#else /* MULTITRD */

#ifndef ctNOGLOBALS
#ifdef TRANPROC
#define MAXFILt	(MAXFIL + ctTRN_FILES)
#else
#define MAXFILt	MAXFIL
#endif /* TRANPROC */
#ifdef RTREE
EXTERN COUNT	srtknm,srtdat,tmpdat;
#endif
EXTERN ppCOUNT	ctskymap;
EXTERN pCTIS1	ctis1;
EXTERN ppCTIS2	ctis2;

EXTERN TEXT	ct_fndval[MAXLEN];

EXTERN COUNT	ct_nwrcfg;
EXTERN COUNT	ct_vfsg;

EXTERN COUNT	ct_ismlk;

#ifdef ctOldALCSET
EXTERN pSAVSET  ct_savset;
#else
EXTERN ppSAVSET ct_savset;
#endif

#endif /* ~ctNOGLOBALS */

#ifdef RTREE
#define usrkey(i)	ctis1[i].s1usrkey
#define usrdat(i)	ctis1[i].s1usrdat
#define eusrkey(i)	pis1->s1usrkey
#define eusrdat(i)	pis1->s1usrdat
#endif
#define ct_dtmap(i)	ctis1[i].s1ct_dtmap
#define ct_rvmap(i)	ctis1[i].s1ct_rvmap
#ifdef ctCONDIDX
#define ct_rlkey(i)	ctis1[i].s1ct_rlkey
#endif
#define ct_nlkey(i)	ctis1[i].s1ct_nlkey
#define ct_nlchr(i)	ctis1[i].s1ct_nlchr
#define ct_vfin(i)	ctis1[i].s1ct_vfin
#define ct_kymap(i,j)	ctskymap[i][j]
#define ct_sgpos(i,j)	ctis2[i][j].s2ct_sgpos
#define ct_sglen(i,j)	ctis2[i][j].s2ct_sglen
#define ct_sgmod(i,j)	ctis2[i][j].s2ct_sgmod

#define ect_dtmap(i)	pis1->s1ct_dtmap
#define ect_rvmap(i)	pis1->s1ct_rvmap
#ifdef ctCONDIDX
#define ect_rlkey(i)	pis1->s1ct_rlkey
#endif
#define ect_nlkey(i)	pis1->s1ct_nlkey
#define ect_nlchr(i)	pis1->s1ct_nlchr
#define ect_vfin(i)	pis1->s1ct_vfin
#define ect_kymap(i,j)	ctskymap[i][j]
#define ect_sgpos(i,j)	pis2->s2ct_sgpos
#define ect_sglen(i,j)	pis2->s2ct_sglen
#define ect_sgmod(i,j)	pis2->s2ct_sgmod
#endif

#endif /* CTISAMH */

/* end of ctisam.h */
