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
// Jan 1999
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
  vector<string> waypts;
  
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
      
    } else if (!strcmp(argv[i], "-comp")) {
      
      sprintf(tmp_str, "read_composite = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-data")) {
      
      sprintf(tmp_str, "print_data = true;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-dvi")) {
      
      sprintf(tmp_str, "disable_vsection_interp = true;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-summary")) {
      
      sprintf(tmp_str, "print_summary = true;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-single_buf")) {
      
      sprintf(tmp_str, "read_as_single_part = true;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-float")) {
      
      sprintf(tmp_str, "print_native = false;");
      TDRP_add_override(&override, tmp_str);
      sprintf(tmp_str, "read_encoding_type = ENCODING_FLOAT32;");
      TDRP_add_override(&override, tmp_str);
     
    } else if (!strcmp(argv[i], "-path") || !strcmp(argv[i], "-f")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "path = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      
      sprintf(tmp_str, "specify_file_by_time = false;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-32_bit_hdrs")) {
      
      sprintf(tmp_str, "read_using_32_bit_headers = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-all_hdrs")) {
      
      sprintf(tmp_str, "get_mode = GET_ALL_HEADERS;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-volume")) {
      
      sprintf(tmp_str, "get_mode = GET_VOLUME;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-vsection")) {
      
      sprintf(tmp_str, "get_mode = GET_VSECTION;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-gis")) {
      
      sprintf(tmp_str, "get_mode = GET_GIS;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-linear")) {
      
      sprintf(tmp_str, "read_transform_to_linear = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-printCanonical")) {
      
      sprintf(tmp_str, "print_canonical = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-printChunks")) {
      
      sprintf(tmp_str, "print_chunks = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-bot")) {
      
      sprintf(tmp_str, "start_at_top = FALSE;");
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
	} else if (!strcmp(modestr, "best_forecast")) {
	  sprintf(tmp_str, "read_search_mode = READ_BEST_FORECAST;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "specified_forecast")) {
	  sprintf(tmp_str, "read_search_mode = READ_SPECIFIED_FORECAST;");
	  TDRP_add_override(&override, tmp_str);
	} else {
	  OK = false;
	}
      } else {
	OK = false;
      }

    } else if (!strcmp(argv[i], "-encoding")) {
      
      if (i < argc - 1) {
	char *modestr = argv[++i];
	if (!strcmp(modestr, "asis")) {
	  sprintf(tmp_str, "read_encoding_type = ENCODING_ASIS;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "int8")) {
	  sprintf(tmp_str, "read_encoding_type = ENCODING_INT8;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "int16")) {
	  sprintf(tmp_str, "read_encoding_type = ENCODING_INT16;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "float32")) {
	  sprintf(tmp_str, "read_encoding_type = ENCODING_FLOAT32;");
	  TDRP_add_override(&override, tmp_str);
	} else {
	  OK = false;
	}
      } else {
	OK = false;
      }

    } else if (!strcmp(argv[i], "-nlines_data")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "print_nlines_data = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
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
      
    } else if (!strcmp(argv[i], "-lead")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "read_forecast_lead_time = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      
    } else if (!strcmp(argv[i], "-decimate")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "read_set_decimation = true;");
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "decimation_max_nxy = %s;", argv[++i]);
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
	
    } else if (!strcmp(argv[i], "-format")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "file_format = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      
    } else if (!strcmp(argv[i], "-waypt")) {
      
      if (i < argc - 1) {
	waypts.push_back(argv[++i]);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-threaded")) {
      
      sprintf(tmp_str, "threaded = true;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-time")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "read_search_time = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      sprintf(tmp_str, "specify_file_by_time = true;");
      TDRP_add_override(&override, tmp_str);
	
    } else if (!strcmp(argv[i], "-latest_mod_time")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "latest_valid_mod_time = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "set_latest_valid_mod_time = true;");
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-timelist")) {
      
      sprintf(tmp_str, "get_mode = GET_TIME_LIST;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-timeht")) {
      
      sprintf(tmp_str, "get_mode = GET_TIME_HEIGHT;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-tlist_mode")) {
      
      if (i < argc - 1) {
	char *modestr = argv[++i];
	if (!strcmp(modestr, "valid")) {
	  sprintf(tmp_str, "time_list_mode = TIME_LIST_VALID;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "gen")) {
	  sprintf(tmp_str, "time_list_mode = TIME_LIST_GEN;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "forecast")) {
	  sprintf(tmp_str, "time_list_mode = TIME_LIST_FORECAST;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "gen_plus_forecasts")) {
	  sprintf(tmp_str, "time_list_mode = TIME_LIST_GEN_PLUS_FORECASTS;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "valid_mult_gen")) {
	  sprintf(tmp_str, "time_list_mode = TIME_LIST_VALID_MULT_GEN;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "first")) {
	  sprintf(tmp_str, "time_list_mode = TIME_LIST_FIRST;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "last")) {
	  sprintf(tmp_str, "time_list_mode = TIME_LIST_LAST;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "closest")) {
	  sprintf(tmp_str, "time_list_mode = TIME_LIST_CLOSEST;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "first_before")) {
	  sprintf(tmp_str, "time_list_mode = TIME_LIST_FIRST_BEFORE;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "first_after")) {
	  sprintf(tmp_str, "time_list_mode = TIME_LIST_FIRST_AFTER;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "best_forecast")) {
	  sprintf(tmp_str, "time_list_mode = TIME_LIST_BEST_FORECAST;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "specified_forecast")) {
	  sprintf(tmp_str, "time_list_mode = TIME_LIST_SPECIFIED_FORECAST;");
	  TDRP_add_override(&override, tmp_str);
	} else {
	  OK = false;
	}
      } else {
	OK = false;
      }

    } else if (!strcmp(argv[i], "-tlist_start")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "time_list_start = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-tlist_end")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "time_list_end = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-tlist_gen")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "time_list_gen = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-tlist_search")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "time_list_search = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-tlist_margin")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "time_list_margin = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-tlist_also")) {
      
      sprintf(tmp_str, "read_time_list_also = true;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-hdrs_also")) {
      
      sprintf(tmp_str, "read_field_file_headers_also = true;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-url")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "url = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      sprintf(tmp_str, "specify_file_by_time = true;");
      TDRP_add_override(&override, tmp_str);
	
    } else if (!strcmp(argv[i], "-save")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "save_url = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      sprintf(tmp_str, "save_to_file = true;");
      TDRP_add_override(&override, tmp_str);
	
    } else if (!strcmp(argv[i], "-save_no_print")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "save_url = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      sprintf(tmp_str, "save_to_file = true;");
      TDRP_add_override(&override, tmp_str);
	
      sprintf(tmp_str, "no_print = true;");
      TDRP_add_override(&override, tmp_str);
	
    } else if (!strcmp(argv[i], "-vlevel")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "read_lower_vlevel = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "read_upper_vlevel = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "read_set_vlevel_limits = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-vlevel_lower")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "read_lower_vlevel = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "read_set_vlevel_limits = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-vlevel_upper")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "read_upper_vlevel = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "read_set_vlevel_limits = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-vtype")) {
      
      if (i < argc - 1) {
	char *modestr = argv[++i];
	if (!strcmp(modestr, "z")) {
	  sprintf(tmp_str, "read_set_vlevel_type = TRUE;");
	  TDRP_add_override(&override, tmp_str);
	  sprintf(tmp_str, "read_vlevel_type = VERT_TYPE_Z;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "pres")) {
	  sprintf(tmp_str, "read_set_vlevel_type = TRUE;");
	  TDRP_add_override(&override, tmp_str);
	  sprintf(tmp_str, "read_vlevel_type = VERT_TYPE_PRESSURE;");
	  TDRP_add_override(&override, tmp_str);
	} else if (!strcmp(modestr, "flevel")) {
	  sprintf(tmp_str, "read_set_vlevel_type = TRUE;");
	  TDRP_add_override(&override, tmp_str);
	  sprintf(tmp_str, "read_vlevel_type = VERT_FLIGHT_LEVEL;");
	  TDRP_add_override(&override, tmp_str);
	} else {
	  OK = false;
	}
      } else {
	OK = false;
      }

    } else if (!strcmp(argv[i], "-table")) {
      
      sprintf(tmp_str, "get_mode = GET_TABLE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-test")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "test_n_retrievals = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
      
      sprintf(tmp_str, "test_no_print = TRUE;");
      TDRP_add_override(&override, tmp_str);

    }
    
  } // i

  // set fields if specified

  if (fields.size() > 0) {

    // check for numbers as opposed to names

    bool setFieldNums = true;
    for (size_t ii = 0; ii < fields.size(); ii++) {
      int num;
      if (sscanf(fields[ii].c_str(), "%d", &num) != 1) {
	setFieldNums = false;
	break;
      }
    }

    if (setFieldNums) {

      sprintf(tmp_str, "read_set_field_nums = true;");
      TDRP_add_override(&override, tmp_str);
      
      string numStr = "read_field_nums = { ";
      for (size_t ii = 0; ii < fields.size(); ii++) {
	numStr += fields[ii];
	if (ii != fields.size() - 1) {
	  numStr += ", ";
	} else {
	  numStr += " ";
	}
      }
      numStr += "};";
      TDRP_add_override(&override, numStr.c_str());

    } else {

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

    }

  } // if (fields.size() ...

  // set waypoints if specified

  if (waypts.size() > 0) {
    string wayptStr = "read_vsect_waypts = { ";
    for (size_t ii = 0; ii < waypts.size(); ii++) {
      wayptStr += "{";
      wayptStr += waypts[ii];
      wayptStr += "}";
      if (ii != waypts.size() - 1) {
	wayptStr += ",";
      }
    }
    wayptStr += "};";
    TDRP_add_override(&override, wayptStr.c_str());
  }

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
      << "  [ -32_bit_hdrs ] read using the legacy 32-bit headers\n"
      << "     The default is to use the new 64-bit headers\n"
      << "     Legacy servers will always return 32-bit headers\n"
      << "\n"
      << "  [ -all_hdrs ] only get headers\n"
      << "     exactly as they exist in the file\n"
      << "\n"
      << "  [ -bot ] for GIS mode, start with bottom row\n"
      << "     If not set, defaults to start with top row\n"
      << "\n"
      << "  [ -comp ] composite on read\n"
      << "     This takes the maximum at any height.\n"
      << "\n"
      << "  [ -d, -debug ] print debug messages\n"
      << "\n"
      << "  [ -data ] print field data\n"
      << "\n"
      << "  [ -decimate ? ] set read decimation on: defaults to off\n"
      << "     nxy: specify max number of grid points in plane\n"
      << "\n"
      << "  [ -dvi ] disable vsection interpolation\n"
      << "     Use nearest-neighbor instead\n"
      << "\n"
      << "  [ -encoding ? ] get encoding type for data, default 'asis'\n"
      << "     Options: asis, int8, int16, float32\n"
      << "\n"
      << "  [ -f ? ] set file path, see also -path\n"
      << "\n"
      << "  [ -field ? ] Specify particular field\n"
      << "     Specify name or number\n"
      << "     Use muptiple -field args for multiple fields\n"
      << "     If not specified, all fields will be printed\n"
      << "\n"
      << "  [ -float ] print data as floats\n"
      << "\n"
      << "  [ -format ? ] Specify data format\n"
      << "     Default is:         FORMAT_MDV\n"
      << "     Alternatives are:   FORMAT_XML\n"
      << "                         FORMAT_NCF - netCDF CF\n"
      << "\n"
      << "  [ -gis ] print volume in GIS mode, suitable for importing\n"
      << "     by ArcGIS or similar software\n"
      << "\n"
      << "  [ -hdrs_also ] request headers exacltly as in files.\n"
      << "     Use with -volume or -vsection.\n"
      << "\n"
      << "  [ -latest_mod_time ? ] specify latest valid mod time\n"
      << "     No files written after this time will be considered valid.\n"
      << "     Format is \"YYYY MM DD HH MM SS\"\n"
      << "\n"
      << "  [ -lead ? ] forecast lead time (secs): defaults to 0\n"
      << "     mode: specified forecast\n"
      << "\n"
      << "  [ -linear ] force conversion to linear if data is\n"
      << "     stored as logs\n"
      << "\n"
      << "  [ -margin ? ] time_margin (secs): defaults to 3600\n"
      << "     modes: all except latest\n"
      << "\n"
      << "  [ -mode ? ] get mode for volume and vsection, not time_list\n"
      << "     Options: latest, closest, first_before, first_after,\n"
      << "              best_forecast, specified_forecast\n"
      << "\n"
      << "  [ -nlines_data ? ] only print out first nlines of data fields\n"
      << "\n"
      << "  [ -path ? ] set file path, see also -f\n"
      << "\n"
      << "  [ -printCanonical ] force packed format to print canonical\n"
      << "     format\n"
      << "\n"
      << "  [ -printChunks ] print out chunks by forcing a volume to be read\n"
      << "\n"
      << "  [ -save ? ] save result to Mdvx file at specified URL\n"       
      << "     Url format is \"mdvp:://host:port:dir\"\n"
      << "     Use with -volume or -vsection.\n"
      << "\n"
      << "  [ -save_no_print ? ] while saving, suppress normal print\n"       
      << "     See -save\n"
      << "\n"
      << "  [ -summary ] print short summary\n"
      << "\n"
      << "  [ -single_buf ] get from server as single buffer\n"
      << "\n"
      << "  [ -table ] print in tabular form, suitable for importing\n"
      << "     into spreadsheets etc\n"
      << "\n"
      << "  [ -test ? ] Specify test mode\n"
      << "     Specify number of retrievals for test.\n"
      << "     This is used to test retrieval speed.\n"
      << "\n"
      << "  [ -threaded ] use threading\n"
      << "     Uses DsMdvxThreaded object instead of DsMdvx object\n"
      << "\n"
      << "  [ -time ? ] specify search time\n"       
      << "     Format is \"YYYY MM DD HH MM SS\"\n"
      << "\n"
      << "  [ -timeht ] get time-height profile\n"
      << "     Need to specify tlist_mode and tlist details\n"
      << "\n"
      << "  [ -timelist ] get timelist\n"
      << "     Specify tlist_mode, and a single waypt\n"
      << "\n"
      << "  [ -tlist_mode ? ] set time_list mode\n"
      << "     Options: valid, gen, forecast, first, last,\n"
      << "              gen_plus_forecasts, valid_mult_gen,\n"
      << "              closest, first_before, first_after,\n"
      << "              best_forecast, specified_forecast\n"
      << "     Default: valid\n"
      << "     For valid, gen, gen_plus_forecasts and valid_mult_gen,\n"
      << "         specify tlist_start and tlist_end\n"
      << "     For forecast and specified_forecast\n"
      << "         specify tlist_gen\n"
      << "     For closest, first_before, first_after, \n"
      << "         best_forecast and specified_forecast,\n"
      << "         specify tlist_search and tlist_margin\n"
      << "     For specified_forecast, specify gen_time and\n"
      << "         either search_time or lead secs\n"
      << "\n"
      << "  [ -tlist_start ? ] specify time_list start time\n"
      << "     Format is \"YYYY MM DD HH MM SS\"\n"
      << "\n"
      << "  [ -tlist_end ? ] specify time_list end time\n"       
      << "     Format is \"YYYY MM DD HH MM SS\"\n"
      << "\n"
      << "  [ -tlist_gen ? ] specify time_list gen time\n" 
      << "     Format is \"YYYY MM DD HH MM SS\"\n"
      << "\n"
      << "  [ -tlist_search ? ] specify time_list search time\n"
      << "     Format is \"YYYY MM DD HH MM SS\"\n"
      << "\n"
      << "  [ -tlist_margin ? ] specify time_list search margin (secs)\n"
      << "\n"
      << "  [ -tlist_also ] request time list in addition to data fields.\n"
      << "     Use with -volume or -vsection.\n"
      << "\n"
      << "  [ -url ? ] specify url\n"       
      << "     Format is \"mdvp:://host:port:dir\"\n"
      << "\n"
      << "  [ -v, -verbose ] print verbose debug messages\n"
      << "\n"
      << "  [ -vlevel ? ] set single vlevel\n"
      << "\n"
      << "  [ -vlevel_lower ? ] set lower vlevel limit\n"
      << "\n"
      << "  [ -vlevel_upper ? ] set upper vlevel limit\n"
      << "\n"
      << "  [ -volume ] get volume - the default\n"
      << "\n"
      << "  [ -vsection ] get vsection, using waypts\n"
      << "\n"
      << "  [ -vtype ? ] set vlevel type\n"
      << "     Options: z, pres, flevel\n"
      << "     Default: not set\n"
      << "\n"
      << "  [ -waypt ? ] add a vert section waypt\n"       
      << "     Format is \"lat,lon\"\n"
      << "     For timeht, use a single waypt\n"
      << "\n"
      << "  For forwarding, use this sort of syntax:\n"       
      << "      mdvp:://wmds.rap.ucar.edu::mdv/radar/mosaic?forward_url=http://www.rap.ucar.edu/DsServerTunnel& -mode latest\n"       
      << "\n"
      << endl;

  out << "NOTE: You do not need to use the params option (see below).\n"
      << "      If no params are specified, you deal with the whole file.\n"
      << endl;

  Params::usage(out);
  
}
