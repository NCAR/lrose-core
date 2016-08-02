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
/////////////////////////////////////////////////
// Metar
//
/////////////////////////////////////////////////

#include <string>
#include <rapmath/math_macros.h>
#include <toolsa/str.h>
#include <toolsa/udatetime.h>
#include <toolsa/MsgLog.hh>
#include <physics/thermo.h>

#include "Metar.hh"
#include "MetarLocation.hh"
#include "Metar2Spdb.hh"

#include <rapformats/metar_decode.h>
#include <Spdb/Spdb.hh>
using namespace std;

Metar::Metar(const Params &params,
	     int file_year, int file_month, int file_day, int file_hour, int file_min,
	     int block_hour, int block_min) :
  _params(params)
{
   lat = 0.0;
   lon = 0.0;
   alt = 0.0;
   fileYear = file_year;
   fileMonth = file_month;
   fileDay = file_day;
   fileHour = file_hour;
   fileMin = file_min;
   blockHour = block_hour;
   blockMin = block_min;
}

Metar::~Metar() 
{

}

int
Metar::setMetar(const char* message, char* fileName,
                map< string, MetarLocation*, less<string> >& locations)
{  
   int resetMetarStruct = 1;
   char *messageCopy;
   
   map< string, MetarLocation*, less<string> >::iterator it;

   // 
   // Copy the message so we do not corrupt the original
   //
   rawText = message;
   messageCopy = STRdup(message);

   //
   // Decode a METAR - this uses strtok() so it corrupts the message copy
   //
   if( DcdMETAR(messageCopy, &dcdMetar, resetMetarStruct) == 0 ) {
      stationId = dcdMetar.stnid;
      it = locations.find(stationId);
      if( it != locations.end() ) {
         lat = (*it).second->getLat();
         lon = (*it).second->getLon();
         alt = (*it).second->getAlt();
      } else {
	 POSTMSG( DEBUG, "%s not in the list", stationId.c_str() );
	 STRfree(messageCopy);
	 return (FAILURE);
      }
   } else {
     POSTMSG( DEBUG, "Metar could not be processed");
     POSTMSG( DEBUG, "File: %s", fileName );
     POSTMSG( DEBUG, "Metar: %s", message );
     STRfree(messageCopy);
     return (FAILURE);
   }
   
   int iret = fillStationReport();
   STRfree(messageCopy);

   // compute the hash id

   hashId = Spdb::hash4CharsToInt32(stationReport.station_label);

   if (iret == 0) {
     return (SUCCESS);
   } else {
     return (FAILURE);
   }

}

