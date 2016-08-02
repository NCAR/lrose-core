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
// Jan 1999
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <cstdlib>
#include <string.h>
#include <iostream>
using namespace std;

// Constructor

Args::Args ()
{

}

// Destructor

Args::~Args ()

{

}

// parse the command line
//
// returns 0 on success, -1 on failure  

int Args::parse (int argc, char **argv, const string &prog_name)

{

  _progName = prog_name;
  int iret = 0;

  // initialize

  hostname = "localhost";
  printInterval = 5;
  printContinuous = false;
  printHb = false;
  printMaxintv = false;
  printNreg = false;
  printPlain = false;
  printXml = false;
  printToFile = false;
  outputDir = "/tmp";
  printStatus = false;
  printUptime = false;
  hostTimeOnly = false;
  hostUtimeOnly = false;

  // loop through args
  
  for (int i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(cout);
      exit (0);
      
    } else if (strcmp(argv[i], "-cont") == 0 ||
	       strcmp(argv[i], "-c") == 0) {
      
      printContinuous = true;
      if (i < argc - 1) {
	printInterval = atoi(argv[++i]);
      }
      
    } else if (strcmp(argv[i], "-host") == 0) {
      
      if (i < argc - 1) {
	hostname = argv[++i];
      } else {
	iret = 1;
      }
      
    } else if (strcmp(argv[i], "-debug") == 0) {

      debug = true;

    } else if (strcmp(argv[i], "-hb") == 0) {

      printHb = true;

    } else if (strcmp(argv[i], "-maxint") == 0) {

      printMaxintv = true;

    } else if (strcmp(argv[i], "-nreg") == 0) {

      printNreg = true;

    } else if (strcmp(argv[i], "-plain") == 0) {
      
      printPlain = true;
      printXml = false;

    } else if (strcmp(argv[i], "-xml") == 0) {
      
      printPlain = false;
      printXml = true;

    } else if (strcmp(argv[i], "-odir") == 0) {
      
      if (i < argc - 1) {
	outputDir = argv[++i];
      } else {
	iret = 1;
      }

      printToFile = true;
      
    } else if (strcmp(argv[i], "-norm") == 0) {
      
      printHb = true;
      printUptime = true;
      printStatus = true;

    } else if (strcmp(argv[i], "-host_time") == 0) {
      
      hostTimeOnly = true;

    } else if (strcmp(argv[i], "-host_utime") == 0) {
      
      hostUtimeOnly = true;

    } else if (strcmp(argv[i], "-status") == 0) {
      
      printStatus = true;

    } else if (strcmp(argv[i], "-up") == 0) {

      printUptime = true;

    }
    
  } // i
  
  if (iret) {
    _usage(cerr);
    return -1;
  }

  return 0;
    
}

void Args::_usage(ostream &out)
{

  out << "Usage: " << _progName << " [args as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list\n"
      << "       [ -c, -cont ?] provide a continuous listing\n"
      << "                      updated every n seconds\n"
      << "       [ -debug] print debugging messages\n"
      << "       [ -host ?] hostname - def. is $PROCMAP_HOST\n"
      << "       [ -host_time] print host time only\n"
      << "                     (ignores most args)\n"
      << "       [ -host_utime] print host unix time only\n"
      << "                      (ignores most args)\n"
      << "       [ -hb] print latest heart-beat time.\n"
      << "       [ -maxint] print max interval.\n"
      << "       [ -nreg] print number of registrations.\n"
      << "       [ -norm] normal print: san as -hb -up -status.\n"
      << "       [ -odir ?] output directory\n"
      << "                  output will go to files in this directory\n"
      << "                  instead of stdout\n"
      << "       NOTE: odir is relative to DATA_DIR or RAP_DATA_DIR, if defined,\n"
      << "             unless absolute, i.e. starting with '/' or '.'\n"
      << "       [ -status] prints status - includes info string\n"
      << "       [ -plain] plain listing of all processes\n"
      << "       [ -xml] xml listing os all processes\n"
      << "       [ -up] print uptime.\n"
      << endl;

}





