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

#ifndef ctGVARH
#define ctGVARH

#ifdef VINES
#ifdef RUNTCBTABLE
#include "tasker.h"
#endif
#endif

#ifdef MTDEBUG
EXTERN int bomb;
#endif
#ifdef MTRCVR
EXTERN int ebomb;
#endif

#include "ctrkey.h"

/* #ifndef ctCLIENT */
#include "ctstrc.h"
#ifdef CTBOUND
#include "ctbond.h"
#else
#include "ctcomm.h"
#endif /* CTBOUND */

/*
** optional instance handle types, prototypes and arguments
*/

#ifdef MULTITRD
#define inTName		pCTGV
#define inName		lctgv
#define inExists

#ifdef ctSUPPRESS_thHandle
#define ctSTH
#else
#define thTName		NINT
#define thName		sOWNR
#define thExists
#endif /* ~ctSUPPRESS_thHandle */
#endif /* MULTITRD */

#ifdef ctThrdApp
#define inTName		pCTGVAR
#define inName		ctWNGV
#define inExists
#endif /* ctThrdApp */

#ifdef inExists
#define inType		, inTName
#define inHan		, inName
#define pinHan		, inTName inName
#define linHan		inTName inName;
#else
#define inType
#define inHan
#define pinHan
#define linHan
#endif /* ~inExists */

#ifdef thExists
#define thType		, thTName
#define thHan		, thName
#define pthHan		, thTName thName
#define lthHan		thTName thName;
#else
#define thType
#define thHan
#define pthHan
#define lthHan
#endif /* ~thExists */

#ifdef TRANPROC
#include "cttran.h"
#endif
/* #endif */ /* ~ctCLIENT */

typedef struct {
#ifdef RTREE
	RKEY	s1usrkey;
	RDAT	s1usrdat;
#endif
	COUNT	s1ct_dtmap;
	COUNT	s1ct_rvmap;
#ifdef ctCONDIDX
	COUNT	s1ct_rlkey;
#endif
	COUNT	s1ct_nlkey;
	TEXT	s1ct_nlchr;
	TEXT	s1ct_vfin;
	} CTIS1;
typedef CTIS1 ctMEM *	pCTIS1;

typedef struct {
	COUNT	s2ct_sgpos;
	COUNT	s2ct_sglen;
	COUNT	s2ct_sgmod;
	} CTIS2;
typedef CTIS2 ctMEM *	pCTIS2;
typedef CTIS2 ctMEM * ctMEM * ppCTIS2;

typedef struct reblst {
	struct reblst ctMEM *	nxtreb;
	LONG			trnreb;
	COUNT			numreb;
	COUNT			flgreb;
	TEXT			namreb[MAX_NAME];
	TEXT			tmpreb[MAX_NAME];
	} REBLST;
typedef REBLST ctMEM *	pREBLST;
typedef REBLST ctMEM * ctMEM * ppREBLST;

			/* Global Variables */

#ifdef ctThrds
EXTERN NINT	ctgv_max;
EXTERN LONG	ctgv_mode;
#endif

#ifdef ctNOGLOBALS

#ifdef ctLOCLIB
#define CTBOUNDvar
#define UNBOUNDvar
#ifdef PROTOTYPE
EXTERN pCOUNT (*ctlfseggetp)(COUNT , COUNT ,ppCOUNT );
EXTERN VOID   (*ctlfsegputp)(COUNT ,COUNT ,COUNT ,COUNT ,COUNT );
EXTERN COUNT  (*ctgetseginfop)(COUNT ,COUNT ,COUNT );
#else
EXTERN pCOUNT (*ctlfseggetp)();
EXTERN VOID   (*ctlfsegputp)();
EXTERN COUNT  (*ctgetseginfop)();
#endif
#endif

#ifdef CTBOUND
#ifndef CTBOUNDvar
#define CTBOUNDvar
#endif
#else /* CTBOUND */
#ifndef UNBOUNDvar
#define UNBOUNDvar
#endif
#endif /* CTBOUND */

#define MAXFILt	(MAXFIL + ctTRN_FILES)

