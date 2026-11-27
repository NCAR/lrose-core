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
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 1999
//
//////////////////////////////////////////////////////////

#include "Params.hh"
#include "Args.hh"
#include <string.h>
#include <iostream>
using namespace std;

// Constructor

Args::Args (const string &prog_name)

{
  _progName = prog_name;
  TDRP_init_override(&override);
}


// Destructor

Args::~Args ()

{
  TDRP_free_override(&override);
}

// Parse command line
// Returns 0 on success, -1 on failure

int Args::parse (const int argc, const char **argv)

{

  int iret = 0;
  char tmp_str[BUFSIZ];
  bool got_start = false;
  bool got_end = false;


  // loop through args
  
  for (int i =  1; i < argc; i++) 
    {

      if (!strcmp(argv[i], "--") ||
	  !strcmp(argv[i], "-h") ||
	  !strcmp(argv[i], "-help") ||
	  !strcmp(argv[i], "-man")) 
	{  
	  _usage(cout);
	  exit (0); 
	} 
      else if (!strcmp(argv[i], "-debug")) 
	{
	  sprintf(tmp_str, "debug = DEBUG_ON;");
	  TDRP_add_override(&override, tmp_str); 
	} 
      else if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "-instance")) 
	{
	  if (i < argc - 1) 
	    {
	      sprintf(tmp_str, "instance = %s;", argv[++i]);
	      TDRP_add_override(&override, tmp_str);
	    } 
	} 
      else if (!strcmp(argv[i], "-start"))
	{
	  if (i < argc - 1)
	    {
	      got_start = true;
	      sprintf(tmp_str, "start_time = %s;", argv[++i]);
	      TDRP_add_override(&override, tmp_str);
	    }
	  else
	    {
	      cerr << "Need start time\n";
	      iret = -1;
	    }
	}
      else if (!strcmp(argv[i], "-end"))
	{
	  if (i < argc -1 )
	    {
	      got_end = true;
	      sprintf(tmp_str, "end_time = %s;", argv[++i]);
	      TDRP_add_override(&override, tmp_str);
	    }
	  else
	    {
	      cerr << "Need end time\n";
	      iret = -1;
	    }
	}
      
    } // for i
  
  

  //
  // If start and end times specified, go to ARCHIVE mode
  //
  if ( (got_start && !got_end) || (!got_start && got_end))
    {
      cerr << "Need both start and end times for ARCHIVE mode\n";

      iret = -1;
    }

  if ((got_start) && (got_end))
    {
      TDRP_add_override(&override, "mode = ARCHIVE;");
    }

  if (iret) {
    _usage(cerr);
  }
  
  return iret;
  
}

void Args::_usage(ostream &out)
{

  out << endl;
  out << "Program : " << _progName << endl;
  out << "  Converts Bdry spdb objects to xml." << endl;
  out << endl;

  out << "Usage: " << _progName << " [args as below]\n"
      << "options:\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -h ] produce this list.\n"
      << "       [ -end \"yyyymmddhhmmss\"] end time\n"
      << "          ARCHIVE mode only\n"
      << "       [ -start \"yyyymmddhhmmss\"] start time\n"
      << "          ARCHIVE mode only\n"
      << "       [ -i, -instance ] instance.\n"
      << endl;

  Params::usage(out);
  
}



