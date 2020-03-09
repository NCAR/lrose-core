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
#include <dsdata/DsFcstTimeListTrigger.hh>

#include "Params.hh"
#include "UrlWatcher.hh"
#include "Process.hh"
using namespace std;


//
// Constructor
//
UrlWatcher::UrlWatcher(){
  startTime = 0; endTime = 0;
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
  // Get TDRP args.
  //
  if (TDRP_params.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }                       
  //
  // Set up for input.
  //
  DsFcstTimeListTrigger *trigger = new DsFcstTimeListTrigger();
  if (trigger->init(TDRP_params.TriggerUrl, startTime, endTime) != 0)
  {
    cerr << trigger->getErrStr() << endl;
    return false;
  }
  _dataTrigger = trigger;

  //
  // Input Mdv object should now be all set. Use it in the run function.
  //
  return 0;

}

int UrlWatcher::run(){

  while (!_dataTrigger->endOfData())
  {
    TriggerInfo triggerInfo;
    time_t inputTime = _dataTrigger->next(triggerInfo);
    Process S;
    S.Derive(&TDRP_params, triggerInfo, inputTime);
  } // while
    
  if (_dataTrigger)
    delete _dataTrigger;

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
      cout << "USAGE : MdvFillMissing [-print_params to get parameters and help]" << endl;
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

     






