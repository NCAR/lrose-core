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
 * read_reply.c
 *
 * Reads a reply packet from the client
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

#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <toolsa/sockutil.h>
#include <titan/tdata_server.h>

extern void exit(int);

int read_reply(int sockfd, tdata_reply_t *reply)
{

  SKU_header_t header;

  /*
   * read in the reply
   */

 read_another:

  if (SKU_readh(sockfd, (char *) reply,
		(si32) sizeof(tdata_reply_t), &header, -1L) < 0)
    return (-1);

  /*
   * if this is a notification packet, discard and read another
   */

  if (header.id == TDATA_NOTIFY_PACKET_ID)
    goto read_another;
  
  if (header.id != TDATA_REPLY_PACKET_ID) {
    
    fprintf(stderr, "ERROR - read_reply\n");
    fprintf(stderr, "Packet out of order.\n");
    fprintf(stderr, "Expecting reply packet.\n");
    fprintf(stderr, "Packet header id %d\n", header.id);
    exit(-1);
    
  }

  /*
   * get bytes into host byte order
   */

  reply->status = BE_to_si32((ui32) reply->status);
  
  return (0);
  
}
