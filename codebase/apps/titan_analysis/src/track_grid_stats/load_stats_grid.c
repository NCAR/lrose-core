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
 * load_stats_grid.c
 *
 * Load up the track stats grid
 *
 * Mike Dixon RAP NCAR Boulder CO 80307 USA
 *
 * August 1993
 *
 **************************************************************************/

#include "track_grid_stats.h"

#define DO_CONSTRAIN(x, low, high) if ((x) < (low)) (x) = (low); \
                                else if ((x) > (high)) (x) = (high)

#define SMALL_ANGLE 0.000001
#define PI_BY_TWO 1.570796327
#define ALMOST_PI_BY_TWO (PI_BY_TWO - SMALL_ANGLE)
#ifndef PI
#define PI 3.141592654
#endif
#define ALMOST_PI (PI - SMALL_ANGLE)

/*
 * prototypes
 */

static double get_dur_max_precip(si32 iy,
				 si32 ix,
				 si32 n_scans_for_max,
				 si32 n_scans_in_data,
				 double ***precip_grid);

static void set_ellipse_grid(double ellipse_x,
			     double ellipse_y,
			     double major_radius,
			     double minor_radius,
			     double axis_rotation,
			     si32 *start_ix_p,
			     si32 *start_iy_p,
			     si32 *end_ix_p,
			     si32 *end_iy_p,
			     ui08 **grid);

static void update_ellipse_precip(storm_file_handle_t *s_handle,
				  storm_file_params_t *sparams,
				  storm_file_global_props_t *gprops,
				  si32 *start_ix_p,
				  si32 *start_iy_p,
				  si32 *end_ix_p,
				  si32 *end_iy_p,
				  ui08 **ellipse_grid,
				  ui08 **storm_grid,
				  double **p_grid,
				  grid_stats_t **stats);


/*
 * main routine
 */

void load_stats_grid(storm_file_handle_t *s_handle,
		     track_file_handle_t *t_handle,
		     grid_stats_t **stats)

