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
// Control.cc
//
// Control of surface interpolation - object
//
// Niles Oien for
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////

#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <rapmath/math_macros.h>

#include "Control.hh"
#include "Trigger.hh"
#include "GetDataAndWrite.hh"
using namespace std;

// Constructor

Control::Control(int argc, char **argv)

{

  OK = TRUE;

  // set programe name
  
  _progName = "surf_interp";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = const_cast<char*>(string("unknown").c_str());
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    OK = FALSE;
    return;
  }

  if (!_args.TR_spec && _params.mode == Params::ARCHIVE) {
    fprintf(stderr,"No time range specified in ARCHIVE mode.\n");
    OK = FALSE;
    return;
  }

  if (!OK) {
    return;
  }

  // create the trigger

  if (_params.mode == Params::REALTIME) {
    _trigger = new RealtimeTimeTrigger(_progName, _params);
  } else {
    _trigger = new ArchiveTimeTrigger(_progName, _params,
				      _args.startTime,  _args.endTime);
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  return;

}

// destructor

Control::~Control()

{

  // free up memory

  if (_trigger) {
    delete _trigger;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Control::Run ()
{
  
  PMU_auto_register("Control::Run");
  
  // loop through times

  time_t triggerTime;

  while ((triggerTime = _trigger->next()) >= 0) {

    if (_params.debug) {
      fprintf(stderr, "Processing for %s\n", utimstr(triggerTime));
    }

    GetDataAndWrite(triggerTime, _params);    
    
  } // while

  return (0);

}








