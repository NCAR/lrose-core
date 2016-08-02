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
//////////////////////////////////////////////////////////
// Args.cc : command line args
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <didss/DsURL.hh>
#include <cstring>
#include <cstdlib>
using namespace std;

// Constructor

Args::Args()

{
}

// Destructor

Args::~Args()

{
}

////////////////////////////////////////////////////////////////
// parse the command line - returns 0 on success, -1 on failure

int Args::parse(int argc, char **argv, string &prog_name)

{

  // intialize

  int iret = 0;
  _progName = prog_name;
  debug = false;
  useMgr = false;

  getDeniedServices = false;
  getFailureInfo = false;
  getNumClients = false;
  getNumServers = false;
  getServerInfo = false;
  isAlive = false;
  shutDown = false;
  urlStr = "";

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(cout);
      exit(0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      debug = true;
      
    } else if (!strcmp(argv[i], "-mgr")) {
      
      useMgr = true;
      
    } else if (!strcmp(argv[i], "-denied")) {
      
      getDeniedServices = true;
      
    } else if (!strcmp(argv[i], "-failure")) {
      
      getFailureInfo = true;
      
    } else if (!strcmp(argv[i], "-nclients")) {
      
      getNumClients = true;
      
    } else if (!strcmp(argv[i], "-nservers")) {
      
      getNumServers = true;
      
    } else if (!strcmp(argv[i], "-info")) {
      
      getServerInfo = true;
      
    } else if (!strcmp(argv[i], "-alive")) {

      isAlive = true;
      
    } else if (!strcmp(argv[i], "-shutdown")) {
      
      shutDown = true;

    } else if (!strcmp(argv[i], "-url")) {
      
      if (i < argc - 1) {
	urlStr = argv[++i];
      } else {
	iret = -1;
      }
	
    } // if
      
  } // i

  // check URL

  if (urlStr.size() == 0) {
    cerr << endl << "ERROR - you must specify url." << endl;
    iret = -1;
  }

  if (iret == 0) {
    DsURL url(urlStr);
    if (!url.isValid()) {
      cerr << "Invalid URL: " << urlStr << endl;
      cerr << url.getErrString() << endl;
      iret = -1;
    }
  }
  
  if (iret) {
    _usage(cerr);
    return (-1);
  }

  return (0);
    
}

void Args::_usage(ostream &out)
{
  
  out << "\n"
      << "Usage: " << _progName << " [args as below]\n"
      << "\n"
      << "Required:\n"
      << "\n"
      << "  [ -url ? ] specify URL\n"
      << "\n"
      << "Options:\n"
      << "\n"
      << "  [ --, -h, -help, -man ] produce this list.\n"
      << "  [ -debug ]    print debug messages.\n"
      << "  [ -denied ]   getDeniedServices.\n"
      << "  [ -failure ]  getFailureInfo.\n"
      << "  [ -mgr ] use DsServerMgr to start server if needed.\n"
      << "  [ -nclients ]  getNumClients.\n"
      << "  [ -nservers ] getNumServers.\n"
      << "  [ -info ]     getServerInfo.\n"
      << "  [ -alive ]  check isAlive.\n"
      << "  [ -shutdown ] issue shutDown.\n"
      << "\n"
      << endl;

}







