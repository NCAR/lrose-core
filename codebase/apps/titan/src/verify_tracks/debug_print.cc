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
 * debug_print.c
 *
 * debug printout of grids and other data
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * January 1992
 *
 *****************************************************************************/

#include "verify_tracks.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void print_grid(const char *label,
		       ui08 **forecast_grid,
		       ui08 **verify_grid);

void debug_print(track_file_handle_t *t_handle,
		 vt_simple_track_t *stracks,
		 long ncurrent,
		 vt_entry_index_t *current,
		 ui08 **forecast_grid,
		 ui08 **runs_truth_grid,
		 vt_count_t *count)

{
  
  long i;
  vt_simple_track_t *strack;
  vt_storm_t *storm;

  fprintf(stderr, "Complex track number %ld\n",
	  (long) t_handle->complex_params->complex_track_num);
    
  fprintf(stderr, "From %s to %s\n",
	  utimstr(t_handle->complex_params->start_time),
	  utimstr(t_handle->complex_params->end_time));
    
  fprintf(stderr, "%ld current storm(s)\n", ncurrent);
    
  for (i = 0; i < ncurrent; i++) {

    strack = stracks + current[i].isimple;
    storm = strack->storms + current[i].istorm;

    fprintf(stderr,
	    "Simple num %ld, ientry %ld, scan %ld\n",
	    (long) strack->params.simple_track_num,
	    current[i].istorm,
	    (long) storm->entry.scan_num);

  } /* i */

  print_grid("Verification grid:",
	     forecast_grid, runs_truth_grid);

  print_contingency_table(stderr, "Contingency table:", count);

}

/***************************************************************************
 * print_grid()
 */

static void print_grid(const char *label,
		       ui08 **forecast_grid,
		       ui08 **verify_grid)

{

  int info_found;
  long count;
  long ix, iy;
  long min_ix, max_ix, nx_active;

  /*
   * search for max and min x activity
   */

  min_ix = LARGE_LONG;
  max_ix = 0;
  
  for (iy = Glob->ny - 1; iy >= 0; iy--) {

    for (ix = 0; ix < Glob->nx; ix++) {

      if (verify_grid[iy][ix] ||
	  forecast_grid[iy][ix]) {
	
	min_ix = MIN(min_ix, ix);
	max_ix = MAX(max_ix, ix);
	
      } /* if */
      
    } /* ix */

  } /* iy */

  nx_active = max_ix - min_ix + 1;

  /*
   * print header
   */

  fprintf(stderr, "\n%s\n", label);
  fprintf(stderr, "start_ix = %ld\n", min_ix);

  fprintf(stderr, "     ");
  count = nx_active % 10;
  for (ix = 0; ix < nx_active; ix++) {
    if (count < 10) {
      fprintf(stderr, " ");
    } else {
      fprintf(stderr, "|");
      count = 0;
    }
    count++;
  } /* ix */

  fprintf(stderr, "\n");

  /*
   * print grid
   */

  for (iy = Glob->ny - 1; iy >= 0; iy--) {

    info_found = FALSE;

    for (ix = min_ix; ix <= max_ix; ix++)
      if (verify_grid[iy][ix] ||
	  forecast_grid[iy][ix])
	info_found = TRUE;

    if (info_found) {

      fprintf(stderr, "%4ld ", iy);
    
      for (ix = min_ix; ix <= max_ix; ix++) {

	if (verify_grid[iy][ix] > 0 &&
	    forecast_grid[iy][ix] > 0) {

	  fprintf(stderr, "S");

	} else if (verify_grid[iy][ix] > 0 &&
		   forecast_grid[iy][ix] == 0) {

	  fprintf(stderr, "F");
	  
	  
	} else if (verify_grid[iy][ix] == 0 &&
		   forecast_grid[iy][ix] > 0) {

	  fprintf(stderr, "A");
	  
	} else {

	  fprintf(stderr, "-");
	  
	}
	
      } /* ix */

      fprintf(stderr, "\n");

    } /* if (info_found) */

  } /* iy */

}
