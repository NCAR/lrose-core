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
//   $Id: DsFcstTimeListTrigger.cc,v 1.5 2016/03/03 18:06:33 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DsFcstTimeListTrigger: Class implementing a DsTrigger that triggers
 *                        for each forecast file in a forecast directory
 *                        with a generation time between the given start
 *                        and end times.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2008
 *
 * Nancy Rehak
 *
 * Refactored from code by Sue Dettling
 *
 *********************************************************************/

#include <dsdata/DsFcstTimeListTrigger.hh>
#include <dsdata/MdvTimeListHandler.hh>
#include <dsdata/SpdbTimeListHandler.hh>

using namespace std;


/**********************************************************************
 * Constructors
 */

DsFcstTimeListTrigger::DsFcstTimeListTrigger() :
  DsTrigger(TYPE_TIME_TRIGGER),
  _objectInitialized(false),
  _nextTrigger(0)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

DsFcstTimeListTrigger::~DsFcstTimeListTrigger()
{
  // Do nothing
}


/**********************************************************************
 * init() - Initialize the object.  The object must be initialized
 *          before it can be used.
 *
 * The list will contain all available data times
 * between the start and end times for the given URL.
 *
 * Returns 0 on success, -1 on error.
 * 
 * Use getErrStr() for error message.
 */

int DsFcstTimeListTrigger::init(const string &url, 
				const time_t start_time, 
				const time_t end_time)
{
  const string method_name = "DsFcstTimeListTrigger::init()";
  
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
    
    return false;
  }
  
  vector< time_t > gen_times = mdvx.getTimeList();
  vector< vector< time_t > > all_fcst_times = mdvx.getForecastTimesArray();
  
  if (gen_times.size() != all_fcst_times.size())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Internal program error" << endl;
    cerr << "List of gen times does not match list of forecast times" << endl;
    
    return false;
  }

  for (size_t i = 0; i < gen_times.size(); ++i)
  {
    vector< time_t > fcst_times = all_fcst_times[i];
    vector< time_t >::const_iterator fcst_time;
    
    for (fcst_time = fcst_times.begin(); fcst_time != fcst_times.end();
	 ++fcst_time)
    {
      TriggerInfo trigger_info;
      trigger_info.setIssueTime(gen_times[i]);
      trigger_info.setForecastTime(*fcst_time);
      
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

int DsFcstTimeListTrigger::next()
{
  const string method_name = "DsFcstTimeListTrigger::next()";
  
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

bool DsFcstTimeListTrigger::endOfData() const
{
  assert(_objectInitialized);
  
  if (_nextTrigger >= _triggers.size())
    return true;
  
  return false;
}


/**********************************************************************
 * reset() - Reset to start of data list
 */

void DsFcstTimeListTrigger::reset()
{
  assert(_objectInitialized);
  
  _nextTrigger = 0;
}
