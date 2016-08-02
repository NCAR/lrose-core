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
/***************************************************************************
 * read_udp_beam.c
 *
 * loads a radar beam of lincoln format from the ethernet into the buffer
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * December 1990
 *
 * Hacked from 'getray' code
 *
 ****************************************************************************/

#include "trec.h"

#include <sys/socket.h>
#include <netinet/in.h>

#include <toolsa/sockutil.h>

#define MAX_BEAM_REC_SIZE 65536
#define MAX_UDP_PACKET_BYTES 1512

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*
 * file scope prototypes
 */

static int get_udp_pkt(int sfd, char *inbuf);

static void print_ll_frame_hdr(char *label,
			       ll_udp_frame_hdr_t * lfhdr);

static void print_ncar_frame_hdr(char *label,
				 ncar_udp_frame_hdr_t * lfhdr);

static int open_udp (int port);

/****************************************************************************/

char *read_ll_udp_beam (int udp_port, int debug, char *prog_name)

{

  static char *inbuf;
  static char *outbuf;
  static ll_udp_frame_hdr_t *frame_hdr;
  static int Udp_fd;
  static int first_call = TRUE;
  static int max_out_frames_alloc;

  char *outptr;
  int packet_len;
  int offset;
  int data_len;
  int nframes;
  int seq_num;
  int i;

  ll_params_t *ll_params;
  ll_pkt_gate_t *ll_pkt_gate;

  /*
   * if first call, initialize
   */

  if (first_call) {

    inbuf = (char *) umalloc((ui32) MAX_BEAM_REC_SIZE);
    outbuf = (char *) umalloc((ui32) MAX_BEAM_REC_SIZE);
    max_out_frames_alloc = 1;
    
    frame_hdr = (ll_udp_frame_hdr_t *) inbuf;
    
    /*
     * open udp
     */

    if ((Udp_fd = open_udp(udp_port)) < 0) {
      close(Udp_fd);
      fprintf(stderr, "ERROR - %s:read_udp_beam\n", prog_name);
      fprintf(stderr, "Opening udp port, %d\n", udp_port);
      perror(" ");
      tidy_and_exit(-1);
    }

    first_call = FALSE;

  }

 try_again:

  /*
   * Read the first packet of beam off the net
   */

  frame_hdr->frame_this_pkt = 0;

  while (frame_hdr->frame_this_pkt != 1) {

    packet_len = get_udp_pkt(Udp_fd, inbuf);
  
    /*
     * Check the condition code returned by the read
     */
    
    if(packet_len < 0) {
      fprintf(stderr, "ERROR - %s:read_udp_beam\n", prog_name);
      fprintf(stderr, "Reading ethernet - packet_len = %d\n", packet_len);
      perror("");
      sleep(1);
    }
  
  }

  nframes = frame_hdr->frames_per_beam;
  seq_num = frame_hdr->frame_seq_num;

  if (debug) {
    print_ll_frame_hdr("Initial packet", frame_hdr);
  }

  /*
   * realloc as necessary
   */

  if (nframes > max_out_frames_alloc) {
    outbuf = (char *) urealloc((void *) outbuf,
			       (ui32) (MAX_BEAM_REC_SIZE *
				       nframes));
    max_out_frames_alloc = nframes;
  }

  /*
   * copy ll_params to outbuf
   */
  
  outptr = outbuf;

  offset = sizeof(ll_udp_frame_hdr_t);
  ll_params = (ll_params_t *) (inbuf + offset);
  memcpy((void *) outptr, (void *) ll_params, sizeof(ll_params_t));

  outptr += sizeof(ll_params_t);
  offset += sizeof(ll_params_t);
  
  ll_pkt_gate = (ll_pkt_gate_t *) (inbuf + offset);
  
  if (debug) {
    fprintf(stderr, "pkt_start_gate: %d\n", ll_pkt_gate->pkt_start_gate);
    fprintf(stderr, "pkt_ngates: %d\n", ll_pkt_gate->pkt_ngates);
    fprintf(stderr, "\n");
  }

  /*
   * copy first packet data to outbuf
   */
  
  offset += sizeof(ll_pkt_gate_t);
  data_len = packet_len - offset - sizeof(ll_checksum_t);
  
  memcpy((char *) outptr, inbuf + offset, data_len);
  outptr += data_len;

  if (debug) {
    fprintf(stderr, "Packet, len %d, comprises the following:-\n",
	    (int) packet_len);
    fprintf(stderr, "ll_udp_frame_hdr_t: %d bytes\n",
	    (int) sizeof(ll_udp_frame_hdr_t));
    fprintf(stderr, "ll_params: %d bytes\n", (int) sizeof(ll_params_t));
    fprintf(stderr, "ll_pkt_gate_t: %d bytes\n", (int) sizeof(ll_pkt_gate_t));
    fprintf(stderr, "data: %d bytes, %d gates\n",
	    (int) data_len, (int) (data_len / LL_NFIELDS));
    fprintf(stderr, "checksum: %d bytes\n", (int) sizeof(ll_checksum_t));
    fprintf(stderr, "\n");
  }

  for (i = 0; i < nframes - 1; i++) {
    
    packet_len = get_udp_pkt(Udp_fd, inbuf);
  
    /*
     * Check the condition code returned by the read
     */
    
    if(packet_len < 0) {
      fprintf(stderr, "ERROR - %s:read_udp_beam\n", prog_name);
      fprintf(stderr, "Reading ethernet - packet_len = %d\n", packet_len);
      sleep(1);
      goto try_again;
    }

    if (debug) {
      print_ll_frame_hdr("Subsequent packet", frame_hdr);
    }
    
    seq_num++;
    if (frame_hdr->frame_seq_num != seq_num) {
      fprintf(stderr, "ERROR - %s:read_udp_beam\n", prog_name);
      fprintf(stderr, "Packet seq out of order\n");
      goto try_again;
    }

    offset = sizeof(ll_udp_frame_hdr_t);
    ll_pkt_gate = (ll_pkt_gate_t *) (inbuf + offset);
    
    if (debug) {
      fprintf(stderr, "pkt_start_gate: %d\n", ll_pkt_gate->pkt_start_gate);
      fprintf(stderr, "pkt_ngates: %d\n", ll_pkt_gate->pkt_ngates);
      fprintf(stderr, "\n");
    }
    
    /*
     * copy packet data to outbuf
     */
    
    offset += sizeof(ll_pkt_gate_t);
    data_len = packet_len - offset - sizeof(ll_checksum_t);
    
    memcpy((char *) outptr, inbuf + offset, data_len);
    outptr += data_len;
    
    if (debug) {
      fprintf(stderr, "Packet, len %d, comprises the following:-\n", packet_len);
      fprintf(stderr, "ll_udp_frame_hdr_t: %d bytes\n",
	      (int) sizeof(ll_udp_frame_hdr_t));
      fprintf(stderr, "ll_pkt_gate_t: %d bytes\n",
	      (int) sizeof(ll_pkt_gate_t));
      fprintf(stderr, "data: %d bytes, %d gates\n",
	      (int) data_len, (int) (data_len / LL_NFIELDS));
      fprintf(stderr, "checksum: %d bytes\n", (int) sizeof(ll_checksum_t));
      fprintf(stderr, "\n");
    }

  } /* i */

  return (outbuf);

}

