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
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
///////////////////////////////////////////////////////////////

#include "StormInit2Field.hh"
#include "Params.hh"

#include <didss/DsInputPath.hh>
#include <didss/LdataInfo.hh>
#include <toolsa/str.h>
#include <toolsa/port.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <signal.h>
#include <new>
#include <ctime>
#include <Fmq/NowcastQueue.hh>
#include <toolsa/umisc.h>

using namespace std;

// file scope

static void tidy_and_exit (int sig);
static void out_of_store();
static int exit_count = 0;

// main

int main(int argc, char *argv[])

{

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
  PMU_auto_init("StormInit2Field", TDRP_params.Instance,
                PROCMAP_REGISTER_INTERVAL);

  // set signal handling
  
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);

  // set new() memory failure handler function

  set_new_handler(out_of_store);

  //
  // Get the start and end times. Must be specified.
  //
  time_t start=0, end=0;
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
	  uconvert_to_utime( &startTime );
	  start = startTime.unix_time;
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
	  uconvert_to_utime( &endTime );
	  end = endTime.unix_time;
	}
      }
    }

    if (!(strcmp(argv[i],"-h"))){
      cerr << "Use the -print_params option for documentation." << endl;
      exit(0);
    }
 
  }


  //
  // If start and end times specified, go to archive mode.
  //
  if ((gotStart) && (gotEnd)){
    TDRP_params.Mode = Params::ARCHIVE;
    TDRP_params.triggerMode = Params::TRIGGER_ARCHIVE;
  }

  //
  // Make sure we have the start and end times in archive mode.
  //
  if (TDRP_params.Mode == Params::ARCHIVE){
    if (!gotStart || !gotEnd){
      cerr << "I need the start and end time specified." << endl;
      exit(-1);
    }
  }

  
  // create program object

  StormInit2Field makeFields( &TDRP_params );

  //
  // Use it - mode dependant.
  //

  time_t dataTime, theTime, lastTime=0;
  NowcastQueue nowcastQueue;

  time_t fmqTime;
  size_t count;
  time_t deltaTime;
  string saysWho;

  switch ( TDRP_params.triggerMode ){                                                            

  case Params::TRIGGER_ARCHIVE :
    //
    // Archive mode.
    //
    theTime = start;
    
    do {
      makeFields.Run(theTime - TDRP_params.LookBack,  theTime);
      theTime = theTime + TDRP_params.TriggerInterval;
    } while (theTime < end);
    break;

  case Params::TRIGGER_INTERVAL :
    //
    // Realtime mode - asynchronus, run at specified intervals.
    //

    do {
      //
      // Run right now.
      //
      theTime = time(NULL);
      makeFields.Run(theTime - TDRP_params.LookBack, theTime);
      //
      // Delay for the TriggerInterval before the next run.
      // Register every 15 seconds.
      //
      for (int i=0; i < TDRP_params.TriggerInterval; i++){
	sleep(1);
	if ((i % 15) == 0) PMU_auto_register("Waiting to trigger");
      }
      //
    } while ( 1 ); // It's just endless.
    break ;

  case Params::TRIGGER_LDATAINFO :
    //
    // Realtime mode - syncronus, triggered by local latest_data_info
    //
    while (1){
      PMU_auto_register("Waiting for new data...");
      LdataInfo L(TDRP_params.triggerDir);

      if (!(L.read(TDRP_params.maxValidAge))){
 
	dataTime = L.getLatestTime();
 
	if (dataTime <= lastTime){
	  sleep(1);
	} else {
	  makeFields.Run(dataTime - TDRP_params.LookBack, dataTime);
	  lastTime = dataTime;  
	}
      }
    }
    break;
    

  case Params::TRIGGER_FMQ :


    if (nowcastQueue.init(TDRP_params.triggerFmq, 
			  "StormInit2Field", 
			  TDRP_params.Instance, 
			  TDRP_params.debug)){
      cerr << "Failed to initialize queue " << TDRP_params.triggerFmq << endl;
      exit(-1);
    }

    do {

      int go = 1;

      do {
	sleep(1);
	PMU_auto_register( "Polling for trigger request" );
	if ( nowcastQueue.nextTrigger( saysWho, &fmqTime, &count,
				       &deltaTime ) == 0) {
	  go = 0;
	}
      } while(go);
      makeFields.Run(fmqTime - TDRP_params.LookBack, fmqTime);
  

    } while (1); // Endless loop.

    break;

  }


  // clean up. We only get here in archive mode.

  tidy_and_exit(0);
  return 0;

}

///////////////////
// tidy up on exit

static void tidy_and_exit (int sig)

{

  exit_count++;
  if (exit_count == 1) {
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

  cerr << "FATAL ERROR - program StormInit2Field" << endl;
  cerr << "  Operator new failed - out of store" << endl;
  exit(-1);

}