typedef struct ctgvars
{
   pCEPFNC	ctcepfnc;	/* ctree single entry point func ptr 	*/
   struct ctgvars ctMEM
	       *pinstance;	/* instance link			*/
   pVOID	psysvar;	/* pointer to system-wide  variables	*/
   pVOID	pusrvar;	/* pointer to user-private variables	*/
   pVOID	pctcalbks;	/* pointer to function callbacks	*/
   pVOID	sctcidxStk;	/* conditional index stack 		*/
   UNLONG	ct_taskid;	/* save task ID info			*/
   LONG		sctnusers;	/* number of users			*/
#ifdef ctCLIENT
   LONG		sctdivs;
#else
   LONG		xctdivs;
#endif
   LONG		sctops;		/* set operation state bit mask		*/
   VRLEN	sctxvlen;	/* vlen combined op max buffer length	*/
   VRLEN	ssql_len;
   NINT		sctcidxStkPtr;
   NINT		sctcidxCurCmd;
   NINT		scthshift;	/* huge shift				*/
   NINT		sctmaxparm;
   NINT		sctrunfil;
   NINT		sctrcvfil;
   NINT		sct_adfil;
   NINT		sfrschgset;
   NINT		scurchgset;
   NINT		sfrschgbat;
   NINT		scurchgbat;
   COUNT	suerr_cod;	/* user error cod */
   COUNT	ssysiocod;	/* system error cod */
   COUNT	sisam_err;
   COUNT	sisam_fil;
   COUNT	ssql_status1;
   COUNT	ssql_status2;
   COUNT	scndxerr;
   COUNT	sct_mxfil;	/* maximum files specified in intree. 	*/
   COUNT	sctusrprf;	/* user profile word */
#ifdef ISAM_VARS
#ifdef RTREE
   COUNT	ssrtknm,ssrtdat,stmpdat;
#endif
#endif
   TEXT		instname[IDZ];	/* name of this instance block	*/
   UTEXT 	sct_uflvr;
   UTEXT 	sct_ualgn;
   UTEXT	sct_upntr;	/* client pointer size */
#ifdef CTBOUND
   pVOID		scommCmds[MAXCOMMP];
#else
   pCommFuncPtrs	scommCmds[MAXCOMMP];
#endif
   pVOID		scommGbls[MAXCOMMP];
#ifdef CTBOUNDvar
   LSTANC	sctlist[NUMCTLIST];
   CTFILE   sctfmru;
   pCTFILE  sct_mru;
   ppCTFILE sctfcbhdr;
   ppctICON sctconanc;
#ifdef DBGtree
   pVOID	 sct_walkh;	/* tree walk header			*/
   pVOID	 sct_walkt;	/* tree walk tail			*/
   NINT		 sctlflg;	/* load flag				*/
#endif /* DBGtree */
   pTREBUF	 sct_btree;	/* ptr to beg of b-tree buffers		*/
   pDATBUF	 sct_dbufr;	/* ptr to beg of data file buffers	*/
   pCTFILE	 sct_key;	/* ptr to beg of ct_key file structures	*/
   pCTFILE	 sct_dat;	/* ptr to beg of data file structures	*/
   pCTFILE	 sct_vat;	/* ptr to beg of var data file struc	*/
   pLOKS	 sct_locks[LOKHBINS];
   pLOKS	 sct_ltail[LOKHBINS];
#ifdef DBGhash
   LONG	 	 sdbg_lhsh_u[LOKHBINS];
#endif
   LONG	 sctactfil;	/* c-tree files physically opened	*/
   LONG	 scttotfil;	/* c-tree files opened			*/
   LONG	 scttotblk;	/* c-tree file control blocks in use	*/
   LONG	 sct_spc;
   ULONG sct_dbrqs;	/* data buffer requests			*/
   ULONG sct_dbhit;	/* data buffer hits			*/
   ULONG sct_ibrqs;	/* index buffer requests		*/
   ULONG sct_ibhit;	/* index buffer hits			*/
   ULONG sct_rdops;	/* number of read operations		*/
   ULONG sct_rdbyt;	/* bytes read				*/
   ULONG sct_wrops;	/* number of write operations		*/
   ULONG sct_wrbyt;	/* bytes written			*/
   ULONG sct_rcops;	/* number of comm read operations	*/
   ULONG sct_rcbyt;	/* comm bytes read			*/
   ULONG sct_wcops;	/* number of comm write operations	*/
   ULONG sct_wcbyt;	/* comm bytes written			*/
   ULONG sct_trbeg;	/* # transaction begins			*/
   ULONG sct_trend;	/* # transaction ends			*/
   ULONG sct_trabt;	/* # transaction aborts			*/
   ULONG sct_trsav;	/* # transaction savepoints		*/
   ULONG sct_trrst;	/* # transaction restores		*/
   LONG	 sct_abnod;	/* abort list node count		*/
   LONG	 sct_cmnod;	/* pending node tran count		*/
   LONG	 sct_cmdat;	/* pending data tran count		*/
   LONG	 sct_statflg;	/* CTSTATUS flag			*/
   LONG  sct_langflg;	/* LANGUAGE flag			*/
#ifdef ctNogloTran
   LONG	 sct_numvi;	/* vulnerable index update count	*/
   ULONG sendcnt;	/* end log count			*/
#endif
#ifdef DBG749x
   UINT  sctlstfrq;
#endif
   UINT  sct_tmpseq;	/* temporary file sequence number	*/
   UINT	 sct_lhbins;
   UINT  sctconbins;	/* context bins				*/
   NINT	 sct_dq;	/* shrink task logical Q handle		*/
   NINT	 sct_cq; 	/* checkpoint task logical Q handle	*/
   NINT	 sctfnz;	/* superfile member max name size	*/
   NINT  sctmiroff;	/* if YES, turn off all mirroring	*/
   COUNT sctconid;	/* last assigned context id		*/
#ifdef TRANPROC
   pREBLST   sctrebhed;
   pREBLST   sctcmphed;
   pREBLST   sctmirhed;
   pREBLST   sctiblhed;
   ppSHADLST sct_ubit;
#ifdef DBGhash
   pLONG     sdbg_shsh_u;
#endif
   pTEXT sctmrklst;
   pTEXT sct_usrsp;
   pSHADLST sct_usrsl;
   pTEXT sctlgbf;	/* transaction log buffer ptr */
   ppTEXT    sctlogmemanc;
   ppSHADLST sctlstshd;	/* linked list tail for pre-image list */
   ppSHADLST sctlstsav;	/* last save point pointer */
   ppCOMLST  scttrnanc; /* vulnerable tran hash list anchor */
   pCTFILE sctLnum;
   pCTFILE sctUnum;
   pCTFILE sctSnum;
   CTFILE sctLfil;	/* log file header */
   CTFILE sctUfil;	/* log file header */
   CTFILE sctSfil[2];	/* start file headers */
   LONG  strnwrn;	/* transaction # overflow warning flag */
   LONG	 sprv_logser;	/* previous checkpoint log#	*/
   LONG	 sprv_chkpos;	/* previous checkpoint pos	*/
   LONG	 sprv_prvpos;	/* previous checkpoint back link*/
   LONG	 sctlogchklmt;
   LONG  sctshdmem;	/* shadow memory: excluding control structures */
   LONG  sctusrsum;
   LONG  sctusrtot;
   LONG	 sctdmptim;	/* dynamic dump beginn time */
   LONG	 sctdlgnum;	/* beginning log during dump */
   LONG	 sctelgnum;	/* ending log during dump */
   LONG	 sctdlgpos;	/* position in ctdlgnum */
   LONG	 sctelgpos;	/* position in ctelgnum */
   LONG	 sctcpcnt;	/* check point count */
   LONG	 sctcpchk;	/* check point count less delta */
   LONG  sctelcnt;	/* check point count at last ENDLOG */
   LONG	 sct_lstlog;	/* previous checkpoint log# */
   LONG	 sct_lstpos;	/* previous checkpoint log position */
   LONG	 sct_usrtr[MXU2];/* active transaction # for user or zero */
   LONG	 sct_usrtl[MXU2];/* log file sequence number for BEGTRAN */
   LONG  sct_usrix[MXU2];/* log file index op number */
   LONG  sct_trnhghmrk;	/* threshhold to warn about tran high-water mark */
   VRLEN sct_lbfsiz;	/* log buffer size */
   VRLEN sct_usrsz;
   UINT	 sct_shbins;
   UINT	 sct_shbyts;
   NINT	 sct_tryprvchk;	/* try previous checkpoint */
   NINT  sctsuplog;	/* suppress log flush on begin and end tran */
   NINT  sctfstr;
   NINT	 sct_chktrd;
   NINT	 sct_chkflg;	/* checkpoint in progress flag */
   NINT	 sct_logkep;	/* archive log count */
   NINT  sctskpfil;	/* skip missing files during recovery */
   NINT	 sctskpmir;	/* skip missing mirrors during recovery */
   NINT  sctfixlog;	/* non-zero means log size cannot increase */
   NINT  sctpdmp;	/* preimage dynamic dump flag */
   NINT  sctsflg;	/* ct_strip flag */
   NINT  sctbflg;	/* BAKMOD flag */
   NINT  sctdflg;	/* dynamic dump flag */
   NINT  sct_actrns;	/* # active transactions */
   NINT  sct_usrsi;
   NINT  sct_usrsv;
   NINT  sct_rstflg;	/* TRANRST() flag */
   NINT	 sctlogmem;
   NINT	 sctlogmemmax;
   NINT	 sctlogdet;
   NINT	 sctskpclnfil;
   COUNT sct_usrty[MXU2];/* SHADOW / LOGFIL transaction type */
   COUNT sctshdlfil;	/* shadow swap file # */
   COUNT sct_lgn;	/* current file# in log */
   COUNT sct_lerflg;	/* ctwrtlog error flag */
   TEXT	 sct_chkpt[MXU2];/* check point flag */
#endif /* TRANPROC */
#ifdef ctNogloSize
   pVOID   sctrebhed;
   pVOID   sctcmphed;
   pVOID   sctmirhed;
   pVOID   sctiblhed;
   ppVOID sct_ubit;
#ifdef DBGhash
   pLONG  sdbg_shsh_u;
#endif
   pVOID sctmrklst;
   pVOID sct_usrsp;
   pVOID sct_usrsl;
   pVOID sctlgbf;	/* transaction log buffer ptr */
   ppVOID    sctlogmemanc;
   ppVOID sctlstshd;	/* linked list tail for pre-image list */
   ppVOID sctlstsav;	/* last save point pointer */
   ppVOID  scttrnanc; /* vulnerable tran hash list anchor */
   pVOID sctLnum;
   pVOID sctUnum;
   pVOID sctSnum;
   CTFILE sctLfil;	/* log file header */
   CTFILE sctUfil;	/* log file header */
   CTFILE sctSfil[2];	/* start file headers */
   LONG  strnwrn;	/* transaction # overflow warning flag */
   LONG	 sprv_logser;	/* previous checkpoint log#	*/
   LONG	 sprv_chkpos;	/* previous checkpoint pos	*/
   LONG	 sprv_prvpos;	/* previous checkpoint back link*/
   LONG	 sctlogchklmt;
   LONG  sctshdmem;	/* shadow memory: excluding control structures */
   LONG  sctusrsum;
   LONG  sctusrtot;
   LONG	 sctdmptim;	/* dynamic dump beginn time */
   LONG	 sctdlgnum;	/* beginning log during dump */
   LONG	 sctelgnum;	/* ending log during dump */
   LONG	 sctdlgpos;	/* position in ctdlgnum */
   LONG	 sctelgpos;	/* position in ctelgnum */
   LONG	 sctcpcnt;	/* check point count */
   LONG	 sctcpchk;	/* check point count less delta */
   LONG  sctelcnt;	/* check point count at last ENDLOG */
   LONG	 sct_lstlog;	/* previous checkpoint log# */
   LONG	 sct_lstpos;	/* previous checkpoint log position */
   LONG	 sct_usrtr[MXU2];/* active transaction # for user or zero */
   LONG	 sct_usrtl[MXU2];/* log file sequence number for BEGTRAN */
   LONG  sct_usrix[MXU2];/* log file index op number */
   LONG  sct_trnhghmrk;	/* threshhold to warn about tran high-water mark */
   VRLEN sct_lbfsiz;	/* log buffer size */
   VRLEN sct_usrsz;
   UINT	 sct_shbins;
   UINT	 sct_shbyts;
   NINT	 sct_tryprvchk;	/* try previous checkpoint */
   NINT  sctsuplog;	/* suppress log flush on begin and end tran */
   NINT  sctfstr;
   NINT	 sct_chktrd;
   NINT	 sct_chkflg;	/* checkpoint in progress flag */
   NINT	 sct_logkep;	/* archive log count */
   NINT  sctskpfil;	/* skip missing files during recovery */
   NINT	 sctskpmir;	/* skip missing mirrors during recovery */
   NINT  sctfixlog;	/* non-zero means log size cannot increase */
   NINT  sctpdmp;	/* preimage dynamic dump flag */
   NINT  sctsflg;	/* ct_strip flag */
   NINT  sctbflg;	/* BAKMOD flag */
   NINT  sctdflg;	/* dynamic dump flag */
   NINT  sct_actrns;	/* # active transactions */
   NINT  sct_usrsi;
   NINT  sct_usrsv;
   NINT  sct_rstflg;	/* TRANRST() flag */
   NINT	 sctlogmem;
   NINT	 sctlogmemmax;
   NINT	 sctlogdet;
   NINT	 sctskpclnfil;
   COUNT sct_usrty[MXU2];/* SHADOW / LOGFIL transaction type */
   COUNT sctshdlfil;	/* shadow swap file # */
   COUNT sct_lgn;	/* current file# in log */
   COUNT sct_lerflg;	/* ctwrtlog error flag */
   TEXT	 sct_chkpt[MXU2];/* check point flag */
#endif /* ctNogloSize */
   VRLEN sct_bfsiz;	/* data buffer size			*/
   NINT  sctlogidxfrc;	/* LOGIDX override: YES-on HYS-off	*/
   NINT	 sctrdmp_flg;	/* signals dynamic dump recovery	*/
   NINT	 sctmdmp_flg;	/* signals dyn dump recovery had mirrors*/
   NINT	 sctrflg;	/* automatic recovery flag		*/
   NINT	 sctrbflg;	/* rebuild flag				*/
   NINT	 sctfnstrat;	/* file name conversion startegy	*/
   NINT	 scttflg;	/* stop server in progress		*/
   NINT  sctfilcre;	/* flag for create since checkpoint	*/
   COUNT sct_mxu1;	/* max users plus origin		*/
   NINT	 sct_cmnown;	/* common onwer number (semaphores)	*/
   UINT	 sct_hshft;	/* hash shift parameter			*/
   UINT	 sct_hbins;	/* hash bins for buffers		*/
   UINT	 sct_dshft;	/* datbuf hash shift parm		*/
   UINT	 sct_dbins;	/* datbuf hash bins			*/
#ifdef ctNogloTran
   UINT	 sct_tbins;	/* vulnerable tran# hash bins		*/
#endif
   BHL	 sct_bavl[2];	/* buffer avl list anchors		*/
   ppBHL sct_bhla[NUMANCHOR];/* buffer hash list anchors	*/
#ifdef DBGhash
   pLONG sdbg_bhl_n;
   pLONG sdbg_bhl_d;
#endif
   pTEXT sctsdname;	/* server directory name		*/
   pTEXT sctsvname;	/* server name				*/
   pTEXT sctsqname;	/* server SQL name			*/
#ifdef VINESLOG
   TEXT	 sctlgname[MAX_NAME];
				/* server status log name		*/
#endif

   pTEXT sctscommp[MAXCOMMP];
				/* server comm protocol			*/
   pTEXT sct_del;	/* constant 0xff array			*/
   UINT	 sct_delsiz;	/* size of 0xff array			*/
   UINT	 scts_list_m;	/* list memory block size		*/
   UINT	 scts_sort_m;	/* sort memory buffer size		*/
   UINT	 scts_bufr_m;	/* bufr memory block size		*/
   TEXT	 sctifl_ext[2][EXZ];
				/* default IFIL extensions		*/

   UINT  sct_mxbuf;		/* maximum buffers specified in intree 	*/
   UINT  sct_dxbuf;		/* maximum data file buffers 		*/
   NINT  sct_maxvfil;		/* virtual file limit */
   NINT  sct_numvfil;		/* number of virtual files open		*/
   NINT  sct_avlfil;		/* available file control block counter */
   COUNT sct_ndsec;		/* # of sectors per node specified in intree */
   COUNT sct_ver;		/* defines configuration options */
   TEXT  sctsupsep;

#ifdef ctOldALCBAT
   COUNT   sbatmax;
   COUNT   sbatnum;
   pBATHDR ssavbat;
#else
   pSAVBAT sbatnum;
   ppSAVBAT
	   ssavbat;
#endif
   BATHDR  batch;
   HSTHDR  history;
   pHSTLST histund;
   pSAVHST shstnum;
   ppSAVHST
	   ssavhst;
   LONG    sct_npath[MAXLEV];	/* array to trace path down B-tree. Assumes  */
				/* maximum of MAXLEV - 1 levels in B-tree.   */
   NINT  sct_nelem[MAXLEV];
   NINT  sct_tight[MAXLEV];
   NINT  sct_melem[MAXLEV];
					
   TEXT sct_dupkey[MAXLEN+1];/* for searches of duplicate keys */
   TEXT sspkey[MAXLEN+1];	/* temporary storage for key values during */
				/* node updates 			   */

   NINT  sct_elm;		/* position within B-Tree node	*/
   NINT  sct_tky;		/* result of comparison between target	*/
				/* value and index entry.		*/
				/* ct_tky < 0 => target < index entry	*/
				/*	  = 0 => target = index entry	*/
				/*	  > 0 => target > index entry 	*/
   NINT  sct_tkp;		/* previous value of ct_tky		*/
   NINT  sct_sfxctp;		/* previous value of suffix count	*/
   LONG  sct_lnode;		/* last node found during walk down tree */
   LONG  sct_fnode;		/* node found during search/retrieval    */
   LONG  sct_nwnod;		/* pointer to new node */
   LONG  sct_gsrl;		/* next serial number for data file */
   NINT  sct_trdflg;		/* thread operation flag */
   NINT  sbtlev;		/* b-tree level counter. used as index of */
				/* ct_npath				  */
   TEXT  sct_buf[CTBUFSIZ];	/* temporary io buffer */

/*
** ISAM global variables from ctisam.h
*/

#ifdef ISAM_VARS
   ppCOUNT	sctskymap;
   pCTIS1	sctis1;
   ppCTIS2	sctis2;

   TEXT		sct_fndval[MAXLEN];

   COUNT	sct_nwrcfg;
   COUNT	sct_vfsg;

   COUNT	sct_ismlk;

#ifdef ctOldALCSET
   pSAVSET  sct_savset;

   COUNT   sseqlen;		/* significant length of key	*/
   COUNT   sseqkey;		/* current keyno		*/
   COUNT   sseqnum;		/* current set number		*/
   COUNT   smaxqset;
#else
   ppSAVSET
	   sct_savset;		/* pointer to hash anchors	*/
   pSAVSET sseqnum;		/* current set pointer		*/
   COUNT   sseqlen;		/* significant length of key	*/
   COUNT   sseqkey;		/* current keyno		*/
#endif
   TEXT    sseqbuf[MAXLEN];	/* holds matching "partial" key */
#ifdef MUSTFRCE
   TEXT	   sseqold[MAXLEN];
#else
#ifdef ctLOCLIB
   TEXT    sseqold[MAXLEN];
#endif
#endif /* MUSTFRCE */
#endif

   TEXT sl_lastkey[MAXLEN];
   LONG sl_begnod;
   LONG sl_curnod;
   LONG sl_prvnod;
   LONG sl_numnod;
   LONG sl_ct_nnods[MAXLEV];
   NINT sl_elem;
   NINT sl_started;
   pTREBUF sl_savbp;
   VRLEN sl_hdradj;

   COUNT stranactv;
   COUNT stransavp;
   UCOUNT slstsiz[NUMCTLIST];
#ifdef DBG749x
   UINT slstchk[NUMCTLIST];
#endif
   VRLEN slstcnt[NUMCTLIST];
#endif /* CTBOUNDvar */
#ifdef UNBOUNDvar
   pTEXT	sctusrbuf;
#ifdef ctCLIENT
   pLQMSG	sctlqmsg;
   pLOCLFILE	shlocl;
   pLOCLAUX	shauxl;
#else
   pVOID	ctlqfake;
   pVOID	hloclfake;
   pVOID	hauxlfake;
#endif
   VRLEN	sctusrlen;
   VRLEN	sctusrpos;
   VRLEN	sctsrvlen;
   UTEXT	sctsflvr[8];
   TEXT		sctnodnam[IDZ];
   NINT		sct_lq;		/* logon logical Q handle		*/
   NINT		sctautotfrm;
   NINT		sctmaxdatkey;
   NINT		sctisam_flg;
   NINT		sct_autopos_clr;
   UINT		salign_override;
   COUNT	sctsrvver;
#endif /* UNBOUNDvar */

#ifdef PROTOTYPE
   NINT		(*intcalbak)(pCTINIT1 );
#else
   NINT		(*intcalbak)();
#endif

#ifdef CTBOUNDvar
   pTEXT	sct_rbuf;
#ifdef ctNogloTran
   pTEXT	sctlpathf[10];
   pTEXT	sctlpathm[10];
   pTEXT	slogptr;
   pLONG	sct_rcvlst;
   pLONG	sct_hdrlst;
   pLONG	sct_mbrlst;
   LONG		sct_trndlt;
   LONG		sct_cpdlt;
   LONG		sct_logchn;
   LONG		sct_loglmt;
   LONG		sct_logspc;
   LONG		sct_logrec;
   LONG		sct_logtrg;
   LONG		sct_blgpos;
   LONG		sct_logprg;
   LONG		sct_lnmlog;
   LONG		sct_hnmlog;
   LONG		scttopcmt;
   LONG		scttoplog;
   LONG		sctbegcmt;
   LONG		sctbeglog;
   LONG		slogbyt;
   LONG		slowpnt;
   LONG		shghcmt;
   LONG		shghlog;
   VRLEN	slogrem;
   VRLEN	slbfpag;
   NINT		sct_logspr;
   NINT		sanydelmrk;
   NINT		sintpthflg;
   NINT		sctlpathfa[10];
   NINT		sctlpathma[4];
   COUNT	sreserved;
   COUNT	sct_logblk;
   TEXT		sundflg[(MXU2 + 3) & ~3]; /* ensure a multiple of 4 */
#endif /* ctNogloTran */
   VRLEN	sct_bsz;
   NINT		sctcatcnt;
   COUNT	srerr_cod;
   COUNT	srerr_fil;
   COUNT	sct_fp;
#endif /* CTBOUNDvar */

} CTGVAR, ctMEM * pCTGVAR, ctMEM * ctMEM * ppCTGVAR;