{

  ui08 **ellipse_grid;
  ui08 **storm_grid;
  ui08 *storm_p;

  si32 icomplex, isimple, ientry;
  si32 ix, iy;
  si32 start_ix = 0, start_iy = 0;
  si32 end_ix = 0, end_iy = 0;
  si32 complex_track_num;
  si32 simple_track_num;
  si32 n_scans_for_max_precip;

  double centroid_x, centroid_y;
  double duration_in_hr;
  double tops, volume, area;
  double dbz_max;
  double dx_dt, dy_dt;
  double n_start, n_end;
  double distance_x, distance_y;
  double speed;
  double start_x, start_y;
  double end_x, end_y;
  double sum_start_x, sum_start_y;
  double sum_end_x, sum_end_y;
  double dur_max_precip;
  double inv_grid_sq_area;
  double **p_grid;
  double ***precip_grid;

  complex_track_params_t *ct_params;
  simple_track_params_t *st_params;
  track_file_entry_t *entry;
  storm_file_header_t *sheader;
  track_file_header_t *theader;
  storm_file_params_t *sparams;
  storm_file_global_props_t *gprops;
  track_file_forecast_props_t *fprops;
  grid_stats_t *stat;

  /*
   * initialize
   */

  sheader = s_handle->header;
  theader = t_handle->header;
  sparams = &sheader->params;
  
  inv_grid_sq_area = 1.0 / (Glob->params.grid.dx * Glob->params.grid.dy);
  
  ellipse_grid = (ui08 **) ucalloc2
    ((ui32) Glob->params.grid.ny,
     (ui32) Glob->params.grid.nx, sizeof(ui08));

  storm_grid = (ui08 **) ucalloc2
    ((ui32) Glob->params.grid.ny,
     (ui32) Glob->params.grid.nx, sizeof(ui08));

  precip_grid = (double ***) ucalloc3
    ((ui32) sheader->n_scans,
     (ui32) Glob->params.grid.ny,
     (ui32) Glob->params.grid.nx, sizeof(double));

  /*
   * loop through complex tracks
   */
  
  for (icomplex = 0; icomplex < theader->n_complex_tracks; icomplex++) {
  
    complex_track_num = t_handle->complex_track_nums[icomplex];
    
    if(RfReadComplexTrackParams(t_handle, complex_track_num,
				TRUE, "load_stats_grid"))
      tidy_and_exit(-1);
  
    ct_params = t_handle->complex_params;

    /*
     * continue to next track if this one is too short
     */

    if (ct_params->duration_in_secs < Glob->params.min_duration)
      continue;
    duration_in_hr = (double) ct_params->duration_in_secs / 3600.0;
    
    /*
     * initialize
     */

    n_start = 0.0;
    n_end = 0.0;
    sum_start_x = 0.0;
    sum_start_y = 0.0;
    sum_end_x = 0.0;
    sum_end_y = 0.0;

    /*
     * loop through simple tracks in this complex track
     */
  
    for (isimple = 0;
	 isimple < ct_params->n_simple_tracks; isimple++) {
    
      simple_track_num =
	t_handle->simples_per_complex[complex_track_num][isimple];
      
      if(RfRewindSimpleTrack(t_handle, simple_track_num,
			     "track_view") != R_SUCCESS)
	tidy_and_exit(-1);
    
      st_params = t_handle->simple_params;

      /*
       * loop through the track entries
       */
    
      for (ientry = 0;
	   ientry < st_params->duration_in_scans; ientry++) {
      
	if (RfReadTrackEntry(t_handle, "load_stats_grid") != R_SUCCESS)
	  tidy_and_exit(-1);
      
	entry = t_handle->entry;
	fprops = &entry->dval_dt;
	
	/*
	 * set precip grid pointer
	 */

	p_grid = precip_grid[entry->scan_num];

	/*
	 * read in storm props
	 */

	if (RfReadStormScan(s_handle, entry->scan_num,
			    "load_stats_grid") != R_SUCCESS)
	  tidy_and_exit(-1);

	if (RfReadStormProps(s_handle, entry->storm_num,
			     "load_stats_grid") != R_SUCCESS)
	  tidy_and_exit(-1) ;
      
	gprops = s_handle->gprops + entry->storm_num;

	centroid_x = gprops->proj_area_centroid_x;
	centroid_y = gprops->proj_area_centroid_y;
	tops = gprops->top;
	volume = gprops->volume;
	area = gprops->proj_area;

	dbz_max = gprops->dbz_max;
	dx_dt = fprops->proj_area_centroid_x;
	dy_dt = fprops->proj_area_centroid_y;
	speed = sqrt(dx_dt * dx_dt + dy_dt * dy_dt);
	
	ix = (si32) ((centroid_x - Glob->params.grid.minx) /
		     Glob->params.grid.dx + 0.5);
	iy = (si32) ((centroid_y - Glob->params.grid.miny) /
		     Glob->params.grid.dy + 0.5);

	if (iy >= 0 && iy < Glob->params.grid.ny && ix >= 0 &&
	    ix < Glob->params.grid.nx) {
	  
	  stat = stats[iy] + ix;
	  
	  stat->n_events++;
	  
	  /*
	   * deal with the centroid first
	   */

	  if (entry->history_in_scans == 1) {
	    
	    stat->n_start += inv_grid_sq_area;

	    sum_start_x += centroid_x;
	    sum_start_y += centroid_y;
	    n_start++;

	  } /* if (entry->history_in_scans == 1) */
	  
	  if (entry->history_in_scans == ct_params->duration_in_scans / 2) {
	    stat->n_mid += inv_grid_sq_area;
	  }
	  
	  if (entry->history_in_scans == ct_params->duration_in_scans) {
	    
	    sum_end_x += centroid_x;
	    sum_end_y += centroid_y;
	    n_end++;

	  } /* if (entry->history_in_scans .... */
	  
	} /* if (iy >= 0 ... */
	  
	/*
	 * now update the precip grid
	 */

	update_ellipse_precip(s_handle,
			      sparams, gprops,
			      &start_ix, &start_iy,
			      &end_ix, &end_iy,
			      ellipse_grid, storm_grid, p_grid,
			      stats);
	
	/*
	 * update the stats for the grid points in the storm
	 */

	for (iy = start_iy; iy < end_iy; iy++) {
	  
	  storm_p = storm_grid[iy] + start_ix;
	  stat = stats[iy] + start_ix;
	  
	  for (ix = start_ix; ix < end_ix; ix++) {
	    
	    if (*storm_p) {

	      stat->n_weighted++;
			  
	      stat->v += dy_dt;
	      stat->u += dx_dt;
	      stat->speed += speed;
	      stat->dbz_max += dbz_max;
	      stat->tops += tops;
	      stat->volume += volume;
	      stat->area += area;
	      stat->duration += duration_in_hr;
	      stat->ln_area += log(area);

	    }
	    
	    storm_p++;
	    stat++;
	    
	  } /* ix */
	  
	} /* iy */

      } /* ientry */
    
    } /* isimple */

    /*
     * compute start and end points for complex track
     */

    start_x = sum_start_x / n_start;
    start_y = sum_start_y / n_start;
    end_x = sum_end_x / n_end;
    end_y = sum_end_y / n_end;
  
    ix = (si32) ((start_x - Glob->params.grid.minx) /
		 Glob->params.grid.dx + 0.5);
    iy = (si32) ((start_y - Glob->params.grid.miny) /
		 Glob->params.grid.dy + 0.5);

    if (iy >= 0 && iy < Glob->params.grid.ny && ix >= 0 &&
	ix < Glob->params.grid.nx) {
      
      stat = stats[iy] + ix;

      stat->n_complex++;

      distance_x = end_x - start_x;
      distance_y = end_y - start_y;

      stat->distance +=
	sqrt(distance_x * distance_x + distance_y * distance_y);
      stat->dx += distance_x;
      stat->dy += distance_y;

    } /* if (iy >= 0 && iy < Glob->params.grid.ny && ix >= 0 &&
	 ix < Glob->params.grid.nx) */
      
  } /* icomplex */

  /*
   * update the duration-max precip depth
   */

  n_scans_for_max_precip =
    (si32) ((double) Glob->params.dur_for_max_precip /
	    (double) Glob->params.scan_interval + 0.5);

  for (iy = 0; iy < Glob->params.grid.ny; iy++) {

    stat = stats[iy];
      
    for (ix = 0; ix < Glob->params.grid.nx; ix++) {

      dur_max_precip = get_dur_max_precip(iy, ix,
					  n_scans_for_max_precip,
					  sheader->n_scans,
					  precip_grid);

      if (stat->dur_max_precip < dur_max_precip)
	stat->dur_max_precip = dur_max_precip;

      stat++;

    } /* ix */

  } /* iy */

  /*
   * free up
   */

  ufree2((void **) ellipse_grid);
  ufree2((void **) storm_grid);
  ufree3((void ***) precip_grid);
  
}

