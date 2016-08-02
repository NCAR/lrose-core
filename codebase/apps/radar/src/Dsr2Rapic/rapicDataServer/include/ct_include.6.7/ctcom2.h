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

#ifndef ctCOM2H
#define ctCOM2H

#ifdef  ctPortUNIX
#ifndef ctCLIENT
#define ctUNXCOM_ACTIVE /* Activate multiple bound protocols. */
#endif	/* ~ctCLIENT */
#endif	/*  ctPortUNIX */

#ifdef  ctUNXCOM_ACTIVE 

#ifdef 	ctUNXCOM_ADSP
#define ctNpInit 		ADSPctNpInit
#define ctNpPolice		ADSPctNpPolice
#define ctNpSetAttribute	ADSPctNpSetAttribute   
#define ctNpGetAttribute	ADSPctNpGetAttribute
#define ctNpStart		ADSPctNpStart
#define ctNpStop		ADSPctNpStop
#define ctNpOpen		ADSPctNpOpen
#define ctNpClose		ADSPctNpClose
#define ctNpWrite		ADSPctNpWrite
#define ctNpRead		ADSPctNpRead
#define ctNpPackSiz		ADSPctNpPackSiz
#define ctNpMakeNmPipe		ADSPctNpMakeNmPipe
#define ctNpDisconnectNmPipe	ADSPctNpDisconnectNmPipe
#define ctNpConnectNmPipe	ADSPctNpConnectNmPipe

/**/
#define ctNpGlobals		ADSPctNpGlobals
#define NPfree			ADSPNPfree
#define NPalloc			ADSPNPalloc
#define NPwaitWhile		ADSPNPwaitWhile
#define NPdefer			ADSPNPdefer
#define NPcttcre		ADSPNPcttcre
#define NPrevobj		ADSPNPrevobj
#define NPctsemrqs		ADSPNPctsemrqs
#define NPctsemclr		ADSPNPctsemclr
#define NPctsemwat		ADSPNPctsemwat
#define NPctblkrqs		ADSPNPctblkrqs
#define NPctblkclr		ADSPNPctblkclr
#define NPctblkwat		ADSPNPctblkwat
#define NPcttimrqs		ADSPNPcttimrqs
#define NPcttimclr		ADSPNPcttimclr
#define NPcttimwat		ADSPNPcttimwat
#define NPGetSrVariable		ADSPNPGetSrVariable
#define NPusrstat		ADSPNPusrstat
#define NPctrt_printf		ADSPNPctrt_printf
/**/
#define at_CommunicationPolice	ADSPat_CommunicationPolice
#define at_CommunicationInit	ADSPat_CommunicationInit
#define at_SessionSetAttribute	ADSPat_SessionSetAttribute
#define at_SessionValidate	ADSPat_SessionValidate
#define at_SessionNew		ADSPat_SessionNew
#define at_SessionDispose	ADSPat_SessionDispose
#define at_SessionOpen		ADSPat_SessionOpen
#define at_SessionClose		ADSPat_SessionClose
#define at_SessionSend		ADSPat_SessionSend
#define at_SessionReceive	ADSPat_SessionReceive
#define at_SessionMake		ADSPat_SessionMake
#define at_SessionConnect	ADSPat_SessionConnect
#define at_SessionDisconnect	ADSPat_SessionDisconnect
#define at_CommunicationStart	ADSPat_CommunicationStart
#define at_CommunicationStop	ADSPat_CommunicationStop
/*#define find_syssem		ADSPfind_syssem */
/*#define find_wait		ADSPfind_wait */
#define VectorToBuffer		ADSPVectorToBuffer
#define BufferToVector		ADSPBufferToVector
#define IsPIDAlive		ADSPIsPIDAlive
#endif /* ctUNXCOM_ADSP */

#ifdef 	ctUNXCOM_TCP
#define ctNpInit 		TCPctNpInit
#define ctNpPolice		TCPctNpPolice
#define ctNpSetAttribute	TCPctNpSetAttribute   
#define ctNpGetAttribute	TCPctNpGetAttribute
#define ctNpStart		TCPctNpStart
#define ctNpStop		TCPctNpStop
#define ctNpOpen		TCPctNpOpen
#define ctNpClose		TCPctNpClose
#define ctNpWrite		TCPctNpWrite
#define ctNpRead		TCPctNpRead
#define ctNpPackSiz		TCPctNpPackSiz
#define ctNpMakeNmPipe		TCPctNpMakeNmPipe
#define ctNpDisconnectNmPipe	TCPctNpDisconnectNmPipe
#define ctNpConnectNmPipe	TCPctNpConnectNmPipe

