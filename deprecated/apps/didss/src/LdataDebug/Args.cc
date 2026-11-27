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
// Args.cc
//
// Command line args
//
// Nancy Rehak, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2002
//
//////////////////////////////////////////////////////////

#include <string>

#include <toolsa/umisc.h>

#include "Args.hh"
#include "Params.hh"
using namespace std;

// parse

int Args::parse(int argc, char **argv, string &prog_name)
{
  int iret = 0;
  string tmp_str;

  // intialize

  TDRP_init_override(&override);

  // loop through args
  
  for (int i =  1; i < argc; i++)
  {
    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man"))
    {
      _usage(prog_name, cout);
      exit (0);
    }
    else if (!strcmp(argv[i], "-url"))
    {
      if (i < argc - 1)
      {
	tmp_str = "input_url = \"" + string(argv[++i]) + "\";";
	TDRP_add_override(&override, tmp_str.c_str());
      }
    }
  } // i

  if (iret)
    _usage(prog_name, cerr);

  return (iret);
}

void Args::_usage(string &prog_name, ostream &out)
{

  out << "LdataDebug is used to debug problems with an ldata info\n"
      << "file.  This program just watches an ldata info file using\n"
      << "a DsLdataInfo object and prints out the new data time\n"
      << "information whenever the file changes.\n\n";

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -url ldata_url ] URL to watch.\n"
      << endl;
  
  Params::usage(out);

}







