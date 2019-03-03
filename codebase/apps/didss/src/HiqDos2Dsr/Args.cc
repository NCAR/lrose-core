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
/*********************************************************************
 * Args.cc: class controlling the command line arguments for the
 *          program.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include <cstring>

#include <toolsa/os_config.h>
#include <tdrp/tdrp.h>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Args.hh"
using namespace std;

/**********************************************************************
 * Constructor
 */

Args::Args (int argc, char **argv, char *prog_name) :
  _progName(prog_name)
{
  string tmp_str;

  string mode;
  DateTime start_time;
  DateTime end_time;
  
  // Intialize

  bool okay = true;

  TDRP_init_override(&override);
  
  // search for command options
  
  for (int i =  1; i < argc; i++)
  {
    if (STRequal_exact(argv[i], "--") ||
	STRequal_exact(argv[i], "-help") ||
	STRequal_exact(argv[i], "-man"))
    {
      _usage(stdout);
      exit(0);
    }
    else if (STRequal_exact(argv[i], "-archive"))
    {
      tmp_str = "archive_file = TRUE;";
      TDRP_add_override(&override, tmp_str.c_str());
      
      if (i < argc-1)
      {
	tmp_str = string("archive_file_path = ") + argv[i+1] + ";";
	TDRP_add_override(&override, tmp_str.c_str());
	++i;
      }
      else
      {
	cerr << "ERROR: Missing archive file path on command line" << endl;
	okay = false;
      }
    }
    else if (STRequal_exact(argv[i], "-debug"))
    {
      tmp_str = "debug = true;";
      TDRP_add_override(&override, tmp_str.c_str());
    }
    else if (STRequal_exact(argv[i], "-if"))
    {
      // First set the input type to be FILE

      tmp_str = "input_type = RAP_ARCHIVE_FILE;";
      TDRP_add_override(&override, tmp_str.c_str());

      // Now create the input file list

      bool first_file = true;

      tmp_str = "input_files = { ";

      while (argv[i+1][0] != '-')
      {
	if (first_file)
	  first_file = false;
	else
	  tmp_str += ", ";

	tmp_str += string("\"") + argv[++i] + "\"";
      }

      tmp_str += " };";
      TDRP_add_override(&override, tmp_str.c_str());
    }
    else if (STRequal_exact(argv[i], "-summary"))
    {
      tmp_str = "print_summary = TRUE;";
      TDRP_add_override(&override, tmp_str.c_str());
      
      if ((i < argc - 1) && argv[i+1][0] != '-')
      {
	tmp_str = string("summary_interval = ") + argv[i+1] + ";";
	TDRP_add_override(&override, tmp_str.c_str());
	++i;
      }
    }
  } /* i */

  if (!okay)
  {
    _usage(stderr);
    exit(-1);
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

void Args::_usage(FILE *stream)
{
  fprintf(stream, "%s%s%s",
	  "This program reads raw beam data from an Hiq radar processor\n"
	  "and converts the data to RAP Dsr format.\n"
	  "\n"
	  "Usage:\n\n", _progName.c_str(), " [options] as below:\n\n"
	  "       [ --, -help, -man ] produce this list.\n"
	  "       [ -archive file_path] write raw radar data to archive file\n"
	  "       [ -debug ] debugging on\n"
	  "       [ -if file1 file2 ... ] list of archive files to use as input\n"
	  "       [ -summary [n]] print summary each n records (n default 90)\n"
	  "\n"
    );


  TDRP_usage(stream);
}
