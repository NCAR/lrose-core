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
//   $Id: DsTrigger.cc,v 1.13 2017/12/14 15:47:01 dixon Exp $
//   $Revision: 1.13 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DsTrigger.cc: Abstract base class for providing data set time
 *               information to a client.  
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

#include <dsdata/DsTrigger.hh>
using namespace std;



/**********************************************************************
 * Constructors
 */

DsTrigger::DsTrigger(const trigger_type_t type) :
        _type(type),
        _debug(false),
        _verbose(false)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

DsTrigger::~DsTrigger()
{
  // Do nothing
}

/**********************************************************************
 * next() - Get the next trigger and set the triggerInfo accordingly
 *
 * Returns 0 upon success, -1 upon failure.
 */

int DsTrigger::next(TriggerInfo& clientTriggerInfo)
{
  int iret;

  iret = next();

  if (iret == 0)
    clientTriggerInfo = _triggerInfo;

  return iret;
}


/**********************************************************************
 * nextIssueTime() - Get the next issue time.
 *
 * Returns 0 upon success, -1 upon failure.
 */

int DsTrigger::nextIssueTime(DateTime& dateTime)
{
  const string method_name = "DsTrigger::nextIssueTime()";
  
  int iret;
  
  _clearErrStr();

  // First try to do a next(), then grab the resultant issue time

  iret = next();
 
  if ( iret )
  {
    return -1;
  }
  else
  {
    // record issue time.

    dateTime = _triggerInfo.getIssueTime();

    // An undefined issue time is considered an error

    if ( dateTime == DateTime::NEVER )
    {
      _errStr = method_name + " - Undefined issue time.\n";

      return -1;
    }
  }

  return(0);
}
