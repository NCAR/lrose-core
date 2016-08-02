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

#ifndef ctSTRCH
#define ctSTRCH

			/* Data Structures */
#include "ctsema.h"

#ifdef ctCUSTOM
typedef struct custmem {
	struct
	custmem	*toplink;
	struct
	custmem *botlink;
	pTEXT	 signature;
	} CUSTMEM;
typedef CUSTMEM ctMEM * pCUSTMEM;
#define ctCUSTmemhdr	ctSIZE(CUSTMEM)
#define ctCUSTmemsig	((pTEXT) 0x12344321)
#endif

#ifdef DBG749x
#define DBG749xSIG	0x796a5b4c
#define DBG749xUSE	0x68594a3b
typedef struct pctspc {
	union {
	pLONG	ptr;
	LONG	lng;
	} uctSPC;
	LONG	sig;	/* debug signature */
} pctSPC;
#else
typedef union pctspc {
	pLONG	ptr;
	LONG	lng;
} pctSPC;
#endif

typedef struct {
	TEXT	userid[IDZ];
	LONG	userstmp;
	LONG	usermem;
	LONG	userattr;
	TEXT	userword[PWZ];
	TEXT	userds[DSZ];
} FC_USER;
typedef FC_USER ctMEM *	pFC_USER;

typedef struct {
	TEXT	groupid[IDZ];
	LONG	groupstmp;
	LONG	groupmem;
	LONG	groupattr;
	TEXT	groupds[DSZ];
} FC_GROUP;

typedef struct {
	TEXT	userid[IDZ];
	TEXT	seqnum;
	TEXT	groupid[IDZ];
} FC_UG;

typedef struct batlst {
	struct
	batlst ctMEM
		*batlnk;
	union {
		LONG	 batpos;
		pTEXT	 batkyp;
	} u;
} BATLST;
typedef BATLST ctMEM *	pBATLST;

typedef struct bathdr {
	pBATLST	 sbathed;	/* retrieval list hdr		*/
	pBATLST	 sbatail;	/* retrieval list tail		*/
	pBATLST	 sbatlsp;	/* retrieval list current pos	*/
	pBATLST	 sbatmrk;	/* retrieval list perm mark	*/
	pVOID	 sbatrqp;	/* request pointer		*/
	pTEXT	 sbatcnu;	/* continuation buffer pointer	*/
	pTEXT	 sbatbfp;	/* buffer pointer		*/
	pTEXT	 sbatcrp;	/* current pointer		*/

	pVOID	 sbatrbp;	/* record buffer pointer	*/
	LONG	 sbattot;	/* total count			*/
	LONG	 sbatlkd;	/* number accessible		*/
	LONG	 sbatrem;	/* remaining to be processed	*/
	LONG	 sbatrs1;	/* reserved area #1		*/
	VRLEN	 sbatrbz;	/* record buffer size		*/
	VRLEN	 sbatbfz;	/* buffer size			*/
	VRLEN	 sbatrs2;	/* reserved area #2		*/
	COUNT	 sbatfil;	/* filno			*/
	COUNT	 sbatopc;	/* current function		*/
	COUNT	 sbatvry;	/* verify key/rec flag		*/
	COUNT	 sbatret;	/* return type			*/
	COUNT	 sbatlok;	/* lock type			*/
	COUNT	 sbatcom;	/* complete vs partial flag	*/
	COUNT	 sbatreq;	/* partial key vs filter flag	*/
	UCOUNT	 sbatulk;	/* unlock strategy		*/
	} BATHDR;
typedef BATHDR ctMEM *	pBATHDR;

#ifndef ctOldALCBAT
typedef struct savbat {
	struct savbat
		*sblnk;		/* save batch link */
	BATHDR	 sbhdr;		/* save batch header */
	COUNT	 sbnum;		/* save batch number */
	} SAVBAT;
typedef SAVBAT ctMEM *	pSAVBAT;
typedef SAVBAT ctMEM * ctMEM *	ppSAVBAT;
#endif

typedef struct {
	UCOUNT	repad;		/* alignment padding */
	UCOUNT	remrk;		/* record mark */
	VRLEN	retot;		/* total length */
	VRLEN	relen;		/* in-use length */
	LONG	resup;		/* super file member */
	LONG	reflk;		/* forward link */
	LONG	reblk;		/* back link */
	ULONG	retyp;		/* resource type */
	ULONG	renum;		/* resource number */
	TEXT	renam;		/* optional name */
	} RESHDR;
typedef RESHDR ctMEM *	pRESHDR;

