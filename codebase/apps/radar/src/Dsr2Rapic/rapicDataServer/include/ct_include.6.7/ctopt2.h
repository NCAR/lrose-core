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

#ifndef ctOPT2H
#define ctOPT2H

/************************************/
#ifndef LOW_HIGH
#ifndef HIGH_LOW
One of these must be defined in ctcmpl.h 
#endif
#endif

#ifndef PERC_H
#ifndef PERC_D
#ifndef PERC_HD
One of these must be defined in ctcmpl.h 
#endif
#endif
#endif
/************************************/

/************************************/
/* Server Basic Defaults 			*/
#ifdef ctSRVR
#define ctPortDAC 		/* DAC SQL BCD Support */
/*-------------------*/
#ifdef CTBOUND
#undef CTBOUND
#endif

#ifndef NO_BOUND
#define NO_BOUND
#endif
/*-------------------*/

/*-------------------*/
#ifdef ctNOGLOBALS
#undef ctNOGLOBALS
#endif

#ifndef ctYESGLOBALS
#define ctYESGLOBALS
#endif
/*-------------------*/

#endif /* ctSRVR */
/************************************/


/************************************/
/* c-tree Plus Basic Defaults 		*/
#ifndef ctYESGLOBALS
#ifndef MULTITRD
#ifndef ctNOGLOBALS
#define ctNOGLOBALS
#endif
#endif
#endif

#ifndef NO_RESOURCE
#ifndef RESOURCE
#define RESOURCE
#endif
#endif

#ifndef NO_BATCH
#ifndef CTBATCH
#define CTBATCH
#endif
#endif

#ifndef NO_SUPER
#ifndef CTSUPER
#define CTSUPER
#endif
#endif

#ifndef NO_VARLD
#ifndef VARLDATA
#define VARLDATA
#endif
#endif

#ifndef NO_VARLK
#ifndef VARLKEYS
#define VARLKEYS
#endif
#endif

#ifndef NO_PARMF
#ifndef PARMFILE
#define PARMFILE
#endif
#endif

#ifndef NO_RTREE
#ifndef RTREE
#define RTREE
#endif
#endif

#ifndef NO_ISAM
#ifndef CTS_ISAM
#define CTS_ISAM
#endif
#endif

#ifndef ctPASCALst 
#define ctPASCALst 
#endif

#ifndef NO_CONTEXT
#ifndef ctICONTEXT
#define ctICONTEXT
#endif
#endif

#ifndef UNIFRMAT
#ifndef NO_UNIFM
#define NO_UNIFM
#endif
#endif

#ifndef NO_CONDIDX
#ifndef ctCONDIDX
#define ctCONDIDX	/* enable conditional index support */
#endif
#endif

#ifndef NO_HISTORY
#ifdef  TRANPROC
#ifndef CTHISTORY
#define CTHISTORY	/* enables transaction history */
#endif
#endif
#endif

/* end c-tree Plus basic defaults 	*/
/************************************/

/**********************************************/
/* Other m-tree high level define adjustments */
#ifdef 	ctMTCLIENT /* Multi-Threaded Client */
#ifndef ctThrdApp
#define ctThrdApp
#endif
#endif

#ifdef 	ctMTFPG /* Multi-Threaded FPUTFGET */
#ifndef CTBOUND
#define CTBOUND
#endif
#ifdef NO_BOUND
#undef NO_BOUND
#endif
#ifndef ctThrdFPG
#define ctThrdFPG
#endif
#define ctdefer	ctThrdSleep
#endif
/**********************************************/

/**********************************************/
/* BEHAVIOR Options */

/*--------------------------------------------------------------*/
/* ctBEHAV_SCHSEG maintains the padding behavior of SCHSEG and	*/
/* USCHSEG for CT_FPSTRING, CT_F2STRING and CT_F4STRING. The	*/
/* documentation specifies that NO padding will occur at the	*/
/* end of these three types of fields when used with SCHSEG and */
/* USCHSEG, but padding IS occuring. For example, if a		*/
/* CT_FPSTRING field with a length byte of 6 is used for an 8	*/
/* byte key segment, then c-tree mistakenly adds two bytes of	*/
/* padding (if SCHSEG or USCHSEG is used).			*/
/*								*/
/* DEFAULT: define ctBEHAV_SCHSEG. If ctBEHAV_SCHSEG is NOT	*/
/* defined, then no padding is added consistent with the	*/
/* documentation.						*/
/*--------------------------------------------------------------*/
#define ctBEHAV_SCHSEG

/* END BEHAVIOR Options */
/**********************************************/

/**********************************************/
/* DEBUG Options */

/*--------------------------------------------------------------*/
/* ctWATCH_THRD_START_STOP activates logic to call the 		*/
/* ctWatchThrdStartStop function at three locations: 		*/
/* 1) After successfull cttcre call (thread launched).		*/
/* 2) AT top of launched function (thread began to execute). 	*/
/* 3) From ctehdthread (thread ended). 				*/
/* The ctWatchThrdStartStop function records these events and 	*/
/* also provides a mode to display current thread activity.	*/
/*--------------------------------------------------------------*/
/* #define ctWATCH_THRD_START_STOP */

/*----------------------------------------------------------------------*/
/* CT_DIAGNOSE_LOGON_COMM activates client side debug output		*/
/* similar to the server's DIAGNOSTICS LOGON_COMM keyword.		*/
/* It provides communications info during the initial login process.	*/
/*----------------------------------------------------------------------*/
/* #define CT_DIAGNOSE_LOGON_COMM */

/*----------------------------------------------------------------------*/
/* DBGmemleak activates debug output of memory alloc's and free's	*/
/*----------------------------------------------------------------------*/
/* #define DBGmemleak */

/*----------------------------------------------------------------------*/
/* DBGhash activates debug output of hash bin efficiency		*/
/*----------------------------------------------------------------------*/
/* #define DBGhash */

/* END DEBUG Options */
/**********************************************/

/* 
ctTPND_ERR_RETRY: permit compile-time control for automatic retry of
key updates with TPND_ERR. The value is the number of retrys to
attempt before returning the TPND_ERR. Suggested values for this
value are between three and ten. The retry logic only applies to the
server, and a ten millisecond delay is introduced before each retry.
*/
#define ctTPND_ERR_RETRY 6

#define ctLOGIDX   	/* enables high speed index recovery file mode option */
#define ctDBEGFLSH	/* enables deferred begin tran flushing */

#ifdef ctPortBEGO
#define ALT_KEY_LENGTH		13
#define PUNC_STRIP_LENGTH	34
#endif /* ctPortBEGO */

