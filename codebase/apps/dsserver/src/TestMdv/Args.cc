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
// Jan 1999
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <string.h>
#include <iostream>
using namespace std;

// Constructor

Args::Args (int argc, char **argv, char *prog_name)

{

  char tmp_str[BUFSIZ];

  // intialize

  OK = TRUE;
  TDRP_init_override(&override);
  
  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(prog_name, &cout);
      TDRP_usage(stdout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-volume")) {
      
      sprintf(tmp_str, "mode = GET_VOLUME_MODE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-vsection")) {
      
      sprintf(tmp_str, "mode = GET_VSECTION_MODE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-column")) {
      
      sprintf(tmp_str, "mode = GET_COLUMN_MODE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-time")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "data_time = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = FALSE;
      }
	
    } else if (!strcmp(argv[i], "-url")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "url = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = FALSE;
      }
	
    }
      
    
  } // i
  
  if (!OK) {
    _usage(prog_name, &cerr);
    TDRP_usage(stderr);
    
  }
    
}

// Destructor

Args::~Args ()

{

  TDRP_free_override(&override);

}
  
void Args::_usage(char *prog_name, ostream *out)
{


  *out << endl;
  *out << "Program : " << prog_name << endl;
  *out << "  Tests DsMdv server." << endl;
  *out << endl;

  *out << "Usage: " << prog_name << " [args as below]\n"
       << "options:\n"
       << "       [ -h ] produce this list.\n"
       << "       [ -column ] get-column mode\n"
       << "       [ -debug ] print debug messages\n"
       << "       [ -time ] specify time\n"       
       << "         Format is \"YYYY MM DD HH MM SS\"\n"
       << "       [ -url ] specify url\n"       
       << "         Format is \"mdvp:://host:port:dir\"\n"
       << "       [ -verbose ] print verbose debug messages\n"
       << "       [ -volume ] get-volume mode - the default\n"
       << "       [ -vsection ] get-vsection mode\n"
       << endl;

}





