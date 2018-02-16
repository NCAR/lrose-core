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
// TrMatch.cc
//
// Matching routines for StormTrack object
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

/*********************************************************************
 * match_storms()
 *
 * match storms from time1 to time2
 *
 *********************************************************************/

#define MAX_ELT 100000
#define KM_PER_DEG_AT_EQUATOR 111.12

void StormTrack::_matchStorms(double d_hours)

{
  
  int transpose;
  int grid_type;

  int nvalid_edges = 0;
  long **icost;
  long *match;
  int dim1, dim2;
  int max_storms;
  
  double *xx1, *yy1, *xx2, *yy2;
  double distance, dx_km, dy_km;
  double x_km_scale, y_km_scale;
  double mean_lat, cos_lat;
  double delta_cube_root_volume;
  double speed;
  double cost_scale;
  double cost, max_cost;
  double **dcost;

  /*
   * set local vars
   */

  grid_type = _sfile.scan().grid.proj_type;

  /*
   * set dimensions of cost array. Note that the matrix must
   * be transposed if _storms1.size() exceeds _storms2.size(),
   * because dim1 must be less than or equal to dim2
   */

  if (_storms1.size() <= _storms2.size()) {
    dim1 = _storms1.size();
    dim2 = _storms2.size();
    transpose = FALSE;
    max_storms = _storms2.size();
  } else {
    dim1 = _storms2.size();
    dim2 = _storms1.size();
    transpose = TRUE;
    max_storms = _storms1.size();
  }

  /*
   * allocate mem for cost arrays and match arrray
   */

  dcost = (double **) ucalloc2
    (_storms1.size(), _storms2.size(), sizeof(double));

  icost = (long **) ucalloc2
    (dim1, dim2, sizeof(long));

  match = (long *) ucalloc
    ((_storms1.size() + _storms2.size()), sizeof(long));

  xx1 = (double *) umalloc
    ((_storms1.size() * sizeof(double)));

  yy1 = (double *) umalloc
    ((_storms1.size() * sizeof(double)));

  xx2 = (double *) umalloc
    ((_storms2.size() * sizeof(double)));

  yy2 = (double *) umalloc
    ((_storms2.size() * sizeof(double)));
  
  /*
   * load up storm coordinates
   */

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "Storms at time 1:\n");
  }

  const storm_file_global_props_t *gprops = _sfile.gprops();

  for (size_t i = 0; i < _storms1.size(); i++) {
    
    xx1[i] = _storms1[i]->current.proj_area_centroid_x;
    yy1[i] = _storms1[i]->current.proj_area_centroid_y;
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr,
	      "i, x1, y1 = %4d, %10g, %10g\n", (int) i, xx1[i], yy1[i]);
    }
    
  } /* i */

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "Storms at time 2:\n");
  }

  for (size_t j = 0; j < _storms2.size(); j++) {
    
    xx2[j] = gprops[j].proj_area_centroid_x;
    yy2[j] = gprops[j].proj_area_centroid_y;
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr,
	      "j, x2, y2 = %4d, %10g, %10g\n", (int) j, xx2[j], yy2[j]);
    }
    
  } /* j */
  
  /*
   * load up the dcost array
   */

  max_cost = 0.0;

  for (size_t i = 0; i < _storms1.size(); i++) {

    for (size_t j = 0; j < _storms2.size(); j++) {

      if (_storms1[i]->status.n_match > 0 ||
	  _storms2[j]->status.n_match > 0) {
	
	/*
	 * already matched, so set edge invalid
	 */

	dcost[i][j] = -1.0;

	if (_params.debug >= Params::DEBUG_EXTRA) {
	  fprintf(stderr, "Already matched using overlaps\n");
	  fprintf(stderr, "Storm i - %d to storm j - %d\n", (int) i, (int) j);
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
	  fabs(pow((double) gprops[j].volume, 0.33333333) -
	       pow((double) _storms1[i]->current.volume, 0.33333333));
	
	if (speed <= _params.tracking_max_speed &&
	    _matchFeasible(*_storms1[i], *_storms2[j],
			    d_hours, grid_type)) {
	  
	  /*
	   * edge is valid
	   */
	  
	  cost = 
	    (distance * _params.tracking_weight_distance +
	     delta_cube_root_volume *
	     _params.tracking_weight_delta_cube_root_volume);
	  
	  if (max_cost < cost)
	    max_cost = cost;
	  
	  dcost[i][j] = cost;
	  
	  nvalid_edges++;
	  
	} else {
	  
	  /*
	   * edge is not valid
	   */
	  
	  dcost[i][j] = -1.0;
	  
	} /* if (speed <= _params.tracking_max_speed ... */

	if (_params.debug >= Params::DEBUG_EXTRA) {
	  fprintf(stderr, "Storm i - %d to storm j - %d\n", (int) i, (int) j);
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

      } /* if (storms1[i]->n_match > 0  ... */
	
    } /* j */

  } /* i */

  if (nvalid_edges > 0) {

    /*
     * compute cost scale factor
     */

    cost_scale = (double) MAX_ELT / (max_cost * (double) (max_storms + 1));

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "max_cost = %g\n", max_cost);
      fprintf(stderr, "cost_scale = %g\n", cost_scale);
    }

    /*
     * load up integer cost matrix, transposing as necessary to fulfil the
     * requirement that the first dimension must not exceed the second.
     * Also, the entries are subtracted from MAX_ELT so that the problem
     * may be treated as one of maximization rather than minimization.
     */
    
    for (size_t i = 0; i < _storms1.size(); i++) {

      size_t k = 0, l = 0;

      if (transpose == FALSE)
	k = i;
      else
	l = i;
      
      for (size_t j = 0; j < _storms2.size(); j++) {

	if (transpose == FALSE)
	  l = j;
	else
	  k = j;
	
	cost = dcost[i][j];
	
	if (cost < 0) {
	  icost[k][l] = MAX_ELT / 2;
	} else {
	  icost[k][l] = MAX_ELT - (int) (cost * cost_scale + 0.5);
	}

      } /* j */
      
    } /* i */

    /*
     * print out icost matrix
     */
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      
      fprintf(stderr, "\nICOST MATRIX\n\n");
      
      if (transpose)
	fprintf(stderr, "j\\i ");
      else
	fprintf(stderr, "i\\j ");
      
      for (int j = 0; j < dim2; j++) {
	fprintf(stderr, " %6d", j);
      }
      fprintf(stderr, "\n");
      
      for (int i = 0; i < dim1; i++) {
	fprintf(stderr, "%4d", i);
	for (int j = 0; j < dim2; j++) {
	  fprintf(stderr, " %6ld", icost[i][j]);
	}
	fprintf(stderr, "\n");
      } /* i */
      
    } /* if (_params.debug ... ) */
    
    /*
     * get the bipartite match
     */
    
    umax_wt_bip(icost, dim1, dim2, (int) MAX_ELT / 2, match);
    
    /*
     * load local match arrays. match1 contains the storm numbers at time2
     * which correspond to storms at time 1, and vice versa for match2.
     */

    if (transpose) {

      if (_params.debug >= Params::DEBUG_VERBOSE)
	fprintf(stderr, "\nCost matrix transposed.\n\n");

      for (size_t i = 0; i < _storms2.size(); i++) {

	if (match[i] >= 0) {

	  _storms2[i]->status.match = match[i];
	  _storms1[match[i]]->status.match = i;

	} /* if (match[i] >= 0) */

      } /* i */

    } else {

      for (size_t i = 0; i < _storms1.size(); i++) {

	if (match[i] >= 0) {

	  _storms1[i]->status.match = match[i];
	  _storms2[match[i]]->status.match = i;

	} /* if (match[i] >= 0) */

      } /* i */

    } /* if (transpose) */

    /*
     * print out the match
     */

    if (_params.debug >= Params::DEBUG_EXTRA) {
      
      fprintf(stderr, "Match array\n");
      
      for (size_t i = 0; i < _storms1.size() + _storms2.size(); i++)
	fprintf(stderr, "i = %d, match = %ld\n", (int) i, match[i]);
      
      fprintf(stderr, "\n");

      fprintf(stderr, "Matching 1 to 2\n");

      for (size_t i = 0; i < _storms1.size(); i++)
	fprintf(stderr, "i = %d, match1 = %d\n", (int) i,
		(int) _storms1[i]->status.match);
      
      fprintf(stderr, "\n");

      fprintf(stderr, "Matching 2 to 1:\n");
      
      for (size_t j = 0; j < _storms2.size(); j++)
	fprintf(stderr, "j = %d, match2 = %d\n", (int) j,
		(int) _storms2[j]->status.match);
      
      fprintf(stderr, "\n");

    } /* if (_params.debug ... */

  } else {

    if (_params.debug >= Params::DEBUG_EXTRA) {
      fprintf(stderr, "No valid tracks for match_storms()\n");
    }
    
  } /* if (nvalid_edges > 0) */

  /*
   * free up arrays
   */

  ufree2((void **) dcost);
  ufree2((void **) icost);
  ufree(match);
  ufree(xx1);
  ufree(yy1);
  ufree(xx2);
  ufree(yy2);

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

