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
//   $Date: 2016/03/07 18:17:27 $
//   $Id: Args.cc,v 1.4 2016/03/07 18:17:27 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Args.cc: class controlling the command line arguments for the
 *          program.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2002
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

  DateTime start_time(DateTime::NEVER);
  DateTime end_time(DateTime::NEVER);
  string mode;
  string url_string = "";
  string interval_secs = "";
  
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
      TDRP_add_override(&override, tmp_str.c_str());
    }
    else if (STRequal_exact(argv[i], "-end") ||
	     STRequal_exact(argv[i], "-endtime"))
    {
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
    else if (STRequal_exact(argv[i], "-interval"))
    {
      if (i < argc - 1)
      {
	interval_secs = argv[++i];
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
	mode = argv[++i];
	
	if (STRequal_exact(argv[i], "LATEST_DATA") ||
	    STRequal_exact(argv[i], "TIME_LIST") ||
	    STRequal_exact(argv[i], "INTERVAL_REALTIME") ||
	    STRequal_exact(argv[i], "INTERVAL_ARCHIVE"))
	{
	  tmp_str = "trigger_mode = " + mode + ";";
	  TDRP_add_override(&override, tmp_str.c_str());
	}
	else
	{
	  cerr << "*** Invalid mode entered: " << mode << endl << endl;
	  okay = false;
	}
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
    else if (STRequal_exact(argv[i], "-url"))
    {
      if (i < argc - 1)
      {
	url_string = argv[++i];
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

  if (mode == "LATEST_DATA")
  {
    if (url_string == "")
    {
      cerr <<
	"*** Must include -url in command line when using LATEST_DATA mode" <<
	endl << endl;
      _usage(stderr);
      exit(-1);
    }

    tmp_str = "latest_data_trigger = \"" + url_string + "\";";
    TDRP_add_override(&override, tmp_str.c_str());
  }
  else if (mode == "TIME_LIST")
  {
    if (url_string == "")
    {
      cerr <<
	"*** Must include -url in command line when using LATEST_DATA mode" <<
	endl << endl;
      _usage(stderr);
      exit(-1);
    }

    if (start_time == DateTime::NEVER)
    {
      cerr <<
	"*** Must include -start in command line when using TIME_LIST mode" <<
	endl << endl;
      _usage(stderr);
      exit(-1);
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr <<
	"*** Must include -end in command line when using TIME_LIST mode" <<
	endl << endl;
      _usage(stderr);
      exit(-1);
    }
    
    tmp_str = "time_list_trigger = { \"" +
      url_string + "\", \"" +
      string(start_time.dtime()) + "\", \"" + end_time.dtime() + "\" };";
    TDRP_add_override(&override, tmp_str.c_str());
  }
  else if (mode == "INTERVAL_REALTIME")
  {
    if (interval_secs == "")
    {
      cerr <<
	"*** Must include -interval in command line when using INTERVAL_REALTIME mode" <<
	endl << endl;
      _usage(stderr);
      exit(-1);
    }

    tmp_str = "interval_realtime = " + interval_secs + ";";
    TDRP_add_override(&override, tmp_str.c_str());
  }
  else if (mode == "INTERVAL_ARCHIVE")
  {
    if (interval_secs == "")
    {
      cerr <<
	"*** Must include -interval_secs in command line when using INTERVAL_ARCHIVE mode" <<
	endl << endl;
      _usage(stderr);
      exit(-1);
    }

    if (start_time == DateTime::NEVER)
    {
      cerr <<
	"*** Must include -start in command line when using INTERVAL_ARCHIVE mode" <<
	endl << endl;
      _usage(stderr);
      exit(-1);
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr <<
	"*** Must include -end in command line when using INTERVAL_ARCHIVE mode" <<
	endl << endl;
      _usage(stderr);
      exit(-1);
    }
    
    tmp_str = "interval_archive = { \"" +
      interval_secs + "\", \"" +
      string(start_time.dtime()) + "\", \"" + end_time.dtime() + "\" };";
    TDRP_add_override(&override, tmp_str.c_str());
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
	  "This program calculates the weighted average of each grid\n"
	  "position in a list of fields.\n"
	  "\n"
	  "Usage:\n\n", _progName.c_str(), " [options] as below:\n\n"
	  "       [ --, -help, -man ] produce this list.\n"
	  "       [ -debug ] debugging on\n"
	  "       [ -end yyyy/mm/dd_hh:mm:ss ] end time (TIME_LIST or INTERVAL_ARCHIVE mode)\n"
	  "       [ -interval ? ]   trigger interval in seconds (INTERVAL_REALTIME or INTERVAL_ARCHIVE mode)\n"
	  "       [ -mode ?] LATEST_DATA, TIME_LIST, INTERVAL_REALTIME or INTERVAL_ARCHIVE\n"
	  "       [ -start yyyy/mm/dd_hh:mm:ss ] start time (TIME_LIST or INTERVAL_ARCHIVE mode)\n"
	  "       [ -url url ] trigger url\n"
	  "\n"
	  "When specifying a mode on the command line, the following must also\n"
	  "be specified:\n"
	  "\n"
	  "     LATEST_DATA mode:\n"
	  "           -url\n"
	  "     TIME_LIST mode:\n"
	  "           -url, -start, -end\n"
	  "     INTERVAL_REALTIME mode:\n"
	  "           -interval\n"
	  "     INTERVAL_ARCHIVE mode:\n"
	  "           -interval, -start, -end\n"
	  "\n"
    );


  TDRP_usage(stream);
}