#ifdef ctThrdApp

#ifndef ctCLIENTcore
#define ctWNGV	((pCTGVAR)ctThrdGet())
#endif

EXTERN NINT	ctgv_incr;
EXTERN ppCTGVAR ctgv_thd;
#ifdef ctThrdRet
EXTERN ppVOID	ctgv_ptr;
#endif

#else  /* ctThrdApp */
#ifdef ctThrdFPG

#ifdef CTPERM
EXTERN pCTGVAR ctWNGV;
#else
#ifndef rtThrds
#define ctWNGV	((pCTGVAR)ctThrdGet())
#endif	/* ~rtThrds */
#endif

EXTERN NINT	ctgv_incr;
EXTERN ppCTGVAR ctgv_thd;
#ifdef ctThrdRet
EXTERN ppVOID	ctgv_ptr;
#endif

#else /* ctThrdFPG */

EXTERN pCTGVAR ctWNGV;

#endif /* ctThrdFPG */
#endif /* ctThrdApp */

#define ctumxfil	ct_mxfil
#define cthghfil	ct_mxfil
#define commCmds	ctWNGV->scommCmds
#define commGbls	ctWNGV->scommGbls

#ifdef CTBOUND
#define ctrebhed	ctWNGV->sctrebhed
#define ctcmphed	ctWNGV->sctcmphed
#define ctmirhed	ctWNGV->sctmirhed
#define ctiblhed	ctWNGV->sctiblhed
#define lstsiz		ctWNGV->slstsiz
#ifdef DBG749x
#define lstchk		ctWNGV->slstchk
#endif
#define lstcnt		ctWNGV->slstcnt
#define ctlist		ctWNGV->sctlist

