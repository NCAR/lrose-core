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
// Args.cc
//
// command line args
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2010
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <cstring>
#include <cstdlib>
#include <cstdio>
using namespace std;

// constructor

Args::Args()

{

  debug = false;
  nSamples = 100;
  actualFreqMhz = 25.0;
  samplingFreqMhz = 25.0;
  startPhaseDeg = 30.0;
  peakVal = 100.0;

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

    } else if (!strcmp(argv[i], "-n")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%d", &nSamples) != 1) {
          cerr << "ERROR - specify nSamples" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-freq")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%lg", &actualFreqMhz) != 1) {
          cerr << "ERROR - specify actual frequency in Mhz" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-samp")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%lg", &samplingFreqMhz) != 1) {
          cerr << "ERROR - specify sampling frequency in Mhz" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-start")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%lg", &startPhaseDeg) != 1) {
          cerr << "ERROR - specify start phase in deg" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-peak")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%lg", &peakVal) != 1) {
          cerr << "ERROR - specify peak value" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
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
      << "       [ -debug ] print debug messages\n"
      << "       [ -n ? ] nSamples (default 100)\n"
      << "       [ -freq ?] actual frequency in MHz(default 25.0)\n"
      << "       [ -peak ?] peak value (default 100.0)\n"
      << "       [ -samp ?] sampling frequency in MHz(default 25.0)\n"
      << "       [ -start ?] start phsae in deg(default 30.0)\n"
      << endl;
  
}
