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
// Feb 1999
//
//////////////////////////////////////////////////////////

#include "Params.hh"
#include "Args.hh"
#include <cstring>
#include <iostream>
using namespace std;

// Constructor

Args::Args (const string &prog_name)

{
  _progName = prog_name;
  TDRP_init_override(&override);
}


// Destructor

Args::~Args ()

{
  TDRP_free_override(&override);
}

// Parse command line
// Returns 0 on success, -1 on failure

int Args::parse (const int argc, const char **argv)

{

  int iret = 0;
  char tmp_str[BUFSIZ];

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
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-ldata")) {
      
      sprintf(tmp_str, "search_mode = SEARCH_DSFILEDIST_AND_LDATA;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-noThreads")) {
      
      sprintf(tmp_str, "no_threads = TRUE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-top")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "source_top_dir = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-maxs")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "max_simultaneous_per_host = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "-instance")) {

      if (i < argc - 1) {
	sprintf(tmp_str, "instance = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } // if
    
  } // i
  
  if (iret) {
    _usage(cerr);
  }

  return iret;
    
}

void Args::_usage(ostream &out)
{


  out << endl;
  out << "Program : " << _progName << endl;
  out << "  Distributes files to remote hosts." << endl;
  out << endl;

  out << "Usage: " << _progName << " [args as below]\n"
      << "options:\n"
      << "       [ -d, -debug ] print debug messages\n"
      << "       [ -h ] produce this list.\n"
      << "       [ -i, -instance ] instance.\n"
      << "       [ -ldata ] search for latest_data_info.xml files\n"
      << "       [ -maxs ?] max number of simultaneous transfers to\n"
      << "                  each host - default is 1.\n"
      << "       [ -noThreads ] suppress multi-threading\n"
      << "         Note: use with single directory only\n"
      << "       [ -top ?] set top dir\n"
      << "       [ -v, -verbose ] print verbose debug messages\n"
      << "       [ -vv, -extra  ] print extra verbose debug messages\n"
      << endl;
  
  Params::usage(out);
  
}

