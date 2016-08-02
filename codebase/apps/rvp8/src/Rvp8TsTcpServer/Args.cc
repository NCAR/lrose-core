// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:31:59 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
//////////////////////////////////////////////////////////
// Args.cc : command line args
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>
using namespace std;

// Constructor

Args::Args()
{
  debug = false;
  verbose = false;
  port = 12000;
  regWithProcmap = false;
  instance = "default";
  checkMove = false;
  checkStowed = false;
  invertHvFlag = true;
  outputPacking = SIGMET_FL16;
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
      
    } else if (!strcmp(argv[i], "-no_invert")) {
      
      invertHvFlag = false;

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
	
    } else if (!strcmp(argv[i], "-check_move")) {
      
      checkMove = true;
      
    } else if (!strcmp(argv[i], "-check_stowed")) {
      
      checkStowed = true;
      
    } else if (!strcmp(argv[i], "-fl32")) {
      
      outputPacking = IWRF_FL32;
      
    } else if (!strcmp(argv[i], "-si16")) {
      
      outputPacking = IWRF_SI16;
      
    } else if (!strcmp(argv[i], "-sigmet16")) {
      
      outputPacking = SIGMET_FL16;
      
    } // if
    
  } // i

  if (debug) {
    _print(prog_name, cerr);
  }

  if (iret) {
    _usage(prog_name, cerr);
    return -1;
  }

  return 0;
    
}

void Args::_print(const string &prog_name, ostream &out)
{
  
  out << "Args settings for: " << prog_name << endl;

  cerr << "debug: " << (debug? "true" : "false") << endl;
  cerr << "verbose: " << (verbose? "true" : "false") << endl;
  cerr << "port: " << port << endl;
  cerr << "regWithProcmap: " << (regWithProcmap? "true" : "false") << endl;
  cerr << "checkMove: " << (checkMove? "true" : "false") << endl;
  cerr << "checkStowed: " << (checkStowed? "true" : "false") << endl;
  cerr << "invertHvFlag: " << (invertHvFlag? "true" : "false") << endl;
  if (outputPacking == SIGMET_FL16) {
    cerr << "outputPacking: SIGMET_FL16" << endl;
  } else if (outputPacking == IWRF_FL32) {
    cerr << "outputPacking: IWRF_FL32" << endl;
  } else if (outputPacking == IWRF_SI16) {
    cerr << "outputPacking: IWRF_SI16" << endl;
  }

}

void Args::_usage(const string &prog_name, ostream &out)
{
  
  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help ] produce this list.\n"
      << "       [ -debug ] print debug messages.\n"
      << "       [ -fl32] convert output to IWRF float 32\n"
      << "          default is to use SIGMET FL16 packed floats\n"
      << "       [ -instance ?] instance for procmap\n"
      << "         Forces register with procmap\n"
      << "       [ -check_move ] check for movement\n"
      << "         Does not send data if antenna is stationary\n"
      << "       [ -check_stowed ] check for stowed antenna\n"
      << "         Does not send data if antenna is stowed\n"
      << "         Stowed means 90 deg elevation and not moving\n"
      << "       [ -no_invert ] do not invert the sense of the HV flag\n"
      << "         RVP8 HV flag seems to be stored as 0 for H\n"
      << "         so we invert by default\n"
      << "       [ -port ? ] port for listening for incoming commands\n"
      << "                   or to send commands to, default 12000\n"
      << "       [ -procmap] register with procmap\n"
      << "       [ -si16] convert output to IWRF int 16\n"
      << "          default is to use SIGMET FL16 packed floats\n"
      << "       [ -sigmet16] convert output to SIGMET float 16\n"
      << "          this is the default\n"
      << "       [ -verbose ] print verbose debug messages.\n"
      << endl;

}