bool StormTrack::_matchFeasible(TrStorm &storm1, TrStorm &storm2,
				 double d_hours, int grid_type)

{

  double xx1, yy1;
  double xx2, yy2;
  double xxf, yyf;
  double dx, dy;
  double forecast_dist;
  double error_dist;
    
  /*
   * matches with young storms are always feasible
   * provided field tracker is not used
   */
  
  if (storm1.track.status.history_in_scans < 5 &&
      _params.override_early_storm_motion_using_field_tracker == false) {
    return (true);
  }

  xx1 = storm1.current.proj_area_centroid_x;
  yy1 = storm1.current.proj_area_centroid_y;
  xx2 = storm2.current.proj_area_centroid_x;
  yy2 = storm2.current.proj_area_centroid_y;

  /*
   * compute forecast position
   */

  xxf = xx1 + storm1.track.status.dval_dt.proj_area_centroid_x * d_hours;
  yyf = yy1 + storm1.track.status.dval_dt.proj_area_centroid_y * d_hours;

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

  if (error_dist >= forecast_dist) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "=========================>>>>\a\n");
      fprintf(stderr, "Track %d/%d, invalid match\n",
	      storm1.track.status.complex_track_num,
	      storm1.track.status.simple_track_num);
      fprintf(stderr, "Storm location (x,y) = (%g,%g)\n",
	      xx1, yy1);
      fprintf(stderr, "Forecast point (x,y) = (%g,%g)\n",
	      xxf, yyf);
      fprintf(stderr, "Proposed match at (x,y) = (%g,%g)\n",
	      xx2, yy2);
    }
    return (false);
  }

  return (true);
  
}