#define ctTRAP_COMM	/*
			** Add communications trapping logic at compile.
			** Will not be effective unless DIAGNOSTICS TRAP_COMM
			** is added to server configuration file.
			*/

/**************/
#ifdef MULTITRD
#ifdef CTBOUND
#define ctThrdBnd
#define ctThrds
#endif
#endif
/**************/

/**************/
#ifdef ctThrdApp
#define ctThrds

#ifndef ctNOGLOBALS
#define ctNOGLOBALS
#endif

#ifndef OWNER
#define OWNER	ctOWNER()
#define ctThrdRet
#endif

#endif /* ctThrdApp */
/**************/
#ifdef ctThrdFPG
#define ctThrds

#ifndef ctNOGLOBALS
#define ctNOGLOBALS
#endif

#ifndef OWNER
#define OWNER	ctOWNER()
#define ctThrdRet
#endif

#endif /* ctThrdFPG */
/**************/

/**************/
#ifdef MULTITRD
#ifndef CTBOUND
#ifndef ctSRVR
#define ctSRVR
#endif
#endif
#endif
/**************/

/**********************************************/
#ifdef ctSRVR		/* Server default settings */

#ifndef ctMIRROR
#define ctMIRROR	/* enable file mirrors */
#endif

#ifndef ctCONDIDX
#define ctCONDIDX	/* enable conditional index support */
#endif

#ifndef CTVERBOSE
#define CTVERBOSE
#endif

#endif				/* ctSRVR default settings */
/**********************************************/

/**********************************************/
#ifdef ctBNDSRVR

#ifndef NO_MIRROR
#ifndef ctMIRROR
#define ctMIRROR
#endif
#endif

#ifndef NO_CONDIDX
#ifndef ctCONDIDX
#define ctCONDIDX	/* enable conditional index support */
#endif
#endif

#endif				/* ctBNDSRVR default settings */
/**********************************************/

#ifndef CTBOUND
#ifndef ctSRVR
#ifndef	ctCLIENT
#define ctCLIENT
#endif
#endif
#endif

#ifndef ctSRVR
#ifdef ctSYNC_DELAY
#undef ctSYNC_DELAY
#endif
#endif

#ifdef ctPORTAUTOWIN
#ifndef ctPREFNC
#define ctPREFNC	ct_win_inst
#endif
#define ctAutoID	GetCurrentTask()
#define ctAutoInst
#else /* ctPORTAUTOWIN */
#ifdef ctThrdFPG
#ifndef ctPREFNC
#define ctPREFNC	ctserl
#endif
#ifndef ctPOSTFNC
#define ctPOSTFNC	ctunserl
#endif
#ifdef ctThrdRet
#define ctAutoID	ctThrdID()
#else
#define ctAutoID	OWNER
#endif
#define ctAutoInst
#endif /* ctThrdFPG */
#endif /* ctPORTAUTOWIN */

/*
** ctPREFNC => calling the function ctPREFNC before each c-tree
** function
**
** ctFRCSNG => force single entry point calls
*/

/*
** PREFNC requires either LOCLIB or FRCSNG or CLIENT. If none, select FRCSNG.
*/
#ifdef ctPREFNC
#ifndef ctLOCLIB
#ifndef ctCLIENT
#ifndef ctFRCSNG
#define ctFRCSNG
#endif
#endif
#endif
#endif

/*
** You cannot configure both LOCLIB and FRCSNG. If so, only select LOCLIB.
*/
#ifdef ctLOCLIB
#ifdef ctFRCSNG
#undef ctFRCSNG
#endif
#endif

/*
** FRCSNG uses LOCLIB features, so c-tree must turn on LOCLIB if you select
** FRCSNG. This is not inconsistent with preceding undef since from this point
** forward, ctFRCSNG and ctLOCLIB both being defined is interpreted as
** forcing a single entry point without an actual LOCLIB implementation.
*/
#ifdef ctFRCSNG
#define ctLOCLIB
#endif


#ifdef VINES
/* added for the definintion of size_t */
#include <sys/types.h>
#endif

/* ***			    Capacity Constants			    *** */

#define BFRSIZ		2048	/* default data file buffer size	*/
#define DATABUFS	100	/* default # of data buffers		*/
#define MAXLEN		255	/* maximum key length			*/
#define MAX_NAME	255	/* maximum file name length		*/
#define RESNAME_LEN	64	/* resource logical name limit		*/
#ifndef MAXVFIL
#define	MAXVFIL		500	/* maximum virtual files open at same time */
#endif

/* ***			Variable Length Key & Field Constants	    *** */

#define PADDING		' '	/* trailing key padding			*/
#define FIELD_DELIM	'\0'	/* var length field delimiter		*/

/* ***			SUPERFILE NAME DELIMITER		    *** */
#define CTSUPSEP	'!'

/* ***			   MIRROR NAME DELIMITER		    *** */
#define ctMIRROR_SEPchr	'|'
#define ctMIRROR_SEPstr	"|"

/* ***			  PATH SEPARATOR DEFAULT		    *** */
#ifndef ctPATH_SEP
#define ctPATH_SEP	'?'	/* ctcmpl.h should define ctPATH_SEP	*/
#endif

/* ***			  STATUS LOG NAME		    	    *** */
#ifndef CTSTATUSname
#define CTSTATUSname	"CTSTATUS.FCS"
#endif

/*
 * The following capacity constants are used only by the ISAM routines and
 * the REBUILD routines.
 */

#ifdef ctMAXFIL
#define MAXFIL		ctMAXFIL
#else
/* maximum number of files (non-server) - may be overridden in ctoptn.h 	*/
#define MAXFIL		110	
#endif
#ifdef MULTITRD
#define MAX_DAT_KEY	ctmaxdatkey
#define ctMAX_DAT_KEY	32
#else
#ifdef ctMAX_DAT_KEY
#define MAX_DAT_KEY	ctMAX_DAT_KEY
#else
#define MAX_DAT_KEY	32	/* max number of indices per data file	*/
#endif
#endif

/* ************* ************* ************* ************ ************* */
#define ctHLflavor	1
#define ctLHflavor	2

#define ctBYTEORDER	0
#define ctALIGNMENT	1
#define ctPNTRSIZE	2

#ifndef ctrt_bstrlen
#define ctrt_bstrlen	strlen
#endif

#ifdef ctEVALUATION
#define ctEVALdata	 65311
#define ctEVALlog	500008
#define EVAL_ERR	   649
#ifdef ctMIRROR
#undef ctMIRROR
#endif
#endif

