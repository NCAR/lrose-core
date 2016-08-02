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

#ifndef CTFUNTH
#define CTFUNTH

#ifdef PROTOTYPE
#ifdef CTHISTORY
extern  VOID ctfreehlst(ppHSTLST );
#endif
extern  NINT ctfmatch(pCTFILE hnum,pLONG fid);
#ifdef dbgHGHTRN
extern  NINT dbghghtrn(LONG tran,pTEXT locale,pCTFILE knum);
#endif
#ifdef MULTITRD
extern  VOID ctchkpstate(VOID );
#else
extern  VOID ctfreelp(VOID );
extern  NINT ctrt_getcwd(pTEXT cwd,NINT maxlen);
#endif
extern  LONG ctgettrn(NINT ,NINT );
#ifdef ctLOGIDX
extern  NINT cttrnindx(pTREBUF ,pTEXT ,LONG ,NINT );
extern  pCOMLST cttrnset(LONG );
extern  pCOMLST cttrnfnd(LONG ,NINT ,NINT );
extern  VOID cttrnclr(VOID );
#endif
extern  COUNT lctio(pCTFILE ctnum,LONG logpos,pTEXT tp,VRLEN vlen);
#ifdef ctMIRROR
extern  NINT ctmirlst(pCTFILE ctnum,NINT mirswt);
#endif
extern  NINT ctqchkp(pTEXT sp);
extern  VOID ctqchkpstop(VOID);
extern  VOID cttrnfre(VOID);
extern  COUNT clnleaf(COUNT filno);
extern  VOID ctinvshd(pSHADLST sl pthHan);
extern  UCOUNT ctsumlog(pTRANDSC tp);
extern  NINT ctsumchk(pCTFILE ctnum,VRLEN vlen,LONG logpos,pTRANDSC tp);
extern  VOID cthdrword(COUNT filno,pTEXT buf,NINT len,NINT newone);
extern  COUNT ctlogpidx(pCTFILE knum,COUNT memb,pUTEXT pvbyte);
extern  COUNT ctlogiidx(pCTFILE dnum,COUNT keyno,COUNT idxno,pIFIL ifilptr);
extern  COUNT ctloghwrd(pCTFILE knum,pTEXT oldwrd,pTEXT newwrd,COUNT offset,NINT len);
extern  pTEXT ctgetshd(pVOID bufadr,pSHADLST sl,VRLEN iosize pthHan);
extern  COUNT ctputshd(pSHADLST sl,pTEXT bufadr,VRLEN iosize pthHan);
extern  VOID ctfreshd(pSHADLST sl pthHan);
extern  VOID ctputtenv(NINT sOWNR,pTEXT envbuf,VRLEN len);
extern  NINT ctmrkact(NINT cln_mode,LONG mtran,pTREBUF buffer pthHan);
extern  NINT ctintlog(pCTFILE ctnum);
extern  LONG ctprvlog(pCTFILE ctnum,NINT op_code);
extern  NINT ctautotran(NINT sOWNR,LONG updmod,pCTFILE ctnum);
extern  NINT ctstrtran(pCTFILE ctnum);
extern  VOID ctendtran(NINT serr);
extern  pTEXT ctlname(pTEXT lognam,LONG ln,NINT filetype);
#ifndef comparu
extern  VRLEN comparu(pTEXT np,pTEXT op,VRLEN remlen);
#endif
extern  COUNT ctlgtorc(LONG tfil,NINT must_exist);
extern  VOID ctstund(NINT mode,LONG tranno,LONG lp,LONG ep);
extern  NINT ctputimg(NINT testtyp,NINT typimage,COUNT tranfil,LONG tranpos,pTEXT bp,VRLEN vlen);
extern  VRLEN ctdiff(pTEXT np,VRLEN nl,pTEXT op,VRLEN ol,ppTEXT dp,pVRLEN dspc pthHan);
extern  VOID cttranbf(pTREBUF buf,LONG old,LONG newone);
extern  COUNT ctacttrn(NINT ownr,LONG tran_no,pNINT povrflag);
extern  COUNT cttrnalc(COUNT nusers,COUNT nfiles);
extern  VRLEN cttrnlen(pTRANDSC tp);
extern  COUNT prtlog(pLONG plogpos,COUNT mode,pLONG plogser,COUNT tt,LONG fpos,LONG tn);
extern  COUNT srchshd(pVOID phs,NINT ownr,LONG recbyt,pCTFILE ctnum,VRLEN iosize,pVOID bufadr,ppSHADLST psl,NINT mode,ppSHADLST lsl,pVRLEN pread);
extern  NINT updtshd(NINT ownr,LONG recbyt,pCTFILE ctnum,VRLEN iosize,pVOID bufadr,NINT mode pthHan);
extern  NINT testshd(COUNT loc);
extern  NINT prtshd(COUNT own);
extern  LONG ctupdlg(NINT updmod,NINT trnmod,LONG trnval);
extern  VOID fusrclr(COUNT mode,NINT sOWNR);
extern  LONG ctwrtlog(pLONG ep,pTRANDSC tp,pVOID dp pthHan);
#ifdef VINES
extern  NINT dbuglog(LONG pos,COUNT fil,pTEXT tp,UCOUNT class);
#endif
extern  COUNT ctputhdr(pCTFILE ctnum);
extern  COUNT ctlogext(NINT sOWNR,LONG pntr,pCTFILE ctnum,pVOID tp,NINT len,NINT mode);
extern  COUNT ctlogcmp(pCTFILE ctnum,pCTFILE mtnum,pTEXT hdrbuf);
extern  COUNT ctwrtcmp(pCTFILE ctnum,pCTFILE mtnum,pTEXT hdrbuf);
extern  VOID ctlgqnod(NINT ownr,pCTFILE knum,LONG node);
extern  COUNT ctlogkey(COUNT mode,pCTFILE knum,pTEXT target,LONG recbyt);
extern  COUNT ctlogopn(COUNT filno,pTEXT filnam,pCTFILE ctnum,NINT opnmod);
extern  COUNT ctchkpnti(NINT mode,LONG cmode);
extern COUNT ctlogcfil(NINT,COUNT);

