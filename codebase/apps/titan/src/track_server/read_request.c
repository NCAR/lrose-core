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
 * read_request.c
 *
 * Reads a request packet from the client
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
#include <netinet/in.h>
#include <toolsa/sockutil.h>
#include <toolsa/xdru.h>

int read_request(int sockfd,
		 tdata_request_t *request)

{
  
  SKU_header_t header;
  
  /*
   * read the request struct
   */
  
  if (SKU_readh(sockfd, (char *) request,
		(si32) sizeof(tdata_request_t), &header, -1L) < 0)
    return (-1);
  
  if (header.id != TDATA_REQUEST_PACKET_ID) {
    
    fprintf(stderr, "ERROR - read_reply\n");
    fprintf(stderr, "Packet out of order.\n");
    fprintf(stderr, "Expecting request packet.\n");
    fprintf(stderr, "Packet header id %d\n", header.id);
    exit(-1);
    
  }
  
  /*
   * get bytes into host byte order
   */
  
  BE_to_array_32((ui32 *) request,
		 (si32) sizeof(tdata_request_t));
  
  return (0);
  
}
