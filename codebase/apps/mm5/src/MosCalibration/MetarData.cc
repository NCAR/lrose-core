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
// $Id: MetarData.cc,v 1.24 2016/03/07 01:33:50 dixon Exp $
//
//////////////////////////////////////////////////////////////

#include "MetarData.hh"
#include "MetarPoint.hh"
#include "DataMgr.hh"
#include "MosCalibration.hh"

#include <cmath>
#include <cassert>
#include <dataport/port_types.h>
#include <Spdb/Spdb_typedefs.hh>
#include <rapformats/WxObs.hh>
#include <physics/physics.h>
#include <rapmath/umath.h>

using namespace std;

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
MetarData::createList( char* stationId ) 
{
   assert( stationId );
   
   //
   // Tell the server to get the data.
   //   Here we do *not* want to respect the data type.
   //   That is, when we set the data type (or data type 2)
   //   to zero, we are signifying that we want chunks with
   //   any data type (or data type 2) at this time.  In this
   //   case we are only dealing with data type 2, since
   //   the data type is based on the stationId, and should
   //   never be zero.
   //
   if( dataServer.readData( stationId ) != SUCCESS ) {
      POSTMSG( INFO, "Could not read metar data for this time period" );
      return( 0 );
   }

   //
   // Get the chunks from the server
   //
   int nChunks = dataServer.getNChunks();
   if( nChunks < 1 ) {
      POSTMSG( INFO, "No metar data for this time period" );
      return( 0 );
   }
      
   vector<Spdb::chunk_t> chunks = dataServer.getChunks();

   //
   // Process each chunk
   //
   for( int i = 0; i < nChunks; i++ ) {

     WxObs report;
     report.disassemble(chunks[i].data, chunks[i].len);

     //
     // Find the time
     //
     time_t pointTime = report.getObservationTime();

     //
     // Set up a new metar point
     //
     MetarPoint* newPoint = new MetarPoint( pointTime,
                                            report.getLatitude(), 
                                            report.getLongitude() );
      
     //
     // Calculate the u and v components of the wind from the
     // speed and direction
     //
     // There is some question about whether to use dir_speed_2uv
     // or wind_dir_speed2uv.  However, wind_dir_speed2uv results
     // in u and v being the negative of what dir_speed2uv would
     // output.  The affect of this on the regression will be that
     // the slope and correlation coefficient will be the negative
     // of what they would otherwise be.  However, it will not
     // change the strength of the relationship, nor will it 
     // change the ability to predict the surface data given the
     // model data.
     //
     double speed = report.getWindSpeedMps();
     double dirn = report.getWindDirnDegt();

     if ( speed < 0.0 || speed > 100.0 )
       speed = WxObs::missing;

     if ( dirn < 0.0 || dirn > 360.0 )
       dirn = WxObs::missing;

     if ( speed != WxObs::missing  &&
          dirn != WxObs::missing ) { 
       
       float tmpU, tmpV;
       
       dir_speed_2_uv( speed, dirn, &tmpU, &tmpV );
       newPoint->setU( (double) tmpU );
       newPoint->setV( (double) tmpV );

     }

     if( speed != WxObs::missing ) {
       newPoint->setWspd( speed );
     }
     
     if ( report.getTempC() != WxObs::missing ) { 
       newPoint->setTemp( report.getTempC() );
     }
     
     if ( report.getSeaLevelPressureMb() != WxObs::missing ) { 
       newPoint->setPrs( report.getSeaLevelPressureMb() );
     }
     
     if ( report.getRhPercent() != WxObs::missing ) {
       newPoint->setRH( report.getRhPercent() );
     }
     
     if ( report.getCeilingKm() != WxObs::missing ) { 
       newPoint->setCeiling( report.getCeilingKm() );
     }
     
     if ( report.getVisibilityKm() != WxObs::missing ) { 
       newPoint->setVis( report.getVisibilityKm() );
     }
     
     //
     // Add the point to the map
     //
     dataPoints[ pointTime ] = newPoint;

   }

   return( (int) dataPoints.size() );
}

double
MetarData::getLat() 
{
   if( dataPoints.size() > 0 ) 
      return( (dataPoints.begin())->second->getLat() );
   else
      return( MetarPoint::MISSING_VAL );
}

double
MetarData::getLon() 
{
   if( dataPoints.size() > 0 ) 
      return( (dataPoints.begin())->second->getLon() );
   else
      return( MetarPoint::MISSING_VAL );
}



