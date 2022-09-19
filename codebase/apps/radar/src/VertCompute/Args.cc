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
// Aug 2006
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <cstring>
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
      
    } else if (!strcmp(argv[i], "-d") ||
               !strcmp(argv[i], "-debug")) {

      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-v") ||
               !strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-vv") ||
               !strcmp(argv[i], "-extra")) {
      
      sprintf(tmp_str, "debug = DEBUG_EXTRA;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-instance")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "instance = %s;", argv[++i]);
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

    } else if (!strcmp(argv[i], "-nrevs_global")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "nrevs_for_global_stats = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-start")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "start_time = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-end")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "end_time = \"%s\";", argv[i+1]);
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

      if (inputFileList.size() < 1) {
        cerr << "ERROR - with -f you must specify files to be read" << endl;
        iret = -1;
      } else {
        if (inputFileList[0].find("iwrf_ts") != string::npos) {
          sprintf(tmp_str, "input_mode = TS_FILE_INPUT;");
          TDRP_add_override(&override, tmp_str);
        } else if (inputFileList[0].find(".nc") != string::npos) {
          sprintf(tmp_str, "input_mode = RADX_MOMENTS_INPUT;");
          TDRP_add_override(&override, tmp_str);
        }
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
      << "       [ -d, -debug ] print debug messages\n"
      << "       [ -end \"yyyy mm dd hh mm ss\"] set end time\n"
      << "          applies to RADX_MOMENTS_INPUT and TS_FILE_INPUT only\n"
      << "       [ -f files ] specify input time series file list.\n"
      << "         Read files instead of TsApi.\n"
      << "       [ -nrevs_global ?] number of revs for global stats\n"
      << "         Global stats will be printed after this number of revolutions\n"
      << "       [ -instance ?] instance for procmap\n"
      << "         Forces register with procmap\n"
      << "       [ -nsamples ?] specify number of samples\n"
      << "       [ -start \"yyyy mm dd hh mm ss\"] set start time\n"
      << "          applies to RADX_MOMENTS_INPUT and TS_FILE_INPUT only\n"
      << "       [ -v, -verbose ] print verbose debug messages\n"
      << "       [ -vv, -extra ] print extra verbose debug messages\n"
      << endl;
  
}
