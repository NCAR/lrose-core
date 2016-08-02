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

#ifndef ctPORTH
#define ctPORTH
#include "ctlfun.h"

#define PERFORM_OFF	0
#define PERFORM_ON	1
#define PERFORM_DUMP	2

#ifndef FAST
#define	FAST	register    			/* register variable	   */
#endif
#ifndef PFAST
#define PFAST	register			/* register parameter	   */
#endif
#ifndef EXTERN
#define	EXTERN	extern				/* External variable	   */
#endif
#ifndef GLOBAL
#define	GLOBAL	/* */				/* Global variable	   */
#endif
#ifndef LOCAL
#define	LOCAL	static				/* Local to module	   */
#endif

#ifdef ctBAN5 /* Banyan 5.5 toolkit */
#undef VOID 
#endif

#define VOID	void

#ifndef ctEXPORT
#define ctEXPORT
#endif

typedef VOID ctMEM *	pVOID;
typedef VOID ctMEM * ctMEM *	ppVOID;
typedef short int	COUNT;
typedef COUNT ctMEM *	pCOUNT;
typedef COUNT ctMEM * ctMEM *	ppCOUNT;
typedef unsigned short int	UCOUNT; /* some 16 bit compilers might  */
typedef UCOUNT ctMEM *	pUCOUNT;
				/* require unsigned int instead */
#ifndef SYS_NINT
typedef int		NINT;	/* natural integer */
#endif
#ifndef SYS_UINT
typedef unsigned int	UINT;
#endif

typedef NINT ctMEM *	pNINT;
typedef NINT ctMEM * ctMEM *	ppNINT;
typedef UINT ctMEM *	pUINT;

typedef long int	NLONG;
typedef unsigned long	UNLONG;
typedef UNLONG ctMEM * pUNLONG;

#ifndef SYS_LONG
#ifndef LONG
typedef long int	LONG;
#endif
#endif
typedef LONG ctMEM *	pLONG;

#ifndef SYS_ULONG
#ifndef ULONG
typedef unsigned long	ULONG;
#endif
#endif
typedef ULONG ctMEM *	pULONG;

typedef LONG		VRLEN;
typedef VRLEN ctMEM *	pVRLEN;

typedef LONG		IOSIZE;
typedef IOSIZE ctMEM *	pIOSIZE;
typedef char		TEXT;
typedef TEXT ctMEM *	pTEXT;
typedef TEXT ctMEM * ctMEM * ppTEXT;

#ifndef __PROGRAM_COUNTER_TYPE__
#define __PROGRAM_COUNTER_TYPE__ UINT
#endif

typedef __PROGRAM_COUNTER_TYPE__ PROGRAM_COUNTER;

#ifndef ctProcPtr
#define ctProcPtr
#ifndef ctPortMAC
#ifdef PROTOTYPE
typedef NINT (*ProcPtr)(VOID);
#else
typedef NINT (*ProcPtr)();
#endif
#endif /* ~ctPortMAC */
#endif

#ifndef ctDllDecl
#define ctDllDecl
#endif

typedef FILE *		pFILE;
#ifdef CT_ANSI
typedef pFILE		RNDFILE;
#else
#ifndef ctRNDFILE
typedef int		RNDFILE;
#endif
typedef RNDFILE ctMEM *	pRNDFILE;
#endif

typedef unsigned char	UTEXT;	/* if unsigned char not supported, make char */
typedef UTEXT ctMEM *	pUTEXT;
typedef UTEXT ctMEM * ctMEM * ppUTEXT;
typedef LONG		POINTER,DATET; 
typedef POINTER ctMEM *	pPOINTER;
typedef char	       *TEXTPTR;
#define ctRECPT		LONG
#define pctRECPT	pLONG

#ifdef YES
#undef YES
#endif
#ifdef NO
#undef NO
#endif
#ifdef HYS
#undef HYS
#endif
#define HYS	2
#define	YES	1			/* TRUE	 */
#define	NO	0			/* FALSE */

#define FOREVER	for (;;)

#ifndef ctUPF
#define ctUPF
#define USERPRF_NTKEY	0x0001		/* DO NOT PERFORM AUTO TFRMKEY	*/
#define USERPRF_SAVENV	0x0002		/* SAVENV MODE FOR TRANSACTIONS	*/
#define USERPRF_SQL	0x0004		/* ENABLE SQL SUPPORT		*/
#define USERPRF_SERIAL	0x0008		/* ENABLE STRICT SERIALIZATION	*/
#define USERPRF_MEMABS	0x0010		/* INTERNAL MEMORY MANAGEMENT	*/
#define USERPRF_NDATA	0x0020		/* DO NOT PERFORM AUTO DATA -	*/
					/* 	UNIFRMAT CONVERSION	*/
#define USERPRF_LOCLIB	0x0040		/* USE A LOCAL LIBRARY: not srvr*/
#define USERPRF_PTHTMP	0x0080		/* add tmpname to input path	*/
					/* otherwise use system tmpname */
#define USERPRF_CODCNV	0x0100		/* auto language conversion	*/
#define USERPRF_CLRCHK	0x0200		/* clear transaction logs	*/
#define USERPRF_CUSTOM	0x0400		/* custom server application	*/
#endif /* ctUPF */

#define SYSMON_MAIN	0		/* mirror alerts & dynamic dumps*/
#define SYSMON_OFF	99

#define SHADOW		0x0010		/* shadow file		     */
#define LOGFIL		0x0020		/* log file (=> shadow)	     */
#define TWOFASE		0x0040		/* two phase transaction     */
#define PENDERR		0x0080		/* pending error	     */
#define OVRFASE		0x0001		/* tranovr processing	     */
#define CIPFASE		0x0002		/* promote WRL to WXL phase  */
#define SAVENV		0x0100		/* save environment	     */
#define AUTOTRN		0x0200		/* automatic transaction     */
#define LKSTATE		0x0400		/* LKISAM enabled	     */
#define DELUPDT		0x0800		/* => ct_usrix is for delete */
#define DEFERCP		0x1000		/* defer checkpoint	     */
#define AUTOSAVE	0x2000		/* automatic savepoints	     */
#define COMMIT_SWAP	0x4000		/* TRANEND/TRANBEG on swap   */

