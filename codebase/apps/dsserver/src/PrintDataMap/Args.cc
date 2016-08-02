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
#include <cstdio>
#include <cstring>
#include <cstdlib>
using namespace std;

// Constructor

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

int Args::parse(int argc, char **argv, const string &prog_name)

{

  // intialize

  int iret = 0;
  debug = false;
  instance = "primary";
  cont = false;
  contInterval = 1;
  lateThreshold = -1;
  xml = false;
  plain = false;
  relt = false;
  hostName = "localhost";
  ip = false;
  dates = false;
  latest = false;
  lreg = false;
  size = false;
  status = false;
  dataDir = "";
  dataType = "";
  toFile = false;
  outputDir = "";

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(prog_name, cout);
      exit(0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      debug = true;
      
    } else if (!strcmp(argv[i], "-norm")) {
      
      latest = true;
      dates = true;
      size = true;
      relt = true;
      lreg = true;
      
    } else if (!strcmp(argv[i], "-all")) {
      
      latest = true;
      dates = true;
      size = true;
      
    } else if (!strcmp(argv[i], "-plain")) {
      
      plain = true;
      xml = false;

    } else if (!strcmp(argv[i], "-xml")) {
      
      xml = true;
      plain = false;

    } else if (!strcmp(argv[i], "-relt")) {
      
      latest = true;
      relt = true;
      
    } else if (!strcmp(argv[i], "-ip")) {
      
      ip = true;
      
    } else if (!strcmp(argv[i], "-dates")) {
      
      dates = true;
      
    } else if (!strcmp(argv[i], "-latest")) {
      
      latest = true;
      
    } else if (!strcmp(argv[i], "-lreg")) {
      
      lreg = true;
      
    } else if (!strcmp(argv[i], "-size")) {
      
      size = true;
      
    } else if (!strcmp(argv[i], "-status")) {
      
      status = true;
      
    } else if (!strcmp(argv[i], "-dir")) {
      
      if (i < argc - 1) {
	dataDir = argv[++i];
      } else {
	iret = -1;
      }
      
    } else if (strcmp(argv[i], "-odir") == 0) {
      
      if (i < argc - 1) {
	outputDir = argv[++i];
      } else {
	iret = -1;
      }

      toFile = true;
      
    } else if (!strcmp(argv[i], "-type")) {
      
      if (i < argc - 1) {
	dataType = argv[++i];
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-host")) {
      
      if (i < argc - 1) {
	hostName = argv[++i];
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-instance")) {
      
      if (i < argc - 1) {
	instance = argv[++i];
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-c") ||
	       !strcmp(argv[i], "-cont")) {
      
      cont = true;
      if (i < argc - 1) {
	if (sscanf(argv[++i], "%d", &contInterval) != 1) {
	  iret = -1;
	}
      }
      
    } else if (!strcmp(argv[i], "-later_than")) {
      
      if (i < argc - 1) {
	if (sscanf(argv[++i], "%d", &lateThreshold) != 1) {
	  iret = -1;
	}
      }
      
    } else {
      
      cerr << "ERROR - Invalid command line arg: " << argv[i] << endl;
      iret = -1;
      
    } // if
    
  } // i

  if (iret != 0) {
    _usage(prog_name, cerr);
    return -1;
  }

  if (debug) {
    cerr << prog_name << ": debug mode" << endl;
    cerr << "  instance: " << instance << endl;
    cerr << "  cont: " << cont << endl;
    cerr << "  contInterval: " << contInterval << endl;
    cerr << "  plain: " << plain << endl;
    cerr << "  relt: " << relt << endl;
    cerr << "  host: " << hostName << endl;
    cerr << "  ip: " << ip << endl;
    cerr << "  dates: " << dates << endl;
    cerr << "  latest: " << latest << endl;
    cerr << "  lreg: " << lreg << endl;
    cerr << "  size: " << size << endl;
    cerr << "  dir: " << dataDir << endl;
    cerr << "  datatype: " << dataType << endl;
    cerr << "  status: " << status << endl;
  }
    
  return 0;

}

void Args::_usage(const string &prog_name, ostream &out)
{
  
  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -all] print latest, dates and size.\n"
      << "       [ -c, -cont [interval]]\n"
      << "         continuous operation at interval secs.\n"
      << "         Default interval is 1 sec.\n"
      << "       [ -dates ] print data set date limits.\n"
      << "       [ -debug ] print debug messages.\n"
      << "       [ -dir ? ] match directory name.\n"
      << "       [ -instance ? ] override instance.\n"
      << "         Default is 'primary'.\n"
      << "       [ -host ?] set host name.\n"
      << "         Default is 'localhost'.\n"
      << "       [ -ip] print IP address.\n"
      << "       [ -latest] print latest data time.\n"
      << "       [ -later_than ?] only print entries later than\n"
      << "         the value specified.\n"
      << "       [ -lreg] print latest registration time.\n"
      << "       [ -norm] normal print\n"
      << "       [ -odir ?] output directory\n"
      << "                  output will go to files in this directory\n"
      << "                  instead of stdout\n"
      << "       NOTE: odir is relative to DATA_DIR or RAP_DATA_DIR, if defined,\n"
      << "             unless absolute, i.e. starting with '/' or '.'\n"
      << "       [ -plain] print unformatted.\n"
      << "       [ -relt] print latest time relative to now.\n"
      << "       [ -size] print data set size.\n"
      << "       [ -status] print status.\n"
      << "       [ -type ? ] match datatype.\n"
      << "       [ -xml] xml print\n"
      << endl;
  
}







