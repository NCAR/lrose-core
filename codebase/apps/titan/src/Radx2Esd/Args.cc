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
// Sept 2017
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
  startTime = 0;
  endTime = 0;
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
	
    } else if (!strcmp(argv[i], "-start")) {
      
      if (i < argc - 1) {
	startTime = RadxTime::parseDateTime(argv[++i]);
	if (startTime == RadxTime::NEVER) {
	  OK = false;
	} else {
	  sprintf(tmp_str, "mode = ARCHIVE;");
	  TDRP_add_override(&override, tmp_str);
	}
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-end")) {
      
      if (i < argc - 1) {
	endTime = RadxTime::parseDateTime(argv[++i]);
	if (endTime == RadxTime::NEVER) {
	  OK = false;
	} else {
	  sprintf(tmp_str, "mode = ARCHIVE;");
	  TDRP_add_override(&override, tmp_str);
	}
      } else {
	OK = false;
      }
	
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
      
    } else if (!strcmp(argv[i], "-dbz_field")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "dbz_field_name = \"%s\";", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-n_gates")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "n_gates = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-start_range")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "start_range_km = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-gate_spacing")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "gate_spacing_km = %s;", argv[++i]);
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
	
    } else if (!strcmp(argv[i], "-fixed_angle")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "lower_fixed_angle_limit = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "upper_fixed_angle_limit = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "set_fixed_angle_limits = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-fixed_angle_max")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "upper_fixed_angle_limit = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "set_fixed_angle_limits = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-sweep")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "lower_sweep_num = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "upper_sweep_num = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "set_sweep_num_limits = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-sweep_max")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "read_upper_sweep_num = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "set_sweep_num_limits = true;");
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
      << "  [ -d, -debug ] print debug messages\n"
      << "\n"
      << "\n"
      << "  [ -dbz_field ? ] Specify DBZ field name\n"
      << "\n"
      << "  [ -fixed_angle ? ] set single fixed_angle\n"
      << "     or minimum - see '-fixed_ang_max'\n"
      << "\n"
      << "  [ -fixed_angle_max ? ] set max fixed_angle\n"
      << "     use '-fixed_ang' for setting minimum\n"
      << "\n"
      << "  [ -end \"yyyy mm dd hh mm ss\"] end time\n"
      << "     Sets mode to ARCHIVE\n"
      << "\n"
      << "  [ -f ? ] set file paths\n"
      << "     Sets mode to FILELIST\n"
      << "  [ -finest_geom ] remap to finest range geometry\n"
      << "\n"
      << "  [ -indir ? ] set input directory\n"
      << "\n"
      << "  [ -instance ?] specify the instance\n"
      << "\n"
      << "  [ -max_range ? ] set max range (km)\n"
      << "\n"
      << "  [ -n_gates ? ] set number of gates\n"
      << "\n"
      << "  [ -outdir ? ] set output directory\n"
      << "\n"
      << "  [ -start \"yyyy mm dd hh mm ss\"] start time\n"
      << "     Sets mode to ARCHIVE\n"
      << "\n"
      << "  [ -sweep ? ] set single sweep number\n"
      << "               or minimum - see '-sweep_max'\n"
      << "\n"
      << "  [ -sweep_max ? ] set max sweep number\n"
      << "     use '-sweep' for setting minimum\n"
      << "\n"
      << "  [ -time_offset ? ] set time offset (secs)\n"
      << "\n"
      << "  [ -v, -verbose ] print verbose debug messages\n"
      << "\n"
      << "  [ -vv, -extra ] print extra verbose debug messages\n"
      << "\n"
      << endl;
  
  out << "NOTE: You do not need to use the params option (see below).\n"
      << "      If no params are specified, you deal with the whole file.\n"
      << endl;

  Params::usage(out);
  
}
