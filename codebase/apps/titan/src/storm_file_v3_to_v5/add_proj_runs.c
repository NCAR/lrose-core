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
 * add_proj_runs.c
 *
 * RAP, NCAR, Boulder CO
 *
 * Jan 1996
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "storm_file_v3_to_v5.h"
#include <euclid/boundary.h>
#include <euclid/point.h>
#include <euclid/node.h>

void add_proj_runs(storm_file_handle_t *s_handle,
		   storm_file_global_props_t *gprops)

{

  static Interval *Intervals = NULL;
  static int N_intervals_alloc = 0;
  static Row_hdr *Rowh = NULL;
  static int Nrows_alloc = 0;
  static int Ngrid_alloc = 0;
  static ui08 *Grid = NULL;

  int irun;
  si32 offset;
  si32 min_ix, min_iy, max_ix, max_iy;
  si32 n, nx, ny;
  si32 num_intervals;

  Interval *intvl;
  storm_file_run_t *run;

  if (gprops->n_runs == 0) {
    gprops->n_proj_runs = 0;
    return;
  }

  /*
   * get the limits of the proj area
   */
  
  min_ix = gprops->bounding_min_ix;
  min_iy = gprops->bounding_min_iy;
  max_ix = gprops->bounding_max_ix;
  max_iy = gprops->bounding_max_iy;

  /*
   * compute grid sizes, and set grid params
   */

  nx = max_ix - min_ix + 1;
  ny = max_iy - min_iy + 1;
  n = nx * ny;

  /*
   * check grid memory allocation
   */

  if (n > Ngrid_alloc) {
    if (Grid == NULL) {
      Grid = (ui08 *) umalloc((ui32) (n * sizeof(ui08)));
    } else {
      Grid = (ui08 *) urealloc((char *) Grid,
				 (ui32) (n * sizeof(ui08)));
    }
    Ngrid_alloc = n;
  }

  /*
   * zero out grid for proj area comps
   */

  memset((void *) Grid, 0, (int) n);
  
  /*
   * load up grid with 1's to indicate projected area
   */

  run = s_handle->runs;
  for (irun = 0; irun < gprops->n_runs; irun++, run++) {
    offset = ((run->iy - min_iy) * nx) + (run->ix - min_ix);
    memset((void *) (Grid + offset), 1, (int) run->n);
  }
  
  /*
   * check memory allocation
   */

  EG_alloc_rowh((int) ny, &Nrows_alloc, &Rowh);

  /*
   * get the intervals
   */
  
  num_intervals = EG_find_intervals(ny, nx, Grid,
				    &Intervals, &N_intervals_alloc,
				    Rowh, 1);

  /*
   * alloc for proj runs
   */
  
  gprops->n_proj_runs = num_intervals;

  RfAllocStormProps(s_handle, s_handle->scan->grid.nz,
		    gprops->n_dbz_intervals,
		    gprops->n_runs, gprops->n_proj_runs,
		    "add_proj_runs");

  /*
   * load up the proj runs
   */
  
  run = s_handle->proj_runs;
  intvl = Intervals;
  
  for (irun = 0; irun < gprops->n_proj_runs;
       irun++, run++, intvl++) {
    
    run->ix = intvl->begin + min_ix;
    run->iy = intvl->row_in_plane + min_iy;
    run->iz = 0;
    run->n = intvl->len;

  } /* irun */

  return;

}