#define ctfmru        ctWNGV->sctfmru
#define ct_mru        ctWNGV->sct_mru
#define ctfcbhdr      ctWNGV->sctfcbhdr
#define ctconanc      ctWNGV->sctconanc
#define ctconid       ctWNGV->sctconid
#define ctconbins     ctWNGV->sctconbins
#ifdef DBGtree
#define ct_walkh      ctWNGV->sct_walkh
#define ct_walkt      ctWNGV->sct_walkt
#define ctlflg        ctWNGV->sctlflg
#endif /* DBGtree */
#define ct_btree      ctWNGV->sct_btree
#define ct_dbufr      ctWNGV->sct_dbufr
#define ct_key        ctWNGV->sct_key
#define ct_dat        ctWNGV->sct_dat
#define ct_vat        ctWNGV->sct_vat
#ifdef DBG749x
#define ctlstfrq      ctWNGV->sctlstfrq
#endif
#define ct_tmpseq     ctWNGV->sct_tmpseq
#define ct_lhbins     ctWNGV->sct_lhbins
#define ct_locks      ctWNGV->sct_locks
#ifdef DBGhash
#define dbg_lhsh_u    ctWNGV->sdbg_lhsh_u
#endif
#define ct_ltail      ctWNGV->sct_ltail
#define ctactfil      ctWNGV->sctactfil
#define cttotfil      ctWNGV->scttotfil
#define cttotblk      ctWNGV->scttotblk
#define ct_spc	      ctWNGV->sct_spc
#define ct_dbrqs      ctWNGV->sct_dbrqs
#define ct_dbhit      ctWNGV->sct_dbhit
#define ct_ibrqs      ctWNGV->sct_ibrqs
#define ct_ibhit      ctWNGV->sct_ibhit
#define ct_rdops      ctWNGV->sct_rdops
#define ct_rdbyt      ctWNGV->sct_rdbyt
#define ct_wrops      ctWNGV->sct_wrops
#define ct_wrbyt      ctWNGV->sct_wrbyt
#define ct_rcops      ctWNGV->sct_rcops
#define ct_rcbyt      ctWNGV->sct_rcbyt
#define ct_wcops      ctWNGV->sct_wcops
#define ct_wcbyt      ctWNGV->sct_wcbyt
#define ct_trbeg      ctWNGV->sct_trbeg
#define ct_trend      ctWNGV->sct_trend
#define ct_trabt      ctWNGV->sct_trabt
#define ct_trsav      ctWNGV->sct_trsav
#define ct_trrst      ctWNGV->sct_trrst
#define ct_abnod      ctWNGV->sct_abnod
#define ct_cmnod      ctWNGV->sct_cmnod
#define ct_cmdat      ctWNGV->sct_cmdat
#define ct_statflg    ctWNGV->sct_statflg
#define ct_langflg    ctWNGV->sct_langflg
#define ct_numvi      ctWNGV->sct_numvi
#define endcnt        ctWNGV->sendcnt
#define ct_dq         ctWNGV->sct_dq
#define ct_cq         ctWNGV->sct_cq
#define ctfnz         ctWNGV->sctfnz
#ifndef BOUND
#define ct_lq         ctWNGV->sct_lq
#endif
#define ctmiroff	ctWNGV->sctmiroff
#ifdef TRANPROC
#define ct_ubit(owner)	ctWNGV->sct_ubit
#ifdef DBGhash
#define dbg_shsh_u(owner) ctWNGV->sdbg_shsh_u
#endif
#define ctmrklst	ctWNGV->sctmrklst
#define ct_usrsp	ctWNGV->sct_usrsp
#define ct_usrsl	ctWNGV->sct_usrsl
#define ct_usrsz	ctWNGV->sct_usrsz
#define ctshdmem	ctWNGV->sctshdmem
#define ctusrsum	ctWNGV->sctusrsum
#define ctusrtot	ctWNGV->sctusrtot
#define ct_usrsi	ctWNGV->sct_usrsi
#define ct_usrsv	ctWNGV->sct_usrsv
#define ct_rstflg	ctWNGV->sct_rstflg
#define ct_shbins       ctWNGV->sct_shbins
#define ct_shbyts       ctWNGV->sct_shbyts
#define ct_trnhghmrk	ctWNGV->sct_trnhghmrk
#define ct_tryprvchk	ctWNGV->sct_tryprvchk
#define ctsuplog	ctWNGV->sctsuplog
#define ctfstr		ctWNGV->sctfstr
#define ct_chktrd       ctWNGV->sct_chktrd
#define ctlgbf          ctWNGV->sctlgbf
#define ctlstshd        ctWNGV->sctlstshd
#define ctlstsav        ctWNGV->sctlstsav
#ifdef ctLOGIDX
#define cttrnanc        ctWNGV->scttrnanc
#endif
#define ctLnum          ctWNGV->sctLnum
#define ctUnum          ctWNGV->sctUnum
#define ctSnum          ctWNGV->sctSnum
#define ctLfil          ctWNGV->sctLfil
#define ctUfil          ctWNGV->sctUfil
#define ctSfil          ctWNGV->sctSfil
#define ctdmptim        ctWNGV->sctdmptim
#define ctdlgnum        ctWNGV->sctdlgnum
#define ctelgnum        ctWNGV->sctelgnum
#define ctdlgpos        ctWNGV->sctdlgpos
#define ctelgpos        ctWNGV->sctelgpos
#define ctcpcnt         ctWNGV->sctcpcnt
#define ctcpchk         ctWNGV->sctcpchk
#define ctelcnt         ctWNGV->sctelcnt
#define ct_lstlog       ctWNGV->sct_lstlog
#define ct_lstpos       ctWNGV->sct_lstpos
#define ct_usrtr        ctWNGV->sct_usrtr
#define ct_usrtl        ctWNGV->sct_usrtl
#define ct_usrix        ctWNGV->sct_usrix
#define ct_lbfsiz       ctWNGV->sct_lbfsiz
#define ct_chkflg       ctWNGV->sct_chkflg
#define ct_logkep       ctWNGV->sct_logkep
#define ctskpfil        ctWNGV->sctskpfil
#define ctskpmir        ctWNGV->sctskpmir
#define ctfixlog        ctWNGV->sctfixlog
#define ctpdmp          ctWNGV->sctpdmp
#define ct_usrty        ctWNGV->sct_usrty
#define ctshdlfil       ctWNGV->sctshdlfil
#define ct_lgn          ctWNGV->sct_lgn
#define ctsflg          ctWNGV->sctsflg
#define ctbflg          ctWNGV->sctbflg
#define ctdflg          ctWNGV->sctdflg
#define ct_actrns       ctWNGV->sct_actrns
#define ct_lerflg       ctWNGV->sct_lerflg
#define ct_chkpt        ctWNGV->sct_chkpt
#define ctlogmem        ctWNGV->sctlogmem
#define ctlogdet        ctWNGV->sctlogdet
#define ctlogmemmax     ctWNGV->sctlogmemmax
#define ctlogmemanc     ctWNGV->sctlogmemanc
#define trnwrn		ctWNGV->strnwrn
#define prv_logser	ctWNGV->sprv_logser
#define prv_chkpos	ctWNGV->sprv_chkpos
#define prv_prvpos	ctWNGV->sprv_prvpos
#define ctlogchklmt	ctWNGV->sctlogchklmt
#define ctskpclnfil	ctWNGV->sctskpclnfil
#endif
#define ct_bfsiz      ctWNGV->sct_bfsiz
#define ctlogidxfrc   ctWNGV->sctlogidxfrc
#define ctrdmp_flg    ctWNGV->sctrdmp_flg
#define ctmdmp_flg    ctWNGV->sctmdmp_flg
#define ctrflg        ctWNGV->sctrflg
#define ctrbflg       ctWNGV->sctrbflg
#define ctfnstrat     ctWNGV->sctfnstrat
#define cttflg        ctWNGV->scttflg
#define ctfilcre      ctWNGV->sctfilcre
#define ct_mxu1       ctWNGV->sct_mxu1
#define ct_cmnown     ctWNGV->sct_cmnown
#define ct_hshft      ctWNGV->sct_hshft
#define ct_hbins      ctWNGV->sct_hbins
#define ct_dshft      ctWNGV->sct_dshft
#define ct_dbins      ctWNGV->sct_dbins
#ifdef ctLOGIDX
#define ct_tbins      ctWNGV->sct_tbins
#endif
#define ct_bavl       ctWNGV->sct_bavl
#define ct_bhla       ctWNGV->sct_bhla
#ifdef DBGhash
#define dbg_bhl_n     ctWNGV->sdbg_bhl_n
#define dbg_bhl_d     ctWNGV->sdbg_bhl_d
#endif
#define ctsdname      ctWNGV->sctsdname
#define ctsvname      ctWNGV->sctsvname
#define ctsqname      ctWNGV->sctsqname
#ifdef VINESLOG
#define ctlgname      ctWNGV->sctlgname
#endif
#define ctscommp      ctWNGV->sctscommp
#define cts_list_m    ctWNGV->scts_list_m
#define cts_sort_m    ctWNGV->scts_sort_m
#define cts_bufr_m    ctWNGV->scts_bufr_m
#define ctifl_ext     ctWNGV->sctifl_ext

#define ct_mxbuf      ctWNGV->sct_mxbuf
#define ct_dxbuf      ctWNGV->sct_dxbuf
#define ct_ndsec      ctWNGV->sct_ndsec
#define ct_maxvfil    ctWNGV->sct_maxvfil
#define ct_numvfil    ctWNGV->sct_numvfil
#define ct_avlfil     ctWNGV->sct_avlfil
#define ct_ver        ctWNGV->sct_ver
#define ct_del        ctWNGV->sct_del
#define ct_delsiz     ctWNGV->sct_delsiz
#define ctsupsep      ctWNGV->sctsupsep
#define ct_savbat     ctWNGV->ssavbat
#define ctbatnum      ctWNGV->sbatnum
#define ctbatmax      ctWNGV->sbatmax
#define ctbatch       ctWNGV->batch
#define cthstnum      ctWNGV->shstnum
#define ct_savhst     ctWNGV->ssavhst
#define cthistory     ctWNGV->history
#define cthistund     ctWNGV->histund
#define ct_npath      ctWNGV->sct_npath

#define ct_nelem      ctWNGV->sct_nelem
#define ct_tight      ctWNGV->sct_tight
#define ct_melem      ctWNGV->sct_melem
					       			
#define ct_dupkey     ctWNGV->sct_dupkey
#define spkey         ctWNGV->sspkey

#define ct_elm        ctWNGV->sct_elm
#define ct_tky        ctWNGV->sct_tky

#define ct_tkp        ctWNGV->sct_tkp
#define ct_sfxctp     ctWNGV->sct_sfxctp
#define ct_lnode      ctWNGV->sct_lnode
#define ct_fnode      ctWNGV->sct_fnode
#define ct_nwnod      ctWNGV->sct_nwnod
#define ct_gsrl       ctWNGV->sct_gsrl
#define ct_trdflg     ctWNGV->sct_trdflg
#define btlev         ctWNGV->sbtlev

#define ct_buf        ctWNGV->sct_buf

#ifdef ISAM_VARS
#define ctskymap      ctWNGV->sctskymap
#define ctis1         ctWNGV->sctis1
#define ctis2         ctWNGV->sctis2

#define ct_fndval     ctWNGV->sct_fndval
#define ct_nwrcfg     ctWNGV->sct_nwrcfg
#define ct_vfsg       ctWNGV->sct_vfsg
#define ct_ismlk      ctWNGV->sct_ismlk
#define ct_savset     ctWNGV->sct_savset

#define seqbuf	      ctWNGV->sseqbuf
#ifdef MUSTFRCE
#define seqold	      ctWNGV->sseqold
#endif
#define seqlen	      ctWNGV->sseqlen
#define seqkey	      ctWNGV->sseqkey
#define seqnum	      ctWNGV->sseqnum
#ifdef ctOldALCSET
#define maxqset	      ctWNGV->smaxqset
#endif /* ctOldALCSET */
#endif
#ifdef TRANPROC
#define ctlpathf	ctWNGV->sctlpathf
#define ctlpathm	ctWNGV->sctlpathm
#define ct_trndlt	ctWNGV->sct_trndlt
#define ct_cpdlt	ctWNGV->sct_cpdlt
#define ct_logchn	ctWNGV->sct_logchn
#define ct_loglmt	ctWNGV->sct_loglmt
#define ct_logspc	ctWNGV->sct_logspc
#define ct_logrec	ctWNGV->sct_logrec
#define ct_logtrg	ctWNGV->sct_logtrg
#define ct_blgpos	ctWNGV->sct_blgpos
#define ct_logprg	ctWNGV->sct_logprg
#define ct_lnmlog	ctWNGV->sct_lnmlog
#define ct_hnmlog	ctWNGV->sct_hnmlog
#define ct_logspr	ctWNGV->sct_logspr
#define ct_logblk	ctWNGV->sct_logblk
#define ct_rcvlst	ctWNGV->sct_rcvlst
#define ct_hdrlst	ctWNGV->sct_hdrlst
#define ct_mbrlst	ctWNGV->sct_mbrlst
#define anydelmrk	ctWNGV->sanydelmrk
#define intpthflg	ctWNGV->sintpthflg
#define ctlpathfa	ctWNGV->sctlpathfa
#define ctlpathma	ctWNGV->sctlpathma
#define undflg		ctWNGV->sundflg
#define cttopcmt	ctWNGV->scttopcmt
#define cttoplog	ctWNGV->scttoplog
#define ctbegcmt	ctWNGV->sctbegcmt
#define ctbeglog	ctWNGV->sctbeglog
#define logrem	 	ctWNGV->slogrem
#define lbfpag	 	ctWNGV->slbfpag
#define logptr	 	ctWNGV->slogptr
#define logbyt	 	ctWNGV->slogbyt
#define lowpnt	 	ctWNGV->slowpnt
#define hghcmt	 	ctWNGV->shghcmt
#define hghlog	 	ctWNGV->shghlog
#else
#define tranactv	ctWNGV->stranactv
#define transavp	ctWNGV->stransavp
#endif /* TRANPROC */
#define ctcatcnt	ctWNGV->sctcatcnt
#define ct_bsz		ctWNGV->sct_bsz
#define ct_rbuf		ctWNGV->sct_rbuf
#define rerr_cod	ctWNGV->srerr_cod
#define rerr_fil	ctWNGV->srerr_fil
#define ct_fp		ctWNGV->sct_fp
#else /* CTBOUND */
#define ctusrbuf	ctWNGV->sctusrbuf
#define ctlqmsg		ctWNGV->sctlqmsg
#define hlocl		ctWNGV->shlocl
#define hauxl		ctWNGV->shauxl
#define ctusrlen	ctWNGV->sctusrlen
#define ctusrpos	ctWNGV->sctusrpos
#define ctsrvlen	ctWNGV->sctsrvlen
#define ctsflvr		ctWNGV->sctsflvr
#define ctnodnam	ctWNGV->sctnodnam
#define ct_lq		ctWNGV->sct_lq
#define ctautotfrm	ctWNGV->sctautotfrm
#define ctmaxdatkey	ctWNGV->sctmaxdatkey
#define ctisam_flg	ctWNGV->sctisam_flg
#define ct_autopos_clr	ctWNGV->sct_autopos_clr
#define align_override	ctWNGV->salign_override
#define ctsrvver	ctWNGV->sctsrvver
#endif /* CTBOUND */

