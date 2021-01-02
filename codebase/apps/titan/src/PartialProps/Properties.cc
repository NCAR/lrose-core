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
// November 1997
//
//////////////////////////////////////////////////////////

#include "Properties.hh"
#include "TrackEntry.hh"
#include <toolsa/str.h>
#include <toolsa/file_io.h>
using namespace std;

//////////////
// Constructor

Properties::Properties (char *prog_name,
			int debug,
			SeedCaseTracks::CaseTrack *this_case,
			double altitude_threshold,
			char *output_dir,
			storm_file_handle_t *s_handle,
			track_file_handle_t *t_handle,
			rf_partial_track_t *ptrack)
  
{
  
  // set pointers & vals

  _progName = STRdup(prog_name);
  _debug = debug;
  _case = *this_case;
  _altitudeThreshold = altitude_threshold;
  _outputDir = STRdup(output_dir);
  _s_handle = s_handle;
  _t_handle = t_handle;
  _pTrack = ptrack;
  isOK = TRUE;

  // make sure output dir exists

  if (ta_makedir_recurse(_outputDir)) {
    fprintf(stderr, "ERROR - %s:Properties::Properties\n", _progName);
    fprintf(stderr, "Cannot make output dir\n");
    perror(_outputDir);
    isOK = FALSE;
    return;
  }

  // compute case output path

  char case_output_path[MAX_PATH_LEN];

  sprintf(case_output_path, "%s%s%s.%.3d",
	  _outputDir, PATH_DELIM, "global", _case.num);
  _caseOutputPath = STRdup(case_output_path);
  
  // open case output file

  if ((_caseOutFile = fopen(_caseOutputPath, "w")) == NULL) {
    fprintf(stderr, "ERROR - %s:Properties::Properties\n", _progName);
    fprintf(stderr, "Cannot open case output file\n");
    perror(_caseOutputPath);
    isOK = FALSE;
    return;
  }
  
  // compute tseries output path

  char tseries_output_path[MAX_PATH_LEN];

  sprintf(tseries_output_path, "%s%s%s.%.3d",
	  _outputDir, PATH_DELIM, "tseries", _case.num);
  _tSeriesOutputPath = STRdup(tseries_output_path);
  
  // open tSeries output file

  if ((_tSeriesOutFile = fopen(_tSeriesOutputPath, "w")) == NULL) {
    fprintf(stderr, "ERROR - %s:Properties::Properties\n", _progName);
    fprintf(stderr, "Cannot open time series output file\n");
    perror(_tSeriesOutputPath);
    fclose(_caseOutFile);
    isOK = FALSE;
    return;
  }
  
  // load up scan limits

  if (_load_scan_limits()) {
    isOK = FALSE;
    return;
  }

  // allocate time series properties array and
  // initialize
  
  _tSeriesProps = (time_series_props_t *)
    umalloc(_nScans * sizeof(time_series_props_t));

  time_series_props_t *tseries = _tSeriesProps;
  for (int i = 0; i < _nScans; i++, tseries++) {
    memset(tseries, 0, sizeof(time_series_props_t));
    tseries->base = 100.0;
  }
  
}

/////////////
// destructor

Properties::~Properties ()
  
{

  // free up memmory

  ufree(_tSeriesProps);
  STRfree(_progName);
  STRfree(_outputDir);
  STRfree(_caseOutputPath);
  STRfree(_tSeriesOutputPath);

  // close output files

  fclose(_caseOutFile);
  fclose(_tSeriesOutFile);

}

//////////////
// Compute
//
// Compute props
//

int Properties::compute ()
  
