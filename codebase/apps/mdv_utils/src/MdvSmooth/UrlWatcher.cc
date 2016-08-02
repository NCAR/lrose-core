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
// This is the UrlWatcher class for the MdvSmooth application. It
// watches the input URL for new data to process, OR processes
// an existing data set in archive mode.
//
// The heart of the class is the  DsLdataTrigger object known as _dataTrigger
// here, which is used to monitor the input for new data.
//
#include <toolsa/pmu.h>

#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <dsdata/DsFcstTimeListTrigger.hh>


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
  _dataTrigger = NULL;
  return;
}
//////////////////////////////////////////////  
//
// Destructor. Does nothing, but it avoids use of the default destructor.
//
UrlWatcher::~UrlWatcher(){
  if (_dataTrigger)
    delete _dataTrigger;
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
  PMU_auto_init("MdvSmooth", TDRP.Instance,
                PROCMAP_REGISTER_INTERVAL);     

  //
  // Check the TDRP parameters for vailidity. We should
  // have the same number of input and output field names.
  //
  if (TDRP.InFieldName_n != TDRP.OutFieldName_n){
    cerr << "Invalid number of elements specified in OutFieldName array.";
    cerr << endl;
    cerr << "Check parameter file." << endl;
    exit(-1);      
  }
  //
  // Set up the _dataTrigger object to monitor the input.
  // This depends on what mode we're in - in archive mode
  // we process all data between the start and end time, otherwise
  // in realtime mode we wait for new data to appear.
  //
  if (TDRP.Mode == Params::ARCHIVE){

      DsTimeListTrigger *trigger = new DsTimeListTrigger();
      if (trigger->init(TDRP.TriggerUrl,
			startTime,
			endTime)) {
        cerr << trigger->getErrStr() << endl;
	cerr << "  URL: " << TDRP.TriggerUrl << endl;
	cerr << "  start time: " << DateTime::str(startTime) << endl;
	cerr << "  end time: " << DateTime::str(endTime) << endl;
	return -1;
	
      }
      _dataTrigger = trigger;
  }
  else if  (TDRP.Mode == Params::ARCHIVE_FCST) {
  
    DsFcstTimeListTrigger *trigger = new DsFcstTimeListTrigger();
    if (trigger->init(TDRP.TriggerUrl,
		      startTime,
		      endTime) ) { 
      cerr << trigger->getErrStr() << endl;
      cerr << "  URL: " << TDRP.TriggerUrl << endl;
      cerr << "  start time: " << DateTime::str(startTime) << endl;
      cerr << "  end time: " << DateTime::str(endTime) << endl;
      return -1;
    }
    _dataTrigger = trigger;
    
  } else {
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(TDRP.TriggerUrl,
                      TDRP.MaxRealtimeValidAge,
                      PMU_auto_register) != 0) {
      cerr << trigger->getErrStr() << endl;
      return -1;
    }
    _dataTrigger = trigger;
  }
  
  return 0;
  
}

int UrlWatcher::run(){
  //
  // Use _dataTrigger to get the data time, and pass this off
  // to the process class.
  //
 
  int leadTime = -1;
  time_t Time;

  while (!_dataTrigger->endOfData()) {
    TriggerInfo triggerInfo;
    _dataTrigger->next(triggerInfo);
    Process S;
    if( S.Derive(&TDRP,triggerInfo.getIssueTime(), 
		 triggerInfo.getForecastTime() - triggerInfo.getIssueTime())){
      cerr << "Error processing file at %d " << triggerInfo.getIssueTime() << endl;
      
      return -1;
    }
  } // while

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
      cout << "USAGE : MdvSmooth [-print_params to get parameters]" << endl;
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

     






