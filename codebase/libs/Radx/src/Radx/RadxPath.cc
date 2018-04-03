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
// Path handling in RADX
//
// Based on RAL toolsa/Path class.
//
////////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <Radx/RadxPath.hh>
using namespace std;

// static definitions

const char *RadxPath::DOT = ".";
const char *RadxPath::DOTDOT = "..";
const char *RadxPath::CWD = ".";
const char *RadxPath::RADX_PATH_DELIM = "/";

RadxPath::RadxPath()
{
  _delimiter = UNIX_FILE_DELIMITER;
  _dot = DOT;
}

RadxPath::RadxPath(const string &location)
{
  _delimiter = UNIX_FILE_DELIMITER;
  _dot = DOT;
  setPath(location);
}

RadxPath::RadxPath(const char *location)
{
  _delimiter = UNIX_FILE_DELIMITER;
  _dot = DOT;
  setPath(location);
}

RadxPath::RadxPath(const string &dir, const string &filename)
{
  _delimiter = UNIX_FILE_DELIMITER;
  _dot = DOT;
  setDirectory(dir);
  setFile(filename);
}

RadxPath::RadxPath(const RadxPath &p)
{
  copy(p);
}

RadxPath& RadxPath::operator= (const RadxPath &p)
{
  if (this != &p) {
    copy(p);
  }
  return *this;
}

RadxPath& RadxPath::operator= (const string &location)
{
  setPath(location);
  return *this;
}

RadxPath& RadxPath::operator= (const char* location) 
{
  setPath(location);
  return *this;
}

void RadxPath::copy(const RadxPath &p) 
{
  _path = p._path;
  _directory = p._directory;
  _file = p._file;
  _base = p._base;
  _ext = p._ext;
  _delimiter = p._delimiter;
  _dot = p._dot;
}

void RadxPath::clear()
{
  _path.clear();
  _directory.clear();
  _file.clear();
  _base.clear();
  _ext.clear();
}

bool RadxPath::exists(const char *location)
{
  struct stat statInfo;
  return(RadxPath::doStat(location, statInfo));
}

bool RadxPath::pathExists()
{
  return(exists(_path.c_str())); 
}

bool RadxPath::dirExists()
{ 
  // empty dir must exist - it is the current directory
  if (_directory.empty()) {
    return true;
  }
  return(exists(_directory.c_str()));
}

// is this a directory?

bool RadxPath::isDir()
{ 
  struct stat stat_buf;
  if (stat(_path.c_str(), &stat_buf)) {
    return false;
  }
  if ((stat_buf.st_mode & S_IFMT) == S_IFDIR) {
    return true;
  }
  return false;
}

bool RadxPath::doStat(const char *location, struct stat &statInfo)
{
  if (location == NULL ||
      IS_EMPTY(location) ||
      stat(location, &statInfo)) {
    return false;
  } else {
    return true;
  }
}

// make the directory
// returns 0 on success, -1 on failure.
// errno is set by mkdir call.

int RadxPath::makeDir(int mode /* = 0775*/)
{
  if (dirExists()) {
    return(0);
  } else {
    return(mkdir(_directory.c_str(), mode));
  }
}


// Make the directory recursively.
// All parent directories will be recursively created.
// Returns 0 on success, -1 on failure.

int RadxPath::makeDirRecurse()
{
  if (dirExists()) {
    return(0);
  } else {
    return(makeDirRecurse(_directory.c_str()));
  }
}

void RadxPath::setDirectory(const char *dirName)
{
  setDirectory((string)dirName);
}

void RadxPath::setDirectory(const string &dirName)
{
  _directory.clear();
  if (!dirName.empty() && (dirName != _dot)) {
    _directory = dirName;
  }
  compose();
}

void RadxPath::setDirectory(const char *topDir,
                            time_t year, time_t month, time_t day)
{
  setDirectory((string)topDir, year, month, day);
}

