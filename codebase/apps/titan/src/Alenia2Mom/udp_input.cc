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
 * Routines reading a beam from ALENIA UDP broadcast.
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * May 1997.
 ********************************************************************/

#include "Alenia2Mom.h"
#include <toolsa/membuf.h>
#include <toolsa/sockutil.h>
#include <dataport/bigend.h>
#include <dataport/swap.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctime>

#if defined(__linux)
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#elif defined(__APPLE__)
#include <sys/ioctl.h>
#else
#include <sys/filio.h>
using namespace std;
#endif


/*
 * incoming packet size
 */

#define UDP_MAX_INPUT 1524

/*
 * file scope variables
 */

static int Udp_fd = -1;
static int Debug;
static int DataLen;
static int VolStarting = FALSE;
static int HeaderChecked = FALSE;
static int SyncOk = TRUE;

static date_time_t VolStartTime;
static date_time_t BeamTime;

static MEMbuf *InBuf = NULL;
static MEMbuf *CopyBuf = NULL;
static MEMbuf *PacketBuf = NULL;

/*
 * file scope prototypes
 */

static int compute_data_len(alenia_header_t *al_header);
static int get_udp_frame(ui08 **buf_p, int *len_p);
static int load_packet(void);
static int read_packet(ui08 **buf_p, int *len_p);
static int sync_inbuf(void);

/********************************************************************
 * open_input_udp()
 *
 * Initialize UDP input.
 */

int open_input_udp(int port, int debug)

