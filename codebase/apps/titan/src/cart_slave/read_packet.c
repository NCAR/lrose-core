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
 * read_packet.c: reads a packet
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "cart_slave.h"

int read_packet(si32 *packet_id)

{
  
  int read_success;
  int nread;
  SKU_header_t packet_header;

  /*
   * read a new packet into the buffer
   */

  *packet_id = -1;
  read_success = TRUE;
  
  /*
   * read header with 10 sec timeout
   */

  while (SKU_read_header(Glob->sock_dev, &packet_header, 10000) < 0) {
    /*
     * timeout
     */
    PMU_auto_register("Waiting for header");
    sleep(1);
  }
  PMU_auto_register("Reading header");
  
  if (read_success) {

    if (Glob->params.debug) {
      fprintf(stderr, "Got packet\n");
    }

    *packet_id = packet_header.id;

    if (packet_header.len > Glob->params.max_packet_length) {

      fprintf(stderr, "ERROR - %s:read_packet\n", Glob->prog_name);
      fprintf(stderr,
	      "Size of packet exceeds max packet length\n");
      fprintf(stderr, "Size of packet is %ld\n",
	      (long) packet_header.len);
      fprintf(stderr, "Max packet length set to %ld\n",
	      Glob->params.max_packet_length);
      fprintf(stderr, "Reset max_packet_length in params file \n");
      fprintf(stderr, "Check server max_packet_length parameters too.\n");
      tidy_and_exit(-1);
      
    }
  
    /*
     * read in packet, timing out after 10 secs each time
     */

    while ((nread = SKU_read_timed(Glob->sock_dev, Glob->packet,
				   packet_header.len,
				   -1, 10000)) < 0) {
      /*
       * timeout
       */
      PMU_auto_register("Waiting for packet");
    }
    PMU_auto_register("Reading packet");
    
    if (nread != packet_header.len)
      read_success = FALSE;
    
  } /* if (read_success) */
  
  if (read_success) {

    return(CS_SUCCESS);
      
  } else {

    /*
     * server is down, restart cart_slave because the incoming
     * data parameters may change
     */
    
    tidy_and_exit(RESTART_SIG);
    return (CS_FAILURE);

  } /* if (read_success) */
  
}
