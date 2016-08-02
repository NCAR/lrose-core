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
//   $Id: DsLdataTrigger.cc,v 1.12 2016/03/03 18:06:33 dixon Exp $
//   $Revision: 1.12 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DsLdataTrigger.cc: Class implementing a DsTrigger based on Ldata
 *                    information.
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

#include <dsdata/DsLdataTrigger.hh>
using namespace std;



/**********************************************************************
 * Constructors
 */

DsLdataTrigger::DsLdataTrigger() :
  DsTrigger(TYPE_TIME_TRIGGER),
  _objectInitialized(false)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

DsLdataTrigger::~DsLdataTrigger()
{
  // Do nothing
}


/**********************************************************************
 * init() - Initialize the object.  The object must be initialized
 *          before it can be used.
 *
 * url: URL to watch.
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
 *              If this value is < 0, the next() method won't block but
 *              will instead return the latest time in the ldata file.
 *
 * Returns 0 on success, -1 on error.
 *
 * Use getErrStr() for error message.
 */

int DsLdataTrigger::init(const string &url,
			 const int max_valid_age,
			 const heartbeat_func_t heartbeat_func,
			 const int delay_msec)
{
  const string method_name = "DsLdataTrigger::init()";
  
  _clearErrStr();

  // Record url of data.

  _dsUrl = url;
  
  // Record read characteristics

  _max_valid_age = max_valid_age;
  _heartbeat_func = heartbeat_func;
  _delay_msec = delay_msec;

  // Initialize the URL in the LdataInfo object

  int iret = _ldataInfo.setDirFromUrl( _dsUrl );

  if (iret)
  {
    _errStr = "ERROR - " + method_name + "\n"
      " LdataInfo failed setting dir from url\n";

    return -1;
  }

  _objectInitialized = true;
  
  return 0;
}


/**********************************************************************
 * next() - Get the next trigger and set the triggerInfo accordingly
 *
 * If delay_msec was set to a negative value, this method doesn't block.
 * Otherwise, this method blocks until the ldata information is updated.
 *
 * Returns 0 upon success, -1 upon failure.
 */

int DsLdataTrigger::next()
{
  const string method_name = "DsLdataTrigger::next()";
  
  assert(_objectInitialized);
  
  time_t issueTime;
  string filePath;
	  
  // clear out the old stuff

  _clearErrStr();
  _triggerInfo.clear();

  // Check for end of trigger data

  if (endOfData())
    return -1;

  // Read from ldataInfo and set the trigger info accordlingly
    
  if (_delay_msec < 0)
  {
    if (_ldataInfo.read(_max_valid_age) != 0)
    {
      _errStr = "ERROR - " + method_name + "\n"
	" Error reading ldata info file\n";
      return -1;
    }
  }
  else
  {
    _ldataInfo.readBlocking(_max_valid_age, 
			    _delay_msec, 
			    _heartbeat_func);
  }
  
  issueTime = _ldataInfo.getLatestTime();
  
  _triggerInfo.setIssueTime(issueTime);
  if (_ldataInfo.isFcast())
    _triggerInfo.setForecastTime(issueTime + _ldataInfo.getLeadTime());
  _triggerInfo.setFilePath(_ldataInfo.getDataPath());
  
  return 0;
}


/**********************************************************************
 * endOfData() - Check whether we are at the end of the data.
 */

bool DsLdataTrigger::endOfData() const
{
  assert(_objectInitialized);
  
  return false;
}


/**********************************************************************
 * reset() - Reset to start of data list
 */

void DsLdataTrigger::reset()
{
  assert(_objectInitialized);
  
  // Do nothing
}