/****************************************************************************/

char *read_ncar_udp_beam (int udp_port, int debug, char *prog_name)

{

  static char *inbuf;
  static char *outbuf;
  static ncar_udp_frame_hdr_t *frame_hdr;
  static int Udp_fd;
  static int first_call = TRUE;
  static int max_out_frames_alloc;

  char *outptr;
  int packet_len;
  int offset;
  int data_len;
  int nframes;
  int seq_num;
  int i;

  ll_params_t *ll_params;

  /*
   * if first call, initialize
   */

  if (first_call) {

    inbuf = (char *) umalloc((ui32) MAX_BEAM_REC_SIZE);
    outbuf = (char *) umalloc((ui32) MAX_BEAM_REC_SIZE);
    max_out_frames_alloc = 1;
    
    frame_hdr = (ncar_udp_frame_hdr_t *) inbuf;
    
    /*
     * open udp
     */

    if ((Udp_fd = open_udp(udp_port)) < 0) {
      close(Udp_fd);
      fprintf(stderr, "ERROR - %s:read_udp_beam\n", prog_name);
      fprintf(stderr, "Opening udp port, %d\n", udp_port);
      perror(" ");
      tidy_and_exit(-1);
    }

    first_call = FALSE;

  }

 try_again:

  /*
   * Read the first packet of beam off the net
   */

  frame_hdr->frame_this_pkt = 0;

  while (frame_hdr->frame_this_pkt != 1) {

    packet_len = get_udp_pkt(Udp_fd, inbuf);
  
    /*
     * Check the condition code returned by the read
     */
    
    if(packet_len < 0) {
      fprintf(stderr, "ERROR - %s:read_udp_beam\n", prog_name);
      fprintf(stderr, "Reading ethernet - packet_len = %d\n", packet_len);
      perror("");
      sleep(1);
    }
  
  }

  nframes = frame_hdr->frames_per_beam;
  seq_num = frame_hdr->frame_seq_num;

  if (debug) {
    print_ncar_frame_hdr("Initial packet", frame_hdr);
  }

  /*
   * realloc as necessary
   */

  if (nframes > max_out_frames_alloc) {
    outbuf = (char *) urealloc((void *) outbuf,
			       (ui32) (MAX_BEAM_REC_SIZE *
				       nframes));
    max_out_frames_alloc = nframes;
  }

  /*
   * copy ll_params to outbuf
   */
  
  outptr = outbuf;

  offset = sizeof(ncar_udp_frame_hdr_t);
  ll_params = (ll_params_t *) (inbuf + offset);
  memcpy((void *) outptr, (void *) ll_params, sizeof(ll_params_t));

  outptr += sizeof(ll_params_t);
  offset += sizeof(ll_params_t);
  
  /*
   * copy first packet data to outbuf
   */
  
  offset += sizeof(ll_pkt_gate_t);
  data_len = packet_len - offset;
  
  memcpy((char *) outptr, inbuf + offset, data_len);
  outptr += data_len;

  if (debug) {
    fprintf(stderr, "Packet, len %d, comprises the following:-\n", packet_len);
    fprintf(stderr, "ncar_udp_frame_hdr_t: %d bytes\n",
	    (int) sizeof(ncar_udp_frame_hdr_t));
    fprintf(stderr, "ll_params: %d bytes\n", (int) sizeof(ll_params_t));
    fprintf(stderr, "data: %d bytes, %d gates\n",
	    (int) data_len, (int) (data_len / LL_NFIELDS));
    fprintf(stderr, "\n");
  }

  for (i = 0; i < nframes - 1; i++) {
    
    packet_len = get_udp_pkt(Udp_fd, inbuf);
  
    /*
     * Check the condition code returned by the read
     */
    
    if(packet_len < 0) {
      fprintf(stderr, "ERROR - %s:read_udp_beam\n", prog_name);
      fprintf(stderr, "Reading ethernet - packet_len = %d\n", packet_len);
      sleep(1);
      goto try_again;
    }

    if (debug) {
      print_ncar_frame_hdr("Subsequent packet", frame_hdr);
    }
    
    seq_num++;
    if (frame_hdr->frame_seq_num != seq_num) {
      fprintf(stderr, "ERROR - %s:read_udp_beam\n", prog_name);
      fprintf(stderr, "Packet seq out of order\n");
      goto try_again;
    }

    /*
     * copy packet data to outbuf
     */
    
    offset = sizeof(ncar_udp_frame_hdr_t);
    data_len = packet_len - offset;
    
    memcpy((char *) outptr, inbuf + offset, data_len);
    outptr += data_len;
    
    if (debug) {
      fprintf(stderr, "Packet, len %d, comprises the following:-\n",
	      (int) packet_len);
      fprintf(stderr, "ncar_udp_frame_hdr_t: %d bytes\n",
	      (int) sizeof(ncar_udp_frame_hdr_t));
      fprintf(stderr, "data: %d bytes, %d gates\n",
	      (int) data_len, (int) (data_len / LL_NFIELDS));
      fprintf(stderr, "\n");
    }

  } /* i */

  return (outbuf);

}

