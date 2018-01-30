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
/*********************************************************************
 * InputDir: InputDir object code.  This object monitors an input
 *           directory for new input files or for updates to input
 *           files.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 1996
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <cerrno>
#include <cassert>
#include <ctime>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include <toolsa/str.h>
#include <toolsa/file_io.h>

#include <toolsa/InputDir.hh>
#include <toolsa/DateTime.hh>

using namespace std;

/*
 * Global variables
 */

const int Forever = 1;


/*********************************************************************
 * Constructors
 */

InputDir::InputDir(const string &dir_name,
		   const string &file_substring,
		   bool process_old_files,
		   const string &exclude_substring,
                   bool debug,
                   bool verbose) :
        _dirPtr(NULL),
        _debug(debug),
        _verbose(verbose)
{
  _init(dir_name, file_substring, exclude_substring, process_old_files);
}


// InputDir::InputDir(const char *dir_name,
// 		   const char *file_substring,
// 		   int process_old_files,
// 		   const string &exclude_substring,
//                    bool debug,
//                    bool verbose) :
//         _dirPtr(NULL),
//         _debug(debug),
//         _verbose(verbose)
// {
//   _init(dir_name, file_substring, exclude_substring, process_old_files);
// }


/*********************************************************************
 * Destructor
 */

InputDir::~InputDir(void)
{
  // Close the directory

  if (_dirPtr != NULL) {
    closedir(_dirPtr);
  }
}


/*********************************************************************
 * setDirName() - Set the directory
 */

void InputDir::setDirName(const string &dir_name)
{
  _dirName = dir_name;
  
  if (_dirPtr != 0)
    closedir(_dirPtr);
  
  _dirPtr = opendir(_dirName.c_str());
  if (_dirPtr == NULL) {
    cerr << "ERROR - InputDir::setDirName" << endl;
    cerr << "  Cannot open dir: " << _dirName << endl;
  } else {
    if (_debug) {
      cerr << "InputDir::setDirName" << endl;
      cerr << "  Opened dir: " << _dirName << endl;
    }
  }

}
  
/*********************************************************************
 * getNextFilename() - Gets the next newly detected input filename.
 *                     When there are no new input files, returns
 *                     NULL.  You are expected to call this in a loop
 *                     until there are no new files, so the directory
 *                     is only read on the first call in the loop and
 *                     the update time is only updated when all of the
 *                     new files have been processed.
 */

char *InputDir::getNextFilename(int check_dir_flag,
				int max_input_data_age)
{

  static const string method_name = "InputDir::getNextFilename()";

  struct stat file_stat;
  struct dirent *dir_entry_ptr;
  char *next_file;
  int stat_return;

  if (_dirPtr == NULL) {
    cerr << "ERROR - InputDir::getNextFilename" << endl;
    cerr << "  Input dir does not exist: " << _dirName << endl;
    return NULL;
  }
  
  if (_debug) {
    cerr << "InputDir::getNextFilename(), dir: " << _dirName << endl;
    cerr << "  check_dir_flag: " << check_dir_flag << endl;
    cerr << "  max_input_data_age: " << max_input_data_age << endl;
    cerr << "  lastDirUpdateTime: " << DateTime::strm(_lastDirUpdateTime) << endl;
    cerr << "  lastDataFileTime: " << DateTime::strm(_lastDataFileTime) << endl;
  }

  // See if we need to rewind the directory.

  if (_rewindDirFlag)
  {

    if (!_rewindDir(check_dir_flag)) {
      return NULL;
    }
    
    _rewindDirFlag = FALSE;
    
  } /* endif - _rewindDirFlag */
  
  while (Forever)
  {
    // Read the next entry from the directory

    if ((dir_entry_ptr = readdir(_dirPtr)) == 0)
    {
      // Make sure we rewind the directory before reading it again.
      
      _rewindDirFlag = TRUE;

      // Make sure we don't process the old files again
      
      _lastDirUpdateTime = _lastDataFileTime;
    
      if (_debug) {
        cerr << "  No more new files" << endl;
      }

      // Tell the calling routine that there are no more new files.

      return((char *)0);
    }

    // Make sure we don't return . or ..

    if (STRequal_exact(dir_entry_ptr->d_name, ".") ||
	STRequal_exact(dir_entry_ptr->d_name, ".."))
      continue;
    
    if (_verbose) {
      cerr << "  Checking file: " << dir_entry_ptr->d_name << endl;
    }

    // Make sure the filename contains the appropriate substring

    if ((_fileSubstring.size() > 0) && 
        (strstr(dir_entry_ptr->d_name, _fileSubstring.c_str()) == 0)) {
      if (_verbose) {
        cerr << "  Ignoring file without required string: " << _fileSubstring << endl;
      }
      continue;
    }

    // Make sure the filename doesn't contains the excluded substring

    if ((_excludeSubstring.size() > 0) &&
        (strstr(dir_entry_ptr->d_name, _excludeSubstring.c_str()) != 0)) {
      if (_verbose) {
        cerr << "  Ignoring file with exclude string: " << _excludeSubstring << endl;
      }
      continue;
    }
    
    // Determine the full path for the file

    next_file = new char[strlen(_dirName.c_str()) +
			 strlen(dir_entry_ptr->d_name) + 2];
    strcpy(next_file, _dirName.c_str());
    strcat(next_file, "/");
    strcat(next_file, dir_entry_ptr->d_name);
  
    // Make sure we don't return old files
    
    stat_return = ta_stat(next_file, &file_stat);

    if (stat_return != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error statting file" << endl;
      perror(next_file);
      
      delete [] next_file;
      
      continue;
    }
    
    time_t current_time = time((time_t *)0);
    
    if (_lastDirUpdateTime >= file_stat.st_mtime) {
      if (_verbose) {
        cerr << "  ignoring file, _lastDirUpdateTime > file.st_mtime" << endl;
      }
      delete [] next_file;
      continue;
    } else if (max_input_data_age > 0 &&
               current_time - file_stat.st_mtime > max_input_data_age) {
      if (_verbose) {
        cerr << "  ignoring file, too old" << endl;
      }
      delete [] next_file;
      continue;
    } else {
      if (file_stat.st_mtime > _lastDataFileTime) {
        _lastDataFileTime = file_stat.st_mtime;
      }
      break;
    }
    
    // We can never get here

  } /* endwhile - Forever */
  
  return(next_file);
}


