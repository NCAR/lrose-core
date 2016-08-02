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

#include <toolsa/pmu.h>
#include <Mdv/DsMdvxTimes.hh>
#include <iostream>
#include <strings.h>

#include "Params.hh"
#include "UrlWatcher.hh"
#include "Process.hh"
using namespace std;

//
// Constructor
//
UrlWatcher::UrlWatcher(){
  startTime = 0; endTime = 0; // Default to REALTIME mode.
}
//////////////////////////////////////////////  
//
// Destructor
//
UrlWatcher::~UrlWatcher(){
}

//////////////////////////////////////////////////
// Main method - run.
//
int UrlWatcher::init( int argc, char **argv ){

  //
  // Parse command line args. Pretty minimal for this program.
  //
  if (ParseArgs(argc,argv)) return -1;
  //
  // Get TDRP args and check in to PMU.
  //

  
  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }                       
  
  PMU_auto_init("mdvFusion", P.Instance,
                PROCMAP_REGISTER_INTERVAL);     

  if ((startTime != 0) || (endTime != 0)){
    P.Mode = Params::ARCHIVE;
  }

  //
  // Handle the triggering. There are really four options - 
  //  1/ Trigger based on the data in a certain URL in realtime,
  //  2/ Trigger based on the data in a certain URL in archive mode,
  //  3/ Trigger at regular intervals in realtime, and
  //  4/ Trigger at regular intervals in archive mode.
  //
  // 1 and 2 are referred to as file based triggering, 3 and 4 are time based.
  //
  //
  // Set up for input.
  // Depends on what mode we're in.
  //
  //
  // Handle the setup for the file based ones here.
  //
  if (P.fileBasedTriggering){
    //
    if (P.Mode == Params::ARCHIVE){

      if (InMdv.setArchive(P.TriggerUrl,
			   startTime,
			   endTime)){
	cerr << "Failed to set URL " << P.TriggerUrl << endl;
	return -1;
      }
      
    } else { // REALTIME mode
      
      if (InMdv.setRealtime(P.TriggerUrl,
			    P.MaxRealtimeValidAge,
			    PMU_auto_register)){
	cerr << "Failed to set URL " << P.TriggerUrl << endl;
	return -1;
      }
            
    } // End of if we are in file-based realtime.
  }
  //
  // Input Mdv object should now be all set. 
  // We can use it in the run function.
  //
  return 0;
  
}

int UrlWatcher::run(){

  //
  // Deal with the file-based triggering first.
  //
  if (P.fileBasedTriggering){
    //
    do{
      
      time_t Time;
      
      InMdv.getNext( Time );
      
      if ((Time == (time_t)NULL) && (P.Mode == Params::ARCHIVE)){
	return 0; // Reached end of the list in archive mode.
      }
      
      if (Time != (time_t)NULL){
	Process S;
	S.Derive(&P, Time);
      }
      
    } while (1);
  
  } else {
    //
    // Time-based triggering.
    //
    if ((startTime !=0) && (endTime != 0)){
      //
      // Archive mode.
      //
      time_t triggerTime = startTime;
      do {
	Process S;
	if (P.Debug){
	  cerr << "Archive time trigger at " << utimstr(triggerTime) << endl;
	}
	PMU_auto_register("Archive interval");
	S.Derive(&P, triggerTime);
	triggerTime += P.triggerInterval;
      } while (triggerTime <= endTime);
      return 0;
    } else {
      //
      // Realtime mode. First trigger may be somewhat in the past but that's OK.
      //
      time_t triggerTime = (time(NULL) / P.triggerInterval) * P.triggerInterval;
      do {
	Process S;
	if (P.Debug){
	  cerr << "Realtime time trigger at " << utimstr(triggerTime) << endl;
	}
	S.Derive(&P, triggerTime);
	//
	triggerTime += P.triggerInterval;
	time_t currentTime = 0L;
	do {
	  PMU_auto_register("Realtime interval");
	  sleep(1);
	  currentTime = time(NULL);
	} while (currentTime < triggerTime);
      } while (1);
    }
  }
  return 0;
}

///////////////////////////////////////////////
// Parse command line args.
//
int UrlWatcher::ParseArgs(int argc,char *argv[]){

  for (int i=1; i<argc; i++){
 
    if ( (!strcmp(argv[i], "-h")) ||
         (!strcmp(argv[i], "--")) ||
         (!strcmp(argv[i], "-?")) ) {                
      cout << "USAGE : mdvFusion [-print_params to get parameters]" << endl;
      cout << "[-h or -- or -? to get this help message]" << endl;
      cout << "[-interval YYYYMMDDhhmmss YYYYMMDDhhmmss -params pfile to run in archive mode]" << endl;
      return -1;

    }
    
    if (!strcmp(argv[i], "-interval")){
      i++;
      if (i == argc) {
	cerr << "Must specify start and end times with -interval" << endl;
	exit(-1);
      }
      
      date_time_t T;
      if (6!=sscanf(argv[i],"%4d%2d%2d%2d%2d%2d",
		    &T.year, &T.month, &T.day,
		    &T.hour, &T.min, &T.sec)){
	cerr << "Format for start time is YYYYMMDDhhmmss" << endl;
	return -1;
      }
      uconvert_to_utime( &T );
      startTime = T.unix_time;
    
      i++;
      if (i == argc) {
	cerr << "Must specify end time with -interval" << endl;
	return -1;
      }

      if (6!=sscanf(argv[i],"%4d%2d%2d%2d%2d%2d",
		    &T.year, &T.month, &T.day,
		    &T.hour, &T.min, &T.sec)){
	cerr << "Format for end time is YYYYMMDDhhmmss" << endl;
	exit(-1);
      }
      uconvert_to_utime( &T );
      endTime = T.unix_time;


    }


  }

  return 0; // All systems go.
  
}

     