#define SAVECTREE	1
#define RESTORECTREE	2

#define RES_TYPNUM	2
#define RES_TYPE	4
#define RES_FIRST	8
#define	RES_NAME	16
#define RES_POS		32
#define RES_LENGTH	64
#define RES_LOCK	1
#define RES_NEXT	128
#define RES_UNAVL	256

#define BAT_CAN		0x0001
#define BAT_NXT		0x0002
#define BAT_GET		0x0003
#define BAT_DEL		0x0004
#define BAT_UPD		0x0005
#define BAT_INS		0x0006
#define BAT_OPC_RESV	0x0007

#define BAT_PKEY	0x0000
#define BAT_RESV1	0x0008
#define BAT_VERIFY	0x0010
#define BAT_RET_REC	0x0020
#define BAT_RET_POS	0x0040
#define BAT_RET_KEY	0x0080
#define BAT_GKEY	0x0100
#define BAT_RPOS	0x0200
#define BAT_KEYS	0x0400
#define BAT_LOK_RED	0x0800
#define BAT_LOK_WRT	0x1000
#define BAT_COMPLETE	0x2000
#define BAT_FLTR	0x4000
#define BAT_LOK_KEEP	0x8000

#define EXCLUSIVE 	0x0000		/* file in locked mode	     */
#define SHARED		0x0001		/* file in shared mode	     */
#define	VIRTUAL		0x0000		/* file is virtually opened  */
#define	PERMANENT	0x0002		/* file is physically opened */
#define	ctFIXED		0x0000		/* fixed length data	     */
#define	VLENGTH		0x0004		/* variable length data	     */
#define READFIL		0x0008		/* read only file lock	     */
#define PREIMG		SHADOW		/* transactions w/o recovery */
#define TRNLOG		(LOGFIL | SHADOW) /* PREIMG + recovery	     */
#define WRITETHRU	0x0040		/* write thru buffering	     */
#define CHECKLOCK	0x0080		/* must own lock for update  */
#define DUPCHANEL	0x0100		/* two i/o channels	     */
#define SUPERFILE	0x0200		/* superfile shell	     */
#define CHECKREAD	0x0400		/* must own lock on read     */
#define DISABLERES	0x0800		/* disable resource on create*/
#define MIRROR_SKP	0x2000		/* OK to open primary w/o mir*/
#define OPENCRPT	0x4000		/* open corrupt file	     */
#define LOGIDX		0x8000		/* log index upper level     */

#define OPF_NONE	0x00001L
#define OPF_READ	0x00002L
#define OPF_WRITE	0x00004L
#define OPF_DEF		0x00008L
#define OPF_DELETE	0x00010L
#define OPF_ALL		(OPF_READ | OPF_WRITE | OPF_DEF | OPF_DELETE)

#define GPF_NONE	0x00020L
#define GPF_READ	0x00040L
#define GPF_WRITE	0x00080L
#define GPF_DEF		0x00100L
#define GPF_DELETE	0x00200L
#define GPF_ALL		(GPF_READ | GPF_WRITE | GPF_DEF | GPF_DELETE)

#define WPF_NONE	0x00400L
#define WPF_READ	0x00800L
#define WPF_WRITE	0x01000L
#define WPF_DEF		0x02000L
#define WPF_DELETE	0x04000L
#define WPF_ALL		(WPF_READ | WPF_WRITE | WPF_DEF | WPF_DELETE)

#define OPF_NOPASS	0x08000L /* read only without password */
#define GPF_NOPASS	0x10000L
#define WPF_NOPASS	0x20000L

#define OPF_ISAMKOFF	0x40000L /* suppress full ISAM key buffer support */

#define NONEXCLUSIVE	(READFIL | SHARED)
#define COMPLETE	EXCLUSIVE
#define PARTIAL		SHARED
#define NOTREUSE	0x0010		/* key type modifier: !reuse nodes  */

#define REGADD   	0	/* See ADDKEY: split full nodes in half     */
#define INCADD   	1	/* split nodes to handle increasing values  */
#define DECADD   	2	/* split nodes to handle decreasing values  */

#define FRSADD		0	/* See LOADKEY: first call to LOADKEY	    */
#define NXTADD		1	/* subsequent calls to LOADKEY		    */
#define BLDADD		2	/* call to signal completion of LOADKEY	    */

#ifndef ctLKIMDS
#define ctLKIMDS
#define	FREE		0	/* free all pending data record locks	    */
#define RESET		1	/* same as FREE followed by ENABLE, etc.    */
#define ENABLE		2	/* enable non-blocking data record locks    */
#define ENABLE_BLK	3	/* enable blocking data record locks	    */
#define READREC		4	/* enable read only record locks	    */
#define SUSPEND		5	/* temporarily suspend enable mode	    */
#define RESTORE		6	/* restore enable mode			    */
#define RESTRED		7	/* restore read lock mode		    */
#define RESTORE_BLK	8	/* restore enable_blk mode		    */
#define ctKEEP		9	/* no affect: as if LKISAM not called	    */
#define FREE_TRAN	10	/* used by TRANEND to signal FREE from tran */
#define READREC_BLK	11
#define ctLK_RSV	12
#define ctLK_RSV_BLK	13
#define RESTRED_BLK	14
#define RESTRSV		15
#define RESTRSV_BLK	16
#define SS_LOCK		17
#define FREE_FILE	18	/* free all user locks for given data file  */
#define ctLKMD_RSV	8
#define LK_BLOCK	1
#define TRNBEGLK	(ENABLE | READREC | ctLKMD_RSV | LK_BLOCK)
#endif

