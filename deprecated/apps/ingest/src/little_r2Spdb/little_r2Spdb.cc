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
//
// This is the module of the little_r2Spdb application that does most
// of the work, reading the netCDF and writing the MDV files. Include
// both MDV and netCDF header files.
//
#include "little_r2Spdb.hh"

#include <iostream>
#include <stdlib.h>
#include <cstdio>

#include <Spdb/DsSpdb.hh>
#include <rapformats/station_reports.h>
#include <physics/physics.h>

#include <toolsa/umisc.h>

//
// Constructor - makes a copy of a pointer to the TDRP parameters.
//
little_r2Spdb::little_r2Spdb(Params *P){
  _params = P;
  return;
}
//
// Destructor - does nothing but avoids default destructor.
//
little_r2Spdb::~little_r2Spdb(){
  return;
}
//
// Main method - the conversion.
//
void little_r2Spdb::little_r2SpdbFile( char *FilePath ){
 
  if (_params->debug){
    cerr << "Processing " << FilePath << endl;
  }


  FILE *fp = fopen(FilePath, "r");
  if (fp == NULL){
    cerr << FilePath << " not found." << endl;
    return;
  }

  char Line[2048];

  char b[100];

  char id[5];
  id[4] = char(0);

  DsSpdb Out;

  while(NULL != fgets(Line, 2048, fp)){

    for(int i=0; i < 4; i++)
      id[i] = Line[i+40];

    double lat;
    sscanf(Line,"%lf", &lat);

    double lon;
    for (int k=30; k <40; k++){
      b[k-30] = Line[k];
      b[k-29] = char(0);
    }
    sscanf(b,"%lf", &lon);

    date_time_t T;

    sscanf(Line+326, "%4d%2d%2d%2d%2d%2d",
	   &T.year, &T.month, &T.day, &T.hour, &T.min, &T.sec);

    uconvert_to_utime( &T );

    double elev;
    sscanf(Line+210, "%lf", &elev);


    double pres;
    sscanf(Line+341, "%lf", &pres);
    pres = pres /100.0;
    if ((pres > 1500) || (pres < 800)) pres = -999;

    fgets(Line, 2048, fp);

    double ws, wd, dp, tp;
    sscanf(Line+44, "%lf", &tp);
    sscanf(Line+64, "%lf", &dp);
    sscanf(Line+84, "%lf", &ws);
    sscanf(Line+104, "%lf", &wd);

    tp -= 273.16;
    dp -= 273.16;

    if (
	(wd < 0.0) ||
	(wd > 360) ||
	(ws > 30) || 
	(ws < 0.0)
	) {
      ws = -999; wd = -999;
    }

    if ((tp > 50) || (tp < -50)) tp = -999;
    if ((dp > 50) || (dp < -50)) dp = -999;


    if (_params->debug){
      fprintf(stderr,"\"%s\" ", id);
      
      fprintf(stderr,"Lat=%g Lon=%g %d/%02d/%02d %02d:%02d:%02d at %gm %gmb ", 
	      lat, lon, T.year, T.month, T.day, T.hour, T.min, T.sec,
	      elev, pres);
      

      fprintf(stderr," %gm/s from %g, dew %g temp %gC\n",
	      ws, wd, dp, tp);
    }

    fgets(Line, 2048, fp);
    fgets(Line, 2048, fp);

    station_report_t S;
    memset(&S, 0, sizeof(station_report_t));

    S.lat = lat;
    S.lon = lon;
    S.alt = elev;
    
    if (tp < -100.0)
      S.temp = STATION_NAN;
    else
      S.temp = tp;

    if (dp < -100.0)
      S.dew_point = STATION_NAN;
    else
      S.dew_point = dp;

    S.relhum = STATION_NAN;
    if (
	(S.dew_point != STATION_NAN) &&
	(S.temp != STATION_NAN)
	){
      S.relhum = PHYrelh(S.temp, S.dew_point);
    }

    
    if (ws < -100.0)
      S.windspd = STATION_NAN;
    else
      S.windspd = ws;

    if (wd < -100.0)
      S.winddir = STATION_NAN;
    else
      S.winddir = wd;

    if (pres < -100.0)
      S.pres = STATION_NAN;
    else
      S.pres = pres;

    S.liquid_accum = STATION_NAN;
    S.precip_rate = STATION_NAN;
    S.visibility = STATION_NAN;
    S.rvr = STATION_NAN;
    S.ceiling = STATION_NAN;

    S.time = T.unix_time;

    if (_params->debug) print_station_report( stderr, "REPORT : ", &S);

    station_report_to_be( &S );

    int dataType = Spdb::hash4CharsToInt32( id );
    Out.addPutChunk(dataType,
                    T.unix_time,
                    _params->Expiry + T.unix_time,
                    sizeof(station_report_t),
                    &S);


  }
  
  fclose(fp);
  
  if (Out.put(_params->outUrl,
	      SPDB_STATION_REPORT_ID,
	      SPDB_STATION_REPORT_LABEL) != 0){
    cerr << "SPDB write failed to URL " << _params->outUrl << endl;
  }

  return;
}
