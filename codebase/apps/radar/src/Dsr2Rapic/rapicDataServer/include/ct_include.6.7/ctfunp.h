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

#ifndef ctFUNPH
#define ctFUNPH

#ifndef ctPARMH
#include "ctparm.h"
#endif

#ifdef DllLoadDS
#define _lds_   DllLoadDS
#else
#define _lds_
#endif

#ifdef CTBOUND

#ifndef ctBONDH
#include "ctbond.h"
#endif

#else

#ifndef ctCOMMH
#include "ctcomm.h"
#endif

#endif

#ifdef ctLOCLIB
#ifdef CTBOUND
#ifdef CTPERM
#include "ctlocf.h"
#endif
#endif
#endif
#include "ctdecl.h"

#ifndef ctGVARH
#define pinHan
#define pthHan
#endif

#ifdef PROTOTYPE

extern  UINT         cthash_shift(pCTFILE ctnum);
extern  NINT         ctfpglok(RNDFILE lfd,LONG offset,LONG range,NINT mode);
extern  VRLEN        aGETDODA(COUNT datno,LONG buflen,pVOID bufptr,NINT olign,COUNT mode);
extern  COUNT        iGETNAM(COUNT filno,pTEXT nambuf,VRLEN buflen,COUNT mode pthHan);
#ifdef ctThrdFPG
extern  pVOID        ctserl(NINT fn);
extern  NINT         ctunserl(NINT fn);
#endif
extern  UCOUNT       ctsetextsiz(UCOUNT xtdsiz,UCOUNT untlen);
extern  COUNT        rensup(COUNT filno,pTEXT oldnam,pTEXT newnam);
extern 	pTEXT        chkifilnp(pIFIL tp,pTEXT filnam);
extern  NINT         ctmenu(ppTEXT pi,pTEXT hd,TEXT q,pTEXT quit,NINT ni,NINT mhlp);
extern  VOID         upbuf(pTEXT tp);
extern  NINT         press(pTEXT tp,COUNT err);
extern  NINT         chkwrd(pTEXT tp,pTEXT prompt1,pTEXT prompt2,NINT whlp);
extern  VOID         getid(pTEXT prompt,NINT offset,NINT hlp);
extern  VOID         getwrd(pTEXT tp,NINT size,pTEXT prompt,NINT whlp);
extern  VOID         getsrv(VOID );
extern  NINT         getstpsrv(VOID );
extern  NINT         getioh(pTEXT tp,NINT hlp);
extern  VOID         errmsg(COUNT rc);
extern  COUNT        ct_chkplen(pVRLEN );
#ifdef ctCLIENT
extern  NINT         icttseg(COUNT segpos,COUNT mod,COUNT slen,pTEXT tarptr,pCOUNT aq,pConvMap mp,NINT atf);
#else
extern  NINT         icttseg(COUNT segpos,COUNT mod,COUNT slen,pTEXT tarptr,pCOUNT aq,pConvMap mp);
extern  NINT         ctchkcidx(COUNT datno,COUNT rkeyno);
extern  COUNT        iRDVREC(COUNT datno,ctRECPT recbyt,pVOID recptr,VRLEN bufsiz,NINT redmod pthHan);
#endif /* ctCLIENT */
extern  NINT         ictuseg(COUNT spos,COUNT mod,COUNT slen,pTEXT tarptr,pCOUNT aq,pConvMap mp);
extern  NINT         ctdallc(pNINT ptot,pNINT pcur,ppVOID anchor,NINT objsiz,NINT incr);
extern  NINT         ctfillff(pCTFILE ctnum,ctRECPT begpos,VRLEN iosize);
#ifdef DBG123
extern  NINT         dbg123(pTEXT msg,pTEXT fn,pTEXT rechdr,ctRECPT recbyt);
#endif
#ifdef ctLOG_NEWNOD
extern  NINT         ctlog_newnod(NINT fn,NINT op,NINT km,NINT ow,LONG recbyt);
#endif
#ifdef ctLOG_FILEIO
extern  NINT         ctlog_fileio(NINT fn,NINT op,NINT lc,NINT ow,ctRECPT recbyt,VRLEN iosize,pTEXT buf);
#endif
extern  NINT         ctcmpdlng(ULONG ah,ULONG al,ULONG bh,ULONG bl);
#ifdef ctCLIENT
#ifdef ctCLIENTcore
extern  pConvMap     ctlfschget(COUNT datno pinHan);
extern  NINT         ctsetnlkey(COUNT keyno,NINT cidxflg pinHan);
#endif
#endif
extern  VOID         freectlist(NINT i);
extern  COUNT        iSETCURI(COUNT datno,ctRECPT recbyt,pVOID recptr,VRLEN datlen,COUNT keyno);
#ifdef ctCONDIDX
extern  pCIFIL       ctigetcidx(COUNT datno);
extern  NINT         ctevalcidx(pCIFIL pcifil,COUNT relkey,pVOID recptr,pConvMap schema);
extern  VOID         ctfreecidx(pCIFIL pcifil);
#endif
#ifdef ctFILE_ACCESS
extern  NINT         ctFileAccess(pTEXT filnam);
#endif
extern  COUNT        ctdelupl(LONG snode,pCTFILE knum);
extern  COUNT        ctinsert(pTREBUF,pCTFILE,pTEXT,ctRECPT,NINT,LONG,NINT);
#ifdef DBGtree
extern  NINT         ctlogtree_init(NINT mode);
extern  NINT         ctlogtree_done(VOID );
extern  NINT         ctlogtree_walk(pCTFILE knum,LONG node,NINT lokind);
extern  NINT         ctlogtree_comp(pTEXT kv1,pTEXT kv2,pCTFILE knum);
extern  NINT         ctlogtree_dump(VOID );
#endif
extern  VOID         ctclrcon(COUNT datno);
extern  NINT         ctclrlockon(VOID );
extern  COUNT        iretrec(COUNT datno,LONG recbyt,NINT chgent);
#ifdef DBG9477
extern  NINT         log9477(pCTFILE knum,LONG node,NINT locale);
#endif
#ifdef UNIFRMAT
extern  NINT         ctconvert1(pTEXT tp,pConvMap map,pConvBlk blk,VRLEN len,UTEXT dbyte,NINT cnv_mode);
extern  VOID         rev_fhdr(pTEXT tp);
extern  VOID         rev_shdr(pTEXT tp);
extern  VOID         rev_rhdr(pTEXT tp);
extern  VOID         rev_thdr(pTEXT tp);
extern  VOID         rev_nhdr(pTEXT tp);
extern  VOID         rev_vhdr(pTEXT tp);
#endif
extern  VOID         ctsetkeypos(COUNT keyno,LONG node,NINT elm,pVOID kp,COUNT keylen);
extern  COUNT        rblsavres(COUNT keyno,pTEXT idxname,pTEXT fileword);
extern  COUNT        ctrvrng(COUNT hghno);
#ifndef ctCLIENT
extern  NINT         ctuutbl(ctRECPT pntr,COUNT datno pinHan);
#endif
#ifdef FPUTFGET
#ifdef LOCK_TEST
extern  COUNT        chkutbl(COUNT datno,NINT lokmod,ctRECPT pntr);
#endif
#endif
extern  pTEXT        rbgetbuf(VRLEN bufsiz);
#ifdef ctMTFPG_LOKCNF
extern  NINT         ctchkmlok(pCTFILE ctnum,pNINT mbrmap);
extern  NINT         ctuhashx(LONG uid,LONG pntr);
#else
extern  NINT         ctuhash(COUNT filno,LONG pntr);
#endif
#ifdef FASTCOMP
extern  COUNT        CTCOMP(pTEXT val1,pTEXT val2,COUNT keylen);
extern  UCOUNT       CTCOMPU(pTEXT np, pTEXT op, UCOUNT smlen);
#endif
extern  VRLEN        igetifil(COUNT datno,LONG buflen,pVOID bufptr,pLONG pdf);
#ifdef MULTITRD
extern  NINT         ct_tflmod(pCTFILE ,NINT ,NINT ,LONG );
extern  NINT         ctTCdone(VOID );
extern  NINT         ctendthread(NINT , NINT, NINT (*)(VOID ));
extern  NINT         ctmemmon(VOID );
extern  NINT         ctunblkq(NINT qindex);
extern  NINT         ctblkbsy(pSEMA sema);
extern  pVOID        ctgvallc(NINT sOWNR,NINT alloc);
extern  NINT         ctUserAlive(NINT chkcomm);
extern 	NINT 	     validusr(pLQMSG l,NINT t);
extern  pTEXT 	     ctGetServerName(void);

