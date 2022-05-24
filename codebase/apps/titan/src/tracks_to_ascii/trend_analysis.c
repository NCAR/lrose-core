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
 * trend_analysis.c
 *
 * Opens the files, reads in the headers
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * October 1991
 *
 **************************************************************************/

#include "tracks_to_ascii.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MISSING_TREND -9999.0

typedef struct {

  date_time_t time;

  int pre_trend_volume_monotonic;

  double vol_centroid_z;
  double refl_centroid_z;
  double dbz_max;
  double top;
  double ht_of_dbz_max;
  double max_ht_dbz;
  double vol_percentile;
  double percent_vol_above_dbz;
  double volume;

  double pre_trend_vol_centroid_z;
  double pre_trend_refl_centroid_z;
  double pre_trend_dbz_max;
  double pre_trend_ht_of_dbz_max;
  double pre_trend_max_ht_dbz;
  double pre_trend_vol_percentile;
  double pre_trend_percent_vol_above_dbz;

  double pre_trend_volume;
  double norm_pre_trend_volume;

  double post_trend_volume;
  double norm_post_trend_volume;

  double norm_forecast_trend_volume;

} track_props_t;
  
static double get_refl_max_ht(storm_file_scan_header_t *scan,
			      storm_file_global_props_t *gprops,
			      storm_file_layer_props_t *layer,
			      si32 n_layers,
			      double min_z,
			      double delta_z,
			      storm_file_params_t *sparams,
			      double refl);

static double get_vol_percentile(storm_file_dbz_hist_t *hist,
				 si32 n_dbz_intervals,
				 double low_dbz_threshold,
				 double dbz_hist_interval,
				 storm_file_params_t *sparams,
				 double percentile);

static double get_percent_vol_above(storm_file_dbz_hist_t *hist,
				    si32 n_dbz_intervals,
				    double low_dbz_threshold,
				    double dbz_hist_interval,
				    storm_file_params_t *sparams,
				    double refl);

static int compute_post_trend(si32 scan_num,
			      si32 nscans,
			      date_time_t *time0,
			      double *variable0,
			      double *trend0,
			      si32 nscans_trend,
			      si32 min_nscans_trend,
			      si32 min_nscans_monotonic);

static int compute_pre_trend(si32 scan_num,
			     si32 nscans,
			     date_time_t *time0,
			     double *variable0,
			     double *trend0,
			     si32 nscans_trend,
			     si32 min_nscans_trend,
			     si32 min_nscans_monotonic);

static void compute_trend(si32 start_scan,
			  si32 end_scan,
			  si32 scan_num,
			  date_time_t *time0,
			  double *variable0, double *trend0);

void trend_analysis(storm_file_handle_t *s_handle,
		    track_file_handle_t *t_handle)

