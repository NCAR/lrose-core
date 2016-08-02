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
/**************************************************************************
 * compute_stats.c
 *
 * Compute the stats in the grid, load up the fields
 *
 * Mike Dixon RAP NCAR Boulder CO 80307 USA
 *
 * August 1993
 *
 **************************************************************************/

#include "track_grid_stats.h"

void compute_stats(vol_file_handle_t *v_handle,
		   grid_stats_t **stats,
		   si32 n_scans_processed)

{

  ui08 *uptr;
  ui08 *data_plane;
  
  si32 ix, iy;
  si32 ifield;

  double *valptr;
  double minval, maxval;
  double tmp_scale, tmp_bias;
  double scale, bias;

  grid_stats_t *stat;
  field_params_t *fparams;
  
  /*
   * loop through grid, computing stats
   */

  for (iy = 0; iy < Glob->params.grid.ny; iy++) {
    
    stat = stats[iy];
    
    for (ix = 0; ix < Glob->params.grid.nx; ix++) {
      
      if (stat->n_weighted > 0) {
	stat->percent_activity =
	  ((stat->n_weighted / (double) n_scans_processed) * 100.0);
	stat->u /= stat->n_weighted;
	stat->v /= stat->n_weighted;
	stat->speed /= stat->n_weighted;
	stat->volume /= stat->n_weighted;
	stat->area /= stat->n_weighted;
	stat->ln_area /= stat->n_weighted;
	stat->dbz_max /= stat->n_weighted;
	stat->tops /= stat->n_weighted;
	stat->duration /= stat->n_weighted;
      }

      if (stat->n_complex > 0) {
	stat->distance /= stat->n_complex;
	stat->dx /= stat->n_complex;
	stat->dy /= stat->n_complex;
      }

      if (Glob->params.n_seasons > 0) {
	stat->precip /= (double) Glob->params.n_seasons;
	stat->n_events /= (double) Glob->params.n_seasons;
	stat->n_weighted /= (double) Glob->params.n_seasons;
	stat->n_complex /= (double) Glob->params.n_seasons;
	stat->n_start /= (double) Glob->params.n_seasons;
	stat->n_mid /= (double) Glob->params.n_seasons;
      }
      
      stat++;
      
    } /* ix */
    
  } /* iy */

  /*
   * loop through fields
   */

  for (ifield = 0; ifield < N_STATS_FIELDS; ifield++) {

    fparams = v_handle->field_params[ifield];
    data_plane = v_handle->field_plane[ifield][0];
    
    /*
     * determine the min and max values in the field
     */

    minval = 1.0e100;
    maxval = -1.0e100;
    
    for (iy = 0; iy < Glob->params.grid.ny; iy++) {
      
      stat = stats[iy];
    
      for (ix = 0; ix < Glob->params.grid.nx; ix++) {
	
	valptr = (double *) stat + ifield;
	
	if (*valptr > maxval)
	  maxval = *valptr;
	
	if (*valptr != 0.0 && *valptr < minval)
	  minval = *valptr;

	stat++;
	
      } /* ix */
      
    } /* iy */
    
    /*
     * compute the scale and bias - the data must be scaled between
     * 1 and 255 for dobson files. 0 is reserved for the missing value.
     */

    minval -= fabs(minval) * 0.1;
    maxval += fabs(maxval) * 0.1;

    tmp_scale = (maxval - minval) / 254.0;
    tmp_bias = minval - 2.0 * tmp_scale;

    fparams->bias = (si32) floor (tmp_bias * (double) fparams->factor + 0.5);
    bias = (double) fparams->bias / (double) fparams->factor;
    
    fparams->scale = (si32) floor (tmp_scale * (double) fparams->factor + 0.5);
    scale = (double) fparams->scale / (double) fparams->factor;
    
    if (Glob->params.debug >= DEBUG_NORM) {

      fprintf(stderr, "\nField : %ld\n", (long) ifield);
      fprintf(stderr, "Min, max : %g, %g\n", minval, maxval);
      fprintf(stderr, "Bias, scale : %g, %g\n", bias, scale);

    }

    /*
     * store the scaled data in the field plane
     */

    uptr = data_plane;

    for (iy = 0; iy < Glob->params.grid.ny; iy++) {
      
      stat = stats[iy];
    
      for (ix = 0; ix < Glob->params.grid.nx; ix++) {
	
	valptr = (double *) stat + ifield;

	if (*valptr == 0.0)
	  *uptr = 0;
	else
	  *uptr = (ui08) ((*valptr - bias) / scale + 0.5);

	stat++;
	uptr++;
	
      } /* ix */
      
    } /* iy */
    
  } /* ifield */
  
}

