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
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2012
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
      
    } else if (!strcmp(argv[i], "-vv") ||
               !strcmp(argv[i], "-extra")) {
      
      sprintf(tmp_str, "debug = DEBUG_EXTRA;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-i") ||
               !strcmp(argv[i], "-instance")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "instance = \"%s\";", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-fmq")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "fmq_url = \"%s\";", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "input_mode = FMQ_INPUT;");
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-start")) {
      
      sprintf(tmp_str, "seek_to_start_of_input = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-input_tcp_host")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "input_tcp_host = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "input_mode = TCP_INPUT;");
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-input_tcp_port")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "input_tcp_port = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "input_mode = TCP_INPUT;");
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-output_tcp_port")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "output_tcp_port = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "ops_mode = OPS_MODE_SERVER;");
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-f") ||
               !strcmp(argv[i], "-files")) {
      
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
	sprintf(tmp_str, "input_mode = FILE_LIST;");
	TDRP_add_override(&override, tmp_str);
      }

    } else if (!strcmp(argv[i], "-monitor")) {
      
      sprintf(tmp_str, "print_mode = PRINT_MONITOR;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-summary")) {
      
      sprintf(tmp_str, "print_mode = PRINT_SUMMARY;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-full")) {
      
      sprintf(tmp_str, "print_mode = PRINT_FULL;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-data")) {
      
      sprintf(tmp_str, "print_mode = PRINT_DATA;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-power")) {
      
      sprintf(tmp_str, "print_mode = PRINT_POWER_AND_FREQ;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-georef")) {
      
      sprintf(tmp_str, "print_mode = PRINT_PLATFORM_GEOREF;");
      TDRP_add_override(&override, tmp_str);
      
     } else if (!strcmp(argv[i], "-flags_monitor")) {
      
      sprintf(tmp_str, "flags_in_monitor_mode = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-flags_summary")) {
      
      sprintf(tmp_str, "flags_in_summary_mode = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-scan_name")) {
      
      sprintf(tmp_str, "scan_name_in_summary_mode = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-subsecs_precision")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "subsecs_precision_in_summary_mode = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-gate")) {
      
      sprintf(tmp_str, "print_mode = PRINT_SINGLE_GATE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-range")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "range_for_single_gate = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-update")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "update_interval = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-summary_header_interval")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str,
                "header_interval_in_summary_mode = %s;",
                argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-check_missing")) {
      
      sprintf(tmp_str, "check_for_missing_beams = TRUE;");
      TDRP_add_override(&override, tmp_str);

      if (i < argc - 1) {
	sprintf(tmp_str, "max_delta_angle = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-check_time_increasing")) {
      
      sprintf(tmp_str, "check_for_increasing_time = TRUE;");
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
      << "  [--, -h, -help, -man] produce this list.\n"
      << "  [-check_missing ?] check for missing beams\n"
      << "     Specify the max delta angle (deg) for normal ops.\n"
      << "  [-check_time_increasing ?] check for increaasing time\n"
      << "     Prints warning if time goes backwards.\n"
      << "  [-data] full plus data - very verbose\n"
      << "  [-d, -debug] print debug messages\n"
      << "  [-f, -files ???] specify input file list.\n"
      << "     Reads moments data from specified files.\n"
      << "     Sets FILE_INPUT mode.\n"
      << "  [-flags_summary] tilt flags in summary mode\n"
      << "  [-flags_monitor] flags in monitor mode\n"
      << "  [-fmq ?] input fmq url\n"
      << "     Sets FMQ_INPUT mode.\n"
      << "  [-full] scrolling, all params printed\n"
      << "  [-i, -instance ?] specify instance for procmap\n"
      << "  [-input_tcp_host ?] specify host for input tcp server\n"
      << "  [-input_tcp_port ?] specify port for input tcp server\n"
      << "     Sets TCP_INPUT mode.\n"
      << "  [-gate] print data for single gate\n"
      << "  [-georef] print platform georef info with each beam\n"
      << "     if available\n"
      << "  [-power] print power and frequency summary\n"
      << "  [-range] range for single gate print - km\n"
      << "  [-scan_name] add scan name at end of line in summary mode\n"
      << "  [-start] Seek to start of FMQ\n"
      << "     If not set, reading begins at the end of the FMQ.\n"
      << "  [-summary] scrolling summary\n"
      << "  [-summary_header_interval ?] header print interval (lines)\n"
      << "     in summary mode\n"
      << "  [-output_tcp_port ?] specify port for data output\n"
      << "     Port on which to listen for clients.\n"
      << "     Sets OPS_MODE_SERVER mode.\n"
      << "  [-subsecs_precision ?] set precision of subsecs in summary mode\n"
      << "     default is 0 - i.e. whole secs only\n"
      << "  [-update ?] update interval in seconds\n"
      << "     If 0, every beam printed.\n"
      << "     If < 0, updates on beam count instead of secs\n"
      << "  [-v, -verbose] print verbose debug messages\n"
      << "  [-vv, -extra] print extra verbose debug messages\n"
      << endl;
  
  Params::usage(out);

}
