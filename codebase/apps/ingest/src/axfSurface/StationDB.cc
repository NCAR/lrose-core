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
////////////////////////////////////////////////////////////////////////////////
//
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// August 1999
//
// $Id: StationDB.cc,v 1.4 2016/03/07 01:23:07 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <rapformats/station_reports.h>
#include <symprod/spdb_client.h>
#include <symprod/spdb_products.h>
#include <toolsa/MsgLog.hh>

#include "AxfSurface.hh"
#include "StationDB.hh"
#include "StationReport.hh"
using namespace std;

StationDB::StationDB( char **dest, int nDest, int expSecs )
{
   destinations  = dest;
   nDestinations = nDest;
   expireSecs    = expSecs;
   chunkHdrs     = NULL;
   chunkBuf      = MEMbufCreate();
   nChunks       = 0;
   chunkIdex     = 0;
}

StationDB::~StationDB() 
{
   delete[] chunkHdrs;
   MEMbufDelete( chunkBuf );
}

void 
StationDB::reset(int size)
{
   delete[] chunkHdrs;

   chunkIdex = 0;
   nChunks   = size;
   chunkHdrs = new spdb_chunk_ref_t[size];
   MEMbufReset( chunkBuf );   
}

int 
StationDB::add( const StationReport &stationReport )
{
   station_report_t   report;

   if( chunkIdex > nChunks ) {
      POSTMSG( ERROR, "Cannot add station.\n"
                      "Specified number of stations has been exceeded.");
      return( -1 );
   } 

   if ( INFO_ENABLED ) {
      stationReport.print();
   }

   //
   // Get a copy of the station report
   //
   stationReport.getReport( report );

   //
   // Fill in chunk header
   //
   chunkHdrs[chunkIdex].valid_time  = report.time;
   chunkHdrs[chunkIdex].expire_time = chunkHdrs[chunkIdex].valid_time + 
                                      expireSecs;
   chunkHdrs[chunkIdex].data_type   = stationReport.getId();
   chunkHdrs[chunkIdex].offset      = MEMbufLen( chunkBuf );
   chunkHdrs[chunkIdex].len         = sizeof( station_report_t );
 
   //
   // Convert the station data to BIG ENDIAN 
   //
   station_report_to_be( &report );
 
   //
   // Add data to buffer
   //
   MEMbufAdd( chunkBuf, &report, sizeof(station_report_t) );

   //
   // Increment chunk index
   //
   chunkIdex++;

   return( 0 );
}

int 
StationDB::write()
{
   //
   // Returns the number of destinations successfully written to
   //
   int i, numWrites;
   
   if( nChunks > chunkIdex ) {
      POSTMSG( WARNING, "Expected %d surface observations to be added "
                        "to the database -- received only (%d) observations",
                        nChunks, chunkIdex );
      nChunks = chunkIdex;
   }
   
   for( i=0, numWrites=0; i < nDestinations; i++ ) {

      if ( SPDB_put_over( destinations[i], 
		          SPDB_STATION_REPORT_ID,
		          SPDB_STATION_REPORT_LABEL,
		          nChunks,
		          chunkHdrs,
		          (void *) MEMbufPtr(chunkBuf),
		          MEMbufLen(chunkBuf)) == 0 ) {
         numWrites++;
      }
      else {
         POSTMSG( WARNING, "Couldn't write surface observations to %s",
                           destinations[i] );
      }
   }

   SPDB_reap_children();
   return( numWrites );
}
