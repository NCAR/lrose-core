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
 * update_tracks.c
 *
 * Sets up the entries and structures relevant to the storms at time 2.
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * October 1991
 *
 ***************************************************************************/

#include "storm_track.h"

/*
 * file scope prototypes
 */

static void get_correction_for_combination(storm_status_t *storm2,
					   storm_status_t *storms1,
					   Point_d *pos_corr,
					   double d_hours);

static void get_correction_for_merger(storm_status_t *storm2,
				      storm_status_t *storms1,
				      Point_d *pos_corr);

static void get_correction_for_split(storm_status_t *storm2,
				     storm_status_t *storms1,
				     storm_status_t *storms2,
				     Point_d *pos_corr);

static void handle_combined(storm_file_handle_t *s_handle,
			    track_file_handle_t *t_handle,
			    date_time_t *dtime,
			    si32 scan_num,
			    double d_hours,
			    track_utime_t *track_utime,
			    si32 nstorms1,
			    si32 nstorms2,
			    storm_status_t *storms1,
			    storm_status_t *storms2,
			    storm_status_t *storm2);

/*
 * principal routine
 */
     
void update_tracks(storm_file_handle_t *s_handle,
		   track_file_handle_t *t_handle,
		   date_time_t *dtime,
		   si32 scan_num,
		   double d_hours,
		   track_utime_t *track_utime,
		   si32 nstorms1,
		   si32 nstorms2,
		   storm_status_t *storms1,
		   storm_status_t *storms2,
		   si32 *track_continues)

{

  si32 jstorm;
  storm_status_t *storm2;

  /*
   * loop through all storms at time 2
   */

  storm2 = storms2;
  for (jstorm = 0; jstorm < nstorms2; jstorm++, storm2++) {

    if (storm2->continues) {

      /*
       * this is a continuing track - set the storms2 pointer to the
       * relevant entry in the storms1 array
       */
      
      storm2->track = storms1[storm2->match].track;
      track_continues[storm2->match] = TRUE;
      
    } else if (storm2->starts) {

      /*
       * initialize a new track
       */

      alloc_new_track(t_handle,
		      dtime,
		      track_utime,
		      storm2,
		      scan_num,
		      TRUE,
		      (si32) -1,
		      (si32) 0,
		      scan_num,
		      dtime);
      
    } else {

      /*
       * combined tracks
       */
      
      handle_combined(s_handle,
		      t_handle,
		      dtime,
		      scan_num,
		      d_hours,
		      track_utime,
		      nstorms1, nstorms2,
		      storms1, storms2,
		      storm2);

    } /* if (storm2->is_cont) */

  } /* jstorm */

}

static void handle_combined(storm_file_handle_t *s_handle,
			    track_file_handle_t *t_handle,
			    date_time_t *dtime,
			    si32 scan_num,
			    double d_hours,
			    track_utime_t *track_utime,
			    si32 nstorms1,
			    si32 nstorms2,
			    storm_status_t *storms1,
			    storm_status_t *storms2,
			    storm_status_t *storm2)
     
