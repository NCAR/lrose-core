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
// Feb 2001
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace std;

// Constructor

Args::Args()

{
}

// Destructor

Args::~Args()

{
}

////////////////////////////////////////////////////////////////
// parse the command line - returns 0 on success, -1 on failure

int Args::parse(int argc, char **argv, string &prog_name)

{

  // intialize

  int iret = 0;
  _progName = prog_name;

  readTimeMode = TITAN_SERVER_READ_LATEST;
  requestTime = time(NULL);
  startTime = requestTime;
  endTime = requestTime;
  readTimeMargin = 3600;
  trackSet = TITAN_SERVER_ALL_AT_TIME;
  requestComplexNum = 0;

  debug = false;
  readLprops = false;
  readDbzHist = false;
  readRuns = false;
  readProjRuns = false;
  readCompressed = false;
  printAsXml = false;

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(cout);
      exit(0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      debug = true;
      
    } else if (!strcmp(argv[i], "-compress")) {
      
      readCompressed = true;
      
    } else if (!strcmp(argv[i], "-lprops")) {
      
      readLprops = true;
      
    } else if (!strcmp(argv[i], "-hist")) {
      
      readDbzHist = true;
      
    } else if (!strcmp(argv[i], "-runs")) {
      
      readRuns = true;
      
    } else if (!strcmp(argv[i], "-proj_runs")) {
      
      readProjRuns = true;
      
    } else if (!strcmp(argv[i], "-xml")) {
      
      printAsXml = true;
      
    } else if (!strcmp(argv[i], "-margin")) {
      
      if (i < argc - 1) {
	int margin;
	if (sscanf(argv[++i], "%d", &margin) != 1) {
	  iret = -1;
	} else {
	  readTimeMargin = margin;
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-mode")) {
      
      if (i < argc - 1) {
	char *modestr = argv[++i];
	if (!strcmp(modestr, "ltime")) {
	  readTimeMode = TITAN_SERVER_READ_LATEST_TIME;
	} else if (!strcmp(modestr, "latest")) {
	  readTimeMode = TITAN_SERVER_READ_LATEST;
	} else if (!strcmp(modestr, "closest")) {
	  readTimeMode = TITAN_SERVER_READ_CLOSEST;
	} else if (!strcmp(modestr, "first_before")) {
	  readTimeMode = TITAN_SERVER_READ_FIRST_BEFORE;
	} else if (!strcmp(modestr, "next_scan")) {
	  readTimeMode = TITAN_SERVER_READ_NEXT_SCAN;
	} else if (!strcmp(modestr, "prev_scan")) {
	  readTimeMode = TITAN_SERVER_READ_PREV_SCAN;
	} else if (!strcmp(modestr, "interval")) {
	  readTimeMode = TITAN_SERVER_READ_INTERVAL;
	} else {
	  iret = -1;
	}
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-time")) {
      
      if (i < argc - 1) {
	date_time_t request;
	if (sscanf(argv[++i], "%d %d %d %d %d %d",
		   &request.year, &request.month, &request.day,
		   &request.hour, &request.min, &request.sec) != 6) {
	  iret = -1;
	} else {
	  uconvert_to_utime(&request);
	  requestTime = request.unix_time;
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-start")) {
      
      if (i < argc - 1) {
	date_time_t start;
	if (sscanf(argv[++i], "%d %d %d %d %d %d",
		   &start.year, &start.month, &start.day,
		   &start.hour, &start.min, &start.sec) != 6) {
	  iret = -1;
	} else {
	  uconvert_to_utime(&start);
	  startTime = start.unix_time;
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-end")) {
      
      if (i < argc - 1) {
	date_time_t end;
	if (sscanf(argv[++i], "%d %d %d %d %d %d",
		   &end.year, &end.month, &end.day,
		   &end.hour, &end.min, &end.sec) != 6) {
	  iret = -1;
	} else {
	  uconvert_to_utime(&end);
	  endTime = end.unix_time;
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-track_num")) {
      
      if (i < argc - 1) {
	int num;
	if (sscanf(argv[++i], "%d", &num) != 1) {
	  iret = -1;
	} else {
	  requestComplexNum = num;
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-track_set")) {
      
      if (i < argc - 1) {
	char *setstr = argv[++i];
	if (!strcmp(setstr, "all_at_time")) {
	  trackSet = TITAN_SERVER_ALL_AT_TIME;
	} else if (!strcmp(setstr, "all_in_file")) {
	  trackSet = TITAN_SERVER_ALL_IN_FILE;
	} else if (!strcmp(setstr, "single_track")) {
	  trackSet = TITAN_SERVER_SINGLE_TRACK;
	} else if (!strcmp(setstr, "current_entries")) {
	  trackSet = TITAN_SERVER_CURRENT_ENTRIES;
	} else {
	  iret = -1;
	}
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-url")) {
      
      if (i < argc - 1) {
	url = argv[++i];
      } else {
	iret = -1;
      }
	
    } // if
      
  } // i

  // check if path is set

  if (url.size() == 0) {
    cerr << endl << "ERROR - you must specify url." << endl;
    iret = -1;
  }
  
  if (iret) {
    _usage(cerr);
    return (-1);
  }

  return (0);
    
}

void Args::_usage(ostream &out)
{
  
  out << "\n"
      << "Usage: " << _progName << " [args as below]\n"
      << "\n"
      << "  [ --, -h, -help, -man ] produce this list.\n"
      << "\n"
      << "  [ -compress ] compress data from server on read.\n"
      << "                ignored for local reads.\n"
      << "\n"
      << "  [ -debug ] print debug messages.\n"
      << "\n"
      << "  [ -hist ] include dbz histogram.\n"
      << "\n"
      << "  [ -lprops ] include layer props.\n"
      << "\n"
      << "  [ -margin ? ] time_margin (secs): defaults to 3600\n"
      << "     modes: all except latest\n"
      << "\n"
      << "  [ -mode ? ] set mode\n"
      << "     options: ltime, latest, closest, first_before,\n"
      << "              next_scan, prev_scan, interval\n"
      << "\n"
      << "  [ -proj_runs ] include projected area runs.\n"
      << "\n"
      << "  [ -runs ] include storm runs.\n"
      << "\n"
      << "  [ -time ? ] specify search time\n"       
      << "    Format is \"YYYY MM DD HH MM SS\"\n"
      << "\n"
      << "  [ -start ? ] specify start time - interval mode\n"       
      << "    Format is \"YYYY MM DD HH MM SS\"\n"
      << "\n"
      << "  [ -end ? ] specify end time - interval mode\n"       
      << "    Format is \"YYYY MM DD HH MM SS\"\n"
      << "\n"
      << "  [ -track_set ? ] set track set\n"
      << "     options: all_at_time, all_in_file, \n"
      << "              single_track, current_entries\n"
      << "     For single_track, also specify -track_num\n"
      << "\n"
      << "  [ -track_num ? ] set track number\n"
      << "     Used with track_set = single_track\n"
      << "\n"
      << "  [ -url ] specify url\n"       
      << "    Format is \"titanp:://host:port:dir\"\n"
      << "\n"
      << "  [ -xml ] print as XML instead of normal ASCII.\n"
      << "\n"
      << endl;

  out << _progName
      << " produces ASCII or XML output from TITAN server." << endl;
  out << "  Output goes to stdout." << endl << endl;

}







