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
 *********************************************************************/

#include <iostream>
#include <string>

#include <string.h>

#include <toolsa/os_config.h>
#include <tdrp/tdrp.h>
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

  start_time = DateTime::NEVER;
  end_time = DateTime::NEVER;
  string mode = "REALTIME";
  
  // Intialize

  bool okay = true;

  TDRP_init_override(&override);

  // search for command options
  
  for (int i =  1; i < argc; i++)
  {

    if (STRequal_exact(argv[i], "--") ||
	       STRequal_exact(argv[i], "-h") ||
	       STRequal_exact(argv[i], "-help") ||
	       STRequal_exact(argv[i], "--help") ||
	       STRequal_exact(argv[i], "-man"))
      {
	_usage(stdout);
	exit(0);
      }
    else if (STRequal_exact(argv[i], "-debug"))
    {
      tmp_str = "debug = true;";
      TDRP_add_override(&override, tmp_str.c_str());
    }
    else if (STRequal_exact(argv[i], "-end") ||
	     STRequal_exact(argv[i], "-endtime"))
    {
      mode = "ARCHIVE";
      if (i < argc - 1)
      {
	if (end_time.set(argv[++i]) == DateTime::NEVER)
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
    else if (STRequal_exact(argv[i], "-out_dir") ||
	     STRequal_exact(argv[i], "-o"))
    {
      if (i < argc - 1)
      {
	++i;
	tmp_str = "output_url = " + string(argv[i]) + ";";
	TDRP_add_override(&override, tmp_str.c_str());
      }
      else
      {
	okay = false;
      }
    }
     else if (STRequal_exact(argv[i], "-start") ||
	     STRequal_exact(argv[i], "-starttime"))
    {
      mode = "ARCHIVE";
      if (i < argc - 1)
      {
	if (start_time.set(argv[++i]) == DateTime::NEVER)
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
    _usage(stderr);
    exit(-1);
  }
    
  // If a mode was entered on the command line, make sure that
  // the other appropriate information was also entered.

  if (mode == "ARCHIVE")
  {
    if (start_time == DateTime::NEVER)
    {
      cerr <<
	"*** Must include -start in command line when using ARCHIVE mode" <<
	endl << endl;
      _usage(stderr);
      exit(-1);
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr <<
	"*** Must include -end in command line when using ARCHIVE mode" <<
	endl << endl;
      _usage(stderr);
      exit(-1);
    }
    
    TDRP_add_override(&override, "run_mode = ARCHIVE;");
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
  fprintf(stream, "%s%s",
	  _progName.c_str(), " runs a already created random forest"
	  " with Mdv input and output data sets\n"
#ifdef _OPENMP
	  "   Set OMP_NUM_THREADS environment variable for multithreaded runs.\n"
#endif
	  "Usage:\n"
	  "       [ --, -help, -man ] produce this list.\n"
	  "       [ -debug ] debugging on\n"
	  "       [ -end yyyy/mm/dd_hh:mm:ss ] end time\n"
	  "       [ -out_url url] output url\n"
	  "       [ -start yyyy/mm/dd_hh:mm:ss ] start time\n"
	  "\n"
    );


  TDRP_usage(stream);
}






