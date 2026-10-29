////////////////////////////////////////////////////////////////////////////////
// Sounding class 
//   base class for SoundingPut and SoundingGet
//
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// April 1999
//
// $Id: Sounding.cc,v 1.18 2003/01/25 23:35:35 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <rapmath/umath.h>
#include <spdbFormats/Sounding.hh>
using namespace std;

//
// Constants
//
const char *Sounding::INVALID_NAME  = "invalid";
const char *Sounding::DEFAULT_NAME  = "default";

const char *Sounding::SONDE_NAME    = "Radio Sonde";
const char *Sounding::PROFILER_NAME = "Profiler";
const char *Sounding::RUC_NAME      = "RUC";
const char *Sounding::VAD_NAME      = "VAD";

const char *Sounding::EMPTY_STRING  = "";


void
Sounding::init()
{
   //
   // Initialize members
   //
   sourceId          = DEFAULT_ID;
   sourceFmt         = EMPTY_STRING;

   valid             = false;
   numObs            = 0;
   numAlloc          = 0;

   pressure          = NULL;
   altitude          = NULL;
   uwind             = NULL;
   vwind             = NULL;
   wwind             = NULL;
   windSpeed         = NULL;
   windDir           = NULL;
   rh                = NULL;
   temp              = NULL;

   missingValue      = MAXDOUBLE;

   siteLat           = 0.0;
   siteLon           = 0.0;
   siteAlt           = 0.0;
   siteId            = 0;
   siteName          = EMPTY_STRING;

   launchTime        = 0;
   leadSecs          = 0;
}

Sounding::~Sounding() 
{
   clearData();
}

void
Sounding::getStats( field_t field, double *min, double *max, double *avg )
{
   switch( field ) {
      case ALTITUDE:
           getStats( altitude, min, max, avg );
           break;
      case PRESSURE:
           getStats( pressure, min, max, avg );
           break;
      case U_WIND:
           getStats( uwind, min, max, avg );
           break;
      case V_WIND:
           getStats( vwind, min, max, avg );
           break;
      case W_WIND:
           getStats( wwind, min, max, avg );
           break;
      case REL_HUMIDITY:
           getStats( rh, min, max, avg );
           break;
      case TEMPERATURE:
           getStats( temp, min, max, avg );
           break;
   }
}

void
Sounding::getStats( double *fieldPtr, double *min, double *max, double *avg )
{
   //
   // Degenerate case
   //
   *min = *max = *avg = missingValue;
   if ( numObs <= 0 )
      return;

   //
   // Initialize min/max to the first field value
   //
   double value, minVal, maxVal, sumVal;
   minVal = maxVal = fieldPtr[0];
   sumVal = 0.0;

   int count = 0;

   for( int i=1; i < numObs; i++ ) {
      value = fieldPtr[i];
      if( value != missingValue ) {
	 
	 sumVal += value;
         count++;
	 
	 if ( value > maxVal )
	    maxVal = value;
	 if ( value < minVal )
	    minVal = value;
      }
       
   }

   //
   // Return the values that were requested,
   // If count is zero, the initial values for
   // min, max and avg will apply, i.e. they
   // will be set to the missing value.
   //
   if( count > 0 ) {
      *min = minVal;
      *max = maxVal;
      *avg = sumVal / count;
   }
}

const char*
Sounding::getSourceName()
{
   switch( sourceId ) {
      case SONDE_ID:
           return SONDE_NAME  ;
           break;
      case PROFILER_ID:
           return PROFILER_NAME  ;
           break;
      case RUC_ID:
           return RUC_NAME  ;
           break;
      case VAD_ID:
           return VAD_NAME  ;
           break;
      case DEFAULT_ID:
           return DEFAULT_NAME  ;
           break;
      case INVALID_ID:
           return INVALID_NAME  ;
           break;
   }

   return EMPTY_STRING;
}

void
Sounding::getDirSpeed( double uVal, double vVal, double *dir, double *speed )
{
   float dirVal, speedVal;
   assert( dir && speed );

   //
   // Get the direction and speed in radar coordinates,
   // i.e. clockwise from true north
   //
   uv_2_wind_dir_speed( (float)uVal, (float)vVal, &dirVal, &speedVal );

   *dir   = dirVal;
   *speed = speedVal;
}

void
Sounding::clearData() 
{
    delete[] pressure;
    delete[] altitude;
    delete[] uwind;
    delete[] vwind;
    delete[] wwind;
    delete[] rh;
    delete[] temp;
    delete[] windSpeed;
    delete[] windDir;
}

int
Sounding::resetData( int numPts ) 
{
   if( numPts <= 0 ) {
      valid = false;
      return( -1 );
   }
   
   if( numPts > numAlloc ) {
      clearData();

      pressure  = new double[numPts];
      altitude  = new double[numPts];
      uwind     = new double[numPts];
      vwind     = new double[numPts];
      wwind     = new double[numPts];
      rh        = new double[numPts];
      temp      = new double[numPts];
      windSpeed = new double[numPts];
      windDir   = new double[numPts];

      numAlloc = numPts;
   }

   for( int i = 0; i < numPts; i++ ) {
      pressure[i]  = missingValue;
      altitude[i]  = missingValue;
      uwind[i]     = missingValue;
      vwind[i]     = missingValue;
      wwind[i]     = missingValue;
      rh[i]        = missingValue;
      temp[i]      = missingValue;
      windSpeed[i] = missingValue;
      windDir[i]   = missingValue;
   }

   numObs = numPts;
   return( 0 );
   
}