/*********************************************************************
 * get_dur_max_precip()
 *
 * returns the max precip depth for the specified duration for
 * the given point
 */

static double get_dur_max_precip(si32 iy,
				 si32 ix,
				 si32 n_scans_for_max_precip,
				 si32 n_scans_in_data,
				 double ***precip_grid)

{

  si32 offset, increment;
  si32 i;
  double *pp1, *pp2;
  double running_depth;
  double max_depth;

  if (n_scans_for_max_precip > n_scans_in_data)
    return (0.0);

  offset = iy * Glob->params.grid.nx + ix;
  increment = Glob->params.grid.ny * Glob->params.grid.nx;

  /*
   * compute the starting values
   */
  
  pp1 = **precip_grid + offset;
  pp2 = **precip_grid + offset;
  running_depth = 0;
  
  for (i = 0; i < n_scans_for_max_precip; i++) {

    running_depth += *pp2;
    pp2 += increment;

  } /* i */

  max_depth = running_depth;

  /*
   * move through the array keeping track of the max
   * running depth
   */

  for (i = n_scans_for_max_precip; i < n_scans_in_data; i++) {

    running_depth -= *pp1;
    running_depth += *pp2;

    max_depth = MAX(max_depth, running_depth);

    pp1 += increment;
    pp2 += increment;

  } /* i */

  return (max_depth);

}


