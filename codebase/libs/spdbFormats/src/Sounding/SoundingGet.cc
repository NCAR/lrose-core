////////////////////////////////////////////////
//
// SoundingGet:  Reads sounding data from spdb
//
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// April 1999
//
// $Id: SoundingGet.cc,v 1.3 2003/01/25 23:35:35 dixon Exp $
//
///////////////////////////////////////////////
#include <cassert>
#include <rapmath/umath.h>
#include <symprod/spdb_client.h>
#include <toolsa/DateTime.hh>
#include <spdbFormats/SoundingGet.hh>
using namespace std;

SoundingGet::~SoundingGet()
{
   clearProducts();

   if( inputChunkHdrs || inputChunkData )
      SPDB_free_get();
}

void
SoundingGet::init( const char* urlStr, time_t margin,
                   double minWindsAlt, double maxWindsAlt,
                   double avgUwind, double avgVwind,
                   double missingVal, bool useDefaults )
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
   missingValue      = missingVal;
   useSounding       = !useDefaults;

   minAlt            = minWindsAlt;
   maxAlt            = maxWindsAlt;
   if( minAlt != -1 && maxAlt != -1 )
        userAltLimits = true;
   else
        userAltLimits = false;

   nInputChunks      = 0;
   inputChunkHdrs    = NULL;
   inputChunkData    = NULL;
   activeProduct     = -1;
}

int
SoundingGet::readSounding( time_t when, int whichSiteId )
{
   int                  i, status;
   bool                 gotData;
   SNDG_spdb_product_t *productPtr;

   //
   // Clear out the old products
   //
   clearProducts();

   //
   // Fetch all soundings for the specified time
   //
   status = fetchData( when, whichSiteId, &gotData );

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
SoundingGet::fetchData( time_t when, int whichSiteId, bool* gotData )
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
   if ( spdbMgr.getFirstBefore( url, when, timeMargin, whichSiteId ) != 0 ) {
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

   siteId   = whichSiteId;
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

      double currentMin = MAXDOUBLE;
      double currentMax = -1.0;
      double alt;
      int    nGoodPts   = 0;
	 
      for( int i = 0; i < numObs; i++ ) {
         alt = altitude[i];
         if( alt != missingValue ) {
            if( alt < currentMin )
               currentMin = alt;
	    if( alt > currentMax )
               currentMax = alt;
            nGoodPts++;
         }
      }

      if( nGoodPts == 0 )
	 valid = false;

      if( !userAltLimits ) {
         minAlt = currentMin;
      }
      else {
         //
         // Make sure the min. alt specified by the application
         // is contained in our current dataset
         //
         if ( minAlt < currentMin ) {
            minAlt = currentMin;
         }
      }
      
      if( !userAltLimits ) {
         maxAlt = currentMax;
      }
      else {
         //
         // Make sure the max. alt specified by the application
         // is contained in our current dataset
         //
         if ( maxAlt > currentMax ) {
            maxAlt = currentMax;
         }
      }
   }
}

int
SoundingGet::advect( time_t fromWhen, time_t extrapSeconds, 
                     double *kmX, double *kmY,
                     double *u,   double *v )
{
   if ( readSounding( fromWhen ) < 0 ) {
      return( -1 );
   }

   double uVal, vVal;
   getUV( &uVal, &vVal );

   *kmX = uVal * extrapSeconds / 1000;
   *kmY = vVal * extrapSeconds / 1000;

   //
   // Also, return u/v if the caller asks
   //
   if ( u )
      *u = uVal;
   if ( v )
      *v = vVal;

   return( 0 );
}

double *
SoundingGet::getPres() 
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
SoundingGet::getAlts() 
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
SoundingGet::getU() 
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
SoundingGet::getV() 
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
SoundingGet::getW() 
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
SoundingGet::getRH() 
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
SoundingGet::getTemp() 
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
SoundingGet::getWindSpeed()
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
SoundingGet::getWindDir()
{
   if( useSounding && valid ) {
      for( int i = 0; i < numObs; i++ ) {
	 if( windDir[i] != missingValue )
	    return( windDir );
      }
   }
   
   return( NULL );
}

int
SoundingGet::getIndex( double alt )
{
   int i = 0, index;
   int lastGoodIdex = -1;

   if( !useSounding || !valid )
      return( -1 );

   //
   // Check for the zero'th altitude
   //
   if ( altitude[0] != missingValue && altitude[0] > alt ) {
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
SoundingGet::getUV( double *u, double *v, double speedRange )
{
   int altsUsed = 0;

   double uVal, vVal;
   double uSum = 0, vSum = 0;
    
   assert( u && v );
   if( useSounding && valid ) {

      int minAltIdex = getIndex( minAlt );
      int maxAltIdex = getIndex( maxAlt );

      for( int i = minAltIdex; i <= maxAltIdex; i++ ) {
	 //                                           
         // Make sure u,v in range.
         // If it's not, skip to the next level.
         //
         uVal = uwind[i];
         vVal = vwind[i];
         if( uVal != missingValue && vVal != missingValue ) {
	    if ((speedRange == MAXDOUBLE) || 
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
SoundingGet::getUV( double alt, double *uVal, double *vVal )
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
SoundingGet::getDirSpeed( double alt, double *dir, double *speed )
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
