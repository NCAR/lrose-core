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
 * match_storms.c: match storms from time1 to time2
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "storm_track.h"
#include <toolsa/pjg.h>

#define MAX_ELT 100000
#define KM_PER_DEG_AT_EQUATOR 111.12

static int match_feasible(storm_status_t *storm1,
			  storm_status_t *storm2,
			  double d_hours,
			  int grid_type);

void match_storms(storm_file_handle_t *s_handle,
		  si32 nstorms1, si32 nstorms2,
		  storm_status_t *storms1,
		  storm_status_t *storms2,
		  double d_hours)

{
  
  int transpose;
  int grid_type;

  long i,j, k, l;
  long nvalid_edges = 0;
  long **icost;
  long *match;
  long dim1, dim2;
  long max_storms;
  
  double *xx1, *yy1, *xx2, *yy2;
  double distance, dx_km, dy_km;
  double x_km_scale, y_km_scale;
  double mean_lat, cos_lat;
  double delta_cube_root_volume;
  double speed;
  double cost_scale;
  double cost, max_cost;
  double **dcost;

  storm_file_params_t *sparams;

  /*
   * set local vars
   */

  sparams = &s_handle->header->params;
  grid_type = s_handle->scan->grid.proj_type;

  /*
   * set dimensions of cost array. Note that the matrix must
   * be transposed if nstorms1 exceeds nstorms2, because dim1 must be less
   * than or equal to dim2
   */

  if (nstorms1 <= nstorms2) {
    dim1 = nstorms1;
    dim2 = nstorms2;
    transpose = FALSE;
    max_storms = nstorms2;
  } else {
    dim1 = nstorms2;
    dim2 = nstorms1;
    transpose = TRUE;
    max_storms = nstorms1;
  }

  /*
   * allocate mem for cost arrays and match arrray
   */

  dcost = (double **) ucalloc2
    ((ui32) nstorms1, (ui32) nstorms2, (ui32) sizeof(double));

  icost = (long **) ucalloc2
    ((ui32) dim1, (ui32) dim2, (ui32) sizeof(long));

  match = (long *) ucalloc
    ((ui32) (nstorms1 + nstorms2), (ui32) sizeof(long));

  xx1 = (double *) umalloc
    ((ui32) (nstorms1 * sizeof(double)));

  yy1 = (double *) umalloc
    ((ui32) (nstorms1 * sizeof(double)));

  xx2 = (double *) umalloc
    ((ui32) (nstorms2 * sizeof(double)));

  yy2 = (double *) umalloc
    ((ui32) (nstorms2 * sizeof(double)));
  
  /*
   * load up storm coordinates
   */

  if (Glob->params.debug >= DEBUG_EXTRA) {
    fprintf(stderr, "Storms at time 1:\n");
  }

  for (i = 0; i < nstorms1; i++) {
    
    xx1[i] = storms1[i].current.proj_area_centroid_x;
    yy1[i] = storms1[i].current.proj_area_centroid_y;
    
    if (Glob->params.debug >= DEBUG_EXTRA) {
      fprintf(stderr,
	      "i, x1, y1 = %4ld, %10g, %10g\n", i, xx1[i], yy1[i]);
    }
    
  } /* i */

  if (Glob->params.debug >= DEBUG_EXTRA) {
    fprintf(stderr, "Storms at time 2:\n");
  }

  for (j = 0; j < nstorms2; j++) {
    
    xx2[j] = s_handle->gprops[j].proj_area_centroid_x;
    yy2[j] = s_handle->gprops[j].proj_area_centroid_y;
    
    if (Glob->params.debug >= DEBUG_EXTRA) {
      fprintf(stderr,
	      "j, x2, y2 = %4ld, %10g, %10g\n", j, xx2[j], yy2[j]);
    }
    
  } /* j */
  
  /*
   * load up the dcost array
   */

  max_cost = 0.0;

  for (i = 0; i < nstorms1; i++) {

    for (j = 0; j < nstorms2; j++) {

      if (storms1[i].n_match > 0 ||
	  storms2[j].n_match > 0) {

	/*
	 * already matched, so set edge invalid
	 */

	dcost[i][j] = -1.0;

	if (Glob->params.debug >= DEBUG_VERBOSE) {
	  fprintf(stderr, "Already matched using overlaps\n");
	  fprintf(stderr, "Storm i - %ld to storm j - %ld\n", i, j);
	  fprintf(stderr, "xx1[i], yy1[i]: (%g, %g)\n", xx1[i], yy1[i]);
	  fprintf(stderr, "xx2[j], yy2[j]: (%g, %g)\n", xx2[j], yy2[j]);
	  fprintf(stderr, "Already matched using overlaps\n");
	}

      } else {

	if (grid_type == TITAN_PROJ_LATLON) {

	  /*
	   * compute factors to convert delta lat/lon to km
	   */
	
	  mean_lat = (yy2[j] + yy1[i]) / 2.0;
	  cos_lat = cos(mean_lat * DEG_TO_RAD);
	  x_km_scale = KM_PER_DEG_AT_EQUATOR * cos_lat;
	  y_km_scale = KM_PER_DEG_AT_EQUATOR;
	
	} else {
	  
	  x_km_scale = 1.0;
	  y_km_scale = 1.0;
	  
	}
	
	dx_km = (xx2[j] - xx1[i]) * x_km_scale;
	dy_km = (yy2[j] - yy1[i]) * y_km_scale;
	
	distance = sqrt (dx_km * dx_km + dy_km * dy_km);
	speed = distance / d_hours;
	
	delta_cube_root_volume =
	  fabs(pow(s_handle->gprops[j].volume, 0.33333333) -
	       pow(storms1[i].current.volume, 0.33333333));
	
	if (speed <= Glob->params.max_tracking_speed &&
	    match_feasible(storms1 + i, storms2 + j,
			   d_hours, grid_type)) {
	  
	  /*
	   * edge is valid
	   */
	  
	  cost = 
	    (distance * Glob->params.weight_distance +
	     delta_cube_root_volume *
	     Glob->params.weight_delta_cube_root_volume);
	  
	  if (max_cost < cost)
	    max_cost = cost;
	  
	  dcost[i][j] = cost;
	  
	  nvalid_edges++;
	  
	} else {
	  
	  /*
	   * edge is not valid
	   */
	  
	  dcost[i][j] = -1.0;
	  
	} /* if (speed <= Glob->params.max_tracking_speed ... */

	if (Glob->params.debug >= DEBUG_VERBOSE) {
	  fprintf(stderr, "Storm i - %ld to storm j - %ld\n", i, j);
	  fprintf(stderr, "xx1[i], yy1[i]: (%g, %g)\n", xx1[i], yy1[i]);
	  fprintf(stderr, "xx2[j], yy2[j]: (%g, %g)\n", xx2[j], yy2[j]);
	  fprintf(stderr, "distance, d_hours, speed = %g, %g, %g\n",
		  distance, d_hours, speed);
	  if (dcost[i][j] < 0) {
	    fprintf(stderr, "Edge INVALID\n");
	  } else {
	    fprintf(stderr, "Edge valid\n");
	  }
	}

      } /* if (storms1[i].n_match > 0  ... */
	
    } /* j */

  } /* i */

  if (nvalid_edges > 0) {

    /*
     * compute cost scale factor
     */

    cost_scale = (double) MAX_ELT / (max_cost * (double) (max_storms + 1));

    if (Glob->params.debug >= DEBUG_EXTRA) {
      fprintf(stderr, "max_cost = %g\n", max_cost);
      fprintf(stderr, "cost_scale = %g\n", cost_scale);
    }

    /*
     * load up integer cost matrix, transposing as necessary to fulfil the
     * requirement that the first dimension must not exceed the second.
     * Also, the entries are subtracted from MAX_ELT so that the problem
     * may be treated as one of maximization rather than minimization.
     */

    for (i = 0; i < nstorms1; i++) {

      if (transpose == FALSE)
	k = i;
      else
	l = i;

      for (j = 0; j < nstorms2; j++) {

	if (transpose == FALSE)
	  l = j;
	else
	  k = j;

	cost = dcost[i][j];

	if (cost < 0) {
	  icost[k][l] = MAX_ELT / 2;
	} else {
	  icost[k][l] = MAX_ELT - (long) (cost * cost_scale + 0.5);
	}

      } /* j */
      
    } /* i */

    /*
     * print out icost matrix
     */
    
    if (Glob->params.debug >= DEBUG_VERBOSE) {
      
      fprintf(stderr, "\nICOST MATRIX\n\n");
      
      if (transpose)
	fprintf(stderr, "j\\i ");
      else
	fprintf(stderr, "i\\j ");
      
      for (j = 0; j < dim2; j++)
	fprintf(stderr, " %6ld", j);
      fprintf(stderr, "\n");
      
      for (i = 0; i < dim1; i++) {
	
	fprintf(stderr, "%4ld", i);
	
	for (j = 0; j < dim2; j++)
	  fprintf(stderr, " %6ld", icost[i][j]);
	fprintf(stderr, "\n");
	
      } /* i */
      
    } /* if (Glob->params.debug ... ) */
    
    /*
     * get the bipartite match
     */
    
    umax_wt_bip(icost, dim1, dim2, (long) MAX_ELT / 2, match);
    
    /*
     * load local match arrays. match1 contains the storm numbers at time2
     * which correspond to storms at time 1, and vice versa for match2.
     */

    if (transpose) {

      if (Glob->params.debug >= DEBUG_EXTRA)
	fprintf(stderr, "\nCost matrix transposed.\n\n");

      for (i = 0; i < nstorms2; i++) {

	if (match[i] >= 0) {

	  storms2[i].match = match[i];
	  storms1[match[i]].match = i;

	} /* if (match[i] >= 0) */

      } /* i */

    } else {

      for (i = 0; i < nstorms1; i++) {

	if (match[i] >= 0) {

	  storms1[i].match = match[i];
	  storms2[match[i]].match = i;

	} /* if (match[i] >= 0) */

      } /* i */

    } /* if (transpose) */

    /*
     * print out the match
     */

    if (Glob->params.debug >= DEBUG_VERBOSE) {
      
      fprintf(stderr, "Match array\n");
      
      for (i = 0; i < nstorms1 + nstorms2; i++)
	fprintf(stderr, "i = %ld, match = %ld\n", i, match[i]);
      
      fprintf(stderr, "\n");

      fprintf(stderr, "Matching 1 to 2\n");

      for (i = 0; i < nstorms1; i++)
	fprintf(stderr, "i = %ld, match1 = %ld\n", i,
		(long) storms1[i].match);
      
      fprintf(stderr, "\n");

      fprintf(stderr, "Matching 2 to 1:\n");
      
      for (j = 0; j < nstorms2; j++)
	fprintf(stderr, "j = %ld, match2 = %ld\n", j,
		(long) storms2[j].match);
      
      fprintf(stderr, "\n");

    } /* if (Glob->params.debug ... */

  } else {

    if (Glob->params.debug >= DEBUG_VERBOSE) {
      fprintf(stderr, "No valid tracks for match_storms()\n");
    }
    
  } /* if (nvalid_edges > 0) */

  /*
   * free up arrays
   */

  ufree2((void **) dcost);
  ufree2((void **) icost);
  ufree((char *) match);
  ufree((char *) xx1);
  ufree((char *) yy1);
  ufree((char *) xx2);
  ufree((char *) yy2);

}

