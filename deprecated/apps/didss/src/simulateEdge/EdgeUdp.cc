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
// Edge Udp class
//////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/sockutil.h>
#include <toolsa/file_io.h>
#include <toolsa/MsgLog.hh>

#if defined(__linux)
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#else
#include <sys/filio.h>
#endif

#include "EdgeUdp.hh"
#include "SimEdge.hh"
using namespace std;

EdgeUdp::EdgeUdp( int portNum, int maxSize ) 
{
   port            = portNum;
   maxPktSize      = maxSize;
   udpFd           = -1;

   memset( (void *) &localAddress, (int) 0, sizeof( localAddress ) );
   memset( (void *) &outputAddress, (int) 0, sizeof( outputAddress ) );
   
}

EdgeUdp::~EdgeUdp() 
{
   if( udpFd >= 0 )
      close( udpFd );
}

int
EdgeUdp::init( char* broadcastAddress ) 
{
  //
   // Open the socket
   //
   POSTMSG( INFO, "Openning udp socket" );
   if( (udpFd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP )) < 0 ) {
      POSTMSG( ERROR, "Could not open UDP socket, port %d", port );
      return( FAILURE );
   }
   
   //
   // Bind local address to the socket
   //
   localAddress.sin_port        = htons( port );
   localAddress.sin_family      = AF_INET;
   localAddress.sin_addr.s_addr = htonl( INADDR_ANY );
   
   POSTMSG( INFO, "Binding address to socket" );
   if( bind( udpFd, (struct sockaddr *) &localAddress, 
             sizeof( localAddress ) ) < 0 ) {
      POSTMSG( ERROR, "Could not bind UDP socket, port %d", port );
      return( FAILURE );
   }
   
   //
   // Set socket for broadcast
   //
   POSTMSG( INFO, "Setting socket for broadcast" );
   int option = 1;
   if( setsockopt( udpFd, SOL_SOCKET, SO_BROADCAST,
		   (char *) &option, sizeof( option ) ) < 0 ) {
      POSTMSG( ERROR, "Could not set broadcast on, port %d", port );
      return( FAILURE );
   }
   
   //
   // Set up output address
   //
   outputAddress.sin_family = AF_INET;
   outputAddress.sin_port   = htons( port );

#if (defined SUNOS4) || (defined SUNOS5)
   outputAddress.sin_addr.S_un.S_addr = inet_addr( broadcastAddress );
#else
   if( inet_aton( broadcastAddress, &outputAddress.sin_addr ) == 0 ) {
      POSTMSG( ERROR, "Cannot translate address %s", broadcastAddress );
      return( FAILURE );
   }
#endif

   return( SUCCESS );
}

int
EdgeUdp::writeUdp( char* buffer, int bufLen ) 
{
  if( udpFd < 0 ) {
      POSTMSG( ERROR, "Udp socket not set" );
      return( FAILURE );
   }

   POSTMSG( INFO, "Sending buffer to socket" );
   if( sendto( udpFd, buffer, bufLen, 0,
               (struct sockaddr *) &outputAddress,
               sizeof( outputAddress ) ) != bufLen ) {
      POSTMSG( ERROR, "Could not send message" );
      return( FAILURE );
   }
 
   return( SUCCESS );

}

      


   
   
   



