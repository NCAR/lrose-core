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
 * udp_output.c
 *
 * Routines writing a ray to UDP broadcast.
 *
 * From Gary Blackburn code - putmsg.
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * May 1997.
 ********************************************************************/

#include "Alenia2Mom.h"
#include <dataport/bigend.h>
#include <rapformats/ridds.h>
#include <rapformats/swap.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;

static int Udp_fd = -1;
static int Debug;
struct sockaddr_in Out_address;

typedef struct {
  ncar_udp_frame_hdr_t hdr;
  ui08 data[UDP_OUTPUT_SIZE - sizeof(ncar_udp_frame_hdr_t)];
} ncar_udp_frame_t;

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
#if (defined SUNOS4) || (defined SUNOS5)
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
  
  return (0);

}

/*------------------------------------------------------------------------*/

int write_output_udp(ui08 *buffer, int buflen)

{

  static int frame_seq_num = 0;

  ui08 *bufptr;

  int i;
  int hdr_len;
  int max_data_len;
  int data_len;
  int frame_len;
  int num_frames = 0;
  int data_left = buflen;
  
  ncar_udp_frame_t frame;

  hdr_len = sizeof(ncar_udp_frame_hdr_t);
  max_data_len = UDP_OUTPUT_SIZE - hdr_len;
  bufptr = buffer;

  if (Udp_fd < 0) {
    fprintf(stderr, "ERROR - write_output_ncar_udp - init not done\n");
    return (-1);
  }

  /*
   * determine number of frames for this data
   */
  
  num_frames = ((buflen - 1) / max_data_len) + 1;

  for (i = 0; i < num_frames; i++) {

    frame.hdr.frames_per_beam = num_frames;
    if (i == 0) {
      frame.hdr.frame_this_pkt = FIRST_PKT;
    } else {
      frame.hdr.frame_this_pkt = CONTINUE_PKT;
    }
    frame.hdr.frame_seq_num = ++frame_seq_num;

  
    if (Debug > 1) {
      fprintf (stderr, "seq# %d, frame# %d, total frames %d\n",
	       frame_seq_num, frame.hdr.frame_this_pkt,
	       num_frames);
    }

    BE_from_ncar_udp_frame_hdr(&frame.hdr);

    if(data_left >= max_data_len) {
      data_len = max_data_len;
    } else {
      data_len = data_left;
    }
    
    memcpy(frame.data, bufptr, data_len);
    data_left -= data_len;
    bufptr += data_len;

    frame_len = hdr_len + data_len;

    if (sendto (Udp_fd, (char *) &frame, frame_len, 0,
		(struct sockaddr *) &Out_address, sizeof(Out_address)) !=
	frame_len) {
      fprintf(stderr, "write_output_ncar_udp - could not send pkt %d\n", i);
      perror ("sendto error");
      return (-1);
    }

  } /* i */

  return (0);

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
  }
}

