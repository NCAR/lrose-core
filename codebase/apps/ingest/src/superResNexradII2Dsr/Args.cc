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
//
// Class to deal with command line arguments. Include a few
// things, and use the standard namespace.
//
#include "Args.hh"
#include "Params.hh"
#include <cstring>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
using namespace std;

// Constructor. This does most of the work.

Args::Args (int argc, char **argv, char *prog_name)

{

  char tmp_str[BUFSIZ];
  //
  // Intialize
  //
  OK = TRUE;
  nFiles = 0;
  TDRP_init_override(&override); 
  //
  // Loop through the command line arguments.
  //
  for (int i =  1; i < argc; i++) {
    //
    // If this is a request to print the help message, then print
    // it and exit.
    //
    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      usage(prog_name, cout);
      exit(0);
      //      
    } else if (!strcmp(argv[i], "-debug")) {

      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-debug_hdrs")) {

      sprintf(tmp_str, "debug = DEBUG_HEADERS;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-debug_data")) {

      sprintf(tmp_str, "debug = DEBUG_DATA;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-f")) {
      //
      // The "-f" option means that file names follow.
      // Set the mode to archive and make a note of the filenames.
      //
      sprintf(tmp_str, "mode = READ_ARCHIVE_FILES;");

      TDRP_add_override(&override, tmp_str);

      if (i < argc - 1) {

	int j;
	//
	// Search for next arg which starts with '-'
	//
	for (j = i + 1; j < argc; j++)
	  if (argv[j][0] == '-')
	    break;
	//
	// Compute the number of files
	//
	nFiles = j - i - 1;
	//
	// Set file name array
	//
	filePaths = argv + i + 1;
	//
	// Update i
	//
	i=i+nFiles;
	
      }
      
    } // if
    
  } // i

  if (!OK) {
    usage(prog_name, cerr);
  }
    
}
//
// Destructor. Frees up some memory.
//
Args::~Args ()

{
  TDRP_free_override(&override);
}
//
// The 'usage' method prints a simple help message.
//
void Args::usage(char *prog_name, ostream &out)
{

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -f file_paths] file paths list - ARCHIVE MODE\n"
      << "       [ -debug ] normal_debugging\n"
      << "       [ -debug_hdrs ] headers debugging\n"
      << "       [ -debug_data ] data debugging\n"
      << endl;
  
  Params::usage(out);

}






