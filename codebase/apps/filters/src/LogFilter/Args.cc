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
// June 2000
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace std;

// parse

int Args::parse(int argc, char **argv, string &prog_name)

{

  // Set to defaults

  int iret = 0;
  tInterval = 3600;
  debug = false;
  hourMode = false;
  singleFileMode = false;
  keepRunningOnError = false;
  timeStampEachLine = true;

  // loop through args
  
  for (int i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "-h")) {
      
      _usage(prog_name, cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      debug = true;
      
    } else if (!strcmp(argv[i],"-d")) {
      
      if (i < argc - 1) {
	logDir = argv[++i];
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i],"-p")) {
      
      if (i < argc - 1) {
	procName = argv[++i];
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-noLineStamp")) {
      
      timeStampEachLine = false;
      
    } else if (!strcmp(argv[i],"-hourly")) {
      
      hourMode = true;
      singleFileMode = false;
      
    } else if (!strcmp(argv[i],"-single")) {
      
      singleFileMode = true;
      hourMode = false;
      
    } else if (!strcmp(argv[i],"-keepRunningOnError")) {

      keepRunningOnError = true;

    } else if (!strcmp(argv[i],"-i")) {
      
      if (i < argc - 1) {
	instance = argv[++i];
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i],"-t")) {
      
      if (i < argc - 1) {
	if ((sscanf(argv[++i], "%d", &tInterval)) != 1) {
	  iret = -1;
	}
      } else {
	iret = -1;
      }

    } // if
    
  } // i

  if (logDir.size() == 0) {
    cerr << "ERROR - " << prog_name << ":Args::parse()" << endl;
    cerr << "  You must specify log dir with -d" << endl;
    iret = -1;
  }

  if (procName.size() == 0) {
    cerr << "ERROR - " << prog_name << ":Args::parse()" << endl;
    cerr << "  You must specify process name with -p" << endl;
    iret = -1;
  }

  if (tInterval == 0 || tInterval < -1) {
    cerr << "ERROR - " << prog_name << ":Args::parse()" << endl;
    cerr << " Time interval must be either positive, or -1 to disable."
	 << endl;
    iret = -1;
  }

  if (iret) {
    _usage(prog_name, cerr);
  }

  if (debug) {
    cerr << "LogFilter:" << endl;
    cerr << "  logDir: " << logDir << endl;
    cerr << "  procName: " << procName << endl;
    if (instance.size() > 0) {
      cerr << "  instance: " << instance << endl;
    }
    cerr << "  tInterval: " << tInterval << endl;
    if (hourMode) 
      cerr << " Hour Mode is set" << endl;
    else
      cerr << " Day Mode is set" << endl;

    if (keepRunningOnError){
      cerr << " Will keep running on error." << endl;
    } else {
      cerr << " Will exit on error." << endl;
    }

  }

  return (iret);
    
}

void Args::_usage(string &prog_name, ostream &out)
{

  out << "Usage: " << prog_name
      << " -d logdir -p procname [options as below]\n"
      << "options:\n"
      << "       [ -h ] produce this list.\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -p ] set process name \n"
      << "       [ -i ] set process instance\n"
      << "       [ -hourly ] set the logs to be placed in hourly files\n"
      << "       [ -single ] create a single log file, not one per day\n"
      << "       [ -t ] interval between time stamp prints (sec)\n"
      << "              defaults to 3600, -1 disables printing\n"
      << "       [ -keepRunningOnError ] keep running if an error occurs\n"
      << "       [ -noLineStamp ] do not time-stamp every line\n"
      << endl;

  out << "LogFilter reads ascii log messages from stdin, and copies\n"
      << "  them to a log file in the specified log directory.\n"
      << "  The file name is logdir/yyyymmdd/procname.instance.log, or\n"
      << "  logdir/yyyymmdd/procname.log if the instance is not specfied.\n"
      << "  yyyymmdd is is the current date. The default behavior is for a\n"
      << "  new file to be created each day.\n"
      << "  If the -hourly option is set the hour is appended to the end\n"
      << "  of the filename, and a new file is created every hour.\n"
      << "  Time stamp information is also inserted into the log stream,\n"
      << "  at the interval specified with -t. Set to -1 to disable.\n"
      << endl;
  
}







