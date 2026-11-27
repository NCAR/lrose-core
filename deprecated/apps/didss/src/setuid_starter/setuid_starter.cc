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
///////////////////////////////////////////////////////////////////////////
//  setuid_starter.cc
// 
//  Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// 
//  Jan 2001
// 
///////////////////////////////////////////////////////////////////////////
// 
//   SetuidStarter allows you starts scripts as another user.
//  'chmod +s' does not work on scripts, apparently for security
//  reasons. So this wrapper allows you to do it. You need to
//  'chmod +s' on  this executable to get it to start things as
//  the user and group of the file.
// 
///////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <cstring>
using namespace std;

int main(int argc, char **argv)
     
{

  fprintf(stderr, "argc: %d\n", argc);

  // check command line

  if (argc < 2 || !strcmp(argv[1], "-h")) {
    fprintf(stderr, "Usage: setuid_starter ...\n");
    fprintf(stderr,
	    "  setuid_starter allows you to start scripts as a\n"
	    "  different user. You must 'chmod +s' this executable.\n"
	    "  The args passed to setuid_starter are then executed\n"
	    "  with the user and group id of setuid_starter.\n");
    return -1;
  }

  string args;

  for (int i = 1; i < argc; i++) {
    args += argv[i];
    args += " ";
  }

  return (system(args.c_str()));

  return (0);

}