#define ctKEEP_OUT	41	/* keep locks obtained outside of tran and
				   not touched inside of tran		   */
#define ctKEEP_OUT_ALL	42	/* keep locks obtained outside of tran,
				   whether or not touched in transaction   */

#define ctISAMLOCK	1	/* isam level lock			   */
#define ctNEWRECFLG	2	/* new record flag			   */
#define ctTRANLOCK	4	/* lock obtained or touched in transaction */
#define ctTRANOUT	8	/* lock obtained outside of transaction	   */

#define RECLEN		0
#define KEYLEN		1
#define FILTYP		2
#define FILMOD		3
#define REVMAP		4
#define KEYTYP		5
#define KEYDUP		6
#define LOGSIZ		10
#define PHYSIZ		11
#define NODSIZ		12
#define KEYMEM		13
#define KEYPAD		14
#define FLDELM		15
#define RECPAD		16
#define MIRRST		17
#define RELKEY		18
#define PERMSK		19
#define FILDEF		20
#define ALIGNM		21
#define EXTSIZ		22
#define SUPTYP		23
#define SUPMBR		24

#define DATA_FILE	0
#define INDX_FILE	1
#define VDAT_FILE	2

#define NON_SUPER	0
#define HST_SUPER	1
#define MBR_SUPER	2
#define IDX_SUPER	6

#define FILNAM		0
#define FRSFLD		1
#define LSTFLD		2
#define IDXNAM		3
#define MIRNAM		4
#define OWNNAM		5
#define GRPNAM		6

#define ctSERNUMhdr	1
#define ctTSTAMPhdr	2
#define ctNUMENThdr	3
#define ctTIMEIDhdr	4
#define ctLOGEOFhdr	5
#define ctPHYEOFhdr	6
#define ctDELSTKhdr	7

#define ctALIGNhdr	17
#define ctFLAVORhdr	18
#define ctXFLSEThdr	19
#define ctXFL_ONhdr	20
#define ctXFLOFFhdr	21
#define ctEXTSIZhdr	22

#define ctUSERhdr	32	/* this bit signifies non-permanent, user only*/
#define ctISAMKBUFhdr	33

#define CHKNUL		'\1'

/* Transaction Modes */

#ifndef ctNONE
#define ctNONE		0
#else /* for backward compatibility */
#define NONE		0
#endif

#define RCVMOD		0
#define BAKMOD		1
#define FWDMOD		2
#define MBRMOD		3

#define TRNNUM		0
#define TRNTIM		1

#define SAVCURI		0
#define RSTCURI		1
#define SWTCURI		2

/* Isam Segment Modes */

#define REGSEG		0	/* no modification		*/
#define INTSEG		1	/* unsigned integer		*/
#define UREGSEG		2	/* upper case translation	*/
#define SRLSEG		3	/* 4 byte auto serial number	*/
#define VARSEG		4	/* no mod. var length field	*/
#define UVARSEG		5	/* upper case. var length field	*/
#define YOURSEG1	6	/* reserved for your use	*/
#define YOURSEG2	7	/* reserved for your use	*/
#define SGNSEG		8	/* signed integer segment	*/
#define FLTSEG		9	/* floating point segment	*/
#define DECSEG		10	/* SQL BCD segment		*/
#define BCDSEG		11	/* reserved for future use	*/
#define SCHSEG		12	/* field number from schema	*/
#define USCHSEG		13	/* upper case field # from sch	*/
#define VSCHSEG		14	/* fixed len treated as var	*/
#define UVSCHSEG	15	/* VSCHSEG + upper case		*/
#define XTDSEG		256	/* beginning of xtd segments	*/
#define SEGMSK		0x010f	/* permits a seg mode range 0-15
				   and 256-271 */
#define DSCSEG		0x0010	/* mode + 16 for descending	*/
#define ALTSEG		0x0020	/* mode + 32 for alternate seq	*/
#define ENDSEG		0x0040	/* reverses bytes in segment to */
				/* permit end of field searches	*/
#define RSVSEG		0x0080  /* reserve segment modifier */


#define OPS_ONCE_LOK	0x0004		/* 2 byte values to pass in rqst*/
#define OPS_ONCE_BLK	0x0020
#define OPS_RSVD_2B2	0x0040		/* reserved for future use	*/
#define OPS_RSVD_2B3	0x0080
#define OPS_RSVD_2B4	0x0100

#define OPS_STATE_OFF	0x00000000
#define OPS_STATE_SET	0x00000001
#define OPS_STATE_ON	0x00000002
#define OPS_STATE_RET	0x00000003
#define OPS_STATE_VRET	0x00000004

#define OPS_UNLOCK_ADD	0x00000001	/* auto unlock on add		*/
#define OPS_UNLOCK_RWT	0x00000002	/* auto unlock on rewrite	*/
#define OPS_UNLOCK_UPD	(OPS_UNLOCK_ADD | OPS_UNLOCK_RWT)
#define OPS_LOCKON_GET	((LONG) OPS_ONCE_LOK) /* lock next fetch only	*/
#define OPS_VARLEN_CMB	0x00000008	/* get var len on one fetch	*/
#define OPS_SERVER_SHT	0x00000010	/* server is shutting down	*/
#define OPS_LOCKON_BLK	((LONG) OPS_ONCE_BLK) /* lock next fetch only	*/
					/* RSVD_2B2			*/
					/* RSVD_2B3			*/
					/* RSVD_2B4			*/