#ifdef ctMIRROR
#ifdef ctMIRRORsave
#define ctsave		mbsave
#endif
#else
#define ctopen(c,m)	mbopen(c,c->flname,m)
#define ctsave		mbsave
#endif

#ifdef PROTOTYPE
typedef NINT (*pctFUNC)(pVOID );
#define ctProtoVOID VOID
#else
typedef NINT (*pctFUNC)();
#define ctProtoVOID /* */
#endif

#ifdef MULTITRD
#ifndef ctSEMBLK
#define ctblkrqs	ctsemrqs
#define ctblkclr	ctsemclr
#define ctblkwat	ctsemwat
#endif

#ifndef ctSEMTIM
#define SEMAtim		SEMA
#define pSEMAtim	pSEMA
#define cttimrqs	ctblkrqs
#define cttimclr	ctblkclr
#define cttimwat	ctblkwat
#endif

#ifndef ctSEMUTX
#define SEMAmut		SEMA
#define pSEMAmut	pSEMA
#define ctmutrqs(s,o)	ctsemrqs(s,WAIT,o)
#define ctmutclr	ctsemclr
#endif

#ifdef ctCREATE_SEMA
#ifdef ctSTATIC_SEMA
**** ctCREATE_SEMA and ctSTATIC_SEMA are mutually exclusive options ****
#endif
#endif
#ifdef ctCREATE_SEMA
#define ctREUSE_SEMA
#endif
#ifdef ctSTATIC_SEMA
#define ctREUSE_SEMA
#endif

#endif

#ifdef ctCLIENT
#ifndef ctLOCFH
#define ctLOCFH
#endif
#endif

#ifdef TRANPROC
#define ctNogloTran
#endif

#ifdef  ctLOGIDX
#ifndef TRANPROC
#undef  ctLOGIDX
#endif
#endif

#ifdef  CTHISTORY
#ifndef TRANPROC
#undef  CTHISTORY
#endif
#endif


#ifdef MULTITRD

#define MAX_KEY_SEG	ctmaxkeyseg
#ifndef ctMAX_KEY_SEG
#define ctMAX_KEY_SEG	12
#endif

#else /* MULTITRD */

#ifdef ctMAX_KEY_SEG
#define MAX_KEY_SEG	ctMAX_KEY_SEG
#else
#define MAX_KEY_SEG	12	/* maximum number of segments per key	*/
#endif

#endif /* MULTITRD */

/* ******************************************************************** */
/* ************************     S T O P    **************************** */
/* ***     D O   N O T   C H A N G E   T H E    F O L L O W I N G   *** */
/* ***   W I T H O U T   C H E C K I N G   W I T H   F A I R C O M  *** */
/* ******************************************************************** */

#ifndef ctrt_monitor
#define ctrt_monitor	ctrt_printf
#endif

#ifdef ctMIRROR
#ifdef FPUTFGET
   *** Mirrored files are not supported under FPUTFGET ***
#endif
#endif

#define ctGhatSize	9 /* ctghat array size-see ctkrnl.c & ctgvar.h */
#define ctGhatSN 	4

#ifndef ctCLIENT
#ifdef ctSRVR
#define ctdivs	((LONG) ctghat[ctGhatSN])
#else
#define ctdivs	((LONG) 0)
#endif
#endif

#ifdef ctLOCLIB
#ifndef ctNOGLOBALS
#define ctNOGLOBALS
#endif
#ifndef ctCONV
#define ctCONV
#endif
#endif

#ifdef ctNOGLOBALS

#ifdef ctLOCLIB
#ifndef TRANPROC
#define ctNogloSize
#define ctNogloTran
#endif
#endif /* ctLOCLIB */

#endif /* ctNOGLOBALS */

#ifndef ctMEMCHRtxt
#define ctMEMCHRtxt	pVOID
#endif

#ifndef ctSIZE
#define ctSIZE(X) ((UINT)sizeof(X))
#endif

#ifndef ctSIZET
#define ctSIZET	size_t
#endif

#ifndef ctALLOC
#define ct_alloc(n,s)	calloc(((ctSIZET) (n)),((ctSIZET)(s)))
#define ctafree(p)	free(p)
#endif

#ifndef ctFileNone
#ifdef CT_ANSI
#define ctFileNone		((RNDFILE) NULL)
#else
#define ctFileNone		((RNDFILE) -1)
#endif
#endif /* ~ctFileNone */

#ifndef ctFileFailure
#ifdef CT_ANSI
#define ctFileFailure		== (RNDFILE) NULL
#else
#define ctFileFailure		< (RNDFILE) 0
#endif
#endif /* ~ctFileFailure */

#ifndef ctVDECL
#define ctVDECL ctDECL
#endif

#ifdef ctCONV
#define PARM2 ,((COUNT) 0)
#define PARM3 ,((pVOID)NULL)
#define PARM4 ,((pLONG)NULL)
#define PARM5 ,((pVOID)NULL)
#define PARM6 ,((pVRLEN)NULL)
#define PARM7 ,((COUNT)0)
#else
#define ctreeCONV
#define ctCONV
#endif

#ifndef ctMAXUSERS
#define ctMAXUSERS	528
#endif
#define ADMUSERS	7
#define MAXCOMMP	10
#define MXU2		(ctMAXUSERS + ADMUSERS + MAXCOMMP + 1)
#ifndef MAXAPPS
#define MAXAPPS		(ctMAXUSERS + 1)
#endif

#define	ITIMretry	10
#define NOocsema	-199

#define TRD_SQL		0x0001
#define TRD_COMMP	0x0002
#define TRD_SBLD	0x0004
#define TRD_ISAM	0x0008
#define TRD_DUMP	0x0010
#define TRD_ADMIN	0x0020
#define TRD_SMON	0x0040
/* 0x0080 and 0x0100 and 0x0200 reserved for 3 additional monitors */
#define TRD_LOGON	0x0400
#define TRD_DNODE	0x0800	/* delete node thread */
#define TRD_RTREE	0x1000	/* r-tree execute thread */

#define USE_DEFAULT	(-2L)
#define SUSE_DEFAULT	(-2)
#define UDLK_TRN	(-3)

#define CLN_CHKACT	1
#define CLN_CHKABT	2
#define CLN_GETNOD	(4 | CLN_CHKACT | CLN_CHKABT)
#define CLN_GETRCV	8
#define CLN_CLOSE	(16 | CLN_CHKACT)
#define CLN_STRIP	(32 | CLN_CHKABT)
#define CLN_ABTNOD	(64 | CLN_CHKACT | CLN_CHKABT)
#define CLN_SPLNOD	(128 | CLN_CHKACT | CLN_CHKABT)
#define CLN_DELETE	256
#define CLN_RTRY	(-1)
#define CLN_SKIP	0
#define CLN_COMMIT	1
#define CLN_UNDO	2

