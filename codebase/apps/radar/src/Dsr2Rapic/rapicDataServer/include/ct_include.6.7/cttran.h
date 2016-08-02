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

#ifndef CTTRANH
#define CTTRANH

typedef struct trandsc {
	LONG	trannum;
	LONG	tranpos;
	LONG	tranfil;
	VRLEN	tranlen;
	VRLEN	tranpln;
	UCOUNT	trantyp;
	UCOUNT	tranusr;
	} TRANDSC;
typedef TRANDSC ctMEM *	pTRANDSC;

typedef struct shadlst {
	struct shadlst ctMEM *shadlnk; /* list link */
	struct shadlst ctMEM *shadprv; /* list back link */
	LONG		      shadpos; /* actual file location */
	struct shadlst ctMEM *shadbak; /* save point back link */
	struct shadlst ctMEM *shadfwd; /* save point forward link */
	struct shadlst ctMEM *shadhsh; /* hash link */
	pTEXT	              shadloc; /* pointer to info in shadow buffer */
	VRLEN		      shadlen; /* actual record length */
	UCOUNT		      shadtyp; /* type of shadow entry */
	UCOUNT		      shadfil; /* actual file number */
	COUNT		      shadswp; /* swap space indicator */
	COUNT		      shadcnt; /* hash count */
	COUNT		      shadsav; /* savepoint */
	UCOUNT		      shadshf; /* shadow hash shift factor */
	} SHADLST;
typedef SHADLST ctMEM *	pSHADLST;
typedef SHADLST ctMEM * ctMEM * ppSHADLST;

typedef struct {
	VRLEN	segoff;
	VRLEN	seglen;
	} DIFSEG;
typedef DIFSEG ctMEM *	pDIFSEG;

#define FILCRE_CHECK	 1L
#define STDR_CHECK	 0L
#define FINAL_CHECK	-1L
#define BDUMP_CHECK	-2L
#define EDUMP_CHECK	-3L
#define IDLE_CHECK	-4L
#define API_CHECK	-5L
#define PDUMP_CHECK	-6L

#define SLNULL		(pSHADLST) NULL
#define DATCOM_WINDOW	2
#define IDXABT_WINDOW	4
#define MAXREGIONS	10

#define DIFF0		20
#define DIFF1		42
#define DIFF2		64
#define SIZLOGKEY	(1 + ctSIZE(LONG))

#define CHKPHDR		10	/* number of longs which begin checkpoint rec */
#define CM_CHKPHDR	0	/* commit mode				      */
#define LL_CHKPHDR	1	/* previous chkpnt log			      */
#define LP_CHKPHDR	2	/* previous chkpnt log position		      */
#define TR_CHKPHDR	3	/* active transactions			      */
#define FL_CHKPHDR	4	/* active files				      */
#define AN_CHKPHDR	5	/* abort nodes				      */
#define CN_CHKPHDR	6	/* commit nodes				      */
#define CD_CHKPHDR	7	/* commit dat				      */
#define VI_CHKPHDR	8	/* vulnerable index updates		      */
#define A1_CHKPHDR	9	/* # of auxhdr blocks (each CHKPHDR in len)   */
#define LG_CHKPHDR	0	/* login ID information		      	      */
#define XU_CHKPHDR	1	/* MXU2 value				      */

#ifdef VINES_JANET
#define DD_CHUNK	(VRLEN) 8092 
#define LOGBLOCKS	5	
#define BLOGPOS		1024L
#else
#define DD_CHUNK	(VRLEN) 59392
#define LOGBLOCKS	24
#define BLOGPOS		2048L
#endif

#define CTTRDELTA	100L	/* # of updates which cause flush */
#ifdef ctCTCPDELTA
#define CTCPDELTA	ctCTCPDELTA
#else
#define CTCPDELTA	2L	/* # of chkpnts (less 1) which cause flush */
#endif
#ifndef LOGCHUNKX
#define LOGCHUNKX	1100000L
#endif
#ifndef LOGCHUNKN
#define LOGCHUNKN	550000L
#endif
#define LOGLIMIT	2500000L
#define LOGSPR		NO
#define LOGPURGE	4

