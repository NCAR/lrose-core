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

#define IU	0
#define IG	1
#define IUG	2

#ifndef ctUSRLMTMSG
#define ctUSRLMTMSG	26
#endif

static NINT rdycnt;
       SEMA ctmainwait;
      ULONG actkey[]  = {0x12345678L,0,0,0,0,0x87654321L};
      pTEXT ctadmmir;
static TEXT log_cug[] = "Created User/Group Information";
static TEXT log_uge[] = "Could not process User/Group Information";
static TEXT log_cre[] = "Delete FAIRCOM.FCS and restart server";

static TEXT	devlop1[] = "* * * * * * * * * * * * * * * * * * * * * * * * *";
static TEXT	devlop2[] = "    DEVELOPER EDITION ONLY. NOT FOR RESALE!";
static TEXT	devlop3[] = "    NOT LICENSED FOR PRODUCTION USE.";
static TEXT	devlop4[] = "    PRODUCTION SYSTEMS REQUIRE A VALID";
static TEXT	devlop5[] = "    SERIALIZED LICENSE FROM FAIRCOM.";
static TEXT	devlop6[] = "    THANK YOU FOR CHOOSING FAIRCOM.";
static TEXT	devlop7[] = "* * * * * * * * * * * * * * * * * * * * * * * * *";
static TEXT	DACnon[] = "DataFlex Server by FairCom(R) is Operational   -SN 00000000\n";
       TEXT	DACrts[] = "Startup DataFlex Server by FairCom(R) - V6.07.28c(Build-0802)";
static TEXT	signon[] = "FairCom(R) V6.07.28c Server Is Operational     -SN 00000000\n";
#ifdef ctPortBEGO
       TEXT	starts[] = "Startup Winnebago/FairCom(R) Server - V6.07.28c(Build-0802)  ";
#else
       TEXT	starts[] = "Startup FairCom(R) Server - V6.07.28c(Build-0802)            ";
#endif
static TEXT	snmsg[]  = "Invalid Serial Number. Please check with FairCom.            ";
       TEXT	srvcfg[] = "ctsrvr.cfg";
static TEXT	curpmg[] = "Warning: possible auto curp problem. Call FairCom. mfn";
static TEXT	usrlmt[] = "%d User Limit";
static TEXT	memuse[] = "System memory in use: ";
static TEXT	memtot[] = "System memory available: ";
static TEXT	initer[] = "Could not initialize server. Error";
static TEXT	logerr[] = "Could not establish logon area. Error";
static TEXT	cpyrgt[] = "Copyright 1984-1997 FairCom Corporation\nALL RIGHTS RESERVED.";
static TEXT	cfgint[] = "CONFIGURATION OPTION INTERNAL ERROR: Call FairCom...         ";
static TEXT	cfgprm[] = "DO NOT RECOGNIZE CONFIGURATION KEYWORD...";
static TEXT	cfgmem[] = "No memory available for configuration";
static TEXT	cfgdmp[] = "Could not schedule Dynamic Dump...";
static TEXT	cfgdok[] = "Dynamic Dump Scheduled...";
static TEXT	cfgfil[] = "Startup without configuration file";
static TEXT	unlic[]  = "SERVER NEEDS ACTIVATION KEY! Execute 'fcactvat' program first";
static TEXT	tmperr[] = "Server TMPNAME_PATH is not valid. Check server configuration.";
static TEXT	nomirs[] = "ALL MIRRORS TURNED OFF IN CONFIGURATION FILE";
static TEXT	uidmsg[] = "User ID file processing error";
static TEXT	ugnmsg[] = "User/Group file processing error";
static TEXT	ugcmsg[] = "User-Group file closing error";
#ifdef rtSRVR
#ifdef rtJAPAN
static TEXT	rtsmsg[] = "r-tree(R) Enabled Server - Japanese Support";
#else
static TEXT	rtsmsg[] = "r-tree(R) Enabled Server";
#endif
#endif /* rtSRVR */
#ifdef VINES				
static TEXT	cfgstk[] = "Request to StreetTalk failed.  Error";
static TEXT	cfgarc[] = "Startup without configuration information";
static TEXT	cfgnpr[] = "NO CONFIGURATION VALUE PROVIDED FOR KEYWORD";
#endif
#ifdef ctTRAP_COMM
static TEXT	ctTCmsg[] = "COMMUNICATIONS TRAPPING ENABLED";
#endif
static LONG	ctsrvdly = 10000L;	/* default 60 sec */
#ifdef ctEVALUATION
static TEXT 	evalsv2[] = "    EVALUATION COPY ONLY. NOT FOR RESALE!";
#endif