#endif
#ifndef ctrt_filcmp
extern  NINT         ctrt_filcmp(pTEXT n1,pTEXT n2);
#endif
#ifdef ctSQL
extern ctCONV  LONG  ctDECL sqllock(COUNT lokmod);
extern  COUNT        ADDRECs(COUNT datno,pVOID recptr,NINT srlmod);
extern  COUNT        ADDVRECs(COUNT datno,pVOID recptr,VRLEN varlen,NINT srlmod);
extern NINT blksrlint(NINT,NINT);
#endif
#ifndef nd_digPROTO
#define nd_digPROTO
extern  NINT         nd_dig(pTEXT digbuf,NINT digno);
extern  VOID         nd_digenc(pTEXT digbuf,NINT digno,NINT val);
#endif
extern  pTEXT        uTFRMKEY(COUNT keyno,pVOID	tarptr);
extern  pTEXT        i2TFRMKEY(COUNT keyno,pVOID tarptr,LONG pntr,NINT usepntr);
extern  VOID         ctsysint(pCTINIT1 pconfig);
extern  VOID         ctclnhdr(pCTFILE ctnum);
extern  COUNT        chkvfin(COUNT datno,COUNT k,COUNT slen);
extern  VRLEN ctDECL ctcdelm(pTEXT tp,NINT ch,VRLEN len);

#ifndef CTBOUND
#ifdef ctCLIENTcore
extern pMTEXT recommbuf(pSrvrGbls pGlobals, VRLEN len, VRLEN olen  pinHan);
#endif
extern ctCONV  COUNT ctDECL COMMBUF(pULONG memchg,LONG newslct,VRLEN bufsiz,pLQMSG lqp);
extern pCommFuncPtrs ctCommLoad(ppVOID commGbl,pTEXT dllname);
extern VOID ctCommUnload(ppVOID commGbl);
#endif
extern  VOID         ctlnhdr(pCTFILE ctnum);
extern  COUNT        ishtifil(COUNT datno,NINT cloze);
extern  pTREBUF      ctintnod(pTREBUF buf,pCTFILE knum,LONG node);
extern  COUNT        ctgetsec(COUNT filno,pCTFILE ctnum,pTEXT uid,pTEXT gid,pTEXT pwd,pLONG pm);
extern  NINT         ctkw(pTEXT kw,pTEXT *kw_lst,NINT nkw);
#ifdef ctCLIENT
#ifdef ctCLIENTcore
extern  pCOUNT       ctlfsegget(COUNT keyno,COUNT numseg,ppCOUNT paltseq pinHan);
extern  COUNT        ctgetseginfo(COUNT keyno,COUNT segno,COUNT mode pinHan);
#endif
#else /* ctCLIENT */
extern  pCOUNT       ctlfsegget(COUNT keyno,COUNT numseg,ppCOUNT paltseq);
extern  COUNT        ctgetseginfo(COUNT keyno,COUNT segno,COUNT mode);
#endif /* ctCLIENT */
extern  VOID         ctlfsegput(COUNT keyno,COUNT segno,COUNT pos,COUNT len,COUNT mod);
extern  VOID	     ctmakid(pLONG id,pCTFILE ctnum);
extern  pTEXT        ctrtnam(pTEXT sp);
extern  COUNT        ctspcopn(NINT op_code,pCTFILE ctnum);
#ifndef ctCLIENT
extern  VOID         ctuftbl(COUNT ufil,pCTFILE ctnum,NINT uncond pthHan);
extern  COUNT        ctultbl(ctRECPT pntr,COUNT datno,NINT lokmod,NINT ismflg pthHan);
#endif
extern ctCONV  LONG  ctDECL ctdidx(COUNT idx,pTEXT names);
extern  VOID         setifil(pIFIL ifilptr,pIFILBLK piblk,pTEXT dataextn,pTEXT indxextn,pTEXT fileword,LONG permmask,pTEXT groupid);
extern ctCONV  COUNT ctDECL iTMPNAME(pTEXT bufptr,VRLEN bufsiz);
extern  COUNT        ctkeytrn(pCTFILE knum);
extern  ctRECPT      seqikey(COUNT keyno,pTEXT idxval,NINT mode);
extern  COUNT        putdodas(COUNT datno,pDATOBJ doda,UCOUNT numfld);
extern  COUNT        putdodan(COUNT datno,pDATOBJ doda,UCOUNT numfld);
extern  NINT	     inamidx(NINT ctNAMINDXmode,pCTFILE ctnum,pLONG id,LONG filno);
#ifdef ctSRVR
extern  NINT         ctlaunch(pTEXT );
extern  VOID         ctsrvclndwn(VOID );
extern  COUNT        ctaddusr(COUNT datno,pVOID recptr);
extern  COUNT        ctchgusr(pTEXT bufptr);
#endif
extern  VOID         mbfreel(pVOID objptr);
extern  VOID         mbfrenl(ppVOID pobjptr);
#ifdef ctHFREE
extern  VOID         cthfree(pVOID p);
#endif
extern  COUNT        igetdefblk(COUNT filno,pDEFDEF defp,COUNT srvrmode);
extern  COUNT        ctsetseq(pCTFILE knum,pCOUNT aq);
#ifdef MULTITRD
extern  VOID         ctsetnmda(pLOGBLK logb,pLQMSG logq,COUNT fils);
extern  VOID         ctsetuginfo(pLQMSG lqp);
extern  COUNT        ctsetres(pCTFILE ctnum,COUNT ufilno,UCOUNT filmod,pCRESEC pcresec);
#else
extern  COUNT        ctsetres(pCTFILE ctnum,UCOUNT filmod);
#endif
extern  VOID         ctfusrclr(NINT mode,NINT sOWNR);
extern  VOID         ctfilok(VOID );
extern  VOID         ctiflnam(pIFILBLK piblk,pTEXT nambuf,NINT mode,pTEXT ext);
#ifdef NEWIFIL
extern  pTEXT        makifil(pTEXT buf,pTEXT dext,pTEXT iext,LONG defwrd,UINT rlen,pUINT plen,NINT mode);
#else
extern  pTEXT        makifil(pTEXT buf,pTEXT dext,pTEXT iext);
#endif
#ifndef CTBOUND
extern  COUNT        ctgetseg(COUNT keyno,pTEXT bufptr,VRLEN size);
extern  COUNT        ctgetmap(COUNT datno,pTEXT bufptr,VRLEN size);
#else
extern  NINT         ctStopUser(VOID );
extern  VOID         ctStopServer(NINT retmod);
#endif
extern ctCONV  COUNT ctDECL OPNIFILz(pIFILBLK pifilblk);
extern ctCONV  COUNT ctDECL PUTIFILz(pIFILBLK pifilblk);
extern ctCONV  COUNT ctDECL CREIFILz(pIFILBLK pifilblk);
extern ctCONV  COUNT ctDECL PRMIIDXz(pIFILBLK pifilblk);
extern ctCONV  COUNT ctDECL RBLIIDXz(pIFILBLK pifilblk);
extern ctCONV  COUNT ctDECL TMPIIDXz(pIFILBLK pifilblk);
extern ctCONV  COUNT ctDECL RBLIFILz(pIFILBLK pifilblk);
extern ctCONV  COUNT ctDECL CMPIFILz(pIFILBLK pifilblk);
extern         COUNT CLIFILz(pIFILBLK pifilblk);
extern         COUNT DELIFILz(pIFILBLK pifilblk);
extern  COUNT        iintree(pCTINIT1 pconfig);
extern ctCONV  COUNT ctDECL TRANRDY(VOID );
extern  UINT         ctadjadr(UINT alignment,UINT fkind,pVOID fadr);
extern  NINT         ctadjfld(UINT fkind,pTEXT fadr);
extern  pVOID        ctrdef(COUNT datno,pUINT plen,NINT type);
extern  COUNT        getrhdr(pCTFILE dnum,LONG pntr,pRESHDR presblk,NINT namode);
extern  COUNT        putrhdr(pCTFILE dnum,LONG pntr,pRESHDR presblk);
extern ctCONV  COUNT ctDECL SETDEFBLK(COUNT datno,pDEFDEF defp);
extern ctCONV  COUNT ctDECL GETDEFBLK(COUNT datno,pDEFDEF defp);
extern  pTEXT        cptifil(pIFIL ifilp,pTEXT buf,pVRLEN plen,pTEXT dext, pTEXT iext);
extern  COUNT        ctrstdef(COUNT datno,pIFIL ip,pTEXT dext,pTEXT iext);
#ifndef cpybig
extern  pTEXT        bigadr(pTEXT tp,VRLEN offset);
extern  VOID         cpybig(pTEXT dp,pTEXT sp,VRLEN n);
#endif
#ifdef MULTITRD
extern  COUNT        getintr(FILE *ifd,pCOUNT pbufs,pCOUNT pkeys,pCOUNT psecs,pCOUNT pdats,COUNT userprof,pLQMSG lqp,COUNT files);
#else
extern  COUNT        getintr(FILE *ifd,pCOUNT pbufs,pCOUNT pkeys,pCOUNT psecs,pCOUNT pdats,COUNT userprof);
#endif
extern  LONG         ctfsize(pCTFILE ctnum);
extern  COUNT        ctrcvlog(pTEXT msg,NINT err);
extern  pTEXT        ctdate(LONG pt);
#ifdef CTSUPER
extern  COUNT        cthstopn(COUNT ufilno,pTEXT filnam,UCOUNT filmod);
extern  COUNT        ctcresi(COUNT datno,UCOUNT filmod);
#ifdef MULTITRD
extern  COUNT        cresmem(COUNT hstno,COUNT memno,pTEXT afilnam,pTEXT sfilnam, UCOUNT len,COUNT keytyp,COUNT keydup,COUNT memb,UCOUNT filmod,pCRESEC pcresec);
#else
extern  COUNT        cresmem(COUNT hstno,COUNT memno,pTEXT afilnam,pTEXT sfilnam, UCOUNT len,COUNT keytyp,COUNT keydup,COUNT memb,UCOUNT filmod);
#endif
extern  NINT         ctsname(pTEXT fn,pTEXT sfn);
#endif
#ifndef ctCLIENT

