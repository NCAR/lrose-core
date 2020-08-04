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
 *   $Id: DsSpecificFcstLdataTrigger.hh,v 1.3 2016/03/03 18:06:34 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DsSpecificFcstLdataTrigger: Class implementing a DsTrigger that issues
 *                             a trigger whenever a new file for a specific
 *                             forecast period is detected in the Ldata
 *                             information.  This trigger can only be used
 *                             with MDV URLs pointing to data in a RAP
 *                             forecast-style directory structure.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2005
 *
 * Nancy Rehak
 *
 * LATEST_DATA_MODE:
 *     setLatestData passes in a URL to watch, as well as
 *     as max valid age and a heartbeat function to be called while
 *     waiting for new data to arrive. 
 *
 *     nextIssueTime passes in a DateTime reference. When the routine is 
 *     called, the input directory is watched for a new file and when a new
 *     file arrives, it's issue time is recorded in the DateTime object.
 *     
 *     nextFile passes in a reference to a string. When the routine is called,
 *     the issue time of a new file is determined and from this time,
 *     an mdv filepath constructed and recorded in the string.
 ************************************************************************/

#ifndef DsSpecificFcstLdataTrigger_HH
#define DsSpecificFcstLdataTrigger_HH

#include <string>
#include <vector>
#include <cassert>

#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTrigger.hh>

using namespace std;


class DsSpecificFcstLdataTrigger : public DsTrigger
{

public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  DsSpecificFcstLdataTrigger();


  /**********************************************************************
   * Destructor
   */

  virtual ~DsSpecificFcstLdataTrigger();


  /**********************************************************************
   * init() - Initialize the object.  The object must be initialized
   *          before it can be used.
   *
   * url: URL to watch.
   *
   * fcst_periods: list of forecast periods, in seconds, to use for
   *               triggering.  If files are received with forecast
   *               periods not in this list, they will not cause a
   *               trigger event.
   *
   * use_gen_time: Flag indicating whether to use the generate time or
   *               the forecast valid time as the trigger time.
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
   *
   * Returns 0 on success, -1 on error.
   *
   * Use getErrStr() for error message.
   */

  int init(const string &url,
	   const vector< int > fcst_periods,
	   const bool use_gen_time,
	   const int max_valid_age,
	   const heartbeat_func_t heartbeat_func = NULL,
	   const int delay_msec = 5000,
           const bool latest_only_flag = false);
  

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
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _objectInitialized;

  heartbeat_func_t _heartbeatFunc;
  int _delayMsec;
  
  DsLdataTrigger _ldataTrigger;
  vector< int > _fcstPeriods;
  bool _useGenTime;
  bool _latestOnlyFlag;
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

  int _nextAll();
  int _nextLatestOnly();
  
};

#endif /* DsSpecificFcstLdataTrigger_HH */
