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
 * load_grid.c
 *
 * grid utilities
 *
 * Public routines are :
 *
 *  load_forecast_grid()
 *  load_truth_grid()
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * January 1992
 *
 ****************************************************************************/

#include "verify_tracks.h"

/*
 * macro to constrain value to given limits
 */

#define DO_CONSTRAIN(x, low, high) if ((x) < (low)) (x) = (low); \
                                else if ((x) > (high)) (x) = (high)

/* #define RAD_TO_DEG 57.29577951308092 */
/* #define DEG_TO_RAD 0.01745329251994372 */
#define SMALL_ANGLE 0.000001
#define PI_BY_TWO 1.570796327
#define ALMOST_PI_BY_TWO (PI_BY_TWO - SMALL_ANGLE)
#ifndef PI
#define PI 3.141592654
#endif
#define ALMOST_PI (PI - SMALL_ANGLE)

static void
load_ellipse_forecast_grid(storm_file_params_t *sparams,
			   track_file_params_t *tparams,
			   storm_file_scan_header_t *scan,
			   storm_file_global_props_t *gprops,
			   track_file_entry_t *entry,
			   double lead_time,
			   ui08 **grid);


static void
load_ellipse_truth_grid(storm_file_params_t *sparams,
			storm_file_scan_header_t *scan,
			storm_file_global_props_t *gprops,
			ui08 **grid);

static void load_ellipse_grid(double ellipse_x,
			      double ellipse_y,
			      double major_radius,
			      double minor_radius,
			      double axis_rotation,
			      ui08 **grid);

static void
load_polygon_forecast_grid(storm_file_params_t *sparams,
			   track_file_params_t *tparams,
			   storm_file_scan_header_t *scan,
			   storm_file_global_props_t *gprops,
			   track_file_entry_t *entry,
			   double lead_time,
			   ui08 **grid);

static void load_polygon_truth_grid(storm_file_params_t *sparams,
				    storm_file_scan_header_t *scan,
				    storm_file_global_props_t *gprops,
				    ui08 **grid);

static void load_runs_truth_grid(vt_storm_t *storm,
				 ui08 **grid);

static int point_in_polygon(double centroid_x,
			    double centroid_y,
			    double *radials,
			    double start_az,
			    double delta_az,
			    long n_sides,
			    double grid_dx,
			    double grid_dy,
			    double search_x,
			    double search_y);

/****************************************************************************
 * load_forecast_grid()
 *
 * loads the forecast grid based on the forecast for the current storm
 *
 ****************************************************************************/

void load_forecast_grid(storm_file_handle_t *s_handle,
			track_file_handle_t *t_handle,
			track_file_entry_t *entry,
			storm_file_global_props_t *gprops,
			double lead_time,
			ui08 **grid)

{

  storm_file_params_t *sparams;
  track_file_params_t *tparams;

  sparams = &s_handle->header->params;
  tparams = &t_handle->header->params;
			    
  if (Glob->forecast_method == STORM_ELLIPSE) {

    load_ellipse_forecast_grid(sparams,
			       tparams,
			       s_handle->scan,
			       gprops,
			       entry,
			       lead_time,
			       grid);

  } else if (Glob->forecast_method == STORM_POLYGON) {

    load_polygon_forecast_grid(sparams,
			       tparams,
			       s_handle->scan, gprops,
			       entry,
			       lead_time, grid);
    
  }

}

/**************************************************************************
 * load_truth_grid()
 *
 **************************************************************************/

void load_truth_grid(storm_file_handle_t *s_handle,
		     vt_storm_t *storm,
		     ui08 **grid)

{

  storm_file_params_t *sparams;
  sparams = &s_handle->header->params;

  if (Glob->verify_method == STORM_ELLIPSE) {
  
    load_ellipse_truth_grid(sparams, s_handle->scan,
			    &storm->gprops, grid);

  } else if (Glob->verify_method == STORM_POLYGON) {
    
    load_polygon_truth_grid(sparams, s_handle->scan,
			    &storm->gprops, grid);
    
  } else if (Glob->verify_method == STORM_RUNS) {

    load_runs_truth_grid(storm, grid);
    
  }

}

