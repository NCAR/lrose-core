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
// PrecipAccum.cc
//
// PrecipAccum object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
///////////////////////////////////////////////////////////////

#include "PrecipAccum.hh"
#include "RunningDriver.hh"
#include "TodDriver.hh"
#include "TotalDriver.hh"
#include "ClimoDriver.hh"
#include "SingleFileDriver.hh"
#include "SingleFileFcstDriver.hh"
using namespace std;

// Constructor

PrecipAccum::PrecipAccum(int argc, char **argv)

{

  isOK = TRUE;

  // set programe name
  
  _progName = "PrecipAccum";
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

  // check params
  
  // check start and end in ARCHIVE mode

  if ((_params.mode == Params::ARCHIVE) &&
      (_args.startTime == 0 || _args.endTime == 0)) {
    cerr << "ERROR - PrecipAccum" << endl;
    cerr << "  In ARCHIVE mode start and end times must be specified." << endl;
    isOK = false;
    return;
  }
  if (_params.input_is_precip) {
    if (_params.generate_max_dbz_grid) {
      cerr << "ERROR - PrecipAccum" << endl;
      cerr << "  input_is_precip set to true." << endl;
      cerr << "  generate_max_dbz_grid set to true." << endl;
      cerr << "  max_dbz cannot be determined from precip." << endl;
      isOK = false;
      return;
    }
    if (_params.generate_max_vil_grid) {
      cerr << "ERROR - PrecipAccum" << endl;
      cerr << "  input_is_precip set to true." << endl;
      cerr << "  generate_max_vil_grid set to true." << endl;
      cerr << "  max_vil cannot be determined from precip." << endl;
      isOK = false;
      return;
    }
    if (_params.input_is_rate) {
      cerr << "ERROR - PrecipAccum" << endl;
      cerr << "  Both input_is_precip and input_is_rate are  true." << endl;
      cerr << "  You can only set one of these options." << endl;
      isOK = false;
      return;
    }
  } // if (_params.input_is_precip)

  // initialize process registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
}

// destructor

PrecipAccum::~PrecipAccum()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int PrecipAccum::Run ()
{

  PMU_auto_register("PrecipAccum::Run");
  
  // create the accumulation driver

  MethodDriver *driver;
  
  if (_params.accum_method == Params::RUNNING_ACCUM) {

    driver = new RunningDriver(_progName, _args, _params);

  } else if (_params.accum_method == Params::ACCUM_FROM_TIME_OF_DAY) {

    driver = new TodDriver(_progName, _args, _params);

  } else if (_params.accum_method == Params::TOTAL_ACCUM) {

    driver = new TotalDriver(_progName, _args, _params);

  } else if (_params.accum_method == Params::CLIMO_ACCUM) {

    driver = new ClimoDriver(_progName, _args, _params);

  } else if (_params.accum_method == Params::SINGLE_FILE_FORECAST) {
    
    driver = new SingleFileFcstDriver(_progName, _args, _params);

  } else {
  
    // assume single file

    driver = new SingleFileDriver(_progName, _args, _params);

  }

  if (driver->Run()) {
    delete(driver);
    return (-1);
  }
      
  delete(driver);
  return 0;

}


