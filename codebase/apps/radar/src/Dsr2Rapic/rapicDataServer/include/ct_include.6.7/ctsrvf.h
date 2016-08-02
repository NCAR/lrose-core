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

#ifndef ctSRVFH
#define ctSRVFH

#ifdef ctLOCLIB

#ifndef ctFRCSNG
#define cttseg		loccttseg
#define ctuseg		locctuseg
#endif

#define putdodan	srv_putdodan
#define putdodas	srv_putdodas
#define cptifil		srv_cptifil
#define ctadjadr	srv_ctadjadr
#define ctadjfld	srv_ctadjfld
#define makifil		srv_makifil
#define ctgetseginfo	srv_ctgetseginfo
#define cthdrcfld	srv_cthdrcfld
#define ctlfsegget	srv_ctlfsegget
#define ctlfsegput	srv_ctlfsegput
#define ctsysint	srv_ctsysint
#define cttime		srv_cttime
#define ierr		srv_ierr
#define vtclose		srv_vtclose
#define ctasskey	srv_ctasskey

#endif /* ctLOCLIB */

#endif /* ctSRVFH */
