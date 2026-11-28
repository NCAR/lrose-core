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
/////////////////////////////////////////////////////////////
// Trigger.cc
//
// Trigger mechanism 
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2004
//
///////////////////////////////////////////////////////////////

#include "Trigger.hh"
#include "Params.hh"
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/TriggerInfo.hh>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/toolsa_macros.h>
#include <iostream>
#include <cstdlib>  
#include <unistd.h> 
using namespace std;

////////////////////////////////////////////////////////////////////////////
//
// Abstract base class
//
////////////////////////////////////////////////////////////////////////////

//////////////
// constructor
//

Trigger::Trigger(const string &prog_name, const Params &params) :
  _progName(prog_name), _params(params)

{
  return;
}

/////////////
// destructor
//

Trigger::~Trigger()

{
  return;
}

////////////////////////////////////////////////////////////////////////////
//
// Archive mode time-based derived class
//
////////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

ArchiveTimeTrigger::ArchiveTimeTrigger(const string &prog_name,
				       const Params &params,
				       time_t start_time,
				       time_t end_time)
  : Trigger(prog_name, params)
  
{
  
  _startTime = start_time;
  _endTime = end_time;
  
  // anchor trigger times on the hour

  time_t sTime = _startTime - params.time_trigger_offset;
  time_t startHour = (sTime / SECS_IN_HOUR) * SECS_IN_HOUR;
  int secsSinceHour = sTime - startHour;

  if (_params.time_trigger_interval == 0){
    cerr << "time_trigger_interval cannot be 0, edit params and re-run" << endl;
    exit(-1);
  }

  int nTrig = secsSinceHour / _params.time_trigger_interval;
  if (_startTime != _endTime){
    _prevTime = startHour + nTrig * _params.time_trigger_interval;
    _nextTime = _prevTime + _params.time_trigger_interval;
  } else {
    _prevTime = _nextTime = _startTime;
  }

  if (_params.debug) {
    cerr << "Archive trigger" << endl;
    cerr << "  Start time: " << utimstr(_startTime) << endl;
    cerr << "  End   time: " << utimstr(_endTime) << endl;
    cerr << "  Prev  time: " << utimstr(_prevTime) << endl;
    cerr << "  Next  time: " << utimstr(_nextTime) << endl;
  }


  return;

}

/////////////
// destructor

ArchiveTimeTrigger::~ArchiveTimeTrigger()

{
  return;
}

////////////////////////////////////////
// get next time - returns -1 on failure

time_t ArchiveTimeTrigger::next()

{
  
  // check if _nextTime crosses hour - if so, set it to the
  // next hour
  
  long prevHour = _prevTime / SECS_IN_HOUR;
  long nextHour = _nextTime / SECS_IN_HOUR;
  if (nextHour != prevHour) {
    _nextTime = nextHour * SECS_IN_HOUR;
  }
  
  time_t triggerTime = _nextTime + _params.time_trigger_offset;
  _prevTime = _nextTime;
  _nextTime += _params.time_trigger_interval;
  
  if (_params.debug) {
    cerr << "  Next trigger: " << utimstr(triggerTime) << endl;
  }
  
  if (triggerTime > _endTime) {
    if (_params.debug) {
      cerr << "  Done" << endl;
    }
    return -1;
  }
  
  return triggerTime;

}

////////////////////////////////////////////////////////////////////////////
//
// Archive mode file-based derived class
//
////////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

ArchiveFileTrigger::ArchiveFileTrigger(const string &prog_name,
				       const Params &params,
				       time_t start_time, time_t end_time)
  : Trigger(prog_name, params)

{
  
  _inputPath = new DsMdvxInput;
  _inputPath->setArchive(_params._input_urls[0].url,
			 start_time, end_time);
  
}

/////////////
// destructor

ArchiveFileTrigger::~ArchiveFileTrigger()
  
{
  delete _inputPath;
}

////////////////////////////////////////
// get next time - returns -1 on failure

time_t ArchiveFileTrigger::next()
  
{
  
  time_t data_time;
  while(!_inputPath->endOfData()) {
    if(_inputPath->getTimeNext(data_time)) {
      cerr << "ERROR - " << _progName << ":ArchiveFileTrigger::next"  << endl;
      cerr << _inputPath->getErrStr() << endl;
      return -1;
    }

    return data_time;

  }

  return -1;
  
}

////////////////////////////////////////////////////////////////////////////
//
// Realtime mode time-based derived class
//
////////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

RealtimeTimeTrigger::RealtimeTimeTrigger(const string &prog_name,
					 const Params &params)
  : Trigger(prog_name, params)
  
{

  // set up the initial trigger times
  
  time_t startTime = time(NULL) - params.time_trigger_offset;
  time_t startHour = (startTime / SECS_IN_HOUR) * SECS_IN_HOUR;
  int secsSinceHour = startTime - startHour;

  if (_params.time_trigger_interval == 0){
    cerr << "time_trigger_interval cannot be 0, edit params and re-run" << endl;
    exit(-1);
  }

  int nTrig = secsSinceHour / _params.time_trigger_interval;
  _prevTime = startHour + nTrig * _params.time_trigger_interval;
  _nextTime = _prevTime + _params.time_trigger_interval;

  if (_params.debug) {
    cerr << "Realtime trigger" << endl;
    cerr << "  Prev  time: " << utimstr(_prevTime) << endl;
    cerr << "  Next  time: " << utimstr(_nextTime) << endl;
  }

}

