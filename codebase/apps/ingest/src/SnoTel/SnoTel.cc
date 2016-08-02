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
#include <cstdlib>
#include <iostream>
#include <string.h>
#include <toolsa/umisc.h>
#include <Spdb/DsSpdb.hh>

#include <rapformats/station_reports.h>
//
// For stat.
//
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>        


#include "SnoTel.hh"
using namespace std;
//
// constructor - does everything.
//
SnoTel::SnoTel(char *Filename, Params *P){

  if (P->debug){
    cerr << "Processing file " << Filename << endl;
  }
  //
  // See if we can get the date from the filename in
  // YYYYMMDD.txt format (time take as midnight). If not,
  // use the input file name ctime as the data time.
  //
  int gotTime = 0;
  date_time_t T;
  if (strlen(Filename) >= strlen("YYYYMMDD.txt")){
    char *date = Filename + strlen(Filename) - strlen("YYYYMMDD.txt");
    if (P->debug){
      cerr << "Attempting to decode time from " << date << endl;
    }
    if (3==sscanf(date,"%4d%2d%2d.", &T.year, &T.month, &T.day)){
      T.hour = 0; T.min = 0; T.sec = 0;
      uconvert_to_utime( &T );
      gotTime = 1;
      if (P->debug){
	cerr << "Taking time from input file name : " << utimstr( T.unix_time ) << endl;
      }
    }
  }

  if (!(gotTime)){
    //
    // Stat the file to get data time.
    //
    struct stat buf;
    if (stat(Filename, &buf)) {
      cerr << "Could not stat " << Filename << endl;
      return;
    }
    T.unix_time = buf.st_ctime;
    uconvert_from_utime( &T );
    if (P->debug){
      cerr << "Taking time from input file ctime : " << utimstr( T.unix_time ) << endl;
    }
  }
  //
  // Open the input file.
  //
  FILE *fp = fopen(Filename,"rt");
  if (fp == NULL){
    cerr << "Could not find file " << Filename << endl;
    return;
  }
  

  const int LineLen = 1024;
  char Line[LineLen];

  DsSpdb OutSpdb;
  OutSpdb.clearPutChunks();
  OutSpdb.addUrl(P->OutUrl);   

  int numRead = 0;

  while(NULL!=fgets(Line,LineLen,fp)){

    if (P->debug){
      cerr << "Line : " << Line;
    }

    float elev;                           // Elevation in feet.
    float sweCurrent, sweAvg, swePercent; // Snow Water Equivalents
    float tpCurrent, tpAvg, tpPercent;    // Total Percent
    
    const int endOfLabel = 24; // First 24 characters of line may be label.

    if (7!=sscanf(Line +  endOfLabel,
		  "%f %f %f %f %f %f %f",
		  &elev, 
		  &sweCurrent, &sweAvg, &swePercent,
		  &tpCurrent, &tpAvg, &tpPercent)){

      if (P->debug){
	cerr << "Line not decoded." << endl;
      }
    } else {
      //
      // Pull out the station name and
      // parse the spaces out from the station name.
      //
      char loc[30];
      Line[endOfLabel]=(char)0;
      sprintf(loc,"%s",Line);
      //
      // Parse spaces off end.
      //
      char *p = loc + strlen(loc) - 1;
      do {
	if (*p == ' ') *p = char(0);
	p--;
      } while ((p != loc) && (*p == ' '));
      //
      // And the start.
      //
      char *cleanName = loc;
      do {
	if (*cleanName == ' ') cleanName++;
      } while (*cleanName == ' ');


      if (P->debug){
	cerr << "Decoded as : ";
	cerr << "|" << cleanName << "|" << endl;
	cerr << " Elevation " << elev << endl;
	cerr << " SWE current " << sweCurrent << endl;
	cerr << " SWE Average " << sweAvg << endl;
	cerr << " SWE percent " << swePercent << endl;
	
	cerr << " TP current " << tpCurrent << endl;
	cerr << " TP Average " << tpAvg << endl;
	cerr << " TP percent " << tpPercent << endl;

	cerr << endl;
      }

      float lat, lon, alt;
      char metarID[8];
      if (LocateSnoTelStation(P->SnoTelLocationFile,
			      cleanName, &lat, &lon, 
			      &alt, metarID)){
	cerr << "WARNING : Failed to locate station " << cleanName << endl;
      } else {
	if (P->debug){
	  cerr << "Station " << cleanName << " located at ";
	  cerr << lat << " " << lon << " " << alt << endl;
	  cerr << "METAR ID is " << metarID << endl;
	}
	//
	// Set up the station report.
	//
	station_report_t M; // The metar.
 
	M.time = T.unix_time;

	time_t dataTime = M.time;
 
	M.accum_start_time = M.time;
	M.weather_type = 0;
	M.lat = lat;
	M.lon = lon;
	M.alt = alt;

	M.temp =  STATION_NAN;                                     
	M.relhum =  STATION_NAN;
	M.pres = STATION_NAN; 
	M.windspd = STATION_NAN;  
	M.winddir = STATION_NAN;  
	M.dew_point = STATION_NAN;
	M.windgust =     STATION_NAN;
	M.precip_rate =  STATION_NAN;
	M.visibility =   STATION_NAN;
	M.rvr =          STATION_NAN;
	M.ceiling =      STATION_NAN;        
	
	if (P->percentOfAverage){   // Want values as percent of average
	  if (P->snowPrecip){       // Want snow precip only
	    M.liquid_accum = swePercent;
	  } else {                  // Want total precip
	    M.liquid_accum = tpPercent;
	  }
	} else {                    // Want actual values, not percentages.
	  if (P->snowPrecip){       // Want snow precip only
	    M.liquid_accum = sweCurrent;
	  } else {                  // Want total precip
	    M.liquid_accum = tpCurrent;
	  }
	}
	int dataType = Spdb::hash4CharsToInt32( metarID );   
	int dataType2 = 0;

	station_report_to_be( &M );
 
	// add output chunk to buffer
 
	OutSpdb.addPutChunk(dataType,
			    dataTime,
			    dataTime + P->expiry,
			    sizeof(M),
			    (void *) &M,
			    dataType2);
	numRead++;
      }
    }
  }
  
  if (OutSpdb.put(SPDB_STATION_REPORT_ID,
                  SPDB_STATION_REPORT_LABEL)) {
    cerr << "ERROR - SnoTel" << endl;
    cerr << "  Cannot put output to url: " << P->OutUrl << endl;
    return;         
  }

  fclose(fp);

  if (P->debug){
    cerr << numRead << " SnoTel stations read to " <<  P->OutUrl << endl;
  }

}
//
//
// destructor - does nothing.
//
SnoTel::~SnoTel (){
}