/**************************************************************
 * resolve_matches.c
 *
 * Matches are derived in two ways:
 *
 *  1. find_overlap() matches storms based on overlap
 *     between current shape and the previous forecast shape.
 *  2. For those storms which were not matched in step 1,
 *     match_storms() finds matches between storms
 *     based on distance apart, similar attributes etc.
 *
 * This routine adds the step 2 matches with those from step 1.
 *
 */

void StormTrack::_resolveMatches()
     
{

  for (size_t istorm = 0; istorm < _storms1.size(); istorm++) {
    TrStorm &storm1 = *_storms1[istorm];
    storm1.status.starts = FALSE;
    storm1.status.stops = FALSE;
    storm1.status.continues = FALSE;
    storm1.status.has_split = FALSE;
    storm1.status.has_merger = FALSE;
  }
    
  for (size_t jstorm = 0; jstorm < _storms2.size(); jstorm++) {
    TrStorm &storm2 = *_storms2[jstorm];
    storm2.status.starts = FALSE;
    storm2.status.stops = FALSE;
    storm2.status.continues = FALSE;
    storm2.status.has_split = FALSE;
    storm2.status.has_merger = FALSE;
  }
    
  for (size_t jstorm = 0; jstorm < _storms2.size(); jstorm++) {
    TrStorm &storm2 = *_storms2[jstorm];
    
    if (storm2.status.match >= 0) {
      
      storm2.status.continues = TRUE;

    } else if (storm2.status.n_match == 0) {

      storm2.status.starts = TRUE;

    } else if (storm2.status.n_match == 1 &&
	       _storms1[storm2.match_array[0].storm_num]->status.n_match == 1) {

      storm2.status.continues = TRUE;
      storm2.status.match = storm2.match_array[0].storm_num;

    } else {

      if (storm2.status.n_match > 1) {
	storm2.status.has_merger = TRUE;
      }

      TrStorm::track_match_t *match2 = storm2.match_array;
      for (int j = 0; j < storm2.status.n_match; j++, match2++) {
	TrStorm &storm1 = *_storms1[match2->storm_num];
	if (storm1.status.n_match > 1) {
	  storm2.status.has_split = TRUE;
	  TrStorm::track_match_t *match1 = storm1.match_array;
	  for (int i = 0; i < storm1.status.n_match; i++, match1++) {
	    if (_storms2[match1->storm_num]->status.n_match > 1) {
	      storm2.status.has_merger = TRUE;
	    }
	  } /* i */
	}
      } /* j */

    } /* if (storm2.status.match > 0) */

  } /* jstorm */

  for (size_t istorm = 0; istorm < _storms1.size(); istorm++) {

    TrStorm &storm1 = *_storms1[istorm];
    
    if (storm1.status.match >= 0) {

      storm1.status.continues = TRUE;

    } else if (storm1.status.n_match == 0) {

      storm1.status.stops = TRUE;

    } else if (storm1.status.n_match == 1 &&
	       _storms2[storm1.match_array[0].storm_num]->status.n_match == 1) {
      
      storm1.status.continues = TRUE;
      storm1.status.match = storm1.match_array[0].storm_num;

    } else {

      if (storm1.status.n_match > 1) {
	storm1.status.has_split = TRUE;
      }

      TrStorm::track_match_t *match1 = storm1.match_array;
      for (int i = 0; i < storm1.status.n_match; i++, match1++) {
	TrStorm &storm2 = *_storms2[match1->storm_num];
	if (storm2.status.n_match > 1) {
	  storm1.status.has_merger = TRUE;
	  TrStorm::track_match_t *match2 = storm2.match_array;
	  for (int j = 0; j < storm2.status.n_match; j++, match2++) {
	    if (_storms1[match2->storm_num]->status.n_match > 1) {
	      storm1.status.has_split = TRUE;
	    }
	  } /* i */
	}
      } /* j */

      for (int i = 0; i < storm1.status.n_match; i++) {
	if (_storms2[storm1.match_array[i].storm_num]->status.n_match > 1) {
	  storm1.status.has_merger = TRUE;
	}
      } /* i */

    } /* if (storm1.status.match > 0) */

  } /* istorm */

  return;

}

