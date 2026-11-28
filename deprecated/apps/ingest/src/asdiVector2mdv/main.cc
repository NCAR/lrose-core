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
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <Spdb/DsSpdb.hh>

#include "asdiGridder.hh"

static void tidy_and_exit (int sig);

void usage();

int main(int argc, char *argv[]){

 // Trap.
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);

  // Get the TDRP parameters.

  Params *P = new  Params();

  if (P->loadFromArgs(argc, argv, NULL, NULL)){
    fprintf(stderr,"Specify params file with -params option.\n");
    exit(-1);
  }

  // Check in.

  PMU_auto_init("asdiVector2mdv", P->instance, PROCMAP_REGISTER_INTERVAL);

  time_t start_time = 0L;
  time_t end_time   = 0L;
  //
  // See if we can parse start and end times from
  // the command line.
  //
  for (int i=0; i < argc; i++){
    //
    //
    if (!(strcmp(argv[i],"-h"))){
      usage(); tidy_and_exit(0);
    }
    //
    if (!(strcmp(argv[i],"-start"))){
      //
      i++;
      if (i > argc -1){
	usage(); tidy_and_exit(-1);
      }
      //
      date_time_t T;
      if (6 != sscanf(argv[i], "%d %d %d %d %d %d",
		      &T.year, &T.month, &T.day, &T.hour, &T.min, &T.sec)){
	usage(); tidy_and_exit(-1);
      }
      uconvert_to_utime( &T );
      start_time = T.unix_time;
    }

    if (!(strcmp(argv[i],"-end"))){
      //
      i++;
      if (i > argc -1){
	usage(); tidy_and_exit(-1);
      }
      //
      date_time_t T;
      if (6 != sscanf(argv[i], "%d %d %d %d %d %d",
		      &T.year, &T.month, &T.day, &T.hour, &T.min, &T.sec)){
	usage(); tidy_and_exit(-1);
      }
      uconvert_to_utime( &T );
      end_time = T.unix_time;
    }
  }
  //
  // If start and end times were specified, use archive mode.
  //
  if ((start_time != 0L) && (end_time != 0L))
    P->mode = Params::ARCHIVE;
  //
  // Process based on mode.
  //
  if (P->mode == Params::REALTIME){
    //
    // Realtime mode.
    //
    // Figure out the next output time.
    //
    time_t now = time(NULL);
    time_t nextTime = (time_t) (P->outputInterval*floor(double(now) / double(P->outputInterval)));
    if (P->debug)
      cerr << "First output will be at " << utimstr(nextTime) << endl;
      //
    while(1){
      //
      // Wait until the input database catches up.
      // Need at least 3 consecutive entires past the
      // end time before starting.
      //
      time_t first, last, latest;
      int count = 0;
      do {
	PMU_auto_register("Waiting on new data");
	DsSpdb timeGet;
	timeGet.getTimes(P->input_url, first, last, latest);
	if (P->debug){
	  cerr << "Database is at " << utimstr(latest);
	  cerr << " waiting for " << utimstr(nextTime);
	  cerr << " count = " << count << " (need 3)" << endl;
	}
	//
	// Wait for a minute while the database updates.
	//
	for (int k=0; k < 60; k++){
	  sleep(1);
	  PMU_auto_register("Waiting for data");
	}
	if (latest < nextTime){
	  count = 0;
	} else {
	  count++;
	}
      } while (count < 3);
      //
      asdiGridder *G = new asdiGridder( P );
      G->makeGrid(nextTime);
      delete G;
      nextTime +=  P->outputInterval;
    }
  } else {
    //
    // Archive mode.
    //
    asdiGridder G( P );
    for (time_t dataTime = start_time; dataTime <= end_time; dataTime += P->outputInterval){
      G.makeGrid( dataTime );
    }
  }
  //
  tidy_and_exit(0);
  return 0;

}

/////////////////////////////////////////////

static void tidy_and_exit (int sig){
  PMU_auto_unregister();
  exit(sig);
}

void usage(){
  //
  cerr << endl << "USAGE : " << endl;
  cerr << "asdiVector2mdv -print_params                    Gives parameters and documentation." << endl;
  cerr << "asdiVector2mdv -params file.params              Sets parameter file." << endl;
  cerr << "asdiVector2mdv -h                               Prints this message." << endl;
  cerr << "asdiVector2mdv -start \"YYYY MM DD hh mm ss\"     Sets start time." << endl;
  cerr << "asdiVector2mdv -end \"YYYY MM DD hh mm ss\"       Sets end time." << endl;
  cerr << endl << "Niles Oien July 2004" << endl << endl;
  //
  return;
}
