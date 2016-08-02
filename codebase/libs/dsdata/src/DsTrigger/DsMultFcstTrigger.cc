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
//   $Date: 2016/03/03 18:06:33 $
//   $Id: DsMultFcstTrigger.cc,v 1.4 2016/03/03 18:06:33 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
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
 *********************************************************************/

#include <algorithm>

#include <dsdata/DsMultFcstTrigger.hh>
#include <Mdv/DsMdvx.hh>

using namespace std;


/**********************************************************************
 * Constructors
 */

DsMultFcstTrigger::DsMultFcstTrigger() :
  DsTrigger(TYPE_TIME_TRIGGER),
  _objectInitialized(false),
  _mode(LDATA_MODE)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

DsMultFcstTrigger::~DsMultFcstTrigger()
{
  // Do nothing
}


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

int DsMultFcstTrigger::initLdataMode(const string &url,
				     const vector< int > fcst_periods,
				     const int max_valid_age,
				     const heartbeat_func_t heartbeat_func,
				     const int delay_msec)
{
  const string method_name = "DsMultFcstTrigger::initLdataMode()";
  
  _clearErrStr();

  _mode = LDATA_MODE;
  
  if (_ldataTrigger.init(url, max_valid_age, heartbeat_func, delay_msec) != 0)
  {
    _errStr = "ERROR - " + method_name + "\n";
    _errStr += _ldataTrigger.getErrStr();

    return -1;
  }

  // Save the triggering forecast periods

  _fcstPeriods = fcst_periods;
  
  _objectInitialized = true;
  
  return 0;
}


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

int DsMultFcstTrigger::initTimeListMode(const string &url,
					const vector< int > fcst_periods,
					const time_t start_time,
					const time_t end_time)
{
  const string method_name = "DsMultFcstTrigger::initTimeListMode()";
  
  _clearErrStr();

  _mode = TIME_LIST_MODE;
  _timeListNextIndex = 0;
  
  // Get the trigger times

  DsMdvx time_list;
  
  time_list.clearTimeListMode();
  time_list.setTimeListModeGenPlusForecasts(url, start_time, end_time);
  
  if (time_list.compileTimeList() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error compiling time list" << endl;
    cerr << time_list.getErrStr() << endl;
    
    return -1;
  }
  
  const vector< time_t > gen_times = time_list.getTimeList();
  const vector< vector< time_t > > all_fcst_times =
    time_list.getForecastTimesArray();
  
  if (gen_times.size() != all_fcst_times.size())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Gen times array size different from fcst time array size" << endl;
    cerr << "num gen times: " << gen_times.size() << endl;
    cerr << "num fcst times: " << all_fcst_times.size() << endl;
    
    return -1;
  }
  
  for (unsigned int i = 0; i < gen_times.size(); ++i)
  {
    const vector< time_t > fcst_times = all_fcst_times[i];
    
    bool missing_lead_time = false;
    vector< int >::const_iterator fcst_iter;
    
    for (fcst_iter = _fcstPeriods.begin(); fcst_iter != _fcstPeriods.end();
	 ++fcst_iter)
    {
      if (find(fcst_times.begin(), fcst_times.end(), *fcst_iter)
	  == fcst_times.end())
      {
	missing_lead_time = true;
	break;
      }

    } /* endfor - fcst_iter */

    if (!missing_lead_time)
      _timeList.push_back(gen_times[i]);
    
  } /* endfor - i */
  
  // Save the triggering forecast periods

  _fcstPeriods = fcst_periods;
  
  _objectInitialized = true;
  
  return 0;
}


/**********************************************************************
 * next() - Get the next trigger and set the triggerInfo accordingly
 *
 * Returns 0 upon success, -1 upon failure.
 */

int DsMultFcstTrigger::next()
{
  assert(_objectInitialized);
  _clearErrStr();
  
  switch (_mode)
  {
  case LDATA_MODE :
    return _nextLdata();
    break;
    
  case TIME_LIST_MODE :
    return _nextTimeList();
    break;
  }
  
  // We should never get here

  return -1;
}


