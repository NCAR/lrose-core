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

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 18:06:33 $
 *   $Id: DsIntervalTrigger.hh,v 1.5 2016/03/03 18:06:33 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DsIntervalTrigger.hh: Class implementing a DsTrigger that processes
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
 * ONE_TIME_MODE:
 *      setOneTime passes in an issue time. 
 *      This time is returned in a DateTime object when nextIssueTime is called.
 *       
 *      nextFile() is not valid for this mode.
 *
 *      endOfData returns true when the data after the first call to nextIssueTime.
 ************************************************************************/

#ifndef DsIntervalTrigger_HH
#define DsIntervalTrigger_HH

/*
 **************************** includes **********************************
 */

#include <string>
#include <vector>
#include <cassert>

#include <dsdata/DsTrigger.hh>
using namespace std;


/*
 ************************* class definitions ****************************
 */

class DsIntervalTrigger : public DsTrigger
{

public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  DsIntervalTrigger();


  /**********************************************************************
   * Destructor
   */

  virtual ~DsIntervalTrigger();


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

  int init(const int interval_secs,
	   const int start_secs = 0,
	   const int sleep_interval = 1,
	   const heartbeat_func_t heartbeat_func = NULL);
  

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

  int init(const int interval_secs,
	   const time_t start_time,
	   const time_t end_time);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**********************************************************************
   * next() - Get the next trigger and set the triggerInfo accordingly
   *
   * Returns 0 upon success, -1 upon failure.
   */

  int next();
  

  /**********************************************************************
   * endOfData() - Check whether we are at the end of the data.
   */

  bool endOfData() const;
  

  /**********************************************************************
   * reset() - Reset to start of data list
   */

  void reset();
  

private:

  //////////////////
  // Private types //
  //////////////////

  typedef enum
  {
    REALTIME_MODE,
    ARCHIVE_MODE
  } mode_t;
  

  /////////////////////
  // Private members //
  /////////////////////

  bool _objectInitialized;
  
  mode_t _mode;
  
  int _intervalSecs;
  time_t _nextTriggerTime;

  // REALTIME_MODE members

  int _sleepInterval;
  heartbeat_func_t _heartbeatFunc;

  // ARCHIVE_MODE members

  time_t _startTime;
  time_t _endTime;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /**********************************************************************
   * _nextArchive() - Figure out the next trigger time in archive mode.
   *
   * Returns 0 upon success, -1 upon failure.
   */

  int _nextArchive();
  
  /**********************************************************************
   * _nextRealtime() - Wait until the next trigger time.
   *
   * Returns 0 upon success, -1 upon failure.
   */

  int _nextRealtime();
  
};

#endif /* DsIntervalTrigger_HH */


