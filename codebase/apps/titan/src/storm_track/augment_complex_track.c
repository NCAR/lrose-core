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
 * augment_complex_track.c
 *
 * Adds a simple track to a complex track
 *
 * Returns the complex track number
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * October 1991
 *
 *******************************************************************************/

#include "storm_track.h"

si32 augment_complex_track(track_file_handle_t *t_handle,
			   track_status_t *track,
			   si32 simple_track_num,
			   si32 complex_track_num)

{

  complex_track_params_t *ct_params;

  /*
   * read in the complex track params
   */

  if (RfReadComplexTrackParams(t_handle, complex_track_num, FALSE,
			       "augment_complex_tracks"))
    tidy_and_exit(-1);

  ct_params = t_handle->complex_params;

  /*
   * realloc the array for simple track nums
   */
  
  t_handle->simples_per_complex[complex_track_num] = (si32 *) urealloc
    ((char *) t_handle->simples_per_complex[complex_track_num],
     ((ct_params->n_simple_tracks + 1) * sizeof(si32)));
  
  /*
   * update complex params
   */

  t_handle->simples_per_complex[complex_track_num]
    [ct_params->n_simple_tracks] = simple_track_num;
  
  ct_params->n_simple_tracks++;
  track->n_simple_tracks = ct_params->n_simple_tracks;

  t_handle->nsimples_per_complex[complex_track_num] =
    ct_params->n_simple_tracks;

  /*
   * rewrite the complex track params
   */

  if (RfWriteComplexTrackParams(t_handle, complex_track_num,
				"augment_complex_tracks"))
    tidy_and_exit(-1);

  /*
   * set the complex_track_offset to zero because this complex track already
   * has a complex_params slot in the file
   */
  
  t_handle->complex_track_offsets[simple_track_num] = 0;

  return (complex_track_num);

}
