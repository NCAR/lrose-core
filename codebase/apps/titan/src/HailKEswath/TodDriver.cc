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
// TodDriver.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////
//
// Tod accumulation derived class
//
///////////////////////////////////////////////////////////////

#include "TodDriver.hh"
#include "Trigger.hh"
#include "MaxData.hh"
#include "OutputFile.hh"

#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
using namespace std;

//////////////
// Constructor

TodDriver::TodDriver(const string &prog_name, const Args &args, const Params &params)
  : MethodDriver(prog_name, args, params)

{

  _maxData = NULL;
  _trigger = NULL;

  // create accum object

  _maxData = new MaxData(_progName, _params);
  
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

TodDriver::~TodDriver()
  
{
  if (_maxData) {
    delete(_maxData);
  }
  if (_trigger) {
    delete(_trigger);
  }
}

//////////////////////////////////////////////////
// Run

int TodDriver::Run ()
{

  PMU_auto_register("MethodDriver::Run");
  
  // create periods object

  TodPeriod periods(_progName, _params);
  if (!periods.OK) {
    return -1;
  }
  
  // loop through times

  while ((_triggerTime = _trigger->next()) >= 0) {
    
    PMU_auto_register("MethodDriver::Run");
    
    if (_params.debug) {
      fprintf(stderr, "----> Trigger time: %s\n", utimstr(_triggerTime));
    }

    // do the accumulation

    if (_doAccum(periods)) {
      return (-1);
    }
      
  } // while
  
  return (0);

}

/////////////////////////////////////////////////////
// TodDriver::_doAccum()
//
// Compute tod accumulations, writing out each time.
//

int TodDriver::_doAccum(TodPeriod &periods)

{

  PMU_auto_register("TodDriver::_doAccum");

  // if period has changed, create new AccumData object
  
  if (periods.set(_triggerTime)) {
    delete (_maxData);
    _maxData = new MaxData(_progName, _params);
    _startSearchNext = periods.periodStart;
    if (_params.debug >= Params::DEBUG_NORM) {
      fprintf(stderr, "==========================================\n");
      fprintf(stderr, "TodDriver: new period %s to %s\n",
	      utimstr(periods.periodStart),
	      utimstr(periods.periodEnd));
      fprintf(stderr, "==========================================\n");
    }
  }

  // get list of input files in the running time period
  
  time_t dataStart = _startSearchNext + 1;
  time_t dataEnd = _triggerTime;
  
  if (_params.debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "TodDriver: data for %s to %s\n",
	    utimstr(dataStart), utimstr(dataEnd));
  }
  
  DsInputPath inputFiles(_progName, _params.debug,
			 _params.input_hail_data_dir,
			 dataStart, dataEnd);
  
  // set target period
  
  _maxData->setTargetPeriod((double) _triggerTime -
			  (double) periods.periodStart);
  
  // loop through the input files
  
  char *inputPath;
  while ((inputPath = inputFiles.next()) != NULL) {
    
    PMU_auto_register("TodDriver::_doAccum - reading in file");
    
    // process this file
    
    if (_maxData->processFile(inputPath)) {
      cerr << "TodDriver::_doAccum" << endl;
      cerr << "  Cannot process file: " << inputPath << endl;
      cerr << "  " << DateTime::str() << endl;
    }

  } // while ((inputPath ...
  
  if (_maxData->dataFound()) {

    PMU_auto_register("TodDriver::_doAccum - data found");

    // write out
    
    OutputFile out(_progName, _params);
    
    if (out.write(periods.periodStart,
		  _triggerTime,
		  _triggerTime,
		  _maxData->grid(), 
		  _maxData->maxHailKeFlux(),
		  _maxData->maxHailMassFlux())) {
      cerr << "ERROR - TodDriver::Run" << endl;
      return -1;
    }
    
    // set time for start of search next time
    
    _startSearchNext = _maxData->latestDataTime() + 1;
  
  } // if (_maxData->dataFound())

  return 0;

}

      
