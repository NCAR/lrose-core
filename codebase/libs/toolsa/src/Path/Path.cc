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
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// January 1998
//
// $Id: Path.cc,v 1.33 2019/02/28 17:53:40 prestop Exp $
//
////////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/time.h>
#include <toolsa/Path.hh>
#include <toolsa/file_io.h>
#include <unistd.h> // Added by Niles for getpid() proto
#include <stdlib.h>

using namespace std;
//
// static definitions
//
const char *Path::DOT            = ".";
const char *Path::DOTDOT         = "..";
const char *Path::CWD            = ".";
const char *Path::EMPTY_STRING   = "";

Path::Path()
{
   delimiter = UNIX_FILE_DELIMITER;
   dot = DOT;
}

Path::Path( const string &location )
{
   delimiter = UNIX_FILE_DELIMITER;
   dot = DOT;
   setPath( location );
}

Path::Path( const char *location )
{
   delimiter = UNIX_FILE_DELIMITER;
   dot = DOT;
   setPath( location );
}

Path::Path( const string &dir, const string &filename )
{
   delimiter = UNIX_FILE_DELIMITER;
   dot = DOT;
   setDirectory( dir );
   setFile( filename );
}

Path::Path( const Path &p )
{
   copy( p );
}

Path& Path::operator= ( const Path &p )
{
   if (this != &p) {
     copy( p );
   }
   return *this;
}

Path& Path::operator= ( const string &location )
{
   setPath( location );
   return *this;
}

Path& Path::operator= ( const char* location ) 
{
   setPath( location );
   return *this;
}

void Path::copy( const Path &p ) 
{
   path      = p.path;
   directory = p.directory;
   file      = p.file;
   base      = p.base;
   ext       = p.ext;
   delimiter = p.delimiter;
   dot       = p.dot;
}

void Path::clear()
{
   //
   // Can't use the erase method until we are all upgraded to libc6
   //
   //path.erase();
   //directory.erase();
   //file.erase();
   //

   path      = EMPTY_STRING;
   directory = EMPTY_STRING;
   file      = EMPTY_STRING;
   base      = EMPTY_STRING;
   ext       = EMPTY_STRING;
}

bool Path::exists( const char *location )
{
   struct stat  statInfo;
   return( Path::stat( location, statInfo ));
}

bool Path::pathExists()
{
  return( exists( path.c_str() )); 
}

bool Path::dirExists()
{ 
  // empty dir must exist - it is the current directory
  if ( directory.empty() ) {
    return true;
  }
  return( exists( directory.c_str() ));
}


bool Path::stat( const char *location, struct stat &statInfo )
{
   if ( location == NULL  ||
        ISEMPTY( location )  ||
        ta_stat( location, &statInfo ) )
      return false;
   else
      return true;
}

// make the directory
// returns 0 on success, -1 on failure.
// errno is set by mkdir call.

int Path::makeDir( int mode /* = 0775*/ )
{
   if ( dirExists() )
      return( 0 );
   else
      return( mkdir( directory.c_str(), mode ));
}


// Make the directory recursively.
// All parent directories will be recursively created.
// Returns 0 on success, -1 on failure.

int Path::makeDirRecurse()
{
   if ( dirExists() )
      return( 0 );
   else
      return( ta_makedir_recurse( directory.c_str() ) );
}

void Path::setDirectory( const char *dirName )
{
  setDirectory((string)dirName);
}

void Path::setDirectory( const string &dirName )
{
   //directory.erase();
   directory = EMPTY_STRING;

   if ( !dirName.empty() && (dirName != dot) )
      directory = dirName;

   compose();
}

void Path::setDirectory( const char *topDir,
                         time_t year, time_t month, time_t day )
{
  setDirectory((string)topDir, year, month, day);
}

void Path::setDirectory( const string &topDir,
                         time_t year, time_t month, time_t day )
{
   //directory.erase();
   directory = EMPTY_STRING;

   if ( !topDir.empty() ) {
      char dirName[10];
      sprintf( dirName, "%4ld%02ld%02ld", year, month, day );
      if ( topDir != dot) {
	directory = topDir + delimiter + dirName;
      } else {
	directory = dirName;
      }
   }
   compose();
}

const string& Path::getDirectory() const
{
  if (directory.empty()) {
    return dot;
  } else {
    return directory;
  }
}

