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
// August 2000
//
///////////////////////////////////////////////////////////////

#include "Trigger.hh"
#include <toolsa/pmu.h>


using namespace std;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
// Abstract base class
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////
// constructor
//

Trigger::Trigger(const string &prog_name, const Params &params) :
  _progName(prog_name), _params(params)

{

}

/////////////
// destructor
//

Trigger::~Trigger()

{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
// Archive mode time-based derived class
//
// Derived class for archive mode trigger based on time
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

ArchiveTimeTrigger::ArchiveTimeTrigger(const string &prog_name,
				       const Params &params,
				       time_t start_time, time_t end_time)
  : Trigger(prog_name, params)

{

  _startTime = start_time;
  _endTime = end_time;
  _prevTime = _startTime - _params.trigger_interval;
  
}

/////////////
// destructor

ArchiveTimeTrigger::~ArchiveTimeTrigger()

{
}

////////////////////////////////////////
// get next time - returns -1 on failure

time_t ArchiveTimeTrigger::next()

{

  time_t thisTime = _prevTime + _params.trigger_interval;
  if (thisTime > _endTime) {
    return (-1);
  } else {
    _prevTime = thisTime;
    return (thisTime);
  }

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
// Realtime mode time-based derived class
//
// Derived class for realltime mode trigger based on time
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

RealtimeTimeTrigger::RealtimeTimeTrigger(const string &prog_name,
					 const Params &params)
  : Trigger(prog_name, params)
  
{

  // set up the initial trigger times
  
  time_t now = time(NULL);
  time_t startHour = (now / 3600) * 3600;
  int secsSinceHour = now - startHour;
  int nTrig = secsSinceHour / _params.trigger_interval;
  _prevWallclockTime = startHour + nTrig * _params.trigger_interval;
  _nextWallclockTime = _prevWallclockTime + _params.trigger_interval;

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

  // check if _nextWallclockTime crosses hour - if so, set it to the
  // next hour

  long prevHour = _prevWallclockTime / 3600;
  long nextHour = _nextWallclockTime / 3600;

  if (nextHour != prevHour) {
    _nextWallclockTime = nextHour * 3600;
  }

  // wait for next time

  time_t now = time(NULL);
  while (now < _nextWallclockTime) {
    PMU_auto_register("In RealtimeTimeTrigger::next");
    sleep(1);
    now = time(NULL);
  }

  _prevWallclockTime = _nextWallclockTime;
  _nextWallclockTime += _params.trigger_interval;
  
  return (_prevWallclockTime - _params.realtime_data_lag_time);

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
// RealtimeDataTrigger
//
// Derived class for realtime mode trigger based on data
// arrival
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

RealtimeDataTrigger::RealtimeDataTrigger(const string &prog_name,
					 const Params &params)
  : RealtimeTimeTrigger(prog_name, params),
    _inputUrl(_params.input_url),
    _ldata(_inputUrl, _params.debug >= Params::DEBUG_VERBOSE)
  
{

  _prevTime = 0;

}

/////////////
// destructor

RealtimeDataTrigger::~RealtimeDataTrigger()

{

}

////////////////////////////////////////
// get next time
//
// If data arrives within the trigger interval, the latest data
// time is returned. Otherwise, the current time is returned.

time_t RealtimeDataTrigger::next()

{

  // check if _nextWallclockTime crosses hour - if so, set it to the
  // next hour

  long prevHour = _prevWallclockTime / 3600;
  long nextHour = _nextWallclockTime / 3600;

  if (nextHour != prevHour) {
    _nextWallclockTime = nextHour * 3600;
  }

  // get data set info from data mapper

  time_t now = time(NULL);
  while (now < _nextWallclockTime) {
    
    if (_ldata.read(_params.max_realtime_valid_age) == 0) {
      _prevTime = _ldata.getLatestTime();
      _prevWallclockTime = _nextWallclockTime;
      _nextWallclockTime += _params.trigger_interval;
      return (_ldata.getLatestTime());
    }

    PMU_auto_register("In RealtimeTimeTrigger::next");
    sleep(1);
    now = time(NULL);

  }

  _prevWallclockTime = _nextWallclockTime;
  _nextWallclockTime += _params.trigger_interval;
  return (_prevWallclockTime - _params.realtime_data_lag_time);

}



