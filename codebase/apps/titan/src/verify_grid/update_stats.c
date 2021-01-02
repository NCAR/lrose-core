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
 * update_stats.c
 *
 * Performs verification on the file
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * July 1992
 *
 ***********************************************************************/

#include "verify_grid.h"

void update_stats(char *detect_file_path,
		  statistics_t *stats)

{
  
  static vol_file_handle_t truth_index;
  static vol_file_handle_t detect_index;
  static int first_call = TRUE;
  
  char *truth_file_path;
  ui08 *truth_plane, *detect_plane;

  si32 i, interval;
  si32 npoints;

  double truth_scale, truth_bias;
  double detect_scale, detect_bias;
  double truth_val, detect_val;

  cart_params_t *truth_cart;
  cart_params_t *detect_cart;

  field_params_t *truth_field;
  field_params_t *detect_field;

  /*
   * initialize
   */
  
  if (first_call) {
    
    init_indices(&truth_index, &detect_index);
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

  if (RfReadVolume(&detect_index, "update_stats") != R_SUCCESS)
    tidy_and_exit(-1);

  truth_index.vol_file_path = truth_file_path;
  
  if (RfReadVolume(&truth_index, "update_stats") != R_SUCCESS)
    tidy_and_exit(-1);

  /*
   * check that the cartesian grids match
   */

  truth_cart = &truth_index.vol_params->cart;
  detect_cart = &detect_index.vol_params->cart;

  if (truth_cart->nx != detect_cart->nx ||
      truth_cart->ny != detect_cart->ny) {
    
    fprintf(stderr, "ERROR - %s:update_stats\n", Glob->prog_name);
    fprintf(stderr, "Cartesian grids do not match\n\n");

    fprintf(stderr, "\nFile '%s'\n", detect_file_path);
    print_cart_params(&detect_index.vol_params->cart);

    fprintf(stderr, "\nFile '%s'\n", truth_file_path);
    print_cart_params(&truth_index.vol_params->cart);

    tidy_and_exit(-1);

  }

  /*
   * check that there is only 1 plane
   */

  if (truth_cart->nz != 1) {

    fprintf(stderr, "ERROR - %s:update_stats\n", Glob->prog_name);
    fprintf(stderr, "Truth data has %ld planes - should be 1\n",
	    (long) truth_cart->nz);
    tidy_and_exit(-1);

  }

  if (detect_cart->nz != 1) {

    fprintf(stderr, "ERROR - %s:update_stats\n", Glob->prog_name);
    fprintf(stderr, "Detection data has %ld planes - should be 1\n",
	    (long) detect_cart->nz);
    tidy_and_exit(-1);
    
  }

  /*
   * check that field number is valid
   */

  if (Glob->params.truth_field > truth_index.vol_params->nfields - 1) {

    fprintf(stderr, "ERROR - %s:update_stats\n", Glob->prog_name);
    fprintf(stderr, "Truth data field %ld too large\n",
	    Glob->params.truth_field);
    fprintf(stderr, "Max allowed is %ld\n",
	    (long) (truth_index.vol_params->nfields - 1));
    tidy_and_exit(-1);

  }

  if (Glob->params.detect_field > detect_index.vol_params->nfields - 1) {

    fprintf(stderr, "ERROR - %s:update_stats\n", Glob->prog_name);
    fprintf(stderr, "Detect data field %ld too large\n",
	    Glob->params.detect_field);
    fprintf(stderr, "Max allowed is %ld\n",
	    (long) (detect_index.vol_params->nfields - 1));
    tidy_and_exit(-1);

  }

  /*
   * accumulate statistics data
   */

  truth_field = truth_index.field_params[Glob->params.truth_field];
  truth_scale =
    (double) truth_field->scale / (double) truth_field->factor;
  truth_bias =
    (double) truth_field->bias / (double) truth_field->factor;

  detect_field = detect_index.field_params[Glob->params.detect_field];
  detect_scale =
    (double) detect_field->scale / (double) detect_field->factor;
  detect_bias =
    (double) detect_field->bias / (double) detect_field->factor;
  
  truth_plane = truth_index.field_plane[Glob->params.truth_field][0];
  detect_plane = detect_index.field_plane[Glob->params.detect_field][0];
  
  npoints = truth_cart->nx * truth_cart->ny;

  for (i = 0; i < npoints; i++) {

    truth_val = (double) *truth_plane * truth_scale + truth_bias;
    detect_val = (double) *detect_plane * detect_scale + detect_bias;

    if (detect_val >= Glob->params.detect_level_lower &&
	detect_val <= Glob->params.detect_level_upper) {
    
      interval = (si32)
	floor ((truth_val - stats->hist_low_limit) /
	       stats->hist_interval_size);

      if (interval >= 0 && interval < stats->hist_n_intervals) {
	
	stats->n_total += 1.0;
	stats->n_per_interval[interval] += 1.0;
	stats->sumx += truth_val;
	stats->sum2x += truth_val * truth_val;
	
      } /* if (interval ... */

    } /* if (detect_val ... */

    truth_plane++;
    detect_plane++;

  } /* i */

}
