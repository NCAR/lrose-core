/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
#ifndef RDACSP_H
#define RDACSP_H

#define RDP_DEFAULT_PORT 14998

/* maximum message sizes */
/* these include RDP_HDR */

#define RDP_MAXTOSERVER 2500
#define RDP_MAXFROMSERVER 6500

#define RDP_MAX_STRIKE_PACKING 10

/* product codes */

#define PRODCODE_Log 0 /* log reflectivity */
#define PRODCODE_Lin 1 /* linear reflectivity */
#define PRODCODE_Vel 2 /* velocity */
#define PRODCODE_Turb 3 /* turbulence */
#define SPECPROD_Angle 50 /* angle info only (OffsetToData = 0) */

#define PRODCODE_EngLog 0x80 /* log reflectivity (in engineering units) */
#define PRODCODE_EngLin 0x81 /* linear reflectivity (eng units) */
#define PRODCODE_EngVel 0x82 /* velocity (eng units) */
#define PRODCODE_EngTurb 0x83 /* turbulence (eng units) */

#define PRODCODE_EngUnitsMask 0x80

/*
 * error codes in Status field of header
 */

#define STATUS_ASYNC 0x8000
#define RDPERR_NOERROR 0
#define RDPERR_ERROR 1 /* unknown */
#define RDPERR_PERMISSIONS 2
#define RDPERR_NOTCAPABLE 3
#define RDPERR_BADPARMS 4
#define RDPERR_LOGIN 5
#define RDPERR_NOTAVAILABLE 6

/* message ids */

#define ID_RDP_Login 0 /* (s) */
#define ID_RDP_SetSite 1 /* (s) */
#define ID_RDP_SetPgm 2 /* (s) */
#define ID_RDP_SetCfg 3 /* (s) */
#define ID_RDP_LoadCfg 4 /* (s) */
#define ID_RDP_SaveCfg 5 /* (s) */
#define ID_RDP_LoadMask 6 /* (s) */
#define ID_RDP_SaveMask 7 /* (s) */

#define ID_RDP_GetSite 8 /* (s/a) */
#define ID_RDP_GetCurPgm 9 /* (s/a) */
#define ID_RDP_GetCurStep 10 /* (s/a) */
#define ID_RDP_GetMainPgm 11 /* (s) */
#define ID_RDP_GetRayParms 12 /* (s/a) */
#define ID_RDP_GetRayData 13 /* (a) */
#define ID_RDP_SelectAsync 14 /* (s) */
#define ID_RDP_GetCfg 15 /* (s) */
#define ID_RDP_CfgNotify 16 /* (a) */
#define ID_RDP_Overflow 17 /* (a) */
#define ID_RDP_GetRawData 18 /* (a) */
#define ID_RDP_ClearMask 19 /* (s) */
#define ID_RDP_GetStrikeData 20 /* (a) */
#define ID_RDP_SetOutputs 21 /* (s) */
#define ID_RDP_GetInputs 22 /* (s/a) */

/*
 * select async delivery bits
 */

#define AS_LogParms 0x00000001L
#define AS_LinParms 0x00000002L
#define AS_VelParms 0x00000004L
#define AS_TurbParms 0x00000008L
#define AS_LogData 0x00000010L
#define AS_LinData 0x00000020L
#define AS_VelData 0x00000040L
#define AS_TurbData 0x00000080L

#define AS_Site 0x00000400L
#define AS_CurPgm 0x00000800L
#define AS_CurStep 0x00001000L
#define AS_RawLog 0x00002000L
#define AS_RawLin 0x00004000L
#define AS_CfgNotify 0x00008000L

#define AS_AngData 0x00010000L /* this is only sent if no other
				* product with ang info is sent
				* (including raw!) */

#define AS_EngData 0x00200000L /*this is a modidier that should
				* only be set if AS_xxxData is set
				* (xxx=Log|Lin|Vel|Turb). Level
				* table is ignored (still sent)
				* and data is in real-world units
				*/

#define AS_StrikeData 0x00400000L /* Lightning strike data */
#define AS_GetInputs 0x00800000L /* input bits (and user outputs) */

