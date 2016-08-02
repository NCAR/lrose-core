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
      
    } else if (!strcmp(argv[i], "-ag")) {
      
      sprintf(tmp_str, "aggregate_sweep_files_on_read = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-ag_all")) {
      
      sprintf(tmp_str, "aggregate_all_files_on_read = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-native")) {
      
      sprintf(tmp_str, "print_mode = PRINT_MODE_NATIVE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-dorade_format")) {
      
      sprintf(tmp_str, "print_mode = PRINT_MODE_DORADE_FORMAT;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-rays")) {
      
      sprintf(tmp_str, "print_rays = true;");
      TDRP_add_override(&override, tmp_str);

      sprintf(tmp_str, "print_ray_summary = false;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-ray_table")) {
      
      sprintf(tmp_str, "print_ray_table = true;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-summary")) {
      
      sprintf(tmp_str, "print_ray_summary = true;");
      TDRP_add_override(&override, tmp_str);
      
      sprintf(tmp_str, "print_rays = false;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-relaxed_limits")) {
      
      sprintf(tmp_str, "read_apply_strict_angle_limits = false;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-data")) {
      
      sprintf(tmp_str, "print_data = true;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-vol_fields")) {
      
      sprintf(tmp_str, "load_volume_fields_from_rays = true;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-meta_only")) {
      
      sprintf(tmp_str, "read_meta_data_only = true;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-no_trans")) {
      
      sprintf(tmp_str, "ignore_antenna_transitions = true;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-path") || !strcmp(argv[i], "-f")) {
      
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
      } else {
	OK = false;
      }
      
      if (inputFileList.size() > 0) {
	sprintf(tmp_str, "path = \"%s\";", inputFileList[0].c_str());
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      
      sprintf(tmp_str, "specify_file_by_time = false;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-mode")) {
      
      if (i < argc - 1) {
	char *modestr = argv[++i];
	if (!strcmp(modestr, "latest")) {
	  sprintf(tmp_str, "read_search_mode = READ_LATEST;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "closest")) {
	  sprintf(tmp_str, "read_search_mode = READ_CLOSEST;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "first_before")) {
	  sprintf(tmp_str, "read_search_mode = READ_FIRST_BEFORE;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "first_after")) {
	  sprintf(tmp_str, "read_search_mode = READ_FIRST_AFTER;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "rays_in_interval")) {
	  sprintf(tmp_str, "read_search_mode = READ_RAYS_IN_INTERVAL;");
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
      
    } else if (!strcmp(argv[i], "-field")) {
      
      if (i < argc - 1) {
	fields.push_back(argv[++i]);
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
      sprintf(tmp_str, "specify_file_by_time = true;");
      TDRP_add_override(&override, tmp_str);
	
    } else if (!strcmp(argv[i], "-start")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "read_start_time = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      sprintf(tmp_str, "specify_file_by_time = true;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-end")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "read_end_time = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      sprintf(tmp_str, "specify_file_by_time = true;");
      TDRP_add_override(&override, tmp_str);
	
    } else if (!strcmp(argv[i], "-dir")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "dir = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      sprintf(tmp_str, "specify_file_by_time = true;");
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
	
    } else if (!strcmp(argv[i], "-radar_num")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "read_radar_num = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "read_set_radar_num = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-preserve_sweeps")) {
      
      sprintf(tmp_str, "preserve_sweeps = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-rem_long")) {
      
      sprintf(tmp_str, "remove_long_range_rays = TRUE;");
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
      
    } else if (!strcmp(argv[i], "-apply_georefs")) {
      
      sprintf(tmp_str, "apply_georeference_corrections = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-rem_miss")) {
      
      sprintf(tmp_str, "remove_rays_with_all_data_missing = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-rem_trans")) {
      
      sprintf(tmp_str, "remove_rays_with_antenna_transitions = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-dwell_secs")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "read_dwell_secs = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }

    } else if (!strcmp(argv[i], "-dwell_stats")) {
      
      if (i < argc - 1) {
	char *statsStr = argv[++i];
	if (!strcmp(statsStr, "mean")) {
	  sprintf(tmp_str, "read_dwell_stats = DWELL_STATS_MEAN;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(statsStr, "median")) {
	  sprintf(tmp_str, "read_dwell_stats = DWELL_STATS_MEDIAN;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(statsStr, "maximum")) {
	  sprintf(tmp_str, "read_dwell_stats = DWELL_STATS_MAXIMUM;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(statsStr, "minimum")) {
	  sprintf(tmp_str, "read_dwell_stats = DWELL_STATS_MINIMUM;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(statsStr, "middle")) {
	  sprintf(tmp_str, "read_dwell_stats = DWELL_STATS_MIDDLE;");
	  TDRP_add_override(&override, tmp_str);
	} else {
	  OK = false;
	}
      } else {
	OK = false;
      }

    }
	
  } // i

  // set fields if specified

  if (fields.size() > 0) {
    
    sprintf(tmp_str, "read_set_field_names = true;");
    TDRP_add_override(&override, tmp_str);
    
    string nameStr = "read_field_names = { ";
    for (size_t ii = 0; ii < fields.size(); ii++) {
      nameStr += "\"";
      nameStr += fields[ii];
      nameStr += "\"";
      if (ii != fields.size() - 1) {
        nameStr += ", ";
      } else {
        nameStr += " ";
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
      << "  [ -ag_all ] aggregate files in input list on read.\n"
      << "          ALL FILES in the input list are aggregated into a volume.\n"
      << "          See '-f' option.\n"
      << "\n"
      << "  [ -ang ? ] set single fixed angle\n"
      << "             or minimum - see '-ang_max'\n"
      << "\n"
      << "  [ -ang_max ? ] set max fixed angle\n"
      << "                 use '-ang' for setting minimum\n"
      << "\n"
      << "  [ -apply_georefs] apply georeference corrections on read.\n"
      << "      For moving platforms, measured georeference information is\n"
      << "      somtimes available.\n"
      << "      If so, this is applied and appropriate corrections made.\n"
      << "      Earth-centric azimuth and elevation angles will be computed.\n"
      << "\n"
      << "  [ -change_lat_sign] change the sign of the radar latitude\n"
      << "    Useful for RAPIC files that always have a positive latitude.\n"
      << "\n"
      << "  [ -const_ngates ] force number of gates constant for all rays\n"
      << "                    Added gates will be filled with missing values\n"
      << "\n"
      << "  [ -data ] print field data\n"
      << "\n"
      << "  [ -d, -debug ] print debug messages\n"
      << "\n"
      << "  [ -dir ? ] Directory if searching by time\n"       
      << "\n"
      << "  [ -dorade_format ] print format of dorade structs\n"
      << "\n"
      << "  [ -dwell_secs ? ] set dwell length (secs)\n"
      << "                    Applies to rays_in_interval mode\n"
      << "\n"
      << "  [ -dwell_stats ? ] method for computing stats on the dwell\n"
      << "                     Applies to rays_in_interval mode\n"
      << "     Options: mean, median, maximum, minimum, middle\n"
      << "              Middle refers to middle ray in dwell sequence\n"
      << "\n"
      << "  [ -end \"yyyy mm dd hh mm ss.???\"] set end time\n"
      << "           Applies to rays_in_interval mode\n"
      << "           ss can be decimal secs to support sub-second precision\n"
      << "\n"
      << "  [ -f ? ] set file path, see also -path\n"
      << "\n"
      << "  [ -field ? ] Specify particular field\n"
      << "     Specify name or number\n"
      << "     Use muptiple -field args for multiple fields\n"
      << "     If not specified, all fields will be printed\n"
      << "\n"
      << "  [ -margin ? ] time_margin (secs): defaults to 3600\n"
      << "     applies to all time search modes except latest\n"
      << "\n"
      << "  [ -meta_only ] only read sweep and field meta data\n"
      << "     rays and data will not be read\n"
      << "\n"
      << "  [ -mode ? ] mode if searching by time\n"
      << "     Options: latest, closest, first_before, first_after,\n"
      << "              rays_in_interval\n"
      << "\n"
      << "  [ -native ] print in native format\n"
      << "              no translation into Radx\n"
      << "\n"
      << "  [ -no_trans ] ignore rays with antenna transition flag set\n"
      << "\n"
      << "  [ -path ? ] set file path, see also -f\n"
      << "\n"
      << "  [ -preserve_sweeps ] preserve sweep details as they are in file.\n"
      << "     This generally applies to NEXRAD data - by default we\n"
      << "     consolidate sweeps by combining split-cut sweeps\n"
      << "     into a single sweep.\n"
      << "     If this flag is true, we leave the sweeps unchanged.\n"
      << "\n"
      << "  [ -radar_num ? ] set radar number\n"
      << "    Applies to NOAA HRD data. LF radar = 1, TA radar = 2\n"
      << "    Generally not needed\n"
      << "\n"
      << "  [ -rays ] print ray meta data\n"
      << "\n"
      << "  [ -ray_table ] print table of ray properties\n"
      << "\n"
      << "  [ -relaxed_limits ] If set, turn off strict checking when using\n"
      << "         the options -ang and -ang_max, or -sweep and -sweep_max.\n"
      << "         If relaxed, and no data lies within the specified limits,\n"
      << "         then the closest applicable sweep will be read.\n"
      << "\n"
      << "  [ -rem_miss ] remove rays in which data at all gates and\n"
      << "                for all fields is missing\n"
      << "\n"
      << "  [ -rem_short ] remove short range rays\n"
      << "                 Remove NEXRAD short-range Doppler sweeps\n"
      << "\n"
      << "  [ -rem_long ] remove long range rays\n"
      << "                Remove NEXRAD long-range Non-Doppler sweeps\n"
      << "\n"
      << "  [ -rem_trans ] remove rays with antenna transitions\n"
      << "\n"
      << "  [ -start \"yyyy mm dd hh mm ss.???\"] set start time\n"
      << "           Applies to rays_in_interval mode\n"
      << "           ss can be decimal secs to support sub-second precision\n"
      << "\n"
      << "  [ -summary ] print summary of each ray\n"
      << "\n"
      << "  [ -sweep ? ] set single sweep number\n"
      << "               or minimum - see '-sweep_max'\n"
      << "\n"
      << "  [ -sweep_max ? ] set max sweep number\n"
      << "               use '-sweep' for setting minimum\n"
      << "\n"
      << "  [ -time ? ] specify search time\n"       
      << "     Format is \"YYYY MM DD HH MM SS\"\n"
      << "\n"
      << "  [ -trim_sur ] trim surveillance sweeps to 360 degrees\n"
      << "                Remove extra rays in each surveillance sweep\n"
      << "\n"
      << "  [ -v, -verbose ] print verbose debug messages\n"
      << "  [ -vv, -extra ] print extra verbose debug messages\n"
      << "\n"
      << "  [ -vol_fields ] load up volume fields from rays\n"
      << "     If not set, fields are left managed by rays\n"
      << "\n"
      << endl;
  
  out << "NOTE: You do not need to use the params option (see below).\n"
      << "      If no params are specified, you deal with the whole file.\n"
      << endl;

  Params::usage(out);
  
}
