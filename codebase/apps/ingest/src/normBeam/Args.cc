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
// November 1999
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <string.h>
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
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-monitor")) {
      
      sprintf(tmp_str, "print_type = MONITOR;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-summary")) {
      
      sprintf(tmp_str, "print_type = SUMMARY;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-full")) {
      
      sprintf(tmp_str, "print_type = FULL;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-data")) {
      
      sprintf(tmp_str, "print_type = DATA;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-gate")) {
      
      sprintf(tmp_str, "print_type = SINGLE_GATE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-range")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "range_for_single_gate = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-start")) {
      
      sprintf(tmp_str, "seek_to_start_of_input = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-update")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "update_interval = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-url")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "fmq_url = \"%s\";", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
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
      << "  [-debug] print debug messages\n"
      << "  [-monitor] non-scrolling single-line printout\n"
      << "  [-summary] scrolling summary\n"
      << "  [-full] scrolling, all params printed\n"
      << "  [-data] full plus data - very verbose\n"
      << "  [-gate] print data for single gate\n"
      << "  [-range] range for single gate print - km\n"
      << "  [-start] Seek to start of FMQ\n"
      << "     If not set, reading begins at the end of the FMQ.\n"
      << "  [-update] update interval in seconds\n"
      << "    If 0, every beam printed.\n"
      << "    If < 0, updates on beam count instead of secs\n"
      << "  [-url ?] input url\n"
      << endl;
  
  Params::usage(out);

}







