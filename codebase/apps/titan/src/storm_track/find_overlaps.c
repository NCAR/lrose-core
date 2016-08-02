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
 * find_overlaps.c
 *
 * Load up the polygon overlap areas between forecast shape of storms at
 * time 1 and actual shape of storms at time 2.
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * March 1996
 *
 ****************************************************************************/

#include "storm_track.h"

/*
 * file scope prototypes
 */

static ui32 compute_overlap(ui08 *overlap_grid, ui32 npoints_grid);

static ui08 *init_tmp_grid(ui32 npoints_grid);

static ui08 *init_overlap_grid(ui32 npoints_grid);

static void load_overlaps(storm_file_handle_t *s_handle,
			  storm_status_t *storms1,
			  storm_status_t *storms2,
			  int istorm, int jstorm,
			  bounding_box_t *box1,
			  bounding_box_t *box2);

static ui32 load_current_poly(storm_file_handle_t *s_handle,
			      storm_status_t *storm,
			      bounding_box_t *both,
			      ui32 nx, ui32 ny,
			      ui08 *data_grid,
			      int val);

static ui32 load_current_runs(storm_status_t *storm,
			      bounding_box_t *both,
			      ui32 nx,
			      ui08 *data_grid,
			      int val);

static ui32 load_forecast_poly(storm_file_handle_t *s_handle,
			       storm_status_t *storm,
			       bounding_box_t *both,
			       ui32 nx, ui32 ny,
			       ui08 *data_grid,
			       int val);

static ui32 load_forecast_runs(storm_status_t *storm,
			       bounding_box_t *both,
			       ui32 nx,
			       ui32 ny,
			       titan_grid_t *grid,
			       ui08 *overlap_grid,
			       ui08 *tmp_grid);

static void print_overlap(ui08 *overlap_grid, ui32 nx, ui32 ny);

/*
 * main
 */

void find_overlaps(storm_file_handle_t *s_handle,
		   si32 nstorms1,
		   si32 nstorms2,
		   storm_status_t *storms1,
		   storm_status_t *storms2,
		   double d_hours)
     
{

  int istorm, jstorm;
  int overlap_in_x, overlap_in_y;
  int bounds_overlap;

  storm_status_t *storm1;
  storm_status_t *storm2;
  
  bounding_box_t *box2;
  bounding_box_t *box1;
  
  for (istorm = 0; istorm < nstorms1; istorm++) {
    
    storm1 = storms1 + istorm;

    for (jstorm = 0; jstorm < nstorms2; jstorm++) {
	
      storm2 = storms2 + jstorm;
	
      /*
       * check for overlap of bounding boxes
       */
	
      box1 = &storm1->box_for_overlap;
      box2 = &storm2->box_for_overlap;
	
      if (box1->min_ix <= box2->max_ix &&
	  box1->max_ix >= box2->min_ix) {
	overlap_in_x = TRUE;
      } else {
	overlap_in_x = FALSE;
      }

      if (box1->min_iy <= box2->max_iy &&
	  box1->max_iy >= box2->min_iy) {
	overlap_in_y = TRUE;
      } else {
	overlap_in_y = FALSE;
      }

      if (overlap_in_x && overlap_in_y) {
	bounds_overlap = TRUE;
      } else {
	bounds_overlap = FALSE;
      }

      if (bounds_overlap) {

	if (Glob->params.debug >= DEBUG_VERBOSE) {

	  fprintf(stderr, "bounding_boxes - time1:storm %d "
		  "overlaps with time2:storm %d\n",
		  istorm, jstorm);
	  
	  fprintf(stderr, "Storm 1 centroid, area: %g, %g, %g\n",
		  storm1->current.proj_area_centroid_x,
		  storm1->current.proj_area_centroid_y,
		  storm1->current.proj_area);
	  
	  fprintf(stderr, "Storm 2 centroid, area: %g, %g, %g\n",
		  storm2->current.proj_area_centroid_x,
		  storm2->current.proj_area_centroid_y,
		  storm2->current.proj_area);
	  
	  fprintf(stderr,
		  "Storm 1: box_min_ix, box_min_iy, box_max_ix, "
		  "box_max_iy: %ld, %ld, %ld, %ld\n",
		  (long) box1->min_ix,
		  (long) box1->min_iy,
		  (long) box1->max_ix,
		  (long) box1->max_iy);

	  fprintf(stderr,
		  "Storm 2: box_min_ix, box_min_iy, box_max_ix, "
		  "box_max_iy: %ld, %ld, %ld, %ld\n",
		  (long) box2->min_ix,
		  (long) box2->min_iy,
		  (long) box2->max_ix,
		  (long) box2->max_iy);

	  fprintf(stderr, "forecast_x, forecast_y: %g, %g\n",
		  storm1->track->forecast_x, storm1->track->forecast_y);

	  fprintf(stderr, "forecast_area, length_ratio: %g, %g\n",
		  storm1->track->forecast_area,
		  storm1->track->forecast_length_ratio);
	  
	} /* if (Glob->params.debug >= DEBUG_VERBOSE) */

	load_overlaps(s_handle, storms1, storms2,
		      istorm, jstorm,
		      box1, box2);

      } /* if (bounds_overlap) */
      
    } /* jstorm */

  } /* istorm */

  return;

}

