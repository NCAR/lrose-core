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
///////////////////////////////////////////////
// $Id: EdgePkt.cc,v 1.10 2016/03/06 23:53:41 dixon Exp $
//
// Edge Packet base class
//////////////////////////////////////////////

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/MsgLog.hh>

#include "EdgePkt.hh"
#include "Test2Edge.hh"
using namespace std;

const int EdgePkt::MAX_PKT_SIZE  = 1524;
const int EdgePkt::HEADER_SIZE   = 40;
const int EdgePkt::STATUS_MOMENT = 0x0020;

EdgePkt::EdgePkt( int portNum, char* logName ) 
{
   azimuth         = 0;
   elevation       = 0;
   checkSum        = 0;
   uncompressedLen = 0;
   compressedLen   = 0;
   moment          = 0;
   compType        = NONE;

   buffer          = new char[MAX_PKT_SIZE];
   bufLen          = MAX_PKT_SIZE;

   port            = portNum;
   udpFd           = -1;

   MEM_zero( localAddress );
   MEM_zero( outputAddress );

   logFileName     = STRdup( logName );
   
}

EdgePkt::~EdgePkt() 
{
   if( buffer )
      delete[] buffer;

   if( udpFd >= 0 ) {
      close( udpFd );
   }

   if( logFileName ) 
      STRfree( logFileName );

}

int
EdgePkt::initUdp( char* broadcastAddress ) 
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
EdgePkt::sendUdp() 
{
   if( udpFd < 0 ) {
      POSTMSG( ERROR, "Udp socket not set" );
      return( FAILURE );
   }

   POSTMSG( INFO, "Sending buffer to socket" );
   if( sendto( udpFd, buffer, compressedLen, 0,
               (struct sockaddr *) &outputAddress,
               sizeof( outputAddress ) ) != compressedLen ) {
      POSTMSG( ERROR, "Could not send message" );
      return( FAILURE );
   }
 
   return( SUCCESS );
}

int
EdgePkt::loadHdr()
{
   int compression;
   
   switch( compType ) {
       case NONE:
	  compression = 0;
	  break;
	  
       case LZW:
	  compression = 1;
	  break;
	  
       case RUN_LEN:
	  compression = 2;
	  break;

       default:
	  compression = 3;
	  break;
	  
   }

   sprintf( buffer, "%04x %04x %08x %4d %4d %04x %1d",
            azimuth, elevation, checkSum, uncompressedLen,
            compressedLen, moment, compression );

   FILE *logFilePtr;
   if( (logFilePtr = fopen( logFileName, "a" )) != NULL ) {
      fprintf( logFilePtr, "%s", buffer );
      fclose( logFilePtr );
   } else {
      POSTMSG( ERROR, "Could not open log file" );
      return( FAILURE );
   }

   POSTMSG( DEBUG, "azimuth = %d, elevation = %d", azimuth, elevation );

   return( SUCCESS );
   
}

   
   
   



