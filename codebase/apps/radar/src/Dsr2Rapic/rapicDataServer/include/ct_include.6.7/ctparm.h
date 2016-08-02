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

#ifndef ctPARMH
#define ctPARMH

#ifndef ctIFILH
#include "ctifil.h"
#endif
#ifdef ctSQL
#include "ctpsql.h"
#endif

/*
** Logon Block:
*/

typedef struct logblk {
	COUNT	 files;		/* number of files & indices */
	COUNT	 userprof;	/* user profile mask */
	COUNT	 ibuffers;	/* # index buffers */
	COUNT	 dbuffers;	/* # of data buffers */
	COUNT	 nsectors;	/* # of sectors per buffer */
	COUNT	 reservd1;	/* reserved */
	VRLEN	 reservd2;	/* reserved */
	TEXT	 userid[IDZ];	/* user id */
	TEXT	 userword[PWZ];	/* user password */
	TEXT	 servname[FNZ];	/* optional server name */
	TEXT	 lbpad[PDZ];
	} LOGBLK;
typedef LOGBLK ctMEM *	pLOGBLK;

/*
The ibuffers, dbuffers and nsectors members are ignored when connecting to a
FairCom Server. The userid and userword members are ignored in non-server
operation. The userprof member controls whether or not to perform automatic
TFRMKEY's on targets sent to ISAM level search routines. Set userprof to
USERPRF_NTKEY if you wish to disable the automatic TFRMKEY. The servname
member supports multiple server environments.
*/

/*
** ISAM Block:
*/

typedef struct isamblk {
	LOGBLK	 isamlog;	/* login block */
	LONG	 permmask;	/* CREISAM permission mask */
	TEXT	 groupid[IDZ];	/* CREISAM Group id */
	TEXT	 fileword[PWZ];	/* file password */
	TEXT	 parmname[FNZ];	/* parameter file name */
	TEXT	 ispad[PDZ];
	} ISAMBLK;
typedef ISAMBLK ctMEM *	pISAMBLK;

/*
The isamlog member is simply a logon block interpreted as above. The optional
permmask and groupid members are only used by CREISAM to set the files'
permission masks and passwords. The fileword member is used by CREISAM to set
the password and by OPNISAM to access the file. The parmname member holds
the parameter file name.
*/


/*
** IFIL Block:
*/

typedef struct ifilblk {
	LONG	 permmask;	/* CREIFIL permission mask */
	TEXT	 groupid[IDZ];	/* CREIFIL Group id */
	TEXT	 fileword[PWZ];	/* file password */
	TEXT	 dataextn[EXZ];	/* data extension */
	TEXT	 indxextn[EXZ];	/* indx extension */
	TEXT	 ifpad[PDZ];
	pIFIL	 ptr_ifil;	/* IFIL pointer */
	} IFILBLK;
typedef IFILBLK ctMEM *	pIFILBLK;

/*
The permmask and groupid members hold the optional security information
assigned at file creation. The fileword holds the optional password assigned
at creation or the password passed at a subsequent file open. The dataextn
and indxextn members hold optional file name extensions for the data and
index files, respectively. If a file name extension member begins with a
null byte, then the default extension is used, ".dat" for data files and
".idx" for index files. To signify that no extension should be added,
pass the member containing only blanks (and a null terminator). (Both
fiel name extension members cannot be blank since the data file and index
must have unique names.) The ptr_ifil member points to a traditional IFIL
structure.
*/


/*
** Create File Block:
*/

typedef struct creblk {
	UCOUNT	 length;	/* record / key length */
	UCOUNT	 extdsize;	/* file extent size */
	COUNT	 filemode;	/* file mode */
	COUNT	 keytyp;	/* index key type */
	COUNT	 keydup;	/* index duplicate flag */
	COUNT	 member;	/* index member # */
	LONG	 permmask;	/* permission mask */
	TEXT	 groupid[IDZ];	/* group id */
	TEXT	 fileword[PWZ];	/* file password */
	TEXT	 filename[FNZ];	/* file name */
	TEXT	 crpad[PDZ];
	} CREBLK;
typedef CREBLK ctMEM *	pCREBLK;

