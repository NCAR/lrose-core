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
 * compute_history_in_secs.c
 *
 * compute history_in_secs from history_in_scans
 *
 * Returns the history_in_secs
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * January 1994
 *
 *******************************************************************************/

#include "storm_track.h"

si32 compute_history_in_secs(track_status_t *track,
			     date_time_t *dtime,
			     si32 history_in_scans)

{
  
  double dhist_in_scans;
  si32 history_in_secs;
  
  /*
   * compute history in seconds, adjusting for the half scan at each
   * end of the track - the scan times refer to the mid time
   */
  
  dhist_in_scans = (double) history_in_scans;
  
  if (history_in_scans == 1) {
    
    history_in_secs = 0;
    
  } else {
    
    history_in_secs = (si32)
      ((double) (dtime->unix_time - track->time_origin.unix_time) *
       (dhist_in_scans / (dhist_in_scans - 1.0)) + 0.5);
    
  } /* if (history_in_scans == 1) */
  
  return (history_in_secs);

}


