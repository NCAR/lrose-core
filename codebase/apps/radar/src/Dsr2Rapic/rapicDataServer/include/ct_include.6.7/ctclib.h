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

#ifdef ctPortUNIX
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

extern int errno;
#endif /* ctPortUNIX */


#ifndef ctPortUNIX
#include <fcntl.h>
#include <sys\types.h>		
#include <sys\stat.h>
#include <share.h>		
#ifndef ctPortOS2
#include <sys\locking.h>	
#endif
#include <io.h>  
#include <direct.h>
#include <errno.h>		   
#include <time.h>
#include <stdarg.h>
#include <limits.h>
#endif /* ~ctPortUNIX */

#ifdef ctPortOS2
#define INCL_BASE
#define INCL_DOS
#define INCL_DOSFILEMGR
#define INCL_DOSSEMAPHORES
#include <os2.h>
#include <signal.h>
#endif

#ifdef ctPortWIN16
/* these temp buffers are used by several functions and are declared here */
/* so that they will have a segment address of DS and not SS */
char TempBuf[MAX_NAME], TempBuf1[MAX_NAME];
HANDLE hInst;                /* handle to c-tree instance */
extern unsigned char	_osmajor;

#define HA_OFFSET(vp)	((((UCOUNT far *) &(vp)))[0])
#define HA_SELCTR(vp)	((((UCOUNT far *) &(vp)))[1])
#define BLOCK		((UCOUNT) 63488)
#endif /* ctPortWIN16 */

#ifdef ctPortDOS
#define HA_OFFSET(vp)	((((UCOUNT *) &(vp)))[0])
#define HA_SELCTR(vp)	((((UCOUNT *) &(vp)))[1])
#define BLOCK		((UCOUNT) 63488)
#endif /* ctPortDOS */

/****************************************************************************/
/* ctclib prototypes */
#ifdef PROTOTYPE

/* ctsthd.c */
/* extern  VOID _lds_   ctdefer(LONG ); */
extern VOID killsrvr(NINT);
extern VOID stopsrvr(NINT);
/* extern  NINT         ctendthread(NINT ,NINT (*)(VOID )); */

/* extern  NINT DllLoadDS ctsemclr(pSEMA sema,NINT own);*/
/* extern  NINT DllLoadDS ctsemcls(pSEMA sema); */
/* extern  NINT DllLoadDS ctsemrqs(pSEMA sema,LONG wait,NINT own); */
/* extern  NINT         ctsemwat(pSEMA sema,LONG wait,NINT own); */
/* extern  NINT DllLoadDS ctblkclr(pSEMA sema,NINT own); */
/* extern  NINT         ctblkwat(pSEMA sema,LONG wait,NINT own); */
/* extern  NINT DllLoadDS ctblkrqs(pSEMA sema,LONG wait,NINT own); */
/* extern  VOID         cttsus(LONG tid); */
/* extern  NINT         cttcre(pNINT ,NINT (*)(VOID ),UINT ,NLONG ); */

/* ctstim.c */
/* extern  LONG         cttime(VOID ); */
/* extern  pTEXT        ctdate(LONG pt); */

/* ctsdio.c */
/* extern  LONG         ctfsize(pCTFILE ctnum); */
/* extern  COUNT        ctseek(RNDFILE cfd,LONG recbyt); */
/* extern  COUNT        ctsysio(COUNT op_code,RNDFILE cfd,LONG recbyt,pVOID bufadr,VRLEN iosize,pLONG pretsiz); */

/* ctsmem.c */
/* extern  NINT         ctcfill(pTEXT tp,NINT ch,VRLEN len); */

/* ctsstr.c */
/* extern  NINT         ctrt_filcmp(pTEXT n1,pTEXT n2); */
/* extern  NINT         ctcnvfn(pTEXT fn); */


/* ctsint.c */
/* extern VOID  ctsysint(pCTINIT1 pconfig);	*/		/* already in ctfunp.h */

/* ctsctu.c */
/* extern ctCONV LONG ctDECL CTUSER(pTEXT command,pTEXT bufptr,VRLEN bufsiz); */

/* ctslck.c */
/* extern COUNT LOCK(LONG node,pCTFILE knum); */
/* extern COUNT UNLOCK(LONG node,pCTFILE knum); */
/* extern COUNT DLOCK(ctRECPT recbyt,pCTFILE dnum,NINT lokmod); */
/* extern COUNT RLOCK(ctRECPT recbyt,pCTFILE dnum); */
/* extern COUNT UDLOCK(ctRECPT recbyt,pCTFILE dnum); */

/* ctsfio.c */
extern NINT ctGetCwd(pTEXT cwd, NINT buflen);
extern NINT ctFileAccess(pTEXT filnam);
/* extern COUNT 	dltfil(pTEXT filnam); */
/* extern COUNT 	renfil(pTEXT oldnam,pTEXT newnam); */
/* extern COUNT 	mbclos(pCTFILE ctnum,COUNT clmode); */
/* extern static    	dupopen(ctnum,acflag,shflag,zerofd); */
/* extern RNDFILE 	mbopen (pCTFILE ctnum,pTEXT fn,COUNT opmode); */
/* extern RNDFILE 	mbcrat(pCTFILE ctnum,COUNT opmode); */
/* extern VOID 		flushdos(COUNT datno); */
/* extern COUNT 	ctsync(pCTFILE ctnum,NINT mode); */

/* ctsdos.c */
#ifdef ctPort16BIT
pTEXT mblllc(NINT numobj,VRLEN sizobj);
VOID cthfree(pVOID p);
VOID ctbfill(pVOID dp,NINT ch,VRLEN n);
pTEXT bigadr(pTEXT tp,VRLEN offset);
void cpybig(pTEXT dp,pTEXT sp,VRLEN n);
VOID cpylodl(ppTEXT hdp,pVOID sp,VRLEN n);
VOID cpysrcl(pVOID dp,ppTEXT hsp,VRLEN n);
static UINT doio(NINT op_code,NINT cfd,pVOID bufadr,UINT len);
VRLEN comparu(pTEXT np,pTEXT op,VRLEN remlen);
NINT CTCFILL( pTEXT tp, NINT ch, UINT len );
#ifdef ctPortWIN16
ctCONV BOOL ctDECL ViewCtreeError( HWND hWnd );
#endif
#endif /* ctPort16BIT */

#endif /* PROTOTYPE */

#ifndef PROTOTYPE
extern VOID killsrvr();
extern VOID stopsrvr();
extern NINT ctGetCwd();
extern NINT ctFileAccess();
#endif
/****************************************************************************/

/* end of ctclib.h */