#ifndef __MWERKS__
long int atol();
#endif

#ifdef PROTOTYPE
extern  COUNT 	dyndmp(pVOID bufptr,VRLEN bufsiz);
		NINT  	ctntio(COUNT ,pTEXT ,pVAB ,pLQMSG );
LOCAL 	NINT 	nu_msg(pTEXT ,NINT );
#else
extern  COUNT 	dyndmp();
		NINT 	ctntio();
LOCAL 	NINT 	nu_msg();
#endif

#ifdef PROTOTYPE
extern 	VOID 	logonNamedPipe(UNLONG);
extern 	pTEXT 	GetPipeName(VOID);
extern 	ULONG 	WriteErrorCode(UCOUNT, pLQMSG);
extern 	NINT 	NewUser(pTEXT);
extern 	COUNT 	doserver(pLQMSG, pVOID);
extern 	VOID 	NewLogon(UNLONG);
#else
extern 	VOID 	logonNamedPipe();
extern 	pTEXT 	GetPipeName();
extern 	ULONG 	WriteErrorCode();
extern 	NINT 	NewUser();
extern 	COUNT 	doserver();
extern 	VOID 	NewLogon();
#endif

COUNT	cti_svvr = 3; /* server version: 2 hetero, 3 auto curp/lock */
NINT	ctmusers;
NINT	ctsqldebug;
pTEXT	ctsqlfile;
LONG	ct_sesout;
LONG	naptim=0L;
LONG	tctmemch;
LONG	ct_compflg;
LONG	ct_diagflg;
NINT	ct_diaginttot;
NINT	ct_diagintnum;
NINT	ct_diagstrtot;
NINT	ct_diagstrnum;
NINT	ct_ddtot;
NINT	ct_ddnum;
pLONG	ct_diagint;
ppTEXT	ct_diagstr;
ppTEXT	ct_ddscript;
#define DIAG_INCR	12

#ifdef ctSYNC_DELAY
LONG	ct_syncdelay;
#endif
NINT	ctsemmsl;
NINT	tasker_pc,tasker_sp,tasker_np;
NINT	tasker_loop,rqst_delay,rqst_delta;
NINT	ctlog9074;

extern LONG  ctqcnt[];
extern LONG  ctqlmt[];
extern LONG  nodedtime;
extern COUNT ct_mxu1;
extern LONG  ct_cmtdly;

#ifdef TRANPROC
extern LONG ct_trndlt;
extern LONG ct_cpdlt;
extern LONG ct_logprg;
#endif
#ifdef MTDEBUG
extern LONG tmpWAIT;
extern int bomb;
#endif

extern ctCONV COUNT (ctDECL *ctfptr[])();
extern ctCONV LONG  (ctDECL *ctlptr[])();
extern ctCONV VRLEN (ctDECL *ctvptr[])();
extern ctCONV pTEXT (ctDECL *cttptr[])();

NINT ctpf_stat;
static LONG pf_beg,pf_end,pf_cum;
static LONG pf_frq[CTI_MXFN + 1];

#define ctPFNAMES

LONG FM_cnt = 0L;

