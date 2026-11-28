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
// $Id: StationReport.hh,v 1.6 2016/03/07 01:23:07 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
  
#include <string>
#include <float.h>
#include <rapformats/station_reports.h>
using namespace std;


class StationReport {
 public:

   enum reportType { STATION_TYPE, METAR_TYPE };

   StationReport();
   StationReport( const char* name, const char* desc, int id,
                  float lat, float lon, float alt,
                  float missingValue = FLT_MAX,
                  reportType type = STATION_TYPE );
  ~StationReport();
   
   const string&   getName() const { return( name ); }
   const string&   getDescription() const { return( description ); }
   const int       getId() const { return( id ); }
   const float&    getMissingValue() const { return( missingValue ); }

   void     print() const;
   void     clearObs();
   void     getReport( station_report_t &reportCopy ) const
                     { reportCopy = report; }

   void     set( time_t when, float  temperature,
                              float  dewPoint,
                              float  relHumidity,
                              float  windSpeed,
                              float  windDir,
                              float  windGust,
                              float  pressure,
                              float  precipRate,
                              float  visibility,
                              float  runwayVisRange,
                              float  ceiling,
                              float  liquidAccum,
                              time_t accumSinceWhen );

   void setObsTime( time_t when ){ report.time = when; }
   void setObsTime( DateTime when ){ report.time = when.utime(); }

   void setTemperature( float temperature )
                 { report.temp = temperature == missingValue 
                               ? STATION_NAN : temperature; }
   void setDewPoint( float dewPoint )
              { report.dew_point = dewPoint == missingValue 
                                 ? STATION_NAN : dewPoint; }
   void setRelHumidity( float relHumidity )
                 { report.relhum = relHumidity == missingValue 
                                 ? STATION_NAN : relHumidity; }
   void setWindSpeed( float windSpeed )
               { report.windspd = windSpeed == missingValue 
                                ? STATION_NAN : windSpeed; }
   void setWindDir( float windDir )
             { report.winddir = windDir == missingValue 
                              ? STATION_NAN : windDir; }
   void setWindGust( float windGust )
              { report.windgust = windGust == missingValue 
                                ? STATION_NAN : windGust; }
   void setPressure( float pressure )
              { report.pres = pressure == missingValue 
                            ? STATION_NAN : pressure; }

   void setLiquidAccum( float liquidAccum, time_t sinceWhen )
                 { report.liquid_accum = liquidAccum == missingValue 
                                       ? STATION_NAN : liquidAccum;
                   report.accum_start_time = sinceWhen; }
   void setPrecipRate( float precipRate )
                { report.precip_rate = precipRate == missingValue 
                                     ? STATION_NAN : precipRate; }
   void setVisibility( float visibility )
                { report.visibility = visibility == missingValue 
                                    ? STATION_NAN : visibility; }
   void setRunwayVisRange( float runwayVisRange )
                    { report.rvr = runwayVisRange == missingValue 
                                 ? STATION_NAN : runwayVisRange; }
   void setCeiling( float ceiling )
             { report.ceiling = ceiling == missingValue 
                              ? STATION_NAN : ceiling; }

 private:

   string             name;
   string             description;
   int                id;
   float              missingValue;

   station_report_t   report;

   //
   // Disallow copy constuctor for now
   //
   StationReport( const StationReport& source );
};
