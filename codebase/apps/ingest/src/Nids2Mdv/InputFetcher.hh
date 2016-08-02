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
// InputFetcher.hh
//
// Abstract class to fetch file names for processing.
//   Subclasses implement Archive and Realtime fetching strategies.
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2001
//
/////////////////////////////////////////////////////////////

#ifndef InputFetcher_HH
#define InputFetcher_HH

#include <vector>
#include <string>
#include <iostream>
#include <didss/DsInputPath.hh>
#include <dataport/port_types.h>
#include <rapformats/nids_file.h>
#include "Params.hh"
#include "OutputMdv.hh"
using namespace std;

class InputFetcher {
  
public:

  // constructor

  InputFetcher(const string & progName,
               const bool &   verbose);

  // destructor

  virtual ~InputFetcher();

  // process a file

  virtual int initInputPaths() = 0;
  virtual int fetchNextFile(string & nextFile) = 0;

protected:
  
  string _progName;
  bool   _verbose;
  DsInputPath * _input;

private:

  InputFetcher();

};

#endif

