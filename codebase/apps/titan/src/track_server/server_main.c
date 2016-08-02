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
 * server_main.c
 *
 * Main server function
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1992
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "track_server.h"

#define WAIT_NORMAL 1
#define WAIT_HANDSHAKE 5

void server_main(int sockfd)

{

  int notification_on = FALSE;
  int forever = TRUE;

  char *error_text;

  struct timeval wait_time;
  fd_set read_fd;
  tdata_request_t request;

  /*
   * check for connect handshake from client - if
   * none after a few seconds, the client is in
   * product mode, and we need to use server_product.
   * This is a mode in which the client just reads
   * the incoming data, and does not receive notification.
   * There is no handshaking in this mode.
   */

  wait_time.tv_usec = 0;
  wait_time.tv_sec = WAIT_HANDSHAKE;
  
  FD_ZERO(&read_fd);
  FD_SET(sockfd, &read_fd);

  if (select(FD_SETSIZE, FD_SET_P &read_fd,
	     FD_SET_P 0, FD_SET_P 0, &wait_time) <= 0) {

    /*
     * no handshaking - use product mode
     */

    server_product(sockfd);
    return;

  }

  /*
   * hang on a select, which will return as soon as there
   * is something to be read on the socket, or if the
   * client goes down
   */

  while (forever) {

    /*
     * set up the select mask for reading on the socket
     */

    FD_ZERO(&read_fd);
    FD_SET(sockfd, &read_fd);
    
    /*
     * set wait time for select to time out
     */
    
    wait_time.tv_usec = 0;
    wait_time.tv_sec = WAIT_NORMAL;

    /*
     * hang on select, waiting for incoming requests
     */
    
    if (select(FD_SETSIZE, FD_SET_P &read_fd,
	       FD_SET_P 0, FD_SET_P 0, &wait_time) > 0) {

      /*
       * read the request
       */

      if (read_request(sockfd, &request)) {

	/*
	 * read failed - this means that the client has gone
	 * down, so return
	 */

	if (Glob->params.debug >= DEBUG_NORM)
	  fprintf(stderr, "Client died, so server will exit.\n");

	return;

      } else {

	if (process_request(sockfd, &request,
			    &notification_on, &error_text)) {

	  /*
	   * process request failed, client probably down or
	   * server hanging up
	   */
	  
	  if (Glob->params.debug >= DEBUG_NORM &&
	      strcmp(error_text, "server hanging up")) {
	    fprintf(stderr, "ERROR - %s:server_main:process_request\n",
		   Glob->prog_name);
	    fprintf(stderr, "%s\n", error_text);
	  }
	  
	  return;
	  
	}

      } /* if (read_request(sockfd, ...  */

    } else {

      /*
       * check for new data if applicable
       */

      if (new_data()) {
	
	/*
	 * notify as applicable
	 */
	
	if (notification_on) {

	  if (Glob->params.debug >= DEBUG_NORM)
	    fprintf(stderr, "Notifying new data available\n");

	  if(write_notify(sockfd)) {

	    /*
	     * write failed, client probably down
	     */

	    return;

	  }

	} /* if (notification_on) */
	
      } /* if (new_data()) */

    } /* if (select(FD_SETSIZE, &read_fd, ... */

  } /* while (forever) */

}

