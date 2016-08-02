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
 * identify.c
 *
 * identifies the data runs which make up a storm
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * April 1995
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "storm_ident.h"

void identify(vol_file_handle_t *v_handle,
	      storm_file_handle_t *s_handle,
	      si32 scan_num)

{

  static Row_hdr *Rowh = NULL;
  static int Nrows_alloc = 0;
  static Interval *Intervals = NULL;
  static int N_intervals_alloc = 0;

  static int N_ints_alloc_clump = 0;
  static Clump_order *Clumps = NULL;
  static Interval **Interval_order = NULL;

  ui08 *vol_array;

  int i, iz;
  int nplanes, nrows_per_vol, ncols;
  int nrows_per_plane;
  int npoints_per_plane;
  int num_clumps, num_intervals;

  Clump_order *clump;

  PMU_auto_register("Identifying storms");
    
  /*
   * initialize
   */

  ncols = Glob->nx;
  nrows_per_plane = Glob->ny;
  npoints_per_plane = nrows_per_plane * ncols;
  nplanes = Glob->max_valid_layer - Glob->min_valid_layer + 1;
  nrows_per_vol = nrows_per_plane * nplanes;

  /*
   * alloc as necessary
   */

  EG_alloc_rowh(nrows_per_vol, &Nrows_alloc, &Rowh);

  if (Glob->params.create_verification_files) {
    alloc_verification_grids();
  }

  if (Glob->params.create_morphology_files) {
    alloc_morphology_grids();
  }

  /*
   * create the composite grid
   */

  create_comp_grid(v_handle);

  /*
   * get array from which to determine intervals
   */
  
  if (nplanes > 1) {
      
    vol_array = ucalloc (npoints_per_plane * nplanes, 1);
    for (iz = 0; iz < nplanes; iz++) {
      memcpy((vol_array + iz * npoints_per_plane),
	     (v_handle->field_plane[Glob->params.dbz_field]
	      [iz + Glob->min_valid_layer]),
	     (int) npoints_per_plane);
    } /* iz */
    
  } else {
    
    vol_array =
      v_handle->field_plane[Glob->params.dbz_field][Glob->min_valid_layer];
    
  }
  
  /*
   * get intervals
   */

  if (Glob->params.debug >= DEBUG_VERBOSE) {
    fprintf(stderr, "Cpu time before find_intervals: %g secs\n",
	    (double) clock() / 1000000.0);
  }
  
  num_intervals = EG_find_intervals_3d(nplanes,
				       nrows_per_vol,
				       nrows_per_plane,
				       ncols,
				       vol_array,
				       &Intervals, &N_intervals_alloc,
				       Rowh,
				       Glob->low_dbz_byte);

  if (nplanes > 1) {
    ufree ((char *) vol_array);
  }
  
  if (Glob->params.debug >= DEBUG_VERBOSE) {
    fprintf(stderr, "Cpu time after find_intervals: %g secs\n",
	    (double) clock() / 1000000.0);
    fprintf(stderr, "Number of intervals = %d\n", num_intervals);
    fprintf(stderr, "Number of intervals allocated = %d\n",
	    N_intervals_alloc);
  }

  /*
   * allocate space for intervals and clumps
   */

  EG_alloc_clumps(num_intervals, &N_ints_alloc_clump,
		  &Clumps, &Interval_order);

  /*
   * set clump ids in new_intervals to NULL_ID
   */

  EG_reset_clump_id(Intervals, num_intervals);
  
  /*
   * clump
   */
  
  if (Glob->params.debug >= DEBUG_VERBOSE) {
    fprintf(stderr, "Cpu time before rclump_3d: %g secs\n",
	    (double) clock() / 1000000.0);
  }
  
  num_clumps = EG_rclump_3d(Rowh, nrows_per_plane, nplanes, TRUE, 1,
			    Interval_order, Clumps);

  if (Glob->params.debug >= DEBUG_VERBOSE) {
    fprintf(stderr, "Number of clumps  =  %d\n", num_clumps);
    fprintf(stderr, "Cpu time after rclump_3d: %g secs\n",
	    (double) clock() / 1000000.0);
  }

  /*
   * update the verification grid
   */
  
  if (Glob->params.create_verification_files) {
    clump = Clumps + 1;
    for (i = 0; i < num_clumps; i++, clump++) {
      update_all_storms_grid(clump);
    }
  }

  process_clumps(v_handle, s_handle, scan_num, nplanes,
		 num_clumps, Clumps);
  
  if (Glob->params.debug >= DEBUG_VERBOSE) {
    fprintf(stderr, "Cpu time after process_clumps: %g secs\n",
	    (double) clock() / 1000000.0);
  }
  
  if (Glob->params.create_verification_files) {
    write_verification_file(v_handle);
  }

  if (Glob->params.create_morphology_files) {
    write_morphology_file(v_handle);
  }

  return;

}
