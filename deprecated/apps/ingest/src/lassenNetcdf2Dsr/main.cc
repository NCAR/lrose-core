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
#include "lassenNetcdf2Dsr.hh"

int tiltNumFromTiltChar(char tiltChar){

  if (tiltChar == '1') return 0;
  if (tiltChar == '2') return 1;
  if (tiltChar == '3') return 2;
  if (tiltChar == '4') return 3;
  if (tiltChar == '5') return 4;

  if (tiltChar == '6') return 5;
  if (tiltChar == '7') return 6;
  if (tiltChar == '8') return 7;
  if (tiltChar == '9') return 8;
  if (tiltChar == 'A') return 9;

  if (tiltChar == 'B') return 10;
  if (tiltChar == 'C') return 11;
  if (tiltChar == 'D') return 12;
  if (tiltChar == 'E') return 13;
  if (tiltChar == 'F') return 14;

  if (tiltChar == 'G') return 15;
  if (tiltChar == 'H') return 16;
  if (tiltChar == 'I') return 17;
  if (tiltChar == 'J') return 18;
  if (tiltChar == 'K') return 19;

  return -1; // Failed.

}


char tiltCharFromTiltNum(int tiltNum){

  switch (tiltNum){

  case 0 :
    return '1';
    break;

  case 1 :
    return '2';
    break;

  case 2 :
    return '3';
    break;

  case 3 :
    return '4';
    break;

  case 4 :
    return '5';
    break;

  case 5 :
    return '6';
    break;

  case 6 :
    return '7';
    break;

  case 7 :
    return '8';
    break;

  case 8 :
    return '9';
    break;

  case 9 :
    return 'A';
    break;

  case 10 :
    return 'B';
    break;

  case 11 :
    return 'C';
    break;

  case 12 :
    return 'D';
    break;

  case 13 :
    return 'E';
    break;

  case 14 :
    return 'F';
    break;

  case 15 :
    return 'G';
    break;

  case 16 :
    return 'H';
    break;

  case 17 :
    return 'I';
    break;

  case 18 :
    return 'J';
    break;

  case 19 :
    return 'K';
    break;

  }

  return '-'; // Failed.
}

int doy(date_time_t T){

  // Figure out unit time of start of year.
  date_time_t S;
  S.year = T.year; S.month = 1; S.day = 1; S.hour = 0; S.min = 0; S.sec = 0;
  uconvert_to_utime( &S );

  return 1 + (T.unix_time - S.unix_time)/86400;

}



void construeFileName(char *dir, char location, char field, date_time_t time, int tiltNum,
		      char *filename){

  int dayOfYear = doy(time);
  char tiltChar = tiltCharFromTiltNum(tiltNum);

  sprintf(filename, dir, time.year % 100, dayOfYear, location, field, 
	  time.year % 100, dayOfYear, time.hour, time.min, tiltChar);

  return;

}


int searchForReflectivityFile(char *dir,      // Input - directory to search.
			      char locChar,   // Input - location character in filename
			      date_time_t *T, // Input - time to start looking from
			      int tiltNum,    // Input - tilt number
			      int timeSearch, // Input - seconds to look ahead for next file
			      char *zFilename // Output - found filename, if any
			      ){

  for (int isearch = 0; isearch < timeSearch+1; isearch+=60){

    date_time_t S;
    S.unix_time = T->unix_time + isearch;
    uconvert_from_utime( &S );

    construeFileName(dir, locChar, 'Z', S, tiltNum, zFilename);

    struct stat buf;
    if (0==stat(zFilename, &buf)){
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

  int tiltNum = tiltNumFromTiltChar( startFileToUse[13] );
  if ( tiltNum == -1){
    fprintf(stderr,"ERROR : Cannot parse start tilt from %s\n",
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

  if (P.debug)
    fprintf(stderr, "Start time is %s, start tilt is number %d, location is %c\n", utimstr(T.unix_time), tiltNum+1, locChar);

  const int numTiltsInVol = 20;

  int numMissingTilts = 0;

  //
  // Initialize the FMQ.
  //
  lassenNetcdf2Dsr L( &P );

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

    char zFilename[1024];
    if (searchForReflectivityFile(P.Zdir, locChar, &T, tiltNum, 
				  P.searchTime, zFilename)){
      //
      // Failed to find file, try next tilt.
      //
      if (P.debug) fprintf(stderr, "Missing tilt %c\n",tiltCharFromTiltNum(tiltNum) );

      //
      // If it was the last tilt we didn't find, send an EOV anyway.
      //
      if (tiltNum == numTiltsInVol-1){
	L.sendEOV();
      }

      numMissingTilts++;

      if (numMissingTilts == numTiltsInVol){
	if (P.debug) fprintf(stderr, "Whole volume of missing tilts - assuming no more files, exiting.\n");
	break; // No more files.
      }

      tiltNum++; // Next tilt to hunt for.

      if (tiltNum == numTiltsInVol){
	tiltNum = 0;

	L.sendEOV();

	// We need to advance the time here, otherwise we will
	// start finding lower tilts that have already been processed.

	T.unix_time += 60; // Need to advance by at least a minute due to no seconds in filenames.
	uconvert_from_utime( &T );
	if (P.debug) fprintf(stderr, "Advancing time to %s\n", utimstr(T.unix_time));
      }
      continue;
    } else {
      //
      // Success, have a reflectivity file, construe the wind filename
      // and process the pair. Reset missing tilts count.
      //
      numMissingTilts = 0;

      char uFilename[1024];
      construeFileName(P.Udir, locChar, 'U', T, tiltNum, uFilename);

      // Make sure it exists.
      struct stat buf;
      if (0==stat(uFilename, &buf)){
	if (P.debug) fprintf(stderr,"Have Z, U pair %s and %s - processing\n", zFilename, uFilename);
	L.processFilePair(  zFilename, uFilename, T.unix_time, tiltNum);
	sleep(P.outputFmq.delayAfterSendingTiltSecs);
      } else {
	if (P.debug) fprintf(stderr,"WARNING : Z file %s has no U counterpart!\n", zFilename);
	L.processFilePair(  zFilename, NULL, T.unix_time, tiltNum);
	sleep(P.outputFmq.delayAfterSendingTiltSecs);
      }

      //
      // Save out that filename, if desired.
      //
      if (P.saveState.doSaveState){
	FILE *fp = fopen(P.saveState.stateFilename, "w");
	if (fp == NULL){
	  fprintf(stderr,"Failed to create state file %s\n", P.saveState.stateFilename);
	  exit(-1);
	}

	fprintf(fp, "%s", zFilename);

	fclose(fp);
      }

    }

    tiltNum++;
    if (tiltNum == numTiltsInVol){
      if (!(P.sendEOVonTiltDecrease)) L.sendEOV();
      tiltNum = 0;
    }
  } while (1); // As long as we're finding files.

  L.sendEOV(); // Flush buffers

  return 0;

}
