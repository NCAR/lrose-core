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
// EuMetSat2Mdv.hh
//
// EuMetSat2Mdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2000
//
///////////////////////////////////////////////////////////////

#ifndef EuMetSat2Mdv_H
#define EuMetSat2Mdv_H

#include <toolsa/umisc.h>
#include <string>
#include <iostream>

#include "Args.hh"
#include "Params.hh"
#include "OutputFile.hh"
#include "ChannelSet.hh"
#include "FieldSet.hh"

using namespace std;

class DsInputPath;

class EuMetSat2Mdv {
  
public:

  // constructor

  EuMetSat2Mdv (int argc, char **argv);

  // destructor
  
  ~EuMetSat2Mdv();

  // run 

  int Run();

  // data members

  int OK;

protected:
  
private:

  string _progName;
  Args _args;
  Params _params;
  char *_paramsPath;
  DsInputPath *_input;

  ChannelSet *_channelSet;
  FieldSet *_fieldSet;

  vector<OutputFile *> _outputFiles;

  int _processFile (const char *file_path);

  void _loadTimeOrderedPaths(vector<string> &timeOrderedPaths);
  int _loadFileTimes(const vector<string> &filePaths,
                     vector<time_t> &fileTimes);
  

};

#endif
