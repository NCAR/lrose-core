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
// main driver for Storms2Xml
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
///////////////////////////////////////////////////////////////

#include "Storms2Xml.hh"
#include "Params.hh"

#include <toolsa/str.h>
#include <toolsa/port.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>

#include <didss/DsInputPath.hh>
#include <didss/LdataInfo.hh>

#include <signal.h>
#include <cstring>

#include <toolsa/umisc.h>

// file scope

static void tidy_and_exit (int sig);
static void out_of_store();
static Storms2Xml *_prog;
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
  // Check in.
  //
  PMU_auto_init("Storms2Xml", TDRP_params.Instance,
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

  time_t archiveStart=0, archiveEnd=0;

  if (gotStart && gotEnd){
    TDRP_params.triggerMode = Params::TRIGGER_ARCHIVE;
    uconvert_to_utime( &startTime );
    uconvert_to_utime( &endTime );
    archiveStart = startTime.unix_time;
    archiveEnd   = endTime.unix_time;
  }

  //
  // Make sure the output directory exists.
  //
  if (ta_makedir_recurse(TDRP_params.outDir)){
    cerr << "Failed to create output directory : ";
    cerr << TDRP_params.outDir << " - exiting." << endl;
    exit(-1);
  }
  
  // create program object

  Storms2Xml S2Xml( &TDRP_params );


  // run it - mode dependant.

  int iret=0;
  const bool forever = true;
  time_t lastTime = 0;

  switch ( TDRP_params.triggerMode ){

  case Params::TRIGGER_ARCHIVE :
    //
    // Archive mode.
    //

    time_t archiveTriggerTime;
    archiveTriggerTime = archiveStart;

    do {

      iret = S2Xml.Run( archiveTriggerTime - TDRP_params.lookBack, 
			    archiveTriggerTime );
      
      archiveTriggerTime += TDRP_params.timeStep;

    } while( archiveTriggerTime <= archiveEnd );

    // clean up and exit.

    tidy_and_exit(iret); // In fact never get beyond here.

    return (iret);
    exit(iret);
    break;


  case Params::TRIGGER_INTERVAL :
    //
    // Realtime mode.
    //

    do {

      time_t end = time(NULL);
      time_t start = end - TDRP_params.lookBack;
      iret = S2Xml.Run( start, end );
      
      if (iret ==0){
	for (int i=0; i < TDRP_params.timeStep; i++){
	  sleep(1); PMU_auto_register("Waiting in realtime mode ...");
	}
      }
    } while ( forever );

    tidy_and_exit( 0 );
    break;
    

  case Params::TRIGGER_LDATAINFO :


    while(forever){

      LdataInfo L(TDRP_params.titanDir);
      L.read(TDRP_params.maxValidAge);

      time_t dataTime = L.getLatestTime();

      if (dataTime == lastTime){
	PMU_auto_register("Waiting for new data...");
	sleep(1);
      } else {
	iret = S2Xml.Run( dataTime - TDRP_params.lookBack, 
			  dataTime );
	lastTime = dataTime;
      }
    }


    tidy_and_exit( 0 );
    break;


  default :

    cerr << "Unrecognized trigger mode." << endl;
    exit(-1);
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

  cerr << "FATAL ERROR - program Storms2Xml" << endl;
  cerr << "  Operator new failed - out of store" << endl;
  exit(-1);

}



