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
//   $Date: 2016/03/06 23:53:41 $
//   $Id: Args.cc,v 1.5 2016/03/06 23:53:41 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Args.cc: class controlling the command line arguments for the
 *          program.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include <string.h>

#include <toolsa/os_config.h>
#include <toolsa/udatetime.h>
#include <tdrp/tdrp.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Args.hh"
using namespace std;

/**********************************************************************
 * Constructor
 */

Args::Args (int argc, char **argv, char *prog_name) :
  startTime(-1),
  endTime(-1),
  _progName(prog_name)
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
    else if (STRequal_exact(argv[i], "-end"))
    {
      if (i < argc - 1)
      {
	if ((endTime = _parseTime(argv[i+1], "end")) < 0)
	{
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
    else if (STRequal_exact(argv[i], "-start"))
    {
      if (i < argc - 1)
      {
	if ((startTime = _parseTime(argv[i+1], "start")) < 0)
	{
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

/**************************************************************
 * _parseTime()
 */

time_t Args::_parseTime(char *time_string, char *label)
{
  const string routine_name = "_parseTime()";
  
  date_time_t time_struct;
  
  if (sscanf(time_string, "%d %d %d %d %d %d",
	     &time_struct.year, &time_struct.month, &time_struct.day,
	     &time_struct.hour, &time_struct.min, &time_struct.sec) != 6)
  {
    cerr << "ERROR: " << _className() << "::" << routine_name << endl;
    cerr << "Incorrect format for " << label << " time." << endl;
    cerr << "Trying to decode '" << time_string << "'" << endl;
    cerr << "Format is 'yyyy mm dd hh mm ss" << endl;

    return -1;
  }
  else
  {
    uconvert_to_utime(&time_struct);
    return time_struct.unix_time;
  }
}


/**********************************************************************
 * _usage() - Print the usage for this program.
 */

void Args::_usage(FILE *stream)
{
  fprintf(stream, "%s%s%s",
	  "Usage:\n\n", _progName.c_str(), " [options] as below:\n\n"
	  "       [ --, -help, -man ] produce this list.\n"
	  "       [ -debug ] debugging on\n"
	  "       [ -end yyyy/mm/dd_hh:mm:ss ] end time (ARCHIVE mode)\n"
	  "       [ -mode ?] ARCHIVE or REALTIME\n"
	  "       [ -start yyyy/mm/dd_hh:mm:ss ] start time (ARCHIVE mode)\n"
	  "\n");


  TDRP_usage(stream);
}






