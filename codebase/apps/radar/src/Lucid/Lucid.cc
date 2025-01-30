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
// Lucid.cc
//
// Lucid display
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2023
//
///////////////////////////////////////////////////////////////
//
// Lucid is the Qt replacement for CIDD
//
///////////////////////////////////////////////////////////////

#include "Lucid.hh"
#include "GuiManager.hh"
#include "LegacyParams.hh"
#include <toolsa/mem.h>
#include <toolsa/Path.hh>
#include <qtplot/ColorMap.hh>

#include <string>
#include <iostream>
#include <QApplication>
#include <QErrorMessage>

#define THIS_IS_MAIN 1 /* This is the main module */
#include "cidd.h"

using namespace std;

// Constructor

Lucid::Lucid(int argc, char **argv) :
        _args("Lucid")

{

  OK = true;
  _guiManager = NULL;

  // set programe name

  _progName = strdup("Lucid");

  // initialize legacy CIDD structs
  
  init_globals();

  // check for legacy params file
  // if found create a temporary tdrp file based on the legacy file

  string legacyParamsPath;
  char tdrpParamsPath[5000];
  bool usingLegacyParams = false;
  if (_args.getLegacyParamsPath(argc, (const char **) argv, legacyParamsPath) == 0) {
    // gd.db_name = strdup(legacyParamsPath.c_str());
    Path lpPath(legacyParamsPath);
    snprintf(tdrpParamsPath, 4999,
             "/tmp/Lucid.%s.%d.tdrp", lpPath.getFile().c_str(), getpid());
    LegacyParams lParams;
    lParams.translateToTdrp(legacyParamsPath, tdrpParamsPath);
    usingLegacyParams = true;
  }

  // get command line args
  
  if (_args.parse(argc, (const char **) argv)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = false;
    return;
  }
  
  // load TDRP params from command line

  char *paramsPath = (char *) "unknown";
  if (usingLegacyParams) {
    if (_params.loadApplyArgs(tdrpParamsPath,
                              argc, argv,
                              _args.override.list)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with TDRP parameters." << endl;
      OK = false;
      return;
    }
    _paramsPathRequested = legacyParamsPath;
    _paramsPathUsed = tdrpParamsPath;
  } else {
    if (_params.loadFromArgs(argc, argv,
                             _args.override.list,
                             &paramsPath)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with TDRP parameters." << endl;
      OK = false;
      return;
    }
    _paramsPathRequested = paramsPath;
    _paramsPathUsed = paramsPath;
  }

  if (_params.debug) {
    cerr << "Using params path: " << _paramsPathUsed.getPath() << endl;
  }
  
  if (_params.fields_n < 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  0 fields specified" << endl;
    cerr << "  At least 1 field is required" << endl;
    OK = false;
    return;
  }
    
    // initialize globals, get/set defaults, establish data sources etc.

  if (init_data_space(this)) {
    OK = false;
    return;
  }

  // init process mapper registration
  
  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

}

// destructor

Lucid::~Lucid()

{

  if (_guiManager) {
    delete _guiManager;
  }

}

//////////////////////////////////////////////////
// Run

int Lucid::RunApp(QApplication &app)
{

  gd.finished_init = 1;

  // create cartesian display
  
  _guiManager = new GuiManager;
  return _guiManager->run(app);
  
}

