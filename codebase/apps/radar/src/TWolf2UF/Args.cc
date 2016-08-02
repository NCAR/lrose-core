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
// Oct 2010
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
  verbose = false;

  beamWidthDeg = -9999;
  prf = -9999;
  noisePowerDbm = -9999;

  instrumentName = "twolf";
  outputDir = "./output";

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

    } else if (!strcmp(argv[i], "-name")) {
      
      if (i < argc - 1) {
        instrumentName = argv[++i];
      } else {
 	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-odir")) {
      
      if (i < argc - 1) {
        outputDir = argv[++i];
      } else {
 	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-prf")) {
      
      if (i < argc - 1) {
        double val;
        if (sscanf(argv[++i], "%lg", &val) == 1) {
          prf = val;
        }
      } else {
 	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-noise")) {
      
      if (i < argc - 1) {
        double val;
        if (sscanf(argv[++i], "%lg", &val) == 1) {
          noisePowerDbm = val;
        }
      } else {
 	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-bwidth")) {
      
      if (i < argc - 1) {
        double val;
        if (sscanf(argv[++i], "%lg", &val) == 1) {
          beamWidthDeg = val;
        }
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
        cerr << "ERROR - you must specify files to be read" << endl;
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
      << "       [ -bwidth ?] specify beam width in deg\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -f ? ? ] specify file list.\n"
      << "       [ -name ?] specify instrument name\n"
      << "       [ -noise ?]specify noise power in dBm\n"
      << "       [ -odir ?] specify output directory\n"
      << "       [ -prf ?] specify PRF\n"
      << "       [ -verbose ] print verbose debug messages\n"
      << endl;
  
}
