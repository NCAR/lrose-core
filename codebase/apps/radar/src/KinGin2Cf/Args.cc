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
// March 2012
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
      
    } else if (!strcmp(argv[i], "-disag")) {
      
      sprintf(tmp_str, "write_individual_sweeps = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-native")) {
      
      sprintf(tmp_str, "output_native_byte_order = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-cfradial")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_CFRADIAL;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-dorade")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_DORADE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-foray")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_FORAY;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-nexrad")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_NEXRAD;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-uf")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_UF;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-cf_classic")) {
      
      sprintf(tmp_str, "netcdf_style = CLASSIC;");
      TDRP_add_override(&override, tmp_str);
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_CFRADIAL;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-cf_nc64bit")) {
      
      sprintf(tmp_str, "netcdf_style = NC64BIT;");
      TDRP_add_override(&override, tmp_str);
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_CFRADIAL;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-cf_netcdf4")) {
      
      sprintf(tmp_str, "netcdf_style = NETCDF4;");
      TDRP_add_override(&override, tmp_str);
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_CFRADIAL;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-cf_netcdf4_classic")) {
      
      sprintf(tmp_str, "netcdf_style = NETCDF4_CLASSIC;");
      TDRP_add_override(&override, tmp_str);
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_CFRADIAL;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-outstart")) {
      
      sprintf(tmp_str, "compute_output_path_using_end_time = FALSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-remMiss")) {
      
      sprintf(tmp_str, "remove_rays_with_all_data_missing = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-keepLong")) {
      
      sprintf(tmp_str, "remove_long_range_rays = FALSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-remShort")) {
      
      sprintf(tmp_str, "remove_short_range_rays = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-constNgates")) {
      
      sprintf(tmp_str, "set_ngates_constant = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
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
	sprintf(tmp_str, "mode = FILELIST;");
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
	
    } else if (!strcmp(argv[i], "-lat")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "radar_latitude_deg = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "override_radar_location = TRUE;");
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-lon")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "radar_longitude_deg = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "override_radar_location = TRUE;");
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-alt")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "radar_altitude_meters = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "override_radar_location = TRUE;");
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-compress")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "output_compressed = TRUE;");
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "compression_level = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "netcdf_style = NETCDF4;");
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
	
    } else if (!strcmp(argv[i], "-elev")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "lower_elevation_limit = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "upper_elevation_limit = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "set_elevation_limits = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-elevMax")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "upper_elevation_limit = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "set_elevation_limits = true;");
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
	
    } else if (!strcmp(argv[i], "-sweepMax")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "read_upper_sweep_num = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "set_sweep_num_limits = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (argv[i][0] == '-') {

      cerr<< "====>> WARNING - invalid command line argument: '"
          << argv[i] << "' <<====" << endl;

    }
    
  } // i

  // set fields if specified

  if (fields.size() > 0) {
    
    sprintf(tmp_str, "set_output_fields = true;");
    TDRP_add_override(&override, tmp_str);
    
    string nameStr = "output_fields = { ";
    for (size_t ii = 0; ii < fields.size(); ii++) {
      string fieldStr = "\"";
      fieldStr += fields[ii];
      fieldStr += "\", ";
      nameStr += "{ ";
      nameStr += fieldStr;
      nameStr += fieldStr;
      nameStr += fieldStr;
      nameStr += fieldStr;
      nameStr += "\"\", ";
      nameStr += "OUTPUT_ENCODING_ASIS }";
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

  out << endl;
  out << "KinGin2Cf reads radar moments from the Kin-San and Gin-San radars in Japan. It converts the data into Radx and writes the data to files, normally in CfRadial format." << endl;
  out << endl;

  out << "Usage: " << _progName << " [args as below]\n"
      << "Options:\n"
      << "\n"
      << "  [ -h ] produce this list.\n"
      << "\n"
      << "  [ -ag ] aggregate sweep files into volume on read.\n"
      << "          Files with the SAME VOLUME NUMBER in the name are aggregated.\n"
      << "          Applies to CfRadial and DORADE sweep files.\n"
      << "\n"
      << "  [ -ag_all ] aggregate files in input list on read.\n"
      << "          ALL FILES in the input list are aggregated into a volume.\n"
      << "          See '-f' option.\n"
      << "          Applies to CfRadial and DORADE sweep files.\n"
      << "\n"
      << "  [ -alt ? ] override radar altitude (m)\n"
      << "\n"
      << "  [ -cfradial ] convert to cfradial (the default)\n"
      << "\n"
      << "  [ -cf_classic ] output classic-style netcdf (the default)\n"
      << "  [ -cf_netcdf4 ] output netcdf4 style\n"
      << "  [ -cf_classic4 ] output classic-style netcdf4\n"
      << "  [ -cf_nc64bit ] output 64-bit NC netcdf\n"
      << "                  The above only apply to cfradial output.\n"
      << "\n"
      << "  [ -constNgates ] force number of gates constant for all rays\n"
      << "                   Added gates will be filled with missing values\n"
      << "\n"
      << "  [ -compress ? ] compress output\n"
      << "                  specifiy compression level [1-9]\n"
      << "                  For cfradial, forces netcdf4 mode\n"
      << "\n"
      << "  [ -d, -debug ] print debug messages\n"
      << "\n"
      << "  [ -dorade ] convert to dorade\n"
      << "\n"
      << "  [ -disag ] dis-aggregate into sweep files on write\n"
      << "          optional for CfRadial files\n"
      << "          always applies to DORADE sweep files\n"
      << "\n"
      << "  [ -elev ? ] set single elevation\n"
      << "              or minimum - see '-elevMax'\n"
      << "\n"
      << "  [ -elevMax ? ] set max elevation\n"
      << "                 use '-elev' for setting minimum\n"
      << "\n"
      << "  [ -end \"yyyy mm dd hh mm ss\"] end time\n"
      << "           Sets mode to ARCHIVE\n"
      << "\n"
      << "  [ -f, -paths ? ] set file paths\n"
      << "           Sets mode to FILELIST\n"
      << "\n"
      << "  [ -field ? ] Specify particular field\n"
      << "     Specify name or number\n"
      << "     Use multiple -field args for multiple fields\n"
      << "     If not specified, all fields will be used\n"
      << "\n"
      << "  [ -foray ] convert to FORAY-1 netcdf\n"
      << "\n"
      << "  [ -indir ? ] set input directory\n"
      << "\n"
      << "  [ -keepLong ] keep long range rays\n"
      << "                Keep NEXRAD long-range non-Doppler sweeps\n"
      << "                Default is to remove them\n"
      << "\n"
      << "  [ -lat ? ] override radar latitude (deg)\n"
      << "\n"
      << "  [ -lon ? ] override radar longitude (deg)\n"
      << "\n"
      << "  [ -native ] output in host-native byte ordering\n"
      << "              instead of swapping into big-endian\n"
      << "\n"
      << "  [ -nexrad ] convert to NEXRAD archive level 2\n"
      << "\n"
      << "  [ -outdir ? ] set output directory\n"
      << "\n"
      << "  [ -outstart ? ] compute output path using start time\n"
      << "                  default is to use end time\n"
      << "\n"
      << "  [ -remMiss ] remove rays in which data at all gates and\n"
      << "               for all fields is missing\n"
      << "\n"
      << "  [ -remShort ] remove short range rays\n"
      << "               Remove NEXRAD short-range Doppler sweeps\n"
      << "\n"
      << "  [ -start \"yyyy mm dd hh mm ss\"] start time\n"
      << "           Sets mode to ARCHIVE\n"
      << "\n"
      << "  [ -sweep ? ] set single sweep number\n"
      << "               or minimum - see '-sweepMax'\n"
      << "\n"
      << "  [ -sweepMax ? ] set max sweep number\n"
      << "                  use '-sweep' for setting minimum\n"
      << "\n"
      << "  [ -uf ] convert to universal format\n"
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