/*******************
 * compute_overlap()
 */

static ui32 compute_overlap(ui08 *overlap_grid, ui32 npoints_grid)

{

  ui32 i;
  ui32 count = 0;

  for (i = 0; i < npoints_grid; i++, overlap_grid++) {
    if (*overlap_grid == 3) {
      count++;
    }
  }
  
  return (count);
  
}

/**********************
 * init_tmp_grid()
 *
 * alloc and init the forecast grid
 */

static ui08 *init_tmp_grid(ui32 npoints_grid)

{

  static ui32 n_alloc = 0;
  static ui08 *grid_array = NULL;

  ui32 nbytes = npoints_grid * sizeof(ui08);

  if (nbytes > n_alloc) {
    if (grid_array == NULL) {
      grid_array = (ui08 *) umalloc(nbytes);
    } else {
      grid_array = (ui08 *) urealloc((char *) grid_array, nbytes);
    }
    n_alloc = nbytes;
  }

  memset((void *) grid_array, 0, nbytes);
  
  return (grid_array);

}

/**********************
 * init_overlap_grid()
 *
 * alloc and init the overlap grid
 */

static ui08 *init_overlap_grid(ui32 npoints_grid)

{

  static ui32 n_alloc = 0;
  static ui08 *grid_array = NULL;

  ui32 nbytes = npoints_grid * sizeof(ui08);

  if (nbytes > n_alloc) {
    if (grid_array == NULL) {
      grid_array = (ui08 *) umalloc(nbytes);
    } else {
      grid_array = (ui08 *) urealloc((char *) grid_array, nbytes);
    }
    n_alloc = nbytes;
  }

  memset((void *) grid_array, 0, nbytes);
  
  return (grid_array);

}

/***************************************************
 * load_current_poly()
 *
 * Load up grid with polygon in the current position
 *
 * Returns number of points touched in grid
 */

static ui32 load_current_poly(storm_file_handle_t *s_handle,
			      storm_status_t *storm,
			      bounding_box_t *both,
			      ui32 nx, ui32 ny,
			      ui08 *data_grid,
			      int val)