/*
 * compress codes (will probably not implement all!)
 */

#define COMPRESS_BYTENONE 0 /* data is byte per bin */
#define COMPRESS_BYTERUN 1  /* data is repeat, value byte pairs
			     * (0 in repeat means 256) */
#define COMPRESS_NIBNONE 2  /* data is nib per bin
			     * (the low order nibble comes first) */
#define COMPRESS_NIBRUN 3   /* low nib is value, upper nib is repeat
			     * (0 in repeat means 16) */

/*
 * radar codes
 */

#define RADAR_EEC0 0 /* enterprise 6 refl levels [9 vel/turb levels] */
#define RADAR_EEC1 1 /* enterprise 16 refl levels [? vel/turb levels] */
#define RADAR_RT701 2 /* collins (5 color levels) [16 vel levels] */
#define RADAR_BSI_HD 3 /* bsi HD (no doppler) 16 levs, up to 1024 bins */
#define RADAR_BSI_HDD 4 /* bsi HDD 16 refl/vel/turb levels */
#define RADAR_BSI_HDD90 5 /* new 90 kw doppler */
#define RADAR_BSI_HDD250 6 /* new 250 kw doppler */

/*
 * permissions
 */

#define PM_READ 0x0001
#define PM_CONTROL 0x0002
#define PM_CONFIG 0x0004
 
/***************************
 * Header for all messages
 ***************************/

typedef struct
{
  ui16 Length;  /* length of message (includes this header) */
  ui08 XA5;     /* magic number (0xa5) */
  ui08 Id;      /* message type */
  ui16 Status;  /* if from server: 0 indicates no error
		 * except 0x8000 indicates async data
		 * if from client, 0 unless otherwise specified */
} RDP_HDR;

/****************************
 * big messages
 ****************************/

typedef struct
{
  ui08 Body[RDP_MAXTOSERVER - sizeof(RDP_HDR)];
} RDP_ANY;

typedef struct
{
  ui08 Body[RDP_MAXFROMSERVER - sizeof(RDP_HDR)];
} RDP_ANY_R;

/****************************
 * RDP_Login
 ****************************/
/*
 * sync only
 */

typedef struct
{
  si08 User[32]; /* user name */
  si08 Password[32]; /* password (in the clear) */
} RDP_Login;

typedef struct
{
  ui16 Permissions;
} RDP_Login_R;

/****************************
 * RDP_SetSite
 ****************************/
/*
 * sync only
 */

#ifdef NOTNOW
typedef struct
{
} RDP_SetSite;
#endif

#ifdef NOTNOW
typedef struct
{
} RDP_SetSite_R;
#endif

typedef struct
{
  ui16 padding;
  ui16 VersionNum; /* 102 == ver 1.02 */
  ui16 ProtocolNum; /* 100 == 1.00 */
  si08 SiteName[32]; /* up to 31 char name of site, 0 terminated */
  si08 RadarName[32]; /* eg. "BSI HDD-250" */
  ui16 RadarType; /* RADAR_xxx code */
  ui32 Capabilities; /* RCAP_xxx bits */
  si32 Latitude; /* scaled degs (1000000 == 1 deg) */
  si32 Longitude; /* (these signed per convention) */
  si32 Altitude_AAT; /* meters above average terrain */
  si32 Altitude_ASL; /* meters above sea level */
  ui16 Angle360; /* typically 16384 to indicate 14 bit angle
		 * converter (if not Rdacs simulates it) */
} SITECFG;


/****************************
 * RDP_SetPgm
 ****************************/
/*
 * sync only
 */

typedef struct
{
  ui16 Flags; /* Bit 0: set if interrupt program */
} RDP_SetPgm;

#ifdef NOTNOW
typedef struct
{
} RDP_SetPgm_R;
#endif

/****************************
 * RDP_SetCfg
 ****************************/
/*
 * sync only
 */

#ifdef NOTNOW
typedef struct
{
} RDP_SetCfg;
#endif


#ifdef NOTNOW
typedef struct
{
} RDP_SetCfg_R;
#endif