/* COMPATABILITY definitions */
#define ctCompat5000		((LONG) 0x001)
#define ctCompatLogon		((LONG) 0x002)
#define ctCompatSpcmgt		((LONG) 0x004)
#define ctCompatStrmfl		((LONG) 0x008)
#define ctCompatWThru		((LONG) 0x010)
#define ctCompatMP		((LONG) 0x020)

/* DIAGNOSTICS definitions */
#define ctDiagnoseWalk		((LONG) 0x00001)
#define ctDiagnoseCompare	((LONG) 0x00002)
#define ctDiagnoseLeak		((LONG) 0x00004)
#define ctDiagnoseTlog		((LONG) 0x00008)
#define ctDiagnoseQlog		((LONG) 0x00010)
#define ctDiagnoseLlog		((LONG) 0x00020)
#define ctDiagnoseSpcmgt	((LONG) 0x00040)
#define ctDiagnoseComm1		((LONG) 0x00080)
#define ctDiagnoseComm2		((LONG) 0x00100)
#define ctDiagnoseLogonComm	((LONG) 0x00200)
#define ctDiagnoseThdDmp	((LONG) 0x00400)
#define ctDiagnoseDebug		((LONG) 0x00800)
#define ctDiagnoseTrapComm	((LONG) 0x01000)

/* CTSTATUS_MASK definitions */
#define ctStatusVDP		((LONG) 0x001)
#define ctStatusFID		((LONG) 0x002)

#define C254		(C255 - 1)

#define	LOGTYPE		0
#define	STRFILE		2
#define TNAMES		4
#define IDXLOG		6
#define DATLOG		8

#define MRKSIZE		4
#define RESIDSIZE	(2 * ctSIZE(LONG))
#define RESHDRSIZ	(ctSIZE(UCOUNT) + 2 * ctSIZE(VRLEN) + 3 * ctSIZE(LONG))
#define FRESHDRINC	RESHDRSIZ
#define VRESHDRINC	(3 * ctSIZE(LONG))

#define CONVERT_NUL	0
#define CONVERT_IN	1
#define CONVERT_OUT	2

#define ctREGULAR_LOGON
#define LOGON		0
#define LOGOFF		1
#define STOPSRVR	2
#define STARTSRVR	3
#define LOGONu		4

#ifndef COMMIT_DELAY
#define COMMIT_DELAY	10L	/* milliseconds */
#endif
#define CHECK_DELAY	50L
#ifndef ctMILLISECOND
#define ctMILLISECOND(t)	(t)
#endif

#define WAIT		-1L	/* inifinite	*/
#define WAITnodebug	-2L
#define NOWAIT		0L
#define SHORTWAIT	1L	/* smallest tick */
#define NOT_OP_RCV	-3
#define NOT_OP_FID	-4
#define NOT_OP_TRY	-5

#define SHRINKQ		0
#define CHKPNTQ		1
#define ctQUEUE		2
#define MONITORQ0	3
#define ctMAX_MONITOR	2

#ifndef ctSTACKSIZE
#define ctSTACKSIZE	16384
#endif
#define QCHUNK_SIZE	10000L
#define ctAbsMem	4
#define ctGidMem	2

#define SETBIT		0
#define TSTBIT		1
#define SHDHBINS	128
#define LOKHBINS	16
#define TRNHBINS	128
#define ctCONBINS	6
#define ctMEMSIG	0x4385

#define SEQNXT	1
#define SEQPRV	2

#define EXS		0	/* exists			*/
#define SPC		1	/* pending			*/
#define UND		2	/* undoing pending		*/
#define RVS		3	/* part of SPC pair		*/
/* -------------------------------------------------------------*/
#define SPC_DIF		4	/* pending for dif tran		*/
#define UND_DIF		5	/* undoing pending for dif tran	*/

#define FN_MASK		0x00ff
#define FN_MASK_ISAM	0x0100
#define FN_MASK_LONG	0x0200
#define FN_MASK_VRLEN	0x0400
#define FN_MASK_OUTPUT	0x0800
#define FN_MASK_LOGON	0x1000
#define FN_MASK_NFCHK	0x2000
#define FN_MASK_OUTRET	0x4000

#define FN_BASE_LONG	150
#define FN_BASE_VRLEN	180
#define FN_BASE_TEXT	188

/* control structure list types for ctgetlst() & ctputlst() */
#define PI_UNIT		16

#define SHDTYP		0
#define RS2TYP		1	/* available for use */
#define PI1TYP		2
#define PI2TYP		3
#define PI4TYP		4
#define PI8TYP		5
#define PIwTYP		6
#define PIxTYP		7
#define PIyTYP		8
#define PIzTYP		9
#define BATTYP		10
#define ILKTYP		11
#define RS1TYP		12	/* available for use */

#define SEMTYP		13    /* These types do not require semaphore control */
#define COMTYP		13
#define ABTTYP		14
#define IXCTYP		15
#define DTCTYP		16
#define CTCTYP		17
#define LOKTYP		18
#define RS3TYP		19	/* available for use */
#define IXBTYP		20
#define DTBTYP		21
#define NUMCTLIST	22

#ifdef MULTITRD
#define FIRST_USER_TASK	2

#define CTSORTBUF	16000
#define CTLISTBLK	16384		/* list   alloc block size	*/
#define CTLISTBUF	64000		/* buffer alloc block size	*/
#define CTDELSIZ	((UINT) 32768)	/* ct_del[] size		*/
#else
#define CTSORTBUF	16000
#define CTLISTBLK	2048		/* list   alloc block size	  */
#define CTDELSIZ	((UINT) 2048)	/* ct_del[] size (must be >= 512) */
#endif

#define COMMONOWN	ct_cmnown


/* ************	     ctNOGLOBALS REQUIRES CTS_ISAM	  ************* */
#ifdef ctNOGLOBALS
#ifndef CTS_ISAM
#define CTS_ISAM
#endif
#ifndef ISAM_VARS
#define ISAM_VARS
#endif
#endif

/* *********	    DISABLE FEATURES DEPENDENT ON CTS_ISAM      ******** */
#ifndef CTS_ISAM

#ifdef RTREE
#undef RTREE
#define NO_RTREE
#endif

#ifdef CTBATCH
#undef CTBATCH
#define NO_BATCH
#endif

#ifdef PARMFILE
#undef PARMFILE
#define NO_PARMF
#endif

#endif /* ~CTS_ISAM */

