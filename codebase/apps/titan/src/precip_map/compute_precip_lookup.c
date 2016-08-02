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

#include "precip_map.h"

/*******************************************************************
 * compute_precip_lookup.c
 *
 * computes dbz-precip lookup table
 *
 * Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
 *
 * August 1993
 */

void compute_precip_lookup(double *precip_lookup,
			   field_params_t *dbz_fparams,
			   double coeff, double expon)

{

  si32 i;
  double dbz_scale, dbz_bias;
  double dbz, z;
  double precip_rate;

  dbz_scale = (double) dbz_fparams->scale / (double) dbz_fparams->factor;
  dbz_bias = (double) dbz_fparams->bias / (double) dbz_fparams->factor;

  for (i = 0; i < 256; i++) {

    dbz = (double) i * dbz_scale + dbz_bias;

    if (dbz < Glob->params.low_refl_threshold)
      dbz = -100.0;

    if (dbz > Glob->params.hail_refl_threshold)
      dbz = Glob->params.hail_refl_threshold;

    z = pow(10.0, dbz / 10.0);

    precip_rate = pow((z / coeff), (1.0 / expon));

    if (precip_rate < 0.001)
      precip_lookup[i] = 0.0;
    else
      precip_lookup[i] = precip_rate;

  } /* i */

  if (Glob->params.debug >= DEBUG_EXTRA) {
    fprintf(stderr, "ZR coeff, expon: %g, %g\n", coeff, expon);
  }

}

