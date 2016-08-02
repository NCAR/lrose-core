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
// NewFilesInputStrategy : Strategy class for handling
//                      new files in a directory.
//
// Carl Drews, RAL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 2004
//
//////////////////////////////////////////////////////////

#include "NewFilesInputStrategy.hh"

#include <sys/types.h>
#include <dirent.h>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <iostream>
#include <cstring>

#include <toolsa/file_io.h>


using namespace std;

NewFilesInputStrategy::NewFilesInputStrategy(const string &data_dir,
						   const string &input_substring,
						   const int max_valid_age,
						   heartbeat_t heartbeat_func,
                                                   const bool process_old_files,
						   const bool debug) :
  InputStrategy(debug),
  _dataDir(data_dir),
  _inputSubstring(input_substring),
  _maxValidAge(max_valid_age),
  _heartbeatFunc(heartbeat_func),
  _processOldFiles(process_old_files),
  _debug(debug)
{
}

NewFilesInputStrategy::~NewFilesInputStrategy()
{
}


void NewFilesInputStrategy::init()
{
  if (_processOldFiles)
    return;

  // Create a list of existing files now in the data directory.
  // These files will be considered to be "already processed".

  // examine the input directory
  DIR *dirPtr = opendir(_dataDir.c_str());
  if (dirPtr == NULL) {
    cerr << "Error: Cannot open input directory " << _dataDir
      << " because " << errno << endl;
    return;
  }

  // read the directory entries
  struct dirent *entry = NULL;
  struct stat fileStatus;
  do {
    // get file name
    entry = readdir(dirPtr);
    if (entry == NULL)
      continue;

    // is this one of the files we are looking for?
    if (strstr(entry->d_name, _inputSubstring.c_str()) == NULL)
      continue;

    // get file modification time and size
    string fullPath = _dataDir  + "/" + entry->d_name;
    if (ta_stat(fullPath.c_str(), &fileStatus) != 0) {
      cerr << "Error: Cannot stat file " << fullPath
        << " because " << errno;
      continue;
    }

    // is this file older than the maximum age?
    if (_maxValidAge > 0) {
      time_t current_time = time((time_t *)NULL);
      if (current_time - fileStatus.st_mtime <= _maxValidAge)
        continue;
    }

    // save this file info as "processed"
    fileInfo info;
    info.name = entry->d_name;
    info.modified = fileStatus.st_mtime;
    info.size = fileStatus.st_size;
    _processed.push_back(info);
  } while (entry != NULL);

  // close the input directory
  closedir(dirPtr);
}


bool NewFilesInputStrategy::alreadyProcessed(string &filename,
  time_t modified, off_t size)
{
  // loop through the processed files and compare
  for (vector<fileInfo>::iterator file = _processed.begin(); file != _processed.end(); file++) {
    if (file->name != filename)
      continue;
    if (file->modified != modified)
      continue;
    if (file->size != size)
      continue;

    return true;
  }

  return false;
}


const string& NewFilesInputStrategy::getNextFile()
{
  _currentInputPath = "";

  // examine the input directory
  DIR *dirPtr = opendir(_dataDir.c_str());
  if (dirPtr == NULL) {
    cerr << "Error: Cannot open input directory " << _dataDir
      << " because " << errno << endl;
    return _currentInputPath;
  }

  // read the directory entries
  struct dirent *entry = NULL;
  struct stat fileStatus;
  bool nextFileFound = false;
  do {
    // file name
    entry = readdir(dirPtr);
    if (entry == NULL)
      continue;

    // is this one of the files we are looking for?
    if (strstr(entry->d_name, _inputSubstring.c_str()) == NULL)
      continue;

    // file modification time and size
    string fullPath = _dataDir  + "/" + entry->d_name;
    if (ta_stat(fullPath.c_str(), &fileStatus) != 0) {
      cerr << "Error: Cannot stat file " << fullPath
        << " because " << errno;
      continue;
    }

    // is this file past its "Sell By" date?
    if (_maxValidAge > 0) {
      time_t current_time = time((time_t *)NULL);
      if (current_time - fileStatus.st_mtime > _maxValidAge)
        continue;
    }

    // has this file already been processed?
    string filename = entry->d_name;
    if (alreadyProcessed(filename, fileStatus.st_mtime, fileStatus.st_size))
      continue;

    // Next file found.  Log it as "processed".
    fileInfo info;
    info.name = entry->d_name;
    info.modified = fileStatus.st_mtime;
    info.size = fileStatus.st_size;
    _processed.push_back(info);

    // prepare to return this file
    _currentInputPath = fullPath;
    nextFileFound = true;
  } while (entry != NULL && !nextFileFound);

  // close the input directory
  closedir(dirPtr);

  return _currentInputPath;
}


const string& NewFilesInputStrategy::next()
{
  getNextFile();

  while (_currentInputPath == "") {
    if (_heartbeatFunc != NULL) {
      _heartbeatFunc("Checking for data");
    }

    sleep(10);
    getNextFile();
  }

  return _currentInputPath;
}

