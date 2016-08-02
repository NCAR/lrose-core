// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
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

#include "polar_ingest.h"
#include <toolsa/sockutil.h>

#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>

#if defined(__linux)
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#elif defined(__APPLE__)
#include <sys/ioctl.h>
#else
#include <sys/filio.h>
#endif

#include <sys/socket.h>
#include <netinet/in.h>

/*
 * file scope variables
 */

static int Udp_fd = -1;
static int Udp_port;

/*
 * file scope prototypes
 */

static void close_udp (void);

static int get_udp_pkt(char *inbuf);

static void print_ll_frame_hdr(char *label,
			       ll_udp_frame_hdr_t * lfhdr);

static void print_ncar_frame_hdr(char *label,
				 ncar_udp_frame_hdr_t * lfhdr);

static void open_udp (void);

/************************************************************************/

char *read_ll_udp_beam (void)

{

  static char *inbuf;
  static char *outbuf;
  static ll_udp_frame_hdr_t *frame_hdr;
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

    Udp_port = Glob->udp_port;
    
    first_call = FALSE;

  }

 try_again:

  /*
   * Read the first packet of beam off the net
   */

  frame_hdr->frame_this_pkt = 0;

  while (frame_hdr->frame_this_pkt != 1) {

    packet_len = get_udp_pkt(inbuf);
  
    /*
     * Check the condition code returned by the read
     */
    
    if(packet_len < 0) {
      frame_hdr->frame_this_pkt = 0;
    }
  
    BE_to_ll_udp_frame_hdr(frame_hdr);

  }

  nframes = frame_hdr->frames_per_beam;
  seq_num = frame_hdr->frame_seq_num;

  if (Glob->debug) {
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
  BE_to_ll_params(ll_params);
  memcpy((void *) outptr, (void *) ll_params, sizeof(ll_params_t));

  outptr += sizeof(ll_params_t);
  offset += sizeof(ll_params_t);
  
  ll_pkt_gate = (ll_pkt_gate_t *) (inbuf + offset);
  
  if (Glob->debug) {
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

  if (Glob->debug) {
    fprintf(stderr, "Packet, len %d, comprises the following:-\n",
	    packet_len);
    fprintf(stderr, "ll_udp_frame_hdr_t: %d bytes\n",
	    (int) sizeof(ll_udp_frame_hdr_t));
    fprintf(stderr, "ll_params: %d bytes\n",
	    (int) sizeof(ll_params_t));
    fprintf(stderr, "ll_pkt_gate_t: %d bytes\n",
	    (int) sizeof(ll_pkt_gate_t));
    fprintf(stderr, "data: %d bytes, %d gates\n",
	    data_len, data_len / LL_NFIELDS);
    fprintf(stderr, "checksum: %d bytes\n",
	    (int) sizeof(ll_checksum_t));
    fprintf(stderr, "\n");
  }

  for (i = 0; i < nframes - 1; i++) {
    
    packet_len = get_udp_pkt(inbuf);
  
    /*
     * Check the condition code returned by the read
     */
    
    if(packet_len < 0) {
      goto try_again;
    }

    BE_to_ll_udp_frame_hdr(frame_hdr);

    if (Glob->debug) {
      print_ll_frame_hdr("Subsequent packet", frame_hdr);
    }
    
    seq_num++;
    if (frame_hdr->frame_seq_num != seq_num) {
      fprintf(stderr, "ERROR - %s:read_udp_beam\n", Glob->prog_name);
      fprintf(stderr, "Packet seq out of order\n");
      goto try_again;
    }

    offset = sizeof(ll_udp_frame_hdr_t);
    ll_pkt_gate = (ll_pkt_gate_t *) (inbuf + offset);
    
    if (Glob->debug) {
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
    
    if (Glob->debug) {
      fprintf(stderr, "Packet, len %d, comprises the following:-\n",
	      packet_len);
      fprintf(stderr, "ll_udp_frame_hdr_t: %d bytes\n",
	      (int) sizeof(ll_udp_frame_hdr_t));
      fprintf(stderr, "ll_pkt_gate_t: %d bytes\n",
	      (int) sizeof(ll_pkt_gate_t));
      fprintf(stderr, "data: %d bytes, %d gates\n",
	      data_len, data_len / LL_NFIELDS);
      fprintf(stderr, "checksum: %d bytes\n", (int) sizeof(ll_checksum_t));
      fprintf(stderr, "\n");
    }

  } /* i */

  return (outbuf);

}

/****************************************************************************/

char *read_ncar_udp_beam (void)

{

  static char *inbuf;
  static char *outbuf;
  static ncar_udp_frame_hdr_t *frame_hdr;
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
    
    Udp_port = Glob->udp_port;

    first_call = FALSE;

  }

 try_again:

  /*
   * Read the first packet of beam off the net
   */

  frame_hdr->frame_this_pkt = 0;

  while (frame_hdr->frame_this_pkt != 1) {

    packet_len = get_udp_pkt(inbuf);
  
    BE_to_ncar_udp_frame_hdr(frame_hdr);

    /*
     * Check the condition code returned by the read
     */
    
    if(packet_len < 0) {
      frame_hdr->frame_this_pkt = 0;
    }
  
  }

  nframes = frame_hdr->frames_per_beam;
  seq_num = frame_hdr->frame_seq_num;

  if (Glob->debug) {
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
  BE_to_ll_params(ll_params);
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

  if (Glob->debug) {
    fprintf(stderr, "Packet, len %d, comprises the following:-\n",
	    packet_len);
    fprintf(stderr, "ncar_udp_frame_hdr_t: %d bytes\n",
	    (int) sizeof(ncar_udp_frame_hdr_t));
    fprintf(stderr, "ll_params: %d bytes\n", (int) sizeof(ll_params_t));
    fprintf(stderr, "data: %d bytes, %d gates\n",
	    data_len, data_len / LL_NFIELDS);
    fprintf(stderr, "\n");
  }

  for (i = 0; i < nframes - 1; i++) {
    
    packet_len = get_udp_pkt(inbuf);
  
    /*
     * Check the condition code returned by the read
     */
    
    if(packet_len < 0) {
      goto try_again;
    }

    BE_to_ncar_udp_frame_hdr(frame_hdr);

    if (Glob->debug) {
      print_ncar_frame_hdr("Subsequent packet", frame_hdr);
    }
    
    seq_num++;
    if (frame_hdr->frame_seq_num != seq_num) {
      fprintf(stderr, "ERROR - %s:read_udp_beam\n", Glob->prog_name);
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
    
    if (Glob->debug) {
      fprintf(stderr, "Packet, len %d, comprises the following:-\n",
	      packet_len);
      fprintf(stderr, "ncar_udp_frame_hdr_t: %d bytes\n",
	      (int) sizeof(ncar_udp_frame_hdr_t));
      fprintf(stderr, "data: %d bytes, %d gates\n",
	      data_len, data_len / LL_NFIELDS);
      fprintf(stderr, "\n");
    }

  } /* i */

  return (outbuf);

}

/*******************************************************************
 * Opens up an UDP socket for use as a broadcaster.   Returns a file 
 * descriptor or number < 0 if a fatal error is detected 
 */

static void open_udp (void)

{
  int val = 1;
  int valen = sizeof(val);
  struct sockaddr_in addr;        /* address structure for socket */
  int blocking_flag = 1;          /* argument for ioctl call */
  
  /* open a socket */
  if  ((Udp_fd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    perror ("Could not open UDP socket");
    return;
  }

  /* make the socket non-blocking */
  if (ioctl(Udp_fd, FIONBIO, &blocking_flag) != 0) {
    perror("Could not make UDP socket non-blocking");
    close_udp();
    return;
  }
  
  /*
   * set the socket for reuse
   */

#if (defined IRIX5) || (defined IRIX6)
  setsockopt(Udp_fd, SOL_SOCKET,SO_REUSEPORT, &val, valen);  
#else
  setsockopt(Udp_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, valen);
#endif

  /* bind local address to the socket */
  memset((void *) &addr, 0, sizeof (addr));
  addr.sin_port = htons (Udp_port);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl (INADDR_ANY);
  
  /* bind local address to socket */
  if (bind (Udp_fd, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
    perror ("Couldn't bind a name to UDP socket\n");
    close_udp();
  }
  
  return;
  
}

/*******************************************************************
 * Opens up an UDP socket for use as a broadcaster.   Returns a file 
 * descriptor or number < 0 if a fatal error is detected 
 */

static void close_udp (void)

{
  close (Udp_fd);
  Udp_fd = -1;
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


/***************
 * get_udp_pkt()
 */

static int get_udp_pkt(char *inbuf)

{

  int addrlen = sizeof (struct sockaddr_in);
  int packet_len;
  int iret;
  struct sockaddr_in from;   /* address where packet came from */

  /*
   * open if necessary
   */

  if (Udp_fd < 0) {
    while (Udp_fd < 0) {
      open_udp();
      PMU_auto_register("Trying to open udp");
      sleep(1);
    }
    PMU_force_register("Udp open");
  }
  
  /*
   * wait on socket for up to 10 secs at a time
   */

  while ((iret = SKU_read_select(Udp_fd, 10000)) == -1) {
    /*
     * timeout
     */
    PMU_auto_register("Waiting for udp data");
  }
  
  if (iret == 1) {
    
    /*
     * data is available for reading
     */
    
    PMU_auto_register("Reading udp data");

    errno = EINTR;
    while (errno == EINTR ||
	   errno == EWOULDBLOCK) {
      errno = 0;
      packet_len =
	recvfrom (Udp_fd, inbuf, MAX_UDP_PACKET_BYTES, 0, 
		  (struct sockaddr *)&from, (socklen_t *) &addrlen);

      if (errno == EINTR) {
	PMU_auto_register("Reading udp data - EINTR");
      }
      else if (errno == EWOULDBLOCK) {
	PMU_auto_register("Reading udp data - EWOULDBLOCK");
	sleep(1);
      }
    }
  
    if (packet_len > 0) {

      /*
       * success
       */

      return (packet_len);

    } else {
      
      fprintf(stderr, "ERROR - %s:read_udp_beam\n", Glob->prog_name);
      fprintf(stderr, "Reading ethernet - packet_len = %d\n", packet_len);
      perror("");
      return (-1);
      
    }
    
  } else {
    
    /*
     * failure - close socket
     */
    
    close_udp();
    return (-1);
    
  }
  
}