typedef struct lstitm {
	struct
	lstitm ctMEM
	       *lstlnk;
	pTEXT	lstbuf;
} LSTITM;
typedef LSTITM ctMEM *	pLSTITM;
typedef LSTITM ctMEM * ctMEM * ppLSTITM;

typedef struct lsthed {
	struct
	lsthed ctMEM
	       *hedprv;
	struct
	lsthed ctMEM
	       *hednxt;
	struct
	lstanc ctMEM
	       *hedanc;
	pLSTITM hedstk;
	pTEXT   hednew;
	UCOUNT	hedcnt;
	UCOUNT	hedtot;
	UCOUNT	hedavl;
	UCOUNT	hedrsv;
} LSTHED;
typedef LSTHED ctMEM *	pLSTHED;
typedef LSTHED ctMEM * ctMEM * ppLSTHED;

typedef struct lstanc {
	pLSTHED	ancact;		/* active list */
	LONG	anctot;		/* total items */
	LONG	ancavl;		/* # of avl items */
	UINT	anclst;		/* # of lists */
	UINT	anclav;		/* # of list with avl items */
} LSTANC;
typedef LSTANC ctMEM *	pLSTANC;

/*
** exactly one and only one of the following must be defined:
**		ABTnint
**		ABTint2
**		ABTint4
*/
#ifndef ABTnint
#define ABTnint
#endif

#ifdef ABTint2
#undef ABTnint
#ifdef ABTint4
#undef ABTint4
#endif
#endif

#ifdef ABTint4
#undef ABTnint
#endif

typedef struct abtlst {
	struct
	abtlst ctMEM
	       *ablink;
	LONG	abpos;	/* ABNODSIZ is based on 4 fields being contiguous */
	LONG	abtran;
	LONG	abkey;
#ifdef ABTnint
	NINT	abnum;
#endif
#ifdef ABTint2
	COUNT	abnum;
#endif
#ifdef ABTint4
	LONG	abnum;
#endif
} ABTLST;
typedef ABTLST ctMEM *	pABTLST;
#ifdef ABTnint
#define ABNODSIZ	(3 * ctSIZE(LONG) + ctSIZE(NINT))
#endif
#ifdef ABTint2
#define ABNODSIZ	(3 * ctSIZE(LONG) + ctSIZE(COUNT))
#endif
#ifdef ABTint4
#define ABNODSIZ	(4 * ctSIZE(LONG))
#endif

typedef struct comlst {
	struct
	comlst ctMEM
	       *cmlink;
	LONG	cmtran;
} COMLST;
typedef COMLST ctMEM *	pCOMLST;
typedef COMLST ctMEM * ctMEM * ppCOMLST;

typedef struct dqmsg {
	LONG	dnode;
	LONG	dtime;
	COUNT	dmemb;
	COUNT	dflmd;
	COUNT	datmp;
	COUNT	drsvd;
	TEXT	dfile[MAX_NAME + 1];
} DQMSG;
typedef DQMSG ctMEM *	pDQMSG;

typedef struct dedlok {
	COUNT	blkdby;	/* blocked by user#   */
	COUNT	blking;	/* # of users blocked */
} DEDLOK;
typedef DEDLOK ctMEM *	pDEDLOK;

typedef struct ctconbuf {	/* context key value info	*/
	COUNT	cb_key;		/* keyno			*/
	COUNT	cb_len;		/* key length			*/
	TEXT	cb_flg;		/* key existence flag		*/
	TEXT	cb_rsv;		/* reserved			*/
	TEXT	cb_val[2];	/* key value buffer head	*/
	} ctCONBUF;
typedef ctCONBUF ctMEM * pctCONBUF;
#define ctCONBUFSIZ	ctSIZE(ctCONBUF)

typedef struct cticon {		/* ISAM context			*/
	struct cticon ctMEM
	       *iconptr;	/* context pointer (link)	*/
	pctCONBUF
		iconbuf;	/* alloc'd key value info	*/
	ctRECPT	iconpos;	/* context record position	*/
	LONG	iconrsv;	/* reserved			*/
	COUNT	iconid;		/* context ID			*/
	COUNT	icondat;	/* user datno			*/
	COUNT	iconkeys;	/* number of keys in context	*/
	COUNT	irsvd;		/* reserved			*/
	} ctICON;
typedef ctICON ctMEM *	pctICON;
typedef ctICON ctMEM * ctMEM * ppctICON;

