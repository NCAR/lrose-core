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
#include <cstdio>
#include <iostream>
#include <toolsa/str.h>
using namespace std;

// Constructor

Args::Args()
{
  TDRP_init_override(&override);
}

// Destructor

Args::~Args()
{
  TDRP_free_override(&override);
}

//////////////////////
// parse command line
//
// Returns 0 on success, -1 on failue.

int Args::parse (int argc, char **argv, string &prog_name)

{
  
  char tmp_str[BUFSIZ];
  int iret = 0;
  do_clean = false;
  do_delete = false;

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(prog_name, cout);
      TDRP_usage(stdout);
      exit(0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-instance")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "instance = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-noThreads")) {
      
      sprintf(tmp_str, "no_threads = TRUE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-clean")) {
      
      do_clean = true;
      
    } else if (!strcmp(argv[i], "-delete")) {
      
      if (i < argc - 3) {
	delete_datatype = argv[++i];
	delete_dir = argv[++i];
	delete_hostname = argv[++i];
	do_delete = true;
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-save")) {
      
      sprintf(tmp_str, "save_state = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-noSave")) {
      
      sprintf(tmp_str, "save_state = FALSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-savePath")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "save_state_path = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-saveSecs")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "save_state_secs = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-purge")) {
      
      sprintf(tmp_str, "purge_old_entries = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-noPurge")) {
      
      sprintf(tmp_str, "purge_old_entries = FALSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-purgeAge")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "purge_age_days = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-addType")) {
      
      if (i < argc - 1) {
	string newType = argv[++i];
	extraTypes.push_back(newType);
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-params")) {
      
      i++;
      
    } else if (!Params::isArgValid(argv[i])) {
      
      cerr << "ERROR - Invalid command line arg: " << argv[i] << endl;
      iret = -1;

    } // if
    
  } // i

  if (iret) {
    _usage(prog_name, cerr);
    TDRP_usage(stderr);
    return -1;
  }

  return 0;
    
}

void Args::_usage(const string &prog_name, ostream &out)
{
  
  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       ----- debugging ----------------------------------\n"
      << "       [ -debug ] print debug messages.\n"
      << "       [ -verbose ] print verbose debug messages.\n"
      << "       ----- running ------------------------------------\n"
      << "       [ -instance ? ] override instance. Default 'primary'.\n"
      << "       [ -noThreads ] force single-threaded operation.\n"
      << "       ----- deleting ------------------------------------\n"
      << "       [ -clean ] clean out store on startup.\n"
      << "                  Note: overrides delete.\n"
      << "       [ -delete datatype dir host ] delete specified entry.\n"
      << "         Use 'all' as a wildcard for datatype, dir or host.\n"
      << "       ----- saving state -------------------------------\n"
      << "       [ -save ] save state (default)\n"
      << "       [ -noSave ] do not save state\n"
      << "       [ -savePath ? ] path for saving state to file\n"
      << "         Default is '/tmp/dmap_state_table'\n"
      << "       [ -saveSecs ? ] how often to save state, secs, default 10\n"
      << "         Default is 10\n"
      << "       ----- purging old entries ------------------------\n"
      << "       [ -purge ] purge old entries (default)\n"
      << "       [ -noPurge ] do not purge old entries\n"
      << "       [ -purgeAge ? ] age in days for purging, default 1.0\n"
      << "       ----- adding data types ---------------------------\n"
      << "       [ -addType ? ] add a data type to the list of valid types\n"
      << "         Default list is: raw, mdv, spdb, titan, www, www_content\n"
      << "                          grib, sim, simulate, md, nc\n"
      << endl;

  out << "NOTE: this is the simplified version of the DataMapper.\n"
      << "      This only uses TCP/IP for message passing.\n"
      << "      The input FMQ is no longer used.\n"
      << "      Fowarding of messages has been disabled.\n"
      << endl;
  
}

