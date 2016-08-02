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

/*   ctsysi.h     last modified 12/03/96   */
#ifndef ctSYSIH
#define ctSYSIH

/* change to reflect current shipping working version */

#define	ctVersionid 0x7776728CL

#ifdef ctPort88OPEN
#define ctPortid	0x11222211L
#endif

#ifdef ctPortATTUNIX386
#define ctPortid	0x11333311L
#endif

#ifdef ctPortVINES
#define ctPortid	0x11444411L
#endif

#ifdef ctPortCHORUS
#define ctPortid	0x11555511L
#endif

#ifdef ctPortOS2_2x
#define ctPortid	0x11666611L
#endif

#ifdef ctPortALPHA
#define ctPortid	0x11777711L
#endif

#ifdef ctPortHP9000
#define ctPortid	0x11888811L
#endif

#ifdef ctPortINTERACTIVE
#define ctPortid	0x11999911L
#endif

#ifdef ctPortLYNXOS386
#define ctPortid	0x11AAAA11L
#endif

#ifdef ctPortAPPLEAUX
#define ctPortid	0x11BBBB11L
#endif

#ifdef ctPortNECEWS4800
#define ctPortid	0x11CCCC11L
#endif

#ifdef ctPortWNT386
#define ctPortid	0x11DDDD11L
#endif

#ifdef ctPortQNX
#define ctPortid	0x11EEEE11L
#endif

#ifdef ctPortRS6000
#define ctPortid	0x11FFFF11L
#endif

#ifdef ctPortQNX16
#define ctPortid	0x22111122L
#endif

#ifdef ctPortOS2_1x
#define ctPortid	0x22333322L
#endif

#ifdef ctPortSCO386
#define ctPortid	0x22444422L
#endif

#ifdef ctPortMSDOS
#define ctPortid	0x22555522L
#endif

#ifdef ctPortSUN41
#define ctPortid	0x22666622L
#endif

#ifdef ctPortMSWINDOWS
#define ctPortid	0x22777722L
#endif

#ifdef ctPortSOLARIS
#define ctPortid	0x22888822L
#endif

#ifdef ctPortNLM
#define ctPortid	0x22999922L
#endif

#ifdef ctPortMIPSABI
#define ctPortid	0x22AAAA22L
#endif

#ifdef ctPortAPPLE7
#define ctPortid	0x22BBBB22L
#endif

#ifdef ctPortWIN95
#define ctPortid	0x22CCCC22L
#endif

#ifdef ctPortLINUX
#define ctPortid	0x22DDDD22L
#endif

#ifdef ctPortSYSMAN
#define ctPortid	0x22EEEE22L
#endif

#ifdef ctPortWNTALPHA
#define ctPortid	0x22FFFF22L
#endif

#ifdef ctPortWNTMIPS
#define ctPortid	0x33111133L
#endif

#ifdef ctPortSOL386
#define ctPortid	0x33222233L
#endif

#ifdef ctPortWindowsNT_PowerPC     /* Windows NT Power PC */
#define ctPortid	0x33333333L
#endif

/* The following were added at the request of Yoshi */

#ifdef ctPortSolarisPowerPC     /* Solaris Power PC */
#define ctPortid	0x33444433L
#endif

#ifdef ctPort386BSD     /* 386 BSD UNIX */
#define ctPortid	0x33555533L
#endif

/* Please use 		0x33666633L next and change this comment */

#endif /* ctSYSIH */

/* end of ctsysi.h */
