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
// March 2006
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
using namespace std;

// constructor

Args::Args()

{

  debug = false;

  wavelengthCm = 10.68;
  horizBeamWidthDeg = 0.92;
  vertBeamWidthDeg = 0.92;
  antGainDb = 45.6;

  peakPowerDbm = 86.0;
  pulseWidthUs = 1.0;
 
  twoWayWaveguideLoss = 0.0;
  twoWayRadomeLoss = 0.0;
  receiverMismatchLoss = 0.0;

  kSquaredWater = 0.93;

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

    } else if (!strcmp(argv[i], "-wl")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%lg", &wavelengthCm) != 1) {
          cerr << "ERROR - specify wavelength in cm" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-hbw")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%lg", &horizBeamWidthDeg) != 1) {
          cerr << "ERROR - specify horiz beam width in deg" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-vbw")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%lg", &vertBeamWidthDeg) != 1) {
          cerr << "ERROR - specify vert beam width in deg" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-gain")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%lg", &antGainDb) != 1) {
          cerr << "ERROR - specify antenna gain in Db" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-kk")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%lg", &kSquaredWater) != 1) {
          cerr << "ERROR - specify k-squared" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-powerDbm")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%lg", &peakPowerDbm) != 1) {
          cerr << "ERROR - specify peak power in dBm" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-powerW")) {
      
      double peakPowerW = 0.0;
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%lg", &peakPowerW) != 1) {
          cerr << "ERROR - specify peak power in watts" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }

      if (iret == 0) {
        peakPowerDbm = 10.0 * log10(peakPowerW * 1000.0);
      }
      
    } else if (!strcmp(argv[i], "-pw")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%lg", &pulseWidthUs) != 1) {
          cerr << "ERROR - specify pulse width in microsecs" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-wgl")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%lg", &twoWayWaveguideLoss) != 1) {
          cerr << "ERROR - specify waveguide loss in dB" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-rdl")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%lg", &twoWayRadomeLoss) != 1) {
          cerr << "ERROR - specify radome loss in dB" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-rcl")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%lg", &receiverMismatchLoss) != 1) {
          cerr << "ERROR - specify receiver loss in dB" << endl;
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
      << "       [ -wl ? ] wavelength in cm (default 10.68)\n"
      << "       [ -hbw ?] horiz beam width in deg (default 0.92)\n"
      << "       [ -vbw ?] vert beam width in deg (default 0.92)\n"
      << "       [ -gain ?] antenna gain in dB (default 45.6)\n"
      << "       [ -kk ?] k-squared for water (default 0.93)\n"
      << "       [ -powerW ?] peak power in watts\n"
      << "       [ -powerDbm ?] peak power in dBm (default 86.0)\n"
      << "       [ -pw ?] pulse width in microsec (default 1.0)\n"
      << "       [ -wgl ?] two-way waveguide loss in dB (default 0.0)\n"
      << "       [ -rdl ?] two-way radome loss in dB (default 0.0)\n"
      << "       [ -rcl ?] receiver mismatch loss in dB (default 0.0)\n"
      << endl;
  
}
