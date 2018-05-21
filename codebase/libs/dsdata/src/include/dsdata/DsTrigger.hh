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
 *   $Date: 2017/12/14 15:47:01 $
 *   $Id: DsTrigger.hh,v 1.15 2017/12/14 15:47:01 dixon Exp $
 *   $Revision: 1.15 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DsTrigger.hh: Abstract base class for providing data set time
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
 ************************************************************************/

#ifndef DSTRIGGER_HH
#define DSTRIGGER_HH

#include <string>
#include <vector>
#include <cassert>

#include <dsdata/TriggerInfo.hh>
using namespace std;

class DsTrigger
{

public:

  //////////////////
  // Public types //
  //////////////////

  /**********************************************************************
   * heartbeat function
   */

  typedef void (*heartbeat_func_t)(const char *label);


  /**********************************************************************
   * Trigger type - defines the type of trigger each trigger is.  A time
   * trigger returns the trigger information as a time value.  A file
   * trigger returns the trigger information as a file path.
   */

  typedef enum
  {
    TYPE_TIME_TRIGGER,
    TYPE_FILE_TRIGGER
  } trigger_type_t;

     

  ////////////////////
  // Public methods //
  ////////////////////

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  DsTrigger(const trigger_type_t type);


  /**********************************************************************
   * Destructor
   */

  virtual ~DsTrigger();

  ////////////////////
  // Setting debugging
  ////////////////////

  void setDebug(bool val) { _debug = val; }

  void setVerbose(bool val) { 
    _verbose = val;
    if (_verbose) {
      _debug = true;
    }
  }

  ////////////////////
  // Access methods //
  ////////////////////

  /**********************************************************************
   * getType() - Get the type of trigger.
   *
   * Returns the trigger type
   */

  trigger_type_t getType() const
  {
    return _type;
  }
  

  /**********************************************************************
   * next() - Get the next trigger and set the triggerInfo accordingly
   *
   * Returns 0 upon success, -1 upon failure.
   */

  virtual int next() = 0;
  virtual int next(TriggerInfo& triggerInfo);
  

  /**********************************************************************
   * nextIssueTime() - Get the next issue time.
   *
   * Returns 0 upon success, -1 upon failure.
   */

  virtual int nextIssueTime(DateTime& dateTime);
  

  /**********************************************************************
   * endOfData() - Check whether we are at the end of the data.
   */

  virtual bool endOfData() const = 0;
  

  /**********************************************************************
   * reset() - Reset to start of data list
   */

  virtual void reset() = 0;

  /**********************************************************************
   * get the trigger info, after a call to next();
   */
  
  const TriggerInfo &getTriggerInfo() const { return _triggerInfo; }

  ///////////////////
  // Error methods //
  ///////////////////

  /**********************************************************************
   * getErrStr() - Return the error string.
   */

  const string& getErrStr() const
  {
    return getErrString();
  }

  const string& getErrString() const
  {
    return _errStr;
  }


protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  string _errStr;

  TriggerInfo _triggerInfo;

  trigger_type_t _type;

  bool _debug;
  bool _verbose;

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**********************************************************************
   * _clearErrStr() - Set _errStr to the empty string.
   */

  void _clearErrStr()
  {
    _errStr = "";
  }

  
private:
  
};

#endif /* DSTRIGGER_HH */


