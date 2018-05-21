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
/************************************************************************
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
 ************************************************************************/

#ifndef InputDir_H
#define InputDir_H

#include <cstdio>
#include <string>
#include <cassert>

#include <sys/types.h>
#include <dirent.h>

using namespace std;


class InputDir
{

public :
  
  // Constructors
  //
  // Note: if process_old_files is false, only the latest file will be processed.
  
  InputDir(const string &dir_name = "",
           const string &file_substring = "",
           bool process_old_files = false,
           const string &exclude_substring = "",
           bool debug = false,
           bool verbose = false);
  
  // InputDir(const char *dir_name,
  //          const char *file_substring,
  //          int process_old_files,  // Set to 1 to force scan of old files
  //          const string &exclude_substring = "",
  //          bool debug = false,
  //          bool verbose = false);
  
  // Destructor
  
  virtual ~InputDir(void);
  
  // Setting debugging
  
  void setDebug(bool val) { _debug = val; }
  
  void setVerbose(bool val) { 
    _verbose = val;
    if (_verbose) {
      _debug = true;
    }
  }

  // Get the next input filename.  Call this in a tight loop until
  // NULL is returned (indicating no more new files).
  
  virtual char *getNextFilename(int check_dir_flag, // Set flag to 1 to check for new/updated files
                                int max_input_file_age = -1);
  
  // get latest time of any file in the dir

  time_t getLatestFileTime();

  ////////////////////
  // set methods    //
  ////////////////////
  
  void setDirName(const string &dir_name);
  
  inline void setFileSubstring(const string &file_substring)
  {
    _fileSubstring = file_substring;
  }
  
  inline void setExcludeSubstring(const string &exclude_substring)
  {
    _excludeSubstring = exclude_substring;
  }
  
  inline void setProcessOldFiles(const bool process_old_files)
  {
    if (process_old_files) {
      _lastDirUpdateTime = -1;
    } else {
      _lastDirUpdateTime = getLatestFileTime() - 1;
    }
  }
  
  // Create public access to private method _rewindDir()
  inline bool rewindDir(const int check_dir_flag)
  {
    bool returnBool;
    _lastDirUpdateTime = -1;
    returnBool = _rewindDir(check_dir_flag);
    return(returnBool);
  }
  
  ////////////////////
  // get methods    //
  ////////////////////
  
  const string &getDirName() const { return _dirName; }
  const string &getFileSubstring() const { return _fileSubstring; }
  const string &getExcludeSubstring() const { return _excludeSubstring; }

  time_t getLastDirUpdateTime() const { return _lastDirUpdateTime; }
  time_t getLastDirRewindTime() const { return _lastDirRewindTime; }
  time_t getLastDataFileTime() const { return _lastDataFileTime; }
  
protected :

  string _dirName;
  DIR *_dirPtr;
  string _fileSubstring;
  string _excludeSubstring;
  // char _errorMsg[1024];
  
  time_t _lastDirUpdateTime;
  time_t _lastDirRewindTime;
  time_t _lastDataFileTime;
  bool _rewindDirFlag;
  
  bool _debug;
  bool _verbose;

  void _init(const string &dir_name,
	     const string &file_substring,
	     const string &exclude_substring,
	     const bool process_old_files);
  
  bool _rewindDir(const int check_dir_flag);

};


#endif
