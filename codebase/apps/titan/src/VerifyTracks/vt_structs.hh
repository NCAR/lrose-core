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
// structure definitions for VerifyTracks
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2005
//
////////////////////////////////////////////////////////////////

#ifndef vt_structs_hh
#define vt_structs_hh

#include <titan/track.h>

// struct definitions

typedef struct {
  double x, y;
} vt_point_t;

// struct for storm entry in a track

typedef struct {
  
  track_file_entry_t entry;
  storm_file_global_props_t gprops;
  storm_file_run_t *runs;
  
  // lookup tables to relate the scan cart grid
  // to the verification grid
  
  long *x_lookup;
  long *y_lookup;
  
} vt_storm_t;

// struct for simple track and its storms

typedef struct {
  
  simple_track_params_t params;
  vt_storm_t *storms;
  
} vt_simple_track_t;

// struct for index to a track entry

typedef struct {
  
  long isimple;
  long istorm;
  
} vt_entry_index_t;

// struct for contingency data

typedef struct {
  
  double n_success;
  double n_failure;
  double n_false_alarm;
  double n_non_event;
  
} vt_count_t;

// struct for rmse computations

typedef struct {
  
  track_file_forecast_props_t sum_sq_error;
  track_file_forecast_props_t sum_error;
  track_file_forecast_props_t rmse;
  track_file_forecast_props_t bias;
  
  // distance error btn forecast and verify cells 
  
  double sum_dist_error ;
  double sum_sq_dist_error ;
  
  // correlation coefficient
  
  track_file_forecast_props_t sumx;
  track_file_forecast_props_t sumy;
  track_file_forecast_props_t sumx2;
  track_file_forecast_props_t sumy2;
  track_file_forecast_props_t sumxy;
  track_file_forecast_props_t corr;
  
  // norm refers to error normalized relative to the
  // data value, i.e. as a fraction of the correct val
  
  track_file_forecast_props_t norm_sum_sq_error;
  track_file_forecast_props_t norm_sum_error;
  track_file_forecast_props_t norm_rmse;
  track_file_forecast_props_t norm_bias;
  
  double n_movement;
  double n_growth;
  
} vt_stats_t;

#endif
