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
 *	 		August 1, 1997
 */

#define P1_NO_LEN	0
#define P1_RCKLEN	1
#define P1_VRKLEN	2
#define P1_KEYLEN	3
#define P1_RECLEN	4
#define P1_VARLEN	5
#define P1_OPNLEN	6
#define P1_ISMLEN	7
#define P1_IFLLEN	8
#define P1_LOGLEN	9
#define P1_CRELEN	10
#define P1_ESTLEN	11
#define P1_TRNLEN	12
#define P1_KYSLEN	13
#define P2_KEYLEN	14
#define P2_RECLEN	15
#define P1_RKYLEN	16
#define P2_RKYLEN	17
#define P1_RORLEN	18
#define A2_RETFIL	19
#define A2_LOGOFF	20
#define P1_RESLEN	21
#define P2_VARLEN	22
#define P1_BATLEN	23
#define P1_SQLLEN	24
#define P1_SEQLEN	25
#define A2_FNCRET	26
#define P1_LNGLEN	27
#define P1_DEFLEN	28
#define P1_PADLEN	29
#define P1_DDXLEN	30
#define P1_PWZLEN	31
#define P1_SQRLEN	32
#define P1_COMLEN	33
#define P1_GIFLEN	34
#define P1_USRLLN	35
#define P1_USRINF	36
#define P1_IDZLEN	37
#define P1_IOPLEN	38
#define P1_STRING	39
#define P1_STRSHR	40
#define P1_CFGLEN	41
				/*
				** reserve 42 through 49 for ctCUSTOM use
				*/
#define P1_CUSOP1	42	/* ptr1 custom length */
#define P1_CUSOP2	43
#define P2_CUSOP2	44

#define P1_HSTLEN	50
#define P1_KVRLEN	51
#define P1_RVRLEN	52
#define P2_PVRLEN	53
#define P1_IOPXLN	54
#define P12_LAST	54	/* last valid inp/out typ value		*/

#define SP_SETCUR	0x0001	/* maintain local current ISAM pos	*/
#define SP_ZLEN		0x0002	/* zero output length on error		*/
#define SP_LOWVAR	0x0004	/* low level auto var length		*/
#define SP_CLSFIL	0x0008	/* close or deletes files		*/
#define SP_APPXFN	0x0010	/* special ctappx processing		*/
#define SP_CUST01	0x0020	/* special custom processing		*/
#define SP_CUST02	0x0040	/* special custom processing		*/
/*			0x0080	   future use				*/
#define SP_PLEN		0x0100	/* input/output plen argument		*/
#define SP_CHKMAP	0x0200	/* check segment & key maps		*/
#define SP_RTREE	0x0400	/* r-tree launch function		*/

#define SP_ISMRET	(SP_SETCUR | SP_ZLEN)
#define SP_ISMVAR	(SP_SETCUR | SP_PLEN)
#define SP_APPMAP	(SP_APPXFN | SP_CHKMAP)


typedef struct {
	COUNT	numarg;
	COUNT	inptyp;
	COUNT	outtyp;
	COUNT	spltyp;
} FNARRAY;

