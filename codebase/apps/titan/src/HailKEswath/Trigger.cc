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

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
// Abstract base class
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
// Archive mode time-based derived class
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
// Archive mode file-based derived class
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

ArchiveFileTrigger::ArchiveFileTrigger(const string &prog_name,
				       const Params &params,
				       time_t start_time, time_t end_time)
  : Trigger(prog_name, params)

{

  _inputPath = new DsInputPath(_progName,
			       _params.debug >= Params::DEBUG_VERBOSE,
			       _params.input_hail_data_dir,
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
    fprintf(stderr, "ERROR - %s:ArchiveFileTrigger::next\n",
	    _progName.c_str());
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

  if (sscanf(dateStr, "%4d%2d%2d", &path_time.year,
	     &path_time.month, &path_time.day) != 3) {
    fprintf(stderr, "ERROR - %s:ArchiveFileTrigger::next\n",
	    _progName.c_str());
    fprintf(stderr, "Invalid path: %s\n", path);
    fprintf(stderr, "Cannot decode date.\n");
    return (-1);
  }
  
  if (sscanf(timeStr, "%2d%2d%2d", &path_time.hour, &path_time.min, &path_time.sec) != 3) {
    fprintf(stderr, "ERROR - %s:ArchiveFileTrigger::next\n",
	    _progName.c_str());
    fprintf(stderr, "Invalid path: %s\n", path);
    fprintf(stderr, "Cannot decode time.\n");
    return (-1);
  }

  if (!uvalid_datetime(&path_time)) {
    fprintf(stderr, "ERROR - %s:ArchiveFileTrigger::next\n",
	    _progName.c_str());
    fprintf(stderr, "Invalid path: %s\n", path);
    fprintf(stderr, "Invalid date/time from path name.\n");
    return (-1);
  }

  uconvert_to_utime(&path_time);

  return (path_time.unix_time);

}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
// Realtime mode time-based derived class
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

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

  _prevTime = _nextTime;
  _nextTime += _params.time_trigger_interval;
  
  return (_prevTime);

}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
// Realtime mode file-based derived class
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

RealtimeFileTrigger::RealtimeFileTrigger(const string &prog_name,
					 const Params &params)
  : Trigger(prog_name, params),
    _ldata(_params.input_hail_data_dir,
	   _params.debug >= Params::DEBUG_VERBOSE)


{
  
}

/////////////
// destructor

RealtimeFileTrigger::~RealtimeFileTrigger()

{

}

////////////////////////////////////////
// get next time - returns -1 on failure

time_t RealtimeFileTrigger::next()

{

  // wait for change in latest info,
  // sleep 1 second between tries.

  _ldata.readBlocking(_params.trigger_time_margin, 1000,
		      PMU_auto_register);
  
  return (_ldata.getLatestTime());

}






