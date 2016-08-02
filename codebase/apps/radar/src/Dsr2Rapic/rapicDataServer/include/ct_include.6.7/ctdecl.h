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

#ifndef ctDECLH
#define ctDECLH

#include "ctifil.h"

#ifdef PROTOTYPE

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ctThrds
#include "ctatrd.h"
extern ctCONV  NINT  ctDECL ctThrdInit(NINT ,LONG ,pctINIT );
extern ctCONV  NINT  ctDECL ctThrdTerm(VOID );
extern ctCONV  NINT  ctDECL ctThrdCreate(pctFUNC ,pVOID ,pVOID ,LONG );
extern ctCONV  VOID  ctDECL ctThrdExit(VOID );
extern ctCONV  NINT  ctDECL ctThrdSleep(LONG );
extern ctCONV  pVOID ctDECL ctThrdData(VOID );
extern ctCONV  NINT  ctDECL ctThrdDataSet(pVOID );
extern ctCONV  NINT  ctDECL ctThrdMutexGet(pctMUTEX );
extern ctCONV  NINT  ctDECL ctThrdMutexTry(pctMUTEX );
extern ctCONV  NINT  ctDECL ctThrdMutexRel(pctMUTEX );
extern ctCONV  NINT  ctDECL ctThrdMutexCls(pctMUTEX );
extern ctCONV  NINT  ctDECL ctThrdBlockGet(pctBLOCK ,LONG );
extern ctCONV  NINT  ctDECL ctThrdBlockWait(pctBLOCK ,LONG );
extern ctCONV  NINT  ctDECL ctThrdBlockRel(pctBLOCK );
extern ctCONV  NINT  ctDECL ctThrdBlockCls(pctBLOCK );
extern ctCONV  NINT  ctDECL ctThrdQueueOpen(VOID );
extern ctCONV  NINT  ctDECL ctThrdQueueClose(NINT );
extern ctCONV  NINT  ctDECL ctThrdQueueWrite(NINT ,pVOID ,NINT );
extern ctCONV  NINT  ctDECL ctThrdLIFOWrite(NINT ,pVOID ,NINT );
extern ctCONV  NINT  ctDECL ctThrdQueueRead(NINT ,pVOID ,NINT ,LONG );
extern ctCONV  NINT  ctDECL ctThrdQueueMlen(NINT ,LONG );
extern ctCONV  LONG  ctDECL ctThrdQueueCount(NINT );
extern ctCONV  NINT  ctDECL ctThrdSemapInit(pctSEMAP ,NINT );
extern ctCONV  NINT  ctDECL ctThrdSemapGet(pctSEMAP );
extern ctCONV  NINT  ctDECL ctThrdSemapTry(pctSEMAP );
extern ctCONV  NINT  ctDECL ctThrdSemapRel(pctSEMAP );
extern ctCONV  NINT  ctDECL ctThrdSemapCls(pctSEMAP );
extern ctCONV  NINT  ctDECL ctThrdHandle(VOID );
#endif
extern ctCONV  COUNT ctDECL ADDKEY(COUNT keyno,pVOID target,LONG recbyt,COUNT typadd);
extern ctCONV  COUNT ctDECL ADDREC(COUNT datno,pVOID recptr);
extern ctCONV  COUNT ctDECL ADDRES(COUNT datno,pVOID resptr,VRLEN varlen);
extern ctCONV  COUNT ctDECL ADDVREC(COUNT datno,pVOID recptr,VRLEN varlen);
extern ctCONV  COUNT ctDECL ALCBAT(COUNT numbat);
extern ctCONV  COUNT ctDECL ALCSET(COUNT numset);
extern ctCONV  COUNT ctDECL AVLFILNUM(COUNT numfils);
extern ctCONV  COUNT ctDECL BATSET(COUNT filno,pVOID request,pVOID bufptr,VRLEN bufsiz,UCOUNT mode);
extern ctCONV  COUNT ctDECL CHGBAT(COUNT batnum);
extern ctCONV  COUNT ctDECL CHGHST(COUNT hstnum);
extern ctCONV  COUNT ctDECL CHGICON(COUNT contextid);
extern ctCONV  COUNT ctDECL CHGSET(COUNT setnum);
extern ctCONV  COUNT ctDECL CLIFIL(struct ifil ctMEM *ifilptr);
extern ctCONV  COUNT ctDECL CLNIDXX(pTEXT filnam,pTEXT fileword);
extern ctCONV  COUNT ctDECL CLISAM(VOID );
extern ctCONV  COUNT ctDECL CLRFIL(COUNT datno);
extern ctCONV  COUNT ctDECL CLSFIL(COUNT filno,COUNT filmod);
extern ctCONV  COUNT ctDECL CLSICON(COUNT contextid);
extern ctCONV  COUNT ctDECL CMPIFIL(struct ifil ctMEM *ifilptr);
extern ctCONV  COUNT ctDECL CMPIFILX(struct ifil ctMEM *ifilptr,pTEXT dataextn,pTEXT indxextn,LONG permmask,pTEXT groupid,pTEXT fileword);
extern ctCONV  COUNT ctDECL CREDAT(COUNT datno,pTEXT filnam,UCOUNT datlen,UCOUNT xtdsiz,COUNT filmod);
extern ctCONV  COUNT ctDECL CREDATX(COUNT datno,pTEXT filnam,UCOUNT datlen,UCOUNT xtdsiz,COUNT filmod,LONG permmask,pTEXT groupid,pTEXT fileword);
extern ctCONV  COUNT ctDECL CREIDX(COUNT keyno,pTEXT filnam,COUNT keylen,COUNT keytyp,COUNT keydup,COUNT nomemb,UCOUNT xtdsiz,COUNT filmod);
extern ctCONV  COUNT ctDECL CREIDXX(COUNT keyno,pTEXT filnam,COUNT keylen,COUNT keytyp,COUNT keydup,COUNT nomemb,UCOUNT xtdsiz,COUNT filmod,LONG permmask,pTEXT groupid,pTEXT fileword);
extern ctCONV  COUNT ctDECL CREIFIL(struct ifil ctMEM *ifilptr);
extern ctCONV  COUNT ctDECL CREIFILX(struct ifil ctMEM *ifilptr,pTEXT dataextn,pTEXT indxextn,LONG permmask,pTEXT groupid,pTEXT fileword);
extern ctCONV  COUNT ctDECL CREISAM(pTEXT filnam);
extern ctCONV  COUNT ctDECL CREISAMX(pTEXT filnam,COUNT userprof,pTEXT userid,pTEXT userword,pTEXT servname,LONG permmask,pTEXT groupid,pTEXT fileword);
extern ctCONV  COUNT ctDECL CREMEM(COUNT keyno,COUNT keylen,COUNT keytyp,COUNT keydup,COUNT membno);
extern ctCONV  COUNT ctDECL CTCHKPNT(VOID );
extern ctCONV  COUNT ctDECL CTFLUSH(COUNT filno);
extern ctCONV  COUNT ctDECL CTHIST(COUNT filno,pVOID target,pVOID bufptr,LONG recbyt,VRLEN bufsiz,UCOUNT mode);
extern ctCONV  UINT  ctDECL ctotAlign(UINT align);
extern ctCONV  COUNT ctDECL CTSBLDX(pTEXT filnam,pTEXT fileword);
extern ctCONV  COUNT ctDECL CTSQL(pVOID ,pVOID ,VRLEN, COUNT ,LONG );
#ifdef ctThrdApp
extern ctCONV  pVOID ctDECL ctThrdGet(VOID);
extern ctCONV  pVOID ctDECL ctThrdClr(VOID);
extern ctCONV  VOID  ctDECL ctThrdPut(VOID);
extern ctCONV  NINT  ctDECL ctThrdSet(VOID);
extern ctCONV  NINT  ctDECL ctThrdNum(VOID);
#endif
extern ctCONV  COUNT ctDECL cttestfunc(VOID);
extern ctCONV  COUNT ctDECL cttseg(COUNT segpos,COUNT mod,COUNT slen,pTEXT tarptr,pCOUNT aq,pConvMap mp);
extern ctCONV  COUNT ctDECL ctuseg(COUNT spos,COUNT mod,COUNT slen,pTEXT tarptr,pCOUNT aq,pConvMap mp);
#ifdef ctPREV_66A3_CTUSER
extern ctCONV  LONG  ctDECL CTUSER(pTEXT command);
#else
extern ctCONV  LONG  ctDECL CTUSER(pTEXT command,pTEXT bufptr,VRLEN bufsiz);
#endif
extern ctCONV  LONG  ctDECL DATENT(COUNT datno);
extern ctCONV  LONG  ctDECL DELBLD(COUNT keyno,pVOID target);
extern ctCONV  COUNT ctDECL DELCHK(COUNT keyno,pVOID target,LONG recbyt);
extern ctCONV  COUNT ctDECL DELIFIL(struct ifil ctMEM *ifilptr);
extern ctCONV  COUNT ctDECL DELFIL(COUNT filno);
extern ctCONV  COUNT ctDECL DELREC(COUNT datno);
extern ctCONV  COUNT ctDECL DELRES(COUNT datno,pVOID resptr);
extern ctCONV  COUNT ctDECL DELRFIL(COUNT datno);
extern ctCONV  COUNT ctDECL DELVREC(COUNT datno);
extern ctCONV  COUNT ctDECL ENARES(COUNT datno);
extern ctCONV  LONG  ctDECL EQLKEY(COUNT keyno,pVOID target);
extern ctCONV  COUNT ctDECL EQLREC(COUNT keyno,pVOID target,pVOID recptr);
extern ctCONV  LONG  ctDECL ESTKEY(COUNT keyno,pVOID idxval1,pVOID idxval2);
extern ctCONV  COUNT ctDECL EXPIFIL(struct ifil ctMEM *ifilptr);
extern ctCONV  COUNT ctDECL EXPIFILX(struct ifil ctMEM *ifilptr,pTEXT dataextn,pTEXT indxextn,LONG permmask,pTEXT groupid,pTEXT fileword);
extern ctCONV  LONG  ctDECL FRCKEY(COUNT keyno,pVOID idxval,COUNT fractal);
extern ctCONV  COUNT ctDECL FREBAT(VOID );
extern ctCONV  COUNT ctDECL FREHST(VOID );
extern ctCONV  COUNT ctDECL FRESET(VOID );
extern ctCONV  COUNT ctDECL frmkey(COUNT keyno,pTEXT recptr,pTEXT txt,LONG pntr,VRLEN datlen);
extern ctCONV  LONG  ctDECL FRSKEY(COUNT keyno,pVOID idxval);
extern ctCONV  COUNT ctDECL FRSREC(COUNT filno,pVOID recptr);
extern ctCONV  COUNT ctDECL FRSSET(COUNT keyno,pVOID target,pVOID recptr,COUNT siglen);
extern ctCONV  COUNT ctDECL GETALTSEQ(COUNT keyno,pCOUNT altseq);
#ifdef ctCONDIDX
extern ctCONV  VRLEN ctDECL GETCRES(COUNT datno,LONG bufsiz,pVOID idxcnd);
extern ctCONV  VRLEN ctDECL GETCIDX(COUNT keyno,LONG bufsiz,pVOID idxcnd);
#endif
extern ctCONV  pVOID ctDECL GETCTREE(pTEXT regid);
extern ctCONV  pTEXT ctDECL GETCURK(COUNT keyno,pVOID idxval,pVRLEN plen);
extern ctCONV  pTEXT ctDECL GETCURKL(COUNT keyno,pVOID idxval);
extern ctCONV  LONG  ctDECL GETCURP(COUNT datno);
extern ctCONV  VRLEN ctDECL GETDODA(COUNT datno,LONG buflen,pVOID bufptr,COUNT mode);
extern ctCONV  LONG  ctDECL GETFIL(COUNT filno,COUNT mode);
extern ctCONV  VRLEN ctDECL GETIFIL(COUNT datno,LONG buflen,pVOID bufptr);
extern ctCONV  COUNT ctDECL GETMNAME(COUNT superFileNo, pTEXT nambuf, VRLEN buflen,COUNT memberFileNo);
extern ctCONV  COUNT ctDECL GETNAM(COUNT filno,pTEXT nambuf,VRLEN buflen,COUNT mode);
extern ctCONV  LONG  ctDECL GETRES(COUNT datno,pVOID resptr,pVOID bufptr,VRLEN bufsiz,COUNT resmode);
extern ctCONV  VRLEN ctDECL GETVLEN(COUNT datno);
extern ctCONV  LONG  ctDECL GTEKEY(COUNT keyno,pVOID target,pVOID idxval);
extern ctCONV  COUNT ctDECL GTEREC(COUNT keyno,pVOID target,pVOID recptr);
extern ctCONV  LONG  ctDECL GTKEY(COUNT keyno,pVOID target,pVOID idxval);
extern ctCONV  COUNT ctDECL GTREC(COUNT keyno,pVOID target,pVOID recptr);
extern ctCONV  VRLEN ctDECL GTVLEN(COUNT datno,LONG recbyt);
extern ctCONV  LONG  ctDECL IDXENT(COUNT keyno);
extern ctCONV  COUNT ctDECL INTISAM(COUNT bufs,COUNT fils,COUNT sect);
extern ctCONV  COUNT ctDECL INTISAMX(COUNT bufs,COUNT fils,COUNT sect,COUNT dbufs,COUNT userprof,pTEXT userid,pTEXT userword,pTEXT servname);
extern ctCONV  COUNT ctDECL INTREE(COUNT bufs,COUNT fils,COUNT sect);
extern ctCONV  COUNT ctDECL INTREEX(COUNT bufs,COUNT fils,COUNT sect,COUNT dbufs,COUNT userprof,pTEXT userid,pTEXT userword,pTEXT servname);
extern ctCONV  COUNT ctDECL IOPERFORMANCE(pVOID bufptr);
extern ctCONV  COUNT ctDECL IOPERFORMANCEX(pVOID bufptr);
extern ctCONV  COUNT ctDECL LKISAM(COUNT lokmod);
extern ctCONV  COUNT ctDECL LOADKEY(COUNT keyno,pVOID target,LONG recbyt,COUNT typadd);
extern ctCONV  COUNT ctDECL LOKREC(COUNT datno,COUNT lokmod,LONG recbyt);
extern ctCONV  LONG  ctDECL LSTKEY(COUNT keyno,pVOID idxval);
extern ctCONV  COUNT ctDECL LSTREC(COUNT filno,pVOID recptr);
extern ctCONV  COUNT ctDECL LSTSET(COUNT keyno,pVOID target,pVOID recptr,COUNT siglen);
extern ctCONV  LONG  ctDECL LTEKEY(COUNT keyno,pVOID target,pVOID idxval);
extern ctCONV  COUNT ctDECL LTEREC(COUNT keyno,pVOID target,pVOID recptr);
extern ctCONV  LONG  ctDECL LTKEY(COUNT keyno,pVOID target,pVOID idxval);
extern ctCONV  COUNT ctDECL LTREC(COUNT keyno,pVOID target,pVOID recptr);
extern ctCONV  COUNT ctDECL MIDSET(COUNT keyno,pVOID recptr,COUNT siglen);
extern ctCONV  LONG  ctDECL NEWREC(COUNT datno);
extern ctCONV  LONG  ctDECL NEWVREC(COUNT datno,VRLEN varlen);
extern ctCONV  pTEXT ctDECL NXTCTREE(VOID );
extern ctCONV  LONG  ctDECL NXTKEY(COUNT keyno,pVOID idxval);
extern ctCONV  COUNT ctDECL NXTREC(COUNT filno,pVOID recptr);
extern ctCONV  COUNT ctDECL NXTSET(COUNT keyno,pVOID recptr);
extern ctCONV  COUNT ctDECL OPNFIL(COUNT ufilno,pTEXT filnam,COUNT filmod);
extern ctCONV  COUNT ctDECL OPNFILX(COUNT filno,pTEXT filnam,COUNT filmod,pTEXT fileword);
extern ctCONV  COUNT ctDECL OPNICON(COUNT datno,COUNT keyno,COUNT contextid);
extern ctCONV  COUNT ctDECL OPNIFIL(struct ifil ctMEM *ifilptr);
extern ctCONV  COUNT ctDECL OPNIFILX(struct ifil ctMEM *ifilptr,pTEXT dataextn,pTEXT indxextn,pTEXT fileword);
extern ctCONV  COUNT ctDECL OPNISAM(pTEXT filnam);
extern ctCONV  COUNT ctDECL OPNISAMX(pTEXT filnam,COUNT userprof,pTEXT userid,pTEXT userword,pTEXT servname,pTEXT fileword);
extern ctCONV  COUNT ctDECL OPNRFIL(COUNT filno,pTEXT filnam,COUNT filmod);
extern ctCONV  COUNT ctDECL OPNRFILX(COUNT filno,pTEXT filnam,COUNT filmod,pTEXT fileword);
extern ctCONV  LONG  ctDECL ORDKEY(COUNT keyno,pVOID target,VRLEN offset,pVOID idxval);
extern ctCONV  COUNT ctDECL PRMIIDX(struct ifil ctMEM *ifilptr);
extern ctCONV  LONG  ctDECL PRVKEY(COUNT keyno,pVOID idxval);
extern ctCONV  COUNT ctDECL PRVREC(COUNT filno,pVOID recptr);
extern ctCONV  COUNT ctDECL PRVSET(COUNT keyno,pVOID recptr);
#ifdef ctCONDIDX
extern ctCONV  COUNT ctDECL PUTCRES(COUNT datno,pVOID idxcnd,VRLEN varlen);
#endif
extern ctCONV  COUNT ctDECL PUTDODA(COUNT datno,pDATOBJ doda,UCOUNT numfld);
extern ctCONV  COUNT ctDECL PUTFIL(COUNT filno,COUNT filmod);
extern ctCONV  COUNT ctDECL PUTHDR(COUNT filno,LONG hdrval,COUNT mode);
extern ctCONV  COUNT ctDECL PUTIFIL(struct ifil ctMEM *ifilptr);
extern ctCONV  COUNT ctDECL PUTIFILX(struct ifil ctMEM *ifilptr,pTEXT dataextn,pTEXT indxextn,pTEXT fileword);
extern ctCONV  COUNT ctDECL TMPIIDXX(struct ifil ctMEM *ifilptr,LONG permmask,pTEXT groupid,pTEXT fileword);
extern ctCONV  COUNT ctDECL TRANABT(VOID );
extern ctCONV  COUNT ctDECL TRANABTX(COUNT lokmod);
extern ctCONV  COUNT ctDECL TRANCLR(VOID );
extern ctCONV  LONG  ctDECL TSTVREC(COUNT datno,VRLEN varlen);
extern ctCONV  COUNT ctDECL RBLIIDX(struct ifil ctMEM *ifilptr);
extern ctCONV  COUNT ctDECL RBLIFIL(struct ifil ctMEM *ifilptr);
extern ctCONV  COUNT ctDECL RBLIFILX(struct ifil ctMEM *ifilptr,pTEXT dataextn,pTEXT indxextn,LONG permmask,pTEXT groupid,pTEXT fileword);
extern ctCONV  COUNT ctDECL RDVREC(COUNT datno,LONG recbyt,pVOID recptr,VRLEN bufsiz);
extern ctCONV  COUNT ctDECL REDIREC(COUNT datno,LONG recbyt,pVOID recptr);
extern ctCONV  COUNT ctDECL REDREC(COUNT datno,LONG recbyt,pVOID recptr);
extern ctCONV  COUNT ctDECL REDVREC(COUNT datno,pVOID recptr,VRLEN bufsiz);
extern ctCONV  COUNT ctDECL REGCTREE(pTEXT regid);
extern ctCONV  COUNT ctDECL ctRENFIL(COUNT filno,pTEXT newname);
extern ctCONV  COUNT ctDECL RETREC(COUNT datno,LONG recbyt);
extern ctCONV  COUNT ctDECL RETVREC(COUNT datno,LONG recbyt);
extern ctCONV  LONG  ctDECL RNGENT(COUNT keyno, pVOID key1, pVOID key2);
extern ctCONV  COUNT ctDECL RRDREC(COUNT datno,pVOID recptr);
extern ctCONV  COUNT ctDECL RWTREC(COUNT datno,pVOID recptr);
extern ctCONV  COUNT ctDECL RWTVREC(COUNT datno,pVOID recptr,VRLEN varlen);
extern ctCONV  COUNT ctDECL SECURITY(COUNT filno,pVOID bufptr,VRLEN bufsiz,COUNT mode);
extern ctCONV  LONG  ctDECL SERIALNUM(COUNT datno);
extern ctCONV  COUNT ctDECL SETALTSEQ(COUNT keyno,pCOUNT altseq);
extern ctCONV  COUNT ctDECL SETCURI(COUNT datno,LONG recbyt,pVOID recptr,VRLEN datlen);
extern ctCONV  COUNT ctDECL SETDEFBLK(COUNT datno,pDEFDEF defp);
extern ctCONV  COUNT ctDECL SETFLTR(COUNT filno,pTEXT expression);
extern ctCONV  NINT  ctDECL SETLOGPATH(pTEXT path,NINT mode);
extern ctCONV  COUNT ctDECL SETNODE(pTEXT nodename);
extern ctCONV  LONG  ctDECL SETOPS(LONG ops,VRLEN dat);
extern ctCONV  COUNT ctDECL SETVARBYTS(COUNT filno,pUTEXT pvbyte);
#ifndef CTBOUND
extern ctCONV  COUNT ctDECL SETSFLV(COUNT mode, UTEXT value);
extern ctCONV  COUNT ctDECL SETWNAM(pTEXT pName);
#endif
extern ctCONV  COUNT ctDECL SPCLSAV(VOID );
extern ctCONV  COUNT ctDECL SQR(pVOID bufptr,VRLEN bufsiz,COUNT funcId,LONG cursorNumber);
extern ctCONV  COUNT ctDECL STPSRV(pTEXT userword,pTEXT servname,COUNT delay);
extern ctCONV  COUNT ctDECL STPUSR(VOID );
extern ctCONV  COUNT ctDECL SWTCTREE(pTEXT regid);
extern ctCONV  COUNT ctDECL SYSCFG(pVOID bufptr);
extern ctCONV  COUNT ctDECL SYSMON(COUNT mode,LONG timeout,pTEXT buffer,VRLEN buflen);
extern ctCONV  pTEXT ctDECL TFRMKEY(COUNT keyno,pVOID tarptr);
extern ctCONV  COUNT ctDECL TMPNAME(pTEXT bufptr,VRLEN bufsiz);
extern ctCONV  LONG  ctDECL TRANBEG(COUNT mode);
extern ctCONV  COUNT ctDECL TRANEND(COUNT mode);
extern ctCONV  COUNT ctDECL TRANRST(COUNT savpnt);
extern ctCONV  COUNT ctDECL TRANSAV(VOID );
extern ctCONV  COUNT ctDECL UNRCTREE(pTEXT regid);
extern ctCONV  COUNT ctDECL UPDCIDX(COUNT keyno,pTEXT condexpr);
extern ctCONV  COUNT ctDECL UPDCURI(COUNT datno,COUNT mode);
extern ctCONV  COUNT ctDECL UPDRES(COUNT datno,pVOID resptr,VRLEN varlen);
extern ctCONV  pTEXT ctDECL WCHCTREE(VOID );
extern ctCONV  COUNT ctDECL WRTREC(COUNT datno,LONG recbyt,pVOID recptr);
extern ctCONV  COUNT ctDECL WRTVREC(COUNT datno,LONG recbyt,pVOID recptr,VRLEN varlen);
extern ctCONV  COUNT ctDECL setfndval(pVOID bufptr,VRLEN bufsiz);
#ifdef ctreeVARLST
extern ctCONV  COUNT ctVDECL ctree();
#else
extern ctCONV COUNT ctVDECL ctree(COUNT fn,COUNT filno,pVOID ptr1,pLONG plong,pVOID ptr2,pVRLEN psize,COUNT mode);
#endif

