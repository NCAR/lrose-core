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
 *   $Id: DsTimeListTrigger.hh,v 1.4 2016/03/03 18:06:34 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DsTimeListTrigger.hh: Class implementing a DsTrigger based on a
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
 * TIME_LIST_MODE:
 *     setTimeList passes in a URL and a start and end time.
 *     A vector of available times is compiled and stored.
 *     
 *     nextIssueTime passes in a DateTime reference. When the routine is called,
 *     the times in the vector are recorded in the DateTime object.
 *     
 *     nextFile passes in a reference to a string. When the routine is called,
 *     the times in the vector are used to construct an mdv filepath and this
 *     path is recorded in the string.
 * 
 *     If nextIssueTime or nextFile returns an error, 
 *     the data stored in the vector has been exhausted.
 *     endOfData returns true when the data is exhausted.
 ************************************************************************/

#ifndef DsTimeListTrigger_HH
#define DsTimeListTrigger_HH

/*
 **************************** includes **********************************
 */

#include <string>
#include <vector>
#include <cassert>

#include <didss/DsURL.hh>
#include <didss/LdataInfo.hh>
#include <dsdata/DsTrigger.hh>
#include <dsdata/TimeListHandler.hh>
#include <Mdv/DsMdvxTimes.hh>
using namespace std;


/*
 ************************* class definitions ****************************
 */

class DsTimeListTrigger : public DsTrigger
{

public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  DsTimeListTrigger();


  /**********************************************************************
   * Destructor
   */

  virtual ~DsTimeListTrigger();


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

  int init(const string &url, 
	   const time_t start_time, 
	   const time_t end_time);
  

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
  
  TimeListHandler *_timeListHandler;
  

  /////////////////////
  // Private methods //
  /////////////////////

};

#endif /* DsTimeListTrigger_HH */


