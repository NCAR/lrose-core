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

#include <cstdio>
#include <string.h>
#include <cstdlib> 

#include <Spdb/DsSpdb.hh>
#include <rapformats/station_reports.h>
#include <physics/physics.h>
#include <toolsa/umisc.h>
using namespace std;


int main(int argc, char *argv[]){


  const int deltaT = 300;

  long NumOut = 0;

  date_time_t start, end;

  start.year = 2000;
  start.month = 6;
  start.day = 26;
  start.hour = 23;
  start.min=0;
  start.sec = 0;


  end.year = 2000;
  end.month = 6;
  end.day = 29;
  end.hour = 1;
  end.min=0;
  end.sec = 0;
 
  uconvert_to_utime( &start );
  uconvert_to_utime( &end );


  time_t t = start.unix_time;


  DsSpdb InSpdb, OutSpdb;


  OutSpdb.clearPutChunks();
  OutSpdb.clearUrls();
  OutSpdb.addUrl("./ProcessedLLWAS");   
  station_report_t *report; 

  do{ // Main loop through time.

    if (InSpdb.getInterval("../llwasIngest/Out",
			   t,
			   t +  deltaT - 1)){

      fprintf(stderr,"Get interval failure!\n");
      exit(-1);
    }

    if (InSpdb.getNChunks() == 0){
      t = t + deltaT;
      continue;
    }

    //
    // Loop through stations at this time.
    //
    float Ubar = 0.0;
    float Vbar = 0.0;
    int Num = 0;
    report = (station_report_t *) InSpdb.getChunks()[0].data;
    for (int i=0; i < InSpdb.getNChunks(); i++){
      station_report_from_be(&report[i]);

      if ( fabs(report[i].windspd) < 0.01) continue; // skip 0 entries

      Ubar = Ubar + PHYwind_u(report[i].windspd, report[i].winddir);
      Vbar = Vbar + PHYwind_v(report[i].windspd, report[i].winddir);
      Num++;

    }

    if (Num > 0){
      Ubar = Ubar/Num;
      Vbar = Vbar/Num;
      station_report_t S;
      S.time = t + deltaT;
      S.winddir = PHYwind_dir(Ubar, Vbar);
 
      S.windspd = PHYwind_speed(Ubar,Vbar);

      S.weather_type = 0;
	
      S.temp = STATION_NAN;
      S.dew_point = STATION_NAN;
      S.relhum = STATION_NAN;
      S.windgust = STATION_NAN;
      S.pres = STATION_NAN;
      S.liquid_accum = STATION_NAN;
      S.precip_rate = STATION_NAN;
      S.visibility = STATION_NAN;
      S.rvr = STATION_NAN;
      S.ceiling = STATION_NAN;

      S.lat = 32.912500;
      S.lon = -97.091111;
      S.alt = 190.0;
      //
      //
      //
      int dataType = Spdb::hash4CharsToInt32( "LWAS" );

      time_t vt = S.time;

      station_report_to_be( &S );

      OutSpdb.addPutChunk(dataType,
			  vt,
			  vt + 3600, // Hour valid time - hardcoded.
			  sizeof( station_report_t),
			  (void *) &S );
      NumOut++;
      
    }

    t = t + deltaT;

  } while (t < end.unix_time);


  
  OutSpdb.put(SPDB_STATION_REPORT_ID,
	      SPDB_STATION_REPORT_LABEL);

  fprintf(stdout,"%ld metars written.\n",
	  NumOut);
    

  return 0;


}


