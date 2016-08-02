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

#ifndef ctCOMMH
#define ctCOMMH

#include "ctcom2.h"

#define ntpUSEHANDLE  /* Activate n-tree Plus Handle passed from server*/

#ifdef  ctPortNTREE_PLUS
#ifdef 	ctPortUNIX
#ifdef 	MULTITRD
#ifndef ctCOMM_POLICE_THREAD
#define ctCOMM_POLICE_THREAD
#endif 	/* ~ctCOMM_POLICE_THREAD */
#endif 	/* MULTITRD */
#endif  /* ctPortUNIX */
#endif  /* ctPortNTREE_PLUS */

#ifdef  ctPortNTREE_PLUS
#ifdef  ctPortNATIVE_THREADS
#ifdef  ctCOMM_POLICE_THREAD
/* This undef will invoke native thread blocking reads  */
/* without selected thread 				*/
/* #undef  ctCOMM_POLICE_THREAD */ 
#endif 	/* ctCOMM_POLICE_THREAD */
#endif  /* ctPortNATIVE_THREADS */
#endif  /* ctPortNTREE_PLUS */

#ifdef ctPortMAC
#ifndef ntpUSEHANDLE
#define ntpUSEHANDLE
#endif
#endif

#ifdef ctThrdApp
#ifndef ntpUSEHANDLE
#define ntpUSEHANDLE
#endif
#endif


/****************************************************/
/* Names available to use for COMM_PROTOCOL keyword */
#define ctUNXCOM_TCP_NAME 	"F_TCPIP"
#define ctUNXCOM_TCP_NAM2 	"FSTCPIP"
#define ctUNXCOM_TCP_NAM3 	"FMacTCP"
#define ctUNXCOM_ADSP_NAM 	"FADSP"
#define ctUNXCOM_SHM_NAME 	"FSHAREMM"
#define ctUNXCOM_QUE_NAME 	"FUNXMSGS"
#define ctUNXCOM_NTP_NAME 	"FNTREEP"
#define ctUNXCOM_QNX_NAME 	"FQNXMSGS"

/* n-tree Plus COMM_PROTOCOL protocol names */
#define ctUNXCOM_NTP_SHAREMM 	"SHARED_MEMORY"
#define ctUNXCOM_NTP_SHARED 	"SHARED"
#define ctUNXCOM_NTP_SHM 	"SHM"
#define ctUNXCOM_NTP_SM 	"SM"
#define ctUNXCOM_NTP_NETBIOS 	"NETBIOS"
#define ctUNXCOM_NTP_NB 	"NB"
#define ctUNXCOM_NTP_TCP_IP 	"TCP/IP"
#define ctUNXCOM_NTP_TCPIP 	"TCPIP"
#define ctUNXCOM_NTP_TCP 	"TCP"
#define ctUNXCOM_NTP_SPX 	"SPX"

/* These defines must coincide with enums in ntreelib.h */
#define ntpNTREE_TCP	((NINT)1)
#define ntpNTREE_SPX	((NINT)2)
#define ntpNTREE_NET	((NINT)3)
#define ntpNTREE_SHM	((NINT)4)

/* Legacy Protocol Identifiers */
#define lcyNTREE_TCP	((NINT)100)
#define lcyNTREE_SHM	((NINT)400)
#define lcyNTREE_QUE	((NINT)500)
#define lcyNTREE_QNX	((NINT)600)
/****************************************************/

/****************************************************************************/
#ifndef ctSEMA
#include "ctsema.h"
#endif

#ifdef VINESSEC
#ifndef ctST
#ifdef __OS2__
#include <streetta.h>
#else
#ifdef ctBAN5
#include <ST.h>
#else
#include <streettalk.h>
#endif
#endif
#define ctST
#endif
#endif

#ifdef ctSP
typedef char far * far *	ppMTEXT;
typedef char far *	pMTEXT;
#else
typedef char ctMEM *	pMTEXT;
typedef char ctMEM *	ctMEM * ppMTEXT;
#endif