#ifdef ctCLIENT
#define ctdivs        ctWNGV->sctdivs
#endif
#define ctnusers      ctWNGV->sctnusers
#define ctops	      ctWNGV->sctops
#define ctxvlen	      ctWNGV->sctxvlen
#define sql_len       ctWNGV->ssql_len
#define cthshift      ctWNGV->scthshift
#define ctmaxparm     ctWNGV->sctmaxparm
#define uerr_cod      ctWNGV->suerr_cod
#define sysiocod      ctWNGV->ssysiocod
#define isam_err      ctWNGV->sisam_err
#define isam_fil      ctWNGV->sisam_fil
#define sql_status1   ctWNGV->ssql_status1
#define sql_status2   ctWNGV->ssql_status2
#define cndxerr       ctWNGV->scndxerr
#define ctcidxStk     ctWNGV->sctcidxStk
#define ctcidxStkPtr  ctWNGV->sctcidxStkPtr
#define ctcidxCurCmd  ctWNGV->sctcidxCurCmd
#define ctrunfil      ctWNGV->sctrunfil
#define ctrcvfil      ctWNGV->sctrcvfil
#define ct_adfil      ctWNGV->sct_adfil
#define frschgset     ctWNGV->sfrschgset
#define curchgset     ctWNGV->scurchgset
#define frschgbat     ctWNGV->sfrschgbat
#define curchgbat     ctWNGV->scurchgbat
#define ct_mxfil      ctWNGV->sct_mxfil
#define ctusrprf      ctWNGV->sctusrprf
#ifdef ISAM_VARS
#ifdef RTREE
#define srtknm        ctWNGV->ssrtknm
#define srtdat        ctWNGV->ssrtdat
#define tmpdat        ctWNGV->stmpdat
#endif
#endif
#define ct_uflvr      ctWNGV->sct_uflvr
#define ct_ualgn      ctWNGV->sct_ualgn
#define ct_upntr      ctWNGV->sct_upntr

#ifdef PROTOTYPE

#ifdef __cplusplus
extern "C" {
#endif

ctCONV pCTGVAR ctDECL GetCtreeGV( void );
ctCONV void ctDECL SetCtreeGV( pCTGVAR );
ctCONV pCTGVAR ctDECL InitCtreeGV( void );
#ifdef ctPortWINDOWSMSDOS
ctCONV BOOL ctDECL ViewCtreeError( HWND hWnd );
#endif

#ifdef __cplusplus
}
#endif

#else
ctCONV pCTGVAR ctDECL GetCtreeGV();
ctCONV void ctDECL SetCtreeGV();
ctCONV pCTGVAR ctDECL InitCtreeGV();
#ifdef ctPortWINDOWSMSDOS
ctCONV BOOL ctDECL ViewCtreeError();
#endif
#endif


#else /* ctNOGLOBALS */

#ifdef CTBOUND
EXTERN pVOID		commCmds[MAXCOMMP];
#else
EXTERN pCommFuncPtrs	commCmds[MAXCOMMP];
#endif
EXTERN pVOID		commGbls[MAXCOMMP];

#ifdef ctCLIENT
EXTERN pTEXT	ctusrbuf;
EXTERN pLQMSG	ctlqmsg;
EXTERN pLOCLFILE hlocl;
EXTERN pLOCLAUX  hauxl;
EXTERN pVOID	ctcidxStk;
EXTERN VRLEN	ctusrlen;
EXTERN VRLEN	ctusrpos;
EXTERN VRLEN	ctsrvlen;
EXTERN UTEXT	ctsflvr[8];
EXTERN TEXT	ctnodnam[IDZ];
EXTERN NINT	ct_lq;
EXTERN NINT	ctautotfrm;
EXTERN NINT	ctmaxparm;
EXTERN NINT	ctmaxdatkey;
EXTERN NINT	ctisam_flg;
EXTERN NINT	ct_autopos_clr;
EXTERN UINT	align_override;
EXTERN COUNT	ctsrvver;

EXTERN LONG	 ctnusers;	/* number of users			*/
EXTERN LONG	 ctdivs;
EXTERN LONG	 ctops;		/* set operation state bit mask		*/
EXTERN VRLEN	 ctxvlen;	/* vlen combined op max buffer length	*/
EXTERN VRLEN	 sql_len;
EXTERN NINT	 ctcidxStkPtr;
EXTERN NINT	 ctcidxCurCmd;
EXTERN NINT	 cthshift;	/* huge shift				*/
EXTERN NINT	 ctrunfil;
EXTERN NINT	 ctrcvfil;
EXTERN NINT	 ct_adfil;
EXTERN NINT	 frschgset;
EXTERN NINT	 curchgset;
EXTERN NINT	 frschgbat;
EXTERN NINT	 curchgbat;
EXTERN COUNT uerr_cod;		/* user error cod */
EXTERN COUNT sysiocod;		/* system error cod */
EXTERN COUNT isam_err;
EXTERN COUNT isam_fil;
EXTERN COUNT sql_status1;
EXTERN COUNT sql_status2;
EXTERN COUNT cndxerr;		/* conditional index error cod */
EXTERN COUNT ct_mxfil;		/* maximum files specified in intree. 	*/
EXTERN COUNT ctusrprf;		/* user profile word */
#ifdef RTREE
EXTERN COUNT	srtknm,srtdat,tmpdat;
#endif
EXTERN UTEXT 			ct_uflvr;
EXTERN UTEXT 			ct_ualgn;
EXTERN UTEXT			ct_upntr;

#else /* ctCLIENT */

EXTERN LSTANC	ctlist[NUMCTLIST];

#ifdef MULTITRD
EXTERN ppNINT	  ct_wf;
EXTERN pNINT	  ct_vf;
EXTERN pNINT	  sct_usrsi;
#ifdef ctCUSTOM
EXTERN ppVOID	  spcustmem;
EXTERN pLONG	  scustops;
#endif
#ifdef DBGhash
EXTERN SEMAmut	  dbg_hsema;
#endif
EXTERN SEMAtim	  ctdnode_sema;
EXTERN SEMA	  ctstrfil_sem;
EXTERN SEMA	  ctmemtrk_sem;
EXTERN LONG	  ctchkidle;	/* seconds between chkpnt idle test */
EXTERN LONG	  nodedtime;
EXTERN LONG	  ctlokch;	/* lock check increment		*/
EXTERN LONG	  ctlokup;	/* lock ckeck limit		*/
EXTERN LONG	  ctlokdn;	/* lock check limit		*/
EXTERN LONG	  ctloknm;	/* net locks over unlocks	*/
EXTERN NINT	  ctlokfl;	/* net locks == 0 control flag	*/
EXTERN NINT	  ct_gstnot;	/* if YES, no guest logons	*/
EXTERN UINT	  ctchktran;	/* idle chkpnt activity test	*/
EXTERN NINT	  ctmaxdatkey;
EXTERN NINT	  ctmaxkeyseg;
EXTERN NINT	  ctdnodeRunning;
EXTERN NINT	  ctqchkpRunning;
EXTERN NINT	  ctactusr;	/* number of threads active in forground */
EXTERN NINT	  ct_mq[ctMAX_MONITOR];
EXTERN NINT	  ct_smon[ctMAX_MONITOR];
EXTERN UINT	  ct_trdmon[ctMAX_MONITOR];
EXTERN COUNT	  ctadmfiles;
#else /* ~MULTITRD */
#ifndef TRANPROC
EXTERN COUNT	  tranactv;
EXTERN COUNT	  transavp;
#endif
#endif /* ~MULTITRD */
EXTERN CTFILE    ctfmru;
EXTERN pCTFILE   ct_mru;
EXTERN ppCTFILE  ctfcbhdr;
EXTERN pTREBUF	 ct_btree;	/* ptr to beg of b-tree buffers		*/
EXTERN pDATBUF	 ct_dbufr;	/* ptr to beg of data file buffers	*/
EXTERN pCTFILE	 ct_key;	/* ptr to beg of ct_key file structures	*/
EXTERN pCTFILE	 ct_dat;	/* ptr to beg of data file structures	*/
EXTERN pCTFILE	 ct_vat;	/* ptr to beg of var data file struc	*/
EXTERN pLONG	 sctshdmem;	/* shadow memory: excluding cntl structures */
EXTERN pLONG	 sctusrsum;	/* mem usage sum */
EXTERN pLONG	 sctusrtot;	/* mem usage limit */
EXTERN pTEXT	 ctsig_rdy;	/* name of exe to launch on server ready*/
EXTERN pTEXT	 ctsig_mir;	/* name of exe to launch on 1st mir trm	*/
EXTERN pTEXT	 ctsig_dwn;	/* name of exe to launch on server down	*/
EXTERN LONG	 ctactfil;
EXTERN LONG	 cttotfil;
EXTERN LONG	 cttotblk;
EXTERN LONG	 ctmemtot;	/* space aggregate limit */
EXTERN LONG	 ctmemsum;	/* current aggregate sum */
EXTERN LONG	 ctmemusr;	/* system default user limit */
EXTERN LONG	 ctmemhgh;	/* system memory highwater mark */
EXTERN LONG	 ctmemup;	/* MEMORY_MONITOR limit */
EXTERN LONG	 ctmemdn;	/* MEMORY_MONITOR limit */
EXTERN LONG	 ctmemch;	/* MEMORY_MONITOR delta */
EXTERN ULONG	 ctmemtrk_up;
EXTERN ULONG	 ctmemtrk_dn;
EXTERN LONG	 ctmemtrk_ch;
EXTERN ULONG	 ctmemtrk_net;
EXTERN VRLEN	 ct_qblock;	/* SQL record buffer block size		*/
EXTERN VRLEN	 ct_gstmem;	/* guest memory limit			*/
EXTERN LONG	 ct_timout;	/* inactive tran timeout		*/
EXTERN LONG	 ct_usrmrl;	/* user memory attribute bits		*/
extern ULONG	 ctghat[ctGhatSize];

