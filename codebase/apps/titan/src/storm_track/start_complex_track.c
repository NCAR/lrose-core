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
/*******************************************************************************
 * start_complex_track.c
 *
 * starts up a new complex track, returns the complex track number
 * assigned to the new track
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * October 1991
 *
 *******************************************************************************/

#include "storm_track.h"

si32 start_complex_track(track_file_handle_t *t_handle,
			 track_status_t *track,
			 si32 simple_track_num,
			 track_utime_t *track_utime)

{

  si32 complex_track_num;
  complex_track_params_t *ct_params;
  simple_track_params_t *st_params;

  /*
   * set complex track number and increment n_complex_tracks,
   * checking for array space. The complex track number is set
   * to the current simple track number
   */
  
  complex_track_num = simple_track_num;
  t_handle->header->max_complex_track_num = complex_track_num;
  t_handle->header->n_complex_tracks++;
  
  if (RfAllocTrackComplexArrays(t_handle,
				t_handle->header->n_complex_tracks,
				"start_complex_track")) {
    tidy_and_exit(-1);
  }
  
  t_handle->complex_track_nums[t_handle->header->n_complex_tracks - 1] =
    complex_track_num;

  /*
   * initialize complex track params
   */

  ct_params = t_handle->complex_params;
  st_params = t_handle->simple_params;
  
  memset ((void *)  ct_params,
          (int) 0, (size_t) sizeof(complex_track_params_t));

  ct_params->complex_track_num = complex_track_num;
  ct_params->n_simple_tracks = 1;
  track->n_simple_tracks = ct_params->n_simple_tracks;
  t_handle->simples_per_complex[complex_track_num] = (si32 *) umalloc
    (sizeof(si32));
  t_handle->simples_per_complex[simple_track_num][0] = simple_track_num;
  t_handle->nsimples_per_complex[simple_track_num] =
    ct_params->n_simple_tracks;

  ct_params->start_scan = st_params->start_scan;
  ct_params->end_scan = st_params->end_scan;
  ct_params->duration_in_scans = st_params->duration_in_scans;
  ct_params->duration_in_secs = st_params->duration_in_secs;

  ct_params->start_time = st_params->start_time;
  ct_params->end_time = st_params->end_time;

  /*
   * set start and end julian time arrays
   */

  track_utime[complex_track_num].start_complex =
    track_utime[simple_track_num].start_simple;
  
  track_utime[complex_track_num].end_complex =
    track_utime[simple_track_num].end_simple;
  
  /*
   * write complex track params to file
   */
  
  if (RfWriteComplexTrackParams(t_handle, complex_track_num,
				"start_complex_track")) {
    tidy_and_exit(-1);
  }

  return (complex_track_num);

}