typedef struct fusr {
	LONG	pmsk;	 /* permission mask		*/
	LONG	nent;	 /* incremental nument chg	*/
	LONG	curp;	 /* current byte position	*/
	LONG	curt;	 /* temporary byte position	*/
	LONG	curr;	 /* current resource position	*/
	LONG	retnod;	 /* current node position	*/
#ifdef ctSQL
	LONG	wlkcnt;	 /* write lock count		*/
#endif
	pTEXT   keyp;	 /* current key value ptr	*/
	pTEXT   keyt;	 /* temporary key value ptr	*/
	pTEXT   keyl;	 /* low level key result	*/
	pVOID	datflt;	 /* pointer to data filter	*/
	pctICON	concur;	 /* current context		*/
	NINT	retelm;	 /* current element within node */
	COUNT	flmp;	 /* user to internal file# map	*/
	COUNT	rsvd;
	TEXT	fupd;	 /* file update flag		*/
	TEXT	plen;	 /* keyp length flag		*/
	TEXT	tlen;	 /* keyt length flag		*/
	TEXT	supd;	 /* serialization update flag	*/
} FUSR;
typedef FUSR ctMEM *	pFUSR;
typedef FUSR ctMEM * ctMEM * ppFUSR;

typedef struct blklst {
	SEMA	semab;
	struct
	blklst ctMEM
	       *blklnk;
	struct
	reclok ctMEM
	       *blklok;
	pVOID	blkfcb;
	COUNT	blkusr;
	COUNT	blktyp;
#ifdef MTDEBUG
	TEXT	semtyp;
	TEXT	semnam[SNAMELEN];
#endif
	} BLKLST;
typedef BLKLST ctMEM *	pBLKLST;
typedef BLKLST ctMEM * ctMEM * ppBLKLST;

typedef struct bfhlst {
	struct bfhlst ctMEM *flnk;
	struct bfhlst ctMEM *blnk;
	pVOID		     hbuf;
	} BHL;
typedef BHL ctMEM *	pBHL;
typedef BHL ctMEM * ctMEM * ppBHL;

typedef struct datbuf {
	struct
	datbuf ctMEM
		*nxtbuf;
	pTEXT	 datorg;
	LONG	 datpos;
#ifdef MULTITRD
	LONG	 datpps;
	SEMA	 datsem;
#endif
#ifdef MULTITRD
	BHL	 datbhl[3];
#else
	BHL	 datbhl[2];
#endif
#ifdef TRANPROC
	LONG	 trstmp;
	LONG	 cpstmp;
	pCOMLST	 datcom;
#endif
	VRLEN	 dathgh;
	COUNT	 datfil;
#ifdef MULTITRD
	COUNT	 datpfn;
#endif
	COUNT	 datupd;
} DATBUF;
typedef DATBUF ctMEM *	pDATBUF;

#define BEGCTL		phyrec
#define iostart(ctnum)	((pTEXT) &ctnum->phyrec)

typedef struct wfils2 {	/* Partial File Control Structure	*/
	ctRECPT	phyrec;	/* last byte offset of file		*/
	ctRECPT	delstk;	/* top of delete stack: fixed len data	*/
	ctRECPT	numrec;	/* last byte offset written		*/
	LONG	reshdr;	/* resource header			*/
	LONG	lstmbr;	/* last super file member/position	*/
	ctRECPT	sernum;	/* serial number			*/
} CTFILE2;
typedef CTFILE2 ctMEM *	pCTFILE2;