/*********************************************************************
 * set_ellipse_grid()
 *
 * Flags regions in the grid with 1's if the given
 * ellipse crosses into or contains the region.
 *
 * The method uses 3 steps.
 *
 * 1) Flag the region containing the ellipse centroid.
 *
 * 2) Consider all vertical grid lines which intersect the ellipse.
 *    Flag all regions on either side of such a line for that
 *    line segment which crosses the ellipse.
 *
 * 3) Consider all horizontal grid lines which intersect the ellipse.
 *    Flag all regions on either side of such a line for that
 *    line segment which crosses the ellipse.
 *
 *********************************************************************/

static void set_ellipse_grid(double ellipse_x,
			     double ellipse_y,
			     double major_radius,
			     double minor_radius,
			     double axis_rotation,
			     si32 *start_ix_p,
			     si32 *start_iy_p,
			     si32 *end_ix_p,
			     si32 *end_iy_p,
			     ui08 **grid)

{

  si32 ix, iy, ix1, iy1, ix2, iy2;
  si32 start_ix, start_iy;
  si32 end_ix, end_iy;

  double grid_rotation, theta;
  double slope_prime, intercept_prime;
  double sin_rotation, cos_rotation, tan_rotation;

  double xprime1, yprime1, xprime2, yprime2;
  double x_1, y_1, x_2, y_2;
  double start_x, start_y;
  double end_x, end_y;
  double line_x, line_y;

  /*
   * compute the grid_rotation, taking care to avoid 0, pi/2 and
   * pi, so that the trig functions will not fail. Remember that
   * the axis_rotation is relative to True North, and we need to
   * compute the grid rotation relative to the mathmatically
   * conventional axes
   */

  theta = 90.0 - axis_rotation;

  if (theta == 0.0)
    grid_rotation = SMALL_ANGLE;
  else if (theta == 90.0)
    grid_rotation = ALMOST_PI_BY_TWO;
  else if (theta == -90.0)
    grid_rotation = - ALMOST_PI_BY_TWO;
  else if (theta == 180.0 || theta == -180.0)
    grid_rotation = ALMOST_PI;
  else
    grid_rotation = theta * DEG_TO_RAD;

  sin_rotation = sin(grid_rotation);
  cos_rotation = cos(grid_rotation);
  tan_rotation = tan(grid_rotation);
  
  /*
   * compute the start and end x and y - these values are
   * chosen for a circle of radius major_radius, which will
   * enclose the ellipse
   */

  start_x = ellipse_x - major_radius;
  start_y = ellipse_y - major_radius;

  end_x = ellipse_x + major_radius;
  end_y = ellipse_y + major_radius;

  /*
   * set the end and start grid indices
   */

  start_ix = (si32) ((start_x - Glob->params.grid.minx) /
		     Glob->params.grid.dx + 0.49);
  DO_CONSTRAIN(start_ix, 0, Glob->params.grid.nx - 1);

  start_iy = (si32) ((start_y - Glob->params.grid.miny) /
		     Glob->params.grid.dy + 0.49);
  DO_CONSTRAIN(start_iy, 0, Glob->params.grid.ny - 1);

  end_ix = (si32) ((end_x - Glob->params.grid.minx) /
		   Glob->params.grid.dx + 0.51);
  DO_CONSTRAIN(end_ix, 0, Glob->params.grid.nx - 1);

  end_iy = (si32) ((end_y - Glob->params.grid.miny) /
		   Glob->params.grid.dy + 0.51);
  DO_CONSTRAIN(end_iy, 0, Glob->params.grid.ny - 1);

  *start_ix_p = start_ix;
  *start_iy_p = start_iy;
  *end_ix_p = end_ix;
  *end_iy_p = end_iy;

  /*
   * flag the grid region which contains the ellipse centroid
   */

  ix = (si32) ((ellipse_x - Glob->params.grid.minx) /
	       Glob->params.grid.dx + 0.5);
  iy = (si32) ((ellipse_y - Glob->params.grid.miny) /
	       Glob->params.grid.dy + 0.5);

  if (ix >= start_ix && ix <= end_ix &&
      iy >= start_iy && iy <= end_iy)
    grid[iy][ix] = 1;

  /*
   * loop through the vertical lines which intersect the ellipse
   */

  for (ix = start_ix; ix < end_ix; ix++) {

    /*
     * compute the slope and intercept of this line in the
     * transformed coordinate system with ths origin at the
     * center of the ellipse and the x-axis along the major
     * axis. The prime values refer to the transformed
     * coord system.
     */

    
    line_x = Glob->params.grid.minx + ((double) ix + 0.5) *
      Glob->params.grid.dx;

    slope_prime = 1.0 / tan_rotation;

    intercept_prime  = - (line_x - ellipse_x) / sin_rotation;

    if (uline_through_ellipse(major_radius, minor_radius,
			      slope_prime, intercept_prime,
			      &xprime1, &yprime1,
			      &xprime2, &yprime2)) {

      /*
       * transform the points back into grid coords
       */

      y_1 = ellipse_y + xprime1 * sin_rotation + yprime1 * cos_rotation;
      y_2 = ellipse_y + xprime2 * sin_rotation + yprime2 * cos_rotation;

      if  (y_1 <= y_2) {

	iy1 = (si32) ((y_1 - Glob->params.grid.miny) /
		      Glob->params.grid.dy + 0.5);
	iy2 = (si32) ((y_2 - Glob->params.grid.miny) /
		      Glob->params.grid.dy + 0.5);

      } else {

	iy1 = (si32) ((y_2 - Glob->params.grid.miny) /
		      Glob->params.grid.dy + 0.5);
	iy2 = (si32) ((y_1 - Glob->params.grid.miny) /
		      Glob->params.grid.dy + 0.5);

      }

      iy1 = MAX(iy1, start_iy);
      iy2 = MIN(iy2, end_iy);

      for (iy = iy1; iy <= iy2; iy++) {

	grid[iy][ix] = 1;
	grid[iy][ix + 1] = 1;

      } /* iy */

    } /* if (uline_through_ellipse(major_radius ... */

  } /* ix */

  /*
   * loop through the horizontal lines which intersect the ellipse
   */

  for (iy = start_iy; iy < end_iy; iy++) {

    /*
     * compute the slope and intercept of this line in the
     * transformed coordinate system with ths origin at the
     * center of the ellipse and the x-axis along the major
     * axis. The prime values refer to the transformed
     * coord system.
     */

    
    line_y = Glob->params.grid.miny +
      ((double) iy + 0.5) * Glob->params.grid.dy;

    slope_prime = - tan_rotation;

    intercept_prime  = (line_y - ellipse_y) / cos_rotation;

    if (uline_through_ellipse(major_radius, minor_radius,
			      slope_prime, intercept_prime,
			      &xprime1, &yprime1,
			      &xprime2, &yprime2)) {

      /*
       * transform the points back into grid coords
       */

      x_1 = ellipse_x + xprime1 * cos_rotation - yprime1 * sin_rotation;
      x_2 = ellipse_x + xprime2 * cos_rotation - yprime2 * sin_rotation;

      if  (x_1 <= x_2) {

	ix1 = (si32) ((x_1 - Glob->params.grid.minx) /
		      Glob->params.grid.dx + 0.5);
	ix2 = (si32) ((x_2 - Glob->params.grid.minx) /
		      Glob->params.grid.dx + 0.5);

      } else {

	ix1 = (si32) ((x_2 - Glob->params.grid.minx) /
		      Glob->params.grid.dx + 0.5);
	ix2 = (si32) ((x_1 - Glob->params.grid.minx) /
		      Glob->params.grid.dx + 0.5);

      }

      ix1 = MAX(ix1, start_ix);
      ix2 = MIN(ix2, end_ix);

      for (ix = ix1; ix <= ix2; ix++) {

	grid[iy][ix] = 1;
	grid[iy + 1][ix] = 1;

      } /* ix */

    } /* if (uline_through_ellipse(major_radius ... */

  } /* iy */

}

