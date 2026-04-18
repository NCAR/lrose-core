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
// RunningDriver.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////
//
// Running accumulation derived class
//
//////////////////////////////////////////////////////////////


#include "RunningDriver.hh"
#include "Trigger.hh"
#include "MaxData.hh"
#include "OutputFile.hh"

#include <toolsa/pmu.h>
#include <didss/DsInputPath.hh>
using namespace std;

//////////////
// Constructor

RunningDriver::RunningDriver(const string &prog_name,
			     const Args &args, const Params &params)
  : MethodDriver(prog_name, args, params)
{

  _trigger = NULL;

  // create the trigger

  if (_params.mode == Params::REALTIME) {
    // REALTIME mode
    if (_params.trigger == Params::TIME_TRIGGER) {
      _trigger = new RealtimeTimeTrigger(_progName, _params);
    } else {
      _trigger = new RealtimeFileTrigger(_progName, _params);
    }
  } else {
    // ARCHIVE mode
    if (_params.trigger == Params::TIME_TRIGGER) {
      _trigger = new ArchiveTimeTrigger(_progName, _params,
					_args.startTime,  _args.endTime);
    } else {
      _trigger = new ArchiveFileTrigger(_progName, _params,
					_args.startTime, _args.endTime);
    }
  }

}

//////////////
// destructor

RunningDriver::~RunningDriver()

{
  if (_trigger) {
    delete(_trigger);
  }
}

//////////////////////////////////////////////////
// Run

int RunningDriver::Run ()
{

  PMU_auto_register("MethodDriver::Run");
  
  // loop through times

  while ((_triggerTime = _trigger->next()) >= 0) {
    
    PMU_auto_register("MethodDriver::Run");
    
    if (_params.debug) {
      fprintf(stderr, "----> Trigger time: %s\n", utimstr(_triggerTime));
    }
    
    // do the accumulation

    if (_doAccum()) {
      return (-1);
    }
      
  } // while
  
  return (0);

}

/////////////////////////////////////////////////////
// RunningDriver::_doAccum()
//
// Compute running accumulations, writing out each time.
//

int RunningDriver::_doAccum()
{

  PMU_auto_register("RunningDriver::_doAccum");

  // get list of input files in the running time period
  
  time_t periodStart = _triggerTime - _params.running_duration + 1;
  time_t periodEnd = _triggerTime;

  if (_params.debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "RunningDriver: period %s to %s\n",
	    utimstr(periodStart), utimstr(periodEnd));
  }

  DsInputPath inputFiles(_progName, _params.debug,
			 _params.input_hail_data_dir,
			 periodStart, periodEnd);
  
  // set up max data object
  
  MaxData maxData(_progName, _params);
  maxData.setTargetPeriod((double) _params.running_duration);

  // loop through the input files
  
  char *inputPath;
  while ((inputPath = inputFiles.next()) != NULL) {

    PMU_auto_register("RunningDriver::_doAccum - reading in file");

    // process this file
    
    if (maxData.processFile(inputPath)) {
      cerr << "RunningDriver::_doAccum" << endl;
      cerr << "  Cannot process file: " << inputPath << endl;
      cerr << "  " << DateTime::str() << endl;
    }

  } // while ((inputPath ...
  
  if (maxData.dataFound()) {
    
    PMU_auto_register("RunningDriver::_doAccum - data found");

    // write out
    
    OutputFile out(_progName, _params);
    
    if (out.write(periodStart,
		  periodEnd,
		  periodEnd,
		  maxData.grid(),
		  maxData.maxHailKeFlux(),
		  maxData.maxHailMassFlux())) {
      cerr << "ERROR - RunningDriver::Run" << endl;
      return -1;
    }
    
  }

  return 0;

}

      
