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
/* hilda.c */
/* Read tape, pass to hilda via socket */
/* 18 Nov '94  CloudQuest, Carolina, South Africa */

#include "read_bprp_tape.h"
#include <toolsa/umisc.h>
#include <toolsa/ttape.h>

#define MAX_BLOCK_SIZE 65536
#define PACER_REC_SIZE 512

/*
 * write_response
 *
 * Change the raycount on old tapes before writing out
 *
 * Returns 0 on success, -1 on error
 */

static int write_response(int client_fd,
			  bprp_response_t *response,
			  int old_tape)

{

  bprp_beam_t *beam;
  static int prev_raycount = 0;
  int old_raycount;

  beam = (bprp_beam_t *) response->data;

  if (old_tape) {
    old_raycount = beam->hdr.raycount;
    if (old_raycount == 0 && prev_raycount > 0) {
      beam->hdr.raycount = prev_raycount + 1 - 513;
    } else {
      beam->hdr.raycount = old_raycount - 513;
    }
    prev_raycount = old_raycount;
    beam->hdr.viphi *= 8;
    beam->hdr.viplo *= 8;
  }
  
  if (SKU_write(client_fd, response, sizeof(bprp_response_t), 20) !=
      sizeof(bprp_response_t)) {
    fprintf(stderr, "read_bprp_tape: ERROR writing to client\n");
    return (-1);
  }

  return (0);

}

/*
 * process()
 *
 * returns 0 at end of tape, -1 on error
 */

static int process(int client_fd,
		   char *tape_device,
		   int swap,
		   int old_tape,
		   int wait_msecs)

{

  int tape;
  int forever = TRUE;
  int errcnt;
  int irec, nrecs;
  si32 nread;
  char buffer[MAX_BLOCK_SIZE];
  char *source;
  bprp_response_t response;

  /*
   * open tape device
   */

  if((tape = open(tape_device, O_RDONLY)) < 0) {
    fprintf(stderr, "read_bprp_tape: ERROR opening tape unit\n");
    perror(tape_device);
    return(-1);
  }

  /*
   * set variable block size
   */
  
  if (TTAPE_set_var(tape)) {
    fprintf(stderr, "ERROR - read_bprp_tape - TAPE_set_var\n");
    fprintf(stderr, "Cannot set to variable block size.\n");
    perror(tape_device);
    return(-1);
  }
  
  /*
   * set up response
   */

  memset((void *) &response, 0, sizeof(response));
  response.length = BE_from_ui32(sizeof(response));
  response.magik = BE_from_ui32(RADAR_MAGIK);
  response.reference = BE_from_ui32(0);
  response.version = BE_from_ui32(RADAR_VERSION);
  response.response = BE_from_ui32(RADAR_DATA);
  
  /*
   * loop till error
   */

  while (forever) {

    errcnt = 0;
    nread = 0;

    while (nread <= 0) {
      nread = read (tape, buffer, MAX_BLOCK_SIZE);
      errcnt++;
      if (errcnt > 20) {
	if (TTAPE_fwd_space_file(tape, 1L)) {
	  fprintf(stderr, "read_bprp_tape: Logical end of tape\n");
	  close(tape);
	  return(0);
	} else {
	  fprintf(stderr, "read_bprp_tape: Read error - spacing forward\n");
	}
	errcnt = 0;
      }
    }

    /*
     * compute number of recs
     */

    nrecs = nread / PACER_REC_SIZE;

    /*
     * swap bytes if required
     */

    if (swap) {
      SWAP_array_16((ui16 *) buffer, nread);
    }

    /*
     * send response for each record read
     */

    source = buffer;

    for (irec = 0; irec < nrecs; irec++, source += PACER_REC_SIZE) {

      memcpy((void *) response.data, (void *) source, PACER_REC_SIZE);

      if (write_response(client_fd, &response, old_tape)) {
	close(tape);
	return (-1);
      }

      if (wait_msecs > 0) {
	uusleep(wait_msecs * 1000);
      }

    } /* irec */

  } /* while (forever) */

  close(tape);
  return (0);

}

int main(int argc, char** argv)

{

  char *tape_device;
  int port;
  int swap;
  int old_tape;
  int proto_fd;
  int client_fd;
  int wait_msecs;
  int forever = TRUE;

  /*
   * ignore SIGPIPE - disconnect from client
   */
  
  signal(SIGPIPE, SIG_IGN);
  
  /*
   * parse command line args
   */

  if (parse_args(argc, argv, &tape_device, &port, &swap,
		 &old_tape, &wait_msecs)) {
    return (-1);
  }

  /*
   * open prototype fd for client side
   */
    
  if ((proto_fd = SKU_open_server(port)) < 0) {
    fprintf(stderr, "ERROR - read_bprp_tape\n");
    fprintf(stderr, "Cannot open port %d for output\n", port);
    perror("");
    return(-1);
  }
    
  while (forever) {

    if ((client_fd = SKU_get_client(proto_fd)) < 0) {
      
      /*
       * no client - error
       */

      return (-1);

    }

    if (process(client_fd, tape_device, swap, old_tape, wait_msecs) == 0) {
      return (0);
    } else {
      fprintf(stderr, "read_bprp_tape: socket error - retrying\n");
    }

    sleep(1);
    close(client_fd);

  } /* while */

  return (0);

}
