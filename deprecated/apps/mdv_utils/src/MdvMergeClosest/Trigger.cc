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
#include "Parms.hh"
#include <toolsa/DateTime.hh>
#include <toolsa/LogMsg.hh>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>

////////////////////////////////////////////////////////////////////////////
//
// Abstract base class
//
////////////////////////////////////////////////////////////////////////////

//////////////
// constructor
//

Trigger::Trigger(const string &prog_name, const Parms &params) :
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
				       const Parms &params)
  : Trigger(prog_name, params)
  
{
  
  _startTime = params._archiveT0;
  _endTime = params._archiveT1;
  
  // anchor trigger times on the hour

  DateTime start_time_obj(_startTime);
  DateTime next_time(start_time_obj.getYear(), start_time_obj.getMonth(),
		     start_time_obj.getDay(), 0, 0, 0);
  _nextTime = next_time.utime() + params._triggerOffset;
  while (_nextTime < _startTime)
    _nextTime += params._triggerInterval;
  
  _prevTime = _nextTime - params._triggerInterval;
  
  LOGF(LogMsg::DEBUG, "  Start time: %s", DateTime::strn(_startTime).c_str());
  LOGF(LogMsg::DEBUG, "  End   time: %s", DateTime::strn(_endTime).c_str());
  LOGF(LogMsg::DEBUG, "  Prev  time: %s", DateTime::strn(_prevTime).c_str());
  LOGF(LogMsg::DEBUG, "  Next  time: %s", DateTime::strn(_nextTime).c_str());
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
  
  time_t triggerTime = _nextTime + _params._triggerOffset;
  _prevTime = _nextTime;
  _nextTime += _params._triggerInterval;
  
  LOGF(LogMsg::DEBUG, "  Next trigger: %s",
       DateTime::strn(triggerTime).c_str());
  
  if (triggerTime > _endTime)
  {
    LOG(LogMsg::DEBUG, "Done");
    return -1;
  }
  
  return triggerTime;

}

////////////////////////////////////////////////////////////////////////////
//
// Realtime mode time-based derived class
//
////////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

RealtimeTimeTrigger::RealtimeTimeTrigger(const string &prog_name,
					 const Parms &params)
  : Trigger(prog_name, params)
  
{

  // set up the initial trigger times
  
  DateTime current_time(time(0));
  DateTime next_time(current_time.getYear(), current_time.getMonth(),
		     current_time.getDay(), 0, 0, 0);
  
  time_t current_utime = current_time.utime();
  _nextTime = next_time.utime() + params._triggerOffset;
  
  while (_nextTime < current_utime)
    _nextTime += params._triggerInterval;
  
  _prevTime = _nextTime - params._triggerInterval;
  
  LOGF(LogMsg::DEBUG, "  Prev  time: %s", DateTime::strn(_prevTime).c_str());
  LOGF(LogMsg::DEBUG, "  Next  time: %s", DateTime::strn(_nextTime).c_str());
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
  
  time_t triggerTime = _nextTime + _params._triggerOffset;
  LOGF(LogMsg::DEBUG, "  Next trigger: %s",
       DateTime::strn(triggerTime).c_str());

  time_t now = time(NULL);
  while (now < triggerTime) {
    PMU_auto_register("In RealtimeTimeTrigger::next");
    umsleep(1000);
    now = time(NULL);
  }

  _prevTime = _nextTime;
  _nextTime += _params._triggerInterval;
  
  return triggerTime;
}