EXTERN ULONG	 ct_dbrqs;	/* data buffer requests			*/
EXTERN ULONG	 ct_dbhit;	/* data buffer hits			*/
EXTERN ULONG	 ct_ibrqs;	/* index buffer requests		*/
EXTERN ULONG	 ct_ibhit;	/* index buffer hits			*/
EXTERN ULONG	 ct_rdops;	/* number of read operations		*/
EXTERN ULONG	 ct_rdbyt;	/* bytes read				*/
EXTERN ULONG	 ct_wrops;	/* number of write operations		*/
EXTERN ULONG	 ct_wrbyt;	/* bytes written			*/
EXTERN ULONG	 ct_rcops;	/* number of comm read operations	*/
EXTERN ULONG	 ct_rcbyt;	/* comm bytes read			*/
EXTERN ULONG	 ct_wcops;	/* number of comm write operations	*/
EXTERN ULONG	 ct_wcbyt;	/* comm bytes written			*/
EXTERN ULONG	 ct_trbeg;	/* # transaction begins			*/
EXTERN ULONG	 ct_trend;	/* # transaction ends			*/
EXTERN ULONG	 ct_trabt;	/* # transaction aborts			*/
EXTERN ULONG	 ct_trsav;	/* # transaction savepoints		*/
EXTERN ULONG	 ct_trrst;	/* # transaction restores		*/
EXTERN LONG	 ct_abnod;	/* abort list node count		*/
EXTERN LONG	 ct_cmnod;	/* pending node tran count		*/
EXTERN LONG	 ct_cmdat;	/* pending data tran count		*/
EXTERN LONG	 ct_statflg;	/* CTSTATUS flag			*/
EXTERN LONG	 ct_langflg;	/* LANGUAGE flag			*/
#ifdef ctLOGIDX
EXTERN LONG	 ct_numvi;	/* vulnerable index update count	*/
#endif
EXTERN LONG	 ctquemon;	/* node queue monitor threshhold	*/
EXTERN LONG	 ctnusers;	/* number of users			*/
EXTERN LONG	 ctstatus_size;	/* size limit on CTSTATUS.FCS		*/
EXTERN NINT	 ctstatus_purge;/* CTSTATUS.FCS purge indicator		*/
#ifdef DBG749x
EXTERN UINT	 ctlstfrq;
#endif
EXTERN UINT	 ct_tmpseq;	/* temporary file sequence number	*/
EXTERN NINT	 ctmiroff;	/* if YES, turn off all mirroring	*/
EXTERN NINT	 ctdedmon;	/* deadloack monitor flag		*/
EXTERN NINT	 ctfncmon;	/* function request monitor		*/
EXTERN NINT	 ctchkmon;	/* checkpoint monitor flag		*/
EXTERN NINT	 ct_fhlimit;	/* override file handle limits/default	*/
EXTERN NINT	 cthshift;	/* huge shift				*/
EXTERN NINT	 ct_dq;		/* shrink task logical Q handle		*/
EXTERN NINT	 ct_cq; 	/* checkpoint task logical Q handle	*/
EXTERN NINT	 ctfnz;		/* superfile member max name size	*/
#ifndef CTBOUND
EXTERN NINT	 ct_lq;		/* logon logical Q handle		*/
EXTERN pTEXT	 ct_tmppth;	/* temporary file default server path	*/
EXTERN pFILE	 ctfncfp;	/* optional file ptr for func monitor	*/
#endif
#ifdef TRANPROC
EXTERN ppTEXT	 ctlogmemanc;
EXTERN pREBLST	 ctrebhed;	/* index rebuild list */
EXTERN pREBLST	 ctcmphed;	/* compress rebuild list */
EXTERN pREBLST	 ctmirhed;	/* mirror sync list */
EXTERN pREBLST	 ctiblhed;	/* data (IFIL) rebuild list */
EXTERN LONG	 prv_logser;	/* previous checkpoint log#		*/
EXTERN LONG	 prv_chkpos;	/* previous checkpoint pos		*/
EXTERN LONG	 prv_prvpos;	/* previous checkpoint back link	*/
EXTERN LONG	 ctlogchklmt;	/* override checkpoint interval		*/
EXTERN LONG	 ct_trnhghmrk;	/* threshhold to warn tran high mark	*/
EXTERN UINT	 ct_shbins;	/* shadow hash bins			*/
EXTERN UINT	 ct_shbyts;	/* shadow hash bytes			*/
EXTERN NINT	 ct_tryprvchk;	/* try previous checkpoint		*/
EXTERN NINT	 ctsuplog;	/* suppress log flush on begin\end tran */
EXTERN NINT	 ctfstr;	/* oldest start file #			*/
EXTERN NINT	 ct_chktrd;	/* thread id of checkpoint thread	*/
#endif
EXTERN UINT	 ct_lhbins;	/* lock hash bins			*/
EXTERN UINT	 ctconbins;	/* context bins				*/
#ifdef MULTITRD
EXTERN NINT	 ct_commp;	/* # of comm protocols in use		*/
#endif
EXTERN VRLEN	 ct_bfsiz;	/* data buffer size			*/
EXTERN NINT	 ctlogidxfrc;	/* LOGIDX override: YES-on HYS-off	*/
EXTERN NINT	 ctrdmp_flg;	/* signals dynamic dump recovery	*/
EXTERN NINT	 ctmdmp_flg;	/* signals dyn dump recovery had mirrors*/
EXTERN NINT	 ctrflg;	/* automatic recovery flag		*/
EXTERN NINT	 ctlogmem;	/* preload log files to memory in rcvy	*/
EXTERN NINT	 ctlogmemmax;	/* max log files to memory in rcvy	*/
EXTERN NINT	 ctlogdet;	/* output recovery details		*/
EXTERN NINT	 ctskpclnfil;	/* if on, skip clean files in recovery	*/
EXTERN NINT	 ctfnstrat;	/* file name conversion startegy	*/
EXTERN NINT	 cttflg;	/* stop server in progress		*/
EXTERN NINT	 ctfilcre;	/* flag for create since checkpoint	*/
EXTERN COUNT	 ct_mxu1;	/* max users plus origin		*/
EXTERN NINT	 ct_cmnown;	/* common onwer number (semaphores)	*/
EXTERN UINT	 ct_hshft;	/* hash shift parameter			*/
EXTERN UINT	 ct_hbins;	/* hash bins for buffers		*/
EXTERN UINT	 ct_dshft;	/* datbuf hash shift parm		*/
EXTERN UINT	 ct_dbins;	/* datbuf hash bins			*/
#ifdef ctLOGIDX
EXTERN UINT	 ct_tbins;	/* vulnerable tran# hash bins		*/
#endif
EXTERN BHL	 ct_bavl[2];	/* buffer avl list anchors		*/
EXTERN ppBHL     ct_bhla[NUMANCHOR];
				/* buffer hash list anchors		*/
#ifdef DBGhash
EXTERN pLONG	 dbg_bhl_n;
EXTERN pLONG	 dbg_bhl_d;
#endif
EXTERN pTEXT	 ctsdname;	/* server directory name		*/
EXTERN pTEXT	 ctsvname;	/* server name				*/
EXTERN pTEXT	 ctsqname;	/* server SQL name			*/
#ifdef VINESLOG
EXTERN TEXT	 ctlgname[MAX_NAME];
				/* server status log name		*/
#endif

#ifdef ctTRAP_COMM
#ifdef MULTITRD
#ifndef CTBOUND
EXTERN ULONG	 ctTCpos;	/* TrapComm buffer pos			*/
EXTERN ULONG	 ctTCsiz;	/* TrapComm buffer size			*/
EXTERN pFILE	 ctTCfp;	/* TrapComm stream file pointer		*/
EXTERN SEMAmut	 ctTCsema;	/* TrapComm mutex			*/
EXTERN pTEXT	 ctTCbuf;	/* TrapComm buffer pointer		*/
#endif
#endif
#endif /* ctTRAP_COMM */

EXTERN pTEXT	 ctscommp[MAXCOMMP];
				/* server comm protocol			*/
				/* TrapComm mutex			*/

EXTERN pTEXT	 ct_del;	/* constant 0xff array			*/
EXTERN UINT	 ct_delsiz;	/* size of 0xff array			*/
EXTERN NINT	 ct_sqlsupr;	/* sql superfile flag			*/
EXTERN UINT	 cts_list_m;	/* list memory block size		*/
EXTERN UINT	 cts_sort_m;	/* sort memory buffer size		*/
EXTERN UINT	 cts_bufr_m;	/* bufr memory block size		*/
EXTERN TEXT	 ctifl_ext[2][EXZ];
				/* default IFIL extensions		*/
#ifdef MULTITRD
EXTERN UTEXT	 ct_sflvr;	/* system flavor			*/
EXTERN UTEXT	 ct_salgn;	/* system alignment			*/
EXTERN UTEXT	 ct_spntr;	/* system pointer size			*/
#endif

EXTERN UINT  ct_mxbuf;		/* maximum buffers specified in intree 	*/
EXTERN UINT  ct_dxbuf;		/* maximum data file buffers 		*/
#ifndef ctGINIT
extern NINT  ct_maxvfil;	/* maximum virtual files opened		*/
#endif
EXTERN NINT  ctrunfil;
EXTERN NINT  ctrcvfil;
EXTERN NINT  ct_adfil;
EXTERN NINT  ct_numvfil;	/* number of virtual files open		*/
EXTERN NINT  ct_avlfil;		/* available file control block counter */
EXTERN COUNT ct_mxfil;		/* maximum files specified in intree. 	*/
EXTERN COUNT ct_ndsec;		/* # of sectors per node specified in intree */
EXTERN COUNT ct_ver;		/* defines configuration options */
EXTERN TEXT  ctsupsep;
#ifdef MULTITRD
EXTERN SEMA ct_ismema;		/* ctismem semaphore */
EXTERN SEMA ct_srvsema;
EXTERN SEMA ct_vtsema;		/* vtclose semaphore */
EXTERN SEMA ct_nmvsema;		/* ct_numvfil sema */
EXTERN SEMA ct_avlsema;		/* available file counter */
EXTERN SEMA ct_lfwsema;		/* wait for collective log flush */
EXTERN SEMA ct_chksema;		/* checkpoint semaphore */
EXTERN SEMA ct_abtsema;		/* node abort list semaphore */
EXTERN SEMA ct_comsema;		/* buffer commit list semaphore */
EXTERN SEMA ct_uhnsema;		/* user handle semaphore */
EXTERN SEMAmut ct_gvsema;	/* ctgv semaphore */
EXTERN SEMAmut ct_logsema; 	/* log position/buffer semaphore */
EXTERN SEMA    ct_lflsema;	/* log file i/o semaphore */
EXTERN SEMAmut ct_gnsema;	/* get node (getnod) semaphore */
EXTERN SEMAmut ct_ocsema;	/* open / create semaphore */
EXTERN SEMAmut ct_llsema;	/* lock list semaphore */
EXTERN SEMAmut ct_facsema;	/* file lru access count */
EXTERN SEMAmut ct_utrsema; 	/* ct_usrtr[] semaphore */
EXTERN SEMAmut ct_memsema; 	/* memory allocation semaphore */
EXTERN SEMAmut ct_concsema; 	/* multi-procesor concurrency */
EXTERN COUNT ct_numusr;		/* active users */
#endif
#ifdef ctSRVR
EXTERN SEMAmut ct_MPsema;	/* multi-processor mutex */
#endif

