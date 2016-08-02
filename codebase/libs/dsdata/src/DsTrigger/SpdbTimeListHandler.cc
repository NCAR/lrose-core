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
//   $Id: SpdbTimeListHandler.cc,v 1.4 2016/03/03 18:06:33 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * SpdbTimeListHandler: Class which handles the processing of a time
 *                     list based on an MDV URL.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2002
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <dsdata/SpdbTimeListHandler.hh>
#include <Spdb/DsSpdb.hh>
using namespace std;


/**********************************************************************
 * Constructors
 */

SpdbTimeListHandler::SpdbTimeListHandler(const string &url,
				       const time_t start_time,
				       const time_t end_time) :
  TimeListHandler(url, start_time, end_time)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

SpdbTimeListHandler::~SpdbTimeListHandler()
{
  // Do nothing
}


/**********************************************************************
 * init() - Initialize the object.  The object must be initialized
 *          before it can be used.
 *
 * Returns true on success, false on error.
 * 
 * Use getErrStr() for error message.
 */

bool SpdbTimeListHandler::init()
{
  const string method_name = "SpdbTimeListHandler::init()";
  
  _clearErrStr();

  // Compile the time list from the database

  DsSpdb spdb_handle;
  
  if (spdb_handle.compileTimeList(_url, _startTime, _endTime) != 0)
  {
    _errStr = "ERROR - " + method_name + "\n";
    _errStr += spdb_handle.getErrStr();
    
    return false;
  }
  
  // Save pointers into the compiled time list for use in the other
  // methods.

  _timeList = spdb_handle.getTimeList();
  _nextTime = _timeList.begin();
  
  return true;
}


/**********************************************************************
 * next() - Get the next trigger and set the triggerInfo accordingly
 *
 * Returns the next data time upon success, -1 upon failure or if there
 * are no more data times to process.
 */

time_t SpdbTimeListHandler::next()
{
  const string method_name = "SpdbTimeListHandler::next()";
  
  time_t issue_time;
  
  // clear out the old stuff

  _clearErrStr();

  // Check for end of trigger data

  if (endOfData())
    return -1;

  // Retrieve the issue time and increment the time iterator so that we
  // are ready for the next time period.

  issue_time = *_nextTime;
  
  ++_nextTime;
  
  return issue_time;
}


/**********************************************************************
 * endOfData() - Check whether we are at the end of the data.
 */

bool SpdbTimeListHandler::endOfData() const
{
  return _nextTime == _timeList.end();
}


/**********************************************************************
 * reset() - Reset to start of data list
 */

void SpdbTimeListHandler::reset()
{
  _nextTime = _timeList.begin();
}