{

  si32 kstorm;
  si32 ihist;
  si32 nchildren;
  si32 history_in_scans, nhist;
  si32 oldest_combining_storm;
  si32 new_simple_track;
  si32 complex_track;
  si32 scan_origin;

  double storm1_wt, storm2_wt;
  double sum_storm2_wt;

  storm_track_props_t *current;
  Point_d *pos_corr, *corr;

  date_time_t *time_origin;
  storm_status_t *storm1;
  track_match_t *match;
  storm_track_props_t *combined_history;
  storm_track_props_t *contribution;

  /*
   * find complex track num and oldest scan in
   * contributing storms
   */
  
  history_in_scans = 0;
  
  match = storm2->match_array;
  for (kstorm = 0; kstorm < storm2->n_match; kstorm++, match++) {
    
    storm1 = storms1 + match->storm_num;
    
    if (history_in_scans < storm1->track->history_in_scans) {
      history_in_scans = storm1->track->history_in_scans;
      scan_origin = storm1->track->scan_origin;
      time_origin = &storm1->track->time_origin;
      oldest_combining_storm = match->storm_num;
    }

    /*
     * compute lowest track num
     */
    
    if (kstorm == 0) {
      complex_track = storm1->track->complex_track_num;
    } else {
      if (storm1->track->complex_track_num != complex_track) {
	fprintf(stderr, "\aERROR - %s:update_tracks\n", Glob->prog_name);
	fprintf(stderr, "Consolidate_complex_tracks failed.\n");
	fprintf(stderr,
		"Complex nums %ld and %ld should have been consolidated.\n",
		(long) complex_track,
		(long) storm1->track->complex_track_num);
      }
    }
    
  } /* kstorm */
  
  /*
   * read in complex track params for the lowest complex track number
   */
  
  if (RfReadComplexTrackParams(t_handle, complex_track, FALSE,
			       "update_tracks")) {
    tidy_and_exit(-1);
  }
  
  /*
   * allocate simple track
   */
  
  new_simple_track =
    alloc_new_track(t_handle,
		    dtime,
		    track_utime,
		    storm2,
		    scan_num,
		    FALSE,
		    complex_track,
		    history_in_scans,
		    scan_origin,
		    time_origin);
  
  storm2->track->history_in_scans = history_in_scans;
  storm2->track->history_in_secs =
    compute_history_in_secs(storm2->track, dtime,
			    history_in_scans);
  
  if (Glob->params.debug >= DEBUG_EXTRA) {
    fprintf(stderr, "\ncombined simple track number %ld\n",
	    (long) new_simple_track);
  }
  
  /*
   * amend the parents params to include this child
   */
  
  match = storm2->match_array;
  for (kstorm = 0; kstorm < storm2->n_match; kstorm++, match++) {
    
    storm1 = storms1 + match->storm_num;
    
    if (RfReadSimpleTrackParams
	(t_handle, storm1->track->simple_track_num, 
	 "update_tracks")) {
      tidy_and_exit(-1);
    }
    
    nchildren = t_handle->simple_params->nchildren + 1;
    t_handle->simple_params->nchildren = nchildren;
    t_handle->simple_params->child[nchildren - 1] = new_simple_track;
    
    if (RfWriteSimpleTrackParams
	(t_handle, storm1->track->simple_track_num, "update_tracks")) {
      tidy_and_exit(-1);
    }
    
  } /* kstorm */
  
  /*
   * amend the new simple track params to include its parents
   */
  
  if (RfReadSimpleTrackParams
      (t_handle, new_simple_track, "update_tracks")) {
    tidy_and_exit(-1);
  }
  
  t_handle->simple_params->nparents = storm2->n_match;
  
  match = storm2->match_array;
  for (kstorm = 0; kstorm < storm2->n_match; kstorm++, match++) {
    
    storm1 = storms1 + match->storm_num;
    
    t_handle->simple_params->parent[kstorm] =
      storm1->track->simple_track_num;
    
    if (Glob->params.debug >= DEBUG_EXTRA) {
      fprintf(stderr, "parent of new track %ld\n",
	      (long) storm1->track->simple_track_num);
    }
    
  } /* kstorm */
  
  if (RfWriteSimpleTrackParams
      (t_handle, new_simple_track, "update_tracks")) {
    tidy_and_exit(-1);
  }
  
  /*
   * set pointer to current value struct
   */
  
  current = &storm2->current;
  
  /*
   * foreach combining storm, compute the correction between the
   * forecast position and the combined centroid
   */

  pos_corr = (Point_d *) umalloc (storm2->n_match * sizeof(Point_d));
  
  if (storm2->has_split && !storm2->has_merger) {

    get_correction_for_split(storm2, storms1, storms2, pos_corr);

  } else if (!storm2->has_split && storm2->has_merger) {

    get_correction_for_merger(storm2, storms1, pos_corr);

  } else {

    get_correction_for_combination(storm2, storms1, pos_corr, d_hours);

  }

  /*
   * load up history data
   */
  
  if (history_in_scans > Glob->params.forecast_weights.len)
    nhist = Glob->params.forecast_weights.len;
  else
    nhist = history_in_scans;

  combined_history = storm2->track->history;
  for (ihist = 0; ihist < nhist; ihist++, combined_history++) {

    /*
     * get time from oldest storm entry
     */
    
    combined_history->time =
      storms1[oldest_combining_storm].track->history[ihist].time;
    
    match = storm2->match_array;
    corr = pos_corr;
    sum_storm2_wt = 0.0;
    for (kstorm = 0; kstorm < storm2->n_match;
	 kstorm++, match++, corr++) {
      
      storm1 = storms1 + match->storm_num;

      if (ihist < storm1->track->history_in_scans) {
	
	storm1_wt = match->overlap / storm1->sum_overlap;
	storm2_wt = match->overlap / storm2->sum_overlap;
	sum_storm2_wt += storm2_wt;
	
	contribution = storm1->track->history + ihist;
	
	/*
	 * Positions are computed after adding the correction
	 * between the current and forecast positions
	 *
	 * The sum is weighted by the area contribution
	 * of the time2 storms
	 */
	
	combined_history->proj_area_centroid_x +=
	  (storm2_wt *
	   (contribution->proj_area_centroid_x + corr->x));
	
	combined_history->proj_area_centroid_y +=
	  (storm2_wt *
	   (contribution->proj_area_centroid_y + corr->y));
	
	/*
	 * Heights are weighted sums
	 *
	 * The sum is weighted by the area contribution
	 * of the time2 storms
	 */
	
	combined_history->vol_centroid_z +=
	  (storm2_wt * contribution->vol_centroid_z);
	
	combined_history->vol_centroid_z +=
	  (storm2_wt * contribution->vol_centroid_z);
	
	combined_history->top +=
	  (storm2_wt * contribution->top);
	
	/*
	 * for dbz_max, take max
	 */
	
	combined_history->dbz_max =
	  MAX(combined_history->dbz_max, contribution->dbz_max);

	/*
	 * For size, the sum is weighted by the area
	 * contribution of the time1 storms.
	 */
	
	combined_history->volume += storm1_wt * contribution->volume;
	combined_history->precip_flux += storm1_wt * contribution->precip_flux;
	combined_history->mass += storm1_wt * contribution->mass;
	combined_history->proj_area += storm1_wt * contribution->proj_area;

      } /* if (ihist < storm1->track->history_in_scans) */
	
    } /* kstorm */
    
    /*
     * adjust movement props for the sum_storm2_wt, since
     * not all tracks have history at all times
     */
    
    combined_history->proj_area_centroid_x /= sum_storm2_wt;
    combined_history->proj_area_centroid_y /= sum_storm2_wt; 
    combined_history->vol_centroid_z /= sum_storm2_wt;
    combined_history->vol_centroid_z /= sum_storm2_wt;
    combined_history->top /= sum_storm2_wt;
	
  } /* ihist */
  
  /*
   * free up resources
   */
  
  ufree((char *) pos_corr);

  return;

}

