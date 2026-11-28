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

#include <stdio.h>
#include <string.h>
#include <toolsa/umisc.h>
#include <rapformats/station_reports.h>
#include <Spdb/DsSpdb.hh>
#include <rapmath/math_macros.h>
#include <physics/physics.h>
#include <physics/thermo.h>


int  decode_wind_line( char *Line, char *stationID, 
		       time_t dataTime, double lat, double lon,
		       double alt);
int  decode_pressure_line( char *Line, char *stationID, 
			   time_t dataTime, double lat, double lon,
			   double alt);
void getExisting(time_t dataTime, char *stationID, 
		 station_report_t *S, double lat, double lon, double alt);
void saveNew(station_report_t S, char *stationID);
void get_lat_lon(char *stationID, double *lat, double *lon, double *alt);
double decodeStr(char *Line, int start, int len);

int main(int argc, char *argv[]){

  char fileName[1024];

  if (argc < 2){
    fprintf(stdout,"Input file name -->");
    fscanf(stdin,"%s",fileName);
  } else {
    sprintf(fileName,"%s",argv[1]);
  }

  //
  // Parse the station ID and the date from the filename.
  //
  char *p = fileName + strlen(fileName) - strlen("WLD.ihop.20020511.00");

  char stationID[8];

  stationID[0] = 'K';
  stationID[1] = p[0];
  stationID[2] = p[1];
  stationID[3] = p[2];
  stationID[4] = char(0);

  p = fileName + strlen(fileName) - strlen("20020511.00");
  int year, month, day;
  if (3 != sscanf(p,"%4d%2d%2d", &year, &month, &day)){
    fprintf(stderr,"Cannot parse date from filename %s\n",fileName);
    exit(-1);
  }
 

  FILE *fp = fopen(fileName,"rt");
  if (fp == NULL){
    fprintf(stderr,"Cannot open %s\n", fileName);
    exit(-1);
  }

  fprintf(stderr,"Looking at station %s on %d/%02d/%02d\n",
	  stationID, year, month, day);

  double lat, lon, alt;
  get_lat_lon(stationID, &lat, &lon, &alt);
  fprintf(stderr,"Station located at %g, %g (%g)\n", 
	  lat, lon, alt);

  if (year != 2002){
    fprintf(stderr,"Not IHOP data.\n");
    exit(-1);
  }

  const int MODE_UNKNOWN  = 0;
  const int MODE_WIND     = 1;
  const int MODE_PRESSURE = 2;

  int Mode = MODE_UNKNOWN;

  const int LineSize = 1024;
  char Line[LineSize];


  int first = 1;
  date_time_t startTime, endTime, dataTime;
  while (NULL != fgets(Line,LineSize,fp)){
    //
    // See if we can parse the time out of the line.
    //
    if (strlen(Line) >= strlen("LISTING 12HR PAGE #1 FROM: 05/12/2002 16:59 THRU 05/13/2002 04:59")){
      char *p = strstr(Line, "FROM:");
      if (p != NULL){
	if (10 == sscanf(p,
			 "FROM: %d/%d/%d %d:%d THRU %d/%d/%d %d:%d",
			 &startTime.month, &startTime.day, &startTime.year,
			 &startTime.hour, &startTime.min,
			 &endTime.month,  &endTime.day, &endTime.year,
			 &endTime.hour, &endTime.min)){
	  endTime.sec = 0; startTime.sec = 0;
	  first = 1;
	  uconvert_to_utime( &startTime );  uconvert_to_utime( &endTime );
	  dataTime = startTime;
	}
      }
    }

    //
    // Reset to an unkown mode at the start of a new listing.
    //
    if (NULL != strstr(Line, "LISTING")){
      Mode = MODE_UNKNOWN;
    }

    if (!strncmp(Line,
		 "     UTC  VIS1  D/N1  VIS2  D/N2  VIS3  D/N3  WIND DIR/SPD",
		 strlen("     UTC  VIS1  D/N1  VIS2  D/N2  VIS3  D/N3  WIND DIR/SPD"))){
      Mode = MODE_WIND;
      fprintf(stderr,"%s","Wind section encountered.\n");
      first = 1;
      continue;
    }

    if (!strncmp(Line,
		 " UTC  WX    TS   PRECIP SUN   SNOW   ZR     PRESS1  PRESS2  PRESS3  TEMP DEWPT",
		 strlen(" UTC  WX    TS   PRECIP SUN   SNOW   ZR     PRESS1  PRESS2  PRESS3  TEMP DEWPT"))){
      Mode = MODE_PRESSURE;
      fprintf(stderr,"%s","Pressure section encountered.\n");
      first = 1;
      continue;
    }
    //
    // Decide what time it is, if we are not in unknown time mode.
    //
    bool gotTime = false;
    int hour, min;
    if (Mode != MODE_UNKNOWN){

      if (2==sscanf(Line,"%2d%2d",&hour, &min)){
	if (
	    (hour < 24) &&
	    (hour > -1) &&
	    (min < 60) &&
	    (min > -1)
	    ){
	  gotTime = true;
	}
      }
      if (!gotTime) continue; // Skip the line.
    }
    //
    // Decode the time. Cope with day wraps.
    //
    time_t lastTime = dataTime.unix_time;
    dataTime.hour = hour;    dataTime.min = min;
    uconvert_to_utime( &dataTime );
    //
    // Cope with day wrappings.
    //

    // fprintf(stderr,"%s",Line);

    if ((!first) && (Mode != MODE_UNKNOWN)){
      if (dataTime.unix_time < lastTime){
	//
	// Advance by one day.
	//
	dataTime.unix_time = dataTime.unix_time + 86400;
	uconvert_from_utime (&dataTime);
	//
	// Check to see that we are still in bounds.
	//
	if (
	    (dataTime.unix_time < startTime.unix_time) ||
	    (dataTime.unix_time > endTime.unix_time)
	    ){
	  fprintf(stderr,"Timing error :\n%s not in %s to %s\n",
		  utimstr( dataTime.unix_time ),
		  utimstr( startTime.unix_time ),
		  utimstr( endTime.unix_time ));
	  exit(-1);
	}
      }
    }
    //
    if (first) first = 0;
    // fprintf(stderr,"%s\n",utimstr(dataTime.unix_time));


    //
    // Decode the line and write it out, as appropriate for the line type.
    //
    switch (Mode){

    default :
      //
      // Unknown mode, do noting.
      //
      break;

    case MODE_WIND :
      if (decode_wind_line( Line, stationID, dataTime.unix_time,
			    lat, lon, alt)){
	Mode = MODE_UNKNOWN;
      }
      break;

    case MODE_PRESSURE :
      if (decode_pressure_line( Line, stationID, dataTime.unix_time,
				lat, lon, alt)){
	Mode = MODE_UNKNOWN;
      }
      break;

    }

    if (Mode != MODE_UNKNOWN){
      if (dataTime.unix_time == endTime.unix_time){
	Mode = MODE_UNKNOWN;
	fprintf(stderr,"End of section reached.\n\n");
	first = 1;
      }
    }

  }

  fclose(fp);

  return 0;

}

