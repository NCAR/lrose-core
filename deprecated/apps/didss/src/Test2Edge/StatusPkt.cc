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
////////////////////////////////////////////////////////////////////////////
// $Id: StatusPkt.cc,v 1.11 2016/03/06 23:53:41 dixon Exp $
//
// Status Packet class
///////////////////////////////////////////////////////////////////////////

#include <toolsa/MsgLog.hh>
#include "StatusPkt.hh"
#include "Test2Edge.hh"
using namespace std;

const unsigned int StatusPkt::RANGE_MAX = 99999999;
const int StatusPkt::STATUS_FLAGS       = 9;
const int StatusPkt::BITE_SECTIONS      = 2;
const int StatusPkt::BITE_FLAGS         = 10;
const int StatusPkt::STRING_PRINT_LEN   = 18;
const int StatusPkt::STATUS_LEN         = 350;

StatusPkt::StatusPkt( int portnum, char* logName ) 
          : EdgePkt( portnum, logName ) 
{
   prf1          = 1;
   prf2          = 1;
   range         = 1;
   samples       = 1;

   gw1           = 2;
   gw2           = 2;
   gwPartition   = 2;
   rangeAvg      = 2;
   gates         = 2;

   momentEnable  = 3;
   softwareSim   = 3;

   scanType      = 4;
   targetAz      = 4;
   targetElev    = 4;
   speed         = 4;
   antennaSpeed  = 4;
   elevSpeed     = 4;
   startAngle    = 4;
   stopAngle     = 4;

   dataTime      = 5;
   siteName      = "";
   radarType     = "";
   jobName       = "";
   
   lon           = new LatLon( 0.0 );
   lat           = new LatLon( 0.0 );
   antennaHeight = 6;

   scdFlag       = 7;
   
   sigprocFlag   = 8;
   interfaceType = 8;
   radarPower    = 8;
   servo         = 8;
   radiate       = 8;
   flags         = 8;

   tcfZ          = 9;
   tcfU          = 9;
   tcfV          = 9;
   tcfW          = 9;
   clutterFilter = 9;
   sqi           = 9;
   pulseWidth    = 9;
   fold          = 9;
   
   rcuStatus     = new unsigned int[STATUS_FLAGS];
   memset( (void *) rcuStatus, (int) 0, 
           STATUS_FLAGS*sizeof( unsigned int ) );
   
   rcuBite       = new unsigned int*[BITE_SECTIONS];
   for( int i = 0; i < BITE_SECTIONS; i++ ) {
      rcuBite[i] = new unsigned int[BITE_FLAGS];
      memset( (void *) rcuBite[i], (int) 0, 
              BITE_FLAGS*sizeof( unsigned int ) );
   }
   
   rcuBiteDt     = 0;

}

StatusPkt::~StatusPkt() 
{
   if( lon ) 
      delete lon;
   if( lat )
      delete lat;
   
   if( rcuStatus )
      delete[] rcuStatus;
   if( rcuBite ) {
      for( int i = 0; i < BITE_SECTIONS; i++ ) {
	 if( rcuBite[i] )
	    delete[] rcuBite[i];
      }
      delete[] rcuBite;
   }
}

int
StatusPkt::init( char* broadcastAddress,
                 double latitude, double longitude, 
                 char* site, char* radar, char* job,
                 unsigned int prf, unsigned int nGates,
                 unsigned int gateSpacing )
{   
   if( initUdp( broadcastAddress ) != SUCCESS )
      return( FAILURE );
   
   prf1         = prf;
   gates        = nGates;
   gw1          = gateSpacing;
   range        = gates * gw1;
   momentEnable = 1;

   if( range > RANGE_MAX ) {
      POSTMSG( ERROR, "Range too large" );
      return( FAILURE );
   }

   if( site )
      siteName = site;
   if( radar )
      radarType = radar;
   if( job )
      jobName = job;

   lat->set( latitude );
   lon->set( longitude );

   int siteLen = siteName.size();
   if( siteLen > STRING_PRINT_LEN )
      siteLen = STRING_PRINT_LEN;
   int radarLen = radarType.size();
   if( radarLen > STRING_PRINT_LEN )
      radarLen = STRING_PRINT_LEN;
   int jobLen = jobName.size();
   if( jobLen > STRING_PRINT_LEN )
      jobLen = STRING_PRINT_LEN;
   
   uncompressedLen = HEADER_SIZE + STATUS_LEN + siteLen + radarLen + jobLen;
   compressedLen  = uncompressedLen;

   return( SUCCESS );
}

void
StatusPkt::simulate( time_t now )
{
   moment       = STATUS_MOMENT;

   azimuth     += 1;
   if( azimuth >= 360 ) {
      elevation += 1;
      azimuth    = 0;
   }
   if( elevation >= 10 ) {
      elevation = 0;
   }
   
   targetAz     = azimuth;
   targetElev   = elevation;
   dataTime     = now;
   momentEnable = moment;
}