{

  // load up the time series props data for each scan

  if (_load_tseries_props()) {
    return (-1);
  }

  /*
   * compute stats for each data element in the time series
   */
  
  _compute_tseries_stats(&_tSeriesProps->dbz_max, 
			 &_tSeriesProps->scan_time,
			 &_mean.dbz_max,
			 &_max.dbz_max,
			 &_maxRoi.dbz_max);

  _compute_tseries_stats(&_tSeriesProps->precip_flux, 
			 &_tSeriesProps->scan_time,
			 &_mean.precip_flux,
			 &_max.precip_flux,
			 &_maxRoi.precip_flux);

  _compute_tseries_stats(&_tSeriesProps->volume, 
			 &_tSeriesProps->scan_time,
			 &_mean.volume,
			 &_max.volume,
			 &_maxRoi.volume);

  _compute_tseries_stats(&_tSeriesProps->volume_above_alt, 
			 &_tSeriesProps->scan_time,
			 &_mean.volume_above_alt,
			 &_max.volume_above_alt,
			 &_maxRoi.volume_above_alt);

  _compute_tseries_stats(&_tSeriesProps->mass, 
			 &_tSeriesProps->scan_time,
			 &_mean.mass,
			 &_max.mass,
			 &_maxRoi.mass);

  _compute_tseries_stats(&_tSeriesProps->mass_above_alt, 
			 &_tSeriesProps->scan_time,
			 &_mean.mass_above_alt,
			 &_max.mass_above_alt,
			 &_maxRoi.mass_above_alt);

  _compute_tseries_stats(&_tSeriesProps->area, 
			 &_tSeriesProps->scan_time,
			 &_mean.area,
			 &_max.area,
			 &_maxRoi.area);

  _compute_tseries_stats(&_tSeriesProps->ht_of_dbz_max, 
			 &_tSeriesProps->scan_time,
			 &_mean.ht_of_dbz_max,
			 &_max.ht_of_dbz_max,
			 &_maxRoi.ht_of_dbz_max);

  _compute_tseries_stats(&_tSeriesProps->refl_centroid_z, 
			 &_tSeriesProps->scan_time,
			 &_mean.refl_centroid_z,
			 &_max.refl_centroid_z,
			 &_maxRoi.refl_centroid_z);

  _compute_tseries_stats(&_tSeriesProps->ht_max_minus_centroid_z, 
			 &_tSeriesProps->scan_time,
			 &_mean.ht_max_minus_centroid_z,
			 &_max.ht_max_minus_centroid_z,
			 &_maxRoi.ht_max_minus_centroid_z);

  _compute_tseries_stats(&_tSeriesProps->top, 
			 &_tSeriesProps->scan_time,
			 &_mean.top,
			 &_max.top,
			 &_maxRoi.top);

  _compute_tseries_stats(&_tSeriesProps->base, 
			 &_tSeriesProps->scan_time,
			 &_mean.base,
			 &_max.base,
			 &_maxRoi.base);

  // compute integral quantities

  _compute_tseries_integrals();

  // compute durations

  time_t t_start =
    _tSeriesProps[0].scan_time - (_tSeriesProps[0].scan_duration / 2);

  time_t t_end =
    _tSeriesProps[_nScans-1].scan_time + (_tSeriesProps[_nScans-1].scan_duration / 2);

  _durationBeforeDecision = _case.ref_time - t_start;
  _durationAfterDecision = t_end - _case.ref_time;

  return (0);

}
    
//////////////
// Print()
//
// Print props to output file.
//

void Properties::print()
  