#ifdef GNSERVER
EXTERN pDEDLOK ctdedlok;	/* pointer to deadlock detection array */
#endif

#ifdef MULTITRD

#ifdef ctSTATIC_SEMA
EXTERN SEMA ctsresema[MXU2];
#endif

typedef struct {
	NINT	 slOWNR;	/* self-referencing thread handle (OWNER) */
#ifdef ctCREATE_SEMA
	SEMA	 sresema;	/* reuseable semaphore for temp CTFILE	*/
#endif
	BATHDR	 batch;
#ifdef ctOldALCBAT
	COUNT	 sbatmax;
	COUNT	 sbatnum;
	pBATHDR	 ssavbat;
#else
	pSAVBAT	 sbatnum;
	ppSAVBAT ssavbat;
#endif
	HSTHDR	 history;
	pHSTLST	 histund;
	pSAVHST	 shstnum;
	ppSAVHST ssavhst;
	pVOID	 sctcidxStk;
#ifdef rtSRVR
	pVOID	 srtinst;	/* r-tree instance (rtGVAR)		*/
#endif

	LONG  sctops;		/* set operation state bit mask		*/
	VRLEN sctxvlen;		/* vlen combined op max buffer length	*/
	LONG  sabnlog;		/* begtran log for abandoned tran	*/
	LONG  sresrv1;		/* spare members */
	LONG  sresrv2;
	LONG  sresrv3;

#ifndef CTBOUND
	pVOID suserlqp;		/* user's request block */
	LONG  sresrv4;		/* spare members */
	LONG  sresrv5;
	LONG  sctops_clr;	/* operation state clear mask		*/
	LONG  sautopos;		/* auto current ISAM position */
	LONG  sautolen;		/* auto variable length size */
	LONG  slogtime;		/* logon time */
	LONG  strntime;		/* last begin tran time */
	LONG  srqstime;		/* last request time */
	LONG  srqsfunc;		/* last rquest function # */
	LONG  susrrsvd[6];	/* spare members */
	NINT  snbrfile;		/* number of files in use */
	NINT  sactflag;		/* active / inactive flag */
	TEXT  snodname[32];	/* node name */
#endif

	LONG  sfncret;		/* special function ret values: only MULTITRD */
	LONG  sct_npath[MAXLEV];/* array to trace path down B-tree. Assumes  */
				/* maximum of MAXLEV - 1 levels in B-tree.   */
	LONG  sct_lnode;	/* last node found during walk down tree */
	LONG  sct_fnode;	/* node found during search/retrieval    */
	LONG  sct_nwnod;	/* pointer to new node */
	LONG  sct_gsrl;		/* next serial number for data file */
	NINT  sct_trdflg;	/* thread operation flag */
	NINT  sctcidxStkPtr;
	NINT  sctcidxCurCmd;
	NINT  sabnflg;		/* counts stage of abandon tran processing */
	COUNT sctconid;
	TEXT  sct_userid[IDZ];	/* unique user id */
	TEXT  spare[PWZ];	/* unused ???? */
	UGRUP sct_userg[MAX_UGRUP];
				/* group list header */
	pFUSR sct_fup;		/* FUSR pointer for current file */
	ppctICON  sctconanc;	/* hash bins for context */
#ifdef DBGtree
	pVOID	  sct_walkh;	/* tree-walk degug list head */
	pVOID	  sct_walkt;	/* tree-walk degug list tail */
	NINT	  sctlflg;	/* load key flag	     */
#endif /* DBGtree */
#ifdef TRANPROC
	ppSHADLST sct_ubit;	/* hash bins for srchshd */
#ifdef DBGhash
	pLONG     sdbg_shsh_u;
#endif
	pTEXT sctmrklst;	/* temp storage for exc marks during split */
	pTEXT sct_usrsp;	/* pointer to shadow swap buffer */
	pSHADLST sct_usrsl;	/* current shadow list element */
	VRLEN sct_usrsz;	/* swap buffer size */
	LONG  sct_usrtm;	/* activity time stamp (seconds) */
	LONG  gvresv2;
	NINT  sct_usrsv;	/* tran savepoint info */
	NINT  sct_rstflg;	/* TRANRST() flag */
#else
	COUNT stranactv;
	COUNT stransavp;
#endif
	NINT  sctrbflg;		/* rebuild flag */
	NINT  swhdrflg;		/* write header flag */
	ppLOKS sct_locks;	/* user lock table head */
	ppLOKS sct_ltail;	/* user lock table tail */
#ifdef DBGhash
	pLONG  sdbg_lhsh_u;
#endif
	COUNT sctusrprf;	/* user profile word */
	COUNT sctumxfil;	/* max files for user */
	COUNT scthghfil;	/* highest file number limit */
	NINT  sct_nelem[MAXLEV];
	NINT  sct_tight[MAXLEV];
	NINT  sct_melem[MAXLEV];
	NINT  sct_tkp;		/* previous value of ct_tky		*/
	NINT  sct_sfxctp;	/* previous value of suffix count	*/
	NINT  sbtlev;		/* b-tree level counter. used as index of */
				/* ct_npath				  */
	NINT  sct_elm;		/* position within B-Tree node	*/
	NINT  sct_tky;		/* result of comparison between target	*/
				/* value and index entry.		*/
				/* ct_tky < 0 => target < index entry	*/
				/*	  = 0 => target = index entry	*/
				/*	  > 0 => target > index entry 	*/
					
	TEXT sct_dupkey[MAXLEN+1];/* for searches of duplicate keys */
	TEXT sspkey[MAXLEN+1];	/* temporary storage for key values during */
				/* node updates 			   */

	TEXT sct_buf[CTBUFSIZ];	/* temporary io buffer */
	UTEXT sct_uflvr;	/* user flavor */
	UTEXT sct_ualgn;	/* user alignment */
	UTEXT sct_upntr;	/* user pointer size */

	TEXT sl_lastkey[MAXLEN];
	LONG sl_begnod;
	LONG sl_curnod;
	LONG sl_prvnod;
	LONG sl_numnod;
	LONG sl_ct_nnods[MAXLEV];
	NINT sl_elem;
	NINT sl_started;
	pTREBUF sl_savbp;
	VRLEN sl_hdradj;

	pTEXT sb_ct_rbuf;
	pctRECPT sb_ct_pbuf;
	COUNT sb_rerr_cod;
	COUNT sb_rerr_fil;
	COUNT sb_ct_fp;
	COUNT sb_ct_redom;
	VRLEN sb_ct_bsz;
	LONG sb_ct_spc;

	pTEXT	sql_tbuf;
	pTEXT	sql_spos;
	LONG	 sql_olen;
	LONG	 sql_rlen;
	pVOID	 sql_db;
	} CTGV;
typedef CTGV ctMEM *	pCTGV;

typedef struct {
	pCTIS1  s1;
	ppCTIS2 s2;
	ppCOUNT sct_kymap;
#ifdef ctOldALCSET
	pSAVSET sct_savset;

	COUNT   sseqlen;		/* significant length of key	*/
	COUNT   sseqkey;		/* current keyno		*/
	COUNT   sseqnum;		/* current set number		*/
	COUNT   smaxqset;
#else
	ppSAVSET
		sct_savset;		/* pointer to hash anchors	*/
	pSAVSET	sseqnum;		/* current set pointer		*/
	COUNT   sseqlen;		/* significant length of key	*/
	COUNT   sseqkey;		/* current keyno		*/
#endif
	COUNT	sct_nwrcfg;
	COUNT	sct_vfsg;
	COUNT	sct_ismlk;
#ifdef RTREE
	COUNT	ssrtknm;
	COUNT	ssrtdat;
	COUNT	stmpdat;
#endif

	TEXT    sseqbuf[MAXLEN];	/* holds matching "partial" key */
	TEXT    sseqold[MAXLEN];	/* holds previous "partial" key */
	TEXT	sct_fndval[MAXLEN];
	} CTIS;
typedef CTIS ctMEM *	pCTIS;
typedef CTIS ctMEM * ctMEM * ppCTIS;

#ifdef CTBOUND
EXTERN pVOID   ctThrdUserVar[MXU2];
#else
EXTERN pTEXT   sctusrbuf[MXU2]; /* user commbuffer */
EXTERN VRLEN   sctusrlen[MXU2]; /* commbuffer length */
EXTERN VRLEN   sctusrpos[MXU2]; /* position in buffer */
EXTERN NINT    sctclnver[MXU2]; /* client version from pb.pbvr */
#endif

EXTERN CTSTATV ctstatv[MXU2];
EXTERN VRLEN   ssql_len[MXU2];
#ifdef VINES
#ifdef RUNTCBTABLE
EXTERN NINT    ctusrmapId;
EXTERN taskid  ctusrhmap[MXU2];
#else
EXTERN NINT    ctusrhmap[MXU2];
#endif
#else
EXTERN NINT    ctusrhmap[MXU2];
#endif

EXTERN NINT	ctua[MXU2];
EXTERN NINT	ctma[MXU2];
#ifdef CTBOUND
EXTERN NINT	ctst[MXU2];
#endif

EXTERN pCTGV   ctgv[MXU2];

#ifdef CTTHRDH
#ifdef MULTITRD
EXTERN NLONG   ctrcparm[MXU2];
#endif
#endif

EXTERN ppCTIS  ctis;

EXTERN ppFUSR  ct_fusr;
EXTERN pBLKLST ct_blk;

