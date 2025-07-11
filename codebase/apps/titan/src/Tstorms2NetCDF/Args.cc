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
// Command line args
//
// Mike Dixon, EOL, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2025
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <cstring>
#include <toolsa/umisc.h>
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
  char tmp_str[256];

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(prog_name, cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-d") ||
               !strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-v") ||
               !strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-to_legacy")) {
      
      sprintf(tmp_str, "convert_to_legacy = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-truncate")) {
      
      sprintf(tmp_str, "test_truncation = TRUE;");
      TDRP_add_override(&override, tmp_str);

      if (i < argc - 1) {
	snprintf(tmp_str, 256, "truncation_scan_number = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      } else {
	sprintf(tmp_str, "truncation_scan_number = 0;");
	TDRP_add_override(&override, tmp_str);
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
	
      sprintf(tmp_str, "input_mode = ARCHIVE;");
      TDRP_add_override(&override, tmp_str);

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
	
      sprintf(tmp_str, "input_mode = ARCHIVE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-valid")) {
      
      if (i < argc - 1) {
	date_time_t valid;
	if (sscanf(argv[++i], "%d %d %d %d %d %d",
		   &valid.year, &valid.month, &valid.day,
		   &valid.hour, &valid.min, &valid.sec) != 6) {
	  iret = -1;
	} else {
	  uconvert_to_utime(&valid);
	  xmlValidTime = valid.unix_time;
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
      
      sprintf(tmp_str, "input_mode = FILELIST;");
      TDRP_add_override(&override, tmp_str);

    } // if
    
  } // i

  if (iret) {
    _usage(prog_name, cerr);
  }

  return (iret);
    
}

void Args::_usage(string &prog_name, ostream &out)
{

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -d, -debug ] print debug messages\n"
      << "       [ -end \"yyyy mm dd hh mm ss\"] end time\n"
      << "         sets input_mode to ARCHIVE\n"
      << "       [ -f ? ?] input track file list (.th5 files)\n"
      << "         sets input_mode to FILELIST\n"
      << "       [ -start \"yyyy mm dd hh mm ss\"] start time\n"
      << "         sets input_mode to ARCHIVE\n"
      << "       [ -to_legacy ] reverse sense\n"
      << "          convert from netcdf, write legacy files\n"
      << "       [ -v, -verbose ] print verbose debug messages\n"

      << endl;
  
  Params::usage(out);

}