/*
** TRAP_COMM header definition. Each entry in log begins with such a header.
**
** A connect is designated by an entry with 0xefef in the ownr member. The
** variable region after this connect header has a two byte thread ID and
** two bytes of flavor information (the clen member should be 4 for this
** connect entry).
**
** A disconnect looks like a connect except that ownr is 0xecec.
**
** A end of log is designated by an entry with 0xeded in the ownr member
** and a zero clen member.
**
** Note that each entry has client and server flavor values so that the type
** of data in the log and the flavor of the header can be determined.
*/
typedef struct TCh_ {
	ULONG	clen;	/* variable region length	*/
	ULONG	olen;	/* possible output length	*/
	UCOUNT	ownr;	/* thread ID	 		*/
	} TC_BUFHDR, * pTC_BUFHDR;
#define TCHDRSIZ	10

typedef struct TCc_ {
	UCOUNT	cmds;
	UCOUNT	ownr;
	UTEXT	flvr;
	UTEXT	sflv;
	} TC_CON, * pTC_CON;
#define TCCONSIZ	6

typedef struct TCb_ {
	LONG	stmp;
	UTEXT	sflv;
	} TC_BEG, * pTC_BEG;
#define TCBEGSIZ	5

#define ctTC_CONN	0xefef
#define ctTC_BEGN	0xeeee
#define ctTC_FNSH	0xeded
#define ctTC_DISC	0xecec
#define ctTC_ACKN	0xebeb

#define ctNPATTRIBUTE_RECEIVE_TIMEOUT	1
#define ctNPATTRIBUTE_SEND_TIMEOUT	2

/* Defines used to identify server variable to get */
#define CTNPct_compflg 	0
#define CTNPcttflg     	1
#define CTNPct_sesout  	2
#define CTNPctgv  	3

#define CTNPDELIMITER 			'@'
#define CTNPDELPROTOCOL			'^'
#define CTNPDEL2PROTOCOL		':'

typedef struct {
	pCOUNT	 lfseg;	/* optional segment/map definitions */
	COUNT	 lfvar; /* # of var segments used in keys */
	UCOUNT	 lflen;	/* length */
	COUNT	 lftyp;	/* clstyp */
	COUNT	 lfmod;	/* file mode */
	COUNT	 lfkey;	/* key type & dup mode */
	COUNT	 lfrev;	/* key to dat map */
	pCOUNT	 lfalt;	/* alternative collating sequence */
} LOCLFILE;
typedef LOCLFILE ctMEM *	pLOCLFILE;

			/*
			** lfseg & lfvar not part of message; but app filno is.
			*/
#define LOCLSIZE	(5 * ctSIZE(COUNT) + ctSIZE(UCOUNT))

typedef struct {
#ifdef ctCONDIDX
	pVOID	cndx;	/* conditional index ptr */
#endif
	LONG	curp;	/* current ISAM position */
	LONG	vlen;	/* combined variable length */
	LONG	amsk;	/* auxiliary mask */
#ifdef ctCONDIDX
	COUNT	relk;	/* relative key number */
#else
	COUNT	rsv2;	/* reserved */
#endif
	COUNT	dres;	/* disable resource flag */
} LOCLAUX;
typedef LOCLAUX ctMEM *	pLOCLAUX;

typedef struct reqpb {
	COUNT	 pbfn;	/* function #		*/
	COUNT	 pbfl;	/* file #		*/
	LONG	 pblg;	/* long value		*/
	VRLEN	 pbsz;	/* length		*/
	VRLEN	 pbol;	/* output length	*/
	COUNT	 pbmd;	/* mode			*/
	COUNT	 pbvr;	/* version #		*/
	COUNT	 pbna;	/* # of arguments	*/
	UCOUNT	 pbmp;	/* request map		*/
} reqPARMB;
typedef reqPARMB ctMEM *	preqPARMB;

typedef struct	rsppb {
	COUNT	 qbfn;	/* function #		*/
	COUNT	 qbfl;	/* file #		*/
	LONG	 qblg;	/* long value		*/
	LONG	 qbrv;	/* 2nd long		*/
	LONG	 qbr3;	/* 3rd long		*/
	VRLEN	 qbsz;	/* size value		*/
	COUNT	 qbsh;	/* short value		*/
	COUNT	 qber;	/* error code		*/
	COUNT	 qbef;	/* error file (ISAM)	*/
	COUNT	 qbio;	/* sysio code		*/
	LONG	 qbat;	/* attention channel	*/
} rspPARMB;
typedef rspPARMB ctMEM *	prspPARMB;

