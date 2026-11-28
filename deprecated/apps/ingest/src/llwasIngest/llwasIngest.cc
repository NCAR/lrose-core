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

//
// File scope.
//
void  ReadLocFile(char *LocFile, int *NumStations,
	      int **StationNums, 
		  float **StationLats, 
		  float **StationLons, 
		  float **StationAlts,
		  float *MagDec);


int main(int argc, char *argv[]){


  //
  // Get the name of the station location file.
  //
  char LocFile[ 1024 ];
  if (argc < 2) {
    fprintf( stdout, "Station location filename --->");
    fscanf( stdin, "%s", LocFile);
  } else {
    sprintf(LocFile,"%s",argv[1] );
  }
  //
  // And read it.
  //
  int NumStations;
  int *StationNums;
  float *StationLats;
  float *StationLons;
  float *StationAlts;
  float MagDec;
  //
  // Read the station location information.
  //
  ReadLocFile(LocFile, &NumStations,
	      &StationNums, &StationLats, &StationLons, &StationAlts,
	      &MagDec);
  //
  // Print it out.
  //
  for (int i=0; i< NumStations; i++){
    fprintf(stdout,"%d\t%d\t(%g, %g)\t%g\n",
	    i,StationNums[i], StationLats[i], StationLons[i], StationAlts[i] );
  }
  //
  // Get the name of the actual data file.
  //
  char DataFile[ 1024 ];
  if (argc < 3) {
    fprintf( stdout, "Actual data filename --->");
    fscanf( stdin, "%s", DataFile);
  } else {
    sprintf(DataFile,"%s",argv[2] );
  }
  //
  // Get the name of the output DsSpdb location.
  //
  char OutputUrl[ 1024 ];
  if (argc < 3) {
    fprintf( stdout, "Output DsSpdb URL --->");
    fscanf( stdin, "%s", OutputUrl);
  } else {
    sprintf(OutputUrl,"%s",argv[3] );
  }
  //
  // Read the file, which has this format :
  //
  //
  // -------------------------------------------
  // TimeStamp     : 06/27/2000-10:22:02
  // Station Count : 6
  //    0   U:    0.870  V:   -4.920
  //     1   U:    0.000  V:   -5.000
  //     2   U:   -2.050  V:   -5.640
  //     3   U:   -3.500  V:   -6.060
  //     4   U:   -2.390  V:   -6.580
  //     5   U:   -3.000  V:   -5.200     
  // -------------------------------------------
  // TimeStamp     : 06/27/2000-10:22:15   
  //
  // And write it out as Spdb.
  //
  FILE *ifp = fopen(DataFile,"rt");
  if (ifp == NULL){
    fprintf(stderr,"Failed to open %s\n", DataFile);
    exit(-1);
  }
  //
  // Before we go into the main file parsing loop,
  // set up the DsSpdb object.
  //
  DsSpdb OutSpdb;
  OutSpdb.clearPutChunks();
  OutSpdb.clearUrls();
  OutSpdb.addUrl(OutputUrl);


  station_report_t *ToBeAdded;
  ToBeAdded = (station_report_t *)malloc(NumStations * sizeof(station_report_t));
  
  int *dataTypes;
  dataTypes = (int *)malloc(NumStations * sizeof(int));

  long MetarsOut = 0;
  int StatCount;
  char Line[1024];
  while(NULL!=fgets(Line,1024, ifp)){      
    if (!(strncmp(Line, "TimeStamp", strlen("TimeStamp")))){
      //
      // We have a timestamp, decode it
      //
      date_time_t U;
      if (6!= sscanf(Line,"TimeStamp     : %d/%d/%d-%d:%d:%d",
		     &U.month, &U.day, &U.year,
		     &U.hour, &U.min, &U.sec)){
	fprintf(stderr,"Failed to decode time %s",Line);
	continue;
      }
      uconvert_to_utime( &U );

      if (MetarsOut % 25 == 0)
	fprintf(stdout,"Time : %s\n",utimstr(U.unix_time ));

      //
      // Then the station count.
      //
      if (
	  (NULL == fgets(Line,1024, ifp)) ||
	  (strncmp(Line,"Station Count",strlen("Station Count"))) ||
	  (1 != sscanf(Line, "Station Count : %d",&StatCount))
	  ){
	fprintf(stderr,"Failed to decode station count in %s",Line);
      }
      //
      // The read the entries, which just seem to be
      // U and V, into station reports, and add them to the
      // output.
      //

      int NumFound = 0;

      for(int k=0; k< StatCount; k++){
	if (NULL == fgets(Line,1024, ifp)){
	  fprintf(stderr,"Ran out of entries in mid-station.\n");
	  exit(-1);
	}
	int StatIndex;
	float u,v;
	if (3 != sscanf(Line,"%d  U: %f  V: %f",
			&StatIndex, &u, &v)){
	  fprintf(stderr,"Failed to decode %s",Line);
	  exit(-1);
	}

	//
	// As a quality control issue, do not accept entries
	// for which both u and v are 0.0
	//
	if (
	    (u == 0.0) &&
	    (v == 0.0)
	    ){
	  continue;
	}

	//
	// Convert from Knots to meters / sec
	//
	u=u*0.5145;
	v=v*0.5145;

	if (StatIndex >= NumStations){
	  fprintf(stderr, "Index %d out of range.\n",StatIndex);
	  exit(-1);
	}

	//
	// Set up a station_report_t
	//
	station_report_t S;
	S.time = U.unix_time;
	S.winddir = PHYwind_dir(u,v) + MagDec;
	if (S.winddir < 0.0)   S.winddir = S.winddir + 360.0;
	if (S.winddir > 360.0) S.winddir = S.winddir - 360.0;

	S.windspd = PHYwind_speed(u,v);

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

	S.lat = StationLats[StatIndex];
	S.lon = StationLons[StatIndex];
	S.alt = StationAlts[StatIndex];
	//
	// Use the data type you get by labelling the
	// station Lnnn where nnn is the 3 digit station number.
	//

	char StatID[5];
	sprintf(StatID,"L%03d",StatIndex+1);

	int dataType = Spdb::hash4CharsToInt32( StatID );

	ToBeAdded[NumFound] = S;
	dataTypes[NumFound] = dataType;
	NumFound++;
      }

      if (NumFound > 0.7*NumStations){ // Quality control check on writing data.
	for(int l=0; l < NumFound; l++){

	  time_t valid_t = ToBeAdded[l].time;
	  int dataT = dataTypes[l];

	  station_report_to_be( &ToBeAdded[l] );

	  OutSpdb.addPutChunk(dataT,
			      valid_t,
			      valid_t + 3600, // Hour valid time - hardcoded.
			      sizeof( station_report_t),
			      (void *) &ToBeAdded[l] );
	}

	MetarsOut = MetarsOut + NumFound;

      }

    }
  }

  fclose( ifp ); // End of loop through input data. Time to write output.


  fprintf(stdout,"Writing %ld metars to %s\n",MetarsOut,OutputUrl);

  OutSpdb.put(SPDB_STATION_REPORT_ID,
	      SPDB_STATION_REPORT_LABEL);
    

  //
  // Let go of the station location info.
  //
  free(StationNums); free(StationLons);
  free(StationAlts); free(StationLats);


  fprintf(stdout,"%ld metars written to %s\n",
	  MetarsOut, OutputUrl);

  free(ToBeAdded); free(dataTypes);

  return 0;


}


