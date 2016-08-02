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
 * provide_data
 *
 * Handles a request for data
 *
 * returns 0 on success, -1 on failure
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1992
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "track_server.h"

int provide_data(int sockfd,
		 tdata_request_t *request,
		 storm_file_handle_t *s_handle,
		 track_file_handle_t *t_handle,
		 int *storm_locked_p,
		 int *track_locked_p)

{
  
  char error_text[TDATA_TEXT_LEN];
  
  si32 i;
  si32 *track_set;
  si32 n_current_tracks;
  si32 dtime;

  if (Glob->params.debug) {
    fprintf(stderr, "*** provide_data ***\n");
  }

  if (request->source == TDATA_REALTIME &&
      !Glob->params.realtime_avail) {
    
    if(write_reply(sockfd, TDATA_FAILURE,
		   "No realtime data available")) {
      return (-1);
    }
    
    return (0);
    
  }
  
  if (request->source == TDATA_ARCHIVE) {
    
    if (get_archive_info (s_handle, t_handle,
			  request, &dtime, error_text,
			  storm_locked_p, track_locked_p)) {
      
      if(write_reply(sockfd, TDATA_FAILURE, error_text))
	return (-1);

      return (0);
      
    }
    
  } else {
    
    if (get_realtime_info (request, s_handle, t_handle,
			   &dtime, error_text,
			   storm_locked_p, track_locked_p)) {

      if(write_reply(sockfd, TDATA_FAILURE, error_text))
	return (-1);

      return (0);
      
    }
    
  } /* if (request->source == TDATA_ARCHIVE) */
  
  /*
   * get the set of track numbers which are within the
   * requested time window
   */

  if (request->mode == TDATA_COMPLETE ||
      request->mode == TDATA_BASIC_WITH_PARAMS) {

    if (get_track_set(request->track_set, t_handle,
		      dtime, &n_current_tracks,
		      &track_set, error_text)) {
    
      if (write_reply(sockfd, TDATA_FAILURE, error_text))
	return (-1);
      
      return (0);
      
    } /* if ((track_set = ... */
  
    if (Glob->params.debug >= DEBUG_NORM) {
      
      fprintf(stderr, "ntracks : %ld\n", (long) n_current_tracks);
      fprintf(stderr, "track nums : ");
      
      for (i = 0; i < n_current_tracks; i++)
	fprintf(stderr, "%ld ", (long) track_set[i]);
      
      fprintf(stderr, "\n");
      
    } /* if (Glob->params.debug >= DEBUG_NORM) */

  } /* if (request->mode == TDATA_COMPLETE || ... */
  
  /*
   * send the data
   */
  
  if(write_reply(sockfd, TDATA_SUCCESS, ""))
    return (-1);
  
  if (request->mode == TDATA_COMPLETE) {
    
    if (write_complete_data(sockfd, s_handle, t_handle,
			    dtime, n_current_tracks, track_set))
      return (-1);
    
  } else if (request->mode == TDATA_BASIC_WITH_PARAMS) {
    
    if (write_basic_with_params(request, sockfd, s_handle, t_handle,
				dtime, n_current_tracks, track_set))
      return (-1);
    
  } else if (request->mode == TDATA_BASIC_WITHOUT_PARAMS) {
    
    if (write_basic_without_params(request, sockfd,
				   s_handle, t_handle, dtime))
      return (-1);
    
  }

  return (0);
  
}