typedef NLONG	Displace;
typedef UNLONG	RefVal;

typedef struct {
	pMTEXT	pMsg;
	ULONG	msgLen;
} MSGVEC;

typedef MSGVEC ctMEM * pMSGVEC;

#define ctSETMSG(m, p, l)		m.pMsg = (pMTEXT)p; \
										m.msgLen = (ULONG)l
#define ctGETMSG(msg)						msg.pMsg
#define ctGETMSGLEN(msg)					msg.msgLen

typedef struct {
	ULONG		 location;
	RefVal		 value;
} Reference;

typedef struct vab {
	ULONG		 flags;
	Reference	 ref;
	Displace	 reqLength;
	Displace	 posOffset;
	struct vab	*vabMate;
	struct vab	*nxtVab;
} VAB;
typedef VAB ctMEM *	pVAB;

/****************************************************************************/
/* Begin from ctcvec.h */
#ifdef ctSRVR
#ifndef MAXAPPS
  ----Server compilation must have MAXAPP already defined ----
#endif
#else /* ~ctSRVR */

#ifdef MAXAPPS
#undef MAXAPPS /* remove definition from ctopt2.h */
#endif

#define MAXAPPS	1 /* Client only has 1 */
#endif /* ~ctSRVR */

#ifdef  ctDLLOAD
#ifndef BUILDASDLL
#define BUILDASDLL
#endif
#endif

/*********************************************/
#ifdef BUILDASDLL

#ifdef ctPortWINDOWSNT
#define DllExport	__declspec(dllexport)
#define DllLoadDS	
#endif /* ctPortWINDOWSNT */

#ifdef ctPortOS2_2x
#define DllLoadDS	
#define DllExport
#endif /* ctPortOS2_2x */

#ifdef ctPortMAC
#define DllLoadDS	
#define DllExport
#endif /* ctPortOS2_2x */

/* Defaults DLL types */
#ifndef DllLoadDS
#define DllLoadDS	_loadds
#endif

#ifndef DllExport
#define DllExport	_export
#endif

#endif /* BUILDASDLL */
/*********************************************/

/*********************************************/
#ifndef BUILDASDLL 

#ifndef DllExport
#define DllExport
#endif

#ifndef DllLoadDS
#define DllLoadDS
#endif 

#endif /* ~BUILDASDLL */
/*********************************************/

#ifndef far
#define far
#endif

/**************************/ 

#define MAXSERVERS			 1
#define NPCALLVECTORS
#define MAXCTNPCALLBACKS		11
#define MAXCTNPCMDS			12

#define SYNC		false
#define ASYNC		true

#ifndef DeadLock
#define DeadLock  			(~0)
#define NoWait   			(0)
#define ShortWait 			(9)
#define WaitForever 		(~0)
#endif

#define ctMAX_RETRIES	4
#define ctMIN_TIMEOUT	2

typedef struct sessto {
	LONG	startTick;
	NINT	tripTime;
	NINT	minTimeout;
	NINT	currTimeout;
	NINT	nextTimeout;
	NINT	meanDev;
	TEXT	nRetries;
} SessRtt, ctMEM *pSessRtt;

typedef struct sessblk
{
	LONG	hSessionId;
	LONG	hSessionData;
	UCOUNT	recTimeOut;
	UCOUNT	sndTimeOut;
	SessRtt	recRtt;
	SessRtt	sndRtt;
	TEXT	remoteName[MAX_NAME];
	TEXT	localName[MAX_NAME];

	UCOUNT	localSid;		/*	session handle from open	*/
	UCOUNT	srvSid;			/*	server session id if appropriate */

	UCOUNT	maxCmdSz;		/*	maximum request size		*/
	UCOUNT	msgSize;		/*	appl message size			*/

	ULONG	lsttick;		/*	tickcount at last msg or tickle */
	ULONG	attributes;		/* n-tree Plus uses this for protocol id */
	ULONG	reserved;
	pVOID	pGlobals;		/* Backward pointer to n-tree Plus Global Handle */
	NINT	ntOWNR;			/* Thread Owner */
} Sessblk, ctMEM *pSessblk;