/////////////////////////////////////////////////////////////////////
//
//
//
int  decode_wind_line( char *Line, char *stationID, 
		       time_t dataTime, double lat, double lon,
		       double alt){

  station_report_t S;
  getExisting(dataTime, stationID, &S, lat, lon, alt);

  //
  // Some unit conversion is probably needed here.
  //
  
  /* This is actually the extinction co-efficient - not usable.
  double vis = decodeStr(Line,
			 strlen("     UTC  "),
			 strlen(" 0.242 "));
  if (vis > -999.0) S.visibility = vis;
  */

  double windDir = decodeStr(Line,
			     strlen("     UTC  VIS1  D/N1  VIS2  D/N2  VIS3  D/N3  "),
			     strlen(" 154 "));
  if (windDir > -999.0) S.winddir = windDir;

  double windSpd = decodeStr(Line,
			     strlen("     UTC  VIS1  D/N1  VIS2  D/N2  VIS3  D/N3  WIND DIR"),
			     strlen(" 15 "));
  if (windSpd > -999.0) S.windspd = windSpd * 0.5145; // Convert from knots to m/s

  double gust = decodeStr(Line,
			     strlen("     UTC  VIS1  D/N1  VIS2  D/N2  VIS3  D/N3  WIND DIR/SPD 5SEC "),
			     strlen(" 15 "));
  if (gust > -999.0) S.windgust = gust * 0.5145; // Convert from knots to m/s

  saveNew(S, stationID);

  /*
  fprintf(stderr,"Wind line at %s Decoded : %s", 
	  utimstr( dataTime ), Line);
	  */
  return 0;

}

