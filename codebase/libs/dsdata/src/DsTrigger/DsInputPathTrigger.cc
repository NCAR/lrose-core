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

// RCS info
//   $Author: gaydos $
//   $Locker:  $
//   $Date: 2019/01/03 16:37:59 $
//   $Id: DsInputPathTrigger.cc,v 1.5 2019/01/03 16:37:59 gaydos Exp $
//   $Revision: 1.5 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DsInputPathTrigger: Class implementing a DsTrigger which returns new
 *                    files as they appear in an input directory.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <sys/types.h>
#include <toolsa/file_io.h>
#include <unistd.h>

#include <dsdata/DsInputPathTrigger.hh>

using namespace std;



/**********************************************************************
 * Constructors
 */

DsInputPathTrigger::DsInputPathTrigger() :
  DsTrigger(TYPE_FILE_TRIGGER),
  _objectInitialized(false),
  _endOfData(false),
  _inputPath(NULL),
  _heartbeatFunc(NULL)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

DsInputPathTrigger::~DsInputPathTrigger()
{
  if (_inputPath) {
    delete _inputPath;
  }
}


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

int DsInputPathTrigger::init(const string &input_dir,
			     const int max_file_age,
			     const heartbeat_func_t heartbeat_func,
 			     bool use_ldata_info /* = true */,
                             bool latest_file_only /* = true */) 
{
  const string method_name = "DsInputPathTrigger::init()";
  
  _clearErrStr();
 
  // Initialize the InputDir object

  _inputPath = new DsInputPath("DsTrigger",
			       false,
			       input_dir,
			       max_file_age,
			       heartbeat_func,
                               use_ldata_info,
                               latest_file_only);
  
  _heartbeatFunc = heartbeat_func;
  
  _objectInitialized = true;
  
  return 0;
}


/**********************************************************************
 * next() - Get the next trigger and set the triggerInfo accordingly
 *
 * Returns 0 upon success, -1 upon failure.
 */

int DsInputPathTrigger::next()
{
  const string method_name = "DsInputPathTrigger::next()";
  
  assert(_objectInitialized);
  
  // clear out the old stuff

  _clearErrStr();
  _triggerInfo.clear();

  // Check for end of trigger data

  if (endOfData())
      return -1;

  // Wait for the next available input file

  char *next_filename;
  
  while ((next_filename = _inputPath->next()) == 0)
  {
    _heartbeatFunc("Waiting for data");

    sleep(1);
    
    if ((next_filename = _inputPath->next()) != 0)
      break;
  }
  
  // Set the trigger info

  struct stat file_stat;
  
  if (ta_stat(next_filename, &file_stat) != 0)
  {
    _errStr = "ERROR - " + method_name + "\n";
    _errStr += string("Unable to stat new data file: ") + next_filename + "\n";
    
    return -1;
  }
  
  _triggerInfo.setFilePath(next_filename);
  _triggerInfo.setIssueTime(file_stat.st_mtime);
  
  return 0;
}


/**********************************************************************
 * endOfData() - Check whether we are at the end of the data.
 */

bool DsInputPathTrigger::endOfData() const
{
  return _endOfData;
}


/**********************************************************************
 * reset() - Reset to start of data list
 */

void DsInputPathTrigger::reset()
{
  assert(_objectInitialized);
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
