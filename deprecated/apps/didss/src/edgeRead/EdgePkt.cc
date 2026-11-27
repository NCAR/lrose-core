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
// Edge Packet base class
//////////////////////////////////////////////////////////////////////////

#include <cerrno>
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
#elif defined(__APPLE__)
#include <sys/ioctl.h>
#else
#include <sys/filio.h>
#endif

#include "EdgePkt.hh"
#include "Status.hh"
#include "Ray.hh"
#include "EdgeRead.hh"
using namespace std;

const int EdgePkt::MAX_PKT_SIZE = 1524;
const int EdgePkt::HEADER_SIZE  = 40;

EdgePkt::EdgePkt( int portNum, char* logFileName ) 
{
   azimuth         = 0;
   elevation       = 0;
   checkSum        = 0;
   uncompressedLen = 0;
   compressedLen   = 0;
   moment          = 0;
   compType        = NONE;

   statusInfo      = new Status();
   beamData        = new Ray();

   buffer          = new char[MAX_PKT_SIZE];
   bufLen          = MAX_PKT_SIZE;
   packetLen       = 0;

   port            = portNum;
   udpFd           = -1;

   if( logFileName ) {
      logName      = STRdup( logFileName );
      logEnabled   = true;
   } else {
      logName      = NULL;
      logEnabled   = false;
   }

   memset( (void *) &udpAddress, (int) 0, sizeof( udpAddress ) );
   memset( (void *) buffer, (int) 0, MAX_PKT_SIZE );
   
}

EdgePkt::~EdgePkt() 
{
   if( buffer )
      delete[] buffer;

   if( udpFd >= 0 ) {
      close( udpFd );
   }

   if( logName )
      STRfree( logName );
}

int
EdgePkt::init() 
{
   int val          = 1;
   int valLen       = sizeof( val );
   int blockingFlag = 1;

   //
   // Open the socket
   //
   POSTMSG( INFO, "Openning udp socket" );
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
   if( bind( udpFd, (struct sockaddr *) &udpAddress, 
             sizeof( udpAddress ) ) < 0 ) {
      POSTMSG( ERROR, "Could not bind UDP socket, port %d", port );
      return( FAILURE );
   }

   return( SUCCESS );
}

int
EdgePkt::readUdp() 
{
  struct sockaddr_in fromAddress;
  int                status;

  socklen_t addrLen = sizeof( struct sockaddr_in );

  //
  // Wait on socket for up to 10 secs at a time
  //
  while( (status = SKU_read_select(udpFd, 10000)) == -1 ) {
     POSTMSG( DEBUG, "Waiting to read udp data" );
  }
  
  if( status == -2 ) {
     POSTMSG( ERROR, "Cannot read from udp" );
     return( FAILURE );
  }

  errno = EINTR;
  while (errno == EINTR || errno == EWOULDBLOCK) {
     errno     = 0;
     packetLen = recvfrom( udpFd, buffer, MAX_PKT_SIZE, 0, 
                           (struct sockaddr *) &fromAddress, 
                           &addrLen);
      
     if (errno == EWOULDBLOCK)
        sleep(1);
  }
  
  if (packetLen <= 0) {
     POSTMSG( ERROR, "Empty packet" );
     return( FAILURE );
  }

  if( readMsg( packetLen ) != SUCCESS )
     return( FAILURE );

  return( SUCCESS );

}

int
EdgePkt::printInfo() 
{
   //
   // If the log is not enabled, do nothing
   //
   if( !logEnabled ) 
      return( SUCCESS );

   //
   // Open log file
   //
   FILE *logFp;
   if( (logFp = ta_fopen_uncompress( logName, "a")) == NULL ) {
      POSTMSG( ERROR, "Could not open log file" );
      return( FAILURE );
   }

   printHdr( logFp );
   statusInfo->printInfo( logFp );

   fclose( logFp );
   return( SUCCESS );
}

int
EdgePkt::readMsg( int packetLen ) 
{
   if( readHdr() != SUCCESS )
      return( FAILURE );
   
   if( uncompress() != SUCCESS )
      return( FAILURE );
   
   if( moment == 1 || moment == 2 || moment == 4 || moment == 8 ) {
      fprintf( stderr, "moment = %d\n", moment );
      if( beamData->readMsg( buffer+HEADER_SIZE, 
                             uncompressedLen ) != SUCCESS )
        POSTMSG( WARNING, "Could not read data packet" );
   } else if( moment == 32 ) {
     if( statusInfo->readMsg( buffer+HEADER_SIZE ) == SUCCESS ) {
       //
       // Check azimuth and elevation
       //
       if( azimuth != statusInfo->getAzimuth() ||
	   elevation != statusInfo->getElevation() ) {
	  POSTMSG( ERROR, "Azimuth and elevation in status message "
		   "do not match the header" );
          return( FAILURE );
       }
     } else {
       POSTMSG( WARNING, "Could not read status packet" );
     }
   } 

   return( SUCCESS );
}

int
EdgePkt::readHdr() 
{
   int binaryAz, binaryEl, compression;
   
   int nParts = sscanf( buffer, "%04x %04x %08x %4d %4d %04x %1d",
	                &binaryAz, &binaryEl, &checkSum, &uncompressedLen,
	                &compressedLen, &moment, &compression );
   
   if( nParts != 7 ) {
      POSTMSG( ERROR, "Couldn't read the header" );
      return( FAILURE );
   }

   azimuth   = binaryAz/65536.0 * 360.0;
   elevation = binaryEl/65536.0 * 360.0;

   switch( compression ) {
       case 0: 
	  compType = NONE;
	  break;
	  
       case 1:
	  compType = LZW;
	  break;
	  
       case 2:
	  compType = RUN_LEN;
	  break;
	  
       default:
	  compType = UNKNOWN;
	  break;
   }
   
   return( SUCCESS );
}

int
EdgePkt::uncompress() 
{
   switch( compType ) {
       case NONE:
	  return( SUCCESS );
	  break;
	  
       case LZW:
	  POSTMSG( ERROR, "LZW compression not supported" );
	  return( FAILURE );
	  break;
	  
       case RUN_LEN:
	  POSTMSG( ERROR, "RUN_LEN compression not supported" );
	  return( FAILURE );
	  break;
	  
       case UNKNOWN:
	  POSTMSG( ERROR, "Compression type unknown" );
	  return( FAILURE );
	  break;
   }
   
   return( SUCCESS );
}

void
EdgePkt::printHdr( FILE* stream ) 
{
   //
   // Print out header information
   //
   fprintf( stream, "HEADER:\n\n" );
   fprintf( stream, "azimuth = %f\n", azimuth );
   fprintf( stream, "elevation = %f\n", elevation );
   fprintf( stream, "uncompressed length = %d\n", uncompressedLen );
   fprintf( stream, "compressed length = %d\n", compressedLen );
   fprintf( stream, "moment = %d\n", moment );
      
   switch( compType ) {
       case NONE:
	  fprintf( stream, "no compression\n\n" );
	  break;
	     
       case LZW:
	  fprintf( stream, "lzw compression\n\n" );
	  break;
	     
       case RUN_LEN:
	  fprintf( stream, "run length encoding\n\n" );
	  break;
	     
       case UNKNOWN:
	  fprintf( stream, "compression type unknown\n\n" );
	  break;
   }

   
}

      


   
   
   



