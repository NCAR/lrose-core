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
// Modified from SpdbQuery by Mike Dixon
// RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 1999
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <didss/DsURL.hh>
#include <toolsa/umisc.h>
#include <cstring>
#include <string>
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

  // initialize

  int iret = 0;
  _progName = prog_name;
  debug = false;
  timeLabel = false;
  table = false;
  remarks = false;
  reverse = false;
  mode = closestMode;
  format = AOAWS;
  requestTime = time(NULL);
  endTime = requestTime;
  startTime = endTime - 86400;
  timeMargin = 86400;
  maxItems = -1;
  bool urlSet = false;
  urlStr = "";
  stationStr = "";

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
      
    } else if (!strcmp(argv[i], "-time_label")) {

      timeLabel = true;
      
    } else if (!strcmp(argv[i], "-table")) {

      table = true;
      
    } else if (!strcmp(argv[i], "-remarks")) {

      remarks = true;
      
    } else if (!strcmp(argv[i], "-reverse")) {
      
      reverse = true;
      
    } else if (!strcmp(argv[i], "-xml")) {
      
      xml = true;
      
    } else if (!strcmp(argv[i], "-mode")) {
      
      if (i < argc - 1) {
	char *modestr = argv[++i];
	if (!strcmp(modestr, "exact")) {
	  mode = exactMode;
	} else if (!strcmp(modestr, "closest")) {
	  mode = closestMode;
	} else if (!strcmp(modestr, "interval")) {
	  mode = intervalMode;
	} else if (!strcmp(modestr, "valid")) {
	  mode = validMode;
	} else if (!strcmp(modestr, "latest")) {
	  mode = latestMode;
	} else if (!strcmp(modestr, "before")) {
	  mode = firstBeforeMode;
	} else if (!strcmp(modestr, "after")) {
	  mode = firstAfterMode;
	} else {
	  iret = -1;
	}
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-format")) {
      
      if (i < argc - 1) {
	char *outstr = argv[++i];
	if (!strcmp(outstr, "aoaws")) {
	  format = AOAWS;
	} else if (!strcmp(outstr, "aoaws_wide")) {
	  format = AOAWS_WIDE;
	} else if (!strcmp(outstr, "wsddm")) {
	  format = WSDDM;
	} else {
	  iret = -1;
	}
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-station")) {

      if (i < argc - 1) {
	stationStr = argv[++i];
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
      
    } else if (!strcmp(argv[i], "-margin")) {
      
      if (i < argc - 1) {
	if (sscanf(argv[++i], "%d", &timeMargin) != 1) {
	  iret = -1;
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-max")) {
      
      if (i < argc - 1) {
	if (sscanf(argv[++i], "%d", &maxItems) != 1) {
	  iret = -1;
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-url")) {
      
      if (i < argc - 1) {
	urlStr = argv[++i];
	urlSet = true;
      } else {
	iret = -1;
      }
	
    } // if
    
  } // i

  if (stationStr == "") {
    cerr << "ERROR - you must specify a METAR station." << endl;
    iret = -1;
  }
  
  if (urlStr == "") {
    cerr << "ERROR - you must specify the URL." << endl;
    iret = -1;
  } else {
    DsURL url(urlStr);
    if (!url.isValid()) {
      cerr << "Invalid URL: " << urlStr << endl;
      cerr << url.getErrString() << endl;
      iret = -1;
    }
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
      << "Usage: " << _progName << " -url -station [options]\n"
      << "Options:\n"
      << "[-mode ?] [-format ?] [-time ?] [-start ?] [-end ?] [-margin ?]\n"
      << "[-debug] [-- -h -help -man]\n"
      << "\n"
      << "Purpose: Get METAR station data from the specified URL\n"
      << "         and print the results to STDOUT.\n"
      << "\n"
      << "Required:\n"
      << "\n"
      << "  [ -url ? ] specify URL\n"
      << "     URL format: spdbp:://host:port:directory (port optional)\n"
      << "                   or\n"
      << "                 directory\n"
      << "\n"
      << "Options (descriptions):\n"
      << "\n"
      << "  [ --, -h, -help, -man ] produce this list.\n"
      << "\n"
      << "  [ -station ?] specify METAR station id\n"
      << "     This must be a valid ICAO identifier\n"
      << "     (e.g., KDEN),\n"
      << "     May be comma-delimited list, no spaces, e.g. 'KDEN,KSFO'\n"
      << "     If 'all', then all stations are printed.\n"
      << "\n"
      << "  [ -debug ] print debug messages.\n"
      << "\n"
      << "  [ -format ? ] output format for station report.\n"
      << "     options: aoaws, aoaws_wide, wsddm. Default: aoaws\n"
      << "\n"
      << "  [ -max ? ] Max number of items to be printed.\n"
      << "     Defaults to unlimited.\n"
      << "\n"
      << "  [ -mode ? ] get mode\n"
      << "     options: exact, closest, interval, valid,\n"
      << "              latest, before, after, times\n"
      << "\n"
      << "  [ -remarks] print remarks in table is available.\n"
      << "\n"
      << "  [ -table] print as table (ignored for xml format).\n"
      << "\n"
      << "  [ -xml] print as xml.\n"
      << "\n"
      << "  [ -time_label] print valid time as label ahead of data.\n" 
      << "\n"
      << "  [ -time ? ] request_time: defaults to now\n"
      << "     modes: exact, closest, firstBefore, firstAfter, valid\n"
      << "     format: \"YYYY MM DD HH MM SS\"\n"
      << "\n"
      << "  [ -start ? ] start_time: defaults to (now - 1 day)\n"
      << "     mode: interval only\n"
      << "     format: \"YYYY MM DD HH MM SS\"\n"
      << "\n"
      << "  [ -end ? ] end_time: defaults to now\n"
      << "     mode: interval only\n"
      << "     format: \"YYYY MM DD HH MM SS\"\n"
      << "\n"
      << "  [ -margin ? ] time_margin (secs): defaults to 1 day\n"
      << "     modes: closest, latest, firstBefore, firstAfter\n"
      << "\n"
      << "  [ -reverse ] print in time-revese order.\n"
      << endl;

}







