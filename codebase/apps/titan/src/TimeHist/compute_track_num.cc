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
/*********************************************************************
 * compute_track_num.c
 *
 * Gets the number of the track closest to the focus point as defined
 * by a double click in the cappi plot window.
 *
 * The track number is stored in the track shared memory header
 *
 * RAP, NCAR, Boulder CO
 *
 * April 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "TimeHist.hh"
#include <toolsa/DateTime.hh>
#include <rapformats/titan_grid.h>
#include <titan/SeedCaseTracks.hh>
using namespace std;

/*
 * file scope
 */

static int _firstCall = TRUE;
static int _prevPointerSeqNum = -1;
static int _prevShmemComplexTrackNum;
static int _prevShmemSimpleTrackNum;

static SeedCaseTracks _cases;
static SeedCaseTracks::CaseTrack _currentCase;
static int _prevCaseComplexTrackNum = -1;
static int _prevCaseSimpleTrackNum = -1;

static int find_from_centroid(coord_export_t *coord,
			      int *complex_track_num_p,
			      int *simple_track_num_p,
			      int *complex_index_p,
			      int *simple_index_p);

static int find_from_previous(int complex_track_num,
			      int simple_track_num,
			      int *complex_index_p,
			      int *simple_index_p);

static void set_case_track(int complex_track_num,
                           int simple_track_num);

/*
 * main routine
 */

void compute_track_num(void)

{

  int track_found = FALSE;
  int select_track_type;
  
  int complex_track_num;
  int simple_track_num;
  int complex_index, simple_index;

  time_hist_shmem_t *tshmem;
  coord_export_t *coord;

  if (Glob->verbose) {
    fprintf(stderr, "** compute_track_num **\n");
  }

  /*
   * on first call, initialize case track module
   */

  if (_firstCall) {
    if (Glob->use_case_tracks) {
      _cases.setDebug(Glob->verbose);
      if (_cases.readCaseFile(Glob->case_tracks_file_path)) {
	fprintf(stderr, "WARNING - %s:compute_track_num\n",
		Glob->prog_name);
	fprintf(stderr, "Cannot read in case track file\n");
	fprintf(stderr, "Proceeding without case tracks\n");
	Glob->use_case_tracks = FALSE;
      }
    } /* if (Glob->use_case_tracks) */
    _firstCall = FALSE;
  }

  // if the call to this routine was cause by a change in time,
  // in turn caused by a change in case, then leave the track
  // numbers as set by the case
  
  /*
   * set pointers and local variables
   */

  coord = Glob->coord_export;
  tshmem = Glob->track_shmem;

  // initialize partial track params if first time through
  
  if (tshmem->partial_track_ref_time == 0) {
    tshmem->partial_track_ref_time = tshmem->time;
    tshmem->partial_track_past_period = Glob->partial_track_past_period;
    tshmem->partial_track_future_period = Glob->partial_track_future_period;
  }

  /*
   * Check if user has selected a track type
   */

  if (tshmem->select_track_type == NO_TRACK_TYPE) {

    /*
     * no selection made, use current track type
     */

    select_track_type = tshmem->track_type;
    
  } else {
  
    /*
     * set track type from shmem
     */
    
    select_track_type = tshmem->select_track_type;

  }

  /*
   * reset the variable, ready for next selection
   */

  tshmem->select_track_type = NO_TRACK_TYPE;

  /*
   * check whether pointer has been activated since last time
   */

  if (_prevPointerSeqNum != coord->pointer_seq_num) {

    /*
     * new pointer position, find track numbers
     */

    if (find_from_centroid(coord,
			   &complex_track_num, &simple_track_num,
			   &complex_index, &simple_index) == 0) {

      track_found = TRUE;
      
      if (Glob->use_case_tracks) {

	/*
	 * If we are using case tracks, set track in shmem
	 */

	set_case_track(complex_track_num, simple_track_num);

      } /* if (Glob->use_case_tracks) */

    }

  } else {

    /*
     * pointer has not changed position, use previous numbers
     * to find indices
     */
    
    if (find_from_previous(_prevShmemComplexTrackNum,
			   _prevShmemSimpleTrackNum,
			   &complex_index,
			   &simple_index) == 0) {
      track_found = TRUE;
      complex_track_num = _prevShmemComplexTrackNum;
      simple_track_num = _prevShmemSimpleTrackNum;
    }

  }

  tshmem->track_seq_num++;

  /*
   * if no track found, set complex track number to -1.
   * This will indicate to this and the main display
   * that no track data is available
   */
  
  if (!track_found) {

    // _prevPointerSeqNum = -1;
    tshmem->complex_track_num = -1;
    tshmem->simple_track_num = -1;
    Glob->complex_index = 0;
    Glob->simple_index = 0;
    return;

  }

  /*
   * set previous values
   */

  _prevPointerSeqNum = coord->pointer_seq_num;
  _prevShmemComplexTrackNum = complex_track_num;
  _prevShmemSimpleTrackNum = simple_track_num;

  /*
   * load up shmem
   */
  
  tshmem->track_type = select_track_type;
  tshmem->complex_track_num = complex_track_num;
  Glob->complex_index = complex_index;

  switch (select_track_type) {

    case COMPLEX_TRACK:
      tshmem->simple_track_num = -1;
      Glob->simple_index = 0;
      break;

    case SIMPLE_TRACK:
      tshmem->simple_track_num = simple_track_num;
      Glob->simple_index = simple_index;
      break;

    case PARTIAL_TRACK:
      tshmem->simple_track_num = simple_track_num;
      Glob->simple_index = simple_index;
      break;

  } /* switch */

  return;
  
}

