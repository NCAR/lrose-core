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
#include "Params.hh"
#include <string.h>
#include <cstdlib>

// Constructor

Args::Args (int argc, char **argv, char *prog_name)

{

  char tmp_str[BUFSIZ];

  // intialize

  OK = TRUE;
  TDRP_init_override(&override);
  
  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "-h")) {
      
      _usage(prog_name, cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-modify")) {
      
      sprintf(tmp_str, "modify_and_print = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } // if
    
  } // i

  if (!OK) {
    _usage(prog_name, cerr);
  }
    
}

// Destructor

Args::~Args ()

{

  TDRP_free_override(&override);

}
  
void Args::_usage(char *prog_name, ostream &out)
{

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ -h ] produce this list.\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -modify ] modify selected params and print\n"
      << "       [ -verbose ] print verbose debug messages\n"
      << endl;

  Params::usage(out);

}