/**/
#define ctNpGlobals		TCPctNpGlobals
#define NPfree			TCPNPfree
#define NPalloc			TCPNPalloc
#define NPwaitWhile		TCPNPwaitWhile
#define NPdefer			TCPNPdefer
#define NPcttcre		TCPNPcttcre
#define NPrevobj		TCPNPrevobj
#define NPctsemrqs		TCPNPctsemrqs
#define NPctsemclr		TCPNPctsemclr
#define NPctsemwat		TCPNPctsemwat
#define NPctblkrqs		TCPNPctblkrqs
#define NPctblkclr		TCPNPctblkclr
#define NPctblkwat	       	TCPNPctblkwat
#define NPcttimrqs		TCPNPcttimrqs
#define NPcttimclr		TCPNPcttimclr
#define NPcttimwat		TCPNPcttimwat
#define NPGetSrVariable		TCPNPGetSrVariable
#define NPusrstat		TCPNPusrstat
#define NPctrt_printf		TCPNPctrt_printf
/**/
#define at_CommunicationPolice	TCPat_CommunicationPolice
#define at_CommunicationInit	TCPat_CommunicationInit
#define at_SessionSetAttribute	TCPat_SessionSetAttribute
#define at_SessionValidate	TCPat_SessionValidate
#define at_SessionNew		TCPat_SessionNew
#define at_SessionDispose	TCPat_SessionDispose
#define at_SessionOpen		TCPat_SessionOpen
#define at_SessionClose		TCPat_SessionClose
#define at_SessionSend		TCPat_SessionSend
#define at_SessionReceive	TCPat_SessionReceive
#define at_SessionMake		TCPat_SessionMake
#define at_SessionConnect	TCPat_SessionConnect
#define at_SessionDisconnect	TCPat_SessionDisconnect
#define at_CommunicationStart	TCPat_CommunicationStart
#define at_CommunicationStop	TCPat_CommunicationStop
/*#define find_syssem		TCPfind_syssem */
/*#define find_wait		TCPfind_wait */
#define VectorToBuffer		TCPVectorToBuffer
#define BufferToVector		TCPBufferToVector
#define IsPIDAlive		TCPIsPIDAlive
#endif /* ctUNXCOM_TCP */

#ifdef 	ctUNXCOM_SPX
#define ctNpInit 		SPXctNpInit
#define ctNpPolice		SPXctNpPolice
#define ctNpSetAttribute	SPXctNpSetAttribute   
#define ctNpGetAttribute	SPXctNpGetAttribute
#define ctNpStart		SPXctNpStart
#define ctNpStop		SPXctNpStop
#define ctNpOpen		SPXctNpOpen
#define ctNpClose		SPXctNpClose
#define ctNpWrite		SPXctNpWrite
#define ctNpRead		SPXctNpRead
#define ctNpPackSiz		SPXctNpPackSiz
#define ctNpMakeNmPipe		SPXctNpMakeNmPipe
#define ctNpDisconnectNmPipe	SPXctNpDisconnectNmPipe
#define ctNpConnectNmPipe	SPXctNpConnectNmPipe

/**/
#define ctNpGlobals		SPXctNpGlobals
#define NPfree			SPXNPfree
#define NPalloc			SPXNPalloc
#define NPwaitWhile		SPXNPwaitWhile
#define NPdefer			SPXNPdefer
#define NPcttcre		SPXNPcttcre
#define NPrevobj		SPXNPrevobj
#define NPctsemrqs		SPXNPctsemrqs
#define NPctsemclr		SPXNPctsemclr
#define NPctsemwat		SPXNPctsemwat
#define NPctblkrqs		SPXNPctblkrqs
#define NPctblkclr		SPXNPctblkclr
#define NPctblkwat	       	SPXNPctblkwat
#define NPcttimrqs		SPXNPcttimrqs
#define NPcttimclr		SPXNPcttimclr
#define NPcttimwat		SPXNPcttimwat
#define NPGetSrVariable		SPXNPGetSrVariable
#define NPusrstat		SPXNPusrstat
#define NPctrt_printf		SPXNPctrt_printf
/**/
#define at_CommunicationPolice	SPXat_CommunicationPolice
#define at_CommunicationInit	SPXat_CommunicationInit
#define at_SessionSetAttribute	SPXat_SessionSetAttribute
#define at_SessionValidate	SPXat_SessionValidate
#define at_SessionNew		SPXat_SessionNew
#define at_SessionDispose	SPXat_SessionDispose
#define at_SessionOpen		SPXat_SessionOpen
#define at_SessionClose		SPXat_SessionClose
#define at_SessionSend		SPXat_SessionSend
#define at_SessionReceive	SPXat_SessionReceive
#define at_SessionMake		SPXat_SessionMake
#define at_SessionConnect	SPXat_SessionConnect
#define at_SessionDisconnect	SPXat_SessionDisconnect
#define at_CommunicationStart	SPXat_CommunicationStart
#define at_CommunicationStop	SPXat_CommunicationStop
/*#define find_syssem		SPXfind_syssem */
/*#define find_wait		SPXfind_wait */
#define VectorToBuffer		SPXVectorToBuffer
#define BufferToVector		SPXBufferToVector
#define IsPIDAlive		SPXIsPIDAlive
#endif /* ctUNXCOM_SPX */