/*
The length member specifies the record length for data files or the key length
for index files. The extdsize member contains the file szie extension parameter,
and the filemode member contains the file mode to use at creation. The keytyp,
keydup and member members of the parameter block are ignored for data files and
represent the key type, key duplicate and index member number, respectively, for
index files. The permmask, groupid and fileword members hold the optional
file security information. Finally, filename contains the file name to be used
at create.
*/


/*
** Open File Block:
*/

typedef struct openblk {
	COUNT	 filemode;	/* file mode */
	TEXT	 fileword[PWZ];	/* file password */
	TEXT	 filename[FNZ];	/* file name */
	} OPENBLK;
typedef OPENBLK ctMEM *	pOPENBLK;

/*
The filemode member holds the file mode to be used at open time. The fileword
member contains the optional password, and the filename member contains the
file name to be used at open.
*/


/*
** Key Estimate Block:
*/

typedef struct estmblk {
	TEXT	key1[MAXLEN];	/* 1st key */
	TEXT	key2[MAXLEN];	/* 2nd key */
	} ESTMBLK;
typedef ESTMBLK ctMEM *	pESTMBLK;

/*
The Key Estimate Block is used with the ESTKEY function which estimates
the number of key values ocurring between key1 and key2.
*/

typedef struct cresec {
		LONG	 permmask;	/* permission mask */
		TEXT	 groupid[IDZ];	/* opt group id */
		TEXT	 fileword[PWZ];	/* opt file password */
		TEXT	 scpad[PDZ];
	} CRESEC;
typedef CRESEC ctMEM *	pCRESEC;

#define LOGBLKLEN	ctSIZE(LOGBLK)
#define ISAMBLKLEN	ctSIZE(ISAMBLK)
#define OPENBLKLEN	ctSIZE(OPENBLK)
#define IFILBLKLEN	(4 + IDZ + PWZ + EXZ + EXZ)
#define CREBLKLEN	ctSIZE(CREBLK)
#define ESTMBLKLEN	(2 * MAXLEN)

#define p1LOGBLK	((pLOGBLK)ptr1)
#define p1CREBLK	((pCREBLK)ptr1)
#define p1OPENBLK	((pOPENBLK)ptr1)
#define p1ISAMBLK	((pISAMBLK)ptr1)
#define p1IFILBLK	((pIFILBLK)ptr1)
#define p1ESTMBLK	((pESTMBLK)ptr1)
#define p1PKEYBLK	((pPKEYREQ)ptr1)
#define p1DEFDEF	((pDEFDEF)ptr1)

/******************************************************************************/

#define FN_PUTFIL	1
#define FN_UPDCURI	(2 | FN_MASK_ISAM)

#define FN_DELFIL	3
#define FN_LKISAM	(4 | FN_MASK_ISAM | FN_MASK_NFCHK)
#define FN_DELREC	(5 | FN_MASK_ISAM)
#define FN_ALCSET	(6 | FN_MASK_ISAM | FN_MASK_NFCHK)
#define FN_CHGSET	(7 | FN_MASK_ISAM | FN_MASK_NFCHK)
#define FN_DELVREC	(8 | FN_MASK_ISAM)

#define FN_CLISAM	(9 | FN_MASK_ISAM | FN_MASK_NFCHK)
#define FN_STPUSR	(10 | FN_MASK_NFCHK)
#define FN_FRESET	(11 | FN_MASK_ISAM | FN_MASK_NFCHK)
#define FN_CLSFIL	12

