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
 * update_runs_grid.c
 *
 * Flags regions in the forecast grid with 1's if the runs
 * when moved by dx and dy exist in the region.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1992
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "verify_day.h"

void update_runs_grid(storm_file_handle_t *s_handle,
		      si32 n_runs,
		      double forecast_dx,
		      double forecast_dy,
		      ui08 **forecast_grid)

{

  si32 ix, jx, jy, irun;

  double x, y;

  titan_grid_t *grid;
  storm_file_run_t *run;

  /*
   * compute cartesian params
   */

  grid = &s_handle->scan->grid;

  /*
   * loop through runs
   */
      
  for (irun = 0; irun < n_runs; irun++) {
    
    run = s_handle->runs + irun;

    y = (double) run->iy * grid->dy + grid->miny + forecast_dy;
  
    jy = (si32) ((y - Glob->miny) / Glob->dy + 0.5);

    if (jy >= 0 && jy < Glob->ny) {

      for (ix = run->ix; ix < run->ix + run->n; ix++) {
	
	x = (double) ix * grid->dx + grid->minx + forecast_dx;
	jx = (si32) ((x - Glob->minx) / Glob->dx + 0.5);

	if (jx >= 0 && jx < Glob->nx)
	  forecast_grid[jy][jx] = 1;
	
      } /* ix */

    } /* if (jy >= 0 ... */

  } /* irun */
      
}