/****************************************************************************
 * _checkMatches()
 *
 * Check the matches to make sure the max number of parents and
 * children is not exceeded.
 *
 * In cases in which the max is exceeded, the storms with the smallest
 * overlaps are deleted from the match array.
 *
 ****************************************************************************/

void StormTrack::_checkMatches()
     
{

  /*
   * check on MAX_CHILDREN limit for storms1 entries
   */

  for (size_t istorm = 0; istorm < _storms1.size(); istorm++) {
    
    TrStorm &storm = *_storms1[istorm];

    while (storm.status.n_match > MAX_CHILDREN) {

      if (_params.debug >= Params::DEBUG_VERBOSE) {
	fprintf(stderr,
		"MAX_CHILDREN limit - deleting smallest from storm1 %d\n",
		(int) istorm);
      }

      _deleteSmallestOverlap(istorm, _storms1, _storms2);
      
    } /* while */

  } /* istorm */

  /*
   * check on MAX_PARENTS limit for storms2 entries
   */

  for (size_t jstorm = 0; jstorm < _storms2.size(); jstorm++) {
    
    TrStorm &storm = *_storms2[jstorm];

    while (storm.status.n_match > MAX_PARENTS) {

      if (_params.debug >= Params::DEBUG_VERBOSE) {
	fprintf(stderr,
		"MAX_PARENTS limit - deleting smallest from storm2 %d\n",
		(int) jstorm);
      }

      _deleteSmallestOverlap(jstorm, _storms2, _storms1);
      
    } /* while */

  } /* jstorm */

 return;

}