#define OPS_ADMOPN	0x00000200	/* ADMIN forced open (OPNFIL)	*/
#define OPS_OMITCP	0x00000400	/* omit files from checkpoint	*/
#define OPS_SKPDAT	0x00000800	/* replaces SKIPDATA file mode	*/
#define OPS_MIRROR_NOSWITCH 0x00001000	/* don't continue operation if  */
					/* mirror or primary fails.	*/

#define OPS_CLIENT_TRM	0x00002000	/* server has been terminated	*/
#define OPS_MIRROR_TRM	0x00004000	/* a primary or mirror has been */
					/* shutdown			*/
#define OPS_FUNCTION_MON 0x00008000	/* toggle function monitor	*/
#define OPS_LOCK_MON	0x00010000	/* toggle lock monitor		*/
#define OPS_TRACK_MON	0x00020000	/* toggle memory track monitor	*/
#define OPS_AUTOISAM_TRN 0x00040000	/* automatic ISAM transactions	*/
#define OPS_SERIAL_UPD	0x00080000	/* update serial# in SERIALNUM	*/
#define OPS_MEMORY_SWP	0x00100000	/* memory swapping active	*/
#define OPS_COMMIT_SWP	0x00200000	/* auto commit on swap occurred	*/
/* USE THIS VALUE NEXT	0x00400000					*/

#define OPS_once	(OPS_ONCE_LOK | OPS_ONCE_BLK)
#define OPS_lockon	(OPS_LOCKON_GET | OPS_LOCKON_BLK)
#define OPS_monitors	(OPS_FUNCTION_MON | OPS_LOCK_MON | OPS_TRACK_MON)
#define OPS_internal	(OPS_SERVER_SHT	| OPS_ADMOPN | OPS_OMITCP | OPS_SKPDAT | OPS_CLIENT_TRM)
#define OPS_server	(OPS_COMMIT_SWP | OPS_SERVER_SHT | OPS_CLIENT_TRM | OPS_MIRROR_TRM | OPS_MEMORY_SWP)
			/*
			** the above states are set on server side (for
			** client/server applications)
			*/
#define OPS_permanent	(OPS_SERVER_SHT | OPS_CLIENT_TRM | OPS_MIRROR_TRM | OPS_MEMORY_SWP)
			/*
			** the above states not to be cleared by a SET zero.
			*/


#define DEF_IFIL	0
#define DEF_MAP		1
#define DEF_NAMES	2
#define DEF_SQL1	3
#define DEF_SQL2	4
#define DEF_SQL3	5
#define DEF_DTREE1	6
#define DEF_DTREE2	7 
#define DEF_DTREE3	8 
#define DEF_NATLNG1	9
#define DEF_NATLNG2	10
#define DEF_NATLNG3	11
#define DEF_RESRVD1	12
/*
** ................	..
*/
#define DEF_RESRVD20	31
#define DEF_NUMBER	32

#define FCRES_DATA	1
#define FCRES_SCRT	2
#define FCRES_IDX	3
#define FCRES_SQL	4
#define FCRES_CIDX	5
#define FCRNAM_LEN	8

/* synonyms */

#define tfrmkey		TFRMKEY
#define alcset		ALCSET
#define chgset		CHGSET

#define FNSYSABS	1
#define FNSRVDIR	2
#define FNLOCSRV	3

typedef struct ctinit1 {
	COUNT	 initype;	/* MUST BE SET TO ONE (1) */
	COUNT	 maxuser;	/* maximum number of users */
	LONG	 logspac;	/* space for log files in MB */
	LONG	 totmem;	/* total memory (0 => no limit) */
	LONG	 usrmem;	/* default limit per user (0 => no limit) */
	LONG	 coopcmt;	/* co-operative commit factor */
	LONG	 idxcmem;	/* index buffer memory */
	LONG	 datcmem;	/* data buffer memory */
	pTEXT	 srvdirc;	/* server file directory */
	pTEXT	 evenstr;	/* even start file */
	pTEXT	 odd_str;	/* odd start file */
	pTEXT	 evenlog;	/* even log files */
	pTEXT	 odd_log;	/* odd log files */
	pTEXT	 preimgf;	/* pre-image swap file prefix */
	pTEXT	 srvrnam;	/* server logical name */
	UINT	 idxbufs;	/* number of index buffers */
	UINT	 datbufs;	/* number of data buffers */
	COUNT	 maxfile;	/* maximum number of data files and indices */
	COUNT	 bufsect;	/* 128 sectors per buffer */
	COUNT	 reservd;
	COUNT	 filname;	/* file name strategy:	0 no modification,
				** FNSYSABS		1 change to system abs,
				** FNSRVDIR		2 prepend srv direct,
				** FNLOCSRV		3 same as 2, but only
				**			  for mbopen\mbcrat
				*/
	LONG	 reservl;
	pTEXT	 evenstrm;	/* even start file */
	pTEXT	 odd_strm;	/* odd start file */
	pTEXT	 evenlogm;	/* even log files */
	pTEXT	 odd_logm;	/* odd log files */
} CTINIT1;
typedef CTINIT1 ctMEM *	pCTINIT1;

