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
/*********************************************
 * ds_radar_ts.c
 *
 * C routines for ds_radar_ts struct
 *
 * Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
 *
 * Sept 2007
 */

#include <dataport/bigend.h>
#include <rapformats/ds_radar_ts.h>
#include <toolsa/mem.h>

/***************************
 * initialize pulse header
 */

void ts_pulse_hdr_init(ts_pulse_hdr_t *pulse)

{
  
  memset(pulse, 0, sizeof(ts_pulse_hdr_t));

  pulse->version = DS_RADAR_TS_MISSING;
  pulse->radarId = DS_RADAR_TS_MISSING;
  pulse->elevation = DS_RADAR_TS_MISSING;
  pulse->azimuth = DS_RADAR_TS_MISSING;
  pulse->prt = DS_RADAR_TS_MISSING;
  pulse->prtNext = DS_RADAR_TS_MISSING;
  pulse->pulseWidth = DS_RADAR_TS_MISSING;
  pulse->burstMag[0] = DS_RADAR_TS_MISSING;
  pulse->burstMag[1] = DS_RADAR_TS_MISSING;
  pulse->burstArg[0] = DS_RADAR_TS_MISSING;
  pulse->burstArg[1] = DS_RADAR_TS_MISSING;
  pulse->burstArgDiff[0] = DS_RADAR_TS_MISSING;
  pulse->burstArgDiff[1] = DS_RADAR_TS_MISSING;
  pulse->measXmitPowerDbmH = DS_RADAR_TS_MISSING;
  pulse->measXmitPowerDbmV = DS_RADAR_TS_MISSING;
  pulse->scale = DS_RADAR_TS_MISSING;
  pulse->bias = DS_RADAR_TS_MISSING;

}

/***************************
 * initialize ops info
 */

void ts_ops_info_init(ts_ops_info_t *info)

{
  
  memset(info, 0, sizeof(ts_ops_info_t));

  info->version = DS_RADAR_TS_MISSING;
  info->radarId = DS_RADAR_TS_MISSING;
  info->altitudeM = DS_RADAR_TS_MISSING;
  info->latitudeDeg = DS_RADAR_TS_MISSING;
  info->longitudeDeg = DS_RADAR_TS_MISSING;
  info->startRangeM = DS_RADAR_TS_MISSING;
  info->gateSpacingM = DS_RADAR_TS_MISSING;
  info->scanMode = DS_RADAR_TS_MISSING;
  info->xmitRcvMode = DS_RADAR_TS_MISSING;
  info->prfMode = DS_RADAR_TS_MISSING;
  info->xmitPhaseMode = DS_RADAR_TS_MISSING;
  info->wavelengthCm = DS_RADAR_TS_MISSING;
  info->beamWidthDegH = DS_RADAR_TS_MISSING;
  info->beamWidthDegV = DS_RADAR_TS_MISSING;
  info->targetEl = DS_RADAR_TS_MISSING;
  info->targetAz = DS_RADAR_TS_MISSING;
  info->sectorAz1 = DS_RADAR_TS_MISSING;
  info->sectorAz2 = DS_RADAR_TS_MISSING;
  info->rhiEl1 = DS_RADAR_TS_MISSING;
  info->rhiEl2 = DS_RADAR_TS_MISSING;
  
  sprintf(info->radarName, "unknown");
  sprintf(info->siteName, "unknown");

  ds_radar_calib_init(&info->calib);
  
}

/****************************
 * print pulse header
 */

void ts_pulse_hdr_print(FILE *out, const char *spacer,
                        ts_pulse_hdr_t *pulse)

