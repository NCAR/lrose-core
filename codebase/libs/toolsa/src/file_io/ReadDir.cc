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
// ReadDir.cc
//
// Wraps readdir_r, for making thread-safe readdir calls
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// April 2000
//
////////////////////////////////////////////////////////////////////


#include <toolsa/ReadDir.hh>
#include <unistd.h>
#include <cerrno>
#include <toolsa/os_config.h>
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

// constructor

ReadDir::ReadDir()

{
  _dirp = NULL;
}

// destructor

ReadDir::~ReadDir()
  
{
  close();
}

//////////////////////////////////////////
// open the directory 
//
// Also sets up the buffer for readdir_r.

int ReadDir::open(const char *dir_path)
  
{
  
  // The following is from the man page on solaris:
  //
  //   The readdir_r() function is equivalent to  readdir()  except
  //   that a buffer result must be supplied by the caller to store
  //   the result. The  size  should  be  sizeof(struct  dirent)  +
  //   {NAME_MAX}    (that   is,   pathconf(_PC_NAME_MAX))   +   1.
  //   _PC_NAME_MAX is defined in  <unistd.h>.
  
  // reserve buffer for the results

  int bufSize =
    sizeof(struct  dirent) + pathconf(dir_path, _PC_NAME_MAX) + 1;
  _buf.reserve(bufSize);
  
  if ((_dirp = opendir(dir_path)) == NULL) {
    return -1;
  }
  
  return 0;
  
}

////////////////////////////////////////
// read the next entry in the directory

struct dirent *ReadDir::read()
  
{
  
  return readdir(_dirp);

  // #if defined(SUNOS5) || defined (SUNOS5_INTEL)
  //   return (readdir(_dirp));
  // #else
  //   struct dirent *result;
  //   if (readdir_r(_dirp, (struct dirent *) _buf.getPtr(), &result)) {
  //     return NULL;
  //   }
  //   return result;
  // #endif

}

//////////////////////////////////////////
// close the directory 
//
// Also free up the buffer.

int ReadDir::close()
  
{

  _buf.free();
  if (_dirp != NULL) {
    if (closedir(_dirp)) {
      return -1;
    }
  }
  _dirp = NULL;
  return 0;

}


