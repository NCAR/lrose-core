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
// TrForecast.cc
//
// TrForecast class
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1999
//
///////////////////////////////////////////////////////////////

#include "TrForecast.hh"
#include <rapmath/math_macros.h>
#include <toolsa/pjg.h>
#include <toolsa/sincos.h>
#include <vector>
#include <algorithm>
#include <toolsa/pmu.h>
using namespace std;

#define KM_PER_DEG_AT_EQUATOR 111.12

//////////////
// constructor
//

TrForecast::TrForecast(const string &prog_name, const Params &params) :
  Worker(prog_name, params)

{
  _projIsLatLon = false;
}

/////////////
// destructor
//

TrForecast::~TrForecast()

{

}

/**************
 * compute()
 *
 * Returns 0 on success, -1 on failure
 */

int TrForecast::compute(TitanStormFile &sfile,
			TrStorm &storm, int storm_num)

{

  int three_d_analysis;
  int vol_trend_monotonic;
  double forecast_scale;
  TrTrack &track = storm.track;

  // set the area change ratio

  _set_area_change_ratio(track);

  /*
   * set forecast_valid flag
   */
  
  track.status.forecast_valid = true;
  if (track.status.history_in_secs <
      _params.tracking_min_history_for_valid_forecast) {
    track.status.forecast_valid = false;
  }
  
  /*
   * no forecast if there is no track history
   */
  
  if (track.status.history_in_scans == 1) {
    return (0);
  }

  /*
   * smooth relative speed changes?
   */

  if (_params.tracking_limit_rel_speed_change) {
    _limit_rel_speed_change(sfile, track);
  }

  /*
   * scale forecasts by history?
   *
   * If not, scale by 0.1 if only 2 scans available so far
   */
  
  if (_params.tracking_scale_forecasts_by_history) {
    forecast_scale = ((double) track.status.history_in_secs /
		      (double) _params.tracking_history_for_scaling);
    forecast_scale = MIN(forecast_scale, 1.0);
  } else {
    if (track.status.history_in_scans > 2) {
      forecast_scale = 1.0;
    } else {
      forecast_scale = 0.1;
    }
  }
  
  /*
   * decide if we have two_d data or not
   */

  if (sfile.scan().grid.nz == 1) {
    three_d_analysis = FALSE;
  } else {
    three_d_analysis = TRUE;
  }

  /*
   * compute the forecast for this track entry
   */

  if (_params.tracking_forecast_type == Params::TREND ||
      _params.tracking_forecast_type == Params::PARABOLIC) {
    
    _trend_forecast(storm, three_d_analysis, forecast_scale);
    
  } else if (_params.tracking_forecast_type == Params::REGRESSION) {

    /*
     * decide whether the volume trend is monotonic
     */
    
    vol_trend_monotonic = _get_vol_monotonicity (track);
    
    if (vol_trend_monotonic) {
      _trend_forecast(storm, three_d_analysis, forecast_scale);
    } else {
      if (_regression_forecast(sfile, storm_num, storm, forecast_scale)) {
	return (-1);
      }
    }

  } /* if (_params.tracking_forecast_type ... */

  return (0);

}

/////////////////////////////////////////
// compute speed and dirn for all tracks

void TrForecast::compute_speed_and_dirn(TitanStormFile &sfile,
					vector<TrStorm*> &storms)

{

  // set projection details

  if (sfile.scan().grid.proj_type == TITAN_PROJ_LATLON) {
    _projIsLatLon = true;
  } else {
    _projIsLatLon = false;
  }

  // initially copy the motion rates into the smoothed values
  
  for (size_t i = 0; i < storms.size(); i++) {
    _copy_props(storms[i]->track);
  }

  // compute unsmoothed speed and direction
  // using field tracker if requested
  
  _load_speed_and_dirn(sfile, storms, true);
  if (_useFieldTracker) {
    return;
  }
  
  // smooth the speed and direction forecasts if required
  // and if field tracker was not used
  
  if (_params.tracking_spatial_smoothing ||
      _params.tracking_smooth_invalid_forecasts ||
      _params.tracking_smooth_fast_growth_decay) {

    _smooth_spatial_forecasts(sfile, storms);

    // compute smoothed speed and direction
    
    _load_speed_and_dirn(sfile, storms, false);
    
  }

}

///////////////////////////////////////////
// csmooth spatial forecasts for all storms

void TrForecast::_smooth_spatial_forecasts(TitanStormFile &sfile,
					   vector<TrStorm*> &storms)

{
  
  /*
   * allocate the distance array. This is a 2-d array indicating
   * which storms are within the smoothing_radius of each other.
   * If storms are within the radius, the value is set to the 
   * distance between them in km. If not, the value is set to -1.
   */
  
  fl32 **distance_array = (fl32 **) umalloc2
    (storms.size(), storms.size(), sizeof(fl32));
  
  /*
   * load up the distance array. This is a 2-d array indicating
   * which storms are within the smoothing_radius of each other
   */
  
  _load_distance_array(sfile, storms, distance_array);
  
  /*
   * loop through the storms, computing the smoothed stats
   */
  
  for (size_t istorm = 0; istorm < storms.size(); istorm++) {
    
    TrTrack &this_track = storms[istorm]->track;

    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "SMOOTH_SPATIAL_FORECASTS" << endl;
      cerr << "  complex_track_num: "
	   << this_track.status.complex_track_num << endl;
      cerr << "  simple_track_num: "
	   << this_track.status.simple_track_num << endl;
      cerr << "  forecast_valid: "
	   << this_track.status.forecast_valid << endl;
    }

    // Smooth in case of fast growth/decay?
    
    if (_params.tracking_smooth_fast_growth_decay) {
      
      double growth_threshold = _params.tracking_smoothing_growth_threshold;
      double decay_threshold = _params.tracking_smoothing_decay_threshold;

      if (this_track.status.area_change_ratio >= growth_threshold ||
	  this_track.status.area_change_ratio <= decay_threshold) {

	if (_params.debug >= Params::DEBUG_EXTRA) {
	  cerr << "******************************************" << endl;
	  cerr << "  smooth fast growth/decay" << endl;
	  cerr << "  area_change_ratio: "
	       << this_track.status.area_change_ratio << endl;
	  cerr << "  growth_threshold: " << growth_threshold << endl;
	  cerr << "  decay_threshold: " << decay_threshold << endl;
	}
	
	_smooth_motion(storms, istorm, this_track,
		       distance_array, true, true, false);
	continue;
	
      }

    } // fast growth/decay?

    // smooth invalid forecasts?

    if (_params.tracking_smooth_invalid_forecasts &&
	!this_track.status.forecast_valid) {
      _smooth_motion(storms, istorm, this_track,
		     distance_array, true, false, false);
      continue;
    } // invalid forecast
    
    // check for smoothing of all valid storms

    if (_params.tracking_spatial_smoothing &&
	this_track.status.forecast_valid) {
      _smooth_motion(storms, istorm, this_track,
		     distance_array, false, false,
		     _params.tracking_smooth_erratic_only);
      continue;
    } // invalid forecast
    
    // no smoothing

    // copy_props(this_track);

  } /* istorm */
  
  /*
   * set history to smoothed value
   */
  
  for (size_t i = 0; i < storms.size(); i++) {
    TrTrack &this_track = storms[i]->track;
    this_track.status.history_in_secs =
      (si32) (this_track.status.smoothed_history_in_secs + 0.5);
  } // i
  
  /*
   * free up
   */

  ufree2((void **) distance_array);
  
}


