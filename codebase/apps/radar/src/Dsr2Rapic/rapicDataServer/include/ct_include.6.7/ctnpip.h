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

#ifndef ctNAMEPIPE
#define ctNAMEPIPE

#ifdef CTBOUND
#include "ctbond.h"
#endif
#include "ctcvec.h"

#ifdef ctDLLOAD
#define BUILDASDLL
#endif

#define NERR_SUCCESS		0x00
#define NERR_BROKEN_PIPE	0x01
#define NERR_INVALID_PARAMETER	0x02
#define NERR_BUFFER_TO_SMALL	0x03
#define NERR_TIME_OUT		0x04
#define NERR_INTERRUPT		0x05
#define NERR_INVALID_HANDLE	0x06
#define NERR_INVALID_NAME	0x07
#define NERR_PATH_NOT_FOUND	0x08
#define NERR_NOT_ENOUGH_MEM	0x09
#define NERR_ADVERTISING	0x0A
#define NERR_RECEIVE		0x0B
#define NERR_SEND		0x0C
#define NERR_ENDPOINT		0x0D

/* SERVER should be defined to the name of the registered server. It
 * is used by AppleTalk Name Binding Protocol to get the proper 
 * session listening socket for the server.
 */
#define CTRE		0x43434343L

#ifdef LOW_HIGH
#define ctHIWORD(l) ((COUNT)((((LONG)(l)) >> 16) & 0xFFFF))
#define ctLOWORD(l) ((COUNT)(LONG)(l))
#else
#define ctLOWORD(l) ((COUNT)((((LONG)(l)) >> 16) & 0xFFFF))
#define ctHIWORD(l) ((COUNT)(LONG)(l))
#endif

/*
 * prototypes
 */

#ifdef PROTOTYPE
extern pSessblk 	at_SessionValidate(pSrvrGbls pGlobals, UCOUNT);
extern UINT 		at_SessionNew(pSrvrGbls pGlobals, UCOUNT, pTEXT, ULONG, UCOUNT, UCOUNT, UCOUNT);
extern VOID 		at_SessionDispose(pSrvrGbls pGlobals, pSessblk);
extern NINT 		at_SessionOpen(pSrvrGbls pGlobals, pSessblk);
extern VOID 		at_SessionClose(pSrvrGbls pGlobals, pSessblk);
extern NINT		at_SessionSend(pSrvrGbls pGlobals, pSessblk, pMTEXT, ULONG, pMTEXT, ULONG, pULONG);
extern NINT		at_SessionReceive(pSrvrGbls pGlobals, pSessblk, pMTEXT, ULONG, pMTEXT, ULONG, pULONG);
extern NINT 		at_SessionMake(pSrvrGbls pGlobals, pSessblk);
extern NINT 		at_SessionConnect(pSrvrGbls pGlobals, pSessblk);
extern NINT 		at_SessionDisconnect(pSrvrGbls pGlobals, pSessblk);
extern VOID 		at_CommunicationInit(pSrvrGbls pGlobals);
extern NINT 		at_CommunicationStart(pSrvrGbls pGlobals, pTEXT);
extern NINT 		at_SessionSetAttribute(pSrvrGbls pGlobals, pSessblk, NINT, pVOID);
extern VOID 		at_CommunicationStop(pSrvrGbls pGlobals);
extern NINT 		at_CommunicationPolice(pSrvrGbls pGlobals);

#else

extern pSessblk 	at_SessionValidate();
extern UINT 		at_SessionNew();
extern VOID 		at_SessionDispose();
extern NINT 		at_SessionOpen();
extern VOID 		at_SessionClose();
extern NINT		at_SessionSend();
extern NINT		at_SessionReceive();
extern NINT 		at_SessionMake();
extern NINT 		at_SessionConnect();
extern NINT 		at_SessionDisconnect();
extern VOID 		at_CommunicationInit();
extern NINT 		at_CommunicationStart();
extern VOID 		at_CommunicationStop();
extern NINT 		at_SessionSetAttribute();
extern NINT 		at_CommunicationPolice();

#endif /* ~PROTOTYPE */
#endif /* ~ctNAMEPIPE */

/* end ctnpip.h */
