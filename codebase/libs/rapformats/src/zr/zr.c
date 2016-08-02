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
/*********************************************************************
 * zr.c
 *
 * Routines for handling the trec gauge data
 *
 * RAP, NCAR, Boulder CO
 *
 * Mike Dixon
 *
 * Oct 1997
 *
 *********************************************************************/

#include <toolsa/umisc.h>
#include <rapformats/zr.h>
#include <dataport/bigend.h>

/****************************
 * zr_params_to_BE
 *
 * Swaps ZR params to BigEnd in place
 */

void zr_params_to_BE(zr_params_t *zrp)

{
  BE_from_array_32(zrp, ZR_NBYTES_32);
}

/****************************
 * zr_params_from_BE
 *
 * Swaps ZR params from BigEnd in place
 */

void zr_params_from_BE(zr_params_t *zrp)

{
  BE_to_array_32(zrp, ZR_NBYTES_32);
}

/****************************
 * zr_params_print
 *
 * Prints out struct
 */

void zr_params_print(FILE *out, const char *spacer, zr_params_t *zrp)

{

  fprintf(out, "\n");
  fprintf(out, "-----------------------------------\n");
  fprintf(out, "%sZR PARAMS\n", spacer);

  fprintf(out, "%s  calibration time: %s\n", spacer,
	  utimstr(zrp->calib_time));
  switch (zrp->mode) {

  case ZR_MODE_CALIBRATED:
    fprintf(out, "%s  Mode: CALIBRATED\n", spacer);
    break;

  case ZR_MODE_DEFAULT:
    fprintf(out, "%s  Mode: DEFAULT\n", spacer);
    break;

  case ZR_MODE_COASTING:
    fprintf(out, "%s  Mode: COASTING\n", spacer);
    break;

  } /* switch */

  fprintf(out, "%s  accum_period (secs): %d\n", spacer,
	  zrp->accum_period);
  fprintf(out, "%s  n_valid_pairs: %d\n", spacer,
	  zrp->n_valid_pairs);
  fprintf(out, "%s  z_to_gauge_lag (secs): %d\n", spacer,
	  zrp->z_to_gauge_lag);
  fprintf(out, "%s  mean_gauge_accum (mm): %f\n", spacer,
	  (double) zrp->mean_gauge_accum);
  fprintf(out, "%s  coefficient: %f\n", spacer,
	  (double) zrp->coeff);
  fprintf(out, "%s  exponent:    %f\n", spacer,
	  (double) zrp->expon);

  fprintf(out, "-----------------------------------\n");

}

