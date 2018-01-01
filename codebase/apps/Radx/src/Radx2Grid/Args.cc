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
// May 2012
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
  startTimeSet = false;
  endTimeSet = false;
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
	sprintf(tmp_str, "instance = %s;", argv[i+1]);
	TDRP_add_override(&override, tmp_str);
      }
	
    } else if (!strcmp(argv[i], "-field")) {
      
      if (i < argc - 1) {
	fields.push_back(argv[++i]);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-format")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "output_format = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }

    } else if (!strcmp(argv[i], "-grid_z_geom")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "grid_z_geom = { %s };", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-z_level_array")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "z_level_array = { %s };", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "specify_individual_z_levels = TRUE;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-grid_xy_geom")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "grid_xy_geom = { %s };", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-latlon")) {
      
      sprintf(tmp_str, "grid_projection = PROJ_LATLON;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-remap_to_latlon")) {
      
      sprintf(tmp_str, "auto_remap_flat_to_latlon = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-start")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "start_time = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "mode = ARCHIVE;");
        TDRP_add_override(&override, tmp_str);
        startTimeSet = true;
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-end")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "end_time = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "mode = ARCHIVE;");
        TDRP_add_override(&override, tmp_str);
        endTimeSet = true;
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
        sprintf(tmp_str, "specify_output_filename = TRUE;");
        TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }
	
    } else if (!strcmp(argv[i], "-beam_width")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "beam_width_deg_h = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "beam_width_deg_v = %s;", argv[i]);
	TDRP_add_override(&override, tmp_str);
	sprintf(tmp_str, "override_beam_width = TRUE;");
	TDRP_add_override(&override, tmp_str);
      } else {
	OK = false;
      }

    } else if (!strcmp(argv[i], "-add_angle_fields")) {
      
      sprintf(tmp_str, "output_angle_fields = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-add_range_field")) {
      
      sprintf(tmp_str, "output_range_field = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-add_height_field")) {
      
      sprintf(tmp_str, "output_height_field = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-add_coverage_field")) {
      
      sprintf(tmp_str, "output_coverage_field = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-add_time_field")) {
      
      sprintf(tmp_str, "output_time_field = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-ppi")) {
      
      sprintf(tmp_str, "interp_mode = INTERP_MODE_PPI;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-polar")) {
      
      sprintf(tmp_str, "interp_mode = INTERP_MODE_POLAR;");
      TDRP_add_override(&override, tmp_str);
      
    }
    
  } // i

  // set fields if specified

  if (fields.size() > 0) {
    
    sprintf(tmp_str, "select_fields = true;");
    TDRP_add_override(&override, tmp_str);
    
    string selStr = "selected_fields = { ";
    for (size_t ii = 0; ii < fields.size(); ii++) {
      selStr += "{ ";
      selStr += "\"";
      selStr += fields[ii];
      selStr += "\", TRUE }";
      if (ii != fields.size() - 1) {
        selStr += ", ";
      } else {
        selStr += " ";
      }
    }
    selStr += "};";
    TDRP_add_override(&override, selStr.c_str());
    
  } // if (fields.size() ...

  if (!OK) {
    _usage(cerr);
    return -1;
  }

  return 0;
    
}

