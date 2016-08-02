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
 * server_product.c
 *
 * Product server function - one-way communications
 *
 * RAP, NCAR, Boulder CO
 *
 * Jan 1993
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "track_server.h"

static int connection_down(int sd)
     
{

  int maxfdp1;
  fd_set read_fd;
  
  struct timeval wait;
  
  /* set timeval structure */
  wait.tv_sec = 0;
  wait.tv_usec = 100;
  
  /* listen only on sd socket */
  FD_ZERO(&read_fd);
  FD_SET(sd, &read_fd);
  maxfdp1 = sd + 1;

  if (0 != select(maxfdp1, &read_fd, NULL, NULL, &wait)) {
    return (1); /* select failed - connection down */
  } else {
    return (0);
  }

}

void server_product(int sockfd)

{

  char error_text[TDATA_TEXT_LEN];
  
  int forever = TRUE;
  int storm_locked, track_locked;

  si32 dtime;
  
  tdata_request_t request;
  static storm_file_handle_t s_handle;
  static track_file_handle_t t_handle;
  
  /*
   * initialize the file indices
   */
  
  RfInitStormFileHandle(&s_handle, Glob->prog_name);

  RfInitTrackFileHandle(&t_handle, Glob->prog_name);

  /*
   * set up the request
   */

  memset ((void *) &request,
	  (int) 0,
	  (size_t) sizeof(tdata_request_t));

  request.request = TDATA_REQUEST_DATA;
  request.mode = TDATA_PRODUCT;
  request.source = TDATA_REALTIME;
  request.track_set = TDATA_ALL_AT_TIME;
  request.target_entries = TDATA_CURRENT_ENTRIES_ONLY;

  /*
   * loop waiting for new data
   */
  
  while (forever) {

    /*
     * check for new data if applicable
     */
    
    if (new_data()) {
      
      if (Glob->params.debug >= DEBUG_NORM)
	fprintf(stderr, "New data available\n");
      
      /*
       * initialize file locking flags - the files get locked in
       * get_realtime_info()
       */
      
      storm_locked = FALSE;
      track_locked = FALSE;
    
      if (get_realtime_info (&request, &s_handle, &t_handle,
			     &dtime, error_text,
			     &storm_locked, &track_locked)) {

	fprintf(stderr, "%s:server_product\n", Glob->prog_name);
	fprintf(stderr, "%s\n", error_text);

      } else {
	
	if (write_product(sockfd,
			  &s_handle,
			  &t_handle,
			  dtime)) {

	  fprintf(stderr, "%s:server_product\n", Glob->prog_name);
	  fprintf(stderr, "Error writing product\n");

	} /* if (write_product(... */

      } /* if (get_realtime_info (... */

      /*
       * unlock files
       */
      
      if (storm_locked && (s_handle.header_file != NULL)) {
	if (ta_unlock_file(s_handle.header_file_path,
			   s_handle.header_file)) {
	  fprintf(stderr, "WARNING - %s:server_product\n", Glob->prog_name);
	}
      }
      
      if (track_locked && (t_handle.header_file != NULL)) {
	if (ta_unlock_file(t_handle.header_file_path,
			   t_handle.header_file)) {
	  fprintf(stderr, "WARNING - %s:server_product\n", Glob->prog_name);
	}
      }
      
    } /* if (new_data()) */

    sleep (1);

    if (connection_down(sockfd)) {
      fprintf(stderr, "track server exiting - broken connection\n");
      return;
    }
    
  } /* while (forever) */

}
