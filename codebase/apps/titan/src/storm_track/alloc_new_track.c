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
/*****************************************************************************
 * alloc_new_track.c
 *
 * allocates memory for a new track, and creates track params in the track
 * file for this track
 *
 * returns the simple track number
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * October 1991
 *
 ******************************************************************************/

#include "storm_track.h"

si32 alloc_new_track(track_file_handle_t *t_handle,
		     date_time_t *dtime,
		     track_utime_t *track_utime,
		     storm_status_t *storm,
		     si32 scan_num,
		     int new_complex_track,
		     si32 complex_track_num,
		     si32 history_in_scans,
		     si32 scan_origin,
		     date_time_t *time_origin)

{

  si32 simple_track_num;
  simple_track_params_t *st_params;
  track_status_t *track;

  /*
   * set track number and increment ntracks,
   * checking for array space
   */
  
  simple_track_num = t_handle->header->n_simple_tracks;
  t_handle->header->max_simple_track_num = simple_track_num;
  t_handle->header->n_simple_tracks++;
  
  if (RfAllocTrackSimpleArrays(t_handle,
			       t_handle->header->n_simple_tracks,
			       "alloc_new_track")) {
    tidy_and_exit(-1);
  }
  
  if (RfAllocSimplesPerComplex(t_handle,
			       t_handle->header->n_simple_tracks,
			       "alloc_new_track")) {
    tidy_and_exit(-1);
  }
  
  if (Glob->params.debug >= DEBUG_EXTRA)
    fprintf(stderr, "\nStarting simple track %ld\n",
	    (long) simple_track_num);

  /*
   * initialize track_data struct
   */

  storm->track = (track_status_t *) ucalloc
    ((ui32) 1, (ui32) sizeof(track_status_t));

  track = storm->track;

  track->history = (storm_track_props_t *) ucalloc
    ((ui32) MAX_NWEIGHTS_FORECAST,
     (ui32) sizeof(storm_track_props_t));

  track->simple_track_num = simple_track_num;
  track->duration_in_scans = 0;
  track->history_in_scans = history_in_scans;
  track->scan_origin = scan_origin;

  memcpy ((void *) &track->time_origin,
          (void *) time_origin,
          (size_t) sizeof(date_time_t));

  /*
   * compute history in seconds
   */
  
  track->history_in_secs =
    compute_history_in_secs(track,
			    dtime,
			    history_in_scans);

  /*
   * set up the track params
   */

  st_params = t_handle->simple_params;

  memset ((void *)  st_params,
          (int) 0, (size_t) sizeof(simple_track_params_t));
  
  st_params->simple_track_num = simple_track_num;
  st_params->start_scan = scan_num;
  st_params->end_scan = scan_num;
  st_params->duration_in_scans = 0;
  st_params->duration_in_secs = 0;
  st_params->history_in_scans = history_in_scans;
  st_params->scan_origin = scan_origin;
  
  st_params->start_time = dtime->unix_time;
  st_params->end_time = dtime->unix_time;
  st_params->time_origin = time_origin->unix_time;

  track_utime[simple_track_num].start_simple = dtime->unix_time;
  track_utime[simple_track_num].end_simple = dtime->unix_time;

  if (new_complex_track) {

    track->complex_track_num =
      start_complex_track(t_handle, track,
			  simple_track_num,
			  track_utime);

    if (Glob->params.debug >= DEBUG_EXTRA)
      fprintf(stderr, "starting complex track %ld\n",
	      (long) track->complex_track_num);

  } else {
    
    track->complex_track_num =
      augment_complex_track(t_handle, track,
			    simple_track_num,
			    complex_track_num);

    if (Glob->params.debug >= DEBUG_EXTRA)
      fprintf(stderr, "augmenting complex track %ld\n",
	      (long) track->complex_track_num);

  } /* if (new_complex_track) */

  st_params->complex_track_num = track->complex_track_num;

  /*
   * write simple track params to file
   */
  
  if (RfWriteSimpleTrackParams(t_handle,
			       simple_track_num,
			       "alloc_new_track")) {
    tidy_and_exit(-1);
  }

  return (simple_track_num);

}