/*********************************************
 * find_from_centroid()
 *
 * Routine for TitanServer data
 *
 */

static int find_from_centroid(coord_export_t *coord,
                              int *complex_track_num_p,
                              int *simple_track_num_p,
                              int *complex_index_p,
                              int *simple_index_p)

{

  int track_found = FALSE;
  
  int complex_track_num = 0;
  int simple_track_num = 0;
  int complex_index = 0, simple_index = 0;

  double focus_lat, focus_lon;
  double storm_focus_x, storm_focus_y;
  double delta_x, delta_y;
  double range_from_focus;
  double min_range_from_focus = LARGE_DOUBLE;

  titan_grid_comps_t display_comps;
  titan_grid_comps_t storm_comps;

  int n_complex_tracks = Glob->_dsTitan.complex_tracks().size();

  /*
   * compute the focus point in terms of the storm grid
   */

  if (n_complex_tracks > 0) {
    time_hist_shmem_t *tshmem = Glob->track_shmem;
    TITAN_init_proj(&tshmem->grid, &display_comps);
    TITAN_init_proj(&Glob->titan_grid, &storm_comps);
    TITAN_xy2latlon(&display_comps,
                    coord->focus_x, coord->focus_y,
                    &focus_lat, &focus_lon);
    TITAN_latlon2xy(&storm_comps, focus_lat, focus_lon,
                    &storm_focus_x, &storm_focus_y);
  }
	
  /*
   * loop through the complex tracks
   */

  for (size_t icomplex = 0;
       icomplex < Glob->_dsTitan.complex_tracks().size(); icomplex++) {
    
    const TitanComplexTrack *ctrack =
      Glob->_dsTitan.complex_tracks()[icomplex];
    
    /*
     * loop through the simple tracks
     */
    
    for (size_t isimple = 0;
	 isimple < ctrack->simple_tracks().size(); isimple++) {
      
      const TitanSimpleTrack *strack = ctrack->simple_tracks()[isimple];
      int duration = strack->entries().size();
      
      /*
       * loop through the entries
       */
      
      for (int ientry = 0; ientry < duration; ientry++) {
	
	const TitanTrackEntry *tentry = strack->entries()[ientry];
	const storm_file_global_props_t *gprops = &tentry->gprops();
	
	delta_x = gprops->proj_area_centroid_x - storm_focus_x;
	delta_y = gprops->proj_area_centroid_y - storm_focus_y;
	
	range_from_focus =
	  sqrt(pow(delta_x, 2.0) + pow(delta_y, 2.0));

	if (range_from_focus < min_range_from_focus) {
	  
	  complex_track_num =
	    ctrack->complex_params().complex_track_num;

	  simple_track_num =
	    strack->simple_params().simple_track_num;
	  
	  min_range_from_focus = range_from_focus;

	  complex_index = icomplex;
	  simple_index = isimple;

	  track_found = TRUE;

	} /* if (range_from_focus < min_range_from_focus) */
	
      } /* ientry */
      
    } /* isimple */

  } /* icomplex */

  if (track_found) {
    *complex_track_num_p = complex_track_num;
    *simple_track_num_p = simple_track_num;
    *complex_index_p = complex_index;
    *simple_index_p = simple_index;
    return (0);
  } else {
    return (-1);
  }

}

