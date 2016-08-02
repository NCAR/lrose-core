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
 *   $Id: DsLdataTrigger.hh,v 1.7 2016/03/03 18:06:33 dixon Exp $
 *   $Revision: 1.7 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DsLdataTrigger.hh: Class implementing a DsTrigger based on Ldata
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

#ifndef DsLdataTrigger_HH
#define DsLdataTrigger_HH

#include <string>
#include <vector>
#include <cassert>

#include <didss/DsURL.hh>
#include <dsserver/DsLdataInfo.hh>
#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvxTimes.hh>

using namespace std;


class DsLdataTrigger : public DsTrigger
{

  friend class DsMultFcstTrigger;
  friend class DsSpecificFcstLdataTrigger;
  friend class DsSpecificGenLdataTrigger;
  
public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  DsLdataTrigger();


  /**********************************************************************
   * Destructor
   */

  virtual ~DsLdataTrigger();


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

  int init(const string &url,
	   const int max_valid_age,
	   const heartbeat_func_t heartbeat_func = NULL,
	   const int delay_msec = 5000);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**********************************************************************
   * next() - Get the next trigger and set the triggerInfo accordingly.
   *
   * If delay_msec was set to a negative value, this method doesn't block.
   * Otherwise, this method blocks until the ldata information is updated.
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
  
  DsURL _dsUrl;

  int _max_valid_age;
  heartbeat_func_t _heartbeat_func;
  int _delay_msec;
  DsMdvxTimes _dsMdvxTimes;
  DsLdataInfo _ldataInfo;


  /////////////////////
  // Private methods //
  /////////////////////

};

#endif /* DsLdataTrigger_HH */


