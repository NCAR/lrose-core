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

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 18:06:33 $
 *   $Id: DsFmqTrigger.hh,v 1.4 2016/03/03 18:06:33 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DsFmqTrigger.hh: Class implementing a DsTrigger using an FMQ.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2000
 *
 * Nancy Rehak
 *
 * Refactored from code by Sue Dettling
 *
 * FMQ_MODE:
 *     The setFmq passes in the url of the fmq to watch.
 *     
 *     nextIssueTime passes in a DateTime reference. When the routine is called, 
 *     the fmq is read and the issue time of the new file is recorded 
 *     in the DateTime object.
 ************************************************************************/

#ifndef DsFmqTrigger_HH
#define DsFmqTrigger_HH

/*
 **************************** includes **********************************
 */

#include <string>
#include <vector>
#include <cassert>

#include <didss/DsURL.hh>
#include <dsdata/DsTrigger.hh>
#include <Fmq/DsFmq.hh>
using namespace std;


/*
 ************************* class definitions ****************************
 */

class DsFmqTrigger : public DsTrigger
{

public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  DsFmqTrigger();


  /**********************************************************************
   * Destructor
   */

  virtual ~DsFmqTrigger();


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
  
  int init(const string &url, 
	   const string &procName, 
	   const bool debug = false,
	   const DsFmq::openMode mode = DsFmq::READ_WRITE, 
	   const DsFmq::openPosition position = DsFmq::END,
	   const bool compression = false, 
	   const size_t numSlots = 1024, 
	   const size_t bufSize = 10000,
	   const size_t msecSleep = 1000,
	   MsgLog *msgLog = NULL);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**********************************************************************
   * next() - Get the next trigger and set the triggerInfo accordingly
   *
   * Returns 0 upon success, -1 upon failure.
   */

  int next();
  

  /**********************************************************************
   * endOfData() - Check whether we are at the end of the data.
   */

  bool endOfData() const;
  

  /**********************************************************************
   * reset() - Reset to start of data list
   */

  void reset();
  

  /**********************************************************************
   * fireTrigger() - Write issue time and process name to fmq.
   */

  int fireTrigger(TriggerInfo& clientTriggerInfo);
  

private:

  ///////////////////
  // Private types //
  ///////////////////

  /**********************************************************************
   * fmq message id's
   */

  enum msgType_t
  {
    ISSUE_TIME = 4
  };


  /////////////////////
  // Private members //
  /////////////////////

  bool _objectInitialized;
  
  DsURL _dsUrl;

  DsFmq _dsFmq;
  TriggerInfo _triggerInfo;


  /////////////////////
  // Private methods //
  /////////////////////

};

#endif /* DsFmqTrigger_HH */


