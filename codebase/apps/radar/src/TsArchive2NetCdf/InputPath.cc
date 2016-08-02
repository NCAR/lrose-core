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
//////////////////////////////////////////////////////////
// InputPath.cc : Input data system file handling
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2005
//
//////////////////////////////////////////////////////////

#include <iostream>
#include <ctime>
#include <cerrno>
#include <cstdio>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "InputPath.hh"
using namespace std;

#define MAX_PATH_LEN 1024
#define PATH_DELIM "/"

/////////////////////////////
// Constructor - Archive mode
//
// Pass in a vector of file paths.

InputPath::InputPath (const vector< string >& file_paths)
{

  _mode = ARCHIVE_MODE;
  _recurse = true;
  _heartbeat_func = NULL;

  // set up file paths vector
  
  for (size_t i = 0; i < file_paths.size(); i++) {
    const string &path = file_paths[i];
    time_t modTime;
    if (_setFileModTime(path, modTime) == 0) {
      _insertPathPair(path, modTime);
    }
  }

  // initialize posn to start of map
  
  reset();
  
}

/////////////////////////////////////////////////////////////////////
// Constructor - realtime mode
//
// Pass in (a) the input directory to be watched.
//         (b) the max valid age for a realtime file (secs)
//             the routine will wait for a file with the age
//             less than this.
//         (c) pointer to heartbeat_func. If NULL this is ignored.
//             If non-NULL, this is called once per second while
//             the routine is waiting for new data.
//         (d) latest_file_only flag. Only applies if use_ldata_info is
//             false. If set, the routine returns the latest file.
//             If false, it returns the earliest file which is younger than
//             the max valid age and which has not been used yet.
//
// next() will recurse looking for files in subdirectories. However,
// you need to understand the way in which directory modify times are
// set. A directory modify time is set only when a file is added or
// removed from the directory, not if an existing file is
// modified. next() will traverse into a subdirectory if is has been
// modified within the last max_valid_age secs.

InputPath::InputPath(const string &input_dir,
		     int max_valid_age /* = 3600 */,
		     Input_heartbeat_t heartbeat_func /* = NULL*/,
		     bool latest_file_only /* = true */)
  
{

  _input_dir = input_dir;
  _max_age = max_valid_age;
  _heartbeat_func = heartbeat_func;
  _latest_file_only = latest_file_only;
  _latest_time_used = -1;
  _dir_scan_sleep_secs = 5;
  _file_quiescence_secs = 5;
  _mode = REALTIME_MODE;
  _recurse = true;
  _checkSubString = false;

}

/////////////
// Destructor

InputPath::~InputPath()
{

}

/////////////////////////
// reset to start of list
// 
// Archive mode only.

void InputPath::reset()

{
  _posn = _pathTimes.begin();
}

/////////////////////////
// step back by one entry
// 
// Archive mode only.
//
// Returns 0 on success, -1 if already at start of list

int InputPath::stepBack()

{
  if (_posn  == _pathTimes.begin()) {
    return -1;
  }
  _posn--;
  return 0;
}

///////////////////////////////////////
// set the required data file extension
//
// If set, only files with this extension will be returned

void InputPath::setSearchExt(const string &search_ext)

{
  _search_ext = search_ext;
}

///////////////////////////////////////
// set a substring that must be in the filename
// for it to be considered valid.
//
// If set, only files with this string in
// their full path will be returned. Applies
// only to realtime mode.

void InputPath::setSubString(const string &subString)

{
  _checkSubString = true;
  _subString = subString;
}

///////////////////////////////////////////////////////////
// set the directory scan sleep interval - secs
//
// This only applies to REALTIME mode, This is the time the search
// routine sleeps between each scan of the directory looking for new
// data. The default is 5, and the maximum is 50, so that the routine
// will register with the procmap at least once per minute.

void InputPath::setDirScanSleep(int dir_scan_sleep_secs /* = 5*/ )

{
  if (dir_scan_sleep_secs < 1) {
    dir_scan_sleep_secs = 1;
  } else {
    _dir_scan_sleep_secs = dir_scan_sleep_secs;
  }
}

///////////////////////////////////////////////////////////
// set the file quiescence time - secs
//
// This only applies to REALTIME mode. The next() routine, when
// searching for new files, waits for files to be quiescent before
// assuming they are complete on disk. This is the number of secs of
// quiescence we wait.

void InputPath::setFileQuiescence(int file_quiescence_secs /* = 5*/ )

{
  if (file_quiescence_secs < 1) {
    _file_quiescence_secs = 1;
  } else {
    _file_quiescence_secs = file_quiescence_secs;
  }
}

///////////////////////////////////////////////////////////
// set the recursion flag (default is true)
//
// This only applies to REALTIME mode. The next() routine, when
// searching for new files, will recurse down new directories unless
// the recursion flag is set to false.

void InputPath::setRecursion(bool recursionFlag /* = true*/ )
{
  _recurse = recursionFlag;
}

//////////////////////////////////////////////////
// get next file path
//
// Realtime and Archive modes only
//
// If block is true, blocks until data is available.
// If block is false, returns immediately if no data is available.
// Blocking is only applicable in Realtime mode.
//
// Returns the path of the next available file.
// If no file is available, returns NULL.

char *InputPath::next(bool block /* = true */)

{

  // loop until we get (a) a file which matches the extension
  // or (b) an error

  while (true) {
    
    switch(_mode)  {
      
    case ARCHIVE_MODE:
      if (_nextArchive()) {
	return NULL;
      }
      break;
      
    case REALTIME_MODE :
      
      if (_nextRealtime(block)) {
	return NULL;
      }
      break;
      
    } //  switch(_mode)
    
    if (_hasExt(_returned_path, _search_ext)) {
      return (char *) _returned_path.c_str();
    }
    
  } // while

  return NULL;

}

