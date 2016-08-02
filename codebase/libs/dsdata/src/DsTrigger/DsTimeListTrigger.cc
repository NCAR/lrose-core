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
//   $Id: DsTimeListTrigger.cc,v 1.9 2016/03/03 18:06:33 dixon Exp $
//   $Revision: 1.9 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DsTimeListTrigger.cc: Class implementing a DsTrigger based on a
 *                       list of data times.
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

#include <dsdata/DsTimeListTrigger.hh>
#include <dsdata/MdvTimeListHandler.hh>
#include <dsdata/SpdbTimeListHandler.hh>
using namespace std;


/**********************************************************************
 * Constructors
 */

DsTimeListTrigger::DsTimeListTrigger() :
  DsTrigger(TYPE_TIME_TRIGGER),
  _objectInitialized(false),
  _timeListHandler(NULL)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

DsTimeListTrigger::~DsTimeListTrigger()
{
  if (_timeListHandler) {
    delete _timeListHandler;
  }
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

int DsTimeListTrigger::init(const string &url, 
			    const time_t start_time, 
			    const time_t end_time)
{
  const string method_name = "DsTimeListTrigger::init()";
  
  _clearErrStr();

  // the timeList approach is only currently supported for mdv protocol

  DsURL ds_url(url);
  string protocol = ds_url.getProtocol();

  if (protocol.size() == 0) {
    // if no protocol, prepend the likely protocol based on
    // information from the url
    if (url.find("/mdv/", 0) != string::npos) {
      protocol = "mdvp";
    } else if (url.find("/spdb/", 0) != string::npos) {
      protocol = "spdbp";
    }
  }
  
  if (protocol == "mdvp")
  {
    MdvTimeListHandler *time_list_handler =
      new MdvTimeListHandler(url, start_time, end_time);

    if (!time_list_handler->init())
    {
      _errStr = "ERROR - " + method_name + "\n";
      _errStr += "Error creating MdvTimeListHandler object\n";
      _errStr += time_list_handler->getErrStr();
      
      delete time_list_handler;
      
      return -1;
    }
    
    _timeListHandler = time_list_handler;
  }
  else if (protocol == "spdbp")
  {
    SpdbTimeListHandler *time_list_handler =
      new SpdbTimeListHandler(url, start_time, end_time);

    if (!time_list_handler->init())
    {
      _errStr = "ERROR - " + method_name + "\n";
      _errStr += "Error creating SpdbTimeListHandler object\n";
      _errStr += time_list_handler->getErrStr();
      
      delete time_list_handler;
      
      return -1;
    }
    
    _timeListHandler = time_list_handler;
  }
  else
  {
    _errStr = "ERROR - " + method_name + "\n";
    _errStr += "  DsTimeListTrigger does not support protocol: '" +
      protocol + "'\n";
    _errStr += "  Please use a fully-qualified URL\n";
    _errStr += "  Should start with mdvp:://localhost::... or spdbp:://localhost:: ...\n";
    return -1;
  }

  _objectInitialized = true;
  
  return 0;
}


/**********************************************************************
 * next() - Get the next trigger and set the triggerInfo accordingly
 *
 * Returns 0 upon success, -1 upon failure.
 */

int DsTimeListTrigger::next()
{
  const string method_name = "DsTimeListTrigger::next()";
  
  assert(_objectInitialized);
  
  // clear out the old stuff

  _clearErrStr();
  _triggerInfo.clear();

  // Get the next issue time

  time_t issue_time = _timeListHandler->next();
  
  if (issue_time < 0)
  {
    _errStr = "ERROR - " + method_name + "\n";
    _errStr += _timeListHandler->getErrStr();

    return -1;
  }
  else
  {
    // Set the trigger info

    _triggerInfo.setIssueTime(issue_time);
  }

  return 0;
}


/**********************************************************************
 * endOfData() - Check whether we are at the end of the data.
 */

bool DsTimeListTrigger::endOfData() const
{
  assert(_objectInitialized);
  
  return _timeListHandler->endOfData();
}


/**********************************************************************
 * reset() - Reset to start of data list
 */

void DsTimeListTrigger::reset()
{
  assert(_objectInitialized);
  
  _timeListHandler->reset();
}