/*********************************************************
 * get_correction_for_combination()
 *
 * merger and split - use forecast (x, y) to compute
 * correction
 */

static void get_correction_for_combination(storm_status_t *storm2,
					   storm_status_t *storms1,
					   Point_d *pos_corr,
					   double d_hours)

{

  int kstorm;
  double val, dval_dt;
  Point_d pos_forecast;
  Point_d *corr;
  storm_status_t *storm1;
  track_match_t *match;
  
  match = storm2->match_array;
  corr = pos_corr;
  
  for (kstorm = 0; kstorm < storm2->n_match;
       kstorm++, match++, corr++) {
    
    storm1 = storms1 + match->storm_num;
    
    val = storm1->track->history->proj_area_centroid_x;
    dval_dt = storm1->track->dval_dt.proj_area_centroid_x;
    pos_forecast.x = val + dval_dt * d_hours;
    corr->x = storm2->current.proj_area_centroid_x - pos_forecast.x;
    
    val = storm1->track->history->proj_area_centroid_y;
    dval_dt = storm1->track->dval_dt.proj_area_centroid_y;
    pos_forecast.y = val + dval_dt * d_hours;
    corr->y = storm2->current.proj_area_centroid_y - pos_forecast.y;
    
  }

  return;

}

/*********************************************************
 * get_correction_for_merger()
 *
 * merger only - use merge centroid for correction
 */

static void get_correction_for_merger(storm_status_t *storm2,
				      storm_status_t *storms1,
				      Point_d *pos_corr)

{

  int kstorm;

  double sum_x = 0.0;
  double sum_y = 0.0;
  double sum_area = 0.0;
  double proj_area;

  Point_d centroid;
  Point_d *corr;
  
  storm_status_t *storm1;
  track_match_t *match;

  /*
   * compute merge centroid
   */

  match = storm2->match_array;
  for (kstorm = 0; kstorm < storm2->n_match; kstorm++, match++) {
    storm1 = storms1 + match->storm_num;
    proj_area = storm1->current.proj_area;
    sum_area += proj_area;
    sum_x += storm1->current.proj_area_centroid_x * proj_area;
    sum_y += storm1->current.proj_area_centroid_y * proj_area;
  }
  
  centroid.x = sum_x / sum_area;
  centroid.y = sum_y / sum_area;

  /*
   * compute correction
   */

  match = storm2->match_array;
  corr = pos_corr;
  for (kstorm = 0; kstorm < storm2->n_match;
       kstorm++, match++, corr++) {
    storm1 = storms1 + match->storm_num;
    corr->x = centroid.x - storm1->current.proj_area_centroid_x;
    corr->y = centroid.y - storm1->current.proj_area_centroid_y;
  } /* kstorm */

  return;

}

/*********************************************************
 * get_correction_for_split()
 *
 * split only - use split centroid to compute correction
 */

static void get_correction_for_split(storm_status_t *storm2,
				     storm_status_t *storms1,
				     storm_status_t *storms2,
				     Point_d *pos_corr)

{

  int i;

  double sum_x = 0.0;
  double sum_y = 0.0;
  double sum_area = 0.0;
  double proj_area;

  Point_d centroid;
  storm_status_t *storm1, *split;
  track_match_t *match;
  
  storm1 = storms1 + storm2->match_array[0].storm_num;
  
  match = storm1->match_array;
  for (i = 0; i < storm1->n_match; i++, match++) {
    split = storms2 + match->storm_num;
    proj_area = split->current.proj_area;
    sum_area += proj_area;
    sum_x += split->current.proj_area_centroid_x * proj_area;
    sum_y += split->current.proj_area_centroid_y * proj_area;
  }
  
  centroid.x = sum_x / sum_area;
  centroid.y = sum_y / sum_area;
      
  pos_corr->x = storm2->current.proj_area_centroid_x - centroid.x;
  pos_corr->y = storm2->current.proj_area_centroid_y - centroid.y;

  return;

}

