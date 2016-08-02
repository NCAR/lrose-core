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
//   $Date: 2016/03/04 02:22:11 $
//   $Id: Args.cc,v 1.3 2016/03/04 02:22:11 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Args.cc: class controlling the command line arguments for the
 *          program.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2004
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include <string.h>

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

  string trigger_mode = "";
  string interval_secs = "";
  string interval_start_secs = "";
  DateTime start_time = DateTime::NEVER;
  DateTime end_time = DateTime::NEVER;
  
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
	     STRequal_exact(argv[i], "-endtime") ||
	     STRequal_exact(argv[i], "-end_time"))
    {
      if (i < argc - 1)
      {
	++i;
	end_time.set(argv[i]);
	if (end_time == DateTime::NEVER)
	{
	  cerr << "Error parsing end time string: " << argv[i] << endl;
	  
	  okay = false;
	}
      }
      else
      {
	okay = false;
      }
    }
    else if (STRequal_exact(argv[i], "-interval_secs") ||
	     STRequal_exact(argv[i], "-int"))
    {
      if (i < argc - 1)
      {
	++i;
	interval_secs = argv[i];
      }
      else
      {
	okay = false;
      }
    }
    else if (STRequal_exact(argv[i], "-interval_start_secs") ||
	     STRequal_exact(argv[i], "-int_start"))
    {
      if (i < argc - 1)
      {
	++i;
	interval_start_secs = argv[i];
      }
      else
      {
	okay = false;
      }
    }
    else if (STRequal_exact(argv[i], "-mode"))
    {
      if (i < argc - 1)
      {
	++i;
	trigger_mode = argv[i];
	tmp_str = "trigger_mode = " + trigger_mode + ";";
	TDRP_add_override(&override, tmp_str.c_str());
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
	tmp_str = "output_dir = " + string(argv[i]) + ";";
	TDRP_add_override(&override, tmp_str.c_str());
      }
      else
      {
	okay = false;
      }
    }
    else if (STRequal_exact(argv[i], "-start") ||
	     STRequal_exact(argv[i], "-starttime") ||
	     STRequal_exact(argv[i], "-start_time"))
    {
      if (i < argc - 1)
      {
	++i;
	start_time.set(argv[i]);
	if (start_time == DateTime::NEVER)
	{
	  cerr << "Error parsing start time string: " << argv[i] << endl;
	  
	  okay = false;
	}
      }
      else
      {
	okay = false;
      }
    }
  } /* endfor - i */

  if (trigger_mode == "INTERVAL_REALTIME")
  {
    if (interval_secs == "")
    {
      cerr << "Must specify -interval_secs when setting -mode to INTERVAL_REALTIME" << endl;
      
      okay = false;
    }
    else if (interval_start_secs == "")
    {
      cerr << "Must specify -interval_start_secs when setting -mode to INTERVAL_REALTIME" << endl;
      
      okay = false;
    }
    else
    {
      // Everything is okay, set realtime params
      tmp_str = "interval_realtime_trigger = { " +
	interval_secs +	", " +
	interval_start_secs + " };";
      TDRP_add_override(&override, tmp_str.c_str());
    }
  }
  else if (trigger_mode == "INTERVAL_ARCHIVE")

  {
    if (interval_secs == "")
    {
      cerr << "Must specify -interval_secs when setting -mode to INTERVAL_ARCHIVE" << endl;
      
      okay = false;
    }
    else if (start_time == DateTime::NEVER)
    {
      cerr << "Must specify -start when setting -mode to INTERVAL_ARCHIVE" << endl;
      
      okay = false;
    }
    else if (end_time == DateTime::NEVER)
    {
      cerr << "Must specify -end when setting -mode to INTERVAL_ARCHIVE" << endl;
      
      okay = false;
    }
    else
    {
      // Everything is okay, set realtime params
      tmp_str = "interval_archive_trigger = { \"" +
	string(start_time.dtime()) + "\", \"" +
	end_time.dtime() + "\", " +
	interval_secs +	" };";
      TDRP_add_override(&override, tmp_str.c_str());
    }
  }
  
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
	  "Program for interpolating point data (data that can be\n"
	  "represented as lat,lon,value) into a 2D MDV grid.\n"
	  "Create a parameter file to see what types of point data\n"
	  "are currently supported.\n"
	  "\n"
	  "Usage:\n\n", _progName.c_str(), " [options] as below:\n\n"
	  "       [ --, -help, -man ] produce this list.\n"
	  "       [ -debug ] debugging on.\n"
	  "       [ -end <time> ] data end time.\n"
	  "                   Used only if mode is set to INTERVAL_ARCHIVE.\n"
	  "       [ -interval_secs <secs> ] trigger interval in seconds.\n"
	  "       [ -interval_start_secs <secs> ] number of seconds after\n"
	  "                   the hour for starting the intervals.\n"
	  "                   Used only if mode is set to INTERVAL_REALTIME.\n"
          "       [ -mode <mode> ] operational mode: INTERVAL_REALTIME or\n"
	  "                             or INTERVAL_ARCHIVE.\n"
	  "       [ -out_dir dir] output directory.\n"
	  "       [ -start <time> ] data start time.\n"
	  "                   Used only if mode is set to INTERVAL_ARCHIVE.\n"
	  "\n"
	  "If -mode is set, you must also use the following args:\n"
	  "    INTERVAL_REALTIME:\n"
	  "         -interval_secs\n"
	  "         -interval_start_secs\n"
	  "    INTERVAL_ARCHIVE:\n"
	  "         -interval_secs\n"
	  "         -start\n"
	  "         -end\n"
    );


  TDRP_usage(stream);
}