/***********************************************************************
 * _deleteSmallestOverlap()
 *
 * Delete match with smallest overlap. This is done if MAX_CHILDREN or
 * MAX_PARENTS is exceeded.
 */

void StormTrack::_deleteSmallestOverlap(int this_storm_num,
					  vector<TrStorm*> &these_storms,
					  vector<TrStorm*> &other_storms)
				    
{

  int match_num = 0;
  double min_overlap = 1.0e99;
  TrStorm::track_match_t *match;

  TrStorm &this_storm = *these_storms[this_storm_num];
  
  /*
   * delete smallest overlap from this storm's matches
   */

  match = this_storm.match_array;

  for (int i = 0; i < this_storm.status.n_match; i++, match++) {
    if (match->overlap < min_overlap) {
      min_overlap = match->overlap;
      match_num = match->storm_num;
    }
  } /* i */

  _deleteMatch(this_storm, match_num);

  /*
   * delete the matching entry in the other storms array
   */

  TrStorm &match_storm = *other_storms[match_num];
  _deleteMatch(match_storm, this_storm_num);

}

/************************************************************
 * _deleteMatch()
 *
 * Delete match from match array
 */

void StormTrack::_deleteMatch(TrStorm &storm, int storm_num)

{

  int complex_track_num = 0;
  TrStorm::track_match_t *match, *match_1;

  /*
   * find the index of the match to be removed
   */
  
  int  index = -1;
  match = storm.match_array;
  for (int i = 0; i < storm.status.n_match; i++, match++) {
    if (match->storm_num == storm_num) {
      index = i;
      complex_track_num = match->complex_track_num;
      break;
    }
  } /* i */

  if (index < 0) {
    fprintf(stderr, "ERROR - %s:check_matches\n", _progName.c_str());
    fprintf(stderr, "Cannot find storm_num %d to remove\n",
	    storm_num);
    return;
  }

  /*
   * Check if complex num for index is used by a different match.
   * If not, subtract from sum_simple_tracks.
   */

  if (storm.status.sum_simple_tracks > 0) {
    bool complex_used_elsewhere = false;
    match = storm.match_array;
    for (int i = 0; i < storm.status.n_match; i++, match++) {
      if (i != index) {
	if (match->complex_track_num == complex_track_num) {
	  complex_used_elsewhere = true;
	  break;
	} 
      }
    }
    if (!complex_used_elsewhere) {
      storm.status.sum_simple_tracks -= match->n_simple_tracks;
    }
  }

  /*
   * subtract from sum_overlap
   */
  
  storm.status.sum_overlap -= match->overlap;

  /*
   * move all of the entries above the index down by 1
   */

  match = storm.match_array + index;
  match_1 = match + 1;
  
  for (int i = index; i < storm.status.n_match - 1; i++, match++, match_1++) {
    *match = *match_1;
  } /* i */

  /*
   * decrement the number of matches
   */

  storm.status.n_match--;

  return;

}

