// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2025                                         
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
      
    } else if (!strcmp(argv[i], "-debug") ||
               !strcmp(argv[i], "-d")) {

      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
 
    } else if (!strcmp(argv[i], "-verbose") ||
               !strcmp(argv[i], "-v")) {

      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-vv")) {

      sprintf(tmp_str, "debug = DEBUG_EXTRA;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-procmap")) {
      
      sprintf(tmp_str, "register_with_procmap = true;");
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
      
    } else if (!strcmp(argv[i], "-port")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "udp_port = %s;", argv[++i]);
        TDRP_add_override(&override, tmp_str);
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
      << "       [ -debug, -d ] print debug messages\n"
      << "       [ -fmq ?] output FMQ name\n"
      << "       [ -instance ?] instance for procmap\n"
      << "         Forces register with procmap\n"
      << "       [ -nbytes ?] size of FMQ in bytes\n"
      << "       [ -npulses ?] number of pulses per FMQ message\n"
      << "       [ -nslots ?] size of FMQ in slots\n"
      << "       [ -port ?]  port on which to listen for UDP data\n"
      << "       [ -procmap] register with procmap\n"
      << "       [ -verbose, -v ] print verbose debug messages\n"
      << "       [ -vv ] print extra verbose debug messages\n"
      << endl;
  
}
