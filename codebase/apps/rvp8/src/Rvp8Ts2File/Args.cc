// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:29:54 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
//////////////////////////////////////////////////////////
// Args.cc
//
// command line args
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <string.h>
using namespace std;

// constructor

Args::Args()

{
  debug = false;
  verbose = false;
  oneFile = false;
  maxDegrees = 90;
  maxPulses = 50000;
  outDir = ".";
}

// destructor

Args::~Args()

{

}

// parse

int Args::parse(int argc, char **argv, string &prog_name)

{

  int iret = 0;

  // loop through args
  
  for (int i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      usage(prog_name, cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-debug")) {

      debug = true;

    } else if (!strcmp(argv[i], "-verbose")) {

      debug = true;
      verbose = true;

    } else if (!strcmp(argv[i], "-oneFile")) {
      
      oneFile = true;
      
    } else if (!strcmp(argv[i], "-maxDegrees")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%d", &maxDegrees) != 1) {
          cerr << "ERROR - specify maxDegrees as an integer" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-maxPulses")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%d", &maxPulses) != 1) {
          cerr << "ERROR - specify maxPulses as an integer" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-outDir")) {
      
      if (i < argc - 1) {
	outDir = argv[++i];
      } else {
	iret = -1;
      }
	
    } // if
    
  } // i
  
  if (iret) {
    usage(prog_name, cerr);
  }

  if (debug) {
    cerr << "Options set as follows: " << endl;
    cerr << "  oneFile: " << (oneFile? "true" : "false") << endl;
    cerr << "  maxDegrees: " << maxDegrees << endl;
    cerr << "  maxPulses: " << maxPulses << endl;
    cerr << "  outDir: " << outDir << endl;
  }

  return (iret);
    
}

void Args::usage(string &prog_name, ostream &out)
{

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -single] write a single file then quit\n"
      << "       [ -maxDegrees ?] specify max # of degrees per file\n"
      << "         Default is 90\n"
      << "       [ -maxPulses ?] specify max # of pulses per file\n"
      << "         Default is 50000\n"
      << "       [ -outDir ?] specify output directory for file\n"
      << "          default '.'\n"
      << "       [ -oneFile] write one file then quit\n"
      << "         In this mode, maxPulses are written to the file\n"
      << "       [ -verbose ] print verbose debug messages\n"
      << endl;
  
}