{

  int i;
  long count;
  double ray_len;
  double *ray;
  double az, delta_az;

  Point_d *point;
  Point_d poly[N_POLY_SIDES + 1];

  storm_file_params_t *sparams = &s_handle->header->params;
  titan_grid_t *grid = &s_handle->scan->grid;

  /*
   * load up poly - this is the polygon from the current
   * position of the storm
   */
  
  point = poly;
  ray = storm->current.proj_area_rays;
  az = sparams->poly_start_az * DEG_TO_RAD;
  delta_az = sparams->poly_delta_az * DEG_TO_RAD;
  
  for (i = 0; i < N_POLY_SIDES;
       i++, point++, ray++, az += delta_az) {

    ray_len = *ray;
    point->x = (storm->current.proj_area_centroid_x +
		sin(az) * ray_len * grid->dx);
    point->y = (storm->current.proj_area_centroid_y +
		cos(az) * ray_len * grid->dy);
    
  } /* i */

  poly[N_POLY_SIDES] = poly[0];

  count = EG_fill_polygon(poly, N_POLY_SIDES + 1,
			  nx, ny,
			  grid->minx + both->min_ix * grid->dx,
			  grid->miny + both->min_iy * grid->dy,
			  grid->dx, grid->dy,
			  data_grid, val);

  return (count);

}

/**************************************************
 * load_current_runs()
 *
 * Load up grid with runs in their current position
 *
 * Returns number of points touched in grid
 */

static ui32 load_current_runs(storm_status_t *storm,
			      bounding_box_t *both,
			      ui32 nx,
			      ui08 *data_grid,
			      int val)

{

  ui08 *gp;
  ui32 i, irun;
  ui32 index;
  ui32 count = 0;
  storm_file_run_t *run;

  run = storm->proj_runs;
  for (irun = 0; irun < storm->n_proj_runs; irun++, run++) {

    index = (((run->iy - both->min_iy) * nx) +
	     (run->ix - both->min_ix));

    gp = data_grid + index;

    for (i = 0; i < run->n; i++, gp++) {
      *gp += val;
      count++;
    } /* i */

  } /* irun */
  
  return (count);
  
}

/*****************************************************
 * load_forecast_poly()
 *
 * Loads up grid with polygon in the forecast position
 *
 * Returns number of points touched in grid
 */

static ui32 load_forecast_poly(storm_file_handle_t *s_handle,
			       storm_status_t *storm,
			       bounding_box_t *both,
			       ui32 nx, ui32 ny,
			       ui08 *data_grid,
			       int val)

{

  int i;
  long count;
  double ray_len;
  double *ray;
  double az, delta_az;

  Point_d *point;
  Point_d poly[N_POLY_SIDES + 1];

  storm_file_params_t *sparams = &s_handle->header->params;
  titan_grid_t *grid = &s_handle->scan->grid;

  /*
   * load up poly - this must take into account the movement
   * of the storm
   */
  
  point = poly;
  ray = storm->current.proj_area_rays;
  az = sparams->poly_start_az * DEG_TO_RAD;
  delta_az = sparams->poly_delta_az * DEG_TO_RAD;
  
  for (i = 0; i < N_POLY_SIDES;
       i++, point++, ray++, az += delta_az) {

    ray_len = *ray * storm->track->forecast_length_ratio;
    point->x = storm->track->forecast_x + sin(az) * ray_len * grid->dx;
    point->y = storm->track->forecast_y + cos(az) * ray_len * grid->dy;
    
  } /* i */

  poly[N_POLY_SIDES] = poly[0];

  count = EG_fill_polygon(poly, N_POLY_SIDES + 1,
			  nx, ny,
			  grid->minx + both->min_ix * grid->dx,
			  grid->miny + both->min_iy * grid->dy,
			  grid->dx, grid->dy,
			  data_grid, val);

  return (count);

}

/***********************
 * load_forecast_runs()
 *
 * Load up overlap grid with storm runs from tmp grid,
 * taking into account storm motion and growth
 *
 * Returns number of points touched in overlap grid.
 */

static ui32 load_forecast_runs(storm_status_t *storm,
			       bounding_box_t *both,
			       ui32 nx, ui32 ny,
			       titan_grid_t *grid,
			       ui08 *overlap_grid,
			       ui08 *tmp_grid)
     
