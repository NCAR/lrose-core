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

#ifndef ctLOCFH
#define ctLOCFH

#ifdef ctFRCSNG
#ifdef ctPOSTFNC
#define frmkey		intfrmkey
#define TFRMKEY		intTFRMKEY
#endif
#else /* ~ctFRCSNG */
#define STPSRV		locSTPSRV
#define TFRMKEY		locTFRMKEY
#define frmkey		locfrmkey
#define cttseg		loccttseg
#define ctuseg		locctuseg
#endif /* ~ctFRCSNG */

#define CTCHKPNT	locCTCHKPNT
#define CTFLUSH		locCTFLUSH
#define OPNICON		locOPNICON
#define CHGICON		locCHGICON
#define CLSICON		locCLSICON
#define TRANABT		locTRANABT
#define TRANABTX	locTRANABTX
#ifdef ctCONDIDX
#define UPDCIDX		locUPDCIDX
#endif
#define ADDKEY		locADDKEY
#define ADDREC		locADDREC
#define ADDRES		locADDRES
#define ADDVREC		locADDVREC
#define ALCBAT		locALCBAT
#define ALCSET		locALCSET
#define AVLFILNUM	locAVLFILNUM
#define TRANBEG		locTRANBEG
#define CHGBAT		locCHGBAT
#define CHGHST		locCHGHST
#define CHGSET		locCHGSET
#define CLNIDXX		locCLNIDXX
#define TRANCLR		locTRANCLR
#define CLSFIL		locCLSFIL
#define CLIFIL		locCLIFIL
#define CLISAM		locCLISAM
#define CLRFIL		locCLRFIL
#define CMPIFIL		locCMPIFIL
#define CMPIFILX	locCMPIFILX
#define TRANEND		locTRANEND
#define CREDAT		locCREDAT
#define CREDATX		locCREDATX
#define CREIFIL		locCREIFIL
#define CREIFILX	locCREIFILX
#define CREIDX		locCREIDX
#define CREIDXX		locCREIDXX
#define CREMEM		locCREMEM
#define CREISAM		locCREISAM
#define CREISAMX	locCREISAMX
#define CTHIST		locCTHIST
#define GETCURP		locGETCURP
#define GETCURK		locGETCURK
#define GETCURKL	locGETCURKL
#define CTUSER		locCTUSER
#define DELFIL		locDELFIL
#define DELIFIL		locDELIFIL
#define DELCHK		locDELCHK
#define DELBLD		locDELBLD
#define DELREC		locDELREC
#define DELRES		locDELRES
#define DELRFIL		locDELRFIL
#define DELVREC		locDELVREC
#define BATSET		locBATSET
#define ENARES		locENARES
#define ESTKEY		locESTKEY
#define EXPIFIL		locEXPIFIL
#define EXPIFILX	locEXPIFILX
#define FRSSET		locFRSSET
#define FRSKEY		locFRSKEY
#define FRSREC		locFRSREC
#define FREBAT		locFREBAT
#define FREHST		locFREHST
#define FRESET		locFRESET
#define GETALTSEQ	locGETALTSEQ
#ifdef ctCONDIDX
#define GETCRES		locGETCRES
#define GETCIDX		locGETCIDX
#endif
#define GETDODA		locGETDODA
#define GETFIL		locGETFIL
#define GTEKEY		locGTEKEY
#define GTEREC		locGTEREC
#define GTKEY		locGTKEY
#define GTREC		locGTREC
#define GETIFIL		locGETIFIL
#define EQLKEY		locEQLKEY
#define LTEKEY		locLTEKEY
#define LTEREC		locLTEREC
#define LTKEY		locLTKEY
#define LTREC		locLTREC
#define ORDKEY		locORDKEY
#define EQLREC		locEQLREC
#define GETRES		locGETRES
#define SERIALNUM	locSERIALNUM
#define GETMNAME	locGETMNAME
#define GETNAM		locGETNAM
#define TMPNAME		locTMPNAME
#define INTREE		locINTREE
#define INTREEX		locINTREEX
#define INTISAM		locINTISAM
#define INTISAMX	locINTISAMX
#define FRCKEY		locFRCKEY
#define LSTSET		locLSTSET
#define LSTKEY		locLSTKEY
#define LSTREC		locLSTREC
#define LOADKEY		locLOADKEY
#define LOKREC		locLOKREC
#define LKISAM		locLKISAM
#define NEWREC		locNEWREC
#define NEWVREC		locNEWVREC
#define NXTSET		locNXTSET
#define NXTKEY		locNXTKEY
#define NXTREC		locNXTREC
#define IDXENT		locIDXENT
#define DATENT		locDATENT
#define RNGENT		locRNGENT
#define OPNFIL		locOPNFIL
#define OPNFILX		locOPNFILX
#define OPNIFIL		locOPNIFIL
#define OPNIFILX	locOPNIFILX
#define OPNISAM		locOPNISAM
#define OPNISAMX	locOPNISAMX
#define OPNRFIL		locOPNRFIL
#define OPNRFILX	locOPNRFILX
#define PRMIIDX		locPRMIIDX
#define MIDSET		locMIDSET
#define PRVSET		locPRVSET
#define PRVKEY		locPRVKEY
#define PRVREC		locPRVREC
#ifdef ctCONDIDX
#define PUTCRES		locPUTCRES
#endif
#define PUTDODA		locPUTDODA
#define PUTIFIL		locPUTIFIL
#define PUTIFILX	locPUTIFILX
#define REDREC		locREDREC
#define REDIREC		locREDIREC
#define REDIVREC	locREDIVREC
#define RDVREC		locRDVREC
#define RBLIIDX		locRBLIIDX
#define RBLIFIL		locRBLIFIL
#define RBLIFILX	locRBLIFILX
#define RETREC		locRETREC
#define RETVREC		locRETVREC
#define SPCLSAV		locSPCLSAV
#define RRDREC		locRRDREC
#define REDVREC		locREDVREC
#define ctRENFIL	locctRENFIL
#define UPDCURI		locUPDCURI
#define TRANBAK		locTRANBAK
#define TRANRST		locTRANRST
#define RWTREC		locRWTREC
#define RWTVREC		locRWTVREC
#define SECURITY	locSECURITY
#define SETALTSEQ	locSETALTSEQ
#define SETCURI		locSETCURI
#define SETFLTR		locSETFLTR
#define SETNODE		locSETNODE
#define SETOPS		locSETOPS
#define TRANSAV		locTRANSAV
#define SETVARBYTS	locSETVARBYTS
#ifndef sqlSTOP
#define STPUSR		locSTPUSR
#endif
#define SYSMON		locSYSMON
#define CTSBLDX		locCTSBLDX
#define TMPIIDXX	locTMPIIDXX
#define GTVLEN		locGTVLEN
#define PUTFIL		locPUTFIL
#define UPDRES		locUPDRES
#define GETVLEN		locGETVLEN
#define WRTREC		locWRTREC
#define WRTVREC		locWRTVREC

#define ctdidx		locctdidx
#define SETDEFBLK	locSETDEFBLK
#define setfndval	locsetfndval
#define TSTVREC		locTSTVREC
#define cttestfunc	loccttestfunc
#define IOPERFORMANCE	locIOPERFORMANCE
#define IOPERFORMANCEX	locIOPERFORMANCEX
#define reset_cur	locreset_cur
#define reset_cur2	locreset_cur2
#define SYSCFG		locSYSCFG
#define PUTHDR		locPUTHDR

#define EQLVREC		locEQLVREC
#define FRSVREC		locFRSVREC
#define NXTVREC		locNXTVREC
#define PRVVREC		locPRVVREC
#define LSTVREC		locLSTVREC
#define GTEVREC		locGTEVREC
#define GTVREC		locGTVREC
#define LTEVREC		locLTEVREC
#define LTVREC		locLTVREC
#define FRSVSET		locFRSVSET
#define NXTVSET		locNXTVSET
#define PRVVSET		locPRVVSET
#define LSTVSET		locLSTVSET
#define MIDVSET		locMIDVSET
#endif /* ctLOCFH */

/* end of ctlocf.h */