extern ctCONV  COUNT ctDECL EQLVREC(COUNT keyno,pVOID target,pVOID recptr,pVRLEN plen);
extern ctCONV  COUNT ctDECL FRSVREC(COUNT filno,pVOID recptr,pVRLEN plen);
extern ctCONV  COUNT ctDECL GTEVREC(COUNT keyno,pVOID target,pVOID recptr,pVRLEN plen);
extern ctCONV  COUNT ctDECL GTVREC(COUNT keyno,pVOID target,pVOID recptr,pVRLEN plen);
extern ctCONV  COUNT ctDECL LSTVREC(COUNT filno,pVOID recptr,pVRLEN plen);
extern ctCONV  COUNT ctDECL LTEVREC(COUNT keyno,pVOID target,pVOID recptr,pVRLEN plen);
extern ctCONV  COUNT ctDECL LTVREC(COUNT keyno,pVOID target,pVOID recptr,pVRLEN plen);
extern ctCONV  COUNT ctDECL NXTVREC(COUNT filno,pVOID recptr,pVRLEN plen);
extern ctCONV  COUNT ctDECL PRVVREC(COUNT filno,pVOID recptr,pVRLEN plen);

extern ctCONV  COUNT ctDECL FRSVSET(COUNT keyno,pVOID target,pVOID recptr,COUNT siglen,pVRLEN plen);
extern ctCONV  COUNT ctDECL LSTVSET(COUNT keyno,pVOID target,pVOID recptr,COUNT siglen,pVRLEN plen);
extern ctCONV  COUNT ctDECL MIDVSET(COUNT keyno,pVOID recptr,COUNT siglen,pVRLEN plen);
extern ctCONV  COUNT ctDECL NXTVSET(COUNT keyno,pVOID recptr,pVRLEN plen);
extern ctCONV  COUNT ctDECL PRVVSET(COUNT keyno,pVOID recptr,pVRLEN plen);
extern ctCONV  COUNT ctDECL REDIVREC(COUNT datno,LONG recbyt,pVOID recptr,pVRLEN plen);

