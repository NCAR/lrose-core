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
// TrStorm.cc
//
// TrStorm class - used for storing storm status in tracking
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1999
//
///////////////////////////////////////////////////////////////

#include "TrStorm.hh"
#include <toolsa/mem.h>
using namespace std;

//////////////
// constructor
//

TrStorm::TrStorm()

{

  match_array = NULL;
  // _n_match_alloc = 0;

  proj_runs = NULL;
  // _n_proj_runs_alloc = 0;

  MEM_zero(status);
  MEM_zero(box_for_overlap);
  MEM_zero(current);

}

/////////////
// destructor
//

TrStorm::~TrStorm()

{

}

////////////////////
// alloc_match_array


void TrStorm::alloc_match_array(int n_match)

{

  match_array = (track_match_t *)
    _match_buf.reserve(n_match * sizeof(track_match_t));
  
  return;

}

////////////////////
// alloc_proj_runs

void TrStorm::alloc_proj_runs(int n_proj_runs)
     
{

  proj_runs = (storm_file_run_t *)
    _proj_runs_buf.reserve(n_proj_runs * sizeof(storm_file_run_t));

  return;

}

///////////////
// load_props()
//
// load storm props
//
// Returns 0 on success, -1 on failure. Failure indicates that
// storm runs are not available
//

int TrStorm::load_props(int storm_num,
			const date_time_t &scan_time,
			TitanStormFile &sfile)
     
{

  int iret = 0;
  
  /*
   * set local variables
   */
  
  const storm_file_global_props_t *gprops = sfile.gprops() + storm_num;
    
  /*
   * load props struct
   */
  
  current.time = scan_time;
  
  current.proj_area_centroid_x = gprops->proj_area_centroid_x;
  current.proj_area_centroid_y = gprops->proj_area_centroid_y;
  current.vol_centroid_z = gprops->vol_centroid_z;
  current.refl_centroid_z = gprops->refl_centroid_z;
  current.top = gprops->top;
  
  current.dbz_max = gprops->dbz_max;
  current.dbz_mean = gprops->dbz_mean;
  current.volume = gprops->volume;
  current.precip_flux = gprops->precip_flux;
  current.mass = gprops->mass;
  current.proj_area = gprops->proj_area;

  for (int j = 0; j < N_POLY_SIDES; j++) {
    current.proj_area_rays[j] = gprops->proj_area_polygon[j];
  }
    
  current.bound.min_ix = gprops->bounding_min_ix;
  current.bound.min_iy = gprops->bounding_min_iy;
  current.bound.max_ix = gprops->bounding_max_ix;
  current.bound.max_iy = gprops->bounding_max_iy;

  status.n_proj_runs = gprops->n_proj_runs;

  if (status.n_proj_runs > 0) {
      
    /*
     * read proj runs
     */

    if (sfile.ReadProjRuns(storm_num)) {
      cerr << "ERROR - TrStorm::load_props" << endl;
      cerr << sfile.getErrStr() << endl;
      status.n_proj_runs = 0;
      return -1;
    }

    /*
     * copy over to storm array
     */
    
    alloc_proj_runs(status.n_proj_runs);

    memcpy(proj_runs,
	   sfile.proj_runs(),
	   status.n_proj_runs * sizeof(storm_file_run_t));
    
  } else {
    
    // proj_runs not available
    
    iret = -1;

  } /* if (status.n_proj_runs > 0) */
      
  return (iret);

}

////////////////////////////
// update_times()
//
// update times for an existing track
//

void TrStorm::update_times(date_time_t *dtime,
			   vector<track_utime_t> &track_utime,
			   int n_forecasts)

{
  
  /*
   * set local variables
   */

  TrTrack::props_t *hist = track.status.history;
  int simple_track_num = track.status.simple_track_num;

  /*
   * update storm track data struct
   */
  
  track.status.duration_in_scans++;
  track.status.history_in_scans++;

  /*
   * compute history in seconds, adjusting for the half scan at each
   * end of the track - the scan times refer to the mid time
   */
  
  track.compute_history_in_secs(dtime);

  /*
   * compute duration in seconds, adjusting for the half scan at each
   * end of the track - the scan times refer to the mid time
   */
  
  double duration_in_scans = (double) track.status.duration_in_scans;

  int duration_in_secs;

  if (track.status.duration_in_scans == 1) {
    
    duration_in_secs = 0;

  } else {

    duration_in_secs = (int)
      ((double) (dtime->unix_time -
		 track_utime[simple_track_num].start_simple) *
       (duration_in_scans / (duration_in_scans - 1.0)) + 0.5);
    
  } /* if (duration_in_scans == 1) */

  track.status.duration_in_secs = duration_in_secs;
  
  /*
   * move history data array up by one slot
   */

  for (int ihist = n_forecasts - 1; ihist > 0; ihist--) {
    hist[ihist] = hist[ihist - 1];
  }

  /*
   * load first history slot
   */

  hist[0] = current;

}