{

  si32 ix, jx, iy, jy;
  si32 index, jndex;
  ui32 count = 0;
  ui08 val;

  double x, y;
  double grid_ix, grid_iy;
  double xratio, yratio;
  double forecast_ix, forecast_iy;

  titan_grid_t fcast_grid;
  storm_track_props_t *current = &storm->current;
  track_status_t *track = storm->track;

  /*
   * compute current posn in terms of the grid
   */
  
  grid_ix = (current->proj_area_centroid_x - grid->minx) / grid->dx;
  grid_iy = (current->proj_area_centroid_y - grid->miny) / grid->dy;
  
  /*
   * Compute the cartesian grid parameters for the forecast time.
   * Here we are assuming that the storm grows or decays in area
   * but keeps the same shape. We use the fcast_grid as a computational
   * tool to compute the position of each grid forecast point
   */
      
  fcast_grid.dx = grid->dx * track->forecast_length_ratio;
  fcast_grid.dy = grid->dy * track->forecast_length_ratio;
  
  fcast_grid.minx = track->forecast_x - grid_ix * fcast_grid.dx;
  fcast_grid.miny = track->forecast_y - grid_iy * fcast_grid.dy;

  y = storm->box_for_overlap.min_iy * grid->dy + grid->miny;
  yratio = grid->dy / fcast_grid.dy;
  forecast_iy = (y - fcast_grid.miny) / fcast_grid.dy;
  
  for (iy = storm->box_for_overlap.min_iy;
       iy <= storm->box_for_overlap.max_iy;
       iy++, forecast_iy += yratio) {
    
    jy = (long) (forecast_iy + 0.5);

    if (jy > both->min_iy + ny - 1) {
      jy = both->min_iy + ny - 1;
    }
    if (jy < both->min_iy) {
      jy = both->min_iy;
    }

    x = storm->box_for_overlap.min_ix * grid->dx + grid->minx;
    xratio = grid->dx / fcast_grid.dx;
    forecast_ix = (x - fcast_grid.minx) / fcast_grid.dx;
    
    for (ix = storm->box_for_overlap.min_ix;
	 ix <= storm->box_for_overlap.max_ix;
	 ix++, forecast_ix += xratio) {
      
      jx = (long) (forecast_ix + 0.5);

      if (jx > both->min_ix + nx - 1) {
	jx = both->min_ix + nx - 1;
      }
      if (jx < both->min_ix) {
	jx = both->min_ix;
      }
      jndex = (jy - both->min_iy) * nx + (jx - both->min_ix);
      val = tmp_grid[jndex];

      if (val) {
	index = (iy - both->min_iy) * nx + (ix - both->min_ix);
	overlap_grid[index] = val;
	count++;
      }
      
    } /* ix */
    
  } /* iy */

  return (count);

}

static void load_overlaps(storm_file_handle_t *s_handle,
			  storm_status_t *storms1,
			  storm_status_t *storms2,
			  int istorm, int jstorm,
			  bounding_box_t *box1,
			  bounding_box_t *box2)