#define FN_TRANEND	(13 | FN_MASK_NFCHK)
#define FN_TRANRST	(14 | FN_MASK_NFCHK)
#define FN_TRANABT	(15 | FN_MASK_NFCHK)
#define FN_TRANCLR	(16 | FN_MASK_NFCHK)
#define FN_CLRFIL	(17 | FN_MASK_ISAM)
#define FN_DELRFIL	(18 | FN_MASK_ISAM)
#define FN_ALCBAT	(19 | FN_MASK_ISAM | FN_MASK_NFCHK)
#define FN_CHGBAT	(20 | FN_MASK_ISAM | FN_MASK_NFCHK)
#define FN_FREBAT	(21 | FN_MASK_ISAM | FN_MASK_NFCHK)
#define FN_PERFORM	(22 | FN_MASK_NFCHK)
#define FN_USERLIST	(23 | FN_MASK_NFCHK | FN_MASK_OUTPUT)
#define FN_USERINFO	(24 | FN_MASK_NFCHK | FN_MASK_OUTPUT)
#define FN_CTKLUSR	(25 | FN_MASK_NFCHK)
#define FN_CTTESTFUNC	(26 | FN_MASK_NFCHK)
#define FN_CHGICON	(27 | FN_MASK_ISAM | FN_MASK_NFCHK)
#define FN_CLSICON	(28 | FN_MASK_ISAM | FN_MASK_NFCHK)
#define FN_CTFLUSH	(29 | FN_MASK_NFCHK)
#define FN_CTCHKPNT	30
#define FN_TRANABTX	(31 | FN_MASK_NFCHK)
#define FN_CHGHST	(32 | FN_MASK_NFCHK)
#define FN_FREHST	(33 | FN_MASK_NFCHK)

#define FN_RENFIL	36
#define FN_UPDCIDX	(37 | FN_MASK_ISAM)
#define FN_SETFLTR	38
#define FN_ADDUSR	(39 | FN_MASK_ISAM) /* FN_MASK_KEYSET */
#define FN_ADDREC	(40 | FN_MASK_ISAM) /* FN_MASK_KEYSET */
#define FN_RWTREC	(41 | FN_MASK_ISAM) /* FN_MASK_KEYSET */

#define FN_NXTREC	(42 | FN_MASK_ISAM | FN_MASK_OUTPUT)
#define FN_PRVREC	(43 | FN_MASK_ISAM | FN_MASK_OUTPUT)
#define FN_FRSREC	(44 | FN_MASK_ISAM | FN_MASK_OUTPUT)
#define FN_LSTREC	(45 | FN_MASK_ISAM | FN_MASK_OUTPUT)
#define FN_RRDREC	(46 | FN_MASK_ISAM | FN_MASK_OUTPUT)
#define FN_NXTSET	(47 | FN_MASK_ISAM | FN_MASK_OUTPUT)
#define FN_PRVSET	(48 | FN_MASK_ISAM | FN_MASK_OUTPUT)

#define FN_SETALTSEQ	49
#define FN_GETALTSEQ	(50 | FN_MASK_OUTPUT)
#define FN_SETDEFBLK	51
#define FN_MIDSET	(52 | FN_MASK_ISAM | FN_MASK_OUTPUT)
#define FN_PUTDODA	53
#define FN_SETVARBYTS	54

#define FN_FRSVSET	(55 | FN_MASK_ISAM)
#define FN_LSTVSET	(56 | FN_MASK_ISAM)
#define FN_MIDVSET	(57 | FN_MASK_ISAM | FN_MASK_OUTPUT)

#define FN_SYSMON	(59 | FN_MASK_LONG | FN_MASK_VRLEN | FN_MASK_OUTPUT | FN_MASK_NFCHK)
#define FN_WRTVREC	(60 | FN_MASK_LONG | FN_MASK_VRLEN)
#define FN_RDVREC	(61 | FN_MASK_LONG | FN_MASK_VRLEN | FN_MASK_OUTPUT)

#define FN_REDREC	(62 | FN_MASK_LONG | FN_MASK_OUTPUT)
#define FN_WRTREC	(63 | FN_MASK_LONG)
#define FN_SETCURI	(64 | FN_MASK_LONG | FN_MASK_VRLEN | FN_MASK_ISAM)

#define FN_RETREC	(65 | FN_MASK_LONG)
#define FN_RETVREC	(66 | FN_MASK_LONG)
#define FN_reset_cur	(67 | FN_MASK_LONG | FN_MASK_OUTPUT | FN_MASK_ISAM)
#define FN_SETCURI2	(68 | FN_MASK_LONG | FN_MASK_VRLEN | FN_MASK_ISAM)

#define FN_EQLVREC	(69 | FN_MASK_ISAM)
#define FN_GTEVREC	(70 | FN_MASK_ISAM)
#define FN_LTEVREC	(71 | FN_MASK_ISAM)
#define FN_GTVREC	(72 | FN_MASK_ISAM)
#define FN_LTVREC	(73 | FN_MASK_ISAM)