{
  
  fprintf(out, "\n");
  fprintf(out, "==================== ts_pulse_hdr ==================\n");
  fprintf(out, "%s version: %d\n", spacer, pulse->version);
  fprintf(out, "%s radarId: %d\n", spacer, pulse->radarId);
  fprintf(out, "%s pulseSeqNum: %d\n", spacer, pulse->pulseSeqNum);
  fprintf(out, "%s timeSecsUTC: %d\n", spacer, pulse->timeSecsUTC);
  fprintf(out, "%s timeNanoSecs: %d\n", spacer, pulse->timeNanoSecs);
  fprintf(out, "%s elevation: %g\n", spacer, pulse->elevation);
  fprintf(out, "%s azimuth: %g\n", spacer, pulse->azimuth);
  fprintf(out, "%s prt: %g\n", spacer, pulse->prt);
  fprintf(out, "%s prtNext: %g\n", spacer, pulse->prtNext);
  fprintf(out, "%s pulseWidth: %g\n", spacer, pulse->pulseWidth);
  fprintf(out, "%s nGates: %d\n", spacer, pulse->nGates);
  fprintf(out, "%s tiltNum: %d\n", spacer, pulse->tiltNum);
  fprintf(out, "%s volNum: %d\n", spacer, pulse->volNum);
  fprintf(out, "%s nChannels: %d\n", spacer, pulse->nChannels);
  fprintf(out, "%s iqEncoding: %d\n", spacer, pulse->iqEncoding);
  fprintf(out, "%s hvFlag: %d\n", spacer, pulse->hvFlag);
  fprintf(out, "%s antennaTransition: %d\n", spacer, pulse->antennaTransition);
  fprintf(out, "%s phaseCohered: %d\n", spacer, pulse->phaseCohered);
  fprintf(out, "%s status: %d\n", spacer, pulse->status);
  fprintf(out, "%s nData: %d\n", spacer, pulse->nData);
  fprintf(out, "%s iqOffset[0]: %d\n", spacer, pulse->iqOffset[0]);
  fprintf(out, "%s iqOffset[1]: %d\n", spacer, pulse->iqOffset[1]);
  fprintf(out, "%s burstMag[0]: %g\n", spacer, pulse->burstMag[0]);
  fprintf(out, "%s burstMag[1]: %g\n", spacer, pulse->burstMag[1]);
  fprintf(out, "%s burstArg[0]: %g\n", spacer, pulse->burstArg[0]);
  fprintf(out, "%s burstArg[1]: %g\n", spacer, pulse->burstArg[1]);
  fprintf(out, "%s burstArgDiff[0]: %g\n", spacer, pulse->burstArgDiff[0]);
  fprintf(out, "%s burstArgDiff[1]: %g\n", spacer, pulse->burstArgDiff[1]);
  fprintf(out, "%s measXmitPowerDbmH: %g\n", spacer, pulse->measXmitPowerDbmH);
  fprintf(out, "%s measXmitPowerDbmV: %g\n", spacer, pulse->measXmitPowerDbmV);
  fprintf(out, "%s scale: %g\n", spacer, pulse->scale);
  fprintf(out, "%s bias: %g\n", spacer, pulse->bias);
  fprintf(out, "%s nGatesBurst: %d\n", spacer, pulse->nGatesBurst);

  fprintf(out, "-------------------- RVP8 section ------------------\n");
  fprintf(out, "%s iVersion: %d\n", spacer, (int) pulse->version);
  fprintf(out, "%s iFlags: %d\n", spacer, (int) pulse->rvp8.iFlags);
  fprintf(out, "%s iMSecUTC: %d\n", spacer, (int) pulse->timeNanoSecs / 1000000);
  fprintf(out, "%s iTimeUTC: %d\n", spacer, (int) pulse->timeSecsUTC);
  fprintf(out, "%s iBtimeAPI: %d\n", spacer, (int) pulse->rvp8.iBtimeAPI);
  fprintf(out, "%s iSysTime: %d\n", spacer, (int) pulse->rvp8.iSysTime);
  fprintf(out, "%s iPrevPRT: %d\n", spacer, (int) pulse->rvp8.iPrevPRT);
  fprintf(out, "%s iNextPRT: %d\n", spacer, (int) pulse->rvp8.iNextPRT);
  fprintf(out, "%s iSeqNum: %d\n", spacer, (int) pulse->pulseSeqNum);
  fprintf(out, "%s iAqMode: %d\n", spacer, (int) pulse->rvp8.iAqMode);
  fprintf(out, "%s iAz: %d\n", spacer, (int) pulse->rvp8.iAz);
  fprintf(out, "%s iEl: %d\n", spacer, (int) pulse->rvp8.iEl);
  fprintf(out, "%s iNumVecs: %d\n", spacer, pulse->rvp8.iNumVecs);
  fprintf(out, "%s iMaxVecs: %d\n", spacer, pulse->rvp8.iMaxVecs);
  fprintf(out, "%s iVIQPerBin: %d\n", spacer, (int) pulse->rvp8.iVIQPerBin);
  fprintf(out, "%s iTgBank: %d\n", spacer, (int) pulse->rvp8.iTgBank);
  fprintf(out, "%s iTgWave: %d\n", spacer, (int) pulse->rvp8.iTgWave);
  fprintf(out, "%s uiqPerm.iLong[0]: %d\n", spacer, (int) pulse->rvp8.uiqPerm[0]);
  fprintf(out, "%s uiqPerm.iLong[1]: %d\n", spacer, (int) pulse->rvp8.uiqPerm[1]);
  fprintf(out, "%s uiqOnce.iLong[0]: %d\n", spacer, (int) pulse->rvp8.uiqOnce[0]);
  fprintf(out, "%s uiqOnce.iLong[1]: %d\n", spacer, (int) pulse->rvp8.uiqOnce[1]);
  fprintf(out, "%s RX[0].fBurstMag: %g\n", spacer, pulse->rvp8.fBurstMag[0]);
  fprintf(out, "%s RX[0].iBurstArg: %d\n", spacer, (int) pulse->rvp8.iBurstArg[0]);
  fprintf(out, "%s RX[1].fBurstMag: %g\n", spacer, pulse->rvp8.fBurstMag[1]);
  fprintf(out, "%s RX[1].iBurstArg: %d\n", spacer, (int) pulse->rvp8.iBurstArg[1]);
  fprintf(out, "----------------------------------------------------\n");

 }

