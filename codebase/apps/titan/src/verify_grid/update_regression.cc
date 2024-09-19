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
/************************************************************************
 * update_regression.c
 *
 * Obtains regression pairs and prints to output
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * July 1992
 *
 ***********************************************************************/

#include "verify_grid.h"

/*
 * prototypes
 */

static void load_means(vol_file_handle_t *index,
		       mean_stats_t **means,
		       si32 field);

/*
 * main
 */

void update_regression(char *detect_file_path,
		       FILE *fout)


{
  
  static vol_file_handle_t truth_index;
  static vol_file_handle_t detect_index;
  static mean_stats_t **truth_stats, **detect_stats;
  static int first_call = TRUE;
  
  char *truth_file_path;

  si32 ix, iy;
  si32 truth_time, detect_time;

  mean_stats_t *truth_stat, *detect_stat;

  /*
   * initialize
   */
  
  if (first_call) {
    
    init_indices(&truth_index, &detect_index);

    truth_stats = (mean_stats_t **) umalloc2
      ((ui32) Glob->params.grid.ny,
       (ui32) Glob->params.grid.nx, sizeof(mean_stats_t));
    
    detect_stats = (mean_stats_t **) umalloc2
      ((ui32) Glob->params.grid.ny,
       (ui32) Glob->params.grid.nx, sizeof(mean_stats_t));
    
    first_call = FALSE;
    
  } /* if (first_call) */

  /*
   * get the file path for the truth data - if this returns
   * error, there is no truth data within the time margin of
   * the detect data, so return early
   */

  if ((truth_file_path = get_truth_path(detect_file_path)) == NULL)
    return;

  if (Glob->params.debug >= DEBUG_NORM) {

    fprintf(stderr, "Detection file '%s'\n", detect_file_path);
    fprintf(stderr, "Truth     file '%s'\n", truth_file_path);

  }

  /*
   * read in the data from the truth and detection files
   */

  detect_index.vol_file_path = detect_file_path;

  if (RfReadVolume(&detect_index, "update_regression") != R_SUCCESS)
    return;

  truth_index.vol_file_path = truth_file_path;
  
  if (RfReadVolume(&truth_index, "update_regression") != R_SUCCESS)
    return;

  /*
   * check that there is only 1 plane
   */

  if (truth_index.vol_params->cart.nz != 1) {

    fprintf(stderr, "ERROR - %s:update_regression\n", Glob->prog_name);
    fprintf(stderr, "Truth data has %ld planes - should be 1\n",
	    (long) truth_index.vol_params->cart.nz);
    return;

  }

  if (detect_index.vol_params->cart.nz != 1) {

    fprintf(stderr, "ERROR - %s:update_regression\n", Glob->prog_name);
    fprintf(stderr, "Detection data has %ld planes - should be 1\n",
	    (long) detect_index.vol_params->cart.nz);
    return;
    
  }

  /*
   * check that field number is valid
   */

  if (Glob->params.truth_field > truth_index.vol_params->nfields - 1) {

    fprintf(stderr, "ERROR - %s:update_regression\n", Glob->prog_name);
    fprintf(stderr, "Truth data field %ld too large\n",
	    Glob->params.truth_field);
    fprintf(stderr, "Max allowed is %ld\n",
	    (long) (truth_index.vol_params->nfields - 1));
    return;

  }

  if (Glob->params.detect_field > detect_index.vol_params->nfields - 1) {

    fprintf(stderr, "ERROR - %s:update_regression\n", Glob->prog_name);
    fprintf(stderr, "Detect data field %ld too large\n",
	    Glob->params.detect_field);
    fprintf(stderr, "Max allowed is %ld\n",
	    (long)  (detect_index.vol_params->nfields - 1));
    return;

  }

  /*
   * load up the truth and detection computation arrays
   */
  
  load_means(&truth_index, truth_stats, Glob->params.truth_field);
  load_means(&detect_index, detect_stats, Glob->params.detect_field);

  /*
   * print the stats pairs to the file
   */

  truth_time = Rfrtime2utime(&truth_index.vol_params->mid_time);
  detect_time = Rfrtime2utime(&detect_index.vol_params->mid_time);

  for (iy = 0; iy < Glob->params.grid.ny; iy++) {

    for (ix = 0; ix < Glob->params.grid.nx; ix++) {
      
      truth_stat = truth_stats[iy] + ix;
      detect_stat = detect_stats[iy] + ix;
      
      if (truth_stat->n > 0 && detect_stat->n > 0) {
	
/*	if (truth_stat->mean >= Glob->params.min_regression_val ||
	    detect_stat->mean >= Glob->params.min_regression_val) { */

	if (truth_stat->mean >= Glob->params.min_regression_val &&
	    detect_stat->mean >= Glob->params.min_regression_val) {

	  fprintf (fout, "%ld %ld %g %g\n",
		   (long) truth_time, (long) detect_time,
		   truth_stat->mean,
		   detect_stat->mean);

	} /* if (truth_stat->mean >= Glob->params.min_regression_val ... */

      } /* if (truth_stat->n > 0 && detect_stat->n > 0) */
	       
    } /* ix */

  } /* iy */

}

