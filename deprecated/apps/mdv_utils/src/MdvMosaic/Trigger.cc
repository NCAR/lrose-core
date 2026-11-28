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

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//
// Abstract base class
//
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

//////////////
// constructor
//

Trigger::Trigger(char *prog_name, MdvMosaic_tdrp_struct *params)

{

  OK = TRUE;
  _progName = STRdup(prog_name);
  _params = params;

}

/////////////
// destructor
//

Trigger::~Trigger()

{

  STRfree(_progName);

}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//
// Archive mode time-based derived class
//
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

ArchiveTimeTrigger::ArchiveTimeTrigger(char *prog_name, MdvMosaic_tdrp_struct *params,
				       time_t start_time, time_t end_time)
  : Trigger(prog_name, params)

{

  _startTime = start_time;
  _endTime = end_time;
  _prevTime = _startTime - _params->time_trigger_interval;
  
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

  time_t thisTime = _prevTime + _params->time_trigger_interval;
  if (thisTime > _endTime) {
    return (-1);
  } else {
    _prevTime = thisTime;
    return (thisTime);
  }

}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//
// Archive mode file-based derived class
//
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

ArchiveFileTrigger::ArchiveFileTrigger(char *prog_name, MdvMosaic_tdrp_struct *params,
				       time_t start_time, time_t end_time)
  : Trigger(prog_name, params)

{

  _inputPath = new DsInputPath(_progName,
			       _params->debug >= DEBUG_VERBOSE,
			       _params->input_dirs.val[0],
			       start_time, end_time);
  
}

/////////////
// destructor

ArchiveFileTrigger::~ArchiveFileTrigger()

{
  delete (_inputPath);
}

////////////////////////////////////////
// get next time - returns -1 on failure

time_t ArchiveFileTrigger::next()

{

  char *path = _inputPath->next();

  if (path == NULL) {
    return (-1);
  }

  // find the slash delimiter for the data subdir

  char *slash = strstr(path, PATH_DELIM);
  if (slash == NULL) {
    fprintf(stderr, "ERROR - %s:ArchiveFileTrigger::next\n", _progName);
    fprintf(stderr, "Invalid path: %s\n", path);
    return (-1);
  }
  char *last_slash;
  while (slash != NULL) {
    last_slash = slash;
    slash = strstr(slash + 1, PATH_DELIM);
  }

  // copy the date and time strings

  char dateStr[16];
  char timeStr[16];
  strncpy(dateStr, last_slash - 8, 8);
  strncpy(timeStr, last_slash + 1, 6);

  // decode date and time

  date_time_t path_time;

  if (sscanf(dateStr, "%4d%2d%2d", &path_time.year, &path_time.month, &path_time.day) != 3) {
    fprintf(stderr, "ERROR - %s:ArchiveFileTrigger::next\n", _progName);
    fprintf(stderr, "Invalid path: %s\n", path);
    fprintf(stderr, "Cannot decode date.\n");
    return (-1);
  }
  
  if (sscanf(timeStr, "%2d%2d%2d", &path_time.hour, &path_time.min, &path_time.sec) != 3) {
    fprintf(stderr, "ERROR - %s:ArchiveFileTrigger::next\n", _progName);
    fprintf(stderr, "Invalid path: %s\n", path);
    fprintf(stderr, "Cannot decode time.\n");
    return (-1);
  }

  if (!uvalid_datetime(&path_time)) {
    fprintf(stderr, "ERROR - %s:ArchiveFileTrigger::next\n", _progName);
    fprintf(stderr, "Invalid path: %s\n", path);
    fprintf(stderr, "Invalid date/time from path name.\n");
    return (-1);
  }

  uconvert_to_utime(&path_time);

  return (path_time.unix_time);

}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//
// Realtime mode time-based derived class
//
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

RealtimeTimeTrigger::RealtimeTimeTrigger(char *prog_name, MdvMosaic_tdrp_struct *params)
  : Trigger(prog_name, params)
    
{

  // set up the initial trigger times

  time_t now = time(NULL);
  time_t startHour = (now / 3600) * 3600;
  int secsSinceHour = now - startHour;
  int nTrig = secsSinceHour / _params->time_trigger_interval;
  _prevTime = startHour + nTrig * _params->time_trigger_interval;
  _nextTime = _prevTime + _params->time_trigger_interval;

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

  _prevTime = _nextTime;
  _nextTime += _params->time_trigger_interval;
  
  return (_prevTime);

}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//
// Realtime mode file-based derived class
//
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

RealtimeFileTrigger::RealtimeFileTrigger(char *prog_name, MdvMosaic_tdrp_struct *params)
  : Trigger(prog_name, params)

{

  // init latest data handle
  LDATA_init_handle(&_lData, _progName, _params->debug);
  
}

/////////////
// destructor

RealtimeFileTrigger::~RealtimeFileTrigger()

{

  // free up latest data info handle
  LDATA_free_handle(&_lData);

}

////////////////////////////////////////
// get next time - returns -1 on failure

time_t RealtimeFileTrigger::next()

{

  // wait for change in latest info,
  // sleep 1 second between tries.

  LDATA_info_read_blocking(&_lData, _params->input_dirs.val[0],
			   _params->trigger_time_margin, 1000,
			   PMU_auto_register);

  return (_lData.ltime.unix_time);

}



