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
//////////////////////////////////////////////////////////
// Properties.cc
//
// Storm properties computations
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// January 1998
//
//////////////////////////////////////////////////////////

#include "Properties.hh"
#include "TrackEntry.hh"
#include <toolsa/str.h>
#include <rapmath/math_macros.h>
using namespace std;

//////////////
// Constructor

Properties::Properties (const char *prog_name,
			const Params &params,
			storm_file_handle_t *s_handle,
			track_file_handle_t *t_handle) :
        _params(params)
  
{
  
  // set pointers & vals

  _progName = STRdup(prog_name);
  _debug = _params.debug;
  _s_handle = s_handle;
  _t_handle = t_handle;
  OK = TRUE;

  // initialize mem buffer for time series array

  _tSeriesBuf = MEMbufCreate();
  
}

/////////////
// destructor

Properties::~Properties ()
  
{

  // free up memmory

  MEMbufDelete(_tSeriesBuf);
  STRfree(_progName);

}

//////////////
// Compute
//
// Compute props
//

int Properties::compute (int complex_track_num)
  
{

  _complexTrackNum = complex_track_num;

  // read in complex track params

  if(RfReadComplexTrackParams(_t_handle, complex_track_num, TRUE,
			      "Properties::compute")) {
    return(-1);
  }
  complex_track_params_t *ct_params = _t_handle->complex_params;

  // basic props

  _nScans = ct_params->duration_in_scans;
  _startScan = ct_params->start_scan;
  _endScan = ct_params->end_scan;
  _startTime = ct_params->start_time;
  _endTime = ct_params->end_time;
  _dbzThresh = _s_handle->header->params.low_dbz_threshold;

  if (ct_params->duration_in_secs < _params.min_duration ||
      ct_params->duration_in_secs > _params.max_duration) {
    return (-1);
  }
  _duration = (double) ct_params->duration_in_secs / 3600.0;

  // allocate space for the time_series

  _tSeriesProps = (time_series_props_t *)
    MEMbufPrepare(_tSeriesBuf, _nScans * sizeof(time_series_props_t));
  memset(_tSeriesProps, 0, _nScans * sizeof(time_series_props_t));

  // load up the time series props data for each scan

  if (_load_tseries_props()) {
    return (-1);
  }

  // start point

  _startX = _tSeriesProps[0].x;
  _startY = _tSeriesProps[0].y;

  // compute stats for data elements in the time series

  _compute_tseries_stats(&_tSeriesProps->area, 
			 &_mean.area,
			 &_min.area,
			 &_max.area,
			 &_sdev.area);
  
  // reflectivity distribution
  
  _compute_tseries_stats(&_tSeriesProps->area_dbz_fit, 
			 &_mean.area_dbz_fit,
			 &_min.area_dbz_fit,
			 &_max.area_dbz_fit,
			 &_sdev.area_dbz_fit);

  // shape
  
  _compute_tseries_stats(&_tSeriesProps->ellipse_ratio, 
			 &_mean.ellipse_ratio,
			 &_min.ellipse_ratio,
			 &_max.ellipse_ratio,
			 &_sdev.ellipse_ratio);
  
  _compute_tseries_stats(&_tSeriesProps->ellipse_orient, 
			 &_mean.ellipse_orient,
			 &_min.ellipse_orient,
			 &_max.ellipse_orient,
			 &_sdev.ellipse_orient);
  
  // speed and direction

  _compute_tseries_stats(&_tSeriesProps->u, 
			 &_mean.u,
			 &_min.u,
			 &_max.u,
			 &_sdev.u);

  _compute_tseries_stats(&_tSeriesProps->v, 
			 &_mean.v,
			 &_min.v,
			 &_max.v,
			 &_sdev.v);

  _speed = sqrt(_mean.u *_mean.u + _mean.v * _mean.v);
  if (_mean.u == 0.0 && _mean.v == 0.0) {
    _dirn = 0.0;
  } else {
    _dirn = atan2(_mean.u, _mean.v) * RAD_TO_DEG;
    if (_dirn < 0.0) {
      _dirn += 360.0;
    }
  }
      
  // compute serial correlations

  _compute_serial_corr(&_tSeriesProps->u, &_serialCorrU);
  _compute_serial_corr(&_tSeriesProps->v, &_serialCorrV);

  // compute time series integrals

  _compute_tseries_integrals();

  // compute fit to area history

  _compute_area_fit();

  return (0);

}
    
//////////////
// Print()
//
// Print props to output file.
//

void Properties::print(FILE *out)
  
