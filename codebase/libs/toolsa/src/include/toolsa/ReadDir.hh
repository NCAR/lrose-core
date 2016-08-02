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
////////////////////////////////////////////////////////////////////
// ReadDir.hh
//
// Wraps readdir_r, for making thread-safe readdir calls
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// April 2000
//
////////////////////////////////////////////////////////////////////

#ifndef ReadDir_HH
#define ReadDir_HH


#include <sys/types.h>
#include <dirent.h>
#include <string>
#include <toolsa/MemBuf.hh>
using namespace std;

///////////////////////////////////////////////////////////////////
// NOTES ON USE:
//
// This class provides local memory in which the readdir_r can store
// its results.
//
// To use, instantiate a local copy of the class, use the open()
// method instead of opendir() and the read() method instead of
// readdir().

class ReadDir {

public:

  // default constructor

  ReadDir();
  
  // destructor
  
  ~ReadDir();

  // open the directory - returns 0 on success, -1 on error.
  // errno is as set by opendir().
  
  int open(const char *dir_path);
  
  // read the next entry in the directory
  // Same return vals as readdir()
  
  struct dirent *read();
  
  // close the directory - returns 0 on success, -1 on error.
  // errno is as set by closedir().
  
  int close();
  
protected:

  string _dirPath;
  MemBuf _buf;
  DIR *_dirp;

private:
  
  //////////////////////////////////////////////////
  // Copy constructor and operator =
  // private with no body provided.
  //   Do not use.
  
  ReadDir(const ReadDir & orig);
  ReadDir&  operator = ( const ReadDir& orig );
  
};
  
#endif 
