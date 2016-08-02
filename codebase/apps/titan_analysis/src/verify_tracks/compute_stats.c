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
/****************************************************************************
 * compute_stats.c
 *
 * computes the errors for forecast data
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * January 1992
 *
 ****************************************************************************/

#include "verify_tracks.h"

static vt_stats_t *Stats;

static void load_stats(fl32 *prop_p, double n);

void compute_stats(vt_stats_t *stats)

{

  Stats = stats;

  /*
   * compute stats
   */

  if (Stats->n_movement > 0) {
    
    load_stats(&Stats->bias.proj_area_centroid_x, Stats->n_movement);
    load_stats(&Stats->bias.proj_area_centroid_y, Stats->n_movement);
    load_stats(&Stats->bias.vol_centroid_z, Stats->n_movement);
    load_stats(&Stats->bias.refl_centroid_z, Stats->n_movement);
    load_stats(&Stats->bias.top, Stats->n_movement);
    load_stats(&Stats->bias.smoothed_speed, Stats->n_movement);
    load_stats(&Stats->bias.smoothed_direction, Stats->n_movement);
    
  } /* if (Stats->n_movement > 0) */

  if (Stats->n_growth > 0) {

    load_stats(&Stats->bias.dbz_max, Stats->n_growth);
    load_stats(&Stats->bias.volume, Stats->n_growth);
    load_stats(&Stats->bias.precip_flux, Stats->n_growth);
    load_stats(&Stats->bias.mass, Stats->n_growth);
    load_stats(&Stats->bias.proj_area, Stats->n_growth);

  } /* if (Stats->n_growth > 0) */
  
}

/*****************************************************************
 * load_stats()
 */

static void load_stats(fl32 *prop_p, double n)

{
  
  long offset;
  double sumx, sumx2;
  double sumy, sumy2;
  double sumxy;
  double corr;
  double num, denomsq;

  /*
   * compute the offset into the struct
   */

  offset = prop_p - (fl32 *) &Stats->bias;

  /*
   * bias and rmse
   */

  *((fl32 *) &Stats->bias + offset) =
    *((fl32 *) &Stats->sum_error + offset) / n;

  *((fl32 *) &Stats->rmse + offset) =
    sqrt(*((fl32 *) &Stats->sum_sq_error + offset) / n);

  *((fl32 *) &Stats->norm_bias + offset) =
    *((fl32 *) &Stats->norm_sum_error + offset) / n;

  *((fl32 *) &Stats->norm_rmse + offset) =
    sqrt(*((fl32 *) &Stats->norm_sum_sq_error + offset) / n);

  /*
   * correlation coefficient
   */

  sumx = *((fl32 *) &Stats->sumx + offset);
  sumx2 = *((fl32 *) &Stats->sumx2 + offset);
  sumy = *((fl32 *) &Stats->sumy + offset);
  sumy2 = *((fl32 *) &Stats->sumy2 + offset);
  sumxy = *((fl32 *) &Stats->sumxy + offset);
  
  denomsq = (n * sumx2 - sumx * sumx) * (n * sumy2 - sumy * sumy);
  num = n * sumxy - sumx * sumy;

  if (denomsq > 0.0) {
    corr = num / sqrt(denomsq);
  } else {
    corr = 0.0;
  }
  
  *((fl32 *) &Stats->corr + offset) = corr;
  
}