typedef struct wfils3 {	/* Partial File Control Structure		*/
	ctRECPT	phyrec;	/* last byte offset of file		00x	*/
	ctRECPT	delstk;	/* top of delete stack: fixed len data	04x	*/
	ctRECPT	numrec;	/* last byte offset written		08x	*/
	LONG	reshdr;	/* resource header			0cx	*/
	LONG	lstmbr;	/* last super file member/position	10x	*/
	ctRECPT	sernum;	/* serial number			14x	*/
	ctRECPT nument;	/* active entries			18x	*/
	LONG    root;	/* B-Tree root 				1cx	*/
	LONG	fileid;	/* unique file id			20x	*/
	LONG	servid;	/* unique server id			24x	*/
	COUNT	verson;	/* configuration options at create	28x	*/
	UCOUNT	recsiz;	/* node record size			2ax	*/
	UCOUNT	reclen;	/* data record length			2cx	*/
	UCOUNT	extsiz;	/* extend file (chunk) size		2ex	*/
	UCOUNT	flmode;	/* file mode (virtual, etc)		30x	*/
	UCOUNT	logtyp; /* permanent components of file mode	32x	*/
	UCOUNT	maxkbl;	/* maximum key bytes leaf-var		34x	*/
	UCOUNT	maxkbn;	/* maximum key bytes non leaf-var 	36x	*/
	TEXT	updflg;	/* update (corrupt) flag		38x	*/
	TEXT	ktype;	/* file type flag			39x	*/
	TEXT	autodup;/* duplicate flag			3ax	*/
	TEXT	deltyp; /* flag for type of idx delete		3bx	*/
	UTEXT	keypad; /* padding byte				3cx	*/
	UTEXT	flflvr;	/* file flavor				3dx	*/
	UTEXT	flalgn;	/* file alignment			3ex	*/
	UTEXT	flpntr;	/* file pointer size			3fx	*/
	UCOUNT	clstyp; /* flag for file type			40x	*/
	COUNT   length;	/* key length 				42x	*/
	COUNT	nmem;	/* number of members			44x	*/
	COUNT	kmem;	/* member number			46x	*/
	LONG	lanchr;	/* left most leaf anchor		48x	*/
	LONG	supid;	/* super file member #			4cx	*/
	LONG	hdrpos;	/* header position			50x	*/
	LONG	sihdr;	/* superfile header index hdr position	54x	*/
	LONG	timeid;	/* time id#				58x	*/
	UCOUNT	suptyp;	/* super file type 			5cx	*/
	UCOUNT	maxmrk;	/* maximum exc mark entries per leaf	5ex	*/
	UCOUNT	namlen;	/* MAX_NAME at creation			60x	*/
	UCOUNT	xflmod; /* extended file mode info		62x	*/
	LONG	defrel;	/* file def release mask		64x	*/
	LONG	hghtrn;	/* tran# high water mark for idx	68x	*/
	LONG	hdrseq; /* wrthdr sequence #			6cx	*/
	LONG	tstamp; /* update time stamp			70x	*/
	LONG	rs3[3];	/* future use				74x	*/
} CTFILE3;
typedef CTFILE3 ctMEM *	pCTFILE3;

