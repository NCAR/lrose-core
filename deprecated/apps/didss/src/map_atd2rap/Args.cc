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
//   $Id: Args.cc,v 1.5 2016/03/06 23:53:42 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Args.cc: class controlling the command line arguments for the
 *          program.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstring>
#include <cstdlib>

#include <toolsa/os_config.h>
#include <toolsa/str.h>

#include "Args.hh"
using namespace std;

/**********************************************************************
 * Constructor
 */

Args::Args (int argc, char **argv, char *prog_name)
{
  // Intialize

  bool okay = true;

  // search for command options
  
  if (argc == 2)
  {
    // Must be requesting usage

    if (STRequal_exact(argv[1], "--") ||
	STRequal_exact(argv[1], "-help") ||
	STRequal_exact(argv[1], "-man"))
    {
      _usage(prog_name, stdout);
      exit(0);
    }
    else
    {
      okay = false;
    }
  }
  else if (argc == 3)
  {
    // Command line must contain input/output file names.
    
    if (argv[1][0] == '-' ||
	argv[2][0] == '-')
      okay = false;
    else
    {
      inputFilename = argv[1];
      outputFilename = argv[2];
    }
  }
  else
  {
    // Invalid command line.

    okay = false;
  }
  
  if (!okay)
  {
    fprintf(stdout, "Error in command line -- try again\n\n");
    _usage(prog_name, stderr);
    exit(0);
  }
    
}


/**********************************************************************
 * Destructor
 */

Args::~Args(void)
{
  // Do nothing
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _usage() - Print the usage for this program.
 */

void Args::_usage(char *prog_name, FILE *stream)
{
  fprintf(stream, "%s%s%s%s%s",
	  "Usage:\n\n",
	  prog_name, " <input ATD map filename> <output RAP map filename>\n\n"
	  "               OR\n\n",
	  prog_name, " [ --, -help, -man ] to produce this list.\n"
	  "\n");

}






