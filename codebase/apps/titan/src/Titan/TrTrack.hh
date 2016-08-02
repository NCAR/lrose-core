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
// TrTrack.hh
//
// TrTrack class - used for storing track status and history
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1999
//
///////////////////////////////////////////////////////////////

#ifndef TrTrack_HH
#define TrTrack_HH

#include <titan/TitanTrackFile.hh>
#include <vector>
using namespace std;

////////////////////////////////
// TrTrack

class TrTrack {
  
public:

  // typedefs

  typedef struct {
    int min_ix, min_iy, max_ix, max_iy;
  } bounding_box_t;

  typedef struct {
    double proj_area_centroid_x;
    double proj_area_centroid_y;
    double vol_centroid_z;
    double refl_centroid_z;
    double top;
    double dbz_max;
    double dbz_mean;
    double volume;
    double precip_flux;
    double mass;
    double proj_area;
    double smoothed_proj_area_centroid_x;
    double smoothed_proj_area_centroid_y;
    double smoothed_speed;
    double smoothed_direction;
    double proj_area_rays[N_POLY_SIDES];
    bounding_box_t bound;
    date_time_t time;
  } props_t;

  typedef struct {

    bool forecast_valid;

    double smoothed_history_in_secs;
    double forecast_x;
    double forecast_y;
    double forecast_area;
    double forecast_length_ratio;
    double area_change_ratio;
    
    int n_parents;
    int scan_origin;
    int simple_track_num;
    int complex_track_num;
    int n_simple_tracks;
    int duration_in_scans;
    int duration_in_secs;
    int entry_offset;
    int history_in_scans;
    int history_in_secs;
    
    date_time_t time_origin;

    props_t dval_dt;
    props_t history[MAX_NWEIGHTS_FORECAST];

  } status_t;

  // constructor
  
  TrTrack();

  // destructor
  
  virtual ~TrTrack();

  // status data

  status_t status;

  // member functions

  // initialize new track

  int init_new(TitanTrackFile &tfile,
	       date_time_t *dtime,
	       vector<track_utime_t> &track_utime,
	       int scan_num,
	       bool new_complex_track,
	       int complex_track_num,
	       int history_in_scans,
	       int scan_origin,
	       date_time_t *time_origin,
	       bool debug);

  // compute history in seconds

  void compute_history_in_secs(date_time_t *dtime);

  // start complex track

  int start_complex_track(TitanTrackFile &tfile,
			  vector<track_utime_t> &track_utime);
  
  // augment_complex_track
  
  int augment_complex_track(TitanTrackFile &tfile);

protected:
  
private:

};

#endif
