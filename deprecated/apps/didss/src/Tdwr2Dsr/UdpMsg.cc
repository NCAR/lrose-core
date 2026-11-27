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
// UdpMsg.cc
//
// UdpMsg object
//
// Gary Blackburn, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2001
//
///////////////////////////////////////////////////////////////
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <toolsa/pmu.h>
#include <iostream>
#include <string.h>
#include <toolsa/sockutil.h>


//#include <toolsa/mem.h>
//#include <toolsa/str.h>

#if defined(__linux)
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#else
#include <sys/filio.h>
#endif

#include "UdpMsg.hh"
using namespace std;

UdpMsg::UdpMsg( int portNum, int maxSize ) 
{
   _port            = portNum;
   _maxPktSize      = maxSize;
   _udp_fd           = -1;
   int val          = 1;
   int valLen       = sizeof( val );
   int blockingFlag = 1;


   memset( (void *) &_input_addr, (int) 0, sizeof( sockaddr_in ) );

   //
   // Open the socket
   //
   PMU_auto_register( "Opening udp socket" );
   if( (_udp_fd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP )) < 0 ) {
      cerr << "ERROR, opening UDP socket, port " << _port << endl;
      exit (0);
   }

   cerr << "opened socked, fd = " << _udp_fd << endl;
   //
   // Make the socket non-blocking 
   //
   if (ioctl(_udp_fd, FIONBIO, &blockingFlag) != 0) {
      cerr << "ERROR, configuring non-blocking socket, port " << _port << endl;
      exit (0);
   }

   //
   // Set the socket for reuse
   //
   setsockopt(_udp_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, valLen);
   
   //
   // Bind address to the socket
   //
   _input_addr.sin_port        = htons( _port );
   _input_addr.sin_family      = AF_INET;
   _input_addr.sin_addr.s_addr = htonl( INADDR_ANY );
   
   cerr << "Binding address to socket" << endl;
   PMU_auto_register( "Binding address to socket" );
   if( bind( _udp_fd, (struct sockaddr *) &_input_addr, sizeof( _input_addr ) ) < 0 ) {
      cerr << "ERROR, Could not bind UDP socket, port " << _port << endl;
      exit (0);
   }

}

// destructor
UdpMsg::~UdpMsg() 
{
   if( _udp_fd >= 0 )
      close( _udp_fd );
}



////////////////////////////////////////////////////////////////////

int
UdpMsg::readUdp( char* buffer ) 
{
  int                bufLen = 0;
   
  struct sockaddr_in addr;
  int                status;

  socklen_t          addrLen = sizeof( struct sockaddr_in );

  PMU_auto_register( "Reading udp data" );
  /*
   * wait on socket for up to 10 secs at a time
   */

  while ((status = SKU_read_select(_udp_fd, 10000)) == -1) {
  /*
   * timeout
   */
     PMU_auto_register("Waiting for udp data");
  }

  bufLen    = recvfrom( _udp_fd, buffer, _maxPktSize, 0, 
                           (struct sockaddr *) &addr, 
                           &addrLen);
      
  return( bufLen );

}

      
////////////////////////////////////////////////////////////////////



   
   
   



