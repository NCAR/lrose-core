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
// November 1997
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <toolsa/str.h>
#include <toolsa/umisc.h>
using namespace std;

// constructor

Args::Args()

{
  TDRP_init_override(&override);
}

// destructor

Args::~Args()

{
  TDRP_free_override(&override);
}

// parse

int Args::parse(int argc, char **argv, string &prog_name)

{

  int iret = 0;
  char tmp_str[256];

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(prog_name, cout);
      exit(0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-zeros")) {
      
      sprintf(tmp_str, "set_missing_val_in_interp = FALSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-arith")) {
      
      sprintf(tmp_str, "stat_type = ARITH_MEAN;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-geom")) {
      
      sprintf(tmp_str, "stat_type = GEOM_MEAN;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-quart1")) {
      
      sprintf(tmp_str, "stat_type = FIRST_QUARTILE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-quart2")) {
      
      sprintf(tmp_str, "stat_type = SECOND_QUARTILE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-quart3")) {
      
      sprintf(tmp_str, "stat_type = THIRD_QUARTILE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-interp")) {
      
      sprintf(tmp_str, "write_interp_files = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } // if
    
  } // i

  if (iret) {
    _usage(prog_name, cerr);
  }

  return iret;

}

void Args::_usage(string &prog_name, ostream &out)
{

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -arith ] arithmetic means\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -geom ] geometric means\n"
      << "       [ -interp ] write interp tseries files\n"
      << "       [ -quart1 ] first quartile\n"
      << "       [ -quart2 ] second quartile\n"
      << "       [ -quart3 ] third quartile\n"
      << "       [ -verbose ] print verbose debug messages\n"
      << "       [ -zeros ] use 0 instead of missing value when\n"
      << "                  no storm exists at the interp time\n"
      << endl;
  
  Params::usage(out);

}






