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
////////////////////////////////////////////////
//
// SoundingPut:  Writes sounding data to spdb
//
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// April 1999
//
///////////////////////////////////////////////
#include <cassert>
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <Spdb/Product_defines.hh>
#include <Spdb/SoundingPut.hh>
using namespace std;


void
SoundingPut::init(const string &url, 
                  source_t sourceIdx, const char *sourceFmtx)
{

  // base class

  Sounding::init();
  
  // derived class
  
  spdbMgr.addUrl(url);
  sourceId = sourceIdx;
  sourceFmt = sourceFmtx;

}

void
SoundingPut::init( vector< string* >& urlStrs, 
                   source_t sourceIdx, const char *sourceFmtx,
                   int siteIdx, const char *siteNamex,
                   double lat, double lon, double altOfSite,
                   double missingVal )
{
   //
   // Base class initialization
   //
   Sounding::init();

   //
   // Allow multiple output spdb destinations
   //

   assert( urlStrs.size() > 0 );
   vector< string* >::iterator urlItem = urlStrs.begin();

   for( urlItem = urlStrs.begin(); urlItem != urlStrs.end(); urlItem++ ) {
      spdbMgr.addUrl( **urlItem );
   }
   
   //
   // Specialized member initialization
   //
   sourceId          = sourceIdx;
   sourceFmt         = sourceFmtx;
   siteId            = siteIdx;
   if( siteNamex != NULL )
      siteName       = siteNamex;

   siteLat           = lat;
   siteLon           = lon;
   siteAlt           = altOfSite;

   missingValue      = missingVal;
}

void
SoundingPut::init( const string& urlStr, 
                   source_t sourceIdx, const char *sourceFmtx,
                   int siteIdx, const char *siteNamex,
                   double lat, double lon, double altOfSite,
                   double missingVal )
{

   Sounding::init();
   spdbMgr.addUrl( urlStr );

   sourceId          = sourceIdx;
   sourceFmt         = sourceFmtx;
   siteId            = siteIdx;

   if( siteNamex != NULL )
      siteName       = siteNamex;

   siteLat           = lat;
   siteLon           = lon;
   siteAlt           = altOfSite;

   missingValue      = missingVal;

}

int
SoundingPut::set( time_t launch, int numPts, double *height, 
                  double *u, double *v, double *w, 
                  double *prs, double *relHum, double *temperature,
                  double *divergence)
{
   //
   // Must have at least height, u and v for the sounding
   //
   if( height == NULL || u == NULL || v == NULL ) {
      valid = false;
      return( -1 );
   }

   //
   // Clear out any old data
   //
   if( resetData( numPts ) ) {
      return( -1 );
   }
   
   //
   // Copy in the minimal required sounding data
   //
   memcpy((void *) altitude, (void *) height, numPts*sizeof(double));
   memcpy((void *) uwind, (void *) u, numPts*sizeof(double));
   memcpy((void *) vwind, (void *) v, numPts*sizeof(double));

   //
   // Copy in the optional sounding data
   //
   if( w != NULL ) {
      memcpy((void *) wwind, (void *) w, numPts*sizeof(double));
   }
   if( prs != NULL ) {
      memcpy((void *) pressure, (void *) prs, numPts*sizeof(double));
   }
   if( relHum != NULL ) {
      memcpy((void *) rh, (void *) relHum, numPts*sizeof(double));
   }
   if( temperature != NULL ) {
      memcpy((void *) temp, (void *) temperature, numPts*sizeof(double));
   }
   if( divergence != NULL ) {
     memcpy((void *) div, (void *) divergence, numPts*sizeof(double));
   }

   //
   // Set related members
   //
   launchTime = launch;
   valid = true;

   return( 0 );
}

int 
SoundingPut::set( time_t launch, 
                  vector<double> *height,
                  vector<double> *u, vector<double> *v, vector<double> *w,
                  vector<double> *prs, 
                  vector<double> *relHum, 
                  vector<double> *temperature /* = NULL */,
                  vector<double> *divergence  /* = NULL */ )
{
   size_t  i, numPts;

   //
   // Must have at least height, u and v for the sounding
   //
   if( height == NULL || u == NULL || v == NULL ) {
      valid = false;
      return( -1 );
   }

   //
   // Clear out any old data
   //
   numPts = height->size();
   if( resetData( numPts ) ) {
      return( -1 );
   }

   //
   // Copy in the minimal required sounding data
   //
   assert( u->size() == numPts  &&  v->size() == numPts );
   for( i=0; i < numPts; i++ ) {
      altitude[i] = (*height)[i];
      uwind[i] = (*u)[i];
      vwind[i] = (*v)[i];
   }

   //
   // Copy in the optional sounding data
   //
   if( w != NULL ) {
      assert( w->size() == numPts );
      for( i=0; i < numPts; i++ ) {
         wwind[i] = (*w)[i];
      }
   }
   if( prs != NULL ) {
      assert( prs->size() == numPts );
      for( i=0; i < numPts; i++ ) {
         pressure[i] = (*prs)[i];
      }
   }
   if( relHum != NULL ) {
      assert( relHum->size() == numPts );
      for( i=0; i < numPts; i++ ) {
         rh[i] = (*relHum)[i];
      }
   }
   if( temperature != NULL ) {
      assert( temperature->size() == numPts );
      for( i=0; i < numPts; i++ ) {
         temp[i] = (*temperature)[i];
      }
   }
   if( divergence != NULL ) {
     assert( divergence->size() == numPts );
      for( i=0; i < numPts; i++ ) {
        div[i] = (*divergence)[i];
      }
   }

   //
   // Set related members
   //
   launchTime = launch;
   valid = true;

   return( 0 );
}