{

  static int first_call = TRUE;
  static track_props_t *tprops, *tp;

  char format[256];

  si32 isimple, ientry;
  si32 iscan, istorm;
  si32 nentries;
  si32 simple_track_num;
  si32 nscans;
  si32 start_scan;
  si32 n_layers;
  si32 n_dbz_intervals;

  double nf;
  double min_z, delta_z;
  double low_dbz_threshold;
  double dbz_hist_interval;
  double vol_centroid_z;
  double refl_centroid_z;
  double dbz_max;
  double top;
  double ht_of_dbz_max;
  double max_ht_dbz;
  double vol_percentile;
  double percent_vol_above_dbz;
  double volume;
  double f1, f2, f3;

  complex_track_params_t *complex_params;
  storm_file_params_t *sparams;
  storm_file_global_props_t *gprops;
  track_file_entry_t *entry;

  complex_params = t_handle->complex_params;
  sparams = &s_handle->header->params;

  nscans = complex_params->duration_in_scans;
  start_scan = complex_params->start_scan;

  /*
   * allocate memory for track props
   */

  if (first_call) {
    
    tprops = (track_props_t *) umalloc
      ((ui32) (nscans * sizeof(track_props_t)));

    first_call = FALSE;

  } else {

    tprops = (track_props_t *) urealloc
      ((char *) tprops,
       (ui32) (nscans * sizeof(track_props_t)));

  } /* if (first_call) */

  tp = tprops;

  for (iscan = 0; iscan < nscans; iscan++) {

    tp->vol_centroid_z = 0.0;
    tp->refl_centroid_z = 0.0;
    tp->dbz_max = -LARGE_DOUBLE;
    tp->top = -LARGE_DOUBLE;
    tp->ht_of_dbz_max = -LARGE_DOUBLE;
    tp->max_ht_dbz = -LARGE_DOUBLE;
    tp->vol_percentile = 0.0;
    tp->percent_vol_above_dbz = 0.0;
    tp->volume = 0.0;

    tp->pre_trend_vol_centroid_z = MISSING_TREND;
    tp->pre_trend_refl_centroid_z = MISSING_TREND;
    tp->pre_trend_dbz_max = MISSING_TREND;
    tp->pre_trend_ht_of_dbz_max = MISSING_TREND;
    tp->pre_trend_max_ht_dbz = MISSING_TREND;
    tp->pre_trend_vol_percentile = MISSING_TREND;
    tp->pre_trend_percent_vol_above_dbz = MISSING_TREND;
    tp->pre_trend_volume = MISSING_TREND;
    tp->norm_pre_trend_volume = MISSING_TREND;
    tp->post_trend_volume = MISSING_TREND;
    tp->norm_post_trend_volume = MISSING_TREND;
    tp->norm_forecast_trend_volume = MISSING_TREND;

    tp++;

  } /* iscan */

  /*
   * read in simple tracks
   */
  
  for (isimple = 0;
       isimple < t_handle->complex_params->n_simple_tracks; isimple++) {
    
    simple_track_num =
      t_handle->simples_per_complex[complex_params->complex_track_num][isimple];
	  
    /*
     * read in simple track params and prepare entries for reading
     */
	  
    if(RfRewindSimpleTrack(t_handle, simple_track_num,
			   "trend_analysis") != R_SUCCESS)
      tidy_and_exit(-1);
    
    nentries = t_handle->simple_params->duration_in_scans;
    
    /*
     * loop through the entries
     */
	  
    for (ientry = 0; ientry < nentries; ientry++) {
	    
      /*
       * read in track entry
       */
	    
      if (RfReadTrackEntry(t_handle, "trend_analysis") != R_SUCCESS)
	tidy_and_exit(-1);

      entry = t_handle->entry;

      iscan = entry->scan_num - start_scan;
      istorm = entry->storm_num;

      tp = tprops + iscan;

      if (RfReadStormScan(s_handle, entry->scan_num,
			  "trend_analysis") != R_SUCCESS)
	tidy_and_exit(-1);

      tp->time.unix_time = s_handle->scan->time;
      uconvert_from_utime(&tp->time);

      min_z = s_handle->scan->min_z;
      delta_z = s_handle->scan->delta_z;
      
      if (RfReadStormProps(s_handle, istorm,
			   "trend_analysis") != R_SUCCESS)
	tidy_and_exit(-1);

      gprops = s_handle->gprops + istorm;
      n_layers = gprops->n_layers;

      vol_centroid_z = gprops->vol_centroid_z;
      refl_centroid_z = gprops->refl_centroid_z;
      dbz_max = gprops->dbz_max;
      top = gprops->top;
      ht_of_dbz_max = gprops->ht_of_dbz_max;
      volume = gprops->volume;

      max_ht_dbz = get_refl_max_ht(s_handle->scan,
				   gprops, s_handle->layer, n_layers,
				   min_z, delta_z, sparams,
				   Glob->dbz_for_max_ht);

      n_dbz_intervals = gprops->n_dbz_intervals;
      low_dbz_threshold = sparams->low_dbz_threshold;
      dbz_hist_interval = sparams->dbz_hist_interval;

      vol_percentile = get_vol_percentile(s_handle->hist, 
					  n_dbz_intervals,
					  low_dbz_threshold,
					  dbz_hist_interval,
					  sparams,
					  Glob->vol_percentile);
					     
      percent_vol_above_dbz =
	get_percent_vol_above(s_handle->hist, 
			      n_dbz_intervals,
			      low_dbz_threshold,
			      dbz_hist_interval,
			      sparams,
			      Glob->dbz_for_percent_vol_above);

      tp->vol_centroid_z += (vol_centroid_z * volume);
      tp->refl_centroid_z += (refl_centroid_z * volume);
      tp->dbz_max = MAX(tp->dbz_max, dbz_max);
      tp->top = MAX(tp->top, top);
      tp->ht_of_dbz_max = MAX(tp->ht_of_dbz_max, ht_of_dbz_max);
      tp->max_ht_dbz = MAX(tp->max_ht_dbz, max_ht_dbz);
      tp->vol_percentile += (vol_percentile * volume);
      tp->percent_vol_above_dbz += (percent_vol_above_dbz * volume);
      tp->volume += volume;
      
    } /* ientry */
	  
  } /* isimple */

  /*
   * compute the volume-weighted stats
   */
  
  tp = tprops;
  
  for (iscan = 0; iscan < nscans; iscan++) {

    tp->vol_centroid_z /= tp->volume;
    tp->refl_centroid_z /= tp->volume;
    tp->vol_percentile /= tp->volume;
    tp->percent_vol_above_dbz /= tp->volume;

    tp++;

  } /* iscan */

  /*
   * compute the pre-trends
   */

  tp = tprops;
  
  for (iscan = 0; iscan < nscans; iscan++) {
    
    compute_pre_trend(iscan,
		      nscans,
		      &tprops->time,
		      &tprops->vol_centroid_z,
		      &tprops->pre_trend_vol_centroid_z,
		      Glob->nscans_pre_trend,
		      Glob->min_nscans_pre_trend,
		      Glob->min_nscans_monotonic);
    
    compute_pre_trend(iscan,
		      nscans,
		      &tprops->time,
		      &tprops->refl_centroid_z,
		      &tprops->pre_trend_refl_centroid_z,
		      Glob->nscans_pre_trend,
		      Glob->min_nscans_pre_trend,
		      Glob->min_nscans_monotonic);
    
    compute_pre_trend(iscan,
		      nscans,
		      &tprops->time,
		      &tprops->dbz_max,
		      &tprops->pre_trend_dbz_max,
		      Glob->nscans_pre_trend,
		      Glob->min_nscans_pre_trend,
		      Glob->min_nscans_monotonic);
    
    compute_pre_trend(iscan,
		      nscans,
		      &tprops->time,
		      &tprops->ht_of_dbz_max,
		      &tprops->pre_trend_ht_of_dbz_max,
		      Glob->nscans_pre_trend,
		      Glob->min_nscans_pre_trend,
		      Glob->min_nscans_monotonic);
    
    compute_pre_trend(iscan,
		      nscans,
		      &tprops->time,
		      &tprops->max_ht_dbz,
		      &tprops->pre_trend_max_ht_dbz,
		      Glob->nscans_pre_trend,
		      Glob->min_nscans_pre_trend,
		      Glob->min_nscans_monotonic);
    
    compute_pre_trend(iscan,
		      nscans,
		      &tprops->time,
		      &tprops->vol_percentile,
		      &tprops->pre_trend_vol_percentile,
		      Glob->nscans_pre_trend,
		      Glob->min_nscans_pre_trend,
		      Glob->min_nscans_monotonic);
    
    compute_pre_trend(iscan,
		      nscans,
		      &tprops->time,
		      &tprops->percent_vol_above_dbz,
		      &tprops->pre_trend_percent_vol_above_dbz,
		      Glob->nscans_pre_trend,
		      Glob->min_nscans_pre_trend,
		      Glob->min_nscans_monotonic);
    
    tp->pre_trend_volume_monotonic =
      compute_pre_trend(iscan,
			nscans,
			&tprops->time,
			&tprops->volume,
			&tprops->pre_trend_volume,
			Glob->nscans_pre_trend,
			Glob->min_nscans_pre_trend,
			Glob->min_nscans_monotonic);
    
    tp++;
    
  } /* iscan */
  
  /*
   * compute the post-trends
   */

  for (iscan = 0; iscan < nscans; iscan++) {

    compute_post_trend(iscan,
		       nscans,
		       &tprops->time,
		       &tprops->volume,
		       &tprops->post_trend_volume,
		       Glob->nscans_post_trend,
		       Glob->min_nscans_post_trend,
		       Glob->min_nscans_monotonic);

  } /* iscan */

  /*
   * compute the forecast
   */

  tp = tprops;

  for (iscan = 0; iscan < nscans; iscan++) {

    if (tp->pre_trend_volume != MISSING_TREND)
      tp->norm_pre_trend_volume = tp->pre_trend_volume / tp->volume;

    if (tp->post_trend_volume != MISSING_TREND)
      tp->norm_post_trend_volume = tp->post_trend_volume / tp->volume;

    /*
     * if monotonic, use the trend. Otherwise, use the regression
     */
    
    if (tp->pre_trend_volume_monotonic) {

      tp->norm_forecast_trend_volume = tp->norm_pre_trend_volume;

    } else {
      
      nf = 0.0;
      f1 = 0.0;
      f2 = 0.0;

      if (tp->top >= 5.0) {
	f1 = -0.441 + 3.88 * exp(-0.370 * tp->top);
	nf++;
      } else {
	f1 = 0.0;
      }

      if (tp->max_ht_dbz >= 5.0) {
	f2 = -0.772 + 0.036 * tp->max_ht_dbz;
	nf++;
      } else {
	f2 = 0.0;
      }

      f3 = -0.571 + 430.0 * exp(-0.168 * tp->vol_percentile);
      nf++;
      
      if (nf > 0.0)
	tp->norm_forecast_trend_volume = ((f1 + f2 + f3) / nf);
      else
	tp->norm_forecast_trend_volume = tp->norm_pre_trend_volume;
      
    } /* if (tp->pre_trend_volume_monotonic) */

    tp++;
    
  } /* iscan */

  /*
   * print out the results
   */

  sprintf(format, "%s %s %s %s %s\n",
	  "%ld %ld %ld",
	  "%ld %ld %ld %ld %ld %ld",
	  "%g %g %g %g %g %g %g %g %g",
	  "%g %g %g %g %g %g %g",
	  "%g %g %g %g %g");

  tp = tprops;

  for (iscan = 0; iscan < nscans; iscan++) {

    printf(format,

	   complex_params->complex_track_num,
	   nscans,
	   iscan,

	   tp->time.year,
	   tp->time.month,
	   tp->time.day,
	   tp->time.hour,
	   tp->time.min,
	   tp->time.sec,

	   tp->vol_centroid_z,
	   tp->refl_centroid_z,
	   tp->dbz_max,
	   tp->top,
	   tp->ht_of_dbz_max,
	   tp->max_ht_dbz,
	   tp->vol_percentile,
	   tp->percent_vol_above_dbz,
	   tp->volume,

	   tp->pre_trend_vol_centroid_z,
	   tp->pre_trend_refl_centroid_z,
	   tp->pre_trend_dbz_max,
	   tp->pre_trend_ht_of_dbz_max,
	   tp->pre_trend_max_ht_dbz,
	   tp->pre_trend_vol_percentile,
	   tp->pre_trend_percent_vol_above_dbz,

	   tp->pre_trend_volume,
	   tp->norm_pre_trend_volume,
	   tp->post_trend_volume,
	   tp->norm_post_trend_volume,
	   tp->norm_forecast_trend_volume);

    tp++;

  } /* iscan */

}