#ifndef thType
#define thType	     /* RAB: Error with MSC6 with ctjump.c */
#endif
extern  LONG         ctdhupd(LONG ,NINT ,pCTFILE ,COUNT thType);

extern  pLSTITM      ctgetlst(NINT listype pinHan);
extern  VOID         ctputlst(pLSTITM lp,NINT listype pinHan);
#endif
#ifndef ctsfill
extern  VOID         ctsfill(pVOID dp,NINT ch,UINT n);
#endif
#ifndef ctbfill
extern  VOID         ctbfill(pVOID dp,NINT ch,VRLEN n);
#endif
extern  VOID         ctsetlst(VRLEN nodepage,VRLEN datapage,UINT nidx,UINT ndat);
extern  pTEXT        ctgetmem(VRLEN iosize);
extern  VOID         ctputmem(pTEXT loc);
extern  pTEXT        ctnt_getmem(VRLEN iosize); /* communications use - see ctasup.c*/
extern  VOID         ctnt_putmem(pTEXT loc);	/* communications use - see ctasup.c*/
extern  VOID         ctputmemn(ppVOID ploc);
#ifndef CTBOUND
extern  VOID         ctbldlcl(NINT sOWNR,COUNT filno,pCTFILE ctnum,NINT auxflg);
#endif
extern  VOID         ctcatend(NINT mode,NINT loc,COUNT	fil,LONG pos);
extern  NINT         ctcfill(pTEXT tp,NINT ch,VRLEN len);
extern  LONG         ctpstk(pTEXT sp);
extern  VOID         ctrstk(VOID );
#ifndef ctThrdFPG
extern  NINT _lds_   ctdefer(LONG );
#endif
extern 	LONG _lds_ 	ctGetSrVariable(NINT this_var); /* Get Server Variable */
extern 	LONG _lds_ 	ctNPctrt_printf(pTEXT p);
extern  LONG         cttime(VOID );
#ifndef ctsysnap
extern  NINT	     ctsysnap(LONG);
#endif
extern  COUNT        iaddkey(COUNT keyno,pTEXT target,LONG recbyt,NINT typadd);
extern  COUNT        adroot(pCTFILE knum,LONG lpntr,LONG rpntr,pTEXT idxval);
extern  pTREBUF      newnod(PFAST pCTFILE knum,pLONG pnode,NINT virgin);
extern  COUNT        ctwrtbuf(pDATBUF db);
extern  NINT         ctflushd(pCTFILE ctnum,NINT keep);
extern  NINT         ctblock(NINT op_code,pCTFILE ctnum,LONG recbyt,pTEXT recptr,VRLEN len,pLONG pretsiz);
extern  pTEXT        mballc(NINT numobj,UINT sizobj);
#ifndef mblllc
extern  pTEXT        mblllc(NINT numobj,VRLEN sizobj);
#endif
extern  VOID         mbfren(ppVOID pobjptr);
extern  VOID         mbfree(pVOID objptr);
#ifdef MTMEMRY
extern  LONG         arep(COUNT mode);
#endif
extern  COUNT        uerr(COUNT err_no);
extern  VOID         terr(NINT err_no);
extern  VOID         revobj(pTEXT tp,NINT len);
extern  VOID         revbyt(pTEXT tp,NINT len);
extern  VOID         revwrd(pTEXT tp,NINT len);
extern  VOID         ctchknum(NINT opmode);
extern  VOID         ctinrnum(NINT opmode,RNDFILE retval);
extern  VOID         cpylod(ppTEXT hdp,pVOID sp,UINT n);
#ifndef cpylodl
extern  VOID         cpylodl(ppTEXT hdp,pVOID sp,VRLEN n);
#endif
extern  VOID         cpynbuf(pTEXT dp,pTEXT sp,UINT n);
extern  VOID         cpysrc(pVOID dp,ppTEXT hsp,UINT n);
#ifndef cpysrcl
extern  VOID         cpysrcl(pVOID dp,ppTEXT hsp,VRLEN n);
#endif
extern  NINT         ctcomexc(NINT mode,LONG tran,ppCOMLST hcmlist);
extern  VOID         ctlowhsh(pBHL pbhl,pBHL anchor,NINT end);
extern  COUNT        ctfixdel(pCTFILE ctnum,pCTFILE knum);
extern  COUNT        ctio(NINT op_code,pCTFILE ctnum,LONG recbyt,pVOID bufadr,VRLEN iosize);
extern  NINT         ctrelhdr(pCTFILE ctnum);
extern  NINT         ctrelbuf(NINT bad,pTREBUF bp,NINT sOWNR);
extern  UCOUNT       diffimag(pTEXT newptr,UCOUNT newlen,pTEXT oldptr,UCOUNT oldlen);
extern  NINT         compar(pTEXT val1,pTEXT val2,pCTFILE knum);
extern  ppTEXT       chkcopy(VOID );
extern  pTEXT        ctputk(COUNT keyno,pTEXT recptr,LONG pntr,NINT mode,VRLEN datlen);
extern  COUNT        tstrec(PFAST pCTFILE dnum,LONG recbyt);
extern  COUNT        prthdr(COUNT filno);
extern  COUNT        prtnod(COUNT filno,LONG nodex);
extern  COUNT        chkidx(COUNT filno);
extern  COUNT        idelchk(COUNT keyno,pTEXT target,LONG recbyt,NINT blkmode);
extern  NINT         ctdnode(pTEXT sp);
extern  VOID	     ctdnodestop(VOID);
extern  VOID         ctqnode(pCTFILE knum,LONG node,NINT mode);
extern  VOID         ctrmvexc(pTREBUF buffer,UINT elm);
extern  COUNT        ctdelkey(PFAST pCTFILE knum,pTEXT idxval,LONG pntr,NINT blkmode);
extern  COUNT        rtnode(pTREBUF buffer,LONG node,pCTFILE knum);
extern  COUNT        ckflmp(COUNT filno,NINT owner);
extern  pFUSR        ctrvfl(NINT sOWNR,COUNT afil);
extern  COUNT        ctdwnfil(NINT sOWNR,COUNT filno,NINT cloze,COUNT ufil,NINT direct);
extern  COUNT        chkopn(pTEXT fn,NINT mode);
extern  NINT         chkredf(COUNT ufil,pCTFILE ctnum pthHan);
extern  COUNT        getctf(VOID );
extern  COUNT        getctm(COUNT blocks,pCTFILE btnum);
extern  VOID         retctf(COUNT filno);
extern  COUNT        ctfree(NINT mode);
extern  NINT         ctstpsrv(VOID );
extern  NINT         ctrelusr(NINT owner);
extern  LONG         extfil(pCTFILE ctnum,VRLEN rsize,VRLEN align);
extern  pCTFILE      tstfnm(COUNT filno pthHan);
extern  COUNT        vtclose(VOID );
extern  pFILE	     ctgetstream(pTEXT sn,pTEXT mode,NINT repeat);
extern  VOID         inrfil(PFAST pCTFILE ctnum,NINT mode,NINT direct);
extern  COUNT        tstupd(PFAST pCTFILE ctnum);
extern  COUNT        redhdr(PFAST pCTFILE ctnum);
extern  COUNT        filhdr(PFAST pCTFILE ctnum);
extern  COUNT        wrthdr(PFAST pCTFILE ctnum);
extern  COUNT        iopnfil(COUNT filno,pTEXT filnam,UCOUNT filmod);
extern  COUNT        wrtnod(PFAST pTREBUF buffer);
extern  COUNT        iclsfil(COUNT filno,UCOUNT filmod);
extern ctCONV  COUNT ctDECL reset_cur(PFAST COUNT keyno,ctRECPT pntr,pTEXT recptr);
#ifdef ctCLIENT
extern  ctCONV COUNT ctDECL reset_cur2(PFAST COUNT keyno,ctRECPT pntr,pTEXT recptr,pVRLEN plen);
#else
extern  ctCONV COUNT ctDECL reset_cur2(PFAST COUNT keyno,ctRECPT pntr,pTEXT recptr,pVRLEN plen pthHan);
#endif
extern  COUNT        RTSCRIPT(pTEXT script,pDATOBJ extdoda,pTEXT outname,LONG options,pTEXT errbuf,VRLEN buflen,COUNT filmod);
extern  LONG         chkism(COUNT datno);
extern  VOID         setsrlpos(COUNT datno,pTEXT recptr);
extern  VOID         iundo(COUNT op_code,COUNT datno,COUNT i,LONG pntr,LONG old_pntr);
extern  COUNT        addikey(COUNT datno,pTEXT recptr,LONG pntr,VRLEN datlen);
extern  COUNT        rwtikey(COUNT datno,pTEXT recptr,LONG pntr,LONG old_pntr,VRLEN datlen);
extern  COUNT        delikey(COUNT datno,LONG pntr);
#ifdef MULTITRD
extern  NINT         ctiisam(pLOGBLK plog,pLQMSG lqp);
extern  VOID         ctfisam(NINT sOWNR,COUNT fils);
#else
extern  NINT         ctiisam(COUNT bufs,COUNT fils,COUNT sect,COUNT dbufs,COUNT userprof);
extern  VOID         ctfisam(COUNT fils);
#endif
extern  COUNT        setudat(FILE *ifd,COUNT datno);
extern  COUNT        setukey(FILE *ifd,COUNT keyno,COUNT keytyp);
extern  pCTFILE      tstifnm(COUNT filno pthHan);
extern  COUNT        ierr(COUNT err_cod,COUNT file_no);
extern  COUNT        addlok(LONG pntr,COUNT datno);
extern  TEXT         ucase(TEXT t);
extern  COUNT        ctasskey(COUNT keyno,pTEXT recptr,PFAST pTEXT txt,VRLEN datlen);
extern  COUNT        getdatr(FILE *ifd,pCOUNT pdatno,pTEXT pname,pUCOUNT pdatln,pCOUNT pdkeys,pUCOUNT pxtsiz,pCOUNT pfilmd);
extern  COUNT        getidxr(FILE *ifd,COUNT datno,COUNT dfilemd,COUNT j,pCOUNT pkeyno,pTEXT pname,pCOUNT pklen,pCOUNT pktyp,pCOUNT pdflg,pCOUNT pnmem,pUCOUNT pxtsiz,pCOUNT pfilmd);
extern  COUNT        getambr(FILE *ifd,COUNT datno,COUNT dfilemd,COUNT j,COUNT keyno,pCOUNT pklen,pCOUNT pktyp,pCOUNT pdflg);
extern  VOID         ctsetlog(pCTFILE ctnum,UCOUNT filmod);
extern  COUNT        icredat(COUNT datno,pTEXT filnam,pTEXT sfilnam,UCOUNT datlen,UCOUNT xtdsiz,UCOUNT filmod,pCTFILE hnum,LONG hp);
extern  NINT         ctidxcap(pCTFILE ,COUNT ,COUNT ,COUNT ,COUNT );
extern  COUNT        icreidx(COUNT keyno,pTEXT filnam,pTEXT sfilnam,COUNT keylen,COUNT keytyp,COUNT keydup,COUNT nomemb,UCOUNT xtdsiz,UCOUNT filmod,pCTFILE hnum,LONG hp);
extern  pCTFILE      icremem(COUNT keyno,COUNT keylen,COUNT keytyp,COUNT keydup,COUNT membno);
extern  UINT         ctelmexc(pTREBUF buffer,NINT elm,pULONG ptran);
extern  NINT         ctexcp(pTREBUF buffer,NINT elm,NINT adflag,LONG tran);
#ifdef MULTITRD
extern  NINT         ctgetkbf(NINT sOWNR,COUNT keyno,COUNT keylen);
#else
extern  NINT         ctgetkbf(pCTFILE knum,COUNT keylen);
#endif
#ifndef ctCLIENT
extern  NINT         nodser(PFAST pTREBUF buffer,pTEXT idxval,TEXT stratg,NINT adflag pthHan);
#endif
extern  VOID         ctputhsh(pTREBUF buf,NINT list,NINT end);
extern  VOID         ctclrhsh(pTREBUF buf);
#ifndef ctCLIENT
extern  pTREBUF      ctgetnod(NINT lokind,LONG node,pCTFILE knum pthHan);
extern  pTREBUF      lrubuf(NINT mode,LONG node,pCTFILE knum pthHan);
#endif
extern  VOID         ctnodcap(pTREBUF buffer,pCTFILE knum,NINT lf);
#ifdef FPUTFGET
extern  LONG         gtroot(PFAST pCTFILE knum);
#endif
extern  pTEXT        valpnt(PFAST pTREBUF buffer,PFAST NINT elm);
extern  pTEXT        hghpnt(PFAST pTREBUF buffer);
extern  LONG         nodpnt(PFAST pTREBUF buffer,PFAST NINT elm);
extern  LONG         drnpnt(PFAST pTREBUF buffer,PFAST NINT elm);
extern  pTEXT        expval(PFAST pTREBUF bp,NINT n);
extern  COUNT        nodctl(pCTFILE ctnum,LONG node,pTREBUF buffer,pVHDR pvrhdr);
extern  COUNT        ct_strip(PFAST pCTFILE knum);
extern  COUNT        iloadkey(COUNT keyno,pTEXT target,LONG recbyt,COUNT typadd,pLONG pnv);
extern  VOID         newleaf(PFAST pTREBUF bp,PFAST pCTFILE kp);
extern  VOID         nonleaf(PFAST pTREBUF bp,PFAST pCTFILE kp);
extern  COUNT        rerr(COUNT err_cod,COUNT file_no);
extern  COUNT        yesno(VOID );
extern  COUNT        vcparm(pUCOUNT pp,UCOUNT rv,pTEXT txt);
extern  COUNT        vtparm(pTEXT pp,UCOUNT rv,pTEXT txt);
extern  COUNT        prnprm(UCOUNT hv,UCOUNT rv,pTEXT txt);
extern  COUNT        RBLDATX(COUNT datno,pTEXT datnam,COUNT datlen,UCOUNT xtdsiz,COUNT filemd,pTEXT fileword);
extern  COUNT        RBLIDX(COUNT datno,pTEXT datname,COUNT keyno,pTEXT fileword,pFILE tfp,pLONG duprej,NINT purge);
extern  COUNT        RBLMEM(COUNT datno,pTEXT datname,COUNT keyno,COUNT membno,pTEXT fileword,pFILE tfp,pLONG duprej,NINT purge);
extern  VOID         ctupdkey(pCTFILE knum);
extern  COUNT        scndat(pCTFILE dnum);
#ifndef ctCLIENT
extern  VOID         pshlst(pRECLOK plok,pCTFILE ctnum,NINT bin pinHan);
#endif
extern  COUNT        ctaddblk(ppBLKLST blkh,ppBLKLST blkt,NINT sOWNR,pRECLOK cp,pVOID cb,NINT loktyp);
extern  NINT         ctremblk(ppBLKLST blkh,ppBLKLST blkt,pRECLOK cp,pCTFILE ctnum);
extern  COUNT        cts_lok(NINT usrn,COUNT filno,NINT loktyp,NINT mode,LONG recbyt,NINT dlkfre);
extern  COUNT        LOCK(LONG node,pCTFILE knum);
extern  NINT         iunlock(NINT usr,LONG pos,pCTFILE ctnum);
extern  COUNT        UNLOCK(LONG node,pCTFILE knum);
extern  COUNT        DLOCK(LONG recbyt,pCTFILE dnum,NINT lokmod);
#ifdef MULTITRD
extern  COUNT        RLOCK(LONG recbyt,pCTFILE dnum,NINT lokmod);
#else
extern  COUNT        RLOCK(LONG recbyt,pCTFILE dnum);
#endif
extern  COUNT        UDLOCK(LONG recbyt,pCTFILE dnum);
extern  LONG         ieqlkey(COUNT keyno,pTEXT target);
extern  LONG         igtekey(COUNT keyno,pTEXT target,pTEXT idxval);
extern  LONG         ilstkey(COUNT keyno,pTEXT idxval);
extern  LONG         nxtikey(COUNT keyno,pTEXT idxval);
extern  LONG         prvikey(COUNT keyno,pTEXT idxval);
extern  LONG         igtkey(PFAST COUNT keyno,pTEXT target,pTEXT idxval);
extern  NINT         ctscnexc(pTREBUF buffer,NINT elm);
extern  NINT         ctrevexc(pTREBUF buffer,NINT elm);
extern  LONG         fndkey(PFAST pCTFILE knum,pTEXT target,TEXT stratg,pTEXT idxval);
#ifndef ctCLIENT
extern  COUNT        chkset(COUNT keyno,LONG recpos,pTEXT recptr,pVRLEN plen pthHan);
#endif
extern  COUNT        setset(COUNT keyno,pTEXT target,PFAST COUNT siglen);
extern  COUNT        igetaltseq(COUNT keyno,pCOUNT altseq,COUNT srvrmode);
extern  COUNT        hdrupd(pCTFILE knum,LONG chgnum);
extern  COUNT        ctmrkexc(pTREBUF buffer,NINT elm,UINT adflag,NINT shflag,LONG trbf);
extern  VOID         ctrstexc(pCTFILE ctnum,pTREBUF buf);
#ifndef ctCLIENT
extern  LONG         ctabtexc(NINT mode,COUNT keyno,LONG node,LONG tran,pLONG ptran,NINT num pthHan);
#endif
extern  COUNT        ctabtnod(pCTFILE ctnum);
extern  COUNT        ctclup(NINT sOWNR,pTREBUF buffer,NINT tranmode,NINT shrink);
extern  COUNT        putnod(PFAST pTREBUF buf,PFAST COUNT nodsiz,NINT hash,NINT tryulk);
extern  VOID         prpdup(pTEXT ip,pCTFILE knum,pLONG pntrp);
extern  pTREBUF      movrgt(pTEXT idxval,PFAST pCTFILE knum,PFAST pTREBUF buffer,NINT adflag);
extern  VOID         shfrgt(PFAST NINT n,pTREBUF bp,UINT strbyt);
extern  COUNT        ideladd(pCTFILE knum,pTEXT td,LONG rd,pTEXT ta,LONG ra,NINT cf);
extern  VOID         insexp(PFAST pTREBUF bp,pTEXT ip,LONG pntr);
extern  COUNT        delexp(PFAST pTREBUF bp);
extern  LONG         inewvrec(pCTFILE vnum,COUNT ufil,VRLEN varlen,NINT lnkflg,NINT cntflg);
extern  COUNT        iretvrec(COUNT datno,COUNT ufil,LONG recbyt);
extern  COUNT        getvhdr(PFAST pCTFILE vnum,LONG pntr,pVHDR phdr);
extern  COUNT        putvhdr(PFAST pCTFILE vnum,LONG pntr,pVHDR phdr);
extern  COUNT        frmlkey(pCTFILE ctnum,pTEXT pkeybuf,pTEXT plen,LONG pntr);
extern  COUNT        chkvhdr(PFAST pVHDR phdr);
#ifndef ctCLIENT
extern  VRLEN        prprdv(COUNT datno,LONG recbyt,pTEXT recptr,VRLEN bufsiz,NINT redmod pthHan);
#endif
extern  LONG         chkvsm(COUNT datno);
extern  COUNT        ctVERIFY(COUNT keyno,pLONG pcnt);
extern  NINT         link_verify(pCTFILE knum,LONG node,LONG pnode,NINT lev,NINT pelm,NINT pnkv);
extern  NINT         leaf_verify(pCTFILE knum,pLONG pcnt);
extern  NINT         leaf_extra(COUNT keyno,COUNT datno);
extern  COUNT        ctseek(RNDFILE cfd,LONG recbyt);
extern  NINT         ctswitch(pCTFILE ctnum);
extern  COUNT        ictio(NINT op_code,pCTFILE ctnum,LONG recbyt,pVOID bufadr,VRLEN iosize,pLONG pretsiz);
extern  COUNT        ctsysio(COUNT op_code,RNDFILE cfd,LONG recbyt,pVOID bufadr,VRLEN iosize,pLONG pretsiz);
extern  COUNT        dltfil(pTEXT filnam);
extern  COUNT        cnvdltfil(pTEXT fn);
extern  COUNT        renfil(pTEXT oldnam,pTEXT newnam);
extern  NINT         cttcre(pNINT ,NINT (*)(VOID ),UINT ,NLONG );
extern  NINT         ctqcre(NINT qindex,NINT qsize);
extern  NINT         ctqwrt(NINT qindex,pVOID qmsgptr,NINT qmsglen);
extern  NINT         ctqred(NINT qindex,pVOID qmsgptr,LONG timeout);
extern NINT ctqchk(pDQMSG);
extern  NINT         ctmqwrt(NINT mode,pTEXT msg,COUNT rc);
extern  VOID         cttsksetup(VOID );
extern  NINT _lds_   ctsemclr(pSEMA sema,NINT own);
extern  NINT         ctsemwat(pSEMA sema,LONG wait,NINT own);
extern  NINT _lds_   ctsemrqs(pSEMA sema,LONG wait,NINT own);
#ifdef ctCREATE_SEMA
extern  NINT _lds_   ctsemcls(pSEMA sema);
#endif
#ifdef ctSEMBLK
extern  NINT _lds_   ctblkclr(pSEMA sema,NINT own);
extern  NINT         ctblkwat(pSEMA sema,LONG wait,NINT own);
extern  NINT _lds_   ctblkrqs(pSEMA sema,LONG wait,NINT own);
#endif
#ifdef ctSEMTIM
extern  NINT _lds_   cttimclr(pSEMAtim sema,NINT own);
extern  NINT         cttimwat(pSEMAtim sema,LONG wait,NINT own);
extern  NINT _lds_   cttimrqs(pSEMAtim sema,LONG wait,NINT own);
#endif
#ifdef ctSEMUTX
extern  NINT         ctmutclr(pSEMAmut sema,NINT own);
extern  NINT         ctmutrqs(pSEMAmut sema,NINT own);
#endif
#ifdef ctSEMCOM
#ifndef CTBOUND
extern  NINT	     ctcomclr(pLQMSG ,NINT , NINT);
extern  NINT	     ctcomrqs(pLQMSG ,NINT , LONG ,NINT);
extern  NINT	     ctcomwat(pLQMSG ,NINT , LONG ,NINT);
#endif
#endif
extern  NINT         ctlclose(pCTFILE ctnum,COUNT clmode,NINT cleanup);
extern  COUNT        mbclos(pCTFILE ctnum,COUNT clmode);
#ifdef ctMIRROR
extern  NINT         ctswtmir(pCTFILE ctnum,NINT err);
extern  COUNT        ctmirhdr(pCTFILE ctnum,ctRECPT mhdrpos);
extern  NINT         ctmname(pTEXT fn,ppTEXT mp);
extern  NINT         ctmalert(pTEXT pn,pTEXT mn,NINT err);
extern  COUNT        ctmopen(pCTFILE ctnum,UCOUNT filmod,NINT cleanup);
extern  RNDFILE      ctopen(pCTFILE ctnum,UCOUNT filmod);
extern  COUNT        ctmcreate(pCTFILE ctnum,NINT idxflag,UCOUNT filmod);
#ifdef MULTITRD
extern  VOID         ctfrcmir(pCTFILE ctnum,NINT sOWNR);
#else
extern  VOID         ctfrcmir(pCTFILE ctnum);
#endif
#endif
extern  RNDFILE      mbopen(pCTFILE ctnum,pTEXT fn,COUNT opmode);
extern  RNDFILE      mbcrat(pCTFILE ctnum,COUNT opmode);
extern  COUNT        mbsave(pCTFILE ctnum);
#ifndef ctsave
extern  COUNT        ctsave(pCTFILE ctnum);
#endif
extern  VOID         flushdos(COUNT datno);
extern  COUNT        filrng(COUNT filno,COUNT nm,UCOUNT filmod);
extern  COUNT        setimap(COUNT i,pIFIL ip,COUNT keyno,COUNT exsidx);
#ifdef BIGCHECK
extern  NINT         CTCFILL(pTEXT tp, NINT ch, UINT smlen);
#endif