void Path::setFile( const char *fileName )
{
  setFile((string)fileName);
}

void Path::setFile( const string &fileName )
{
   //file.erase();
   file = EMPTY_STRING;

   if ( !fileName.empty() )
      file = fileName;

   compose();
}

void Path::setFile( const time_t hour, const time_t min, const time_t sec, const char *ext )
{
  setFile(hour, min, sec, (string)ext);
}

void Path::setFile( const time_t hour, const time_t min, const time_t sec, const string &ext )
{
   //file.erase();
   file = EMPTY_STRING;

   char fileName[10];
   sprintf( fileName, "%02ld%02ld%02ld", hour, min, sec );
   file = fileName;

   if ( !ext.empty() )
      file += DOT + ext;

   compose();
}

void Path::setFile( const DateTime &file_time, const char *ext )
{
  setFile(file_time, (string)ext);
}

void Path::setFile( const DateTime &file_time, const string &ext )
{
   //file.erase();
   file = EMPTY_STRING;

   char fileName[10];
   sprintf( fileName, "%02d%02d%02d",
	    file_time.getHour(), file_time.getMin(), file_time.getSec() );
   file = fileName;

   if ( !ext.empty() )
      file += DOT + ext;

   compose();
}

void Path::setFile( const char *base, const DateTime &file_time, const char *ext )
{
  setFile((string)base, file_time, (string)ext);
}

void Path:: setFile( const string &base, const DateTime &file_time, 
		     const string &ext)
{
  file = base + DOT + file_time.getStrPlain();

  if ( !ext.empty() ) {
      file += DOT;
      file += ext;
   }

   compose(); 
}


void Path::setPath( const char *location )
{
  setPath((string)location);
}

void Path::setPath( const string &location )
{
   clear();

   if ( !location.empty() )
      parse( location );
}

void Path::setBase( const char *baseName )
{
  setBase((string)baseName);
}

void Path::setBase( const string &baseName )
{
   //base.erase();
   base = EMPTY_STRING;

   if ( !baseName.empty() )
      base = baseName;

   file = base + DOT + ext;

   compose();
}

void Path::setExt( const char *extName )
{
  setExt((string)extName);
}

void Path::setExt( const string &extName )
{
   //ext.erase();
   ext = EMPTY_STRING;

   if ( !extName.empty() )
      ext = extName;

   file = base + DOT + ext;

   compose();
}

void Path::parse( const string &newPath )
{
   size_t    delimPos = newPath.rfind( delimiter );
   size_t    length   = newPath.length();

   if ( newPath  == DOTDOT || 
        newPath  == DOT    || 
        delimPos == length-1 ) {
      //
      // The path is just a directory name (no file name)
      //
      directory = newPath;
   }

   else {
      //
      // We've got a file name
      //
      if ( delimPos > length ) {
         //
         // The path is just a file name (current directory implied)
         //
         file      = newPath;
         directory = EMPTY_STRING;
      } 

      else {
         //
         // This is a composite name (directory and file)
         //
         file      = newPath.substr( delimPos+1 );
         directory = newPath.substr( 0, delimPos );
      }
   }

   //
   // Compose the full path from the results of our parsing
   //
   compose();
}

void Path::compose()
{
   //path.erase();                                         
   path = EMPTY_STRING;

   if ( directory.empty() ) {
     path = file;
   } else if ( file.empty() ) {
     path = directory;
   } else {
     path = directory + delimiter + file;
   }

   // extension

   ext = "";
   size_t dotPos = file.rfind( DOT );
   if (dotPos != string::npos) {
     ext.assign(file, dotPos + 1, string::npos);
     base.assign(file, 0, dotPos);
   } else {
     base = file;
   }

}

void Path::print(ostream &out) const
{

  cout << "path: " << getPath() << endl;
  cout << "delimiter: " << getDelimiter() << endl;
  cout << "directory: " << getDirectory() << endl;
  cout << "file: " << getFile() << endl;
  cout << "base: " << getBase() << endl;
  cout << "ext: " << getExt() << endl;

}

// Compute and return a temporary path, in the same directory as
// the full path.
//
// If tmp_file_name is not NULL, it is used for the file name.
// If it is NULL, the name is 'tmp_pid.tmp', where pid is
// determined using the getpid() function.

string Path::computeTmpPath(const char *tmp_name /* = NULL*/ )

