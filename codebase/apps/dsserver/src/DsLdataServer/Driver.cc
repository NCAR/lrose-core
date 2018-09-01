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
// Driver object for DsLdataServer
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2002
//
///////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstdio>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <dsserver/DsLocator.hh>
#include "Driver.hh"
#include "DsLdataServer.hh"
#include "Args.hh"
#include "Params.hh"
using namespace std;

// Constructor

Driver::Driver(int argc, char **argv)

{

  OK = TRUE;
  _server = NULL;
  
  // set programe name and default params path
  
  _progName = "DsLdataServer";
  _paramsPath = (char *) "unknown";

  // parse command line args
  
  if (_args.parse(argc, argv, _progName.c_str())) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &_paramsPath)) {
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
  if (_server) {
    delete _server;
  }
}

//////////////////////////////////////////////////
// Run

int Driver::Run()
{

  if (_params.debug) {
    cerr << "  Listening on port: " << _params.port << endl;
  }

  // wait for clients

  bool forever = true;

  while (forever) {
    
    // create server object
    
    _server = new DsLdataServer(_progName, _params.instance,
				_params.port, -1, -1,
				(_params.debug >= Params::DEBUG_NORM),
				(_params.debug >= Params::DEBUG_VERBOSE),
				_params.no_threads,
                                _params.run_secure,
                                _params.run_read_only,
				_params);
    
    if (!_server->isOkay()) {
      cerr << "ERROR - Driver" << endl;
      cerr << "  Cannot construct DmapServer" << endl;
      return (-1);
    }
    
    _server->waitForClients(1000);
    
  }
  
  return (0);
  
}



