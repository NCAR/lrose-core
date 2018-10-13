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
/**
 * @file Sounding.hh
 * @brief Information for a sounding
 * @class Sounding
 * @brief Information for a sounding
 *
 *
 *  Client interface to sounding database
 *  and base class for SoundingPut and SoundingGet
 *  The relationship of these three classes is based loosely on 
 *  the "builder pattern" where:
 *   - the client == director
 *   - Sounding   == builder
 *   - SoundingPut and SoundingGet == concrete builders
 */
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// April 1999
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _SOUNDING_HH_
#define _SOUNDING_HH_

#include <string>
#include <vector>
#include <cfloat>
#include <Spdb/sounding.h>
#include <Spdb/DsSpdb.hh>
#include <cassert>
using namespace std;

class Sounding
{
public:

   enum source_t { INVALID_ID = -1,
                   DEFAULT_ID =  0,
                   SONDE_ID,
                   PROFILER_ID,
                   RUC_ID,
                   VAD_ID };

   static const char* SONDE_NAME;
   static const char* PROFILER_NAME;
   static const char* RUC_NAME;
   static const char* VAD_NAME;
   static const char* DEFAULT_NAME;
   static const char* INVALID_NAME;
   static const char* EMPTY_STRING;

   enum field_t { ALTITUDE,
                  PRESSURE,
                  U_WIND,
                  V_WIND,
                  W_WIND,
                  REL_HUMIDITY,
                  TEMPERATURE,
                  DIVERGENCE };

   Sounding(){};
   virtual ~Sounding();

   //
   // Default initialization
   //
   void init();

   /////////////////////////////////////////////////////////////////////////////
   // PUT methods are overloaded by SoundingPut class
   /////////////////////////////////////////////////////////////////////////////
   //
   // Put: initialization
   //

   //
   // Put: Set descriptive info
   //
   virtual void           setSiteId( int id )
                            { assert( false ); }
   virtual void           setSiteName( const char* name )
                            { assert( false ); }
   virtual void           setSiteName( const string& name )
                            { assert( false ); }
   virtual void           setSourceId( source_t id )
                            { assert( false ); }
   virtual void           setLocation( double lat, double lon, double alt ) 
                            { assert( false ); }
   //
   // Put: Set sounding data from arrays of doubles
   //
   virtual int set( time_t launch, 
                    int numPts, 
                    double *height, 
                    double *u, 
                    double *v,
        	    double *w = NULL, 
                    double *prs = NULL,
                    double *relHum = NULL, 
                    double *temperature = NULL,
                    double *divergence = NULL)
                  { assert( false ); }

   //
   // Put: Set sounding data from vectors of doubles
   //
   virtual int set( time_t launch,
                    vector<double> *height,
                    vector<double> *u, 
                    vector<double> *v,
                    vector<double> *w = NULL, 
                    vector<double> *prs = NULL,
                    vector<double> *relHum = NULL,
                    vector<double> *temperature = NULL,
                    vector<double> *divergence = NULL)
                  { assert( false ); }

   /////////////////////////////////////////////////////////////////////////////
   // GET methods are overloaded by SoundingGet class
   /////////////////////////////////////////////////////////////////////////////
   //
   // Get: initialization
   //

   //
   // Get: Examine state and descriptive info
   //
   virtual void           useDefaults() { assert( false ); }
   virtual void           useSndgData() { assert( false ); }
   virtual int            getSiteId() const { return siteId; }
   virtual const string&  getSiteName() const { return siteName; }
   virtual int            getSourceId() const { return sourceId; }
   virtual double         getLat() const { return siteLat; }
   virtual double         getLon() const { return siteLon; }
   virtual time_t         getLaunchTime() const { return launchTime; }

   // 
   // Get: Reading sounding from spdb
   //
   // int                    readSounding( time_t when, int whichSiteId = 0 )
   //                                    { assert( false ); }

   virtual void           loadProduct( int productIndex )
                                     { assert( false ); }

   //
   // Get: Examine sounding data
   //
   virtual double *getPres() const { assert( false ); }
   virtual double *getAlts() const { assert( false ); }
   virtual double *getU() const { assert( false ); }
   virtual double *getV() const { assert( false ); }
   virtual double *getW() const { assert( false ); }
   virtual double *getRH() const { assert( false ); }
   virtual double *getTemp() const { assert( false ); }
   virtual double *getDivergence() const { assert( false ); }
   virtual double *getWindSpeed() const { assert( false ); }
   virtual double *getWindDir() const { assert( false ); }

   //
   // Get: Examine derived data
   // 
   virtual int     getNumPoints() const { assert( false ); }
   virtual void    getUV( double *u, double *v,
                          double speedRange = DBL_MAX ) const
                      { assert( false ); }
   virtual void    getUV( double alt, double *uVal, double *vVal ) const
                      { assert( false ); }
   virtual void    getDirSpeed( double alt, double *dir, double *speed ) const
                      { assert( false ); }
   virtual int     advect( time_t fromWhen, time_t extrapSeconds, 
                           double *kmX, double *kmY,
                           double *u = NULL, double *v = NULL )
                      { assert( false ); }

   //
   // Methods handled generically by this base class, thus not virtual
   //
   void           getStats( field_t field, 
                            double *min, double *max, double *avg ) const;
   bool           isValid() const{ return valid; }
   void           setMissingValue( double value ){ missingValue = value; }
   double         getMissingValue() const{ return missingValue; }

   //
   // SourceId is limited to the types defined above by source_t
   // SourceName is derived from sourceId
   //
   const char*    getSourceName() const;

   //
   // Static utility for tranlating u/v to dir/speed
   // *USING WIND CONVENTION* for identifing FROM direction
   //
   static void getDirSpeed( double uVal, double vVal,
                            double *dir, double *speed );

   // access to spdb manager - for error messages etc

   const DsSpdb &getSpdbMgr() const { return spdbMgr; }

protected:

   //
   // Descriptive sounding info
   //
   source_t     sourceId;
   string       sourceFmt;
   time_t       launchTime;                  // unix time
   int          leadSecs;                    // offset from valid data time
   double       siteLat;                     // degrees
   double       siteLon;                     // degrees
   double       siteAlt;                     // km
   int          siteId;
   string       siteName;
   double       missingValue;
   bool         valid;

   //
   // Standard sounding data
   //
   double *pressure;                    // mb
   double *altitude;                    // m
   double *uwind;                       // m/s
   double *vwind;                       // m/s
   double *wwind;                       // m/s
   double *rh;                          // %
   double *temp;                        // degrees C
   double *div;                         // divergence /s * 1.0e5

   //
   // Derived sounding data
   //
   double *windSpeed;
   double *windDir;

   //
   // Spdb database management
   //
   DsSpdb    spdbMgr;

   //
   // Management of sounding data
   //
   int     numObs;
   int     resetData( int numPts );
   
private:

   //
   // Management of sounding data
   //
   int     numAlloc;
   void    clearData();

   void    getStats( double *fieldPtr, double *min, double *max, double *avg ) const;
   
};

#endif