/*****************
 * print_matches()
 */

void StormTrack::_printMatches()

{

  TrStorm::track_match_t *match;

  fprintf(stderr, "\n*** MATCHES/OVERLAPS - STORM1 ARRAY ***\n\n");

  for (size_t i = 0; i < _storms1.size(); i++) {

    TrStorm &storm1 = *_storms1[i];

    if (storm1.status.match >= 0) {
      
      _printStatus(storm1);
      fprintf(stderr, "Storm1 %4d matches storm2 %4d, "
	      "n_simple %4d, complex_num %4d\n",
	      (int) i, (int) storm1.status.match,
	      storm1.track.status.n_simple_tracks,
	      storm1.track.status.complex_track_num);
      
    } else if (storm1.status.sum_overlap > 0) {

      _printStatus(storm1);
      fprintf(stderr,
	      "Storm1 %4d,     sum_overlap %10g,"
	      " n_simple   %4d, complex_num %4d\n",
	      (int) i, storm1.status.sum_overlap,
	      (int) storm1.track.status.n_simple_tracks,
	      (int) storm1.track.status.complex_track_num);

      match = storm1.match_array;
      
      for (int j = 0; j < storm1.status.n_match; j++, match++) {
	fprintf(stderr, "  overlaps storm2 %4d, area %10g\n",
		(int) match->storm_num,
		match->overlap);
      } /* j */

    } else {

      _printStatus(storm1);
      fprintf(stderr, "Storm1 %4d not matched\n", (int) i);

    }

    fprintf(stderr, "\n");

  } /* i */

  fprintf(stderr, "\n*** MATCHES/OVERLAPS - STORM2 ARRAY ***\n\n");

  for (size_t i = 0; i < _storms2.size(); i++) {

    TrStorm &storm2 = *_storms2[i];

    if (storm2.status.match >= 0) {

      _printStatus(storm2);
      fprintf(stderr, "Storm2 %4d matches storm1 %4d, "
	      "n_simple %4d, complex_num %4d\n",
	      (int) i, (int) storm2.status.match,
	      _storms1[storm2.status.match]->track.status.n_simple_tracks,
	      _storms1[storm2.status.match]->track.status.complex_track_num);
      
    } else if (storm2.status.sum_overlap > 0) {

      _printStatus(storm2);
      fprintf(stderr,
	      "Storm2 %4d,     sum_overlap %10g, sum_simple %4d\n",
	      (int) i, storm2.status.sum_overlap,
	      (int) storm2.status.sum_simple_tracks);

      match = storm2.match_array;

      for (int j = 0; j < storm2.status.n_match; j++, match++) {
	fprintf(stderr, "  overlaps storm1 %4d, area %10g,"
		" n_simple   %4d, complex_num %4d\n",
		(int) match->storm_num,
		match->overlap,
		(int) match->n_simple_tracks,
		(int) match->complex_track_num);
      } /* j */

    } else {

      _printStatus(storm2);
      fprintf(stderr, "Storm2 %4d not matched\n", (int) i);
      
    }

    fprintf(stderr, "\n");

  } /* i */

  return;

}

/****************
 * print_status()
 */

void StormTrack::_printStatus(TrStorm &storm)

{

  if (storm.status.starts) {
    fprintf(stderr, "STARTS\n");
  }
  if (storm.status.stops) {
    fprintf(stderr, "STOPS\n");
  }
  if (storm.status.continues) {
    fprintf(stderr, "IS_CONT\n");
  }
  if (storm.status.has_split) {
    fprintf(stderr, "IS_SPLIT\n");
  }
  if (storm.status.has_merger) {
    fprintf(stderr, "IS_MERGER\n");
  }
  if (storm.status.has_merger && storm.status.has_split) {
    fprintf(stderr, "\a");
  }
  return;

}



