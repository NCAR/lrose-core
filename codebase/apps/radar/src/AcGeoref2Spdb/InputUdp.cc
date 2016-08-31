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
///////////////////////////////////////////////////////////////
// InputUdp.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2016
//
///////////////////////////////////////////////////////////////
//
// InputUdp reads data in from UDP packets
//
////////////////////////////////////////////////////////////////

#include "InputUdp.hh"
#include <toolsa/pmu.h>
#include <toolsa/sockutil.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>
#if defined(__linux)
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#else
#include <sys/filio.h>
#endif

using namespace std;

// Constructor

InputUdp::InputUdp()
  
{
  _udpFd = -1;
}


// destructor

InputUdp::~InputUdp()

{
  closeUdp();
}

///////////////////
// open port

int InputUdp::openUdp(int port, bool debug)
  
{

  closeUdp();
  
  // get socket

  if  ((_udpFd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    int errNum = errno;
    cerr << "ERROR - InputUdp::open" << endl;
    cerr << "  Could not create socket." << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // set the socket for reuse

  int val = 1;
  int valen = sizeof(val);
  setsockopt(_udpFd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, valen);
  
  // bind local address to the socket

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof (addr));
  addr.sin_port = htons(port);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if (bind (_udpFd, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
    int errNum = errno;
    cerr << "ERROR - InputUdp::open" << endl;
    cerr << "  Bind error, udp port: " << port << endl;
    cerr << "  " << strerror(errNum) << endl;
    closeUdp();
    return -1;
  }

  if (debug) {
    cerr << "Opened UDP port: " << port << endl;
  }
  
  return 0;
  

}

/////////////
// close port

void InputUdp::closeUdp()
  
{
  
  if (_udpFd >= 0) {
    close(_udpFd);
  }
  _udpFd = -1;

}

///////////////////////////////////////////
// read data from socket, puts into buffer
//
// Returns 0 on success, -1 on failure

int InputUdp::readPacket()

{

  PMU_auto_register("Reading UDP");
  
  // check for data, using select
  
  while (true) {

    int iret = SKU_read_select(_udpFd, 1000);

    if (iret == 1) {
      break;
    } // success

    if (iret == -2) {
      cerr << "ERROR - InputUdp::readPacket()" << endl;
      cerr << "  Cannot perform select on UDP socket" << endl;
      return -1;
    }

    // timeout, so register with procmap

    PMU_auto_register("Zzzzz");

  }

  struct sockaddr_in from_name;
  int fromlen = sizeof(from_name);
  _len = recvfrom(_udpFd, _buf, maxUdpBytes, 0,
                  (struct sockaddr *) &from_name, (socklen_t*)&fromlen);
  if (_len < 0) {
    int errNum = errno;
    cerr << "ERROR - InputUdp::readPacket()" << endl;
    cerr << "  recvfrom: " << strerror(errNum) << endl;
    return -1;
  }

  if (_len < 2) {
    cerr << "ERROR reading packet, too short, len: " << _len << endl;
    return -1;
  }
  
  return 0;

}

