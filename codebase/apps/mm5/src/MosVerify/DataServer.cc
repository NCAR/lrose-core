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
// 
// Reads data from an spdb server and creates a list of entries 
//
// $Id: DataServer.cc,v 1.8 2016/03/07 01:33:51 dixon Exp $
//
///////////////////////////////////////////////////////////////
#include <ctime>
#include <cassert>
#include <toolsa/utim.h>
#include <rapmath/math_macros.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MsgLog.hh>
#include <symprod/spdb_products.h>
#include <rapformats/ComboPt.hh>
#include <rapformats/GenPt.hh>

#include "MosVerify.hh"
#include "DataServer.hh"
using namespace std;

void
DataServer::init( char* url, time_t start, 
                  time_t end, bool respectTypes ) 
{
   respectDataTypes = respectTypes;
   urlStr           = url;
   startTime        = start;
   endTime          = end;
}

int
DataServer::readData( char* stationId, int leadTime )
{
   assert( stationId );
   
   int dataType = Spdb::hash4CharsToInt32( stationId );

   time_t now      = time( NULL );
   time_t newStart = MIN( startTime, now );
   time_t newEnd   = MIN( endTime, now );

   //
   // Get data
   //
   if ( spdbMgr.getInterval( urlStr, 
                             newStart,
                             newEnd,
                             dataType,
                             leadTime,
                             false,
                             respectDataTypes ) != 0 ) {
      POSTMSG( INFO, "Problem reading data with url %s", urlStr.c_str() );
      return( FAILURE );
   }

   int nChunks = spdbMgr.getNChunks();
   if( nChunks < 1 ) {

      DateTime start( newStart );
      DateTime end( newEnd );

      if( respectDataTypes ) {
         POSTMSG( INFO, "No metar data for station %s with lead time = %d "
                  "between %s and %s", stationId, leadTime, start.dtime(),
                  end.dtime() );
      }
      else {
         POSTMSG( INFO, "No metar data for station %s between %s and %s", 
                  stationId, start.dtime(), end.dtime() );
      }
   }
   
   return( SUCCESS );
}








