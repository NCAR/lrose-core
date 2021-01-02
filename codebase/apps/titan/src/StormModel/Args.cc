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
// February 1998
//
//////////////////////////////////////////////////////////

#include "Args.h"
#include <toolsa/str.h>
#include <toolsa/umisc.h>
using namespace std;

// Constructor

Args::Args (int argc, char **argv, char *prog_name)

{

  char tmp_str[BUFSIZ];

  // intialize

  OK = TRUE;
  Done = FALSE;
  checkParams = FALSE;
  printParams = FALSE;
  printShort = FALSE;
  paramsFilePath = (char *) NULL;
  TDRP_init_override(&override);

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      usage(prog_name, stdout);
      Done = TRUE;
      
    } else if (!strcmp(argv[i], "-check_params")) {
      
      checkParams = TRUE;
      
    } else if (!strcmp(argv[i], "-print_params")) {
      
      printParams = TRUE;
      
    } else if (!strcmp(argv[i], "-print_short")) {
      
      printShort = TRUE;
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-mdebug")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "malloc_debug_level = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = FALSE;
      }
	
    } else if (!strcmp(argv[i], "-deltas")) {
      
      sprintf(tmp_str, "output_deltas = TRUE;");
      TDRP_add_override(&override, tmp_str);
	
    } else if (!strcmp(argv[i], "-statcode")) {
      
      sprintf(tmp_str, "do_statcode_tests = TRUE;");
      TDRP_add_override(&override, tmp_str);
	
    } else if (!strcmp(argv[i], "-params")) {
	
      if (i < argc - 1) {
	paramsFilePath = argv[++i];
      } else {
	OK = FALSE;
      }
      
    } // if
    
  } // i

  if (!OK) {
    usage(prog_name, stderr);
  }
    
}

void Args::usage(char *prog_name, FILE *out)
{

  fprintf(out, "%s%s%s%s",
	  "Usage: ", prog_name, " [options as below]\n",
	  "options:\n"
	  "       [ --, -h, -help, -man ] produce this list.\n"
	  "       [ -check_params ] check parameter usage\n"
	  "       [ -debug ] print debug messages\n"
	  "       [ -deltas ] output deltas\n"
	  "       [ -mdebug level ] set malloc debug level\n"
	  "       [ -params ?] params file path\n"
	  "       [ -print_params ] print parameter usage\n"
	  "       [ -print_short ] print short parameter usage\n"
	  "       [ -statcode ] run statcode tests\n"
	  "       [ -verbose ] print verbose debug messages\n"
	  "\n");
  
  fprintf(out, "\n\n");

}