typedef struct SrvrSessGbls {
	UCOUNT	srvNbr;
	UCOUNT	maxcmd;
/*	UCOUNT	maxmsgin;  */
/*	UCOUNT	maxmsgout; */
	UCOUNT	maxusers;
	UCOUNT	activeUsers;
	TEXT	origName[MAX_NAME];	
	LONG	hServerSessionData;
	pVOID	pGlobals;		/* Backward pointer to n-tree Plus Global Handle */
} SrvrSessGbls, ctMEM *pSrvSessGbls;

typedef struct SrvrGbls {
LONG		hServerGlobalData;
SEMA		srvSema;
/*TEXT 		isWNE; */		/* nonzero if WaitNextEvent trap is valid */
/*NINT		newat; */		/* non zero if new version of at drivers */
LONG  		signature;		/* inited flag	*/
COUNT		nbrSessions;		/* active session count */
ULONG		maxPacketSize;		/* maximum size transmission on this adapter */
pVOID		com_cmds;      		/* Communications functions pointers */
pVOID 		srv_callBackCmds; 	/* Server's function pointers */
pSrvSessGbls	sSess[MAXSERVERS + 1];	/* array of ptrs to sessions */
pSessblk 	cSess[MAXAPPS + 1];
TEXT		sName[MAX_NAME];
TEXT		ProtocolText[24];	/* Protocol Console text */
pVOID 		ntpSession;		/* n-tree Plus base point */
pVOID		ntDllLoadHandle;	/* Load Library Handle use to FreeLibrary on Unload */
NINT		ntpProtocol;		/* n-tree Plus Protocol id */
NINT		ntpShutDown;		/* Server Shutdown indicator */
SEMA		SelectProtect;		/* Select Thread Protect Semaphore */
#ifdef ctPortWIN32
LONG		tlsIndex;		/* TlsAlloc Index to pass to DLL */
#endif
} SrvrGbls, ctMEM * pSrvrGbls, ctMEM * ctMEM * ppSrvrGbls;

/* end from ctcvec.h */
/****************************************************************************/


/****************************************************************************/
/* Prototypes of Communications DLL functions called from server			*/
#ifdef PROTOTYPE
typedef NINT DllLoadDS (*NpInitFuncPtr)(pSrvrGbls pGlobals);
typedef NINT DllLoadDS (*NpStartFuncPtr)(pSrvrGbls pGlobals, pTEXT name, pTEXT comm);
typedef NINT DllLoadDS (*NpMakeFuncPtr)(pSrvrGbls pGlobals, pTEXT name, pULONG handle,
						UCOUNT omode, UCOUNT pmode, ULONG size1, ULONG size2, LONG timeout,
						pTEXT ss);
typedef NINT DllLoadDS (*NpConnectFuncPtr)(pSrvrGbls pGlobals, UCOUNT handle);
typedef NINT DllLoadDS (*NpReadFuncPtr)(pSrvrGbls pGlobals, UCOUNT handle, pMTEXT pMsg1,
						ULONG msg1Len, pMTEXT pMsg2, ULONG msg2Len, pULONG bytesread);
typedef NINT DllLoadDS (*NpDisconnectFuncPtr)(pSrvrGbls pGlobals, UCOUNT handle);
typedef NINT DllLoadDS (*NpWriteFuncPtr)(pSrvrGbls pGlobals, UCOUNT handle, pMTEXT pMsg1,
						ULONG msg1Len, pMTEXT pMsg2, ULONG msg2Len, pULONG byteswritten);
typedef NINT DllLoadDS (*NpSetAttrFuncPtr)(pSrvrGbls pGlobals, UCOUNT handle,
							NINT attribute, pVOID pValue);
typedef NINT DllLoadDS (*NpGetAttrFuncPtr)(pSrvrGbls pGlobals, UCOUNT handle,
						NINT attribute, pVOID pValue);
typedef NINT DllLoadDS (*NpCloseFuncPtr)(pSrvrGbls pGlobals, UCOUNT handle);
typedef NINT DllLoadDS (*NpStopFuncPtr)(pSrvrGbls pGlobals);
typedef NINT DllLoadDS (*NpOpenFuncPtr)(pSrvrGbls pGlobals, pTEXT name, pUCOUNT handle, 
						pUCOUNT action, ULONG size, ULONG attribute, UCOUNT openflag,
						UCOUNT openmode, ULONG reserved);
