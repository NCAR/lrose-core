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
// main driver for stormDist
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
///////////////////////////////////////////////////////////////

#include "stormDist.hh"
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
static stormDist *_prog;
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
  PMU_auto_init("stormDist", TDRP_params.Instance,
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

  if (gotStart && gotEnd){
    uconvert_to_utime( &startTime );
    uconvert_to_utime( &endTime );
    start = startTime.unix_time;
    end = endTime.unix_time;
  } else {
    cerr << "Specify start and end times on the command line." << endl;
    cerr << "use -print_params to see syntax." << endl;
    exit(-1);
  }


  // create program object

  stormDist StormDist( &TDRP_params );


  // run it 

  int  iret = StormDist.Run( start, end );

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

  cerr << "FATAL ERROR - program stormDist" << endl;
  cerr << "  Operator new failed - out of store" << endl;
  exit(-1);

}



