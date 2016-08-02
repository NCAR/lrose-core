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
// TrTrack.cc
//
// TrTrack class - used for storing track status and history
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1999
//
///////////////////////////////////////////////////////////////

#include "TrTrack.hh"
#include <iostream>
#include <toolsa/mem.h>
using namespace std;

//////////////
// constructor
//

TrTrack::TrTrack()

{

  MEM_zero(status);

}

/////////////
// destructor
//

TrTrack::~TrTrack()

{

}

///////////////////////
// init_new
//
// initialize new track
//
// returns 0 on success, -1 on error

int TrTrack::init_new(TitanTrackFile &tfile,
		      date_time_t *dtime,
		      vector<track_utime_t> &track_utime,
		      int scan_num,
		      bool new_complex_track,
		      int complex_track_num,
		      int history_in_scans,
		      int scan_origin,
		      date_time_t *time_origin,
		      bool debug)

{

  /*
   * set track number and increment ntracks,
   * checking for array space
   */
  
  status.n_parents = 0;
  status.simple_track_num = tfile._header.n_simple_tracks;
  tfile._header.max_simple_track_num = status.simple_track_num;
  tfile._header.n_simple_tracks++;

  tfile.AllocSimpleArrays(tfile._header.n_simple_tracks);
  tfile.AllocSimplesPerComplex(tfile._header.n_simple_tracks);
  
  if (debug) {
    fprintf(stderr, "\nStarting simple track %d\n", status.simple_track_num);
  }
  
  /*
   * initialize track_data struct
   */
  
  MEM_zero(status.history);
  status.duration_in_scans = 0;
  status.history_in_scans = history_in_scans;
  status.scan_origin = scan_origin;
  status.time_origin = *time_origin;

  /*
   * compute history in seconds
   */
  
  compute_history_in_secs(dtime);

  /*
   * set up the track params
   */

  simple_track_params_t &st_params = tfile._simple_params;
  MEM_zero(st_params);
  
  st_params.simple_track_num = status.simple_track_num;
  st_params.start_scan = scan_num;
  st_params.end_scan = scan_num;
  st_params.duration_in_scans = 0;
  st_params.duration_in_secs = 0;
  st_params.history_in_scans = status.history_in_scans;
  st_params.scan_origin = status.scan_origin;
  
  st_params.start_time = dtime->unix_time;
  st_params.end_time = dtime->unix_time;
  st_params.time_origin = status.time_origin.unix_time;

  track_utime[status.simple_track_num].start_simple = dtime->unix_time;
  track_utime[status.simple_track_num].end_simple = dtime->unix_time;
  
  if (new_complex_track) {

    status.complex_track_num = status.simple_track_num;
    if (start_complex_track(tfile, track_utime)) {
      return (-1);
    }
    if (debug) {
      fprintf(stderr, "starting complex track %d\n",
	      status.complex_track_num);
    }

  } else {

    status.complex_track_num = complex_track_num;
    if (augment_complex_track(tfile)) {
      return (-1);
    }
    if (debug) {
      fprintf(stderr, "augmenting complex track %d\n",
	      status.complex_track_num);
    }

  } /* if (new_complex_track) */

  st_params.complex_track_num = status.complex_track_num;

  /*
   * write simple track params to file
   */

  if (tfile.WriteSimpleParams(status.simple_track_num)) {
    cerr << "ERROR - Titan:TrTrack::init_new" << endl;
    cerr << tfile.getErrStr() << endl;
    return -1;
  }

  return (0);

}

///////////////////////
// start_complex_track
//
// Expects simple_track_num to be set already
// Sets complex_track_num
//
// returns 0 on success, -1 on error

int TrTrack::start_complex_track(TitanTrackFile &tfile,
				 vector<track_utime_t> &track_utime)
  