/****************************
 * RDP_LoadCfg
 ****************************/
/*
 * sync only
 */

typedef struct
{
  ui16 CfgNum;
} RDP_LoadCfg;

#ifdef NOTNOW
typedef struct
{
} RDP_LoadCfg_R;
#endif

/****************************
 * RDP_SaveCfg
 ****************************/
/*
 * sync only
 */

typedef struct
{
  ui16 CfgNum;
} RDP_SaveCfg;

#ifdef NOTNOW
typedef struct
{
} RDP_SaveCfg_R;
#endif

/****************************
 * RDP_LoadMask
 ****************************/
/*
 * sync only
 */

typedef struct
{
  ui16 MaskNum;
} RDP_LoadMask;

#ifdef NOTNOW
typedef struct
{
} RDP_LoadMask_R;
#endif

/****************************
 * RDP_SaveMask
 ****************************/
/*
 * sync only
 */

typedef struct
{
  ui16 MaskNum;
} RDP_SaveMask;

#ifdef NOTNOW
typedef struct
{
} RDP_SaveMask_R;
#endif

/****************************
 * RDP_GetSite
 ****************************/
/*
 * sync or async
 */


#ifdef NOTNOW
typedef struct
{
} RDP_GetSite;
#endif


#ifdef NOTNOW
typedef struct
{
} RDP_GetSite_R;
#endif



/****************************
 * RDP_GetCurPgm
 ****************************/
/*
 * sync or async
 */

#ifdef NOTNOW
typedef struct
{
} RDP_GetCurPgm;
#endif

typedef struct
{
  ui16 Flags; /* Bit 0: set if interrupt program */
} RDP_GetCurPgm_R;

/****************************
 * RDP_GetCurStep
 ****************************/
/*
 * sync or async
 */

#ifdef NOTNOW
typedef struct
{
} RDP_GetCurStep;
#endif

typedef struct
{
  ui16 Flags; /* Bit 0: set if interrupt program */
} RDP_GetCurStep_R;

/****************************
 * RDP_GetMainPgm
 ****************************/
/*
 * sync only
 */

#ifdef NOTNOW
typedef struct
{
} RDP_GetMainPgm;
#endif

typedef struct
{
  ui16 Flags; /* Bit 0: 0 */
} RDP_GetMainPgm_R;

/****************************
 * RDP_GetRayParms
 ****************************/
/*
 * sync or async
 */

typedef struct
{
  ui16 ProductCode;
} RDP_GetRayParms;

typedef struct
{
  ui16 padding;     /* for alignment */
  ui16 ProductCode; /* PRODCODE_xxx */
  ui16 NumberBins; /* Number of samples */
  ui16 NumberLevels; /* Number of data levels in Data[] */
  /* (eg 16 implies range 0 to 15) */
  ui32 SkipTime; /* first sample delay time nanosecs */
  ui32 BinTime; /* sample separate time nanosecs */
  ui16 ThresholdSize; /* in words (typical NumberLevels-1) */
  ui16 ThresholdOffset;/* 0 if none */
} RDP_GetRayParms_R;

/****************************
 * RDP_GetRayData
 ****************************/
/*
 * async only
 */
/* Note, the packet may be padded to be even number of bytes
 * so get number of bins from RDP_GetRayParms_R and stop
 */

typedef struct
{
  ui32 TimeMs; /* milliseconds since connect */
  ui08 ProductCode; /* PRODCODE_xxx */
  ui08 CompressType; /* COMPRESS_xxx */
  ui16 Azimuth; /* azimuth counter */
  ui16 Elevation; /* elevation counter */
  ui16 OffsetToData; /* provision for change to struct */

} RDP_GetRayData_R;

/****************************
 * RDP_SelectAsync
 ****************************/
/*
 * sync only
 */

typedef struct
{
  ui32 SelectBits; /* AS_xxx */
  ui16 RawPacing;  /* pacing value if AS_RawXxx or AS_AngData 
		   * or AS_EngData selected */
} RDP_SelectAsync;

