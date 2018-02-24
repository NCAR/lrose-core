// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/////////////////////////////////////////////////////////////
// TrOverlaps.cc
//
// TrOverlaps class - overlaps for tracking
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1999
//
///////////////////////////////////////////////////////////////

#include "TrOverlaps.hh"
#include <euclid/point.h>
#include <euclid/geometry.h>
#include <rapmath/math_macros.h>
#include <rapmath/trig.h>
using namespace std;

//////////////
// constructor
//

TrOverlaps::TrOverlaps(const string &prog_name, const Params &params) :
  Worker(prog_name, params)

{

  _n_tmp_grid_alloc = 0;
  _tmp_grid_array = NULL;

  _n_overlap_grid_alloc = 0;
  _overlap_grid_array = NULL;

}

/////////////
// destructor
//

TrOverlaps::~TrOverlaps()

{

  if (_tmp_grid_array) {
    ufree(_tmp_grid_array);
  }

  if (_overlap_grid_array) {
    ufree(_overlap_grid_array);
  }

}

////////////////
// find overlaps

void TrOverlaps::find(const TitanStormFile &sfile,
		      vector<TrStorm*> &storms1,
		      vector<TrStorm*> &storms2,
		      double d_hours)
  
{

  int overlap_in_x, overlap_in_y;
  int bounds_overlap;

  TrTrack::bounding_box_t *box2;
  TrTrack::bounding_box_t *box1;
  
  for (size_t istorm = 0; istorm < storms1.size(); istorm++) {
    
    TrStorm &storm1 = *storms1[istorm];

    for (size_t jstorm = 0; jstorm < storms2.size(); jstorm++) {
	
      TrStorm &storm2 = *storms2[jstorm];
	
      /*
       * check for overlap of bounding boxes
       */
	
      box1 = &storm1.box_for_overlap;
      box2 = &storm2.box_for_overlap;
	
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
	
	if (_params.debug >= Params::DEBUG_EXTRA) {

	  fprintf(stderr, "bounding_boxes - time1:storm %d "
		  "overlaps with time2:storm %d\n",
		  (int) istorm, (int) jstorm);
	  
	  fprintf(stderr, "Storm 1 centroid, area: %g, %g, %g\n",
		  storm1.current.proj_area_centroid_x,
		  storm1.current.proj_area_centroid_y,
		  storm1.current.proj_area);
	  
	  fprintf(stderr, "Storm 2 centroid, area: %g, %g, %g\n",
		  storm2.current.proj_area_centroid_x,
		  storm2.current.proj_area_centroid_y,
		  storm2.current.proj_area);
	  
	  fprintf(stderr,
		  "Storm 1: box_min_ix, box_min_iy, box_max_ix, "
		  "box_max_iy: %d, %d, %d, %d\n",
		  box1->min_ix,
		  box1->min_iy,
		  box1->max_ix,
		  box1->max_iy);

	  fprintf(stderr,
		  "Storm 2: box_min_ix, box_min_iy, box_max_ix, "
		  "box_max_iy: %d, %d, %d, %d\n",
		  box2->min_ix,
		  box2->min_iy,
		  box2->max_ix,
		  box2->max_iy);

	  fprintf(stderr, "forecast_x, forecast_y: %g, %g\n",
		  storm1.track.status.forecast_x,
		  storm1.track.status.forecast_y);

	  fprintf(stderr, "forecast_area, length_ratio: %g, %g\n",
		  storm1.track.status.forecast_area,
		  storm1.track.status.forecast_length_ratio);
	  
	} /* if (_params.debug >= Params::DEBUG_EXTRA) */

	load_overlaps(sfile, storm1, storm2,
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

int TrOverlaps::compute_overlap(ui08 *overlap_grid, int npoints_grid)

{

  int i;
  int count = 0;

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
 * alloc and init the tmp grid
 */

void TrOverlaps::init_tmp_grid(int nbytes)

{

  if (nbytes > _n_tmp_grid_alloc) {
    if (_tmp_grid_array == NULL) {
      _tmp_grid_array = (ui08 *) umalloc(nbytes);
    } else {
      _tmp_grid_array = (ui08 *) urealloc(_tmp_grid_array, nbytes);
    }
    _n_tmp_grid_alloc = nbytes;
  }

  memset(_tmp_grid_array, 0, nbytes);
  
}

/**********************
 * init_overlap_grid()
 *
 * alloc and init the overlap grid
 */

void TrOverlaps::init_overlap_grid(int nbytes)

{

  if (nbytes > _n_overlap_grid_alloc) {
    if (_overlap_grid_array == NULL) {
      _overlap_grid_array = (ui08 *) umalloc(nbytes);
    } else {
      _overlap_grid_array = (ui08 *) urealloc(_overlap_grid_array, nbytes);
    }
    _n_overlap_grid_alloc = nbytes;
  }

  memset(_overlap_grid_array, 0, nbytes);
  
}

/***************************************************
 * load_current_poly()
 *
 * Load up grid with polygon in the current position
 *
 * Returns number of points touched in grid
 */

int TrOverlaps::load_current_poly(const TitanStormFile &sfile,
				  TrStorm &storm,
				  TrTrack::bounding_box_t *both,
				  int nx, int ny,
				  ui08 *data_grid,
				  int val)

{

  int i;
  int count;
  double ray_len;
  double *ray;
  double az, delta_az;
  double sinAz, cosAz;

  Point_d *point;
  Point_d poly[N_POLY_SIDES + 1];

  const storm_file_params_t &sparams = sfile.header().params;
  const titan_grid_t &grid = sfile.scan().grid;

  /*
   * load up poly - this is the polygon from the current
   * position of the storm
   */
  
  point = poly;
  ray = storm.current.proj_area_rays;
  az = sparams.poly_start_az * DEG_TO_RAD;
  delta_az = sparams.poly_delta_az * DEG_TO_RAD;
  rap_sincos(az, &sinAz, &cosAz);
  
  for (i = 0; i < N_POLY_SIDES;
       i++, point++, ray++, az += delta_az) {

    ray_len = *ray;
    point->x = (storm.current.proj_area_centroid_x + sinAz * ray_len * grid.dx);
    point->y = (storm.current.proj_area_centroid_y + cosAz * ray_len * grid.dy);
    
  } /* i */

  poly[N_POLY_SIDES] = poly[0];

  count = EG_fill_polygon(poly, N_POLY_SIDES + 1,
			  nx, ny,
			  grid.minx + both->min_ix * grid.dx,
			  grid.miny + both->min_iy * grid.dy,
			  grid.dx, grid.dy,
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

int TrOverlaps::load_current_runs(TrStorm &storm,
				  TrTrack::bounding_box_t *both,
				  int nx,
				  ui08 *data_grid,
				  int val)

{

  ui08 *gp;
  int i, irun;
  int index;
  int count = 0;
  storm_file_run_t *run;

  run = storm.proj_runs;
  for (irun = 0; irun < storm.status.n_proj_runs; irun++, run++) {

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

int TrOverlaps::load_forecast_poly(const TitanStormFile &sfile,
				   TrStorm &storm,
				   TrTrack::bounding_box_t *both,
				   int nx, int ny,
				   ui08 *data_grid,
				   int val)

{

  int i;
  int count;
  double ray_len;
  double *ray;
  double az, delta_az;
  double sinAz, cosAz;

  Point_d *point;
  Point_d poly[N_POLY_SIDES + 1];

  const storm_file_params_t &sparams = sfile.header().params;
  const titan_grid_t &grid = sfile.scan().grid;

  /*
   * load up poly - this must take into account the movement
   * of the storm
   */
  
  point = poly;
  ray = storm.current.proj_area_rays;
  az = sparams.poly_start_az * DEG_TO_RAD;
  delta_az = sparams.poly_delta_az * DEG_TO_RAD;
  rap_sincos(az, &sinAz, &cosAz);
  
  for (i = 0; i < N_POLY_SIDES;
       i++, point++, ray++, az += delta_az) {

    ray_len = *ray * storm.track.status.forecast_length_ratio;
    point->x = storm.track.status.forecast_x + sinAz * ray_len * grid.dx;
    point->y = storm.track.status.forecast_y + cosAz * ray_len * grid.dy;
    
  } /* i */

  poly[N_POLY_SIDES] = poly[0];

  count = EG_fill_polygon(poly, N_POLY_SIDES + 1,
			  nx, ny,
			  grid.minx + both->min_ix * grid.dx,
			  grid.miny + both->min_iy * grid.dy,
			  grid.dx, grid.dy,
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

int TrOverlaps::load_forecast_runs(TrStorm &storm,
				   TrTrack::bounding_box_t *both,
				   int nx, int ny,
				   const titan_grid_t &grid,
				   ui08 *overlap_grid,
				   ui08 *tmp_grid)
     
{

  si32 ix, jx, iy, jy;
  si32 index, jndex;
  int count = 0;
  ui08 val;

  double x, y;
  double grid_ix, grid_iy;
  double xratio, yratio;
  double forecast_ix, forecast_iy;

  titan_grid_t fcast_grid;
  TrTrack::props_t *current = &storm.current;
  TrTrack &track = storm.track;

  /*
   * compute current posn in terms of the grid
   */
  
  grid_ix = (current->proj_area_centroid_x - grid.minx) / grid.dx;
  grid_iy = (current->proj_area_centroid_y - grid.miny) / grid.dy;
  
  /*
   * Compute the cartesian grid parameters for the forecast time.
   * Here we are assuming that the storm grows or decays in area
   * but keeps the same shape. We use the fcast_grid as a computational
   * tool to compute the position of each grid forecast point
   */
      
  fcast_grid.dx = grid.dx * track.status.forecast_length_ratio;
  fcast_grid.dy = grid.dy * track.status.forecast_length_ratio;
  
  fcast_grid.minx = track.status.forecast_x - grid_ix * fcast_grid.dx;
  fcast_grid.miny = track.status.forecast_y - grid_iy * fcast_grid.dy;

  y = storm.box_for_overlap.min_iy * grid.dy + grid.miny;
  yratio = grid.dy / fcast_grid.dy;
  forecast_iy = (y - fcast_grid.miny) / fcast_grid.dy;
  
  for (iy = storm.box_for_overlap.min_iy;
       iy <= storm.box_for_overlap.max_iy;
       iy++, forecast_iy += yratio) {
    
    jy = (int) (forecast_iy + 0.5);

    if (jy > both->min_iy + ny - 1) {
      jy = both->min_iy + ny - 1;
    }
    if (jy < both->min_iy) {
      jy = both->min_iy;
    }

    x = storm.box_for_overlap.min_ix * grid.dx + grid.minx;
    xratio = grid.dx / fcast_grid.dx;
    forecast_ix = (x - fcast_grid.minx) / fcast_grid.dx;
    
    for (ix = storm.box_for_overlap.min_ix;
	 ix <= storm.box_for_overlap.max_ix;
	 ix++, forecast_ix += xratio) {
      
      jx = (int) (forecast_ix + 0.5);

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

void TrOverlaps::load_overlaps(const TitanStormFile &sfile,
			       TrStorm &storm1,
			       TrStorm &storm2,
			       int istorm, int jstorm,
			       TrTrack::bounding_box_t *box1,
			       TrTrack::bounding_box_t *box2)

{

  int nx, ny, npoints_grid;
  int npoints_1, npoints_2, npoints_overlap;

  double area_grid;
  double area_1, area_2, area_overlap;
  double fraction_1, fraction_2, sum_fraction;
  
  TrTrack::bounding_box_t both;
  
  const titan_grid_t &grid = sfile.scan().grid;

  area_grid = grid.dx * grid.dy;

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
  
  both.min_ix = MIN(both.min_ix, storm1.current.bound.min_ix);
  both.min_iy = MIN(both.min_iy, storm1.current.bound.min_iy);
  both.max_ix = MAX(both.max_ix, storm1.current.bound.max_ix);
  both.max_iy = MAX(both.max_iy, storm1.current.bound.max_iy);
  
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
  
  if (_params.debug >= Params::DEBUG_EXTRA) {

    fprintf(stderr, "BOTH:\n");
    fprintf(stderr, "min_ix, min_iy: %d, %d\n",
	    both.min_ix, both.min_iy);
    fprintf(stderr, "max_ix, max_iy: %d, %d\n",
	    both.max_ix, both.max_iy);
    fprintf(stderr, "nx, ny: %d, %d\n", nx, ny);
	
  }
  
  if (_params.tracking_use_runs_for_overlaps) {

    /*
     * load tmp grid with storm1 runs
     */
    
    init_tmp_grid(npoints_grid);
    init_overlap_grid(npoints_grid);
    
    load_current_runs(storm1, &both, nx, _tmp_grid_array, 1);
    
    /*
     * copy this storm onto the overlap grid, taking account of
     * movement and growth
     */
    
    npoints_1 = load_forecast_runs(storm1, &both, nx, ny, grid,
				   _overlap_grid_array,
				   _tmp_grid_array);
    
    /*
     * add points for storm2 runs
     */
    
    npoints_2 = load_current_runs(storm2, &both, nx, _overlap_grid_array, 2);

  } else {

    init_overlap_grid(npoints_grid);
    
    npoints_1 = load_forecast_poly(sfile, storm1, &both,
				   nx, ny, _overlap_grid_array, 1);

    npoints_2 = load_current_poly(sfile, storm2, &both,
				  nx, ny, _overlap_grid_array, 2);

  }
  
  /*
   * compute overlap
   */
  
  npoints_overlap = compute_overlap(_overlap_grid_array, npoints_grid);
  
  /*
   * compute areas
   */
  
  area_1 = (double) npoints_1 * area_grid;
  area_2 = (double) npoints_2 * area_grid;
  area_overlap = (double) npoints_overlap * area_grid;
  
  fraction_1 = area_overlap / area_1;
  fraction_2 = area_overlap / area_2;
  
  sum_fraction = fraction_1 + fraction_2;
  
  if (sum_fraction > _params.tracking_min_sum_fraction_overlap) {

    if (_params.debug >= Params::DEBUG_EXTRA) {
      
      fprintf(stderr, "-------------------------\n");
      fprintf(stderr, "area_1, area_2, area_overlap: "
	      "%g, %g, %g\n",
	      area_1, area_2, area_overlap);
      fprintf(stderr, "fraction_1, fraction_2, sum_fraction: "
	      "%g, %g, %g\n",
	      fraction_1, fraction_2, sum_fraction);
      print_overlap(_overlap_grid_array, nx, ny);
      fprintf(stderr, "-------------------------\n");
	    
    }

    add_overlap(storm1, storm2,
		istorm, jstorm,
		area_overlap);
							
  } /* if (sum_fraction > _params.min_sum_fraction_overlap) */

}

void TrOverlaps::print_overlap(ui08 *overlap_grid, int nx, int ny)

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

void TrOverlaps::add_overlap(TrStorm &storm1,
			     TrStorm &storm2,
			     int istorm, int jstorm,
			     double area_overlap)

{

  int i;
  int complex_already_present;
  
  si32 merging_complex_num;
  si32 n_simple_after_merger;

  TrStorm::track_match_t *match;
  
  /*
   * check if complex num has already been added to merger
   */
  
  merging_complex_num = storm1.track.status.complex_track_num;
  match = storm2.match_array;

  complex_already_present = FALSE;
  for (i = 0; i < storm2.status.n_match; i++, match++) {
    if (match->complex_track_num == merging_complex_num) {
      complex_already_present = TRUE;
      break;
    }
  }

  if (complex_already_present) {
    n_simple_after_merger = storm2.status.sum_simple_tracks;
  } else {
    n_simple_after_merger =
      storm2.status.sum_simple_tracks + storm1.track.status.n_simple_tracks;
  }

  /*
   * add data to storm1 array - potential splits
   */
  
  storm1.alloc_match_array(storm1.status.n_match + 1);
  match = storm1.match_array + storm1.status.n_match;
  
  match->storm_num = jstorm;
  match->overlap = area_overlap;

  storm1.status.sum_overlap += area_overlap;
  storm1.status.n_match++;
    
  /*
   * add data to storm2 array - potential mergers
   */
  
  storm2.alloc_match_array(storm2.status.n_match + 1);
  match = storm2.match_array + storm2.status.n_match;
  
  match->complex_track_num = merging_complex_num;
  match->storm_num = istorm;
  match->overlap = area_overlap;
  match->n_simple_tracks = storm1.track.status.n_simple_tracks;

  storm2.status.sum_overlap += area_overlap;
  storm2.status.n_match++;

  if (!complex_already_present) {
    storm2.status.sum_simple_tracks = n_simple_after_merger;
  }
  
  return;
  
}
