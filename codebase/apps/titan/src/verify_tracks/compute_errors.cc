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
/***************************************************************************
 * compute_errors.c
 *
 * Compute the bias and rmse
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * january 1992
 *
 ***************************************************************************/

#include "verify_tracks.h"

#define KM_PER_DEG_AT_EQUATOR 111.12

/*
 * file_scope variables
 */

static track_file_forecast_props_t *PropsForecast;
static track_file_forecast_props_t *PropsVerify;
static vt_stats_t *ComplexStats;
static vt_stats_t *FileStats;
static vt_stats_t *TotalStats;

/*
 * prototypes
 */

static void compute_spd_and_dirn(track_file_forecast_props_t *current,
				 track_file_forecast_props_t *future,
				 double x_scale_factor,
				 double y_scale_factor);

static void load_errors(fl32 *forecast_ptr,
			double scale_factor,
			int vol_weighted,
			int dirn_prop);

double compute_dist_error(track_file_forecast_props_t *props_forecast,
                          track_file_forecast_props_t *props_verify,
                          double x_scale_factor,
                          double y_scale_factor);
		      
/*
 * main routine
 */

void compute_errors(storm_file_handle_t *s_handle,
		    long nverify,
		    track_file_forecast_props_t *props_current,
		    track_file_forecast_props_t *props_forecast,
		    track_file_forecast_props_t *props_verify,
		    vt_stats_t *complex_stats,
		    vt_stats_t *file_stats,
		    vt_stats_t *total_stats)

{

  int grid_type;
  double mean_lat, cos_lat;
  double x_scale_factor, y_scale_factor;
  double delta_r; /* rjp 9 Apr 2002 */

  /*
   * set pointers
   */

  PropsForecast = props_forecast;
  PropsVerify = props_verify;
  ComplexStats = complex_stats;
  FileStats = file_stats;
  TotalStats = total_stats;

  /*
   * for LATLON grids compute the scale factors for
   * converting diffs in lat/lon into km
   */

  grid_type = s_handle->scan->grid.proj_type;
  
  if (grid_type == TITAN_PROJ_LATLON) {

    /*
     * compute mean lat
     */

    mean_lat =
      props_current->proj_area_centroid_y / props_current->volume;
    cos_lat = cos(mean_lat * DEG_TO_RAD);
    x_scale_factor = KM_PER_DEG_AT_EQUATOR * cos_lat;
    y_scale_factor = KM_PER_DEG_AT_EQUATOR;

  } else {

    x_scale_factor = 1.0;
    y_scale_factor = 1.0;

  }

  /*
   * load up errors for selected props
   */

  if (nverify > 0) {

    /*
     * speed and direction comps
     */

    compute_spd_and_dirn(props_current,
			 props_forecast,
			 x_scale_factor,
			 y_scale_factor);
    
    compute_spd_and_dirn(props_current,
			 props_verify,
			 x_scale_factor,
			 y_scale_factor);

    load_errors(&PropsForecast->proj_area_centroid_x,
		x_scale_factor, TRUE, FALSE);
    load_errors(&PropsForecast->proj_area_centroid_y,
		y_scale_factor, TRUE, FALSE);
    load_errors(&PropsForecast->vol_centroid_z, 1.0, TRUE, FALSE);
    load_errors(&PropsForecast->refl_centroid_z, 1.0, TRUE, FALSE);
    load_errors(&PropsForecast->top, 1.0, TRUE, FALSE);
    load_errors(&PropsForecast->smoothed_speed, 1.0, FALSE, FALSE);
    load_errors(&PropsForecast->smoothed_direction, 1.0, FALSE, TRUE);

    /* 
     * calculate distance between forecast and verify locations 
     * and load into error stats structures
     * rjp 9 Apr 2002
     */ 
    delta_r=compute_dist_error(props_forecast,
                       props_verify,
                       x_scale_factor,
                       y_scale_factor);

    complex_stats->sum_dist_error += delta_r ;
    complex_stats->sum_sq_dist_error += delta_r*delta_r ;
    file_stats->sum_dist_error += delta_r ;
    file_stats->sum_sq_dist_error += delta_r*delta_r ;
    total_stats->sum_dist_error += delta_r ;
    total_stats->sum_sq_dist_error += delta_r*delta_r ;

    /*
     * increment counters
     */
    
    complex_stats->n_movement++;
    file_stats->n_movement++;
    total_stats->n_movement++;

  } /* if (nverify > 0) */

  load_errors(&PropsForecast->dbz_max, 1.0, FALSE, FALSE);
  load_errors(&PropsForecast->volume, 1.0, FALSE, FALSE);
  load_errors(&PropsForecast->precip_flux, 1.0, FALSE, FALSE);
  load_errors(&PropsForecast->mass, 1.0, FALSE, FALSE);
  load_errors(&PropsForecast->proj_area, 1.0, FALSE, FALSE);

  /*
   * increment counters
   */

  complex_stats->n_growth++;
  file_stats->n_growth++;
  total_stats->n_growth++;

}


/*********************************************************************
 * compute_spd_and_dirn()
 *
 * compute the speed and direction from dx and dy
 *
 *********************************************************************/

static void compute_spd_and_dirn(track_file_forecast_props_t *current,
				 track_file_forecast_props_t *future,
				 double x_scale_factor,
				 double y_scale_factor)

