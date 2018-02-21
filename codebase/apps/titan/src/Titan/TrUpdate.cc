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
///////////////////////////////////////////////////////////////
// TrUpdate.cc
//
// Update routines for StormTrack object
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1999
//
///////////////////////////////////////////////////////////////

#include "StormTrack.hh"
#include <rapmath/umath.h>
#include <toolsa/pjg.h>
using namespace std;

/***************************************************************************
 * _updateTracks()
 *
 * Update the entries and structures relevant to the storms at time 2.
 *
 * returns 0 on success, -1 on failure
 *
 ***************************************************************************/

int StormTrack::_updateTracks(date_time_t *dtime,
			      int scan_num,
			      double d_hours)

{

  /*
   * loop through all storms at time 2
   */

  for (size_t jstorm = 0; jstorm < _storms2.size(); jstorm++) {

    TrStorm &storm2 = *_storms2[jstorm];

    if (storm2.status.continues) {

      /*
       * this is a continuing track - copy the track information across
       */
      
      storm2.track = _storms1[storm2.status.match]->track;
      storm2.track.status.n_parents = storm2.status.n_match;
      
    } else if (storm2.status.starts) {

      /*
       * initialize a new track
       */

      storm2.track.init_new(_tfile, dtime, _trackUtime, scan_num,
			    true, -1, 0, scan_num, dtime,
			    _params.debug >= Params::DEBUG_VERBOSE);
      
    } else {

      /*
       * combined tracks
       */
      
      if (_handleCombined(dtime, scan_num, d_hours, storm2)) {
	return (-1);
      }
      
    } /* if (storm2.status.continues) */

  } /* jstorm */

  return (0);

}

/********************************************************************
 * _handleCombined()
 *
 * handle the case of a continuing tarck which has a merger or split
 *
 * Returns 0 on success, -1 on failure.
 */

int StormTrack::_handleCombined(date_time_t *dtime,
				int scan_num,
				double d_hours,
				TrStorm &storm2)
     