/*********************************************
 * Finds indices from previous track numbers
 *
 * Fills out complex_index_p,
 *           simple_index_p.
 *
 * Returns 0 on success, -1 on failure (no track found)
 */

static int find_from_previous(int complex_track_num,
                              int simple_track_num,
                              int *complex_index_p,
                              int *simple_index_p)
  
{

  /*
   * loop through the complex tracks
   */

  for (size_t icomplex = 0;
       icomplex < Glob->_dsTitan.complex_tracks().size(); icomplex++) {
    
    const TitanComplexTrack *ctrack =
      Glob->_dsTitan.complex_tracks()[icomplex];
    
    /*
     * loop through the simple tracks
     */
    
    for (size_t isimple = 0;
	 isimple < ctrack->simple_tracks().size(); isimple++) {
      
      const TitanSimpleTrack *strack = ctrack->simple_tracks()[isimple];

      if (strack->simple_params().simple_track_num == simple_track_num) {
	*complex_index_p = icomplex;
	*simple_index_p = isimple;
	return (0);
      }

    } // isimple

  } // icomplex

  /*
   * not found
   */
  
  return (-1);

}

/**************************************
 * set_case_track()
 *
 * Sets the case track in shared memory
 */

static void set_case_track(int complex_track_num,
                           int simple_track_num)

{

  // if the track numbers have not changed, then do not
  // recopmute partial track just because time has changed

  if (complex_track_num == _prevCaseComplexTrackNum &&
      simple_track_num == _prevCaseSimpleTrackNum) {
    return;
  }
  _prevCaseComplexTrackNum = complex_track_num;
  _prevCaseSimpleTrackNum = simple_track_num;
  
  /*
   * get the past & future period for partial tracks
   * from the case file if applicable
   */
  
  time_hist_shmem_t *tshmem = Glob->track_shmem;

  if (_cases.findCase(tshmem->time,
                      complex_track_num,
                      simple_track_num,
                      _currentCase) == 0) {
    
    /*
     * case found
     */
    
    tshmem->case_num = _currentCase.num;
    tshmem->partial_track_ref_time = _currentCase.ref_time;
    tshmem->partial_track_past_period = _currentCase.ref_minus_start;
    tshmem->partial_track_future_period = _currentCase.end_minus_ref;

  } else {
    
    /*
     * no case found
     */
    
    tshmem->case_num = -1;
    tshmem->partial_track_ref_time = tshmem->time;
    tshmem->partial_track_past_period = Glob->partial_track_past_period;
    tshmem->partial_track_future_period = Glob->partial_track_future_period;
    
  }
  
}

/**************************************
 * Go to the next case
 * Sets the case track in shared memory
 */