/*********************************************************************
 * compute_pre_trend()
 *
 * computes the pre-trend for a variable
 *
 * Returns TRUE if monotonic, FALSE otherwise.
 *
 * Monotonic check is made backwards from current scan.
 */

/*ARGSUSED*/

static int compute_pre_trend(si32 scan_num,
			     si32 nscans,
			     date_time_t *time0,
			     double *variable0,
			     double *trend0,
			     si32 nscans_trend,
			     si32 min_nscans_trend,
			     si32 min_nscans_monotonic)

{

  int monotonic, increasing;

  si32 iscan;
  si32 start_scan;
  si32 end_scan;
  
  double *variable;
  double y, next_y;

  end_scan = scan_num;
  start_scan = end_scan - nscans_trend;
  
  if (start_scan < 0)
    start_scan = 0;

  if ((end_scan - start_scan) < min_nscans_trend)
    return (FALSE);
    
  /*
   * check for monotonocity
   */
  
  if ((end_scan - start_scan) < min_nscans_monotonic)
    return (FALSE);

  monotonic = TRUE;
  
  if (min_nscans_monotonic > 1) {

    variable = (double *)
      ((char *) variable0 + end_scan * sizeof(track_props_t));
    next_y = *variable;

    variable = (double *)
      ((char *) variable - sizeof(track_props_t));
    y = *variable;

    if (next_y - y >= 0.0)
      increasing = TRUE;
    else
      increasing = FALSE;
      
    for (iscan = end_scan - 1;
	 iscan > end_scan - min_nscans_monotonic;
	 iscan--) {
	
      variable = (double *)
	((char *) variable - sizeof(track_props_t));

      next_y = y;
      y = *variable;
	
      if (next_y - y >= 0.0) {
	if (!increasing) {
	  monotonic = FALSE;
	  break;
	}
      } else {
	if (increasing) {
	  monotonic = FALSE;
	  break;
	}
      } /* if (next_y - y >= 0.0) */
      
    } /* iscan */
      
  } /* if (min_nscans_monotonic > 1) */
  
  compute_trend(start_scan,
		end_scan,
		scan_num,
		time0,
		variable0,
		trend0);

  return (monotonic);
  
}