/****************************
 * print ops info
 */

void ts_ops_info_print(FILE *out, const char *spacer,
                       ts_ops_info_t *info)

{

  int ii;

  fprintf(out, "\n");
  fprintf(out, "==================== ts_ops_info ==================\n");

  fprintf(out, "%s version: %d\n", spacer, info->version);
  fprintf(out, "%s radarId: %d\n", spacer, info->radarId);
  fprintf(out, "%s altitudeM: %g\n", spacer, info->altitudeM);
  fprintf(out, "%s latitudeDeg: %g\n", spacer, info->latitudeDeg);
  fprintf(out, "%s longitudeDeg: %g\n", spacer, info->longitudeDeg);
  fprintf(out, "%s startRangeM: %g\n", spacer, info->startRangeM);
  fprintf(out, "%s gateSpacingM: %g\n", spacer, info->gateSpacingM);
  fprintf(out, "%s scanMode: %d\n", spacer, info->scanMode);
  fprintf(out, "%s xmitRcvMode: %d\n", spacer, info->xmitRcvMode);
  fprintf(out, "%s prfMode: %d\n", spacer, info->prfMode);
  fprintf(out, "%s xmitPhaseMode: %d\n", spacer, info->xmitPhaseMode);
  fprintf(out, "%s wavelengthCm: %g\n", spacer, info->wavelengthCm);
  fprintf(out, "%s beamWidthDegH: %g\n", spacer, info->beamWidthDegH);
  fprintf(out, "%s beamWidthDegV: %g\n", spacer, info->beamWidthDegV);
  fprintf(out, "%s targetEl: %g\n", spacer, info->targetEl);
  fprintf(out, "%s targetAz: %g\n", spacer, info->targetAz);
  fprintf(out, "%s sectorAz1: %g\n", spacer, info->sectorAz1);
  fprintf(out, "%s sectorAz2: %g\n", spacer, info->sectorAz2);
  fprintf(out, "%s rhiEl1: %g\n", spacer, info->rhiEl1);
  fprintf(out, "%s rhiEl2: %g\n", spacer, info->rhiEl2);

  fprintf(out, "%s radarName: %s\n", spacer, info->radarName);
  fprintf(out, "%s siteName: %s\n", spacer, info->siteName);

  ds_radar_calib_print(out, spacer, &info->calib);

  fprintf(out, "-------------------- RVP8 section ------------------\n");
  fprintf(out, "%s iVersion: %d\n", spacer, info->rvp8.iVersion);
  fprintf(out, "%s iMajorMode: %d\n", spacer, info->rvp8.iMajorMode);
  fprintf(out, "%s iPolarization: %d\n", spacer, info->rvp8.iPolarization);
  fprintf(out, "%s iPhaseModSeq: %d\n", spacer, info->rvp8.iPhaseModSeq);
  fprintf(out, "%s iTaskSweep: %d\n", spacer, info->rvp8.iTaskSweep);
  fprintf(out, "%s iTaskAuxNum: %d\n", spacer, info->rvp8.iTaskAuxNum);
  fprintf(out, "%s sTaskName: %s\n", spacer, info->rvp8.sTaskName);
  fprintf(out, "%s sSiteName: %s\n", spacer, info->rvp8.sSiteName);
  fprintf(out, "%s iAqMode: %d\n", spacer, info->rvp8.iAqMode);
  fprintf(out, "%s iUnfoldMode: %d\n", spacer, info->rvp8.iUnfoldMode);
  fprintf(out, "%s iPWidthCode: %d\n", spacer, info->rvp8.iPWidthCode);
  fprintf(out, "%s fPWidthUSec: %g\n", spacer, info->rvp8.fPWidthUSec);
  fprintf(out, "%s fDBzCalib: %g\n", spacer, info->rvp8.fDBzCalib);
  fprintf(out, "%s iSampleSize: %d\n", spacer, info->rvp8.iSampleSize);
  fprintf(out, "%s iMeanAngleSync: %d\n", spacer, info->rvp8.iMeanAngleSync);
  fprintf(out, "%s iFlags: %d\n", spacer, info->rvp8.iFlags);
  fprintf(out, "%s iPlaybackVersion: %d\n", spacer, info->rvp8.iPlaybackVersion);
  fprintf(out, "%s fSyClkMHz: %g\n", spacer, info->rvp8.fSyClkMHz);
  fprintf(out, "%s fWavelengthCM: %g\n", spacer, info->rvp8.fWavelengthCM);
  fprintf(out, "%s fSaturationDBM: %g\n", spacer, info->rvp8.fSaturationDBM);
  fprintf(out, "%s fRangeMaskRes: %g\n", spacer, info->rvp8.fRangeMaskRes);

  fprintf(out, "%s iRangeMask: ", spacer);
  for (ii = 0; ii < TS_GATE_MASK_LEN; ii++) {
    fprintf(out, "%d", info->rvp8.iRangeMask[ii]);
    if (ii != TS_GATE_MASK_LEN - 1) {
      fprintf(out, " ");
    }
  }
  fprintf(out, "\n");

  fprintf(out, "%s fNoiseDBm: %g, %g\n", spacer,
	  info->rvp8.fNoiseDBm[0], info->rvp8.fNoiseDBm[1]);

  fprintf(out, "%s fNoiseStdvDB: %g, %g\n", spacer,
	  info->rvp8.fNoiseStdvDB[0], info->rvp8.fNoiseStdvDB[1]);

  fprintf(out, "%s fNoiseRangeKM: %g\n", spacer, info->rvp8.fNoiseRangeKM);
  fprintf(out, "%s fNoisePRFHz: %g\n", spacer, info->rvp8.fNoisePRFHz);
  
  fprintf(out, "%s iGparmLatchSts: %d, %d\n", spacer,
	  info->rvp8.iGparmLatchSts[0],
	  info->rvp8.iGparmLatchSts[1]);

  fprintf(out, "%s iGparmImmedSts: %d, %d, %d, %d, %d, %d\n", spacer,
	  info->rvp8.iGparmImmedSts[0],
	  info->rvp8.iGparmImmedSts[1],
	  info->rvp8.iGparmImmedSts[2],
	  info->rvp8.iGparmImmedSts[3],
	  info->rvp8.iGparmImmedSts[4],
	  info->rvp8.iGparmImmedSts[5]);

  fprintf(out, "%s iGparmDiagBits: %d, %d, %d, %d\n", spacer,
	  info->rvp8.iGparmDiagBits[0],
	  info->rvp8.iGparmDiagBits[1],
	  info->rvp8.iGparmDiagBits[2],
	  info->rvp8.iGparmDiagBits[3]);
  
  fprintf(out, "%s sVersionString: %s\n", spacer, info->rvp8.sVersionString);

}
