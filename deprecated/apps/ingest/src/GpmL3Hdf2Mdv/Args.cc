//=============================================================================
//
//  (c) Copyright, 2008 University Corporation for Atmospheric Research (UCAR).
//      All rights reserved. 
//
//      File: $RCSfile: Args.cc,v $
//      Version: $Revision: 1.3 $  Dated: $Date: 2015/11/12 22:15:20 $
//
//=============================================================================

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
        STRequal_exact(argv[i], "-h") ||
        STRequal_exact(argv[i], "--help") ||
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
    else if (STRequal_exact(argv[i], "-f"))
    {
      while (argc > i+1 &&
	     argv[i+1][0] != '-')
	_inputFiles.push_back(argv[++i]);

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
 * _usage()
 */

void Args::_usage(FILE *stream)
{
  fprintf(stream, "%s%s%s",
	  "This program converts GPM IMERG3 HDF5 files into MDV format.\n"
	  "\n"
	  "Usage:\n\n", _progName.c_str(), " [options] as below:\n\n"
	  "       [ --, -help, -man ]\n"
	  "           produce this list.\n"
	  "       [ -debug ]\n"
	  "           turn debugging on\n"
	  "       [ -f list of files to process\n"
	  "\n"
    );


  TDRP_usage(stream);
}