{

  int nchildren;
  int history_in_scans, nhist;
  int oldest_combining_storm = 0;
  int new_simple_track;
  int complex_track = 0;
  int scan_origin = 0;

  double storm1_wt, storm2_wt;
  double sum_storm2_wt;

  Point_d *pos_corr, *corr;

  date_time_t *time_origin = NULL;
  TrStorm::track_match_t *match;
  TrTrack::props_t *combined_history;
  TrTrack::props_t *contribution;

  /*
   * find complex track num and oldest scan in
   * contributing storms
   */
  
  history_in_scans = 0;
  
  match = storm2.match_array;
  for (int kstorm = 0; kstorm < storm2.status.n_match; kstorm++, match++) {
    
    TrStorm &storm1 = *_storms1[match->storm_num];
    
    if (history_in_scans < storm1.track.status.history_in_scans) {
      history_in_scans = storm1.track.status.history_in_scans;
      scan_origin = storm1.track.status.scan_origin;
      time_origin = &storm1.track.status.time_origin;
      oldest_combining_storm = match->storm_num;
    }

    /*
     * compute lowest track num
     */
    
    if (kstorm == 0) {
      complex_track = storm1.track.status.complex_track_num;
    } else {
      if (storm1.track.status.complex_track_num != complex_track) {
	fprintf(stderr, "\aERROR - %s:TrUpdate\n", _progName.c_str());
	fprintf(stderr, "_handleCombined() failed.\n");
	fprintf(stderr,
		"Complex nums %d and %d should have been consolidated.\n",
		complex_track,
		storm1.track.status.complex_track_num);
	return (-1);
      }
    }
    
  } /* kstorm */
  
  /*
   * read in complex track params for the lowest complex track number
   */

  if (_tfile.ReadComplexParams(complex_track, false)) {
    cerr << "ERROR - " << _progName << "StormTrack::_handleCombined" << endl;
    cerr << _tfile.getErrStr() << endl;
    return -1;
  }

  /*
   * allocate simple track
   */
  
  if (storm2.track.init_new(_tfile, dtime, _trackUtime, scan_num,
			    false, complex_track, history_in_scans,
			    scan_origin, time_origin,
			    _params.debug >= Params::DEBUG_VERBOSE)) {
    fprintf(stderr, "\aERROR - %s:TrUpdate\n", _progName.c_str());
    fprintf(stderr, "_handleCombined() failed.\n");
    fprintf(stderr, "Cannot init_new()\n");
    fprintf(stderr, "Complex track: %d\n", complex_track);
    return (-1);
  }

  new_simple_track = storm2.track.status.simple_track_num;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "\ncombined simple track number %d\n",
	    (int) new_simple_track);
  }
  
  /*
   * amend the parents params to include this child
   */
  
  match = storm2.match_array;
  for (int kstorm = 0; kstorm < storm2.status.n_match; kstorm++, match++) {
    
    TrStorm &storm1 = *_storms1[match->storm_num];

    if (_tfile.ReadSimpleParams(storm1.track.status.simple_track_num)) {
      cerr << "ERROR - " << _progName << "StormTrack::_handleCombined" << endl;
      cerr << _tfile.getErrStr() << endl;
      return -1;
    }

    nchildren = _tfile._simple_params.nchildren + 1;
    _tfile._simple_params.nchildren = nchildren;
    _tfile._simple_params.child[nchildren - 1] = new_simple_track;
    
    if (_tfile.WriteSimpleParams(storm1.track.status.simple_track_num)) {
      cerr << "ERROR - " << _progName << "StormTrack::_handleCombined" << endl;
      cerr << _tfile.getErrStr() << endl;
      return -1;
    }

  } /* kstorm */
  
  /*
   * amend the new simple track params to include its parents
   */

  if (_tfile.ReadSimpleParams(new_simple_track)) {
    cerr << "ERROR - " << _progName << "StormTrack::_handleCombined" << endl;
    cerr << _tfile.getErrStr() << endl;
    return -1;
  }

  _tfile._simple_params.nparents = storm2.status.n_match;
  
  match = storm2.match_array;
  for (int kstorm = 0; kstorm < storm2.status.n_match; kstorm++, match++) {
    
    TrStorm &storm1 = *_storms1[match->storm_num];
    
    _tfile._simple_params.parent[kstorm] =
      storm1.track.status.simple_track_num;
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "parent of new track %d\n",
	      storm1.track.status.simple_track_num);
    }
    
  } /* kstorm */
  
  if (_tfile.WriteSimpleParams(new_simple_track)) {
    cerr << "ERROR - " << _progName << "StormTrack::_handleCombined" << endl;
    cerr << _tfile.getErrStr() << endl;
    return -1;
  }

  /*
   * set pointer to current value struct
   */
  
  // TrTrack::props_t *current = &storm2.current;
  
  /*
   * for each combining storm, compute the correction between the
   * forecast position and the combined centroid
   */

  pos_corr = (Point_d *) umalloc (storm2.status.n_match * sizeof(Point_d));
  
  if (storm2.status.has_split && !storm2.status.has_merger) {

    _getCorrectionForSplit(storm2, pos_corr);

  } else if (!storm2.status.has_split && storm2.status.has_merger) {

    _getCorrectionForMerger(storm2, pos_corr);

  } else {

    _getCorrectionForCombination(storm2, pos_corr, d_hours);
    
  }

  /*
   * load up history data
   */
  
  if (history_in_scans > _params.tracking_forecast_weights_n)
    nhist = _params.tracking_forecast_weights_n;
  else
    nhist = history_in_scans;

  combined_history = storm2.track.status.history;
  for (int ihist = 0; ihist < nhist; ihist++, combined_history++) {

    /*
     * get time from oldest storm entry
     */
    
    combined_history->time =
      _storms1[oldest_combining_storm]->track.status.history[ihist].time;
    
    match = storm2.match_array;
    corr = pos_corr;
    sum_storm2_wt = 0.0;
    for (int kstorm = 0; kstorm < storm2.status.n_match;
	 kstorm++, match++, corr++) {
      
      TrStorm &storm1 = *_storms1[match->storm_num];

      if (ihist < storm1.track.status.history_in_scans) {
	
	storm1_wt = match->overlap / storm1.status.sum_overlap;
	storm2_wt = match->overlap / storm2.status.sum_overlap;
	sum_storm2_wt += storm2_wt;
	
	contribution = storm1.track.status.history + ihist;
	
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

	combined_history->dbz_mean =
	  (combined_history->dbz_mean + contribution->dbz_mean) / 2.0;

	/*
	 * For size, the sum is weighted by the area
	 * contribution of the time1 storms.
	 */
	
	combined_history->volume += storm1_wt * contribution->volume;
	combined_history->precip_flux += storm1_wt * contribution->precip_flux;
	combined_history->mass += storm1_wt * contribution->mass;
	combined_history->proj_area += storm1_wt * contribution->proj_area;

      } /* if (ihist < storm1.track.status.history_in_scans) */
	
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
  
  ufree(pos_corr);

  return (0);

}

/*********************************************************
 * get_correction_for_combination()
 *
 * merger and split - use forecast (x, y) to compute
 * correction
 */

void StormTrack::_getCorrectionForCombination(TrStorm &storm2,
					      Point_d *pos_corr,
					      double d_hours)

{

  double val, dval_dt;
  Point_d pos_forecast;
  Point_d *corr;
  TrStorm::track_match_t *match;
  
  match = storm2.match_array;
  corr = pos_corr;
  
  for (int kstorm = 0; kstorm < storm2.status.n_match;
       kstorm++, match++, corr++) {
    
    TrStorm &storm1 = *_storms1[match->storm_num];
    
    val = storm1.track.status.history->proj_area_centroid_x;
    dval_dt = storm1.track.status.dval_dt.proj_area_centroid_x;
    pos_forecast.x = val + dval_dt * d_hours;
    corr->x = storm2.current.proj_area_centroid_x - pos_forecast.x;
    
    val = storm1.track.status.history->proj_area_centroid_y;
    dval_dt = storm1.track.status.dval_dt.proj_area_centroid_y;
    pos_forecast.y = val + dval_dt * d_hours;
    corr->y = storm2.current.proj_area_centroid_y - pos_forecast.y;
    
  }

  return;

}

/*********************************************************
 * get_correction_for_merger()
 *
 * merger only - use merge centroid for correction
 */

void StormTrack::_getCorrectionForMerger(TrStorm &storm2,
					 Point_d *pos_corr)

{

  double sum_x = 0.0;
  double sum_y = 0.0;
  double sum_area = 0.0;
  double proj_area;

  Point_d centroid;
  Point_d *corr;
  
  TrStorm::track_match_t *match;

  /*
   * compute merge centroid
   */

  match = storm2.match_array;
  for (int kstorm = 0; kstorm < storm2.status.n_match; kstorm++, match++) {
    TrStorm &storm1 = *_storms1[match->storm_num];
    proj_area = storm1.current.proj_area;
    sum_area += proj_area;
    sum_x += storm1.current.proj_area_centroid_x * proj_area;
    sum_y += storm1.current.proj_area_centroid_y * proj_area;
  }
  
  centroid.x = sum_x / sum_area;
  centroid.y = sum_y / sum_area;

  /*
   * compute correction
   */

  match = storm2.match_array;
  corr = pos_corr;
  for (int kstorm = 0; kstorm < storm2.status.n_match;
       kstorm++, match++, corr++) {
    TrStorm &storm1 = *_storms1[match->storm_num];
    corr->x = centroid.x - storm1.current.proj_area_centroid_x;
    corr->y = centroid.y - storm1.current.proj_area_centroid_y;
  } /* kstorm */

  return;

}

/*********************************************************
 * get_correction_for_split()
 *
 * split only - use split centroid to compute correction
 */

void StormTrack::_getCorrectionForSplit(TrStorm &storm2,
					Point_d *pos_corr)

{

  double sum_x = 0.0;
  double sum_y = 0.0;
  double sum_area = 0.0;
  double proj_area;

  Point_d centroid;
  TrStorm::track_match_t *match;
  
  TrStorm &storm1 = *_storms1[storm2.match_array[0].storm_num];
  
  match = storm1.match_array;
  for (int i = 0; i < storm1.status.n_match; i++, match++) {
    TrStorm &split = *_storms2[match->storm_num];
    proj_area = split.current.proj_area;
    sum_area += proj_area;
    sum_x += split.current.proj_area_centroid_x * proj_area;
    sum_y += split.current.proj_area_centroid_y * proj_area;
  }
  
  centroid.x = sum_x / sum_area;
  centroid.y = sum_y / sum_area;
      
  pos_corr->x = storm2.current.proj_area_centroid_x - centroid.x;
  pos_corr->y = storm2.current.proj_area_centroid_y - centroid.y;

  return;

}
