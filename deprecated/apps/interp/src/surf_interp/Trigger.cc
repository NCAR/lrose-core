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
// August 1998
//
///////////////////////////////////////////////////////////////

#include "Trigger.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
using namespace std;

/////////////////////////////////////////////////////////////////////////
//
// Abstract base class
//
/////////////////////////////////////////////////////////////////////////

//////////////
// constructor
//

Trigger::Trigger(const string &prog_name, const Params &params) :
  _progName(prog_name),
  _params(params)

{
}

/////////////
// destructor
//

Trigger::~Trigger()

{
}

/////////////////////////////////////////////////////////////////////////
//
// Archive mode time-based derived class
//
/////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

ArchiveTimeTrigger::ArchiveTimeTrigger(const string &prog_name,
				       const Params &params,
				       time_t start_time, time_t end_time)
  : Trigger(prog_name, params)
     
{
  
  _startTime = start_time;
  _endTime = end_time;
  _prevTime = _startTime - _params.time_trigger_interval;
  
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

  time_t thisTime = _prevTime + _params.time_trigger_interval;
  if (thisTime > _endTime) {
    return (-1);
  } else {
    _prevTime = thisTime;
    return (thisTime);
  }

}


///////////////////////////////////////////////////////////////////////
//
// Realtime mode time-based derived class
//
///////////////////////////////////////////////////////////////////////

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
  int nTrig = secsSinceHour / _params.time_trigger_interval;
  _prevTime = startHour + nTrig * _params.time_trigger_interval;
  _nextTime = _prevTime + _params.time_trigger_interval;

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

  // check if _nextTime crosses hour - if so, set it to the
  // next hour

  long prevHour = _prevTime / 3600;
  long nextHour = _nextTime / 3600;

  if (nextHour != prevHour) {
    _nextTime = nextHour * 3600;
  }

  // wait for next time

  time_t now = time(NULL);
  while (now < _nextTime) {
    PMU_auto_register("In RealtimeTimeTrigger::next");
    sleep(1);
    now = time(NULL);
  }

  // Increment until _nextTime is off in the future.

  do {
    _prevTime = _nextTime;
    _nextTime += _params.time_trigger_interval;
    now = time(NULL);
  } while (_nextTime <= now);

  return (_prevTime);
  
}
