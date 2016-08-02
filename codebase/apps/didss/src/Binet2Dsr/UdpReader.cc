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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/06 23:53:39 $
//   $Id: UdpReader.cc,v 1.5 2016/03/06 23:53:39 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * UdpReader: Class for objects used to read beam data from a UDP port.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/sockutil.h>
#include <toolsa/MsgLog.hh>

#if defined(__linux)
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#else
#include <sys/filio.h>
#endif

#include "UdpReader.hh"
using namespace std;


/*********************************************************************
 * Constructor
 */

UdpReader::UdpReader(const int port, const bool debug) :
  Reader(debug),
  _port(port),
  _udpFd(-1)
{
}


/*********************************************************************
 * Destructor
 */

UdpReader::~UdpReader() 
{
  if (_udpFd >= 0)
    close(_udpFd);
}


/*********************************************************************
 * init() - Initialize the reader.
 *
 * Returns true on success, false on failure.
 */

bool UdpReader::init() 
{
  static const string method_name = "UdpReader::init()";
  
//  _debug = true;

  // Open the socket

  if (_debug)
    cerr << "Opening udp socket on port " << _port << endl;
  
  PMU_auto_register("Opening UDP socket");

  if ((_udpFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Could not open UDP socket, port " << _port << endl;
    
    return false;
  }

  // Make the socket non-blocking 

  int blocking_flag = 1;

  if (ioctl(_udpFd, FIONBIO, &blocking_flag) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Could not make socket non-blocking, port " << _port << endl;
    
    return false;
  }

  // Set the socket for reuse

  int val = 1;

  setsockopt(_udpFd, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(val));
   
  // Bind address to the socket

  struct sockaddr_in udp_address;

  memset((void *)&udp_address, (int) 0, sizeof(udp_address));

  udp_address.sin_port = htons(_port);
  udp_address.sin_family = AF_INET;
  udp_address.sin_addr.s_addr = htonl( INADDR_ANY );
   
  if (_debug)
    cerr << "Binding address to socket" << endl;
  
  PMU_auto_register("Binding address to socket");

  if (bind(_udpFd, (struct sockaddr *)&udp_address, 
	   sizeof(udp_address)) < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Could not bind UDP socket, port " << _port << endl;
    
    return false;
  }

  if (_debug)
    cerr << "UdpReader object successfully initialized" << endl;

  return true;
}


/*********************************************************************
 * _readBytes() - Read the next group of bytes from the source.
 *
 * Returns the number of bytes read from the source.
 */

int UdpReader::_readBytes(char *buffer, const int buffer_size)
{
  static const string method_name = "UdpReader::readMsg()";
  
  int buf_len = 0;
   
  struct sockaddr_in from_address;
  int status;

#if defined(__linux)
  socklen_t addrLen = sizeof(struct sockaddr_in);
#else
  int addrLen = sizeof(struct sockaddr_in);
#endif

  // Wait on socket for up to 10 secs at a time

  while ((status = SKU_read_select(_udpFd, 10000)) == -1)
  {
    PMU_auto_register("Waiting to read UDP data");
  
    if (_debug)
      cerr << "Waiting to read udp data" << endl;
  }
  
  if (status == -2)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Cannot read from UDP port" << endl;
    
    return -1;
  }

  errno = EINTR;
  while (errno == EINTR || errno == EWOULDBLOCK)
  {
    PMU_auto_register("Reading UDP data");

    errno = 0;
    buf_len = recvfrom(_udpFd, buffer, buffer_size, 0, 
		       (struct sockaddr *) &from_address, 
		       &addrLen);
      
    if (errno == EWOULDBLOCK)
      sleep(1);
  }

  return buf_len;
}
