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
// MM5 data 
//
// $Id: MM5Data.cc,v 1.5 2016/03/07 01:33:51 dixon Exp $
//
//////////////////////////////////////////////////////////////
#include <values.h>
#include <cmath>
#include <cassert>
#include <dataport/port_types.h>
#include <Spdb/Spdb_typedefs.hh>
#include <rapformats/ComboPt.hh>
#include <rapformats/GenPt.hh>
#include <physics/physics.h>
#include <toolsa/DateTime.hh>

#include "MM5Data.hh"
#include "MM5Point.hh"
#include "DataMgr.hh"
#include "MosVerify.hh"
using namespace std;

//
// Constants
//
const string MM5Data::RH       = "RH";
const string MM5Data::TEMP     = "Temp";
const string MM5Data::PRS      = "PRESSURE";
const string MM5Data::U        = "U";
const string MM5Data::V        = "V";
const string MM5Data::W        = "W";

MM5Data::MM5Data( DataServer& server ) 
    : dataServer( server )
{
}

MM5Data::~MM5Data()
{
   clear();
}

void
MM5Data::clear() 
{
   map< time_t, MM5Point*, less< time_t > >::iterator it;
   
   for( it = dataPoints.begin(); it != dataPoints.end(); it++ ) {
      delete ((*it).second);
   }
   dataPoints.erase( dataPoints.begin(), dataPoints.end() );
}

int
MM5Data::createList( char* stationId, int leadTime ) 
{
   assert( stationId );
   
   //
   // Tell the server to get the data
   //
   if( dataServer.readData( stationId, leadTime ) != SUCCESS ) {
      POSTMSG( INFO, "Couldn't read model data for this time period" );
      return( 0 );
   }
   
   //
   // Get the chunks from the server
   //
   int nChunks = dataServer.getNChunks();
   if( nChunks < 1 ) {
      POSTMSG( INFO, "No model data for this time period" );
      return( 0 );
   }
      
   vector<Spdb::chunk_t> chunks = dataServer.getChunks();

   //
   // Process each chunk
   //
   ComboPt comboPt;
   
   for( int i = 0; i < nChunks; i++ ) {

      comboPt.disassemble( chunks[i].data, chunks[i].len );

      const GenPt& singleLevelPt = comboPt.get1DPoint();
      const GenPt& multiLevelPt  = comboPt.get2DPoint();

      //
      // Get the point time
      //
      time_t pointTime = multiLevelPt.getTime();

      //
      // Set up a new MM5 data point
      //
      MM5Point* newPoint = new MM5Point( pointTime, 
                                         singleLevelPt.getLat(),
                                         singleLevelPt.getLon() );

      //
      //  Get the data
      //
      double rh    = get2DVal( RH, multiLevelPt, pointTime, 
                               leadTime, stationId );
      double temp  = get2DVal( TEMP, multiLevelPt, pointTime, 
                               leadTime, stationId );
      double prs   = get2DVal( PRS, multiLevelPt, pointTime, 
                               leadTime, stationId );
      double uComp = get2DVal( U, multiLevelPt, pointTime, 
                               leadTime, stationId );
      double vComp = get2DVal( V, multiLevelPt, pointTime, 
                               leadTime, stationId );

      if( rh != GenPt::missingVal )
         newPoint->setRH( rh );
      
      if( temp != GenPt::missingVal )
         newPoint->setTemp( temp );

      if( prs != GenPt::missingVal )
         newPoint->setPrs( prs );

      if( uComp != GenPt::missingVal )
         newPoint->setU( -1 * uComp );

      if( vComp != GenPt::missingVal )
         newPoint->setV( -1 * vComp );

      if( uComp != GenPt::missingVal && vComp != GenPt::missingVal ) {
         double wspd = sqrt( uComp*uComp + vComp*vComp );
         newPoint->setWspd( wspd );

         double wdir = PHYwind_dir( uComp, vComp );
         newPoint->setWdir( wdir );
      }

      //
      // Put the data point into the map
      //
      dataPoints[ pointTime ] = newPoint;
      
   }
   
   return( (int) dataPoints.size() );
   
}

double 
MM5Data::get2DVal( const string& fieldName, const GenPt& multiLevelPt,
                   time_t dataTime, int fcastTime, char* id ) 
{
   DateTime when( dataTime );

   int fieldNum = multiLevelPt.getFieldNum( fieldName );
   if( fieldNum < 0 ) {
      POSTMSG( INFO, "Cannot find field %s at %s for lead time = %d "
                      "and station id = %s", 
               fieldName.c_str(), when.dtime(), fcastTime, id );
      return( MM5Point::MISSING_VAL );
   }
   return( multiLevelPt.get2DVal( 0, fieldNum ));
   
}
