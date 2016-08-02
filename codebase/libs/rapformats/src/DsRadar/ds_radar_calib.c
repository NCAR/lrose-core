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
 * ds_radar_calib.c
 *
 * C routines for ds_radar_calib struct
 *
 * Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
 *
 * Sept 2007
 */

#include <dataport/bigend.h>
#include <rapformats/ds_radar_calib.h>
#include <toolsa/mem.h>

/**********************
 * BE swapping routines
 */

/********************
 * ds_radar_calib_init()
 *
 * Initialize radar calibation struct
 */

void ds_radar_calib_init(ds_radar_calib_t *calib)
{

  memset(calib, 0, sizeof(ds_radar_calib_t));

  calib->wavelengthCm = DS_RADAR_CALIB_MISSING;
  calib->beamWidthDegH = DS_RADAR_CALIB_MISSING;
  calib->beamWidthDegV = DS_RADAR_CALIB_MISSING;
  calib->antGainDbH = DS_RADAR_CALIB_MISSING;
  calib->antGainDbV = DS_RADAR_CALIB_MISSING;
  calib->pulseWidthUs = DS_RADAR_CALIB_MISSING;
  calib->xmitPowerDbmH = DS_RADAR_CALIB_MISSING;
  calib->xmitPowerDbmV = DS_RADAR_CALIB_MISSING;
  calib->twoWayWaveguideLossDbH = DS_RADAR_CALIB_MISSING;
  calib->twoWayWaveguideLossDbV = DS_RADAR_CALIB_MISSING;
  calib->twoWayRadomeLossDbH = DS_RADAR_CALIB_MISSING;
  calib->twoWayRadomeLossDbV = DS_RADAR_CALIB_MISSING;
  calib->receiverMismatchLossDb = DS_RADAR_CALIB_MISSING;
  calib->radarConstH = DS_RADAR_CALIB_MISSING;
  calib->radarConstV = DS_RADAR_CALIB_MISSING;
  calib->noiseDbmHc = DS_RADAR_CALIB_MISSING;
  calib->noiseDbmHx = DS_RADAR_CALIB_MISSING;
  calib->noiseDbmVc = DS_RADAR_CALIB_MISSING;
  calib->noiseDbmVx = DS_RADAR_CALIB_MISSING;
  calib->receiverGainDbHc = DS_RADAR_CALIB_MISSING;
  calib->receiverGainDbHx = DS_RADAR_CALIB_MISSING;
  calib->receiverGainDbVc = DS_RADAR_CALIB_MISSING;
  calib->receiverGainDbVx = DS_RADAR_CALIB_MISSING;
  calib->sunPowerDbmHc = DS_RADAR_CALIB_MISSING;
  calib->sunPowerDbmHx = DS_RADAR_CALIB_MISSING;
  calib->sunPowerDbmVc = DS_RADAR_CALIB_MISSING;
  calib->sunPowerDbmVx = DS_RADAR_CALIB_MISSING;
  calib->noiseSourcePowerDbmH = DS_RADAR_CALIB_MISSING;
  calib->noiseSourcePowerDbmV = DS_RADAR_CALIB_MISSING;
  calib->powerMeasLossDbH = DS_RADAR_CALIB_MISSING;
  calib->powerMeasLossDbV = DS_RADAR_CALIB_MISSING;
  calib->couplerForwardLossDbH = DS_RADAR_CALIB_MISSING;
  calib->couplerForwardLossDbV = DS_RADAR_CALIB_MISSING;
  calib->zdrCorrectionDb = 0;
  calib->ldrCorrectionDbH = 0;
  calib->ldrCorrectionDbV = 0;
  calib->systemPhidpDeg = 0;
  calib->testPowerDbmH = DS_RADAR_CALIB_MISSING;
  calib->testPowerDbmV = DS_RADAR_CALIB_MISSING;
  calib->receiverSlopeDbHc = DS_RADAR_CALIB_MISSING;
  calib->receiverSlopeDbHx = DS_RADAR_CALIB_MISSING;
  calib->receiverSlopeDbVc = DS_RADAR_CALIB_MISSING;
  calib->receiverSlopeDbVx = DS_RADAR_CALIB_MISSING;
  calib->i0DbmHc = DS_RADAR_CALIB_MISSING;
  calib->i0DbmHx = DS_RADAR_CALIB_MISSING;
  calib->i0DbmVc = DS_RADAR_CALIB_MISSING;
  calib->i0DbmVx = DS_RADAR_CALIB_MISSING;
  calib->dynamicRangeDbHc = DS_RADAR_CALIB_MISSING;
  calib->dynamicRangeDbHx = DS_RADAR_CALIB_MISSING;
  calib->dynamicRangeDbVc = DS_RADAR_CALIB_MISSING;
  calib->dynamicRangeDbVx = DS_RADAR_CALIB_MISSING;
  calib->kSquaredWater = DS_RADAR_CALIB_MISSING;
  calib->dbzCorrection = 0;

}