#define FN_GETMNAME	(74 | FN_MASK_VRLEN | FN_MASK_OUTPUT)
#define FN_GETNAM	(75 | FN_MASK_VRLEN | FN_MASK_OUTPUT)
#define FN_GETSEG	(76 | FN_MASK_VRLEN | FN_MASK_OUTPUT | FN_MASK_ISAM)
#define FN_GETMAP	(77 | FN_MASK_VRLEN | FN_MASK_OUTPUT | FN_MASK_ISAM)
#define FN_SECURITY	(78 | FN_MASK_VRLEN)

#define FN_FRSSET	(80 | FN_MASK_ISAM)
#define FN_LSTSET	(81 | FN_MASK_ISAM)

#define FN_EQLREC	(82 | FN_MASK_ISAM)
#define FN_GTEREC	(83 | FN_MASK_ISAM)
#define FN_LTEREC	(84 | FN_MASK_ISAM)
#define FN_GTREC	(85 | FN_MASK_ISAM)
#define FN_LTREC	(86 | FN_MASK_ISAM)
#define FN_REDIREC	(87 | FN_MASK_ISAM | FN_MASK_LONG | FN_MASK_OUTPUT)

#define FN_CHGUSR	(89 | FN_MASK_NFCHK)
#define FN_CREISAM	(90 | FN_MASK_ISAM | FN_MASK_LOGON | FN_MASK_NFCHK)
#define FN_OPNISAM	(91 | FN_MASK_ISAM | FN_MASK_LOGON | FN_MASK_NFCHK)

#define FN_DELIFIL	(93 | FN_MASK_ISAM)
#define FN_RBLIFIL	(94 | FN_MASK_ISAM | FN_MASK_NFCHK)
#define FN_CREIFIL	(95 | FN_MASK_ISAM | FN_MASK_NFCHK)
#define FN_CLIFIL	(96 | FN_MASK_ISAM)
#define FN_OPNIFIL	(97 | FN_MASK_ISAM | FN_MASK_NFCHK)
#define FN_PRMIIDX	(98 | FN_MASK_ISAM)
#define FN_TMPIIDX	(99 | FN_MASK_ISAM)
#define FN_PUTIFIL	(100| FN_MASK_ISAM | FN_MASK_NFCHK)
#define FN_CMPIFIL	(101| FN_MASK_ISAM | FN_MASK_NFCHK)
#define FN_EXPIFIL	(102| FN_MASK_ISAM | FN_MASK_NFCHK)
#define FN_RBLIIDX	(103| FN_MASK_ISAM)

#define FN_NXTVREC	(104 | FN_MASK_ISAM | FN_MASK_OUTPUT)
#define FN_PRVVREC	(105 | FN_MASK_ISAM | FN_MASK_OUTPUT)
#define FN_FRSVREC	(106 | FN_MASK_ISAM | FN_MASK_OUTPUT)
#define FN_LSTVREC	(107 | FN_MASK_ISAM | FN_MASK_OUTPUT)
#define FN_NXTVSET	(108 | FN_MASK_ISAM | FN_MASK_OUTPUT)
#define FN_PRVVSET	(109 | FN_MASK_ISAM | FN_MASK_OUTPUT)

#define FN_INTREE	(110 | FN_MASK_LOGON | FN_MASK_NFCHK)
#define FN_INTISAM	(111 | FN_MASK_ISAM | FN_MASK_LOGON | FN_MASK_NFCHK)

#define FN_SETNODE	(115 | FN_MASK_NFCHK)

#define FN_ADDVREC	(120 | FN_MASK_VRLEN | FN_MASK_ISAM) /* FN_MASK_KEYSET*/
#define FN_RWTVREC	(121 | FN_MASK_VRLEN | FN_MASK_ISAM) /* FN_MASK_KEYSET*/
#define FN_REDVREC	(122 | FN_MASK_VRLEN | FN_MASK_ISAM | FN_MASK_OUTPUT)
#define FN_DELRES	123
#define FN_ENARES	124
#define FN_UPDRES	(125 | FN_MASK_VRLEN)
#define FN_ADDRES	(126 | FN_MASK_VRLEN)
#ifdef ctCONDIDX
#define FN_PUTCRES	(127 | FN_MASK_VRLEN | FN_MASK_ISAM)
#endif

