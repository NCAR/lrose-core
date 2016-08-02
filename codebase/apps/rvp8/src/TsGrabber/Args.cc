// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:30:34 
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

  runMode = PRINT_MODE;
  inputMode = TS_API_INPUT_MODE;
  serverPort = 13000;
  invertHvFlag = true;
  dualChannel = false;

  nSamples = 5000;
  startGate = 50;
  nGates = 500;
  fastAlternating = false;

  printMode = PRINT_SUMMARY;
  labelInterval = 30;
  onceOnly = false;
  printHvFlag = false;
  printBinAngles = false;

  regWithProcmap = false;
  instance = "default";

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

    } else if (!strcmp(argv[i], "-once")) {

      onceOnly = true;

    } else if (!strcmp(argv[i], "-full")) {

      runMode = PRINT_MODE;
      printMode = PRINT_FULL;

    } else if (!strcmp(argv[i], "-hv")) {

      printHvFlag = true;

    } else if (!strcmp(argv[i], "-server")) {

      runMode = SERVER_MODE;
      onceOnly = true;

    } else if (!strcmp(argv[i], "-ascope")) {

      runMode = ASCOPE_MODE;

    } else if (!strcmp(argv[i], "-binang")) {

      printBinAngles = true;

    } else if (!strcmp(argv[i], "-cal")) {

      runMode = CAL_MODE;
      inputMode = TS_API_INPUT_MODE;

    } else if (!strcmp(argv[i], "-dual")) {

      dualChannel = true;

    } else if (!strcmp(argv[i], "-alt")) {

      fastAlternating = true;
      dualChannel = true;

    } else if (!strcmp(argv[i], "-port")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%d", &serverPort) != 1) {
          cerr << "ERROR - specify server port as an integer" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-no_invert")) {

      invertHvFlag = false;

    } else if (!strcmp(argv[i], "-nsamples")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%d", &nSamples) != 1) {
          cerr << "ERROR - specify nSamples as an integer" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-labels")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%d", &labelInterval) != 1) {
          cerr << "ERROR - specify label interval as an integer" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-ngates")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%d", &nGates) != 1) {
          cerr << "ERROR - specify ngates as an integer" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-gate")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%d", &startGate) != 1) {
          cerr << "ERROR - specify gate number as in integer" << endl;
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
	
    } else if (!strcmp(argv[i], "-f")) {
      
      if (i < argc - 1) {
	// load up file list vector. Break at next arg which
	// start with -
	for (int j = i + 1; j < argc; j++) {
	  if (argv[j][0] == '-') {
	    break;
	  } else {
	    inputFileList.push_back(argv[j]);
	  }
	}
      } else {
	iret = -1;
      }

      if (inputFileList.size() < 1) {
        cerr << "ERROR - with -f you must specify files to be read" << endl;
        iret = -1;
      } else {
        inputMode = FILE_INPUT_MODE;
      }

    } // if
    
  } // i

  if (iret) {
    usage(prog_name, cerr);
  }

  return (iret);
    
}

void Args::usage(string &prog_name, ostream &out)
{

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -alt ] fast alternating dual-pol mode\n"
      << "       [ -ascope ] prints data for each gate\n"
      << "       [ -binang ] prints binary angle data bits\n"
      << "         Does not apply to alt mode.\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -cal ] perform calibration\n"
      << "       [ -dual ] have dual channels\n"
      << "       [ -f files ] specify input tsarchive file list.\n"
      << "         Read files instead of TsApi.\n"
      << "       [ -full ] prints every pulse\n"
      << "       [ -gate ?] specify gate number to be monitored\n"
      << "       [ -instance ?] instance for registering with procmap\n"
      << "         Server mode only\n"
      << "       [ -hv ] prints HV flag in Ascope mode\n"
      << "       [ -labels ?] number of lines between labels (default 60)\n"
      << "       [ -ngates ?] specify number of gates for averaging\n"
      << "         defaults to 1\n"
      << "       [ -nsamples ?] specify number of samples\n"
      << "       [ -no_invert ] do not invert the sense of the HV flag\n"
      << "         RVP8 HV flag seems to be stored as 0 for H\n"
      << "         so we invert by default\n"
      << "       [ -once ] print once and exit\n"
      << "       [ -port ?] specify port number in server mode\n"
      << "         defaults to 13000\n"
      << "       [ -procmap] register with procmap in server mode\n"
      << "       [ -server ] run in server mode, defaults false\n"
      << "       [ -verbose ] print verbose debug messages\n"
      << endl;
  
}
