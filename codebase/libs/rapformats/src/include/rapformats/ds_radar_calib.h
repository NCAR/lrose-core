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

/*******************************************************************
 * ds_radar_calib.h
 *
 * Radar data calibration.
 ******************************************************************/

#ifndef ds_radar_calib_h
#define ds_radar_calib_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <dataport/port_types.h>

/*
 * if a value is missing, it is set to the missing value
 * defined here
 */
  
#define DS_RADAR_CALIB_NAME_LEN 16
#define DS_RADAR_CALIB_MISSING -9999.0F

typedef struct {
  
  /* name - null terminated, so max of 15 chars */

  char radarName[DS_RADAR_CALIB_NAME_LEN];

  /* time of calibration */

  si32 year, month, day, hour, min, sec;

  /* radar parameters */
  
  fl32 wavelengthCm;  /* radar wavelength in cm */
  fl32 beamWidthDegH; /* beam width H in degrees */
  fl32 beamWidthDegV; /* beam width V in degrees */
  fl32 antGainDbH;    /* antenna gain H in degrees */
  fl32 antGainDbV;    /* antenna gain V in degrees */

  /* pulse width and transmit power */

  fl32 pulseWidthUs;   /* pulse width in Us */
  fl32 xmitPowerDbmH;  /* peak transmit H power in dBm */
  fl32 xmitPowerDbmV;  /* peak transmit V power in dBm */
  
  /* 2-way waveguide loss from feedhorn to measurement plane.
   * Set to 0 is the loss is incorporated into the antenna gain */
  
  fl32 twoWayWaveguideLossDbH;
  fl32 twoWayWaveguideLossDbV;
  
  /* 2-way Radome loss (dB)
   * Set to 0 is the loss is incorporated into the antenna gain */

  fl32 twoWayRadomeLossDbH;
  fl32 twoWayRadomeLossDbV;

  /* receiver mistmatch loss (dB) */
  
  fl32 receiverMismatchLossDb;

  /* Radar constant for each waveguide */
  
  fl32 radarConstH;
  fl32 radarConstV;

  /* noise Dbm - noise level for each channel, from calibration
   * SNR is computed relative to these noise values */

  fl32 noiseDbmHc; /* calibrated noise value, dBm - Hc H co-polar */
  fl32 noiseDbmHx; /* calibrated noise value, dBm - Hx H cross-polar */
  fl32 noiseDbmVc; /* calibrated noise value, dBm - Vc V co-polar */
  fl32 noiseDbmVx; /* calibrated noise value, dBm - Vx V cross-polar */

  /* Receiver gain for each channel - dB 
   * Gain from waveguide power to digitized power */
  
  fl32 receiverGainDbHc;
  fl32 receiverGainDbHx;
  fl32 receiverGainDbVc;
  fl32 receiverGainDbVx;
  
  /* Base reflectivity at 1 km, for SNR of 0.
   * dBZ = baseDbz1km + SNR + 20log10(rangeKm) + (atmosAtten * rangeKm)
   * BaseDbz1km can be computed as follows:
   *   baseDbz1km = noiseDbm - receiverGainDb - radarConst
   * However, sometimes these values are not available, and the
   * baseDbz1km value must be used as is. This is the case for RVP8 time
   * series data */
  
  fl32 baseDbz1kmHc;
  fl32 baseDbz1kmHx;
  fl32 baseDbz1kmVc;
  fl32 baseDbz1kmVx;

  /* Sun power for each channel - dBm */
  
  fl32 sunPowerDbmHc;
  fl32 sunPowerDbmHx;
  fl32 sunPowerDbmVc;
  fl32 sunPowerDbmVx;

  /* noise source power */
  
  fl32 noiseSourcePowerDbmH; /* H power in dBm */
  fl32 noiseSourcePowerDbmV; /* V power in dBm */

  /* Power measurement loss from ref. point
   * to power meter sensor, each channel.
   * This will generally be positive, to indicate a loss.
   * If there is an amplifier in the calibration circuit, use a negative
   * number to indicate a gain */

  fl32 powerMeasLossDbH;
  fl32 powerMeasLossDbV;

  /* Directional coupler forward loss for H and V
   * This will be negative */
  
  fl32 couplerForwardLossDbH;
  fl32 couplerForwardLossDbV;

  /* ZDR / LDR corrections, system PHIDP */

  fl32 zdrCorrectionDb;  /* ZDR correction, dB */
  fl32 ldrCorrectionDbH; /* LDR correction, dB, H */
  fl32 ldrCorrectionDbV; /* LDR correction, dB, V */
  fl32 systemPhidpDeg;   /* system phipd - degrees */

  /* test power */
  
  fl32 testPowerDbmH;
  fl32 testPowerDbmV;

  /* slope of linear part of receiver response curve 
   * in dB units */

  fl32 receiverSlopeDbHc;
  fl32 receiverSlopeDbHx;
  fl32 receiverSlopeDbVc;
  fl32 receiverSlopeDbVx;

  /* I0 values represent the real noise floor, taking receiver
   * gain into account: I0 = (noise - receiver gain) */

  fl32 i0DbmHc; /* calibrated I0 noise, dBm - Hc H co-polar */
  fl32 i0DbmHx; /* calibrated I0 noise, dBm - Hx H cross-polar */
  fl32 i0DbmVc; /* calibrated I0 noise, dBm - Vc V co-polar */
  fl32 i0DbmVx; /* calibrated I0 noise, dBm - Vx V cross-polar */

  /* Dynamic range in dB per channel */

  fl32 dynamicRangeDbHc; /* dynamic range, dB - Hc H co-polar */
  fl32 dynamicRangeDbHx; /* dynamic range, dB - Hx H cross-polar */
  fl32 dynamicRangeDbVc; /* dynamic range, dB - Vc V co-polar */
  fl32 dynamicRangeDbVx; /* dynamic range, dB - Vx V cross-polar */

  /* di-electric constant for water */

  fl32 kSquaredWater;

  /* DBZ correction */

  fl32 dbzCorrection;  /* DBZ correction, dB */

  /* spares */

  si32 spare1[8];

} ds_radar_calib_t;

/*
 * function prototypes
 */

/********************
 * ds_radar_calib_init()
 *
 * Initialize radar calib struct
 */

extern void ds_radar_calib_init(ds_radar_calib_t *calib);

/***********************
 * BE_to_ds_radar_calib()
 *
 * Convert BE to ds_radar_calib_t
 */

extern void BE_to_ds_radar_calib(ds_radar_calib_t *calib);
     
/*************************
 * BE_from_ds_radar_calib()
 *
 * Convert ds_radar_calib_t to BE
 */

extern void BE_from_ds_radar_calib(ds_radar_calib_t *calib);

/*************************
 * Printing()
 */

extern void ds_radar_calib_print(FILE *out, const char *spacer,
                                 ds_radar_calib_t *rcalib);

#ifdef __cplusplus
}
#endif

#endif