/*********************************************************************
 * _init() - Initialize the object.
 */

void InputDir::_init(const string &dir_name,
		     const string &file_substring,
		     const string &exclude_substring,
		     const bool process_old_files)
{
  // Keep the directory name

  _dirName = dir_name;
  
  // Try to open the directory

  if (_dirName != "") {
    _dirPtr = opendir(_dirName.c_str());
    if (_dirPtr == NULL) {
      cerr << "ERROR - InputDir::_init" << endl;
      cerr << "  Cannot open dir: " << _dirName << endl;
    }
    if (_debug) {
      cerr << "InputDir::_init(), opened dir: " << _dirName << endl;
    }
  }

  // Save the file substrings

  _fileSubstring = file_substring;
  _excludeSubstring = exclude_substring;
  
  // Initialize the internal times

  if (process_old_files)
  {
    // use all files
    _lastDirUpdateTime = -1;
    _lastDataFileTime = 0;
  }
  else
  {
    // only use latest file
    _lastDirUpdateTime = getLatestFileTime() - 1;
    _lastDataFileTime = _lastDirUpdateTime;
  }
  
  _lastDirRewindTime = 0;
  
  _rewindDirFlag = true;
}


/*********************************************************************
 * _rewindDir() - Rewind the main directory.
 *
 * Returns true if the directory was successfully rewound, false otherwise.
 */

bool InputDir::_rewindDir(const int check_dir_flag)
{
  static const string method_name = "InputDir::_rewindDir()";
  
  // First see if the directory has changed since we were last here.

  if (check_dir_flag)
  {
    struct stat dir_stat;

    int stat_return = ta_stat(_dirName.c_str(), &dir_stat);
    assert(stat_return == 0);

    if (_lastDirUpdateTime >= dir_stat.st_mtime)
      return false;

    // Set the last directory rewind time to the directory change time.
    // This is done because I think we might be missing some files because
    // of NFS timing problems.

    if (_lastDirRewindTime > dir_stat.st_mtime)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Time went backwards for directory change time using stat." << endl;
      cerr << "Could signal a NFS problem" << endl;
    }
    else
    {
      _lastDirRewindTime = dir_stat.st_mtime;
    }
  
  }
  else
  {
    _lastDirRewindTime = time((time_t *)0);
  }
  
  // Rewind the directory to get the new entries
  
  if (_dirPtr != NULL) {
    rewinddir(_dirPtr);
  }

  return true;
}

/*********************************************************************
 * getLatestFileTime()
 *
 *   returns the time of the latest file in the directory
 */

time_t InputDir::getLatestFileTime()
{

  time_t latestTime = 0;

  if (_debug) {
    cerr << "InputDir::getlatestFileTime(), dir: " << _dirName << endl;
  }

  if (_dirPtr == NULL) {
    cerr << "ERROR - InputDir::getLatestFileTime" << endl;
    cerr << "  Input dir does not exist: " << _dirName << endl;
    return -1;
  }
  
  struct dirent *dir_entry_ptr = NULL;
  while ((dir_entry_ptr = readdir(_dirPtr)) != NULL) {
    
    // skip dot files

    if (STRequal_exact(dir_entry_ptr->d_name, ".") ||
	STRequal_exact(dir_entry_ptr->d_name, "..")) {
      continue;
    }
    
    // Make sure the filename contains the appropriate substring

    if ((_fileSubstring.size() > 0) && 
        (strstr(dir_entry_ptr->d_name, _fileSubstring.c_str()) == 0)) {
      if (_verbose) {
        cerr << "  Ignoring file without required string: " << _fileSubstring << endl;
      }
      continue;
    }

    // Make sure the filename doesn't contains the excluded substring

    if ((_excludeSubstring.size() > 0) &&
        (strstr(dir_entry_ptr->d_name, _excludeSubstring.c_str()) != 0)) {
      if (_verbose) {
        cerr << "  Ignoring file with exclude string: " << _excludeSubstring << endl;
      }
      continue;
    }
    
    // Determine the full path for the file
    
    string filePath(_dirName);
    filePath += PATH_DELIM;
    filePath += dir_entry_ptr->d_name;
    
    struct stat file_stat;
    if (ta_stat(filePath.c_str(), &file_stat) == 0) {
      if (file_stat.st_mtime > latestTime) {
        latestTime = file_stat.st_mtime;
      }
    }

  } /* endwhile - Forever */

  _rewindDir(false);
  return latestTime;

}