#ifdef 	ctUNXCOM_QUE
#define ctNpInit 		QUEctNpInit
#define ctNpPolice		QUEctNpPolice
#define ctNpSetAttribute   	QUEctNpSetAttribute   
#define ctNpGetAttribute	QUEctNpGetAttribute
#define ctNpStart		QUEctNpStart
#define ctNpStop		QUEctNpStop
#define ctNpOpen		QUEctNpOpen
#define ctNpClose		QUEctNpClose
#define ctNpWrite		QUEctNpWrite
#define ctNpRead		QUEctNpRead
#define ctNpPackSiz		QUEctNpPackSiz
#define ctNpMakeNmPipe		QUEctNpMakeNmPipe
#define ctNpDisconnectNmPipe	QUEctNpDisconnectNmPipe
#define ctNpConnectNmPipe	QUEctNpConnectNmPipe
/**/
#define ctNpGlobals		QUEctNpGlobals
#define NPfree			QUENPfree
#define NPalloc			QUENPalloc
#define NPwaitWhile		QUENPwaitWhile
#define NPdefer			QUENPdefer
#define NPcttcre		QUENPcttcre
#define NPrevobj		QUENPrevobj
#define NPctsemrqs		QUENPctsemrqs
#define NPctsemclr		QUENPctsemclr
#define NPctsemwat		QUENPctsemwat
#define NPctblkrqs		QUENPctblkrqs
#define NPctblkclr		QUENPctblkclr
#define NPctblkwat		QUENPctblkwat
#define NPcttimrqs		QUENPcttimrqs
#define NPcttimclr		QUENPcttimclr
#define NPcttimwat		QUENPcttimwat
#define NPGetSrVariable		QUENPGetSrVariable
#define NPusrstat		QUENPusrstat
#define NPctrt_printf		QUENPctrt_printf
/**/
#define at_CommunicationPolice	QUEat_CommunicationPolice
#define at_CommunicationInit	QUEat_CommunicationInit
#define at_SessionSetAttribute	QUEat_SessionSetAttribute
#define at_SessionValidate	QUEat_SessionValidate
#define at_SessionNew		QUEat_SessionNew
#define at_SessionDispose	QUEat_SessionDispose
#define at_SessionOpen		QUEat_SessionOpen
#define at_SessionClose		QUEat_SessionClose
#define at_SessionSend		QUEat_SessionSend
#define at_SessionReceive	QUEat_SessionReceive
#define at_SessionMake		QUEat_SessionMake
#define at_SessionConnect	QUEat_SessionConnect
#define at_SessionDisconnect	QUEat_SessionDisconnect
#define at_CommunicationStart	QUEat_CommunicationStart
#define at_CommunicationStop	QUEat_CommunicationStop
#define find_syssem		QUEfind_syssem
#define find_wait		QUEfind_wait
#define VectorToBuffer		QUEVectorToBuffer
#define BufferToVector		QUEBufferToVector
#define IsPIDAlive		QUEIsPIDAlive

#endif /* ctUNXCOM_QUE */