/////////////
// destructor

RealtimeTimeTrigger::~RealtimeTimeTrigger()

{
}

////////////////////////////////////////
// get next time - returns -1 on failure

time_t RealtimeTimeTrigger::next()

{

  // check if trigger crosses hour - if so, set it to the
  // next hour
  
  long prevHour = _prevTime / SECS_IN_HOUR;
  long nextHour = _nextTime / SECS_IN_HOUR;
  if (nextHour != prevHour) {
    _nextTime = nextHour * SECS_IN_HOUR;
  }
  
  // wait for trigger time
  
  time_t triggerTime = _nextTime + _params.time_trigger_offset;
  if (_params.debug) {
    cerr << "  Next trigger: " << utimstr(triggerTime) << endl;
  }

  time_t now = time(NULL);
  while (now < triggerTime) {
    PMU_auto_register("In RealtimeTimeTrigger::next");
    umsleep(1000);
    now = time(NULL);
  }

  _prevTime = _nextTime;
  _nextTime += _params.time_trigger_interval;
  
  return triggerTime;

}

////////////////////////////////////////////////////////////////////////////
//
// Realtime mode file-based derived class
//
////////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

RealtimeFileTrigger::RealtimeFileTrigger(const string &prog_name,
					 const Params &params)
  : Trigger(prog_name, params)
{

  DsLdataTrigger *trigger = new DsLdataTrigger();
  trigger->init(_params._input_urls[0].url, -1, PMU_auto_register);

  _mdvxTriggers.push_back(trigger);
  _currentUrlTimes.push_back(0);
  _lastUrlTime = 0;

  for (int ii = 1; ii < _params.input_urls_n; ii++) {

    DsLdataTrigger *t = new DsLdataTrigger();
    t->init(_params._input_urls[ii].url, -1, 0, -1);

    _mdvxTriggers.push_back(t);   
    _currentUrlTimes.push_back(0);
  }
  
  _mainTrigger = trigger;
}

/////////////
// destructor

RealtimeFileTrigger::~RealtimeFileTrigger()
{
  for (size_t i = 0; i < _mdvxTriggers.size(); i++){
    delete _mdvxTriggers[i];
  }
}

////////////////////////////////////////
// get next time - returns -1 on failure

time_t RealtimeFileTrigger::next()

{
  time_t when = 0;

  TriggerInfo trigger_info;

  // Trigger off the first URL
  _mainTrigger->next(trigger_info);
  
  if (_params.debug)
    cerr << "*** Processing data for time: "
         << DateTime(trigger_info.getIssueTime()) << endl;

  when = trigger_info.getIssueTime();


  // now check that all the URLs that must update
  // before we can go on have done so.  
  bool neededHaveUpdated = false;
  int waitedTimes = 0;
  int margin = _params.time_trigger_margin;
  while (!neededHaveUpdated && waitedTimes < margin) {
    neededHaveUpdated = true;
    _updateCurrent();
    for (int ii = 1; ii < _params.input_urls_n; ii++){
      if (_params._input_urls[ii].must_update) {

	if (_currentUrlTimes[ii] + margin < when) {
	  neededHaveUpdated = false;
	  PMU_auto_register("Waiting for all URLs to update.");
	  umsleep(1000);
	  waitedTimes++;
	  if(waitedTimes == margin) {
	    cerr << "WARNING: Url number " << ii << " has not updated within margin time." << endl;
	    cerr << "Switching Url number " << ii << " to be main trigger url." << endl;

	    delete _mdvxTriggers[0];
	    delete _mdvxTriggers[ii];

	    DsLdataTrigger *trigger = new DsLdataTrigger();
	    trigger->init(_params._input_urls[ii].url, -1, PMU_auto_register);
	    _mdvxTriggers[0] = trigger;
	    _mainTrigger = trigger;

	    DsLdataTrigger *t = new DsLdataTrigger();
	    t->init(_params._input_urls[0].url, -1, 0, -1);
	    _mdvxTriggers[ii] = t;

	    Params::input_url_t tmp = _params._input_urls[ii];
	    _params._input_urls[ii] = _params._input_urls[0];
	    _params._input_urls[0] = tmp;

	    time_t timeTemp = _currentUrlTimes[0];
	    _currentUrlTimes[0] = _currentUrlTimes[ii];
	    _currentUrlTimes[ii] = timeTemp;
	  }
	  break;
	}
      }
    } // ii
  } // while


  // Store the last trigger time before we return it.

  _lastUrlTime = when;

  // Return the time.
  
  return when;
}

/////////////////////////////////////////////////
//
// Update the current time array
// by looking at the actual URLs.
void RealtimeFileTrigger::_updateCurrent()
{

  for (int ii = 1; ii < _params.input_urls_n; ii++){
    
    TriggerInfo trigger_info;
    _mdvxTriggers[ii]->next(trigger_info);

    if (trigger_info.getIssueTime() > _currentUrlTimes[ii]){
      
      // If we have a current data time, and it is greater
      // than the time we used to have, update the current array.
      
      _currentUrlTimes[ii] = trigger_info.getIssueTime();
      
    }

  } // ii

}