#ifdef NOTNOW
typedef struct
{
} RDP_SelectAsync_R;
#endif

/****************************
 * RDP_GetCfg
 ****************************/
/*
 * sync only
 */

#ifdef NOTNOW
typedef struct
{
} RDP_GetCfg;
#endif

#ifdef NOTNOW
typedef struct
{
} RDP_GetCfg_R;
#endif


/****************************
 * RDP_CfgNotify
 ****************************/
/*
 * async only
 */

#ifdef NOTNOW
typedef struct
{
} RDP_CfgNotify_R;
#endif

/****************************
 * RDP_Overflow
 ****************************/
/*
 * async only
 *
 * Selected automatically when async data products selected.
 * Sent if product data was discarded because of clogged pipe.
 * Only type RDP_GetRayData_R will be discarded.
 */

typedef struct
{
  ui16 LostCount; /* number of ProductData's lost */
} RDP_Overflow_R;



/****************************
* RDP_GetRawData
****************************/
/*
 * async only
 */
typedef struct
{
  ui16 Format; /* depends on server type */
} RDP_GetRawData_R;

/****************************
* RDP_ClearMask
***************************/
/*
 * sync only
 */

/* for hdd, MaskId is 0 to 4 for log, 0x10 to 0x14 for linear */

typedef struct
{
  ui16 MaskId;
} RDP_ClearMask;

#ifdef NOTNOW
typedef struct
{
} RDP_ClearMask_R;
#endif

/****************************
 * RDP_GetStrikeData
 ****************************/
/*
 * async only
 */
typedef struct
{
  si32 Time; /* UTC from 1/1/70 */
  ui16 Millisec; /* / fraction of second */
  si32 Latitude; /* degrees x1000 scaled int */
  si32 Longitude; /* degrees x1000 scaled int */
  si32 Strength; /* x10 scaled integer */
  ui16 Multiplicity; /* si16 Angle; */
  si16 SemiMajorAxis; /* / x10 scaled integer */
  si16 Eccentricity; /* / x10 scaled integer */
  ui16 ChiSquare; /* / */
} RDP_STRIKE;

/* / == valid only if Format == 1 */

typedef struct
{
  ui16 Format; /* 0/1 = extended fields not_valid/valid, */
  ui16 Count; /* num strikes max RDP_MAX_STRIKE_PACKING */

} RDP_GetStrikeData_R;




/****************************
 * RDP_SetOutputs
 ****************************/
/*
 * sync only
 */

typedef struct
{
  ui32 PulseTime_ms; /* 0 for level change */
  ui32 ChangeMask[2]; /* bits to change or pulse */
  ui32 Outputs[2]; /* [0] = Sysout, [1] = user outputs */
} RDP_SetOutputs;

#ifdef NOTNOW
typedef struct
{
} RDP_SetOutputs_R;
#endif

/****************************
 * RDP_GetInputs
 ****************************/
/*
 * sync or async
 */

/* Note, when delivered async, it will only be sent when
 * inputs or outputs[1] change (not when outputs[0] changes)
 */

#ifdef NOTNOW
typedef struct
{
} RDP_GetInputs;
#endif

#define SYSIN_MAINPWR 0x00000001
#define SYSIN_SERVOPWR 0x00000002
#define SYSIN_MODPWR 0x00000004
#define SYSIN_WARMUP 0x00000008
#define SYSIN_STANDBY 0x00000010
#define SYSIN_OPERATE 0x00000020
#define SYSIN_ALM_ANY 0x00000040
#define SYSIN_ALM_MOD 0x00000080
#define SYSIN_ALM_PRF 0x00000100
#define SYSIN_ALM_STALO 0x00000200
#define SYSIN_ALM_COHO 0x00000400
#define SYSIN_ALM_AFC 0x00000800
#define SYSIN_MANUAL_CTL 0x00001000


typedef struct
{
  ui32 Inputs[2]; /* [0] = Sysin, [1] = user inputs */
  ui32 Outputs[2]; /* [0] = Sysout, [1] = user outputs */
} RDP_GetInputs_R;


#endif /* RDACSP_H */


