///////////////////////////////////////////////////////////////
// TitanStormIdentDriver.cc
//
// TitanStormIdentDriver object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// Dave Albo
//
// August 2014
//
///////////////////////////////////////////////////////////////

#include "TitanStormIdentDriver.hh"
#include "StormIdent.hh"
#include <toolsa/pmu.h>

// Constructor

TitanStormIdentDriver::TitanStormIdentDriver(int argc, char **argv) :
  _progName("TitanStormIdent"),
  _args(_progName)

{

  OK = true;

  // copyright messaege

  ucopyright((char *) _progName.c_str());

  // parse command line args

  if (_args.parse(argc, (const char **) argv)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = false;
    return;
  }

  // load TDRP params from command line

  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = false;
    return;
  }

  // check start and end in ARCHIVE mode
  
  if (_params.mode == Params::ARCHIVE &&
      (_args.startTime == 0 || _args.endTime == 0)) {
    fprintf(stderr, "ERROR - %s\n", _progName.c_str());
    fprintf(stderr,
	    "In ARCHIVE mode start and end times must be specified.\n");
    fprintf(stderr, "Run '%s -h' for usage\n", _progName.c_str());
    OK = false;
    return;
  }

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

TitanStormIdentDriver::~TitanStormIdentDriver()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int TitanStormIdentDriver::Run ()
{

  // register with procmap
  
  PMU_auto_register("TitanStormIdentDriver::Run");

  if (_params.mode == Params::ARCHIVE) {

    return _runArchive();

  } else {

    return _runRealtime();
    
  }

}

//////////////////////////////////////////////////
// _runRealtime

int TitanStormIdentDriver::_runRealtime ()
{

  // loop forever

  while (true) {
    
    // register with procmap
    
    PMU_auto_register("TitanStormIdentDriver::_runRealtime");
    
    // run StormIdent
    
    StormIdent ident(_progName, _params);
    if (ident.runRealtime()) {
      cerr << "ERROR - TitanStormIdentDriver::_runRealtime" << endl;
      umsleep (1000);
    }
   
  } // while
  
  return 0;

}


//////////////////////////////////////////////////
// _runArchive

int TitanStormIdentDriver::_runArchive ()
{
  // run StormIdent

  StormIdent ident(_progName, _params);

  if (ident.runArchive(_args.startTime, _args.endTime)) {
    cerr << "ERROR - TitanStormIdentDriver::_runArchive" << endl;
    return -1;
  }

  return 0;

}

