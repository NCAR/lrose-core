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
// Sept 1998
//
//////////////////////////////////////////////////////////

using namespace std;

#include "Args.hh"
#include <string.h>
#include <cstdlib>

// Constructor

Args::Args (int argc, char **argv, char *prog_name)

{

  char tmp_str[BUFSIZ];

  // intialize

  OK = TRUE;
  topDir = NULL;
  TDRP_init_override(&override);
  
  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "-h")) {
      
      _usage(prog_name, stdout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-top")) {
      
      if (i < argc - 1) {
	topDir = argv[++i];
      } else {
	OK = FALSE;
      }
      
    } // if
    
  } // i
  
  if (topDir == NULL) {
    OK = FALSE;
  }

  if (!OK) {
    _usage(prog_name, stderr);
  }
    
}

// Destructor

Args::~Args ()

{

  TDRP_free_override(&override);

}
  
void Args::_usage(char *prog_name, FILE *out)
{

  fprintf(out, "\n");

  fprintf(out, "Program %s:\n", prog_name);

  fprintf(out, "%s",
	  "  Reads in starting parameters, optionally using the defaults.\n"
	  "  It then recurses through a directory tree. If it finds a \n"
	  "  file named '_params' anywhere, it creates a new Params\n"
	  "  object using the copy constructor and then loads in\n"
	  "  the new params from the file. When it pops up out of\n"
	  "  the directory these changes are lost and the processing\n"
	  "  reverts to the parameters at the higher level.\n\n");

  fprintf(out, "%s%s%s%s",
	  "Usage: ", prog_name, " [args as below] -top topDir\n",
	  "options:\n"
	  "       [ -h ] produce this list.\n"
	  "       [ -debug ] print debug messages\n"
	  "       [ -verbose ] print verbosedebug messages\n"
	  "\n");

  fprintf(out, "NOTE: the -top arg is required.\n\n");

  TDRP_usage(out);

}