typedef VRLEN DllLoadDS (*NpPackSizFuncPtr)(pSrvrGbls pGlobals, VRLEN size, pVRLEN ps_size);
typedef NINT DllLoadDS (*NpPoliceFuncPtr)(pSrvrGbls pGlobals);

#else	/* ~PROTOTYPE */

typedef NINT DllLoadDS (*NpInitFuncPtr)();
typedef NINT DllLoadDS (*NpStartFuncPtr)();
typedef NINT DllLoadDS (*NpMakeFuncPtr)();
typedef NINT DllLoadDS (*NpConnectFuncPtr)();
typedef NINT DllLoadDS (*NpReadFuncPtr)();
typedef NINT DllLoadDS (*NpDisconnectFuncPtr)();
typedef NINT DllLoadDS (*NpWriteFuncPtr)();
typedef NINT DllLoadDS (*NpSetAttrFuncPtr)();
typedef NINT DllLoadDS (*NpGetAttrFuncPtr)();
typedef NINT DllLoadDS (*NpCloseFuncPtr)();
typedef NINT DllLoadDS (*NpStopFuncPtr)();
typedef NINT DllLoadDS (*NpOpenFuncPtr)();
typedef VRLEN DllLoadDS (*NpPackSizFuncPtr)();
typedef NINT DllLoadDS (*NpPoliceFuncPtr)();

#endif 	/* ~PROTOTYPE */


typedef struct CommFuncStruct {
NpInitFuncPtr 		NpINIT;
NpStartFuncPtr 		NpSTART;
NpStopFuncPtr		NpSTOP;
NpMakeFuncPtr 		NpMAKE;
NpConnectFuncPtr	NpCONNECT;
NpDisconnectFuncPtr	NpDISCONNECT;
NpReadFuncPtr		NpREAD;
NpWriteFuncPtr		NpWRITE;
NpSetAttrFuncPtr	NpSETATTR;
NpGetAttrFuncPtr	NpGETATTR;
NpOpenFuncPtr		NpOPEN;
NpCloseFuncPtr		NpCLOSE;
NpPackSizFuncPtr	NpPACKSIZ;
NpPoliceFuncPtr		NpPOLICE;
} CommFuncPtrs, ctMEM *pCommFuncPtrs;

/****************************************************************************/

/****************************************************************************/
/* Prototypes of server functions that will be called from communications DLL */
#ifdef PROTOTYPE
typedef NINT DllLoadDS  (*SvDeferFuncPtr)(LONG);
typedef NINT DllLoadDS  (*SvCttcreFuncPtr)(pNINT ,NINT (*)(VOID ),UINT ,NLONG );

typedef NINT DllLoadDS  (*SvBlkrqsFuncPtr)(pSEMA sema,LONG wait,NINT own);
typedef NINT DllLoadDS  (*SvBlkclrFuncPtr)(pSEMA sema,NINT own);
typedef NINT DllLoadDS 	(*SvBlkwatFuncPtr)(pSEMA sema, LONG wait, NINT own);

typedef NINT DllLoadDS 	(*SvSemRqsFuncPtr)(pSEMA sema,LONG wait,NINT own);
typedef NINT DllLoadDS 	(*SvSemClrFuncPtr)(pSEMA sema,NINT own);
typedef NINT DllLoadDS 	(*SvSemWatFuncPtr)(pSEMA sema, LONG wait, NINT own);

typedef NINT DllLoadDS 	(*SvTimRqsFuncPtr)(pSEMA sema,LONG wait,NINT own);
typedef NINT DllLoadDS 	(*SvTimClrFuncPtr)(pSEMA sema,NINT own);
typedef NINT DllLoadDS 	(*SvTimWatFuncPtr)(pSEMA sema, LONG wait, NINT own);

typedef pTEXT       	(*SvMballcFuncPtr)(NINT numobj,UINT sizobj);
typedef VOID        	(*SvMbfreeFuncPtr)(pVOID objptr);
typedef pTEXT       	(*SvGetMemFuncPtr)(VRLEN iosize);
typedef VOID        	(*SvPutMemFuncPtr)(pTEXT loc);