extern  NINT ctNINTlog(pCTFILE ctnum);
extern  NINT ctflslog(pCTFILE ctnum,NINT trntyp,NINT sOWNR);
extern  COUNT tranovr(NINT mode,COUNT lokmod);
extern  NINT ctlgfile(NINT op_code,pCTFILE ctnum,LONG begtrn,VRLEN beglen,LONG logser,pLONG chkpos);
extern  COUNT ctstrfil(VOID );
extern  COUNT cttrnmem(pCTFILE ctnum,VRLEN vlen,pVRLEN pvspc,ppTEXT pbp,LONG logpos pthHan);
extern  NINT ctlgswitch(pCTFILE ctnum,NINT op_code,LONG chgser);
extern  LONG TRANBAK(COUNT trnmod,LONG trnval);
extern  LONG TRANFWD(COUNT trnmod,LONG prvpos);

#else /* PROTOTYPE */

#ifdef CTHISTORY
extern  VOID ctfreehlst();
#endif
extern  NINT ctfmatch();
#ifdef dbgHGHTRN
extern  NINT dbghghtrn();
#endif
#ifdef MULTITRD
extern  VOID ctchkpstate();
#else
extern  VOID ctfreelp();
extern  NINT ctrt_getcwd();
#endif
extern  LONG ctgettrn();
#ifdef ctLOGIDX
extern  NINT cttrnindx();
extern  pCOMLST cttrnset();
extern  pCOMLST cttrnfnd();
extern  VOID cttrnclr();
#endif
extern  COUNT lctio();
#ifdef ctMIRROR
extern  NINT ctmirlst();
#endif
extern  NINT ctqchkp();
extern  VOID ctqchkpstop();
extern  VOID cttrnfre();
extern  COUNT clnleaf();
extern  VOID ctinvshd();
static  COUNT clshdr();
extern  UCOUNT ctsumlog();
extern  NINT ctsumchk();
extern  VOID cthdrword();
extern  COUNT ctlogpidx();
extern  COUNT ctlogiidx();
extern  COUNT ctloghwrd();
static  COUNT chkfdel();
extern  pTEXT ctgetshd();
extern  COUNT ctputshd();
static  NINT ctalcshd();
extern  VOID ctfreshd();
extern  VOID ctputtenv();
static  pCTFILE rcvsi();
static  NINT ctsetexc();
extern  NINT ctmrkact();
extern  NINT ctintlog();
extern  LONG ctprvlog();
extern  NINT ctautotran();
extern  NINT ctstrtran();
extern  VOID ctendtran();
static  NINT rcvhdrupd();
extern  pTEXT ctlname();
#ifndef comparu
extern  VRLEN comparu();
#endif
extern  COUNT ctlgtorc();
static  NINT put_lgtorc();
static  NINT tread();
extern  VOID ctstund();
static  COUNT wrtlnk();
#ifdef MULTITRD
static  VOID setkeypos();
#endif
#ifndef CTBOUND
static  pTEXT getkeyptr();
#endif
static  NINT hashshd();
extern  NINT ctputimg();
extern  VRLEN ctdiff();
extern  VOID cttranbf();
extern  COUNT ctacttrn();
extern  COUNT cttrnalc();
static  LONG getlogfil();
extern  VRLEN cttrnlen();
extern  COUNT prtlog();
extern  COUNT srchshd();
extern  NINT updtshd();
extern  NINT testshd();
extern  NINT prtshd();
extern  LONG ctupdlg();
extern  VOID fusrclr();
static  NINT tlogbuf();
extern  LONG ctwrtlog();
#ifdef VINES
extern  NINT dbuglog();
#endif
extern  COUNT ctputhdr();
extern  COUNT ctlogext();
extern  COUNT ctlogcmp();
extern  COUNT ctwrtcmp();
extern  VOID ctlgqnod();
extern  COUNT ctlogkey();
extern  COUNT ctlogopn();
static  COUNT chkpnt();
extern  COUNT ctchkpnti();
static  NINT chksavpnt();
static  COUNT forcel();
static  COUNT forcei();
extern  NINT ctNINTlog();
extern  NINT ctflslog();
static  COUNT force();
static  VOID clrshdlst();
static  pTEXT tranent();
extern  COUNT tranovr();
/* ******************************* */ 
extern  NINT ctlgfile();
extern  COUNT ctstrfil();
static  COUNT ctrcvopn();
static  VOID trancls();
static  COUNT tranmgt();
extern  COUNT cttrnmem();
static  COUNT transcn();
static  VOID adjnent();
extern  NINT ctlgswitch();
static  LONG getprvlog();
static  COUNT tranund();
static  COUNT trandat();
static  COUNT tranidx();
static  COUNT tranred();
static  LONG tranupd();
extern  LONG TRANBAK();
extern  LONG TRANFWD();
static  COUNT tranrcv();
extern COUNT ctlogcfil();
/* ******************************* */

#endif /* PROTOTYPE */

#endif /* CTFUNTH */
