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
/**
 *
 * @file Args.cc
 *
 * @class Args
 *
 * Class representing the command line arguments
 *  
 * @date 9/24/2010
 *
 */

#include <iostream>
#include <string>

#include <string.h>

#include <toolsa/os_config.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Args.hh"

using namespace std;

/**********************************************************************
 * Constructor
 */

Args::Args (int argc, char **argv, char *prog_name) :
  ciddParamFileName(""),
  _progName(prog_name)
{
  // Intialize

  bool okay = true;
  
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
      debug = true;
    }
    else if (STRequal_exact(argv[i], "-params"))
    {
      ciddParamFileName = argv[++i];
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
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _usage()
 */

void Args::_usage(FILE *stream)
{
  fprintf(stream, "%s%s%s",
	  "This program converts a CIDD parameter file to a Jazz XML file.\n"
	  "Not all CIDD functionality is handled in the Jazz XML files,\n"
	  "so some functionality will be lost.  The Jazz XML file is sent\n"
	  "to stdout.  Any warnings are sent to stderr.\n"
	  "\n"
	  "Usage:\n\n", _progName.c_str(), " [options] as below:\n\n"
	  "       [ --, -help, -man ] produce this list.\n"
	  "       [ -debug ] debugging on\n"
	  "       [ -params <file> ] the original CIDD param file\n"
	  "\n"
	  "NOTE: Before using this utility, you should make the following\n"
	  "changes to your CIDD parameter file:\n"
	  "-- Change color_file_subdir and map_file_subdir to either specify\n"
	  "   the desired subdirectories relative to the ending location of\n"
	  "   the XML file or using environment variables or using explicit\n"
	  "   paths.\n"
	  "-- Rename all CIDD colorscale files to *.colors.  This is the only\n"
	  "   extension recognized by JADE.\n"
	  "-- Remove any \"blank\" fields.  These are those unavailable fields\n"
	  "   that are included in the field list to provide spacing or labels\n"
	  "   in the list in CIDD.\n"
    );
}
