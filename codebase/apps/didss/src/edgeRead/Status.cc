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
// $Id: Status.cc,v 1.3 2016/03/06 23:53:42 dixon Exp $
//
// Status class
///////////////////////////////////////////////////////////////////////////

#include <toolsa/mem.h>
#include <toolsa/file_io.h>
#include <toolsa/MsgLog.hh>
#include <toolsa/DateTime.hh>

#include "Status.hh"
#include "EdgeRead.hh"
using namespace std;

const int Status::STATUS_FLAGS       = 9;
const int Status::BITE_SECTIONS      = 2;
const int Status::BITE_FLAGS         = 10;
const int Status::MAX_STRING_LEN     = 18;
const int Status::STATUS_LEN         = 292;

Status::Status() 
{
   prf1          = 0;
   prf2          = 0;
   range         = 0;
   samples       = 0;

   gw1           = 0.0;
   gw2           = 0.0;
   gwPartition   = 0;
   rangeAvg      = 0;
   gates         = 0;

   momentEnable  = 0;
   softwareSim   = 0;

   scanType      = 0;
   targetAz      = 0;
   targetElev    = 0;
   speed         = 0;
   antennaSpeed  = 0;
   elevSpeed     = 0;
   startAngle    = 0;
   stopAngle     = 0;

   dataTime      = 0;
   siteName      = (char *) ucalloc( MAX_STRING_LEN, sizeof( char ) );
   radarType     = (char *) ucalloc( MAX_STRING_LEN, sizeof( char ) );
   jobName       = (char *) ucalloc( MAX_STRING_LEN, sizeof( char ) );
   
   lon           = new LatLon( 0.0 );
   lat           = new LatLon( 0.0 );
   antennaHeight = 0;

   scdFlag       = 0;
   
   sigprocFlag   = 0;
   interfaceType = 0;
   radarPower    = 0;
   servo         = 0;
   radiate       = 0;
   flags         = 0;

   tcfZ          = 0;
   tcfU          = 0;
   tcfV          = 0;
   tcfW          = 0;
   clutterFilter = 0;
   sqi           = 0;
   pulseWidth    = 0;
   fold          = 0;
   
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

Status::~Status() 
{
   if( lon ) 
      delete lon;
   if( lat )
      delete lat;

   if( siteName )
      ufree( siteName );
   if( radarType )
      ufree( radarType );
   if( jobName )
      ufree( jobName );
   
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
Status::readMsg( char* buffer )
{
   char *bufferPtr = buffer;

   //
   // Record 1
   //
   if( sscanf( bufferPtr, "%4d %4d %8d %3d",
               &prf1, &prf2, &range, &samples ) != 4 ) {
      POSTMSG( ERROR, "Could not read record 1, %s", bufferPtr );
      return( FAILURE );
   }
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 2
   //
   if( sscanf( bufferPtr, "%f %f %8d %3d %4d",
               &gw1, &gw2, &gwPartition,
               &rangeAvg, &gates ) != 5 ) {
      POSTMSG( ERROR, "Could not read record 2, %s", bufferPtr );
      return( FAILURE );
   }
   bufferPtr += strlen( bufferPtr ) + 1;

   //
   // Record 3
   //
   if( sscanf( bufferPtr, "%02x %1d",
               &momentEnable, &softwareSim ) != 2 ) {
      POSTMSG( ERROR, "Could not read record 3, %s", bufferPtr );
      return( FAILURE );
   }
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 4
   //
   int binTargAz, binTargEl;
   if( sscanf( bufferPtr, "%1d %04x %04x %04x %04x %04x %04x %04x",
               &scanType, &binTargAz, &binTargEl, &speed, &antennaSpeed,
               &elevSpeed, &startAngle, &stopAngle ) != 8 ) {
      POSTMSG( ERROR, "Could not read record 4, %s", bufferPtr );
      return( FAILURE );
   }
   targetAz   = binTargAz/65536.0 * 360.0;
   targetElev = binTargEl/65536.0 * 360.0;

   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 5 - Note that we cannot use the size function on
   // the strings since there may be a null terminator before
   // the end of this string (since we are using substrings)
   //
   if( sscanf( bufferPtr, "%08x \"%[^\"]\" \"%[^\"]\" \"%[^\"]\"",
	       &dataTime, siteName, radarType, jobName ) != 4 ) {
      POSTMSG( ERROR, "Could not read record 5, %s", bufferPtr );
      return( FAILURE );
   }
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 6
   //
   int lonDeg, lonMin, lonSec;
   int latDeg, latMin, latSec;
   if( sscanf( bufferPtr, "%4d %3d %3d %4d %3d %3d %5d",
               &lonDeg, &lonMin, &lonSec, &latDeg, &latMin, &latSec,
               &antennaHeight ) != 7 )  {
      POSTMSG( ERROR, "Could not read record 6, %s", bufferPtr );
      return( FAILURE );
   }

   lon->set( lonDeg, lonMin, lonSec );
   lat->set( latDeg, latMin, latSec );
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 7
   //
   int binaryAz, binaryEl;
   if( sscanf( bufferPtr, "%04x %04x %04x",
	       &binaryAz, &binaryEl, &scdFlag ) != 3 ) {
      POSTMSG( ERROR, "Could not read record 7, %s", bufferPtr );
      return( FAILURE );
   }
   azimuth   = binaryAz/65536.0 * 360.0;
   elevation = binaryEl/65536.0 * 360.0;

   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 8
   //
   if( sscanf( bufferPtr, "%1d %1d %2d %2d %2d",
	       &sigprocFlag, &interfaceType, &radarPower, 
               &servo, &radiate ) != 5 ) {
      POSTMSG( ERROR, "Could not read record 8, %s", bufferPtr );
      return( FAILURE );
   }
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 9
   //
   if( sscanf( bufferPtr, "%04x %04x %04x %04x %04x %1d %3d %1d %1d",
	       &flags, &tcfZ, &tcfU, &tcfV, &tcfW, &clutterFilter, &sqi,
	       &pulseWidth, &fold ) != 9 ) {
      POSTMSG( ERROR, "Could not read record 9, %s", bufferPtr );
      return( FAILURE );
   }
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 10
   //
   if( sscanf( bufferPtr, "%02x %02x %02x %02x %02x %02x %02x %02x %02x",
	       &rcuStatus[0], &rcuStatus[1], &rcuStatus[2], &rcuStatus[3],
	       &rcuStatus[4], &rcuStatus[5], &rcuStatus[6], &rcuStatus[7],
	       &rcuStatus[8] ) != 9 ) {
      POSTMSG( ERROR, "Could not read record 10, %s", bufferPtr );
      return( FAILURE );
   }
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 11
   //
   if( sscanf( bufferPtr, "%02x %02x %02x %02x %02x %02x "
               "%02x %02x %02x %02x", 
               &rcuBite[0][0], &rcuBite[0][1], 
               &rcuBite[0][2], &rcuBite[0][3], 
               &rcuBite[0][4], &rcuBite[0][5], 
               &rcuBite[0][6], &rcuBite[0][7], 
               &rcuBite[0][8], &rcuBite[0][9] ) != 10 )  {
      POSTMSG( ERROR, "Could not read record 11, %s", bufferPtr );
      return( FAILURE );
   }
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 12
   //
   if( sscanf( bufferPtr, "%02x %02x %02x %02x %02x %02x %02x "
               "%02x %02x %02x %5d", 
                &rcuBite[1][0], &rcuBite[1][1], 
                &rcuBite[1][2], &rcuBite[1][3], 
                &rcuBite[1][4], &rcuBite[1][5], 
                &rcuBite[1][6], &rcuBite[1][7], 
                &rcuBite[1][8], &rcuBite[1][9], &rcuBiteDt ) != 11 ) {
      POSTMSG( ERROR, "Could not read record 12, %s", bufferPtr );
      return( FAILURE );
   }

   return( SUCCESS );
   
}

void
Status::printInfo( FILE* stream ) 
{
  
   //
   // Print status information
   //
   fprintf( stream, "STATUS:\n\n" );
   fprintf( stream, "prf = %d\n", prf1 );
   fprintf( stream, "range = %d\n", range );
   fprintf( stream, "samples = %d\n", samples );
   fprintf( stream, "gate spacing = %f\n", gw1 );
   fprintf( stream, "number of gates = %d\n", gates );
   fprintf( stream, "scan strategy = %d\n", scanType );
   fprintf( stream, "target azimuth = %f\n", targetAz );
   fprintf( stream, "target elevation = %f\n", targetElev );      

   DateTime beamTime( (time_t) dataTime );
   fprintf( stream, "data time: %s\n", beamTime.dtime() );
   
   fprintf( stream, "site name: %s\n", siteName );
   
   fprintf( stream, "latitude = %f\n", lat->getValue() );
   fprintf( stream, "longitude = %f\n", lon->getValue() );
   fprintf( stream, "pulse width = %d\n\n", pulseWidth );

   POSTMSG( DEBUG, "Printing message to log" );
   
}

   
   

   
   
   
   
   
   

   
   
