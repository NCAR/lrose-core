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
////////////////////////////////////////////////////////////////////////////////
//  Driver.cc
//  Driver for TsFmq2Tcp application class
//
//  Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
//  Feb 2009
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/DateTime.hh>

#include "Driver.hh"
#include "TsFmq2Tcp.hh"

using namespace std;

Driver::Driver()
{
  _paramPath = NULL;
  _server = NULL;
  _progName = "TsFmq2Tcp";
  TDRP_init_override(&_tdrpOverride);
}

Driver::~Driver()
{
  TDRP_free_override(&_tdrpOverride);
  if (_server) {
    delete _server;
  }
}

int Driver::init(int argc, char **argv)
{
  
  // handle command line args
    
  if (_processArgs(argc, argv) != 0) {
    return -1;
  }
  
  ucopyright(_progName.c_str());

  // read params

  if (_readParams(argc, argv) != 0) {
    return -1;
  }
  if (_params.debug && _paramPath) {
    cerr << "Loaded Parameter file: " << _paramPath << endl;
  }
  
  // Instantiate the server
  
  if (_params.debug) {
    cerr << "Instantiating the TsFmq2Tcp server." << endl;
  }
  _server = new TsFmq2Tcp(_progName, _params);
   
  if (!_server->isOkay()) {
    cerr << "ERROR - " << _progName << endl;
    cerr << "  Cannot initialize fmq server" << endl;
    cerr << "  " << _server->getErrString() << endl;
    return -1;
  }
   
  return 0;

}

// print usage

void Driver::_usage(ostream &out)
{
  out << "Usage: " << _progName << " [options as below]\n"
      << "       [ --, -h, -help, -man ] produce this list\n"
      << "       [ -cmax ? ] set max number of clients\n"
      << "       [ -d, -debug ] write debug messages\n"
      << "       [ -instance ? ] set instance (default primary)\n"
      << "         Default is port number.\n"
      << "       [ -noThreads ] turn off threads for debugging\n"
      << "       [ -port ? ] set port number.\n"
      << "       [ -v, -verbose ] produce verbose debug messages\n"
      << "       [ -vv, -extra, ] produce extra verbose debug messages\n"
      << endl;
   Params::usage(out);
}

// process the command line args, which can override the parameters

int Driver::_processArgs(int argc, char **argv) 

{

  char paramVal[256];
  int iret = 0;

  // Process each argument

  for(int ii = 1; ii < argc; ii++) {

    if ( !strcmp(argv[ii], "--") ||
	 !strcmp(argv[ii], "-h") ||
	 !strcmp(argv[ii], "-help") ||
	 !strcmp(argv[ii], "-man")) {

      _usage(cout);
      return -1;

    } else if (!strcmp(argv[ii], "-d") ||
               !strcmp(argv[ii], "-debug")) {
      
      sprintf(paramVal, "debug = DEBUG_NORM;");
      TDRP_add_override(&_tdrpOverride, paramVal);

    } else if (!strcmp(argv[ii], "-v") ||
               !strcmp(argv[ii], "-verbose")) {
      
      sprintf(paramVal, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&_tdrpOverride, paramVal);

    } else if (!strcmp(argv[ii], "-vv") ||
               !strcmp(argv[ii], "-extra")) {
      
      sprintf(paramVal, "debug = DEBUG_EXTRA;");
      TDRP_add_override(&_tdrpOverride, paramVal);

    } else if (!strcmp(argv[ii], "-noThreads")) {

      sprintf(paramVal, "no_threads = TRUE;");
      TDRP_add_override(&_tdrpOverride, paramVal);
    
    } else if (!strcmp(argv[ii], "-instance")) {

      if (ii < argc - 1) {
	sprintf(paramVal, "instance = \"%s\";", argv[++ii]);
	TDRP_add_override(&_tdrpOverride, paramVal);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[ii], "-port")) {

      if (ii < argc - 1) {
	sprintf(paramVal, "port = %s;", argv[++ii]);
	TDRP_add_override(&_tdrpOverride, paramVal);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[ii], "-cmax")) {

      if (ii < argc - 1) {
	sprintf(paramVal, "max_clients = %s;", argv[++ii]);
	TDRP_add_override(&_tdrpOverride, paramVal);
      } else {
	iret = -1;
      }

    } // if

  } // i

  if (iret) {
    _usage(cerr);
  }

  return iret;

}

// read in TDRP parameters

int Driver::_readParams(int argc, char **argv)
{

  // Read the parameter file
  
  if (_params.loadFromArgs(argc, argv,
			   _tdrpOverride.list, &_paramPath) != 0) {

    cerr << "ERROR: " << _progName << endl;
    cerr << "  Unable to load parameters." << endl;
    if (_paramPath) {
      cerr << "  Check syntax of parameter file: " << _paramPath << endl;
    }
    return -1;

  }
  
  return 0;

}

int Driver::run()
{
  
  while(true) {
    _server->waitForClients();
  }
  
  return(0);

}