void go_to_next_case()
  
{

  if (!Glob->use_case_tracks) {
    return;
  }

  time_hist_shmem_t *tshmem = Glob->track_shmem;

  SeedCaseTracks::CaseTrack nextCase;
  if (_currentCase.ref_time == 0) {
    if (_cases.getNextCase(tshmem->time, nextCase)) {
      if (Glob->debug) {
        cerr << "  Cannot go to next case" << endl;
        cerr << "  Current time is: "
             << DateTime::strm(tshmem->time) << endl;
      }
      return;
    }
  } else {
    if (_cases.getNextCase(_currentCase, nextCase)) {
      if (Glob->debug) {
        cerr << "  Cannot go to next case" << endl;
        cerr << "  Current case is last case, num: "
             << _currentCase.num << endl;
      }
      return;
    }
  }

  // set current case to next case

  _currentCase = nextCase;
  
  tshmem->case_num = _currentCase.num;
  tshmem->partial_track_ref_time = _currentCase.ref_time;
  tshmem->partial_track_past_period = _currentCase.ref_minus_start;
  tshmem->partial_track_future_period = _currentCase.end_minus_ref;

  tshmem->time = _currentCase.ref_time;
  tshmem->mode = TDATA_ARCHIVE;
  
  tshmem->complex_track_num = _currentCase.complex_track_num;
  tshmem->simple_track_num = _currentCase.simple_track_num;
  tshmem->select_track_type = PARTIAL_TRACK;
  tshmem->track_type = PARTIAL_TRACK;
  tshmem->track_seq_num++;
  tshmem->main_display_must_update = TRUE;

  _prevShmemComplexTrackNum = _currentCase.complex_track_num;
  _prevShmemSimpleTrackNum = _currentCase.simple_track_num;

  _prevCaseComplexTrackNum = _currentCase.complex_track_num;
  _prevCaseSimpleTrackNum = _currentCase.simple_track_num;

}

/**************************************
 * Go to the prev case
 * Sets the case track in shared memory
 */

void go_to_prev_case()
  
{

  time_hist_shmem_t *tshmem = Glob->track_shmem;

  if (!Glob->use_case_tracks) {
    return;
  }

  SeedCaseTracks::CaseTrack prevCase;

  if (_currentCase.ref_time == 0) {
    if (_cases.getPrevCase(tshmem->time, prevCase)) {
      if (Glob->debug) {
        cerr << "  Cannot go to prev case" << endl;
        cerr << "  Current time is: "
             << DateTime::strm(tshmem->time) << endl;
      }
      return;
    }
  } else {
    if (_cases.getPrevCase(_currentCase, prevCase)) {
      if (Glob->debug) {
        cerr << "  Cannot go to prev case" << endl;
        cerr << "  Current case is first case, num: "
             << _currentCase.num << endl;
      }
      return;
    }
  }

  // set current case to prev case

  _currentCase = prevCase;
  
  tshmem->case_num = _currentCase.num;
  tshmem->partial_track_ref_time = _currentCase.ref_time;
  tshmem->partial_track_past_period = _currentCase.ref_minus_start;
  tshmem->partial_track_future_period = _currentCase.end_minus_ref;

  tshmem->time = _currentCase.ref_time;
  tshmem->mode = TDATA_ARCHIVE;
  
  tshmem->complex_track_num = _currentCase.complex_track_num;
  tshmem->simple_track_num = _currentCase.simple_track_num;
  tshmem->track_type = PARTIAL_TRACK;
  tshmem->track_seq_num++;
  tshmem->main_display_must_update = TRUE;

  _prevCaseComplexTrackNum = _currentCase.complex_track_num;
  _prevCaseSimpleTrackNum = _currentCase.simple_track_num;

  _prevShmemComplexTrackNum = _currentCase.complex_track_num;
  _prevShmemSimpleTrackNum = _currentCase.simple_track_num;

}

