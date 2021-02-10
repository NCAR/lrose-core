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
// $Id: Path.hh,v 1.23 2019/02/28 17:53:40 prestop Exp $
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _PATH_INC_
#define _PATH_INC_


#include <ctime>
//#include <cstdlib>
#include <string>

#include <toolsa/DateTime.hh>

using namespace std;

class Path {

public:
  
   Path();
   Path( const string &fullPath );
   Path( const char *fullPath );
   Path( const string &dir, const string &filename );
   Path( const Path &p );
  ~Path(){};

   //
   // Constants for path management
   //
   enum fileDelimiter { UNIX_FILE_DELIMITER = '/',
                        DOS_FILE_DELIMITER  = '\\'
                      };

   static const char *DOT;
   static const char *DOTDOT;
   static const char *CWD;
   static const char *EMPTY_STRING;

   Path&  operator= ( const Path& p );
   Path&  operator= ( const string& location );
   Path&  operator= ( const char* location );

   void   copy( const Path& p );
   void   clear();
   bool   isValid(){ return( path.empty() ? false : true ); }

   // Make the directory.
   // Returns 0 on success, -1 on failure.
   // errno is set.
   int    makeDir( int mode=0775 );

   // Make the directory recursively.
   // All parent directories will be recursively created.
   // Returns 0 on success, -1 on failure.
   int    makeDirRecurse();

   //
   // Static methods can be called with any path, i.e.,
   // these methods do not rely on the members of the Path class
   //
   static bool    stat( const char *location, struct stat &statInfo );
   static bool    stat( const string &location, struct stat &statInfo )
                        { return( Path::stat( location.c_str(), statInfo )); }
   static bool    stat( const Path &location, struct stat &statInfo )
                        { return( Path::stat( location.getPath(), statInfo )); }

   static bool    exists( const char *location );
   static bool    exists( const string &location )
                        { return( exists( location.c_str() )); }
   static bool    exists( const Path &location )
                        { return( exists( location.getPath().c_str() )); }

   //
   // Class-specific methods for checking the existence of a path
   //
   bool           pathExists();
   bool           dirExists();

   const string&  getPath()      const { return path; }
   const string&  getDirectory() const;
   const string&  getFile()      const { return file; }
   const string&  getBase()      const { return base; }
   const string&  getExt()       const { return ext; }
   const string&  getDelimiter() const { return delimiter; }
   const string&  getDelimeter() const { return delimiter; } // deprecated

   void   setPath( const char *newPath );
   void   setPath( const string &newPath );

   void   setDirectory( const char *dir );
   void   setDirectory( const string &dir );
   void   setDirectory( const char *topDir, 
                        time_t year, time_t month, time_t day );
   void   setDirectory( const string &topDir, 
                        time_t year, time_t month, time_t day );

   void   setFile( const char *file );
   void   setFile( const string &file );
   void   setFile( const time_t hour, const time_t min, const time_t sec, const char *ext );
   void   setFile( const time_t hour, const time_t min, const time_t sec, const string &ext );
   void   setFile( const DateTime &file_time, const char *ext );
   void   setFile( const DateTime &file_time, const string &ext );
   void   setFile( const char *base, const DateTime &file_time, const char *ext );
   void   setFile( const string &base, const DateTime &file_time, const string &ext);
  
   void   setBase( const char *baseName );
   void   setBase( const string &baseName );

   void   setExt( const char *extName );
   void   setExt( const string &extName );

   void   setDelimiter( const char *d ){ delimiter = d; }
   void   setDelimiter( const string &d ){ delimiter = d; };

   void   setDelimeter( const char *d ){ delimiter = d; }    // deprecated
   void   setDelimeter( const string &d ){ delimiter = d; }; // deprecated

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
  static void stripDir1(const string &dir, const string &path, string &file);

  // Get the path of the executable binary that is running

  static string getExecPath();

  // Get the path of a file relative to the
  // executable binary that is running
  
  static string getPathRelToExec(const string &relPath);

  // looks for a directory in the path that is all digits & 8 characters long.
  // if a dated dir cannot be found, pre, date, and post are set to empty strings.
  //
  // e.g. given path="/some/path/to/YYYYMMDD/more/stuff.file"
  // returns:
  //          pre="/some/path/to/"
  //          date="YYYMMDD"
  //          post="/more/stuff.file"

  static void splitDatedDir(const string &path, string &pre, string &date,
			    string &post);
  
private:

   string delimiter;
   string path;
   string directory;
   string file;
   string base;
   string ext;
   string dot;

   void   parse( const string &location );
   void   compose();
};

//
// Macro for testing for empty string
//
#ifndef ISEMPTY
#define ISEMPTY(a)  ( a[0] == '\0' )
#endif

#endif