typedef struct ctinit {
	pTEXT	evenstr;	/* START_EVEN			*/
	pTEXT	odd_str;	/* START_ODD			*/
	pTEXT	evenlog;	/* LOG_EVEN			*/
	pTEXT	odd_log;	/* LOG_ODD			*/
	pTEXT	preimgf;	/* PREIMAGE_FILE		*/
	pTEXT	evenstrm;	/* START_EVEN_MIRROR		*/
	pTEXT	odd_strm;	/* START_ODD_MIRROR		*/
	pTEXT	evenlogm;	/* LOG_EVEN_MIRROR		*/
	pTEXT	odd_logm;	/* LOG_ODD_MIRROR		*/

	pTEXT	resrvt0;
	pTEXT	resrvt1;
	pTEXT	resrvt2;
	pTEXT	resrvt3;
	pTEXT	resrvt4;

	LONG	logspac;	/* LOG_SPACE			*/
	LONG	idxcmem;	/* IDX_MEMORY			*/
	LONG	datcmem;	/* DAT_MEMORY			*/
	LONG	nodedel;	/* NODE_DELAY			*/
	LONG	statsiz;	/* CTSTATUS_SIZE		*/
	LONG	chkidle;	/* CHECKPOINT_IDLE		*/
	LONG	chkintv;	/* CHECKPOINT_INTERVAL		*/
	LONG	memmont;	/* MEMORY_MONITOR		*/
	LONG	trkmont;	/* MEMORY_TRACK			*/
	LONG	lokmont;	/* LOCK_MONITOR			*/
	LONG	coopcmt;	/* COMMIT_DELAY			*/

	LONG	resrvl0;
	LONG	resrvl1;
	LONG	resrvl2;
	LONG	resrvl3;
	LONG	resrvl4;

	NINT	maxuser;	/* USERS			*/
	NINT	maxfile;	/* FILES			*/
	NINT	rcvfile;	/* RECOVER_FILES		*/
	NINT	pagesiz;	/* PAGE_SIZE			*/
	NINT	keeplog;	/* KEEP_LOGS			*/
	NINT	maxdkey;	/* MAX_DAT_KEY			*/
	NINT	maxkseg;	/* MAX_KEY_SEG			*/
	NINT	skpfile;	/* SKIP_MISSING_FILES		*/
	NINT	skpmfil;	/* SKIP_MISSING_MIRRORS		*/
	NINT	mirrors;	/* MIRRORS			*/
	NINT	flogidx;	/* FORCE_LOGIDX			*/
	NINT	chkmont;	/* CHECKPOINT_MONITOR		*/
	NINT	dedmont;	/* DEALOCK_MONITOR		*/
	NINT	maxvrtf;	/* MAX_VIRTUAL_FILES		*/
	NINT	fixdlog;	/* FIXED_LOG_SIZE		*/
	NINT	langflg;	/* LANGUAGE			*/

	NINT	resrvi0;
	NINT	resrvi1;
	NINT	resrvi2;
	NINT	resrvi3;
	NINT	resrvi4;
} ctINIT;
typedef ctINIT ctMEM *	pctINIT;

typedef struct pkeyreq {
	LONG	btotal;		/* total in set			*/
	LONG	bavail;		/* number available		*/
	LONG	breturn;	/* number returned		*/
	COUNT	siglen;		/* significant partial key len	*/
	TEXT	target[255];	/* partial key target		*/
	} PKEYREQ;
typedef PKEYREQ ctMEM *	pPKEYREQ;
#define PKEYLEN	(3 * ctSIZE(LONG) + ctSIZE(COUNT))

typedef struct usrprf {
	COUNT	utaskid;	/* internal task id		*/
	COUNT	uactflg;	/* active request indicator	*/
	TEXT	uname[32];	/* user id string		*/
	TEXT	ucominfo[32];	/* communication info		*/
	TEXT	urqst[16];	/* last request function name	*/
	LONG	umemsum;	/* user memory			*/
	LONG	ulogtim;	/* logon time			*/
	LONG	ucurtim;	/* current time			*/
	LONG	urqstim;	/* time of last request		*/
	LONG	utrntim;	/* time of last TRANBEG		*/
	LONG	unbrfil;	/* number of open files		*/
	LONG	urqstfn;	/* last request function #	*/
	TEXT	unodnam[32];	/* node ID information		*/
	} USRPRF;
typedef USRPRF ctMEM * pUSRPRF;
#define USRLSTSIZ	256

#define ALTSEQSIZ	256
#define ALTSEQBYT	(ALTSEQSIZ * ctSIZE(COUNT))

typedef struct {
	LONG	blkoff;		/* offset from beginning of resource	*/
	UCOUNT	blklen;		/* total length    			*/
	UCOUNT	blkutl;		/* utilized length 			*/
	} DEFBLK;
typedef DEFBLK ctMEM *	pDEFBLK;

typedef struct {
	ULONG	restyp;
	ULONG	resnum;
	TEXT	resnam[FCRNAM_LEN];
	LONG	reslen;
	LONG	restim;
	DEFBLK	resblk[DEF_NUMBER];
	} RESDEF;
typedef RESDEF ctMEM *	pRESDEF;

typedef struct defdef {
	pVOID	 ddefptr;	/* pointer to definition data	*/
	VRLEN	 ddeflen;	/* actual length		*/
	VRLEN	 ddeftot;	/* total length including slack */
	COUNT	 ddefnum;	/* DEF_xxxx or IDX kmem		*/
	COUNT	 ddeftyp;	/* data or index definition	*/
	} DEFDEF;
typedef DEFDEF ctMEM *	pDEFDEF;

typedef struct defdefz {
	LONG	 ddefptr;	/* pointer to definition data	*/
	VRLEN	 ddeflen;	/* actual length		*/
	VRLEN	 ddeftot;	/* total length including slack */
	COUNT	 ddefnum;	/* DEF_xxxx or IDX kmem		*/
	COUNT	 ddeftyp;	/* data or index definition	*/
	} DEFDEFz;

typedef struct defdefw {
	LONG	 ddefptr;	/* pointer to definition data	*/
	LONG	 ddefptr2;
	VRLEN	 ddeflen;	/* actual length		*/
	VRLEN	 ddeftot;	/* total length including slack */
	COUNT	 ddefnum;	/* DEF_xxxx or IDX kmem		*/
	COUNT	 ddeftyp;	/* data or index definition	*/
	LONG	 ddefpad;
	} DEFDEFw;


#define ctDODA

#ifdef ct8P_Simulate
			/* add padding to simulate 8-byte pointers	*/
