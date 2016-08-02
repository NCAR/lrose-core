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
 *   $Id: DsInputDirTrigger.hh,v 1.8 2016/03/03 18:06:33 dixon Exp $
 *   $Revision: 1.8 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DsInputDirTrigger: Class implementing a DsTrigger which returns new
 *                    files as they appear in an input directory.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2005
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef DsInputDirTrigger_HH
#define DsInputDirTrigger_HH

#include <string>
#include <vector>
#include <cassert>

#include <dsdata/DsTrigger.hh>
#include <toolsa/InputDir.hh>

using namespace std;


class DsInputDirTrigger : public DsTrigger
{

public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  DsInputDirTrigger();


  /**********************************************************************
   * Destructor
   */

  virtual ~DsInputDirTrigger();


  /**********************************************************************
   * init() - Initialize the object.  The object must be initialized
   *          before it can be used.
   *
   * input_dir: directory to watch for new files.
   *
   * file_substring: Only files whose names contain this substring will
   *                 act as triggers.
   *
   * process_old_files: Flag indicating whether to process old files
   *                    in the input directory.
   *
   * recurse: If true, the object will recurse through the input directory
   *          looking for files.  If false, the object will only look in
   *          the specified input directory.
   *
   * exclude_substring: Files whose names contain this substring will not
   *                    act as triggers.
   *
   *  Returns 0 on success, -1 on error.
   *
   * Use getErrStr() for error message.
   */

  int init(const string &input_dir,
	   const string &file_substring,
	   const bool process_old_files,
	   const heartbeat_func_t heartbeat_func = 0,
	   const bool recurse = false,
	   const string &exclude_substring = "",
           const int check_interval_secs = 1);
  

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
 
  bool _processOldFiles;
 
  InputDir *_inputDir;

  heartbeat_func_t _heartbeatFunc;

  int _checkIntervalSecs;
  
};

#endif /* DsInputDirTrigger_HH */


