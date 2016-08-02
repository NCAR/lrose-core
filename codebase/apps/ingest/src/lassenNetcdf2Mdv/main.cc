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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include "Params.hh"
#include <toolsa/umisc.h>
#include "lassenNetcdf2Mdv.hh"


int doy(date_time_t T){

  // Figure out unit time of start of year.
  date_time_t S;
  S.year = T.year; S.month = 1; S.day = 1; S.hour = 0; S.min = 0; S.sec = 0;
  uconvert_to_utime( &S );

  return 1 + (T.unix_time - S.unix_time)/86400;

}



void construeFileName(char *dir, char location, char trailing,
		      char field, date_time_t time,
		      char *filename){

  int dayOfYear = doy(time);

  sprintf(filename, dir, time.year % 100, dayOfYear, location, trailing, 
	  time.year % 100, dayOfYear, time.hour, time.min, field);

  //  fprintf(stderr, "Searching for %s\n", filename);

  return;

}


int searchForFile(char *dir,      // Input - directory to search.
		  char locChar,   // Input - location character in filename
		  char fieldChar, // Input - field char in filename
		  char trailingChar, // Input - trailing char in filename
		  date_time_t *T, // Input - time to start looking from
		  int timeSearch, // Input - seconds to look ahead for next file
		  char *Filename // Output - found filename, if any
		  ){

  for (int isearch = 0; isearch < timeSearch+1; isearch+=60){ // Advance by 60 secs as filenames have no seconds

    date_time_t S;
    S.unix_time = T->unix_time + isearch;
    uconvert_from_utime( &S );

    construeFileName(dir, locChar, fieldChar, trailingChar, S, Filename);

    struct stat buf;
    if (0==stat(Filename, &buf)){
      // Found a file matching the spec. Advance the time and return.
      *T = S;
      return 0;
    }
  }

  return -1; // Failed.

}


int main(int argc, char *argv[]){


  Params P;
  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }

  //
  // Read the state, if we can and it exists.
  //
  char startFileToUse[1024];
  sprintf(startFileToUse, "%s", P.startFile);

  if (P.saveState.readStateAtStart){
    FILE *ifp = fopen(P.saveState.stateFilename, "r");
    if (ifp != NULL){ // State file found.
      char str[1024];
      fscanf(ifp, "%s", str);
      if (strlen(str)>= strlen("AZ_070010000_B.nc")){
	sprintf(startFileToUse, str + strlen(str) - strlen("AZ_070010000_B.nc"));
      }
      fclose(ifp);
	}
  }

  if (P.debug){
    fprintf(stderr,"Starting with filename %s\n", startFileToUse);
  }


  int startYear, startDay, startHour, startMin;

  if (4 != sscanf(startFileToUse+3, "%2d%3d%2d%2d",
		  &startYear, &startDay, &startHour, &startMin)){
    fprintf(stderr,"ERROR : Cannot parse start time from %s\n",
	    startFileToUse);
    exit(-1);
  }


  date_time_t T;
  T.year = startYear + 2000; T.month = 1; T.day = 1;
  T.hour = startHour; T.min = startMin; T.sec = 0;
  uconvert_to_utime( &T );
  T.unix_time += (startDay-1)*86400;
  uconvert_from_utime( &T );

  char locChar = startFileToUse[0];
  char fieldChar = startFileToUse[1];
  char trailChar = startFileToUse[13];

  if (P.debug)
    fprintf(stderr, "Start time is %s\n", utimstr(T.unix_time));

  //
  // Initialize the writer.
  //
  lassenNetcdf2Mdv L( &P );

  date_time_t stopTime;
  stopTime.year = P.stopTime.year;  stopTime.month = P.stopTime.month;  stopTime.day = P.stopTime.day;
  stopTime.hour = P.stopTime.hour;  stopTime.min = P.stopTime.min;  stopTime.sec = P.stopTime.sec;
 	uconvert_to_utime( &stopTime );

  do {

    if (P.stopTime.stopAfterThisTime){
      if (T.unix_time > stopTime.unix_time){
        fprintf(stderr,"Stop time exceeded, exiting.\n");
        if (P.stopTime.runStopScript){
          system(P.stopTime.stopScript);
          exit(0);
        }
      }
    }

    
    char Filename[1024];
    if (searchForFile(P.dir, locChar, fieldChar, trailChar, &T, P.searchTime, Filename)){
      //
      // Failed to find file, exit loop
      //
      break;
    } else {
      //
      // Success, have a file, process it.
      //
      if (P.debug) fprintf(stderr,"Processing file %s\n", Filename);
      L.processFile( Filename, T.unix_time);

      // Save that filename, if desired.
      if (P.saveState.doSaveState){
	FILE *fp = fopen(P.saveState.stateFilename, "w");
	if (fp == NULL){
	  fprintf(stderr,"Failed to create state file %s\n", P.saveState.stateFilename);
	  exit(-1);
	}

	fprintf(fp, "%s", Filename);

	fclose(fp);
      }

      T.unix_time += 60;
      uconvert_from_utime ( &T );
    }

  } while (1); // As long as we're finding files.

  
  return 0;

}