{

  ui08 *overlap_grid;
  ui08 *tmp_grid;

  ui32 nx, ny, npoints_grid;
  ui32 npoints_1, npoints_2, npoints_overlap;

  double area_grid;
  double area_1, area_2, area_overlap;
  double fraction_1, fraction_2, sum_fraction;
  
  bounding_box_t both;
  
  storm_status_t *storm1 = storms1 + istorm;
  storm_status_t *storm2 = storms2 + jstorm;
  
  titan_grid_t *grid = &s_handle->scan->grid;

  area_grid = grid->dx * grid->dy;

  /*
   * compute dimensions for bounding box for both
   * storms
   */
  
  both.min_ix = MIN(box1->min_ix, box2->min_ix);
  both.min_iy = MIN(box1->min_iy, box2->min_iy);
  both.max_ix = MAX(box1->max_ix, box2->max_ix);
  both.max_iy = MAX(box1->max_iy, box2->max_iy);
  
  /*
   * enlarge grid to make sure it will cover the
   * current storm1 position
   */
  
  both.min_ix = MIN(both.min_ix, storm1->current.bound.min_ix);
  both.min_iy = MIN(both.min_iy, storm1->current.bound.min_iy);
  both.max_ix = MAX(both.max_ix, storm1->current.bound.max_ix);
  both.max_iy = MAX(both.max_iy, storm1->current.bound.max_iy);
  
  /*
   * add 1 pixel margin
   */
  
  both.min_ix -= 1;
  both.min_iy -= 1;
  both.max_ix += 1;
  both.max_iy += 1;
  
  nx = both.max_ix - both.min_ix + 1;
  ny = both.max_iy - both.min_iy + 1;
  npoints_grid = nx * ny;
  
  if (Glob->params.debug >= DEBUG_VERBOSE) {

    fprintf(stderr, "BOTH:\n");
    fprintf(stderr, "min_ix, min_iy: %d, %d\n",
	    both.min_ix, both.min_iy);
    fprintf(stderr, "max_ix, max_iy: %d, %d\n",
	    both.max_ix, both.max_iy);
    fprintf(stderr, "nx, ny: %ld, %ld\n", (long) nx, (long) ny);
	
  }
  
  if (Glob->params.use_runs_for_overlaps) {

    /*
     * load tmp grid with storm1 runs
     */
    
    tmp_grid = init_tmp_grid(npoints_grid);
    overlap_grid = init_overlap_grid(npoints_grid);
    
    load_current_runs(storm1, &both, nx, tmp_grid, 1);
    
    /*
     * copy this storm onto the overlap grid, taking account of
     * movement and growth
     */
    
    npoints_1 = load_forecast_runs(storm1, &both, nx, ny, grid,
				   overlap_grid, tmp_grid);
    
    /*
     * add points for storm2 runs
     */
    
    npoints_2 = load_current_runs(storm2, &both, nx, overlap_grid, 2);

  } else {

    overlap_grid = init_overlap_grid(npoints_grid);
    
    npoints_1 = load_forecast_poly(s_handle, storm1, &both,
				   nx, ny, overlap_grid, 1);

    npoints_2 = load_current_poly(s_handle, storm2, &both,
				  nx, ny, overlap_grid, 2);

  }
  
  /*
   * compute overlap
   */
  
  npoints_overlap = compute_overlap(overlap_grid, npoints_grid);
  
  /*
   * compute areas
   */
  
  area_1 = (double) npoints_1 * area_grid;
  area_2 = (double) npoints_2 * area_grid;
  area_overlap = (double) npoints_overlap * area_grid;
  
  fraction_1 = area_overlap / area_1;
  fraction_2 = area_overlap / area_2;
  
  sum_fraction = fraction_1 + fraction_2;
  
  if (sum_fraction > Glob->params.min_sum_fraction_overlap) {

    if (Glob->params.debug >= DEBUG_VERBOSE) {
	    
      fprintf(stderr, "-------------------------\n");
      fprintf(stderr, "area_1, area_2, area_overlap: "
	      "%g, %g, %g\n",
	      area_1, area_2, area_overlap);
      fprintf(stderr, "fraction_1, fraction_2, sum_fraction: "
	      "%g, %g, %g\n",
	      fraction_1, fraction_2, sum_fraction);
      print_overlap(overlap_grid, nx, ny);
      fprintf(stderr, "-------------------------\n");
	    
    }

    add_overlap(storms1, storms2,
		istorm, jstorm,
		area_overlap);
							
  } /* if (sum_fraction > Glob->params.min_sum_fraction_overlap) */

}

static void print_overlap(ui08 *overlap_grid, ui32 nx, ui32 ny)

{

  ui08 *gp;
  int ix, iy;

  fprintf(stderr, "OVERLAP GRID\n");
  fprintf(stderr, "1 - forcast, 2 - current, 3 - both\n");
  
  for (iy = 0; iy < ny; iy++) {
    
    gp = overlap_grid + (ny - iy - 1) * nx;
    
    for (ix = 0; ix < nx; ix++, gp++) {
      fprintf(stderr, "%d", *gp);
    } /* ix */
    
    fprintf(stderr, "\n");

  } /* iy */

  return;

}

