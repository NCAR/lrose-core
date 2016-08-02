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
///////////////////////////////////////////////////////////////
//
// main driver for StormInitLocation
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
///////////////////////////////////////////////////////////////

#include "StormInitLocation.hh"
#include "Params.hh"

#include <didss/DsInputPath.hh>
#include <didss/LdataInfo.hh>

#include <toolsa/str.h>
#include <toolsa/port.h>
#include <toolsa/pmu.h>
#include <signal.h>
#include <new>
#include <cstring>

#include <toolsa/umisc.h>

using namespace std;

// file scope

static void tidy_and_exit (int sig);
static void out_of_store();
static StormInitLocation *_prog;
static int exit_count = 0;

// main

int main(int argc, char *argv[])

{

  for (int i=1; i < argc; i++){
    if (!(strcmp("-h",argv[i]))){
      cerr << "Use the -print_params option for documentation." << endl;
      exit(0);
    }
  }


  //
  // Get TDRP parameters.
  //

  Params TDRP_params;
  if (TDRP_params.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Failed to even load the parameters." << endl;
    return -1;
  }

  //
  // Check in to the big gray hotel.
  //
  PMU_auto_init("StormInitLocation", TDRP_params.Instance,
                PROCMAP_REGISTER_INTERVAL);

  // set signal handling
  
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);

  // set new() memory failure handler function

  set_new_handler(out_of_store);

  //
  // Get the start and end times, if specified.
  //

  date_time_t startTime, endTime;
  bool gotStart=false, gotEnd=false;
  for (int i=1; i < argc; i++){

    if (!(strcmp(argv[i],"-start"))){
      i++;
      if (i < argc){
	if (6==sscanf(argv[i],"%d %d %d %d %d %d",
		      &startTime.year, &startTime.month, &startTime.day, 
		      &startTime.hour, &startTime.min, &startTime.sec)){
	  gotStart = true;
	}
      }
    }

    if (!(strcmp(argv[i],"-end"))){
      i++;
      if (i < argc){
	if (6==sscanf(argv[i],"%d %d %d %d %d %d",
		      &endTime.year, &endTime.month, &endTime.day, 
		      &endTime.hour, &endTime.min, &endTime.sec)){
	  gotEnd = true;
	}
      }
    }
  }

  time_t start=0, end=0;
  bool archiveMode = false;

  if (gotStart && gotEnd){
    archiveMode = true;
    uconvert_to_utime( &startTime );
    uconvert_to_utime( &endTime );
    start = startTime.unix_time;
    end = endTime.unix_time;
  }

  //
  // If we have start and end times specified, use
  // the archive mode.
  //  

  if (archiveMode){
    TDRP_params.triggerMode = Params::TRIGGER_ARCHIVE;
  }

  // create program object

  StormInitLocation StormInit( &TDRP_params );


  // run it - mode dependant.

  int iret=0;
  time_t lastTime = 0;

  switch ( TDRP_params.triggerMode ){

  case Params::TRIGGER_ARCHIVE :
    //
    // Archive mode.
    //
    iret = StormInit.Run( start, end, archiveMode );

    // clean up
    
    tidy_and_exit(iret);
    return (iret);
    break;

  case Params::TRIGGER_INTERVAL :
    //
    // Realtime mode - in asynchronus intervals.
    //
    do {

      time_t end = time(NULL);
      time_t start = end - TDRP_params.lookback_time;
      iret = StormInit.Run( start, end, archiveMode );
      
      if (iret ==0){
	for (int i=0; i < TDRP_params.time_trigger_interval; i++){
	  sleep(1); PMU_auto_register("Waiting in realtime mode ...");
	}
      }
    } while ( 1 );
    break;

  case Params::TRIGGER_LDATAINFO :
    //
    // Realtime mode - trigger from directory.
    //
    do{

      LdataInfo *L = new LdataInfo(TDRP_params.triggerDir);

      if (!(L->read(TDRP_params.maxValidAge))){

	time_t dataTime = L->getLatestTime();

	if (dataTime <= lastTime){
	  PMU_auto_register("Waiting for new data...");
	  sleep(1);
	} else {
	  time_t end = dataTime;
	  time_t start = end - TDRP_params.lookback_time;
	  iret = StormInit.Run( start, end, archiveMode );
	  lastTime = dataTime;
	  
	}
      }
      delete L;
    } while( 1 );
    break;

  }


  // clean up

  tidy_and_exit(iret);
  return (iret);
  
}

///////////////////
// tidy up on exit

static void tidy_and_exit (int sig)

{

  exit_count++;
  if (exit_count == 1) {
    delete(_prog);
    exit(sig);
  }

}

////////////////////////////////////
// out_of_store()
//
// Handle out-of-memory conditions
//

static void out_of_store()

{

  cerr << "FATAL ERROR - program StormInitLocation" << endl;
  cerr << "  Operator new failed - out of store" << endl;
  exit(-1);

}