typedef struct {
	pTEXT   fsymb;	/* ptr to optional symbol			*/
	ULONG	pad1;
	pTEXT   fadr;	/* adr of structure member or other item	*/
	ULONG	pad2;
	UCOUNT	ftype;	/* type indicator				*/
	UCOUNT	flen;	/* item length information			*/
	COUNT	fwhat;	/* structure classification			*/
	COUNT	flev;	/* file level reference				*/
	UCOUNT	fhrc;	/* file hierarchy				*/
	COUNT	frsv;	/* unused					*/
	ULONG	pad3;
	pVOID	fusrp;	/* user assignable information block		*/
	ULONG	pad4;
	pVOID	frsrv;	/* reserved					*/
	ULONG	pad5;
	} DATOBJ;
#else /* ct8P_Simulate */
typedef struct {
	pTEXT   fsymb;	/* ptr to optional symbol			*/
	pTEXT   fadr;	/* adr of structure member or other item	*/
	UCOUNT	ftype;	/* type indicator				*/
	UCOUNT	flen;	/* item length information			*/
	COUNT	fwhat;	/* structure classification			*/
	COUNT	flev;	/* file level reference				*/
	UCOUNT	fhrc;	/* file hierarchy				*/
	COUNT	frsv;	/* unused					*/
	pVOID	fusrp;	/* user assignable information block		*/
	pVOID	frsrv;	/* reserved					*/
	} DATOBJ;
#endif /* ct8P_Simulate */

typedef DATOBJ ctMEM *	pDATOBJ;

#ifndef CTBOUND
typedef struct {
	ULONG   fsymb;
	ULONG   fadr;
	UCOUNT	ftype;
	UCOUNT	flen;
	COUNT	fwhat;
	COUNT	flev;
	UCOUNT	fhrc;
	COUNT	frsv;
	ULONG	fusrp;
	ULONG	frsrv;
	} DATOBJ4;
typedef DATOBJ4 ctMEM *	pDATOBJ4;

typedef struct {
	ULONG   fsymb;
	ULONG	fpad1;
	ULONG   fadr;
	ULONG	fpad2;
	UCOUNT	ftype;
	UCOUNT	flen;
	COUNT	fwhat;
	COUNT	flev;
	UCOUNT	fhrc;
	COUNT	frsv;
	ULONG	fpad3;
	ULONG	fusrp;
	ULONG	fpad4;
	ULONG	frsrv;
	ULONG	fpad5;
	} DATOBJ8;
typedef DATOBJ8 ctMEM *	pDATOBJ8;
#endif

typedef struct convmap {
	UTEXT	flavor;
	UTEXT	align;
	UTEXT	flddelm;
	UTEXT	padding;
	VRLEN	maplen;
	VRLEN	nbrflds;
	VRLEN	nbrblks;
	} ConvMap;
typedef ConvMap ctMEM *	pConvMap;

typedef struct convblk {
	UCOUNT	len;
	UTEXT	kind;
	UTEXT	repcnt;
	} ConvBlk;
typedef ConvBlk ctMEM *	pConvBlk;

#define SCHEMA_MAP		1
#define SCHEMA_NAMES		2
#define SCHEMA_MAPandNAMES	3
#define SCHEMA_DODA		4

#define SegOff(struc, member)    	((NINT)&(((struc *)0)->member))
#define ArraySegOff(struc, member)	((NINT) (((struc *)0)->member))

#ifndef ctDTYPES
#define ctDTYPES
#define CT_BOOL		(1 << 3)
#define CT_CHAR		(2 << 3)
#define CT_CHARU	(3 << 3)
#define CT_INT2		((4 << 3) + 1)
#define CT_INT2U	((5 << 3) + 1)
#define CT_INT4		((6 << 3) + 3)
#define CT_INT4U	((7 << 3) + 3)
#define CT_MONEY	((8 << 3) + 3)
#define CT_DATE		((9 << 3) + 3)
#define CT_TIME		((10 << 3) + 3)
#define CT_SFLOAT	((11 << 3) + 3)
#define CT_DFLOAT	((12 << 3) + 7)
#ifdef ctPortDAC
#define CT_SQLBCD	(13 << 3)
#else
#define CT_SQLBCDold	((13 << 3) + 3)
#define CT_SQLBCD	((13 << 3) + 4)
#endif
#define CT_EFLOAT	((14 << 3) + 7)
#define CT_TIMESold	((15 << 3) + 3)
#define CT_TIMES	((15 << 3) + 4)
#define CT_ARRAY	(16 << 3)
#define CT_RESRVD	(17 << 3)

#define CT_FSTRING	(18 << 3)
#define CT_FPSTRING	(19 << 3)
#define CT_F2STRING	(20 << 3)
#define CT_F4STRING	(21 << 3)
#define CT_STRING	(CT_FSTRING + 2)
#define CT_PSTRING	(CT_FPSTRING + 2)
#define CT_2STRING	(CT_F2STRING + 2)
#define CT_4STRING	(CT_F4STRING + 2)
#define CT_LAST		CT_4STRING
#endif

#define CT_STRFLT	(CT_LAST + 1)
#define CT_STRLNG	(CT_LAST + 2)
#define CT_NUMSTR	(CT_LAST + 3)
#define CT_DBLSTR	(CT_LAST + 4)
#define CT_SUBSTR	(CT_LAST + 5)
#define CT_WLDCRD	(CT_LAST + 6)

#define SEC_FILEWORD	1
#define SEC_FILEGRUP	2
#define SEC_FILEMASK	3
#define SEC_FILEOWNR	4

#define ctSEGLEN	1
#define ctSEGMOD	2
#define ctSEGPOS	3

/*
 * I/O Performance Monitoring
 */

#define	DataBufferRequests	0
#define DataBufferHits		1
#define	IndexBufferRequests	2
#define IndexBufferHits		3
#define NbrReadOperations	4
#define NbrBytesRead		5
#define NbrWriteOperations	6
#define NbrBytesWritten		7
#define ctIOPLMT		8

