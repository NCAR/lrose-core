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
 * update_forecast_grid.c
 *
 * Flags the forecast grid with 255 at each current storm location.
 * Then places the storm data, moved by forecast_dx and forecast_dy,
 * in the forecast grid.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1992
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "grid_forecast.hh"

void update_forecast_grid(storm_file_handle_t *s_handle,
			  vol_file_handle_t *v_handle,
			  ui08 *forecast_grid,
			  si32 n_runs,
			  double forecast_dx,
			  double forecast_dy)

{

  ui08 *dbz_grid;

  si32 jy, irun;
  si32 start_ix, end_ix;
  si32 index, jndex;
  si32 nx, ny;
  si32 dix, diy;

  double dx, dy;

  titan_grid_t *grid;
  storm_file_run_t *run;

  dbz_grid = v_handle->field_plane[Glob->dbz_field][0];

  /*
   * compute cartesian params
   */

  grid = &s_handle->scan->grid;
  nx = grid->nx;
  ny = grid->ny;
  dx = grid->dx;
  dy = grid->dy;

  /*
   * compute the grid steps by which the storm is to be moved
   */

  if (forecast_dx >= 0)
    dix = (si32) (forecast_dx / dx + 0.5);
  else
    dix = (si32) (forecast_dx / dx - 0.5);

  if (forecast_dy >= 0)
    diy = (si32) (forecast_dy / dy + 0.5);
  else
    diy = (si32) (forecast_dy / dy - 0.5);

  /*
   * load present storm locations with flag
   */

  for (irun = 0; irun < n_runs; irun++) {
    
    run = s_handle->runs + irun;

    index = run->iy * nx + run->ix;

    memset((char *) (forecast_grid + index),
	   (int) DATA_MOVED_FLAG, (int) run->n);

  } /* irun */

  /*
   * load forecast locations
   */
  
  for (irun = 0; irun < n_runs; irun++) {
    
    run = s_handle->runs + irun;

    jy = run->iy + diy;
    
    if (jy >= 0 && jy < ny) {

      start_ix = run->ix + dix;
      end_ix = start_ix + run->n - 1;

      if (start_ix < nx - 1 && end_ix > 0) {

	start_ix = MAX(start_ix, 0);
	end_ix = MIN(end_ix, ny - 1);

	index = run->iy * nx + run->ix;
	jndex = jy * nx + start_ix;

	memcpy ((void *) (forecast_grid + jndex),
		(void *) (dbz_grid + index),
		(size_t) (end_ix - start_ix + 1));

      } /* if (start_ix ... */

    } /* if (jy ... */

  } /* irun */
      
}