{

  /*
   * increment n_complex_tracks, checking for array space.
   */
  
  tfile._header.max_complex_track_num = status.complex_track_num;
  tfile._header.n_complex_tracks++;

  tfile.AllocComplexArrays(tfile._header.n_complex_tracks);

  int itr = tfile._header.n_complex_tracks - 1;
  tfile._complex_track_nums[itr] = status.complex_track_num;
  
  /*
   * initialize complex track params
   */

  complex_track_params_t &ct_params = tfile._complex_params;
  simple_track_params_t &st_params = tfile._simple_params;
  MEM_zero(ct_params);
  
  ct_params.complex_track_num = status.complex_track_num;
  ct_params.n_simple_tracks = 1;
  status.n_simple_tracks = ct_params.n_simple_tracks;
  tfile._simples_per_complex[status.complex_track_num]
    = (si32 *) umalloc (sizeof(si32));
  tfile._simples_per_complex[status.simple_track_num][0] =
    status.simple_track_num;
  tfile._nsimples_per_complex[status.simple_track_num] =
    ct_params.n_simple_tracks;

  ct_params.start_scan = st_params.start_scan;
  ct_params.end_scan = st_params.end_scan;
  ct_params.duration_in_scans = st_params.duration_in_scans;
  ct_params.duration_in_secs = st_params.duration_in_secs;

  ct_params.start_time = st_params.start_time;
  ct_params.end_time = st_params.end_time;

  /*
   * set start and end julian time arrays
   */

  track_utime[status.complex_track_num].start_complex =
    track_utime[status.simple_track_num].start_simple;
  
  track_utime[status.complex_track_num].end_complex =
    track_utime[status.simple_track_num].end_simple;
  
  /*
   * write complex track params to file
   */
  
  if (tfile.WriteComplexParams(status.complex_track_num)) {
    cerr << "ERROR - Titan:TrTrack::start_complex_track" << endl;
    cerr << tfile.getErrStr() << endl;
    return -1;
  }

  return (0);

}

////////////////////////
// augment_complex_track
//
// Adds a simple track to a complex track
//
// Expects simple_track_num and complex_track_num to be set already
//
// returns 0 on success, -1 on error

int TrTrack::augment_complex_track(TitanTrackFile &tfile)

{

  /*
   * read in the complex track params
   */

  if (tfile.ReadComplexParams(status.complex_track_num, false)) {
    cerr << "ERROR - Titan:TrTrack::augment_complex_track" << endl;
    cerr << tfile.getErrStr() << endl;
    return -1;
  }

  complex_track_params_t &ct_params = tfile._complex_params;
  
  /*
   * realloc the array for simple track nums
   */
  
  tfile._simples_per_complex[status.complex_track_num] = (si32 *) urealloc
    (tfile._simples_per_complex[status.complex_track_num],
     ((ct_params.n_simple_tracks + 1) * sizeof(si32)));
  
  /*
   * update complex params
   */

  tfile._simples_per_complex[status.complex_track_num]
    [ct_params.n_simple_tracks] = status.simple_track_num;
  
  ct_params.n_simple_tracks++;
  status.n_simple_tracks = ct_params.n_simple_tracks;

  tfile._nsimples_per_complex[status.complex_track_num] =
    ct_params.n_simple_tracks;

  /*
   * rewrite the complex track params
   */

  if (tfile.WriteComplexParams(status.complex_track_num)) {
    cerr << "ERROR - Titan:TrTrack::augment_complex_track" << endl;
    cerr << tfile.getErrStr() << endl;
    return -1;
  }

  /*
   * set the complex_track_offset to zero because this complex track already
   * has a complex_params slot in the file
   */
  
  tfile._complex_track_offsets[status.simple_track_num] = 0;
  
  return (0);

}

////////////////////////////
// compute_history_in_secs()
//
// Sets history_in_secs
//
// Expects history_in_scans and time_origin to have been set.
//

void TrTrack::compute_history_in_secs(date_time_t *dtime)

{
  
  double dhist_in_scans;
  
  /*
   * compute history in seconds, adjusting for the half scan at each
   * end of the track - the scan times refer to the mid time
   */
  
  dhist_in_scans = (double) status.history_in_scans;
  
  if (status.history_in_scans == 1) {
    
    status.history_in_secs = 0;
    
  } else {
    
    status.history_in_secs = (si32)
      ((double) (dtime->unix_time - status.time_origin.unix_time) *
       (dhist_in_scans / (dhist_in_scans - 1.0)) + 0.5);
    
  } /* if (history_in_scans == 1) */

}