/***************************************************************
 * update_ellipse_precip()
 *
 * Updates the grids based on the storm precip area ellipses,
 * and uses the reflectivity distribution to estimate the
 * precip swath
 */

static void update_ellipse_precip(storm_file_handle_t *s_handle,
				  storm_file_params_t *sparams,
				  storm_file_global_props_t *gprops,
				  si32 *start_ix_p,
				  si32 *start_iy_p,
				  si32 *end_ix_p,
				  si32 *end_iy_p,
				  ui08 **ellipse_grid,
				  ui08 **storm_grid,
				  double **p_grid,
				  grid_stats_t **stats)

{

  ui08 *ellipse_p;

  si32 interval;
  si32 n_dbz_intervals;
  si32 start_ix, start_iy;
  si32 end_ix, end_iy;
  si32 ix, iy;
  si32 ngrid;

  double low_dbz_threshold;
  double dbz_hist_interval;
  double ellipse_x;
  double ellipse_y;
  double ellipse_area;
  double major_radius;
  double minor_radius;
  double frac_major_radius;
  double frac_minor_radius;
  double axis_rotation;
  double low_dbz;
  double mean_dbz;
  double accum_depth, incr_depth;
  double precip_rate, precip_depth;
  double area_fraction;
  double percent_area;
  double z;
  double *precip_p;

  storm_file_dbz_hist_t *hist;
  grid_stats_t *stat;

  /*
   * set parameters
   */

  low_dbz_threshold = sparams->low_dbz_threshold;
  dbz_hist_interval = sparams->dbz_hist_interval;
  n_dbz_intervals = gprops->n_dbz_intervals;
  ngrid = Glob->params.grid.nx * Glob->params.grid.ny;

  ellipse_x = gprops->proj_area_centroid_x;
  ellipse_y = gprops->proj_area_centroid_y;
  major_radius = gprops->proj_area_major_radius;
  minor_radius = gprops->proj_area_minor_radius;

  /*
   * check that aspect ratio is not ridiculous, which happens when
   * all points line up
   */

  if (major_radius / minor_radius > 100.0) {

    ellipse_area = gprops->proj_area;
    major_radius = sqrt(ellipse_area / PI);
    minor_radius = major_radius;

  }

  axis_rotation = gprops->proj_area_orientation;
  
  /*
   * compute the area and incremental precip depth for
   * each reflectivity interval
   */

  low_dbz = low_dbz_threshold;
  mean_dbz = low_dbz + dbz_hist_interval / 2.0;
  
  accum_depth = 0.0;
  area_fraction = 1.0;
  hist = s_handle->hist;

  for (interval = 0; interval < n_dbz_intervals; interval++, hist++) {

    z = pow(10.0, low_dbz / 10.0);
    precip_rate = pow((z / Glob->params.z_r_coeff),
		      (1.0 / Glob->params.z_r_exponent));
    precip_depth =
      (precip_rate * (double) Glob->params.scan_interval) / 3600.0;
    incr_depth = precip_depth - accum_depth;

    percent_area = hist->percent_area;

    /*
     * zero out ellipse grid
     */
    
    memset(*ellipse_grid, 0, ngrid);

    /*
     * set points in ellipse grid
     */

    frac_major_radius = major_radius * sqrt(area_fraction);
    frac_minor_radius = minor_radius * sqrt(area_fraction);

    set_ellipse_grid(ellipse_x,
		     ellipse_y,
		     frac_major_radius,
		     frac_minor_radius,
		     axis_rotation,
		     &start_ix, &start_iy,
		     &end_ix, &end_iy,
		     ellipse_grid);

    /*
     * for lowest interval, copy the ellipse grid to the
     * storm definition grid
     */

    if (interval == 0) {

      memcpy(*storm_grid, *ellipse_grid, ngrid);

      *start_ix_p = start_ix;
      *end_ix_p = end_ix;
      *start_iy_p = start_iy;
      *end_iy_p = end_iy;

    } /* if (interval == 0) */

    /*
     * set the grids
     */

    for (iy = start_iy; iy < end_iy; iy++) {

      ellipse_p = ellipse_grid[iy] + start_ix;
      precip_p = p_grid[iy] + start_ix;
      stat = stats[iy] + start_ix;

      for (ix = start_ix; ix < end_ix; ix++) {

	if (*ellipse_p) {
	  *precip_p += incr_depth;
	  stat->precip += incr_depth;
	}

	ellipse_p++;
	precip_p++;
	stat++;

      } /* ix */

    } /* iy */

    /*
     * increment and test for end of loop conditions
     */
  
    accum_depth = precip_depth;
    area_fraction -= percent_area / 100.0;
    low_dbz += dbz_hist_interval;
    mean_dbz += dbz_hist_interval;
    
    if (low_dbz > Glob->params.hail_dbz_threshold)
      break;

    if (area_fraction <= 0.001)
      break;

  } /* interval */

}

