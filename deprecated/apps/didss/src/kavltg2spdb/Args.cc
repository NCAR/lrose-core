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
//   $Date: 2016/03/06 23:53:42 $
//   $Id: Args.cc,v 1.7 2016/03/06 23:53:42 dixon Exp $
//   $Revision: 1.7 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Args.cc: class controlling the command line arguments for the
 *          kavltg2spdb program.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 1998
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstring>
#include <cstdlib>

#include <tdrp/tdrp.h>
#include <toolsa/str.h>

#include "Args.hh"
using namespace std;

/**********************************************************************
 * Constructor
 */

Args::Args (int argc, char **argv, char *prog_name)
{
  char tmp_str[BUFSIZ];

  // Intialize

  okay = true;

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
      sprintf(tmp_str, "debug = true;");
      TDRP_add_override(&override, tmp_str);
    }
    else if (STRequal_exact(argv[i], "-dir"))
    {
      if (i < argc - 1)
      {
	sprintf(tmp_str, "input_dir = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      }
    }
    else if (STRequal_exact(argv[i], "-runmode"))
      {
	if (i < argc - 1)
	  {
	    if (STRequal_exact(argv[i+1], "REALTIME"))
	      {
		sprintf(tmp_str, "runmode = REALTIME;");
		TDRP_add_override(&override, tmp_str);
	      }
	    else if (STRequal_exact(argv[i+1], "ARCHIVE"))
	      {
		sprintf(tmp_str, "runmode = ARCHIVE;");
		TDRP_add_override(&override, tmp_str);
	      }
	    else
	      {
		_usage(prog_name, stdout);
		exit(0);
	      } /* endif STRequal_exact */
	  }
	else
	  {
	    _usage(prog_name, stdout);
	    exit(0);
	  } /* endif i < argc - 1 */

      } /* endif STRequal_exact */
    
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
	  "Usage:\n\n", prog_name, " [options] as below:\n\n"
	  "       [ --, -help, -man ] produce this list.\n"
	  "       [ -debug ] debugging on\n"
	  "       [ -dir <data_dir> ] input data directory\n"
	  "       [ -runmode <run_mode> ] program operation mode\n"
	  "            uses the following modes, REALTIME is default:\n"
	  "                   ARCHIVE  process data from file\n"
	  "                   REALTIME process data when available.\n"
	  "\n");


  TDRP_usage(stream);
}






