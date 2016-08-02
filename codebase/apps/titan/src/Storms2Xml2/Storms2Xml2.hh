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
// Storms2Xml2.h
//
// Storms2Xml2 object
//
// A Person, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2001
//
///////////////////////////////////////////////////////////////
//
// Storms2Xml2 produces SPDB output from TITAN binary files.
//
///////////////////////////////////////////////////////////////

#ifndef Storms2Xml2_H
#define Storms2Xml2_H

#include "Args.hh"
#include "Params.hh"
#include <toolsa/os_config.h>
#include <cstdio>

class Storms2Xml2 {
  
public:
  
  // constructor

  Storms2Xml2(int argc, char **argv);

  // destructor
  
  ~Storms2Xml2();

  // run 

  int Run();

  // is constructor OK?

  bool OK;
  
protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  time_t outTime;
  FILE *outFile;
  char outRelFileName[MAX_PATH_LEN];
  char outAbsFilePath[MAX_PATH_LEN];

  // functions

  int _run(time_t start, time_t end);

  int _openOutput(time_t when);
  void _closeOutput();

};

#endif
