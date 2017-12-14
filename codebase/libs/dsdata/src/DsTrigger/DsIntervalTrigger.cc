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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2017/12/14 15:47:01 $
//   $Id: DsIntervalTrigger.cc,v 1.12 2017/12/14 15:47:01 dixon Exp $
//   $Revision: 1.12 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DsIntervalTrigger.cc: Class implementing a DsTrigger that processes
 *                      one data time.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2000
 *
 * Nancy Rehak
 *
 * Refactored from code by Sue Dettling
 *
 *********************************************************************/

#include <dsdata/DsIntervalTrigger.hh>
#include <toolsa/DateTime.hh>
#include <unistd.h>
using namespace std;


/**********************************************************************
 * Constructors
 */

DsIntervalTrigger::DsIntervalTrigger() :
  DsTrigger(TYPE_TIME_TRIGGER),
  _objectInitialized(false),
  _intervalSecs(300),
  _sleepInterval(1)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

DsIntervalTrigger::~DsIntervalTrigger()
{
  // Do nothing
}


/**********************************************************************
 * init() - Initialize the object.  The object must be initialized
 *          before it can be used.  This initialization method is for
 *          realtime mode.
 *
 * interval_secs: the trigger interval in seconds.
 *
 * start_secs: the number of seconds after the hour to start the
 *             intervals.
 *
 * sleep_interval: the number of seconds to sleep when waiting
 *                 for the trigger time.  The heartbeat_func
 *                 will be called before each sleep interval.
 *
 * heartbeat_func: the function to call before each sleep interval
 *                 to cause the appropriate heartbeat action.
 *
 * The first trigger will be the next multiple of interval_secs
 * after start_secs after the last even hour.
 *
 * Returns 0 on success, -1 on error.
 *
 * Use getErrStr() for error message.
 */

int DsIntervalTrigger::init(const int interval_secs,
			    const int start_secs,
			    const int sleep_interval,
			    const heartbeat_func_t heartbeat_func)
{
  // Save the interval values

  _intervalSecs = interval_secs;
  
  // Save the mode

  _mode = REALTIME_MODE;
  
  // Save the heartbeat function

  _heartbeatFunc = heartbeat_func;
  _sleepInterval = sleep_interval;
  
  // Calculate the next trigger time

  DateTime current_time(time(0));
  DateTime trigger_start(current_time.getYear(), current_time.getMonth(),
			 current_time.getDay(), current_time.getHour(),
			 0, 0);

  _nextTriggerTime = trigger_start.utime() + start_secs;

  while (_nextTriggerTime < current_time.utime())
    _nextTriggerTime += _intervalSecs;
  
  _objectInitialized = true;
  
  return 0;
}


/**********************************************************************
 * init() - Initialize the object.  The object must be initialized
 *          before it can be used.  This initialization method is for
 *          processing past data.
 *
 * interval_secs: the trigger interval in seconds.
 *
 * start_time: the first trigger time to use.
 *
 * end_time: the last trigger time to use.
 *
 * Returns 0 on success, -1 on error.
 *
 * Use getErrStr() for error message.
 */

int DsIntervalTrigger::init(const int interval_secs,
			    const time_t start_time,
			    const time_t end_time)
{
  // Save the interval values

  _intervalSecs = interval_secs;
  _startTime = start_time;
  _endTime = end_time;
  
  // Save the mode

  _mode = ARCHIVE_MODE;
  
  // Calculate the next trigger time

  _nextTriggerTime = start_time;
  
  _objectInitialized = true;
  
  return 0;
}


/**********************************************************************
 * next() - Get the next trigger and set the triggerInfo accordingly
 *
 * Returns 0 upon success, -1 upon failure.
 */

int DsIntervalTrigger::next()
{
  const string method_name = "DsIntervalTrigger::next()";
  
  assert(_objectInitialized);
  
  // clear out the old stuff

  _clearErrStr();
  _triggerInfo.clear();

  // Get the next trigger time depending on the mode

  int status = 0;
  
  switch (_mode)
  {
  case REALTIME_MODE :
    status = _nextRealtime();
    break;
    
  case ARCHIVE_MODE :
    status = _nextArchive();
    break;
  } /* endswitch - _mode */
  
  // Set the trigger info

  _triggerInfo.setIssueTime(_nextTriggerTime);

  _nextTriggerTime += _intervalSecs;
  
  return status;
}


/**********************************************************************
 * endOfData() - Check whether we are at the end of the data.
 */

bool DsIntervalTrigger::endOfData() const
{
  assert(_objectInitialized);
  
  switch (_mode)
  {
  case REALTIME_MODE :
    return false;
    
  case ARCHIVE_MODE :
    return (_nextTriggerTime > _endTime);
  } /* endswitch - _mode */
  
  return false;
}


/**********************************************************************
 * reset() - Reset to start of data list
 */

void DsIntervalTrigger::reset()
{
  assert(_objectInitialized);

  switch (_mode)
  {
  case REALTIME_MODE :
    break;
    
  case ARCHIVE_MODE :
    _nextTriggerTime = _startTime;
    break;
  } /* endswitch - _mode */
  
}


/**********************************************************************
 * PRIVATE METHODS
 **********************************************************************/

/**********************************************************************
 * _nextArchive() - Figure out the next trigger time in archive mode.
 *
 * Returns 0 upon success, -1 upon failure.
 */

int DsIntervalTrigger::_nextArchive()
{
  const string method_name = "DsIntervalTrigger::_nextArchive()";
  
  // If we are past the end time, then we need to return failure

  if (_nextTriggerTime > _endTime)
    return -1;
  
  return 0;
}


/**********************************************************************
 * _nextRealtime() - Wait until the next trigger time.
 *
 * Returns 0 upon success, -1 upon failure.
 */

int DsIntervalTrigger::_nextRealtime()
{
  const string method_name = "DsIntervalTrigger::_nextRealtime()";
  
  // Calculate the next trigger time

  time_t current_time = time(0);
  
  while (_nextTriggerTime < current_time)
    _nextTriggerTime += _intervalSecs;
  
  // Wait for the next trigger

  while (current_time < _nextTriggerTime)
  {
    if (_heartbeatFunc != 0)
      _heartbeatFunc("Waiting for trigger time");
    
    sleep(_sleepInterval);

    current_time = time(0);
  }
  
  return 0;
}