/****************************************************************************
 * load_ellipse_forecast_grid.c
 *
 * loads the forecast grid based on the ellipse forecast for the current storm
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * January 1992
 *
 ****************************************************************************/

static void
load_ellipse_forecast_grid(storm_file_params_t *sparams,
			   track_file_params_t *tparams,
			   storm_file_scan_header_t *scan,
			   storm_file_global_props_t *gprops,
			   track_file_entry_t *entry,
			   double lead_time,
			   ui08 **grid)

{

  double f_area;
  double f_proj_area;
  double f_centroid_x;
  double f_centroid_y;
  double f_ellipse_scale;
  double f_major_radius;
  double f_minor_radius;
  double f_orientation ;
  double lead_time_hr, growth_period_hr;

  track_file_forecast_props_t *fprops = &entry->dval_dt;

  lead_time_hr = lead_time / 3600.0;
  growth_period_hr = Glob->forecast_growth_period / 3600.0;

  /*
   * compute the area forecast - only update the forecast
   * grid if this is greater than 1.0
   */

  f_area = gprops->proj_area + fprops->proj_area * lead_time_hr;
  
  if (f_area >= 1.0) {

    if (Glob->zero_growth) {

      f_proj_area = gprops->proj_area;

    } else if (Glob->parabolic_growth) {

      if (fprops->proj_area < 0.0) {

	/*
	 * linear on decay
	 */

	f_proj_area = gprops->proj_area +
	  fprops->proj_area * lead_time_hr;

      } else {

	/*
	 * parabloic on growth
	 */
	
	f_proj_area = gprops->proj_area +
	  fprops->proj_area * lead_time_hr -
	  ((fprops->proj_area * lead_time_hr * lead_time_hr) /
	   (growth_period_hr * 2.0));
	
      }
	
    } else {
      
      f_proj_area = gprops->proj_area +
	fprops->proj_area * lead_time_hr;

    }
    
    if (f_proj_area < 1.0) {
      f_proj_area = 1.0;
    }
    
    f_centroid_x =
      (gprops->proj_area_centroid_x +
       fprops->proj_area_centroid_x * lead_time_hr);
    
    f_centroid_y =
      (gprops->proj_area_centroid_y +
       fprops->proj_area_centroid_y * lead_time_hr);
    
    f_ellipse_scale =
      (sqrt(f_proj_area / gprops->proj_area)) *
	Glob->forecast_scale_factor;
    
    f_major_radius =
      gprops->proj_area_major_radius * f_ellipse_scale;
    
    f_minor_radius =
      gprops->proj_area_minor_radius * f_ellipse_scale;
    
    f_orientation =  gprops->proj_area_orientation;
    
    if (Glob->debug) {
      
      fprintf(stderr, "forecast ellipse\n");
      fprintf(stderr,
	      "x, y, maj_r, min_r, theta : %g, %g, %g, %g, %g\n",
	      f_centroid_x, f_centroid_y,
	      f_major_radius, f_minor_radius, f_orientation);
      
    }

    /*
     * update the grid
     */
    
    load_ellipse_grid(f_centroid_x, f_centroid_y,
		      f_major_radius, f_minor_radius,
		      f_orientation,
		      grid);
    
  } /* if (f_area >= 1.0) */
  
}

/***************************************************************************
 * load_ellipse_truth_grid()
 *
 * loads the ellipse truth grid based on the ellipse props
 * for the actual storm at the forecast time
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * January 1992
 *
 ***************************************************************************/

static void
load_ellipse_truth_grid(storm_file_params_t *sparams,
			storm_file_scan_header_t *scan,
			storm_file_global_props_t *gprops,
			ui08 **grid)

{

  double proj_area;
  double centroid_x;
  double centroid_y;
  double major_radius;
  double minor_radius;
  double orientation ;

  proj_area = gprops->proj_area;
    
  if (proj_area < 1.0) {
    proj_area = 1.0;
    
  }

  centroid_x = gprops->proj_area_centroid_x;
  centroid_y = gprops->proj_area_centroid_y;
  major_radius = gprops->proj_area_major_radius;
  minor_radius = gprops->proj_area_minor_radius;
  orientation = gprops->proj_area_orientation;

  if (Glob->debug) {

    fprintf(stderr, "verification ellipse\n");
    fprintf(stderr,
	    "x, y, maj_r, min_r, theta : %g, %g, %g, %g, %g\n",
	    centroid_x, centroid_y,
	    major_radius, minor_radius, orientation);

  }

  /*
   * update the grid
   */
    
  load_ellipse_grid(centroid_x, centroid_y,
		    major_radius, minor_radius,
		    orientation,
		    grid);
  
}