#ifdef DBGtree
#define ct_walkh	lctgv->sct_walkh
#define ct_walkt	lctgv->sct_walkt
#define ctlflg		lctgv->sctlflg
#endif /* DBGtree */
#define ct_ubit(owner)	ctgv[owner]->sct_ubit
#ifdef DBGhash
#define dbg_shsh_u(owner) ctgv[owner]->sdbg_shsh_u
#endif
#define ctconanc	lctgv->sctconanc
#define ctconid		lctgv->sctconid
#define ct_rstflg	lctgv->sct_rstflg
#define tranactv	lctgv->stranactv
#define transavp	lctgv->stransavp
#define abnlog		lctgv->sabnlog
#ifndef CTBOUND
#define ctusrbuf	sctusrbuf[sOWNR]
#define ctusrlen	sctusrlen[sOWNR]
#define ctusrpos	sctusrpos[sOWNR]
#define ctclnver	sctclnver[sOWNR]
#define ctops_clr	lctgv->sctops_clr
#endif
#ifdef ctCREATE_SEMA
#define resema		lctgv->sresema
#endif
#ifdef ctSTATIC_SEMA
#define resema		ctsresema[sOWNR]
#endif
#define lOWNER		lctgv->slOWNR
#define ctbatch		lctgv->batch
#define ctbatnum	lctgv->sbatnum
#define ctbatmax	lctgv->sbatmax
#define ct_savbat	lctgv->ssavbat
#define cthistory	lctgv->history
#define cthistund	lctgv->histund
#define cthstnum	lctgv->shstnum
#define ct_savhst	lctgv->ssavhst
#define ct_locks	lctgv->sct_locks
#define ct_ltail	lctgv->sct_ltail
#ifdef DBGhash
#define dbg_lhsh_u	lctgv->sdbg_lhsh_u
#endif
#define ctusrprf	lctgv->sctusrprf
#define ctumxfil	lctgv->sctumxfil
#define cthghfil	lctgv->scthghfil
#define ctops		lctgv->sctops
#define ctxvlen		lctgv->sctxvlen
#define ctcidxStk	lctgv->sctcidxStk
#define ctcidxStkPtr	lctgv->sctcidxStkPtr
#define ctcidxCurCmd	lctgv->sctcidxCurCmd
#define autopos		lctgv->sautopos
#define autolen		lctgv->sautolen
#define logtime		lctgv->slogtime
#define trntime		lctgv->strntime
#define rqstime		lctgv->srqstime
#define rqsfunc		lctgv->srqsfunc
#define userlqp		lctgv->suserlqp
#define nodname		lctgv->snodname
#define nbrfile		lctgv->snbrfile
#define actflag		lctgv->sactflag
#define cominfo		lctgv->scominfo
#define fncret		lctgv->sfncret
#define ct_npath	lctgv->sct_npath
#define ct_nelem	lctgv->sct_nelem
#define ct_tight	lctgv->sct_tight
#define ct_melem	lctgv->sct_melem
#define ct_usrsi	sct_usrsi[sOWNR]
#ifdef ctCUSTOM
#define pcustmem	spcustmem[sOWNR]
#define custops		scustops[sOWNR]
#endif
#define ctusrsum	sctusrsum[sOWNR]
#define ctusrtot	sctusrtot[sOWNR]
#ifdef TRANPROC
#define ctmrklst	lctgv->sctmrklst
#define ctshdmem	sctshdmem[sOWNR]
#define ct_usrsv	lctgv->sct_usrsv
#define ct_usrsp	lctgv->sct_usrsp
#define ct_usrsl	lctgv->sct_usrsl
#define ct_usrsz	lctgv->sct_usrsz
#define ct_usrtm	lctgv->sct_usrtm
#endif
#define whdrflg		lctgv->swhdrflg
#define ctrbflg		lctgv->sctrbflg
#define ct_trdflg	lctgv->sct_trdflg
#define ct_userid	lctgv->sct_userid
#define ct_sqlpwd	lctgv->sct_sqlpwd
#define ct_userg	lctgv->sct_userg
#define ct_fup		lctgv->sct_fup
#define ct_dupkey	lctgv->sct_dupkey
#define spkey		lctgv->sspkey
#define ct_elm		lctgv->sct_elm
#define ct_tky		lctgv->sct_tky
#define ct_tkp		lctgv->sct_tkp
#define ct_sfxctp	lctgv->sct_sfxctp
#define ct_lnode	lctgv->sct_lnode
#define ct_fnode	lctgv->sct_fnode
#define ct_nwnod	lctgv->sct_nwnod
#define btlev		lctgv->sbtlev
#ifdef CTPERM
#define uerr_cod	ctstatv[sOWNR].uec
#define sysiocod	ctstatv[sOWNR].sec
#define isam_err	ctstatv[sOWNR].iec
#define isam_fil	ctstatv[sOWNR].ifl
#define cndxerr		ctstatv[sOWNR].cec
#else
#define uerr_cod	ctstatv[OWNER].uec
#define sysiocod	ctstatv[OWNER].sec
#define isam_err	ctstatv[OWNER].iec
#define isam_fil	ctstatv[OWNER].ifl
#define cndxerr		ctstatv[OWNER].cec
#endif
#define sql_len		ssql_len[sOWNR]
#define sql_status1	ctstatv[sOWNR].ql1
#define sql_status2	ctstatv[sOWNR].ql2
#define ct_uflvr	lctgv->sct_uflvr
#define ct_ualgn	lctgv->sct_ualgn
#define ct_upntr	lctgv->sct_upntr
#define ct_buf		lctgv->sct_buf
#define ct_gsrl		lctgv->sct_gsrl
#define ct_rbuf		lctgv->sb_ct_rbuf
#define ct_pbuf		lctgv->sb_ct_pbuf
#define rerr_cod	lctgv->sb_rerr_cod
#define rerr_fil	lctgv->sb_rerr_fil
#define ct_fp		lctgv->sb_ct_fp
#define ct_redom	lctgv->sb_ct_redom
#define ct_bsz		lctgv->sb_ct_bsz
#define ct_spc		lctgv->sb_ct_spc

#else /* MULTITRD */

EXTERN BATHDR  ctbatch;
EXTERN LONG    ct_spc;
EXTERN LONG    ct_npath[MAXLEV];/* array to trace path down B-tree. Assumes  */
				/* maximum of MAXLEV - 1 levels in B-tree.   */
#ifdef ctOldALCBAT
EXTERN COUNT   ctbatnum;
EXTERN COUNT   ctbatmax;
EXTERN pBATHDR ct_savbat;
#else
EXTERN pSAVBAT ctbatnum;
EXTERN ppSAVBAT
	       ct_savbat;
#endif
EXTERN HSTHDR	cthistory;
EXTERN pHSTLST	cthistund;
EXTERN pSAVHST	cthstnum;
EXTERN ppSAVHST	ct_savhst;
EXTERN pVOID	ctcidxStk;
EXTERN NINT  ct_nelem[MAXLEV];
EXTERN NINT  ct_tight[MAXLEV];
EXTERN NINT  ct_melem[MAXLEV];
EXTERN NINT  ctcidxStkPtr;
EXTERN NINT  ctcidxCurCmd;
EXTERN COUNT ctconid;
EXTERN ppctICON
	     ctconanc;	/* hash bins for context */
#ifdef DBGtree
EXTERN pVOID ct_walkh;
EXTERN pVOID ct_walkt;
EXTERN NINT  ctlflg;
#endif /* DBGtree */
#ifdef TRANPROC
#define ct_ubit(owner)	sct_ubit
EXTERN ppSHADLST sct_ubit;	/* hash bins for srchshd */
#ifdef DBGhash
#define dbg_shsh_u(owner)	sdbg_shsh_u
EXTERN pLONG sdbg_shsh_u;
#endif
EXTERN pTEXT ctmrklst;
EXTERN pTEXT ct_usrsp;
EXTERN pSHADLST ct_usrsl;
EXTERN VRLEN ct_usrsz;
EXTERN LONG  ctshdmem;
EXTERN LONG  ctusrsum;
EXTERN LONG  ctusrtot;
EXTERN NINT  ct_usrsi;
EXTERN NINT  ct_usrsv;
EXTERN NINT  ct_rstflg;		/* TRANRST() flag */
#endif
EXTERN NINT  ctrbflg;
EXTERN ppLOKS ct_locks;
EXTERN ppLOKS ct_ltail;
#ifdef DBGhash
EXTERN pLONG  dbg_lhsh_u;
#endif
#ifdef ctCUSTOM
EXTERN pVOID  pcustmem;
EXTERN LONG   custops;
#endif
EXTERN LONG   ctops;		/* set operation state bit mask		*/
EXTERN VRLEN  ctxvlen;		/* vlen combined op max buffer length	*/
					
EXTERN TEXT ct_dupkey[MAXLEN+1];/* for searches of duplicate keys */
EXTERN TEXT spkey[MAXLEN+1];	/* temporary storage for key values during */
				/* node updates 			   */

EXTERN COUNT ctusrprf;		/* user profile word */
EXTERN NINT  ct_elm;		/* position within B-Tree node	*/
EXTERN NINT  ct_tky;		/* result of comparison between target	*/
				/* value and index entry.		*/
				/* ct_tky < 0 => target < index entry	*/
				/*	  = 0 => target = index entry	*/
				/*	  > 0 => target > index entry 	*/
EXTERN NINT  ct_tkp;		/* previous value of ct_tky		*/
EXTERN NINT  ct_sfxctp;		/* previous value of suffix count	*/
EXTERN LONG  ct_lnode;		/* last node found during walk down tree */
EXTERN LONG  ct_fnode;		/* node found during search/retrieval    */
EXTERN LONG  ct_nwnod;		/* pointer to new node */
EXTERN LONG  ct_gsrl;		/* next serial number for data file */
EXTERN NINT  ct_trdflg;		/* thread operation flag */
EXTERN NINT  btlev;		/* b-tree level counter. used as index of */
				/* ct_npath				  */
EXTERN COUNT uerr_cod;		/* user error cod */
EXTERN COUNT sysiocod;		/* system error cod */
EXTERN COUNT isam_err;
EXTERN COUNT isam_fil;
EXTERN VRLEN sql_len;
EXTERN COUNT sql_status1;
EXTERN COUNT sql_status2;
EXTERN COUNT cndxerr;		/* conditional index error cod */
EXTERN UTEXT ct_uflvr;
EXTERN UTEXT ct_ualgn;
EXTERN UTEXT ct_upntr;
EXTERN TEXT  ct_buf[CTBUFSIZ];	/* temporary io buffer */

#define ctumxfil	ct_mxfil
#define cthghfil	ct_mxfil

#endif

EXTERN UINT	ctlog9477;

#endif /* ctCLIENT */
#endif /* ctNOGLOBALS */

#include "ctfunp.h"
#endif /* ctGVARH */

/* end of ctgvar.h  */
