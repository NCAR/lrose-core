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
// March 2014
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

  char tmp_str[BUFSIZ];

  // intialize

  int iret = 0;

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-write_cfradial")) {
      
      sprintf(tmp_str, "write_volume_to_output_file = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-print_height_table")) {
      
      sprintf(tmp_str, "print_range_height_table = TRUE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-reverse_sweep_order")) {
      
      sprintf(tmp_str, "reverse_sweep_order_in_vol = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-max_ht")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "age_hist_max_ht_km = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      }
	
    } else if (!strcmp(argv[i], "-max_range")) {
      
      sprintf(tmp_str, "set_max_range = TRUE;");
      TDRP_add_override(&override, tmp_str);
      if (i < argc - 1) {
	sprintf(tmp_str, "max_range_km = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      }
	
    } else if (!strcmp(argv[i], "-nbins")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "n_bins_age_histogram = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      }
	
    } else if (!strcmp(argv[i], "-outdir")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "output_dir = \"%s\";", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      }
	
    } else if (!strcmp(argv[i], "-scan")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "scan_name = \"%s\";", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      }
	
    } else if (!strcmp(argv[i], "-f")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "specify_mode = SPECIFY_FILE;");
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "specified_file_path = \"%s\";", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      }
	
    } // if
    
  } // i

  if (iret) {
    _usage(cerr);
  }

  return (iret);
    
}

void Args::_usage(ostream &out)

{

  out << "Usage: " << _progName << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -f ?] path to radar file if specified\n"
      << "       [ -outdir ?] set the output directory for file\n"
      << "       [ -max_ht ?] set max ht in km\n"
      << "       [ -max_range ?] set max range in km\n"
      << "       [ -nbins ?] set number of bins in histogram\n"
      << "       [ -print_height_table ] print range-ht table to stdout\n"
      << "       [ -reverse_sweep_order ] reverse sweep order in volume but\n"
      << "          preserve times on rays. Used for debugging and testing.\n"
      << "       [ -verbose ] print verbose debug messages\n"
      << "       [ -write_cfradial ] write volume to cfradial file\n"
      << endl;

  Params::usage(out);

}


