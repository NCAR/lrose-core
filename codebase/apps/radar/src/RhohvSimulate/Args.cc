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
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <string.h>
#include <toolsa/umisc.h>
using namespace std;

// Constructor

Args::Args (const string &prog_name)

{
  _progName = prog_name;
  str = "test1,test2";
  spacer = ",";

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

  char tmp_str[BUFSIZ];

  // intialize

  int iret = 0;
  TDRP_init_override(&override);

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-d") ||
               !strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-v") ||
               !strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-shv")) {
      
      sprintf(tmp_str, "transmit_mode = SIMULTANEOUS;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-fhv")) {
      
      sprintf(tmp_str, "transmit_mode = ALTERNATING;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-signal")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "signal_power = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-mult")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "multiplier = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-noise")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "noise_power = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (argv[i][0] == '-') {

      cerr<< "====>> WARNING - invalid command line argument: '"
          << argv[i] << "' <<====" << endl;

    } // if
    
  } // i

  if (iret) {
    _usage(cerr);
  }

  return (iret);
    
}

void Args::_usage(ostream &out)

{

  out << endl;
  out << "This app simulates time series data and tests the computation of rhohv for various signal-to-noise ratios." << endl;
  out << endl;

  out << "Usage: " << _progName << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -d, -debug ] print debug messages\n"
      << "       [ -shv ?] set transmit mode to SIMULTANEOUS\n"
      << "       [ -fhv ?] set transmit mode to ALTERNATING\n"
      << "       [ -signal ?] set signal power in dBm\n"
      << "       [ -mult ?] set signal multiplier (linear)\n"
      << "       [ -noise ?] set noise power in dBm\n"
      << "       [ -v, -verbose ] print verbose debug messages\n"
      << endl;

  Params::usage(out);

}