/*********************************************************************
 * compute_post_trend()
 *
 * computes the post-trend for a variable
 *
 * Returns TRUE if monotonic, FALSE otherwise.
 *
 * Monotonic check is made forwards from current scan.
 */

static int compute_post_trend(si32 scan_num,
			      si32 nscans,
			      date_time_t *time0,
			      double *variable0,
			      double *trend0,
			      si32 nscans_trend,
			      si32 min_nscans_trend,
			      si32 min_nscans_monotonic)

{

  int monotonic, increasing;

  si32 iscan;
  si32 start_scan;
  si32 end_scan;
  
  double *variable;
  double y, next_y;

  start_scan = scan_num;
  end_scan = start_scan + nscans_trend;

  if (end_scan > nscans - 1)
    end_scan = nscans - 1;
  
  if ((end_scan - start_scan) < min_nscans_trend)
    return (FALSE);
    
  /*
   * check for monotonocity
   */
  
  if ((end_scan - start_scan) < min_nscans_monotonic)
    return (FALSE);

  monotonic = TRUE;
  
  if (min_nscans_monotonic > 1) {

    variable = (double *)
      ((char *) variable0 + start_scan * sizeof(track_props_t));
    y = *variable;
    
    variable = (double *)
      ((char *) variable + sizeof(track_props_t));
    next_y = *variable;
    
    if (next_y - y >= 0.0)
      increasing = TRUE;
    else
      increasing = FALSE;
      
    for (iscan = start_scan + 1;
	 iscan < start_scan + min_nscans_monotonic;
	 iscan++) {
      
      variable = (double *)
	((char *) variable - sizeof(track_props_t));
      
      y = next_y;
      next_y = *variable;
	
      if (next_y - y >= 0.0) {
	if (!increasing) {
	  monotonic = FALSE;
	  break;
	}
      } else {
	if (increasing) {
	  monotonic = FALSE;
	  break;
	}
      } /* if (next_y - y >= 0.0) */
      
    } /* iscan */
    
  } /* if (min_nscans_monotonic > 1) */
  
  compute_trend(start_scan,
		end_scan,
		scan_num,
		time0,
		variable0,
		trend0);

  return (monotonic);
  
}

