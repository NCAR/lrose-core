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

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Args.cc: class controlling the command line arguments for the
 *          program.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2001
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
  _progName(prog_name),
  _infile(""),
  _outfile("")
{
  string tmp_str;

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
    else if (STRequal_exact(argv[i], "-debug"))
    {
      tmp_str = "debug = true;";
      TDRP_add_override(&override, (char *)tmp_str.c_str());
    }
    else if (STRequal_exact(argv[i], "-end") ||
	     STRequal_exact(argv[i], "-endtime"))
    {
      if (i < argc - 1)
      {
	_endTime = DateTime::parseDateTime(argv[++i]);

	if (_endTime == DateTime::NEVER)
	{
	  cerr << "Error parsing end time string: " << argv[i] << endl;
	  
	  okay = false;
	}
	else
	{
	  tmp_str = "mode = ARCHIVE;";
	  TDRP_add_override(&override, (char *)tmp_str.c_str());
	}
      }
      else
      {
	okay = false;
      }
    }
    else if (STRequal_exact(argv[i], "-infile"))
    {
      if (i < argc - 1)
	_infile = argv[++i];
      else
	okay = false;
    }
    else if (STRequal_exact(argv[i], "-mode"))
    {
      if (i < argc - 1)
      {
	tmp_str = "mode = " + string(argv[++i]) + ";";
	TDRP_add_override(&override, (char *)tmp_str.c_str());
      }
      else
      {
	okay = false;
      }
    }
    else if (STRequal_exact(argv[i], "-outfile"))
    {
      if (i < argc - 1)
	_outfile = argv[++i];
      else
	okay = false;
    }
    else if (STRequal_exact(argv[i], "-start") ||
	     STRequal_exact(argv[i], "-starttime"))
    {
      if (i < argc - 1)
      {
	_startTime = DateTime::parseDateTime(argv[++i]);

	if (_startTime == DateTime::NEVER)
	{
	  cerr << "Error parsing start time string: " << argv[i] << endl;
	  
	  okay = false;
	}
	else
	{
	  tmp_str = "mode = ARCHIVE;";
	  TDRP_add_override(&override, (char *)tmp_str.c_str());
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
	  "Usage:\n\n", _progName.c_str(), " [options] as below:\n\n"
	  "       [ --, -help, -man ] produce this list.\n"
	  "       [ -debug ] debugging on\n"
	  "       [ -end endtime ] data end time (used only if ARCHIVE mode and input style is RAP_DIRECTORY)\n"
	  "       [ -infile filepath ] input file path (overrides other modes if specified)\n"
	  "       [ -mode ?] ARCHIVE or REALTIME\n"
	  "       [ -outfile filepath ] output file path (ignored if -infile not specified)\n"
	  "       [ -start starttime ] data start time (used only if ARCHIVE mode and input style is RAP_DIRECTORY)\n"
	  "\n");


  TDRP_usage(stream);
}






