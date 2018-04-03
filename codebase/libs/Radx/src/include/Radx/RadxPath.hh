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
////////////////////////////////////////////////////////////////////////////////
//
// RadxPath - class for handling paths
//
// Based on RAL toolsa/Path class.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef RADX_PATH_HH
#define RADX_PATH_HH

#include <ctime>
#include <string>
#include <Radx/RadxTime.hh>

using namespace std;

class RadxPath {

public:
 
  RadxPath();
  RadxPath(const string &fullPath);
  RadxPath(const char *fullPath);
  RadxPath(const string &dir, const string &filename);
  RadxPath(const RadxPath &p);
  ~RadxPath(){};

  // Constants for path management

  enum fileDelimiter {
    UNIX_FILE_DELIMITER = '/',
    DOS_FILE_DELIMITER = '\\'
  };

  static const char *DOT;
  static const char *DOTDOT;
  static const char *CWD;

  RadxPath& operator= (const RadxPath& p);
  RadxPath& operator= (const string& location);
  RadxPath& operator= (const char* location);
 
  void copy(const RadxPath& p);
  void clear();
  bool isValid(){ return(_path.empty() ? false : true); }

  // is this a directory?
  
  bool isDir();

  // Make the directory.
  // Returns 0 on success, -1 on failure.
  // errno is set.
  
  int makeDir(int mode=0775);

  // Make the directory recursively.
  // All parent directories will be recursively created.
  // Returns 0 on success, -1 on failure.
  
  int makeDirRecurse();

  // Static methods can be called with any path, i.e.,
  // these methods do not rely on the members of the RadxPath class

  static bool doStat(const char *location, struct stat &statInfo);
  static bool doStat(const string &location, struct stat &statInfo)
  { return(RadxPath::doStat(location.c_str(), statInfo)); }
  static bool doStat(const RadxPath &location, struct stat &statInfo)
  { return(RadxPath::doStat(location.getPath(), statInfo)); }

  static bool exists(const char *location);
  static bool exists(const string &location)
  { return(exists(location.c_str())); }
  static bool exists(const RadxPath &location)
  { return(exists(location.getPath().c_str())); }

  // Class-specific methods for checking the existence of a path

  bool pathExists();
  bool dirExists();

  const string& getPath() const { return _path; }
  const string& getDirectory() const;
  const string& getFile() const { return _file; }
  const string& getBase() const { return _base; }
  const string& getExt() const { return _ext; }
  const string& getDelimiter() const { return _delimiter; }
  const string& getDelimeter() const { return _delimiter; } // deprecated

  void setPath(const char *newPath);
  void setPath(const string &newPath);

  void setDirectory(const char *dir);
  void setDirectory(const string &dir);
  void setDirectory(const char *topDir, 
                    time_t year, time_t month, time_t day);
  void setDirectory(const string &topDir, 
                    time_t year, time_t month, time_t day);

  void setFile(const char *file);
  void setFile(const string &file);
  void setFile(const time_t hour, const time_t min, const time_t sec,
               const char *ext);
  void setFile(const time_t hour, const time_t min, const time_t sec,
               const string &ext);
  void setFile(const RadxTime &file_time, const char *ext);
  void setFile(const RadxTime &file_time, const string &ext);
  void setFile(const char *base, const RadxTime &file_time, const char *ext);
  void setFile(const string &base, const RadxTime &file_time,
               const string &ext);
 
  void setBase(const char *baseName);
  void setBase(const string &baseName);

  void setExt(const char *extName);
  void setExt(const string &extName);

  void setDelimiter(const char *d){ _delimiter = d; }
  void setDelimiter(const string &d){ _delimiter = d; };

  void print(ostream &out) const;

  // Compute and return a temporary path, in the same directory as
  // the full path.
  //
  // If tmp_file_name is non-empty, it is used for the file name.
  // If it is empty, the name is 'tmp_pid.tmp', where pid is
  // determined using the getpid() function.
  
  string computeTmpPath(const char *tmp_name = NULL);

  // Strip dir from the start of the given path, to leave the
  // file portion.

  static void stripDir(const string &dir, const string &path, string &file);

  static int makeDir(const char *path);
  static int makeDirRecurse(const char *path);

  // constants

  static const int RADX_MAX_PATH_LEN = 1024;
  static const char *RADX_PATH_DELIM;

private:

  string _delimiter;
  string _path;
  string _directory;
  string _file;
  string _base;
  string _ext;
  string _dot;

  void parse(const string &location);
  void compose();

};


// Macro for testing for empty string

#ifndef IS_EMPTY
#define IS_EMPTY(a) (a[0] == '\0')
#endif

#endif
