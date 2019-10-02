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
//   $Author: dave $
//   $Locker:  $
//   $Date: 2014/08/28 18:14:52 $
//   $Id: DsSpecificFcstTimeListTrigger.cc,v 1.4 2014/08/28 18:14:52 dave Exp $
//   $Revision: 1.4 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DsSpecificFcstTimeListTrigger: Class implementing a DsTrigger that triggers
 *                                for each forecast file in a forecast directory
 *                                with a generation time between the given start
 *                                and end times and one of a specified list of
 *                                forecast lead times.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2016
 *
 * Nancy Rehak
 *
 * Refactored from code by Sue Dettling
 *
 *********************************************************************/

#include <algorithm>

#include <dsdata/DsSpecificFcstTimeListTrigger.hh>
#include <dsdata/MdvTimeListHandler.hh>
#include <dsdata/SpdbTimeListHandler.hh>

using namespace std;


/**********************************************************************
 * Constructors
 */

DsSpecificFcstTimeListTrigger::DsSpecificFcstTimeListTrigger() :
  DsTrigger(TYPE_TIME_TRIGGER),
  _objectInitialized(false),
  _nextTrigger(0)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

DsSpecificFcstTimeListTrigger::~DsSpecificFcstTimeListTrigger()
{
  // Do nothing
}


/**********************************************************************
 * init() - Initialize the object.  The object must be initialized
 *          before it can be used.
 *
 * The list will contain all available data times
 * between the start and end times for the given URL
 * with a forecast lead time (in seconds) in the specified list.
 *
 * Returns 0 on success, -1 on error.
 * 
 * Use getErrStr() for error message.
 */

int DsSpecificFcstTimeListTrigger::init(const string &url,
                                        const vector< int > &fcst_lead_times,
                                        const time_t start_time, 
                                        const time_t end_time)
{
  const string method_name = "DsSpecificFcstTimeListTrigger::init()";

  _clearErrStr();

  // Get the list of times

  DsMdvx mdvx;
  
  mdvx.setTimeListModeGenPlusForecasts(url, start_time, end_time);

  if (mdvx.compileTimeList() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error compiling time list" << endl;
    cerr << "   url: " << url << endl;
    cerr << "   start_time: " << DateTime::str(start_time) << endl;
    cerr << "   end_time: " << DateTime::str(end_time) << endl;
    
    return -1;
  }
  
  vector< time_t > gen_times = mdvx.getTimeList();
  vector< vector< time_t > > all_fcst_times = mdvx.getForecastTimesArray();
  
  if (gen_times.size() != all_fcst_times.size())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Internal program error" << endl;
    cerr << "List of gen times does not match list of forecast times" << endl;
    
    return -1;
  }

  for (size_t i = 0; i < gen_times.size(); ++i)
  {
    time_t gen_time = gen_times[i];
    
    vector< time_t > fcst_times = all_fcst_times[i];
    vector< time_t >::const_iterator fcst_time_iter;
    
    for (fcst_time_iter = fcst_times.begin(); fcst_time_iter != fcst_times.end();
	 ++fcst_time_iter)
    {
      time_t fcst_time = *fcst_time_iter;

      int lead_secs = fcst_time - gen_time;
      if (find(fcst_lead_times.begin(), fcst_lead_times.end(), lead_secs) == fcst_lead_times.end())
        continue;

      TriggerInfo trigger_info;
      trigger_info.setIssueTime(gen_time);
      trigger_info.setForecastTime(fcst_time);
      
      _triggers.push_back(trigger_info);
    }
  }
  
  _nextTrigger = 0;
  
  _objectInitialized = true;

  return 0;
}


/**********************************************************************
 * next() - Get the next trigger and set the triggerInfo accordingly
 *
 * Returns 0 upon success, -1 upon failure.
 */

int DsSpecificFcstTimeListTrigger::next()
{
  const string method_name = "DsSpecificFcstTimeListTrigger::next()";
  
  assert(_objectInitialized);
  
  // clear out the old stuff

  _clearErrStr();
  _triggerInfo.clear();

  // If we are at the end of the data, return an error

  if (endOfData())
    return -1;
  
  // Get the next trigger

  _triggerInfo = _triggers[_nextTrigger];
  _nextTrigger++;
  
  return 0;
}


/**********************************************************************
 * endOfData() - Check whether we are at the end of the data.
 */

bool DsSpecificFcstTimeListTrigger::endOfData() const
{
  assert(_objectInitialized);
  
  if (_nextTrigger >= _triggers.size())
    return true;
  
  return false;
}


/**********************************************************************
 * reset() - Reset to start of data list
 */

void DsSpecificFcstTimeListTrigger::reset()
{
  assert(_objectInitialized);
  
  _nextTrigger = 0;
}
