// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////
//
// header for UDP data structures
// in Sigmet realtime display version 2
//
///////////////////////////////////////

#ifndef _data_hh_included__
#define _data_hh_included__

#include <dataport/port_types.h>

#define N_MOMENTS_MAX 10

#define UDP_SIZE (0x07ff)
#define UDP_TYPE (~UDP_SIZE)

#define POLARIZATION_H    (0)
#define POLARIZATION_V    (1)
#define POLARIZATION_ALT  (2)
#define POLARIZATION_SIM  (3)

#define FIELD_XHDR          (0)
#define FIELD_DBT           (1)
#define FIELD_DBZ           (2)
#define FIELD_VEL           (3)
#define FIELD_WIDTH         (4)
#define FIELD_ZDR           (5)
#define FIELD_ORAIN         (6)
#define FIELD_DBZC          (7)
#define FIELD_DBT2          (8)
#define FIELD_DBZ2          (9)
#define FIELD_VEL2         (10)
#define FIELD_WIDTH2       (11)
#define FIELD_ZDR2         (12)
#define FIELD_RRATE2       (13)
#define FIELD_KDP          (14)
#define FIELD_KDP2         (15)
#define FIELD_PHIDP        (16)
#define FIELD_VELC         (17)
#define FIELD_SQI          (18)
#define FIELD_RHOHV        (19)
#define FIELD_RHOHV2       (20)
#define FIELD_DBZC2        (21)
#define FIELD_VELC2        (22)
#define FIELD_SQI2         (23)
#define FIELD_PHIDP2       (24)
#define FIELD_LDRH         (25)
#define FIELD_LDRH2        (26)
#define FIELD_LDRV         (27)
#define FIELD_LDRV2        (28)
#define FIELD_FLAGS        (29)
#define FIELD_FLAGS2       (30)
#define FIELD_FLOAT32      (31)
#define FIELD_HEIGHT       (32)
#define FIELD_VIL2         (33)
#define FIELD_NULL         (34)
#define FIELD_SHEAR        (35)
#define FIELD_DIVERGE2     (36)
#define FIELD_FLIQUID2     (37)
#define FIELD_USER         (38)
#define FIELD_OTHER        (39)
#define FIELD_DEFORM2      (40)
#define FIELD_VVEL2        (41)
#define FIELD_HVEL2        (42)
#define FIELD_HDIR2        (43)
#define FIELD_AXDIL2       (44)
#define FIELD_TIME2        (45)
#define FIELD_RHOH         (46)
#define FIELD_RHOH2        (47)
#define FIELD_RHOV         (48)
#define FIELD_RHOV2        (49)
#define FIELD_PHIH         (50)
#define FIELD_PHIH2        (51)
#define FIELD_PHIV         (52)
#define FIELD_PHIV2        (53)
#define FIELD_USER2        (54)
#define FIELD_HCLASS       (55)
#define FIELD_HCLASS2      (56)
#define FIELD_ZDRC         (57)
#define FIELD_ZDRC2        (58)
#define FIELD_TEMP16       (59)
#define FIELD_VIR16        (60)
#define FIELD_DBTV8        (61)
#define FIELD_DBTV16       (62)
#define FIELD_DBZV8        (63)
#define FIELD_DBZV16       (64)
#define FIELD_SNR8         (65)
#define FIELD_SNR16        (66)
#define FIELD_ALBEDO8      (67)
#define FIELD_ALBEDO16     (68)
#define FIELD_VILD16       (69)
#define FIELD_TURB16       (70)

typedef struct ymds_time
{
  ui32 sec;
  ui16 millsec;
  ui16 year, month, day;
} udp_time_t;

typedef struct volume_header {
  ui16 packetCode;
  ui16 polType;
  char siteNameShort[4]; // null terminated
  char siteNameLong[20]; // null terminated
  ui32 latAsInt;
  ui32 lonAsInt;
  udp_time_t sweepStartTime;
  si32 rangeFirstBinCm;
  ui32 binSpacingCm;
  si32 nBins;
  ui16 nBinsPacket;
  si16 nMoments;
  ui16 momData[N_MOMENTS_MAX];
  ui32 dualPrtScheme;
  si32 prf;
  si32 wavelengthCmOver100;
  char taskName[16]; // null terminated
  ui08 displayId;
  char spare[3];
} volume_header_t;

typedef struct beam_header {
  ui16 packetCode;
  ui16 packetSeqNum;
  char siteNameShort[4]; // null terminated
  ui16 startAz;
  ui16 endAz;
  ui16 endElev;
  ui16 timeOffsetSecs;
  ui08 momDataIndex;
  ui08 origin0RangeIndex;
  ui16 momDataSize;
  ui16 nBinsBeam;
  ui08 displayId;
  ui08 momData[1]; // pointer into data
} beam_header_t;

#endif
