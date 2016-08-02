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
#include <math.h>
#include <toolsa/pmu.h>
#include <physics/physics.h>
#include <stdlib.h>

#include <rapformats/station_reports.h>
#include <Spdb/DsSpdb.hh>


#include "mysqlSurface2Spdb.hh"

//
// Constructor. Copies parameters.
//
mysqlSurface2Spdb::mysqlSurface2Spdb(Params *TDRP_params){
  //
  //
  // Point to the TDRP parameters.
  //
  _params = TDRP_params;
  return;
}
//
// Destructor. Does nothing.
//
mysqlSurface2Spdb::~mysqlSurface2Spdb(){
  return;
}
//
// Main routine.
//
void mysqlSurface2Spdb::mysqlSurface2SpdbFile( char *filename ){
  //
  // Initialize variables.
  //

  if (_params->debug){
    time_t now = time(NULL);
    fprintf(stderr,"Processing file %s at %s\n", 
	    filename, utimstr(now));
  }

  FILE *fp = fopen(filename,"r");
  if (fp == NULL){
    fprintf(stderr, "%s not found.\n", filename);
    return;
  }

  _inputFormat=_unknownFormat;

  const int lineLen = 1024;
  char Line[lineLen];

  station_report_t R;

  int numRead = 0;

  DsSpdb Out;

  while (NULL!=fgets(Line, lineLen, fp)){

    int stationID;
    date_time_t T;
    if (_params->debug >= Params::DEBUG_DATA){
      fprintf(stderr,"Line : %s", Line);
    }
    int retVal = _parseLine(Line, &R, &stationID, &T);
    // fprintf(stderr,"RETVAL : %d\n", retVal);
    if (retVal == -1){
      if (_params->debug >= Params::DEBUG_DATA){
	fprintf(stderr,"  -- Line rejected.\n");
      }
      continue;
    } else {
      //
      // Do some debugging, if requested.
      //
      if (_params->debug >= Params::DEBUG_DATA){
	fprintf(stderr,"Station ID : %d\n", stationID);
	print_station_report(stderr, "  ", &R);
      }
      numRead++;
      //
      //
      // Save the point out.
      //

      station_report_to_be( &R );

      Out.addPutChunk(  stationID,
			T.unix_time + _params->timeOffset,
			T.unix_time + _params->timeOffset + _params->Expiry,
			sizeof( R ), &R, 0);

    }

  }

  if (_params->debug){
    fprintf(stderr,"%d station entries read.\n", numRead);
  }

  fclose( fp );

  if (Out.put( _params->outUrl,
	       SPDB_STATION_REPORT_ID,
	       SPDB_STATION_REPORT_LABEL)){
    fprintf(stderr,"Failed to put surface data\n");
    exit(-1);
  }

  return;

}

