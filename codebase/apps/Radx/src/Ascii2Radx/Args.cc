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
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2016
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <iostream>
#include <Radx/RadxTime.hh>
using namespace std;

// Constructor

Args::Args ()
{
  TDRP_init_override(&override);
}

// Destructor

Args::~Args ()

{
  TDRP_free_override(&override);
}

// parse the command line
//
// returns 0 on success, -1 on failure  

int Args::parse (int argc, char **argv, string &prog_name)

{

  _progName = prog_name;

  char tmp_str[BUFSIZ];
  bool OK = true;
  vector<string> fields;
  
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
      
    } else if (!strcmp(argv[i], "-vv") ||
               !strcmp(argv[i], "-extra")) {
      
      sprintf(tmp_str, "debug = DEBUG_EXTRA;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-instance")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "instance = \"%s\";", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      }
	
    } else if (!strcmp(argv[i], "-bufr_ascii")) {
      
      sprintf(tmp_str, "input_type = BUFR_ASCII;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-italy_ascii")) {
      
      sprintf(tmp_str, "input_type = ITALY_ASCII;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-italy_ros2")) {
      
      sprintf(tmp_str, "input_type = ITALY_ROS2;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-print_ros2")) {
      
      sprintf(tmp_str, "print_ros2_to_stdout = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-cf1")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_CFRADIAL1;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-cf2")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_CFRADIAL2;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-odim")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_ODIM_HDF5;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-to_float32")) {
      
      sprintf(tmp_str, "output_encoding = OUTPUT_ENCODING_FLOAT32;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-to_int16")) {
      
      sprintf(tmp_str, "output_encoding = OUTPUT_ENCODING_INT16;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-to_int08")) {
      
      sprintf(tmp_str, "output_encoding = OUTPUT_ENCODING_INT08;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-f")) {
      
      if (i < argc - 1) {
	// load up file list vector. Break at next arg which
	// start with -
	for (int j = i + 1; j < argc; j++) {
	  if (argv[j][0] == '-') {
	    break;
	  } else {
	    inputFileList.push_back(argv[j]);
	  }
	}
	sprintf(tmp_str, "mode = FILELIST;");
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      
    } else if (!strcmp(argv[i], "-indir")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "input_dir = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-outdir")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "output_dir = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-outname")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "output_filename = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "output_filename_mode = SPECIFY_FILE_NAME;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-suffix")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "output_filename_suffix = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-time_offset")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "time_offset_secs = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "apply_time_offset = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    }
    
  } // i

  if (!OK) {
    _usage(cerr);
    return -1;
  }

  return 0;
    
}

void Args::_usage(ostream &out)
{

  out << "Usage: " << _progName << " [args as below]\n"
      << "Options:\n"
      << "\n"
      << "  [ -h ] produce this list.\n"
      << "\n"
      << "  [ -bufr_ascii ] set input type to BUFR_ASCII\n"
      << "\n"
      << "  [ -cf1 ] convert to cfradial1 (the default)\n"
      << "  [ -cf2 ] convert to cfradial2\n"
      << "  [ -odim ] convert to ODIM HDF5\n"
      << "\n"
      << "  [ -d, -debug ] print debug messages\n"
      << "\n"
      << "  [ -f, -paths ? ] set file paths\n"
      << "     Sets mode to FILELIST\n"
      << "\n"
      << "  [ -indir ? ] set input directory\n"
      << "\n"
      << "  [ -instance ?] specify the instance\n"
      << "\n"
      << "  [ -outdir ? ] set output directory\n"
      << "\n"
      << "  [ -outname ? ] specify output file name\n"
      << "     file of this name will be written to outdir\n"
      << "\n"
      << "  [ -italy_ascii ] set input type to ITALY ASCII\n"
      << "  [ -italy_ros2 ] set input type to ITALY ROS2\n"
      << "  [ -print_ros2 ] print ITALY ROS2 data to stdout.\n"
      << "    This will be producde the ITALY ASCII format.\n"
      << "\n"
      << "  [ -suffix ? ] specify output file name suffix\n"
      << "     Suffix goes just before the extension\n"
      << "\n"
      << "  [ -time_offset ? ] set time offset (secs)\n"
      << "\n"
      << "\n"
      << "  [ -to_float32 ] convert all fields to 32-bit floats\n"
      << "  [ -to_int16 ] convert all fields to 16-bit signed integers\n"
      << "  [ -to_in08 ] convert all fields to 8-bit signed integers\n"
      << "\n"
      << "  [ -v, -verbose ] print verbose debug messages\n"
      << "\n"
      << "  [ -vv, -extra ] print extra verbose debug messages\n"
      << "\n"
      << endl;
  
  Params::usage(out);
  
}
