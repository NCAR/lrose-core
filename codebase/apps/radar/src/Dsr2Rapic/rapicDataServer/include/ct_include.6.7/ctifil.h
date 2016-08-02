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

#ifndef ctIFILH
#define ctIFILH

#define DAT_EXTENT	".dat"
#define IDX_EXTENT	".idx"

typedef struct iseg {
	COUNT	soffset,	/* segment offset	*/
		slength,	/* segment length	*/
		segmode;	/* segment mode		*/
	} ISEG;
typedef ISEG ctMEM *	pISEG;
#define ISEG_LEN	6
#define ISEG_PLEN	6

#ifdef ct8P_Simulate
typedef struct iidx {
	COUNT	ikeylen,	/* key length		*/
		ikeytyp,	/* key type		*/
		ikeydup,	/* duplicate flag	*/
		inulkey,	/* null ct_key flag	*/
		iempchr,	/* empty character	*/
		inumseg;	/* number of segments	*/
	COUNT	pad00,pad01;
	pISEG   seg;		/* segment information	*/
	LONG	pad1;
	pTEXT   ridxnam;	/* r-tree symbolic name	*/
	LONG	pad2;
	pTEXT	aidxnam;	/* (opt) index filnam	*/
	LONG	pad3;
	pCOUNT	altseq;		/* (opt) alt sequence	*/
	LONG	pad4;
	pUTEXT	pvbyte;		/* (opt) ptr to pad byte*/
	LONG	pad5;
	} IIDX;
#else /* ct8P_Simulate */
typedef struct iidx {
	COUNT	ikeylen,	/* key length		*/
		ikeytyp,	/* key type		*/
		ikeydup,	/* duplicate flag	*/
		inulkey,	/* null ct_key flag	*/
		iempchr,	/* empty character	*/
		inumseg;	/* number of segments	*/
	pISEG   seg;		/* segment information	*/
	pTEXT   ridxnam;	/* r-tree symbolic name	*/
	pTEXT	aidxnam;	/* (opt) index filnam	*/
	pCOUNT	altseq;		/* (opt) alt sequence	*/
	pUTEXT	pvbyte;		/* (opt) ptr to pad byte*/
	} IIDX;
#endif /* ct8P_Simulate */

typedef IIDX ctMEM *	pIIDX;
#define IIDX_LEN	12

typedef struct {
	COUNT	c[6];
	LONG	seg;
	LONG	ridxnam;
	LONG	aidxnam;
	LONG	altseq;
	LONG	pvbyte;
	} IIDXz;
typedef IIDXz ctMEM *	pIIDXz;
#define IIDX_PLEN	ctSIZE(IIDXz)

typedef struct {
	COUNT	c[6];
	LONG	seg;
	LONG	ridxnam;
	} IIDX1z;

#ifdef ct8P_Simulate
typedef struct ifil {
	pTEXT   pfilnam;	/* file name (w/o ext)	*/
	LONG	pad1;
	COUNT	dfilno;		/* data file number	*/
	UCOUNT	dreclen;	/* data record length	*/
	UCOUNT	dxtdsiz;	/* data file ext size	*/
	COUNT	dfilmod;	/* data file mode	*/
	COUNT	dnumidx;	/* number of indices	*/
	UCOUNT	ixtdsiz;	/* index file ext size	*/
	COUNT	ifilmod;	/* index file mode	*/
	COUNT	pad20,pad21,pad22;
	pIIDX   ix;		/* index information	*/
	LONG	pad3;
	pTEXT   rfstfld;	/* r-tree 1st fld name	*/
	LONG	pad4;
	pTEXT   rlstfld;	/* r-tree last fld name	*/
	LONG	pad5;
	COUNT	tfilno;		/* temporary file number*/
	} IFIL;
#else /* ct8P_Simulate */
typedef struct ifil {
	pTEXT   pfilnam;	/* file name (w/o ext)	*/
	COUNT	dfilno;		/* data file number	*/
	UCOUNT	dreclen;	/* data record length	*/
	UCOUNT	dxtdsiz;	/* data file ext size	*/
	COUNT	dfilmod;	/* data file mode	*/
	COUNT	dnumidx;	/* number of indices	*/
	UCOUNT	ixtdsiz;	/* index file ext size	*/
	COUNT	ifilmod;	/* index file mode	*/
	pIIDX   ix;		/* index information	*/
	pTEXT   rfstfld;	/* r-tree 1st fld name	*/
	pTEXT   rlstfld;	/* r-tree last fld name	*/
	COUNT	tfilno;		/* temporary file number*/
	} IFIL;
#endif /* ct8P_Simulate */
typedef IFIL ctMEM *	pIFIL;
#define IFIL_LEN	14
#define IFIL_PLEN	36

typedef struct ifilz {
	LONG    pfilnam;	/* file name (w/o ext)	*/
	COUNT	dfilno;		/* data file number	*/
	UCOUNT	dreclen;	/* data record length	*/
	UCOUNT	dxtdsiz;	/* data file ext size	*/
	COUNT	dfilmod;	/* data file mode	*/
	COUNT	dnumidx;	/* number of indices	*/
	UCOUNT	ixtdsiz;	/* index file ext size	*/
	COUNT	ifilmod;	/* index file mode	*/
	LONG    ix;		/* index information	*/
	LONG    rfstfld;	/* r-tree 1st fld name	*/
	LONG    rlstfld;	/* r-tree last fld name	*/
	COUNT	tfilno;		/* temporary file number*/
	} IFILz;
typedef IFILz ctMEM *	pIFILz;

#ifdef ctCONDIDX
typedef struct ccond {
	COUNT	relkey;		/* relative key number: 0,1,...		*/
	COUNT	explen;		/* length of tokenized expression	*/
	LONG	resrvd;		/* reserved for future use		*/
	pTEXT	cexpr;		/* ptr to tokenized expression (not a
				   NULL terminated ASCII string)	*/
	} CCOND;
typedef CCOND ctMEM *	pCCOND;
typedef CCOND ctMEM * ctMEM * ppCCOND;

typedef struct cifil {
	COUNT	cnumstat;	/* number of static  conditions		*/
	COUNT	cnumdynm;	/* number of dynamic conditions		*/
	ppCCOND	condstat;	/* ptr to array of static  CCOND's	*/
	ppCCOND	conddynm;	/* ptr to array of dynamic CCOND's	*/
	} CIFIL;
typedef CIFIL ctMEM *	pCIFIL;
#endif

#endif /* ctIFILH */

/* end of ctifil.h */