/* ***********        FPUTONLY NO LONGER SUPPORTED          *********** */
#ifdef FPUTONLY
This configuartion option is no longer supported. Use NOTFORCE and
a file mode including WRITETHRU to force updates (on a file by file basis)
to disk. DO NOT USE WRITETHRU WITH A FILE SUPPORTING TRANSACTION LOGGING.
#endif

/* ***********          NO_BOUND IMPLIES NOTFORCE          *********** */
#ifndef CTBOUND
#ifdef FPUTFGET
#undef FPUTFGET
#define NOTFORCE
#endif
#endif

/* ***********        CTSUPER & RESOURCE IMPLY VARLDATA    *********** */
#ifdef CTSUPER
#ifndef VARLDATA
#define VARLDATA
#endif
#endif
#ifdef RESOURCE
#ifndef VARLDATA
#define VARLDATA
#endif
#endif

/* ***********       FPUTFGET CANNOT SUPPORT TRANPROC       *********** */
#ifdef FPUTFGET
#ifdef TRANPROC
#undef TRANPROC
#endif
#endif

/* ***********    DEFINE MLTORTRN IF MULTITRD OR TRANPROC   *********** */
#undef MLTORTRN
#ifdef MULTITRD
#define MLTORTRN
#endif
#ifdef TRANPROC
#ifndef MLTORTRN
#define MLTORTRN
#endif
#endif

/* ***********   CTSERVER & MULTITRD ARE MUTUALLY EXCLUSIVE  ********* */
#ifdef CTSERVER
#ifdef MULTITRD
	*** CTSERVER & MULTITRD are mutually exclusive ***
#endif
#endif

/* ***********  GNSERVER indicates either CTSERVER or MULTITRD  ****** */
#ifdef GNSERVER
#undef GNSERVER
#endif
#ifdef CTSERVER
#define GNSERVER
#endif
#ifdef MULTITRD
#define GNSERVER
#endif

/* ***********     FPUTFGET CANNOT BE USED WITH THE SERVER    ********* */
#ifdef GNSERVER
#ifdef FPUTFGET
#undef FPUTFGET
#define NOTFORCE
#endif
#endif

/* ***********   MUSTFRCE indicates multi-user requirements  ********* */
#ifdef MUSTFRCE
#undef MUSTFRCE
#endif
#ifdef FPUTFGET
#define MUSTFRCE
#endif
#ifdef GNSERVER
#define MUSTFRCE
#endif

/* ***********       DOSFLUSH APPLIES ONLY TO FPUTFGET       ********* */
#ifdef NOTFORCE
#ifdef DOSFLUSH
#undef  DOSFLUSH
#define NO_FLUSH
#endif
#endif

/* ***********       CTASYNCR APPLIES ONLY TO MULTITRD       ********** */
#ifdef CTASYNCR
#ifndef MULTITRD
#undef CTASYNCR
#endif
#endif

/* *************     UNIFRMAT APPLIES ONLY TO HIGH_LOW    ************* */
#ifdef LOW_HIGH
#ifdef UNIFRMAT
#undef  UNIFRMAT
#define NO_UNIFM
#endif
#endif

/* *************     UNIFRMAT DOES NOT SUPPORT TRANPROC    ************* */
#ifdef UNIFRMAT
#ifdef TRANPROC
---- cannot compile a UNIFRMAT/TRANPROC c-tree LIBRARY ----
#endif
#endif

/* **************     DEFAULT FPUTFGET LOCKING MODELS     ************* */
#define ctLOCK_BLOCK	1		/* used in calls to ctfpglok */
#define ctLOCK_NBLCK	2
#define ctLOCK_UNLCK	4

#ifdef FPUTFGET
#ifndef ctMAX_USRLOKS
#define ctMAX_USRLOKS	256		/* users */
#endif
#ifndef ctMAX_MBRLOKS
#define ctMAX_MBRLOKS	1024		/* superfile members */
#endif
#ifndef ctLOCK_TOP
#define ctLOCK_TOP	0x7fffffff	/* 2GB limit */
#endif
#define ctLOCK_OFFSET	(ctLOCK_TOP - (ctMAX_MBRLOKS * ctMAX_USRLOKS))

#ifndef ctLOCK_EXTENDED
#ifndef ctLOCK_DIRECT
#ifndef ctLOCK_DUMMY

#ifdef ctPortUNIX
#define ctLOCK_DIRECT
#else  /* ~ctPortUNIX */
#define ctLOCK_EXTENDED
#endif /* ~ctPortUNIX */

#endif /* ~ctLOCK_DUMMY */
#endif /* ~ctLOCK_DIRECT */
#endif /* ~ctLOCK_EXTENDED */

#ifdef ctPortUNIX
#ifdef ctMTFPG
#ifndef ctMTFPG_LOKCNF
#define ctMTFPG_LOKCNF 
#endif
#endif /* ctMTFPG */
#endif /* ctPortUNIX */

#ifndef ctLOCK_DUMMY

#ifndef CTSUPER_noMBRLOCK
#ifndef CTSUPER_MBRLOCK
#define CTSUPER_MBRLOCK
#endif
#endif

#ifndef ctLOKCNT
#define ctLOKCNT
#endif

#endif /* ~ctLOCK_DUMMY */

#endif /* FPUTFGET */

/* ************* ************* ************* ************ ************* */


#define PERMFLAG	(VLENGTH | PREIMG | LOGFIL | WRITETHRU | CHECKLOCK | SUPERFILE | CHECKREAD | LOGIDX)

#define NUMCHANEL	2	/* # io channels per file when DUPCHANEL on */
#define MRKSIZ		4	/* exceptional element marks per byte */

#define MAXLEV		11	/* maximum height of B-Tree + 1		*/
#define	VARI_LEN	(ctSIZE(VRLEN) + 2 * ctSIZE(LONG))
#define ctNODEhdr	18

#define HDRSIZ   128
/* permanent header information */
#define DHDRSIZ  HDRSIZ

#define PDZ	2	 /* padding size */
#define EXZ	8	 /* Maximum IFIL name extension size		   */
#define PWZ	10	 /* Maximum password length (including terminator) */
#define IDZ	32	 /* Maximum ID name length  (including terminator) */
#define DSZ	64	 /* group/user description information		   */
#define FNZ	256	 /* parm block maximum name size		   */

#ifdef VINES
#define STZ	64	 /* streettalk length  				   */
#endif

#ifdef  FPUTFGET
#define PRDRPEAT	10	/* # of times to retry predsr node	*/
#else
#define PRDRPEAT	0
#endif

