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
/***********************************************************************
 * Public time series API for RVP8 from
 *
 *          SIGMET INCORPORATED, WESTFORD MASSACHUSETTS, U.S.A.
 */

#ifndef _RVP8_TS_API_H
#define _RVP8_TS_API_H

#define DSPTASKNAMELEN 16  
#define DSPSITENAMELEN 16  
#define USERMASKLEN 512
#define MAX_CHANNELS 2

#include <dataport/port_types.h>

typedef struct {

  ui16 iSweep;
  ui16 iAuxNum;
  char sTaskName[1+DSPTASKNAMELEN];
  ui08 iScanType;
  char pad22x2[2];

} rvp8_task_id_t;

typedef struct {

  ui08 iVersion;
  ui08 iMajorMode;
  ui08 iPolarization;
  ui08 iPhaseModSeq;

  rvp8_task_id_t taskID;

  char sSiteName[1+DSPSITENAMELEN];

  ui08 iAqMode;
  ui08 iUnfoldMode;

  ui08 iPWidthCode;
  fl32  fPWidthUSec;

  fl32  fDBzCalib;
  si16 iSampleSize;
  ui16  iMeanAngleSync;

  ui32 iFlags;
  si16 iPlaybackVersion;

  char pad66x38[38];

  fl32 fGdrOffset;
  fl32 fXdrOffset;

  fl32 fSyClkMHz;
  fl32 fWavelengthCM;
  fl32 fSaturationDBM;

  fl32 fRangeMaskRes;
  ui16 iRangeMask[USERMASKLEN];

  fl32 fNoiseDBm[MAX_CHANNELS];
  fl32 fNoiseStdvDB[MAX_CHANNELS];
  fl32 fNoiseRangeKM;
  fl32 fNoisePRFHz;

  ui16 iGparmLatchSts[2];
  ui16 iGparmImmedSts[6];
  ui16 iGparmDiagBits[4];

  char sVersionString[12];
  char pad1212x1188[1188];

} rvp8_ops_info_t;

typedef struct {

  si32 iDataOff;
  fl32  fBurstMag;
  ui16  iBurstArg;
  ui08 iWrapIQ;
  char  pad11x9[9];

} rvp8_pulse_hdr_rx;

typedef struct {
  ui32 iLong[2];
} uiqbits_t;

typedef struct {

  ui08 iVersion;
  ui08 iFlags;
  ui16 iMSecUTC;
  ui32 iTimeUTC;

  ui32 iBtimeAPI;
  ui32 iSysTime;
  ui32 iPrevPRT, iNextPRT;

  ui32 iSeqNum;
  ui08 iAqMode;

  ui08 iPolarBits;
  ui16 iTxPhase;

  ui32 iNanoUTC;

  char pad36x4[4];

  ui16 iPedAz, iPedEl;
  ui16 iAzV, iElV;
  ui16 iAz, iEl;

  si16 iNumVecs;
  si16 iMaxVecs;
  ui08 iVIQPerBin;

  ui08 iTgBank;
  ui16 iTgWave;

  uiqbits_t uiqPerm;
  uiqbits_t uiqOnce;

  rvp8_pulse_hdr_rx Rx[MAX_CHANNELS];

  char pad116x56[56];

} rvp8_pulse_hdr_t;

#endif