/////////////////////////////////////////////////////////////
// set file modify time
//
// Returns 0 on success, -1 on failure

int InputPath::_setFileModTime(const string &path, time_t &modTime)

{

  struct stat buf;
  if (stat(path.c_str(), &buf)) {
    return -1;
  }
  modTime = buf.st_mtime;
  return 0;

}

////////////////////////////////////
// get next file path - archive mode
//
// Returns 0 on success, -1 on failure
// On success loads path in _returned_path

int InputPath::_nextArchive()

{

  // go through the file name vector
  
  while (_posn != _pathTimes.end()) {
    
    const string &path = (*_posn).first;
    _posn++;
    
    // Make sure the file exists before returning it
    
    struct stat buf;
    if (stat(path.c_str(), &buf) == 0) {
      if (S_ISREG(buf.st_mode)) {
	_returned_path = path;
	return 0;
      }
    }
    
  } // while
    
  // If we reach this point, we have returned all of the
  // existing files.

  return -1;

}

////////////////////////////////////////////////////////
// get next file path - realtime mode
//
// Returns 0 on success, -1 on failure
// On success loads path in _returned_path

int InputPath::_nextRealtime(bool block)

{

  while (true) {
    
    // no latest_data_info
    // recursively search the directory for new files
    
    time_t closestValidTime = -1;
    
    if (_find_next_path(_input_dir,
			closestValidTime) == 0) {
      _latest_time_used = closestValidTime;
      return 0;
    }

    // return with error now if non-blocking

    if (!block) {
      return -1;
    }

    if (_heartbeat_func != NULL) {
      _heartbeat_func("DSINP_next: waiting for files");
    }
    sleep(_dir_scan_sleep_secs);

  }
    
  return -1;

}

////////////////////////////////////////////////////////////
// Find next realtime path
//
// Finds next realtime path in a directory.
//
// Returns 0 on success, -1 if an error is encountered.
// On success loads path in _returned_path
//

int InputPath::_find_next_path(const string &input_dir,
			       time_t &closest_valid_time)
  
{

  DIR *dirp;
  if ((dirp = opendir(input_dir.c_str())) == NULL) {
    if (_debug) {
      int errNum = errno;
      cerr << "ERROR: InputPath::_find_next_path" << endl;
      cerr << "  Cannot open dir: " << input_dir << endl;
      cerr << "  " << strerror(errNum) << endl;
    }
    return (-1);
  }
  
  // read directory looking for data files to be transferred

  time_t now = time(NULL);
  struct dirent *dp;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    // exclude dir entries and files beginning with '.' and '_'
    
    if (dp->d_name[0] == '.') {
      continue;
    }
    
    // check file time
    
    char filePath[MAX_PATH_LEN];
    sprintf(filePath, "%s%s%s", input_dir.c_str(), PATH_DELIM, dp->d_name);
    struct stat fileStat;
    if (stat(filePath, &fileStat)) {
      if (_debug) {
	int errNum = errno;
	cerr << "WARNING: InputPath::_find_next_path" << endl;
	cerr << "  Cannot stat file: " << filePath << endl;
	cerr << "  " << strerror(errNum) << endl;
      }
      continue;
    }

    int age = now - fileStat.st_mtime;

    if (!S_ISDIR(fileStat.st_mode)) {
      if (fileStat.st_mtime <= _latest_time_used) {
	continue;
      }
    }

    if (age > _max_age || age < _file_quiescence_secs) {
      continue;
    }

    // for directories, search recursively, if requested
    
    if (_recurse && S_ISDIR(fileStat.st_mode)) {
      char dirPath[MAX_PATH_LEN];
      sprintf(dirPath, "%s%s%s", input_dir.c_str(), PATH_DELIM, dp->d_name);
      _find_next_path(dirPath, closest_valid_time);
      continue;
    }

    if (!S_ISREG(fileStat.st_mode)) {
      continue;
    }

    //
    // If we are checking that a substring must be in the filename,
    // then do that.
    //
    if (_checkSubString){
      if (strstr(dp->d_name, _subString.c_str()) == NULL) {
	continue;
      }
    }

    // make sure we're returning the first relevant file
    
    if (closest_valid_time < 0) {
      closest_valid_time = fileStat.st_mtime;
      _returned_path = filePath;
    } else {
      if (_latest_file_only) {
	if (closest_valid_time < fileStat.st_mtime) {
	  closest_valid_time = fileStat.st_mtime;
	  _returned_path = filePath;
	}
      } else {
	if (closest_valid_time > fileStat.st_mtime) {
	  closest_valid_time = fileStat.st_mtime;
	  _returned_path = filePath;
	}
      }
    }
    
  } // dp
  
  closedir(dirp);
  
  if (closest_valid_time < 0) {
    return -1;
  } else {
    return 0;
  }

}


////////////////////////////////////
// test for file extension, if set
//

bool InputPath::_hasExt(const string &path, const string &ext)

{

  if (ext.size() == 0) {
    return true;
  }

  size_t extLen = ext.size();

  // uncompressed file

  string thisExt = path.substr(path.size() - extLen, extLen);
  if (thisExt == ext) {
    return true;
  }

  return false;

}

////////////////////////////////////////////////
// Insert the path/time pair, provided the file
// exists and is a plain file or link.
//

void InputPath::_insertPathPair(const string &path, time_t file_time)
  
{

  // insert if file exists
  
  struct stat buf;
  if (stat(path.c_str(), &buf) == 0) {
    if (S_ISREG(buf.st_mode)) {
      PathTimePair pathTime;
      pathTime.first = path;
      pathTime.second = file_time;
      _pathTimes.insert(pathTime);
    }
  }

}

