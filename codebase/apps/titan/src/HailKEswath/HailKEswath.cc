/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
///////////////////////////////////////////////////////////////
// PrecipAccum.cc
//
// PrecipAccum object
//
//
///////////////////////////////////////////////////////////////

#include "HailKEswath.hh"
#include "RunningDriver.hh"
#include "TodDriver.hh"
#include "TotalDriver.hh"
#include "SingleFileDriver.hh"
using namespace std;

// Constructor

HailKEswath::HailKEswath(int argc, char **argv)

{

  isOK = TRUE;
  // set programe name
  
  _progName = "HailKEswath";
  ucopyright((char *) _progName.c_str());
  
  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
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
    cerr << "ERROR - HailKEswath" << endl;
    cerr << "  In ARCHIVE mode start and end times must be specified." << endl;
    isOK = false;
    return;
  }
  
  // initialize process registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
}

// destructor

HailKEswath::~HailKEswath()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int HailKEswath::Run ()
{

  PMU_auto_register("HailKEswath::Run");
  
  // create the accumulation driver

  MethodDriver *driver = nullptr;
  
  if (_params.max_method == Params::RUNNING_MAX) {

    driver = new RunningDriver(_progName, _args, _params);

  } else if (_params.max_method == Params::MAX_FROM_TIME_OF_DAY) {

    driver = new TodDriver(_progName, _args, _params);

  } else if (_params.max_method == Params::TOTAL_MAX) {

    driver = new TotalDriver(_progName, _args, _params);

  } else if (_params.max_method == Params::SINGLE_FILE) {

    driver = new SingleFileDriver(_progName, _args, _params);

  }

  if (driver->Run()) {
    delete(driver);
    return (-1);
  }
      
  delete(driver);
  return 0;

}


