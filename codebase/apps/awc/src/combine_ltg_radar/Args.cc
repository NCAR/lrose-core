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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 01:51:41 $
//   $Id: Args.cc,v 1.7 2016/03/07 01:51:41 dixon Exp $
//   $Revision: 1.7 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Args.cc: class controlling the command line arguments for the
 *          program.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <cstdlib>

#include <toolsa/os_config.h>
#include <tdrp/tdrp.h>
#include <toolsa/str.h>

#include "Args.hh"

/**********************************************************************
 * Constructor
 */

Args::Args (int argc, char **argv, char *prog_name) :
  okay(true),
  numInputFiles(0),
  inputFileList(0),
  startTime(DateTime::NEVER),
  endTime(DateTime::NEVER)
{
  char tmp_str[BUFSIZ];

  // Intialize

  TDRP_init_override(&override);
  
  // search for command options
  
  for (int i =  1; i < argc; i++)
  {
    if (STRequal_exact(argv[i], "--") ||
	STRequal_exact(argv[i], "-h") ||
	STRequal_exact(argv[i], "-help") ||
	STRequal_exact(argv[i], "-man"))
    {
      _usage(prog_name, stdout);
      exit(0);
    }
    else if (STRequal_exact(argv[i], "-debug"))
    {
      sprintf(tmp_str, "debug_level = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
    }
    else if (STRequal_exact(argv[i], "-end") ||
	     STRequal_exact(argv[i], "-endtime"))
    {
      if (i < argc - 1)
      {
	if (endTime.set(argv[++i]) == DateTime::NEVER)
	{
	  cerr << "*** Invalid end time string entered: " <<
	    argv[i] << endl << endl;
	  
	  okay = false;
	}
      }
      else
      {
	okay = false;
      }
    }	
    else if (STRequal_exact(argv[i], "-mode"))
    {
      ++i;
      
      if (STRequal_exact(argv[i], "ARCHIVE") ||
	  STRequal_exact(argv[i], "ARCHIVE_MODE"))
      {
	sprintf(tmp_str, "mode = ARCHIVE_MODE;");
	TDRP_add_override(&override, tmp_str);
      }
      else if (STRequal_exact(argv[i], "REALTIME") ||
	  STRequal_exact(argv[i], "REALTIME_MODE"))
      {
	sprintf(tmp_str, "mode = REALTIME_MODE;");
	TDRP_add_override(&override, tmp_str);
      }
      else if (STRequal_exact(argv[i], "TIME_LIST") ||
	  STRequal_exact(argv[i], "TIME_LIST_MODE"))
      {
	sprintf(tmp_str, "mode = TIME_LIST_MODE;");
	TDRP_add_override(&override, tmp_str);
      }
      else
      {
	cerr << "ERROR: Invalid mode specified: " << argv[i] << endl;
	okay = false;
      }
    }
    else if (STRequal_exact(argv[i], "-if"))
    {
      if (i < argc - 1)
      {
	int j;
	
	// Search for next arg which starts with '-'

	for (j = i + 1; j < argc; j++)
	  if (argv[j][0] == '-')
	    break;
	
	// Compute number of files

	numInputFiles = j - i - 1;

	// Set file name array

	inputFileList = argv + i + 1;
      }
      else
      {
	okay = false;
      }
    }
    else if (STRequal_exact(argv[i], "-start") ||
	     STRequal_exact(argv[i], "-starttime"))
    {
      if (i < argc - 1)
      {
	if (startTime.set(argv[++i]) == DateTime::NEVER)
	{
	  cerr << "*** Invalid start time string entered: " <<
	    argv[i] << endl << endl;
	  
	  okay = false;
	}
      }
      else
      {
	okay = false;
      }
    }	
  } /* i */

  if (!okay)
  {
    _usage(prog_name, stderr);
  }
    
}


/**********************************************************************
 * Destructor
 */

Args::~Args(void)
{
  TDRP_free_override(&override);
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _usage() - Print the usage for this program.
 */

void Args::_usage(char *prog_name, FILE *stream)
{
  fprintf(stream, "%s%s%s",
	  "This program combines a lightning MDV grid with a radar\n"
          "(or equivalent) MDV grid to create an interest field in\n"
          "the same units as the radar grid.\n"
	  "\n"
	  "Usage:\n\n", prog_name, " [options as below]\n"
	  "options:\n"
	  "       [ --, -h, -help, -man, -usage] produce this list.\n"
	  "       [ -debug ] print debug messages\n"
	  "       [ -end yyyy/mm/dd_hh:mm:ss ] end time (TIME_LIST mode)\n"
	  "       [-if input_file_list (ARCHIVE mode) ]\n"
	  "       [-mode ARCHIVE | REALTIME | TIME_LIST ]\n"
	  "       [ -start yyyy/mm/dd_hh:mm:ss ] start time (TIME_LIST mode)\n"
	  "\n");

  TDRP_usage(stream);
}
