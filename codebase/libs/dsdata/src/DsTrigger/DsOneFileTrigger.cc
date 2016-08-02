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
//   $Id: DsOneFileTrigger.cc,v 1.4 2016/03/03 18:06:33 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DsOneFileTrigger: Class implementing a DsTrigger that triggers
 *                   off of a single file in realtime.  The trigger
 *                   occurs when the file has been updated and hasn't
 *                   changed for a given amount of time.  This trigger
 *                   is used when the input file is overwritten in
 *                   realtime or when the input file is pointed to by
 *                   a symbolic link that is updated when new data is
 *                   received.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2008
 *
 * Nancy Rehak
 *
 * Refactored from code by Sue Dettling
 *
 *********************************************************************/

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <dsdata/DsOneFileTrigger.hh>

using namespace std;

/**********************************************************************
 * Constructors
 */

DsOneFileTrigger::DsOneFileTrigger() :
  DsTrigger(TYPE_FILE_TRIGGER),
  _objectInitialized(false),
  _lastFileTime(0)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

DsOneFileTrigger::~DsOneFileTrigger()
{
  // Do nothing
}


/**********************************************************************
 * init() - Initialize the object.  The object must be initialized
 *          before it can be used.
 *
 * file_path: Full path of input file to watch
 *
 * sleep_secs: Number of seconds the object will sleep between checks
 *             of the status of the input file.
 *
 * no_change_secs: Number of seconds that the file must be unchanged before
 *                 the trigger occurs.
 *
 * heartbeat_func: pointer to heartbeat_func.
 *                 If NULL this is ignored.
 *                 If non-NULL, this is called once per delay_msecs while
 *                 the routine is polling for new data.
 *
 * Returns 0 on success, -1 on error.
 *
 * Use getErrStr() for error message.
 */

int DsOneFileTrigger::init(const string &file_path,
			   const int sleep_secs,
			   const int no_change_secs,
			   const heartbeat_func_t heartbeat_func)
{
  const string method_name = "DsOneFileTrigger::init()";
  
  _clearErrStr();

  // Record the file path and read characteristics

  _filePath = file_path;
  _sleepSecs = sleep_secs;
  _noChangeSecs = no_change_secs;
  _heartbeat_func = heartbeat_func;

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

int DsOneFileTrigger::next()
{
  const string method_name = "DsOneFileTrigger::next()";
  
  assert(_objectInitialized);
  
  // clear out the old stuff

  _clearErrStr();
  _triggerInfo.clear();

  // Wait for the file to update

  struct stat file_stat;
  
  bool first_time = true;
  bool file_updated = false;
  
  while (!file_updated)
  {
    if (first_time)
      first_time = false;
    else
      sleep(_sleepSecs);
    
    if (stat(_filePath.c_str(), &file_stat) != 0)
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Error stating realtime file: " << _filePath << endl;
      cerr << "Will try again in " << _sleepSecs << " seconds" << endl;
      
      continue;
    }

    time_t curr_file_time = file_stat.st_mtime;
    
    if (curr_file_time <= _lastFileTime)
      continue;
    
    // If we get here, the file has updated but we want to wait a few
    // seconds before actually processing it to make sure that it's
    // complete.

    time_t curr_time = time(0);
    
    if (curr_file_time + _noChangeSecs < curr_time)
    {
      _lastFileTime = curr_file_time;
      file_updated = true;
    }
  }

  // Update the trigger information

  _triggerInfo.setIssueTime(0);
  _triggerInfo.setForecastTime(0);
  _triggerInfo.setFilePath(_filePath);
  
  return 0;
}


/**********************************************************************
 * endOfData() - Check whether we are at the end of the data.
 */

bool DsOneFileTrigger::endOfData() const
{
  assert(_objectInitialized);
  
  return false;
}


/**********************************************************************
 * reset() - Reset to start of data list
 */

void DsOneFileTrigger::reset()
{
  assert(_objectInitialized);
  
  // Do nothing
}
