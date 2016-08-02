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
//   $Date: 2016/03/04 02:22:09 $
//   $Id: Args.cc,v 1.6 2016/03/04 02:22:09 dixon Exp $
//   $Revision: 1.6 $
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
  _archiveStartTime(0),
  _archiveEndTime(0)
{
  const string routine_name = "Constructor";
  
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
      _usage(prog_name, stdout);
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
	if ((_archiveEndTime = _convertTimeString(argv[++i])) == 0)
	{
	  cerr << "ERROR: " << _className() << "::" << routine_name << endl;
	  cerr << "Invalid time string specified with -end option" << endl;
	
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
	cerr << "ERROR: " << _className() << "::" << routine_name << endl;
	cerr << "User must provide time string with -end option" << endl;
	
	okay = false;
      }
    }
    else if (STRequal_exact(argv[i], "-mode"))
    {
      if (i < argc - 1)
      {
	tmp_str = "mode = ";
	tmp_str += argv[i+1];
	tmp_str += ";";
	TDRP_add_override(&override, (char *)tmp_str.c_str());
      }
      else
      {
	cerr << "ERROR: " << _className() << "::" << routine_name << endl;
	cerr << "User must provide mode string with -end option" << endl;
	
	okay = false;
      }
    }
    else if (STRequal_exact(argv[i], "-start"))
    {
      if (i < argc - 1)
      {
	if ((_archiveStartTime = _convertTimeString(argv[++i])) == 0)
	{
	  cerr << "ERROR: " << _className() << "::" << routine_name << endl;
	  cerr << "Invalid time string specified with -start option" << endl;
	
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
	cerr << "ERROR: " << _className() << "::" << routine_name << endl;
	cerr << "User must provide time string with -start option" << endl;
	
	okay = false;
      }
    }
    
  } /* i */

  if (!okay)
  {
    _usage(prog_name, stderr);
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
 * _convertTimeString() - Convert a time string entered on the command
 *                        line to a UNIX time value.
 *
 * Returns the converted time value if successful, 0 if unsuccessful.
 */

time_t Args::_convertTimeString(const char *time_string)
{
  date_time_t time_struct;
  
  if (sscanf(time_string, "%d %d %d %d %d %d",
	     &time_struct.year, &time_struct.month, &time_struct.day,
	     &time_struct.hour, &time_struct.min, &time_struct.sec) != 6)
  {
    return 0;
  }
  else
  {
    uconvert_to_utime(&time_struct);
    return time_struct.unix_time;
  }

  return 0;
}


/**********************************************************************
 * _usage() - Print the usage for this program.
 */

void Args::_usage(char *prog_name, FILE *stream)
{
  fprintf(stream, "%s%s%s",
	  "Usage:\n\n", prog_name, " [options] as below:\n\n"
	  "       [ --, -help, -man ] produce this list.\n"
	  "       [ -debug ] debugging on\n"
	  "       [ -end \"yyyy mm dd hh mm ss\" ] end time\n"
	  "                             ARCHIVE mode only\n"
	  "       [ -mode mode ] operating mode\n"
	  "                           can be either ARCHIVE or REALTIME\n"
	  "       [ -start \"yyyy mm dd hh mm ss\" ] start time\n"
	  "                                 ARCHIVE mode only\n"
	  "\n");


  TDRP_usage(stream);
}






