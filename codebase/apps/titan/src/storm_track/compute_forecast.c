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
 * compute_forecast.c
 *
 * routines for computing the forecast for a given track entry
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "storm_track.h"

#define KM_PER_DEG_AT_EQUATOR 111.12

static int get_monotonicity(track_status_t *track,
			    si32 item);

#ifdef NOTNOW
static double get_percent_vol_above(storm_file_params_t *sparams,
				    storm_file_dbz_hist_t *hist,
				    si32 n_dbz_intervals,
				    double low_dbz_threshold,
				    double dbz_hist_interval,
				    double refl);
#endif

static double get_refl_max_ht(storm_file_params_t *sparams,
			      storm_file_scan_header_t *scan,
			      storm_file_global_props_t *gprops,
			      storm_file_layer_props_t *layer,
			      si32 n_layers,
			      double min_z,
			      double delta_z,
			      double refl);

static void get_trend(track_status_t *track,
		      si32 item,
		      double *weight,
		      double forecast_scale,
		      int zero_growth,
		      int zero_decay);

static double get_vol_percentile(storm_file_params_t *sparams,
				 storm_file_dbz_hist_t *hist,
				 si32 n_dbz_intervals,
				 double low_dbz_threshold,
				 double dbz_hist_interval,
				 double percentile);

static void regression_forecast(storm_file_handle_t *s_handle,
				si32 storm_num,
				storm_status_t *storm,
				double forecast_scale);

static void trend_forecast(storm_status_t *storm,
			   int three_d_analysis,
			   double forecast_scale);

/*
 * main routine
 */

void compute_forecast(storm_file_handle_t *s_handle,
		      storm_status_t *storms,
		      si32 storm_num)

{

  int three_d_analysis;
  int vol_trend_monotonic;
  si32 vol_item;
  double forecast_scale;
  storm_status_t *storm;
  track_status_t *track;

  storm = storms + storm_num;
  track = storm->track;

  /*
   * no forecast if there is no track history
   */

  if (track->history_in_scans == 1) {
    return;
  }

  /*
   * scale forecasts by history?
   *
   * If not, scale by 0.1 if only 2 scans available so far
   */
  
  if (Glob->params.scale_forecasts_by_history) {
    forecast_scale =
      ((double) track->history_in_secs /
       (double) Glob->params.history_for_scaling);
    forecast_scale = MIN(forecast_scale, 1.0);
  } else {
    if (track->history_in_scans > 2) {
      forecast_scale = 1.0;
    } else {
      forecast_scale = 0.1;
    }
  }
  
  /*
   * decide if we have two_d data or not
   */

  if (s_handle->scan->grid.nz == 1)
    three_d_analysis = FALSE;
  else
    three_d_analysis = TRUE;

  /*
   * compute the forecast for this track entry
   */

  if (Glob->params.forecast_type == TREND ||
      Glob->params.forecast_type == PARABOLIC) {
    
    trend_forecast(storm, three_d_analysis, forecast_scale);
    
  } else if (Glob->params.forecast_type == REGRESSION) {

    /*
     * decide whether the volume trend is monotonic
     */
    
    vol_item =
      (((si32) &track->history->volume -
	(si32) track->history) / sizeof(double));
    
    vol_trend_monotonic = get_monotonicity (track, vol_item);
    
    if (vol_trend_monotonic) {
      trend_forecast(storm, three_d_analysis, forecast_scale);
    } else {
      regression_forecast(s_handle, storm_num, storm, forecast_scale);
    }

  } /* if (Glob->params.forecast_type ... */

}

/***************************************************************************
 * get_monotonicity()
 *
 * Determines whether the trend was monotonic or not.
 *
 * Returns TRUE of the trend was monotonic for the past
 * 4 scans, FALSE otherwise.
 *
 ****************************************************************************/

static int get_monotonicity(track_status_t *track, si32 item)

{

  int nscans_monotonic = 4;
  int monotonic, increasing;
  si32 i;
  si32 nhist;
  double y, prev_y;

  if (track->history_in_scans > Glob->params.forecast_weights.len)
    nhist = Glob->params.forecast_weights.len;
  else
    nhist = track->history_in_scans;
  
  if (nhist < 2) {
    
    monotonic = FALSE;
    
  } else {

    monotonic = TRUE;
    
    for (i = 0; i < nscans_monotonic - 1; i++) {
      
      y = *((double *) (track->history + i) + item);
      prev_y = *((double *) (track->history + i + 1) + item);
      
      if (i == 0) {

	if ((y - prev_y) >= 0.0)
	  increasing = TRUE;
	else
	  increasing = FALSE;

      } else {
	
	if ((y - prev_y) >= 0.0) {
	  if (!increasing) {
	    monotonic = FALSE;
	    break;
	  }
	} else {
	  if (increasing) {
	    monotonic = FALSE;
	    break;
	  }
	} /* if ((y - prev_y) >= 0.0) */

      } /* if (i == 0) */
	
    } /* i */
    
  } /* if (track->history_in_scans ... */
  
  return (monotonic);

}

