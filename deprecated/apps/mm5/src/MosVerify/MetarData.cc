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
////////////////////////////////////////////////////////////
//
// Metar data for MOS calibration
//
// $Id: MetarData.cc,v 1.10 2016/03/07 01:33:51 dixon Exp $
//
//////////////////////////////////////////////////////////////

#include "MetarData.hh"
#include "MetarPoint.hh"
#include "DataMgr.hh"
#include "MosVerify.hh"
using namespace std;

#include <cassert>
#include <cmath>
#include <dataport/port_types.h>
#include <Spdb/Spdb_typedefs.hh>
#include <rapformats/station_reports.h>
#include <physics/physics.h>
#include <rapmath/umath.h>

MetarData::MetarData( DataServer& server ) 
    : dataServer( server )
{
}

MetarData::~MetarData()
{
   clear();
}

void 
MetarData::clear() 
{
   map< time_t, MetarPoint*, less< time_t > >::iterator it;
   
   for( it = dataPoints.begin(); it != dataPoints.end(); it++ ) {
      delete ((*it).second);
   }
   dataPoints.erase( dataPoints.begin(), dataPoints.end() );

}

int
MetarData::createList( char* stationId, int leadTime ) 
{
   assert( stationId );
   
   //
   // Tell the server to get the data
   //
   if( dataServer.readData( stationId, leadTime ) != SUCCESS ) {
      return( 0 );
   }

   //
   // Get the chunks from the server
   //
   int nChunks = dataServer.getNChunks();
   if( nChunks < 1 ) {
      return( 0 );
   }
      
   vector<Spdb::chunk_t> chunks = dataServer.getChunks();

   //
   // Process each chunk
   //
   for( int i = 0; i < nChunks; i++ ) {

      station_report_t *report = (station_report_t *) (chunks[i].data);
      
      station_report_from_be( report );

      //
      // Find the time
      //
      time_t pointTime = (time_t) report->time;

      //
      // Set up a new metar point
      //
      MetarPoint* newPoint = new MetarPoint( pointTime, report->lat, 
                                             report->lon );
      

      if( report->windspd != STATION_NAN ) {
         newPoint->setWspd( report->windspd );
      }

      if( report->winddir != STATION_NAN ) {
         newPoint->setWdir( report->winddir );
      }
      
      if ( report->temp != STATION_NAN ) { 
         newPoint->setTemp( report->temp );
      }

      if ( report->pres != STATION_NAN ) { 
         newPoint->setPrs( report->pres );
      }
   
      if ( report->relhum != STATION_NAN ) {
         newPoint->setRH( report->relhum );
      }

      if ( report->ceiling != STATION_NAN ) { 
         newPoint->setCeiling( report->ceiling );
      }

      if ( report->visibility != STATION_NAN ) { 
         newPoint->setVis( report->visibility );
      }

      //
      // Add the point to the map
      //
      dataPoints[ pointTime ] = newPoint;

   }

   return( (int) dataPoints.size() );
}