/*********************************************************************
 * load_ellipse_grid()
 *
 * Flags regions in the grid with 1's if the given
 * projected area ellipse crosses into or contains the region.
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
 * RAP, NCAR, Boulder CO
 *
 * November 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

static void load_ellipse_grid(double ellipse_x,
			      double ellipse_y,
			      double major_radius,
			      double minor_radius,
			      double axis_rotation,
			      ui08 **grid)

{

  long ix, iy, ix1, iy1, ix2, iy2;
  long start_ix, start_iy;
  long end_ix, end_iy;

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

  start_ix = (long) ((start_x - Glob->minx) / Glob->dx + 0.49);
  start_ix = MAX(start_ix, 0);

  start_iy = (long) ((start_y - Glob->miny) / Glob->dy + 0.49);
  start_iy = MAX(start_iy, 0);

  end_ix = (long) ((end_x - Glob->minx) / Glob->dx + 0.51);
  end_ix = MIN(end_ix, Glob->nx - 1);

  end_iy = (long) ((end_y - Glob->miny) / Glob->dy + 0.51);
  end_iy = MIN(end_iy, Glob->ny - 1);

  /*
   * flag the grid region which contains the ellipse centroid
   */

  ix = (long) ((ellipse_x - Glob->minx) / Glob->dx + 0.5);
  iy = (long) ((ellipse_y - Glob->miny) / Glob->dy + 0.5);

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

    
    line_x = Glob->minx + ((double) ix + 0.5) * Glob->dx;

    slope_prime = 1.0 / tan_rotation;

    intercept_prime  = - (line_x - ellipse_x) / sin_rotation;

    if (uline_through_ellipse(major_radius, minor_radius,
			      slope_prime, intercept_prime,
			      &xprime1, &yprime1,
			      &xprime2, &yprime2) == TRUE) {

      /*
       * transform the points back into grid coords
       */

      y_1 = ellipse_y + xprime1 * sin_rotation + yprime1 * cos_rotation;
      y_2 = ellipse_y + xprime2 * sin_rotation + yprime2 * cos_rotation;

      if  (y_1 <= y_2) {

	iy1 = (long) ((y_1 - Glob->miny) / Glob->dy + 0.5);
	iy2 = (long) ((y_2 - Glob->miny) / Glob->dy + 0.5);

      } else {

	iy1 = (long) ((y_2 - Glob->miny) / Glob->dy + 0.5);
	iy2 = (long) ((y_1 - Glob->miny) / Glob->dy + 0.5);

      }

      iy1 = MAX(iy1, 0);
      iy2 = MIN(iy2, Glob->ny - 1);

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

    
    line_y = Glob->miny + ((double) iy + 0.5) * Glob->dy;

    slope_prime = - tan_rotation;

    intercept_prime  = (line_y - ellipse_y) / cos_rotation;

    if (uline_through_ellipse(major_radius, minor_radius,
			      slope_prime, intercept_prime,
			      &xprime1, &yprime1,
			      &xprime2, &yprime2) == TRUE) {

      /*
       * transform the points back into grid coords
       */

      x_1 = ellipse_x + xprime1 * cos_rotation - yprime1 * sin_rotation;
      x_2 = ellipse_x + xprime2 * cos_rotation - yprime2 * sin_rotation;

      if  (x_1 <= x_2) {

	ix1 = (long) ((x_1 - Glob->minx) / Glob->dx + 0.5);
	ix2 = (long) ((x_2 - Glob->minx) / Glob->dx + 0.5);

      } else {

	ix1 = (long) ((x_2 - Glob->minx) / Glob->dx + 0.5);
	ix2 = (long) ((x_1 - Glob->minx) / Glob->dx + 0.5);

      }

      ix1 = MAX(ix1, 0);
      ix2 = MIN(ix2, Glob->nx - 1);

      for (ix = ix1; ix <= ix2; ix++) {

	grid[iy][ix] = 1;
	grid[iy + 1][ix] = 1;

      } /* ix */

    } /* if (uline_through_ellipse(major_radius ... */

  } /* iy */

}