{

  date_time_t ref_time, start_time, end_time;

  start_time.unix_time = _startTime;
  uconvert_from_utime(&start_time);

  end_time.unix_time = _endTime;
  uconvert_from_utime(&end_time);

  // basic output

  fprintf(_caseOutFile, "num: %d\n", _case.num);

  fprintf(_caseOutFile, "seed_flag: %d\n", _case.seed_flag);
  fprintf(_caseOutFile, "num_flares: %d\n", _case.num_flares);
  ref_time.unix_time = _case.ref_time;
  uconvert_from_utime(&ref_time);
  fprintf(_caseOutFile, "ref_time: %d %d %d %d %d %d %d\n",
	  ref_time.year, ref_time.month, ref_time.day, 
	  ref_time.hour, ref_time.min, ref_time.sec,
	  (int) ref_time.unix_time);
  fprintf(_caseOutFile, "seed_duration: %d\n", _case.seed_duration);
  fprintf(_caseOutFile, "complex_track_num: %d\n", _case.complex_track_num);
  fprintf(_caseOutFile, "simple_track_num: %d\n", _case.simple_track_num);
  fprintf(_caseOutFile, "cloud_base: %g\n", _case.cloud_base);
  fprintf(_caseOutFile, "mixing_ratio: %g\n", _case.mixing_ratio);
  fprintf(_caseOutFile, "temp_ccl: %g\n", _case.temp_ccl);
  fprintf(_caseOutFile, "deltat_500mb: %g\n", _case.deltat_500mb);
  fprintf(_caseOutFile, "altitude_threshold: %g\n", _altitudeThreshold);

  fprintf(_caseOutFile, "start_time: %d %d %d %d %d %d %d\n",
	  start_time.year, start_time.month, start_time.day, 
	  start_time.hour, start_time.min, start_time.sec,
	  (int) start_time.unix_time);

  fprintf(_caseOutFile, "end_time: %d %d %d %d %d %d %d\n",
	  end_time.year, end_time.month, end_time.day, 
	  end_time.hour, end_time.min, end_time.sec,
	  (int) end_time.unix_time);

  fprintf(_caseOutFile, "start_scan: %d\n", _startScan);
  fprintf(_caseOutFile, "end_scan: %d\n", _endScan);
  fprintf(_caseOutFile, "nscans: %d\n", _nScans);

  fprintf(_caseOutFile, "duration_before_decision: %d\n",
	  _durationBeforeDecision);
  fprintf(_caseOutFile, "duration_after_decision: %d\n",
	  _durationAfterDecision);
  
  // stats output

  fprintf(_caseOutFile, "dbz_max_mean: %g\n", _mean.dbz_max);
  fprintf(_caseOutFile, "dbz_max_max: %g\n", _max.dbz_max);
  fprintf(_caseOutFile, "dbz_max_max_roi: %g\n", _maxRoi.dbz_max);

  fprintf(_caseOutFile, "precip_flux_mean: %g\n", _mean.precip_flux);
  fprintf(_caseOutFile, "precip_flux_max: %g\n", _max.precip_flux);
  fprintf(_caseOutFile, "precip_flux_max_roi: %g\n", _maxRoi.precip_flux);

  fprintf(_caseOutFile, "volume_mean: %g\n", _mean.volume);
  fprintf(_caseOutFile, "volume_max: %g\n", _max.volume);
  fprintf(_caseOutFile, "volume_max_roi: %g\n", _maxRoi.volume);

  fprintf(_caseOutFile, "volume_above_alt_mean: %g\n", _mean.volume_above_alt);
  fprintf(_caseOutFile, "volume_above_alt_max: %g\n", _max.volume_above_alt);
  fprintf(_caseOutFile, "volume_above_alt_max_roi: %g\n", _maxRoi.volume_above_alt);

  fprintf(_caseOutFile, "mass_mean: %g\n", _mean.mass);
  fprintf(_caseOutFile, "mass_max: %g\n", _max.mass);
  fprintf(_caseOutFile, "mass_max_roi: %g\n", _maxRoi.mass);

  fprintf(_caseOutFile, "mass_above_alt_mean: %g\n", _mean.mass_above_alt);
  fprintf(_caseOutFile, "mass_above_alt_max: %g\n", _max.mass_above_alt);
  fprintf(_caseOutFile, "mass_above_alt_max_roi: %g\n", _maxRoi.mass_above_alt);

  fprintf(_caseOutFile, "area_mean: %g\n", _mean.area);
  fprintf(_caseOutFile, "area_max: %g\n", _max.area);
  fprintf(_caseOutFile, "area_max_roi: %g\n", _maxRoi.area);

  fprintf(_caseOutFile, "ht_of_dbz_max_mean: %g\n", _mean.ht_of_dbz_max);
  fprintf(_caseOutFile, "ht_of_dbz_max_max: %g\n", _max.ht_of_dbz_max);
  fprintf(_caseOutFile, "ht_of_dbz_max_max_roi: %g\n", _maxRoi.ht_of_dbz_max);

  fprintf(_caseOutFile, "refl_centroid_z_mean: %g\n", _mean.refl_centroid_z);
  fprintf(_caseOutFile, "refl_centroid_z_max: %g\n", _max.refl_centroid_z);
  fprintf(_caseOutFile, "refl_centroid_z_max_roi: %g\n", _maxRoi.refl_centroid_z);

  fprintf(_caseOutFile, "ht_max_minus_centroid_z_mean: %g\n",
	  _mean.ht_max_minus_centroid_z);
  fprintf(_caseOutFile, "ht_max_minus_centroid_z_max: %g\n",
	  _max.ht_max_minus_centroid_z);
  fprintf(_caseOutFile, "ht_max_minus_centroid_z_max_roi: %g\n",
	  _maxRoi.ht_max_minus_centroid_z);

  fprintf(_caseOutFile, "top_mean: %g\n", _mean.top);
  fprintf(_caseOutFile, "top_max: %g\n", _max.top);
  fprintf(_caseOutFile, "top_max_roi: %g\n", _maxRoi.top);
  fprintf(_caseOutFile, "base_max_roi: %g\n", _maxRoi.base);

  // integrals
  
  fprintf(_caseOutFile, "ATI: %g\n", _tSeriesProps[_nScans - 1].ati);
  fprintf(_caseOutFile, "precip_mass: %g\n", _tSeriesProps[_nScans - 1].precip_mass);
  fprintf(_caseOutFile, "VCDI: %g\n", _tSeriesProps[_nScans - 1].vcdi);

  // time series output

  fprintf(_tSeriesOutFile, "nscans: %d\n", _nScans);

  fprintf(_tSeriesOutFile,
	  "delta_time n_parts dbz_max precip_flux volume mass "
	  "area ht_of_dbz_max refl_centroid_z ht_max_minus_centroid_z "
	  "base top volume_above_alt mass_above_alt "
	  "ATI precip_mass VCDI\n");

  time_series_props_t *tseries = _tSeriesProps;
  for (int i = 0; i < _nScans; i++, tseries++) {
    fprintf(_tSeriesOutFile, "%6d ", tseries->delta_time);
    fprintf(_tSeriesOutFile, "%3d ", tseries->n_parts);
    fprintf(_tSeriesOutFile, "%5.1f ", tseries->dbz_max);
    fprintf(_tSeriesOutFile, "%8.4f ", tseries->precip_flux);
    fprintf(_tSeriesOutFile, "%8.4f ", tseries->volume);
    fprintf(_tSeriesOutFile, "%8.4f ", tseries->mass);
    fprintf(_tSeriesOutFile, "%8.4f ", tseries->area);
    fprintf(_tSeriesOutFile, "%6.3f ", tseries->ht_of_dbz_max);
    fprintf(_tSeriesOutFile, "%6.3f ", tseries->refl_centroid_z);
    fprintf(_tSeriesOutFile, "%8.4f ", tseries->ht_max_minus_centroid_z);
    fprintf(_tSeriesOutFile, "%6.3f ", tseries->base);
    fprintf(_tSeriesOutFile, "%6.3f ", tseries->top);
    fprintf(_tSeriesOutFile, "%8.4f ", tseries->volume_above_alt);
    fprintf(_tSeriesOutFile, "%8.4f ", tseries->mass_above_alt);
    fprintf(_tSeriesOutFile, "%8.2f ", tseries->ati);
    fprintf(_tSeriesOutFile, "%8.2f ", tseries->precip_mass);
    fprintf(_tSeriesOutFile, "%8.4f ", tseries->vcdi);
    fprintf(_tSeriesOutFile, "\n");
  }

}
    
