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
 *   $Author: rehak $
 *   $Locker:  $
 *   $Date: 2008/02/13 15:01:24 $
 *   $Id: DsSpecificFcstTimeListTrigger.hh,v 1.1 2008/02/13 15:01:24 rehak Exp $
 *   $Revision: 1.1 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DsSpecificFcstTimeListTrigger: Class implementing a DsTrigger that triggers
 *                                for each forecast file in a forecast directory
 *                                with a generation time between the given start
 *                                and end times and one of a specified list of
 *                                forecast lead times.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2016
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef DsSpecificFcstTimeListTrigger_HH
#define DsSpecificFcstTimeListTrigger_HH

#include <string>
#include <vector>
#include <cassert>

#include <dsdata/DsTrigger.hh>

using namespace std;


class DsSpecificFcstTimeListTrigger : public DsTrigger
{

public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  DsSpecificFcstTimeListTrigger();


  /**********************************************************************
   * Destructor
   */

  virtual ~DsSpecificFcstTimeListTrigger();


  /**********************************************************************
   * init() - Initialize the object.  The object must be initialized
   *          before it can be used.
   *
   * The list will contain all available data times
   * between the start and end times for the given URL
   * with a forecast lead time (in seconds) in the specified list.
   *
   * Returns 0 on success, -1 on error.
   * 
   * Use getErrStr() for error message.
   */

  int init(const string &url,
           const vector< int > &fcst_lead_times,
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
  
  vector< TriggerInfo > _triggers;
  size_t _nextTrigger;
  
  /////////////////////
  // Private methods //
  /////////////////////

};

#endif /* DsSpecificFcstTimeListTrigger_HH */


