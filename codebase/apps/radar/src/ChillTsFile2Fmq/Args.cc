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
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2007
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
      
    } else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "-debug")) {

      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
 
    } else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "-verbose")) {

      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-vv")) {

      sprintf(tmp_str, "debug = DEBUG_EXTRA;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-procmap")) {
      
      sprintf(tmp_str, "register_with_procmap = true;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-blocking")) {
      
      sprintf(tmp_str, "output_fmq_blocking = true;");
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
      
    } else if (!strcmp(argv[i], "-ninfo")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "n_pulses_per_info = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-npulses")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "n_pulses_per_message = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-fmq")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "output_fmq_path = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-nbytes")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "output_fmq_size = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-nslots")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "output_fmq_nslots = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-host")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "ts_tcp_server_host = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "input_mode = TCP_MODE;");
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-port")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "ts_tcp_server_port = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "input_mode = TCP_MODE;");
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-sim")) {

      // SIMULATE mode

      sprintf(tmp_str, "mode = SIMULATE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-archive")) {

      // SIMULATE mode

      sprintf(tmp_str, "mode = ARCHIVE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-f")) {

      // specify file list

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
      << "       [ -archive ] specify archive mode.\n"
      << "         Go through file list once only, no delays.\n"
      << "       [ -blocking ] block FMQ on write, if reader is not keeping pace\n"
      << "       [ -d, -debug ] print debug messages\n"
      << "       [ -f ? ] specify input file list.\n"
      << "       [ -fmq ? ] output FMQ name\n"
      << "       [ -instance ? ] instance for procmap\n"
      << "         Forces register with procmap\n"
      << "       [ -ninfo ? ] number of pulses before new info msg\n"
      << "         Every so often we write an info message\n"
      << "       [ -nbytes ? ] size of FMQ in bytes\n"
      << "       [ -npulses ? ] number of pulses per FMQ message\n"
      << "       [ -nslots ? ] size of FMQ in slots\n"
      << "       [ -port ? ]  port from which to read TCP data\n"
      << "       [ -procmap ] register with procmap\n"
      << "       [ -sim ] specify simulate mode.\n"
      << "         Repeatedly loop through file list.\n"
      << "       [ -v, -verbose ] print verbose debug messages\n"
      << "       [ -vv ] print extra verbose debug messages\n"
      << endl;
  
}
