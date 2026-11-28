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
//   $Date: 2016/03/04 02:22:15 $
//   $Id: Args.cc,v 1.4 2016/03/04 02:22:15 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Args.cc: class controlling the command line arguments for the
 *          program.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstring>
#include <cstdlib>

#include <toolsa/os_config.h>
#include <tdrp/tdrp.h>
#include <toolsa/str.h>

#include "Args.hh"
using namespace std;

/**********************************************************************
 * Constructor
 */

Args::Args (int argc, char **argv, char *prog_name) :
  okay(true)
{
  string tmp_str;

  // Intialize

  TDRP_init_override(&override);
  
  // search for command options
  
  for (int i = 1; i < argc; i++)
  {
    if (STRequal_exact(argv[i], "--") ||
        STRequal_exact(argv[i], "-h") ||
        STRequal_exact(argv[i], "-help") ||
        STRequal_exact(argv[i], "-man"))
    {
      _usage(prog_name, stderr);
      exit(0);
    }
    else if (!strcmp(argv[i], "-if"))
    {
      if (i < argc - 1)
      {
        // Search for next arg which starts with '-'

        for (int j = i + 1; j < argc; j++)
	{
          if (argv[j][0] == '-')
            break;

	  _inputFileList.push_back(argv[j]);
	}
	
      } // endif - (i < argc - 1)

    } // (!strcmp(argv[i], "-if"))
    else if (STRequal_exact(argv[i], "-mode"))
    {
      if (i < argc - 1)
      {
	tmp_str = string("run_mode = ") + argv[++i] + ";";
        TDRP_add_override(&override, tmp_str.c_str());
      }
      else
      {
	okay = false;
      }
    }
    else
    {
      // Arg is not a keyword
    }
  } // endfor i = 1

  if (!okay)
  {
    _usage(prog_name, stderr);
    exit(1);
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

void Args::_usage(const char *prog_name, FILE *stream)
{
  fprintf(stream, "%s%s%s%s",
          "Usage: ", prog_name, " [options as below]\n",
          "options:\n"
          "       [ --, -h, -help, -man ] prints help or usage\n"
          "       [ -if input_file_list (ARCHIVE mode only)]\n"
          "       [ -mode ?] ARCHIVE or REALTIME\n"
          "\n");

  TDRP_usage(stream);
}
