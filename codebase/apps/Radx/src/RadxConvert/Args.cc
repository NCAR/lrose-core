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
      
    } else if (!strcmp(argv[i], "-instance")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "instance = \"%s\";", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      }
	
    } else if (!strcmp(argv[i], "-version")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "version_override = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      }
	
    } else if (!strcmp(argv[i], "-title")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "title_override = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      }
	
    } else if (!strcmp(argv[i], "-institution")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "institution_override = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      }
	
    } else if (!strcmp(argv[i], "-references")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "references_override = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      }
	
    } else if (!strcmp(argv[i], "-source")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "source_override = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      }
	
    } else if (!strcmp(argv[i], "-history")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "history_override = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      }
	
    } else if (!strcmp(argv[i], "-comment")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "comment_override = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      }
	
    } else if (!strcmp(argv[i], "-author")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "author_override = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      }
	
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
      
    } else if (!strcmp(argv[i], "-cf2")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_CFRADIAL2;");
      TDRP_add_override(&override, tmp_str);

      sprintf(tmp_str, "netcdf_style = NETCDF4;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-ncxx")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_NCXX;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-dorade")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_DORADE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-odim_hdf5")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_ODIM_HDF5;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-finest_geom")) {
      
      sprintf(tmp_str, "remap_to_finest_range_geometry = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-predom_geom")) {
      
      sprintf(tmp_str, "remap_to_predominant_range_geometry = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-foray")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_FORAY;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-nexrad")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_NEXRAD;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-nssl_mrd")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_NSSL_MRD;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-uf")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_UF;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-mdv")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_MDV_RADIAL;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-cf_classic")) {
      
      sprintf(tmp_str, "netcdf_style = CLASSIC;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-cf_nc64bit")) {
      
      sprintf(tmp_str, "netcdf_style = NC64BIT;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-cf_netcdf4")) {
      
      sprintf(tmp_str, "netcdf_style = NETCDF4;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-cf_netcdf4_classic")) {
      
      sprintf(tmp_str, "netcdf_style = NETCDF4_CLASSIC;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-force_vary")) {
      
      sprintf(tmp_str, "output_force_ngates_vary = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-out_start")) {
      
      sprintf(tmp_str, "output_filename_mode = START_TIME_ONLY;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-out_end")) {
      
      sprintf(tmp_str, "output_filename_mode = END_TIME_ONLY;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-rem_miss")) {
      
      sprintf(tmp_str, "remove_rays_with_all_data_missing = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-clear_trans")) {
      
      sprintf(tmp_str, "clear_transition_flag_on_all_rays = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-rem_trans")) {
      
      sprintf(tmp_str, "remove_rays_with_antenna_transitions = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-preserve_sweeps")) {
      
      sprintf(tmp_str, "preserve_sweeps = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-keep_long")) {
      
      sprintf(tmp_str, "remove_long_range_rays = FALSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-rem_short")) {
      
      sprintf(tmp_str, "remove_short_range_rays = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-recompute_sweep_angles")) {
      
      sprintf(tmp_str, "recompute_sweep_fixed_angles = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-reload_sweep_info")) {
      
      sprintf(tmp_str, "reload_sweep_info_from_rays = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-reload_volume_info")) {
      
      sprintf(tmp_str, "reload_volume_info_from_rays = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-transitions_optimize_sur")) {
      
      sprintf(tmp_str, "optimize_surveillance_transitions = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-transitions_max_elev_error")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "optimized_transitions_max_elev_error = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-adjust_sweep_limits_using_angles")) {
      
      sprintf(tmp_str, "adjust_sweep_limits_using_angles = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-trim_sur")) {
      
      sprintf(tmp_str, "trim_surveillance_sweeps_to_360deg = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-const_ngates")) {
      
      sprintf(tmp_str, "set_ngates_constant = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-ldata")) {
      
      sprintf(tmp_str, "write_latest_data_info = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-change_lat_sign")) {
      
      sprintf(tmp_str, "change_radar_latitude_sign = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-apply_georefs")) {
      
      sprintf(tmp_str, "apply_georeference_corrections = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-to_float32")) {
      
      sprintf(tmp_str, "set_output_encoding_for_all_fields = TRUE;");
      TDRP_add_override(&override, tmp_str);
      sprintf(tmp_str, "output_encoding = OUTPUT_ENCODING_FLOAT32;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-to_int32")) {
      
      sprintf(tmp_str, "set_output_encoding_for_all_fields = TRUE;");
      TDRP_add_override(&override, tmp_str);
      sprintf(tmp_str, "output_encoding = OUTPUT_ENCODING_INT32;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-to_int16")) {
      
      sprintf(tmp_str, "set_output_encoding_for_all_fields = TRUE;");
      TDRP_add_override(&override, tmp_str);
      sprintf(tmp_str, "output_encoding = OUTPUT_ENCODING_INT16;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-to_int08")) {
      
      sprintf(tmp_str, "set_output_encoding_for_all_fields = TRUE;");
      TDRP_add_override(&override, tmp_str);
      sprintf(tmp_str, "output_encoding = OUTPUT_ENCODING_INT08;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-prop_std_name")) {
      
      sprintf(tmp_str, "write_using_proposed_standard_name_attr = TRUE;");
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
	
    } else if (!strcmp(argv[i], "-write_other")) {
      
      sprintf(tmp_str, "write_other_fields_unchanged = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
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
	
    } else if (!strcmp(argv[i], "-name")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "instrument_name = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "override_instrument_name = TRUE;");
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-max_range")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "max_range_km = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "set_max_range = TRUE;");
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-start_range")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "start_range_km = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "override_start_range = TRUE;");
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-gate_spacing")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "gate_spacing_km = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "override_gate_spacing = TRUE;");
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
	
    } else if (!strcmp(argv[i], "-outname")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "output_filename = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "output_filename_mode = SPECIFY_FILE_NAME;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-outprefix")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "output_filename_prefix = \"%s\";", argv[++i]);
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
	
    } else if (!strcmp(argv[i], "-relaxed_limits")) {
      
      sprintf(tmp_str, "apply_strict_angle_limits = false;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-sort_sweeps")) {
      
      sprintf(tmp_str, "sort_sweeps_by_fixed_angle = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
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
	
    } else if (!strcmp(argv[i], "-vol_num")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "starting_volume_number = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "override_volume_number = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-vol_num_auto")) {
      
      if (i < argc - 1) {
        i++;
	sprintf(tmp_str, "starting_volume_number = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "autoincrement_volume_number = true;");
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
	
    } else if (!strcmp(argv[i], "-instrument_type")) {
      
      if (i < argc - 1) {
        i++;
        if (!strcmp(argv[i], "radar")) {
          sprintf(tmp_str, "instrument_type = INSTRUMENT_RADAR;");
        } else if (!strcmp(argv[i], "lidar")) {
          sprintf(tmp_str, "instrument_type = INSTRUMENT_LIDAR;");
        } else {
          _usage(cerr);
          cerr << "ERROR - INVALID INSTRUMENT TYPE >>> " << argv[i] << " <<<" << endl;
          return -1;
        }
        TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "override_instrument_type = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-platform_type")) {
      
      if (i < argc - 1) {
        i++;
        if (!strcmp(argv[i], "fixed")) {
          sprintf(tmp_str, "platform_type = PLATFORM_FIXED;");
        } else if (!strcmp(argv[i], "vehicle")) {
          sprintf(tmp_str, "platform_type = PLATFORM_VEHICLE;");
        } else if (!strcmp(argv[i], "ship")) {
          sprintf(tmp_str, "platform_type = PLATFORM_SHIP;");
        } else if (!strcmp(argv[i], "aircraft_fore")) {
          sprintf(tmp_str, "platform_type = PLATFORM_AIRCRAFT_FORE;");
        } else if (!strcmp(argv[i], "aircraft_aft")) {
          sprintf(tmp_str, "platform_type = PLATFORM_AIRCRAFT_AFT;");
        } else if (!strcmp(argv[i], "aircraft_tail")) {
          sprintf(tmp_str, "platform_type = PLATFORM_AIRCRAFT_TAIL;");
        } else if (!strcmp(argv[i], "aircraft_belly")) {
          sprintf(tmp_str, "platform_type = PLATFORM_AIRCRAFT_BELLY;");
        } else if (!strcmp(argv[i], "aircraft_roof")) {
          sprintf(tmp_str, "platform_type = PLATFORM_AIRCRAFT_ROOF;");
        } else if (!strcmp(argv[i], "aircraft_nose")) {
          sprintf(tmp_str, "platform_type = PLATFORM_AIRCRAFT_NOSE;");
        } else if (!strcmp(argv[i], "sat_orbit")) {
          sprintf(tmp_str, "platform_type = PLATFORM_SATELLITE_ORBIT;");
        } else if (!strcmp(argv[i], "sat_geostat")) {
          sprintf(tmp_str, "platform_type = PLATFORM_SATELLITE_GEOSTAT;");
        } else {
          _usage(cerr);
          cerr << "ERROR - INVALID PLATFORM TYPE >>> " << argv[i] << " <<<" << endl;
          return -1;
        }
        TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "override_platform_type = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-primary_axis")) {
      
      if (i < argc - 1) {
        i++;
        if (!strcmp(argv[i], "z")) {
          sprintf(tmp_str, "primary_axis = PRIMARY_AXIS_Z;");
        } else if (!strcmp(argv[i], "y")) {
          sprintf(tmp_str, "primary_axis = PRIMARY_AXIS_Y;");
        } else if (!strcmp(argv[i], "x")) {
          sprintf(tmp_str, "primary_axis = PRIMARY_AXIS_X;");
        } else if (!strcmp(argv[i], "z_prime")) {
          sprintf(tmp_str, "primary_axis = PRIMARY_AXIS_Z_PRIME;");
        } else if (!strcmp(argv[i], "y_prime")) {
          sprintf(tmp_str, "primary_axis = PRIMARY_AXIS_Y_PRIME;");
        } else if (!strcmp(argv[i], "x_prime")) {
          sprintf(tmp_str, "primary_axis = PRIMARY_AXIS_X_PRIME;");
        } else {
          _usage(cerr);
          cerr << "ERROR - INVALID INSTRUMENT TYPE >>> " << argv[i] << " <<<" << endl;
          return -1;
        }
        TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "override_primary_axis = true;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
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
      nameStr += "OUTPUT_ENCODING_ASIS, ";
      nameStr += "SCALING_DYNAMIC, ";
      nameStr += "0.01, ";
      nameStr += "0.0 ";
      nameStr += " }";
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
      << "  [ -adjust_sweep_limits_using_angles ]\n"
      << "     Adjust sweep limits by comparing fixed and measured angles\n"
      << "\n"
      << "  [ -ag ] aggregate sweep files into volume on read.\n"
      << "     Files with the SAME VOLUME NUMBER in the name are aggregated.\n"
      << "    Applies to CfRadial and DORADE sweep files.\n"
      << "\n"
      << "  [ -ag_all ] aggregate files in input list on read.\n"
      << "     ALL FILES in the input list are aggregated into a volume.\n"
      << "     See '-f' option.\n"
      << "\n"
      << "  [ -alt ? ] override radar altitude (m)\n"
      << "\n"
      << "  [ -apply_georefs] apply georeference corrections on read.\n"
      << "     For moving platforms, measured georeference information is sometimes\n"
      << "     available. If so, this is applied and appropriate corrections made.\n"
      << "     Earth-centric azimuth and elevation angles will be computed.\n"
      << "\n"
      << "  [ -author ? ] override the author string\n"
      << "\n"
      << "  [ -change_lat_sign] change the sign of the radar latitude\n"
      << "    Useful for RAPIC files that always have a positive latitude.\n"
      << "\n"
      << "  [ -cfradial ] convert to cfradial (the default)\n"
      << "  [ -cf2 ] convert to cfradial2, forces use of netcdf4\n"
      << "\n"
      << "  [ -cf_classic ] output classic-style netcdf (the default)\n"
      << "  [ -cf_netcdf4 ] output netcdf4 style\n"
      << "  [ -cf_classic4 ] output classic-style netcdf4\n"
      << "  [ -cf_nc64bit ] output 64-bit NC netcdf\n"
      << "     The above only apply to cfradial and foray output.\n"
      << "\n"
      << "  [ -clear_trans ] clear antenna transition flag on all rays\n"
      << "\n"
      << "  [ -comment ? ] override the comment string\n"
      << "\n"
      << "  [ -compress ? ] compress output\n"
      << "     specifiy compression level [1-9]\n"
      << "     For cfradial, forces netcdf4 mode\n"
      << "\n"
      << "  [ -const_ngates ] force number of gates constant for all rays\n"
      << "     Added gates will be filled with missing values\n"
      << "\n"
      << "  [ -d, -debug ] print debug messages\n"
      << "\n"
      << "  [ -dorade ] convert to dorade\n"
      << "\n"
      << "  [ -disag ] dis-aggregate into sweep files on write\n"
      << "     optional for CfRadial files\n"
      << "     always applies to DORADE sweep files\n"
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
      << "  [ -f, -paths ? ] set file paths\n"
      << "     Sets mode to FILELIST\n"
      << "\n"
      << "  [ -field ? ] Specify particular field\n"
      << "     Specify name or number\n"
      << "     Use multiple -field args for multiple fields\n"
      << "     If not specified, all fields will be used\n"
      << "\n"
      << "  [ -finest_geom ] remap to finest range geometry\n"
      << "\n"
      << "  [ -foray ] convert to FORAY-1 netcdf\n"
      << "\n"
      << "  [ -force_vary ] force use of ragged arrays for CfRadial\n"
      << "     even if ngates is constant for all rays\n"
      << "\n"
      << "  [ -gate_spacing ? ] set gate spacing for ray geometry (km)\n"
      << "\n"
      << "  [ -history ? ] override the history string\n"
      << "\n"
      << "  [ -indir ? ] set input directory\n"
      << "\n"
      << "  [ -instance ?] specify the instance\n"
      << "\n"
      << "  [ -institution ? ] override the institution string\n"
      << "\n"
      << "  [ -instrument_type ? ] override instrument type\n"
      << "     Options are: radar, lidar\n"
      << "\n"
      << "  [ -keep_long ] keep long range rays\n"
      << "     Keep NEXRAD long-range non-Doppler sweeps\n"
      << "     Default is to remove them\n"
      << "\n"
      << "  [ -lat ? ] override radar latitude (deg)\n"
      << "\n"
      << "  [ -lon ? ] override radar longitude (deg)\n"
      << "\n"
      << "  [ -ldata ] write _latest_data_info files\n"
      << "\n"
      << "  [ -max_range ? ] set max range (km)\n"
      << "\n"
      << "  [ -mdv ] convert to MDV in radial coords\n"
      << "\n"
      << "  [ -native ] output in host-native byte ordering\n"
      << "\n"
      << "  [ -name ? ] override instrument name\n"
      << "\n"
      << "  [ -ncxx ] convert to cfradial using Ncxx classes\n"
      << "\n"
      << "  [ -nexrad ] convert to NEXRAD archive level 2\n"
      << "\n"
      << "  [ -nssl_mrd ] convert to NSSL MRD format\n"
      << "\n"
      << "  [ -odim_hdf5 ] convert to ODIM HDF5\n"
      << "\n"
      << "  [ -outdir ? ] set output directory\n"
      << "\n"
      << "  [ -outname ? ] specify output file name\n"
      << "     file of this name will be written to outdir\n"
      << "\n"
      << "  [ -outprefix ? ] specify output file name prefix\n"
      << "     if not set, standard prefix will be used\n"
      << "\n"
      << "  [ -out_end ? ] compute output path using end time\n"
      << "     default is to use both start and end times\n"
      << "\n"
      << "  [ -out_start ? ] compute output path using start time\n"
      << "     default is to use both start and end times\n"
      << "\n"
      << "  [ -platform_type ? ] override platform type. Options are:\n"
      << "     fixed, vehicle, ship, aircraft_fore, aircraft_aft, aircraft_tail\n"
      << "     aircraft_belly, aircraft_roof, aircraft_nose\n"
      << "     sat_orbit, sat_geostat\n"
      << "\n"
      << "  [ -predom_geom ] remap to predominant range geometry\n"
      << "\n"
      << "  [ -preserve_sweeps ] preserve sweep details as they are in file.\n"
      << "     This generally applies to NEXRAD data - by default we\n"
      << "     consolidate sweeps by combining split-cut sweeps\n"
      << "     into a single sweep.\n"
      << "     If this flag is true, we leave the sweeps unchanged.\n"
      << "\n"
      << "  [ -primary_axis ? ] override primary axis\n"
      << "     Options are: z, y, x, z_prime, y_prime, x_prime\n"
      << "\n"
      << "  [ -prop_std_name ] use 'proposed_standard_name' attribute\n"
      << "                     instead of 'standard_name' attribute\n"
      << "                     for CfRadial files\n"
      << "\n"
      << "  [ -radar_num ? ] set radar number\n"
      << "     Applies to NOAA HRD data. LF radar = 1, TA radar = 2\n"
      << "     Generally not needed\n"
      << "     If set to 1 will force convert to assume a lower fuselage radar\n"
      << "     If set to 2 will force convert to assume a tail radar\n"
      << "\n"
      << "  [ -recompute_sweep_angles ]\n"
      << "     Recompute sweep fixed angles from ray angles\n"
      << "\n"
      << "\n"
      << "  [ -references ? ] override the references string\n"
      << "\n"
      << "  [ -reload_sweep_info ]\n"
      << "     Forces a reload of sweep information from the individual rays\n"
      << "\n"
      << "  [ -reload_volume_info ]\n"
      << "     Forces a reload of volume information from the individual rays\n"
      << "\n"
      << "  [ -relaxed_limits ] If set, turn off strict checking when using the\n"
      << "     options -ang and -ang_max, or -sweep and -sweep_max.\n"
      << "     If relaxed, and no data lies within the specified limits,\n"
      << "     then the closest applicable sweep will be read.\n"
      << "\n"
      << "  [ -rem_miss ] remove rays in which data at all gates and\n"
      << "     for all fields is missing\n"
      << "\n"
      << "  [ -rem_short ] remove short range rays\n"
      << "     Remove NEXRAD short-range Doppler sweeps\n"
      << "\n"
      << "  [ -rem_trans ] remove rays with antenna transitions\n"
      << "\n"
      << "  [ -sort_sweeps ] sort sweeps by fixed angle\n"
      << "     Sorts in ascending order. Rays are reordered accordingly\n"
      << "\n"
      << "  [ -source ? ] override the source string\n"
      << "\n"
      << "  [ -start \"yyyy mm dd hh mm ss\"] start time\n"
      << "     Sets mode to ARCHIVE\n"
      << "\n"
      << "  [ -start_range ? ] set start range for ray geometry (km)\n"
      << "\n"
      << "  [ -sweep ? ] set single sweep number\n"
      << "               or minimum - see '-sweep_max'\n"
      << "\n"
      << "  [ -sweep_max ? ] set max sweep number\n"
      << "     use '-sweep' for setting minimum\n"
      << "\n"
      << "  [ -time_offset ? ] set time offset (secs)\n"
      << "\n"
      << "  [ -title ? ] override the title string\n"
      << "\n"
      << "  [ -transitions_optimize_sur ] optimize the antenna transitions\n"
      << "     in surveillance mode.\n"
      << "\n"
      << "  [ -transitions_max_elev_error ? ] max elevation error when optimizing\n"
      << "     transitions. See -transitions_optimize_sur\n"
      << "     Default is 0.25 deg\n"
      << "\n"
      << "  [ -trim_sur ] trim surveillance sweeps to 360 degrees\n"
      << "     Remove extra rays in each surveillance sweep\n"
      << "\n"
      << "  [ -to_float32 ] convert all fields to 32-bit floats\n"
      << "  [ -to_int32 ] convert all fields to 16-bit signed integers\n"
      << "  [ -to_int16 ] convert all fields to 16-bit signed integers\n"
      << "  [ -to_in08 ] convert all fields to 8-bit signed integers\n"
      << "\n"
      << "  [ -uf ] convert to universal format\n"
      << "\n"
      << "  [ -v, -verbose ] print verbose debug messages\n"
      << "\n"
      << "  [ -vv, -extra ] print extra verbose debug messages\n"
      << "\n"
      << "  [ -version ? ] override the version string\n"
      << "\n"
      << "  [ -vol_num ? ] specify volume number\n"
      << "     Overrides the volume number in the data\n"
      << "\n"
      << "  [ -vol_num_auto ? ] specify incrementing volume numbers,\n"
      << "     starting at the number specified.\n"
      << "     Overrides the volume number in the data\n"
      << "\n"
      << "  [ -write_other ] option to write other fields unchanged.\n"
      << "     Default is that if -fields is used, only the specified fields\n"
      << "     will be written in the output files.\n"
      << "\n"
      << endl;
  
  out << "NOTE: You do not need to use the params option (see below).\n"
      << "      If no params are specified, you deal with the whole file.\n"
      << endl;

  Params::usage(out);
  
}
