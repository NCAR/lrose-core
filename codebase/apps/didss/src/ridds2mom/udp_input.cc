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
/*******************************************************************
 * udp_input.c
 *
 * Routines reading a beam from RIDDS UDP broadcast.
 *
 * From Gary Blackburn code - nexrad_getmsg.
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * May 1997.
 ********************************************************************/

#include "ridds2mom.h"
#include <rapformats/ridds.h>
#include <rapformats/swap.h>
#include <toolsa/membuf.h>
#include <toolsa/sockutil.h>
#include <dataport/bigend.h>
#include <sys/socket.h>
#include <netinet/in.h>

#if defined(__linux)
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#else
#include <sys/filio.h>
using namespace std;
#endif


/*
 * incoming packet size
 */

#define UDP_MAX_INPUT 1524

static int Udp_fd = -1;
static int Debug = 0;
static int Verbose = 0;
static MEMbuf *Mbuf = NULL;

/*
 * prototypes
 */

static void BE_to_ridds_frame_hdr(ridds_frame_hdr *hdr);

static int get_udp_pkt(ui08 **buf_p, int *len_p);

/********************************************************************
 * open_input_udp()
 *
 * Initialize UDP input.
 */

int open_input_udp(int port, int debug, int verbose /* = 0*/ )

{

  struct sockaddr_in addr;        /* address structure for socket */
  int blocking_flag = 1;          /* argument for ioctl call */
  int val = 1;
  int valen = sizeof(val);

  Debug = debug;
  Verbose = verbose;

  /*
   * allocate memory buffer
   */

  Mbuf = MEMbufCreate();
  
  /* open a socket */
  if  ((Udp_fd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    fprintf(stderr, "Could not open UDP socket, port %d\n", port);
    perror ("socket error:");
    return (-1);
  }

  /* make the socket non-blocking */
  if (ioctl(Udp_fd, FIONBIO, &blocking_flag) != 0) {
    fprintf(stderr, "Could not make socket non-blocking, port %d\n", port);
    perror ("ioctl error:");
    close_output_udp();
    return (-1);
  }
  
  /*
   * set the socket for reuse
   */
  setsockopt(Udp_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, valen);

  /* bind local address to the socket */
  memset((void *) &addr, 0, sizeof (addr));
  addr.sin_port = htons (port);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl (INADDR_ANY);
  
  if (bind (Udp_fd, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
    fprintf(stderr, "Could not bind UDP socket, port %d\n", port);
    perror ("bind error:");
    close_output_udp();
    return (-1);
  }
  
  return (0);
  
}

/*------------------------------------------------------------------------*/

int read_input_udp(ui08 **buf_p, int *len_p)

{

  int in_progress = FALSE;
  int forever = TRUE;
  int pkt_type;
  ridds_frame_hdr hdr;
  ui08 *frame_buf;
  int frame_len;
  int prev_seq = -1;
  
  if (Udp_fd < 0) {
    fprintf(stderr, "ERROR - read_input_udp - init not done\n");
    return (-1);
  }

  while (forever) {

    /*
     * get a UDP packet
     */
    
    if (get_udp_pkt(&frame_buf, &frame_len)) {
      return (-1);
    }

    /*
     * make local copy of hdr, and swap relevant items
     */
    
    hdr = *((ridds_frame_hdr *) frame_buf);
    BE_to_ridds_frame_hdr(&hdr);

    /*
     * check for correct sequence - if out of sequence,
     * a packet has been dropped so start again
     */

    if ((int)hdr.fr_seq != prev_seq + 1) {
      in_progress = FALSE;
    }
    prev_seq = hdr.fr_seq;

    /*
     * if not in progress, look for packet with frame number 1
     */

    if (!in_progress) {
      if (hdr.frame_num == 1) {
	/* reset */
	in_progress =  TRUE;
	pkt_type = hdr.msg_hdr.message_type;
	MEMbufReset(Mbuf);
      } else {
	/* need to get another packet */
	continue;
      }
    }
    
    /*
     * Add this frame's data to the buffer
     */

    MEMbufAdd(Mbuf, frame_buf + sizeof(ridds_frame_hdr), hdr.data_len);

    /*
     * if this is the last frame in the sequence, check type
     */

    if (hdr.frame_num == hdr.nframes) {
      if (pkt_type == DIGITAL_RADAR_DATA) {
	*buf_p = (ui08*)MEMbufPtr(Mbuf);
	*len_p = MEMbufLen(Mbuf);
	return (0);
      } else {
	in_progress = FALSE;
      }
    }

  } /* while (forever) */

  return (-1); /* suppress compiler warning */

}
  
/******************************************************************
 * close_input_udp()
 */

void close_input_udp(void)

{
  /*
   * close UDP socket
   */

  if (Udp_fd >= 0) {
    close(Udp_fd);
    Udp_fd = -1;
    if (Debug) {
      fprintf(stderr, "Closing input UDP\n");
    }
  }

  /*
   * free up memory buffer
   */
  
  if (Mbuf != NULL) {
    MEMbufDelete(Mbuf);
  }

}

/***************
 * get_udp_pkt()
 *
 * Reads a udp packet, looping as necessary to perform PMU
 * registrations.
 *
 * returns 0 on success, -1 on failure.
 */

static int get_udp_pkt(ui08 **buf_p, int *len_p)

{

  static ui08 buffer[UDP_MAX_INPUT];
#if defined(__linux)
  socklen_t addrlen = sizeof (struct sockaddr_in);
#else
  int addrlen = sizeof (struct sockaddr_in);
#endif
  int packet_len;
  int iret;
  struct sockaddr_in from;   /* address where packet came from */

  /*
   * wait on socket for up to 10 secs at a time
   */

  if (Verbose) {
    cerr << "Waiting on udp ..." << endl;
  }

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
      packet_len = recvfrom (Udp_fd, buffer, UDP_MAX_INPUT, 0, 
			     (struct sockaddr *)&from, &addrlen);
      
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

      if (Verbose) {
	cerr << "Received packet, len: " << packet_len << endl;
      }
      
      *buf_p = buffer;
      *len_p = packet_len;
      return (0);

    } else {
      
      fprintf(stderr, "ERROR - %s:read_udp_beam\n", Glob->prog_name);
      fprintf(stderr, "Reading ethernet - packet_len = %d\n", packet_len);
      perror("");
      return (-1);
      
    }

  } /* if (iret == 1) */

  return (-1);
    
  
}

/*************************
 * BE_to_ridds_frame_hdr()
 *
 * Swap relevant portions of the header
 */

static void BE_to_ridds_frame_hdr(ridds_frame_hdr *hdr)

{
  BE_to_array_32(&hdr->fr_seq, 4);
  BE_to_array_16(&hdr->nframes, 2);
  BE_to_array_16(&hdr->frame_num, 2);
  BE_to_array_16(&hdr->data_len, 2);
}

