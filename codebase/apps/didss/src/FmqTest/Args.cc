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
// Octember 2021
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
      
    } else if (!strcmp(argv[i], "-instance")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "instance = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      }
      
    } else if (!strcmp(argv[i], "-input_path")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "input_file_path = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      }
      
    } else if (!strcmp(argv[i], "-output_url")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "output_fmq_url = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      }
      
    } else if (!strcmp(argv[i], "-write_count")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "write_count = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      }
      
    } else if (!strcmp(argv[i], "-sleep_msecs")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "write_sleep_msecs = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      }
      
    } else if (!strcmp(argv[i], "-compress")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "output_compression = TRUE;");
	TDRP_add_override(&override, tmp_str);
      }
      
    } else if (!strcmp(argv[i], "-blocking")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "write_blocking = TRUE;");
	TDRP_add_override(&override, tmp_str);
      }
      
    } else if (!strcmp(argv[i], "-n_slots")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "output_n_slots = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      }
      
    } else if (!strcmp(argv[i], "-buf_size")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "output_buf_size = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      }
      
    } else if (!strcmp(argv[i], "-dmap_reg_interval")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "data_mapper_reg_interval = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
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
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -blocking ] write FMQ in blocking mode, default false\n"
      << "       [ -buf_size ? ] size of buffer in output FMQ, default 10M\n"
      << "       [ -compress ] compress FMQ messages, default false\n"
      << "       [ -d, -debug ] print debug messages\n"
      << "       [ -dmap_reg_interval ? ] how often to reg with data mapper (secs)\n"
      << "       [ -input_path ? ] specify input file path\n"
      << "       [ -instance ? ] specify instance for reg with procmap\n"
      << "       [ -n_slots ? ] number of slots in output FMQ, default 1K\n"
      << "       [ -output_url ? ] specify output fmq url\n"
      << "       [ -sleep_msecs ? ] sleep between writes (msecs), default 1000\n"
      << "       [ -v, -verbose ] print verbose debug messages\n"
      << "       [ -write_count ? ] number of times to write\n"
      << "                          default 1, -1 for no limit\n"
      << endl;
  
  Params::usage(out);

}







