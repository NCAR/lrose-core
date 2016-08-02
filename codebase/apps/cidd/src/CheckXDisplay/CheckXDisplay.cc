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
// CheckXDisplay.cc
//
// CheckXDisplay object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2013
//
///////////////////////////////////////////////////////////////
//
// CheckXDisplay checks that the current X display is available
// Exits with 0 if avaiblable, 1 on error.
//
///////////////////////////////////////////////////////////////

#include <X11/Xlib.h>
#include <cmath>
#include "CheckXDisplay.hh"

using namespace std;

// Constructor

CheckXDisplay::CheckXDisplay(int argc, char **argv)
  
{
  
  isOK = true;
  
  // set programe name
  
  _progName = "CheckXDisplay";
  
  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *)"unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }
  
}

// destructor

CheckXDisplay::~CheckXDisplay()
  
{

}

//////////////////////////////////////////////////
// Check if display is available
//
// Returns 0 if yes, 1 if no

int CheckXDisplay::Run()
{

  const char *displayName = getenv("DISPLAY");
  if (strlen(_params.display_name) > 0) {
    displayName = _params.display_name;
  }

  if (_params.debug) {
    cerr << "Testing display: " << displayName << endl;
  }
  
  Display *display = XOpenDisplay(displayName);
  if (display == NULL) {
    if (_params.debug) {
      cerr << "ERROR - cannot open display: " << displayName << endl;
    }
    return 1;
  }

  if (_params.debug) {
    cerr << "SUCCESS - opened display: " << displayName << endl;
  }

  XCloseDisplay(display);

  return 0;

}

