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
 * Class controlling the command line arguments for the program.
 *  
 * @date 7/19/2011
 *
 */

#include <iostream>
#include <string>

#include <cstring>

#include <toolsa/os_config.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Args.hh"

using namespace std;


/**********************************************************************
 * Constructor
 */

Args::Args (int argc, char **argv, char *prog_name) :
  _progName(prog_name),
  _fileName(""),
  _printAngles(false),
  _printData(false),
  _startTime(DateTime::NEVER),
  _endTime(DateTime::NEVER)
{
  string tmp_str;

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
    else if (STRequal_exact(argv[i], "-angles"))
    {
      _printAngles = true;
    }
    else if (STRequal_exact(argv[i], "-data"))
    {
      _printData = true;
    }
    else if (STRequal_exact(argv[i], "-start"))
    {
      if (i == argc-1 || argv[i+1][0] == '-')
      {
	cerr << "*** Must specify a time with the -start option" << endl << endl;
	okay = false;
      }
      else
      {
	_startTime.set(argv[i+1]);

	if (_startTime == DateTime::NEVER)
	{
	  cerr << "*** Invalid start time string" << endl << endl;
	  okay = false;
	}
	++i;
      }
    }
    else if (STRequal_exact(argv[i], "-end"))
    {
      if (i == argc-1 || argv[i+1][0] == '-')
      {
	cerr << "*** Must specify a time with the -end option" << endl << endl;
	okay = false;
      }
      else
      {
	_endTime.set(argv[i+1]);

	if (_endTime == DateTime::NEVER)
	{
	  cerr << "*** Invalid end time string" << endl << endl;
	  okay = false;
	}
	++i;
      }
    }
    else
    {
      _fileName = argv[i];
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
	  "This program reads a DeTect radar raw data file and prints the\n"
	  "contents of the file to stdout.\n"
	  "\n"
	  "Usage:\n\n", _progName.c_str(), " [options] <file-path>\n"
	  "Options:\n\n"
	  "       [ --, -help, -man ] produce this list.\n"
	  "       [ -angles ]         print the angle information.\n"
	  "       [ -data ]           print the data.\n"
	  "       [ -start <date> ]   the start time for the data.\n"
          "                           If not included, printing starts at the\n"
	  "                           beginning of the file.\n"
	  "       [ -end <date> ]     the end time for the data.\n"
          "                           If not included, printing continues\n"
	  "                           through the end of the file.\n"
	  "\n"
    );
}