typedef struct wfils {	/* File Control Structure 		*/
	struct wfils ctMEM
	       *nxtfcb;	/* CTFILE link				*/
	ctRECPT	phyrec;	/* last byte offset of file		*/
	ctRECPT	delstk;	/* top of delete stack: fixed len data	*/
	ctRECPT	numrec;	/* last byte offset written		*/
	LONG	reshdr;	/* resource header			*/
	LONG	lstmbr;	/* last super file member/position	*/
	ctRECPT	sernum;	/* serial number			*/
	ctRECPT nument;	/* active entries			*/
	LONG    root;	/* B-Tree root 				*/
	LONG	fileid;	/* unique file id			*/
	LONG	servid;	/* unique server id			*/
	COUNT	verson;	/* configuration options at create	*/
	UCOUNT	recsiz;	/* node record size			*/
	UCOUNT	reclen;	/* data record length			*/
	UCOUNT	extsiz;	/* extend file (chunk) size		*/
	UCOUNT	flmode;	/* file mode (virtual, etc)		*/
	UCOUNT	logtyp; /* permanent components of file mode	*/
	UCOUNT	maxkbl;	/* maximum key bytes leaf-var		*/
	UCOUNT	maxkbn;	/* maximum key bytes non leaf-var 	*/
	TEXT	updflg;	/* update (corrupt) flag		*/
	TEXT	ktype;	/* file type flag			*/
	TEXT	autodup;/* duplicate flag			*/
	TEXT	deltyp; /* flag for type of idx delete		*/
	UTEXT	keypad; /* padding byte				*/
	UTEXT	flflvr;	/* file flavor				*/
	UTEXT	flalgn;	/* file alignment			*/
	UTEXT	flpntr;	/* file pointer size			*/
	UCOUNT	clstyp; /* flag for file type			*/
	COUNT   length;	/* key length 				*/
	COUNT	nmem;	/* number of members			*/
	COUNT	kmem;	/* member number			*/
	LONG	lanchr;	/* left most leaf anchor		*/
	LONG	supid;	/* super file member #			*/
	LONG	hdrpos;	/* header position			*/
	LONG	sihdr;	/* superfile header index hdr position	*/
	LONG	timeid;	/* time id#				*/
	UCOUNT	suptyp;	/* super file type 			*/
	UCOUNT	maxmrk;	/* maximum exc mark entries per leaf	*/
	UCOUNT	namlen;	/* MAX_NAME at creation			*/
	UCOUNT	xflmod; /* extended file mode info		*/
	LONG	defrel;	/* file def release mask		*/
	LONG	hghtrn;	/* tran# high water mark for idx	*/
	LONG	hdrseq; /* wrthdr sequence #			*/
	LONG	tstamp;	/* update time stamp			*/
	LONG	rs3[3];

	/* end of permanent information */

	LONG	hdroff;	/* index member header offset		*/
	LONG	nodstk;	/* temporary index delete stk ptr	*/
	LONG	botstk; /* temporary index delete stk ptr	*/
	LONG	tmptrn;	/* temporary tran# high water mark	*/
	LONG	tflmod; /* temporary file attributes		*/
#ifdef ctNogloTran
	pCOMLST	ixlist; /* vulnerable node list head ptr	*/
	LONG	lstent;	/* nument at last clean point		*/
	LONG	loghnd;	/* unique id entered into tran log	*/
	LONG	dellog;	/* delete file high water mark log#	*/
	LONG	delpos;	/*   "     "    "     "    "   position	*/
	COUNT	omitcp;	/* omit from checkpoint (flag)		*/
	COUNT	resrv1;
#endif
#ifdef ctSQL
	LONG	wlkcnt;	/* write lock count			*/
#endif
#ifndef MULTITRD
	LONG   	retnod;	/* current node				*/
	LONG	nent;	/* incremental chg to nument		*/
	LONG	curp;	/* current byte position		*/
	LONG	curt;	/* temporary byte position		*/
	LONG	curr;	/* current resource position		*/
	pTEXT   keyp;	/* current key value ptr		*/
	pTEXT   keyt;	/* temporary key value ptr		*/
	pTEXT   keyl;	/* low level key result			*/
	pVOID	datflt;	/* pointer to data filter		*/
	pctICON	concur;	/* current context			*/
#else /* MULTITRD */
	SEMA	semaf;	/* permanent info semaphore		*/
	SEMA	thdrf;	/* temporary info semaphore		*/
	SEMA	iosema[2 * NUMCHANEL];
			/* file i/o semaphore			*/
	struct reclok ctMEM
	      **rlokh;	/* head of record locks			*/
	struct reclok ctMEM
	      **rlokt;	/* tail of record locks			*/
	struct wfils ctMEM
	       *hmem;	/* host member ptr			*/
	struct wfils ctMEM
	       *xmem;	/* next member ptr			*/
#ifdef DBGhash
	pLONG	dbg_lhsh_f;
#endif

#endif /* MULTITRD */
	struct wfils ctMEM
	       *psuper;	/* ptr to super file control struct	*/
	struct wfils ctMEM
	       *puse;	/* prev link for use list		*/
	struct wfils ctMEM
	       *nuse;	/* next link for use list		*/
	pVOID	idxflt;	/* pointer to index filter		*/
	pABTLST ablist; /* node abort list pointer		*/
	pCOUNT  altseq;	/* alternative sequence			*/
	pConvMap schema;/* record schema Map			*/
#ifdef ctMIRROR
	pTEXT	mflname;/* pointer to mirror file name		*/
	RNDFILE	mfd;
	NINT	mstatus;/* mirror status			*/
#else
#ifdef ctLOCLIB
	pTEXT	mflname;
	RNDFILE	mfd;
	NINT	mstatus;
#endif
#endif /* ctMIRROR */
#ifdef MULTITRD
	RNDFILE	afd[2 * NUMCHANEL];
			/* file descritor			*/
#ifdef ctNogloTran
	NINT	dmpflg; /* track clean dyn dump for non-TRNLOG	*/
			/* during recovery, show clean TRNLOG	*/
#endif
	NINT	numchn;	/* number of channels			*/
	UINT	toggle; /* multi channel toggle			*/
#else
	RNDFILE	ctfd;	/* file descritor			*/
	NINT	retelm;	/* current node element			*/
#ifdef ctNogloTran
	NINT	dmpflg;
#endif
#endif
	UINT	hshshf;	/* hash shift factor			*/
	UCOUNT	schlen;	/* Map length				*/
	COUNT	usrcnt;	/* number of users with file open	*/
	COUNT	filnum;	/* file number assigned by user		*/
	UCOUNT	srlpos;	/* offset to serial # in record (ISAM)	*/
	UCOUNT	rcvcre;	/* recovery create flag			*/
	UCOUNT	rcvwrt;	/* recovery update flag			*/
#ifdef MULTITRD

#else
	UCOUNT	fupd;	/* per transaction update flag		*/
	UCOUNT	plen;	/* keyp length flag			*/
	UCOUNT	tlen;	/* keyt length flag			*/
#endif
#ifdef FPUTFGET
	UCOUNT	savprm;	/* save virtual/permanent info		*/
	NINT	lokcnt;
	NINT	tstmbr;
	LONG	uniqid;
#else
#ifdef ctLOCLIB
	UCOUNT	savprm;
	NINT	lokcnt;
	NINT	tstmbr;
	LONG	uniqid;
#endif
#endif /* FPUTFGET */
	TEXT	chnacs;	/* 'y' 'n' 'v'				*/
	TEXT	flname[MAX_NAME];	/* file name		*/
} CTFILE;
typedef CTFILE ctMEM *	pCTFILE;
typedef CTFILE ctMEM * ctMEM * ppCTFILE;
#define ctkey(filno)	ctfcbhdr[(filno)]
#define	DATFILE		CTFILE
#define	KEYFILE		CTFILE
#define	VATFILE		CTFILE

