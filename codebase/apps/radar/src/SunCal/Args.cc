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
// July 2006
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <cstring>
#include <toolsa/udatetime.h>
using namespace std;

// constructor

Args::Args()

{
  TDRP_init_override(&override);
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
  char tmp_str[4096];

  // loop through args
  
  for (int i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      usage(prog_name, cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-debug") || !strcmp(argv[i], "-d")) {

      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose") || !strcmp(argv[i], "-v")) {

      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-extra") || !strcmp(argv[i], "-vv")) {

      sprintf(tmp_str, "debug = DEBUG_EXTRA;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-instance")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "instance = \"%s\";", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "register_with_procmap = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-nsamples")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "n_samples = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-ngates")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "n_gates = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-gate")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "start_gate = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-tsf") ||
               !strcmp(argv[i], "-momf")) {
      
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
        cerr << "ERROR - with -f you must specify files to be read" << endl;
        iret = -1;
      } else {
        if (!strcmp(argv[i], "-tsf")) {
          sprintf(tmp_str, "input_mode = TS_FILELIST_INPUT;");
        } else {
          sprintf(tmp_str, "input_mode = MOMENTS_FILELIST_INPUT;");
        }
        TDRP_add_override(&override, tmp_str);
      }

    } else if (!strcmp(argv[i], "-archive_start")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "archive_start_time = \"%s\";", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "input_mode = MOMENTS_ARCHIVE_INPUT;");
        TDRP_add_override(&override, tmp_str);
	date_time_t start;
	if (sscanf(argv[i+1], "%d %d %d %d %d %d",
		   &start.year, &start.month, &start.day,
		   &start.hour, &start.min, &start.sec) != 6) {
          cerr << "ERROR - SunCal::Args" << endl;
          cerr << "  Bad date/time for -archive_end" << endl;
          cerr << "  Format is \"yyyy mm dd hh mm ss\"" << endl;
          iret = -1;
        }
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-archive_end")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "archive_end_time = \"%s\";", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "input_mode = MOMENTS_ARCHIVE_INPUT;");
        TDRP_add_override(&override, tmp_str);

	date_time_t end;
	if (sscanf(argv[i+1], "%d %d %d %d %d %d",
		   &end.year, &end.month, &end.day,
		   &end.hour, &end.min, &end.sec) != 6) {
          cerr << "ERROR - SunCal::Args" << endl;
          cerr << "  Bad date/time for -archive_end" << endl;
          cerr << "  Format is \"yyyy mm dd hh mm ss\"" << endl;
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
      << "       [ -archive_end \"yyyy mm dd hh mm ss\"] end time\n"
      << "         Sets mode to MOMENTS_ARCHIVE_INPUT\n"
      << "       [ -archive_start \"yyyy mm dd hh mm ss\"] start time\n"
      << "         Sets mode to MOMENTS_ARCHIVE_INPUT\n"
      << "       [ -d, -debug ] print debug messages\n"
      << "       [ -gate ?] specify gate number to be monitored\n"
      << "       [ -instance ?] instance for procmap\n"
      << "         Forces register with procmap\n"
      << "       [ -mode ?] input mode for data. Options are:\n"
      << "          TS_FILELIST_INPUT\n"
      << "          TS_FMQ_INPUT,\n"
      << "          TS_REALTIME_DIR_INPUT\n"
      << "          MOMENTS_REALTIME_FILE_INPUT\n"
      << "          MOMENTS_ARCHIVE_INPUT\n"
      << "          MOMENTS_FILELIST_INPUT\n"
      << "       [ -momf files ] specify input moments file list.\n"
      << "         Read files specified on command line.\n"
      << "         Sets mode to MOMENTS_FILELIST_INPUT.\n"
      << "       [ -ngates ?] specify number of gates for averaging\n"
      << "         defaults to 1\n"
      << "       [ -nsamples ?] specify number of samples\n"
      << "       [ -tsf files ] specify input tsarchive file list.\n"
      << "         Read files specified on command line.\n"
      << "         Sets mode to TS_FILELIST_INPUT.\n"
      << "       [ -v, -verbose ] print verbose debug messages\n"
      << "       [ -vv, -extra ] print extra verbose debug messages\n"
      << endl;
  
  Params::usage(out);
  
}
