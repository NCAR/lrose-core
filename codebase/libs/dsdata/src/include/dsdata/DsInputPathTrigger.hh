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
 *   $Author: gaydos $
 *   $Locker:  $
 *   $Date: 2019/01/03 16:37:59 $
 *   $Id: DsInputPathTrigger.hh,v 1.3 2019/01/03 16:37:59 gaydos Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DsInputPathTrigger: Class implementing a DsTrigger which returns new
 *                     files as they appear in a DS input directory.
 *                     The DS input directory structure requires files
 *                     to be named using the following pattern:
 *                             <file_path>/YYYYMMDD/hhmmss.<ext>
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2010
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef DsInputPathTrigger_HH
#define DsInputPathTrigger_HH

#include <string>
#include <vector>
#include <cassert>

#include <didss/DsInputPath.hh>
#include <dsdata/DsTrigger.hh>

using namespace std;


class DsInputPathTrigger : public DsTrigger
{

public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  DsInputPathTrigger();


  /**********************************************************************
   * Destructor
   */

  virtual ~DsInputPathTrigger();


  /**********************************************************************
   * init() - Initialize the object.  The object must be initialized
   *          before it can be used.
   *
   * input_dir: directory to watch for new files.
   * max_file_age: maximum valid age for a file in seconds
   * heartbeat_func: pointer to the heartbeat function
   * use_ldata_info flag: If true, we use the latest_data_info
   *            file, if false we watch the directory recursively
   *            for new files.
   * latest_file_only flag: Only applies if use_ldata_info is
   *            false. If set, the routine returns the latest file.
   *            If false, it returns the earliest file which is younger than
   *            the max valid age and which has not been used yet.
   *
   *  Returns 0 on success, -1 on error.
   *
   * Use getErrStr() for error message.
   */

  int init(const string &input_dir,
	   const int max_file_age,
	   const heartbeat_func_t heartbeat_func = 0,
           bool use_ldata_info = true,
           bool latest_file_only = true);

  

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
 
  bool _endOfData;
 
  DsInputPath *_inputPath;

  heartbeat_func_t _heartbeatFunc;
  
};

#endif /* DsInputPathTrigger_HH */


