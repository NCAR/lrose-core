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
// March 1998
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <toolsa/udatetime.h>
#include <didss/DsInputPath.hh>
#include <rapmath/math_macros.h>
using namespace std;

// constructor

Args::Args()

{
  TDRP_init_override(&override);
  startTime = 0;
  endTime = 0;
  stampTime = 0;
}

// destructor

Args::~Args()

{
  TDRP_free_override(&override);
}

// parse

int Args::parse(int argc, char **argv, string &prog_name)

{

  int iret = 0;
  char tmp_str[256];

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(prog_name, cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-titan")) {
      
      sprintf(tmp_str, "track_data_type = TITAN_TRACKS;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-model")) {
      
      sprintf(tmp_str, "track_data_type = MODEL_TRACKS;");
      TDRP_add_override(&override, tmp_str);
      
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
	  sprintf(tmp_str, "track_data_type = TITAN_TRACKS;");
	  TDRP_add_override(&override, tmp_str);
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
	  sprintf(tmp_str, "track_data_type = TITAN_TRACKS;");
	  TDRP_add_override(&override, tmp_str);
	}
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-stamp")) {
      
      if (i < argc - 1) {
	date_time_t stamp;
	if (sscanf(argv[++i], "%d %d %d %d %d %d",
		   &stamp.year, &stamp.month, &stamp.day,
		   &stamp.hour, &stamp.min, &stamp.sec) != 6) {
	  iret = -1;
	} else {
	  uconvert_to_utime(&stamp);
	  stampTime = stamp.unix_time;
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
      
    } // if
    
  } // i

  if (iret) {
    _usage(prog_name, cerr);
  }

  // if -f used and no start and end time specified, compute start
  // and end time

  if (inputFileList.size() > 0 && startTime == 0 && endTime == 0) {

    time_t ftime = DsInputPath::getDataTime(inputFileList[0]);
    startTime = ftime;
    endTime = ftime + SECS_IN_DAY - 1;

    for (size_t ii = 1; ii < inputFileList.size(); ii++) {
      ftime = DsInputPath::getDataTime(inputFileList[ii]);
      startTime = MIN(startTime, ftime);
      endTime = MAX(endTime, (ftime + SECS_IN_DAY - 1));
    }

  }

  return (iret);
    
}

void Args::_usage(string &prog_name, ostream &out)
{

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -end \"yyyy mm dd hh mm ss\"] end time\n"
      << "          TITAN_TRACKS only\n"
      << "       [ -f ?] input file list\n"
      << "          Required for MODEL_TRACKS only\n"
      << "          For TITAN_TRACKS overrides start and end time\n"
      << "       [ -model ] use model track data files (overrides -titan)\n"
      << "       [ -start \"yyyy mm dd hh mm ss\"] start time\n"
      << "          TITAN_TRACKS only\n"
      << "       [ -stamp \"yyyy mm dd hh mm ss\"] file stamp time\n"
      << "          Optional - specify time to stamp results\n"
      << "          If not specified, uses output_time_stamp parameter\n"
      << "       [ -titan ] use TITAN track data files (default)\n"
      << "       [ -verbose ] print verbose debug messages\n"
      << endl;

  out << "Note: you must specify start and end dates." << endl << endl;
  
  Params::usage(out);

}