{
  
  fprintf(out, "%d ", _complexTrackNum);
  fprintf(out, "%d ", (int)_startTime);
  
  fprintf(out, "%d ", _nScans);

  fprintf(out, "%g ", _duration);

  fprintf(out, "%g %g ", _gaussD, _gaussAmean);

  fprintf(out, "%g ", _dbzThresh);

  fprintf(out, "%g %g %g ",
	  _mean.area_dbz_fit + _dbzThresh,
	  _min.area_dbz_fit + _dbzThresh,
	  _max.area_dbz_fit + _dbzThresh);
  
  fprintf(out, "%g %g ", _startX, _startY);
  
  fprintf(out, "%g %g %g %g ",
	  _mean.u, _mean.v, _speed, _dirn);
  
  fprintf(out, "%g %g ", _mean.ellipse_ratio, _mean.ellipse_orient);
  
  fprintf(out, "\n");

}

//////////////////////////////////////////
// _load_tseries_props()
//
// load up scan-by-scan time series properties
//
// Returns 0 on success, -1 on failure
//
///////////////////////////////////////////

int Properties::_load_tseries_props()

{

  for (int isimple = 0;
       isimple < _t_handle->nsimples_per_complex[_complexTrackNum]; isimple++) {
    
    int simple_num = _t_handle->simples_per_complex[_complexTrackNum][isimple];

    // prepare the track entries for reading
    
    if(RfRewindSimpleTrack(_t_handle, simple_num,
			   "Properties::_load_tseries_props")) {
      return (-1);
    }
    
    simple_track_params_t *st_params = _t_handle->simple_params;
    
    for (int ientry = 0;
	 ientry < st_params->duration_in_scans; ientry++) {
    
      if (RfReadTrackEntry(_t_handle, "Properties::_load_tseries_props")) {
	return(-1);
      }
    
      track_file_entry_t *entry = _t_handle->entry;
      int scan_num = entry->scan_num;
      int storm_num = entry->storm_num;
    
      if (RfReadStormScan(_s_handle, scan_num,
			  "Properties::_load_tseries_props")) {
	return (-1);
      }
      storm_file_global_props_t *gprops = _s_handle->gprops + storm_num;

      if (RfReadStormProps(_s_handle, storm_num,
			   "Properties::_load_tseries_props")) {
	return (-1);
      }

      double volume_above_alt;
      double mass_above_alt;
      double base;
      double top;
      
      _compute_z_props(gprops, &volume_above_alt,
		       &mass_above_alt, &base, &top);
      
      time_series_props_t *tseries =
	_tSeriesProps + (scan_num - _startScan);

      tseries->x += gprops->proj_area_centroid_x * gprops->proj_area;
      tseries->y += gprops->proj_area_centroid_y * gprops->proj_area;
      
      tseries->scan_time = entry->time;
      tseries->n_parts++;
      tseries->precip_flux += gprops->precip_flux;
      tseries->mass += gprops->mass;
      tseries->mass_above_alt += mass_above_alt;
      tseries->volume += gprops->volume;
      tseries->volume_above_alt += volume_above_alt;

      if (gprops->dbz_max > tseries->dbz_max) {
	tseries->dbz_max = gprops->dbz_max;
	tseries->ht_of_dbz_max = gprops->ht_of_dbz_max;
      }
      
      if (tseries->base > base) {
	tseries->base = base;
      }
      
      if (tseries->top < top) {
	tseries->top = top;
      }
      
      tseries->area += gprops->proj_area;
      
      tseries->refl_centroid_z += gprops->refl_centroid_z * gprops->mass;

      // movement

      track_file_forecast_props_t *fprops = &entry->dval_dt;
      tseries->u = fprops->proj_area_centroid_x;
      tseries->v = fprops->proj_area_centroid_y;

      // dbz_fit

      double area_dbz_fit;
      _compute_elliptical_dbz_fit(&_s_handle->header->params,
				  gprops, _s_handle->hist,
				  &area_dbz_fit);

      if (area_dbz_fit > tseries->area_dbz_fit) {
	tseries->area_dbz_fit = area_dbz_fit;
      }

      // shape

      tseries->ellipse_ratio =
	gprops->proj_area_major_radius / gprops->proj_area_minor_radius;

      tseries->ellipse_orient = gprops->proj_area_orientation;

    } // ientry
    
  } // isimple

  // compute derived quantities

  time_series_props_t *tseries = _tSeriesProps;
  for (int i = 0; i < _nScans; i++, tseries++) {
    tseries->x /= tseries->area;
    tseries->y /= tseries->area;
    tseries->refl_centroid_z /= tseries->mass;
  }
  
  return (0);
  
}

///////////////////////////////////////////////////
// _compute_z_props()
//
// Compute the props related to altitude
//
///////////////////////////////////////////////////

void Properties::_compute_z_props(storm_file_global_props_t *gprops,
				  double *volume_above_alt_p,
				  double *mass_above_alt_p,
				  double *base_p,
				  double *top_p)
  
