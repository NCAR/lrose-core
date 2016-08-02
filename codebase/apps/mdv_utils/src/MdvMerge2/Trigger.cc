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
#include <toolsa/DateTime.hh>
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

  DateTime start_time_obj(start_time);
  DateTime next_time(start_time_obj.getYear(), start_time_obj.getMonth(),
		     start_time_obj.getDay(), 0, 0, 0);
  _nextTime = next_time.utime() + params.time_trigger_offset;
  while (_nextTime < _startTime)
    _nextTime += params.time_trigger_interval;
  
  _prevTime = _nextTime - params.time_trigger_interval;
  
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
  
  DateTime current_time(time(0));
  DateTime next_time(current_time.getYear(), current_time.getMonth(),
		     current_time.getDay(), 0, 0, 0);
  
  time_t current_utime = current_time.utime();
  _nextTime = next_time.utime() + params.time_trigger_offset;
  
  while (_nextTime < current_utime)
    _nextTime += params.time_trigger_interval;
  
  _prevTime = _nextTime - params.time_trigger_interval;
  
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

  // Set up the vectors
  
  for (int ii = 0; ii < _params.input_urls_n; ii++) {
    
    DsMdvxTimes *mdvxTimes = new DsMdvxTimes();
    
    // Set to non-blocking reads to get times.
    
    mdvxTimes->setRealtime(_params._input_urls[ii].url, 
			   _params.max_realtime_valid_age,
			   PMU_auto_register,
			   -1);

    _mdvxTimes.push_back(mdvxTimes);

    _lastUrlTimes.push_back(0);
    _currentUrlTimes.push_back(0);
    
  }
  
  // initialize lastTriggerTime.
  
  _lastTriggerTime = 0;

}

/////////////
// destructor

RealtimeFileTrigger::~RealtimeFileTrigger()

{
  
  for (size_t i = 0; i < _mdvxTimes.size(); i++){
    delete _mdvxTimes[i];
  }

}

////////////////////////////////////////
// get next time - returns -1 on failure

time_t RealtimeFileTrigger::next()

{
  
  time_t when = 0;
  
  // Initialize the time arrays to what is actually out there.

  _initTimeArrays();

  // We remain in this do loop until the return time we have
  // in the variable 'when' exceeds the last trigger time by at
  // least _params.min_time_between_triggers.

  while ((when == 0) ||
	 (when < _lastTriggerTime + _params.min_time_between_file_triggers)) {

    PMU_auto_register("Waiting for new data.");
    umsleep(1000);

    // First, check that all the URLs that must update
    // before we can go on have done so.
    
    bool neededHaveUpdated = false;
    
    while (!neededHaveUpdated) {
      neededHaveUpdated = true;
      for (int ii = 0; ii < _params.input_urls_n; ii++){
	if (_params._input_urls[ii].must_update) {
	  if (!_hasUpdated(ii)) {
	    neededHaveUpdated = false;
	    PMU_auto_register("Waiting for all URLs to update.");
	    umsleep(1000);
	    break;
	  }
	}
      } // ii
    } // while
    
    // Then, check that at least the minimum number of URLs have
    // updated.

    int numUpdated = 0;
    while(numUpdated < _params.min_number_updated_urls) {
      numUpdated = 0;
      for (int ii = 0; ii < _params.input_urls_n; ii++){
	if (_hasUpdated(ii)) {
	  numUpdated++;
	}
      }
      PMU_auto_register("Waiting for enough URLs to update.");
      umsleep(1000);
    } // while

    // OK - if we are here, then the URLs that must update before we
    // go further have updated, and we have the requisite number of URLs
    // that have updated. Find the most recent time from all
    // the URLs, and return the most recent.

    _updateCurrent();
    for (int ii = 0; ii < _params.input_urls_n; ii++) {
      if (_currentUrlTimes[ii] > when){
	when = _currentUrlTimes[ii];
      }
    }

    // End of outermost do loop (below) just makes sure we have
    // met or exceeded the specified min_time_between_triggers.
    
  } // while ((when == 0) ...

  // Store the last trigger time before we return it.

  _lastTriggerTime = when;

  // Update the array of times for each URL.

  _updateLatestToCurrent();

  // Return the time.
  
  return when;

}

/////////////////////////////////////////////////
// Has URL updated?

bool RealtimeFileTrigger::_hasUpdated(int UrlNum)

{

  // Update all current times to what is currently out there
  
  _updateCurrent();

  // See if this particular one has updated.

  if (_currentUrlTimes[UrlNum] > _lastUrlTimes[UrlNum]) {
    return true;
  }

  return false;

}

/////////////////////////////////////////////////
//
// Initialize the time arrays.

void RealtimeFileTrigger::_initTimeArrays()

{

  // Set both the current time and the latest time
  // for the URLs to what is currently out there.

  for (int ii = 0; ii < _params.input_urls_n; ii++) {

    time_t dataTime = 0;

    if (0 == _mdvxTimes[ii]->getNext(dataTime)) {
      _currentUrlTimes[ii] = dataTime;
      _lastUrlTimes[ii] = dataTime;
    } else {
      _currentUrlTimes[ii] = 0;
      _lastUrlTimes[ii] = 0;
    }

  } // ii

}

/////////////////////////////////////////////////
//
// Update the latest time array to the current
// time array. Updates the current time array first.

void RealtimeFileTrigger::_updateLatestToCurrent()

{

  _updateCurrent();

  for (int ii = 0; ii < _params.input_urls_n; ii++){
    if (_currentUrlTimes[ii] > _lastUrlTimes[ii]) {
      _lastUrlTimes[ii] = _currentUrlTimes[ii];
    }
  }

}

/////////////////////////////////////////////////
//
// Update the current time array
// by looking at the actual URLs.

void RealtimeFileTrigger::_updateCurrent()

{

  for (int ii = 0; ii < _params.input_urls_n; ii++){

    time_t dataTime = 0;

    if (0 == _mdvxTimes[ii]->getNext(dataTime)) {

      if (dataTime > _currentUrlTimes[ii]){

	// If we have a current data time, and it is greater
	// than the time we used to have, update the current array.

	_currentUrlTimes[ii] = dataTime;

      }

    }

  } // ii

}