#ifdef MULTITRD
extern  COUNT        INTREEz(pLOGBLK plog,pLQMSG lqp);
extern  COUNT        INTISAMz(pLOGBLK plog,pLQMSG lqp);
extern  COUNT        CREISAMz(pISAMBLK pisam,pLQMSG lqp);
extern  COUNT        OPNISAMz(pISAMBLK pisam,pLQMSG lqp);
 
extern  NINT         ctcnvfn(pTEXT fn);
extern  VOID         cttsus(LONG tid);
extern  NINT         ctloglog(NINT sOWNR,pTEXT userid,NINT mode);
extern  NINT         ctclrsrvr(pLQMSG lqp);
extern  NINT         ctqcls(NINT qindex);
extern  VOID         stopctsrvr(VOID);
extern  NINT         PIFlush(NINT sOWNR,VRLEN iosize);
extern  VOID         ctsetnmda(pLOGBLK logb,pLQMSG logq,COUNT fils);
extern  COUNT        ctugfil(NINT t);
extern  COUNT        ctsetsec(pCTFILE ctnum,pCRESEC pcresec,COUNT filno,LONG respos,pTEXT userid);
extern  COUNT        ctchksec(pCTFILE ctnum,pTEXT fileword,COUNT filno,NINT admflg);
extern  VOID         ctsetuginfo(pLQMSG lqp);
extern  NINT         ctismem(VRLEN iosize,NINT sOWNR);
extern  NINT         ctmemavl(NINT sOWNR,VRLEN iosize,NINT list,NINT syslmt,NINT usrlmt,NINT mattr,NINT force);
extern  NINT         blkcmtint(NINT sOWNR,NINT mode,COUNT lokmod);
#ifndef CTBOUND
extern  NINT         ctntio(COUNT func,pTEXT mp,pVAB pvab,pLQMSG lqp);
extern  NINT         ctlogoff(NINT sOWNR,pTEXT userid);
#endif
extern  VOID         setcommbuf(pULONG memchg,pLQMSG lqp);
extern  VOID         ctdec(pTEXT dp,pTEXT sp,NINT len,LONG k1,LONG k2);
extern  NINT         ctblksrvr(pLQMSG  lqp);
extern  COUNT        srvrwait(NINT sOWNR,pLQMSG l,LONG delay,UCOUNT noMem);
extern  VOID         initctsrvr(VOID);
extern  VOID         srvrcopyright(pTEXT legend);
extern  VOID         srvrflashmsg(pTEXT legend);
extern  VOID         ctenc(pTEXT dp,pTEXT sp,NINT len,LONG k1,LONG k2);
extern  VOID         startctsrvr(VOID);
extern  NINT         ctclrclnt(pLQMSG lqp);
extern NINT ctkilblk(NINT);

