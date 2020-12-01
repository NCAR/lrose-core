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
// IpsAscope.cc
//
// IpsAscope display
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2020
//
///////////////////////////////////////////////////////////////
//
// Support for Independent Pulse Sampling.
//
// IpsAscope is the time series ascope for IPS
//
///////////////////////////////////////////////////////////////

#include "IpsAscope.hh"
#include "AScope.hh"
#include "AScopeReader.hh"
#include "Params.hh"
#include <toolsa/pmu.h>
#include <string>
#include <iostream>
#include <QApplication>

using namespace std;

// Constructor

IpsAscope::IpsAscope(int argc, char **argv) :
        _args("IpsAscope")

{

  OK = true;
  
  // set programe name
  
  _progName = strdup("IpsAscope");

  // get command line args
  
  if (_args.parse(argc, (const char **) argv)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = false;
    return;
  }

  // load TDRP params from command line
  
  char *paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = false;
    return;
  }

  // init process mapper registration
  
  if (_params.register_with_procmap) {
    PMU_auto_init(_progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

}

// destructor

IpsAscope::~IpsAscope()

{

}

//////////////////////////////////////////////////
// Run

int IpsAscope::Run(QApplication &app)
{

  // create the scope
  
  AScope scope(_params);
  scope.setWindowTitle(QString(_params.main_window_title));
  scope.show();
  
  // create the data source reader
  
  AScopeReader reader(_params, scope);
  
  // connect the reader to the scope to receive new time series data
  
  scope.connect(&reader, SIGNAL(newItem(AScope::TimeSeries)),
                &scope, SLOT(newTSItemSlot(AScope::TimeSeries)));
  
  // connect the scope to the reader to return used time series data

  scope.connect(&scope, SIGNAL(returnTSItem(AScope::TimeSeries)),
                &reader, SLOT(returnItemSlot(AScope::TimeSeries)));

  return app.exec();

}