#define FN_ADDKEY	(130 | FN_MASK_LONG)
#define FN_LOADKEY	(131 | FN_MASK_LONG)
#define FN_DELCHK	(132 | FN_MASK_LONG)

#define FN_SETOPS	(136 | FN_MASK_NFCHK | FN_MASK_LONG | FN_MASK_VRLEN)

#define FN_CREDAT	(140 | FN_MASK_NFCHK)
#define FN_CREIDX	(141 | FN_MASK_NFCHK)
#define FN_CREMEM	142

#define FN_GTEKEY	(150 | FN_MASK_LONG | FN_MASK_OUTRET)
#define FN_GTKEY	(151 | FN_MASK_LONG | FN_MASK_OUTRET)
#define FN_LTKEY	(152 | FN_MASK_LONG | FN_MASK_OUTRET)
#define FN_LTEKEY	(153 | FN_MASK_LONG | FN_MASK_OUTRET)

#define FN_DELBLD	(154 | FN_MASK_LONG)
#define FN_EQLKEY	(155 | FN_MASK_LONG)

#define FN_FRSKEY	(156 | FN_MASK_LONG | FN_MASK_OUTPUT | FN_MASK_OUTRET)
#define FN_LSTKEY	(157 | FN_MASK_LONG | FN_MASK_OUTPUT | FN_MASK_OUTRET)

#define FN_GETCURP	(158 | FN_MASK_LONG | FN_MASK_ISAM)
#define FN_NEWREC	(159 | FN_MASK_LONG)
#define FN_DATENT	(160 | FN_MASK_LONG)
#define FN_IDXENT	(161 | FN_MASK_LONG)
#define FN_SERIALNUM	(162 | FN_MASK_LONG)
#define FN_TRANBEG	(163 | FN_MASK_LONG | FN_MASK_NFCHK)

#define FN_NXTKEY	(164 | FN_MASK_LONG | FN_MASK_OUTPUT | FN_MASK_OUTRET)
#define FN_PRVKEY	(165 | FN_MASK_LONG | FN_MASK_OUTPUT | FN_MASK_OUTRET)
#define FN_GETRES	(166 | FN_MASK_LONG | FN_MASK_VRLEN)
#define FN_CTDIDX	(167 | FN_MASK_LONG | FN_MASK_NFCHK)
#define FN_SQLLOCK	(168 | FN_MASK_LONG | FN_MASK_NFCHK)
#ifdef ctPREV_66A3_CTUSER
#define FN_CTUSER	(169 | FN_MASK_LONG | FN_MASK_NFCHK)
#else
#define FN_CTUSER	(169 | FN_MASK_LONG | FN_MASK_VRLEN | FN_MASK_NFCHK)
#endif
#define FN_GETFIL	(170 | FN_MASK_LONG)
#define FN_GTVLEN	(180 | FN_MASK_LONG | FN_MASK_VRLEN)
#define FN_GETVLEN	(181 | FN_MASK_VRLEN | FN_MASK_ISAM)
#define FN_GETIFIL	(182 | FN_MASK_VRLEN | FN_MASK_LONG | FN_MASK_ISAM | FN_MASK_OUTPUT)
#define FN_GETDODA	(183 | FN_MASK_VRLEN | FN_MASK_LONG | FN_MASK_OUTPUT)
#ifdef ctCONDIDX
#define FN_GETCRES	(184 | FN_MASK_VRLEN | FN_MASK_LONG | FN_MASK_ISAM | FN_MASK_OUTPUT)
#define FN_GETCIDX	(185 | FN_MASK_VRLEN | FN_MASK_LONG | FN_MASK_ISAM | FN_MASK_OUTPUT)
#endif

#define FN_NOTSENT	187
#define FN_TFRMKEY	(188 | FN_MASK_OUTPUT)
#define FN_GETCURK	(189 | FN_MASK_VRLEN | FN_MASK_ISAM | FN_MASK_OUTPUT)
#define FN_GETCURKL	(190 | FN_MASK_OUTPUT)

