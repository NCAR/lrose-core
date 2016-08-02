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
//   $Date: 2016/03/07 18:17:26 $
//   $Id: Args.cc,v 1.5 2016/03/07 18:17:26 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Args: class controlling the command line arguments for the
 *       program.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <cstdlib>

#include "Args.hh"

using namespace std;

/**********************************************************************
 * Constructor
 */

Args::Args (int argc, char **argv, char *prog_name) :
  _progName(prog_name)
{
  static const string method_name = "Args::Args ()";

  // Intialize

  bool okay = true;

  TDRP_init_override(&override);
  
  // search for command options
  
  string tmp_str;
  
  for (int i =  1; i < argc; i++)
  {
    if (strcmp(argv[i], "--") == 0 ||
	strcmp(argv[i], "-help") == 0 ||
	strcmp(argv[i], "-man") == 0)
    {
      _usage(stdout);
      exit(0);
    }
    else if (strcmp(argv[i], "-debug") == 0)
    {
      tmp_str = "debug = true;";
      TDRP_add_override(&override, tmp_str.c_str());
    }
    else if (strcmp(argv[i], "-i") == 0)
    {
      while (argc > i+1 &&
	     argv[i+1][0] != '-')
	_inputFileList.push_back(argv[++i]);
    }
    else if (strcmp(argv[i], "-fl") == 0)
    {
      string input_file = argv[++i];

      if (!_readFileListFile(input_file))
	okay = false;
    }
    else if (strcmp(argv[i], "-o") == 0)
    {
      _outputDir = argv[++i];
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
 * _readFileListFile() - Read the file containing the list of input
 *                       files to process.
 *
 * Returns true on success, false on failure.
 */

bool Args::_readFileListFile(const string &file_name)
{
  static const string method_name = "Args::_readFileListFile()";

  // Open the file list file

  ifstream infile(file_name.c_str());
  if (!infile.is_open())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening file list file: " << file_name << endl;

    return false;
  }

  // Read and save the input file paths

  char buffer[1024];

  while (infile.getline(buffer, sizeof(buffer)))
    _inputFileList.push_back(buffer);

  infile.close();

  return true;
}


/**********************************************************************
 * _usage() - Print the usage for this program.
 */

void Args::_usage(FILE *stream)
{
  fprintf(stream, "%s%s%s",
	  "This program reads netCDF floating point time-series files\n"
	  "produced by the NEXRAD A1DA converter program and calculates\n"
	  "the variables needed for the Frederic Fabry's refractivity\n"
	  "algorithm.\n"
	  "\n"
	  "Usage:\n\n", _progName.c_str(), " [options] as below:\n\n"
	  "       [ --, -help, -man ] produce this list.\n"
	  "       [ -debug ] print debug messages while processing\n"
	  "       [ -i <file list> ] list of input files to process\n"
	  "       [ -fl <file list file> ] name of file containing\n"
	  "                     the list of input files to process\n"
	  "                     This file should contain the fully\n"
	  "                     qualified file paths for all of the input\n"
	  "                     files to be processed, with each path on a\n"
	  "                     separate line.\n"
	  "       [ -o <dir> ] output directory\n"
	  "\n"
    );

  TDRP_usage(stream);
}