{

  struct sockaddr_in addr;        /* address structure for socket */
  int blocking_flag = 1;          /* argument for ioctl call */
  int val = 1;
  int valen = sizeof(val);
  
  Debug = debug;

  /*
   * allocate memory buffers
   */

  InBuf = MEMbufCreate();
  CopyBuf = MEMbufCreate();
  PacketBuf = MEMbufCreate();

  /*
   * UDP port
   */

  if  ((Udp_fd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    fprintf(stderr, "Could not open UDP socket, port %d\n", port);
    perror ("socket error:");
    return (-1);
  }

  /*
   * make the socket non-blocking
   */

  if (ioctl(Udp_fd, FIONBIO, &blocking_flag) != 0) {
    fprintf(stderr, "Could not make socket non-blocking, port %d\n", port);
    perror ("ioctl error:");
    close_output_udp();
    return (-1);
  }
  
  /*
   * set the socket for reuse
   */

#if (defined IRIX5) || (defined IRIX6)
  setsockopt(Udp_fd, SOL_SOCKET,SO_REUSEPORT, &val, valen);  
#else
  setsockopt(Udp_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, valen);
#endif

  /*
   * bind local address to the socket
   */

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

/******************
 * read_input_udp()
 *
 * Read a beam buffer from UDP
 *
 * returns 0 on success, -1 on failure
 *
 * Blocks until beam ready
 */

int read_input_udp(ui08 **buf_p, int *len_p)

{

  while (load_packet() != 0) {
    if (read_packet(buf_p, len_p)) {
      return (-1);
    }
  }

  *buf_p = (ui08 *) MEMbufPtr(PacketBuf);
  *len_p = MEMbufLen(PacketBuf);

  return (0);

}

/*------------------------------------------------------------------------*/

static int read_packet(ui08 **buf_p, int *len_p)

{

  ui08 frame_type;
  ui08 *frame_buf;
  int frame_len;
  static int nframes = 0;
  static int nbytes = 0;
  static time_t start_time;
  int nsecs;
  
  if (Udp_fd < 0) {
    fprintf(stderr, "ERROR - read_input_udp - init not done\n");
    return (-1);
  }

  /*
   * get a UDP packet
   */
  
  if (get_udp_frame(&frame_buf, &frame_len)) {
    return (-1);
  }

  if (Glob->params.debug >= DEBUG_VERBOSE) {
    fprintf(stderr, "---------> got udp frame, len %d bytes\n", frame_len);
  }

  /*
   * determine udp frame type - skip over 2 bytes of
   * length
   */
  
  frame_type = *(frame_buf + 2);
  
  if (frame_type == ALENIA_UDP_START) {
    
    if (Glob->params.debug) {
      fprintf(stderr, "------------> START\n");
    }
    nframes = 0;
    nbytes = 0;
    start_time = time(NULL);
    
    MEMbufReset(InBuf);
    MEMbufReset(PacketBuf);
    VolStarting = TRUE;
    SyncOk = TRUE;
    HeaderChecked = FALSE;
    
  } else if ((frame_type == ALENIA_UDP_DATA) && SyncOk) {
    
    nframes++;
    nbytes += frame_len;
    
    if (Glob->params.debug >= DEBUG_VERBOSE) {
      fprintf(stderr, "------------> DATA, %d frames so far\n",
	      nframes);
    }
    
    /*
     * add frame to input buffer
     */
    
    MEMbufAdd(InBuf, frame_buf + 3, frame_len - 3);
    
    VolStarting = FALSE;

  } else if (frame_type == ALENIA_UDP_STOP) {

    nsecs = time(NULL) - start_time;
    
    if (Glob->params.debug) {
      fprintf(stderr, "------------> STOP, %d frames received\n",
	      nframes);
      fprintf(stderr, "============> %d bytes received in %d secs\n",
	      nbytes, nsecs);
      fprintf(stderr, "============> Data rate: %g bytes/sec\n",
	      (double) nbytes / (double) nsecs);
    }
    
    VolStarting = FALSE;
    
  } else if (frame_type == ALENIA_UDP_ABORT) {
    
    if (Glob->params.debug) {
      fprintf(stderr, "------------> ABORT, %d frames received\n",
	      nframes);
    }
    
    VolStarting = FALSE;
    
  } else {
    
    if (Glob->params.debug >= DEBUG_VERBOSE) {
      fprintf(stderr, "------------> OUT_OF_SYNC, %d frames received\n",
	      nframes);
    }
    
    VolStarting = FALSE;
    
  } /* if (frame_type == ... */

  return (0);

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
  
  if (InBuf != NULL) {
    MEMbufDelete(InBuf);
    MEMbufDelete(CopyBuf);
    MEMbufDelete(PacketBuf);
  }

}

/******************************
 * access functions for statics
 */

date_time_t *get_beam_time(void)

{
  return (&BeamTime);
}

/*****************
 * get_udp_frame()
 *
 * Reads a udp packet, looping as necessary to perform PMU
 * registrations.
 *
 * returns 0 on success, -1 on failure.
 */

static int get_udp_frame(ui08 **buf_p, int *len_p)

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

  while ((iret = SKU_read_select(Udp_fd, 10000)) == -1) {
    /*
     * timeout
     */
    PMU_auto_register("Waiting for udp data");
  }
  iret = 1;

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

/***************
 * load_packet()
 *
 * Attempts to load the PacketBuf from the InBuf
 *
 * Returns 0 on success, -1 on failure.
 */

static int load_packet(void)

{

  ui08 *inbuf;
  alenia_header_t al_header;
  static int data_valid;

  al_header = *((alenia_header_t *) MEMbufPtr(InBuf));

  /*
   * check that there are enough bytes in the buffer to check
   * for valid date
   */
  
  if (MEMbufLen(InBuf) < sizeof(alenia_header_t)) {
    return (-1);
  }

  /*
   * if first pkt in vol, set start time
   */

  if (VolStarting) {
    if (decode_date(&al_header, &VolStartTime)) {
      if (Glob->params.debug) {
	fprintf(stderr, "----> VolStarting - decode_date failed- syncing\n");
      }
      if (sync_inbuf()) {
	if (Glob->params.debug) {
	  fprintf(stderr, "----> Sync not successful\n");
	}
	return (-1);
      }
    }
  }

  /*
   * check header for time and data len if not already done
   * for this packet
   */

  if (!HeaderChecked) {

    /*
     * get data time
     */
    
    if (decode_date(&al_header, &BeamTime)) {
      if (Glob->params.debug) {
	fprintf(stderr,
		"----> Checking header - decode_date failed- syncing\n");
      }
      if (sync_inbuf()) {
	if (Glob->params.debug) {
	  fprintf(stderr, "----> Sync not successful\n");
	}
	return (-1);
      }
    }
    
    if (Glob->params.use_wallclock_time) {
      BeamTime.unix_time = time(NULL) + Glob->params.time_correction;
      uconvert_from_utime(&BeamTime);
    } else {
      if (BeamTime.unix_time < VolStartTime.unix_time) {
	BeamTime.unix_time += 86400;
	uconvert_from_utime(&BeamTime);
      }
    }

    DataLen = compute_data_len(&al_header);
    
    data_valid = (al_header.clutter >> 1) & 1;

    HeaderChecked = TRUE;
  
  }

  /*
   * check that there are enough bytes in the buffer
   * for the data
   */
  
  if (MEMbufLen(InBuf) < DataLen) {
    return (-1);
  }

  /*
   * Copy data into Packet buffer, return success
   */
  
  inbuf = (ui08 *) MEMbufPtr(InBuf);
  MEMbufReset(PacketBuf);
  MEMbufAdd(PacketBuf,  inbuf, DataLen);
  
  MEMbufReset(CopyBuf);
  MEMbufAdd(CopyBuf, inbuf + DataLen, MEMbufLen(InBuf) - DataLen);
  MEMbufReset(InBuf);
  MEMbufAdd(InBuf, MEMbufPtr(CopyBuf), MEMbufLen(CopyBuf));

  HeaderChecked = FALSE;

  if (data_valid || !Glob->params.valid_only) {
    return (0);
  } else {
    return (-1);
  }

}

/********************
 * compute_data_len()
 *
 * Compute data len and load into static
 *
 */

static int compute_data_len(alenia_header_t *al_header)
  
{

  int data_len;
  alenia_params_t params;

  /*
   * available fields
   */
  
  params.dbz_avail = al_header->parameters & 1;
  params.zdr_avail = (al_header->parameters >> 1) & 1;
  params.vel_avail = (al_header->parameters >> 2) & 1;
  params.width_avail = (al_header->parameters >> 7) & 1;
  
  params.nfields = 0;
  params.nfields += params.dbz_avail;
  params.nfields += params.zdr_avail;
  params.nfields += params.vel_avail;
  params.nfields += params.width_avail;
  
  /*
   * number of gates
   */
  
  params.ngates =
    ((int) (al_header->avarie >> 6) * 256) + al_header->num_bin_l;

  /*
   * compute data len
   */

  data_len = 
    params.ngates * params.nfields + sizeof(alenia_header_t);

  return (data_len);

}

/**************
 * sync_inbuf()
 *
 * Sync up the input buffer using the date field.
 *
 * Returns 0 on success, -1 on failure.
 *
 * If failed, InBuf is reset to 0 length.
 */

static int sync_inbuf(void)

{

  ui08 *inbuf, *ip;
  int i, len;
  date_time_t dtime;
  alenia_header_t *al_header;
  
  inbuf = (ui08 *) MEMbufPtr(InBuf);
  len = MEMbufLen(InBuf);

  ip = inbuf;

  for (i = 0; i < len - sizeof(alenia_header_t); i++, ip++) {
    
    al_header = (alenia_header_t *) ip;

    if (decode_date(al_header, &dtime) == 0) {

      if (dtime.year == VolStartTime.year &&
	  dtime.month == VolStartTime.month &&
	  dtime.day == VolStartTime.day) {

	/*
	 * found sync point
	 */

	MEMbufReset(CopyBuf);
	MEMbufAdd(CopyBuf, ip, len - i);
	MEMbufReset(InBuf);
	MEMbufAdd(InBuf, MEMbufPtr(CopyBuf), MEMbufLen(CopyBuf));

	return (0);
	
      } /* if (dtime.year == VolStartTime.year && ... */
      
    } /* if (decode_date(al_header, &dtime) == 0) */

  } /* i */
  
  /*
   * no sync point found, reset input buffer
   */

  MEMbufReset(InBuf);

  return (-1);

}