typedef NINT    	(*SvWaitWhileFuncPtr)(pCOUNT F, LONG delay);
typedef VOID DllLoadDS 	(*SvRevObjFuncPtr)(pVOID pData, NINT size);
typedef NINT 		(*SvUsrStatFuncPtr)(NINT own);

typedef	LONG DllLoadDS  (*SvGetSrVarFuncPtr)(NINT this_var);
typedef LONG 		(*SvPrintfFuncPtr)(pTEXT p);

#else 	/* ~PROTOTYPE */

typedef NINT DllLoadDS  (*SvDeferFuncPtr)();
typedef NINT DllLoadDS  (*SvCttcreFuncPtr)();

typedef NINT DllLoadDS  (*SvBlkrqsFuncPtr)();
typedef NINT DllLoadDS  (*SvBlkclrFuncPtr)();
typedef NINT DllLoadDS  (*SvBlkwatFuncPtr)();

typedef NINT DllLoadDS 	(*SvSemRqsFuncPtr)();
typedef NINT DllLoadDS 	(*SvSemClrFuncPtr)();
typedef NINT DllLoadDS 	(*SvSemWatFuncPtr)();

typedef NINT DllLoadDS 	(*SvTimRqsFuncPtr)();
typedef NINT DllLoadDS 	(*SvTimClrFuncPtr)();
typedef NINT DllLoadDS 	(*SvTimWatFuncPtr)();

typedef pTEXT       	(*SvMballcFuncPtr)();
typedef VOID        	(*SvMbfreeFuncPtr)();
typedef pTEXT       	(*SvGetMemFuncPtr)();
typedef VOID        	(*SvPutMemFuncPtr)();

typedef NINT    	(*SvWaitWhileFuncPtr)();
typedef VOID DllLoadDS 	(*SvRevObjFuncPtr)();
typedef NINT 		(*SvUsrStatFuncPtr)();

typedef	LONG DllLoadDS  (*SvGetSrVarFuncPtr)();
typedef LONG 		(*SvPrintfFuncPtr)();

#endif 	/* ~PROTOTYPE */

typedef struct SrvrFuncStruct {
SvDeferFuncPtr		SvDEFER;
SvCttcreFuncPtr		SvCTTCRE;

SvBlkrqsFuncPtr		SvBLKRQS;
SvBlkclrFuncPtr		SvBLKCLR;
SvBlkwatFuncPtr		SvBLKWAT;

SvSemRqsFuncPtr		SvSEMRQS;
SvSemClrFuncPtr		SvSEMCLR;
SvSemWatFuncPtr		SvSEMWAT;

SvTimRqsFuncPtr		SvTIMRQS;
SvTimClrFuncPtr		SvTIMCLR;
SvTimWatFuncPtr		SvTIMWAT;

SvMballcFuncPtr		SvMBALLC;
SvMbfreeFuncPtr		SvMBFREE;

SvGetMemFuncPtr		SvGETMEM;
SvPutMemFuncPtr		SvPUTMEM;

SvWaitWhileFuncPtr 	SvWAITWHILE;
SvRevObjFuncPtr		SvREVOBJ;
SvUsrStatFuncPtr	SvUSRSTAT;

SvGetSrVarFuncPtr 	SvGETSRVAR;
SvPrintfFuncPtr		SvPRINTF;
} SrvrFuncPtrs, ctMEM *pSrvrFuncPtrs;
/****************************************************************************/
#ifdef PROTOTYPE
typedef pCommFuncPtrs DllExport DllLoadDS (ctDllDecl *NpGlobalsFuncPtr)(pSrvrGbls psg);
#else																	
typedef pCommFuncPtrs DllExport DllLoadDS (ctDllDecl *NpGlobalsFuncPtr)();
#endif
/****************************************************************************/

/****************************************************************************/

#ifdef NO_BOUND
typedef struct lqmsg {
	VRLEN	lmlen[OUTSEG]; /* Protocol Byte Box */
	pMTEXT	lctusrbuf;
	pCommFuncPtrs lcommcmds;
	pUGRUP  lgptr;
	SEMA	lclnt;
	SEMA	lsrvr;
	LONG	lcpid;	/* communications protocol id */
	LONG	lupid;
	LONG	lmemu;
	LONG	lattr;
	LONG	lrsv1;
	LONG	lrsv2;
	VRLEN	lsize;
	COUNT	lerrc;
	COUNT	lmode;
	COUNT	lslct;
	UCOUNT	lhdl;
	UCOUNT	lrsvd;
	COUNT	ltask;
	UTEXT	lflvr;
	UTEXT	lalgn;
	TEXT	lusid[IDZ];
	TEXT	luswd[PWZ];
} LQMSG;

