/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 13:58:59
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
//////////////////////////////////////////////////////////
// Args.cc : command line args
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
using namespace std;

// Constructor

Args::Args()
{
  debug = false;
  verbose = false;
  noServer = false;
  noDsp = false;
  host = "localhost";
  port = 11000;
  sendCommands = false;
  queryCommands = false;
  getStatus = false;
  regWithProcmap = false;
  instance = "default";
}

// Destructor

Args::~Args()
{
}

//////////////////////
// parse command line
//
// Returns 0 on success, -1 on failue.

int Args::parse (int argc, char **argv, string &prog_name)

{
  
  int iret = 0;

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(prog_name, cout);
      exit(0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      debug = true;
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      debug = true;
      verbose = true;
      
    } else if (!strcmp(argv[i], "-noServer")) {
      
      noServer = true;
      
    } else if (!strcmp(argv[i], "-noDsp")) {
      
      noDsp = true;
      
    } else if (!strcmp(argv[i], "-send")) {
      
      sendCommands = true;
      
    } else if (!strcmp(argv[i], "-query")) {
      
      queryCommands = true;
      
    } else if (!strcmp(argv[i], "-status")) {
      
      getStatus = true;
      
    } else if (!strcmp(argv[i], "-xml")) {
      
      if (i < argc - 1) {
        xmlPath = argv[++i];
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-host")) {
      
      if (i < argc - 1) {
        host = argv[++i];
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-port")) {
      
      if (i < argc - 1) {
        char *portStr = argv[++i];
        if (sscanf(portStr, "%d", &port) != 1) {
          cerr << "ERROR - bad port: " << portStr << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-procmap")) {
      
      regWithProcmap = true;
      
    } else if (!strcmp(argv[i], "-instance")) {
      
      if (i < argc - 1) {
	instance = argv[++i];
        regWithProcmap = true;
      } else {
	iret = -1;
      }
	
    } // if
    
  } // i

  if (iret) {
    _usage(prog_name, cerr);
    return -1;
  }

  return 0;
    
}

void Args::_usage(const string &prog_name, ostream &out)
{
  
  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help ] produce this list.\n"
      << "       [ -debug ] print debug messages.\n"
      << "       [ -instance ?] instance for procmap\n"
      << "         Forces register with procmap\n"
      << "       [ -noDsp ] do DSP available, do not connect to DSP.\n"
      << "                  Run in messsage test mode only.\n"
      << "       [ -noServer ] do not allow connections.\n"
      << "       [ -host ? ] host to send commands to, if set\n"
      << "       [ -port ? ] port for listening for incoming commands\n"
      << "                   or to send commands to, default 11000\n"
      << "       [ -procmap] register with procmap\n"
      << "       [ -query ] query commands from server.\n"
      << "       [ -send ] send commands in XML file.\n"
      << "       [ -status ] get status from server.\n"
      << "       [ -xml ? ] path for startup XML file\n"
      << "                  or with commands to send\n"
      << "       [ -verbose ] print verbose debug messages.\n"
      << endl;

}

