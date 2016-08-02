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
// RadxReadDir.hh
//
// Wraps readdir_r, for making thread-safe readdir calls
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// June 2010
//
////////////////////////////////////////////////////////////////////

#ifndef RadxReadDir_HH
#define RadxReadDir_HH

#include <sys/types.h>
#include <dirent.h>
#include <string>
#include <Radx/RadxBuf.hh>
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

class RadxReadDir {

public:

  // default constructor

  RadxReadDir();
  
  // destructor
  
  ~RadxReadDir();

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
  RadxBuf _buf;
  DIR *_dirp;

private:
  
  //////////////////////////////////////////////////
  // Copy constructor and operator =
  // private with no body provided.
  //   Do not use.
  
  RadxReadDir(const RadxReadDir & orig);
  RadxReadDir&  operator = ( const RadxReadDir& orig );
  
};
  
#endif 
