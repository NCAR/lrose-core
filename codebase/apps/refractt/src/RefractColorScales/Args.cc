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
// July 1999
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <cstring>
#include <string>
using namespace std;

// parse

int Args::parse(int argc, char **argv, string &prog_name)

{

  int iret = 0;
  output_dir = "/tmp";

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
        !strcmp(argv[i], "-h") ||
        !strcmp(argv[i], "-help") ||
        !strcmp(argv[i], "-man")) {
      
      usage(prog_name, cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-out_dir")) {

      if (i < argc - 1) {
        output_dir = argv[i+1];
      } else {
        iret = -1;
      }
      
    }

    
  } // i

  if (iret) {
    usage(prog_name, cerr);
  }

  return (iret);
    
}

void Args::usage(string &prog_name, ostream &out)
{

  out << "Usage: " << prog_name << " [options as below]\n" << endl;
  out << "Creates Fabry colorscales\n"
      << "options:\n"
      << "  [ -out_dir ? ] directory to which color scales are written\n"
      << "                 default is /tmp\n"
      << endl;

}
