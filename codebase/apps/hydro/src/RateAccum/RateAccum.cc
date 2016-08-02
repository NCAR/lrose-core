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
// RateAccum.cc
//
// RateAccum object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2015
//
///////////////////////////////////////////////////////////////
//
// RateAccum accumulates precip depth by integrating rate over time
// Data is read from and written to MDV files.
//
////////////////////////////////////////////////////////////////

#include "RateAccum.hh"
#include "RunningDriver.hh"
#include "DailyDriver.hh"
#include "TotalDriver.hh"
using namespace std;

// Constructor

RateAccum::RateAccum(int argc, char **argv)

{

  isOK = TRUE;
  
  // set programe name
  
  _progName = "RateAccum";
  ucopyright((char *) _progName.c_str());
  
  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char*)"unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }

  // check start and end in ARCHIVE mode

  if ((_params.mode == Params::ARCHIVE) &&
      (_args.startTime == 0 || _args.endTime == 0)) {
    cerr << "ERROR - RateAccum" << endl;
    cerr << "  In ARCHIVE mode start and end times must be specified." << endl;
    isOK = false;
    return;
  }

  // initialize process registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
}

// destructor

RateAccum::~RateAccum()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RateAccum::Run ()
{

  PMU_auto_register("RateAccum::Run");
  
  // create the accumulation driver

  MethodDriver *driver;
  
  if (_params.accum_method == Params::RUNNING_ACCUM) {

    driver = new RunningDriver(_progName, _args, _params);

  } else if (_params.accum_method == Params::DAILY_ACCUM) {

    driver = new DailyDriver(_progName, _args, _params);

  } else if (_params.accum_method == Params::TOTAL_ACCUM) {

    driver = new TotalDriver(_progName, _args, _params);

  } else {

    cerr << "ERROR - RateAccum::Run" << endl;
    cerr << "  Bad accum method: " << _params.accum_method << endl;
    return -1;

  }

  if (driver->Run()) {
    delete(driver);
    return (-1);
  }
      
  delete(driver);
  return 0;

}