#ifdef ___FNPglobals___
extern  Thread_     *makeThread(ProcPtr thread,int32 size,int32 refcon,int32 priority,int32 freq,int32 narg,VOID *parm);
extern  byte        *memory(int32 nbr, int32 size);
extern  VOID         delayDefer(LONG len);
extern  VOID        *threadInit(VOID *term);
extern  VOID         killThread(Thread_ *t);
extern  int32        getThdTime(VOID);
extern  Thread_     *myThread(VOID);
extern  VOID         sleepThread(Thread_ *t, int32 max);
extern  VOID         DEFER(VOID);
#endif /* ___FNPglobals___ */
#ifdef MULTITRD
extern  VOID	     ctchgpre(pCTFILE ctnum,NINT domembers);
#endif
#endif /* MULTITRD */

extern  VOID         ctidxfrsh(COUNT keyno);
extern  COUNT        chkidxhdr(NINT,pIIDX,COUNT,COUNT,COUNT);

#ifdef TRANPROC
#ifdef SYNC_LOG
extern COUNT ctsync(pCTFILE	ctnum,NINT mode);
#endif
#endif

#ifdef ctSQL

NINT FlushLRU( pVOID, pVOID );
VOID login_auth( pVOID, pUTEXT );
VOID SetSQLTransactionMode( pVOID, COUNT );

