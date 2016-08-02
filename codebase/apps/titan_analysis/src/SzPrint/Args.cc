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
// July 2004
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <cstring>
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
      
      usage(prog_name, cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-adj")) {
      
      sprintf(tmp_str, "use_adjacent_azimuths = true;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-miss")) {
      
      sprintf(tmp_str, "include_missing = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-censor_rph")) {
      
      sprintf(tmp_str, "censor_rph = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-censor_msz")) {
      
      sprintf(tmp_str, "censor_msz = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-long")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "long_mdv_path = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-ncar_long_pp")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "ncar_long_pp_mdv_path = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-ncar_short_pp")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "ncar_short_pp_mdv_path = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-ncar_short_fft")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "ncar_short_fft_mdv_path = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-msz")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "msz_mdv_path = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-rph")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "rph_mdv_path = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-trip")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "trip = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } // if
    
  } // i

  if (iret) {
    usage(prog_name, cerr);
  }

  return (iret);
    
}

void Args::usage(string &prog_name, ostream &out)
{

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "  [ --, -h, -help, -man ] produce this list.\n"
      << "  [ -censor_msz] apply sellis censoring to msz data\n"
      << "  [ -censor_rph] apply sellis censoring to rph data\n"
      << "  [ -debug ] print debug messages\n"
      << "  [ -long ?] set Long PRT file path\n"
      << "  [ -miss] include missing vals in output, as 'NaN'\n"
      << "  [ -msz ?] set Mike's SZ file path\n"
      << "  [ -ncar_long_pp ?] NCAR Long PRT PP file path\n"
      << "  [ -ncar_short_pp ?] NCAR short PRT PP file path\n"
      << "  [ -ncar_short_fft ?] NCAR short PRT FFT file path\n"
      << "  [ -rph ?] set RPH SZ file path\n"
      << "  [ -trip ?] set trip type, default STRONG_TRIP\n"
      << "    STRONG_TRIP, WEAK_TRIP, FIRST_TRIP, SECOND_TRIP\n"
      << "  [ -verbose ] print verbose debug messages\n"
      << endl;

  Params::usage(out);

}
