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
//
// This is the UrlWatcher class for the MdvFine2Coarse application. It
// watches the input URL for new data to process, OR processes
// an existing data set in archive mode.
//
// The heart of the class is the  DsMdvxTimes object known as _InMdv
// here, which is used to monitor the input for new data.
//
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
  return;
}
//////////////////////////////////////////////  
//
// Destructor. Does nothing, but it avoids use of the default destructor.
//
UrlWatcher::~UrlWatcher(){
  return;
}

//////////////////////////////////////////////////
//
// The init method gets the TDRP parametrs, checks in to PMU and
// sets up the input mechanism appropriately.
//
int UrlWatcher::init( int argc, char **argv ){
  //
  // Parse command line args. Pretty minimal for this program.
  //
  if (_ParseArgs(argc,argv)) return -1;
  //
  // Get TDRP parameters and check in to PMU.
  //
  if (TDRP.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }                       
  //  
  PMU_auto_init("MdvFine2Coarse", TDRP.Instance,
                PROCMAP_REGISTER_INTERVAL);     
  //
  // Overwrite the mode we are in if archive mode was specified.
  //
  if ((startTime != 0) || (endTime != 0)){
    TDRP.Mode = Params::ARCHIVE;
  }

  //
  // Set up the _InMdv object to monitor the input.
  // This depends on what mode we're in - in archive mode
  // we process all data between the start and end time, otherwise
  // in realtime mode we wait for new data to appear.
  //
  if (TDRP.Mode == Params::ARCHIVE){
    //
    // Archive mode - set up to process everything between
    // startTime and endTime.
    //
    if (_InMdv.setArchive(TDRP.TriggerUrl,
			 startTime,
			 endTime)){
      cerr << "Failed to set URL " << TDRP.TriggerUrl << endl;
      return -1;
    }
  } else {
    //
    // Realtime mode - set up to watch for new data appearing
    // at the TriggerUrl and process it as it arrives.
    //
    if (_InMdv.setRealtime(TDRP.TriggerUrl,
			  TDRP.MaxRealtimeValidAge,
			  PMU_auto_register)){
      cerr << "Failed to set URL " << TDRP.TriggerUrl << endl;
      return -1;
    }
  } // End of if we are in realtime.

  //
  // _InMdv object should now be all set. Use it in the run function.
  //
  return 0;

}

int UrlWatcher::run(){
  //
  // Use _InMdv to get the data time, and pass this off
  // to the process class.
  //
  do{

    time_t Time;
    _InMdv.getNext( Time );
    
    if ((Time == (time_t)NULL) && (TDRP.Mode == Params::ARCHIVE)){
      return 0; // Reached end of the list in archive mode.
    }

    if (Time != (time_t)NULL){
      //
      // Declare a Process argument and use it to
      // derive a new dataset.
      //
      Process S;
      S.Derive(&TDRP, Time);
    }
  } while (1); // Loop forever.
  
  return 0;

}
///////////////////////////////////////////////
// Parse command line args.
//
int UrlWatcher::_ParseArgs(int argc,char *argv[]){

  for (int i=1; i<argc; i++){
    //
    // Is this a request for help?
    //
    if ( (!strcmp(argv[i], "-h")) ||
         (!strcmp(argv[i], "--")) ||
         (!strcmp(argv[i], "-?")) ) {                
      cout << "USAGE : MdvFine2Coarse [-print_params to get parameters]" << endl;
      cout << "[-h or -- or -? to get this help message]" << endl;
      cout << "[-interval YYYYMMDDhhmmss YYYYMMDDhhmmss -params pfile to run in archive mode]" << endl;
      return -1;

    }
    //
    // Are we setting archive mode?
    //    
    if (!strcmp(argv[i], "-interval")){
      i++;
      if (i == argc) {
	cerr << "Must specify start and end times with -interval" << endl;
	exit(-1);
      }
      //
      // Scan in the start time.
      //      
      date_time_t T;
      if (6!=sscanf(argv[i],"%4d%2d%2d%2d%2d%2d",
		    &T.year, &T.month, &T.day,
		    &T.hour, &T.min, &T.sec)){
	cerr << "Format for start time is YYYYMMDDhhmmss" << endl;
	return -1;
      }
      uconvert_to_utime( &T );
      startTime = T.unix_time;
      //
      // Scan in the end time.
      //    
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

  return 0; // All done.
  
}

     