#ifdef MULTITRD
#define ctfd	afd[0]
typedef struct ctstatV {
	COUNT	uec;	/* uerr_cod	*/
	COUNT	sec;	/* sysiocod	*/
	COUNT	iec;	/* isam_err	*/
	COUNT	ifl;	/* isam_fil	*/
	COUNT	cec;	/* cndxerr	*/
	COUNT	avl;	/* unused	*/
	COUNT	ql1;	/* sql_status1	*/
	COUNT	ql2;	/* sql_status2	*/
	} CTSTATV;
#endif

typedef struct trebuf {	/* B-Tree Buffer			*/
	struct trebuf ctMEM
	       *nxtbuf;
	pTEXT   nodorg;	/* pointer to node ct_origin for I/O	*/
	LONG	nodeid; /* node number				*/
#ifdef MULTITRD
	LONG	pnodeid;/* pending node id			*/
#ifdef TRANPROC
	BHL	bhl[4];	/* buffer hash list pointers		*/
#else
	BHL	bhl[3];	/* buffer hash list pointers		*/
#endif
	SEMA	semaf;	/* buffer semaphore			*/
	pBLKLST bblkh;	/* block list header			*/
	pBLKLST bblkt;	/* block list tail			*/
#else
#ifdef TRANPROC
	BHL	bhl[3];	/* buffer hash list pointers		*/
#else
	BHL	bhl[2];	/* buffer hash list pointers		*/
#endif
#endif
#ifdef TRANPROC
	LONG	trstmp; /* oldest update tran#			*/
	LONG	cpstmp; /* oldest update tran#			*/
	LONG	hghtrn;	/* tran# high water mark for idx	*/
#ifdef ctLOGIDX
	NINT	mrkupd;	/* vulnerable mark update flag		*/
	LONG	watlog; /* log number   high water mark		*/
	LONG	watpos; /* log position high water mark		*/
	pCOMLST lglist; /* pending log index   list stack ptr	*/
#endif
	pCOMLST cmlist;	/* pending tran commit list stack ptr	*/
	pCOMLST ablist;	/* pending tran abort  list stack ptr	*/
#endif
	pCTFILE	keynm;
	pTEXT   mrkpos;
	pTEXT   ct_kyval;/* pointer to key values for node	*/
#ifdef MULTITRD
	COUNT	pkeyid; /* pending key id			*/
#endif
	COUNT	keyid;	/* key number				*/
	COUNT	hstid;	/* host key number			*/
	COUNT   klen;	/* key length 				*/
	UCOUNT  maxb;	/* maximum bytes per node		*/
	UCOUNT	maxmrk;	/* maximum exc mark list entries	*/
	NINT	ktipe;	/* key type compression flags ONLY	*/
	UINT	begbyt;	/* beginning byte of cur var len ct_key	*/
	NINT	actlen;	/* stored length of cur var len ct_key	*/
	NINT	lstpos;	/* cur var len key pos#			*/
	NINT	cursfx;	/* cur var len key suffix count		*/
	TEXT	keyexp[MAXLEN + 4]; /* var key - expansion	*/
	TEXT	update;	/* node update indicator 'y/n' 		*/
	TEXT	confg;	/* regular or duplicate ct_key leaf	*/

/* beginning of permanent node */

	LONG    sucesr;	/* right sibling 		0x00	*/
	LONG    predsr;	/* left sibling 		0x04	*/
	UCOUNT  nkv;	/* number of key values 	0x08	*/
	UCOUNT  nkb;	/* number of key bytes		0x0a	*/
	COUNT	numexc;	/* number of exc marks		0x0c	*/
	COUNT	lowexc;	/* max-min pos of 1st mark	0x0e	*/
	TEXT	bmem;	/* member number		0x10	*/
	TEXT	leaf;	/* leaf indicator LEAF/NOLEAF	0x11	*/
} TREEBUFF;
typedef TREEBUFF ctMEM *	pTREBUF;

typedef struct indxadds {
	LONG	predsr;
	LONG	sucesr;
	TEXT	keyval[MAXLEN];
} INDXADDs;
typedef INDXADDs ctMEM *	pINDXADDs;