void Args::_usage(ostream &out)
{

  Params defaults;

  char gridXyGeomStr[1024];
  snprintf(gridXyGeomStr, 1024, "%d, %d, %g, %g, %g, %g",
           defaults.grid_xy_geom.nx,
           defaults.grid_xy_geom.ny,
           defaults.grid_xy_geom.minx,
           defaults.grid_xy_geom.miny,
           defaults.grid_xy_geom.dx,
           defaults.grid_xy_geom.dy);
  
  char gridZGeomStr[1024];
  snprintf(gridZGeomStr, 1024, "%d, %g, %g",
           defaults.grid_z_geom.nz,
           defaults.grid_z_geom.minz,
           defaults.grid_z_geom.dz);
  
  out << "Usage: " << _progName << " [args as below]\n"
      << "Options:\n"
      << "\n"
      << "  [ -h ] produce this list.\n"
      << "\n"
      << "  [ -d, -debug ] print debug messages\n"
      << "\n"
      << "  [ -add_angle_fields ]\n"
      << "     Create fields azimuth & elevation (deg), alpha, beta, gamma\n"
      << "     and include in output file\n"
      << "\n"
      << "  [ -add_range_field ]\n"
      << "     Create 'range' field and include in output file\n"
      << "     This is slant range from the radar (km)\n"
      << "\n"
      << "  [ -add_height_field ]\n"
      << "     Create 'height' field and include in output file\n"
      << "     This is height of the beam in km MSL\n"
      << "\n"
      << "  [ -add_coverage_field ]\n"
      << "     Create 'coverage' field and include in output file\n"
      << "     This is a 0/1 flag to indicate the extent of radar coverage\n"
      << "\n"
      << "  [ -add_time_field ]\n"
      << "     Create 'time' field and include in output file\n"
      << "     This is time since start of volume (secs)\n"
      << "\n"
      << "  [ -beam_width ? ] override beam width (deg)\n"
      << "\n"
      << "  [ -end \"yyyy mm dd hh mm ss\"]\n"
      << "     Set the end time in archive mode\n"
      << "     Sets mode to ARCHIVE\n"
      << "\n"
      << "  [ -f, -paths ? ] set input file paths\n"
      << "     Sets mode to FILELIST\n"
      << "\n"
      << "  [ -field ? ] Specify field name\n"
      << "     Use multiple -field args for multiple fields\n"
      << "     If no fields specified, all fields will be included\n"
      << "\n"
      << "  [ -format ? ] Specify format for output file:\n"
      << "       CF_NETCDF: the default, NetCDF with CF conventions\n"
      << "       ZEBRA_NETCDF: legacy, NetCDF for ZEBRA\n"
      << "       MDV: NCAR Meteorological Data Volume binary format\n"
      << "       CEDRIC: NCAR CEDRIC binary format\n"
      << "\n"
      << "  [ -grid_xy_geom \"nx, ny, minx, miny, dx, dy\"]\n"
      << "     Set the geometry for constant spacing in (X,Y)\n"
      << "     nx, ny: number of cells in (x,y)\n"
      << "     minx, miny: coords of center of SW grid cell (km or deg)\n"
      << "     dx, xy: spacing between cells in (km or deg) (km)\n"
      << "     Units are deg for LATLON, km for all other projections\n"
      << "     Default is: \"" << gridXyGeomStr << "\"\n"
      << "\n"
      << "  [ -grid_z_geom \"nz, minz, dz\"]\n"
      << "     Set the geometry for constant spacing in Z\n"
      << "     nz: number of z levels\n"
      << "     minz: lowest Z level (km MSL)\n"
      << "     dz: spacing between Z levels (km)\n"
      << "     Default is: \"" << gridZGeomStr << "\"\n"
      << "\n"
      << "  [ -indir ? ] set input directory\n"
      << "     for ARCHIVE mode\n"
      << "     see also -start and -end\n"
      << "\n"
      << "  [ -instance ?] specify the instance\n"
      << "\n"
      << "  [ -latlon] set the output projection to latlon\n"
      << "     Default projection is FLAT - azimuthal_equidistant\n"
      << "     Grid xy units specified in degrees\n"
      << "\n"
      << "  [ -outdir ? ] set output directory\n"
      << "\n"
      << "  [ -outname ? ] specify output file name\n"
      << "     file of this name will be written to outdir\n"
      << "\n"
      << "  [ -ppi ] interp in PPI mode\n"
      << "     interp onto Cart (x,y) grid (km) in the horizontal\n"
      << "     preserve sweep elevation angles (deg) in the vertical\n"
      << "\n"
      << "  [ -polar ] interp in POLAR mode\n"
      << "     maintain in polar coordinates\n"
      << "     interp onto a regular grid in azimuth (deg)\n"
      << "     preserve sweep elevation angles (deg) in the vertical\n"
      << "\n"
      << "  [ -remap_to_latlon] remap to latlon after interpolation\n"
      << "     Grid xy units specified in km\n"
      << "\n"
      << "  [ -start \"yyyy mm dd hh mm ss\"]\n"
      << "     Set the start time in archive mode\n"
      << "     Sets mode to ARCHIVE\n"
      << "\n"
      << "  [ -v, -verbose ] print verbose debug messages\n"
      << "\n"
      << "  [ -vv, -extra ] print extra verbose debug messages\n"
      << "\n"
      << "  [ -z_level_array \"z0, z1, .... , zn-1\" ] specify z levels\n"
      << "     Use this instead of -grid_z_geom for non-constant z spacing\n"
      << "\n"
      << endl;
  
  Params::usage(out);
  
}
