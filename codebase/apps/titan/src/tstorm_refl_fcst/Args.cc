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
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1998
//
//////////////////////////////////////////////////////////

#include <cstdio>

#include <toolsa/os_config.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Args.hh"
using namespace std;

/**************************************************************
 * Constructor
 */

Args::Args(int argc, char **argv, char *prog_name) :
  okay(TRUE),
  startTime(DateTime::NEVER),
  endTime(DateTime::NEVER)
{
  char tmp_str[BUFSIZ];

  // intialize

  _progName = STRdup(prog_name);
  
  TDRP_init_override(&override);

  // loop through args
  
  for (int i =  1; i < argc; i++)
  {
    if (STRequal_exact(argv[i], "--") ||
	STRequal_exact(argv[i], "-h") ||
	STRequal_exact(argv[i], "-help") ||
	STRequal_exact(argv[i], "-man"))
    {
      _usage(stdout);
      exit(0);
    }
    else if (STRequal_exact(argv[i], "-debug"))
    {
      sprintf(tmp_str, "debug_level = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
    }
    else if (STRequal_exact(argv[i], "-endtime") ||
	     STRequal_exact(argv[i], "-end"))
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
      if (i < argc - 1)
      {
	sprintf(tmp_str, "mode = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      }
      else
      {
	okay = FALSE;
      }
    }
    else if (STRequal_exact(argv[i], "-starttime") ||
	     STRequal_exact(argv[i], "-start"))
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
    
  } // i

  if (!okay)
    _usage(stderr);
    
}


/**********************************************************************
 * Destructor
 */

Args::~Args(void)
{
  TDRP_free_override(&override);

  STRfree(_progName);
}
  

/**************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************/

/**************************************************************
 * _usage()
 */

void Args::_usage(FILE *out)
{
  fprintf(out, "%s%s%s%s",
	  "Usage: ", _progName, " [options as below]\n",
	  "options:\n"
	  "       [ --, -h, -help, -man ] produce this list.\n"
	  "       [ -debug ] print debug messages\n"
	  "       [ -end yyyy/mm/dd_hh:mm:ss ] end time (ARCHIVE mode)\n"
	  "       [ -mode ?] ARCHIVE or REALTIME\n"
	  "       [ -start yyyy/mm/dd_hh:mm:ss ] start time (ARCHIVE mode)\n"
	  "\n");
  
  TDRP_usage(out);
  
}