/***********************************************************************
 * load_polygon_forecast_grid()
 *
 * loads the forecast grid based on the polygon forecast
 * for the current storm
 *
 ***********************************************************************/

static void
load_polygon_forecast_grid(storm_file_params_t *sparams,
			   track_file_params_t *tparams,
			   storm_file_scan_header_t *scan,
			   storm_file_global_props_t *gprops,
			   track_file_entry_t *entry,
			   double lead_time,
			   ui08 **grid)

{
  
  long ix, iy, iray;
  long n_sides;
  long min_ix, min_iy, max_ix, max_iy;

  double f_area;
  double f_proj_area;
  double f_polygon_scale;
  double cart_dx, cart_dy;
  double start_az, delta_az;
  double start_az_rad, delta_az_rad;
  double theta, range[N_POLY_SIDES];
  double x, y;
  double search_x, search_y;
  double min_x, min_y, max_x, max_y;
  double lead_time_hr, growth_period_hr;
  
  point_t f_centroid;
  
  track_file_forecast_props_t *fprops = &entry->dval_dt;
  
  lead_time_hr = lead_time / 3600.0;
  growth_period_hr = Glob->forecast_growth_period / 3600.0;

  /*
   * compute the area forecast - only update the forecast
   * grid if this is greater than 1.0
   */
  
  f_area =
    (gprops->proj_area + fprops->proj_area * lead_time_hr);
  
  if (f_area >= 1.0) {
    
    if (Glob->zero_growth) {

      f_proj_area = gprops->proj_area;

    } else if (Glob->parabolic_growth) {

      if (fprops->proj_area < 0.0) {

	/*
	 * linear on decay
	 */

	f_proj_area = gprops->proj_area +
	  fprops->proj_area * lead_time_hr;

      } else {

	/*
	 * parabloic on growth
	 */
	
	f_proj_area = gprops->proj_area +
	  fprops->proj_area * lead_time_hr -
	  ((fprops->proj_area * lead_time_hr * lead_time_hr) /
	   (growth_period_hr * 2.0));
	
      }
	
    } else {
      
      f_proj_area = gprops->proj_area +
	fprops->proj_area * lead_time_hr;

    }
    
    if (f_proj_area < 1.0)
      f_proj_area = 1.0;
    
    f_centroid.x =
      (gprops->proj_area_centroid_x +
       fprops->proj_area_centroid_x * lead_time_hr);
    
    f_centroid.y =
      (gprops->proj_area_centroid_y +
       fprops->proj_area_centroid_y * lead_time_hr);
    
    f_polygon_scale =
      (sqrt(f_proj_area / gprops->proj_area)) *
	Glob->forecast_scale_factor;
    
    /*
     * compute the polygon points
     */
    
    cart_dx = scan->grid.dx;
    cart_dy = scan->grid.dy;
    
    start_az = sparams->poly_start_az;
    delta_az = sparams->poly_delta_az;
    start_az_rad = start_az * DEG_TO_RAD;
    delta_az_rad = delta_az * DEG_TO_RAD;
    n_sides = sparams->n_poly_sides;
    
    theta = start_az_rad;
    
    min_x = LARGE_DOUBLE;
    max_x = -LARGE_DOUBLE;
    min_y = LARGE_DOUBLE;
    max_y = -LARGE_DOUBLE;
    
    for (iray = 0; iray < n_sides; iray++) {
      
      range[iray] =
	gprops->proj_area_polygon[iray] * f_polygon_scale;
    
      x = f_centroid.x + range[iray] * sin(theta) * cart_dx;
      y = f_centroid.y + range[iray] * cos(theta) * cart_dy;
      
      min_x = MIN(x, min_x);
      max_x = MAX(x, max_x);
      min_y = MIN(y, min_y);
      max_y = MAX(y, max_y);
      
      theta += delta_az_rad;
      
    } /* iray */
    
    min_ix = (long) ((min_x - Glob->minx) / Glob->dx + 0.5) - 1; 
    max_ix = (long) ((max_x - Glob->minx) / Glob->dx + 0.5) + 1;
    min_iy = (long) ((min_y - Glob->miny) / Glob->dy + 0.5) - 1;
    max_iy = (long) ((max_y - Glob->miny) / Glob->dy + 0.5) + 1;
    
    DO_CONSTRAIN(min_ix, 1, Glob->nx - 1);
    DO_CONSTRAIN(max_ix, 1, Glob->nx - 1);
    DO_CONSTRAIN(min_iy, 1, Glob->ny - 1);
    DO_CONSTRAIN(max_iy, 1, Glob->ny - 1);
    
    search_y = Glob->miny + ((double) min_iy - 0.5) * Glob->dy;
    
    for (iy = min_iy; iy <= max_iy; iy++) {
      
      search_x = Glob->minx + ((double) min_ix - 0.5) * Glob->dx;
      
      for (ix = min_ix; ix <= max_ix; ix++) {
	
	if (point_in_polygon(f_centroid.x, f_centroid.y, range,
			     start_az, delta_az, n_sides,
			     cart_dx, cart_dy,
			     search_x, search_y)) {
	  
	  grid[iy-1][ix-1] = TRUE;
	  grid[iy-1][ix] = TRUE;
	  grid[iy][ix-1] = TRUE;
	  grid[iy][ix] = TRUE;
	  
	}
	
	search_x += Glob->dx;
	
      } /* ix */
      
      search_y += Glob->dy;
      
    } /* iy */
    
  } /* if (f_area >= 1.0) */
  
}

