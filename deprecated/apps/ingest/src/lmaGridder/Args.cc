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
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1998
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <string.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/DateTime.hh>

using namespace std;

// Constructor

Args::Args (int argc, char **argv, char *prog_name)

{

  char tmp_str[BUFSIZ];

  // intialize

  OK = TRUE;
  nFiles = 0;
  startTime = 0;
  endTime = 0;
  TDRP_init_override(&override); 

  // loop through args

  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      usage(prog_name, cout);
      exit(0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-mode")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "mode = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
	i=i+1; if (i>argc) continue;
      } else {
	OK = FALSE;
      }
		
    } else if (!strcmp(argv[i], "-start")) {
      if (!( ++i <= argc ))
      {
        cerr << "Date must follow -start argument." << endl;
	OK = FALSE;
      }
      startTime = DateTime::parseDateTime( argv[i] );
      if ( startTime == DateTime::NEVER ) 
      {
        cerr << "Bad date/time syntax in -start specification." << endl;
	OK = FALSE;
      }
      else 
      {
        TDRP_add_override( &override, "mode = TIME_INTERVAL;" );
      }
    } else if (!strcmp(argv[i], "-end")) {
      if (!( ++i < argc ))
      {
        cerr << "Date must follow -end argument." << endl;
        OK = FALSE;
      }
      endTime = DateTime::parseDateTime( argv[i] );
      if ( endTime == DateTime::NEVER ) 
      {
        cerr << "Bad date/time syntax in -end specification." << endl;
	OK = FALSE;
      }
      else 
      {
        TDRP_add_override( &override, "mode = TIME_INTERVAL;" );
      }

    } else if (!strcmp(argv[i], "-f")) {
      
      sprintf(tmp_str, "mode = ARCHIVE;");

      TDRP_add_override(&override, tmp_str);

      if(i < argc - 1) {

	int j;

	// search for next arg which starts with '-'

	for (j = i + 1; j < argc; j++)
	  if (argv[j][0] == '-')
	    break;
	
	/*
	 * compute number of files
	 */

	nFiles = j - i - 1;

	// set file name array

	filePaths = argv + i + 1;

	// Update i
	i=i+nFiles;
	
      }
      
    } // if
    
  } // i

  if (!OK) {
    usage(prog_name, cerr);
  }
    
}

// Destructor

Args::~Args ()

{
  TDRP_free_override(&override);
}

void Args::usage(char *prog_name, ostream &out)
{

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -f file_paths] file paths list - ARCHIVE MODE\n"
      << "       [ -start \"YYYY MM DD HH MM SS\" ] Start time  - TIME_INTERVAL MODE\n"
      << "       [ -end \"YYYY MM DD HH MM SS\" ] End time  - TIME_INTERVAL MODE\n"
      << "       [ -mode ?] ARCHIVE, REALTIME or TIME_INTERVAL\n"
      << endl;
  
  Params::usage(out);

}






