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
 * Class controlling the command line arguments for this program.
 *  
 * @date 10/31/2008
 *
 */

#include <fstream>
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
    else if (STRequal_exact(argv[i], "-verbose"))
    {
      tmp_str = "verbose = true;";
      TDRP_add_override(&override, tmp_str.c_str());
    }
    else if (STRequal_exact(argv[i], "-i"))
    {
      while (argc > i+1 &&
	     argv[i+1][0] != '-')
	_inputFileList.push_back(argv[++i]);

      TDRP_add_override(&override, "trigger_mode = FILE_LIST;");
    }
    else if (STRequal_exact(argv[i], "-fl") ||
	     STRequal_exact(argv[i], "-file_list"))
    {
      string input_file = argv[++i];

      if (!_readFileListFile(input_file))
	okay = false;

      TDRP_add_override(&override, "trigger_mode = FILE_LIST;");
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

/**********************************************************************
 * _readFileListFile()
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
 * _usage()
 */

void Args::_usage(FILE *stream)
{
  fprintf(stream, "%s%s%s",
	  "This program converts TRMM HDF4 files into MDV format.\n"
	  "\n"
	  "Usage:\n\n", _progName.c_str(), " [options] as below:\n\n"
	  "       [ --, -help, -man ]\n"
	  "           produce this list.\n"
	  "       [ -debug ]\n"
	  "           turn debugging on\n"
	  "       [ -fl <file list file> ]\n"
	  "           file which contains list of files to process\n"
	  "       [ -i <input file list> ]\n"
	  "           list of netCDF sweep files to process\n"
	  "\n"
    );


  TDRP_usage(stream);
}