/*************************************************************************
 * load_polygon_truth_grid()
 *
 * loads the truth grid based on the polygon for the current storm
 *
 *************************************************************************/

static void load_polygon_truth_grid(storm_file_params_t *sparams,
				    storm_file_scan_header_t *scan,
				    storm_file_global_props_t *gprops,
				    ui08 **grid)

{

  long ix, iy, iray;
  long n_sides;
  long min_ix, min_iy, max_ix, max_iy;

  double polygon_scale;
  double cart_dx, cart_dy;
  double start_az, delta_az;
  double start_az_rad, delta_az_rad;
  double theta, range[N_POLY_SIDES];
  double x, y;
  double search_x, search_y;
  double min_x, min_y, max_x, max_y;

  point_t centroid;

  centroid.x = gprops->proj_area_centroid_x;
  centroid.y = gprops->proj_area_centroid_y;
  polygon_scale = Glob->forecast_scale_factor;
    
  if (Glob->debug) {
    fprintf(stderr, "\ntruth polygon\n");
    fprintf(stderr,
	    "x, y, scale : %g, %g, %g\n",
	    centroid.x, centroid.y, polygon_scale);
  }

  /*
   * compute the polygon points
   */
  
  cart_dx = (scan->grid.dx) * polygon_scale;
  cart_dy = (scan->grid.dy) * polygon_scale;
  
  start_az = (sparams->poly_start_az);
  delta_az = (sparams->poly_delta_az);
  start_az_rad = start_az * DEG_TO_RAD;
  delta_az_rad = delta_az * DEG_TO_RAD;
  n_sides = sparams->n_poly_sides;
  
  theta = start_az_rad;

  min_x = LARGE_DOUBLE;
  max_x = -LARGE_DOUBLE;
  min_y = LARGE_DOUBLE;
  max_y = -LARGE_DOUBLE;

  for (iray = 0; iray < n_sides; iray++) {
    
    range[iray] = gprops->proj_area_polygon[iray];
    x = centroid.x + range[iray] * sin(theta) * cart_dx;
    y = centroid.y + range[iray] * cos(theta) * cart_dy;
    
    min_x = MIN(x, min_x);
    max_x = MAX(x, max_x);
    min_y = MIN(y, min_y);
    max_y = MAX(y, max_y);

    theta += delta_az_rad;
    
  } /* iray */
  
  min_ix = (long) ((min_x - Glob->minx) / Glob->dx + 0.5) - 1; 
  max_ix = (long) ((max_x - Glob->minx) / Glob->dx + 0.5) + 1;
  min_iy = (long) ((min_y - Glob->miny) / Glob->dy + 0.5) - 1;
  max_iy = (long) ((max_y - Glob->miny) / Glob->dy + 0.5) + 1;

  DO_CONSTRAIN(min_ix, 1, Glob->nx - 1);
  DO_CONSTRAIN(max_ix, 1, Glob->nx - 1);
  DO_CONSTRAIN(min_iy, 1, Glob->ny - 1);
  DO_CONSTRAIN(max_iy, 1, Glob->ny - 1);
  
  search_y = Glob->miny + ((double) min_iy - 0.5) * Glob->dy;
  
  for (iy = min_iy; iy <= max_iy; iy++) {
    
    search_x = Glob->minx + ((double) min_ix - 0.5) * Glob->dx;
    
    for (ix = min_ix; ix <= max_ix; ix++) {
      
      if (point_in_polygon(centroid.x, centroid.y, range,
			   start_az, delta_az, n_sides,
			   cart_dx, cart_dy,
			   search_x, search_y)) {
	  
	grid[iy-1][ix-1] = TRUE;
	grid[iy-1][ix] = TRUE;
	grid[iy][ix-1] = TRUE;
	grid[iy][ix] = TRUE;
	
      }
      
      search_x += Glob->dx;
      
    } /* ix */
    
    search_y += Glob->dy;
    
  } /* iy */
  
}

