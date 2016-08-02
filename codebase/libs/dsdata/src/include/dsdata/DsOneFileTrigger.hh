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
 *   $Id: DsOneFileTrigger.hh,v 1.2 2016/03/03 18:06:33 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
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
 ************************************************************************/

#ifndef DsOneFileTrigger_HH
#define DsOneFileTrigger_HH

#include <cassert>
#include <string>

#include <dsdata/DsTrigger.hh>

using namespace std;


class DsOneFileTrigger : public DsTrigger
{

public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  DsOneFileTrigger();


  /**********************************************************************
   * Destructor
   */

  virtual ~DsOneFileTrigger();


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

  int init(const string &file_path,
	   const int sleep_secs,
	   const int no_change_secs,
	   const heartbeat_func_t heartbeat_func);
  

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
  
  string _filePath;
  int _sleepSecs;
  int _noChangeSecs;
  heartbeat_func_t _heartbeat_func;
  time_t _lastFileTime;

};

#endif /* DsOneFileTrigger_HH */