/***************************************************************************
 * get_vol_monotonicity()
 *
 * Determines whether the volume trend is monotonic or not.
 *
 * Returns TRUE of the trend was monotonic for the past
 * 4 scans, FALSE otherwise.
 *
 ****************************************************************************/

int TrForecast::_get_vol_monotonicity(TrTrack &track)

{

  int nscans_monotonic = 4;
  int monotonic = false, increasing = false;
  int i;
  int nhist;
  double y, prev_y;

  if (track.status.history_in_scans > _params.tracking_forecast_weights_n)
    nhist = _params.tracking_forecast_weights_n;
  else
    nhist = track.status.history_in_scans;
  
  if (nhist < 2) {
    
    monotonic = FALSE;
    
  } else {

    monotonic = TRUE;
    
    for (i = 0; i < nscans_monotonic - 1; i++) {
      
      y = track.status.history[i].volume;
      prev_y = track.status.history[i+1].volume;

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
    
  } /* if (track.status.history_in_scans ... */
  
  return (monotonic);

}

/*********************************************************************
 * get_refl_max_ht()
 *
 * Gets max ht for a given reflectivity
 */

double TrForecast::_get_refl_max_ht(const storm_file_layer_props_t *layer,
				    int n_layers,
				    double min_z,
				    double delta_z,
				    double refl)

{

  int ilayer;
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
 * compute the trend forecast for a given item
 *
 ****************************************************************************/

void TrForecast::_get_trend(TrTrack &track,
                            double &val,
			    double forecast_scale,
			    int zero_growth,
			    int zero_decay)

{

  int nhist;

  double x, y;
  double dval_dt;
  double sum_x = 0.0, sum_y = 0.0;
  double sum_xx = 0.0, sum_xy = 0.0;
  double sum_weights = 0.0;
  double start_utime;
  double num, denom;

  // set weights

  const double *weights = _params._tracking_forecast_weights;
  
  // compute the item number in the props_t structure, by
  // computing the offset from the start of the struct
  
  int item = &val - &track.status.dval_dt.proj_area_centroid_x;

  if (track.status.history_in_scans == 1) {

    /*
     * if start of track, set rates of change to zero
     */

    *((double *) &track.status.dval_dt + item) = 0.0;

  } else {

    if (track.status.history_in_scans > _params.tracking_forecast_weights_n)
      nhist = _params.tracking_forecast_weights_n;
    else
      nhist = track.status.history_in_scans;
    
    start_utime = (double) track.status.history[nhist - 1].time.unix_time;

    for (int i = 0; i < nhist; i++) {

      y = *((double *) (track.status.history + i) + item);
      x = (double) track.status.history[i].time.unix_time - start_utime;

      sum_x += x * weights[i];
      sum_y += y * weights[i];
      sum_xx += x * x * weights[i];
      sum_xy += x * y * weights[i];
      sum_weights += weights[i];

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

    *((double *) &track.status.dval_dt + item) = dval_dt * forecast_scale;

  } /* if (track.status.duration_in_scans == 1) */

}

/***************************************************************************
 * set_area_change_ratio()
 *
 *    ratio = (a2 - a1) / a1
 *
 ****************************************************************************/

void TrForecast::_set_area_change_ratio(TrTrack &track)

{

  double ratio = 0.0;

  if (track.status.history_in_scans > 1) {
    
    double a2 = track.status.history[0].proj_area;
    double a1 = track.status.history[1].proj_area;
    if (a1 != 0.0) {
      ratio = (a2 - a1) / a1;
    }
    
  }

  track.status.area_change_ratio = ratio;

}

/********************************************************************
 * get_vol_percentile
 *
 * Gets the given percentile of the volume reflectivity distribution
 */

double TrForecast::_get_vol_percentile(const storm_file_dbz_hist_t *hist,
				       int n_dbz_intervals,
				       double low_dbz_threshold,
				       double dbz_hist_interval,
				       double percentile)

{

  int interval;

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

int TrForecast::_regression_forecast(TitanStormFile &sfile,
				     int storm_num,
				     TrStorm &storm,
				     double forecast_scale)
  
{

  int n_dbz_intervals;
  int n_layers;

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

  const storm_file_params_t &sparams = sfile.header().params;
  const storm_file_scan_header_t &scan = sfile.scan();
  TrTrack &track = storm.track;

  /*
   * compute the forecast for each item
   */

  _get_trend(track, track.status.dval_dt.proj_area_centroid_x,
             forecast_scale, FALSE, FALSE);
  _get_trend(track, track.status.dval_dt.proj_area_centroid_y,
             forecast_scale, FALSE, FALSE);
  _get_trend(track, track.status.dval_dt.vol_centroid_z,
             forecast_scale, FALSE, FALSE);
  _get_trend(track, track.status.dval_dt.refl_centroid_z,
             forecast_scale, FALSE, FALSE);
  _get_trend(track, track.status.dval_dt.top,
             forecast_scale, FALSE, FALSE);

  _get_trend(track, track.status.dval_dt.dbz_max, forecast_scale,
             _params.tracking_zero_growth,
             _params.tracking_zero_decay);
  _get_trend(track, track.status.dval_dt.dbz_mean, forecast_scale,
             _params.tracking_zero_growth,
             _params.tracking_zero_decay);
  _get_trend(track, track.status.dval_dt.volume, forecast_scale,
             _params.tracking_zero_growth,
             _params.tracking_zero_decay);
  _get_trend(track, track.status.dval_dt.precip_flux, forecast_scale,
             _params.tracking_zero_growth,
             _params.tracking_zero_decay);
  _get_trend(track, track.status.dval_dt.mass, forecast_scale,
             _params.tracking_zero_growth,
             _params.tracking_zero_decay);
  _get_trend(track, track.status.dval_dt.proj_area, forecast_scale,
             _params.tracking_zero_growth,
             _params.tracking_zero_decay);

  /*
   * read in the storm props
   */

  if (sfile.ReadProps(storm_num)) {
    cerr << "ERROR - " << _progName
	 << "TrForecast::regression_forecast" << endl;
    cerr << sfile.getErrStr() << endl;
    return -1;
  }

  /*
   * compute the volume forecast based on storm props
   */
  
  min_z = scan.min_z;
  delta_z = scan.delta_z;

  const storm_file_global_props_t *gprops =
    sfile.gprops() + storm_num;
  n_layers = gprops->n_layers;
  top = gprops->top;
  
  max_ht_dbz = _get_refl_max_ht(sfile.lprops(), n_layers,
				min_z, delta_z,
				45.0);
  
  n_dbz_intervals = gprops->n_dbz_intervals;
  low_dbz_threshold = sparams.low_dbz_threshold;
  dbz_hist_interval = sparams.dbz_hist_interval;
  
  vol_percentile = _get_vol_percentile(sfile.hist(), 
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
      
  track.status.dval_dt.volume =
    normalized_dvol_dt * storm.current.volume * forecast_scale;  
  
  /*
   * Recompute the forecast for the mean, precip and projected
   * area to be in proportion to the forecast for volume,taken
   * to the 2/3 power to account for the change from 3D to 2D.
   * Also, adjust mass by normalized dvol_dt.
   */
  
  if (normalized_dvol_dt >= 0.0)
    normalized_darea_dt = 
      pow((track.status.dval_dt.volume / storm.current.volume), 0.6667);
  else
    normalized_darea_dt = 
      -pow(fabs(track.status.dval_dt.volume / storm.current.volume), 0.6667);
  
  track.status.dval_dt.mass = 
    storm.current.mass * normalized_dvol_dt * forecast_scale;
  
  track.status.dval_dt.proj_area = 
    storm.current.proj_area * normalized_darea_dt * forecast_scale;

  return (0);
  
}

/*********************************************************************
 * trend_forecast()
 *
 * compute forecast for a given track entry based on trends only
 *
 *********************************************************************/

void TrForecast::_trend_forecast(TrStorm &storm,
				 int three_d_analysis,
				 double forecast_scale)
  
{

  double normalized_dvol_dt;
  double normalized_darea_dt;

  /*
   * compute the forecast for each item
   */

  TrTrack &track = storm.track;

  _get_trend(track, track.status.dval_dt.proj_area_centroid_x,
             forecast_scale, FALSE, FALSE);
  _get_trend(track, track.status.dval_dt.proj_area_centroid_y,
             forecast_scale, FALSE, FALSE);
  _get_trend(track, track.status.dval_dt.vol_centroid_z,
             forecast_scale, FALSE, FALSE);
  _get_trend(track, track.status.dval_dt.refl_centroid_z,
             forecast_scale, FALSE, FALSE);
  _get_trend(track, track.status.dval_dt.top,
             forecast_scale, FALSE, FALSE);

  _get_trend(track, track.status.dval_dt.dbz_max, forecast_scale,
             _params.tracking_zero_growth,
             _params.tracking_zero_decay);
  _get_trend(track, track.status.dval_dt.dbz_mean, forecast_scale,
             _params.tracking_zero_growth,
             _params.tracking_zero_decay);
  _get_trend(track, track.status.dval_dt.volume, forecast_scale,
             _params.tracking_zero_growth,
             _params.tracking_zero_decay);
  _get_trend(track, track.status.dval_dt.precip_flux, forecast_scale,
             _params.tracking_zero_growth,
             _params.tracking_zero_decay);
  _get_trend(track, track.status.dval_dt.mass, forecast_scale,
             _params.tracking_zero_growth,
             _params.tracking_zero_decay);
  _get_trend(track, track.status.dval_dt.proj_area, forecast_scale,
             _params.tracking_zero_growth,
             _params.tracking_zero_decay);

  /*
   * Recompute the forecast for the mean area and projected
   * area to be in proportion to the forecast for volume,taken
   * to the 2/3 power to account for the change from 3D to 2D.
   * Also, adjust mass by normalized dvol_dt.
   */

  if (three_d_analysis) {

    normalized_dvol_dt = 
      storm.track.status.dval_dt.volume / storm.current.volume;

    if (normalized_dvol_dt >= 0.0) {
      normalized_darea_dt = 
	pow((storm.track.status.dval_dt.volume / storm.current.volume), 0.6667);
    } else {
      normalized_darea_dt = 
	(-pow(fabs(storm.track.status.dval_dt.volume /
		   storm.current.volume), 0.6667));
    }
  
    storm.track.status.dval_dt.mass = 
      storm.current.mass * normalized_dvol_dt * forecast_scale;
    
    storm.track.status.dval_dt.proj_area = 
      storm.current.proj_area * normalized_darea_dt * forecast_scale;
   
  } /* if (three_d_analysis) */
  
}

/*********************************************************************
 * copy_props()
 *
 * Just copy props across, no smoothing
 */

void TrForecast::_copy_props(TrTrack &track)

{

  /*
   * copy storm vals to smoothed stats
   */
  
  track.status.dval_dt.smoothed_proj_area_centroid_x =
    track.status.dval_dt.proj_area_centroid_x;
  
  track.status.dval_dt.smoothed_proj_area_centroid_y =
    track.status.dval_dt.proj_area_centroid_y;
  
  track.status.smoothed_history_in_secs =
    track.status.history_in_secs;
  
}

/*********************************************************************
 * get_extreme_coords()
 *
 * get the extreme coord values for the current storms
 */

void TrForecast::_get_extreme_coords(vector<TrStorm*> &storms,
				     xypair_t *max_coord,
				     xypair_t *min_coord)
  
{

  /*
   * initialize
   */

  max_coord->x = -LARGE_DOUBLE;
  max_coord->y = -LARGE_DOUBLE;
  min_coord->x = LARGE_DOUBLE;
  min_coord->y = LARGE_DOUBLE;

  for (size_t i = 0; i < storms.size(); i++) {

    TrTrack::props_t &current = storms[i]->current;

    max_coord->x =
      MAX(max_coord->x, current.proj_area_centroid_x);
    
    max_coord->y =
      MAX(max_coord->y, current.proj_area_centroid_y);

    min_coord->x =
      MIN(min_coord->x, current.proj_area_centroid_x);
    
    min_coord->y =
      MIN(min_coord->y, current.proj_area_centroid_y);

  } /* i */

}

/*********************************************************************
 * load_distance_array()
 *
 */

void TrForecast::_load_distance_array(const TitanStormFile &sfile,
				      vector<TrStorm*> &storms,
				      fl32 **distance_array)
  
{

  int min_history = _params.tracking_min_history_for_valid_forecast;
  
  double this_centroid_x, this_centroid_y;
  double other_centroid_x, other_centroid_y;
  double distance;
  double dx, dy;
  double theta;

  /*
   * Initialize the array to -1.0.
   * The -1's will be overwritten by the actual distance
   * for storms which are closer than the smoothing distance.
   */
  
  for (size_t i = 0; i < storms.size(); i++) {
    for (size_t j = 0; j < storms.size(); j++) {
      distance_array[i][j] = -1.0;
    }
  }
  
  /*
   * is this a flat or latlon projection?
   */
  
  bool flat_proj;
  if (sfile.scan().grid.proj_type == TITAN_PROJ_FLAT) {
    flat_proj = true;
  } else {
    flat_proj = false;
  }

  /*
   * load array
   */
  
  for (size_t i = 0; i < storms.size(); i++) {
    
    TrStorm &this_storm = *storms[i];
    
    /*
     * On the diagonal (i.e. for this_storm) set to 0.0 if
     * the history_in_secs exceeds min_history.
     */
    
    if (this_storm.track.status.history_in_secs >= min_history) {
      distance_array[i][i] = 0.0;
    }
    
    this_centroid_x = this_storm.current.proj_area_centroid_x;
    this_centroid_y = this_storm.current.proj_area_centroid_y;
    
    for (size_t j = i + 1; j < storms.size(); j++) {
      
      TrStorm &other_storm = *storms[j];
      
      other_centroid_x = other_storm.current.proj_area_centroid_x;
      other_centroid_y = other_storm.current.proj_area_centroid_y;
      
      if (flat_proj) {
	
	dx = this_centroid_x - other_centroid_x;
	dy = this_centroid_y - other_centroid_y;
	distance = sqrt(dx * dx + dy * dy);
	
      } else {
	
	PJGLatLon2RTheta(this_centroid_y, this_centroid_x,
			 other_centroid_y, other_centroid_x,
			 &distance, &theta);
	
      } /* if (flat_proj) */

      if (distance < _params.tracking_smoothing_radius) {

	if (other_storm.track.status.history_in_secs >= min_history) {
	  distance_array[i][j] = distance;
	}
	if (this_storm.track.status.history_in_secs >= min_history) {
	  distance_array[j][i] = distance;
	}

      } /* if (distance < smoothing_radius) */
	
    } /* j */
    
  } /* i */

}

/*********************************************************************
 * Smooth motion for this track
 *
 * returns 0 on success, -1 on failure
 */

void TrForecast::_smooth_motion(vector<TrStorm*> &storms,
				size_t istorm,
				TrTrack &this_track,
				fl32 **distance_array,
				bool ignore_this_storm,
				bool history_override,
				bool erratic_only)
  
{

  // sum up stats for smoothing

  double sum_proj_area_dx_dt = 0.0;
  double sum_proj_area_dy_dt = 0.0;
  double sum_weights = 0.0;
  double sum_history = 0.0;
  double this_storm_centroid_x = 0.0;
  double this_storm_centroid_y = 0.0;
  double this_storm_speed = 0.0;
  double this_storm_dirn = 0.0;
  int nClose = 0;

  for (size_t jstorm = 0; jstorm < storms.size(); jstorm++) {

    const TrTrack::props_t &current = storms[jstorm]->current;
    TrTrack &track = storms[jstorm]->track;
    double distance = distance_array[istorm][jstorm];
    
    if (istorm != jstorm && distance < 0) {
      continue;
    }
    
    if (istorm == jstorm) {
      // compute motion for current storm
      this_storm_centroid_x = current.proj_area_centroid_x;
      this_storm_centroid_y = current.proj_area_centroid_y;
      _compute_motion(this_storm_centroid_x,
                      this_storm_centroid_y,
                      track.status.dval_dt.proj_area_centroid_x,
                      track.status.dval_dt.proj_area_centroid_y,
                      this_storm_speed, this_storm_dirn);
      if (_params.debug >= Params::DEBUG_EXTRA) {
	fprintf(stderr, "SPATIAL_SMOOTHING - THIS STORM\n");
	fprintf(stderr, "-------> Complex track %d, Simple %d\n",
		track.status.complex_track_num,
		track.status.simple_track_num);
      }
    }

    if (ignore_this_storm && istorm == jstorm) {
      continue;
    }
     
    nClose++;
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      fprintf(stderr,
	      "----> Complex, simple, distance, speed, dirn: "
	      "%4d %4d %8g %8g %8g\n",
	      track.status.complex_track_num,
	      track.status.simple_track_num, distance,
	      this_storm_speed, this_storm_dirn);
    }
    
    double proj_area_dx_dt = track.status.dval_dt.proj_area_centroid_x;
    double proj_area_dy_dt = track.status.dval_dt.proj_area_centroid_y;
    
    // compute weight
    
    double weight = 1.0;

    if (_params.smoothing_weight_uses_inverse_distance) {
      weight = 1.0 - distance / _params.tracking_smoothing_radius;
    }
    
    if (_params.smoothing_weight_uses_mean_dbz) {
      double deltaDbz = current.dbz_mean - _params.low_dbz_threshold;
      weight *= deltaDbz;
    }
    
    if (_params.smoothing_weight_uses_top) {
      double deltaHt = current.top - _params.base_threshold;
      weight *= deltaHt;
    }
    
    sum_proj_area_dx_dt += proj_area_dx_dt * weight;
    sum_proj_area_dy_dt += proj_area_dy_dt * weight;
    sum_history += (double) track.status.history_in_secs * weight;
    sum_weights += weight;
    
  } /* jstorm */
  
  // check we have enough storms within the radius of influence
  
  if (nClose < _params.tracking_smoothing_min_nstorms || sum_weights == 0.0) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "-------> too few storms within radius %d\n", nClose);
    }
    return;
  }
  
  // compute mean motion and history
  
  double mean_dx_dt = sum_proj_area_dx_dt / sum_weights;
  double mean_dy_dt = sum_proj_area_dy_dt / sum_weights;
  double mean_history = sum_history / sum_weights;
  
  // compute motion
  
  double mean_speed, mean_dirn;
  _compute_motion(this_storm_centroid_x,
                  this_storm_centroid_y,
                  mean_dx_dt,
                  mean_dy_dt,
                  mean_speed,
                  mean_dirn);
  
  // check erratic motion if required
  
  if (erratic_only) {

    // compute speed error

    double speed_error = fabs(this_storm_speed - mean_speed);
    double speed_error_percent = 0.0;
    if (this_storm_speed > mean_speed) {
      speed_error_percent = (speed_error / this_storm_speed) * 100.0;
    } else {
      speed_error_percent = (speed_error / mean_speed) * 100.0;
    }

    // compute direction error

    double dirn_error_deg = this_storm_dirn - mean_dirn;
    if (dirn_error_deg < 0) {
      dirn_error_deg += 360.0;
    }
    
    // are both speed error and dirn error OK?
    
    if (speed_error_percent < _params.erratic_forecast_speed_error &&
	dirn_error_deg < _params.erratic_forecast_direction_error) {
      return;
    }
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      fprintf(stderr, "-------> Smoothing ERRATIC storm\n");
      fprintf(stderr, "---------> this_storm_speed: %g\n", this_storm_speed);
      fprintf(stderr, "---------> mean_speed:       %g\n", mean_speed);
      fprintf(stderr, "---------> this_storm_dirn: %g\n", this_storm_dirn);
      fprintf(stderr, "---------> mean_dirn:       %g\n", mean_dirn);
    }

  }
  
  /*
   * set the smoothed storm props to the weighted means, in units per hr
   */

  this_track.status.dval_dt.smoothed_proj_area_centroid_x = mean_dx_dt;
  this_track.status.dval_dt.smoothed_proj_area_centroid_y = mean_dy_dt;
  this_track.status.smoothed_history_in_secs = mean_history;
  
  // for history override, reset the (x,y) for the history
  
  if (history_override) {
    
    time_t latest_time = this_track.status.history[0].time.unix_time;
    double latest_x = this_track.status.history[0].proj_area_centroid_x;
    double latest_y = this_track.status.history[0].proj_area_centroid_y;
    
    int nHist = this_track.status.history_in_scans;
    if (nHist > MAX_NWEIGHTS_FORECAST) {
      nHist = MAX_NWEIGHTS_FORECAST;
    }
    
    for (int ii = 1; ii < nHist; ii++) {
      
      time_t prev_time = this_track.status.history[ii].time.unix_time;
      double dh = (latest_time - prev_time) / 3600.0;
      double dx_dt = this_track.status.dval_dt.smoothed_proj_area_centroid_x;
      double dy_dt = this_track.status.dval_dt.smoothed_proj_area_centroid_y;
      double dx = dx_dt * dh;
      double dy = dy_dt * dh;
      double prev_x = latest_x - dx;
      double prev_y = latest_y - dy;
      
      this_track.status.history[ii].proj_area_centroid_x = prev_x;
      this_track.status.history[ii].proj_area_centroid_y = prev_y;
      
    } // ii
    
  } // if (history_override)
  
    //   } else {
  
    //     copy_props(this_track);
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    fprintf(stderr,
	    "SPATIAL_SMOOTHING - END: dx_dt, dy_dt, hist: "
	    "%g, %g, %g\n",
	    this_track.status.dval_dt.smoothed_proj_area_centroid_x,
	    this_track.status.dval_dt.smoothed_proj_area_centroid_y,
	    this_track.status.smoothed_history_in_secs);
  }

}

