/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
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

#include "time_hist.h"
#include <rapformats/titan_grid.h>
#include <titan/case_tracks.h>

/*
 * file scope prototypes
 */

static int find_from_centroid(time_hist_shmem_t *tshmem,
			      tdata_complete_index_t *index,
			      coord_export_t *coord,
			      si32 *complex_track_num_p,
			      si32 *simple_track_num_p,
			      si32 *complex_index_p,
			      si32 *simple_index_p);

static int load_from_previous(tdata_complete_index_t *index,
			      si32 prev_complex_track_num,
			      si32 prev_simple_track_num,
			      si32 *complex_index_p,
			      si32 *simple_index_p);

static void set_case_track_in_shmem(case_track_handle_t *case_handle,
				    time_hist_shmem_t *tshmem,
				    si32 complex_track_num,
				    si32 simple_track_num);

/*
 * main routine
 */

void compute_track_num(void)

{

  static int first_call = TRUE;
  static int prev_pointer_seq_num = -1;
  static si32 prev_complex_track_num;
  static si32 prev_simple_track_num;
  static case_track_handle_t case_handle;
  
  int track_found = FALSE;
  int select_track_type;
  
  si32 complex_track_num;
  si32 simple_track_num;
  si32 complex_index, simple_index;

  tdata_complete_index_t *index;
  time_hist_shmem_t *tshmem;
  coord_export_t *coord;

  if (Glob->debug) {
    fprintf(stderr, "** compute_track_num **\n");
  }

  /*
   * on first call, initialize case track module
   */

  if (first_call) {
    if (Glob->use_case_tracks) {
      init_case_track_handle(&case_handle,
			     Glob->prog_name, Glob->debug);
      if (read_case_track_file(&case_handle,
			       Glob->case_tracks_file_path)) {
	fprintf(stderr, "WARNING - %s:compute_track_num\n",
		Glob->prog_name);
	fprintf(stderr, "Cannot read in case track file\n");
	fprintf(stderr, "Proceeding without case tracks\n");
	Glob->use_case_tracks = FALSE;
	free_case_track_handle(&case_handle);
      }
    } /* if (Glob->use_case_tracks) */
    first_call = FALSE;
  }

  /*
   * set pointers and local variables
   */

  index = &Glob->tdata_index.complete;
  coord = Glob->coord_export;
  tshmem = Glob->track_shmem;

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

  if (prev_pointer_seq_num != coord->pointer_seq_num) {

    /*
     * new pointer position, find track numbers
     */

    if (find_from_centroid(tshmem, index, coord,
			   &complex_track_num, &simple_track_num,
			   &complex_index, &simple_index) == 0) {

      track_found = TRUE;
      
      if (Glob->use_case_tracks) {

	/*
	 * If we are using case tracks, set track in shmem
	 */

	set_case_track_in_shmem(&case_handle, tshmem,
				complex_track_num, simple_track_num);

      } /* if (Glob->use_case_tracks) */

    }

  } else {

    /*
     * pointer has not changed position, use previous numbers
     */

    if (load_from_previous(index,
			   prev_complex_track_num,
			   prev_simple_track_num,
			   &complex_index,
			   &simple_index) == 0) {
      track_found = TRUE;
      complex_track_num = prev_complex_track_num;
      simple_track_num = prev_simple_track_num;
    }

  }

  tshmem->track_seq_num++;

  /*
   * if no track found, set complex track number to -1.
   * This will indicate to this and the main display
   * that no track data is available
   */
  
  if (!track_found) {

    prev_pointer_seq_num = -1;
    tshmem->complex_track_num = -1;
    tshmem->simple_track_num = -1;
    Glob->complex_index = 0;
    Glob->simple_index = 0;
    return;

  }

  /*
   * set previous values
   */

  prev_pointer_seq_num = coord->pointer_seq_num;
  prev_complex_track_num = complex_track_num;
  prev_simple_track_num = simple_track_num;

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
 * Finds track numbers from centroid location.
 *
 * Fills out complex_track_num_p,
 *           simple_track_num_p,
 *           complex_index_p,
 *           simple_index_p.
 *
 * Returns 0 on success, -1 on failure (no track found)
 */

static int find_from_centroid(time_hist_shmem_t *tshmem,
			      tdata_complete_index_t *index,
			      coord_export_t *coord,
			      si32 *complex_track_num_p,
			      si32 *simple_track_num_p,
			      si32 *complex_index_p,
			      si32 *simple_index_p)

{


  int track_found = FALSE;
  
  si32 icomplex, isimple, ientry;
  si32 duration;
  si32 complex_track_num;
  si32 simple_track_num;
  si32 complex_index, simple_index;

  double focus_lat, focus_lon;
  double storm_focus_x, storm_focus_y;
  double delta_x, delta_y;
  double range_from_focus;
  double min_range_from_focus = LARGE_DOUBLE;

  storm_file_params_t *sparams;
  storm_file_global_props_t *gprops;
  simple_track_params_t *st_params;
  complex_track_params_t *ct_params;
  tdata_complete_track_entry_t *track_entry;
  titan_grid_comps_t display_comps;
  titan_grid_comps_t storm_comps;

  /*
   * set pointers and local variables
   */

  sparams = &index->header.sparams;

  /*
   * compute the focus point in terms of the storm grid
   */

  if (index->header.n_complex_tracks > 0) {

    TITAN_init_proj(&tshmem->grid, &display_comps);
    TITAN_init_proj(&index->track_entry[0][0]->scan.grid, &storm_comps);
    
    TITAN_xy2latlon(&display_comps,
		  coord->focus_x, coord->focus_y,
		  &focus_lat, &focus_lon);
    
    TITAN_latlon2xy(&storm_comps, focus_lat, focus_lon,
		  &storm_focus_x, &storm_focus_y);

  }
	
  /*
   * loop through the complex tracks
   */

  for (icomplex = 0;
       icomplex < index->header.n_complex_tracks; icomplex++) {

    ct_params = index->complex_params + icomplex;

    /*
     * loop through the simple tracks
     */

    for (isimple = 0;
	 isimple < ct_params->n_simple_tracks; isimple++) {
      
      st_params = index->simple_params[icomplex] + isimple;
      
      duration = st_params->duration_in_scans;

      /*
       * loop through the entries
       */

      for (ientry = 0; ientry < duration; ientry++) {

	track_entry = index->track_entry[icomplex][isimple] + ientry;
	gprops = &track_entry->gprops;
	
	delta_x = gprops->proj_area_centroid_x - storm_focus_x;
	delta_y = gprops->proj_area_centroid_y - storm_focus_y;
	
	range_from_focus =
	  sqrt(pow(delta_x, 2.0) + pow(delta_y, 2.0));

	if (range_from_focus < min_range_from_focus) {
	  
	  complex_track_num =
	    ct_params->complex_track_num;

	  simple_track_num =
	    st_params->simple_track_num;
	  
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
 * load_from_previous()
 *
 * Loads up indices from previous track numbers
 *
 * Fills out complex_index_p,
 *           simple_index_p.
 *
 * Returns 0 on success, -1 on failure (no track found)
 */

static int load_from_previous(tdata_complete_index_t *index,
			      si32 prev_complex_track_num,
			      si32 prev_simple_track_num,
			      si32 *complex_index_p,
			      si32 *simple_index_p)

{


  si32 icomplex, isimple;
  simple_track_params_t *st_params;
  complex_track_params_t *ct_params;

  /*
   * loop through the complex tracks
   */

  for (icomplex = 0;
       icomplex < index->header.n_complex_tracks; icomplex++) {

    ct_params = index->complex_params + icomplex;

    if (ct_params->complex_track_num == prev_complex_track_num) {
      
      /*
       * loop through the simple tracks
       */
      
      for (isimple = 0;
	   isimple < ct_params->n_simple_tracks; isimple++) {
	
	st_params = index->simple_params[icomplex] + isimple;
      
	if (st_params->simple_track_num == prev_simple_track_num) {
	  *complex_index_p = icomplex;
	  *simple_index_p = isimple;
	  return (0);
	}

      } /* isimple */

    }

  } /* icomplex */

  /*
   * not found
   */
  
  return (-1);

}

/**************************************
 * set_case_track_in_shmem()
 *
 * Sets the case track in shared memory
 */

static void set_case_track_in_shmem(case_track_handle_t *case_handle,
				    time_hist_shmem_t *tshmem,
				    si32 complex_track_num,
				    si32 simple_track_num)

{

  
  /*
   * get the past & future period for partial tracks
   * from the case file if applicable
   */
  
  case_track_t *this_case;
  
  if (case_tracks_find_case(case_handle,
			    tshmem->time,
			    complex_track_num,
			    simple_track_num,
			    &this_case) == 0) {
    
    /*
     * case found
     */
    
    tshmem->case_num = this_case->num;
    tshmem->partial_track_ref_time =
      this_case->ref_time;
    tshmem->partial_track_past_period =
      this_case->ref_minus_start;
    tshmem->partial_track_future_period =
      this_case->end_minus_ref;
    
  } else {
    
    /*
     * no case found
     */
    
    tshmem->case_num = -1;
    tshmem->partial_track_ref_time = tshmem->time;
    tshmem->partial_track_past_period =
      Glob->partial_track_past_period;
    tshmem->partial_track_future_period =
      Glob->partial_track_future_period;
    
  }
  
}