{

  double dx_km, dy_km;
  double speed, direction;
  double xx1, xx2, yy1, yy2;

  /*
   * get start and end coords - the positional parameters
   * are weighted by volume
   */

  xx1 = current->proj_area_centroid_x / current->volume;
  yy1 = current->proj_area_centroid_y / current->volume;

  xx2 = future->proj_area_centroid_x / future->volume;
  yy2 = future->proj_area_centroid_y / future->volume;

  dx_km = (xx2 - xx1) * x_scale_factor;
  dy_km = (yy2 - yy1) * y_scale_factor;

  speed = sqrt(dx_km * dx_km + dy_km * dy_km);

  if (dx_km == 0.0 && dy_km == 0.0)
    direction = 0.0;
  else
    direction = atan2(dx_km, dy_km) * RAD_TO_DEG;

  if (direction < 0.0)
    direction += 360.0;

  future->smoothed_speed = speed;
  future->smoothed_direction = direction;

}


/*********************************************************************
 * compute_dist_error()
 *
 * compute the distance between forecast and verify positions
 * from dx and dy
 *
 *********************************************************************/

double compute_dist_error(track_file_forecast_props_t *forecast,
			  track_file_forecast_props_t *verify,
			  double x_scale_factor,
			  double y_scale_factor)

{

  double dx_km, dy_km, range;
  double xx1, xx2, yy1, yy2;

  /*
   * get forecast and verify coords 
   * Note the positional parameters are weighted by volume
   */

  xx1 = forecast->proj_area_centroid_x / forecast->volume;
  yy1 = forecast->proj_area_centroid_y / forecast->volume;

  xx2 = verify->proj_area_centroid_x / verify->volume;
  yy2 = verify->proj_area_centroid_y / verify->volume;

  dx_km = (xx2 - xx1) * x_scale_factor;
  dy_km = (yy2 - yy1) * y_scale_factor;

  range = sqrt(dx_km * dx_km + dy_km * dy_km);

  return(range);

}


/**********************************************************************
 * load_errors()
 * 
 * compute and load absolute and normalized errors
 */

static void load_errors(fl32 *forecast_ptr,
			double scale_factor,
			int vol_weighted,
			int dirn_prop)
		      
{

  long offset;
  double forecast_val, verify_val;
  double norm_error;
  double sq_error;
  double error;
  double norm_sq_error;

  /*
   * compute the offset into the struct
   */

  offset = forecast_ptr - (fl32 *) PropsForecast;

  /*
   * movement props are weighted by volume
   */
  
  if (vol_weighted) {

    if (PropsForecast->volume == 0.0) {
      forecast_val = 0.0;
    } else {
      forecast_val = *forecast_ptr / PropsForecast->volume;
    }

    if (PropsVerify->volume == 0.0) {
      verify_val = 0.0;
      forecast_val = 0.0;
    } else {
      verify_val = *((fl32 *) PropsVerify + offset) / PropsVerify->volume;
    }

  } else {

    forecast_val = *forecast_ptr;
    verify_val = *((fl32 *) PropsVerify + offset);

  }

  forecast_val *= scale_factor;
  verify_val *= scale_factor;

  error = forecast_val - verify_val;

  /*
   * for direction, allow for the problems at the North line
   */

  if (dirn_prop) {
    if (error > 180.0)
      error -= 360.0;
    else if (error < -180.0)
      error += 360.0;
  } /* if (dirn_prop) */

  sq_error = error * error;

  if (verify_val != 0.0) {
    norm_error = error / verify_val;
    norm_sq_error = sq_error / (verify_val * verify_val);
  } else {
    norm_error = 0.0;
    norm_sq_error = 0.0;
  } 

  *((fl32 *) &ComplexStats->sum_error + offset) += error;
  *((fl32 *) &ComplexStats->sum_sq_error + offset) += sq_error;
  *((fl32 *) &ComplexStats->norm_sum_error + offset) += norm_error;
  *((fl32 *) &ComplexStats->norm_sum_sq_error + offset) += norm_sq_error;
  
  *((fl32 *) &FileStats->sum_error + offset) += error;
  *((fl32 *) &FileStats->sum_sq_error + offset) += sq_error;
  *((fl32 *) &FileStats->norm_sum_error + offset) += norm_error;
  *((fl32 *) &FileStats->norm_sum_sq_error + offset) += norm_sq_error;
  
  *((fl32 *) &TotalStats->sum_error + offset) += error;
  *((fl32 *) &TotalStats->sum_sq_error + offset) += sq_error;
  *((fl32 *) &TotalStats->norm_sum_error + offset) += norm_error;
  *((fl32 *) &TotalStats->norm_sum_sq_error + offset) += norm_sq_error;

  /*
   * correlation coefficient
   */

  *((fl32 *) &TotalStats->sumx + offset) += verify_val;
  *((fl32 *) &TotalStats->sumx2 + offset) += verify_val * verify_val;
  *((fl32 *) &TotalStats->sumy + offset) += forecast_val;
  *((fl32 *) &TotalStats->sumy2 + offset) += forecast_val * forecast_val;
  *((fl32 *) &TotalStats->sumxy + offset) += verify_val * forecast_val;
  
}
