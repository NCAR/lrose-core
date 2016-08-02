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
 * smooth_spatial_forecasts.c
 *
 * Perform spatial smoothing on the forecast vectors
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1994
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "storm_track.h"
#include <rapmath/math_macros.h>

typedef struct {
  double x, y;
} xypair_t;

/*
 * prototypes
 */

static void copy_props(track_status_t *track);

static void get_extreme_coords(storm_status_t *storms,
			       int nstorms,
			       xypair_t *max_coord,
			       xypair_t *min_coord);

static void load_distance_array(storm_status_t *storms,
				int nstorms,
				fl32 **distance_array,
				double smoothing_radius,
				int flat_proj);
				
static void perform_smoothing(storm_status_t *storms,
			      int nstorms,
			      fl32 **distance_array,
			      double smoothing_radius);

/*
 * main routine
 */

void smooth_spatial_forecasts(storm_file_handle_t *s_handle,
			      storm_status_t *storms,
			      int nstorms)

{

  int i;
  int flat_proj;
  fl32 **distance_array;
  double smoothing_radius;
  xypair_t max_coord, min_coord;

  if (!Glob->params.spatial_smoothing) {
    for (i = 0; i < nstorms; i++) {
      copy_props(storms[i].track);
    }
    return;
  }

  /*
   * set grid-type flag
   */

  if (s_handle->scan->grid.proj_type == TITAN_PROJ_FLAT) {
    flat_proj = TRUE;
  } else {
    flat_proj = FALSE;
  }

  /*
   * set smoothing radius
   */
  
  smoothing_radius = Glob->params.smoothing_radius;
  if (smoothing_radius < 1.0) {
    smoothing_radius = 1.0;
  }

  /*
   * get max and min (x, y) vals
   */

  get_extreme_coords(storms, nstorms, &max_coord, &min_coord);
  
  /*
   * allocate the distance array. This is a 2-d array indicating
   * which storms are within the smoothing_radius of each other.
   * If storms are within the radius, the value is set to the 
   * distance between them in km. If not, the value is set to -1.
   */

  distance_array = (fl32 **) umalloc2
    ((ui32) nstorms, (ui32) nstorms, sizeof(fl32));

  /*
   * load up the distance array. This is a 2-d array indicating
   * which storms are within the smoothing_radius of each other
   */
  
  load_distance_array(storms, nstorms,
		      distance_array,
		      smoothing_radius,
		      flat_proj);
		      
  /*
   * perform the smoothing
   */
  
  perform_smoothing(storms, nstorms,
		    distance_array,
		    smoothing_radius);

  /*
   * free up
   */

  ufree2((void **) distance_array);
  
}

/*********************************************************************
 * copy_props()
 *
 * Just copy props across, no smoothing
 */

static void copy_props(track_status_t *track)

{

  /*
   * copy storm vals to smoothed stats
   */
  
  track->dval_dt.smoothed_proj_area_centroid_x =
    track->dval_dt.proj_area_centroid_x;
  
  track->dval_dt.smoothed_proj_area_centroid_y =
    track->dval_dt.proj_area_centroid_y;
  
  track->smoothed_history_in_secs =
    track->history_in_secs;
  
}

/*********************************************************************
 * get_extreme_coords()
 *
 * get the extreme coord values for the current storms
 */

static void get_extreme_coords(storm_status_t *storms,
			       int nstorms,
			       xypair_t *max_coord,
			       xypair_t *min_coord)

{

  int i;
  storm_track_props_t *current;
  
  /*
   * initialize
   */

  max_coord->x = -LARGE_DOUBLE;
  max_coord->y = -LARGE_DOUBLE;
  min_coord->x = LARGE_DOUBLE;
  min_coord->y = LARGE_DOUBLE;

  for (i = 0; i < nstorms; i++) {

    current = &storms[i].current;

    max_coord->x =
      MAX(max_coord->x, current->proj_area_centroid_x);
    
    max_coord->y =
      MAX(max_coord->y, current->proj_area_centroid_y);

    min_coord->x =
      MIN(min_coord->x, current->proj_area_centroid_x);
    
    min_coord->y =
      MIN(min_coord->y, current->proj_area_centroid_y);

  } /* i */

}

/*********************************************************************
 * load_distance_array()
 *
 */

static void load_distance_array(storm_status_t *storms,
				int nstorms,
				fl32 **distance_array,
				double smoothing_radius,
				int flat_proj)
				