void
Metar::printStationReport() 
{
  time_t print_time;

  POSTMSG( INFO, "DECODED METAR REPORT: ");
  
  POSTMSG( INFO, "  msg_id = %d", stationReport.msg_id );

  print_time = stationReport.time;
  POSTMSG( INFO, "  time = %s", asctime(gmtime(&print_time)) );
  
  print_time = stationReport.accum_start_time;
  POSTMSG( INFO, "  accum_start_time = %s", asctime(gmtime(&print_time)) );
  
  POSTMSG( INFO, "  weather_type = %s",
	   weather_type2string(stationReport.weather_type) );
  
  if(stationReport.lat != STATION_NAN) {
      POSTMSG( INFO, "  lat = %.3f deg", stationReport.lat );
  } else {
      POSTMSG( INFO, "  lat = NOT SET");
  }

  if(stationReport.lon != STATION_NAN) {
      POSTMSG( INFO, "  lon = %.3f deg", stationReport.lon );
  } else {
      POSTMSG( INFO, "  lon = NOT SET" );
  }

  if(stationReport.alt != STATION_NAN) {
      POSTMSG( INFO, "  alt = %.3f m", stationReport.alt );
  } else {
      POSTMSG( INFO, "  alt = NOT SET" );
  }

  if(stationReport.temp != STATION_NAN) {
      POSTMSG( INFO, "  temp = %.3f deg C", stationReport.temp );
  } else {
      POSTMSG( INFO, "  temp = NOT SET" );
  }

  if(stationReport.dew_point != STATION_NAN) {
      POSTMSG( INFO, "  dew_point = %.3f deg C", stationReport.dew_point );
  } else {
      POSTMSG( INFO, "  dew_point = NOT SET" );
  }

  if(stationReport.relhum != STATION_NAN) {
      POSTMSG( INFO, "  relhum = %.3f %%", stationReport.relhum );
  } else {
      POSTMSG( INFO, "  relhum = NOT SET" );
  }

  if(stationReport.windspd != STATION_NAN) {
      POSTMSG( INFO, "  windspd = %.3f m/s", stationReport.windspd );
  } else {
      POSTMSG( INFO, "  windspd = NOT SET" );
  }

  if(stationReport.winddir != STATION_NAN) {
      POSTMSG( INFO, "  winddir = %.3f deg", stationReport.winddir );
  } else {
      POSTMSG( INFO, "  winddir = NOT SET" );
  }

  if(stationReport.windgust != STATION_NAN) {
      POSTMSG( INFO, "  windgust = %.3f m/s", stationReport.windgust );
  } else {
      POSTMSG( INFO, "  windgust = NOT SET" );
  }

  if(stationReport.pres != STATION_NAN) {
      POSTMSG( INFO, "  pres = %.3f mb", stationReport.pres );
  } else {
      POSTMSG( INFO, "  pres = NOT SET" );
  }

  if(stationReport.liquid_accum != STATION_NAN) {
      POSTMSG( INFO, "  liquid_accum = %.3f mm", stationReport.liquid_accum );
  } else {
      POSTMSG( INFO, "  liquid_accum = NOT SET" );
  }

  if(stationReport.precip_rate != STATION_NAN) {
      POSTMSG( INFO, "  precip_rate = %.3f mm/hr", stationReport.precip_rate );
  } else {
      POSTMSG( INFO, "  precip_rate = NOT SET" );
  }

  if(stationReport.visibility != STATION_NAN) {
      POSTMSG( INFO, "  visibility = %.3f km", stationReport.visibility );
  } else {
      POSTMSG( INFO, "  visibility = NOT SET" );
  }
   
  if(stationReport.rvr != STATION_NAN) {
      POSTMSG( INFO, "  rvr = %.3f km", stationReport.rvr );
  } else {
      POSTMSG( INFO, "  rvr = NOT SET" );
  }
   
  if(stationReport.ceiling != STATION_NAN) {
      POSTMSG( INFO, "  ceiling = %.3f km", stationReport.ceiling );
  } else {
      POSTMSG( INFO, "  ceiling = NOT SET" );
  }
 
  POSTMSG( INFO, "  Metar weather = <%s>", 
                 stationReport.shared.metar.weather_str );

  POSTMSG( INFO, "  station_label = <%s>", stationReport.station_label );

  POSTMSG( INFO, " ");
  
}

///////////////////////
//  fillStationReport()
//
// Returns 0 on success, -1 on failure
//

int
Metar::fillStationReport()
{

  bool isRealtime = false;
  if (_params.mode == Params::REALTIME ||
      _params.mode == Params::REALTIME_WITH_LDATA) {
    isRealtime = true;
  }
  
  int ob_hour = -1;
  int ob_minute = -1;
  if (dcdMetar.ob_hour != MAXINT && dcdMetar.ob_minute != MAXINT) {
    ob_hour = dcdMetar.ob_hour;
    ob_minute = dcdMetar.ob_minute;
  } else {
    ob_hour = blockHour;
    ob_minute = blockMin;
  }

  time_t valid_time;
  
  if (ob_hour == -1 || ob_minute == -1) {
      
    // time not set
    
    if (isRealtime && _params.guess_time_if_missing) {

      // set time to current hour
      
      time_t now = time(NULL);
      time_t now_hour = (now / 3600) * 3600;
      valid_time = now_hour;
      
    } else {
      
      // time missing - no good
      
      return -1;
      
    }
    
  } else {
    
    //
    // Set time - hours and minutes are provided by the decoded
    // metar struct; seconds are zeroed out
    //
    date_time_t ts;
    ts.year  = fileYear;
    ts.month = fileMonth;
    ts.day   = fileDay;
    ts.hour  = ob_hour;
    ts.min   = ob_minute;
    ts.sec   = 0;  
    uconvert_to_utime(&ts);
    valid_time =  ts.unix_time;

    //
    // correct for time in the future if hour and minute is from
    // previous day
    //
    
    time_t now = time(NULL);
    if (isRealtime && valid_time > now) {
      valid_time -= 86400;
    }
    
  }

  if (decoded_metar_to_report(&dcdMetar, &stationReport,
			      valid_time, lat, lon, alt)) {
    return -1;
  } else {
    return 0;
  }
  
}
