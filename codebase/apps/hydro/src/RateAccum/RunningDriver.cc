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
#include "AccumData.hh"
#include <algorithm>
#include <toolsa/pmu.h>
using namespace std;

//////////////
// Constructor

RunningDriver::RunningDriver(const string &prog_name,
			     const Args &args, const Params &params)
  : MethodDriver(prog_name, args, params)
{

  // create the trigger

  _trigger = NULL;

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

  // set up accum period vector, sort into ascending order

  for (int ii = 0; ii < _params.running_accums_n; ii++) {
    _accumPeriods.push_back(_params._running_accums[ii].period_secs);
  }
  sort(_accumPeriods.begin(), _accumPeriods.end());
  for (size_t jj = 0; jj < _accumPeriods.size(); jj++) {
    for (int ii = 0; ii < _params.running_accums_n; ii++) {
      if (_params._running_accums[ii].period_secs == _accumPeriods[jj]) {
        _outputUrls.push_back(_params._running_accums[ii].output_url);
        break;
      }
    }
  }

  if (_params.debug) {
    cerr << "==== Running driver, accum periods and URLs ====" << endl;
    for (size_t jj = 0; jj < _accumPeriods.size(); jj++) {
      cerr << "  accum period, url: "
           << _accumPeriods[jj] << ", "
           << _outputUrls[jj] << endl;
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
  
  double longestAccumPeriod = _accumPeriods[_accumPeriods.size() - 1];
  time_t periodStart = _triggerTime - longestAccumPeriod;
  time_t periodEnd = _triggerTime;

  // extend the file search by an hour

  time_t searchStart = periodStart - 3600;
  
  // load up path list and durations
  
  _loadPathList(searchStart, periodEnd);

  // work through the input files in reverse order
  // adding up accumulation as we go
  
  int thisPeriod = 0;
  double thisPeriodDuration = _accumPeriods[thisPeriod];
  double sumFileDurations = 0.0;

  // initialize accum object

  _accum->init();
  _accum->setTargetPeriod(thisPeriodDuration);

  // loop through files
  
  for (ssize_t ii = _filePaths.size() - 1; ii >= 0; ii--) {

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Working on accum period: " << thisPeriod << endl;
      cerr << "  sumFileDurations: " << sumFileDurations << endl;
    }
    
    // process this file
    
    if (_accum->processFile(_filePaths[ii],
                            _fileTimes[ii],
                            _fileDurations[ii])) {
      cerr << "RunningDriver::_doAccum" << endl;
      cerr << "  Cannot process file: " << _filePaths[ii] << endl;
      cerr << "  File time: " << DateTime::strm(_fileTimes[ii]) << endl;
    }
    
    sumFileDurations += _fileDurations[ii];
    
    if (sumFileDurations >= thisPeriodDuration || ii == 0) {

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "End of period: " << thisPeriod << endl;
        cerr << "  sumFileDurations: " << sumFileDurations << endl;
      }
    
      if (_accum->write(_outputUrls[thisPeriod])) {
        cerr << "ERROR - RunningDriver::Run" << endl;
        return -1;
      }

      thisPeriod++;
      thisPeriodDuration = _accumPeriods[thisPeriod];
      _accum->setTargetPeriod(thisPeriodDuration);

      if (thisPeriod > (int) _accumPeriods.size() - 1) {
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "Done with running periods" << endl;
        }
        break;
      }
  
    } // if (sumFileDurations >= thisPeriodDuration
    
  } // ii

  _accum->free();
  
  return 0;

}

      
