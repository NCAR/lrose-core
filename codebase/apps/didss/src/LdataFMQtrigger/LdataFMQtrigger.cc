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
// LdataFMQtrigger.cc
//
// LdataFMQtrigger object
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2004
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <didss/RapDataDir.hh>
#include <dsserver/DmapAccess.hh>
#include "LdataFMQtrigger.hh"
using namespace std;

// Constructor

LdataFMQtrigger::LdataFMQtrigger(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "LdataFMQtrigger";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // Initialize the queue. Failing to do this is a fatal error.

  if ( _nowcastQueue.init( _params.fmq_url,
			   "MdvMerge",
			   _params.instance,
			   _params.debug ) != 0 ) {
    cerr << "Failed to initialize FMQ " << _params.fmq_url << endl;
    exit(-1);
  }


  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL );

  return;

}

// destructor

LdataFMQtrigger::~LdataFMQtrigger()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int LdataFMQtrigger::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // Set up LdataInfo

  LdataInfo ldata(_params.inputPath, _params.debug >= Params::DEBUG_VERBOSE);
  
  // wait for data
  
  time_t lastTriggerTime = 0L;

  while (true) {
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      time_t now = time(NULL);
      cerr << "LdataFMQtrigger::Run - waiting for latest data at "
	   << utimstr(now) << endl;
    }

    if (
	(ldata.read(_params.max_realtime_valid_age) == 0) &&
	( ldata.getLatestValidTime() > lastTriggerTime)
	) {
      
      lastTriggerTime = ldata.getLatestValidTime();

      // found new data
            
      if (_params.debug) {
	cerr << "LdataFMQtrigger::Run - got latest data" << endl;
	ldata.print(cerr);
      }
      
      
      // Delay before issuing trigger (may well be delay of 0)
      
      if ((_params.debug) && ( _params.sleep_before_trigger)){
	cerr << "Waiting " <<  _params.sleep_before_trigger;
	cerr << " seconds." << endl;
      }
      
      for (int k=0; k < _params.sleep_before_trigger; k++){
	sleep(1);
	if ( k % 15 == 0){
	  PMU_auto_register("Sleeping before firing trigger.");
	}
      }

      // Fire the trigger.

      time_t nowcastFrequency = (time_t)(_params.nowcast_frequency * 60);
      size_t nowcastCount     = (size_t)(_params.nowcast_count);

      time_t triggerTime;

      if (_params.useCurrentTime){
	triggerTime = time(NULL);
      } else {
	triggerTime = ldata.getLatestValidTime();
      }
      
      if (_params.debug){
	time_t now = time(NULL);
	cerr << "Firing trigger for " << utimstr(triggerTime);
	cerr << " at " << utimstr(now) << endl;
      }
      
      if (_nowcastQueue.fireTrigger( _params.appName,
				     triggerTime,
				     nowcastCount, nowcastFrequency )){
	cerr << "Failed to fire nowcast trigger!" << endl;
	exit(-1);
      }
      
    }

    PMU_auto_register("Waiting for data");
    umsleep(1000);
    
  } // while

  return 0;

}

