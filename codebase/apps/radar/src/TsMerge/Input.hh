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
// Input.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2007
//
///////////////////////////////////////////////////////////////
//
// Input handles time series input from files.
//
////////////////////////////////////////////////////////////////

#ifndef Input_hh
#define Input_hh

#include <string>
#include <vector>
#include <deque>
#include <cstdio>
#include <didss/DsInputPath.hh>
#include "Params.hh"
#include "OpsInfo.hh"
#include "Pulse.hh"
using namespace std;

////////////////////////
// This class

class Input {
  
public:

  // constructor
  
  Input (const string &label,
         const Params &params,
         const vector<string> &fileList);

  // destructor
  
  ~Input();

  // read next pulse
  // returns Pulse on success, NULL on failure (end of data)
  // New Pulse is allocated.
  // Pulse must be deleted by calling function.
  
  Pulse *readNextPulse();

  // get methods

  const OpsInfo *getOpsInfo() { return _opsInfo; }

  // constructor status

  bool isOK;

protected:
  
private:

  // basic

  string _label;
  const Params &_params;

  // input data
  
  DsInputPath *_dsInput;
  FILE *_in;

  // pulse info

  OpsInfo *_opsInfo;
  
  // pulse sequence number
  
  long _pulseSeqNum;
  
  // private functions

  int _openNextFile();
  
};

#endif

