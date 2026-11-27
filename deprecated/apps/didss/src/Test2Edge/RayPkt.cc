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
//////////////////////////////////////////////////////////////////
// $Id: RayPkt.cc,v 1.7 2016/03/06 23:53:41 dixon Exp $
//
// Ray Packet class
/////////////////////////////////////////////////////////////////

#include <math.h>
#include <toolsa/MsgLog.hh>
#include "RayPkt.hh"
#include "Test2Edge.hh"
using namespace std;

RayPkt::RayPkt( int portNum, char* logName ) 
       :EdgePkt( portNum, logName )
{
   numPts      = 0;
   byteData    = NULL;
   currentTime = 0;
}

RayPkt::~RayPkt() 
{
   if( byteData )
      delete[] byteData;
}

int
RayPkt::init( char* broadcastAddress, int nGates ) 
{
   if( initUdp( broadcastAddress ) != SUCCESS ) 
      return( FAILURE );
   
   numPts   = nGates;
   byteData = new ui08[numPts];

   memset( (void *) byteData, (int) 0, numPts );

   uncompressedLen = HEADER_SIZE + numPts;
   compressedLen   = uncompressedLen;
   compType        = NONE;

   return( SUCCESS );
   
}

void 
RayPkt::simulate( time_t now )
{
   static int  count = 0;
   static ui16 lastMoment = 0;

   currentTime = now;
   
   moment = 1;
   for( int i = 0; i < count; i++ )
      moment *= 2;

   count++;
   if( count > 3 )
      count = 0;

   if( moment == 0 || moment == lastMoment ) {
      
      azimuth += 1;
      if( azimuth >= 360 ) {
	 elevation += 1;
         azimuth    = 0;
      }
      if( elevation >= 10 ) {
	 elevation  = 0;
      }
   }

   lastMoment = moment;
}

int
RayPkt::broadcast()
{
   //
   // No check sum is actually performed
   //
   checkSum = 0;
   
   //
   // Header
   //
   if( loadHdr() != SUCCESS ) 
      return( FAILURE );
   char *bufferPtr = buffer + HEADER_SIZE;

   //
   // Data
   //
   memcpy( (void *) bufferPtr, (void *) byteData, numPts );

   //
   // Write buffer to log file
   //
   FILE *logFilePtr;
   if( (logFilePtr = fopen( logFileName, "a" )) != NULL ) {
      fprintf( logFilePtr, "Ray at %s:\n", 
               asctime( localtime(&currentTime) ) );
      fprintf( logFilePtr, "%s", bufferPtr );
      fclose( logFilePtr );
   } else {
      POSTMSG( ERROR, "Could not open log file" );
      return( FAILURE );
   };

   //
   // Write to udp
   //
   if( sendUdp() != SUCCESS )
      return( FAILURE );
   
   return( SUCCESS );
   
}

   
   
