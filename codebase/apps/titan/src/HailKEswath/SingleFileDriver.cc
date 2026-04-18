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
// SingleFileDriver.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////
//
// Single-file accumulation derived class
//
//////////////////////////////////////////////////////////////


#include "SingleFileDriver.hh"
#include "Trigger.hh"
#include "MaxData.hh"
#include "OutputFile.hh"

#include <toolsa/pmu.h>
#include <didss/DsInputPath.hh>
using namespace std;

//////////////
// Constructor

SingleFileDriver::SingleFileDriver(const string &prog_name,
				   const Args &args, const Params &params)
  : MethodDriver(prog_name, args, params)
{

  _trigger = NULL;
  
  // create the trigger - file trigger only
  
  if (_params.mode == Params::REALTIME) {
    // REALTIME mode
    _trigger = new RealtimeFileTrigger(_progName, _params);
  } else {
    // ARCHIVE mode
    _trigger = new ArchiveFileTrigger(_progName, _params,
				      _args.startTime, _args.endTime);
  }

}

//////////////
// destructor

SingleFileDriver::~SingleFileDriver()

{
  if (_trigger) {
    delete(_trigger);
  }
}

//////////////////////////////////////////////////
// Run

int SingleFileDriver::Run ()
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
// SingleFileDriver::_doAccum()
//
// Compute running accumulations, writing out each time.
//

int SingleFileDriver::_doAccum()
{

  PMU_auto_register("SingleFileDriver::_doAccum");

  // get just a single file
  
  time_t periodStart = _triggerTime;
  time_t periodEnd = _triggerTime;
  
  if (_params.debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "SingleFileDriver: period %s to %s\n",
	    utimstr(periodStart), utimstr(periodEnd));
  }
  
  DsInputPath inputFiles(_progName, _params.debug,
			 _params.input_hail_data_dir,
			 periodStart, periodEnd);
  
  // set up accumulation data object
  
  MaxData maxData(_progName, _params);
  maxData.setTargetPeriod((double) _params.running_duration);
  
  // loop through the input files
  
  char *inputPath;
  while ((inputPath = inputFiles.next()) != NULL) {
    
    PMU_auto_register("SingleFileDriver::_doAccum - reading in file");
    
    // process this file
    
    if (maxData.processFile(inputPath)) {
      cerr << "SingleFileDriver::_doAccum" << endl;
      cerr << "  Cannot process file: " << inputPath << endl;
      cerr << "  " << DateTime::str() << endl;
    }

  } // while ((inputPath ...
  
  if (maxData.dataFound()) {
    
    PMU_auto_register("SingleFileDriver::_doAccum - data found");

    // write out
    
    OutputFile out(_progName, _params);

    if (out.write(maxData.dataStartTime(),
		  maxData.dataEndTime(),
		  maxData.dataCentroidTime(),
		  maxData.grid(), 
		  maxData.maxHailKeFlux(),
		  maxData.maxHailMassFlux() )) {
      cerr << "ERROR - SingleFileDriver::Run" << endl;
      return -1;
    }
    
  }

  return 0;

}

      