typedef struct {
	VRLEN	lmlen[OUTSEG];
	TEXT	lctusrbuf[4];
	TEXT	rab_xx_lcommcmds[4];
	TEXT	lgptr[4];
	TEXT	lclnt[8];
	TEXT	lsrvr[8];
	LONG	lcpid;	/* communications protocol id */
	LONG	lupid;
	LONG	lmemu;
	LONG	lattr;
	LONG	lrsv1;
	LONG	lrsv2;
	VRLEN	lsize;
	COUNT	lerrc;
	COUNT	lmode;
	COUNT	lslct;
	UCOUNT	lhdl;
	UCOUNT	lrsvd;
	COUNT	ltask;
	UTEXT	lflvr;
	UTEXT	lalgn;
	TEXT	lusid[IDZ];
	TEXT	luswd[PWZ];
} LOGON_REQUEST, ctMEM *pLOGON_REQUEST;

typedef struct lqmsg2 {
	TEXT	lnodname[IDZ];
	TEXT	ltickle;
	UTEXT	lpntr;
	UTEXT	lcbyt;	/* client ID byte */
	UTEXT	lrsv2;
	TEXT	pad[92];
} LQMSG2;

typedef struct lqrsp {
	LONG	divs;
	LONG	nusers;
	UTEXT	sflvr[8];
	UCOUNT	sndTimeOut;
	UCOUNT	recTimeOut;
	UTEXT	pad[108];
} LQRSP;

#ifdef ctSP
typedef LQMSG far   *	pLQMSG;
typedef LQMSG2 far   *	pLQMSG2;
typedef LQRSP far   *	pLQRSP;
#else
typedef LQMSG ctMEM *	pLQMSG;
typedef LQMSG2 ctMEM *	pLQMSG2;
typedef LQRSP ctMEM *	pLQRSP;
#endif

#ifdef VINESSEC
typedef struct stmsg {
	TEXT	susid[IDZ];
	TEXT	sgrp[IDZ];
	IPCPORT sport;
	ULONG	sstamp;
	ULONG	sdate;
	TEXT	sserver[6];
	TEXT	sfiller[2];
} STMSG;

#ifdef ctSP
typedef STMSG far   *	pSTMSG;
#else
typedef STMSG ctMEM *	pSTMSG;
#endif

#endif /* VINESSEC */

#endif /* NO_BOUND */

#define U_UID(t)	(ctiuser[t].tfilno + 1)
#define G_GID(t)	(ctigroup[t].tfilno + 1)
#define UG_UNM(t)	(ctiug[t].tfilno + 1)

#define	PutMessage	1
#define ctGetMessage	2
#define FnlMessage	3

#define DataTerminated		0x000001L
#define EndOfMessage		0x000002L
#define MustSendNow			0x000004L
#define RefIsMemory			0x000008L
#define MustBeData			0x000010L
#define WaitForCompletion	0x000020L
#define RefIsUnused			0x000040L

#define WaitEverPeekShtdwn (-31939) /* Unused unique value */

/****************************************************************************/
#define gSrvSema			pGlobals->srvSema
/*#define gIsWNE			pGlobals->isWNE */
/*#define gNewAT			pGlobals->newat */
#define gSignature			pGlobals->signature
#define gNbrSessions			pGlobals->nbrSessions
#define gMaxPacketSize			pGlobals->maxPacketSize
#define gSSess				pGlobals->sSess
#define gCSess				pGlobals->cSess
#define gCmds				(pGlobals->com_cmds)
#define gCmds2				pGlobals->com_cmds
#define gSName 				pGlobals->sName 
#define gCallBackCmds 			(pGlobals->srv_callBackCmds)
#define gCallBackCmds2 			pGlobals->srv_callBackCmds
#define ghSrvrGlobalData 		pGlobals->hServerGlobalData
#define gntpSession 		    	pGlobals->ntpSession
#define gntDllLoadHandle		pGlobals->ntDllLoadHandle
#define gntProtocolText			pGlobals->ProtocolText
#define gntShutDown			pGlobals->ntpShutDown
/****************************************************************************/

