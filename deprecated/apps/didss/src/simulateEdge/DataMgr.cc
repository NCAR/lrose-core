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
/////////////////////////////////////////////////
// Data managment class
////////////////////////////////////////////////
#include <unistd.h>
#include <toolsa/str.h>
#include <toolsa/file_io.h>
#include <toolsa/MsgLog.hh>

#include "DataMgr.hh"
#include "EdgeUdp.hh"
#include "SimEdge.hh"
using namespace std;

//
// Constants
//
const int DataMgr::MAX_BUF_LEN = 1524;
const int DataMgr::MAX_LINE    = 256;
const int DataMgr::STATUS      = 32;
const int DataMgr::HEADER_SIZE = 40;

DataMgr::DataMgr() 
{
   archiveFp = NULL;
   filePath  = NULL;
   edgeUdp   = NULL;
   buffer    = new char[MAX_BUF_LEN];
   bufLen    = 0;
}

DataMgr::~DataMgr() 
{
   if( archiveFp )
      fclose( archiveFp );
   if( edgeUdp )
      delete edgeUdp;
   if( buffer )
      delete[] buffer;
   if( filePath )
      STRfree( filePath );
}

int
DataMgr::init( Params& params ) 
{
   //
   // Set up archive file path name
   //
   filePath = STRdup( params.archive_file );
   
   //
   // Initialize udp class
   //
   edgeUdp = new EdgeUdp( params.port, MAX_BUF_LEN );
   edgeUdp->init( params.broadcast_address );
   
   return( SUCCESS );
   
}

int
DataMgr::resetArchive() 
{
   //
   // Close file if it's open
   //
   if( archiveFp ) {
      fclose( archiveFp );
      archiveFp = NULL;
   }
   
   //
   // Open file
   //
   if( (archiveFp = ta_fopen_uncompress( filePath, "r" )) == NULL ) {
      POSTMSG( ERROR, "Could not open archive file %s", filePath );
      return( FAILURE );
   }

   return( SUCCESS );
}


void
DataMgr::writeData() 
{
   char line[MAX_LINE];
   int  moment;
   
   while( fread( (void *) line, (size_t) 1, (size_t) 9, archiveFp ) > 0 ) {
      sscanf( line, "  %d:  ", &bufLen );
      fread( (void *) buffer, (size_t) 1, (size_t) bufLen, 
	     archiveFp );
      if( (moment = findMoment( buffer )) < 0 ) {
         continue;
      }
      if( moment == STATUS ) {
         replaceTime( buffer );
      }
      edgeUdp->writeUdp( buffer, bufLen );
      usleep(30000);
   }

}

int
DataMgr::findMoment( char* buffer ) 
{
   int binaryAz, binaryEl, checkSum;
   int uncompressedLen, compressedLen;
   int moment, compression;
   
   //
   // Read the header
   //
   int nParts = sscanf( buffer, "%04x %04x %08x %4d %4d %04x %1d",
	                &binaryAz, &binaryEl, &checkSum, &uncompressedLen,
	                &compressedLen, &moment, &compression );
   
   if( nParts != 7 ) {
      POSTMSG( ERROR, "Couldn't read the header" );
      return( -1 );
   }

   return( moment );
}

void
DataMgr::replaceTime( char* buffer ) 
{
   char *bufferPtr = buffer + HEADER_SIZE;
  
   //
   // Record 1
   //
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 2
   //
   bufferPtr += strlen( bufferPtr ) + 1;

   //
   // Record 3
   //
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 4
   //
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 5 
   //
   time_t now = time(0);
   sprintf( bufferPtr, "%08x %s", (int) now, bufferPtr+9 );
 
}
