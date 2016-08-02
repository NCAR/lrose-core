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
// SoundingGet:  Reads sounding data from spdb
//
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// April 1999
//
// $Id: SoundingGet.cc,v 1.22 2016/03/03 18:13:05 dixon Exp $
//
///////////////////////////////////////////////
#include <string>
#include <cassert>
#include <toolsa/toolsa_macros.h>
#include <toolsa/mem.h>
#include <dataport/bigend.h>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <Spdb/SoundingGet.hh>
#include <rapformats/sounding_chunk.h>
using namespace std;

SoundingGet::~SoundingGet()
{
   clearProducts();
}

void
SoundingGet::init( const char* urlStr, time_t margin,
                   double minWindsAlt, double maxWindsAlt,
                   double avgUwind, double avgVwind )
{
   //
   // Base class initialization
   //
   Sounding::init();

   //
   // Allow only a single spdb source
   //
   url = urlStr;

   //
   // Specialized member initialization
   //
   timeMargin        = margin;
   avgU              = avgUwind;
   avgV              = avgVwind;
   missingValue      = SNDG_VALUE_UNKNOWN;
   useSounding       = true;

   userMinAlt        = minWindsAlt;
   userMaxAlt        = maxWindsAlt;
   if( userMinAlt != -1 && userMaxAlt != -1 )
        userAltLimits = true;
   else
        userAltLimits = false;

   currentMinAlt     = DBL_MAX;
   currentMaxAlt     = DBL_MIN;

   nInputChunks      = 0;
   inputChunkHdrs    = NULL;
   inputChunkData    = NULL;
   activeProduct     = -1;
}

int
SoundingGet::readSounding( time_t when, int whichSiteId, int data_type2 )
{
   int                                  i, status;
   bool                                 gotData;
   SNDG_spdb_product_t                 *productPtr;

   //
   // Clear out the old products
   //
   clearProducts();

   //
   // Fetch all soundings for the specified time
   //
   status = fetchData( when, whichSiteId, data_type2, &gotData );

   //
   // If we didn't get any chunks, there's nothing else to do here
   //
   if ( !gotData ) {
      return( status );
   }

   //
   // Load up the byte-swapped product list with the retrieved chunks
   //
   for( i=0; i < nInputChunks; i++ ) {
      productPtr = (SNDG_spdb_product_t *)((char*)inputChunkData +
                                           inputChunkHdrs[i].offset);
      SNDG_spdb_product_from_BE( productPtr );
      products.push_back( productPtr );
   }

   //
   // Cache the first product after a read
   //
   loadProduct( 0 );

   return( products.size() );
}

int
SoundingGet::fetchData( time_t when, int whichSiteId, int data_type2, bool* gotData )
{
   //
   // Initialize some members in case we don't connect to the database
   //
   *gotData   = false;
   launchTime = DateTime::NEVER;
   leadSecs   = 0;
   siteId     = 0;

   //
   // If we are using the default values, we're done
   //
   if( !useSounding ) {
      sourceId = DEFAULT_ID;
      return( 0 );
   }
   
   //
   // Fetch sounding from spdb 
   //
   if ( spdbMgr.getFirstBefore( url, when, timeMargin, whichSiteId, data_type2 ) != 0 ) {
      //
      // The SPDB get failed -- don't use the sounding
      //
      valid = false;
      sourceId = INVALID_ID;
      return( -1 );
   }
   else {
      //
      // Get pointers to the headers and data 
      // Set the state of the class later
      //
      nInputChunks   = spdbMgr.getNChunks();
      inputChunkHdrs = spdbMgr.getChunkRefs();
      inputChunkData = spdbMgr.getChunkData();
   }

   //
   // Check number of chunks
   //
   if( nInputChunks == 0 ) {
      valid = false;
      sourceId = DEFAULT_ID;
      return( 0 );
   }

   valid    = true;
   *gotData = true;

   return( nInputChunks );
}