/*********************************************************************
 * compute_trend()
 *
 * computes the trend for a variable
 *
 */

static void compute_trend(si32 start_scan,
			  si32 end_scan,
			  si32 scan_num,
			  date_time_t *time0,
			  double *variable0,
			  double *trend0)

{

  si32 iscan;
  
  double *variable, *trend;

  date_time_t *ttime;

  double x, y;
  double n = 0.0;
  double sum_x = 0.0, sum_y = 0.0;
  double sum_xx = 0.0, sum_xy = 0.0;
  double start_utime;

  start_utime = (double) time0->unix_time;

  ttime = (date_time_t *)
    ((char *) time0 + start_scan * sizeof(track_props_t));
  
  variable = (double *)
    ((char *) variable0 + start_scan * sizeof(track_props_t));
  
  for (iscan = start_scan; iscan <= end_scan; iscan++) {
    
    y = *variable;
    x = (double) ttime->unix_time - start_utime;

    n += 1.0;
    sum_x += x;
    sum_y += y;
    sum_xx += x * x;
    sum_xy += x * y;

    ttime = (date_time_t *)
      ((char *) ttime + sizeof(track_props_t));

    variable = (double *)
      ((char *) variable + sizeof(track_props_t));

    
  } /* i */

  /*
   * rate of change units per hour
   */
  
  trend = (double *)
    ((char *) trend0 + scan_num * sizeof(track_props_t));
  
  *trend = ((n * sum_xy - sum_x * sum_y) /
	    (n * sum_xx - sum_x * sum_x)) * 3600.0;

  return;

}