#ifdef ctMAXMEMB
#define MAXMEMB		ctMAXMEMB
#else
#define	MAXMEMB		31	/* maximum additional index members	*/
#endif

typedef struct ugrup {
	TEXT	 groupid[IDZ];	/* ASCIIZ group id		*/
	} UGRUP;
#ifdef ctSP
typedef UGRUP far   *	pUGRUP;
#else
typedef UGRUP ctMEM *	pUGRUP;
#endif
#define MAX_UGRUP	128	/* maximum number of user groups	*/

#ifndef CTBOUND
#define MAXVABS			3		/* input segments */
#define OUTSEG			(MAXVABS + 1)	/* output segment */
#define BEGMSG_OFFSET		(OUTSEG * ctSIZE(VRLEN))
#define INPUT_OFFSET		(BEGMSG_OFFSET + ctSIZE(reqPARMB))
#define OUTPUT_OFFSET		(BEGMSG_OFFSET + ctSIZE(rspPARMB))
#ifdef ctSP
#define MsgLen(mp,i)		*(((VRLEN far *) mp) + i - 1)
#else
#define MsgLen(mp,i)		*(((VRLEN *) mp) + i - 1)
#endif
#endif

#ifndef ctrt_fwrite
#ifndef ctFWRITE
#define ctrt_fwrite	fwrite
#endif
#endif

#ifndef ctrt_fread
#ifndef ctFREAD
#define ctrt_fread	fread
#endif
#endif

#ifndef ctrt_fseek
#ifndef ctFSEEK
#define ctrt_fseek	fseek
#endif
#endif

/* ******************************************************************** */
/* ***			SYMBOLIC CONSTANTS			   ***  */
/* ******************************************************************** */

#define ctREAD_COMPLETE	1
#define ctREAD_PARTIAL	2

#define	DRNZERO	 	(ctRECPT) 0
#define	NODEZERO	(LONG   ) 0

#define MIRRORD	 	0x0001	/* xflmod */
#define ctPREIMAGE	0x0004	/* xflmod */
#define ctTEMPFILE	0x0020	/* xflmod */

#define ADMOPEN	 	0x1000	/* flmode */

#define ctRBLINDX	((LONG) 0x0001)	/* tflmod */
#define ctISAMKOFF	((LONG) 0x0002)	/* tflmod */
#define ctHOSTOPEN	((LONG) 0x0004)	/* tflmod */
#define ctDYNDMPmod	((LONG) 0x0008)	/* tflmod */

#define tflmodSET	1
#define tflmodTST	2
#define tflmodOFF	3
#define tflmodGET	4

#define ctTEMPINDX	-105
#define CTVFSGnokeys	-10000

#define FRCEADJ	 16
#define THRUADJ	 32
#define HISTADJ	 64
#define CTREAD	 0
#define CTWRITE	 1
#define CTWASYN  2
#define CTNONE	 3
#define CTXTND	 4
#define CTCLRBF	 5
#define CTREADP	 8
#define CTFRED	 (CTREAD  | FRCEADJ)
#define CTFWRT	 (CTWRITE | FRCEADJ)
#define CTURED	 (CTREAD  | THRUADJ)
#define CTUWRT	 (CTWRITE | THRUADJ)
#define CTHISTR	 (CTREADP | HISTADJ)

#define ctDD_FIL	1	/* regular file entries */
#define ctDD_STR	2	/* start file entries */
#define ctDD_LOG	3	/* transaction log entries */
#define ctDD_DEF	4	/* first entry in dump file */
#define ctDD_CLN	5	/* non TRNLOG file is clean */
#define ctDD_END	6	/* last entry in dump file */
#define ctDD_DRT	7	/* non TRNLOG file is dirty */
#define ctDD_CNT	8	/* regular file extension entries */
#define ctDD_RCV	9	/* must force recovery during ctrdmp */
#define ctDD_SUP	10	/* header location info for MBR_SUPER */
#define ctDD_LST	10
#define ctDD_PRE	32	/* file temporarily set to LOGFIL */
#define ctDD_EXT	64	/* file extent size temporarily set non-zero */
#define ctDD_SKP	128	/* skip clean file during recovery */
#define ctDD_FRC	256	/* index recovery forced: modify nument */

#ifdef ctSQL
#define SSO	1	/* SS (strict serializer) logical Open lock */
#define SSCI	2	/* SS commit intent lock */
#define SSC	3	/* SS commit lock */
#define NSCI	4	/* NS (nonstrict serializer) commit intent lock */
#define NSC	5	/* NS commit lock */
#endif
#define RDL	6	/* read lock */
/* #define	WRL	7	** write lock held to reader commits */
/* #define	CIL	8	** pending WRL to WXL promotion */
#define WXL	9	/* exclusive write lock */
#define WXH	10	/* exclusive write lock (no aggregate check) */

#define RES_SRVR	256

#define LEAF     1
#define NOLEAF   0

#define ALPHAKEY	'\0'

/*
** obsolete key types: no longer supported.
**
	INTKEY
	SFLOATKEY
	DFLOATKEY
**
*/

#define COL_PREFIX	'\4'	/* leading character compression	*/
#define COL_SUFFIX	'\10'	/*  8 decimal: padding compression	*/
#define COL_BOTH	'\14'	/* 12 decimal: both of the above	*/
#define ALT_SEQ		'\20'	/* 16 decimal: alternative col sequence	*/
#define MSK_ALT		~ALT_SEQ

#define DUPKEY   '\1'
#define NODUPKEY '\0'

#define REGULAR  '\1'
#define DUPLEAF  '\0'
#define DUPNONL	 '\3'

#define DAT_CLOSE 0
#define IDX_CLOSE 1
#define VAT_CLOSE 2

#define SUPADD	  16
#define MODMODE	  32
#define CHGADD	  64
#define IDXONLY	  128


#define SECSIZ   128		/* logical sector size. DO NOT CHANGE.	*/
#define CTBUFSIZ (MAX_NAME + 1)	/* ct_buf[] size */
#define ctUPDATED C255		/* C255 defined in ctcmpl.h */
#define DELFLG	  C255
#define RESFLG	  C254
#define COMPACT  '\143'		/* file compaction flag: must rebuild indices */
#define CTBADOPN '\122'		/* file corrupt on open */
#define CTBADLOP '\114'		/* leaf level loop: must rebuild indices */
#define CMPMSK	 0x00ff

#define MAXAGE    0xffff	/* max age of lru counter before roll-over */
#define MAXSRLPOS 0xffff	/* no serial segment flag		   */

#define	LH_VER	 0x0001			/* LOW_HIGH option 		*/
#define	HL_VER	 0x0002			/* HIGH_LOW   "			*/