void
SoundingGet::loadProduct( int productIndex )
{
   //
   // Degenerate case -- the state of the class is up to date
   //
   assert( productIndex >= 0 );
   if ( productIndex == activeProduct ) {
      return;
   }

   //
   // Set up the new active product
   //
   activeProduct = productIndex;
   SNDG_spdb_product_t *productPtr = products[productIndex];

   //
   // Set site info
   //
   siteId   = inputChunkHdrs[productIndex].data_type;
   siteLat  = productPtr->lat;
   siteLon  = productPtr->lon;
   siteAlt  = productPtr->alt;
   siteName = productPtr->siteName;

   //
   // Set time info
   //
   launchTime = productPtr->launchTime;
   leadSecs   = productPtr->leadSecs;

   //
   // Set source data info
   //
   sourceId   = (source_t)productPtr->sourceId;
   sourceFmt  = productPtr->sourceFmt ;

   // 
   // Clear out data arrays
   //
   resetData( productPtr->nPoints );
   
   //
   // Fill data arrays
   //
   SNDG_spdb_point_t *pointPtr;

   pointPtr = (SNDG_spdb_point_t *)( (char *)inputChunkData +
                                     sizeof(SNDG_spdb_product_t) - 
                                     sizeof(SNDG_spdb_point_t) );

   for( int i = 0; i < numObs; i++ ) {
      if( pointPtr->pressure != SNDG_VALUE_UNKNOWN )
	 pressure[i] = (double) pointPtr->pressure;
      
      if( pointPtr->altitude != SNDG_VALUE_UNKNOWN )
	 altitude[i] = (double) pointPtr->altitude;
      
      if( pointPtr->u != SNDG_VALUE_UNKNOWN )
	 uwind[i] = (double) pointPtr->u;
      
      if( pointPtr->v != SNDG_VALUE_UNKNOWN )
	 vwind[i] = (double) pointPtr->v;
      
      if( pointPtr->w != SNDG_VALUE_UNKNOWN )
	 wwind[i] = (double) pointPtr->w;
      
      if( pointPtr->rh != SNDG_VALUE_UNKNOWN )
	 rh[i] = (double) pointPtr->rh;
      
      if( pointPtr->temp != SNDG_VALUE_UNKNOWN )
	 temp[i] = (double) pointPtr->temp;

      if( pointPtr->u != SNDG_VALUE_UNKNOWN && 
          pointPtr->v != SNDG_VALUE_UNKNOWN )
         Sounding::getDirSpeed( pointPtr->u, pointPtr->v, 
                                &windDir[i], &windSpeed[i] );
      
      pointPtr++;
   }

   //
   // Set related members
   //
   setAltLimits();
}
   
void
SoundingGet::clearProducts()
{
   activeProduct = -1;
   products.erase( products.begin(), products.end() );
}

void
SoundingGet::setAltLimits()
{
   if( useSounding && valid ) {

      double alt;
      int    nGoodPts   = 0;
	 
      for( int i = 0; i < numObs; i++ ) {
         alt = altitude[i];
         if( alt != missingValue ) {
            if( alt < currentMinAlt )
               currentMinAlt = alt;
	    if( alt > currentMaxAlt )
               currentMaxAlt = alt;
            nGoodPts++;
         }
      }

      if( nGoodPts == 0 )
	 valid = false;

   }
}

double *
SoundingGet::getPres()  const
{
   if( useSounding && valid ) {
      for( int i = 0; i < numObs; i++ ) {
	 if( pressure[i] != missingValue )
	    return( pressure );
      }
   }
   
   return( NULL );
}

double *
SoundingGet::getAlts()  const
{
   if( useSounding && valid ) {
      for( int i = 0; i < numObs; i++ ) {
	 if( altitude[i] != missingValue )
	    return( altitude );
      }
   }
   
   return( NULL );
}

double *
SoundingGet::getU()  const
{
   if( useSounding && valid ) {
      for( int i = 0; i < numObs; i++ ) {
	 if( uwind[i] != missingValue )
	    return( uwind );
      }
   }
   
   return( NULL );
}

double *
SoundingGet::getV() const
{
   if( useSounding && valid ) {
      for( int i = 0; i < numObs; i++ ) {
         if( vwind[i] != missingValue )
	    return( vwind );
      }
   }
   
   return( NULL );
}

double *
SoundingGet::getW() const
{
   if( useSounding && valid ) {
      
      for( int i = 0; i < numObs; i++ ) {
	 if( wwind[i] != missingValue )
	    return( wwind );
      }
   }
   
   return( NULL );
}

double *
SoundingGet::getRH() const
{
   if( useSounding && valid ) {
      for( int i = 0; i < numObs; i++ ) {
	 if( rh[i] != missingValue )
	    return( rh );
      }
   }
   
   return( NULL );
}

double *
SoundingGet::getTemp() const
{
   if( useSounding && valid ) {
      for( int i = 0; i < numObs; i++ ) {
         if( temp[i] != missingValue )
	    return( temp );
      }
   }
   
   return( NULL );
}

double *
SoundingGet::getWindSpeed() const
{
   if( useSounding && valid ) {
      for( int i = 0; i < numObs; i++ ) {
	 if( windSpeed[i] != missingValue )
	    return( windSpeed );
      }
   }
   
   return( NULL );
}

double *
SoundingGet::getWindDir() const
{
   if( useSounding && valid ) {
      for( int i = 0; i < numObs; i++ ) {
	 if( windDir[i] != missingValue )
	    return( windDir );
      }
   }
   
   return( NULL );
}

double *
SoundingGet::getDivergence() const
{
   if( useSounding && valid ) {
      for( int i = 0; i < numObs; i++ ) {
	 if( div[i] != missingValue )
	    return( div );
      }
   }
   
   return( NULL );
}

