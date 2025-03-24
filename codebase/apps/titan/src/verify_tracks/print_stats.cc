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
/*****************************************************************************
 * print_stats.c
 *
 * debug printout of stats
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * January 1992
 *
 *****************************************************************************/

#include "verify_tracks.h"

static vt_stats_t *Stats;
static void print_stat(FILE *fout,
		       const char *label,
		       fl32 *bias_p,
		       int print_norm);

void print_stats(FILE *fout,
		 const char *heading,
                 vt_stats_t *stats)

{

  double x_sum, x_sum_sq, x_mean, x_stdev;

  Stats = stats;

  fprintf(fout, "\n%s\n\n", heading);
  fprintf(fout, "%20s: %g\n\n",
	  "nsamples_movement", stats->n_movement);
  fprintf(fout, "%20s: %g\n\n",
	  "nsamples_growth", stats->n_growth);

  fprintf(fout, "%20s  %10s %10s %10s %10s %10s\n",
	  "Variable", "Bias", "Corr", "Rmse", "Norm bias", "Norm rmse");

  fprintf(fout, "%20s  %10s %10s %10s %10s %10s\n",
	  "--------", "----", "----", "----", "---------", "---------");

  print_stat(fout, "Proj_area_centroid_x",
	     &Stats->bias.proj_area_centroid_x, FALSE);
  print_stat(fout, "Proj_area_centroid_y",
	     &Stats->bias.proj_area_centroid_y, FALSE);
  print_stat(fout, "Vol_centroid_z", &Stats->bias.vol_centroid_z, FALSE);
  print_stat(fout, "Refl_centroid_z", &Stats->bias.refl_centroid_z, FALSE);
  print_stat(fout, "Top", &Stats->bias.top, FALSE);
  print_stat(fout, "Speed", &Stats->bias.smoothed_speed, TRUE);
  print_stat(fout, "Direction", &Stats->bias.smoothed_direction, FALSE);
  print_stat(fout, "dBZ_max", &Stats->bias.dbz_max, TRUE);
  print_stat(fout, "Volume", &Stats->bias.volume, TRUE);
  print_stat(fout, "Precip_flux", &Stats->bias.precip_flux, TRUE);
  print_stat(fout, "Mass", &Stats->bias.mass, TRUE);
  print_stat(fout, "Proj_area", &Stats->bias.proj_area, TRUE);

  /* 
   * calc and print mean distance error between forecast and verify cells 
   */

  fprintf(fout, "\n%20s  %10s %10s \n",
	  "Variable", "Mean", "Stdev" );

  fprintf(fout, "%20s  %10s %10s \n",
	  "--------", "----", "-----" );

  x_sum=Stats->sum_dist_error ;
  x_sum_sq=Stats->sum_sq_dist_error;
  x_mean = x_sum / Stats->n_movement;
  x_stdev=sqrt((x_sum_sq*Stats->n_movement - x_sum*x_sum) / (Stats->n_movement*(Stats->n_movement-1))) ;

  fprintf(fout, "%20s: %10g %10g\n", "Delta_r",x_mean, x_stdev );

  fprintf(fout, "\n");
  fprintf(fout, "Note : bias sense is (forecast - verification).\n");
  
}

/*****************************************************************
 * print_stat()
 */

static void print_stat(FILE *fout,
		       const char *label,
		       fl32 *bias_p,
		       int print_norm)

{
  
  long offset;
  fl32 bias, rmse;
  fl32 norm_bias, norm_rmse;
  fl32 corr;

  /*
   * compute the offset into the struct
   */

  offset = bias_p - (fl32 *) &Stats->bias;

  bias = *((fl32 *) &Stats->bias + offset);
  corr = *((fl32 *) &Stats->corr + offset);
  rmse = *((fl32 *) &Stats->rmse + offset);
  
  if (print_norm) {

    norm_bias = *((fl32 *) &Stats->norm_bias + offset);
    norm_rmse = *((fl32 *) &Stats->norm_rmse + offset);

    fprintf(fout, "%20s: %10g %10g %10g %10.2g %10.2g\n",
	    label, bias, corr, rmse, norm_bias, norm_rmse);

  } else {

    fprintf(fout, "%20s: %10g %10g %10g\n", label, bias, corr, rmse);

  }

}