typedef struct lok {	/* ISAM Lock Table Entry		*/
	struct lok ctMEM
	       *nxtlok;
	COUNT   datfnm;	/* data file number			*/
	UTEXT	ltltyp;	/* lock type				*/
	UTEXT	ltisam; /* isam level lock flag			*/
	ctRECPT recnum;	/* record number			*/
#ifdef ctMTFPG_LOKCNF
	LONG    datuid;	/* unique data file id			*/
#endif /* ctMTFPG_LOKCNF */
} LOKS;
typedef LOKS ctMEM *	pLOKS;
typedef LOKS ctMEM * ctMEM * ppLOKS;

typedef struct vrfmt {	/* beginning of var len records */
	UCOUNT	vpadng;	/* padding for alignment	*/
	UCOUNT	recmrk;	/* record marker		*/
	VRLEN	trclen;	/* total record length		*/
	VRLEN	urclen;	/* utilized record length	*/
	/* super file and resource auxiliary info	*/
	LONG	supmbr;	/* super file member id		*/
	LONG	phylnk;	/* member sequential link	*/
} VHDR;
typedef VHDR ctMEM *	pVHDR;

typedef struct hstlst {
	struct hstlst ctMEM
		       *listlnk;
	LONG		listtrn;
	LONG		listnum;
} HSTLST;
typedef  HSTLST ctMEM *	 pHSTLST;
typedef pHSTLST ctMEM *	ppHSTLST;

typedef struct hsthdr {
	RESHDR		cth_rhdr;	/* var rec hdr (resource also)	    */
	pHSTLST		cth_ntrnlst;	/* list of previously visited trans */ 
	pHSTLST		cth_reclst;	/* list of detail recbyts (by index)*/
	pCTFILE		ctHnum;
	pCTFILE		cth_ctnum;
	pCTFILE		cth_dnum;
	pTEXT		cth_target;	/* current target pointer	    */
	pTEXT		cth_idxval;	/* current image (ctHISTindx)	    */
	pTEXT		cth_image;	/* current image (ctHISTdata)	    */
	ppTEXT		cth_filnam;	/* file name array		    */
	ppTEXT		cth_curuid;
	ppTEXT		cth_curnnm;
	ppTEXT		cth_stpuid;
	ppTEXT		cth_stpnnm;
	pTEXT		cth_ustr;	/* user/node name target string	    */
	pVOID		rs1;
	pVOID		rs2;
	ctRECPT		cth_pos;	/* current target position	    */
	ctRECPT		cth_imgpos;	/* current data image position	    */
	LONG		cth_lognum;	/* current log number		    */
	LONG		cth_logpos;	/* current log offset		    */
	LONG		cth_ctfil;	/* current file handle		    */
	LONG		cth_dtfil;
	LONG		cth_begnum;	/* beginning log number		    */
	LONG		cth_begpos;	/* beginning log offset		    */
	LONG		cth_ctfilb;	/* beginning file handle	    */
	LONG		cth_dtfilb;
	LONG		cth_stpnum;	/* stop at log number		    */
	LONG		cth_stppos;	/* stop at log offset		    */
	LONG		cth_ctfils;	/* stop file handle		    */
	LONG		cth_dtfils;
	LONG		cth_tranno;
	LONG		cth_recbyt;
	LONG		cth_chgbyt;
	LONG		cth_lasttran;
	LONG		cth_lasttime;
	LONG		cth_oldlg;
	LONG		cth_oldlp;
	LONG		cth_nxtpos;
	LONG		cth_imagll;	/* imag low  log		    */
	LONG		cth_imaglp;	/* imag low  pos		    */
	LONG		cth_imaghl;	/* imag high log		    */
	LONG		cth_imaghp;	/* imag high pos		    */
	LONG		cth_imagsl;	/* imag stop log		    */
	LONG		cth_imagsp;	/* imag stop pos		    */
	LONG		rs3;
	LONG		rs4;
	VRLEN		cth_szimage;	/* size of image		    */
	VRLEN		cth_varadj;
	UINT		cth_cusr2;	/* maximum users (cur)		    */
	UINT		cth_susr2;	/* maximum users (stp)		    */
	UINT		cth_cseqn;	/* user ID sequence # (cur)	    */
	UINT		cth_sseqn;	/* user ID sequence # (stp)	    */
	UINT		cth_mode;	/* permanent session attributes	    */
	NINT		cth_start;	/* NO, YES init, HYS retrieval	    */
	NINT		cth_filno;
	NINT		cth_datno;
	NINT		cth_sin;	/* NO index, YES index, HYS idx+dat */
	NINT		cth_klen;
	NINT		cth_klenwo;
	NINT		cth_sflag;	/* search flag			    */
	NINT		cth_tfildb;
	NINT		cth_imagdb;
	NINT		cth_assoc;	/* offset between rechdr and image  */
	NINT		rs5;
	NINT		rs6;
	} HSTHDR;