#define NbrCommReadOperations	8
#define NbrCommBytesRead	9
#define NbrCommWriteOperations	10
#define NbrCommBytesWritten	11
#define NbrTranSavepoint	12
#define NbrTranRestores		13
#define NbrTranBegins		14
#define NbrTranEnds		15
#define NbrTranAborts		16
#define NbrCheckPoints		17
#define SystemTimeValue		18
/*
**				19 - 31 available for future use
*/
#define ctIXPLMT		32


#define ctNO_IDX_BUILD		3
#define updateIFIL		-99
#define purgeIFIL		-101
/* NOTE: each ...IFIL should be smaller by an increasing power of 2: the
** next values should be -105, -113, -129, etc.  This permits arbitray
** combinations of them to be unambiguously determined.
*/

/*
 * YOU MAY ADD ONE OR MORE OF THE FOLLOWING DEFINES TO MAINTAIN 
 * COMPATIBILITY WITH c-tree PLUS RELEASES PRIOR TO RELEASE C.
 *
#define FIXED		ctFIXED
#define KEEP		ctKEEP
 *
 */

#define cfgFILES		0
#define cfgUSERS		1
#define cfgIDX_MEMORY		2
#define cfgDAT_MEMORY		3
#define cfgTOT_MEMORY		4
#define cfgUSR_MEMORY		5
#define cfgPREIMAGE_FILE	6
#define cfgPAGE_SIZE		7
#define cfgCOMMIT		8
#define cfgLOG_SPACE		9
#define cfgLOG_EVEN		10
#define cfgLOG_ODD		11
#define cfgSTART_EVEN		12
#define cfgSTART_ODD		13
#define cfgSERVER_DIRECTORY	14
#define cfgLOCAL_DIRECTORY	15
#define cfgSERVER_NAME		16
#define cfgDUMP			17
#define cfgSQL_TABLES		18
#define cfgKEEP_LOGS		19
#define cfgCOMM_PROTOCOL	20
#define cfgSQL_SUPERFILES	21
#define cfgLIST_MEMORY		22
#define cfgSORT_MEMORY		23
#define cfgBUFR_MEMORY		24
#define cfgPREIMAGE_HASH	25
#define cfgLOCK_HASH		26
#define cfgUSR_MEM_RULE		27
#define cfgGUEST_MEMORY		28
#define cfgQUERY_MEMORY		29
#define cfgTRAN_TIMEOUT		30
#define cfgMAX_DAT_KEY		31
#define cfgSQL_DEBUG		32
#define cfgSEMAPHORE_BLK	33
#define cfgSESSION_TIMEOUT	34
#define cfgTASKER_SLEEP		35
#define cfgFILE_HANDLES		36
#define cfgMEMORY_MONITOR	37
#define cfgTASKER_PC		38
#define cfgTASKER_SP		39
#define cfgTASKER_NP		40
#define cfgNODE_DELAY		41
#define cfgDEADLOCK_MONITOR	42
#define cfgNODEQ_MONITOR	43
#define cfgCOMMIT_DELAY		44
#define cfgCHECKPOINT_MONITOR	45
#define cfgNODEQ_SEARCH		46
#define cfgMAX_KEY_SEG		47
#define cfgFUNCTION_MONITOR	48
#define cfgTASKER_LOOP		49
#define cfgREQUEST_DELAY	50
#define cfgREQUEST_DELTA	51
#define cfg9074_MONITOR		52
#define cfg9477_MONITOR		53
#define cfgSKIP_MISSING_FILES	54
#define cfgTMPNAME_PATH		55
#define cfgLOG_EVEN_MIRROR	56
#define cfgLOG_ODD_MIRROR	57
#define cfgSTART_EVEN_MIRROR	58
#define cfgSTART_ODD_MIRROR	59
#define cfgADMIN_MIRROR		60
#define cfgSKIP_MISSING_MIRRORS	61
#define cfgCOMMENTS		62
#define cfgMIRRORS		63
#define cfg749X_MONITOR		64
#define cfgCOMPATIBILITY	65
#define cfgDIAGNOSTICS		66
#define cfgCONTEXT_HASH		67
#define cfgGUEST_LOGON		68
#define cfgTRANSACTION_FLUSH	69
#define cfgCHECKPOINT_FLUSH	70
#define cfgLOCK_MONITOR		71
#define cfgMEMORY_TRACK		72
#define cfgSUPPRESS_LOG_FLUSH	73
#define cfgPREIMAGE_DUMP	74
#define cfgRECOVER_MEMLOG	75
#define cfgRECOVER_DETAILS	76
#define cfgCHECKPOINT_INTERVAL	77
#define cfgRECOVER_SKIPCLEAN	78
#define cfgSIGNAL_READY		79
#define cfgSIGNAL_MIRROR_EVENT	80
#define cfgCHECKPOINT_IDLE	81
#define cfgSIGNAL_DOWN		82
#define cfgFORCE_LOGIDX		83
#define cfgCHECKPOINT_PREVIOUS	84
#define cfgTRAN_HIGH_MARK	85
#define cfgCTSTATUS_MASK	86
#define cfgCTSTATUS_SIZE	87
#define cfgMONITOR_MASK		88
#define cfgRECOVER_FILES	89
#define cfgSYNC_DELAY		90
#define cfgDIAGNOSTIC_INT	91
#define cfgDIAGNOSTIC_STR	92
#define cfgMAX_VIRTUAL_FILES	93
#define cfgFIXED_LOG_SIZE	94
#define cfgLANGUAGE		95
#define cfgLAST			96 /* end of server config entries */

