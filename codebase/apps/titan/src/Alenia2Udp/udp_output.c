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
/*******************************************************************
 * udp_output.c
 *
 * Routines for writing to UDP broadcast.
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * July 1997.
 ********************************************************************/

#include "Alenia2Udp.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <dataport/bigend.h>
#include <dataport/swap.h>
#include <toolsa/membuf.h>
#ifdef __linux
#include <arpa/inet.h>
#endif

static int Udp_fd = -1;
static int Debug;
static MEMbuf *OutBuf = NULL;
static MEMbuf *CopyBuf = NULL;
struct sockaddr_in Out_address;

static void insert_output_code(ui08 type, ui16 len);
static int transmit(int final);
static int write_frame(ui08 *frame, int frame_len);

/********************************************************************
 * open_output_udp()
 *
 * Initialize UDP broadcast output.
 *
 */

int open_output_udp(char *broadcast_address, int port, int debug)

{

  int option;
  struct sockaddr_in local_addr;

  Debug = debug;

  /*
   * open socket
   */

  if  ((Udp_fd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    fprintf(stderr, "Could not open UDP socket, port %d\n", port);
    perror ("socket error:");
    return (-1);
  }
  
  /*
   * bind local address to the socket
   */
  
  MEM_zero(local_addr);
  local_addr.sin_port = htons (port);
  local_addr.sin_family = AF_INET;
  local_addr.sin_addr.s_addr = htonl (INADDR_ANY);
  
  if (bind (Udp_fd, (struct sockaddr *) &local_addr, 
	    sizeof (local_addr)) < 0) {
    fprintf(stderr, "Could bind UDP socket, port %d\n", port);
    perror ("bind error:");
    close (Udp_fd);
    return (-1);
  }
  
  /*
   * set socket for broadcast
   */
  
  option = 1;
  if (setsockopt(Udp_fd, SOL_SOCKET, SO_BROADCAST,
		 (char *) &option, sizeof(option)) < 0) {
    perror ("Could not set broadcast on - setsockopt error");
    return (-1);
  }

  /*
   * set up destination address structure
   */

  MEM_zero(Out_address);
#ifdef SUNOS4
  Out_address.sin_addr.S_un.S_addr = inet_addr(broadcast_address);
#else
  if (inet_aton(broadcast_address, &Out_address.sin_addr) == 0) {
    fprintf(stderr, "Cannot translate address: %s - may be invalid\n",
            broadcast_address);
    return(-1);
  }
#endif
  Out_address.sin_family = AF_INET;
  Out_address.sin_port = htons (port);

  /*
   * set up memory buffers
   */

  OutBuf = MEMbufCreate();
  CopyBuf = MEMbufCreate();
  
  return (0);

}

/**************************
 * start_udp_transmission()
 */

void start_udp_transmission(void)

{

  /*
   * insert start transmission code
   */

  insert_output_code(ALENIA_UDP_START, 0);

}

/***************************************
 * insert data into output stream
 *
 * Call transmit() to send data.
 *
 * Returns 0 on success, -1 on failure.
 */

int write_output_udp(ui08 *data, ui16 len)

{

  insert_output_code(ALENIA_UDP_DATA, len);
  MEMbufAdd(OutBuf, data, len);
  return transmit(FALSE);

}

/**************************
 * stop_udp_transmission()
 *
 * returns 0 on success, -1 on failure.
 */

int stop_udp_transmission(void)

{

  /*
   * insert stop transmission code
   */

  insert_output_code(ALENIA_UDP_STOP, 0);

  /*
   * send remainder of output buffer
   */

  return transmit(TRUE);

}

/******************************************************************
 * close_output_udp()
 */

void close_output_udp(void)

{
  if (Udp_fd >= 0) {
    close(Udp_fd);
    Udp_fd = -1;
    if (Debug) {
      fprintf(stderr, "Closing output UDP\n");
    }
    if (OutBuf != NULL) {
      MEMbufDelete(OutBuf);
      MEMbufDelete(CopyBuf);
    }
  }
}

/*****************************************
 * insert code and length to output buffer
 */

static void insert_output_code(ui08 type, ui16 len)

{

  ui08 scratch[4];
  ui16 LE_len;

  /*
   * make sure len is LittleEndian
   */

  len++;
  if (BE_is_big_endian()) {
    LE_len = SWAP_ui16(len);
  } else {
    LE_len = len;
  }
  
  memcpy(scratch, &LE_len, 2);
  scratch[2] = type;

  MEMbufAdd(OutBuf, scratch, 3);

}

/****************************************
 * transmit
 *
 * Sends out 1024 byte frames at a time.
 *
 * returns 0 on success, -1 on failure
 */

static int transmit(int final)

{

  ui08 *frame;
  int buflen;
  int nframes;
  int nleft;
  int i;

  /*
   * compute number of frames
   */

  buflen = MEMbufLen(OutBuf);
  nframes = buflen / ALENIA_UDP_LEN;
  nleft = buflen - nframes * ALENIA_UDP_LEN;

  if (nframes == 0) {
    return (0);
  }

  /*
   * send out frames
   */

  frame = MEMbufPtr(OutBuf);
  for (i = 0; i < nframes; i++, frame += ALENIA_UDP_LEN) {
    if (write_frame(frame, ALENIA_UDP_LEN)) {
      return (-1);
    }
  } /* i */

  if (final) {

    /*
     * send out remaining data
     */

    if (nleft > 0) {
      if (write_frame(frame, nleft)) {
	return (-1);
      }
    }

    /*
     * reset buffer
     */

    MEMbufReset(OutBuf);

  } else {

    /*
     * pack remaining data to start of buffer
     */
    
    MEMbufReset(CopyBuf);
    MEMbufAdd(CopyBuf, frame, nleft);
    MEMbufReset(OutBuf);
    MEMbufAdd(OutBuf, MEMbufPtr(CopyBuf), MEMbufLen(CopyBuf));

  } /* if (final) */
    
  return (0);

}

/*------------------------------------------------------------------------*/

static int write_frame(ui08 *frame, int frame_len)

{
  
  if (sendto (Udp_fd, (char *) frame, frame_len, 0,
	      (struct sockaddr *) &Out_address, sizeof(Out_address)) ==
      frame_len) {

    return (0);

  } else {

    fprintf(stderr, "write_frame - could not send pkt\n");
    perror ("UDP error");
    return (-1);

  }

}