#ifdef ctPFNAMES
static char *tp_fnm;
char *pf_fnm[] = {
NULL,
"PUTFIL",
"UPDCURI",
"DELFIL",
"LKISAM",
"DELREC",
"ALCSET",
"CHGSET",
"DELVREC",
"CLISAM",
"STPUSR",
"FRESET",
"CLSFIL",
"TRANEND",
"TRANRST",
"TRANABT",
"TRANCLR",
"CLRFIL",
"DELRFIL",
"ALCBAT",
"CHGBAT",
"FREBAT",
"PERFORM",
"USERLIST",
"USERINFO",
"CTKLUSR",
"CTTESTFUNC",
"CHGICON",
"CLSICON",
"CTFLUSH",
"CTCHKPNT",
"TRANABTX",
"CHGHST",
"FREHST",
NULL,
NULL,
NULL,
"UPDCIDX",
NULL,
"ADDUSR",
"ADDREC",
"RWTREC",
"NXTREC",
"PRVREC",
"FRSREC",
"LSTREC",
"RRDREC",
"NXTSET",
"PRVSET",
"SETALTSEQ",
"GETALTSEQ",
"SETDEFBLK",
"MIDSET",
"PUTDODA",
"SETVARBYTS",
"FRSVSET",
"LSTVSET",
"MIDVSET",
NULL,
"SYSMON",
"WRTVREC",
"RDVREC",
"REDREC",
"WRTREC",
"SETCURI",
"RETREC",
"RETVREC",
"reset_cur",
"SETCURI",
"EQLVREC",
"GTEVREC",
"LTEVREC",
"GTVREC",
"LTVREC",
"GETMNAME",
"GETNAM",
"GETSEG",
"GETMAP",
"SECURITY",
NULL,
"FRSSET",
"LSTSET",
"EQLREC",
"GTEREC",
"LTEREC",
"GTREC",
"LTREC",
"REDIREC",
NULL,
"CHGUSR",
"CREISAM",
"OPNISAM",
NULL,
"DELIFIL",
"RBLIFIL",
"CREIFIL",
"CLIFIL",
"OPNIFIL",
"PRMIIDX",
"TMPIIDX",
"PUTIFIL",
"CMPIFIL",
NULL,
"RBLIIDX",
"NXTVREC",
"PRVVREC",
"FRSVREC",
"LSTVREC",
"NXTVSET",
"PRVVSET",
"INTREE",
"INTISAM",
NULL,
NULL,
NULL,
"SETNODE",
NULL,
NULL,
NULL,
NULL,
"ADDVREC",
"RWTVREC",
"REDVREC",
"DELRES",
"ENARES",
"UPDRES",
"ADDRES",
"PUTCRES",
NULL,
NULL,
"ADDKEY",
"LOADKEY",
"DELCHK",
NULL,
NULL,
NULL,
"SETOPS",
NULL,
NULL,
NULL,
"CREDAT",
"CREIDX",
"CREMEM",
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
"GTEKEY",
"GTKEY",
"LTKEY",
"LTEKEY",
"DELBLD",
"EQLKEY",
"FRSKEY",
"LSTKEY",
"GETCURP",
"NEWREC",
"DATENT",
"IDXENT",
"SERIALNUM",
"TRANBEG",
"NXTKEY",
"PRVKEY",
"GETRES",
"CTDIDX",
"SQLLOCK",
"CTUSER",
"GETFIL",
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
"GTVLEN",
"GETVLEN",
"GETIFIL",
"GETDODA",
"GETCRES",
"GETCIDX",
NULL,
NULL,
"TFRMKEY",
"GETCURK",
"GETCURKL",
NULL,
"CTSBLDX",
"TSTVREC",
"COMMBUF",
"SQR",
"OPNRFIL",
"TMPNAME",
"SQL",
"DYNDMP",
"FRCKEY",
"AVLFILNUM",
"OPNFIL",
"BATSET",
"LOKREC",
"ESTKEY",
"NEWVREC",
"TRANSAV",
"TRANBAK",
"SETFNDVAL",
"IOPERFORMANCE",
"CLNIDXX",
"RNGENT",
"ORDKEY",
"OPNICON",
"SYSCFG",
"PUTHDR",
"CUST_LOGON",
"CUST_LOGOFF",
"CUST_OPC1",
"CUST_OPC2",
"CUST_OPC3",
"CUST_OPC4",
"CTHIST",
"IOPERFORMANCEX",
"REDIVREC",
"reset_cur2",
"RTSCRIPT",
"NOFUNCT"
};
#endif
				/* ctTCread modes...			*/
#define ctTCmsg2	1	/* connect secondary handshake		*/
#define ctTCntio	2	/* regular read				*/
#define ctTCackn	3	/* regular write			*/
#define ctTCconn	4	/* client connect & first message	*/
#define ctTCdisc	5	/* client disconnects			*/
#define ctTCbegn	6	/* startup trap comm log		*/
#define ctTCfnsh	7	/* end log				*/

UCOUNT ctTCspcl[ctTCfnsh + 1] = {
		0,		/* unused				*/
		0,		/* ctTCmsg2				*/
		0,		/* ctTCntio				*/
		ctTC_ACKN,	/* ctTCackn				*/
		ctTC_CONN,	/* ctTCconn				*/
		ctTC_DISC,	/* ctTCdisc				*/
		ctTC_BEGN,	/* ctTCbegn				*/
		ctTC_FNSH	/* ctTCfnsh				*/
	};

/* end ctsrvr.h */
