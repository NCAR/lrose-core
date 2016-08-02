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
 * grid.c
 *
 * grid module - allocates grid, loads grids and
 * performs grid computations
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000
 *
 * July 1995
 *
 * Notes:
 *
 * Forecast2 and Verify2 are 2-dim ui08 arrays.
 * Forecast and Verify are the pointers to the 1-dim ui08 arrays
 * upon which Forecast2 and Verify2 are based.
 *
 ****************************************************************************/

#include "forecast_monitor.h"

static ui08 *Forecast = NULL;
static ui08 *Verify = NULL;
static ui08 **Forecast2 = NULL;
static ui08 **Verify2 = NULL;
static si32 Nx, Ny;
static si32 Min_ix, Max_ix;
static si32 Min_iy, Max_iy;
static double Min_x, Min_y;
static double Dx, Dy;
static ucont_table_t Cont;

void init_grids(void)

{

  Nx = Glob->params.verify_grid.nx;
  Ny = Glob->params.verify_grid.ny;
  Min_x = Glob->params.verify_grid.min_x;
  Min_y = Glob->params.verify_grid.min_y;
  Dx = Glob->params.verify_grid.dx;
  Dy = Glob->params.verify_grid.dy;

  Forecast2 = (ui08 **) umalloc2(Ny, Nx, sizeof(ui08));
  Verify2 = (ui08 **) umalloc2(Ny, Nx, sizeof(ui08));
  Forecast = Forecast2[0];
  Verify = Verify2[0];
  
}

void clear_grids(void)

{
  memset(Forecast, 0, Nx * Ny * sizeof(ui08));
  memset(Verify, 0, Nx * Ny * sizeof(ui08));
  Min_ix = Nx - 1;
  Max_ix = 0;
  Min_iy = Ny - 1;
  Max_iy = 0;
}

void load_forecast_grid(storm_file_handle_t *s_handle,
			track_file_handle_t *t_handle,
			fm_simple_track_t *strack,
			double lead_time)

{

  si32 irun, ix;
  si32 jx, jy;

  double x, y;
  double lead_time_hr;
  double dx_dt, dy_dt, darea_dt;
  double current_x, current_y, current_area;
  double forecast_x, forecast_y, delta_area;
  double forecast_area;
  double grid_x, grid_y;
  double area_ratio, length_ratio;

  storm_file_run_t *run;
  titan_grid_t *grid;
  storm_file_global_props_t *gprops;
  track_file_forecast_props_t *fprops;

  grid = &strack->generate.scan.grid;

  /*
   * compute current posn and area
   */

  gprops = &strack->generate.gprops;

  current_x = gprops->proj_area_centroid_x;
  current_y = gprops->proj_area_centroid_y;
  current_area = gprops->proj_area;
  
  /*
   * compute forecast props in floating point
   */

  fprops = &strack->generate.entry.dval_dt;
  
  dx_dt = fprops->proj_area_centroid_x;
  dy_dt = fprops->proj_area_centroid_y;
  darea_dt = fprops->proj_area;

  /*
   * compute the forecast storm position and area
   */
  
  lead_time_hr = lead_time / 3600.0;
  
  forecast_x = current_x + dx_dt * lead_time_hr;
  forecast_y = current_y + dy_dt * lead_time_hr;
  delta_area = darea_dt * lead_time_hr;

  forecast_area = current_area + delta_area;
  if (forecast_area <= 0.0) {
    return;
  }
  
  /*
   * compute centroid posn in terms of the cartesian grid
   */
  
  grid_x = (current_x - grid->minx) / grid->dx;
  grid_y = (current_y - grid->miny) / grid->dy;
  
  /*
   * Adjust the cart parameters for storm motion and growth
   */
  
  area_ratio = forecast_area / current_area;
  length_ratio = sqrt(area_ratio);

  grid->dx *= length_ratio;
  grid->dy *= length_ratio;
  grid->minx = forecast_x - (grid_x * grid->dx);
  grid->miny = forecast_y - (grid_y * grid->dy);

  /*
   * loop through runs, loading up forecast grid
   */

  run = strack->generate.runs;
  for (irun = 0; irun < strack->generate.n_runs; irun++, run++) {

    y = run->iy * grid->dy + grid->miny;
    x = run->ix * grid->dx + grid->minx;

    for (ix = 0; ix < run->n; ix++, x += grid->dx) {

      jy = (si32) ((y - Min_y) / Dy + 0.5);
      jx = (si32) ((x - Min_x) / Dx + 0.5);

      if (jy >= 0 && jy < Ny && jx >= 0 && jx < Nx) {
	Forecast2[jy][jx] = 1;
	Min_ix = MIN(Min_ix, jx);
	Max_ix = MAX(Max_ix, jx);
	Min_iy = MIN(Min_iy, jy);
	Max_iy = MAX(Max_iy, jy);
      }

    } /* ix */

  } /* irun */

}

