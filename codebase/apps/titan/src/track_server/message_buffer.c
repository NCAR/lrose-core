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
 * message_buffer.c
 *
 * Routines which handle the message buffer for the server
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

static char *Write_buffer = NULL;
static si32 Max_message_len = TDATA_DEFAULT_MAX_MESSAGE_LEN;
static si32 Write_posn = 0;
static si32 Seq_no = 0;

/*********************************************************************
 * set_max_message_len()
 */

int set_max_message_len(si32 len,
			char *error_text)

{
  
  if (len < TDATA_LOWEST_MAX_MESSAGE_LEN) {

    sprintf(error_text,
	    "Requested max len %ld less than minimum of %d",
	    (long) len, TDATA_LOWEST_MAX_MESSAGE_LEN);

    return (-1);

  }

  if (Write_buffer == NULL) {

    Write_buffer = (char *) umalloc((ui32) len);
    
  } else {
    
    Write_buffer = (char *) urealloc(Write_buffer, (ui32) len);
    
  }

  Max_message_len = len;

  return (0);
  
}

/*********************************************************************
 * write_to_buffer()
 *
 * Writes data to buffer, sending packets to the client as the
 * buffer becomes full
 *
 * If id < 0, don't put in struct header
 *
 * returns 0 on success, -1 on failure
 *
 *********************************************************************/

int write_to_buffer(int sockfd,
		    char *data,
		    si32 nbytes,
		    int id)

{
 
  si32 new_bytes;

  tdata_struct_header_t struct_header;

  if (id < 0)
    new_bytes = nbytes;
  else
    new_bytes = nbytes + sizeof(tdata_struct_header_t);
  
  if (new_bytes > Max_message_len) {
    
    fprintf(stderr, "ERROR - track_server:write_to_buffer\n");
    fprintf(stderr, "Max message length %ld too low\n",
	    (long) Max_message_len);
    fprintf(stderr, "Increase to at least %ld\n", (long) new_bytes);
    return(-1);
    
  } 

  /*
   * check that buffer has been allocated
   */
  
  if (Write_buffer == NULL)
    Write_buffer = (char *) umalloc((ui32) Max_message_len);
  
  /*
   * if this write will overflow buffer, write it out
   */

  if (Write_posn + new_bytes > Max_message_len) {

    if (flush_write_buffer(sockfd))
      return (-1);
    
  }

  /*
   * write struct header for id >= 0
   */

  if (id >= 0) {
    
    struct_header.id = BE_from_si32((ui32) id);
    
    memcpy ((void *) (Write_buffer + Write_posn),
	    (void *) &struct_header,
	    (size_t) sizeof(tdata_struct_header_t));

    Write_posn += sizeof(tdata_struct_header_t);
    
  }

  /*
   * write data
   */
  
  memcpy ((void *) (Write_buffer + Write_posn),
          (void *) data,
          (size_t) nbytes);
  
  Write_posn += nbytes;

  return (0);

}

/*********************************************************************
 * flush_write_buffer()
 *
 * If there is data in the buffer, sends it out to the client
 *
 * returns 0 on success, -1 on failure
 *
 *********************************************************************/

int flush_write_buffer(int sockfd)

{

  if (Write_posn > 0) {

    if(SKU_writeh(sockfd, Write_buffer,
		  Write_posn,
		  TDATA_DATA_PACKET_ID,
		  Seq_no) < 0)
      return(-1);

    Seq_no++;
    Write_posn = 0;

  }

  return (0);

}

