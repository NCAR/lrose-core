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
// Bigfoot, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
///////////////////////////////////////////////////////////////

#include "StormInitClimatology.hh"
#include "Params.hh"

#include <toolsa/str.h>
#include <toolsa/port.h>
#include <toolsa/pmu.h>
#include <signal.h>
#include <new>

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
  PMU_auto_init("StormInitClimatology", TDRP_params.Instance,
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


    if (!(strcmp(argv[i],"-yesterday"))){
      //
      // Option to run on yestersay's data. Start and end
      // times run as pecified on the day before now.
      // Allows pseudo-realtime operation.
      //
      // Get today's time.
      //
      date_time_t T;
      T.unix_time = time(NULL);

      //
      // Back up by a day.
      //
      T.unix_time = T.unix_time -86400;
      uconvert_from_utime( &T );

      //
      // Get the start time.
      //
      int h,m,s;
      if (3!=sscanf(TDRP_params.yesterdayStart,
		    "%d:%d:%d", &h, &m, &s)){
	cerr << "yesterdayStart not correctly specified - I cannot cope.";
	cerr << endl;
	exit(-1);
      }
      if (
	  (h > 23) || (m > 59) || (s > 59) ||
	  (h < 0)  || (m < 0)  || (s < 0)
	  ){
	cerr << "yesterdayStart not specified in HH:MM:SS - I cannot cope.";
	cerr << endl;
	exit(-1);
      }

      T.hour = h; T.min = m; T.sec = s;
      uconvert_to_utime( &T );
      start = T.unix_time;

      //
      // Then the end time.
      //
      if (3!=sscanf(TDRP_params.yesterdayEnd,
		    "%d:%d:%d", &h, &m, &s)){
	cerr << "yesterdayEnd not correctly specified - I cannot cope.";
	cerr << endl;
	exit(-1);
      }
      if (
	  (h > 23) || (m > 59) || (s > 59) ||
	  (h < 0)  || (m < 0)  || (s < 0)
	  ){
	cerr << "yesterdayEnd not specified in HH:MM:SS - I cannot cope.";
	cerr << endl;
	exit(-1);
      }
      T.hour = h; T.min = m; T.sec = s;
      uconvert_to_utime( &T );
      if (T.unix_time < start){
	end = end + 86400; // Go forward by a day.
      }

      end = T.unix_time;

      gotEnd = true;  gotStart = true;
    }
  }

  if (!gotStart || !gotEnd){
    cerr << "I need the start and end time specified." << endl;
    exit(-1);
  }

  // create program object

  StormInitClimatology StormInit( &TDRP_params );

  int iret=0;
  iret = StormInit.Run( start, end );

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

  cerr << "FATAL ERROR - program StormInitClimatology" << endl;
  cerr << "  Operator new failed - out of store" << endl;
  exit(-1);

}