/********************************************************************
 * load_means()
 *
 * compute the mean value for a given grid location and store it
 * in the array
 */

static void load_means(vol_file_handle_t *index,
		       mean_stats_t **stats,
		       si32 field)

{

  ui08 *data_plane;
  ui08 *dp;

  si32 ix, iy;
  si32 jx, jy;
  si32 low_iy, low_ix;
  si32 high_iy, high_ix;
  
  double cart_dy, cart_dx;
  double cart_miny, cart_minx;
  double high_y, high_x;
  double scale, bias;
  double data_val;
  
  mean_stats_t *st;
  cart_params_t *cart = &index->vol_params->cart;
  field_params_t *fparams;

  /*
   * initialize
   */

  cart = &index->vol_params->cart;
  fparams = index->field_params[field];
  data_plane = index->field_plane[field][0];

  cart_dy = (double) cart->dy / (double) cart->scaley;
  cart_dx = (double) cart->dx / (double) cart->scalex;
  cart_miny = (double) cart->miny / (double) cart->scaley;
  cart_minx = (double) cart->minx / (double) cart->scalex;

  scale = (double) fparams->scale / (double) fparams->factor;
  bias = (double) fparams->bias / (double) fparams->factor;

  memset((void *) *stats,
	 (int) 0,
	 (size_t) (Glob->params.grid.ny *
		   Glob->params.grid.nx * sizeof(mean_stats_t)));

  /*
   * loop through stats grid
   */

  high_y = Glob->params.grid.miny - (0.5 * Glob->params.grid.dy);
  high_iy = (si32) floor ((high_y - cart_miny) / cart_dy);
    
  for (iy = 0; iy < Glob->params.grid.ny; iy++) {

    /*
     * compute y limits for data plane
     */

    low_iy = high_iy + 1;
    high_y += Glob->params.grid.dy;
    high_iy = (si32) floor ((high_y - cart_miny) / cart_dy);
      
    if (low_iy >= 0 && high_iy < cart->ny) {

      high_x = Glob->params.grid.minx - (0.5 * Glob->params.grid.dx);
      high_ix = (si32) floor ((high_x - cart_minx) / cart_dx);
      
      for (ix = 0; ix < Glob->params.grid.nx; ix++) {
      
	/*
	 * compute x limits for data plane
	 */
	
	low_ix = high_ix + 1;
	high_x += Glob->params.grid.dx;
	high_ix = (si32) floor ((high_x - cart_minx) / cart_dx);
      
	if (low_ix >= 0 && high_ix < cart->nx) {

	  /*
	   * loop through the relevant portion of the data
	   * plane, computing the mean
	   */
      
	  st = stats[iy] + ix;
    
	  for (jy = low_iy; jy <= high_iy; jy++) {
	    
	    dp = data_plane + jy * cart->nx + low_ix;
	  
	    for (jx = low_ix; jx <= high_ix; jx++) {
	      
	      data_val = (double) *dp * scale + bias;
	      
	      st->n++;
	      st->sum += data_val;
	      dp++;

	    } /* jx */
	    
	  } /* jy */

	  /*
	   * compute mean
	   */

	  if (st->n > 0)
	    st->mean = st->sum / (double) st->n;
	  
	} /* if (low_ix >= 0 && high_ix < cart->nx) */

      } /* ix */ 

    } /* if (low_iy >= 0 && high_iy < cart->ny) */

  } /* iy */

}
