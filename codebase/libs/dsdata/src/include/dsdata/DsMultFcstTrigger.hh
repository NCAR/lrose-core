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
 *   $Id: DsMultFcstTrigger.hh,v 1.2 2016/03/03 18:06:33 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DsMultFcstTrigger: Class implementing a DsTrigger that issues a trigger
 *                    when all of the listed forecasts have been received
 *                    for this URL.  This trigger can only be used with MDV
 *                    URLs pointing to data in a RAP forecast-style directory
 *                    structure.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2006
 *
 * Nancy Rehak
 *
 * This trigger has several modes:
 *
 * LDATA_MODE - This mode operates in realtime.  It watches the trigger URL
 *              for new data.  When new data has been received for all of
 *              the given forecast lead times for any gen time, that gen time
 *              is returned as the trigger time.
 *
 * TIME_LIST_MODE - This mode operates on existing data.  It looks through
 *              the data at the trigger URL and returns a trigger for each
 *              gen time that has forecasts for all of the given forecast
 *              lead times.
 *
 ************************************************************************/

#ifndef DsMultFcstTrigger_HH
#define DsMultFcstTrigger_HH

#include <map>
#include <string>
#include <vector>
#include <cassert>

#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTrigger.hh>

using namespace std;


class DsMultFcstTrigger : public DsTrigger
{

public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  DsMultFcstTrigger();


  /**********************************************************************
   * Destructor
   */

  virtual ~DsMultFcstTrigger();


  /**********************************************************************
   * initLdataMode() - Initialize the object in latest data mode.  The
   *                   object must be initialized before it can be used.
   *
   * url: URL to watch.
   *
   * fcst_periods: list of forecast periods, in seconds, to use for
   *               triggering.  If files are received with forecast
   *               periods not in this list, they will not cause a
   *               trigger event.
   *
   * max_valid_age: the max valid age for data (secs)
   *                The object will not return data which has not arrived
   *                within this period. 
   *
   * heartbeat_func: pointer to heartbeat_func.
   *                 If NULL this is ignored.
   *                 If non-NULL, this is called once per delay_msecs while
   *                 the routine is polling for new data.
   *
   * delay_msecs: polling delay in millisecs.
   *              The object will sleep for this time between polling attempts.
   *
   * Returns 0 on success, -1 on error.
   *
   * Use getErrStr() for error message.
   */

  int initLdataMode(const string &url,
		    const vector< int > fcst_periods,
		    const int max_valid_age,
		    const heartbeat_func_t heartbeat_func = NULL,
		    const int delay_msec = 5000);
  

  /**********************************************************************
   * initTimeListMode() - Initialize the object in time list mode.  The
   *                      object must be initialized before it can be used.
   *
   * url: URL to watch.
   *
   * fcst_periods: list of forecast periods, in seconds, to use for
   *               triggering.  If files are received with forecast
   *               periods not in this list, they will not cause a
   *               trigger event.
   *
   * start_time: start of time list period.
   *
   * end_time: end of time list period.
   *
   * Returns 0 on success, -1 on error.
   *
   * Use getErrStr() for error message.
   */

  int initTimeListMode(const string &url,
		       const vector< int > fcst_periods,
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
  

protected:

  /////////////////////
  // Protected types //
  /////////////////////

  typedef enum
  {
    LDATA_MODE,
    TIME_LIST_MODE
  } mode_t;
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _objectInitialized;
  
  mode_t _mode;
  
  DsLdataTrigger _ldataTrigger;
  vector< int > _fcstPeriods;
  
  // Objects used in LDATA mode

  map< time_t, vector< int > > _receivedTriggers;
  
  // Objects used in TIME_LIST mode

  vector< time_t > _timeList;
  unsigned int _timeListNextIndex;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**********************************************************************
   * _nextLdata() - Get the next trigger in latest data mode and set the
   *                triggerInfo accordingly
   *
   * Returns 0 upon success, -1 upon failure.
   */

  int _nextLdata();
  

  /**********************************************************************
   * _nextTimeList() - Get the next trigger in time list mode and set the
   * triggerInfo accordingly
   *
   * Returns 0 upon success, -1 upon failure.
   */

  int _nextTimeList();
  

};

#endif /* DsMultFcstTrigger_HH */
