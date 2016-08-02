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
//////////////////////////////////////////////////////////
// Input.cc
//
// UDP input
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1999
//
//////////////////////////////////////////////////////////

#include "Input.hh"

#include <rapformats/swap.h>
#include <toolsa/membuf.h>
#include <toolsa/sockutil.h>
#include <toolsa/pmu.h>
#include <dataport/bigend.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>

#if defined(__linux)
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#else
#include <sys/filio.h>
using namespace std;
#endif

/////////////
//constructor

Input::Input(string &prog_name, Params  &params) :
  _progName(prog_name), _params(params)

{

  _isOK = true;
  _udpFd = -1;

  // open a socket

  if  ((_udpFd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    fprintf(stderr, "Could not open UDP socket, port %d\n",
	    _params.input_udp_port);
    perror ("socket error:");
    _isOK = false;
    return;
  }

  // make the socket non-blocking

  int blocking_flag = 1;          // argument for ioctl call
  if (ioctl(_udpFd, FIONBIO, &blocking_flag) != 0) {
    fprintf(stderr, "Could not make socket non-blocking, port %d\n",
	    _params.input_udp_port);
    perror ("ioctl error:");
    close (_udpFd);
    _udpFd = -1;
    _isOK = false;
    return;
  }
  
  // set the socket for reuse

  int val = 1;
  int valen = sizeof(val);
  setsockopt(_udpFd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, valen);

  // bind local address to the socket

  struct sockaddr_in addr;        // address structure for socket
  memset((void *) &addr, 0, sizeof (addr));
  addr.sin_port = htons (_params.input_udp_port);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl (INADDR_ANY);
  
  if (bind (_udpFd, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
    fprintf(stderr, "Could not bind UDP socket, port %d\n",
	    _params.input_udp_port);
    perror ("bind error:");
    close (_udpFd);
    _udpFd = -1;
    _isOK = false;
    return;
  }

  if (_params.debug) {
    fprintf(stderr, "Opened UDP socket, port %d\n", _params.input_udp_port);
  }
  
  return;
  
}

//////////////
// destructor

Input::~Input()

{

  if (_udpFd >= 0) {
    close(_udpFd);
    _udpFd = -1;
    if (_params.debug) {
      fprintf(stderr, "Closing input UDP\n");
    }
  }

}

///////////////////
// read a message
//

int Input::readMsg()

{

  if (_params.assemble_ridds_packets) {
    
    return (_readRiddsAndAssemble());
    
  } else {
    
    return (_readSingle());

  }

}

////////////////////////////
// read a single UDP packet

int Input::_readSingle()

{

  // get a UDP packet
  
  if (_getUdpPkt()) {
    return (-1);
  }

  // copy the packet into the message buffer

  _msgBuf.reset();
  _msgBuf.add(_pktBuf, _pktLen);

  return (0);

}

//////////////////////////////////
// read and assemble ridds message

int Input::_readRiddsAndAssemble()

{

  bool in_progress = false;
  bool forever = true;
  int pkt_type;
  ridds_frame_hdr hdr;
  int prev_seq = -1;
  
  while (forever) {

    // get a UDP packet
    
    if (_getUdpPkt()) {
      return (-1);
    }

    // make local copy of hdr, and swap relevant items
    
    hdr = *((ridds_frame_hdr *) _pktBuf);
    _BE2RiddsFrameHdr(&hdr);

    // check for correct sequence - if out of sequence,
    // a packet has been dropped so start again
    
    if ((int)hdr.fr_seq != prev_seq + 1) {
      in_progress = false;
    }
    prev_seq = hdr.fr_seq;

    // if not in progress, look for packet with frame number 1

    if (!in_progress) {
      if (hdr.frame_num == 1) {
	// reset
	in_progress = true;
	pkt_type = hdr.msg_hdr.message_type;
	_msgBuf.reset();
      } else {
	// need to get another packet
	continue;
      }
    }
    
    // Add this frame's data to the buffer
    
    _msgBuf.add(_pktBuf + sizeof(ridds_frame_hdr), hdr.data_len);

    // if this is the last frame in the sequence, check type

    if (hdr.frame_num == hdr.nframes) {
      if (pkt_type == DIGITAL_RADAR_DATA) {
	return (0);
      } else {
	in_progress = false;
      }
    }

  } // while (forever)

  return (-1); // suppress compiler warning

}
  
///////////////////////////////////////////////////////////
// _getUdpPkt()
//
// Reads a udp packet, looping as necessary to perform PMU
// registrations.
//
// returns 0 on success, -1 on failure.
//

int Input::_getUdpPkt()

{

#if defined(__linux)
  socklen_t addrlen = sizeof (struct sockaddr_in);
#else
  int addrlen = sizeof (struct sockaddr_in);
#endif

  int iret;
  struct sockaddr_in from;   // address from which packet came
  
  // wait on socket for up to 1 sec at a time
  
  while ((iret = SKU_read_select(_udpFd, 1000)) == -1) {
    // timeout
    PMU_auto_register("Waiting for udp data");
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "Waiting for udp data\n");
    }
  }

  if (iret == 1) {
    
    // data is available for reading
    
    PMU_auto_register("Reading udp data");
    
    errno = EINTR;
    while (errno == EINTR ||
	   errno == EWOULDBLOCK) {
      errno = 0;
      _pktLen = recvfrom (_udpFd, (void *) _pktBuf, UDP_OUTPUT_SIZE, 0, 
			  (struct sockaddr *)&from, &addrlen);
      if (errno == EINTR) {
	PMU_auto_register("Reading udp data - EINTR");
      } else if (errno == EWOULDBLOCK) {
	PMU_auto_register("Reading udp data - EWOULDBLOCK");
	sleep(1);
      }
    }
    
    if (_pktLen > 0) {

      if (_params.debug >= Params::DEBUG_VERBOSE) {
	fprintf(stderr, "Read packet, %d bytes\n", _pktLen);
      }

      return (0);

    } else {
      
      fprintf(stderr, "ERROR - %s:read_udp_beam\n", _progName.c_str());
      fprintf(stderr, "Reading ethernet - _pktLen = %d\n", _pktLen);
      perror("");
      return (-1);
      
    }
    
  } // if (iret == 1)

  return (-1);
  
}

///////////////////////////
// _BE2RiddsFrameHdr()
//
// Swap relevant portions of the header
//

void Input::_BE2RiddsFrameHdr(ridds_frame_hdr *hdr)

{
  BE_to_array_32(&hdr->fr_seq, 4);
  BE_to_array_16(&hdr->nframes, 2);
  BE_to_array_16(&hdr->frame_num, 2);
  BE_to_array_16(&hdr->data_len, 2);
  BE_to_array_16(&hdr->msg_hdr.message_type, 2);
}