/////////////////////////////////////////////////////////
//
// The method that parses a line from a file. You pass in the line,
// you get back the station report and the ID.
//
int mysqlSurface2Spdb::_parseLine(char *Line, 
				  station_report_t *R, 
				  int *stationID,
				  date_time_t *T){

  if (_inputFormat == _unknownFormat){

    if (NULL != strstr(Line, "vsbk")){
      _inputFormat = _metarFormat;
      if (_params->debug){
	fprintf(stderr,"Input format is METAR\n");
      }
      return -1;
    }

    if (NULL != strstr(Line, "srad")){
      _inputFormat = _samsFormat;
      if (_params->debug){
	fprintf(stderr,"Input format is sams\n");
      }
      return -1;
    }

    return -1; // Cannot proceed, we know not what the format is. Go to the next line.

  }

  //
  // Zero out this memory.
  //
  memset(R, 0, sizeof(station_report_t));
  R->msg_id = STATION_REPORT;
  //
  // Set fields not available to bad.
  //
  R->precip_rate = STATION_NAN;
  R->visibility = STATION_NAN;
  R->rvr = STATION_NAN;
  R->ceiling = STATION_NAN;
  R->shared.station.liquid_accum2 = STATION_NAN;
  R->shared.station.Spare1 = STATION_NAN;
  R->shared.station.Spare2 = STATION_NAN;
  //
  // Try to get the time in 2003-12-18 03:30:00 format.
  //
  char *p;
  p = strtok(Line,",");  
  if (p == NULL){
    if (_params->debug){
      fprintf(stderr,"Missing date/time string.\n");
    }
    return -1;
  }
  
  if (6 != sscanf(p, "%4d-%2d-%2d %2d:%2d:%2d",
		  &T->year, &T->month, &T->day,
		  &T->hour, &T->min, &T->sec)) {
    if (_params->debug){
      fprintf(stderr,"Incorrect date/time string.\n");
    }
    return -1;
  }	
  

  uconvert_to_utime( T );

  //
  // See if it is recent enough.
  //
  if (
      (_params->mode == Params::REALTIME) &&
      (_params->applyTimeTest)
      ){
    time_t age = time(NULL) - T->unix_time;
    if (age < 0) age = -age;
    if (age > _params->timeTestMaxAge){ 
      if (_params->debug){
	fprintf(stderr,"Age is greater than MaxAge\n");
      }
      return -1;
    }
  }
  
  //
  //
  // Set the times. Precip accum is over 15 minutes.
  //
  R->time = T->unix_time;
  //
  if (_params->useRealTime) R->time = time(NULL);
  //
  R->accum_start_time = R->time - 15*60;
  //
  // Then the ID string.
  //
  p = strtok(NULL,",");  
  if (p == NULL){
    if (_params->debug){
      fprintf(stderr,"Missing ID string.\n");
    }
    return -1;
  }
  

  if (_params->hashCodesDirectly){
    *stationID = Spdb::hash4CharsToInt32( p );
  } else {

    if (1 != sscanf(p, _params->IDstringFormat, stationID)){
      if (_params->debug){
	fprintf(stderr,"Problem with ID string.\n");
      }
      return -1;
    }

    if (_params->useLabelsAsDataType){
      char label[16];
      sprintf(label, _params->labelFormat, *stationID);
      *stationID = Spdb::hash4CharsToInt32( label );
      sprintf(R->station_label, "%s", label);
    }
  }
  //
  // Lat, lon and elevation. Return -1 if it is outside of range.
  //  
  p = strtok(NULL,",");  
  if (p == NULL) {
    if (_params->debug){
      fprintf(stderr,"Missing Latitude.\n");
    }
    return -1;
  }
  
  if (_params->scaledSamsFormat) 
    R->lat = atof(p)/10000.0;
  else
    R->lat = atof(p);

  p = strtok(NULL,",");
  if (p == NULL) {
    if (_params->debug){
      fprintf(stderr,"Missing Longitude.\n");
    }
    return -1;
  }
  
  if (_params->scaledSamsFormat) 
    R->lon = atof(p)/10000.0;
  else
    R->lon = atof(p);  


  if (_params->limitRange){
    if (
	(R->lat < _params->region.minLat) ||
	(R->lat > _params->region.maxLat) ||
	(R->lon < _params->region.minLon) ||
	(R->lon > _params->region.maxLon)
	){
      if (_params->debug){
	fprintf(stderr,"Lat/Lon out of range.\n");
      }
      return -1;
    }
  }

  p = strtok(NULL,",");
  if (p == NULL) {
    if (_params->debug){
      fprintf(stderr,"Missing Altitude.\n");
    }
    return -1;
  }

  if (_params->scaledSamsFormat) 
    R->alt = atof(p)/10.0;
  else
    R->alt = atof(p);

  if (_inputFormat == _metarFormat){
    //
    // In this case the visibility in Km is next.
    //
    p = strtok(NULL,",");
    if (p == NULL) {
      if (_params->debug){
	fprintf(stderr,"Missing Visibility.\n");
      }
      return -1;
    }

    if (_params->scaledSamsFormat) 
      R->visibility = atof(p)/10.0;
    else
      R->visibility = atof(p);

    if ((R->visibility < 0.0) || (R->visibility > 300.0)){
      R->visibility = STATION_NAN;
    }
  }


  //
  // Pressure, temp and dew point.
  //
  p = strtok(NULL,",");
  if (p == NULL) {
    if (_params->debug){
      fprintf(stderr,"Missing Pressure.\n");
    }
    return -1;
  }
  
  if (_params->scaledSamsFormat) 
    R->pres = atof(p)/10.0;
  else
    R->pres = atof(p);

  if (_inputFormat == _metarFormat){
    //
    // In skip altm variable
    //
    p = strtok(NULL,",");
    if (p == NULL) {
      if (_params->debug){
	fprintf(stderr,"Missing altm.\n");
      }
      return -1;
    }
  }

  p = strtok(NULL,",");
  if (p == NULL) {
    if (_params->debug){
      fprintf(stderr,"Missing Temperature.\n");
    }
    return -1;
  }
  
  if (_params->scaledSamsFormat) 
    R->temp = atof(p)/10.0;
  else
    R->temp = atof(p);

  p = strtok(NULL,",");
  if (p == NULL) {
    if (_params->debug){
      fprintf(stderr,"Missing dewpoint.\n");
    }
    return -1;
  }

  if (_params->scaledSamsFormat) 
    R->dew_point = atof(p)/10.0;
  else
    R->dew_point = atof(p);

  //
  // Wind directon, speed and gust.
  //
  p = strtok(NULL,",");
  if (p == NULL) {
    if (_params->debug){
      fprintf(stderr,"Missing wind direction.\n");
    }
    return -1;
  }

  if (_params->scaledSamsFormat) 
    R->winddir = atof(p)/10.0;
  else
    R->winddir = atof(p);

  p = strtok(NULL,",");
  if (p == NULL) {
    if (_params->debug){
      fprintf(stderr,"Missing wind speed.\n");
    }
    return -1;
  }

  if (_params->scaledSamsFormat) 
    R->windspd = atof(p)/10.0;
  else
    R->windspd = atof(p);

  p = strtok(NULL,",");
  if (p == NULL) {
    if (_params->debug){
      fprintf(stderr,"Missing wind gust.\n");
    }
    return -1;
  }

  if (_params->scaledSamsFormat) 
    R->windgust = atof(p)/10.0;
  else
    R->windgust = atof(p);

  if (_inputFormat == _samsFormat){
    //
    // Skip wstd, srad
    //
    p = strtok(NULL,",");
    if (p == NULL) {
      if (_params->debug){
	fprintf(stderr,"Missing wstd.\n");
      }
      return -1;
    }

    p = strtok(NULL,",");
    if (p == NULL) {
      if (_params->debug){
	fprintf(stderr,"Missing srad.\n");
      }
      return -1;
    }

    //
    // 15 minute precip.
    //
    p = strtok(NULL,",");
    if (p == NULL) {
      if (_params->debug){
	fprintf(stderr,"Missing 15 minute precip.\n");
      }
      return -1;
    }
    
    if (_params->scaledSamsFormat) 
      R->liquid_accum = atof(p)/10.0;
    else
      R->liquid_accum = atof(p);

    R->accum_start_time = R->time - 15*60;
    
  }

  if (_inputFormat == _metarFormat){
    //
    // Skip current precip
    //
    p = strtok(NULL,",");
    if (p == NULL) {
      if (_params->debug){
	fprintf(stderr,"Missing precip.\n");
      }
      return -1;
    }

    //
    // 24 hour precip
    //
    p = strtok(NULL,",");
    if (p == NULL) {
      if (_params->debug){
	fprintf(stderr,"Missing 24 hour precip.\n");
      }
      return -1;
    }


    if (_params->scaledSamsFormat) 
      R->liquid_accum = atof(p)/10.0;
    else
      R->liquid_accum = atof(p);

    R->accum_start_time = R->time - 24*3600;

  }



      //
      // Do some rudimentary quality control.
      //
  if (
      (R->lat < -90.0) ||
      (R->lat > 90.0) ||
      (R->lon < -360.0) ||
      (R->lon > 360.0) ||
      (R->alt > 10000.0) ||
      (R->alt < -1000.0)
      ){
    //
    // Skip it.
    //
    if (_params->debug){
      fprintf(stderr,"Bad lat, lon, or alt.\n");
    }
    return -1;
  }

  if ((R->pres < 500.0) || (R->pres > 1500.0)) R->pres = STATION_NAN;
  if ((R->winddir < 0.0) || (R->winddir > 360.0)) R->winddir = STATION_NAN;
  if ((R->windspd < 0.0) || (R->windspd > 500.0)) R->windspd = STATION_NAN;
  if ((R->windgust < 0.0) || (R->windgust > 500.0)) R->windgust = STATION_NAN;
  if ((R->temp < -50.0) || (R->temp > 60.0)) R->temp = STATION_NAN;
  if ((R->dew_point < -50.0) || (R->dew_point > 60.0)) R->dew_point = STATION_NAN;
  if ((R->liquid_accum < 0.0) || (R->liquid_accum > 500.0)) R->liquid_accum = STATION_NAN;

  //
  // Calculate the dew point.
  //
  if ((R->temp == STATION_NAN) || (R->dew_point == STATION_NAN)){
    R->relhum = STATION_NAN;
  } else {
    R->relhum  = PHYrelh(R->temp, R->dew_point); 
  }

  //
  // For SAMS station, pressure is station pressure, turn into SLP
  //
  if (_inputFormat == _samsFormat){

    double heightKm = R->alt / 1000.0;
    double factor = exp(-0.119*heightKm - 0.0013*heightKm*heightKm);

    if (R->pres != STATION_NAN){
      R->pres = R->pres / factor;
    }

  }

  return 0;

}