/* Network I/O Locations */
/****************/
#define ctNET_IO_APPX_WRITE 		(*(ctlqmsg->lcommcmds->NpWRITE))
#define ctNET_IO_TRAP_WRITE 		(*(lqmsg.lcommcmds->NpWRITE))
#define ctNET_IO_APPX_READ		(*(ctlqmsg->lcommcmds->NpREAD))
#define ctNET_IO_TRAP_READ		(*(lqmsg.lcommcmds->NpREAD))
#define ctNET_IO_APPX_CLOSE 		(*(lcommcmds->NpCLOSE))	
#define ctNET_IO_APPX_START		(*(pLqMsg->lcommcmds->NpSTART))
#define ctNET_IO_APPX_OPEN		(*(pLqMsg->lcommcmds->NpOPEN))
#define ctNET_IO_APPX_PACKSIZ		(*(pLqMsg->lcommcmds->NpPACKSIZ))
#define ctNET_IO_APPX_WRITE2 		(*(pLqMsg->lcommcmds->NpWRITE))
#define ctNET_IO_APPX_READ2		(*(pLqMsg->lcommcmds->NpREAD))
#define ctNET_IO_APPX_WRITE3		(*(pLqMsg->lcommcmds->NpWRITE))
#define ctNET_IO_APPX_READ3		(*(pLqMsg->lcommcmds->NpREAD))
#define ctNET_IO_APPX_GETATTR		(*(pLqMsg->lcommcmds->NpGETATTR))
#define ctNET_IO_APPX_SETATTR		(*(pLqMsg->lcommcmds->NpSETATTR))
#define ctNET_IO_APPX_GETATTR2		(*(pLqMsg->lcommcmds->NpGETATTR))
#define ctNET_IO_APPX_SETATTR2		(*(pLqMsg->lcommcmds->NpSETATTR))
/****************/

/****************/
#define ctNET_IO_LGON_WRITE		(*(l->lcommcmds->NpWRITE))
#define ctNET_IO_LGON_MAKE		(*(lcommcmds->NpMAKE))
#define ctNET_IO_LGON_CONNECT		(*(lcommcmds->NpCONNECT))
#define ctNET_IO_LGON_READ		(*(lcommcmds->NpREAD))
#define ctNET_IO_LGON_DISCONN		(*(lcommcmds->NpDISCONNECT))
#define ctNET_IO_LGON_READ2		(*(lcommcmds->NpREAD))
#define ctNET_IO_LGON_SETATTR		(*(lcommcmds->NpSETATTR))
#define ctNET_IO_LGON_WRITE2		(*(lcommcmds->NpWRITE))
#define ctNET_IO_LGON_DISCONN2		(*(lcommcmds->NpDISCONNECT))
/****************/

/****************/
#define ctNET_IO_NTIO_WRITE		(*(lqp->lcommcmds->NpWRITE))
#define ctNET_IO_NTIO_READ		(*(lqp->lcommcmds->NpREAD))
/****************/

/****************/
#define ctNET_IO_SUSR_DISCONN	   	(*(lqp->lcommcmds->NpDISCONNECT))
/****************/

/****************/
#define ctNET_IO_SRVR_START		(*commCmds[i]->NpSTART) 
/****************/

/****************/
#define ctNET_IO_ASUP_INIT		(*(cfp->NpINIT))
#define ctNET_IO_ASUP_STOP		(*(lcommcmds->NpSTOP))
#define ctNET_IO_ASUP_PACKSIZ		(*(ctlqmsg->lcommcmds->NpPACKSIZ))
/****************/

/****************/
#define ctNET_IO_SSUP_INIT		(*(cfp->NpINIT))
#define ctNET_IO_SSUP_STOP		(*(lcommcmds->NpSTOP))
#define ctNET_IO_SSUP_POLICE		(*(cfp->NpPOLICE))
/****************/					

/****************/					
#define ctNET_IO_CATEND_DISCONN		(*(cfp->NpDISCONNECT))
/****************/					

/****************************************************************************/
#endif /* ctCOMMH */

/* end of ctcomm.h */
