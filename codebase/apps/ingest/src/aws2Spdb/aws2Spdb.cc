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
#include <math.h>
#include <toolsa/pmu.h>
#include <physics/physics.h>

#include <rapformats/station_reports.h>
#include <Spdb/DsSpdb.hh>


#include "aws2Spdb.hh"

//
// Constructor. Copies parameters.
//
aws2Spdb::aws2Spdb(Params *TDRP_params){
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
aws2Spdb::~aws2Spdb(){
  return;
}
//
// Main routine.
//
void aws2Spdb::aws2SpdbFile( char *filename ){
  //
  // Initialize variables.
  //

  if (_params->debug){
    time_t now = time(NULL);
    fprintf(stderr,"Processing file %s at %s\n", 
	    filename, utimstr(now));
  }

  //
  // Parse the time out of the filename.
  //

  if (strlen(filename) < strlen("AWS_200807200700.dat")){
    fprintf(stderr,"Filename %s too short for date format.\n", filename);
    return;
  }

  char *dateStart = filename + strlen(filename) - strlen("200807200700.dat");

  date_time_t T;
  T.sec = 0;
 
  if (5 != sscanf(dateStart,"%4d%2d%2d%2d%2d",
		  &T.year, &T.month, &T.day,
		  &T.hour, &T.min)){
    fprintf(stderr,"Filename %s not in date format.\n", filename);
    return;
  }

  uconvert_to_utime( &T );

  FILE *fp = fopen(filename,"r");
  if (fp == NULL){
    fprintf(stderr, "%s not found.\n", filename);
    return;
  }

  const int lineLen = 1024;
  char Line[lineLen];

  station_report_t R;


  int numRead = 0;
  while (NULL!=fgets(Line, lineLen, fp)){
    numRead++;
    //
    // Zero out this memory.
    //
    memset(&R, 0, sizeof(R));
    //
    // Set fields not available to bad.
    //
    R.precip_rate = STATION_NAN;
    R.visibility = STATION_NAN;
    R.rvr = STATION_NAN;
    R.ceiling = STATION_NAN;
    //
    float dummy;
    int stationID;
    bool readOK = false;
    //
    // See if we can read the line.
    //
    int year, month, day, hour;
    if (15 == sscanf(Line,
		     "%d %4d%2d%2d%2d %f %f %f %f %f %f %f %f %f %f",
		     &stationID,
		     &year, &month, &day, &hour,
		     &R.lat,
		     &R.lon,
		     &R.alt,
		     &R.winddir,
		     &R.windspd,
		     &R.temp,
		     &R.relhum,
		     &dummy,
		     &R.pres,
		     &R.liquid_accum)){
      readOK = true;
    }

    if (readOK){
      //
      // Do some rudimentary QC.
      //
      if (
	  (R.lat < -90.0) ||
	  (R.lat > 90.0) ||
	  (R.lon < -360.0) ||
	  (R.lon > 360.0) ||
	  (R.alt > 10000.0) ||
	  (R.alt < -1000.0)
	  ){
	//
	// Skip it.
	//
	continue;
      }

      if ((R.pres < 500.0) || (R.pres > 1500.0)) R.pres = STATION_NAN;
      if ((R.winddir < 0.0) || (R.winddir > 360.0)) R.winddir = STATION_NAN;
      if ((R.windspd < 0.0) || (R.windspd > 500.0)) R.windspd = STATION_NAN;
      if ((R.temp < -50.0) || (R.temp > 60.0)) R.temp = STATION_NAN;
      if ((R.relhum < 0.0) || (R.relhum > 100.0)) R.relhum = STATION_NAN;

      if ((R.liquid_accum < 0.0) || (R.liquid_accum > 1000.0)){
	R.liquid_accum = STATION_NAN;
      } else {
	R.accum_start_time = T.unix_time - 3600;
      }



      //
      // Calculate the dew point.
      //
      if ((R.temp == STATION_NAN) || (R.relhum == STATION_NAN)){
	R.dew_point = STATION_NAN;
      } else {
	R.dew_point  = PHYrhdp(R.temp, R.relhum); 
      }
      //
      // Set the time.
      //
      R.time = T.unix_time;

      //
      // Save the point out.
      //
      DsSpdb OutSpdb;

      char id[5];
      sprintf(id,"K%03d", stationID);
      int dataType = Spdb::hash4CharsToInt32(id);
      sprintf(R.station_label, "%s", id);

      station_report_to_be( &R );

      if (OutSpdb.put(_params->outUrl,
		      SPDB_STATION_REPORT_ID,
		      SPDB_STATION_REPORT_LABEL,
		      dataType,
		      T.unix_time + _params->timeOffset,
		      T.unix_time + _params->timeOffset + _params->Expiry,
		      sizeof(station_report_t), &R, 0)){
	fprintf(stderr,"Write to %s failed.\n",
		_params->outUrl);
      }

    }

  }

  if (_params->debug){
    fprintf(stderr,"%d lines read for %s.\n", numRead, filename);
  }


  fclose( fp );

  return;

}
