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
//   $Id: DsOneTimeTrigger.cc,v 1.5 2016/03/03 18:06:33 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DsOneTimeTrigger.cc: Class implementing a DsTrigger that processes
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

#include <dsdata/DsOneTimeTrigger.hh>
using namespace std;



/**********************************************************************
 * Constructors
 */

DsOneTimeTrigger::DsOneTimeTrigger() :
  DsTrigger(TYPE_TIME_TRIGGER),
  _objectInitialized(false)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

DsOneTimeTrigger::~DsOneTimeTrigger()
{
  // Do nothing
}


/**********************************************************************
 * init() - Initialize the object.  The object must be initialized
 *          before it can be used.
 *
 * issue_time: the issue time to use.
 *
 * Returns 0 on success, -1 on error.
 *
 * Use getErrStr() for error message.
 */

int DsOneTimeTrigger::init(const time_t issue_time)
{
  // record issue time

  _itime  = issue_time;

  // set end of data flag to false.

  _endOneTime = false;

  _objectInitialized = true;
  
  return 0;
}


/**********************************************************************
 * next() - Get the next trigger and set the triggerInfo accordingly
 *
 * Returns 0 upon success, -1 upon failure.
 */

int DsOneTimeTrigger::next()
{
  const string method_name = "DsOneTimeTrigger::next()";
  
  assert(_objectInitialized);
  
  time_t issueTime;
  string filePath;
  
  // clear out the old stuff

  _clearErrStr();
  _triggerInfo.clear();

  // Check for end of trigger data

  if (endOfData())
    return -1;

  issueTime = _itime;

  // Set the trigger info

  _triggerInfo.setIssueTime( issueTime );

  // set end of data flag for this mode.

  _endOneTime = true;

  return(0);
}


/**********************************************************************
 * endOfData() - Check whether we are at the end of the data.
 */

bool DsOneTimeTrigger::endOfData() const
{
  assert(_objectInitialized);
  
  if (_endOneTime == true)
    return true;

  return false;
}


/**********************************************************************
 * reset() - Reset to start of data list
 */

void DsOneTimeTrigger::reset()
{
  assert(_objectInitialized);
  
  _endOneTime = false;
}
