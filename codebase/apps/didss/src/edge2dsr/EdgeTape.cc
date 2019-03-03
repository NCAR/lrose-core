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
#include <fcntl.h>
#include <dataport/bigend.h>
#include <toolsa/ttape.h>
#include <toolsa/sockutil.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/MsgLog.hh>

#include "EdgeTape.hh"
#include "Edge2Dsr.hh"
using namespace std;

//
// Constants
//
const int EdgeTape::MAX_TAPE_BLOCK      = 1024;
const int EdgeTape::READ_TRIES          = 1;
const int EdgeTape::READ_TRIES_AT_START = 5;

EdgeTape::EdgeTape( char *dev, int waitMsecs, int hdrSz ) 
{
   device     = STRdup( dev );
   tapeId     = -1;
   waitUsecs  = waitMsecs * 1000;

   if( strchr(device, ':') != NULL )
      remoteTape = true;
   else
      remoteTape = false;

   headerSize = hdrSz;
   beginning  = true;

}

EdgeTape::~EdgeTape() 
{
   if( tapeId >= 0 )
      close( tapeId );
}

int
EdgeTape::init() 
{
   if( !remoteTape ) {
      
      if( (tapeId = open( device, O_RDONLY )) < 0 ) {
	 perror( device );
	 return( FAILURE );
      }
      
      TTAPE_set_var( tapeId );
      POSTMSG( DEBUG, "Successfully opened tape %s, fd = %d",
	       device, tapeId );
   } else {

      if( RMT_open( device, O_RDONLY, 0666 ) < 0 ) {
	 perror( device );
	 return( FAILURE );
      }
      
      POSTMSG( DEBUG, "Successfully opened remote tape %s", device );
   }

  return( SUCCESS );

}

int 
EdgeTape::getMsg( char* buffer, int* bufferLen )
{
   int nread;

   PMU_auto_register( "Reading tape record" );
   if( (nread = getPhysicalRec( buffer )) < 0 ) {
      return( FAILURE );
   }
   *bufferLen = nread;

   uusleep( waitUsecs );
  
   return( SUCCESS );

}

int 
EdgeTape::getPhysicalRec( char* buffer )
{
  int nread = 0;
  int nerr;

  PMU_auto_register( "Reading physical tape record" );

  if( remoteTape ) {

     nread = RMT_read( (char *) buffer, MAX_TAPE_BLOCK );
     if( nread > 0) {
	return( nread );
     }

  } else {

    nerr = 0;
    while( nerr < 50 ) {

      errno = EINTR;
      while (errno == EINTR) {
	errno = 0;
	nread = read( tapeId, buffer, MAX_TAPE_BLOCK );
      }
      
      if( nread > 0 ) {
	return( nread );
      } else {
	nerr++;
      }

      PMU_auto_register( "Reading tape" );
      uusleep(1000);
      
    }
  }

  POSTMSG( INFO, "Tape not ready, or no data left" );
  return( -1 );

}
