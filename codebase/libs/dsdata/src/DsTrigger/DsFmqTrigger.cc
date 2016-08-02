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
//   $Id: DsFmqTrigger.cc,v 1.5 2016/03/03 18:06:33 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DsFmqTrigger.cc: Class implementing a DsTrigger using an FMQ.
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

#include <dsdata/DsFmqTrigger.hh>
using namespace std;



/**********************************************************************
 * Constructors
 */

DsFmqTrigger::DsFmqTrigger() :
  DsTrigger(TYPE_TIME_TRIGGER),
  _objectInitialized(false)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

DsFmqTrigger::~DsFmqTrigger()
{
  // Do nothing
}


/**********************************************************************
 * fireTrigger() - Write issue time and process name to fmq.
 */

int DsFmqTrigger::fireTrigger(TriggerInfo& clientTriggerInfo)
{
  const string method_name = "DsFmqTrigger::fireTrigger()";
  
  assert(_objectInitialized);
  
  // Setup the trigger request by setting our internal trigger info
  // /to the client specification

  _triggerInfo = clientTriggerInfo;

  // Write the trigger Msg to the fmq

  if (_dsFmq.writeMsg(ISSUE_TIME, 0, 
		      _triggerInfo.getMsgFromInfo(),
		      _triggerInfo.getMsgLen()))
  {
    _errStr += method_name + " -  Failed to write to fmq.\n";
    
    return -1;
  }
  else
  {
    return 0;
  }
  
}


/**********************************************************************
 * init() - Initialize the object.  The object must be initialized
 *          before it can be used.
 *
 * url: URL to watch.
 *
 * procName: the process name to include in the FMQ message.
 *
 * debug: debug flag.
 *
 * mode: mode to use when opening the FMQ.
 *
 * position: opening position for the FMQ.
 *
 * compression: flag indicating whether the FMQ should be compressed.
 *
 * numSlots: number of slots in the FMQ when created.
 *
 * bufSize: buffer size of the FMQ when created.
 *
 * msecSleep: milliseconds to sleep.
 *
 * msgLog: message log to use.
 *
 * Returns 0 on success, -1 on error.
 *
 * Use getErrStr() for error message.
 */

int DsFmqTrigger::init(const string &url, 
		       const string &procName, 
		       const bool debug,
		       const DsFmq::openMode mode, 
		       const DsFmq::openPosition position,
		       const bool compression,
		       const size_t numSlots,
		       const size_t bufSize,
		       const size_t msecSleep,
		       MsgLog *msgLog)
{
  const string method_name = "DsFmqTrigger::init()";
  
  _clearErrStr();

  // Record URL of fmq.

  _dsUrl = url;

  // the fmqTrigger approach applies on to the fmq protocol

  if (_dsUrl.getProtocol() != "fmqp")
  {
    _errStr = "ERROR - " + method_name + "\n"
              "FmqTrigger mode requires an fmq protocol\n";

    return -1;
  }

  // initialize DsFmq object

  const char *fmqURL = url.c_str();

  const char *processName = procName.c_str();

  int iret = _dsFmq.init(fmqURL,processName,debug,mode,position,
			 compression,numSlots,bufSize,msecSleep,msgLog);

  if (iret)
  {
    _errStr = "ERROR - " + method_name + "\n";
    
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

int DsFmqTrigger::next()
{
  const string method_name = "DsFmqTrigger::next()";
  
  assert(_objectInitialized);
  
  int iret;
  string filePath;
  
  // clear out the old stuff

  _clearErrStr();
  _triggerInfo.clear();

  // Check for end of trigger data

  if (endOfData())
    return -1;

  // Read message from fmq and record information or return error.

  bool gotMsg;
    
  iret = _dsFmq.readMsg( &gotMsg, ISSUE_TIME);
    
  if ( iret == -1  ||  !gotMsg  ||  _dsFmq.getMsgLen() == 0 )
  {  
    _errStr = method_name + " - Failed to read fmq message.\n";
      
    return( -1 );                                               
  }

  // Set the trigger info
  
  _triggerInfo.setInfoFromMsg(_dsFmq.getMsg());      

  return(0);
}


/**********************************************************************
 * endOfData() - Check whether we are at the end of the data.
 */

bool DsFmqTrigger::endOfData() const
{
  assert(_objectInitialized);
  
  return false;
}


/**********************************************************************
 * reset() - Reset to start of data list
 */

void DsFmqTrigger::reset()
{
  assert(_objectInitialized);
  
  // Do nothing
}