void RadxPath::setDirectory(const string &topDir,
                            time_t year, time_t month, time_t day)
{
  _directory.clear();
  if (!topDir.empty()) {
    char dirName[10];
    sprintf(dirName, "%4ld%02ld%02ld", year, month, day);
    if (topDir != _dot) {
      _directory = topDir + _delimiter + dirName;
    } else {
      _directory = dirName;
    }
  }
  compose();
}

const string& RadxPath::getDirectory() const
{
  if (_directory.empty()) {
    return _dot;
  } else {
    return _directory;
  }
}

void RadxPath::setFile(const char *fileName)
{
  setFile((string)fileName);
}

void RadxPath::setFile(const string &fileName)
{
  _file.clear();
  if (!fileName.empty()) {
    _file = fileName;
  }
  compose();
}

void RadxPath::setFile(const time_t hour, const time_t min, const time_t sec, const char *ext)
{
  setFile(hour, min, sec, (string)ext);
}

void RadxPath::setFile(const time_t hour, const time_t min, const time_t sec, const string &ext)
{
  _file.clear();
  char fileName[10];
  sprintf(fileName, "%02ld%02ld%02ld", hour, min, sec);
  _file = fileName;
  if (!ext.empty()) {
    _file += DOT + ext;
  }
  compose();
}

void RadxPath::setFile(const RadxTime &file_time, const char *ext)
{
  setFile(file_time, (string)ext);
}

void RadxPath::setFile(const RadxTime &file_time, const string &ext)
{
  _file.clear();
  char fileName[10];
  sprintf(fileName, "%02d%02d%02d",
          file_time.getHour(), file_time.getMin(), file_time.getSec());
  _file = fileName;
  
  if (!ext.empty()) {
    _file += DOT + ext;
  }

  compose();
}

void RadxPath::setFile(const char *base, const RadxTime &file_time, const char *ext)
{
  setFile((string)base, file_time, (string)ext);
}

void RadxPath:: setFile(const string &base, const RadxTime &file_time, 
                        const string &ext)
{
  _file = base + DOT + file_time.getStrPlain();

  if (!ext.empty()) {
    _file += DOT;
    _file += ext;
  }

  compose(); 
}


void RadxPath::setPath(const char *location)
{
  setPath((string)location);
}

void RadxPath::setPath(const string &location)
{
  clear();

  if (!location.empty())
    parse(location);
}

void RadxPath::setBase(const char *baseName)
{
  setBase((string)baseName);
}

void RadxPath::setBase(const string &baseName)
{
  _base.clear();
  if (!baseName.empty()) {
    _base = baseName;
  }
  _file = _base + DOT + _ext;
  compose();
}

void RadxPath::setExt(const char *extName)
{
  setExt((string)extName);
}

void RadxPath::setExt(const string &extName)
{

  _ext.clear();
  
  if (!extName.empty()) {
    _ext = extName;
  }

  _file = _base + DOT + _ext;

  compose();
}

void RadxPath::parse(const string &newPath)
{
  size_t delimPos = newPath.rfind(_delimiter);
  size_t length = newPath.length();

  if (newPath == DOTDOT || 
      newPath == DOT || 
      delimPos == length-1) {

    // The path is just a directory name (no file name)

    _directory = newPath;
  }

  else {

    // We've got a file name

    if (delimPos > length) {

      // The path is just a file name (current directory implied)

      _file = newPath;
      _directory.clear();
    } 

    else {

      // This is a composite name (directory and file)

      _file = newPath.substr(delimPos+1);
      _directory = newPath.substr(0, delimPos);
    }
  }


  // Compose the full path from the results of our parsing

  compose();

}

void RadxPath::compose()
{

  _path.clear();

  if (_directory.empty()) {
    _path = _file;
  } else if (_file.empty()) {
    _path = _directory;
  } else {
    _path = _directory + _delimiter + _file;
  }
 
  // extension

  _ext = "";
  size_t dotPos = _file.rfind(DOT);
  if (dotPos != string::npos) {
    _ext.assign(_file, dotPos + 1, string::npos);
    _base.assign(_file, 0, dotPos);
  } else {
    _base = _file;
  }

}

