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
//////////////////////////////////////////////////////////////////////////
// $Id: EdgeUdp.cc,v 1.16 2016/03/06 23:53:42 dixon Exp $
//
// Edge Udp class
//////////////////////////////////////////////////////////////////////////
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
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

#include "EdgeUdp.hh"
#include "Edge2Dsr.hh"
using namespace std;

EdgeUdp::EdgeUdp( int portNum, int maxSize ) 
{
   port            = portNum;
   maxPktSize      = maxSize;
   udpFd           = -1;

   memset( (void *) &udpAddress, (int) 0, sizeof( sockaddr_in ) );
}

EdgeUdp::~EdgeUdp() 
{
   if( udpFd >= 0 )
      close( udpFd );
}

int
EdgeUdp::init() 
{
   int val          = 1;
   int valLen       = sizeof( val );
   int blockingFlag = 1;

   //
   // Open the socket
   //
   POSTMSG( INFO, "Openning udp socket" );
   PMU_auto_register( "Openning udp socket" );
   if( (udpFd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP )) < 0 ) {
      POSTMSG( ERROR, "Could not open UDP socket, port %d", port );
      return( FAILURE );
   }

   //
   // Make the socket non-blocking 
   //
   if (ioctl(udpFd, FIONBIO, &blockingFlag) != 0) {
      POSTMSG( ERROR, "Could not make socket non-blocking, port %d", port);
      return( FAILURE );
   }

   //
   // Set the socket for reuse
   //
   setsockopt(udpFd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, valLen);
   
   //
   // Bind address to the socket
   //
   udpAddress.sin_port        = htons( port );
   udpAddress.sin_family      = AF_INET;
   udpAddress.sin_addr.s_addr = htonl( INADDR_ANY );
   
   POSTMSG( INFO, "Binding address to socket" );
   PMU_auto_register( "Binding address to socket" );
   if( bind( udpFd, (struct sockaddr *) &udpAddress, 
             sizeof( udpAddress ) ) < 0 ) {
      POSTMSG( ERROR, "Could not bind UDP socket, port %d", port );
      return( FAILURE );
   }

   return( SUCCESS );
}

int
EdgeUdp::readUdp( char* buffer ) 
{
  int                bufLen = 0;
   
  struct sockaddr_in fromAddress;
  int                status;

  socklen_t          addrLen = sizeof( struct sockaddr_in );

  //
  // Wait on socket for up to 10 secs at a time
  //
  while( (status = SKU_read_select(udpFd, 10000)) == -1 ) {
     PMU_auto_register( "Waiting to read udp data" );
     POSTMSG( DEBUG, "Waiting to read udp data" );
  }
  
  if( status == -2 ) {
     POSTMSG( ERROR, "Cannot read from udp" );
     return( FAILURE );
  }

  errno = EINTR;
  while (errno == EINTR || errno == EWOULDBLOCK) {
     PMU_auto_register( "Reading udp data" );
     errno     = 0;
     bufLen    = recvfrom( udpFd, buffer, maxPktSize, 0, 
                           (struct sockaddr *) &fromAddress, 
                           &addrLen);
      
     if (errno == EWOULDBLOCK)
        sleep(1);
  }

  return( bufLen );

}

      


   
   
   