#define ADD_LOCK	0	/* cts_lok() flag: adding a lock	*/
#define RED_LOCK	1	/*		   check if red locked	*/
#define MST_LOCK	2	/*		   check if wrt locked	*/

/* ***********     DEFINE LOCKING REQUIREMENTS & TESTS      *********** */
#define LOCK_TEST MST_LOCK

#define SHDSRCH		64	/* no preimg search			*/
/* reserve this bit	128	   for future use			*/
/* reserve ctHISTkchg	256	   for history key change: no data image*/
#define TRANMSK		(~SHDSRCH)

#define LHDRCOMP	1
#define SH_HDRCP	(LHDRCOMP | SHDSRCH)
#define LADDKEY		2
#define SH_ADDKY	(LADDKEY | SHDSRCH)
#define LADDSI		(LADDKEY + 1)
#define LDELKEY		4
#define SH_DELKY	(LDELKEY | SHDSRCH)
#define LDELSI		(LDELKEY + 1)
#define SH_INVLD	6
#define BEGTRAN		7
#define SH_BEGTRAN	(BEGTRAN | SHDSRCH)
#define ENDTRAN		8
#define ABRTRAN		9
#define SH_NLINK	10
#define NEWLINK		SH_NLINK
#define SH_VLINK	11
#define SH_IMAGE	12
#define NEWIMAGE	SH_IMAGE
#define OLDFILL		13
#define OLDIMAGE	14
#define DIFIMAGE	15
#define RDYTRAN		16
#define LHDRSHST	17
#define LOGEXTFIL	18
#define SH_EXTFIL	(LOGEXTFIL | SHDSRCH)
#define NODEXTFIL	19
#define SH_QNODE	20
#define LHDRWORD	21
#define SH_HDRWORD	(LHDRWORD | SHDSRCH)
#define UNDTRAN		22
#define BEG2FASE	23
#define ACK2FASE	24
#define SH_RSTFIL	25
#define LOGRSTFIL	SH_RSTFIL

#define CHKPNT		26
#define OPNTRAN		27
#define CRETRAN		28
#define DELTRAN		29
#define CLSTRAN		30
#define LLOGIN		31
#define LLOGOFF		32
#define UPDTRAN		33
#define ENDLOG		34
#define LPRMIDX		35

#define RESTRAN		36
#define SH_WRITE	37
#define SH_REUSE	(38 | SHDSRCH)
#define SH_RSTRUS	39
#define ABNTRAN		40
#define LPABYTE		41
#define INDXADD		42
#define INDXDEL		43

/* reserve 44 through 47 for tran types that get their own trannum */
#define FRCTRAN		48	/* force log turnover */
#define RENTRAN		49
#define LLOGNODE	50
#define getTRANNUM	50
#define CMPTRAN		51
/* reserve 52 through 60 for tran types that use current trannum */

#define INDXMID		61
#define INDXEND		61
#define INDXERR		62
#define INDXVUL		63
#define INDXLST		63

#define FILEOFFS	12
#define COMITRAN	1L
#define FIRSTRAN	2L

#define NOCTCLUP	1
#define BUFRLOCK	2
#define GNS		3
#define ctGN_CMT	32
#ifdef DBGtree
#define ctGN_ADD	4
#define ctGN_DEL	5
#define ctGN_UND	6
#define ctGN_SRC	7
#define ctGN_ABT	8
#define DBGTREEno	16
#define NOnd		(NO | DBGTREEno)
#define NOCTCLUPnd	(NOCTCLUP | DBGTREEno)
#define BUFRLOCKnd	(BUFRLOCK | DBGTREEno)
#define GNSnd		(GNS | DBGTREEno)
#else
#define NOnd		NO
#define NOCTCLUPnd	NOCTCLUP
#define BUFRLOCKnd	BUFRLOCK
#define GNSnd		GNS
#endif

#ifdef DBGnodeIO
#define DBGnodeIOopnfil	1
#define DBGnodeIOclsfil	2
#define DBGnodeIOextfil	3
#define DBGnodeIOnewnod	4
#define DBGnodeIOwrite	5
#define DBGnodeIOread	6
#define DBGnodeIOinit	7
#define DBGnodeIOreport	8
#define DBGnodeIOfree	9
#define DBGnodeIOgetnod	10
#define DBGnodeIOnlst1	11
#define DBGnodeIOnlst3	12
#define DBGnodeIOplst2	13
#define DBGnodeIOplst5	14
#define DBGnodeIOnlstr1	15
#define DBGnodeIOnlstr2	16
#define DBGnodeIOnlstr3	17
#define DBGnodeIOplstr1	18
#define DBGnodeIOplstr2	19
#define DBGnodeIOplstr3	20
#define DBGnodeIOplstr4	21
#define DBGnodeIOplstr5	22
#define DBGnodeIOplstr6	23
#define DBGnodeIOsucesr	24
#define DBGnodeIOterr	25
#define DBGnodeIOroot	26
#define DBGnodeIOlogoff	27
#define DBGnodeIOupdyes	28
#define DBGnodeIOupdno	29
#define DBGnodeIOidxfrs	30
#define DBGnodeIOchkmtc	31
#define DBGnodeIOflsbuf	32
#define DBGnodeIOrb1	33
#define DBGnodeIOrb2	34
#define DBGnodeIOrb3	35
#define DBGnodeIOrb4	36
#define DBGnodeIOrb5	37
#define DBGnodeIOrb6	38
#define DBGnodeIOrb7	39
#define DBGnodeIOrb8	40
#define DBGnodeIOrb9	41
#define DBGnodeIOrb10	42
#define DBGnodeIOrb11	43
#define DBGnodeIOrb12	44
#define DBGnodeIOrb13	45
#define DBGnodeIOrb14	46
#define DBGnodeIOrb15	47
#define DBGnodeIOrb16	48
#define DBGnodeIOrb17	49
#define DBGnodeIOrb18	50
#define DBGnodeIOrb19	51
#define DBGnodeIOrb20	52
#define DBGnodeIOrb21	53
#define DBGnodeIOrb22	54
#define DBGnodeIOrb23	55
#define DBGnodeIOrb24	56
#define DBGnodeIOlock	57
#define DBGnodeIOusralv	58
#define DBGnodeIOlokerr	59
#define DBGnodeIOunlock	60
#define DBGnodeIOulkerr	61
#define DBGnodeIOaddblk	62
#define DBGnodeIOlokerd	63
#define DBGnodeIOlokagn	64
#define DBGnodeIOloktry	65
#define DBGnodeIOuerr	66
#define DBGnodeIOpner1	67
#define DBGnodeIOpner2	68
#define DBGnodeIOpner3	69
#define DBGnodeIOwrterr	70
#define DBGnodeIOrederr	71
#endif /* DBGnodeIO */

