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
//   $Id: DsSpecificGenLdataTrigger.cc,v 1.3 2016/03/03 18:06:33 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DsSpecificGenLdataTrigger: Class implementing a DsTrigger that issues
 *                            a trigger whenever a new forecast file for a
 *                            specific generation time is detected in the
 *                            Ldata information.  This trigger can only be
 *                            used with MDV URLs pointing to data in a RAP
 *                            forecast-style directory structure.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2010
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <algorithm>

#include <dsdata/DsSpecificGenLdataTrigger.hh>

using namespace std;


/**********************************************************************
 * Constructors
 */

DsSpecificGenLdataTrigger::DsSpecificGenLdataTrigger() :
  DsTrigger(TYPE_TIME_TRIGGER),
  _objectInitialized(false)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

DsSpecificGenLdataTrigger::~DsSpecificGenLdataTrigger()
{
  // Do nothing
}


/**********************************************************************
 * init() - Initialize the object.  The object must be initialized
 *          before it can be used.
 *
 * url: URL to watch.
 *
 * gen_times: list of generation times, in seconds from 0:00, to use for
 *            triggering.  If files are received with generation
 *            times not in this list, they will not cause a trigger event.
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

int DsSpecificGenLdataTrigger::init(const string &url,
				     const vector< int > gen_times,
				     const int max_valid_age,
				     const heartbeat_func_t heartbeat_func,
				     const int delay_msec)
{
  const string method_name = "DsSpecificGenLdataTrigger::init()";
  
  _clearErrStr();

  if (_ldataTrigger.init(url, max_valid_age, heartbeat_func, delay_msec) != 0)
  {
    _errStr = "ERROR - " + method_name + "\n";
    _errStr += _ldataTrigger.getErrStr();

    return -1;
  }

  // Save the triggering forecast periods

  _genTimes = gen_times;
  
  _objectInitialized = true;
  
  return 0;
}


/**********************************************************************
 * next() - Get the next trigger and set the triggerInfo accordingly
 *
 * Returns 0 upon success, -1 upon failure.
 */

int DsSpecificGenLdataTrigger::next()
{
  const string method_name = "DsSpecificGenLdataTrigger::next()";
  
  assert(_objectInitialized);
  _clearErrStr();
  
  while (true)
  {
    if (_ldataTrigger.next() != 0)
    {
      _errStr = "ERROR - " + method_name + "\n";
      _errStr += _ldataTrigger.getErrStr();
      
      return -1;
    }
    
    if (!_ldataTrigger._ldataInfo.isFcast())
    {
      _errStr = "ERROR - " + method_name + "\n";
      _errStr += "Non-forecast trigger received\n";
      
      return -1;
    }
    
    DateTime gen_time = _ldataTrigger._ldataInfo.getLatestTime();
    
    int gen_time_offset = (gen_time.getHour() * 3600) +
      (gen_time.getMin() * 60) + gen_time.getSec();
    
    if (find(_genTimes.begin(), _genTimes.end(), gen_time_offset)
	!= _genTimes.end())
    {
      _triggerInfo = _ldataTrigger._triggerInfo;
      _triggerInfo.setIssueTime(_triggerInfo.getForecastTime());
	
      return 0;
    }
    
  } /* endwhile */
  
  // We should never get here

  return -1;
}


/**********************************************************************
 * endOfData() - Check whether we are at the end of the data.
 */

bool DsSpecificGenLdataTrigger::endOfData() const
{
  assert(_objectInitialized);
  
  return false;
}


/**********************************************************************
 * reset() - Reset to start of data list
 */

void DsSpecificGenLdataTrigger::reset()
{
  assert(_objectInitialized);
  
  // Do nothing
}