#define cfgDISKIO_MODEL		128
#define cfgTRANPROC		129
#define cfgRESOURCE		130
#define cfgCTBATCH		131
#define cfgCTSUPER		132
#define cfgFUTURE1		133
#define cfgVARLDATA		134
#define cfgVARLKEYS		135
#define cfgPARMFILE		136
#define cfgRTREE		137
#define cfgCTS_ISAM		138
#define cfgBOUND		139
#define cfgNOGLOBALS		140
#define cfgPROTOTYPE		141
#define cfgPASCALst		142
#define cfgPASCAL24		143
#define cfgWORD_ORDER		144
#define cfgFUTURE2		145
#define cfgUNIFRMAT		146
#define cfgLOCLIB		147
#define cfgANSI			148
#define cfgFILE_SPECS		149
#define cfgPATH_SEPARATOR	150
#define cfgLOGIDX		151
#define cfgHISTORY		152
#define cfgCONDIDX		153

#define cfgUNIFRMATapp		cfgUNIFRMAT
#define cfgLOCLIBapp		cfgLOCLIB
#define cfgDISKIO_MODELapp	cfgDISKIO_MODEL
#define cfgBOUNDapp		cfgBOUND
#define cfgNOGLOBALSapp		cfgNOGLOBALS

#define cfgTRANPROCapp		161
#define cfgRESOURCEapp		162
#define cfgCTBATCHapp		163
#define cfgCTSUPERapp		164
#define cfgFUTURE1app		165
#define cfgVARLDATAapp		166
#define cfgVARLKEYSapp		167
#define cfgPARMFILEapp		168
#define cfgRTREEapp		169
#define cfgCTS_ISAMapp		170
#define cfgTHREADapp		171
#define cfgINIT_CTREEapp	172
#define cfgPROTOTYPEapp		173
#define cfgPASCALstapp		174
#define cfgPASCAL24app		175
#define cfgWORD_ORDERapp	176
#define cfgFUTURE2app		177
#define cfgANSIapp		178
#define cfgPATH_SEPARATORapp	179
#define cfgLOGIDXapp		180
#define cfgHISTORYapp		181
#define cfgCONDIDXapp		182
#define cfgSERIALNBR		183

#define cfgMEMORY_USAGE		192
#define cfgMEMORY_HIGH		193
#define cfgNET_ALLOCS		194
#define cfgDNODE_QLENGTH	195
#define cfgCHKPNT_QLENGTH	196
#define cfgSYSMON_QLENGTH	197
#define cfgMONAL1_QLENGTH	198
#define cfgMONAL2_QLENGTH	199
#define cfgLOGONS		200
#define cfgNET_LOCKS		201
#define cfgPHYSICAL_FILES	202
#define cfgOPEN_FILES		203
#define cfgOPEN_FCBS		204
#define cfgUSER_FILES		205
#define cfgUSER_MEMORY		206
#define cfgLOG_RECORD_LIMIT	207

#define ctCFGLMT		256	/* elements in configuration array */

/*
 * transaction history modes
 */

#define ctHISTlog	0x0001	/* set initial log or terminate		*/
#define ctHISTfirst	0x0002	/* get starting entry			*/
#define ctHISTnext	0x0004	/* get next     entry			*/
#define ctHISTfrwd	0x0008	/* scan logs forward (instead of back)	*/

#define ctHISTuser	0x0010	/* match user ID			*/
#define ctHISTnode	0x0020	/* match node name			*/
#define ctHISTpos	0x0040	/* match byte offset (record position)	*/
#define ctHISTkey	0x0080	/* match key value			*/

/* reserve .......	0x0100	   for future use			*/
/* reserve .......	0x0200	   for future use			*/
/* reserve .......	0x0400	   for internal use			*/
/* reserve .......	0x0800	   for internal use			*/

#define ctHISTdata	0x1000	/* return data record (image)		*/
#define ctHISTindx	0x2000	/* return key value			*/
#define ctHISTnet	0x4000	/* return net change, not intra details */
#define ctHISTinfo	0x8000	/* return identifying information	*/

#define ctHISTmapmask	0x00ff	/* HSTRSP imgmap length mask		*/
#define ctHISTkdel	0x0100	/* history key delete, no data image	*/
#define ctHISTdatfile	0x0200	/* data file entry returned		*/
#define ctHISTidxfile	0x0400	/* index file entry returned		*/
#define ctHISTvarfile	0x0800	/* varibale length data file entry	*/

typedef struct hstrsp {
	LONG	tranno;
	LONG	recbyt;
	LONG	lognum;
	LONG	logpos;
	LONG	imglen;
	LONG	trntim;
	LONG	trnfil;
	LONG	resrvd;
	UCOUNT	membno;
	UCOUNT	imgmap;
	COUNT	trntyp;
	COUNT	trnusr;
	} HSTRSP;
typedef HSTRSP ctMEM *	pHSTRSP;

#define ctlogALL		1
#define ctlogLOG		2
#define ctlogSTART		3
#define ctlogLOG_EVEN		4
#define ctlogLOG_ODD		5
#define ctlogSTART_EVEN		6
#define ctlogSTART_ODD		7

#define ctlogALL_MIRROR		17
#define ctlogLOG_MIRROR		18
#define ctlogSTART_MIRROR	19
#define ctlogLOG_EVEN_MIRROR	20
#define ctlogLOG_ODD_MIRROR	21
#define ctlogSTART_EVEN_MIRROR	22
#define ctlogSTART_ODD_MIRROR	23

#ifndef ctWAITFOREVER
#define ctWAITFOREVER		((LONG) -1)
#endif
#ifndef ctNOWAIT
#define ctNOWAIT		((LONG) 0)
#endif

#define ctThrdFIXED_THREADS	((LONG) 0x0001)

#define rtENGLISH		1
#define rtSJIS			2
#define rtEUC			4
#define rtLANGUAGEmask		(rtENGLISH | rtSJIS | rtEUC)

#endif /* ctPORTH */

/* end of ctport.h */
