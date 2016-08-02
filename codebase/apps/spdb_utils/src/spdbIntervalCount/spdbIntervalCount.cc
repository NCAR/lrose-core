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
// Just counts the SPDB chunks in regular intervals.
// Niles Oien July 9 2004.
//
#include <iostream>
#include <Spdb/DsSpdb.hh>
#include <toolsa/umisc.h>
#include <ctime>
#include <stdlib.h>

using namespace std;

void usage();

int main(int argc, char *argv[]){
  //
  // If there are no args, print usage.
  //
  if (argc == 1) usage();
  //
  // Set up with the defaults.
  //
  char *url = "spdbp:://localost::spdb/data";
  time_t end_time = time(NULL);
  time_t start_time = end_time - 3600;
  int interval = 300;
  bool plain = false;
  int dataType = 0;
  int dataType2 = 0;
  //
  // Process the args to override defaults if desired.
  //
  for(int i=1; i < argc; i++){

    if (!(strcmp(argv[i], "-print_params"))){
      cerr << endl << "I am a diagnostic program" << endl;
      cerr << "I know not TDRP;" << endl;
      cerr << "Do not attempt amusement" << endl;
      cerr << "by running -print_params on me." << endl;
      usage();
    }

    if (!(strcmp(argv[i], "-plain"))){
      plain = true;
    }

    if (!(strcmp(argv[i], "-interval"))){
      i++;
      if (i > argc -1){
	usage();
      }
      interval = atoi(argv[i]);
    }

    if (!(strcmp(argv[i], "-dataType"))){
      i++;
      if (i > argc -1){
	usage();
      }
      dataType = atoi(argv[i]);
    }

    if (!(strcmp(argv[i], "-dataType2"))){
      i++;
      if (i > argc -1){
	usage();
      }
      dataType2 = atoi(argv[i]);
    }

    if (!(strcmp(argv[i], "-url"))){
      i++;
      if (i > argc -1){
	usage();
      }
      url = argv[i];
    }


    if (!(strcmp(argv[i], "-h"))){
      usage();
    }

    if (!(strcmp(argv[i],"-start"))){
      //
      i++;
      if (i > argc -1){
        usage();
      }
      //
      date_time_t T;
      if (6 != sscanf(argv[i], "%d %d %d %d %d %d",
                      &T.year, &T.month, &T.day, &T.hour, &T.min, &T.sec)){
        usage();
      }
      uconvert_to_utime( &T );
      start_time = T.unix_time;
    }

    if (!(strcmp(argv[i],"-end"))){
      //
      i++;
      if (i > argc -1){
        usage();
      }
      //
      date_time_t T;
      if (6 != sscanf(argv[i], "%d %d %d %d %d %d",
                      &T.year, &T.month, &T.day, &T.hour, &T.min, &T.sec)){
        usage();
      }
      uconvert_to_utime( &T );
      end_time = T.unix_time;
    }

  }
  //
  // Check validity of input args.
  //
  if (
      (interval == 0) ||
      ((interval > 0) && (start_time > end_time)) ||
      ((interval < 0) && (start_time < end_time))
      ){
    cerr << endl << "Requested start time : " << utimstr(start_time) << endl;
    cerr << "Requested end time : " << utimstr(end_time) << endl;
    cerr << "Requested interval : " << interval << " seconds. " << endl;
    if (interval){
      cerr << "I cannot make my journey before I depart in this universe." << endl;
    } else {
      cerr << "Interval is 0, and I do need to see some progress in life." << endl;
    }
    usage();
  }
  //
  // Main loop.
  //
  time_t intervalEndTime = start_time + interval;

  int go=1;
  do {
    
    int margin = interval;
    if (margin < 0) margin = -margin;

    DsSpdb S;
    S.getInterval(url, intervalEndTime - margin, intervalEndTime,
		  dataType, dataType2, true);
    int numFound = S.getNChunks();
    //
    // Print out - the format depends of if -plain was specified.
    //
    if (plain){
      cerr << intervalEndTime - start_time - interval << " " << numFound << endl;
    } else {
      cerr << utimstr(intervalEndTime - margin) << " to ";
      cerr << utimstr(intervalEndTime) << " : ";
      cerr << numFound << " entries found." << endl;
    }
    //
    // Advance through time.
    //
    intervalEndTime += interval;
    //
    if (interval > 0){
      if (intervalEndTime - interval > end_time){
	go = 0;
      }
    } else {
      if (intervalEndTime < end_time) go = 0;
    }

  } while (go);
  //
  return 0;
  //
}

void usage() {

  cerr << endl << "USAGE : " << endl;;
  cerr << "-start \"YYYY MM DD hh mm ss\" : set start time [Default now]." << endl;
  cerr << "-end \"YYYY MM DD hh mm ss\"   : set end time [Default one hour ago]." << endl;
  cerr << "-interval N                  : set time increment, seconds [Default 300]." << endl;
  cerr << "-dataType                    : set SPDB data type    [Default 0]." << endl;
  cerr << "-dataType2                   : set SPDB data type 2  [Default 0]." << endl;
  cerr << "-h                           : print this message [Default false]." << endl;
  cerr << "-plain                       : printout for MatLab [Default false]." << endl;
  cerr << "-url                         : Set SPDB url [Default spdbp:://localost::spdb/data]." << endl;
  cerr << "Setting the end time prior to the start time and setting a negative interval" << endl;
  cerr << "causes the printout order to be reversed." << endl;
  cerr << endl << "Niles Oien July 2004." << endl;
  cerr << endl;
  //
  exit(0);

}