{

  int i, j;
  int min_history = Glob->params.min_history_for_valid_forecast;

  double this_centroid_x, this_centroid_y;
  double other_centroid_x, other_centroid_y;
  double distance;
  double dx, dy;
  double theta;

  storm_status_t *this_storm, *other_storm;

  /*
   * Initialize the array to -1.0.
   * The -1's will be overwritten by the actual distance
   * for storms which are closer than the smoothing distance.
   */

  for (i = 0; i < nstorms; i++) {
    for (j = 0; j < nstorms; j++) {
      distance_array[i][j] = -1.0;
    }
  }
  
  /*
   * load array
   */
  
  for (i = 0; i < nstorms; i++) {
    
    this_storm = storms + i;
    
    /*
     * On the diagonal (i.e. for this_storm) set to 0.0 if
     * the history_in_secs exceeds min_history.
     */
    
    if (this_storm->track->history_in_secs >= min_history) {
      distance_array[i][i] = 0.0;
    }
    
    this_centroid_x = this_storm->current.proj_area_centroid_x;
    this_centroid_y = this_storm->current.proj_area_centroid_y;
    
    for (j = i + 1; j < nstorms; j++) {
      
      other_storm = storms + j;
      
      other_centroid_x = other_storm->current.proj_area_centroid_x;
      other_centroid_y = other_storm->current.proj_area_centroid_y;
      
      if (flat_proj) {
	
	dx = this_centroid_x - other_centroid_x;
	dy = this_centroid_y - other_centroid_y;
	distance = sqrt(dx * dx + dy * dy);
	
      } else {
	
	uLatLon2RTheta(this_centroid_y, this_centroid_x,
		       other_centroid_y, other_centroid_x,
		       &distance, &theta);
	
      } /* if (flat_proj) */

      if (distance < smoothing_radius) {

	if (other_storm->track->history_in_secs >= min_history) {
	  distance_array[i][j] = distance;
	}
	if (this_storm->track->history_in_secs >= min_history) {
	  distance_array[j][i] = distance;
	}

      } /* if (distance < smoothing_radius) */
	
    } /* j */
    
  } /* i */

}

/*********************************************************************
 * perform_smoothing()
 *
 * This is an inverse-distance weighting function
 */

static void perform_smoothing(storm_status_t *storms,
			      int nstorms,
			      fl32 **distance_array,
			      double smoothing_radius)

{

  int i, j;
  int min_history = Glob->params.min_history_for_valid_forecast;
  int smooth_invalid = Glob->params.smooth_invalid_forecasts;

  double proj_area_dx_dt, proj_area_dy_dt;
  double sum_proj_area_dx_dt, sum_proj_area_dy_dt;
  double sum_weights;
  double sum_history;
  double weight;
  double distance;

  track_status_t *this_track, *other_track;

  /*
   * loop through the storms, computing the smoothed stats
   */
  
  for (i = 0; i < nstorms; i++) {
    
    this_track = storms[i].track;

    if (!smooth_invalid &&
	(this_track->history_in_secs < min_history)) {
      copy_props(this_track);
      continue;
    }

    if (Glob->params.debug >= DEBUG_EXTRA) {
      fprintf(stderr,
	      "SPATIAL_SMOOTHING - START : i, dx_dt, dy_dt: "
	      "%d, %g, %g\n",
	      i,
	      this_track->dval_dt.proj_area_centroid_x,
	      this_track->dval_dt.proj_area_centroid_y);
      fprintf(stderr, "----> Complex track %d, simple %d\n",
	      this_track->complex_track_num,
	      this_track->simple_track_num);

    }

    sum_proj_area_dx_dt = 0.0;
    sum_proj_area_dy_dt = 0.0;
    sum_history = 0.0;
    sum_weights = 0.0;

    for (j = 0; j < nstorms; j++) {

      distance = distance_array[i][j];
      
      other_track = storms[j].track;

      if (distance >= 0) {

	other_track = storms[j].track;
	
	if (Glob->params.debug >= DEBUG_EXTRA) {
	  fprintf(stderr, "-------> Complex track %d, Simple %d, "
		  "distance %g\n",
		  other_track->complex_track_num,
		  other_track->simple_track_num, distance);
	}
	  
	proj_area_dx_dt = other_track->dval_dt.proj_area_centroid_x;
	proj_area_dy_dt = other_track->dval_dt.proj_area_centroid_y;

	/*
	 * compute weight as inverse distance function
	 */

	weight = 1.0 - distance / smoothing_radius;
	
	sum_proj_area_dx_dt += proj_area_dx_dt * weight;
	sum_proj_area_dy_dt += proj_area_dy_dt * weight;
	sum_history += (double) other_track->history_in_secs * weight;
	sum_weights += weight;

      } /* if (distance_array >= 0) */

    } /* j */

    /*
     * set the storm props to the weighted means, in units per hr
     */

    if (sum_weights > 0.0) {

      this_track->dval_dt.smoothed_proj_area_centroid_x =
	sum_proj_area_dx_dt / sum_weights;
      
      this_track->dval_dt.smoothed_proj_area_centroid_y =
	sum_proj_area_dy_dt / sum_weights;
      
      this_track->smoothed_history_in_secs = sum_history / sum_weights;

    } else {

      copy_props(this_track);

    }

    if (Glob->params.debug >= DEBUG_EXTRA) {
      fprintf(stderr,
	      "SPATIAL_SMOOTHING - END: dx_dt, dy_dt, hist: "
	      "%g, %g, %g\n",
	      this_track->dval_dt.smoothed_proj_area_centroid_x,
	      this_track->dval_dt.smoothed_proj_area_centroid_y,
	      this_track->smoothed_history_in_secs);
    }

  } /* i */

  /*
   * set history to smoothed value
   */
  
  for (i = 0; i < nstorms; i++) {
    this_track = storms[i].track;
    this_track->history_in_secs =
      (si32) (this_track->smoothed_history_in_secs + 0.5);
  } /* i */
  
}