#define TRN_AFLAG	1
#define TRN_ANODE	1
#define TRN_ADATA	3
#define TRN_AABRT	5

#define TRN_DNODE	2
#define TRN_DDATA	4
#define TRN_DABRT	6

#define ctNAMINDXadd	1
#define ctNAMINDXdel	2

#define DL_SRCH		0
#define DL_ADDR		1
#define DL_DLTR		2
#define DL_RVRS		3
#define DL_TADR		4
#define DL_TDLT		5

#define CURKEY		0
#define TMPKEY		1

#define AVLIST		0		/*    bhl[0] bavl[0] */
#define DALIST		1		/* datbhl[0] bavl[1] */
#define INLIST		2		/*    bhl[1] bhla[0] */
#define DULIST		3		/* datbhl[1] bhla[1] */
#define NUMANCHOR	2
#ifdef MULTITRD
#define PNLIST		4		/*    bhl[2] bhla[2] */
#define DPLIST		5		/* datbhl[2] bhla[3] */
#undef  NUMANCHOR
#define NUMANCHOR	4
#endif

/*
**
** The following modules all use the FCS file extent:  ctopt2.h, ctscrt.h,
**   ctclb2.c, ctclb3.c, ctdump.c, ctldmp.c, ctrdmp.c, ctsadm.c ctsrvr.c.
**
*/
#define FAIRCOMS	"FAIRCOMS"

#define ADMIN		"ADMIN"
#define GUEST		"GUEST"
#ifdef MLTORTRN
#define FAIRSQLS	"SQL.FCS"
#endif

#define CT_V60	0			/* Version 6 Release Flag */
#define CT_V65	5
#define NEWIFIL

/* File Format Indicators */
#define CT_V6	 0x0080			/* c-tree version 6.0		*/

#define	DEF_MASK_IFIL2	0x0001L		/* extended IFIL structures	*/
#define DEF_MASK_SPCMGT	0x0002L		/* unambig superfile space mgmt	*/
#define DEF_MASK_PSCL24	0x0004L		/* complete Pascal string def	*/
#define DEF_MASK_XTDPRM	0x0008L		/* extended permission mask	*/
#define DEF_MASK_OPNTIM	0x0010L		/* tran log file open has time	*/

/* bit masks for use with ct_usrsi */
#define ctMEMidx	0x01
#define ctMEMque	0x02
#define ctMEMswp	0x04
#define ctMEMlok	0x08

#ifdef ctPASCALst
#ifndef ctNotPASCAL24
#define ctPASCAL24
#endif
#endif

#ifndef FPUTFGET
#define gtroot(knum)	(knum->root)
#endif
#ifdef GNSERVER
#define cthkey(knum)	(knum->hmem)
#else
#define cthkey(knum)	(knum->kmem ? ctkey(knum->filnum - knum->kmem) : knum)
#endif

#define ctDATEXT	".dat"
#define ctIDXEXT	".idx"
#define UGFNAME	"FAIRCOM.FCS"
#define UGFMODE	(SHARED | TRNLOG | SUPERFILE | ADMOPEN)

#ifndef ctSQL
#define ctNONSQL
#else
#define NOEXITREPLACEMENT
#endif
#include "ctssql.h"

#ifdef ctPREV_27a0718
typedef struct align_test {
	LONG	m1;
	char	m2;
	LONG	m3;
	} ALIGN_TEST;
#else
typedef struct align_test {
	double	m1;
	char	m2;
	double	m3;
	} ALIGN_TEST;
#endif /* ctPREV_27a0718 */

typedef struct align_bcd {
	double		m1;
	char		m2;
	SQL_DEC		m3;
	} ALIGN_BCD;
typedef struct align_tim {
	double		m1;
	char		m2;
	SQL_D_TIME	m3;
	} ALIGN_TIM;

#ifndef ALIGNMENT_COMP
#define ALIGNMENT_COMP	(SegOff(ALIGN_TEST,m3) - SegOff(ALIGN_TEST,m2))
#endif
#define ALIGNMENT_BCD	(SegOff(ALIGN_BCD ,m3) - SegOff(ALIGN_BCD ,m2))
#define ALIGNMENT_TIM	(SegOff(ALIGN_TIM ,m3) - SegOff(ALIGN_TIM ,m2))

#ifdef REVBIN
#undef REVBIN
#endif
#ifdef LOW_HIGH
#define REVBIN
#endif
#ifdef HIGH_LOW
#ifdef UNIFRMAT
#define REVBIN
#endif
#endif

#define myioBUFLEN	256

#define NAMILEN		12
#define NAMINDX		0
#define LOGINDX		1

#define CO_BASE		1
#define CO_CREATE	2
#define CO_MEMBER	3
#define CO_REGULAR	4

#define ctSQL_FILES	8
#define ctTRN_FILES	3
#define ctRT_FILES	3

#define ctIFIL_8Padj	32   /* IFIL: per index len adjustment for 8-byte ptr */

#ifndef PARM2
#define  ctreeVARLST
#define  PARM2
#define  PARM3
#define  PARM4
#define  PARM5
#define  PARM6
#define  PARM7
#endif

#ifdef PROTOTYPE
#ifdef ctreeVARLST
typedef ctCONV  COUNT (ctVDECL *pCEPFNC)();
#else
typedef ctCONV COUNT (ctVDECL *pCEPFNC)(COUNT ,COUNT ,pVOID ,pLONG ,
				pVOID ,pVRLEN ,COUNT );
#endif
#else /* PROTOTYPE */
typedef ctCONV  COUNT (ctVDECL *pCEPFNC)();
#endif /* PROTOTYPE */

#ifdef ctNOGLOBALS
#define ctREG_DEF_ID	"ctREG_DEF_ID"
#endif

#define ctSRVRSIDE	1
#define ctCLNTSIDE	2
#ifdef ctSEMCOM
#define LCLNT		3
#define LSRVR		4
#else
#define LCLNT	lclnt
#define LSRVR	lsrvr
#define ctcomrqs(l,sem,wait,by)	ctblkrqs(&l->sem,wait,-2)
#define ctcomwat(l,sem,wait,by)	ctblkwat(&l->sem,wait,-1)
#define ctcomclr(l,sem,by)	ctblkclr(&l->sem,-1)
#endif

#endif /* ctOPT2H */

/* end of ctopt2.h */
