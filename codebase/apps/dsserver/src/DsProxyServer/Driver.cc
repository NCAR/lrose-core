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
// Driver.cc
//
// Driver for DsProxyServer
//
// April 1999
//
///////////////////////////////////////////////////////////////
//
// The Driver sets up the server object which does the real work.
//
///////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstdio>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include "Driver.hh"
#include "DsProxyServer.hh"
using namespace std;

// Constructor

Driver::Driver(int argc, char **argv)

{

  OK = TRUE;
  _server = NULL;
  
  // set programe name
  
  _progName = "DsProxyServer";

  // get command line args

  if (_args.parse(argc, argv, _progName.c_str())) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = FALSE;
    return;
  }

  // get TDRP params
  
  char *paramsPath = NULL;
  if (_params.loadFromArgs(argc, argv,
                           _args.override.list,
                           &paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

  return;

}

// destructor

Driver::~Driver()

{

  // free up

  if (_server) {
    delete (_server);
  }

}

//////////////////////////////////////////////////
// Run

int Driver::Run()
{

  // wait for clients

  bool forever = true;

  while (forever) {

    // create server object
    
    _server = new DsProxyServer(_progName,
				_params);
    
    if (!_server->isOkay()) {
      cerr << "Error: Could not create server:" << endl;
      cerr << "    " << _server->getErrString() << endl;
      return (-1);
    }
    
    _server->waitForClients(1000);
    
  }
  
  return (0);
  
}