#ifdef NOTNOW

/********************************************************************
 * get_percent_vol_above
 *
 * Gets the percentage of the volume above a given reflectivity
 * value
 */

static double get_percent_vol_above(storm_file_params_t *sparams,
				    storm_file_dbz_hist_t *hist,
				    si32 n_dbz_intervals,
				    double low_dbz_threshold,
				    double dbz_hist_interval,
				    double refl)

{

  si32erval;

  double high_dbz_threshold;
  double lower_refl, upper_refl;
  double percent_in_interval;
  double percent_below_interval;
  double percent_below_refl;
  double percent_above_refl;
  double ratio;
  storm_float_dbz_hist_t fl_hist;

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

    RfDecodeStormHist(sparams, hist, &fl_hist);
    percent_in_interval = fl_hist.percent_volume;

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

#endif

/*********************************************************************
 * get_refl_max_ht()
 *
 * Gets max ht for a given reflectivity
 */

static double get_refl_max_ht(storm_file_params_t *sparams,
			      storm_file_scan_header_t *scan,
			      storm_file_global_props_t *gprops,
			      storm_file_layer_props_t *layer,
			      si32 n_layers,
			      double min_z,
			      double delta_z,
			      double refl)

{

  si32 ilayer;
  double refl_max_ht;

  refl_max_ht = 0.0;

  for (ilayer = 0; ilayer < n_layers; ilayer++, layer++) {

    if (layer->dbz_max > refl)
      refl_max_ht = min_z + (double) ilayer * delta_z;

  } /* ilayer */

  return (refl_max_ht);

}

/***************************************************************************
 * get_trend()
 *
 * compute the forecast for a given item
 *
 ****************************************************************************/

static void get_trend(track_status_t *track,
		      si32 item,
		      double *weight,
		      double forecast_scale,
		      int zero_growth,
		      int zero_decay)

{

  si32 i;
  si32 nhist;

  double x, y;
  double dval_dt;
  double sum_x = 0.0, sum_y = 0.0;
  double sum_xx = 0.0, sum_xy = 0.0;
  double sum_weights = 0.0;
  double start_utime;
  double num, denom;

  if (track->history_in_scans == 1) {

    /*
     * if start of track, set rates of change to zero
     */

    *((double *) &track->dval_dt + item) = 0.0;

  } else {

    if (track->history_in_scans > Glob->params.forecast_weights.len)
      nhist = Glob->params.forecast_weights.len;
    else
      nhist = track->history_in_scans;
    
    start_utime = (double) track->history[nhist - 1].time.unix_time;

    for (i = 0; i < nhist; i++) {

      y = *((double *) (track->history + i) + item);
      x = (double) track->history[i].time.unix_time - start_utime;

      sum_x += x * weight[i];
      sum_y += y * weight[i];
      sum_xx += x * x * weight[i];
      sum_xy += x * y * weight[i];
      sum_weights += weight[i];

    } /* i */

    /*
     * rate of change units per hour
     */

    num = sum_weights * sum_xy - sum_x * sum_y;
    denom = sum_weights * sum_xx - sum_x * sum_x;

    if (denom != 0.0) {
      dval_dt = (num / denom) * 3600.0;
    } else {
      dval_dt = 0.0;
    }

    if (zero_growth) {
      if (dval_dt > 0.0) {
	dval_dt = 0.0;
      }
    }

    if (zero_decay) {
      if (dval_dt < 0.0) {
	dval_dt = 0.0;
      }
    }

    /*
     * store the forecast values in the track data struct and the
     * track entry struct
     */

    *((double *) &track->dval_dt + item) = dval_dt * forecast_scale;

  } /* if (track->duration_in_scans == 1) */

}

/********************************************************************
 * get_vol_percentile
 *
 * Gets the given percentile of the volume reflectivity distribution
 */

static double get_vol_percentile(storm_file_params_t *sparams,
				 storm_file_dbz_hist_t *hist,
				 si32 n_dbz_intervals,
				 double low_dbz_threshold,
				 double dbz_hist_interval,
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

/*********************************************************************
 * regression_forecast()
 *
 * compute forecast for a given track entry based on regression between
 * storm props and future trends
 *
 *********************************************************************/

static void regression_forecast(storm_file_handle_t *s_handle,
				si32 storm_num,
				storm_status_t *storm,
				double forecast_scale)

{

  si32 i, item;
  si32 n_dbz_intervals;
  si32 n_layers;

  double nf;
  double normalized_dvol_dt;
  double normalized_darea_dt;
  double min_z, delta_z;
  double max_ht_dbz;
  double low_dbz_threshold;
  double dbz_hist_interval;
  double vol_percentile;  
  double top;
  double f1, f2, f3;

  storm_file_global_props_t *gprops;
  storm_file_params_t *sparams = &s_handle->header->params;
  storm_file_scan_header_t *scan = s_handle->scan;
  track_status_t *track = storm->track;

  /*
   * compute the forecast for each item
   */

  item = 0;

  for (i = 0; i < N_MOVEMENT_PROPS_FORECAST; i++) {

    get_trend(track, item,
	      Glob->params.forecast_weights.val,
	      forecast_scale, FALSE, FALSE);
    item++;

  } /* i */

  for (i = 0; i < N_GROWTH_PROPS_FORECAST; i++) {
    
    get_trend(track, item,
	      Glob->params.forecast_weights.val,
	      forecast_scale,
	      Glob->params.zero_growth,
	      Glob->params.zero_decay);
    item++;
    
  } /* i */

  /*
   * read in the storm props
   */
  
  if (RfReadStormProps(s_handle, storm_num, "regression_forecast"))
    tidy_and_exit(-1);
  
  /*
   * compute the volume forecast based on storm props
   */
  
  min_z = scan->min_z;
  delta_z = scan->delta_z;

  gprops = s_handle->gprops + storm_num;
  n_layers = gprops->n_layers;
  top = gprops->top;
  
  max_ht_dbz = get_refl_max_ht(sparams, s_handle->scan, gprops,
			       s_handle->layer, n_layers,
			       min_z, delta_z,
			       45.0);
  
  n_dbz_intervals = gprops->n_dbz_intervals;
  low_dbz_threshold = sparams->low_dbz_threshold;
  dbz_hist_interval = sparams->dbz_hist_interval;
  
  vol_percentile = get_vol_percentile(sparams,
				      s_handle->hist, 
				      n_dbz_intervals,
				      low_dbz_threshold,
				      dbz_hist_interval,
				      98.0);
  
  nf = 0.0;
  f1 = 0.0;
  f2 = 0.0;
  
  if (top >= 5.0) {
    f1 = -0.441 + 3.88 * exp(-0.370 * top);
    nf++;
  } else {
    f1 = 0.0;
  }

  if (max_ht_dbz >= 5.0) {
    f2 = -0.772 + 0.036 * max_ht_dbz;
    nf++;
  } else {
    f2 = 0.0;
  }

  f3 = -0.571 + 430.0 * exp(-0.168 * vol_percentile);
  nf++;
      
  normalized_dvol_dt = ((f1 + f2 + f3) / nf);
      
  track->dval_dt.volume =
    normalized_dvol_dt * storm->current.volume * forecast_scale;  
  
  /*
   * Recompute the forecast for the mean, precip and projected
   * area to be in proportion to the forecast for volume,taken
   * to the 2/3 power to account for the change from 3D to 2D.
   * Also, adjust mass by normalized dvol_dt.
   */
  
  if (normalized_dvol_dt >= 0.0)
    normalized_darea_dt = 
      pow((track->dval_dt.volume / storm->current.volume), 0.6667);
  else
    normalized_darea_dt = 
      -pow(fabs(track->dval_dt.volume / storm->current.volume), 0.6667);
  
  track->dval_dt.mass = 
    storm->current.mass * normalized_dvol_dt * forecast_scale;
  
  track->dval_dt.proj_area = 
    storm->current.proj_area * normalized_darea_dt * forecast_scale;
  
}

/*********************************************************************
 * trend_forecast()
 *
 * compute forecast for a given track entry based on trends only
 *
 *********************************************************************/

static void trend_forecast(storm_status_t *storm,
			   int three_d_analysis,
			   double forecast_scale)

{

  si32 i, item;
  double normalized_dvol_dt;
  double normalized_darea_dt;

  /*
   * compute the forecast for each item
   */

  item = 0;

  for (i = 0; i < N_MOVEMENT_PROPS_FORECAST; i++) {

    get_trend(storm->track, item,
	      Glob->params.forecast_weights.val,
	      forecast_scale, FALSE, FALSE);
    item++;

  } /* i */

  for (i = 0; i < N_GROWTH_PROPS_FORECAST; i++) {

    get_trend(storm->track, item,
	      Glob->params.forecast_weights.val,
	      forecast_scale,
	      Glob->params.zero_growth,
	      Glob->params.zero_decay);
    item++;
    
  } /* i */

  /*
   * Recompute the forecast for the mean area and projected
   * area to be in proportion to the forecast for volume,taken
   * to the 2/3 power to account for the change from 3D to 2D.
   * Also, adjust mass by normalized dvol_dt.
   */

  if (three_d_analysis) {

    normalized_dvol_dt = 
      storm->track->dval_dt.volume / storm->current.volume;

    if (normalized_dvol_dt >= 0.0) {
      normalized_darea_dt = 
	pow((storm->track->dval_dt.volume / storm->current.volume), 0.6667);
    } else {
      normalized_darea_dt = 
	(-pow(fabs(storm->track->dval_dt.volume /
		   storm->current.volume), 0.6667));
    }
  
    storm->track->dval_dt.mass = 
      storm->current.mass * normalized_dvol_dt * forecast_scale;
    
    storm->track->dval_dt.proj_area = 
      storm->current.proj_area * normalized_darea_dt * forecast_scale;
   
  } /* if (three_d_analysis) */
  
}