//////////////////////////////////////////
// _load_scan_limits()
//
// load up scan start and end times etc.
//
// Returns 0 on success, -1 on failure
//
///////////////////////////////////////////

int Properties::_load_scan_limits()

{

  _startScan = 1000000;
  _endScan = -1;
      
  TrackEntry tEntry(_progName, _debug,
 		    _case.complex_track_num,
 		    _t_handle, _pTrack);
  
  // loop through the entries, computing the limits
  
  track_file_entry_t *entry;
  
  while ((entry = tEntry.next()) != NULL) {

    if (entry->scan_num < _startScan) {
      _startScan = entry->scan_num;
      _startTime = entry->time;
    }
    
    if (entry->scan_num > _endScan) {
      _endScan = entry->scan_num;
      _endTime = entry->time;
    }
    
  } // while

  _nScans = _endScan - _startScan + 1;

  // return success if valid entries found

  if (_nScans >= 0) {
    return (0);
  } else {
    return (-1);
  }
  
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

  TrackEntry tEntry(_progName, _debug,
 		    _case.complex_track_num,
 		    _t_handle, _pTrack);
  
  // loop through the entries, computing the properties
  
  track_file_entry_t *entry;
  
  while ((entry = tEntry.next()) != NULL) {
    
    // read in storm props
    
    int scan_num = entry->scan_num;

    if (RfReadStormScan(_s_handle, scan_num,
			"Properties::_load_tseries_props")) {
      return (-1);
    }

    if (RfReadStormProps(_s_handle, entry->storm_num,
			 "Properties::_load_tseries_props")) {
      return (-1);
    }

    double volume_above_alt;
    double mass_above_alt;
    double base;
    double top;

    storm_file_global_props_t *gprops = _s_handle->gprops + entry->storm_num;

    _compute_z_props(gprops, &volume_above_alt, &mass_above_alt, &base, &top);

    time_series_props_t *tseries = _tSeriesProps + (scan_num - _startScan);

    tseries->scan_time = entry->time;
    tseries->delta_time = tseries->scan_time - _case.ref_time;
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

  } // while

  // compute derived quantities

  time_series_props_t *tseries = _tSeriesProps;
  for (int i = 0; i < _nScans; i++, tseries++) {
    tseries->refl_centroid_z /= tseries->mass;
    tseries->ht_max_minus_centroid_z =
      tseries->ht_of_dbz_max - tseries->refl_centroid_z;
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

  if (_altitudeThreshold > top) {

    volume = 0;
    mass = 0;
    
  } else {

    int threshold_layer;
    double threshold_fraction;
  
    if (_altitudeThreshold > base) {
      
      threshold_layer = base_layer + 
	(int) ((_altitudeThreshold - base_z) / delta_z + 0.5);
      double threshold_layer_z = min_z + threshold_layer * delta_z;
      threshold_fraction =
	(_altitudeThreshold - threshold_layer_z) / delta_z + 0.5;

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

  } // if (_altitudeThreshold > top) 

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
// Computes the mean, max and
// max rate of increase (roi) in rate per hour
//
///////////////////////////////////////////////////

void Properties::_compute_tseries_stats(double *tseries_val,
					time_t *scan_time,
					double *mean_p,
					double *max_p,
					double *max_roi_p)
  
{

  time_t prev_time = 0;
  time_t this_time;

  double max = -1.0e9;
  double max_roi = -1.0e9;
  double sum = 0.0;
  double prev_val = 0.0;
  double this_val;

  for (int i = 0; i < _nScans; i++) {

    this_val = *tseries_val;
    this_time = *scan_time;

    sum += this_val;
    if (max < this_val) {
      max = this_val;
    }

    if (i > 0) {
      double delta_time = this_time - prev_time;
      double delta_val = this_val - prev_val;
      double roi = (delta_val / delta_time) * 3600.0; // rate per hr
      if (max_roi < roi) {
	max_roi = roi;
      }
    }

    // store prev values

    prev_val = this_val;
    prev_time = this_time;

    // move ahead in the tseries array

    tseries_val = (double *) ((char *) tseries_val + sizeof(time_series_props_t));
    scan_time = (time_t *) ((char *) scan_time + sizeof(time_series_props_t));

  } // i

  double mean;

  if (_nScans > 0) {
    mean = sum / _nScans;
  } else {
    mean = 0.0;

  } 

  if (_nScans < 2) {
    max_roi = 0.0;
  }

  // set return vals

  *mean_p = mean;
  *max_p = max;
  *max_roi_p = max_roi;
  
}


///////////////////////////////////////////////////
// _compute_tseries_integrals()
//
// Compute selected integrals from the time series
// data.
//
// Computes the ATI, precip_mass and VCDI.
//
///////////////////////////////////////////////////

void Properties::_compute_tseries_integrals()
  
{

  // compute scan durations

  if (_nScans > 1) {

    for (int i = 1; i < _nScans - 1; i++) {
      _tSeriesProps[i].scan_duration =
        (_tSeriesProps[i+1].scan_time - _tSeriesProps[i-1].scan_time) / 2;
    }
    
    _tSeriesProps[0].scan_duration =
      _tSeriesProps[1].scan_time - _tSeriesProps[0].scan_time;
    
    _tSeriesProps[_nScans-1].scan_duration =
      _tSeriesProps[_nScans-1].scan_time - _tSeriesProps[_nScans-2].scan_time;

  } else {

    _tSeriesProps[0].scan_duration = 300;

  }

  // compute integrals

  double ati = 0.0;
  double precip_mass = 0.0;
  double vcdi = 0.0;

  time_series_props_t *tseries = _tSeriesProps;

  for (int i = 0; i < _nScans; i++, tseries++) {

    double t_start = tseries->scan_time - tseries->scan_duration / 2.0;
    double t_end = tseries->scan_time + tseries->scan_duration / 2.0;
    double fraction;

    if (t_start >= _case.ref_time) {
      fraction = 1.0;
    } else if (t_end < _case.ref_time) {
      fraction = 0.0;
    } else {
      fraction = (t_end - _case.ref_time) / (double) tseries->scan_duration;
    }

    ati +=
      tseries->area * tseries->scan_duration * fraction / 3600.0;
    precip_mass +=
      tseries->precip_flux * tseries->scan_duration * fraction / 1000.0;
    vcdi +=
      tseries->ht_max_minus_centroid_z * tseries->scan_duration * fraction / 3600.0;

    // set values in time series struct

    tseries->ati = ati;
    tseries->precip_mass = precip_mass;
    tseries->vcdi = vcdi;

  } // i

}


