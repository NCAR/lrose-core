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
// Jan 2010
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <vector>
#include <iostream>
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

int Args::parse(int argc, char **argv, string &prog_name)

{

  _progName = prog_name;

  char tmp_str[BUFSIZ];
  bool OK = true;
  vector<string> fields1;
  vector<string> fields2;
  
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
      
    } else if (!strcmp(argv[i], "-ag")) {
      
      sprintf(tmp_str, "aggregate_sweep_files_on_read = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-path1") || !strcmp(argv[i], "-f1")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "file1_path = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      
      sprintf(tmp_str, "search_mode = SEARCH_BY_PATH;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-path2") || !strcmp(argv[i], "-f2")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "file2_path = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      
      sprintf(tmp_str, "search_mode = SEARCH_BY_PATH;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-out_path")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "output_file_path = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      
    } else if (!strcmp(argv[i], "-out_append")) {
      
      sprintf(tmp_str, "append_to_output = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-time_mode")) {
      
      if (i < argc - 1) {
	char *modestr = argv[++i];
	if (!strcmp(modestr, "latest")) {
	  sprintf(tmp_str, "time_mode = READ_LATEST;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "closest")) {
	  sprintf(tmp_str, "time_mode = READ_CLOSEST;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "first_before")) {
	  sprintf(tmp_str, "time_mode = READ_FIRST_BEFORE;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "first_after")) {
	  sprintf(tmp_str, "time_mode = READ_FIRST_AFTER;");
	  TDRP_add_override(&override, tmp_str);
	} else {
	  OK = false;
	}
      } else {
	OK = false;
      }

    } else if (!strcmp(argv[i], "-margin")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "read_search_margin = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      
    } else if (!strcmp(argv[i], "-field1")) {
      
      if (i < argc - 1) {
	fields1.push_back(argv[++i]);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-field2")) {
      
      if (i < argc - 1) {
	fields2.push_back(argv[++i]);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-time")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "read_search_time = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      sprintf(tmp_str, "search_mode = SEARCH_BY_TIME;");
      TDRP_add_override(&override, tmp_str);
	
    } else if (!strcmp(argv[i], "-dir1")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "file1_dir = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      sprintf(tmp_str, "search_mode = SEARCH_BY_TIME;");
      TDRP_add_override(&override, tmp_str);
	
    } else if (!strcmp(argv[i], "-dir2")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "file2_dir = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      sprintf(tmp_str, "search_mode = SEARCH_BY_TIME;");
      TDRP_add_override(&override, tmp_str);
	
    } else if (!strcmp(argv[i], "-ang")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "read_lower_fixed_angle = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "read_upper_fixed_angle = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "read_set_fixed_angle_limits = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-ang_max")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "read_upper_fixed_angle = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "read_set_fixed_angle_limits = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-sweep")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "read_lower_sweep_num = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "read_upper_sweep_num = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "read_set_sweep_num_limits = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-sweep_max")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "read_upper_sweep_num = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "read_set_sweep_num_limits = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-no_sweeps")) {
      
      sprintf(tmp_str, "check_sweeps = FALSE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-no_sweep_numbers")) {
      
      sprintf(tmp_str, "check_sweep_numbers = FALSE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-no_rays")) {
      
      sprintf(tmp_str, "check_rays = FALSE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-no_fields")) {
      
      sprintf(tmp_str, "check_fields = FALSE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-no_field_names")) {
      
      sprintf(tmp_str, "check_field_names = FALSE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-no_field_units")) {
      
      sprintf(tmp_str, "check_field_units = FALSE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-no_field_number")) {
      
      sprintf(tmp_str, "check_number_of_fields = FALSE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-no_field_data")) {
      
      sprintf(tmp_str, "check_field_data = FALSE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-match_geom")) {
      
      sprintf(tmp_str, "match_range_geometry = TRUE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-max_range")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "max_range_km = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "set_max_range = TRUE;");
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-max_diff")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "field_data_max_diff_for_match = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-max_angle_diff")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "angle_max_diff_for_match = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-max_nyquist_diff")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "nyquist_max_diff_for_match = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-max_time_diff")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "time_max_diff_for_match = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-report_all")) {
      
      sprintf(tmp_str, "report_all_field_data_diffs = TRUE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-min_percent")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "min_percent_error_for_summary_report = %s;",
                argv[i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-vol_num")) {
      
      sprintf(tmp_str, "check_volume_number = TRUE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-keep_long")) {
      
      sprintf(tmp_str, "keep_long_range_rays = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-rem_short")) {
      
      sprintf(tmp_str, "remove_short_range_rays = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-const_ngates")) {
      
      sprintf(tmp_str, "set_ngates_constant = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-trim_sur")) {
      
      sprintf(tmp_str, "trim_surveillance_sweeps_to_360deg = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-change_lat_sign")) {
      
      sprintf(tmp_str, "change_radar_latitude_sign = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    }
      
  } // i

  // set fields if specified

  if (fields1.size() > 0) {

    if (fields1.size() != fields2.size()) {
      cerr << "ERROR - Args::parse()" << endl;
      cerr << "  If you use -fields1 and -fields2, you must specify" << endl;
      cerr << "  the same number of fields for each" << endl;
      return -1;
    }
    
    sprintf(tmp_str, "specify_field_names = true;");
    TDRP_add_override(&override, tmp_str);
    
    string nameStr = "field_names = { ";
    for (size_t ii = 0; ii < fields1.size(); ii++) {
      nameStr += " { \"";
      nameStr += fields1[ii];
      nameStr += "\", ";
      nameStr += " \"";
      nameStr += fields2[ii];
      nameStr += "\" ";
      if (ii != fields1.size() - 1) {
        nameStr += " }, ";
      } else {
        nameStr += " } ";
      }
    }
    nameStr += "};";
    TDRP_add_override(&override, nameStr.c_str());
    
  } // if (fields.size() ...

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
      << "  [ -ag ] aggregate sweep files into volume on read\n"
      << "          applies to DORADE and CFRADIAL sweep files\n"
      << "\n"
      << "  [ -ang ? ] set single fixed angle\n"
      << "             or minimum - see '-ang_max'\n"
      << "\n"
      << "  [ -ang_max ? ] set max fixed angle\n"
      << "                 use '-ang' for setting minimum\n"
      << "\n"
      << "  [ -change_lat_sign] change the sign of the radar latitude\n"
      << "          Used for RAPIC files that always have a positive latitude.\n"
      << "\n"
      << "  [ -const_ngates ] force number of gates constant for all rays\n"
      << "                    Added gates will be filled with missing values\n"
      << "\n"
      << "  [ -d, -debug ] print debug messages\n"
      << "\n"
      << "  [ -dir1 ? ] Directory for file1 if searching by time\n"       
      << "  [ -dir2 ? ] Directory for file2 if searching by time\n"       
      << "\n"
      << "  [ -f1 ? ] set file1 path\n"
      << "  [ -f2 ? ] set file2 path\n"
      << "\n"
      << "  [ -field1 ? ] Add field for file1\n"
      << "  [ -field2 ? ] Add field for file2\n"
      << "     Use multiple -field args for multiple fields\n"
      << "     You must specify the same number of fields for 1 and 2\n"
      << "     If not specified, all fields will be compared\n"
      << "\n"
      << "  [ -keep_long ] keep long range rays\n"
      << "                 Keep NEXRAD long-range non-Doppler sweeps\n"
      << "                 Default is to remove them\n"
      << "\n"
      << "  [ -match_geom ] match the range geometry\n"
      << "     The geom of the second file is matched to that in the first\n"
      << "\n"
      << "  [ -max_diff ? ] max abs value diff for field data match\n"       
      << "\n"
      << "  [ -max_angle_diff ? ] max abs angle diff (deg)\n"       
      << "\n"
      << "  [ -max_nyquist_diff ? ] max abs nyquist diff (m/s)\n"       
      << "\n"
      << "  [ -max_time_diff ? ] max abs time diff (secs)\n"       
      << "\n"
      << "  [ -max_range ? ] set max range (km)\n"
      << "\n"
      << "  [ -margin ? ] time_margin (secs): defaults to 3600\n"
      << "     applies to all time search modes except latest\n"
      << "\n"
      << "  [ -min_percent ? ] percent of bad points to trigger report\n"       
      << "     If the percentage of bad points is less than this,\n"
      << "     no bad data report will be produced\n"
      << "\n"
      << "  [ -no_fields ] do not check field details\n"
      << "\n"
      << "  [ -no_field_names ] do not check field names\n"
      << "     assumes fields are in correct order\n"
      << "\n"
      << "  [ -no_field_number ] do not check number of fields\n"
      << "\n"
      << "  [ -no_field_units ] do not check field units\n"
      << "\n"
      << "  [ -no_field_data ] do not check field data values\n"
      << "\n"
      << "  [ -no_rays ] do not check ray details\n"
      << "\n"
      << "  [ -no_sweeps ] do not check sweep details\n"
      << "\n"
      << "  [ -no_sweep_numbers ] do not check sweep numbers\n"
      << "\n"
      << "  [ -out_append ] append to output file\n"
      << "    Default is to write a fresh file\n"
      << "\n"
      << "  [ -out_path ? ] set path for output file\n"
      << "    Can optionally be set to 'stdout' or 'stderr'\n"
      << "\n"
      << "  [ -path1 ? ] set file1 path, see also -f1\n"
      << "  [ -path2 ? ] set file1 path, see also -f2\n"
      << "\n"
      << "  [ -rem_short ] remove short range rays\n"
      << "                 Remove NEXRAD short-range Doppler sweeps\n"
      << "\n"
      << "  [ -report_all ] report all gates with errors\n"
      << "\n"
      << "  [ -sweep ? ] set single sweep number\n"
      << "               or minimum - see '-sweep_max'\n"
      << "\n"
      << "  [ -sweep_max ? ] set max sweep number\n"
      << "                   use '-sweep' for setting minimum\n"
      << "\n"
      << "  [ -time ? ] specify search time\n"       
      << "     Format is \"YYYY MM DD HH MM SS\"\n"
      << "\n"
      << "  [ -time_mode ? ] mode if searching by time\n"
      << "     Options: latest, closest, first_before, first_after\n"
      << "\n"
      << "  [ -trim_sur ] trim surveillance sweeps to 360 degrees\n"
      << "                Remove extra rays in each surveillance sweep\n"
      << "\n"
      << "  [ -vol_num ] check the volume number\n"
      << "\n"
      << "  [ -v, -verbose ] print verbose debug messages\n"
      << "  [ -vv, -extra ] print extra verbose debug messages\n"
      << "\n"
      << endl;
  
  Params::usage(out);
  
}
