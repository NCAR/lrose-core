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
// May 2014
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <cstring>
#include <cstdlib>
#include <cstdio>
using namespace std;

const double Args::lightSpeed = 299792458.0;

// constructor

Args::Args()

{

  debug = false;
  frequencyHz = 5.6e9;
  prt1 = 1.0 / 800.0;
  prt2 = 1.0 / 600.0;

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

    } else if (!strcmp(argv[i], "-wlen")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        double wavelengthCm;
        if (sscanf(str, "%lg", &wavelengthCm) != 1) {
          cerr << "ERROR - specify wavelength in cm" << endl;
          iret = -1;
        }
        frequencyHz = lightSpeed / (wavelengthCm / 100.0);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-freq")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%lg", &frequencyHz) != 1) {
          cerr << "ERROR - specify frequency in Hz" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-prt1")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%lg", &prt1) != 1) {
          cerr << "ERROR - specify prt1 in 1/s" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-prt2")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        if (sscanf(str, "%lg", &prt2) != 1) {
          cerr << "ERROR - specify prt2 in 1/s" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-prf1")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        double prf1;
        if (sscanf(str, "%lg", &prf1) != 1) {
          cerr << "ERROR - specify prt1 in 1/s" << endl;
          iret = -1;
        }
        prt1 = 1.0 / prf1;
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-prf2")) {
      
      if (i < argc - 1) {
        char *str = argv[++i];
        double prf2;
        if (sscanf(str, "%lg", &prf2) != 1) {
          cerr << "ERROR - specify prt1 in 1/s" << endl;
          iret = -1;
        }
        prt2 = 1.0 / prf2;
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
      << "       [ -freq ? ] specify frequency in hz\n"
      << "       [ -wlen ? ] or wavelength in cm)\n"
      << "       [ -prf1 ?] specify prf1 in pulses/sec\n"
      << "       [ -prt1 ?] or prt1 in sec\n"
      << "       [ -prf2 ?] specify prf2 in pulses/sec\n"
      << "       [ -prt2 ?] or prt2 in sec\n"
      << endl;
  
}
