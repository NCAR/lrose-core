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
// DataMapper.cc
//
// DataMapper object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////
//
// The DataMapper is a server which keeps track of data set
// details.
//
///////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstdio>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <dsserver/DsLocator.hh>
#include <dsserver/DmapAccess.hh>
#include "DataMapper.hh"
#include "DmapServer.hh"
#include "Args.hh"
#include "Params.hh"
using namespace std;

// Constructor

DataMapper::DataMapper(int argc, char **argv)

{

  OK = TRUE;
  _server = NULL;
  
  // set programe name
  
  _progName = "DataMapper";

  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char*)"unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

  // set up the port number

  _port = DsLocator.getDefaultPort( _progName );
  char *override_str = getenv("DMAP_PORT");
  if (override_str != NULL) {
    int overridePort;
    if ((sscanf(override_str, "%d", &overridePort) == 1) &&
	overridePort > 5000) {
      _port = overridePort;
    } else {
      cerr << "WARNING - DmapSocket::DmapSocket" << endl;
      cerr << "  $DMAP_PORT env variable not a valid port number" << endl;
      cerr << "  Port number must be an integer greater than 5000" << endl;
      cerr << "  Using default port " << _port << endl;
    }
  }

  return;

}

// destructor

DataMapper::~DataMapper()

{
  if (_server) {
    delete (_server);
  }
}

//////////////////////////////////////////////////
// Run

int DataMapper::Run()
{

  // if clean selected, clean out store

  if (_args.do_clean) {

    fprintf(stdout,
	    "Are you sure you want to clean the data mapper store? [y/n]: ");
    fflush(stdout);
    char input[1024];
    if (fgets(input, 1024, stdin) == NULL) {
      cerr << "ERROR on input" << endl;
      return -1;
    }

    if (input[0] == 'y' || input[0] == 'Y') {
      cerr << "Cleaning data mapper store" << endl;
      DmapAccess dmapAccess;
      if (dmapAccess.deleteInfo("all", "all", "all")) {
	cerr << "ERROR - contacting DataMapper to clean store" << endl;
      }
    } else {
      cerr << ".......... cleaning aborted" << endl;
    }
    
    return 0;

  } else if (_args.do_delete) {
    
    fprintf(stdout, "You have asked to delete the following entry:\n");
    fprintf(stdout, "  datatype: %s\n", _args.delete_datatype.c_str());
    fprintf(stdout, "  dir:      %s\n", _args.delete_dir.c_str());
    fprintf(stdout, "  hostname: %s\n", _args.delete_hostname.c_str());
    cerr << "  Deleting ...." << endl;
    DmapAccess dmapAccess;
    if (dmapAccess.deleteInfo(_args.delete_hostname,
			      _args.delete_dir,
			      _args.delete_datatype)) {
      cerr << "ERROR - contacting DataMapper to delete item" << endl;
    }
    
    return 0;

  }
  
  // create list of data types

  vector<string> dataTypes;
  for (int ii = 0; ii < _params.data_types_n; ii++) {
    dataTypes.push_back(_params._data_types[ii]);
  }
  for (size_t ii = 0; ii < _args.extraTypes.size(); ii++) {
    dataTypes.push_back(_args.extraTypes[ii]);
  }
  if (_params.debug) {
    cerr << "Data type list: " << endl;
    for (size_t ii = 0; ii < dataTypes.size(); ii++) {
      cerr << "  " << dataTypes[ii] << endl;
    }
    cerr << endl;
  }
  
  // wait for clients

  if (_params.debug) {
    cerr << "  Listening on port: " << _port << endl;
  }
  while (true) {
    
    // create server object
    
    _server = new DmapServer(_progName, _params.instance,
			     _port, -1, -1,
			     (_params.debug >= Params::DEBUG_NORM),
			     (_params.debug >= Params::DEBUG_VERBOSE),
			     _params.no_threads,
			     _params,
			     dataTypes);
    
    if (!_server->isOkay()) {
      cerr << "ERROR - DataMapper" << endl;
      cerr << "  Cannot construct DmapServer" << endl;
      return (-1);
    }
    
    _server->waitForClients(1000);
    
  }
  
  return (0);
  
}