//////////////////////////////////////////////////////////////////
//
// Routine that reads the file that gives the locations of
// the station. File has typically the following format :
//
//
// # MAG Declination in degrees, needed to rotate LLWAS since LLWAS is
// # relative to Magnetic North
// MAG_DECLINATION 7.0
// #            
// STATION1 "201-CF"
// LAT1 32.897222
// LON1 97.021389
// ALT1 190.00000
// STATION2 "202-N"
// LAT2 32.918611
// LON2 97.026389
// ALT2 190.00000
// STATION3 "203-E"
// LAT3 32.870278
// LON3 97.026944
// ALT3 190.00000
//
//
//
void  ReadLocFile(char *LocFile, int *NumStations,
		  int **StationNums, 
		  float **StationLats, 
		  float **StationLons, 
		  float **StationAlts,
		  float *MagDec){

  FILE *fp = fopen(LocFile,"rt");
  if (fp == NULL){
    fprintf(stderr,"Failed to open %s\n", LocFile);
    exit(-1);
  }

  *NumStations = 0;
  int MagFound = 0;
  char Line[1024];
  while(NULL!=fgets(Line,1024, fp)){
    //
    // See if we can get the magnetic declination. This
    // only occurs once in the file.
    //
    if (!(strncmp(Line,"MAG_DECLINATION",strlen("MAG_DECLINATION")))){
      sscanf(Line,"MAG_DECLINATION %f", MagDec);
      MagFound = 1;
    }

    if (!(strncmp(Line,"STATION",strlen("STATION")))){
      *NumStations = *NumStations + 1;
    }

  }
  rewind (fp );

  if (!(MagFound)){
    fprintf(stderr,"Magnetic declination not found - I cannot cope.\n");
    exit(-1);
  }

  fprintf(stdout,"Magnetic declination is %g\n",*MagDec);
  fprintf(stdout,"%d stations found in %s.\n",*NumStations, LocFile);
  fprintf(stdout,"Reading file...\n");

  //
  // Allocate memory to read the file.
  //
  *StationNums = (int *)malloc(*NumStations * sizeof(int));
  *StationLats = (float *)malloc(*NumStations * sizeof(float));
  *StationLons = (float *)malloc(*NumStations * sizeof(float));
  *StationAlts = (float *)malloc(*NumStations * sizeof(float));
  if (
      (*StationNums == NULL) ||
      (*StationLats == NULL) ||
      (*StationLons == NULL) ||
      (*StationAlts == NULL)
      ){
    fprintf(stderr,"Malloc failed.\n");
    exit(-1);
  }
  //
  // And read in the locations.
  //
  int i=0; int dummy;
  while(NULL!=fgets(Line,1024, fp)){
    if (!(strncmp(Line,"STATION",strlen("STATION")))){

      //
      // Read the station number.
      //
      if (1!=sscanf(Line,"STATION%d",i+*StationNums)){
	fprintf(stderr,"Station number read error.\n");
	exit(-1);
      }
      //
      // Then the lat.
      //
      if (NULL == fgets(Line,1024, fp)){
	fprintf(stdout,"Ran out of station locations in mid file.\n");
	exit(-1);
      }
      if (2!=sscanf(Line,"LAT%d %f",&dummy, i+*StationLats)){
	fprintf(stderr,"Read error for lat\n");
	exit(-1);
      }
      //
      // And the lon.
      //      
      if (NULL == fgets(Line,1024, fp)){
	fprintf(stdout,"Ran out of station locations in mid file.\n");
	exit(-1);
      }
      if (2 != sscanf(Line,"LON%d %f",&dummy, i+*StationLons)){
	fprintf(stderr,"Read error for lon\n");
	exit(-1);
      }
      //
      // Finally the altitude.
      //
      if (NULL == fgets(Line,1024, fp)){
	fprintf(stdout,"Ran out of station locations in mid file.\n");
	exit(-1);
      }
      if (2 != sscanf(Line,"ALT%d %f",&dummy, i+*StationAlts)){
	fprintf(stderr,"Read error for Alts\n");
	exit(-1);
      }
      i++;
    }
  }
  fclose(fp);
  return;

}