#endif  /*  ctSQL  */

#ifdef ctFRCSNG
#ifdef ctPOSTFNC
extern  COUNT        intfrmkey(COUNT keyno,pTEXT recptr,pTEXT txt,LONG pntr,VRLEN datlen);
extern  pTEXT        intTFRMKEY(COUNT keyno,pVOID tarptr);
#endif
#endif


#ifdef ctWATCH_THRD_START_STOP
NINT ctWatchThrdStartStop(NINT Mode, NINT sOWNR, NINT (*taskptr)(VOID));
#endif


#else /* PROTOTYPE */

extern  UINT         cthash_shift();
extern  NINT         ctfpglok();
extern  VRLEN        aGETDODA();
extern  COUNT        iGETNAM();
#ifdef ctThrdFPG
extern  pVOID        ctserl();
extern  NINT         ctunserl();
#endif
extern  UCOUNT       ctsetextsiz();
extern  COUNT        rensup();
extern 	pTEXT        chkifilnp();
extern  NINT         ctmenu();
extern  VOID         upbuf();
extern  NINT         press();
extern  NINT         chkwrd();
extern  VOID         getid();
extern  VOID         getwrd();
extern  VOID         getsrv();
extern  NINT         getstpsrv();
extern  NINT         getioh();
extern  VOID         errmsg();
extern  COUNT        ct_chkplen();
#ifndef ctCLIENT
extern  NINT         ctchkcidx();
extern  COUNT        iRDVREC();
#endif
extern  NINT         icttseg();
extern  NINT         ictuseg();
extern  NINT         ctdallc();
extern  NINT         ctfillff();
#ifdef DBG123
extern  NINT         dbg123();
#endif
#ifdef ctLOG_NEWNOD
extern  NINT         ctlog_newnod();
#endif
#ifdef ctLOG_FILEIO
extern  NINT         ctlog_fileio();
#endif
extern  NINT         ctcmpdlng();
#ifdef ctCLIENT
extern  pConvMap     ctlfschget();
extern  NINT         ctsetnlkey();
#endif
extern  VOID         freectlist();
extern  COUNT        iSETCURI();
#ifdef ctCONDIDX
extern  pCIFIL       ctigetcidx();
extern  NINT         ctevalcidx();
extern  VOID         ctfreecidx();
#endif
#ifdef ctFILE_ACCESS
extern  NINT         ctFileAccess();
#endif
extern  COUNT        ctdelupl();
extern  COUNT        ctinsert();
extern  VOID         ctclrcon();
extern  NINT         ctclrlockon();
extern  COUNT        iretrec();
#ifdef UNIFRMAT
extern  NINT         ctconvert1();
extern  VOID         rev_fhdr();
extern  VOID         rev_shdr();
extern  VOID         rev_rhdr();
extern  VOID         rev_thdr();
extern  VOID         rev_nhdr();
extern  VOID         rev_vhdr();
#endif
extern  VOID         ctsetkeypos();
extern  COUNT        rblsavres();
extern  COUNT        ctrvrng();
extern  NINT         ctuutbl();
#ifdef FPUTFGET
#ifdef LOCK_TEST
extern  COUNT        chkutbl();
#endif
#endif
extern  pTEXT        rbgetbuf();
static  VOID         setver();
#ifdef ctMTFPG_LOKCNF
extern  NINT         ctchkmlok();
extern  NINT         ctuhashx();
#else
extern  NINT         ctuhash();
#endif
#ifdef FASTCOMP
extern  COUNT        CTCOMP();
extern  UCOUNT       CTCOMPU();
#endif
static  pLSTHED      ctgethed();
static  VOID         ctputhed();
extern  VRLEN        igetifil();
#ifdef MULTITRD
extern  NINT         ct_tflmod();
extern  NINT         ctTCdone();
extern  NINT         ctendthread();
extern  NINT         ctmemmon();
extern  NINT         ctunblkq();
extern  NINT         ctblkbsy();
extern  pVOID        ctgvallc();
extern  NINT         ctUserAlive();
extern 	NINT 	     validusr();
extern  pTEXT 	     ctGetServerName();
#endif
#ifndef ctrt_filcmp
extern  NINT         ctrt_filcmp();
#endif
#ifdef ctSQL
extern ctCONV  LONG  ctDECL sqllock();
extern  COUNT        ADDRECs();
extern  COUNT        ADDVRECs();
extern NINT blksrlint();
#endif
static  pTREBUF      lodnod();
#ifndef nd_digPROTO
#define nd_digPROTO
extern  NINT         nd_dig();
extern  VOID         nd_digenc();
#endif
extern  pTEXT        uTFRMKEY();
static  pTEXT        iTFRMKEY();
extern  pTEXT        i2TFRMKEY();
extern  VOID         ctsysint();
extern  VOID         ctclnhdr();
extern  COUNT        chkvfin();
extern  VRLEN ctDECL ctcdelm();
#ifndef CTBOUND
extern  pMTEXT       recommbuf();
extern ctCONV  COUNT ctDECL COMMBUF();
extern  pCommFuncPtrs   ctCommLoad();
extern  VOID 			ctCommUnload();
#endif
extern  VOID         ctlnhdr();
extern  COUNT        ishtifil();
extern  pTREBUF      ctintnod();
extern  COUNT        ctgetsec();
extern  NINT         ctkw();
extern  pCOUNT       ctlfsegget();
extern  VOID         ctlfsegput();
extern  VOID	     ctmakid();
extern  pTEXT        ctrtnam();
extern  COUNT        ctspcopn();
extern  VOID         ctuftbl();
extern  COUNT        ctgetseginfo();
extern ctCONV  LONG  ctDECL ctdidx();
extern  VOID         setifil();
extern ctCONV  COUNT ctDECL iTMPNAME();
extern  COUNT        ctkeytrn();
extern  COUNT        ctultbl();
extern  ctRECPT      seqikey();
extern  COUNT        putdodas();
extern  COUNT        putdodan();
static  pTEXT        padrec();
static  pTEXT        padrec2();
static  COUNT        nxtbat();
extern  NINT	     inamidx();
#ifdef ctSRVR
extern  NINT         ctlaunch();
extern  NINT         ctsrvclndwn();
extern  COUNT        ctaddusr();
extern  COUNT        ctchgusr();
#endif
extern  VOID         mbfreel();
extern  VOID         mbfrenl();
static  COUNT        intred();
extern  COUNT        igetdefblk();
static  pTEXT        getrb();
static  COUNT        frsbat();
static  NINT         ifrebat();
static  COUNT        delbat();
extern  COUNT        ctsetseq();
extern  VOID         ctsetnmda();
extern  VOID         ctsetuginfo();
extern  COUNT        ctsetres();
extern  VOID         ctfusrclr();
extern  VOID         ctfilok();
static  NINT         chkbtrq();
static  NINT         chkbtlk();
static  COUNT        chkbtvf();
extern  VOID         ctiflnam();
static  VOID         ctfctlblk();
static  NINT         cntlist();
extern  pTEXT        makifil();
#ifndef CTBOUND
extern  COUNT        ctgetseg();
extern  COUNT        ctgetmap();
#else
extern  NINT         ctStopUser();
extern  VOID         ctStopServer();
#endif
extern ctCONV  COUNT ctDECL OPNIFILz();
extern ctCONV  COUNT ctDECL PUTIFILz();
extern ctCONV  COUNT ctDECL CREIFILz();
extern ctCONV  COUNT ctDECL PRMIIDXz();
extern ctCONV  COUNT ctDECL RBLIIDXz();
extern ctCONV  COUNT ctDECL TMPIIDXz();
extern ctCONV  COUNT ctDECL RBLIFILz();
extern ctCONV  COUNT ctDECL CMPIFILz();
extern         COUNT CLIFILz();
extern         COUNT DELIFILz();
extern  COUNT        iintree();
extern ctCONV  COUNT ctDECL TRANRDY();
extern  UINT         ctadjadr();
extern  NINT         ctadjfld();
extern  pVOID        ctrdef();
extern  COUNT        getrhdr();
extern  COUNT        putrhdr();
extern  COUNT        SETDEFBLK();
extern  COUNT        GETDEFBLK();
extern  pTEXT        cptifil();
extern  COUNT        ctrstdef();
#ifndef cpybig
extern  pTEXT        bigadr();
extern  VOID         cpybig();
#endif
extern  LONG         ctfsize();
static  NINT         avlspc();
extern  COUNT        ctrcvlog();
extern  pTEXT        ctdate();
#ifdef CTSUPER
extern  COUNT        cthstopn();
extern  COUNT        ctcresi();
extern  COUNT        cresmem();
extern  NINT         ctsname();
#endif
extern  LONG         ctdhupd();
extern  pLSTITM      ctgetlst();
extern  VOID         ctputlst();
#ifndef ctsfill
extern  VOID         ctsfill();
#endif
#ifndef ctbfill
extern  VOID         ctbfill();
#endif
extern  VOID         ctsetlst();
extern  pTEXT        ctgetmem();
extern  VOID         ctputmem();
extern  pTEXT        ctnt_getmem(); /* communications use - see ctasup.c*/
extern  VOID         ctnt_putmem();	/* communications use - see ctasup.c*/
extern  VOID         ctputmemn();
#ifndef CTBOUND
extern  VOID         ctbldlcl();
#endif
extern  VOID         ctcatend();
extern  NINT         ctcfill();
static  pTEXT        ctgstk();
extern  LONG         ctpstk();
extern  VOID         ctrstk();
#ifndef ctThrdFPG
extern  NINT _lds_   ctdefer();
#endif
extern 	LONG _lds_ 	ctGetSrVariable(); /* Get Server Variable */
extern 	LONG _lds_ 	ctNPctrt_printf();
extern  LONG         cttime();
#ifndef ctsysnap
extern  NINT	     ctsysnap();
#endif
extern  COUNT        iaddkey();
static  VOID         ctsplexc();
static  COUNT        insert();
extern  COUNT        adroot();
extern  pTREBUF      newnod();
extern  COUNT        ctwrtbuf();
static  VOID         ctputdsh();
static  pDATBUF      ctgetbuf();
extern  NINT         ctflushd();
static  VOID         clrbuf();
extern  NINT         ctblock();
static  pBHL         ctdshbuf();
extern  pTEXT        mballc();
#ifndef mblllc
extern  pTEXT        mblllc();
#endif
extern  VOID         mbfren();
extern  VOID         mbfree();
#ifdef MTMEMRY
extern  LONG         arep();
#endif
extern  COUNT        uerr();
extern  VOID         terr();
extern  VOID         revobj();
extern  VOID         revbyt();
extern  VOID         revwrd();
extern  VOID         ctchknum();
extern  VOID         ctinrnum();
extern  VOID         cpylod();
#ifndef cpylodl
extern  VOID         cpylodl();
#endif
extern  VOID         cpynbuf();
extern  VOID         cpysrc();
#ifndef cpysrcl
extern  VOID         cpysrcl();
#endif
extern  NINT         ctcomexc();
extern  VOID         ctlowhsh();
extern  COUNT        ctfixdel();
extern  COUNT        ctio();
extern  NINT         ctrelhdr();
extern  NINT         ctrelbuf();
extern  UCOUNT       diffimag();
extern  NINT         compar();
extern  ppTEXT       chkcopy();
extern  pTEXT        ctputk();
extern  COUNT        tstrec();
extern  COUNT        prthdr();
extern  COUNT        prtnod();
extern  COUNT        chkidx();
extern  COUNT        idelchk();
extern  NINT         ctdnode();
extern  VOID	     ctdnodestop();
extern  VOID         ctqnode();
extern  VOID         ctrmvexc();
static  COUNT        srklok();
extern  COUNT        ctdelkey();
extern  COUNT        rtnode();
static  COUNT        updprd();
static  COUNT        delupl();
static  COUNT        binsrc();
extern  COUNT        ckflmp();
extern  pFUSR        ctrvfl();
extern  COUNT        ctdwnfil();
extern  COUNT        chkopn();
extern  NINT         chkredf();
extern  COUNT        getctf();
extern  COUNT        getctm();
extern  VOID         retctf();
extern  COUNT        ctfree();
extern  NINT         ctstpsrv();
extern  NINT         ctrelusr();
extern  COUNT        iNINTree();
extern  LONG         extfil();
extern  pCTFILE      tstfnm();
extern  COUNT        vtclose();
extern  pFILE	     ctgetstream();
extern  VOID         inrfil();
extern  COUNT        tstupd();
extern  COUNT        redhdr();
extern  COUNT        filhdr();
extern  COUNT        wrthdr();
extern  COUNT        iopnfil();
extern  COUNT        wrtnod();
extern  COUNT        iclsfil();
static  COUNT        ismred();
extern  ctCONV COUNT ctDECL reset_cur();
extern  ctCONV COUNT ctDECL reset_cur2();
extern  COUNT        RTSCRIPT();
static  COUNT        reset_phy();
extern  LONG         chkism();
static  COUNT        seqrec();
static  COUNT        bndrec();
static  pTEXT        ctgetmp();
extern  VOID         setsrlpos();
extern  VOID         iundo();
extern  COUNT        addikey();
extern  COUNT        rwtikey();
extern  COUNT        delikey();
extern  VOID         ctfisam();
extern  NINT         ctiisam();
extern  COUNT        setudat();
extern  COUNT        setukey();
extern  pCTFILE      tstifnm();
extern  COUNT        ierr();
extern  COUNT        addlok();
extern  TEXT         ucase();
extern  COUNT        ctasskey();
static  COUNT        tstinm();
extern  COUNT        getNINTr();
extern  COUNT        getdatr();
static  COUNT        setmap();
extern  COUNT        getidxr();
extern  COUNT        getambr();
extern  VOID         ctsetlog();
extern  COUNT        icredat();
extern  NINT         ctidxcap();
extern  COUNT        icreidx();
extern  pCTFILE      icremem();
extern  UINT         ctelmexc();
extern  NINT         ctexcp();
#ifdef MULTITRD
extern  NINT         ctgetkbf();
#else
extern  NINT         ctgetkbf();
#endif
extern  NINT         nodser();
extern  VOID         ctputhsh();
extern  VOID         ctclrhsh();
extern  pTREBUF      ctgetnod();
extern  pTREBUF      lrubuf();
extern  VOID         ctnodcap();
static  COUNT        rednod();
#ifdef FPUTFGET
extern  LONG         gtroot();
#endif
extern  pTEXT        valpnt();
extern  pTEXT        hghpnt();
extern  LONG         nodpnt();
extern  LONG         drnpnt();
extern  pTEXT        expval();
static  pTREBUF      lodins();
extern  COUNT        nodctl();
extern  COUNT        ct_strip();
static  pTREBUF      skim();
extern  COUNT        iloadkey();
extern  VOID         newleaf();
extern  VOID         nonleaf();
extern  COUNT        rerr();
extern  COUNT        yesno();
extern  COUNT        vcparm();
extern  COUNT        vtparm();
extern  COUNT        prnprm();
extern  COUNT        RBLDATX();
extern  COUNT        RBLIDX();
extern  COUNT        RBLMEM();
static  COUNT        chkpar();
extern  VOID         ctupdkey();
static  COUNT        getdt();
extern  COUNT        scndat();
extern  VOID         pshlst();
static  COUNT        tstmode();
static  NINT         ctdlok();
extern  COUNT        ctaddblk();
extern  NINT         ctremblk();
extern  COUNT        cts_lok();
static  COUNT        dellst();
extern  COUNT        LOCK();
extern  NINT         iunlock();
extern  COUNT        UNLOCK();
extern  COUNT        DLOCK();
extern  COUNT        RLOCK();
extern  COUNT        UDLOCK();
extern  LONG         ieqlkey();
static  LONG         ser();
extern  LONG         igtekey();
extern  LONG         ilstkey();
extern  LONG         nxtikey();
extern  LONG         prvikey();
extern  LONG         igtkey();
extern  NINT         ctscnexc();
extern  NINT         ctrevexc();
extern  LONG         fndkey();
extern  COUNT        chkset();
extern  COUNT        setset();
extern  COUNT        igetaltseq();
extern  COUNT        hdrupd();
extern  COUNT        ctmrkexc();
extern  VOID         ctrstexc();
extern  LONG         ctabtexc();
extern  COUNT        ctabtnod();
extern  COUNT        ctclup();
static  COUNT        ctanyexc();
extern  COUNT        putnod();
extern  VOID         prpdup();
extern  pTREBUF      movrgt();
extern  VOID         shfrgt();
extern  COUNT        ideladd();
extern  VOID         insexp();
extern  COUNT        delexp();
extern  LONG         inewvrec();
extern  COUNT        iretvrec();
extern  COUNT        getvhdr();
extern  COUNT        putvhdr();
extern  COUNT        frmlkey();
extern  COUNT        chkvhdr();
extern  VRLEN        prprdv();
extern  LONG         chkvsm();
extern  COUNT        ctVERIFY();
extern  NINT         link_verify();
extern  NINT         leaf_verify();
extern  NINT         leaf_extra();
extern  COUNT        ctseek();
extern  NINT         ctswitch();
extern  COUNT        ictio();
extern  COUNT        ctsysio();
extern  COUNT        dltfil();
extern  COUNT        cnvdltfil();
extern  COUNT        renfil();
extern  NINT         cttcre();
extern  NINT         ctqcre();
extern  NINT         ctqwrt();
extern  NINT         ctqred();
extern NINT ctqchk();
extern  NINT         ctmqwrt();
extern  VOID         cttsksetup();
extern  NINT _lds_   ctsemclr();
extern  NINT         ctsemwat();
extern  NINT _lds_   ctsemrqs();
#ifdef ctCREATE_SEMA
extern  NINT _lds_   ctsemcls();
#endif
#ifdef ctSEMBLK
extern  NINT _lds_   ctblkclr();
extern  NINT         ctblkwat();
extern  NINT _lds_   ctblkrqs();
#endif
#ifdef ctSEMTIM
extern  NINT _lds_   cttimclr();
extern  NINT         cttimwat();
extern  NINT _lds_   cttimrqs();
#endif
#ifdef ctSEMUTX
extern  NINT         ctmutclr();
extern  NINT         ctmutrqs();
#endif
extern  NINT         ctdbw();
extern  NINT         ctdbr();
extern  NINT         ctlclose();
extern  COUNT        mbclos();
#ifdef ctMIRROR
extern  NINT         ctswtmir();
extern  COUNT        ctmirhdr();
extern  NINT         ctmname();
extern  NINT         ctmalert();
extern  COUNT        ctmopen();
extern  RNDFILE      ctopen();
extern  COUNT        ctmcreate();
extern  VOID         ctfrcmir();
#endif
extern  RNDFILE      mbopen();
extern  RNDFILE      mbcrat();
extern  COUNT        mbsave();
#ifndef ctsave
extern  COUNT        ctsave();
#endif
extern  VOID         flushdos();
extern  COUNT        filrng();
extern  COUNT        setimap();
#ifdef BIGCHECK
extern  NINT         CTCFILL();
#endif