#ifdef 	ctUNXCOM_QNX
#define ctNpInit 		QNXctNpInit
#define ctNpPolice		QNXctNpPolice
#define ctNpSetAttribute   	QNXctNpSetAttribute   
#define ctNpGetAttribute	QNXctNpGetAttribute
#define ctNpStart		QNXctNpStart
#define ctNpStop		QNXctNpStop
#define ctNpOpen		QNXctNpOpen
#define ctNpClose		QNXctNpClose
#define ctNpWrite		QNXctNpWrite
#define ctNpRead		QNXctNpRead
#define ctNpPackSiz		QNXctNpPackSiz
#define ctNpMakeNmPipe		QNXctNpMakeNmPipe
#define ctNpDisconnectNmPipe	QNXctNpDisconnectNmPipe
#define ctNpConnectNmPipe	QNXctNpConnectNmPipe
/**/
#define ctNpGlobals		QNXctNpGlobals
#define NPfree			QNXNPfree
#define NPalloc			QNXNPalloc
#define NPwaitWhile		QNXNPwaitWhile
#define NPdefer			QNXNPdefer
#define NPcttcre		QNXNPcttcre
#define NPrevobj		QNXNPrevobj
#define NPctsemrqs		QNXNPctsemrqs
#define NPctsemclr		QNXNPctsemclr
#define NPctsemwat		QNXNPctsemwat
#define NPctblkrqs		QNXNPctblkrqs
#define NPctblkclr		QNXNPctblkclr
#define NPctblkwat		QNXNPctblkwat
#define NPcttimrqs		QNXNPcttimrqs
#define NPcttimclr		QNXNPcttimclr
#define NPcttimwat		QNXNPcttimwat
#define NPGetSrVariable		QNXNPGetSrVariable
#define NPusrstat		QNXNPusrstat
#define NPctrt_printf		QNXNPctrt_printf
/**/
#define at_CommunicationPolice	QNXat_CommunicationPolice
#define at_CommunicationInit	QNXat_CommunicationInit
#define at_SessionSetAttribute	QNXat_SessionSetAttribute
#define at_SessionValidate	QNXat_SessionValidate
#define at_SessionNew		QNXat_SessionNew
#define at_SessionDispose	QNXat_SessionDispose
#define at_SessionOpen		QNXat_SessionOpen
#define at_SessionClose		QNXat_SessionClose
#define at_SessionSend		QNXat_SessionSend
#define at_SessionReceive	QNXat_SessionReceive
#define at_SessionMake		QNXat_SessionMake
#define at_SessionConnect	QNXat_SessionConnect
#define at_SessionDisconnect	QNXat_SessionDisconnect
#define at_CommunicationStart	QNXat_CommunicationStart
#define at_CommunicationStop	QNXat_CommunicationStop
#define find_syssem		QNXfind_syssem
#define find_wait		QNXfind_wait
#define VectorToBuffer		QNXVectorToBuffer
#define BufferToVector		QNXBufferToVector
#define IsPIDAlive		QNXIsPIDAlive

#endif /* ctUNXCOM_QNX */
#ifdef 	ctUNXCOM_NTP
#define ctNpInit 		NTPctNpInit
#define ctNpPolice		NTPctNpPolice
#define ctNpSetAttribute   	NTPctNpSetAttribute   
#define ctNpGetAttribute	NTPctNpGetAttribute
#define ctNpStart		NTPctNpStart
#define ctNpStop		NTPctNpStop
#define ctNpOpen		NTPctNpOpen
#define ctNpClose		NTPctNpClose
#define ctNpWrite		NTPctNpWrite
#define ctNpRead		NTPctNpRead
#define ctNpPackSiz		NTPctNpPackSiz
#define ctNpMakeNmPipe		NTPctNpMakeNmPipe
#define ctNpDisconnectNmPipe	NTPctNpDisconnectNmPipe
#define ctNpConnectNmPipe	NTPctNpConnectNmPipe
/**/
#define ctNpGlobals		NTPctNpGlobals
#define NPfree			NTPNPfree
#define NPalloc			NTPNPalloc
#define NPwaitWhile		NTPNPwaitWhile
#define NPdefer			NTPNPdefer
#define NPcttcre		NTPNPcttcre
#define NPrevobj		NTPNPrevobj
#define NPctsemrqs		NTPNPctsemrqs
#define NPctsemclr		NTPNPctsemclr
#define NPctsemwat		NTPNPctsemwat
#define NPctblkrqs		NTPNPctblkrqs
#define NPctblkclr		NTPNPctblkclr
#define NPctblkwat		NTPNPctblkwat
#define NPcttimrqs		NTPNPcttimrqs
#define NPcttimclr		NTPNPcttimclr
#define NPcttimwat		NTPNPcttimwat
#define NPGetSrVariable		NTPNPGetSrVariable
#define NPusrstat		NTPNPusrstat
#define NPctrt_printf		NTPNPctrt_printf
/**/
#define at_CommunicationPolice	NTPat_CommunicationPolice
#define at_CommunicationInit	NTPat_CommunicationInit
#define at_SessionSetAttribute	NTPat_SessionSetAttribute
#define at_SessionValidate	NTPat_SessionValidate
#define at_SessionNew		NTPat_SessionNew
#define at_SessionDispose	NTPat_SessionDispose
#define at_SessionOpen		NTPat_SessionOpen
#define at_SessionClose		NTPat_SessionClose
#define at_SessionSend		NTPat_SessionSend
#define at_SessionReceive	NTPat_SessionReceive
#define at_SessionMake		NTPat_SessionMake
#define at_SessionConnect	NTPat_SessionConnect
#define at_SessionDisconnect	NTPat_SessionDisconnect
#define at_CommunicationStart	NTPat_CommunicationStart
#define at_CommunicationStop	NTPat_CommunicationStop
#define find_syssem		NTPfind_syssem
#define find_wait		NTPfind_wait
#define VectorToBuffer		NTPVectorToBuffer
#define BufferToVector		NTPBufferToVector
#define IsPIDAlive		NTPIsPIDAlive

