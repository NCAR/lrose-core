///////////////////////////////////////////////////////////////
// TitanDriver.cc
//
// TitanDriver object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1998
//
///////////////////////////////////////////////////////////////

#include "TitanDriver.hh"
#include "FileLock.hh"
#include "StormIdent.hh"
#include "StormTrack.hh"
#include "DataTimes.hh"
#include "Sounding.hh"
#include <toolsa/pmu.h>
#include <titan/track.h>
#include <didss/DsInputPath.hh>
using namespace std;

// Constructor

TitanDriver::TitanDriver(int argc, char **argv) :
  _progName("Titan"),
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

  // check history length

  if (_params.tracking_forecast_weights_n > MAX_NWEIGHTS_FORECAST) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    cerr << "Max size of 'tracking_forecast_weights' array is "
	 << MAX_NWEIGHTS_FORECAST << endl;
    OK = false;
    return;
  }
  
  // check start and end in ARCHIVE and RETRACK mode
  
  if ((_params.mode == Params::ARCHIVE ||
       _params.mode == Params::RETRACK) &&
      (_args.startTime == 0 ||
       _args.endTime == 0)) {
    fprintf(stderr, "ERROR - %s\n", _progName.c_str());
    fprintf(stderr,
	    "In ARCHIVE and RETRACK mode start and end times "
	    "must be specified.\n");
    fprintf(stderr, "Run '%s -h' for usage\n", _progName.c_str());
    OK = false;
    return;
  }

  // check dbz histogram interval is rational

  int nIntervals = (int)
    ((_params.high_dbz_threshold - _params.low_dbz_threshold) /
     (_params.dbz_hist_interval) + 1);
  if (nIntervals > 100) {
    cerr << "WARNING - TitanDriver" << endl;
    cerr << "  Too many dbz intervals: " << nIntervals << endl;
    cerr << "  Requested dbz_hist_interval: "
         << _params.dbz_hist_interval << endl;
    _params.dbz_hist_interval =
      (_params.high_dbz_threshold - _params.low_dbz_threshold) / 100.0;
    cerr << "  Resetting dbz_hist_interval in params to: "
         << _params.dbz_hist_interval << endl;
  }

  // initialize the sounding profile

  Sounding &sndg = Sounding::inst();
  sndg.setParams(&_params);
  
  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // create file lock

  _fileLock = new FileLock(_progName, _params);
  if (!_fileLock->OK) {
    OK = false;
    return;
  }
  
  return;

}

// destructor

TitanDriver::~TitanDriver()

{

  // unregister process

  PMU_auto_unregister();

  if (_fileLock) {
    delete(_fileLock);
  }

}

//////////////////////////////////////////////////
// Run

int TitanDriver::Run ()
{

  // register with procmap
  
  PMU_auto_register("TitanDriver::Run");

  if (_params.mode == Params::RETRACK) {

    return _runRetrack();

  } else if (_params.mode == Params::ARCHIVE) {

    return _runArchive();

  } else if (_params.mode == Params::FORECAST) {

    return _runForecast();

  } else {

    return _runRealtime();
    
  }

}

//////////////////////////////////////////////////
// _runRealtime

int TitanDriver::_runRealtime ()
{

  // loop forever

  while (true) {
    
    // register with procmap
    
    PMU_auto_register("TitanDriver::_runRealtime");
    
    // run StormIdent
    
    StormIdent ident(_progName, _params);
    if (ident.runRealtime()) {
      cerr << "ERROR - TitanDriver::_runRealtime" << endl;
      umsleep (1000);
    }
   
  } // while
  
  return 0;

}


//////////////////////////////////////////////////
// _runArchive

int TitanDriver::_runArchive ()
{

  if (_params.auto_restart) {
    
    // compute the restart time
    
    time_t overlapStartTime;
    time_t startTime;
    time_t restartTime;

    DataTimes::computeRestartTime(_args.startTime,
				  _params.restart_time.hour,
				  _params.restart_time.min,
				  _params.restart_overlap_period,
				  overlapStartTime,
				  startTime,
				  restartTime);
    
    while (startTime < _args.endTime) {
      
      // register with procmap
      
      PMU_auto_register("TitanDriver::_runArchive");
      
      // run

      StormIdent ident(_progName, _params);
      if (ident.runArchive(overlapStartTime, startTime, restartTime)) {
	cerr << "ERROR - TitanDriver::_runArchive" << endl;
	return -1;
      }

      // update time range

      overlapStartTime += SECS_IN_DAY;
      startTime += SECS_IN_DAY;
      restartTime += SECS_IN_DAY;

    } // while

  } else {

    // no auto restart

    StormIdent ident(_progName, _params);
    if (ident.runArchive(_args.startTime, _args.startTime, _args.endTime)) {
      cerr << "ERROR - TitanDriver::_runArchive" << endl;
      return -1;
    }

  }

  return 0;

}


//////////////////////////////////////////////////
// run on forecast data

int TitanDriver::_runForecast()
{

  StormIdent ident(_progName, _params);
  if (ident.runForecast(_args.genTime)) {
    cerr << "ERROR - TitanDriver::_runForecast" << endl;
    return -1;
  }

  return 0;

}


//////////////////////////////////////////////////
// _runRetrack

int TitanDriver::_runRetrack ()

{
  
  // register with procmap
  
  PMU_auto_register("TitanDriver::_runRetrack");

  // create input path object

  DsInputPath input(_progName,
		    _params.debug >= Params::DEBUG_EXTRA,
		    _params.storm_data_dir,
		    _args.startTime, _args.endTime);

  input.setSearchExt(STORM_HEADER_FILE_EXT);

  char *stormPath;

  while ((stormPath = input.next()) != NULL) {
    
    if (_params.debug) {
      cerr << "Retracking file: " << stormPath << endl;
    }

    StormTrack strack(_progName, _params, stormPath);
    strack.ReTrack();

  }
  
  return 0;

}

  