{

  // The storm layers are stored relative to a base layer. However,
  // the z values are stored relative to the grid. Therefore we need
  // to compute the z value of the base layer

  int n_layers = gprops->n_layers;
  int base_layer = gprops->base_layer;
  double min_z = _s_handle->scan->min_z;
  double delta_z = _s_handle->scan->delta_z;
  double base_z = min_z + base_layer * delta_z;
  double base = base_z - delta_z / 2.0;
  double top = base + n_layers * delta_z;

  // Find the layer which encompasses the altitude threshold.
  // For that layer, determine the fraction of the layer which is
  // above the threshold.
  
  double volume;
  double mass;

  if (_params.altitude_threshold > top) {

    volume = 0;
    mass = 0;
    
  } else {

    int threshold_layer;
    double threshold_fraction;
  
    if (_params.altitude_threshold > base) {
      
      threshold_layer = base_layer + 
	(int) ((_params.altitude_threshold - base_z) / delta_z + 0.5);
      double threshold_layer_z = min_z + threshold_layer * delta_z;
      threshold_fraction =
	(_params.altitude_threshold - threshold_layer_z) / delta_z + 0.5;

    } else {

      threshold_layer = base_layer;
      threshold_fraction = 1.0;

    }

    // sum up the volume and mass above the threshold
    
    storm_file_layer_props_t *layer =
      _s_handle->layer + (threshold_layer - base_layer);
    
    volume = layer->area * delta_z * threshold_fraction;
    mass = layer->mass * threshold_fraction;
    
    layer++;
    
    for (int i = threshold_layer - base_layer + 1;
	 i < n_layers; i++, layer++) {
      volume += layer->area * delta_z;
      mass += layer->mass;
    }

  } // if (_params.altitude_threshold > top) 

  // set return values

  *volume_above_alt_p = volume;
  *mass_above_alt_p = mass;
  *base_p = base;
  *top_p = top;

}

///////////////////////////////////////////////////
// _compute_tseries_stats()
//
// Compute the stats for a time series variable.
//
// Computes the mean, min, max and standard deviation
//
///////////////////////////////////////////////////

void Properties::_compute_tseries_stats(double *tseries_val,
					double *mean_p,
					double *min_p,
					double *max_p,
					double *sdev_p)
  
{

  double min = 1.0e99;
  double max = -1.0e99;
  double sumx = 0.0;
  double sumx2 = 0.0;
  double n = 0.0;

  // loop through scans

  for (int i = 0; i < _nScans; i++) {

    // get value in time series
    
    double *tval =
      (double *) ((char *) tseries_val + sizeof(time_series_props_t) * i);
    double this_val = *tval;

    // increment counters for stats

    n++;
    sumx += this_val;
    sumx2 += this_val * this_val;
    if (min > this_val) {
      min = this_val;
    }
    if (max < this_val) {
      max = this_val;
    }

  } // i

  // compute mean

  double mean;

  if (_nScans > 0) {
    mean = sumx / _nScans;
  } else {
    mean = 0.0;
  } 

  // compute standard deviation

  double sdev = 0.0;

  if (_nScans >= 2) {
    double variance = sumx2 / n - pow((sumx / n), 2.0);
    if (variance < 0.0) {
      variance = 0.0;
    }
    sdev = sqrt(variance);
  }

  // set return vals
  
  *mean_p = mean;
  *min_p = min;
  *max_p = max;
  *sdev_p = sdev;
  
}

///////////////////////////////////////////////////
// _compute_serial_corr()
//
// Compute the serial correlation for a time_series variable
//
///////////////////////////////////////////////////

void Properties::_compute_serial_corr(double *tseries_val,
				      double *scorr_p)
  
{
  
  double sumx = 0.0;
  double sumy = 0.0;
  double sumx2 = 0.0;
  double sumy2 = 0.0;
  double sumxy = 0.0;
  double n = 0.0;

  // set first value in series
  
  double prev_val = *tseries_val;

  // loop through scan, accumulating

  for (int i = 1; i < _nScans; i++) {
    
    // get this value in time series
    
    double *tval =
      (double *) ((char *) tseries_val + sizeof(time_series_props_t) * i);
    double this_val = *tval;

    // increment counters for stats

    sumx += prev_val;
    sumx2 += prev_val * prev_val;

    sumy += this_val;
    sumy2 += this_val * this_val;

    sumxy += prev_val * this_val;

    n++;

    prev_val = this_val;

  } // i

  // compute correlation

  double num = n * sumxy - sumx * sumy;
  double denom1 = n * sumx2 - sumx * sumx;
  double denom2 = n * sumy2 - sumy * sumy;
  double denom_sq = denom1 * denom2;

  double scorr;
  if (n < 5) {
    scorr = -9999.0;
  } else if (denom_sq <= 0) {
    scorr = 0.0;
  } else {
    scorr = num / sqrt(denom_sq);
  }

  *scorr_p = scorr;

}