int
SoundingGet::getIndex( double alt ) const
{
   int i = 0, index;
   int lastGoodIdex = -1;

   if( !useSounding || !valid )
      return( -1 );

   //
   // Check for the zero'th altitude
   //
   if ( altitude[0] != missingValue && altitude[0] >= alt ) {
      index = 0;
   }
   else {
      //
      // Find the closest altitude
      //
      while( i < numObs && (altitude[i] == missingValue || 
                            altitude[i] < alt) ) {

         if( altitude[i] != missingValue )
	    lastGoodIdex = i;
         i++;

      }
      
      if (( i >= numObs ) || lastGoodIdex < 0 ||
	  (fabs(altitude[i] - alt) > fabs(altitude[lastGoodIdex] - alt)) )
	 index = lastGoodIdex;
      else 
         index = i;
   }

   return index;
}

void
SoundingGet::getUV( double *u, double *v, double speedRange ) const
{
   int altsUsed = 0;
   int minAltIdex, maxAltIdex;

   double uVal, vVal;
   double uSum = 0, vSum = 0;
    
   assert( u && v );
   if( useSounding && valid ) {

      if( userAltLimits ) {

	 //
	 // If the user limits are out of range, use the defaults
	 //
         if( userMinAlt > currentMaxAlt || userMaxAlt < currentMinAlt ) {
	    *u = avgU;
	    *v = avgV;
	    return;
	 }
	 
	 minAltIdex = getIndex( MAX( userMinAlt, currentMinAlt ) );
	 maxAltIdex = getIndex( MIN( userMaxAlt, currentMaxAlt ) );
      }
      else {
	 minAltIdex = 0;
	 maxAltIdex = numObs - 1;
      }

      //
      // Find the averages
      //
      for( int i = minAltIdex; i <= maxAltIdex; i++ ) {
	 //                                           
         // Make sure u,v in range.
         // If it's not, skip to the next level.
         //
         uVal = uwind[i];
         vVal = vwind[i];
         if( uVal != missingValue && vVal != missingValue ) {
	    if ((speedRange == DBL_MAX) || 
                (uVal >= -1.0*speedRange && uVal <= speedRange && 
                 vVal >= -1.0*speedRange && vVal <= speedRange )) {
	       uSum += uVal;
	       vSum += vVal;
	       altsUsed++;
	    }
	 }
      }
   }
   

   if ( altsUsed > 0 ) {
      *u = uSum/altsUsed;
      *v = vSum/altsUsed;
   } else {
      *u = avgU;
      *v = avgV;
   }
}

void
SoundingGet::getUV( double alt, double *uVal, double *vVal ) const
{
   assert( uVal && vVal );
   if( useSounding && valid ) {

      int index = getIndex( alt );

      if( uwind[index] != missingValue && 
          vwind[index] != missingValue ) {
	 *uVal = uwind[index];
	 *vVal = vwind[index];
         return;
      }
   }

   *uVal = avgU;
   *vVal = avgV;
}

void
SoundingGet::getDirSpeed( double alt, double *dir, double *speed ) const
{
   assert( dir && speed );
   if( useSounding && valid ) {
   
      int index = getIndex( alt );
   
      if( windDir[index] != missingValue &&
          windSpeed[index] != missingValue ) {
	 *dir      = windDir[index];
	 *speed    = windSpeed[index];
         return;
      }
   }

   Sounding::getDirSpeed( (float) avgU, (float) avgV, dir, speed );
}

void*
SoundingGet::getChunk()  const
{
   //
   // Set up the buffer
   //
   void *soundingChunkBuffer =  umalloc( sizeof( sounding_chunk_t ) );
   memset( soundingChunkBuffer, 0, sizeof( sounding_chunk_t ) );

   // 
   // Set a pointer into the buffer for the sounding chunk type
   //
   sounding_chunk_t *soundingChunkPtr = 
      (sounding_chunk_t *) soundingChunkBuffer;

   //
   // Set the data members
   //
   double u, v;
   getUV( &u, &v );
   
   soundingChunkPtr->dataTime = (si32) launchTime;
   soundingChunkPtr->avgU     = u;
   soundingChunkPtr->avgV     = v;
   
   //
   // Set the name
   //
   string source_name = url + "&";
   
   switch( sourceId ) {
       case INVALID_ID:
	 source_name += INVALID_NAME;
	 break;
	  
       case SONDE_ID:
	 source_name += SONDE_NAME;
	 break;
	  
       case PROFILER_ID:
	 source_name += PROFILER_NAME;
	 break;
 
       case RUC_ID:
	 source_name += RUC_NAME;
	 break;
	  
       case VAD_ID:
	 source_name += VAD_NAME;
	 break;
	  
       default:
	 source_name += DEFAULT_NAME;
	 break;
   }

   STRcopy(soundingChunkPtr->name, source_name.c_str(), SNDG_CHUNK_NAME_LEN);
   
   //
   // Byte swapping
   //
   sounding_chunk_to_BE( soundingChunkPtr );
   
    return( soundingChunkBuffer );
}

