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
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2011
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <cstring>
#include <toolsa/umisc.h>
#include <dsdata/DsUrlTrigger.hh>
using std::string;

// constructor

Args::Args()

{
  TDRP_init_override(&override);
  tdrpDebug = false;
  startTimeSet = false;
  endTimeSet = false;
}

// destructor

Args::~Args()

{
  TDRP_free_override(&override);
}

// parse

int Args::parse(int argc, char **argv, string &prog_name)

{

  _progName = prog_name;
  char tmp_str[BUFSIZ];
  int iret = 0;

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-d") ||
               !strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug_norm = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-v") ||
               !strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug_verbose = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-debug_trigger")) {
      
      sprintf(tmp_str, "debug_triggering = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-debug_realtime")) {
      
      sprintf(tmp_str, "debug_show_realtime = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-debug_methods")) {
      
      sprintf(tmp_str, "debug_show_class_and_method = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-instance")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "instance = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "mode = REALTIME;");
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = 1;
      }

    } else if (!strcmp(argv[i], "-input_dir")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "input_dir = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = 1;
      }
	
    } else if (!strcmp(argv[i], "-cart_outdir")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "cartesian_output_dir = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = 1;
      }
	
    } else if (!strcmp(argv[i], "-polar_outdir")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "polar_output_dir = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = 1;
      }
	
    } else if (!strcmp(argv[i], "-start")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "start_time = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "mode = ARCHIVE;");
        TDRP_add_override(&override, tmp_str);
        startTimeSet = true;
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-end")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "end_time = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "mode = ARCHIVE;");
        TDRP_add_override(&override, tmp_str);
        endTimeSet = true;
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
	sprintf(tmp_str, "mode = FILELIST;");
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } // if
    
  } // i
  
  if (iret) {
    _usage(cerr);
  }
  
  return (iret);
    
}

void Args::_usage(ostream &out)
{

  out << "Usage: " << _progName << " [options as below]\n"
      << "Compute QPE at surface in polar/cartesian coords\n"
      << "Options:\n"
      << "\n"
      << "  [ -h ] produce this list.\n"
      << "\n"
      << "  [ -cart_outdir ? ] set directory for cartesian results\n"
      << "\n"
      << "  [ -input_dir ? ] set input data directory\n"
      << "                   REALTIME and ARCHIVE modes\n"
      << "\n"
      << "  [ -d, -debug ] print debug messages\n"
      << "  [ -debug_trigger ] print debug messages about triggering\n"
      << "  [ -debug_realtime ] print debug messages about realtime ops\n"
      << "  [ -debug_methods ] print debug messages about classes and methods\n"
      << "\n"
      << "  [ -end \"yyyy mm dd hh mm ss\"] end time\n"
      << "           Sets mode to ARCHIVE\n"
      << "\n"
      << "  [ -f ? ] set file paths\n"
      << "           Sets mode to FILELIST\n"
      << "\n"
      << "  [ -instance ?] specify the instance\n"
      << "     app will register with procmap using this instance\n"
      << "     forces REALTIME mode\n"
      << "\n"
      << "  [ -polar_outdir ? ] set directory for polar results\n"
      << "\n"
      << "  [ -start \"yyyy mm dd hh mm ss\"] start time\n"
      << "           Sets mode to ARCHIVE\n"
      << "\n"
      << "  [ -v, -verbose ] print verbose debug messages\n"
      << "\n"
      << endl;

  Params::usage(out);

}







