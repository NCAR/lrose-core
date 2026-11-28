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
/////////////////////////////////////////////////////////////
// Args.h: Command line object
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1999
//
/////////////////////////////////////////////////////////////
//
// This is a small class that deals with command line arguments.
// Specifically, if "-f" is specified then a list of input file names
// is expected, if "-h" is specified then a help message is printed
// and the program exits.
//
// Use the following 'ifndef' construct to be sure that this
// file only gets included once.
//
#ifndef ARGS_H
#define ARGS_H
//
// Include a few things and use the standard namespace.
//
#include <cstdio>
#include <iostream>
#include <tdrp/tdrp.h>
using namespace std;
//
// Define the class.
//
class Args {
  
public:

  // constructor - does most of the work.

  Args (int argc, char **argv, char *prog_name);

  // destructor

  ~Args ();

  // public data - how clients access this class.

  char **filePaths;
  int nFiles;
  bool OK;
  tdrp_override_t override;
  char *paramsFilePath;

protected:
  
private:
  //
  // Small routine to print a help message.
  //
  void usage(char *prog_name, ostream &out);
  
};

#endif

