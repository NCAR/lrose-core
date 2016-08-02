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

#include <iostream>
#include <ctime>

#include <Spdb/DsSpdb.hh>
#include <rapformats/station_reports.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <physics/physics.h>

#include "Params.hh"

int main(int argc, char *argv[]){

  Params P;

  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }                       
  
  PMU_auto_init("simStations", P.instance,
                PROCMAP_REGISTER_INTERVAL);  


  time_t nextProcessTime = (time_t) P.frequency * (time(NULL)/P.frequency);


  while (1) { // Loop enternal

    //
    // Wait for next process time.
    //
    time_t now;
    do {
      PMU_auto_register("working away...");
      sleep(1);
      now = time(NULL);
    } while ( now < nextProcessTime );

    //
    // Loop through stations.
    //

    DsSpdb Out;
    Out.clearPutChunks();

    for (int i=0; i < P.station_n; i++){
      
      station_report_t S;
      memset( &S, 0, sizeof (station_report_t));

      S.time = nextProcessTime;
      S.lat = P._station[i].lat;
      S.lon = P._station[i].lon;
      S.alt = P._station[i].alt;
      S.temp = P._station[i].temp;
      S.relhum = P._station[i].relhum;
      S.pres = P._station[i].pres;

      double dewPoint = PHYrhdp(S.temp, S.relhum);

      S.dew_point = dewPoint;

      S.windspd = P._station[i].windSpeed;
      S.winddir = double((S.time/60) % 360);

      S.windgust = P._station[i].windSpeed;

      S.liquid_accum = STATION_NAN;
      S.precip_rate = STATION_NAN;
      S.visibility = STATION_NAN;
      S.rvr = STATION_NAN;
      S.ceiling = STATION_NAN;

      int dataType = Spdb::hash4CharsToInt32( P._station[i].ID );

      station_report_to_be( &S );

      Out.addPutChunk(dataType,
		      nextProcessTime,
		      nextProcessTime,
		      sizeof( station_report_t ),
		      &S);
    }

    Out.put(P.outUrl,
	    SPDB_STATION_REPORT_ID,
	    SPDB_STATION_REPORT_LABEL);
    
     nextProcessTime += P.frequency;

  }



  return 0;

}