/**********************************************************************
 * endOfData() - Check whether we are at the end of the data.
 */

bool DsMultFcstTrigger::endOfData() const
{
  assert(_objectInitialized);
  
  switch (_mode)
  {
  case LDATA_MODE :
    return false;
    
  case TIME_LIST_MODE :
    if (_timeListNextIndex < _timeList.size())
      return false;
    return true;
  
  }
  
  // Should never get here

  return false;
}


/**********************************************************************
 * reset() - Reset to start of data list
 */

void DsMultFcstTrigger::reset()
{
  assert(_objectInitialized);
  
  switch (_mode)
  {
  case LDATA_MODE :
    // Do nothing
    break;
    
  case TIME_LIST_MODE :
    _timeListNextIndex = 0;
    break;
  }
}


/**********************************************************************
 * _nextLdata() - Get the next trigger in latest data mode and set the
 *                triggerInfo accordingly
 *
 * Returns 0 upon success, -1 upon failure.
 */

int DsMultFcstTrigger::_nextLdata()
{
  const string method_name = "DsMultFcstTrigger::_nextLdata()";
  
  while (true)
  {
    // Get the next ldata trigger

    if (_ldataTrigger.next() != 0)
    {
      _errStr = "ERROR - " + method_name + "\n";
      _errStr += _ldataTrigger.getErrStr();
      
      return -1;
    }
    
    // Check the forecast lead time.  If it isn't in our list, wait for
    // another trigger.

    _triggerInfo = _ldataTrigger._triggerInfo;
    int fcst_secs = _ldataTrigger._ldataInfo.getLeadTime();
    
    if (find(_fcstPeriods.begin(), _fcstPeriods.end(), fcst_secs)
	== _fcstPeriods.end())
      continue;
    
    // The forecast lead time is in our list.  If we only have one lead time
    // in our list, then we have satisfied our requirements and can return
    // the trigger

    if (_fcstPeriods.size() == 1)
      return 0;
    
    // If we have a list of lead times, check our received triggers to see
    // if we've satisfied our requirements.

    time_t gen_time = _triggerInfo.getIssueTime();
    map< time_t, vector< int > >::iterator received_triggers;
    
    if ((received_triggers = _receivedTriggers.find(gen_time))
	== _receivedTriggers.end())
    {
      vector< int > lead_times;
      lead_times.push_back(fcst_secs);
      
      _receivedTriggers[gen_time] = lead_times;
      
      continue;
    }
    
    vector< int > received_lead_times = (*received_triggers).second;
    
    received_lead_times.push_back(fcst_secs);
    
    bool missing_lead_time = false;
    vector< int >::const_iterator fcst_iter;
    
    for (fcst_iter = _fcstPeriods.begin(); fcst_iter != _fcstPeriods.end();
	 ++fcst_iter)
    {
      if (find(received_lead_times.begin(), received_lead_times.end(),
	       *fcst_iter) == received_lead_times.end())
      {
	missing_lead_time = true;
	break;
      }
    } /* endfor - fcst_iter */

    if (missing_lead_time)
    {
      _receivedTriggers[gen_time] = received_lead_times;
      continue;
    }
    else
    {
      _receivedTriggers.erase(received_triggers);
      return 0;
    }
    
  } /* endwhile */
  
  // We should never get here

  return -1;
}


/**********************************************************************
 * _nextTimeList() - Get the next trigger in time list mode and set the
 * triggerInfo accordingly
 *
 * Returns 0 upon success, -1 upon failure.
 */

int DsMultFcstTrigger::_nextTimeList()
{
  const string method_name = "DsMultFcstTrigger::_nextLdata()";
  
  if (endOfData())
    return -1;
  
  time_t trigger_time = _timeList[_timeListNextIndex++];
  
  _triggerInfo.setInfo(trigger_time, trigger_time, "", "");

  return 0;
}
