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
//   $Author: nowcast $
//   $Locker:  $
//   $Date: 2018/10/19 18:14:32 $
//   $Id: DsInputDirTrigger.cc,v 1.13 2018/10/19 18:14:32 nowcast Exp $
//   $Revision: 1.13 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DsInputDirTrigger: Class implementing a DsTrigger which returns new
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
#include <toolsa/uusleep.h>
#include <unistd.h>

#include <dsdata/DsInputDirTrigger.hh>

#include <toolsa/InputDir.hh>
#include <toolsa/InputDirRecurse.hh>

using namespace std;



/**********************************************************************
 * Constructors
 */

DsInputDirTrigger::DsInputDirTrigger() :
  DsTrigger(TYPE_FILE_TRIGGER),
  _objectInitialized(false),
  _endOfData(false),
  _processOldFiles(false),
  _inputDir(NULL),
  _heartbeatFunc(NULL),
  _checkIntervalSecs(1)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

DsInputDirTrigger::~DsInputDirTrigger()
{
  if (_inputDir) {
    delete _inputDir;
  }
}


/**********************************************************************
 * init() - Initialize the object.  The object must be initialized
 *          before it can be used.
 *
 * input_dir: directory to watch for new files.
 *
 *  Returns 0 on success, -1 on error.
 *
 * Use getErrStr() for error message.
 */

int DsInputDirTrigger::init(const string &input_dir,
			    const string &file_substring,
			    const bool process_old_files,
			    const heartbeat_func_t heartbeat_func,
			    const bool recurse,
			    const string &exclude_substring,
                            const int check_interval_secs)
{

  const string method_name = "DsInputDirTrigger::init()";
  
  _clearErrStr();
 

  _processOldFiles = process_old_files;

  // Initialize the InputDir object

  if (recurse) {
    _inputDir = new InputDirRecurse(input_dir,
                                    file_substring,
                                    exclude_substring,
                                    process_old_files,
                                    _debug, _verbose);
  } else {
    _inputDir = new InputDir(input_dir,
                             file_substring,
                             process_old_files,
                             exclude_substring,
                             _debug, _verbose);
  }
  
  _heartbeatFunc = heartbeat_func;
  _checkIntervalSecs = check_interval_secs;
  
  _objectInitialized = true;
  
  return 0;
}


/**********************************************************************
 * next() - Get the next trigger and set the triggerInfo accordingly
 *
 * Returns 0 upon success, -1 upon failure.
 */

int DsInputDirTrigger::next()
{
  const string method_name = "DsInputDirTrigger::next()";
  
  assert(_objectInitialized);
  
  // clear out the old stuff

  _clearErrStr();
  _triggerInfo.clear();

  // Check for end of trigger data

  if (endOfData()) {
    if (_debug) {
      cerr << "DsInputDirTrigger, dir: " << _inputDir->getDirName() << endl;
      cerr << "  end of data" << endl;
    }
    return -1;
  }

  // Wait for the next available input file

  char *next_filename;
  
  while ((next_filename = _inputDir->getNextFilename(0, -1)) == 0) {

    // if there isn't an assigned heartbeat function
    // and processing old files, assume user is
    // processing all files in "archive" mode,
    // so break out of loop and signal end of data

    if (_heartbeatFunc == 0 && _processOldFiles) {
      _endOfData = true;
      if (_debug) {
        cerr << "DsInputDirTrigger, dir: " << _inputDir->getDirName() << endl;
        cerr << "  end of data" << endl;
      }
      return -1;
    } else {
       if (_heartbeatFunc) {
          _heartbeatFunc("Waiting for data");
       }
    }

    if (_debug) {
      cerr << "DsInputDirTrigger, dir: " << _inputDir->getDirName() << endl;
      cerr << "  sleeping secs: " << _checkIntervalSecs << endl;
    }

    umsleep(_checkIntervalSecs * 1000);
    
    if ((next_filename = _inputDir->getNextFilename(1, -1)) != 0) {
      break;
    }

  } // while
  
  // Set the trigger info

  struct stat file_stat;
  
  if (ta_stat(next_filename, &file_stat) != 0) {
    _errStr = "ERROR - " + method_name + "\n";
    _errStr += string("Unable to stat new data file: ") + next_filename + "\n";
    if (next_filename){
       delete [] next_filename; 
    }    
    return -1;
  }
  
  _triggerInfo.setFilePath(next_filename);
  _triggerInfo.setIssueTime(file_stat.st_mtime);
  
  if (_debug) {
    cerr << "DsInputDirTrigger, dir: " << _inputDir->getDirName() << endl;
    cerr << "  found file: " << next_filename << endl;
  }
  
  delete[] next_filename;

  return 0;
}


/**********************************************************************
 * endOfData() - Check whether we are at the end of the data.
 */

bool DsInputDirTrigger::endOfData() const
{
  return _endOfData;
}


/**********************************************************************
 * reset() - Reset to start of data list
 */

void DsInputDirTrigger::reset()
{
  assert(_objectInitialized);
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
