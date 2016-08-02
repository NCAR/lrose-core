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
#include "Params.hh"
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <toolsa/DateTime.hh>
using namespace std;

// Constructor

Args::Args ()
{
  TDRP_init_override(&override);
  startTime = 0;
  endTime = 0;
}

// Destructor

Args::~Args ()

{
  TDRP_free_override(&override);
}

// parse the command line
//
// returns 0 on success, -1 on failure  

int Args::parse (int argc, char **argv, string &prog_name)

{

  _progName = prog_name;

  char tmp_str[BUFSIZ];
  bool OK = true;
  runOnce = false;
  
  // loop through args
  
  for (int i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      usage(cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-run_once")) {
      
      runOnce = true;
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-mode")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "mode = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
        cerr << "ERROR - bad mode: " << argv[i] << endl;
	OK = FALSE;
      }
	
    } else if (!strcmp(argv[i], "-trigger")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "trigger = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
        cerr << "ERROR - bad trigger: " << argv[i] << endl;
	OK = FALSE;
      }
      
    } else if (!strcmp(argv[i], "-start")) {
      
      if (i < argc - 1) {
	startTime = DateTime::parseDateTime(argv[++i]);
	if (startTime == DateTime::NEVER)
	{
          cerr << "ERROR - bad start time: " << argv[i] << endl;
	  OK = FALSE;
	}
	else
	{
	  sprintf(tmp_str, "mode = ARCHIVE;");
	  TDRP_add_override(&override, tmp_str);
	}
      } else {
        cerr << "ERROR - no start time supplied" << endl;
	OK = FALSE;
      }
    } else if (!strcmp(argv[i], "-end")) {
      
      if (i < argc - 1) {
	endTime = DateTime::parseDateTime(argv[++i]);
	if (endTime == DateTime::NEVER)
	{
          cerr << "ERROR - bad end time: " << argv[i] << endl;
	  OK = FALSE;
	}
	else
	{
	  sprintf(tmp_str, "mode = ARCHIVE;");
	  TDRP_add_override(&override, tmp_str);
	}
      } else {
        cerr << "ERROR - no end time supplied" << endl;
        OK = FALSE;
      }
    }
    
  } // i
  
  if (!OK) {
    cerr << endl;
    usage(cerr);
    return -1;
  }
  
  return 0;
  
}

void Args::usage(ostream &out)
{

  out << "Usage: " << _progName << " [args as below]\n"
      << "options:\n"
      << "  [ --, -h, -help, -man ] produce this list.\n"
      << "  [ -debug ] print debug messages\n"
      << "  [ -end \"yyyy mm dd hh mm ss\"] end time\n"
      << "    ARCHIVE mode only\n"
      << "  [ -mode ?] ARCHIVE or REALTIME\n"
      << "  [ -run_once] process one file and exit\n"
      << "  [ -start \"yyyy mm dd hh mm ss\"] start time\n"
      << "     ARCHIVE mode only\n"
      << "  [ -trigger ?] TIME_TRIGGER or FILE_TRIGGER\n"
      << "  [ -verbose ] print verbose debug messages\n"
      << endl << endl;
  
  out << "NOTE: for ARCHIVE mode, you must specify the times using\n"
      << "      -start and -end.\n"
      << "      In this case ARCHIVE mode will be automatically invoked."
      << endl << endl;
  
  Params::usage(out);
  
}




