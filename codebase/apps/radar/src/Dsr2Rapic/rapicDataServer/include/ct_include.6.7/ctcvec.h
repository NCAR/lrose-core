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

#ifndef ctCALLVECTORS
#define ctCALLVECTORS

#include "ctcomm.h"

#ifdef PROTOTYPE
extern  pCommFuncPtrs DllExport DllLoadDS ctNpGlobals(pSrvrGbls psg);
extern	NINT  DllLoadDS ctNpInit(pSrvrGbls pGlobals);
extern  NINT  DllLoadDS ctNpStart(pSrvrGbls pGlobals, pTEXT, pTEXT);
extern	NINT  DllLoadDS ctNpStop(pSrvrGbls pGlobals);
extern	NINT  DllLoadDS ctNpOpen(pSrvrGbls pGlobals, pTEXT,pUCOUNT,pUCOUNT,ULONG,ULONG,UCOUNT,UCOUNT,ULONG);
extern	NINT  DllLoadDS ctNpClose(pSrvrGbls pGlobals, UCOUNT);
extern	NINT  DllLoadDS ctNpWrite(pSrvrGbls pGlobals, UCOUNT,pMTEXT,ULONG,pMTEXT,ULONG,pULONG);
extern	NINT  DllLoadDS ctNpRead(pSrvrGbls pGlobals, UCOUNT, pMTEXT,ULONG,pMTEXT,ULONG,pULONG);
extern	NINT  DllLoadDS ctNpMakeNmPipe(pSrvrGbls pGlobals, pTEXT,pULONG,UCOUNT,UCOUNT,ULONG,ULONG,LONG,pTEXT);
extern	NINT  DllLoadDS ctNpDisconnectNmPipe(pSrvrGbls pGlobals, UCOUNT);
extern	NINT  DllLoadDS ctNpConnectNmPipe(pSrvrGbls pGlobals, UCOUNT);
extern	VRLEN DllLoadDS ctNpPackSiz(pSrvrGbls pGlobals, VRLEN, pVRLEN);
extern	NINT  DllLoadDS ctNpSetAttribute(pSrvrGbls pGlobals, UCOUNT, NINT, pVOID);
extern	NINT  DllLoadDS ctNpGetAttribute(pSrvrGbls pGlobals, UCOUNT, NINT, pVOID);
extern	NINT  DllLoadDS ctNpPolice(pSrvrGbls pGlobals);

extern  NINT	NPdefer(pSrvrGbls pGlobals, LONG);
extern  NINT    NPcttcre(pSrvrGbls pGlobals,pNINT ,NINT (*)(VOID ),UINT ,NLONG);

extern	NINT	NPwaitWhile(pSrvrGbls pGlobals, pCOUNT, LONG);

extern	NINT	NPctsemrqs(pSrvrGbls pGlobals, pSEMA, LONG, NINT);
extern	NINT	NPctsemclr(pSrvrGbls pGlobals, pSEMA, NINT);
extern	NINT	NPctsemwat(pSrvrGbls pGlobals, pSEMA, LONG, NINT);

extern	NINT	NPctblkrqs(pSrvrGbls pGlobals, pSEMA, LONG, NINT);
extern	NINT	NPctblkclr(pSrvrGbls pGlobals, pSEMA, NINT);
extern	NINT	NPctblkwat(pSrvrGbls pGlobals, pSEMA, LONG, NINT);

extern	NINT	NPcttimrqs(pSrvrGbls pGlobals, pSEMA, LONG, NINT);
extern	NINT	NPcttimclr(pSrvrGbls pGlobals, pSEMA, NINT);
extern	NINT	NPcttimwat(pSrvrGbls pGlobals, pSEMA, LONG, NINT);

extern  pVOID	NPalloc(pSrvrGbls pGlobals, NINT, UINT);
extern	VOID	NPfree(pSrvrGbls pGlobals, pVOID);
extern  VOID 	NPrevobj(pSrvrGbls pGlobals, pVOID, NINT);
extern  NINT	NPusrstat(pSrvrGbls pGlobals, NINT);
extern  LONG 	NPGetSrVariable(pSrvrGbls pGlobals, NINT this_var);
extern  LONG 	NPctrt_printf(pSrvrGbls pGlobals, pTEXT p);
#else
extern 	pCommFuncPtrs DllExport DllLoadDS ctNpGlobals();
extern	NINT  DllLoadDS ctNpInit();
extern 	NINT  DllLoadDS ctNpStart();
extern	NINT  DllLoadDS ctNpStop();
extern	NINT  DllLoadDS ctNpOpen();
extern	NINT  DllLoadDS ctNpClose();
extern	NINT  DllLoadDS ctNpWrite();
extern	NINT  DllLoadDS ctNpRead();
extern	NINT  DllLoadDS ctNpMakeNmPipe();
extern	NINT  DllLoadDS ctNpDisconnectNmPipe();
extern	NINT  DllLoadDS ctNpConnectNmPipe();
extern	VRLEN DllLoadDS ctNpPackSiz();
extern	NINT  DllLoadDS ctNpSetAttribute();
extern	NINT  DllLoadDS ctNpGetAttribute();
extern	NINT  DllLoadDS ctNpPolice();

extern  NINT	NPdefer();
extern  NINT    NPcttcre();

extern	NINT	NPwaitWhile();

extern	NINT	NPctsemrqs();
extern	NINT	NPctsemclr();
extern	NINT	NPctsemwat();

extern	NINT	NPctblkrqs();
extern	NINT	NPctblkclr();
extern	NINT	NPctblkwat();

extern	NINT	NPcttimrqs();
extern	NINT	NPcttimclr();
extern	NINT	NPcttimwat();

extern  pVOID	NPalloc();
extern	VOID	NPfree();
extern  VOID	NPrevobj();
extern  NINT	NPusrstat();
extern  LONG 	NPGetSrVariable();
extern  LONG 	NPctrt_printf();
#endif

#endif /* ~ctCALLVECTORS */
/* end ctcvec.h */