/*********************************************************************
 * Limit relative speed change in the track motion
 */

void TrForecast::_limit_rel_speed_change(const TitanStormFile &sfile,
					 TrTrack &this_track)
  
{
  
  TrTrack::status_t &status = this_track.status;

  // check we have enough scans

  if (status.history_in_scans < _params.tracking_rel_speed_min_nscans) {
    return;
  }

  // decide how much history to use
  
  int nHistory = status.history_in_scans;
  if (nHistory > _params.tracking_forecast_weights_n) {
    nHistory = _params.tracking_forecast_weights_n;
  }

  // projection type

  bool flat_proj;
  if (sfile.scan().grid.proj_type == TITAN_PROJ_FLAT) {
    flat_proj = true;
  } else {
    flat_proj = false;
  }

  // compute speeds
  
  vector<double> dxs, dys;
  vector<double> speeds;

  for (int ii = 1; ii < nHistory; ii++) {
    
    double dtime = ((double) status.history[ii-1].time.unix_time -
		    (double) status.history[ii].time.unix_time);
    
    double dx, dy;
    
    if (flat_proj) {
      dx = (status.history[ii-1].proj_area_centroid_x -
	    status.history[ii].proj_area_centroid_x);
      dy = (status.history[ii-1].proj_area_centroid_y -
	    status.history[ii].proj_area_centroid_y);
    } else {
      PJGLatLon2DxDy(status.history[ii].proj_area_centroid_y,
		     status.history[ii].proj_area_centroid_x,
		     status.history[ii-1].proj_area_centroid_y,
		     status.history[ii-1].proj_area_centroid_x,
		     &dx, &dy);
    }

    double speed = 0.0;
    if (dtime > 0.0) {
      speed = sqrt(dx * dx + dy * dy) / dtime;
    }

    dxs.push_back(dx);
    dys.push_back(dy);
    speeds.push_back(speed);

  } // ii

  // sort the speeds to get the median
  
  vector<double> sortedSpeeds = speeds;
  sort(sortedSpeeds.begin(), sortedSpeeds.end());
  double median = sortedSpeeds[sortedSpeeds.size() / 2];
  
  // is there a large change?

  vector<bool> largeChanges;
  bool largeChange = false;
  for (size_t ii = 0; ii < speeds.size(); ii++) {
    double relChange = speeds[ii] / median;
    if (relChange  > _params.tracking_rel_speed_max_change) {
      largeChange = true;
      largeChanges.push_back(true);
      if (_params.debug) {
	cerr << "===> bad speed: " << speeds[ii] << endl;
      }
    } else {
      largeChanges.push_back(false);
    }
  } // ii

  if (!largeChange) {
    return;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {

    cerr << "=============== limitRelSpeedChange ==============" << endl;

    cerr << "  complex num: " << status.complex_track_num << endl;
    cerr << "  simple num: " << status.simple_track_num << endl;
    cerr << "  history: " << status.history_in_scans << endl;
    cerr << "  duration: " << status.duration_in_scans << endl;

    cerr << "  Speeds: ";
    for (size_t ii = 0; ii < speeds.size(); ii++) {
      cerr << speeds[ii] << " ";
    }
    cerr << endl;
    cerr << "  median: " << median << endl;
    cerr << endl;

    cerr << "-----> storm positions before adjustment" << endl;
    for (int ii = 0; ii < nHistory; ii++) {
      cerr << "ii, time, xx, yy, dx, dy, speed: "
	   << ii << ", "
	   << utimestr(&status.history[ii].time) << ", "
	   << status.history[ii].proj_area_centroid_x << ", "
	   << status.history[ii].proj_area_centroid_y;
      if (ii == 0) {
	cerr << endl;
	continue;
      }
      double dtime = ((double) status.history[ii-1].time.unix_time -
		      (double) status.history[ii].time.unix_time);
      double dx, dy;
      if (flat_proj) {
	dx = (status.history[ii-1].proj_area_centroid_x -
	      status.history[ii].proj_area_centroid_x);
	dy = (status.history[ii-1].proj_area_centroid_y -
	      status.history[ii].proj_area_centroid_y);
      } else {
	PJGLatLon2DxDy(status.history[ii].proj_area_centroid_y,
		       status.history[ii].proj_area_centroid_x,
		       status.history[ii-1].proj_area_centroid_y,
		       status.history[ii-1].proj_area_centroid_x,
		       &dx, &dy);
      }
      double speed = 0.0;
      if (dtime > 0.0) {
	speed = sqrt(dx * dx + dy * dy) / dtime;
      }
      cerr << ", " << dx << ", " << dy << ", " << speed << endl;
    } // ii

  } // if debug

  // go through the history, adjusting the large change back to the
  // median value
  
  for (int ii = 1; ii < nHistory; ii++) {
    
    double dx, dy;
    if (largeChanges[ii-1]) {
      double ratio = median / speeds[ii-1];
      dx = dxs[ii-1] * ratio;
      dy = dys[ii-1] * ratio;
    } else {
      dx = dxs[ii-1];
      dy = dys[ii-1];
    }
    
    if (flat_proj) {
      status.history[ii].proj_area_centroid_x =
	status.history[ii-1].proj_area_centroid_x - dx;
      status.history[ii].proj_area_centroid_y =
	status.history[ii-1].proj_area_centroid_y - dy;
    } else {
      PJGLatLonPlusDxDy(status.history[ii-1].proj_area_centroid_y,
			status.history[ii-1].proj_area_centroid_x,
			-dx, -dy,
			&status.history[ii].proj_area_centroid_y,
			&status.history[ii].proj_area_centroid_x);
    }
    
  } // ii

  if (_params.debug >= Params::DEBUG_EXTRA) {
    
    cerr << "---> storm positions after adjustment" << endl;
    for (int ii = 0; ii < nHistory; ii++) {
      cerr << "ii, time, xx, yy, dx, dy, speed: "
	   << ii << ", "
	   << utimestr(&status.history[ii].time) << ", "
	   << status.history[ii].proj_area_centroid_x << ", "
	   << status.history[ii].proj_area_centroid_y;
      if (ii == 0) {
	cerr << endl;
	continue;
      }
      double dtime = ((double) status.history[ii-1].time.unix_time -
		      (double) status.history[ii].time.unix_time);
      double dx, dy;
      if (flat_proj) {
	dx = (status.history[ii-1].proj_area_centroid_x -
	      status.history[ii].proj_area_centroid_x);
	dy = (status.history[ii-1].proj_area_centroid_y -
	      status.history[ii].proj_area_centroid_y);
      } else {
	PJGLatLon2DxDy(status.history[ii].proj_area_centroid_y,
		       status.history[ii].proj_area_centroid_x,
		       status.history[ii-1].proj_area_centroid_y,
		       status.history[ii-1].proj_area_centroid_x,
		       &dx, &dy);
      }
      double speed = 0.0;
      if (dtime > 0.0) {
	speed = sqrt(dx * dx + dy * dy) / dtime;
      }
      cerr << ", " << dx << ", " << dy << ", " << speed << endl;
    } // ii

  } // if debug

}

/////////////////////////////////////////
// load speed and dirn for all tracks

void TrForecast::_load_speed_and_dirn(TitanStormFile &sfile,
                                      vector<TrStorm*> &storms,
                                      bool allow_field_tracker)
  
{

  // do we need field tracker?

  DsMdvx fieldTrackerData;
  _useFieldTracker = false;
  MdvxField *uField = NULL;
  MdvxField *vField = NULL;
  MdvxProj fieldProj;
  if (allow_field_tracker &&
      _params.override_early_storm_motion_using_field_tracker) {
    if (_load_field_tracker_data(sfile, fieldTrackerData,
                                 uField, vField, fieldProj) == 0) {
      _useFieldTracker = true;
    }
  }

  // set the storm projection
  
  titan_grid_comps_t titanProj;
  const titan_grid_t &grid = sfile.scan().grid;
  TITAN_init_proj(&grid, &titanProj);
  
  // loop through all storms

  for (size_t i = 0; i < storms.size(); i++) {

    // find track

    TrTrack &track = storms[i]->track;
    const storm_file_global_props_t *gprops = sfile.gprops() + i;

    double speed = 0.0;
    double dirn = 0.0;

    // compute motion from track

    _compute_motion(gprops->proj_area_centroid_x,
                    gprops->proj_area_centroid_y,
                    track.status.dval_dt.smoothed_proj_area_centroid_x,
                    track.status.dval_dt.smoothed_proj_area_centroid_y,
                    speed, dirn);
    
    track.status.dval_dt.smoothed_speed = speed;
    track.status.dval_dt.smoothed_direction = dirn;
    if (speed >	_params.tracking_max_speed_for_valid_forecast) {
      track.status.forecast_valid = false;
    }

    // optionally override speed and dirn from field tracker results

    if (_useFieldTracker) {
      // _override_motion_using_centroid(sfile, uField, vField,
      //                                 titanProj, fieldProj, *gprops, track);
      _override_motion_using_perimeter(sfile, uField, vField,
                                       titanProj, fieldProj, *gprops, track);
    }

  } /* i */

}

/////////////////////////////////////////
// compute motion speed and dirn

void TrForecast::_compute_motion(double storm_x,
                                 double storm_y,
                                 double dx_dt,
                                 double dy_dt,
                                 double &speed,
                                 double &dirn)
  
{

  double dx_km = 0.0, dy_km = 0.0;

  // compute movement over 1 hour
  
  if (_projIsLatLon) {
    
    double start_lon = storm_x;
    double start_lat = storm_y;
    
    double end_lon = start_lon + dx_dt;
    double end_lat = start_lat + dy_dt;

    PJGLatLon2DxDy(start_lat, start_lon, end_lat, end_lon,
		   &dx_km, &dy_km);

  } else {
    
    dx_km = dx_dt;
    dy_km = dy_dt;
    
  } /* if (latlon) */
  
  speed = sqrt(dx_km * dx_km + dy_km * dy_km);
  
  dirn = 0.0;
  if (dx_km != 0.0 || dy_km != 0.0) {
    dirn = atan2(dx_km, dy_km) * RAD_TO_DEG;
  }
  
  if (dirn < 0.0) {
    dirn += 360.0;
  }
    
}


///////////////////////////////////////////////////////
// load up field tracker data from MDV file
// returns 0 on success, -1 on failure

int TrForecast::_load_field_tracker_data(const TitanStormFile &sfile,
                                         DsMdvx &fieldTrackerData,
                                         MdvxField* &uField,
                                         MdvxField* &vField,
                                         MdvxProj &fieldProj)
  
{

  // get data

  while (true) {

    // try to get field tracker data
    
    time_t stormTime = sfile.scan().time;
    fieldTrackerData.setReadTime(Mdvx::READ_CLOSEST,
                                 _params.field_tracker_url,
                                 _params.field_tracker_search_margin_secs,
                                 stormTime);
    fieldTrackerData.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    fieldTrackerData.setReadCompressionType(Mdvx::COMPRESSION_NONE);
    
    if (fieldTrackerData.readVolume() == 0) {
      // success finding file
      break;
    }

    // if not in realtime mode, then failed
    
    if (_params.mode != Params::REALTIME) {
      cerr << "ERROR - TrForecast::_load_field_tracker_data" << endl;
      cerr << "  Could not find field tracker data file" << endl;
      cerr << fieldTrackerData.getErrStr() << endl;
      return -1;
    }
    
    // in realtime mode, wait a while

    time_t now = time(NULL);
    double delaySecs = (double) now - (double) stormTime;
    if (delaySecs > _params.field_tracker_realtime_wait_secs) {
      cerr << "ERROR - TrForecast::_load_field_tracker_data" << endl;
      cerr << "  REALTIME mode - timed out waiting for field tracker data" << endl;
      cerr << fieldTrackerData.getErrStr() << endl;
      return -1;
    }

    // sleep a bit

    PMU_auto_register("Waiting for field tracker data");
    umsleep(2000);

  } // while (!done)
  
  // ensure we have the fields we need

  uField = fieldTrackerData.getField(_params.field_tracker_U_motion_name);
  vField = fieldTrackerData.getField(_params.field_tracker_V_motion_name);
  
  if (uField == NULL) {
    cerr << "ERROR - TrForecast::_load_field_tracker_data" << endl;
    cerr << "  Cannot find U motion field: " << _params.field_tracker_U_motion_name << endl;
    cerr << "  file: " << fieldTrackerData.getPathInUse() << endl;
    return -1;
  }
  
  if (vField == NULL) {
    cerr << "ERROR - TrForecast::_load_field_tracker_data" << endl;
    cerr << "  Cannot find V motion field: " << _params.field_tracker_V_motion_name << endl;
    cerr << "  file: " << fieldTrackerData.getPathInUse() << endl;
    return -1;
  }

  // initialize the projection

  fieldProj.init(fieldTrackerData.getMasterHeader(), uField->getFieldHeader());
  
  return 0;

}

///////////////////////////////////////////////////////
// override speed and dirn using field tracker results
// using the value at the centroid

void TrForecast::_override_motion_using_centroid(TitanStormFile &sfile,
                                                 const MdvxField *uField,
                                                 const MdvxField *vField,
                                                 titan_grid_comps_t &titanProj,
                                                 const MdvxProj &fieldProj,
                                                 const storm_file_global_props_t &gprops,
                                                 TrTrack &track)
  
{

  // cerr << "====================================================================" << endl;
  // cerr << "TTTTTTTTTTTTTTTTT stormTime: " << DateTime::strm(sfile.scan().time) << endl;
  // cerr << "11111111111111111 complex num: " << track.status.complex_track_num << endl;
  // cerr << "11111111111111111 simple num: " << track.status.simple_track_num << endl;

  // do we need to override for this track?
  // check track age

  int age = track.status.history_in_secs;
  if (age > (_params.field_tracker_initial_period_secs + 
             _params.field_tracker_transition_period_secs)) {
    return;
  }

  const Mdvx::field_header_t &uFhdr = uField->getFieldHeader();
  const Mdvx::field_header_t &vFhdr = vField->getFieldHeader();
  int nx = uFhdr.nx;
  int ny = uFhdr.ny;

  // get storm area centroid lat/lon
  
  double stormX = gprops.proj_area_centroid_x;
  double stormY = gprops.proj_area_centroid_y;
  double stormLat, stormLon;
  TITAN_xy2latlon(&titanProj, stormX, stormY, &stormLat, &stormLon);

  // cerr << "3333333333 stormX, stormY: " << stormX << ", " << stormY << endl;
  // cerr << "3333333333 stormLat, stormLon: " << stormLat << ", " << stormLon << endl;
  
  // get search limits in lat/lon

  double radiusKm = _params.field_tracker_search_radius_km;
  double minLat, minLon;
  PJGLatLonPlusDxDy(stormLat, stormLon, -radiusKm, -radiusKm, &minLat, &minLon);
  double maxLat, maxLon;
  PJGLatLonPlusDxDy(stormLat, stormLon, radiusKm, radiusKm, &maxLat, &maxLon);

  // cerr << "4444444444 radiusKm: " << radiusKm << endl;
  // cerr << "3333333333 minLat, minLon: " << minLat << ", " << minLon << endl;
  // cerr << "3333333333 maxLat, maxLon: " << maxLat << ", " << maxLon << endl;
  
  // get field tracker grid search limits

  int minIx, minIy, maxIx, maxIy;
  fieldProj.latlon2xyIndex(minLat, minLon, minIx, minIy);
  fieldProj.latlon2xyIndex(maxLat, maxLon, maxIx, maxIy);
  if (minIx < 0) minIx = 0;
  if (minIy < 0) minIy = 0;
  if (maxIx > nx - 1) maxIx = nx - 1;
  if (maxIy > ny - 1) maxIy = ny - 1;
  
  // cerr << "3333333333 minIx, minIy: " << minIx << ", " << minIy << endl;
  // cerr << "3333333333 maxIx, maxIy: " << maxIx << ", " << maxIy << endl;

  // sum up U and V

  fl32 uMissing = uFhdr.missing_data_value;
  fl32 vMissing = vFhdr.missing_data_value;

  const fl32 *uu = (fl32 *) uField->getVol();
  const fl32 *vv = (fl32 *) vField->getVol();

  double sumU = 0.0, sumV = 0.0;
  int countU = 0, countV = 0;
  for (int iy = minIy; iy <= maxIy; iy++) {
    
    const fl32 *uptr = uu + iy * uFhdr.nx + minIx;
    const fl32 *vptr = vv + iy * uFhdr.nx + minIx;

    for (int ix = minIx; ix <= maxIx; ix++, uptr++, vptr++) {
      if (*uptr != uMissing) {
        // cerr << "uuuuuuuuuuuu: " << *uptr << endl;
        // if (!isfinite(*uptr)) {
        //   cerr << "UUUUUUUUU iy, ix: " << iy << ", " << ix << endl;
        // }
        sumU += *uptr;
        countU++;
      }
      if (*vptr != vMissing) {
        // cerr << "vvvvvvvvvvvv: " << *vptr << endl;
        // if (!isfinite(*vptr)) {
        //   cerr << "VVVVVVVVV iy, ix: " << iy << ", " << ix << endl;
        // }
        sumV += *vptr;
        countV++;
      }
    } // ix
  } // iy

  // compute mean U and V - these are in m/s

  if (countU == 0 || countV == 0) {
    // not enough data
    return;
  }
  double meanU = sumU / countU;
  double meanV = sumV / countV;

  // convert to km/hr

  meanU *= 3.6;
  meanV *= 3.6;
  
  // cerr << "11111111111111 sumU, sumV: " << sumU << ", " << sumV << endl;
  // cerr << "11111111111111 countU, countV: " << countU << ", " << countV << endl;
  // cerr << "11111111111111 meanU, meanV: " << meanU << ", " << meanV << endl;

  // get titan motion in U and V
  
  double titanSpeed = track.status.dval_dt.smoothed_speed;
  double titanDirn = track.status.dval_dt.smoothed_direction;
  double sinDirn, cosDirn;
  ta_sincos(titanDirn * DEG_TO_RAD, &sinDirn, &cosDirn);
  double titanU = titanSpeed * sinDirn;
  double titanV = titanSpeed * cosDirn;
  
  // cerr << "11111111111111 titanU, titanV: " << titanU << ", " << titanV << endl;
  // cerr << "11111111111111 age: " << age << endl;

  // compute weight to merge the speeds

  double titanWt = 1.0;
  if (titanU == 0.0 && titanV == 0.0) {
    titanWt = 0.0;
  } else if (age < _params.field_tracker_initial_period_secs) {
    titanWt = 0.0;
  } else if (age > (_params.field_tracker_initial_period_secs + 
                    _params.field_tracker_transition_period_secs)) {
    titanWt = 1.0;
  } else {
    titanWt = (((double) age -
                (double) _params.field_tracker_initial_period_secs) / 
               ((double) _params.field_tracker_transition_period_secs));
  }

  // cerr << "XXXXXXXX titanWt: " << titanWt << endl;
  // cerr << "11111111111111 meanU, meanV: " << meanU << ", " << meanV << endl;
  // cerr << "11111111111111 titanU, titanV: " << titanU << ", " << titanV << endl;
  
  // compute merged vector

  double combinedU = titanU * titanWt + meanU * (1.0 - titanWt);
  double combinedV = titanV * titanWt + meanV * (1.0 - titanWt);

  // cerr << "11111111111111 combinedU, combinedV: " << combinedU << ", " << combinedV << endl;

  // compute merged speed and direction
  
  double combinedSpeed = sqrt(combinedU * combinedU + combinedV * combinedV);
  double combinedDirn = 0.0;
  if (combinedU != 0.0 || combinedV != 0.0) {
    combinedDirn = atan2(combinedU, combinedV) * RAD_TO_DEG;
  }
  if (combinedDirn < 0.0) {
    combinedDirn += 360.0;
  }
    
  track.status.dval_dt.smoothed_proj_area_centroid_x = combinedU;
  track.status.dval_dt.smoothed_proj_area_centroid_y = combinedV;
  track.status.dval_dt.smoothed_speed = combinedSpeed;
  track.status.dval_dt.smoothed_direction = combinedDirn;

}

///////////////////////////////////////////////////////
// override speed and dirn using field tracker results
// using the values at the perimeter of the storm
//
// TODO - this routine assumes that the titan grid and the
// field tracker grid are the same. Need to incorporate optical
// flow directly into Titan so that we know this to be the case.

void TrForecast::_override_motion_using_perimeter(TitanStormFile &sfile,
                                                  const MdvxField *uField,
                                                  const MdvxField *vField,
                                                  titan_grid_comps_t &titanProj,
                                                  const MdvxProj &fieldProj,
                                                  const storm_file_global_props_t &gprops,
                                                  TrTrack &track)
  
{

  // cerr << "********************************************************************" << endl;
  // cerr << "TTTTTTTTTTTTTTTTT stormTime: " << DateTime::strm(sfile.scan().time) << endl;
  // cerr << "11111111111111111 complex num: " << track.status.complex_track_num << endl;
  // cerr << "11111111111111111 simple num: " << track.status.simple_track_num << endl;

  // do we need to override for this track?
  // check track age

  int age = track.status.history_in_secs;
  if (age > (_params.field_tracker_initial_period_secs + 
             _params.field_tracker_transition_period_secs)) {
    return;
  }

  const Mdvx::field_header_t &uFhdr = uField->getFieldHeader();
  const Mdvx::field_header_t &vFhdr = vField->getFieldHeader();
  int uNx = uFhdr.nx;
  int uNy = uFhdr.ny;
  double uDx = uFhdr.grid_dx;
  double uDy = uFhdr.grid_dy;
  double uMinx = uFhdr.grid_minx;
  double uMiny = uFhdr.grid_miny;

  // get storm area centroid lat/lon
  
  double stormX = gprops.proj_area_centroid_x;
  double stormY = gprops.proj_area_centroid_y;
  double stormLat, stormLon;
  TITAN_xy2latlon(&titanProj, stormX, stormY, &stormLat, &stormLon);

  // cerr << "3333333333 stormX, stormY: " << stormX << ", " << stormY << endl;
  // cerr << "3333333333 stormLat, stormLon: " << stormLat << ", " << stormLon << endl;

  // get centroid location in grid units (in field tracker grid)
  
  double centroidX, centroidY;
  fieldProj.latlon2xy(stormLat, stormLon, centroidX, centroidY);
  // cerr << "3333333333 centroidX, centroidY: " << centroidX << ", " << centroidY << endl;
  // cerr << "3333333333 stormIx, stormIy: "
  //      << (centroidX - uMinx) / uDx << ", "
  //      << (centroidY - uMiny) / uDy << endl;
  
  // loop through the points describing the storm polygon
  // sum up U and V

  fl32 uMissing = uFhdr.missing_data_value;
  fl32 vMissing = vFhdr.missing_data_value;

  const fl32 *uu = (fl32 *) uField->getVol();
  const fl32 *vv = (fl32 *) vField->getVol();

  double sumU = 0.0, sumV = 0.0;
  int countU = 0, countV = 0;

  const storm_file_params_t &sparams = sfile.params();
  double poly_delta_az = sparams.poly_delta_az * DEG_TO_RAD;
  double theta = sparams.poly_start_az * DEG_TO_RAD;
  int nSides = sparams.n_poly_sides;

  for (int iray = 0; iray < nSides; iray++, theta += poly_delta_az) {
    
    double rayLen = gprops.proj_area_polygon[iray];
    double rayX = centroidX + rayLen * sin(theta); // grid units
    double rayY = centroidY + rayLen * cos(theta); // grid units

    int rayIx = (int) floor ((rayX - uMinx) / uDx + 0.5);
    int rayIy = (int) floor ((rayY - uMiny) / uDy + 0.5);
    
    if (rayIx >= 0 && rayIy >= 0 && rayIx < uNx && rayIy < uNy) {

      const fl32 *uptr = uu + rayIy * uNx + rayIx;
      const fl32 *vptr = vv + rayIy * uNx + rayIx;
      
      if (*uptr != uMissing) {
        sumU += *uptr;
        countU++;
      }
      if (*vptr != vMissing) {
        sumV += *vptr;
        countV++;
      }

    }
      
  } /* iray */
  
  // compute mean U and V - these are in m/s
  
  if (countU == 0 || countV == 0) {
    // not enough data
    return;
  }
  double meanU = sumU / countU;
  double meanV = sumV / countV;

  // convert to km/hr

  meanU *= 3.6;
  meanV *= 3.6;
  
  // cerr << "11111111111111 sumU, sumV: " << sumU << ", " << sumV << endl;
  // cerr << "11111111111111 countU, countV: " << countU << ", " << countV << endl;
  // cerr << "11111111111111 meanU, meanV: " << meanU << ", " << meanV << endl;

  // get titan motion in U and V
  
  double titanSpeed = track.status.dval_dt.smoothed_speed;
  double titanDirn = track.status.dval_dt.smoothed_direction;
  double sinDirn, cosDirn;
  ta_sincos(titanDirn * DEG_TO_RAD, &sinDirn, &cosDirn);
  double titanU = titanSpeed * sinDirn;
  double titanV = titanSpeed * cosDirn;
  
  // cerr << "11111111111111 titanU, titanV: " << titanU << ", " << titanV << endl;
  // cerr << "11111111111111 age: " << age << endl;

  // compute weight to merge the speeds

  double titanWt = 1.0;
  if (titanU == 0.0 && titanV == 0.0) {
    titanWt = 0.0;
  } else if (age < _params.field_tracker_initial_period_secs) {
    titanWt = 0.0;
  } else if (age > (_params.field_tracker_initial_period_secs + 
                    _params.field_tracker_transition_period_secs)) {
    titanWt = 1.0;
  } else {
    titanWt = (((double) age -
                (double) _params.field_tracker_initial_period_secs) / 
               ((double) _params.field_tracker_transition_period_secs));
  }

  // cerr << "XXXXXXXX titanWt: " << titanWt << endl;
  // cerr << "11111111111111 meanU, meanV: " << meanU << ", " << meanV << endl;
  // cerr << "11111111111111 titanU, titanV: " << titanU << ", " << titanV << endl;
  
  // compute merged vector
  
  double combinedU = titanU * titanWt + meanU * (1.0 - titanWt);
  double combinedV = titanV * titanWt + meanV * (1.0 - titanWt);

  // cerr << "11111111111111 combinedU, combinedV: " << combinedU << ", " << combinedV << endl;

  // compute merged speed and direction
  
  double combinedSpeed = sqrt(combinedU * combinedU + combinedV * combinedV);
  double combinedDirn = 0.0;
  if (combinedU != 0.0 || combinedV != 0.0) {
    combinedDirn = atan2(combinedU, combinedV) * RAD_TO_DEG;
  }
  if (combinedDirn < 0.0) {
    combinedDirn += 360.0;
  }
  
  track.status.dval_dt.smoothed_proj_area_centroid_x = combinedU;
  track.status.dval_dt.smoothed_proj_area_centroid_y = combinedV;
  track.status.dval_dt.smoothed_speed = combinedSpeed;
  track.status.dval_dt.smoothed_direction = combinedDirn;

  // cerr << "111111111111 combSpeed, combDirn: " << combinedSpeed << ", " << combinedDirn << endl;
  // cerr << "********************************************************************" << endl;

}

