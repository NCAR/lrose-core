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

#ifndef ctLFUNH
#define ctLFUNH


#define	Abort					TRANABT
#define AbortXtd				TRANABTX
#define	AddKey					ADDKEY
#define	AddRecord				ADDREC
#define	AddCtResource				ADDRES
#define	AddVRecord				ADDVREC
#define	AllocateBatch				ALCBAT
#define	AllocateSet				ALCSET
#define AvailableFileNbr			AVLFILNUM
#define	Begin					TRANBEG
#define	BuildKey				frmkey				
#define	ChangeBatch				CHGBAT
#define ChangeHistory				CHGHST
#define ChangeISAMContext			CHGICON
#define	ChangeSet				CHGSET
#define CleanIndexXtd				CLNIDXX
#define	ClearTranError				TRANCLR
#define	CloseCtFile				CLSFIL
#define	CloseIFile				CLIFIL
#define	CloseISAM				CLISAM
#define CloseISAMContext			CLSICON
#define	CloseRFile				CLRFIL
#define	Commit					TRANEND
#define CompactIFile				CMPIFIL
#define CompactIFileXtd				CMPIFILX
#define	CreateDataFile				CREDAT
#define	CreateDataFileXtd			CREDATX
#define	CreateIFile				CREIFIL
#define	CreateIFileXtd				CREIFILX
#define	CreateIndexFile				CREIDX
#define	CreateIndexFileXtd			CREIDXX
#define	CreateIndexMember			CREMEM
#define	CreateISAM				CREISAM
#define	CreateISAMXtd				CREISAMX
#define CtreeCheckPoint				CTCHKPNT
#define CtreeFlushFile				CTFLUSH
#define CtreeUserOperation			CTUSER
#define	CurrentFileOffset			GETCURP
#define	CurrentISAMKey				GETCURK
#define CurrentLowLevelKey			GETCURKL
#define	DeleteCtFile				DELFIL
#define DeleteIFile				DELIFIL
#define	DeleteKey				DELCHK
#define	DeleteKeyBlind				DELBLD
#define	DeleteRecord				DELREC
#define	DeleteCtResource			DELRES
#define DeleteRFile				DELRFIL
#define	DeleteVRecord				DELVREC
#define	DoBatch					BATSET
#define	EnableCtResource			ENARES
#define	EstimateKeySpan				ESTKEY
#define ExportIFile				EXPIFIL
#define ExportIFileXtd				EXPIFILX
#define	FirstInSet				FRSSET
#define	FirstInVSet				FRSVSET
#define	FirstKey				FRSKEY
#define	FirstRecord				FRSREC
#define	FirstVRecord				FRSVREC
#define	FreeBatch				FREBAT
#define FreeHistory				FREHST
#define	FreeSet					FRESET
#define	GetAltSequence				GETALTSEQ
#define GetConditionalIndexResource		GETCRES
#define GetConditionalIndex			GETCIDX
#define GetCtreePointer				GETCTREE
#define	GetDODA					GETDODA
#define	GetCtFileInfo				GETFIL
#define	GetGTEKey				GTEKEY
#define	GetGTERecord				GTEREC
#define	GetGTEVRecord				GTEVREC
#define	GetGTKey				GTKEY
#define	GetGTRecord				GTREC
#define	GetGTVRecord				GTVREC
#define	GetIFile				GETIFIL
#define	GetKey					EQLKEY
#define	GetLTEKey				LTEKEY
#define	GetLTERecord				LTEREC
#define	GetLTEVRecord				LTEVREC
#define	GetLTKey				LTKEY
#define	GetLTRecord				LTREC
#define	GetLTVRecord				LTVREC
#define GetORDKey				ORDKEY
#define	GetRecord				EQLREC
#define	GetVRecord				EQLVREC
#define	GetCtResource				GETRES
#define	GetSerialNbr				SERIALNUM
#define GetSuperFileNames			GETMNAME
#define	GetSymbolicNames			GETNAM
#define	GetCtTempFileName			TMPNAME
#define	InitCTree				INTREE
#define	InitCTreeXtd				INTREEX
#define	InitISAM				INTISAM
#define	InitISAMXtd				INTISAMX
#define	KeyAtPercentile				FRCKEY
#define	LastInSet				LSTSET
#define	LastInVSet				LSTVSET
#define	LastKey					LSTKEY
#define	LastRecord				LSTREC
#define	LastVRecord				LSTVREC
#define	LoadKey					LOADKEY
#define	LockCtData				LOKREC
#define	LockISAM				LKISAM
#define	NewData					NEWREC
#define	NewVData				NEWVREC
#define NextCtree				NXTCTREE
#define	NextInSet				NXTSET
#define	NextInVSet				NXTVSET
#define	NextKey					NXTKEY
#define	NextRecord				NXTREC
#define	NextVRecord				NXTVREC
#define	NbrOfKeyEntries				IDXENT
#define	NbrOfRecords				DATENT
#define NbrOfKeysInRange			RNGENT
#define	OpenCtFile				OPNFIL
#define	OpenCtFileXtd				OPNFILX
#define	OpenIFile				OPNIFIL
#define	OpenIFileXtd				OPNIFILX
#define	OpenISAM				OPNISAM
#define OpenISAMContext				OPNICON
#define	OpenISAMXtd				OPNISAMX
#define	OpenFileWithResource			OPNRFIL
#define	OpenFileWithResourceXtd			OPNRFILX
#define PermIIndex				PRMIIDX
#define	PositionSet				MIDSET
#define	PositionVSet				MIDVSET
#define	PreviousInSet				PRVSET
#define	PreviousInVSet				PRVVSET
#define	PreviousKey				PRVKEY
#define	PreviousRecord				PRVREC
#define	PreviousVRecord				PRVVREC
#define PutConditionalIndexResource		PUTCRES
#define	PutDODA					PUTDODA
#define PutIFile				PUTIFIL
#define PutIFileXtd				PUTIFILX
#define	ReadData				REDREC
#define ReadIsamData				REDIREC
#define ReadIsamVData				REDIVREC
#define	ReadVData				RDVREC
#define RebuildIIndex				RBLIIDX
#define	RebuildIFile				RBLIFIL
#define	RebuildIFileXtd				RBLIFILX
#define RegisterCtree				REGCTREE
#define	ReleaseData				RETREC
#define	ReleaseVData				RETVREC
#define RenameFile				ctRENFIL
#define	ReplaceSavePoint			SPCLSAV				
#define	ReReadRecord				RRDREC
#define	ReReadVRecord				REDVREC
#define	ResetRecord				UPDCURI
#define	RestoreSavePoint			TRANRST
#define	ReWriteRecord				RWTREC
#define	ReWriteVRecord				RWTVREC
#define	Security				SECURITY
#define	SetAlternateSequence			SETALTSEQ
#define SetFilter				SETFLTR
#define SetNodeName				SETNODE
#define SetOperationState			SETOPS
#define	SetRecord				SETCURI
#define	SetSavePoint				TRANSAV
#define	SetVariableBytes			SETVARBYTS
#define StopServer				STPSRV
#ifndef sqlSTOP
#define	StopUser				STPUSR
#endif
#define SuperfilePrepassXtd			CTSBLDX
#define SwitchCtree				SWTCTREE
#define SystemMonitor				SYSMON
#define SystemConfiguration			SYSCFG
#define TempIIndexXtd				TMPIIDXX
#define TransactionHistory			CTHIST
#define	TransformKey				TFRMKEY
#define	TransformSegment			cttseg				
#define	VDataLength				GTVLEN
#define UnRegisterCtree				UNRCTREE
#define	UntransformSegment			ctuseg				
#define UpdateConditionalIndex			UPDCIDX
#define	UpdateFileMode				PUTFIL
#define UpdateHeader				PUTHDR
#define	UpdateCtResource			UPDRES
#define	VRecordLength				GETVLEN
#define WhichCtree				WCHCTREE
#define	WriteData				WRTREC
#define	WriteVData				WRTVREC
#endif /* ctLFUNH */

/* end of ctlfun.h */
