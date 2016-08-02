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
///////////////////////////////////////////////////////////////
// Input.cc
//
// Input object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2007
//
///////////////////////////////////////////////////////////////
//
// Input handles time series input from files.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <iostream>
#include <iomanip>
#include <toolsa/os_config.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/mem.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include "Input.hh"
using namespace std;

////////////////////////////////////////////////////
// Constructor

Input::Input(const string &label,
             const Params &params,
             const vector<string> &fileList) :
        _label(label),
        _params(params)

{

  _dsInput = NULL;
  _opsInfo = NULL;
  _in = NULL;
  _pulseSeqNum = 0;
  isOK = true;
  
  // check that file list is set
  
  if (fileList.size() == 0) {
    cerr << "ERROR: Input::Input." << endl;
    cerr << "  You must specify files for: " << label << endl;
    isOK = false;
    return;
  }

  // initialize the data input path objects
  
  _dsInput = new DsInputPath("TsMerge",
                             _params.debug >= Params::DEBUG_EXTRA,
                             fileList);

  // create OpsInfo object
  
  _opsInfo = new OpsInfo(_params);

}

//////////////////////////////////////////////////////////////////
// destructor

Input::~Input()

{

  if (_dsInput) {
    delete _dsInput;
  }

  if (_opsInfo) {
    delete _opsInfo;
  }

  if (_in != NULL) {
    fclose(_in);
  }

}

//////////////////////////////////////////////////
// read next pulse
// returns Pulse on success, NULL on failure (end of data)
// New Pulse is allocated.
// Pulse must be deleted by calling function.

Pulse *Input::readNextPulse()

{

  if (_in == NULL) {
    if (_openNextFile()) {
      return NULL;
    }
  }
  
  // Create a new pulse object
  
  Pulse *pulse = new Pulse(_params, _opsInfo->fSyClkMHz);
    
  // read in pulse headers and data
    
  while (pulse->read(_in)) {
    
    // failure, so close file and try next one

    fclose(_in);
    _in = NULL;
    if (_openNextFile()) {
      // no more files
      delete pulse;
      return NULL;
    }

  }

  // print missing pulses if requested
  
  if ((int) pulse->iSeqNum!= _pulseSeqNum + 1) {
    if (_params.debug >= Params::DEBUG_VERBOSE && _pulseSeqNum != 0) {
      cout << "**************** Missing seq num: " << _pulseSeqNum
	   << " to " <<  pulse->iSeqNum << " **************" << endl;
    }
  }
  _pulseSeqNum = pulse->iSeqNum;

  return pulse;

}

//////////////////////////////////////////////////////
// open the next available file
// returns 0 on success, -1 on failure (no more files)

int Input::_openNextFile()

{

  if (_in != NULL) {
    fclose(_in);
  }
  _in = NULL;

  char *inputPath = _dsInput->next();
  if (inputPath == NULL) {
    return -1;
  }

  if (_params.debug) {
    cerr << "  Opening " << _label << " file: " << inputPath << endl;
  }

  // open file
  
  if ((_in = fopen(inputPath, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Input::_openNextFile" << endl;
    cerr << "  Cannot open file: " << inputPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // read in ops info
  
  if (_opsInfo->read(_in)) {
    cerr << "ERROR - Input::_openNextFile" << endl;
    cerr << "  Cannot read pulse info" << endl;
    cerr << "  File: " << inputPath << endl;
    fclose(_in);
    _in = NULL;
    return -1;
  }

  return 0;

}