//////////////////////////////////////////////////////////
//
// Routine that locates SnoTel stations.
//
int SnoTel::LocateSnoTelStation(char *LocFile,
				char *StationName,
				float *lat, float *lon, float *alt,
				char *metarID){

  FILE *ifp = fopen(LocFile,"rt");
  if (ifp == NULL){
    cerr << "Unable to open SnoTel location file " << LocFile << endl;
    exit(-1);
  }
  //
  // Cycle through the file looking for the station.
  //
  const int LineLen = 1024;
  char Line[LineLen];

  while(NULL!=fgets(Line,LineLen,ifp)){

    if ( Line[0] != '#'){ // Skip comment lines.
      
      char  inMetarID[16];
      float inLat, inLon, inAlt;
      //
      // Parse the station name out from the curly braces.
      //
      char *startName = strstr(Line, "{");
      char *endName = strstr(Line, "}");
	
      if (
	  (startName != NULL) &&
	  (endName != NULL)
	  ){
	
	char *inName = startName + 1;
	char *restOfLine = endName + 1;
	*endName = char(0);
	//
	// Then try to decode the rest of the line.
	//
	if ( 4 == sscanf(restOfLine,"%f %f %f %4s",
			 &inLat, &inLon, &inAlt, inMetarID)){
	  	  
	  if (!(strcmp(StationName,inName))){
	    *lat = inLat;
	    *lon = inLon;
	    *alt = inAlt;
	    sprintf(metarID,"%s",inMetarID);
	    fclose(ifp);
	    return 0;
	  }

	}
	
      }
	
    }

  }

  fclose(ifp);
  return 1;

}

