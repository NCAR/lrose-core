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
 * process_request.c
 *
 * Processs a request from the client
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
#include <time.h>

#define TIMEOUT_PERIOD 15

int process_request(int sockfd,
		    tdata_request_t *request,
		    int *notification_on,
		    char **message)

{

  static int indices_allocated = FALSE;
  static storm_file_handle_t s_handle;
  static track_file_handle_t t_handle;
  
  char error_text[TDATA_TEXT_LEN];
  int ireturn;
  int storm_locked, track_locked;

  /*
   * initialize the file indices if needed
   */
  
  if (!indices_allocated) {
    
    RfInitStormFileHandle(&s_handle, Glob->prog_name);

    RfInitTrackFileHandle(&t_handle, Glob->prog_name);

    indices_allocated = TRUE;
    
  } /* if (!indices_allocated) */
  
  if (Glob->params.debug >= DEBUG_NORM)
    fprintf(stderr, "child - request code %ld\n", (long) request->request);

  switch(request->request) {

  case TDATA_VERIFY_CONNECT:

    if(write_reply(sockfd, TDATA_SUCCESS, "")) {
      *message = "write_reply failed - verify_connect";
      return(-1);
    }
      
    break;

  case TDATA_REQUEST_NOTIFY:

    if (Glob->params.realtime_avail) {

      if(write_reply(sockfd, TDATA_SUCCESS, "")) {
	*message = "write_reply failed - request_notify realtime";
	return(-1);
      }
      
      *notification_on = TRUE;

    } else {

      if(write_reply(sockfd, TDATA_FAILURE,
		     "No realtime data, so notification not applicable")) {
	*message = "write_reply failed - request_notify archive";
	return(-1);
      }

    } 

    break;

  case TDATA_STOP_NOTIFY:

    if(write_reply(sockfd, TDATA_SUCCESS, "")) {
      *message = "write_reply failed - stop_notify";
      return(-1);
    }
    *notification_on = FALSE;
    break;

  case TDATA_REQUEST_DATA:
  case TDATA_REQUEST_DATA_BEFORE:
  case TDATA_REQUEST_DATA_NEXT_SCAN:
  case TDATA_REQUEST_DATA_PREV_SCAN:

    /*
     * if realtime data requested and not available, let
     * the client know by writing FAILURE reply
     */

    if (request->source == TDATA_REALTIME) {

      if (!Glob->params.realtime_avail) {
	
	if (write_reply(sockfd, TDATA_FAILURE,
			"Realtime track data not available")) {
	  *message = "write_reply failed - no realtime data avail";
	  return(-1);
	}

	return (0);

      } /* if (!Glob->params.realtime_avail) */

    } /* if (request->source == TDATA_REALTIME) */

    /*
     * write current time (GMT) to message queue as last
     * request time
     */

    write_queue(time(NULL));
    
    /*
     * initialize file locking flags - the files get locked in
     * get_realtime_info() or get_archive_info()
     */
    
    storm_locked = FALSE;
    track_locked = FALSE;
    
    ireturn = provide_data(sockfd, request, &s_handle, &t_handle,
			   &storm_locked, &track_locked);

    /*
     * unlock files
     */

    if (storm_locked && (s_handle.header_file != NULL)) {
      if (ta_unlock_file(s_handle.header_file_path,
			 s_handle.header_file)) {
	fprintf(stderr, "WARNING - %s:process_request\n", Glob->prog_name);
      }
      if (Glob->params.debug) {
	fprintf(stderr, "storm header unlocked\n");
      }
    }

    if (track_locked && (t_handle.header_file != NULL)) {
      if (ta_unlock_file(t_handle.header_file_path,
			 t_handle.header_file)) {
	fprintf(stderr, "WARNING - %s:process_request\n", Glob->prog_name);
      }
      if (Glob->params.debug) {
	fprintf(stderr, "track header unlocked\n");
      }
    }

    if(ireturn) {
      *message = "provide_data failed";
      return (-1);
    }

    break;

  case TDATA_SET_MAX_MESSAGE_LEN:

    if (set_max_message_len(request->max_message_len,
			    error_text)) {

      if(write_reply(sockfd, TDATA_FAILURE, error_text)) {
	*message = "write_reply failed - set max len";
	return(-1);
      }
      
    } else {
      
      if(write_reply(sockfd, TDATA_SUCCESS, "")) {
	*message = "write_reply failed - set max len";
	return(-1);
      }
      
    }
    
    break;

  case TDATA_HANGUP:

    if(write_reply(sockfd, TDATA_SUCCESS, "")) {
      *message = "write_reply failed - tdata_hangup";
      return(-1);
    }

    /*
     * return error, so that server will quit
     */

    *message = "server hanging up";
    return(-1);

  } /* switch */

  if (Glob->params.debug >= DEBUG_NORM) {

    if (*notification_on)
      fprintf(stderr, "notification on\n");
    else
      fprintf(stderr, "notification off\n");

  }

  return (0);

}