/*******************************************************************
 * Opens up an UDP socket for use as a broadcaster.   Returns a file 
 * descriptor or number < 0 if a fatal error is detected 
 */

typedef struct sockaddr * sockaddr_t;

static int open_udp (int port)

{
  int udp_fd;                    /* socket descriptor */
  struct sockaddr_in addr;        /* address structure for socket */

  /* open a socket */
  if  ((udp_fd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    perror ("Could not open a UDP socket");
    return (-1);
  }

  /* bind local address to the socket */
  memset((void *) &addr, 0, sizeof (addr));
  addr.sin_port = htons (port);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl (INADDR_ANY);
  
  /* bind local address to socket */
  if (bind (udp_fd, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
    perror ("Couldn't bind a name to a socket - %s\n");
    close (udp_fd);
    return (-1);
  }
  
  return (udp_fd);
  
}

/**********************
 * print_ll_frame_hdr()
 */

static void print_ll_frame_hdr(char *label,
			       ll_udp_frame_hdr_t * lfhdr)

{

  fprintf(stderr, "\n** %s **\n", label);
  
  fprintf(stderr, "Frame header type: LINCOLN\n");

  fprintf(stderr, "frag_seqn: %d\n",
	  (int) lfhdr->frag_hdr.frag_seqn);
  fprintf(stderr, "msg_bytes: %d\n",
	  (int) lfhdr->frag_hdr.msg_bytes);
  fprintf(stderr, "frag_count: %d\n",
	  (int) lfhdr->frag_hdr.frag_count);
  fprintf(stderr, "frag_nmbr: %d\n",
	  (int) lfhdr->frag_hdr.frag_nmbr);
  fprintf(stderr, "frag_offset: %d\n",
	  (int) lfhdr->frag_hdr.frag_offset);
  fprintf(stderr, "data_bytes: %d\n\n",
	  (int) lfhdr->frag_hdr.data_bytes);
  
  fprintf(stderr, "rec_id: %d\n",
	  (int) lfhdr->rec_hdr.rec_id);
  fprintf(stderr, "rec_len: %d\n",
	  (int) lfhdr->rec_hdr.rec_len);
  fprintf(stderr, "rec_seqn: %d\n\n",
	  (int) lfhdr->rec_hdr.rec_seqn);
  
  fprintf(stderr, "frame_seq_num: %d\n",
	  (int) lfhdr->frame_seq_num);
  fprintf(stderr, "frames_per_beam: %d\n",
	  (int) lfhdr->frames_per_beam);
  fprintf(stderr, "frame_this_pkt: %d\n\n",
	  (int) lfhdr->frame_this_pkt);

}

/************************
 * print_ncar_frame_hdr()
 */

static void print_ncar_frame_hdr(char *label,
				 ncar_udp_frame_hdr_t * lfhdr)

{

  fprintf(stderr, "\n** %s **\n", label);
  
  fprintf(stderr, "Frame header type: NCAR\n");

  fprintf(stderr, "frame_seq_num: %d\n",
	  (int) lfhdr->frame_seq_num);
  fprintf(stderr, "frames_per_beam: %d\n",
	  (int) lfhdr->frames_per_beam);
  fprintf(stderr, "frame_this_pkt: %d\n\n",
	  (int) lfhdr->frame_this_pkt);

}


static int get_udp_pkt(int sfd, char *inbuf)

{

  int addrlen = sizeof (struct sockaddr_in);
  int packet_len;
  struct sockaddr_in from;   /* address where packet came from */

  /*
   * wait on socket for up to 10 secs at a time
   */

  while (SKU_read_select(sfd, 10000) < 0) {
    
    /*
     * timeout
     */

    PMU_auto_register("Waiting for udp data");

  }

  PMU_auto_register("Reading udp data");

  /*
   * data is available for reading
   */
  
  errno = EINTR;
  while (errno == EINTR) {
    errno = 0;
    packet_len = recvfrom (sfd, inbuf, MAX_UDP_PACKET_BYTES, 0, 
			   (struct sockaddr *) &from, &addrlen);
  }
  
  return (packet_len);
  
}