#define VOLATILE	24	/* size of dynamic region for SH_HDRCP */
#define SIZCTL		28	/* size of header area for chkpnt logging */
#define TOTCTL		0	/* header area index for LONG phyrec */
#define DELCTL		1	/* header area index for LONG delstk */
#define LOGCTL		2	/* header area index for LONG numrec */	
#define NUMCTL		6	/* header area index for long NUMENT */
/* ----------------------------------------------------------------- */
#define LNGCTL		(SIZCTL / ctSIZE(LONG))
#define ctRCV_cntl	0
#define ctRCV_lnum	LNGCTL
#define ctRCV_lpos	(ctRCV_lnum + 1)
#define ctRCV_fwrd	(ctRCV_lnum + 2)
#define ctRCV_fnam	(ctRCV_lnum + 3)
#define ctRCV_bytes	(ctRCV_fnam * ctSIZE(LONG))
/* ----------------------------------------------------------------- */
#define ctRDM_lnksiz	(ctSIZE(pTEXT) / ctSIZE(LONG))
#define ctRDM_link	0
#define ctRDM_cntl	(ctRCV_cntl + ctRDM_lnksiz)
#define ctRDM_lnum	(ctRCV_lnum + ctRDM_lnksiz)
#define ctRDM_lpos	(ctRCV_lpos + ctRDM_lnksiz)
#define ctRDM_fwrd	(ctRCV_fwrd + ctRDM_lnksiz)
#define ctRDM_fnam	(ctRCV_fnam + ctRDM_lnksiz)
#define ctRDM_bytes	(ctRDM_fnam * ctSIZE(LONG))
/* ----------------------------------------------------------------- */
#define ctHDRLOClocl	0
#define ctHDRLOCsupi	1
#define ctHDRLOClnum	2
#define ctHDRLOClpos	3
#define ctHDRLOCsize	4
/* ----------------------------------------------------------------- */
#define ctMBRLSTsupi	ctRDM_lnksiz
#define ctMBRLSTfili	(ctRDM_lnksiz + 1)
#define ctMBRLSTsrvi	(ctRDM_lnksiz + 2)
#define ctMBRLSTtimi	(ctRDM_lnksiz + 3)
#define ctMBRLSTsize	(ctMBRLSTtimi + 1)


#ifndef ctNOGLOBALS
EXTERN SEMA	 ct_undsema;	/* single tran undo semaphore */
EXTERN SEMA	 ct_1dmsema;	/* enforce only 1 dump at-a-time */
EXTERN SEMA	 ct_dmpsema;	/* dynamic dump semaphore */
EXTERN LONG	 ctdmptim;	/* dynamic dump beginn time */
EXTERN LONG	 ctdlgnum;	/* beginning log during dump */
EXTERN LONG	 ctelgnum;	/* ending log during dump */
EXTERN LONG	 ctdlgpos;	/* position in ctdlgnum */
EXTERN LONG	 ctelgpos;	/* position in ctelgnum */
EXTERN LONG	 ctcpcnt;	/* check point count */
EXTERN LONG	 ctcpchk;	/* check point count less delta */
EXTERN LONG	 ctelcnt;	/* check point count at last ENDLOG */
EXTERN COUNT	 ctshdlfil;	/* shadow swap file # */

EXTERN LONG	 ct_lstlog;	/* previous checkpoint log#		*/
EXTERN LONG	 ct_lstpos;	/* previous checkpoint log position	*/
EXTERN LONG	 ct_usrtr[MXU2];/* active transaction # for user or zero*/
EXTERN LONG	 ct_usrtl[MXU2];/* log file sequence number for BEGTRAN	*/
EXTERN LONG	 ct_usrix[MXU2];/* log file index op number		*/
#ifdef MULTITRD
#ifndef CTBOUND
EXTERN NINT	 ct_usrmd[MXU2];/* save transaction mode		*/
#endif
#endif
EXTERN NINT	 ct_usrty[MXU2];/* SHADOW / LOGFIL transaction type	*/
EXTERN NINT	 ct_chkpt[MXU2];/* check point flag 			*/
EXTERN pTEXT	 ctlgbf;	/* transaction log buffer ptr		*/
EXTERN pSEMA	 ct_usrsema;	/* commit/abort processing semaphore	*/
EXTERN ppSHADLST ctlstshd;	/* linked list tail for pre-image list	*/
EXTERN ppSHADLST ctlstsav;	/* last save point pointer		*/
#ifdef ctLOGIDX
EXTERN ppCOMLST  cttrnanc;	/* vulnerable tran hash list anchor	*/
#endif
EXTERN CTFILE	 ctLfil;	/* log file header */
EXTERN CTFILE	 ctUfil;	/* log file header */
EXTERN pCTFILE	 ctLnum;
EXTERN pCTFILE	 ctUnum;
EXTERN CTFILE	 ctSfil[2];	/* start file headers */
EXTERN pCTFILE	 ctSnum;
EXTERN VRLEN	 ct_lbfsiz;	/* log buffer size */
EXTERN NINT	 ctsflg;	/* ct_strip flag */
EXTERN NINT	 ctbflg;	/* BAKMOD flag */
EXTERN NINT	 ctdflg;	/* dynamic dump flag */
EXTERN NINT	 ct_actrns;	/* # active transactions */
EXTERN NINT	 ctpdmp;	/* pre-iamge dynamic dump flag */
EXTERN NINT	 ct_chkflg;	/* checkpoint in progress flag */
EXTERN NINT	 ct_logkep;	/* archive log count */
EXTERN NINT	 ctskpfil;	/* skip missing files during recovery */
EXTERN NINT	 ctskpmir;	/* skip missing mirrors during recovery */
EXTERN NINT	 ctfixlog;	/* non-zero means log size cannot increase */
EXTERN NINT	 ct_lerflg;	/* ctwrtlog error flag */
EXTERN COUNT	 ct_lgn;	/* current file# in log */
#endif

#include "ctfunt.h"

#endif /* CTTRANH */

/* end of cttran.h */
