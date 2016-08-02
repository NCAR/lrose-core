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

/**
 * @file Args.cc
 *
 * Command line arguments for the Janitor application.
 *
 * @author Mike Dixon
 * @see something
 */

#include "Args.hh"
#include "Params.hh"
#include <cstring>
#include <cstdlib>
using namespace std;

// Constructor

Args::Args()

{
  TDRP_init_override(&override);
}

// Destructor

Args::~Args ()

{
  TDRP_free_override(&override);
}
  
// parse args

int Args::parse(int argc, char **argv, const string &progName)

{

  int iret = 0;
  char tmp_str[BUFSIZ];

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if ( (!strcmp(argv[i], "-h")) ||
	 (!strcmp(argv[i], "-help")) ||
	 (!strcmp(argv[i], "--")) ||
	 (!strcmp(argv[i], "-?"))
	 ) {
      
      _usage(progName, cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-delay")) {
      
      if (i<argc-1) {
	sprintf(tmp_str, "SleepBetweenPasses = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-instance")) {
      
      if (i<argc-1) {
	sprintf(tmp_str, "instance = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

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
      
    } else if (!strcmp(argv[i], "-report")) {
      
      sprintf(tmp_str, "report = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-once")){

      sprintf(tmp_str, "once_only = TRUE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-top")) {
      
      if (i<argc-1) {
	sprintf(tmp_str, "top_dir = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } // if
    
  } // i
  
  if (iret) {
    _usage(progName, cerr);
    return -1;
  }

  return 0;
    
}

void Args::_usage(const string &progName, ostream &out)
{

  out << "Usage: " << progName << " [options as below]\n"
      << "options:\n"
      << "  [ -h, --, -?, -help ] produce this list.\n"
      << "  [ -d, -debug ] print debug messages\n"
      << "  [ -delay ? ] set the delay between passes - secs\n"
      << "  [ -instance ? ] set the program instance\n"
      << "  [ -once ] traverse once and then exit\n"
      << "  [ -report ] turn on reporting\n"
      << "    Writes a report file in each dir\n"
      << "  [ -top ? ] set top level directory\n"
      << "  [ -v, -verbose ] print verbose debug messages\n"
      << "  [ -vv, -extra ] print extra verbose debug messages\n"
      << endl;
  
  out << "  The Janitor performs simple housekeeping tasks.\n"
      << "  It recurses through a directory tree. If it finds a \n"
      << "  file named '_Janitor' anywhere, it reads parameters\n"
      << "  from this file. When the Janitor pops up out of\n"
      << "  the directory these changes are lost and the processing\n"
      << "  reverts to the parameters at the higher level.\n\n"
      << "  The Janitor is described in more depth in its web page.\n"
      << endl;

  Params::usage(out);

}