#define FN_CTSBLD	(192 | FN_MASK_NFCHK)
#define FN_TSTVREC	(193 | FN_MASK_LONG | FN_MASK_VRLEN)
#define FN_COMMBUF	(194 | FN_MASK_VRLEN | FN_MASK_NFCHK | FN_MASK_LONG)
#define FN_SQR		(195 | FN_MASK_VRLEN | FN_MASK_NFCHK | FN_MASK_ISAM | FN_MASK_LONG)
#define FN_OPNRFIL	(196 | FN_MASK_NFCHK | FN_MASK_LONG | FN_MASK_ISAM)
#define FN_TMPNAME	(197 | FN_MASK_VRLEN | FN_MASK_NFCHK | FN_MASK_OUTPUT)
#define FN_SQL		(198 | FN_MASK_VRLEN | FN_MASK_NFCHK | FN_MASK_ISAM | FN_MASK_LONG)
#define FN_DYNDMP	(199 | FN_MASK_VRLEN | FN_MASK_NFCHK)
#define FN_FRCKEY	(200 | FN_MASK_LONG | FN_MASK_OUTPUT | FN_MASK_OUTRET)
#define FN_AVLFILNUM	(201 | FN_MASK_NFCHK)
#define FN_OPNFIL	(202 | FN_MASK_NFCHK)
#define FN_BATSET	(203 | FN_MASK_VRLEN | FN_MASK_ISAM)
#define FN_LOKREC	(204 | FN_MASK_LONG)
#define FN_ESTKEY	(205 | FN_MASK_LONG)
#define FN_NEWVREC	(206 | FN_MASK_LONG | FN_MASK_VRLEN)
#define FN_TRANSAV	(207 | FN_MASK_NFCHK)
#define FN_TRANBAK	(208 | FN_MASK_LONG | FN_MASK_NFCHK)
#define FN_SETFNDVAL	(209 | FN_MASK_VRLEN | FN_MASK_NFCHK)
#define FN_IOPERFORMANCE (210 | FN_MASK_NFCHK | FN_MASK_OUTPUT)
#define FN_CLNIDX	(211 | FN_MASK_NFCHK)
#define FN_RNGENT	(212 | FN_MASK_LONG)
#define FN_ORDKEY	(213 | FN_MASK_LONG | FN_MASK_VRLEN)
#define FN_OPNICON	(214 | FN_MASK_LONG | FN_MASK_ISAM)
#define FN_SYSCFG	(215 | FN_MASK_NFCHK | FN_MASK_OUTPUT)
#define FN_PUTHDR	(216 | FN_MASK_LONG)

#define FN_CUST_LOGON	(217 | FN_MASK_ISAM | FN_MASK_LONG | FN_MASK_LOGON | FN_MASK_NFCHK)
#define FN_CUST_LOGOFF	(218 | FN_MASK_LONG | FN_MASK_NFCHK)
#define FN_CUST_OPC1	(219 | FN_MASK_LONG | FN_MASK_NFCHK)
#define FN_CUST_OPC2	(220 | FN_MASK_NFCHK)
#define FN_CUST_OPC3	(221 | FN_MASK_NFCHK)
#define FN_CUST_OPC4	(222 | FN_MASK_NFCHK)

#define FN_CTHIST	(223 | FN_MASK_NFCHK | FN_MASK_LONG | FN_MASK_VRLEN)
#define FN_IOPERFORMANCEX (224 | FN_MASK_NFCHK | FN_MASK_OUTPUT)
#define FN_REDIVREC	(225 | FN_MASK_ISAM | FN_MASK_LONG | FN_MASK_OUTPUT)
#define FN_reset_cur2	(226 | FN_MASK_ISAM | FN_MASK_LONG | FN_MASK_OUTPUT)
#define FN_RTSCRIPT	(227 | FN_MASK_ISAM | FN_MASK_LONG | FN_MASK_VRLEN | FN_MASK_NFCHK)
#define CTI_MXFN	227

#define FN_WCHCTREE	(CTI_MXFN + 1)
#define FN_NXTCTREE	(CTI_MXFN + 2)
#define FN_SWTCTREE	(CTI_MXFN + 3)
#define FN_GETCTREE	(CTI_MXFN + 4)
#define FN_REGCTREE	(CTI_MXFN + 5)
#define FN_UNRCTREE	(CTI_MXFN + 6)

#endif /* ctPARMH */

/* end of ctparm.h */
