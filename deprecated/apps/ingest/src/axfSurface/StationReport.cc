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
// $Id: StationReport.cc,v 1.5 2016/03/07 01:23:07 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <rapmath/math_macros.h>
#include <toolsa/str.h>
#include <toolsa/MsgLog.hh>
#include <physics/thermo.h>

#include "AxfSurface.hh"
#include "StationReport.hh"
using namespace std;

StationReport::StationReport() :
   missingValue( FLT_MAX )
{ 
   clearObs();
}

StationReport::StationReport( const char* stationName, 
                              const char* stationDesc, 
                              int stationId,
                              float stationLat, 
                              float stationLon, 
                              float stationAlt,
                              float missingVal,
                              reportType tipe )
{
   //
   // Hang onto relevant info which does not get stored in the report struct
   //
   name = stationName;
   description = stationDesc;
   id = stationId;
   missingValue = missingVal;

   //
   // Fill in the report struct with persistent data
   //
   clearObs();
   report.lat = stationLat;
   report.lon = stationLon;
   report.alt = stationAlt;
   report.msg_id = tipe == STATION_TYPE ? STATION_REPORT : METAR_REPORT;
   STRcopy( report.station_label, name.c_str(), ST_LABEL_SIZE );
}

StationReport::~StationReport() 
{
}

void
StationReport::clearObs()
{
   memset( &report, 0, sizeof( station_report_t ));

   report.temp             = STATION_NAN;
   report.dew_point        = STATION_NAN;
   report.relhum           = STATION_NAN;
   report.windspd          = STATION_NAN;
   report.winddir          = STATION_NAN;
   report.windgust         = STATION_NAN;
   report.pres             = STATION_NAN;
   report.precip_rate      = STATION_NAN;
   report.visibility       = STATION_NAN;
   report.rvr              = STATION_NAN;
   report.ceiling          = STATION_NAN;
   report.liquid_accum     = STATION_NAN;
}

void
StationReport::set( time_t when, float  temperature,
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
                                 time_t accumSinceWhen )
{
   //
   // Fill in the report structure with transient data
   //
   report.time = when;
   report.temp = temperature == missingValue ? STATION_NAN : temperature;
   report.dew_point = dewPoint == missingValue ? STATION_NAN : dewPoint;
   report.relhum = relHumidity == missingValue ? STATION_NAN : relHumidity;
   report.windspd = windSpeed == missingValue ? STATION_NAN : windSpeed;
   report.winddir = windDir == missingValue ? STATION_NAN : windDir;
   report.windgust = windGust == missingValue ? STATION_NAN : windGust;
   report.pres = pressure == missingValue ? STATION_NAN : pressure;
   report.precip_rate = precipRate == missingValue ? STATION_NAN : precipRate;
   report.visibility = visibility == missingValue ? STATION_NAN : visibility;
   report.rvr = runwayVisRange == missingValue ? STATION_NAN : runwayVisRange;
   report.ceiling = ceiling == missingValue ? STATION_NAN : ceiling;
   report.liquid_accum = liquidAccum == missingValue ? STATION_NAN : liquidAccum;
   report.accum_start_time = accumSinceWhen;
}

void
StationReport::print() const
{
  time_t print_time;

  POSTMSG( INFO, "STATION REPORT: ");
  
  POSTMSG( INFO, "  msg_id = %d", report.msg_id );

  print_time = report.time;
  POSTMSG( INFO, "  time = %s", asctime(gmtime(&print_time)) );
  
  print_time = report.accum_start_time;
  POSTMSG( INFO, "  accum_start_time = %s", asctime(gmtime(&print_time)) );
  
  POSTMSG( INFO, "  weather_type = %s",
	   weather_type2string(report.weather_type) );
  
  if(report.lat != STATION_NAN) {
      POSTMSG( INFO, "  lat = %.3f deg", report.lat );
  } else {
      POSTMSG( INFO, "  lat = NOT SET");
  }

  if(report.lon != STATION_NAN) {
      POSTMSG( INFO, "  lon = %.3f deg", report.lon );
  } else {
      POSTMSG( INFO, "  lon = NOT SET" );
  }

  if(report.alt != STATION_NAN) {
      POSTMSG( INFO, "  alt = %.3f m", report.alt );
  } else {
      POSTMSG( INFO, "  alt = NOT SET" );
  }

  if(report.temp != STATION_NAN) {
      POSTMSG( INFO, "  temp = %.3f deg C", report.temp );
  } else {
      POSTMSG( INFO, "  temp = NOT SET" );
  }

  if(report.dew_point != STATION_NAN) {
      POSTMSG( INFO, "  dew_point = %.3f deg C", report.dew_point );
  } else {
      POSTMSG( INFO, "  dew_point = NOT SET" );
  }

  if(report.relhum != STATION_NAN) {
      POSTMSG( INFO, "  relhum = %.3f %%", report.relhum );
  } else {
      POSTMSG( INFO, "  relhum = NOT SET" );
  }

  if(report.windspd != STATION_NAN) {
      POSTMSG( INFO, "  windspd = %.3f m/s", report.windspd );
  } else {
      POSTMSG( INFO, "  windspd = NOT SET" );
  }

  if(report.winddir != STATION_NAN) {
      POSTMSG( INFO, "  winddir = %.3f deg", report.winddir );
  } else {
      POSTMSG( INFO, "  winddir = NOT SET" );
  }

  if(report.windgust != STATION_NAN) {
      POSTMSG( INFO, "  windgust = %.3f m/s", report.windgust );
  } else {
      POSTMSG( INFO, "  windgust = NOT SET" );
  }

  if(report.pres != STATION_NAN) {
      POSTMSG( INFO, "  pres = %.3f mb", report.pres );
  } else {
      POSTMSG( INFO, "  pres = NOT SET" );
  }

  if(report.liquid_accum != STATION_NAN) {
      POSTMSG( INFO, "  liquid_accum = %.3f mm", report.liquid_accum );
  } else {
      POSTMSG( INFO, "  liquid_accum = NOT SET" );
  }

  if(report.precip_rate != STATION_NAN) {
      POSTMSG( INFO, "  precip_rate = %.3f mm/hr", report.precip_rate );
  } else {
      POSTMSG( INFO, "  precip_rate = NOT SET" );
  }

  if(report.visibility != STATION_NAN) {
      POSTMSG( INFO, "  visibility = %.3f km", report.visibility );
  } else {
      POSTMSG( INFO, "  visibility = NOT SET" );
  }
   
  if(report.rvr != STATION_NAN) {
      POSTMSG( INFO, "  rvr = %.3f km", report.rvr );
  } else {
      POSTMSG( INFO, "  rvr = NOT SET" );
  }
   
  if(report.ceiling != STATION_NAN) {
      POSTMSG( INFO, "  ceiling = %.3f km", report.ceiling );
  } else {
      POSTMSG( INFO, "  ceiling = NOT SET" );
  }
 
  POSTMSG( INFO, "  Metar weather = <%s>", 
                 report.shared.metar.weather_str );

  POSTMSG( INFO, "  station_label = <%s>", report.station_label );

  POSTMSG( INFO, " ");
}