#ifdef __cplusplus
}
#endif

#else  /* PROTOTYPE */

#ifdef ctThrds
#include "ctatrd.h"
extern ctCONV  NINT  ctDECL ctThrdInit();
extern ctCONV  NINT  ctDECL ctThrdTerm();
extern ctCONV  NINT  ctDECL ctThrdCreate();
extern ctCONV  VOID  ctDECL ctThrdExit();
extern ctCONV  NINT  ctDECL ctThrdSleep();
extern ctCONV  pVOID ctDECL ctThrdData();
extern ctCONV  NINT  ctDECL ctThrdDataSet();
extern ctCONV  NINT  ctDECL ctThrdMutexGet();
extern ctCONV  NINT  ctDECL ctThrdMutexTry();
extern ctCONV  NINT  ctDECL ctThrdMutexRel();
extern ctCONV  NINT  ctDECL ctThrdMutexCls();
extern ctCONV  NINT  ctDECL ctThrdSemaGet();
extern ctCONV  NINT  ctDECL ctThrdSemaWait();
extern ctCONV  NINT  ctDECL ctThrdSemaRel();
extern ctCONV  NINT  ctDECL ctThrdSemaCls();
extern ctCONV  NINT  ctDECL ctThrdQueueOpen();
extern ctCONV  NINT  ctDECL ctThrdQueueClose();
extern ctCONV  NINT  ctDECL ctThrdQueueWrite();
extern ctCONV  NINT  ctDECL ctThrdLIFOWrite();
extern ctCONV  NINT  ctDECL ctThrdQueueRead();
extern ctCONV  NINT  ctDECL ctThrdQueueMlen();
extern ctCONV  LONG  ctDECL ctThrdQueueCount();
extern ctCONV  NINT  ctDECL ctThrdSemapInit();
extern ctCONV  NINT  ctDECL ctThrdSemapGet();
extern ctCONV  NINT  ctDECL ctThrdSemapTry();
extern ctCONV  NINT  ctDECL ctThrdSemapRel();
extern ctCONV  NINT  ctDECL ctThrdSemapCls();
extern ctCONV  NINT  ctDECL ctThrdHandle();
#endif /* ctThrds */
extern ctCONV  COUNT ctDECL ADDKEY();
extern ctCONV  COUNT ctDECL ADDREC();
extern ctCONV  COUNT ctDECL ADDRES();
extern ctCONV  COUNT ctDECL ADDVREC();
extern ctCONV  COUNT ctDECL ALCBAT();
extern ctCONV  COUNT ctDECL ALCSET();
extern ctCONV  COUNT ctDECL AVLFILNUM();
extern ctCONV  COUNT ctDECL BATSET();
extern ctCONV  COUNT ctDECL CHGBAT();
extern ctCONV  COUNT ctDECL CHGHST();
extern ctCONV  COUNT ctDECL CHGICON();
extern ctCONV  COUNT ctDECL CHGSET();
extern ctCONV  COUNT ctDECL CLNIDXX();
extern ctCONV  COUNT ctDECL CLIFIL();
extern ctCONV  COUNT ctDECL DELIFIL();
extern ctCONV  COUNT ctDECL CLISAM();
extern ctCONV  COUNT ctDECL CLRFIL();
extern ctCONV  COUNT ctDECL DELRFIL();
extern ctCONV  COUNT ctDECL CLSFIL();
extern ctCONV  COUNT ctDECL CLSICON();
extern ctCONV  COUNT ctDECL CMPIFIL();
extern ctCONV  COUNT ctDECL CMPIFILX();
extern ctCONV  COUNT ctDECL CREDAT();
extern ctCONV  COUNT ctDECL CREDATX();
extern ctCONV  COUNT ctDECL CREIDX();
extern ctCONV  COUNT ctDECL CREIDXX();
extern ctCONV  COUNT ctDECL CREIFIL();
extern ctCONV  COUNT ctDECL PRMIIDX();
extern ctCONV  COUNT ctDECL CREIFILX();
extern ctCONV  COUNT ctDECL TMPIIDXX();
extern ctCONV  COUNT ctDECL CREISAM();
extern ctCONV  COUNT ctDECL CREISAMX();
extern ctCONV  COUNT ctDECL CREMEM();
extern ctCONV  COUNT ctDECL CTCHKPNT();
extern ctCONV  COUNT ctDECL CTFLUSH();
extern ctCONV  COUNT ctDECL CTHIST();
extern ctCONV  UINT  ctDECL ctotAlign();
extern ctCONV  COUNT ctDECL CTSBLDX();
extern ctCONV  COUNT ctDECL CTSQL();
#ifdef ctThrdApp
extern ctCONV  pVOID ctDECL ctThrdGet();
extern ctCONV  pVOID ctDECL ctThrdClr();
extern ctCONV  VOID  ctDECL ctThrdPut();
extern ctCONV  NINT  ctDECL ctThrdSet();
extern ctCONV  NINT  ctDECL ctThrdNum();
#endif
extern ctCONV  COUNT ctDECL cttestfunc();
extern ctCONV  COUNT ctDECL cttseg();
extern ctCONV  COUNT ctDECL ctuseg();
extern ctCONV  LONG  ctDECL CTUSER();
extern ctCONV  LONG  ctDECL DATENT();
extern ctCONV  LONG  ctDECL DELBLD();
extern ctCONV  COUNT ctDECL DELCHK();
extern ctCONV  COUNT ctDECL DELFIL();
extern ctCONV  COUNT ctDECL DELREC();
extern ctCONV  COUNT ctDECL DELRES();
extern ctCONV  COUNT ctDECL DELVREC();
extern ctCONV  COUNT ctDECL ENARES();
extern ctCONV  LONG  ctDECL EQLKEY();
extern ctCONV  COUNT ctDECL EQLREC();
extern ctCONV  LONG  ctDECL ESTKEY();
extern ctCONV  COUNT ctDECL EXPIFIL();
extern ctCONV  COUNT ctDECL EXPIFILX();
extern ctCONV  LONG  ctDECL FRCKEY();
extern ctCONV  COUNT ctDECL FREBAT();
extern ctCONV  COUNT ctDECL FREHST();
extern ctCONV  COUNT ctDECL FRESET();
extern ctCONV  COUNT ctDECL frmkey();
extern ctCONV  LONG  ctDECL FRSKEY();
extern ctCONV  COUNT ctDECL FRSREC();
extern ctCONV  COUNT ctDECL FRSSET();
extern ctCONV  COUNT ctDECL GETALTSEQ();
#ifdef ctCONDIDX
extern ctCONV  VRLEN ctDECL GETCRES();
extern ctCONV  VRLEN ctDECL GETCIDX();
#endif
extern ctCONV  pVOID ctDECL GETCTREE();
extern ctCONV  pTEXT ctDECL GETCURK();
extern ctCONV  pTEXT ctDECL GETCURKL();
extern ctCONV  LONG  ctDECL GETCURP();
extern ctCONV  VRLEN ctDECL GETDODA();
extern ctCONV  LONG  ctDECL GETFIL();
extern ctCONV  VRLEN ctDECL GETIFIL();
extern ctCONV  COUNT ctDECL GETMNAME();
extern ctCONV  COUNT ctDECL GETNAM();
extern ctCONV  LONG  ctDECL GETRES();
extern ctCONV  VRLEN ctDECL GETVLEN();
extern ctCONV  LONG  ctDECL GTEKEY();
extern ctCONV  COUNT ctDECL GTEREC();
extern ctCONV  LONG  ctDECL GTKEY();
extern ctCONV  COUNT ctDECL GTREC();
extern ctCONV  VRLEN ctDECL GTVLEN();
extern ctCONV  LONG  ctDECL IDXENT();
extern ctCONV  COUNT ctDECL INTISAM();
extern ctCONV  COUNT ctDECL INTISAMX();
extern ctCONV  COUNT ctDECL INTREE();
extern ctCONV  COUNT ctDECL INTREEX();
extern ctCONV  COUNT ctDECL IOPERFORMANCE();
extern ctCONV  COUNT ctDECL IOPERFORMANCEX();
extern ctCONV  COUNT ctDECL LKISAM();
extern ctCONV  COUNT ctDECL LOADKEY();
extern ctCONV  COUNT ctDECL LOKREC();
extern ctCONV  LONG  ctDECL LSTKEY();
extern ctCONV  COUNT ctDECL LSTREC();
extern ctCONV  COUNT ctDECL LSTSET();
extern ctCONV  LONG  ctDECL LTEKEY();
extern ctCONV  COUNT ctDECL LTEREC();
extern ctCONV  LONG  ctDECL LTKEY();
extern ctCONV  COUNT ctDECL LTREC();
extern ctCONV  COUNT ctDECL MIDSET();
extern ctCONV  LONG  ctDECL NEWREC();
extern ctCONV  LONG  ctDECL NEWVREC();
extern ctCONV  pTEXT ctDECL NXTCTREE();
extern ctCONV  LONG  ctDECL NXTKEY();
extern ctCONV  COUNT ctDECL NXTREC();
extern ctCONV  COUNT ctDECL NXTSET();
extern ctCONV  COUNT ctDECL OPNFIL();
extern ctCONV  COUNT ctDECL OPNFILX();
extern ctCONV  COUNT ctDECL OPNICON();
extern ctCONV  COUNT ctDECL OPNIFIL();
extern ctCONV  COUNT ctDECL OPNIFILX();
extern ctCONV  COUNT ctDECL OPNISAM();
extern ctCONV  COUNT ctDECL OPNISAMX();
extern ctCONV  COUNT ctDECL OPNRFIL();
extern ctCONV  COUNT ctDECL OPNRFILX();
extern ctCONV  LONG  ctDECL ORDKEY();
extern ctCONV  LONG  ctDECL PRVKEY();
extern ctCONV  COUNT ctDECL PRVREC();
extern ctCONV  COUNT ctDECL PRVSET();
#ifdef ctCONDIDX
extern ctCONV  COUNT ctDECL PUTCRES();
#endif
extern ctCONV  COUNT ctDECL PUTDODA();
extern ctCONV  COUNT ctDECL PUTFIL();
extern ctCONV  COUNT ctDECL PUTHDR();
extern ctCONV  COUNT ctDECL PUTIFIL();
extern ctCONV  COUNT ctDECL PUTIFILX();
extern ctCONV  COUNT ctDECL TRANABT();
extern ctCONV  COUNT ctDECL TRANABTX();
extern ctCONV  COUNT ctDECL TRANCLR();
extern ctCONV  LONG  ctDECL TSTVREC();
extern ctCONV  COUNT ctDECL RBLIIDX();
extern ctCONV  COUNT ctDECL RBLIFIL();
extern ctCONV  COUNT ctDECL RBLIFILX();
extern ctCONV  COUNT ctDECL RDVREC();
extern ctCONV  COUNT ctDECL REDIREC();
extern ctCONV  COUNT ctDECL REDREC();
extern ctCONV  COUNT ctDECL REDVREC();
extern ctCONV  COUNT ctDECL REGCTREE();
extern ctCONV  COUNT ctDECL ctRENFIL();
extern ctCONV  COUNT ctDECL RETREC();
extern ctCONV  COUNT ctDECL RETVREC();
extern ctCONV  LONG  ctDECL RNGENT();
extern ctCONV  COUNT ctDECL RRDREC();
extern ctCONV  COUNT ctDECL RWTREC();
extern ctCONV  COUNT ctDECL RWTVREC();
extern ctCONV  COUNT ctDECL SECURITY();
extern ctCONV  LONG  ctDECL SERIALNUM();
extern ctCONV  COUNT ctDECL SETALTSEQ();
extern ctCONV  COUNT ctDECL SETCURI();
extern ctCONV  COUNT ctDECL SETDEFBLK();
extern ctCONV  COUNT ctDECL SETFLTR();
extern ctCONV  NINT  ctDECL SETLOGPATH();
extern ctCONV  COUNT ctDECL SETNODE();
extern ctCONV  LONG  ctDECL SETOPS();
extern ctCONV  COUNT ctDECL SETVARBYTS();
#ifndef CTBOUND
extern ctCONV  COUNT ctDECL SETSFLV();
extern ctCONV  COUNT ctDECL SETWNAM();
#endif
extern ctCONV  COUNT ctDECL SPCLSAV();
extern ctCONV  COUNT ctDECL SQR();
extern ctCONV  COUNT ctDECL STPSRV();
extern ctCONV  COUNT ctDECL STPUSR();
extern ctCONV  COUNT ctDECL SWTCTREE();
extern ctCONV  COUNT ctDECL SYSCFG();
extern ctCONV  COUNT ctDECL SYSMON();
extern ctCONV  pTEXT ctDECL TFRMKEY();
extern ctCONV  COUNT ctDECL TMPNAME();
extern ctCONV  LONG  ctDECL TRANBEG();
extern ctCONV  COUNT ctDECL TRANEND();
extern ctCONV  COUNT ctDECL TRANRST();
extern ctCONV  COUNT ctDECL TRANSAV();
extern ctCONV  COUNT ctDECL UNRCTREE();
extern ctCONV  COUNT ctDECL UPDCIDX();
extern ctCONV  COUNT ctDECL UPDCURI();
extern ctCONV  COUNT ctDECL UPDRES();
extern ctCONV  pTEXT ctDECL WCHCTREE();
extern ctCONV  COUNT ctDECL WRTREC();
extern ctCONV  COUNT ctDECL WRTVREC();
extern ctCONV  COUNT ctDECL setfndval();
extern ctCONV  COUNT ctVDECL ctree();

extern ctCONV  COUNT ctDECL EQLVREC();
extern ctCONV  COUNT ctDECL FRSVREC();
extern ctCONV  COUNT ctDECL GTEVREC();
extern ctCONV  COUNT ctDECL GTVREC();
extern ctCONV  COUNT ctDECL LSTVREC();
extern ctCONV  COUNT ctDECL LTEVREC();
extern ctCONV  COUNT ctDECL LTVREC();
extern ctCONV  COUNT ctDECL NXTVREC();
extern ctCONV  COUNT ctDECL PRVVREC();

extern ctCONV  COUNT ctDECL FRSVSET();
extern ctCONV  COUNT ctDECL LSTVSET();
extern ctCONV  COUNT ctDECL MIDVSET();
extern ctCONV  COUNT ctDECL NXTVSET();
extern ctCONV  COUNT ctDECL PRVVSET();
extern ctCONV  COUNT ctDECL REDIVREC();

#endif /* PROTOTYPE */

#endif /* ctDECLH */

/* end of ctdecl.h */
