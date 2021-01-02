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
/*******************************************************************************
 * perform_verification.c
 *
 * Opens the files, reads in the headers
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * October 1991
 *
 *******************************************************************************/

#include "verify_tracks.h"

#define MAX_BRANCHES 50

void perform_verification(storm_file_handle_t *s_handle,
			  track_file_handle_t *t_handle,
			  date_time_t *scan_time,
			  vt_count_t *total_count,
			  vt_stats_t *total_stats)

{
  
  static ui08 **forecast_grid;
  static ui08 **truth_grid;

  static int first_call = TRUE;
  static long nbytes_grid;
  
  int verify_valid;
  
  long icomplex, isimple, istorm;
  long iscan, jscan;
  long nvalid, nverify;
  long nstorms;
  long n_scans;
  long forecast_scan = 0;
  long complex_track_num;
  long simple_track_num;
  long n_simple_tracks;
  long first_scan_searched, last_scan_searched;
  
  double time_diff, min_time_diff;
  double now, forecast_time;
  double forecast_lead_time, closest_lead_time;
  double verify_min_history;
  
  complex_track_params_t *ct_params;

  track_file_params_t *tparams;
  track_file_forecast_props_t props_current;
  track_file_forecast_props_t props_forecast;
  track_file_forecast_props_t props_verify;

  vt_stats_t complex_stats, file_stats;
  vt_simple_track_t *stracks, *strack;
  vt_storm_t *storm;
  vt_entry_index_t valid[MAX_BRANCHES];
  vt_count_t count;
  vt_count_t file_count;
  vt_count_t complex_count;

  n_scans = s_handle->header->n_scans;
  tparams = &t_handle->header->params;
  
  /*
   * allocate grids
   */
  
  if (first_call) {
    
    nbytes_grid = Glob->nx * Glob->ny;
    
    forecast_grid = (ui08 **) ucalloc2
      ((ui32) Glob->ny, (ui32) Glob->nx, (ui32) sizeof(ui08));
    
    truth_grid = (ui08 **) ucalloc2
      ((ui32) Glob->ny, (ui32) Glob->nx, (ui32) sizeof(ui08));
    
    first_call = FALSE;
    
  } /* if (first_call) */
  
  memset ((void *) &file_count,
          (int) 0, (size_t) sizeof(vt_count_t));
  memset ((void *) &file_stats,
          (int) 0, (size_t) sizeof(vt_stats_t));

  /*
   * Set the verify_min_history. If the 'verify_before_forecast_time'
   * flag is set, verify_min_history is set to 0. This allows the
   * inclusion of storms which are too young to have been forecast using
   * this system, and hence verifies all convective activity. If the
   * flag is not set, verify_min_history is set to the sum of the
   * forecast_lead_time and the min_valid_history, since this is
   * the minimum history for a forecast to be valid.
   */

  if (Glob->verify_before_forecast_time)
    verify_min_history = 0.0;
  else
    verify_min_history =
      Glob->forecast_lead_time + Glob->forecast_min_history;

  /*
   * loop through the complex tracks
   */
  
  for (icomplex = 0;
       icomplex < t_handle->header->n_complex_tracks; icomplex++) {
    
    /*
     * initialize
     */
    
    memset ((void *) &complex_count,
            (int) 0, (size_t) sizeof(vt_count_t));
    memset ((void *) &complex_stats,
            (int) 0, (size_t) sizeof(vt_stats_t));
 
    if (Glob->debug) {
       fprintf(stderr, "=========================================================\n");	
    }
   
    /*
     * read in the complex track params
     */

    complex_track_num = t_handle->complex_track_nums[icomplex];
    if(RfReadComplexTrackParams(t_handle, complex_track_num, TRUE,
				"perform_verification"))
      tidy_and_exit(-1);
    
    ct_params = t_handle->complex_params;
    
    /*
     * initialize flags and counters for track problems
     */
    
    ct_params->n_top_missing = 0;
    ct_params->n_range_limited = 0;
    ct_params->start_missing = FALSE;
    ct_params->end_missing = FALSE;
    ct_params->volume_at_start_of_sampling = 0;
    ct_params->volume_at_end_of_sampling = 0;

    /*
     * set flags if track existed at start or end of sampling
     */
    
    if (t_handle->complex_params->start_time == 
	s_handle->header->start_time) {
      ct_params->start_missing = TRUE;
    }
    
    if (t_handle->complex_params->end_time == 
	s_handle->header->end_time) {
      ct_params->end_missing = TRUE;
    }
    
    /*
     * allocate array for simple tracks
     */
    
    n_simple_tracks = t_handle->complex_params->n_simple_tracks;
    stracks = (vt_simple_track_t *) umalloc
      ((ui32) (n_simple_tracks * sizeof(vt_simple_track_t)));
    
    /*
     * read in simple tracks
     */
    
    for (isimple = 0; isimple < n_simple_tracks; isimple++) {
      
      strack = stracks + isimple;
      
      simple_track_num =
	t_handle->simples_per_complex[complex_track_num][isimple];
      
      /*
       * read in simple track params and prepare entries for reading
       */
      
      if(RfRewindSimpleTrack(t_handle, simple_track_num,
			     "perform_verification"))
	tidy_and_exit(-1);
      
      /*
       * find last descendant, insert relevant values in the
       * simple params struct and store
       */
      
      find_last_descendant(t_handle, 0);
      
      /*
       * copy simple params
       */
      
      memcpy ((void *) &strack->params,
              (void *) t_handle->simple_params,
              (size_t) sizeof(simple_track_params_t));
      
      nstorms = t_handle->simple_params->duration_in_scans;
      
      /*
       * allocate space for the entries
       */
      
      strack->storms = (vt_storm_t *) umalloc
	((ui32) (nstorms * sizeof(vt_storm_t)));
      
      for (istorm = 0; istorm < nstorms; istorm++) {
	
	storm = strack->storms + istorm;
	
	/*
	 * read in track entry, copy the structs to the local
	 * array elements
	 */
	
	if (RfReadTrackEntry(t_handle, "perform_verification"))
	  tidy_and_exit(-1);
	
	if (RfReadStormScan(s_handle, t_handle->entry->scan_num,
			    "perform_verification"))
	  tidy_and_exit(-1);
	
	if (RfReadStormProps(s_handle, t_handle->entry->storm_num,
			     "perform_verification"))
	  tidy_and_exit(-1);

	umalloc_verify();
	
	memcpy ((void *) &storm->entry,
		(void *) t_handle->entry,
		(size_t) sizeof(track_file_entry_t));
	
	memcpy ((void *) &storm->gprops,
		(void *) (s_handle->gprops + t_handle->entry->storm_num),
		(size_t) sizeof(storm_file_global_props_t));
	
	storm->runs = (storm_file_run_t *) umalloc
	  ((ui32) (storm->gprops.n_runs * sizeof(storm_file_run_t)));
	
	memcpy ((void *) storm->runs,
		(void *) s_handle->runs,
		(size_t) (storm->gprops.n_runs * sizeof(storm_file_run_t)));

	/*
	 * allocate the lookup tables which relate the scan cartesian
	 * grid to the verification grid
	 */

	storm->x_lookup = (long *) umalloc
	  ((ui32) s_handle->scan->grid.nx * sizeof(long));

	storm->y_lookup = (long *) umalloc
	  ((ui32) s_handle->scan->grid.ny * sizeof(long));

	/*
	 * compute the lookup tables
	 */

	compute_lookup(&s_handle->scan->grid,
		       storm->x_lookup, storm->y_lookup);

	/*
	 * update flags for storm status
	 */
	
	if (storm->gprops.top_missing)
	  ct_params->n_top_missing++;
	
	if (storm->gprops.range_limited)
	  ct_params->n_range_limited++;

	if (storm->entry.time == s_handle->header->start_time) {
	  ct_params->volume_at_start_of_sampling += storm->gprops.volume;
	}
	
	if (storm->entry.time == s_handle->header->end_time) {
	  ct_params->volume_at_end_of_sampling += storm->gprops.volume;
	}
	
      } /* istorm */
      
    } /* isimple */
    
    /*
     * Set the first_scan_searched variable. If the
     * verify_before_forecast_time flag is set, the search begins at
     * the first scan in the storm file, because we need to consider
     * forecasts made before the track starts.
     * If the flag is not set, the search starts at the
     * start of the complex track
     */
    
    if (Glob->verify_before_forecast_time)
      first_scan_searched = 0;
    else
      first_scan_searched = t_handle->complex_params->start_scan;

    /*
     * now loop through all of the scans in the complex track
     */
    
    for (iscan = first_scan_searched;
	 iscan <= t_handle->complex_params->end_scan; iscan++) {
      
      /*
       * initialize forecast and verification grids, etc
       */
      
      memset ((void *) *forecast_grid,
              (int) 0, (size_t) nbytes_grid);
      memset ((void *) *truth_grid,
              (int) 0, (size_t) nbytes_grid);
      memset ((void *) &props_current,
              (int) 0, (size_t) sizeof(track_file_forecast_props_t));
      memset ((void *) &props_forecast,
              (int) 0, (size_t) sizeof(track_file_forecast_props_t));
      memset ((void *) &props_verify,
              (int) 0, (size_t) sizeof(track_file_forecast_props_t));
      nvalid = 0;
      nverify = 0;
      
      /*
       * compute times
       */
      
      now = (double) scan_time[iscan].unix_time;
      forecast_time = now + (double) Glob->forecast_lead_time;
      
      /*
       * Set the last_scan_searched variable. If the verify_after_track_dies
       * flag is set, the search ends at the last scan in the
       * storm file, because we need to consider scans even after the
       * track has terminated. If the flag is not set, the search ends
       * at the end of the complex track
       */

      if (Glob->verify_after_track_dies)
	last_scan_searched = n_scans - 1;
      else
	last_scan_searched = t_handle->complex_params->end_scan;

      /*
       * find scan number which best corresponds to the forecast time
       */
      
      min_time_diff = LARGE_DOUBLE;
      
      for (jscan = iscan;
	   jscan <= last_scan_searched; jscan++) {
	
	time_diff =
	  fabs((double) scan_time[jscan].unix_time - forecast_time);
	
	if (time_diff < min_time_diff) {
	  
	  forecast_scan = jscan;
	  min_time_diff = time_diff;
	  
	} /* if (time_diff < min_time_diff) */
	
      } /* jscan */
      
      closest_lead_time =
	(double) scan_time[forecast_scan].unix_time - now;
      
      /*
       * check the forecast lead time to determine whether there is
       * track data close to that time - if not, set the verify_valid
       * flag to FALSE
       */
      
      if (fabs(closest_lead_time - Glob->forecast_lead_time) <=
	  Glob->forecast_lead_time_margin) {
	
	forecast_lead_time = closest_lead_time;
	verify_valid = TRUE;
	
      } else {
	
	forecast_lead_time = Glob->forecast_lead_time;
	verify_valid = FALSE;
	
      }
      
      if (Glob->debug) {
	
	fprintf(stderr, "=========================\n"); 
	fprintf(stderr, "forecast_lead_time : %g\n", forecast_lead_time);
	fprintf(stderr, "forecast_scan : %ld\n", forecast_scan);
	fprintf(stderr, "verify_valid : %d\n", verify_valid);
	
      }

      if (verify_valid) {
	
	/*
	 * search through all of the entries in the track
	 */
	
	for (isimple = 0; isimple < n_simple_tracks; isimple++) {
	  
	  strack = stracks + isimple;
	  
	  for (istorm = 0;
	       istorm < strack->params.duration_in_scans; istorm++) {
	    
	    storm = strack->storms + istorm;
	    
	    if (storm->entry.scan_num == iscan &&
		storm->entry.history_in_secs >=
		Glob->forecast_min_history) {
	      
	      /*
	       * this storm is at the current scan number, and its
	       * duration exceeds that for starting the forecast, so
	       * compute the forecast and update the forecast grid
	       */
	      
	      if (Glob->debug) {
		
		fprintf(stderr, "Computing Forecast\n");
		fprintf(stderr,
			"Simple num %ld, ientry %ld, scan %ld, time %s\n",
			(long) strack->params.simple_track_num, istorm,
			(long) storm->entry.scan_num,
			utimstr(storm->entry.time));
		
		
	      } /* if (Glob->debug) */
	      
	      /*
	       * load up the forecast props
	       */

	      if (load_props(s_handle, t_handle,
			     &storm->entry,
			     &storm->gprops,
			     forecast_lead_time,
			     &props_forecast)) {
		
		/*
		 * load up the current props
		 */

		load_props(s_handle, t_handle,
			   &storm->entry,
			   &storm->gprops,
			   0.0,
			   &props_current);
		
		/*
		 * there is a valid forecast, so proceed.
		 * The forecast is considered valid it the
		 * forecast volume exceeds the volume
		 * threshold
		 */
		
		load_forecast_grid(s_handle, t_handle,
				   &storm->entry,
				   &storm->gprops,
				   forecast_lead_time,
				   forecast_grid);
		
		valid[nvalid].isimple = isimple;
		valid[nvalid].istorm = istorm;
		nvalid++;
		
	      } /* if (load_props (...... */
	      
	    } /* if (storm->entry.scan_num .... */
	    
	    if ((storm->entry.scan_num == forecast_scan) &&
		storm->entry.history_in_secs >= verify_min_history) {
	      
	      /*
	       * this storm is at the forecast time, so
	       * update the truth grids
	       */
	      
	      if (Glob->debug) {
		
		fprintf(stderr, "Computing Verification\n");
		fprintf(stderr,
			"Simple num %ld, ientry %ld, scan %ld, time %s\n",
			(long) strack->params.simple_track_num, istorm,
			(long) storm->entry.scan_num,
			utimstr(storm->entry.time));
		
		
	      } /* if (Glob->debug) */
	      
	      /*
	       * load props for this scan - the lead time is set to zero
	       */
	      
	      load_props(s_handle, t_handle,
			 &storm->entry,
			 &storm->gprops,
			 0.0,
			 &props_verify);
	      
	      nverify++;
	      
	      load_truth_grid(s_handle, storm, truth_grid);
	      
	    } /* if (storm->entry.scan_num .... */
	    
	  } /* istorm */
	  
	} /* isimple */
	
	if ((nvalid > 0) ||
	    (nverify > 0 && Glob->verify_before_forecast_time)) {
	  
	  /*
	   * compute the contingency data, summing counters for the
	   * complex track params
	   */
	  
	  compute_contingency_data(forecast_grid,
				   truth_grid,
				   nbytes_grid,
				   &count);
	  
	  increment_count(&complex_count, &count);
	  
	  if (Glob->debug)
	    debug_print(t_handle, stracks, nvalid, valid,
			forecast_grid,
			truth_grid,
			&count);
	  
	  /*
	   * Compute the errors between the forecast and
	   * verification data
	   *
	   * The storm entries are updated to include the verification
	   * results in the track file - this is then rewritten.
	   */
	  
	  compute_errors(s_handle,
			 nverify,
			 &props_current,
			 &props_forecast,
			 &props_verify,
			 &complex_stats,
			 &file_stats,
			 total_stats);
	  
	} /* if (nvalid > 0 ... ) */
	
      } /* if (verify_valid) */
      
    } /* iscan */

    /*
     * rewrite the track entries
     */

    strack = stracks;

    for (isimple = 0; isimple < n_simple_tracks; isimple++) {
      
      storm = strack->storms;

      for (istorm = 0;
	   istorm < strack->params.duration_in_scans; istorm++) {
	
	memcpy ((void *) t_handle->entry,
		(void *) &storm->entry,
		(size_t) sizeof(track_file_entry_t));
	    
	if (RfRewriteTrackEntry(t_handle,
				"perform_verification"))
	  tidy_and_exit(-1);
	
	storm++;

      } /* istorm */

      strack++;
	
    } /* isimple */
      
    /*
     * update counts for the file
     */

    increment_count(&file_count, &complex_count);

    /*
     * set contingency data in complex params
     */

    set_counts_to_longs(&complex_count,
			&t_handle->complex_params->ellipse_verify);
    
    /*
     * compute the root mean squared error for the forecast data
     */

    compute_stats(&complex_stats);
    
    load_file_stats(tparams,
		    &complex_stats,
		    &t_handle->complex_params->n_samples_for_forecast_stats,
		    &t_handle->complex_params->forecast_bias,
		    &t_handle->complex_params->forecast_rmse);
    
    /*
     * rewrite complex track params
     */

    if(RfWriteComplexTrackParams(t_handle, t_handle->complex_track_nums[icomplex],
				 "perform_verification"))
      tidy_and_exit(-1);
    
    /*
     * free up structs
     */
    
    strack = stracks;

    for (isimple = 0; isimple < n_simple_tracks; isimple++) {
      
      storm = strack->storms;

      for (istorm = 0;
	   istorm < strack->params.duration_in_scans; istorm++) {
	ufree((char *) storm->runs);
	ufree((char *) storm->x_lookup);
	ufree((char *) storm->y_lookup);
	storm++;
      } /* istorm */
      
      ufree((char *) strack->storms);
      strack++;
      
    } /* isimple */
    
    ufree((char *) stracks);

  } /* icomplex */
  
  /*
   * update total counts
   */

  increment_count(total_count, &file_count);
  
  /*
   * set contingency data in file header
   */

  set_counts_to_longs(&file_count,
		      &t_handle->header->ellipse_verify);
    
  /*
   * compute the root mean squared error for the file forecast data,
   * store in header
   */

  compute_stats(&file_stats);
  
  load_file_stats(tparams,
		  &file_stats,
		  &t_handle->header->n_samples_for_forecast_stats,
		  &t_handle->header->forecast_bias,
		  &t_handle->header->forecast_rmse);
    
  /*
   * rewrite header
   */

  if(RfWriteTrackHeader(t_handle, "perform_verification"))
    tidy_and_exit(-1);

}
