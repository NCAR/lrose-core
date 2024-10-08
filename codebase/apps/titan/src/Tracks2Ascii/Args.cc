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
// July 1999
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

  // intialize

  startTime = 0;
  endTime = 0;

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      usage(prog_name, cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-start")) {
      
      if (i < argc - 1) {
	date_time_t start;
	MEM_zero(start);
	if (sscanf(argv[++i], "%d %d %d %d %d %d",
		   &start.year, &start.month, &start.day,
		   &start.hour, &start.min, &start.sec) != 6) {
	  iret = -1;
	} else {
	  uconvert_to_utime(&start);
	  startTime = start.unix_time;
	  TDRP_add_override(&override, tmp_str);
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-end")) {
      
      if (i < argc - 1) {
	date_time_t end;
	MEM_zero(end);
	if (sscanf(argv[++i], "%d %d %d %d %d %d",
		   &end.year, &end.month, &end.day,
		   &end.hour, &end.min, &end.sec) != 6) {
	  iret = -1;
	} else {
	  uconvert_to_utime(&end);
	  endTime = end.unix_time;
	  TDRP_add_override(&override, tmp_str);
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-complex")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "specify_complex_track_num = true;");
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "complex_track_num = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-simple")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "specify_simple_track_num = true;");
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "simple_track_num = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-complete")) {
      
      sprintf(tmp_str, "target_entity = COMPLETE_TRACK;");
      TDRP_add_override(&override, tmp_str);
	
    } else if (!strcmp(argv[i], "-entry")) {
      
      sprintf(tmp_str, "target_entity = TRACK_ENTRY;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-levels")) {
      
      sprintf(tmp_str, "print_level_properties = TRUE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-initial")) {
      
      sprintf(tmp_str, "target_entity = INITIAL_PROPS;");
      TDRP_add_override(&override, tmp_str);
	
    } else if (!strcmp(argv[i], "-dir")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "input_dir = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
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
    usage(prog_name, cerr);
  }

  return (iret);
    
}

void Args::usage(string &prog_name, ostream &out)
{

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -complex ?] specify complex track number\n"
      << "       [ -complete] set target_entity to COMPLETE_TRACK\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -dir ?] specify input directory\n"
      << "       [ -end \"yyyy mm dd hh mm ss\"] end time\n"
      << "       [ -entry] set target_entity to TRACK_ENTRY\n"
      << "       [ -f ? ?] input file list\n"
      << "       [ -initial] set target_entity to INITIAL_PROPS\n"
      << "       [ -levels] add level properties to output\n"
      << "       [ -simple ?] specify simpletrack number\n"
      << "       [ -start \"yyyy mm dd hh mm ss\"] start time\n"
      << "       [ -verbose ] print verbose debug messages\n"
      << endl;

  out << "Note: you must either specify a file list using -f or" << endl
      << "      start and end dates." << endl << endl;
  
  Params::usage(out);

}