void load_verify_grid(fm_simple_track_t *strack)

{

  si32 irun, ix;
  si32 jx, jy;
  double x, y;
  storm_file_run_t *run;
  titan_grid_t *grid;

  grid = &strack->verify.scan.grid;

  /*
   * loop through runs, loading up verify grid
   */

  run = strack->verify.runs;
  for (irun = 0; irun < strack->verify.n_runs; irun++, run++) {

    y = run->iy * grid->dy + grid->miny;
    x = run->ix * grid->dx + grid->minx;

    for (ix = 0; ix < run->n; ix++, x += grid->dx) {

      jy = (si32) ((y - Min_y) / Dy + 0.5);
      jx = (si32) ((x - Min_x) / Dx + 0.5);

      if (jy >= 0 && jy < Ny && jx >= 0 && jx < Nx) {
	Verify2[jy][jx] = 1;
	Min_ix = MIN(Min_ix, jx);
	Max_ix = MAX(Max_ix, jx);
	Min_iy = MIN(Min_iy, jy);
	Max_iy = MAX(Max_iy, jy);
      }

    } /* ix */

  } /* irun */

}

/*
 * compute_grid_stats()
 *
 * Returns 1 if points found, 0 otherwise
 */

int compute_grid_stats(double *pod_p, double *far_p, double *csi_p)

{

  si32 ix, iy;

  Cont.n_success = 0.0;
  Cont.n_failure = 0.0;
  Cont.n_false_alarm = 0.0;
  Cont.n_non_event = 0.0;

  for (iy = Max_iy; iy >= Min_iy; iy--) {
    for (ix = Min_ix; ix <= Max_ix; ix++) {
      if (Verify2[iy][ix] > 0 &&
	  Forecast2[iy][ix] > 0) {
	Cont.n_success++;
      } else if (Verify2[iy][ix] > 0 &&
		 Forecast2[iy][ix] == 0) {
	Cont.n_failure++;
      } else if (Verify2[iy][ix] == 0 &&
		 Forecast2[iy][ix] > 0) {
	Cont.n_false_alarm++;
      } else {
	Cont.n_non_event++;
      }
    } /* ix */
  } /* iy */

  ucompute_cont(&Cont);

  *pod_p = Cont.pod;
  *far_p = Cont.far;
  *csi_p = Cont.csi;

  if (Cont.n_success > 0 ||
      Cont.n_failure > 0 ||
      Cont.n_false_alarm > 0) {
    return (1);
  } else {
    return (0);
  }
  
}

void print_cont(void) {

  uprint_cont(&Cont, stdout, "", "      ", FALSE, FALSE);
  
}
  
void print_grids(void)

{

  int info_found;
  si32 count;
  si32 ix, iy;
  si32 nx_active;

  nx_active = Max_ix - Min_ix + 1;

  fprintf(stdout, "\n");

  /*
   * print header in debug mode
   */

  if (Glob->params.debug >= DEBUG_NORM) {
    fprintf(stdout, "Min_ix, Max_ix: %ld, %ld\n",
	    (long) Min_ix, (long) Max_ix);
    fprintf(stdout, "Min_iy, Max_iy: %ld, %ld\n",
	    (long) Min_iy, (long) Max_iy);
    fprintf(stdout, "     ");
    count = nx_active % 10;
    for (ix = 0; ix < nx_active; ix++) {
      if (count < 10) {
	fprintf(stdout, " ");
      } else {
	fprintf(stdout, "|");
	count = 0;
      }
      count++;
    } /* ix */
    fprintf(stdout, "\n");
  }

  /*
   * print grid
   */

  for (iy = Max_iy; iy >= Min_iy; iy--) {
    info_found = FALSE;
    for (ix = Min_ix; ix <= Max_ix; ix++)
      if (Verify2[iy][ix] ||
	  Forecast2[iy][ix])
	info_found = TRUE;
    if (info_found) {
      if (Glob->params.debug >= DEBUG_NORM) {
	fprintf(stdout, "%4ld  ", (long) iy);
      } else {
	fprintf(stdout, "      ");
      }
      for (ix = Min_ix; ix <= Max_ix; ix++) {
	if (Verify2[iy][ix] > 0 &&
	    Forecast2[iy][ix] > 0) {
	  fprintf(stdout, "S");
	} else if (Verify2[iy][ix] > 0 &&
		   Forecast2[iy][ix] == 0) {
	  fprintf(stdout, "F");
	} else if (Verify2[iy][ix] == 0 &&
		   Forecast2[iy][ix] > 0) {
	  fprintf(stdout, "A");
	} else {
	  fprintf(stdout, "-");
	}
      } /* ix */
      fprintf(stdout, "\n");
    } /* if (info_found) */
  } /* iy */

}