/********************************************************************
 * get_percent_vol_above
 *
 * Gets the percentage of the volume above a given reflectivity
 * value
 */

static double get_percent_vol_above(storm_file_dbz_hist_t *hist,
				    si32 n_dbz_intervals,
				    double low_dbz_threshold,
				    double dbz_hist_interval,
				    storm_file_params_t *sparams,
				    double refl)

{

  si32 interval;
  double high_dbz_threshold;
  double lower_refl, upper_refl;
  double percent_in_interval;
  double percent_below_interval;
  double percent_below_refl;
  double percent_above_refl;
  double ratio;

  if (refl <= low_dbz_threshold)
    return (100.0);

  high_dbz_threshold =
    low_dbz_threshold + n_dbz_intervals * dbz_hist_interval;

  if (refl >= high_dbz_threshold)
    return (0.0);

  lower_refl = low_dbz_threshold;
  upper_refl = lower_refl + dbz_hist_interval;

  percent_below_interval = 0.0;

  for (interval = 0; interval < n_dbz_intervals; interval++, hist++) {

    percent_in_interval = hist->percent_volume;

    if (refl <= upper_refl) {

      ratio = (refl - lower_refl) / dbz_hist_interval;

      percent_below_refl =
	percent_below_interval + ratio * percent_in_interval;

      percent_above_refl = 100.0 - percent_below_refl;

      return (percent_above_refl);

    } 

    percent_below_interval += percent_in_interval;
    lower_refl += dbz_hist_interval;
    upper_refl += dbz_hist_interval;

  } /* interval */

  return (0.0);

}


/*********************************************************************
 * get_refl_max_ht()
 *
 * Gets max ht for a given reflectivity
 */

static double get_refl_max_ht(storm_file_scan_header_t *scan,
			      storm_file_global_props_t *gprops,
			      storm_file_layer_props_t *layer,
			      si32 n_layers,
			      double min_z,
			      double delta_z,
			      storm_file_params_t *sparams,
			      double refl)

{

  si32 ilayer;
  double layer_dbz_max;
  double refl_max_ht;
  
  refl_max_ht = 0.0;

  for (ilayer = 0; ilayer < n_layers; ilayer++, layer++) {

    layer_dbz_max = layer->dbz_max;
    
    if (layer_dbz_max > refl)
      refl_max_ht = min_z + (double) ilayer * delta_z;

  } /* ilayer */

  return (refl_max_ht);

}

/********************************************************************
 * get_vol_percentile
 *
 * Gets the given percentile of the volume reflectivity distribution
 */

static double get_vol_percentile(storm_file_dbz_hist_t *hist,
				 si32 n_dbz_intervals,
				 double low_dbz_threshold,
				 double dbz_hist_interval,
				 storm_file_params_t *sparams,
				 double percentile)

{

  si32 interval;
  double lower_refl;
  double percent_so_far;
  double percent_below;
  double percent_in_interval;
  double ratio;
  double refl;

  lower_refl = low_dbz_threshold;
  percent_so_far = 0.0;
  percent_below = 0.0;

  for (interval = 0; interval < n_dbz_intervals; interval++, hist++) {

    percent_in_interval = hist->percent_volume;
    percent_so_far += percent_in_interval;

    if (percent_so_far >= percentile) {

      ratio = (percentile - percent_below) / percent_in_interval;
      refl = lower_refl + ratio * dbz_hist_interval;

      return (refl);

    } 

    percent_below = percent_so_far;

    lower_refl += dbz_hist_interval;

  } /* interval */

  return (100.0);

}

