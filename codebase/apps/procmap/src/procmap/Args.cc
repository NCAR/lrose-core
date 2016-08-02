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
// April 2000
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <toolsa/pmu.h>

// parse

int Args::parse(int argc, char **argv, string &prog_name)

{

  int iret = 0;
  debug = false;
  daemon = false;
  maxThreads = 64;

  // loop through args
  
  for (int i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(prog_name, cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      debug = true;
      
    } else if (!strcmp(argv[i], "-daemon")) {
      
      daemon = true;
      
    } else if (!strcmp(argv[i],"-nmax")) {
      
      if (i < argc - 1) {
	if ((sscanf(argv[++i], "%d", &maxThreads)) != 1) {
	  iret = -1;
	}
      } else {
	iret = -1;
      }

    } // if
    
  } // i

  if (iret) {
    _usage(prog_name, cerr);
  }

  return (iret);
    
}

void Args::_usage(string &prog_name, ostream &out)
{

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -daemon ] run in background as daemon\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -nmax ?] set max number of threads (default 64)\n"
      << endl << endl;
  
  out << "NOTES:\n"
      << endl
      << "  procmap maintains a record of the status of each\n"
      << "  process in the system. It listens for connections\n"
      << "  from clients and processes.\n"
      << endl
      << "  procmap usually listens on port number 5433.\n"
      << "  However, if the environment variable DS_BASE_PORT is set,\n"
      << "  procmap will listen on that port plus 3.\n"
      << "  In your current environment, the port is "
      << PMU_get_default_port()<< ".\n"
      << endl
      << "  When a connection is made, procmap spawns a thread\n"
      << "  to handle the communications. For a registration\n"
      << "  request, the child acknowledges the request, and stores the\n"
      << "  registration information. For an info request, the thread\n"
      << "  compiles the required information and sends it to the client.\n"
      << "  Then the thread exits.\n"
      << endl
      << "  The parent, meanwhile, listens for further requests.\n"
      << endl;

}







