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
#include "AccumData.hh"

#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
using namespace std;

//////////////
// Constructor

TodDriver::TodDriver(const string &prog_name, const Args &args, const Params &params)
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

TodDriver::~TodDriver()
  
{
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
    _accum->init();
    _startSearchNext = periods.periodStart + 1;
    if (_params.debug >= Params::DEBUG_NORM) {
      fprintf(stderr, "==========================================\n");
      fprintf(stderr, "TodDriver: new period %s to %s\n",
	      utimstr(periods.periodStart),
	      utimstr(periods.periodEnd));
      fprintf(stderr, "==========================================\n");
    }
  }

  // get list of input files in the running time period
  
  time_t dataStart = _startSearchNext;
  time_t dataEnd = _triggerTime;
  
  if (_params.debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "TodDriver: data for %s to %s\n",
	    utimstr(dataStart), utimstr(dataEnd));
  }
  
  DsInputPath inputFiles(_progName, _params.debug,
			 _params.input_rdata_dir,
			 dataStart, dataEnd);
  
  // set target period
  
  _accum->setTargetPeriod((double) _triggerTime -
			  (double) periods.periodStart);
  
  // loop through the input files
  
  char *inputPath;
  while ((inputPath = inputFiles.next()) != NULL) {
    
    PMU_auto_register("TodDriver::_doAccum - reading in file");
    
    // process this file
    
    if (_accum->processFile(inputPath)) {
      cerr << "TodDriver::_doAccum" << endl;
      cerr << "  Cannot process file: " << inputPath << endl;
      cerr << "  " << DateTime::str() << endl;
    }

  } // while ((inputPath ...
  
  if (_accum->dataFound()) {

    PMU_auto_register("TodDriver::_doAccum - data found");
    
    if (_accum->computeAndWrite(periods.periodStart,
                                _triggerTime,
                                _triggerTime)) {
      cerr << "ERROR - TodDriver::Run" << endl;
      return -1;
    }
    
    // set time for start of search next time
    
    _startSearchNext = _accum->latestDataTime() + 1;
  
  } // if (_accum->dataFound())

  return 0;

}

      