{

  string tmp_path = getDirectory();
  tmp_path += delimiter;

  // if tmp_name is specified, use that

  if (tmp_name != NULL) {
    tmp_path += tmp_name;
    return tmp_path;
  }

  // create a temp name

  // first get current time in high precision and make a string from it
  
  struct timeval tv;
  gettimeofday(&tv, NULL);
  char timeStr[128];
  sprintf(timeStr, "%ld.%ld_", tv.tv_sec, (long) tv.tv_usec);

  // get PID and make a string from it
  
  char pidStr[128];
  sprintf(pidStr, "%d_", (int) getpid());
  
  // concatenate strings into tmp name

  string computed_name("tmp_");
  computed_name += timeStr;
  computed_name += pidStr;
  computed_name += getBase();
  computed_name += ".tmp";

  // change all numerals to lower case letters a to j

  int delta = 'a' - '0';
  for (size_t ii = 0; ii < computed_name.size(); ii++) {
    if (isdigit(computed_name[ii])) {
      computed_name[ii] += delta;
    }
  }

  // add to path

  tmp_path += computed_name;

  return tmp_path;

}

//////////////////////////////////////////////////////////////////
// Strip dir from the start of the given path, to leave the
// file portion.

void Path::stripDir(const string &dir, const string &path, string &file)
{

  string dirStr = dir;

  // if dir is empty, return the string unchanged.

  if (dirStr.size() == 0) {
    file = path;
    return;
  }

  try {

    // add in trailing delimiter if missing
    
    string delimStr = PATH_DELIM;
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
    
  } catch (out_of_range err) {

    // delim longer than path

    file = path;

  } // try/catch

  return;

}

//////////////////////////////////////////////////////////////////
// getExecPath()
// For apple osx, for now we need to use the C-function in cpath.c
// because of some problems with the includes in a C++ compile

//////////////////////////////////////////////////////////////////
// Get the path of the executable binary that is running

extern "C" {
  extern char *get_exec_path();
}

string Path::getExecPath()
{

  char *epath = get_exec_path();
  string execPath(epath);
  free(epath);
  return execPath;

}

#ifdef NOTNOW

//////////////////////////////////////////////////////////////////
// Get the path of the executable binary that is running

string Path::getExecPath()
{

#if defined(__linux__)

#include <linux/limits.h>

  char execPath[PATH_MAX];
  int length = readlink("/proc/self/exe", execPath, sizeof(execPath));
  if (length < 0) {
    return "";
  }
  if (length >= PATH_MAX) {
    return "";
  }
  execPath[length] = '\0';
  return execPath;

#elif defined (__APPLE__)

#include <libproc.h>

  pid_t pid = getpid();
  char execPath[PROC_PIDPATHINFO_MAXSIZE];
  int ret = proc_pidpath(pid, execPath, sizeof(execPath));
  if ( ret <= 0 ) {
    return "";
  }
  
  return execPath;

#else

  return "";

#endif

}

#endif

//////////////////////////////////////////////////////////////////
// STATIC METHOD
//
// looks for a directory in the path that is all digits & 8 characters long.
// if a dated dir cannot be found, pre, date, and post are set to empty strings.
//
// e.g. given path="/some/path/to/YYYYMMDD/more/stuff.file"
// returns:
//          pre="/some/path/to"
//          date="YYYMMDD"
//          post="more/stuff.file" 
void
Path::splitDatedDir(const string &path, string &pre, string &date,
			    string &post)
{

  date="";
  pre="";
  post="";
  
  //tokenize
  string delimiters = "/";
  size_t current;
  size_t next = -1;
  string token;
  do
    {
      current = next + 1;
      next = path.find_first_of( delimiters, current );
      token = path.substr( current, next - current );

      //is it 8 characters long and all digits?
      if ( (token.length() == 8) && (token.find_first_not_of("0123456789") == std::string::npos) ) {
	date = token;
	pre = path.substr(0,current);
	post = path.substr(next);
	return;
      }
    }
  while (next != string::npos);

}

//////////////////////////////////////////////////////////////////
// Get the path of a file relative to the
// executable binary that is running

string Path::getPathRelToExec(const string &relPath)
{

  // get executable path

  Path execPath(getExecPath());

  // combine the base with the rel path
  
  string absPath = execPath.getDirectory();
  absPath += UNIX_FILE_DELIMITER;
  absPath += relPath;

  return absPath;

}



