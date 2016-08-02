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
// Sounding class 
//   base class for SoundingPut and SoundingGet
//
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// April 1999
//
// $Id: Sounding.cc,v 1.10 2016/03/03 18:13:05 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <rapmath/umath.h>
#include <Spdb/Sounding.hh>
#include <cassert>
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
   div               = NULL;

   missingValue      = DBL_MAX;

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
Sounding::getStats( field_t field, double *min, double *max, double *avg ) const
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
      case DIVERGENCE:
           getStats( div, min, max, avg );
           break;
   }
}

void
Sounding::getStats( double *fieldPtr, double *min, double *max, double *avg ) const
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
Sounding::getSourceName() const
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
    delete[] div;
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

      pressure   = new double[numPts];
      altitude   = new double[numPts];
      uwind      = new double[numPts];
      vwind      = new double[numPts];
      wwind      = new double[numPts];
      rh         = new double[numPts];
      temp       = new double[numPts];
      div        = new double[numPts];
      windSpeed  = new double[numPts];
      windDir    = new double[numPts];

      numAlloc = numPts;
   }

   for( int i = 0; i < numPts; i++ ) {
      pressure[i]   = missingValue;
      altitude[i]   = missingValue;
      uwind[i]      = missingValue;
      vwind[i]      = missingValue;
      wwind[i]      = missingValue;
      rh[i]         = missingValue;
      temp[i]       = missingValue;
      div[i]        = missingValue;
      windSpeed[i]  = missingValue;
      windDir[i]    = missingValue;
   }

   numObs = numPts;
   return( 0 );
   
}