void
SoundingPut::setSiteName( const char *name )
{
   if ( name )
      siteName = name;
   else
      siteName = EMPTY_STRING;
}

int
SoundingPut::writeSounding( time_t validTime, time_t expireTime, int lsecs, Spdb::put_mode_t putMode, int data_type2 )
{
   int      status;
   char    *chunkBuf;
   int     chunkLen;

   //
   // If the sounding is not valid, don't write anything
   //
   if( !valid ) {
      return( -1 );
   }

   //
   // Determine length for chunk buffer
   //
   chunkLen = sizeof( SNDG_spdb_product_t ) + numObs*sizeof(SNDG_spdb_point_t);

   //
   // Create the chunk buffer
   //
   chunkBuf = (char *) ucalloc( chunkLen, sizeof(char) );

   //
   // Set the sounding product data into the chunk buffer
   //
   leadSecs = lsecs;
   setProduct( (SNDG_spdb_product_t *) chunkBuf );

   //
   // Write the new chunk to the database
   //
   spdbMgr.setPutMode( putMode );
   status = spdbMgr.put( SPDB_SNDG_ID,
                         SPDB_SNDG_LABEL,
                         siteId,
                         validTime,
                         expireTime,
                         chunkLen,
                         (const void *) chunkBuf,
                         data_type2 );

   ufree( (void *)chunkBuf );
   return( status );
}

void
SoundingPut::setProduct( SNDG_spdb_product_t *soundingPtr )
{
   //
   // Set up header struct
   //
   soundingPtr->launchTime = (si32) launchTime;
   soundingPtr->nPoints    = (si32) numObs;
   soundingPtr->sourceId   = (si32) sourceId;
   soundingPtr->leadSecs   = (si32) leadSecs;
 
   memset( (void *) soundingPtr->spareInts, (int) 0,
	   SNDG_SPARE_INTS * sizeof(si32) );
   
   soundingPtr->lat = (fl32) siteLat;
   soundingPtr->lon = (fl32) siteLon;
   soundingPtr->alt = (fl32) siteAlt;
   soundingPtr->missingVal = (fl32) missingValue;

   memset( (void *) soundingPtr->spareFloats, (int) 0,
	   SNDG_SPARE_FLOATS * sizeof(fl32) );

   STRncopy( soundingPtr->sourceName, getSourceName(),    SOURCE_NAME_LENGTH );
   STRncopy( soundingPtr->sourceFmt,  sourceFmt.c_str(),  SOURCE_FMT_LENGTH );
   STRncopy( soundingPtr->siteName,   siteName.c_str(),   SITE_NAME_LENGTH );

   //
   // Set offset into buffer to where data points begin
   // and copy data into buffer
   //
   int                dataOffset;
   SNDG_spdb_point_t *dataPtr;

   dataOffset = sizeof(SNDG_spdb_product_t) - sizeof(SNDG_spdb_point_t);
   dataPtr    = (SNDG_spdb_point_t *) ((char*)soundingPtr + dataOffset );

   for( int i = 0; i < numObs; i++ ) {
      if( pressure[i] == missingValue )
	 dataPtr->pressure = SNDG_VALUE_UNKNOWN;
      else
	 dataPtr->pressure = (fl32) pressure[i];
      
      if( altitude[i] == missingValue )
	 dataPtr->altitude = SNDG_VALUE_UNKNOWN;
      else
	 dataPtr->altitude = (fl32) altitude[i];
      
      if( uwind[i] == missingValue )
	 dataPtr->u = SNDG_VALUE_UNKNOWN;
      else
	 dataPtr->u = (fl32) uwind[i];
      
      if( vwind[i] == missingValue )
	 dataPtr->v = SNDG_VALUE_UNKNOWN;
      else
	 dataPtr->v = (fl32) vwind[i];
      
      if( wwind[i] == missingValue )
	 dataPtr->w = SNDG_VALUE_UNKNOWN;
      else
	 dataPtr->w = (fl32) wwind[i];
      
      if( rh[i] == missingValue )
	 dataPtr->rh = SNDG_VALUE_UNKNOWN;
      else 
	 dataPtr->rh = (fl32) rh[i];
      
      if( temp[i] == missingValue )
	 dataPtr->temp = SNDG_VALUE_UNKNOWN;
      else
	 dataPtr->temp = (fl32) temp[i];
      
      if( div[i] == missingValue )
         dataPtr->div = SNDG_VALUE_UNKNOWN;
      else
	 dataPtr->div = (fl32) div[i];
      
      memset( (void *) dataPtr->spareFloats, 0, 
	      SNDG_PNT_SPARE_FLOATS * sizeof(fl32) );
      
      dataPtr++;
   }

   SNDG_spdb_product_to_BE( soundingPtr );
}
