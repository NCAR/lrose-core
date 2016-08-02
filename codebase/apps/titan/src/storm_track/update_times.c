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
 * update_times.c
 *
 * update times for an existing track
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * Sept 1991
 *
 *******************************************************************************/

#include "storm_track.h"

void update_times(date_time_t *dtime,
		  track_utime_t *track_utime,
		  storm_status_t *storms,
		  si32 storm_num)

{
  
  si32 ihist;
  si32 simple_track_num;
  si32 duration_in_secs;
  si32 history_in_secs;
  
  double duration_in_scans;
  double history_in_scans;

  track_status_t *track;
  storm_track_props_t *hist;

  /*
   * set local variables
   */

  track = storms[storm_num].track;
  hist = track->history;
  
  simple_track_num = track->simple_track_num;

  /*
   * update storm track data struct
   */
  
  track->duration_in_scans++;
  track->history_in_scans++;

  /*
   * compute history in seconds, adjusting for the half scan at each
   * end of the track - the scan times refer to the mid time
   */
  
  history_in_scans = (double) track->history_in_scans;

  history_in_secs = compute_history_in_secs(track,
					    dtime,
					    history_in_scans);

  track->history_in_secs = history_in_secs;

  /*
   * compute duration in seconds, adjusting for the half scan at each
   * end of the track - the scan times refer to the mid time
   */
  
  duration_in_scans = (double) track->duration_in_scans;

  if (track->duration_in_scans == 1) {

    duration_in_secs = 0;

  } else {

    duration_in_secs = (si32)
      ((double) (dtime->unix_time -
		 track_utime[simple_track_num].start_simple) *
       (duration_in_scans / (duration_in_scans - 1.0)) + 0.5);
    
  } /* if (duration_in_scans == 1) */

  track->duration_in_secs = duration_in_secs;
  
  /*
   * move history data array up by one slot
   */

  for (ihist = Glob->params.forecast_weights.len - 1; ihist > 0; ihist--)
    memcpy ((void *) (hist + ihist),
            (void *) (hist + ihist - 1),
            (size_t) sizeof(storm_track_props_t));
  
  /*
   * load first history slot
   */

  memcpy ((void *) hist,
          (void *) &storms[storm_num].current,
          (size_t) sizeof(storm_track_props_t));

}
