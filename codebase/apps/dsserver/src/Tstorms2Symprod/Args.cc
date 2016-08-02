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
// Nancy Rehak, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
//////////////////////////////////////////////////////////

#include <cstdio>
#include <iostream>
#include <cstdlib>

#include <toolsa/str.h>

#include "Args.hh"
using namespace std;

//////////////////////
// Constructor

Args::Args()
{
  TDRP_init_override(&override);
}

//////////////////////
// Destructor

Args::~Args()
{
  TDRP_free_override(&override);
}

//////////////////////
// parse command line
//
// Returns 0 on success, -1 on failue.

int Args::parse(int argc, char **argv, string &prog_name)
{

  string tmp_str;
  bool OK = true;

  // loop through args
  
  for (int i =  1; i < argc; i++)
  {
    if (STRequal_exact(argv[i], "--") ||
	STRequal_exact(argv[i], "-h") ||
	STRequal_exact(argv[i], "-help") ||
	STRequal_exact(argv[i], "-man"))
    {
      _usage(prog_name, cout);
      TDRP_usage(stdout);
      exit(0);
    }
    else if (STRequal_exact(argv[i], "-debug"))
    {
      tmp_str = "debug = DEBUG_NORM;";
      TDRP_add_override(&override, (char *)tmp_str.c_str());
    }
    else if (STRequal_exact(argv[i], "-verbose"))
    {
      tmp_str = "debug = DEBUG_VERBOSE;";
      TDRP_add_override(&override, (char *)tmp_str.c_str());
    }
    else if (STRequal_exact(argv[i], "-noThreads"))
    {
      tmp_str = "no_threads = TRUE;";
      TDRP_add_override(&override, (char *)tmp_str.c_str());
    }
    else if (STRequal_exact(argv[i], "-port"))
    {
      if (i < argc - 1)
      {
	tmp_str = "port = ";
	tmp_str += argv[++i];
	tmp_str += ";";
	TDRP_add_override(&override, (char *)tmp_str.c_str());
      }
      else
      {
	OK = false;
      }
    }
    else if (STRequal_exact(argv[i], "-qmax"))
    {
      if (i < argc - 1)
      {
	tmp_str = "qmax = ";
	tmp_str += argv[++i];
	tmp_str += ";";
	TDRP_add_override(&override, (char *)tmp_str.c_str());
      }
      else
      {
	OK = false;
      }
    }
    else if (STRequal_exact(argv[i], "-instance"))
    {
      if (i < argc - 1)
      {
	tmp_str = "instance = ";
	tmp_str += argv[++i];
	tmp_str += ";";
	TDRP_add_override(&override, (char *)tmp_str.c_str());
      }
      else
      {
	OK = false;
      }
      
    } // if
    
  } // i

  if (!OK)
  {
    _usage(prog_name, cerr);
    TDRP_usage(stderr);
    return (-1);
  }

  return (0);
}

void Args::_usage(string &prog_name, ostream &out)
{
  
  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -debug ] print debug messages.\n"
      << "       [ -instance ? ] override instance.\n"
      << "         Default is port number.\n"
      << "       [ -noThreads ] force single-threaded operation.\n"
      << "       [ -port ? ] set port number.\n"
      << "       [ -qmax ? ] set max quiescent period (secs).\n"
      << "                   Default - server never exits.\n"
      << "       [ -verbose ] print verbose debug messages.\n"
      << endl;
  
  out << "Note: " << prog_name << " listens on port 5460 by default."
      << endl << endl;

}







