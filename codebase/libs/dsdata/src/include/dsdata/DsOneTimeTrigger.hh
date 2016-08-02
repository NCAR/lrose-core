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
 *   $Date: 2016/03/03 18:06:34 $
 *   $Id: DsOneTimeTrigger.hh,v 1.3 2016/03/03 18:06:34 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DsOneTimeTrigger.hh: Class implementing a DsTrigger that processes
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
 * ONE_TIME_MODE:
 *      setOneTime passes in an issue time. 
 *      This time is returned in a DateTime object when nextIssueTime is called.
 *       
 *      nextFile() is not valid for this mode.
 *
 *      endOfData returns true when the data after the first call to nextIssueTime.
 ************************************************************************/

#ifndef DsOneTimeTrigger_HH
#define DsOneTimeTrigger_HH

/*
 **************************** includes **********************************
 */

#include <string>
#include <vector>
#include <cassert>

#include <dsdata/DsTrigger.hh>
using namespace std;


/*
 ************************* class definitions ****************************
 */

class DsOneTimeTrigger : public DsTrigger
{

public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  DsOneTimeTrigger();


  /**********************************************************************
   * Destructor
   */

  virtual ~DsOneTimeTrigger();


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

  int init(const time_t issue_time);
  

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
  

private:

  /////////////////////
  // Private members //
  /////////////////////

  bool _objectInitialized;
  
  time_t _itime;
  bool _endOneTime;


  /////////////////////
  // Private methods //
  /////////////////////

};

#endif /* DsOneTimeTrigger_HH */