int
StatusPkt::broadcast()
{
   //
   // Header
   //
   if( loadHdr() != SUCCESS ) 
      return( FAILURE );
   char *bufferPtr = buffer + HEADER_SIZE;

   //
   // open log file
   //
   FILE *logFilePtr;
   if( (logFilePtr = fopen( logFileName, "a" )) == NULL ) {
      POSTMSG( ERROR, "Could not open log file" );
      return( FAILURE );
   } 
   
   //
   // Record 1
   //
   sprintf( bufferPtr, "%4d %4d %8d %3d",
            prf1, prf2, range, samples );
   fprintf( logFilePtr, "%s ", bufferPtr );
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 2
   //
   sprintf( bufferPtr, "%5d %5d %8d %3d %4d",
            (int) gw1, (int) gw2, gwPartition,
            rangeAvg, gates );
   fprintf( logFilePtr, "%s ", bufferPtr );
   bufferPtr += strlen( bufferPtr ) + 1;

   //
   // Record 3
   //
   sprintf( bufferPtr, "%02x %1d",
            momentEnable, softwareSim );
   fprintf( logFilePtr, "%s ", bufferPtr );
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 4
   //
   sprintf( bufferPtr, "%1d %04x %04x %04x %04x %04x %04x %04x",
            scanType, targetAz, targetElev, speed, antennaSpeed,
            elevSpeed, startAngle, stopAngle );
   fprintf( logFilePtr, "%s ", bufferPtr );
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 5 - Note that we cannot use the size function on
   // the strings since there may be a null terminator before
   // the end of this string (since we are using substrings)
   //
   string site  = siteName.substr( 0, STRING_PRINT_LEN );
   string radar = radarType.substr( 0, STRING_PRINT_LEN );
   string job   = jobName.substr( 0, STRING_PRINT_LEN );
   sprintf( bufferPtr, "%08x \"%s\" \"%s\" \"%s\"",
	    dataTime, site.c_str(), radar.c_str(), job.c_str() );
   fprintf( logFilePtr, "%s ", bufferPtr );
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 6
   //
   sprintf( bufferPtr, "%3d %3d %3d %3d %3d %3d %5d",
            lon->getDegrees(), lon->getMinutes(), lon->getSeconds(),
            lat->getDegrees(), lat->getMinutes(), lat->getSeconds(),
            antennaHeight );
   fprintf( logFilePtr, "%s ", bufferPtr );
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 7
   //
   sprintf( bufferPtr, "%04x %04x %04x",
	    azimuth, elevation, scdFlag );
   fprintf( logFilePtr, "%s ", bufferPtr );
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 8
   //
   sprintf( bufferPtr, "%1d %1d %2d %2d %2d",
	    sigprocFlag, interfaceType, radarPower, servo, radiate );
   fprintf( logFilePtr, "%s ", bufferPtr );
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 9
   //
   sprintf( bufferPtr, "%04x %04x %04x %04x %04x %1d %3d %1d %1d",
	    flags, tcfZ, tcfU, tcfV, tcfW, clutterFilter, sqi,
	    pulseWidth, fold );
   fprintf( logFilePtr, "%s ", bufferPtr );
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 10
   //
   sprintf( bufferPtr, "%02x %02x %02x %02x %02x %02x %02x %02x %02x",
	    rcuStatus[0], rcuStatus[1], rcuStatus[2], rcuStatus[3],
	    rcuStatus[4], rcuStatus[5], rcuStatus[6], rcuStatus[7],
	    rcuStatus[8] );
   fprintf( logFilePtr, "%s ", bufferPtr );
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 11
   //
   sprintf( bufferPtr, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", 
            rcuBite[0][0], rcuBite[0][1], rcuBite[0][2], rcuBite[0][3], 
            rcuBite[0][4], rcuBite[0][5], rcuBite[0][6], rcuBite[0][7], 
            rcuBite[0][8], rcuBite[0][9] );
   fprintf( logFilePtr, "%s ", bufferPtr );
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 12
   //
   sprintf( bufferPtr, "%02x %02x %02x %02x %02x %02x %02x "
                       "%02x %02x %02x %5d", 
            rcuBite[1][0], rcuBite[1][1], rcuBite[1][2], rcuBite[1][3], 
            rcuBite[1][4], rcuBite[1][5], rcuBite[1][6], rcuBite[1][7], 
            rcuBite[1][8], rcuBite[1][9], rcuBiteDt );
   fprintf( logFilePtr, "%s ", bufferPtr );
   fprintf( logFilePtr, "\n\n" );

   //
   // close the log file
   //
   fclose( logFilePtr );

   //
   // Send the buffer to udp
   //
   if( sendUdp() != SUCCESS )
      return( FAILURE );

   return( SUCCESS );
   
}

   
   
   
   
   
   

   
   