/**************************************************************************
 * load_runs_truth_grid()
 *
 **************************************************************************/

static void load_runs_truth_grid(vt_storm_t *storm,
				 ui08 **grid)

{

  long i, ix, irun;
  long lx, ly;
  long *x_lookup, *y_lookup;

  storm_file_run_t *run;

  x_lookup = storm->x_lookup;
  y_lookup = storm->y_lookup;

  /*
   * loop through runs
   */

  run = storm->runs;
  
  for (irun = 0; irun < storm->gprops.n_runs; irun++) {

    ly = y_lookup[run->iy];

    if (ly >= 0) {

      ix = run->ix;

      for (i = 0; i < run->n; i++) {

	lx = x_lookup[ix];
      
	if (lx >= 0)
	  grid[ly][lx] = TRUE;

	ix++;

      } /* i */

    } /* if (ly >= 0) */
    
    run++;

  } /* irun */

}

/*************************************************************************
 *
 * point_in_polygon()
 *
 * Tests if a point is within the storm polygon
 *
 * The polygon is specified as a series of radial distances from the
 * centroid. The units are in terms of the grid units in which the
 * polygon was originally derived.
 *
 ****************************************************************************/

static int point_in_polygon(double centroid_x,
			    double centroid_y,
			    double *radials,
			    double start_az,
			    double delta_az,
			    long n_sides,
			    double grid_dx,
			    double grid_dy,
			    double search_x,
			    double search_y)

{

  long az_num, next_az_num;

  double delta_x;
  double delta_y;
  double dist, bdist;
  double az_pt, az1;
  double l1, l2, dl;

  /*
   * get the search point relative to the centroid in
   * grid coords
   */

  delta_x = (search_x - centroid_x) / grid_dx;
  delta_y = (search_y - centroid_y) / grid_dy;
  
  dist = sqrt (delta_x * delta_x + delta_y * delta_y);
  
  if (delta_x == 0.0 && delta_y == 0.0) {
    az_pt = 0.0;
  } else {
    az_pt = atan2(delta_x, delta_y) * RAD_TO_DEG;
    if (az_pt < 0)
      az_pt += 360.0;
  }

  /*
   * calculate the index of the ray just below the point in the polygon
   */
  
  az_num = (long) ((az_pt - start_az) / delta_az);
  
  if (az_num < 0)
    az_num += n_sides;

  next_az_num = (az_num + 1) % n_sides;

  /*
   * get the lengths of the line polygon rays on either side of the
   * point - these are in grid coords
   */

  l1 = radials[az_num];
  l2 = radials[next_az_num];
  dl = l2 - l1;

  /*
   * interpolate the ray lengths to determine the boundary distance
   * from the centroid at the azimuth of the point
   */

  az1 = start_az + delta_az * (double) az_num;

  bdist = l1 + ((az_pt - az1) / delta_az) * dl;

  /*
   * if the boundary distance is greater than the distance of the
   * point from the centroid, the point is  within the polygon
   */

  if (dist <= bdist)
    return (TRUE);
  else
    return (FALSE);

}