/////////////////////////////////////////////////////////////////////
//
//
//
int  decode_pressure_line( char *Line, char *stationID, 
			   time_t dataTime, double lat, double lon,
			   double alt){


  station_report_t S;
  getExisting(dataTime, stationID, &S, lat, lon, alt);

  //
  // Some unit conversion is probably needed here.
  //

  double precip = decodeStr(Line,
			    strlen(" UTC  WX    TS   "),
			    strlen("PRECIP"));
  if (precip > -999.0){
    S.liquid_accum = precip / 0.039384; // Convert from inches to mm.
    S.accum_start_time = dataTime - 60; // These are 1 minute accumulations.
  }


  double temp = decodeStr(Line,
			  strlen(" UTC  WX    TS   PRECIP SUN   SNOW   ZR     PRESS1  PRESS2  PRESS3  "),
			  strlen("TEMP "));
  if (temp > -999.0) S.temp = TEMP_F_TO_C(temp);

  double dewpt = decodeStr(Line,
			   strlen(" UTC  WX    TS   PRECIP SUN   SNOW   ZR     PRESS1  PRESS2  PRESS3  TEMP "),
			   strlen("DEWPT"));
  if (dewpt > -999.0) S.dew_point = TEMP_F_TO_C(dewpt);
  
  if (
      (S.dew_point != STATION_NAN) &&
      (S.temp != STATION_NAN)
      ){
    S.relhum = PHYrelh(S.temp, S.dew_point);
  }
 
  double total = 0;  int num = 0;
  double p1 = decodeStr(Line,
			strlen(" UTC  WX    TS   PRECIP SUN   SNOW   ZR     "),
			strlen("28.713"));
  if (p1 > -999.0) {
    num ++; total = total + p1;
  }

  double p2 = decodeStr(Line,
			strlen(" UTC  WX    TS   PRECIP SUN   SNOW   ZR     PRESS1  "),
			strlen("28.713"));
  if (p2 > -999.0) {
    num ++; total = total + p2;
  }

  double p3 = decodeStr(Line,
			strlen(" UTC  WX    TS   PRECIP SUN   SNOW   ZR     PRESS1  PRESS2  "),
			strlen("28.713"));
  if (p3 > -999.0) {
    num ++; total = total + p3;
  }

  if ((num > 0) && (S.temp != STATION_NAN)){
    S.pres = 33.86*total/double(num);
    //
    // Correct to sea level.
    //
    S.pres = PHYmslTempCorrectedP(alt, S.temp, S.pres); 
  }

  saveNew(S, stationID);
  /*
  fprintf(stderr,"Pressure line at %s Decoded : %s", 
	  utimstr( dataTime ), Line);
	  */

  return 0;
   

}

////////////////////////////////////////////////////////////////////
//
//
//

void getExisting(time_t dataTime, char *stationID, station_report_t *S,
		 double lat, double lon, double alt){
  //
  // See if there is an data at this place, time.
  //
  DsSpdb existingData;
  int dataType = Spdb::hash4CharsToInt32( stationID );
  existingData.getExact("./out",
			dataTime,
			dataType,
			0);
  if (existingData.getNChunks() == 0){
    //
    // No data, return nothing but the time and location.
    // Set everything else to 0 or STATION_NAN.
    //
    memset(S, 0, sizeof(station_report_t));
    //
    S->time = dataTime;
    S->lat = lat;
    S->lon = lon;
    S->alt = alt;
    //
    S->temp = STATION_NAN;
    S->dew_point = STATION_NAN;
    S->relhum = STATION_NAN;
    S->windspd = STATION_NAN;
    S->winddir = STATION_NAN;
    S->windgust = STATION_NAN;
    S->pres = STATION_NAN;
    S->liquid_accum = STATION_NAN;
    S->precip_rate = STATION_NAN;
    S->visibility = STATION_NAN;
    S->rvr = STATION_NAN;
    S->ceiling = STATION_NAN;

    return;
  }
  //
  // There are existing data, decode them into S.
  //
  station_report_t *report;
  report = (station_report_t *) existingData.getChunks()[0].data;
  station_report_from_be( report );

  memcpy(S, report, sizeof(station_report_t));

  return;
  

}

///////////////////////////////////////////////////////////
//
//
//
//

void saveNew(station_report_t S, char *stationID){

  time_t dataTime = S.time;
  int dataType = Spdb::hash4CharsToInt32( stationID );

  station_report_to_be( &S );

  DsSpdb Out;

  if (Out.put("./out",
	      SPDB_STATION_REPORT_ID,
	      SPDB_STATION_REPORT_LABEL, 
	      dataType,
	      dataTime,
	      dataTime + 1800,
	      sizeof(station_report_t),
	      &S)){
    fprintf(stderr,"Error on saving data out.\n");
    exit(-1);
  }



  return;

}
///////////////////////////////////////////////////////////
//
//
//
//
void get_lat_lon(char *stationID, double *lat, double *lon, double *alt){

  FILE *ffp = fopen("station.dat","rt");
  if (ffp == NULL){
    fprintf(stderr,"Cannot find station.dat\n");
    exit(-1);
  }

  char Line[1024];

  while (NULL != fgets(Line,1024,ffp)){

    if (!(strncmp(Line,stationID,4))){
      if (3 == sscanf(Line+5,"%lf,%lf,%lf",lat,lon,alt)){
	fclose(ffp);
	return;
      }
    }

  }

  fclose(ffp);
  fprintf(stderr,"Could not locate station %s\n",stationID);
  exit(-1);

}


///////////////////////////////////////////////////////////////
//
//
//
double decodeStr(char *Line, int start, int len){
  //
  // Make a copy of what we have to decode.
  //
  char dataStr[256];
  for (int i=0; i < len; i++){
    dataStr[i]=Line[i+start];
    dataStr[i+1]=char(0);
    if (Line[i] == char(0)) break;
  }

  //
  // Make sure we have at least one digit in the string.
  //
  bool gotDigit = false;
  for (int i=0; i < (int) strlen(dataStr); i++){
    if (isdigit( int(dataStr[i]))){
      gotDigit = true;
      break;
    }
  }

  if (!gotDigit) return -9999.0; // The bad value.

  return atof( dataStr );

}