FNARRAY ctfunc[] = {
	{0},					/* NOFUNCT	*/
	{6},					/* PUTFIL	*/
	{6,0,0,SP_SETCUR},			/* UPDCURI	*/
	{1,0,0,SP_CLSFIL},			/* DELFIL	*/
	{1},					/* LKISAM	*/
	{1},					/* DELREC	*/
	{1},					/* ALCSET	*/
	{1},					/* CHGSET	*/
	{1},					/* DELVREC	*/
	{0,0,A2_LOGOFF,SP_CLSFIL},		/* CLISAM	*/
	{0,0,A2_LOGOFF},			/* STPUSR	*/
	{0},					/* FRESET	*/
	{1,0,A2_FNCRET,SP_CLSFIL},		/* CLSFIL	*/
	{1,0,0,SP_SETCUR},			/* TRANEND	*/
	{1,0,0,SP_SETCUR},			/* TRANRST	*/
	{0,0,0,SP_SETCUR},			/* TRANABT	*/
	{0},					/* TRANCLR	*/
	{1,0,A2_FNCRET,SP_CLSFIL | SP_APPMAP},	/* CLRFIL	*/
	{1,0,A2_FNCRET,SP_CLSFIL | SP_APPMAP},	/* DELRFIL	*/
	{1},					/* ALCBAT	*/
	{1},					/* CHGBAT	*/
	{0},					/* FREBAT	*/
	{1},					/* PERFORM	*/
	{2,0,P1_USRLLN},			/* USERLIST	*/
	{2,0,P1_USRINF},			/* USERINFO	*/
	{2,P1_IDZLEN},				/* CTKLUSR	*/
	{0},					/* CTTESTFUNC	*/
	{1,0,0,SP_SETCUR},			/* CHGICON	*/
	{1},					/* CLSICON	*/
	{1},					/* CTFLUSH	*/
	{0},					/* CTCHKPNT	*/
	{1,0,0,SP_SETCUR},			/* TRANABTX	*/
	{1},					/* CHGHST	*/
	{0},					/* FREHST	*/
	{0},					/* NOFUNCT	*/
	{0},					/* NOFUNCT	*/
	{2,P1_STRING},				/* ctRENFIL	*/
	{2,P1_STRING},				/* UPDCIDX	*/
	{2,P1_STRING},				/* SETFLTR	*/
	{2,P1_RCKLEN,0,SP_SETCUR | SP_APPMAP},	/* ADDUSR	*/
	{2,P1_RCKLEN,0,SP_SETCUR | SP_APPMAP},	/* ADDREC	*/
	{2,P1_RCKLEN,0,SP_SETCUR},		/* RWTREC	*/
	{2,0,P1_RORLEN,SP_ISMRET},		/* NXTREC	*/
	{2,0,P1_RORLEN,SP_ISMRET},		/* PRVREC	*/
	{2,0,P1_RORLEN,SP_ISMRET},		/* FRSREC	*/
	{2,0,P1_RORLEN,SP_ISMRET},		/* LSTREC	*/
	{2,0,P1_RECLEN,SP_ISMRET},		/* RRDREC	*/
	{2,0,P1_RKYLEN,SP_ISMRET},		/* NXTSET	*/
	{2,0,P1_RKYLEN,SP_ISMRET},		/* PRVSET	*/
	{2,P1_SEQLEN},				/* SETALTSEQ	*/
	{2,0,P1_SEQLEN},			/* GETALTSEQ	*/
	{2,P1_DEFLEN},				/* SETDEFBLK	*/
	{6,0,P1_RKYLEN,SP_ISMRET},		/* MIDSET	*/
	{6,0,0,SP_APPXFN},			/* ..PUTDODA..	*/
	{2,P1_PADLEN},				/* SETVARBYTS	*/
	{6,P1_KEYLEN,P2_PVRLEN,SP_ISMVAR},	/* FRSVSET	*/
	{6,P1_KEYLEN,P2_PVRLEN,SP_ISMVAR},	/* LSTVSET	*/
	{6,0,P1_KVRLEN,SP_ISMVAR},		/* MIDVSET	*/
	{0},					/* NOFUNCT	*/
	{5,0,P1_VARLEN},			/* SYSMON	*/
	{5,P1_VARLEN,0,SP_LOWVAR},		/* WRTVREC	*/
	{5,0,P1_VARLEN,SP_LOWVAR},		/* RDVREC	*/
	{3,0,P1_RECLEN,SP_LOWVAR | SP_ZLEN},	/* REDREC	*/
	{3,P1_RECLEN},				/* WRTREC	*/
	{5,P1_KYSLEN,0,SP_SETCUR},		/* SETCURI	*/
	{3},					/* RETREC	*/
	{3},					/* RETVREC	*/
	{3,0,P1_RKYLEN,SP_ISMRET},		/* reset_cur	*/
	{5,0,0,SP_SETCUR},			/* SETCURI2	*/
	{5,P1_KEYLEN,P2_PVRLEN,SP_ISMVAR},	/* EQLVREC	*/
	{5,P1_KEYLEN,P2_PVRLEN,SP_ISMVAR},	/* GTEVREC	*/
	{5,P1_KEYLEN,P2_PVRLEN,SP_ISMVAR},	/* LTEVREC	*/
	{5,P1_KEYLEN,P2_PVRLEN,SP_ISMVAR},	/* GTVREC	*/
	{5,P1_KEYLEN,P2_PVRLEN,SP_ISMVAR},	/* LTVREC	*/
	{6,0,P1_VARLEN},			/* GETMNAME	*/
	{6,0,P1_VARLEN},			/* GETNAM	*/
	{5,0,P1_VARLEN},			/* GETSEG	*/
	{5,0,P1_VARLEN},			/* GETMAP	*/
	{6,P1_VARLEN},				/* SECURITY	*/
	{0},					/* NOFUNCT	*/
	{6,P1_KEYLEN,P2_RKYLEN,SP_ISMRET},	/* FRSSET	*/
	{6,P1_KEYLEN,P2_RKYLEN,SP_ISMRET},	/* LSTSET	*/
	{4,P1_KEYLEN,P2_RKYLEN,SP_ISMRET},	/* EQLREC	*/
	{4,P1_KEYLEN,P2_RKYLEN,SP_ISMRET},	/* GTEREC	*/
	{4,P1_KEYLEN,P2_RKYLEN,SP_ISMRET},	/* LTEREC	*/
	{4,P1_KEYLEN,P2_RKYLEN,SP_ISMRET},	/* GTREC	*/
	{4,P1_KEYLEN,P2_RKYLEN,SP_ISMRET},	/* LTREC	*/
	{3,0,P1_RECLEN,SP_ISMRET},		/* REDIREC	*/
	{0},					/* NOFUNCT	*/
	{2,P1_PWZLEN,0,SP_SETCUR},		/* CHGUSR	*/
	{2,P1_ISMLEN,A2_RETFIL},		/* CREISAM	*/
	{2,P1_ISMLEN,A2_RETFIL},		/* OPNISAM	*/
	{0},					/* NOFUNCT	*/
	{2,P1_IFLLEN,0,SP_CLSFIL | SP_APPMAP},	/* DELIFIL	*/
	{2,P1_IFLLEN},				/* RBLIFIL	*/
	{2,P1_IFLLEN,A2_RETFIL},		/* CREIFIL	*/
	{2,P1_IFLLEN,0,SP_CLSFIL | SP_APPMAP},	/* CLIFIL	*/
	{2,P1_IFLLEN,A2_RETFIL},		/* OPNIFIL	*/
	{2,P1_IFLLEN,A2_RETFIL},		/* PRMIIDX	*/
	{2,P1_IFLLEN,A2_RETFIL},		/* TMPIIDX	*/
	{2,P1_IFLLEN},				/* PUTIFIL	*/
	{2,P1_IFLLEN},				/* CMPIFIL	*/
	{2,P1_IFLLEN},				/* EXPIFIL	*/
	{2,P1_IFLLEN},				/* RBLIIDX	*/
	{5,0,P1_RVRLEN,SP_ISMVAR},		/* NXTVREC	*/
	{5,0,P1_RVRLEN,SP_ISMVAR},		/* PRVVREC	*/
	{5,0,P1_RVRLEN,SP_ISMVAR},		/* FRSVREC	*/
	{5,0,P1_RVRLEN,SP_ISMVAR},		/* LSTVREC	*/
	{5,0,P1_KVRLEN,SP_ISMVAR},		/* NXTVSET	*/
	{5,0,P1_KVRLEN,SP_ISMVAR},		/* PRVVSET	*/
	{2,P1_LOGLEN},				/* INTREE	*/
	{2,P1_LOGLEN},				/* INTISAM	*/
	{0},					/* NOFUNCT	*/
	{0},					/* NOFUNCT	*/
	{0},					/* NOFUNCT	*/
	{2,P1_STRING},				/* SETNODE	*/
	{0},					/* NOFUNCT	*/
	{0},					/* NOFUNCT	*/
	{0},					/* NOFUNCT	*/
	{0},					/* NOFUNCT	*/
	{5,P1_VRKLEN,0,SP_SETCUR | SP_APPMAP},	/* ADDVREC	*/
	{5,P1_VRKLEN,0,SP_SETCUR},		/* RWTVREC	*/
	{5,0,P1_VARLEN,SP_SETCUR | SP_LOWVAR},	/* REDVREC	*/
	{2,P1_RESLEN},				/* DELRES	*/
	{1},					/* ENARES	*/
	{5,P1_VARLEN},				/* UPDRES	*/
	{5,P1_VARLEN},				/* ADDRES	*/
	{5,P1_VARLEN},				/* PUTCRES	*/
	{0},					/* NOFUNCT	*/
	{0},					/* NOFUNCT	*/
	{6,P1_KEYLEN},				/* ADDKEY	*/
	{6,P1_KEYLEN},				/* LOADKEY	*/
	{3,P1_KEYLEN},				/* DELCHK	*/
	{0},					/* NOFUNCT	*/
	{0},					/* NOFUNCT	*/
	{0},					/* NOFUNCT	*/
	{5,0,0,SP_APPXFN},			/* SETOPS	*/
	{0},					/* NOFUNCT	*/
	{0},					/* NOFUNCT	*/
	{0},					/* NOFUNCT	*/
	{2,P1_CRELEN,A2_RETFIL},		/* CREDAT	*/
	{2,P1_CRELEN,A2_RETFIL},		/* CREIDX	*/
	{2,P1_CRELEN,A2_RETFIL},		/* CREMEM	*/
	{0},					/* NOFUNCT	*/
	{0},					/* NOFUNCT	*/
	{0},					/* NOFUNCT	*/
	{0},					/* NOFUNCT	*/
	{0},					/* NOFUNCT	*/
	{0},					/* NOFUNCT	*/
	{0},					/* NOFUNCT	*/
	{4,P1_KEYLEN,P2_KEYLEN},		/* GTEKEY	*/
	{4,P1_KEYLEN,P2_KEYLEN},		/* GTKEY	*/
	{4,P1_KEYLEN,P2_KEYLEN},		/* LTKEY	*/
	{4,P1_KEYLEN,P2_KEYLEN},		/* LTEKEY	*/
	{3,P1_KEYLEN},				/* DELBLD	*/
	{3,P1_KEYLEN},				/* EQLKEY	*/
	{3,0,P1_KEYLEN},			/* FRSKEY	*/
	{3,0,P1_KEYLEN},			/* LSTKEY	*/
	{3,0,0,SP_APPXFN},			/* GETCURP	*/
	{3},					/* NEWREC	*/
	{3},					/* DATENT	*/
	{3},					/* IDXENT	*/
	{3},					/* SERIALNUM	*/
	{3},					/* TRANBEG	*/
	{3,0,P1_KEYLEN},			/* NXTKEY	*/
	{3,0,P1_KEYLEN},			/* PRVKEY	*/
	{6,P1_RESLEN,P2_VARLEN},		/* GETRES	*/
	{3,P1_DDXLEN},				/* ctdidx	*/
	{3},					/* SQLLOCK	*/
#ifdef ctPREV_66A3_CTUSER
	{3,P1_STRING},				/* CTUSER	*/
#else
	{5,P1_STRING,P2_VARLEN}, 	/* CTUSER	*/
#endif
	{6,0,0,SP_APPXFN},			/* GETFIL	*/
	{0},					/* NOFUNCL	*/
	{0},					/* NOFUNCL	*/
	{0},					/* NOFUNCL	*/
	{0},					/* NOFUNCL	*/
	{0},					/* NOFUNCL	*/
	{0},					/* NOFUNCL	*/
	{0},					/* NOFUNCL	*/
	{0},					/* NOFUNCL	*/
	{0},					/* NOFUNCL	*/
	{5,0,0,SP_LOWVAR},			/* GTVLEN	*/
	{5,0,0,SP_LOWVAR},			/* GETVLEN	*/
	{5,0,P1_GIFLEN},			/* GETIFIL	*/
	{6,0,P1_LNGLEN},			/* GETDODA	*/
	{5,0,P1_LNGLEN},			/* GETCRES	*/
	{5,0,P1_LNGLEN},			/* GETCIDX	*/
	{0},					/* NOFUNCV	*/
	{0},					/* NOFUNCV	*/
	{2,P1_KEYLEN,P1_KEYLEN,SP_APPXFN},	/* TFRMKEY	*/
	{5,0,P1_KEYLEN},			/* GETCURK	*/
	{2,0,P1_KEYLEN},			/* GETCURKL	*/
	{0},					/* NOFUNCV	*/
	{2,P1_OPNLEN},				/* CTSBLDX	*/
	{5},					/* TSTVREC	*/
	{5,P1_COMLEN},				/* COMMBUF	*/
	{6,P1_SQRLEN,0,SP_SETCUR},		/* SQR		*/
	{3,P1_OPNLEN,A2_RETFIL},		/* OPNRFIL	*/
	{5,P1_STRSHR,P1_VARLEN},		/* TMPNAME	*/
	{6,P1_SQLLEN,P2_VARLEN,SP_SETCUR},	/* SQL		*/
	{5,P1_VARLEN},				/* DYNDMP	*/
	{6,0,P1_KEYLEN},			/* FRCKEY	*/
	{2,0,P1_TRNLEN},			/* AVLFILNUM	*/
	{2,P1_OPNLEN,A2_RETFIL},		/* OPNFIL	*/
	{6,P1_BATLEN,P2_VARLEN},		/* BATSET	*/
	{6},					/* LOKREC	*/
	{3,P1_ESTLEN},				/* ESTKEY	*/
	{5},					/* NEWVREC	*/
	{2,0,P1_TRNLEN},			/* TRANSAV	*/
	{3,P1_TRNLEN},				/* TRANBAK	*/
	{5,P1_VARLEN},				/* SETFNDVAL	*/
	{2,0,P1_IOPLEN},			/* IOPERFORMANCE*/
	{2,P1_OPNLEN},				/* CLNIDXX	*/
	{3,P1_ESTLEN},				/* RNGENT	*/
	{5,P1_KEYLEN,P2_KEYLEN},		/* ORDKEY	*/
	{6,0,P1_TRNLEN},			/* OPNICON	*/
	{2,0,P1_CFGLEN},			/* SYSCFG	*/
	{6,0,0,SP_SETCUR},			/* PUTHDR	*/
	{3,P1_LOGLEN},				/* CUST_LOGON	*/
	{3,0,A2_LOGOFF,SP_CLSFIL},		/* CUST_LOGOFF	*/
	{3,P1_CUSOP1},				/* CUST_OPC1	*/
	{4,P1_CUSOP2,P2_CUSOP2},		/* CUST_OPC2	*/
	{6},					/* CUST_OPC3	*/
	{6},					/* CUST_OPC4	*/
	{6,P1_HSTLEN,P2_VARLEN,SP_APPXFN},	/* CTHIST	*/
	{2,0,P1_IOPXLN},			/* IOPERFORMANCEX*/
	{5,0,P1_RVRLEN,SP_ISMVAR},		/* REDIVREC	*/
	{5,0,P1_RVRLEN,SP_ISMVAR},		/* reset_cur2	*/
	{6,P1_STRING,P2_VARLEN,SP_RTREE},	/* RTSCRIPT	*/
	{0}					/* NOFUNCT	*/
	};

		/* end of ctfary.h */