#endif /* ctUNXCOM_NTP */

#ifdef 	ctUNXCOM_SHM
#define ctNpInit 		SHMctNpInit
#define ctNpPolice		SHMctNpPolice
#define ctNpSetAttribute   	SHMctNpSetAttribute   
#define ctNpGetAttribute	SHMctNpGetAttribute
#define ctNpStart		SHMctNpStart
#define ctNpStop		SHMctNpStop
#define ctNpOpen		SHMctNpOpen
#define ctNpClose		SHMctNpClose
#define ctNpWrite		SHMctNpWrite
#define ctNpRead		SHMctNpRead
#define ctNpPackSiz		SHMctNpPackSiz
#define ctNpMakeNmPipe		SHMctNpMakeNmPipe
#define ctNpDisconnectNmPipe	SHMctNpDisconnectNmPipe
#define ctNpConnectNmPipe	SHMctNpConnectNmPipe
/**/
#define ctNpGlobals		SHMctNpGlobals
#define NPfree			SHMNPfree
#define NPalloc			SHMNPalloc
#define NPwaitWhile		SHMNPwaitWhile
#define NPdefer			SHMNPdefer
#define NPcttcre		SHMNPcttcre
#define NPrevobj		SHMNPrevobj
#define NPctsemrqs		SHMNPctsemrqs
#define NPctsemclr		SHMNPctsemclr
#define NPctsemwat		SHMNPctsemwat
#define NPctblkrqs		SHMNPctblkrqs
#define NPctblkclr		SHMNPctblkclr
#define NPctblkwat		SHMNPctblkwat
#define NPcttimrqs		SHMNPcttimrqs
#define NPcttimclr		SHMNPcttimclr
#define NPcttimwat		SHMNPcttimwat
#define NPGetSrVariable		SHMNPGetSrVariable
#define NPusrstat		SHMNPusrstat
#define NPctrt_printf		SHMNPctrt_printf
/**/
#define at_CommunicationPolice	SHMat_CommunicationPolice
#define at_CommunicationInit	SHMat_CommunicationInit
#define at_SessionSetAttribute	SHMat_SessionSetAttribute
#define at_SessionValidate	SHMat_SessionValidate
#define at_SessionNew		SHMat_SessionNew
#define at_SessionDispose	SHMat_SessionDispose
#define at_SessionOpen		SHMat_SessionOpen
#define at_SessionClose		SHMat_SessionClose
#define at_SessionSend		SHMat_SessionSend
#define at_SessionReceive	SHMat_SessionReceive
#define at_SessionMake		SHMat_SessionMake
#define at_SessionConnect	SHMat_SessionConnect
#define at_SessionDisconnect	SHMat_SessionDisconnect
#define at_CommunicationStart	SHMat_CommunicationStart
#define at_CommunicationStop	SHMat_CommunicationStop

#define find_syssem		SHMfind_syssem
#define find_wait		SHMfind_wait
#define VectorToBuffer		SHMVectorToBuffer
#define BufferToVector		SHMBufferToVector
#define IsPIDAlive		SHMIsPIDAlive
#endif /* ctUNXCOM_SHM */

#endif /* ctUNXCOM_ACTIVE */
#endif /* ctCOM2H */

/* end of ctcom2.h */
