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
#include "AccumData.hh"

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
			 _params.input_rdata_dir,
			 periodStart, periodEnd);
  
  // initialize accumulation data object
  
  _accum->init();
  _accum->setTargetPeriod((double) _params.running_duration);
  
  // loop through the input files
  
  char *inputPath;
  while ((inputPath = inputFiles.next()) != NULL) {
    
    PMU_auto_register("SingleFileDriver::_doAccum - reading in file");
    
    // process this file
    
    if (_accum->processFile(inputPath)) {
      cerr << "SingleFileDriver::_doAccum" << endl;
      cerr << "  Cannot process file: " << inputPath << endl;
      cerr << "  " << DateTime::str() << endl;
    }

  } // while ((inputPath ...
  
  if (_accum->dataFound()) {
    
    PMU_auto_register("SingleFileDriver::_doAccum - data found");

    if (_accum->computeAndWrite(_accum->dataStartTime(),
                                _accum->dataEndTime(),
                                _accum->dataCentroidTime())) {
      cerr << "ERROR - SingleFileDriver::Run" << endl;
      return -1;
    }
    
  }

  return 0;

}

      