/********************************************************************
 * match_feasible()
 *
 * This function tests for the feasibility of the match.
 *
 * (1) If the track is young (less than 5 scans old) the proposed
 *     match is always considered feasible because we do not have
 *     sufficient evidence to suggest otherwise.
 *
 * (2) If the track is more that 5 scans old, we look at the
 *     forecast position and the position of the proposed match.
 *     We consider a circle cenetered on the forecast location and
 *     with a radius of the forecast distance. If the proposed
 *     match lies within this circle we consider the match feasible.
 *     If not, it is not feasible.
 *
 ********************************************************************/

static int match_feasible(storm_status_t *storm1,
			  storm_status_t *storm2,
			  double d_hours,
			  int grid_type)

{

  double xx1, yy1;
  double xx2, yy2;
  double xxf, yyf;
  double dx, dy;
  double forecast_dist;
  double error_dist;
    

  /*
   * matches with young storms are always feasible
   */
  
  if (storm1->track->history_in_scans < 5) {
    return (TRUE);
  }

  xx1 = storm1->current.proj_area_centroid_x;
  yy1 = storm1->current.proj_area_centroid_y;
  xx2 = storm2->current.proj_area_centroid_x;
  yy2 = storm2->current.proj_area_centroid_y;

  /*
   * compute forecast position
   */

  xxf = xx1 + storm1->track->dval_dt.proj_area_centroid_x * d_hours;
  yyf = yy1 + storm1->track->dval_dt.proj_area_centroid_y * d_hours;

  /*
   * compute dist between current and forecast
   */

  if (grid_type == TITAN_PROJ_LATLON) {
    PJGLatLon2DxDy(yyf, xxf, yy1, xx1, &dx, &dy);
  } else {
    dx = xx1 - xxf;
    dy = yy1 - yyf;
  }
  forecast_dist = sqrt(dx * dx + dy * dy);

  /*
   * compute dist between proposed match and forecast
   */

  if (grid_type == TITAN_PROJ_LATLON) {
    PJGLatLon2DxDy(yyf, xxf, yy2, xx2, &dx, &dy);
  } else {
    dx = xx2 - xxf;
    dy = yy2 - yyf;
  }
  error_dist = sqrt(dx * dx + dy * dy);

  if (error_dist < forecast_dist) {
    return (TRUE);
  } else {
    if (Glob->params.debug >= DEBUG_EXTRA) {
      fprintf(stderr, "=========================>>>>\a\n");
      fprintf(stderr, "Track %d/%d, invalid match\n",
	      storm1->track->complex_track_num,
	      storm1->track->simple_track_num);
      fprintf(stderr, "Storm location (x,y) = (%g,%g)\n",
	      xx1, yy1);
      fprintf(stderr, "Forecast point (x,y) = (%g,%g)\n",
	      xxf, yyf);
      fprintf(stderr, "Proposed match at (x,y) = (%g,%g)\n",
	      xx2, yy2);
    }
    return (FALSE);
  }
  
  
}

