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
// April 1998
//
//////////////////////////////////////////////////////////

#include <cstring>

#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Args.hh"

// Constructor

Args::Args (int argc, char **argv, char *prog_name) :
    startTime(DateTime::NEVER),
    endTime(DateTime::NEVER)
{
  char tmp_str[BUFSIZ];

  // intialize

  OK = TRUE;
  Done = FALSE;
  checkParams = FALSE;
  printParams = FALSE;
  printShort = FALSE;
  paramsFilePath = NULL;
  nFiles = 0;
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
      
    } else if (!strcmp(argv[i], "-mode")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "mode = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = FALSE;
      }
	
    } else if (!strcmp(argv[i], "-start") ||
	       !strcmp(argv[i], "-starttime")) {
      
      if (i < argc - 1) {
	  if (startTime.set(argv[++i]) == DateTime::NEVER)
	  {
	      cerr << "*** Invalid start time string entered: " <<
		  argv[i] << endl << endl;

	      OK = false;
	  }
      } else {
	  OK = FALSE;
      }
	
    } else if (!strcmp(argv[i], "-end") ||
	       !strcmp(argv[i], "-endtime")) {
	if (i < argc - 1) {
	    if (endTime.set(argv[++i]) == DateTime::NEVER)
	    {
		cerr << "*** Invalid end time string entered: " <<
		    argv[i] << endl << endl;

		OK = false;
	    }
	} else {
	    OK = FALSE;
	}
	
    } else if (!strcmp(argv[i], "-params")) {
	
      if (i < argc - 1) {
	paramsFilePath = argv[i+1];
      } else {
	OK = FALSE;
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
	  "       [ -end \"yyyy mm dd hh mm ss\"] end time\n"
	  "                              ARCHIVE and TIME_LIST modes only\n"
	  "       [ -f file_paths] file paths list - ARCHIVE MODE\n"
	  "       [ -mode ?] ARCHIVE, REALTIME, LATEST_DATA, \n"
	  "                  TIME_LIST, or MULTIPLE_URL\n"
	  "       [ -params ?] params file path\n"
	  "       [ -print_params ] print parameters with comments\n"
	  "       [ -print_short ] print parameters - short version\n"
	  "       [ -start \"yyyy mm dd hh mm ss\"] start time\n"
	  "                                  ARCHIVE and TIME_LIST modes only\n"
	  "       [ -verbose ] print verbosedebug messages\n"
	  "\n");

  fprintf(out,
	  "NOTE: for ARCHIVE mode, use either -f or specify the times\n"
	  "      using start and end. In either case ARCHIVE mode will\n"
	  "      be automatically invoked. Start and end times can also\n"
	  "      be used in TIME_LIST mode to override values in the\n"
	  "      parameter file.\n");

  fprintf(out, "\n\n");

  TDRP_usage(out);
}