/***********************
 * BE_to_ds_radar_calib()
 *
 * Convert BE to ds_radar_calib_t
 */

void BE_to_ds_radar_calib(ds_radar_calib_t *calib)
     
{
  BE_to_array_32((char *) calib + DS_RADAR_CALIB_NAME_LEN,
                 sizeof(ds_radar_calib_t) - DS_RADAR_CALIB_NAME_LEN);
}

/*************************
 * BE_from_ds_radar_calib()
 *
 * Convert ds_radar_calib_t to BE
 */

void BE_from_ds_radar_calib(ds_radar_calib_t *calib)

{
  BE_from_array_32((char *) calib + DS_RADAR_CALIB_NAME_LEN,
                   sizeof(ds_radar_calib_t) - DS_RADAR_CALIB_NAME_LEN);
}

/****************************
 * printing
 */

void ds_radar_calib_print(FILE *out, const char *spacer,
                          ds_radar_calib_t *calib)

{
  
  fprintf(out, "\n");
  fprintf(out, "%s ds_radar_calib\n", spacer);
  fprintf(out, "%s ------------\n", spacer);
  
  fprintf(out, "%s wavelengthCm: %g\n", spacer, calib->wavelengthCm);
  fprintf(out, "%s beamWidthDegH: %g\n", spacer, calib->beamWidthDegH);
  fprintf(out, "%s beamWidthDegV: %g\n", spacer, calib->beamWidthDegV);
  fprintf(out, "%s antGainDbH: %g\n", spacer, calib->antGainDbH);
  fprintf(out, "%s antGainDbV: %g\n", spacer, calib->antGainDbV);
  fprintf(out, "%s xmitPowerDbmH: %g\n", spacer, calib->xmitPowerDbmH);
  fprintf(out, "%s xmitPowerDbmV: %g\n", spacer, calib->xmitPowerDbmV);
  fprintf(out, "%s twoWayWaveguideLossDbH: %g\n", spacer, calib->twoWayWaveguideLossDbH);
  fprintf(out, "%s twoWayWaveguideLossDbV: %g\n", spacer, calib->twoWayWaveguideLossDbV);
  fprintf(out, "%s twoWayRadomeLossDbH: %g\n", spacer, calib->twoWayRadomeLossDbH);
  fprintf(out, "%s twoWayRadomeLossDbV: %g\n", spacer, calib->twoWayRadomeLossDbV);
  fprintf(out, "%s receiverMismatchLossDb: %g\n", spacer, calib->receiverMismatchLossDb);
  fprintf(out, "%s kSquaredWater: %g\n", spacer, calib->kSquaredWater);
  fprintf(out, "%s radarConstH: %g\n", spacer, calib->radarConstH);
  fprintf(out, "%s radarConstV: %g\n", spacer, calib->radarConstV);
  fprintf(out, "%s noiseDbmHc: %g\n", spacer, calib->noiseDbmHc);
  fprintf(out, "%s noiseDbmHx: %g\n", spacer, calib->noiseDbmHx);
  fprintf(out, "%s noiseDbmVc: %g\n", spacer, calib->noiseDbmVc);
  fprintf(out, "%s noiseDbmVx: %g\n", spacer, calib->noiseDbmVx);
  fprintf(out, "%s receiverGainDbHc: %g\n", spacer, calib->receiverGainDbHc);
  fprintf(out, "%s receiverGainDbHx: %g\n", spacer, calib->receiverGainDbHx);
  fprintf(out, "%s receiverGainDbVc: %g\n", spacer, calib->receiverGainDbVc);
  fprintf(out, "%s receiverGainDbVx: %g\n", spacer, calib->receiverGainDbVx);
  fprintf(out, "%s receiverSlopeDbHc: %g\n", spacer, calib->receiverSlopeDbHc);
  fprintf(out, "%s receiverSlopeDbHx: %g\n", spacer, calib->receiverSlopeDbHx);
  fprintf(out, "%s receiverSlopeDbVc: %g\n", spacer, calib->receiverSlopeDbVc);
  fprintf(out, "%s receiverSlopeDbVx: %g\n", spacer, calib->receiverSlopeDbVx);
  fprintf(out, "%s i0DbmHc: %g\n", spacer, calib->i0DbmHc);
  fprintf(out, "%s i0DbmHx: %g\n", spacer, calib->i0DbmHx);
  fprintf(out, "%s i0DbmVc: %g\n", spacer, calib->i0DbmVc);
  fprintf(out, "%s i0DbmVx: %g\n", spacer, calib->i0DbmVx);
  fprintf(out, "%s dynamicRangeDbHc: %g\n", spacer, calib->dynamicRangeDbHc);
  fprintf(out, "%s dynamicRangeDbHx: %g\n", spacer, calib->dynamicRangeDbHx);
  fprintf(out, "%s dynamicRangeDbVc: %g\n", spacer, calib->dynamicRangeDbVc);
  fprintf(out, "%s dynamicRangeDbVx: %g\n", spacer, calib->dynamicRangeDbVx);
  fprintf(out, "%s sunPowerDbmHc: %g\n", spacer, calib->sunPowerDbmHc);
  fprintf(out, "%s sunPowerDbmHx: %g\n", spacer, calib->sunPowerDbmHx);
  fprintf(out, "%s sunPowerDbmVc: %g\n", spacer, calib->sunPowerDbmVc);
  fprintf(out, "%s sunPowerDbmVx: %g\n", spacer, calib->sunPowerDbmVx);
  fprintf(out, "%s noiseSourcePowerDbmH: %g\n", spacer, calib->noiseSourcePowerDbmH);
  fprintf(out, "%s noiseSourcePowerDbmV: %g\n", spacer, calib->noiseSourcePowerDbmV);
  fprintf(out, "%s powerMeasLossDbH: %g\n", spacer, calib->powerMeasLossDbH);
  fprintf(out, "%s powerMeasLossDbV: %g\n", spacer, calib->powerMeasLossDbV);
  fprintf(out, "%s couplerForwardLossDbH: %g\n", spacer, calib->couplerForwardLossDbH);
  fprintf(out, "%s couplerForwardLossDbV: %g\n", spacer, calib->couplerForwardLossDbV);
  fprintf(out, "%s zdrCorrectionDb: %g\n", spacer, calib->zdrCorrectionDb);
  fprintf(out, "%s ldrCorrectionDbH: %g\n", spacer, calib->ldrCorrectionDbH);
  fprintf(out, "%s ldrCorrectionDbV: %g\n", spacer, calib->ldrCorrectionDbV);
  fprintf(out, "%s systemPhidpDeg: %g\n", spacer, calib->systemPhidpDeg);
  fprintf(out, "%s testPowerDbmH: %g\n", spacer, calib->testPowerDbmH);
  fprintf(out, "%s testPowerDbmV: %g\n", spacer, calib->testPowerDbmV);
  fprintf(out, "%s dbzCorrection: %g\n", spacer, calib->dbzCorrection);

 }

