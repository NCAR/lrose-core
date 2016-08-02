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
 * get_track_set.c
 *
 * Determines the set of tracks to be sent ot the client.
 *
 * Sets n_current_tracks_p and track_set_p.
 *
 * Returns 0 on success, -1 on failure
 *
 * RAP, NCAR, Boulder CO
 *
 * April 1992
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "track_server.h"

int get_track_set(si32 track_set_type,
		  track_file_handle_t *t_handle,
		  si32 dtime,
		  si32 *n_current_tracks_p,
		  si32 **track_set_p,
		  char *error_text)

{
     
  static si32 *track_set = (si32 *) NULL;

  int track_valid;

  si32 icomplex;
  si32 ntracks;
  si32 track_num;
  si32 *track_num_array;
  si32 *track_set_ptr;

  /*
   * read the complex and simple track start and end julian
   * times - these are used to determine if a track is a
   * valid candidate for display
   */
  
  if (RfReadTrackUtime(t_handle, "get_track_set") != R_SUCCESS) {

    sprintf(error_text, "Error reading utimes of track file");
    return (-1);
    
  }

  /*
   * alloc track_num_array
   */

  track_num_array = (si32 *) umalloc
    ((ui32) (t_handle->header->n_complex_tracks * sizeof(si32)));
  
  switch (track_set_type) {

  case TDATA_ALL_AT_TIME:
    
    /*
     * get set of complex tracks which occur at given time
     */
    
    ntracks = 0;
    
    for (icomplex = 0;
	 icomplex < t_handle->header->n_complex_tracks; icomplex++) {
      
      track_num = t_handle->complex_track_nums[icomplex];
      
      /*
       * check track times overlap with given time
       */
      
      if ((t_handle->track_utime[track_num].start_complex <= dtime) &&
	  (t_handle->track_utime[track_num].end_complex >= dtime)) {
	
	track_num_array[ntracks] = track_num;
	ntracks++;
	
      } /* if */
      
    } /* icomplex */

    track_set_ptr = track_num_array;

    break;

  case TDATA_ALL_IN_FILE:

    /*
     * get all complex tracks
     */

    track_set_ptr = t_handle->complex_track_nums;
    ntracks = t_handle->header->n_complex_tracks;

    break;

  default:

    /*
     * get a particular track
     */

    /*
     * check that the track number is a valid complex track
     */

    track_valid = FALSE;

    for (icomplex = 0;
	 icomplex < t_handle->header->n_complex_tracks; icomplex++) {

      if (t_handle->complex_track_nums[icomplex] == track_num) {
	track_valid = TRUE;
	break;
      }

    } /* icomplex */

    if (track_valid) {
      track_set_ptr = track_num_array;
      track_num_array[0] = track_num;
      ntracks = 1;
    } else {
      ufree ((char *) track_num_array);
      sprintf(error_text,
	      "%ld is not a valid complex track set",
	      (long) track_set_type);
      return (-1);
    }

  } /* switch (track_set_type) */
    
  /*
   * allocate array to hold track numbers
   */

  if (track_set == NULL) {

    track_set = (si32 *) umalloc
      ((ui32) ntracks * sizeof(si32));

  } else {

    track_set = (si32 *) urealloc
      ((char *) track_set, (ui32) (ntracks * sizeof(si32)));

  }

  /*
   * copy track numbers to track set
   */

  memcpy ((void *) track_set,
          (void *) track_set_ptr,
          (size_t) (ntracks * sizeof(si32)));

  ufree ((char *) track_num_array);

  /*
   * set return vals
   */

  *n_current_tracks_p = ntracks;
  *track_set_p = track_set;

  return (0);

}