#ifdef MULTITRD
extern  COUNT        INTREEz();
extern  COUNT        INTISAMz();
extern  COUNT        CREISAMz();
extern  COUNT        OPNISAMz();

extern  NINT         ctcnvfn();
static  NINT         dupopen();
extern  VOID         cttsus();
extern  NINT         ctloglog();
extern  NINT         ctclrsrvr();
extern  NINT         ctqcls();
extern  VOID         stopctsrvr();
extern  NINT         PIFlush();
extern  VOID         ctsetnmda();
extern  COUNT        ctugfil();
extern  COUNT        ctsetsec();
extern  COUNT        ctchksec();
extern  VOID         ctsetuginfo();
extern  NINT         ctismem();
extern  NINT         ctmemavl();
extern  NINT         blkcmtint();
extern  NINT         ctntio();
extern  VOID         setcommbuf();
extern  VOID         ctdec();
extern  NINT         ctblksrvr();
extern  COUNT        srvrwait();
extern  VOID         initctsrvr();
extern  VOID         srvrcopyright();
extern  VOID         srvrflashmsg();
extern  VOID         ctenc();
extern  VOID         startctsrvr();
extern  NINT         ctclrclnt();
extern NINT ctkilblk();

#ifdef ___FNPglobals___
extern  Thread_     *makeThread();
extern  byte        *memory();
extern  VOID         delayDefer();
extern  VOID        *threadInit();
extern  VOID         killThread();
extern  int32        getThdTime();
extern  Thread_     *myThread();
extern  VOID         sleepThread();
extern  VOID         DEFER();
#endif /* ___FNPglobals___ */
#endif /* MULTITRD */

extern  VOID         ctidxfrsh();
extern  COUNT        chkidxhdr();

#ifdef TRANPROC
#ifdef SYNC_LOG
extern COUNT ctsync();
#endif
#endif

#ifdef ctSQL

NINT FlushLRU();
VOID login_auth();
VOID SetSQLTransactionMode();

#endif  /*  ctSQL  */

#ifdef ctFRCSNG
#ifdef ctPOSTFNC
extern  COUNT        intfrmkey();
extern  pTEXT        intTFRMKEY();
#endif
#endif

#ifdef ctWATCH_THRD_START_STOP
NINT ctWatchThrdStartStop();
#endif

#endif /* pPROTOTYPE */

#endif /* ~ctFUNPH */

/* end of ctfunp.h */
