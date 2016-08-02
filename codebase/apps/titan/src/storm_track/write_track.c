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
/**************************************************************************
 * write_track.c
 *
 * write out an existing track
 *
 * Returns the offset to the entry written
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * Oct 1994
 *
 *******************************************************************************/

#include "storm_track.h"

si32 write_track(storm_file_handle_t *s_handle,
		 track_file_handle_t *t_handle,
		 date_time_t *dtime,
		 track_utime_t *track_utime,
		 storm_status_t *storms,
		 si32 storm_num,
		 si32 scan_num)
     
{
  
  static si32 prev_scan_entry_offset;

  si32 simple_track_num;
  si32 complex_track_num;
  si32 entry_offset;
  
  simple_track_params_t *st_params;
  complex_track_params_t *ct_params;
  track_file_entry_t *t_entry;
  track_status_t *track;

  /*
   * set local variables
   */

  track = storms[storm_num].track;
  
  simple_track_num = track->simple_track_num;
  complex_track_num = track->complex_track_num;

  /*
   * set up entry
   */
  
  t_entry = t_handle->entry;

  t_entry->simple_track_num = simple_track_num;
  t_entry->complex_track_num = complex_track_num;
  t_entry->scan_num = scan_num;
  t_entry->storm_num = storm_num;
  t_entry->history_in_scans = track->history_in_scans;
  t_entry->history_in_secs = track->history_in_secs;
  t_entry->scan_origin = track->scan_origin;
  t_entry->duration_in_scans = track->duration_in_scans;
  t_entry->duration_in_secs = track->duration_in_secs;
  t_entry->time = s_handle->scan->time;
  t_entry->time_origin = track->time_origin.unix_time;

  /*
   * load up forecast rates of change
   */

  t_entry->dval_dt.proj_area_centroid_x = track->dval_dt.proj_area_centroid_x;
  t_entry->dval_dt.proj_area_centroid_y = track->dval_dt.proj_area_centroid_y;
  t_entry->dval_dt.vol_centroid_z = track->dval_dt.vol_centroid_z;
  t_entry->dval_dt.refl_centroid_z = track->dval_dt.refl_centroid_z;
  t_entry->dval_dt.top = track->dval_dt.top;
  t_entry->dval_dt.dbz_max = track->dval_dt.dbz_max;
  t_entry->dval_dt.volume = track->dval_dt.volume;
  t_entry->dval_dt.precip_flux = track->dval_dt.precip_flux;
  t_entry->dval_dt.mass = track->dval_dt.mass;
  t_entry->dval_dt.proj_area = track->dval_dt.proj_area;
  t_entry->dval_dt.smoothed_proj_area_centroid_x =
    track->dval_dt.smoothed_proj_area_centroid_x;
  t_entry->dval_dt.smoothed_proj_area_centroid_y =
    track->dval_dt.smoothed_proj_area_centroid_y;
  t_entry->dval_dt.smoothed_speed = track->dval_dt.smoothed_speed;
  t_entry->dval_dt.smoothed_direction = track->dval_dt.smoothed_direction;

  /*
   * set forecast_valid flag
   */

  t_entry->forecast_valid = TRUE;
  
  if (t_entry->history_in_secs <
      Glob->params.min_history_for_valid_forecast) {
    t_entry->forecast_valid = FALSE;
  }
  
  if (track->dval_dt.smoothed_speed >
      Glob->params.max_speed_for_valid_forecast) {
    t_entry->forecast_valid = FALSE;
  }
  
  if (Glob->params.debug >= DEBUG_EXTRA) {
    fprintf(stderr, "Storm %d, forecast_valid %s\n",
	    storm_num,
	    (t_entry->forecast_valid? "TRUE" : "FALSE"));
  }

  /*
   * write this entry to file, returning the offset of the
   * entry in the file. The track->entry_offset tells the
   * write routine where to seek for the previous entry
   * written. entry_offset is the offset for this entry.
   */
  
  if (storm_num == 0)
    prev_scan_entry_offset = 0;
  
  entry_offset =
    RfWriteTrackEntry(t_handle,
		      track->entry_offset,
		      prev_scan_entry_offset,
		      "write_track");
  
  if (entry_offset == R_FAILURE)
    tidy_and_exit(-1);
  
  track->entry_offset = entry_offset;
  prev_scan_entry_offset = entry_offset;

  /*
   * read the track params back in, update and rewrite
   */
  
  if (RfReadSimpleTrackParams(t_handle, simple_track_num,
			      "write_track"))
    tidy_and_exit(-1);
  
  st_params = t_handle->simple_params;

  st_params->end_scan = scan_num;
  st_params->history_in_scans = track->history_in_scans;
  st_params->history_in_secs = track->history_in_secs;
  st_params->duration_in_scans =
    st_params->end_scan - st_params->start_scan + 1;
  st_params->duration_in_secs = track->duration_in_secs;
  
  if (track->duration_in_scans == 1) {
    st_params->first_entry_offset = track->entry_offset;
  }
  
  st_params->end_time = dtime->unix_time;
  track_utime[simple_track_num].end_simple = dtime->unix_time;
  
  if (RfWriteSimpleTrackParams(t_handle, simple_track_num,
			       "write_track"))
    tidy_and_exit(-1);

  /*
   * update the complex track
   */

  if (RfReadComplexTrackParams(t_handle, complex_track_num, FALSE,
			       "write_track"))
    tidy_and_exit(-1);

  /*
   * update this complex track if it has not already been
   * brought up to date
   */

  ct_params = t_handle->complex_params;

  if ((ct_params->end_scan != scan_num) ||
      ct_params->duration_in_scans == 0) {
    
    ct_params->end_scan = scan_num;

    ct_params->duration_in_scans = 
      ct_params->end_scan - ct_params->start_scan + 1;

    track_utime[complex_track_num].end_complex = dtime->unix_time;
      
    if (ct_params->duration_in_scans == 1) {

      ct_params->duration_in_secs = 0;

    } else {

      ct_params->duration_in_secs = (si32)
	(((double) dtime->unix_time -
	  (double) track_utime[complex_track_num].start_complex) *
	 (ct_params->duration_in_scans /
	  (ct_params->duration_in_scans - 1.0)) + 0.5);

    }
      
    ct_params->end_time = dtime->unix_time;

    /*
     * rewrite the amended track params
     */
  
    if (RfWriteComplexTrackParams(t_handle, complex_track_num,
				  "write_track"))
      tidy_and_exit(-1);

  } /* if (ct_params->end_scan != scan_num) */

  return (entry_offset);

}