void RadxPath::print(ostream &out) const
{

  out << "path: " << getPath() << endl;
  out << "delimiter: " << getDelimiter() << endl;
  out << "directory: " << getDirectory() << endl;
  out << "file: " << getFile() << endl;
  out << "base: " << getBase() << endl;
  out << "ext: " << getExt() << endl;

}

// Compute and return a temporary path, in the same directory as
// the full path.
//
// If tmp_file_name is not NULL, it is used for the file name.
// If it is NULL, the name is 'tmp_pid.tmp', where pid is
// determined using the getpid() function.

string RadxPath::computeTmpPath(const char *tmp_name /* = NULL*/)

{

  string tmp_path = getDirectory();
  tmp_path += _delimiter;

  if (tmp_name == NULL) {
    char pid_name[128];
    sprintf(pid_name, "tmp_%d.tmp", getpid());
    tmp_path += pid_name;
  } else {
    tmp_path += tmp_name;
  }

  return tmp_path;

}

//////////////////////////////////////////////////////////////////
// Strip dir from the start of the given path, to leave the
// file portion.

void RadxPath::stripDir(const string &dir, const string &path, string &file)
{

  string dirStr = dir;

  // if dir is empty, return the string unchanged.

  if (dirStr.size() == 0) {
    file = path;
    return;
  }

  // add in trailing delimiter if missing
 
  string delimStr = RADX_PATH_DELIM;
  if (dirStr.substr(dirStr.size() - delimStr.size()) != delimStr) {
    dirStr += delimStr;
  }

  // if dir is at start of path, strip it off.
  // otherwise set file to path unchanged.

  if (dirStr == path.substr(0, dirStr.size())) {
    file = path.substr(dirStr.size());
  } else {
    file = path;
  }

  return;

}

//////////////////////////////////////////////////////////////////
// Utility routine to create a directory. If the directory
// already exists, does nothing.
//
// Returns -1 on error, 0 otherwise.

int RadxPath::makeDir(const char *path)
{

  struct stat stat_buf;
 
  // Status the directory to see if it already exists.

  if (stat(path, &stat_buf) == 0) {
    return 0;
  }
 
  // Directory doesn't exist, create it.

  if (mkdir(path,
            S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {

    // failed
    // check if dir has been made bu some other process
    // in the mean time, in which case return success

    if (stat(path, &stat_buf) == 0) {
      return 0;
    }

    return-1;

  }
 
  return 0;
}


//////////////////////////////////////////////////////////////////
// makeDirRecurse()
//
// Utility routine to create a directory recursively.
// If the directory already exists, does nothing.
// Otherwise it recurses through the path, making all
// needed directories.
//
// Returns -1 on error, 0 otherwise.

int RadxPath::makeDirRecurse(const char *path)
{

  char up_dir[RADX_MAX_PATH_LEN];
  char *last_delim;
  struct stat dir_stat;
  int delim = RADX_PATH_DELIM[0];
 
  // Status the directory to see if it already exists.
  // '/' dir will always exist, so this stops the recursion
  // automatically.
 
  if (stat(path, &dir_stat) == 0) {
    return 0;
  }
 
  // create up dir - one up the directory tree -
  // by searching for the previous delim and removing it
  // from the string.
  // If no delim, try to make the directory non-recursively.
 
  strncpy(up_dir, path, RADX_MAX_PATH_LEN);
  last_delim = strrchr(up_dir, delim);
  if (last_delim == NULL) {
    return (makeDir(up_dir));
  }
  *last_delim = '\0';
 
  // make the up dir
 
  if (makeDirRecurse(up_dir)) {
    return -1;
  }

  // make this dir

  if (makeDir(path)) {
    return-1;
  } else {
    return 0;
  }

}