typedef struct savhst {
	struct savhst
		*shlnk;		/* save history link */
	HSTHDR	 shhdr;		/* save history header */
	COUNT	 shnum;		/* save history number */
	} SAVHST;
typedef SAVHST ctMEM *	pSAVHST;
typedef SAVHST ctMEM * ctMEM *	ppSAVHST;

#define ctHISTkeep	((pCTFILE) 1)

#ifndef ctMEMHDR
/* define ctMEMHDR in ctcmpl.h if default causes alignment problems */
#define ctMEMHDR	ctSIZE(pctSPC)
#endif

typedef struct pi1_s {
	pctSPC	 pi1lnk;
	TEXT	 pi1[PI_UNIT];
} PI1LST;

typedef struct pi2_s {
	pctSPC	 pi2lnk;
	TEXT	 pi2[2 * PI_UNIT];
} PI2LST;

typedef struct pi4_s {
	pctSPC	 pi4lnk;
	TEXT	 pi4[4 * PI_UNIT];
} PI4LST;

typedef struct pi8_s {
	pctSPC	 pi8lnk;
	TEXT	 pi8[8 * PI_UNIT];
} PI8LST;

typedef struct piw_s {
	pctSPC	piwlnk;
	TEXT	piw[16 * PI_UNIT];
} PIwLST;

typedef struct pix_s {
	pctSPC	pixlnk;
	TEXT	pix[32 * PI_UNIT];
} PIxLST;

typedef struct piy_s {
	pctSPC	piylnk;
	TEXT	piy[64 * PI_UNIT];
} PIyLST;

typedef struct piz_s {
	pctSPC	pizlnk;
	TEXT	piz[128 * PI_UNIT];
} PIzLST;

typedef struct reclok {
	struct reclok ctMEM
		*rllink;/* forward link				*/
	struct reclok ctMEM
		*rlrlnk;/* reverse link				*/
	ctRECPT	rlbpos;	/* byte position			*/
	pBLKLST rlblkh;	/* block list head			*/
	pBLKLST rlblkt; /* block list tail			*/
	COUNT	rlusrn;	/* user number of owner			*/
	COUNT	rltype;	/* write lock or read lock		*/
} RECLOK;
typedef RECLOK ctMEM *	pRECLOK;
typedef RECLOK ctMEM * ctMEM * ppRECLOK;

#ifdef ctOldALCSET
typedef struct savset {		/* Multiple-Set Buffer		*/
	COUNT	qlen;		/* significant length		*/
	COUNT	qkey;		/* sequence keyno + 1		*/
	TEXT	qbuf[MAXLEN];	/* ct_key buffer		*/
#ifdef MUSTFRCE
	TEXT	qold[MAXLEN];
#endif
	} SAVSET;
typedef SAVSET ctMEM *	pSAVSET;
#else
typedef struct savset {		/* Multiple-Set Buffer		*/
	struct savset
	       *qlnk;		/* link field for hashing	*/
	COUNT	qnum;		/* save set number		*/
	COUNT	qlen;		/* significant length		*/
	COUNT	qkey;		/* sequence keyno + 1		*/
	TEXT	qbuf[MAXLEN];	/* ct_key buffer		*/
#ifdef MUSTFRCE
	TEXT	qold[MAXLEN];
#endif
	} SAVSET;
typedef SAVSET ctMEM *	pSAVSET;
typedef SAVSET ctMEM * ctMEM *	ppSAVSET;
#endif

#ifdef ctCONDIDX
typedef struct cidxhdr {	/* conditional index header */
	COUNT	relkey;
	COUNT	explen;
	LONG	resrvd;
	} CIDXHDR;
#endif

typedef struct ddmphdr {
	TEXT	ddesc[128];
	TEXT	dfnam[512];
	LONG	dchnk;
	LONG	ddtim;
	LONG	dsize;
	LONG	doffs;
	LONG	dftyp;
	LONG	dctim;
	LONG	dcntl[32];
	TEXT	dpadg[1256];
	} DDMPHDR;
typedef DDMPHDR ctMEM *	pDDMPHDR;

#include "ctifil.h"
#include "ctvrec.h"

#endif /* ctSTRCH */

/* end of ctstrc.h */