///////////////////////////////////////////////////
// _compute_tseries_integrals()
//
// Compute selected integrals from the time series
// data.
//
// Computes the ATI and precip_mass
//
///////////////////////////////////////////////////

void Properties::_compute_tseries_integrals()
  
{

  // time_since_start

  for (int i = 1; i < _nScans; i++) {
    _tSeriesProps[i].time_since_start =
      (_tSeriesProps[i].scan_time -
       _tSeriesProps[0].scan_time) / 3600.0;
  }

  // compute scan durations

  for (int i = 1; i < _nScans - 1; i++) {
    _tSeriesProps[i].scan_duration =
      (_tSeriesProps[i+1].scan_time -
       _tSeriesProps[i-1].scan_time) / 7200.0;
  }

  _tSeriesProps[0].scan_duration =
    (_tSeriesProps[1].scan_time -
     _tSeriesProps[0].scan_time) / 7200.0;

  _tSeriesProps[_nScans-1].scan_duration =
    (_tSeriesProps[_nScans-1].scan_time -
     _tSeriesProps[_nScans-2].scan_time) / 7200.0;

  // compute integrals

  double ati = 0.0;
  double precip_mass = 0.0;

  time_series_props_t *tseries = _tSeriesProps;

  for (int i = 0; i < _nScans; i++, tseries++) {

    ati +=
      tseries->area * tseries->scan_duration;

    precip_mass +=
      tseries->precip_flux * tseries->scan_duration * 3.6;

  } // i

  // store

  _ati = ati;
  _precipMass = precip_mass;

}

///////////////////////////////////////////////////
// _compute_area_fit()
//
// Compute Gaussian fit to area-time data.
//
// Computes the A and D parameters
//
///////////////////////////////////////////////////

void Properties::_compute_area_fit()
  
{

  double sum_ad = 0.0;
  double sum_adt = 0.0;
  double sum_adt2 = 0.0;

  time_series_props_t *tseries = _tSeriesProps;

  // accumulate data for the Gaussian fit

  for (int i = 0; i < _nScans; i++, tseries++) {

    sum_ad += tseries->area * tseries->scan_duration;
    
    sum_adt += (tseries->area * tseries->scan_duration *
		tseries->time_since_start);
    
    sum_adt2 += (tseries->area * tseries->scan_duration *
		 tseries->time_since_start * tseries->time_since_start);

  } // i

  // compute the fit
  
  _gaussD = sqrt((sum_adt2 - ((sum_adt * sum_adt) / sum_ad)) / sum_ad);
  _gaussAmean = sum_ad / _duration;

  return;

}


////////////////////////////////
// _compute_elliptical_dbz_fit()
//
// Computes elliptical fit to rdist dbZ data relative to
// the lower threshold.
//
// Sets max_dbz of the fit.

void Properties::
_compute_elliptical_dbz_fit(storm_file_params_t *sparams,
			    storm_file_global_props_t *gprops,
			    storm_file_dbz_hist_t *hist,
			    double *dbz_fit_p)
  
{

  int ii;
  int n_intervals = gprops->n_dbz_intervals;
  double max_dbz = 0, rel_dbz, density;
  double a, b, y;
  double fit_a;
  double sum_sq_error, min_sum_sq_error, error;
  double d_hist = sparams->dbz_hist_interval;
  storm_file_dbz_hist_t *h;
  
  /*
   * compute max dbz
   */
  
  h = hist;
  for (ii = 0; ii < n_intervals; ii++, h++) {
    if (h->percent_area != 0) {
      max_dbz = ((double) ii + 0.5) * d_hist;
    }
  }

  /*
   * trial and error fit for dbz on either side of max
   */

  min_sum_sq_error = 1.0e10;
  fit_a = max_dbz;
  
  /*
   * loop through range of parameters
   */
  
  for (a = max_dbz - 10; a < max_dbz + 10; a += 0.5) {

    b = 4.0 / (M_PI * a);
    sum_sq_error = 0.0;

    /*
     * sum up sq error for this fit
     */
    
    h = hist;
    for (ii = 0; ii < n_intervals; ii++, h++) {
      if (h->percent_area != 0) {
	rel_dbz = ((double) ii + 0.5) * d_hist;
	density = ((hist[ii].percent_area / 100.0) / d_hist);
	if (rel_dbz > a) {
	  sum_sq_error += density * density;
	} else {
	  y = (b / a) * sqrt(a * a - rel_dbz * rel_dbz);
	  error = y - density;
	  sum_sq_error += error * error;
	}
      }
    } /* ii */
    
    if (sum_sq_error < min_sum_sq_error) {
      min_sum_sq_error = sum_sq_error;
      fit_a = a;
    }

  } /* for (a ... */

  *dbz_fit_p = fit_a;

}

