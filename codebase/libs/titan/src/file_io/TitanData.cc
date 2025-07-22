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
////////////////////////////////////////////////////////////////
// TitanData.cc
//
// Titan data classes, structs etc.
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2025.
//
////////////////////////////////////////////////////////////////

#include <cassert>
#include <titan/TitanData.hh>
#include <toolsa/str.h>

using namespace std;

////////////////////////////////////////////////////////////
// StormParams

TitanData::StormParams::StormParams()
{
  initialize();
}

void TitanData::StormParams::initialize()
{
  low_dbz_threshold = 0.0;
  high_dbz_threshold = 0.0;
  dbz_hist_interval = 0.0;
  hail_dbz_threshold = 0.0;
  base_threshold = 0.0;
  top_threshold = 0.0;
  min_storm_size = 0.0;
  max_storm_size = 0.0;
  morphology_erosion_threshold = 0.0;
  morphology_refl_divisor = 0.0;
  min_radar_tops = 0.0;
  tops_edge_margin = 0.0;
  z_p_coeff = 0.0;
  z_p_exponent = 0.0;
  z_m_coeff = 0.0;
  z_m_exponent = 0.0;
  sectrip_vert_aspect = 0.0;
  sectrip_horiz_aspect = 0.0;
  sectrip_orientation_error = 0.0;
  poly_start_az = 0.0;
  poly_delta_az = 0.0;
  check_morphology = 0;
  check_tops = 0;
  vel_available = 0;
  n_poly_sides = 0;    
  ltg_count_time = 0.0;
  ltg_count_margin_km = 0.0;
  hail_z_m_coeff = 0.0;
  hail_z_m_exponent = 0.0;
  hail_mass_dbz_threshold = 0.0;
  gprops_union_type = 0;
  tops_dbz_threshold = 0.0;
  precip_plane_ht = 0.0;
  low_convectivity_threshold = 0.0;
  high_convectivity_threshold = 0.0;
  precip_computation_mode = PRECIP_FROM_COLUMN_MAX;
}

void TitanData::StormParams::setFromLegacy(const storm_file_params_t &params)
{

  low_dbz_threshold = params.low_dbz_threshold;
  high_dbz_threshold = params.high_dbz_threshold;
  dbz_hist_interval = params.dbz_hist_interval;
  hail_dbz_threshold = params.hail_dbz_threshold;
  base_threshold = params.base_threshold;
  top_threshold = params.top_threshold;
  min_storm_size = params.min_storm_size;
  max_storm_size = params.max_storm_size;
  morphology_erosion_threshold = params.morphology_erosion_threshold;
  morphology_refl_divisor = params.morphology_refl_divisor;
  min_radar_tops = params.min_radar_tops;
  tops_edge_margin = params.tops_edge_margin;
  z_p_coeff = params.z_p_coeff;
  z_p_exponent = params.z_p_exponent;
  z_m_coeff = params.z_m_coeff;
  z_m_exponent = params.z_m_exponent;
  sectrip_vert_aspect = params.sectrip_vert_aspect;
  sectrip_horiz_aspect = params.sectrip_horiz_aspect;
  sectrip_orientation_error = params.sectrip_orientation_error;
  poly_start_az = params.poly_start_az;
  poly_delta_az = params.poly_delta_az;
  check_morphology = params.check_morphology;
  check_tops = params.check_tops;
  vel_available = params.vel_available;
  n_poly_sides = params.n_poly_sides;
  ltg_count_time = params.ltg_count_time;
  ltg_count_margin_km = params.ltg_count_margin_km;
  hail_z_m_coeff = params.hail_z_m_coeff;
  hail_z_m_exponent = params.hail_z_m_exponent;
  hail_mass_dbz_threshold = params.hail_mass_dbz_threshold;
  gprops_union_type = params.gprops_union_type;
  tops_dbz_threshold = params.tops_dbz_threshold;
  precip_plane_ht = params.precip_plane_ht;
  low_convectivity_threshold = params.low_convectivity_threshold;
  high_convectivity_threshold = params.high_convectivity_threshold;

  switch (params.precip_computation_mode) {
    case TITAN_PRECIP_FROM_COLUMN_MAX:
      precip_computation_mode = PRECIP_FROM_COLUMN_MAX;
      break;
    case TITAN_PRECIP_AT_SPECIFIED_HT:
      precip_computation_mode = PRECIP_AT_SPECIFIED_HT;
      break;
    case TITAN_PRECIP_AT_LOWEST_VALID_HT:
      precip_computation_mode = PRECIP_AT_LOWEST_VALID_HT;
      break;
    case TITAN_PRECIP_FROM_LOWEST_AVAILABLE_REFL:
      precip_computation_mode = PRECIP_FROM_LOWEST_AVAILABLE_REFL;
      break;
  }

}

storm_file_params_t TitanData::StormParams::convertToLegacy() const
{
  storm_file_params_t sparams;
  MEM_zero(sparams);
  convertToLegacy(sparams);
  return sparams;
}

void TitanData::StormParams::convertToLegacy(storm_file_params_t &params) const
{

  params.low_dbz_threshold = low_dbz_threshold;
  params.high_dbz_threshold = high_dbz_threshold;
  params.dbz_hist_interval = dbz_hist_interval;
  params.hail_dbz_threshold = hail_dbz_threshold;
  params.base_threshold = base_threshold;
  params.top_threshold = top_threshold;
  params.min_storm_size = min_storm_size;
  params.max_storm_size = max_storm_size;
  params.morphology_erosion_threshold = morphology_erosion_threshold;
  params.morphology_refl_divisor = morphology_refl_divisor;
  params.min_radar_tops = min_radar_tops;
  params.tops_edge_margin = tops_edge_margin;
  params.z_p_coeff = z_p_coeff;
  params.z_p_exponent = z_p_exponent;
  params.z_m_coeff = z_m_coeff;
  params.z_m_exponent = z_m_exponent;
  params.sectrip_vert_aspect = sectrip_vert_aspect;
  params.sectrip_horiz_aspect = sectrip_horiz_aspect;
  params.sectrip_orientation_error = sectrip_orientation_error;
  params.poly_start_az = poly_start_az;
  params.poly_delta_az = poly_delta_az;
  params.check_morphology = check_morphology;
  params.check_tops = check_tops;
  params.vel_available = vel_available;
  params.n_poly_sides = n_poly_sides;
  params.ltg_count_time = ltg_count_time;
  params.ltg_count_margin_km = ltg_count_margin_km;
  params.hail_z_m_coeff = hail_z_m_coeff;
  params.hail_z_m_exponent = hail_z_m_exponent;
  params.hail_mass_dbz_threshold = hail_mass_dbz_threshold;
  params.gprops_union_type = gprops_union_type;
  params.tops_dbz_threshold = tops_dbz_threshold;
  params.precip_plane_ht = precip_plane_ht;
  params.low_convectivity_threshold = low_convectivity_threshold;
  params.high_convectivity_threshold = high_convectivity_threshold;

  switch (precip_computation_mode) {
    case PRECIP_FROM_COLUMN_MAX:
      params.precip_computation_mode = TITAN_PRECIP_FROM_COLUMN_MAX;
      break;
    case PRECIP_AT_SPECIFIED_HT:
      params.precip_computation_mode = TITAN_PRECIP_AT_SPECIFIED_HT;
      break;
    case PRECIP_AT_LOWEST_VALID_HT:
      params.precip_computation_mode = TITAN_PRECIP_AT_LOWEST_VALID_HT;
      break;
    case PRECIP_FROM_LOWEST_AVAILABLE_REFL:
      params.precip_computation_mode = TITAN_PRECIP_FROM_LOWEST_AVAILABLE_REFL;
      break;
  }
  
}

void TitanData::StormParams::print(FILE *out, const char *spacer)
{

  fprintf(out, "%sStorm file parameters : \n", spacer);
  fprintf(out, "%s  Low dBZ threshold : %g\n", spacer, low_dbz_threshold);
  fprintf(out, "%s  High dBZ threshold : %g\n", spacer, high_dbz_threshold);
  fprintf(out, "%s  Hail dBZ threshold : %g\n", spacer, hail_dbz_threshold);
  fprintf(out, "%s  Hail mass dBZ threshold : %g\n", spacer, hail_mass_dbz_threshold);
  fprintf(out, "%s  Dbz hist interval : %g\n", spacer, dbz_hist_interval);
  fprintf(out, "%s  Top threshold (km) : %g\n", spacer, top_threshold);
  fprintf(out, "%s  Base threshold (km) : %g\n", spacer, base_threshold);
  fprintf(out, "%s  Min storm size (km2 or km3) : %g\n", spacer, min_storm_size);
  fprintf(out, "%s  Max storm size (km2 or km3) : %g\n", spacer, max_storm_size);
  fprintf(out, "%s  Check morphology? : %s\n", spacer, BOOL_STR(check_morphology).c_str());
  fprintf(out, "%s  Morphology_erosion_threshold (km) : %g\n", spacer,
	  morphology_erosion_threshold);
  fprintf(out, "%s  Morphology_refl_divisor (dbz/km) : %g\n", spacer,
	  morphology_refl_divisor);
  fprintf(out, "%s  Check tops? : %s\n", spacer, BOOL_STR(check_tops).c_str());
  fprintf(out, "%s  Min_radar_tops (km) : %g\n", spacer, min_radar_tops);
  fprintf(out, "%s  Tops_edge_margin (km) : %g\n", spacer, tops_edge_margin);
  fprintf(out, "%s  Z-R coefficient : %g\n", spacer, z_p_coeff);
  fprintf(out, "%s  Z-R exponent : %g\n", spacer, z_p_exponent);
  fprintf(out, "%s  Z-M coefficient : %g\n", spacer, z_m_coeff);
  fprintf(out, "%s  Z-M exponent : %g\n", spacer, z_m_exponent);
  fprintf(out, "%s  Hail Z-M coefficient : %g\n", spacer, hail_z_m_coeff);
  fprintf(out, "%s  Hail Z-M exponent : %g\n", spacer, hail_z_m_exponent);
  fprintf(out, "%s  Sectrip vert aspect : %g\n", spacer, sectrip_vert_aspect);
  fprintf(out, "%s  Sectrip horiz aspect : %g\n", spacer, sectrip_horiz_aspect);
  fprintf(out, "%s  Sectrip orientation error : %g\n", spacer, sectrip_orientation_error);
  fprintf(out, "%s  Velocity data available? : %s\n", spacer, BOOL_STR(vel_available).c_str());
  fprintf(out, "%s  N_poly_sides : %d\n", spacer, n_poly_sides);
  fprintf(out, "%s  Poly_start_az : %g\n", spacer, poly_start_az);
  fprintf(out, "%s  Poly_delta_az : %g\n", spacer, poly_delta_az);

  switch(gprops_union_type) {
    case UNION_HAIL:
      fprintf(out, "%s  Storm properties union type : HAIL\n", spacer );
      break;
    case UNION_NEXRAD_HDA:
      fprintf(out, "%s  Storm properties union type : NEXRAD HDAL\n",
              spacer );
      break;
    default:
      fprintf(out, "%s  Storm properties union type : NONE\n", spacer );
      break;
  }

  if (tops_dbz_threshold == 0.0) {
    fprintf(out, "%s  Tops dBZ threshold : %g\n", spacer, low_dbz_threshold);
  } else {
    fprintf(out, "%s  Tops dBZ threshold : %g\n", spacer, tops_dbz_threshold);
  }

  if (precip_computation_mode == PRECIP_AT_LOWEST_VALID_HT) {
    fprintf(out, "%s  Precip computed for lowest valid CAPPI ht\n", spacer);
  } else if (precip_computation_mode == PRECIP_AT_SPECIFIED_HT) {
    fprintf(out, "%s  Precip computed from specified ht (km): %g\n",
            spacer, precip_plane_ht);
  } else if (precip_computation_mode == PRECIP_FROM_LOWEST_AVAILABLE_REFL) {
    fprintf(out, "%s  Precip computed from lowest available reflectivity\n",
            spacer);
  } else {
    fprintf(out, "%s  Precip computed from column max\n", spacer);
  }
  
  fprintf(out, "%s  Low convectivity threshold  : %g\n", spacer, low_convectivity_threshold);
  fprintf(out, "%s  High convectivity threshold : %g\n", spacer, high_convectivity_threshold);

  fprintf(out, "\n");

}

////////////////////////////////////////////////////////////
// StormHeader

TitanData::StormHeader::StormHeader()
{
  initialize();
}

void TitanData::StormHeader::initialize()
{
  file_time = 0;
  start_time = 0;
  end_time = 0;
  n_scans = 0;
}

void TitanData::StormHeader::setFromLegacy(const storm_file_header_t &hdr)
{

  file_time = hdr.file_time;
  start_time = hdr.start_time;
  end_time = hdr.end_time;
  n_scans = hdr.n_scans;
  params.setFromLegacy(hdr.params);
  
}

void TitanData::StormHeader::convertToLegacy(storm_file_header_t &hdr) const
{

  hdr.file_time = file_time;
  hdr.start_time = start_time;
  hdr.end_time = end_time;
  hdr.n_scans = n_scans;
  params.convertToLegacy(hdr.params);
  
}

storm_file_header_t TitanData::StormHeader::convertToLegacy() const
{
  storm_file_header_t hdr;
  MEM_zero(hdr);
  convertToLegacy(hdr);
  return hdr;
}

void TitanData::StormHeader::print(FILE *out, const char *spacer)
{

  char spacer2[128];
  snprintf(spacer2, 128, "%s  ", spacer);
  
  fprintf(out, "%sStorm file header :\n", spacer);
  fprintf(out, "%s  Dates and times : \n", spacer);
  fprintf(out, "%s    File   : %s\n",  spacer, utimstr(file_time));
  fprintf(out, "%s    Start  : %s\n",  spacer, utimstr(start_time));
  fprintf(out, "%s    End    : %s\n",  spacer, utimstr(end_time));
  fprintf(out, "%s  Number of scans : %d\n", spacer, n_scans);
  fprintf(out, "\n");

  params.print(out, spacer);

}

////////////////////////////////////////////////////////////
// ScanHeader

TitanData::ScanHeader::ScanHeader()
{
  initialize();
}

void TitanData::ScanHeader::initialize()
{
  time = 0;
  gprops_offset = 0;
  first_offset = 0;
  last_offset = 0;
  min_z = 0.0;
  delta_z = 0.0;
  scan_num = 0;
  nstorms = 0;
  ht_of_freezing = 0.0;
  
  MEM_zero(grid);
  grid.proj_type = Mdvx::PROJ_LATLON;
  grid.nx = 1;
  grid.ny = 1;
  grid.nz = 1;
  grid.dx = 1.0;
  grid.dy = 1.0;
  grid.dz = 1.0;
  grid.dz_constant = 1;
}

void TitanData::ScanHeader::setFromLegacy(const storm_file_scan_header_t &hdr)
{

  time = hdr.time;
  gprops_offset = hdr.gprops_offset;
  first_offset = hdr.first_offset;
  last_offset = hdr.last_offset;
  min_z = hdr.min_z;
  delta_z = hdr.delta_z;
  scan_num = hdr.scan_num;
  nstorms = hdr.nstorms;
  ht_of_freezing = hdr.ht_of_freezing;

  // grid

  grid.proj_origin_lat = hdr.grid.proj_origin_lat;
  grid.proj_origin_lon = hdr.grid.proj_origin_lon;

  grid.minx = hdr.grid.minx;
  grid.miny = hdr.grid.miny;
  grid.minz = hdr.grid.minz;

  grid.dx = hdr.grid.dx;
  grid.dy = hdr.grid.dy;
  grid.dz = hdr.grid.dz;
  grid.dz_constant = hdr.grid.dz_constant;
  
  grid.nx = hdr.grid.nx;
  grid.ny = hdr.grid.ny;
  grid.nz = hdr.grid.nz;

  grid.sensor_x = hdr.grid.sensor_x;
  grid.sensor_y = hdr.grid.sensor_y;
  grid.sensor_z = hdr.grid.sensor_z;
  grid.sensor_lat = hdr.grid.sensor_lat;
  grid.sensor_lon = hdr.grid.sensor_lon;

  STRncopy(grid.unitsx, hdr.grid.unitsx, MDV64_COORD_UNITS_LEN);
  STRncopy(grid.unitsy, hdr.grid.unitsy, MDV64_COORD_UNITS_LEN);
  STRncopy(grid.unitsz, hdr.grid.unitsz, MDV64_COORD_UNITS_LEN);

  switch (hdr.grid.proj_type) {
    case TITAN_PROJ_FLAT:
      grid.proj_type = Mdvx::PROJ_FLAT;
      grid.proj_params.flat.rotation = hdr.grid.proj_params.flat.rotation;
      break;
    case TITAN_PROJ_LAMBERT_CONF:
      grid.proj_type = Mdvx::PROJ_LAMBERT_CONF;
      grid.proj_params.lc2.lat1 = hdr.grid.proj_params.lc2.lat1;
      grid.proj_params.lc2.lat2 = hdr.grid.proj_params.lc2.lat2;
      break;
    case TITAN_PROJ_LATLON:
    default:
      grid.proj_type = Mdvx::PROJ_LATLON;
  }
  
}

storm_file_scan_header_t TitanData::ScanHeader::convertToLegacy() const
{
  storm_file_scan_header_t scan;
  MEM_zero(scan);
  convertToLegacy(scan);
  return scan;
}

void TitanData::ScanHeader::convertToLegacy(storm_file_scan_header_t &hdr) const
{

  hdr.time = time;
  hdr.gprops_offset = gprops_offset;
  hdr.first_offset = first_offset;
  hdr.last_offset = last_offset;
  hdr.min_z = min_z;
  hdr.delta_z = delta_z;
  hdr.scan_num = scan_num;
  hdr.nstorms = nstorms;
  hdr.ht_of_freezing = ht_of_freezing;

  // grid

  hdr.grid.proj_origin_lat = grid.proj_origin_lat;
  hdr.grid.proj_origin_lon = grid.proj_origin_lon;

  hdr.grid.minx = grid.minx;
  hdr.grid.miny = grid.miny;
  hdr.grid.minz = grid.minz;

  hdr.grid.dx = grid.dx;
  hdr.grid.dy = grid.dy;
  hdr.grid.dz = grid.dz;
  hdr.grid.dz_constant = grid.dz_constant;
  
  hdr.grid.nx = grid.nx;
  hdr.grid.ny = grid.ny;
  hdr.grid.nz = grid.nz;

  hdr.grid.sensor_x = grid.sensor_x;
  hdr.grid.sensor_y = grid.sensor_y;
  hdr.grid.sensor_z = grid.sensor_z;
  hdr.grid.sensor_lat = grid.sensor_lat;
  hdr.grid.sensor_lon = grid.sensor_lon;

  STRncopy(hdr.grid.unitsx, grid.unitsx, TITAN_GRID_UNITS_LEN);
  STRncopy(hdr.grid.unitsy, grid.unitsy, TITAN_GRID_UNITS_LEN);
  STRncopy(hdr.grid.unitsz, grid.unitsz, TITAN_GRID_UNITS_LEN);

  switch (grid.proj_type) {
    case Mdvx::PROJ_FLAT:
      hdr.grid.proj_type = TITAN_PROJ_FLAT;
      hdr.grid.proj_params.flat.rotation = grid.proj_params.flat.rotation;
      break;
    case Mdvx::PROJ_LAMBERT_CONF:
      hdr.grid.proj_type = TITAN_PROJ_LAMBERT_CONF;
      hdr.grid.proj_params.lc2.lat1 = grid.proj_params.lc2.lat1;
      hdr.grid.proj_params.lc2.lat2 = grid.proj_params.lc2.lat2;
      break;
    case Mdvx::PROJ_LATLON:
    default:
      hdr.grid.proj_type = TITAN_PROJ_LATLON;
  }
  
}
    
void TitanData::ScanHeader::setFromLegacy(const storm_file_scan_header_t *legacyHdrs,
                                          vector<TitanData::ScanHeader> &scans)
{
  for (size_t ii = 0; ii < scans.size(); ii++) {
    scans[ii].setFromLegacy(legacyHdrs[ii]);
  }
}
    
void TitanData::ScanHeader::convertToLegacy(const vector<TitanData::ScanHeader> &scans,
                                            storm_file_scan_header_t *legacyHdrs)
{
  for (size_t ii = 0; ii < scans.size(); ii++) {
    scans[ii].convertToLegacy(legacyHdrs[ii]);
  }
}
    
void TitanData::ScanHeader::print(FILE *out,
                                  const char *spacer)
     
{
  
  fprintf(out, "%sScan number %d\n", spacer, scan_num);
  fprintf(out, "%snstorms : %d\n", spacer, nstorms);
  fprintf(out, "%sTime: %s\n", spacer, utimestr(udate_time(time)));
  fprintf(out, "%sMin z (km) : %g\n", spacer, min_z);
  fprintf(out, "%sDelta z (km) : %g\n", spacer, delta_z);
  fprintf(out, "%sHeight of freezing (km) : %g\n", spacer, ht_of_freezing);
  fprintf(out, "\n");
  
  printMdvCoord(out, spacer, grid);

  fprintf(out, "\n");
  
}

////////////////////////////////////////////////////////////
// Storm Global Properties

TitanData::StormGprops::StormGprops()
{
  initialize();
}

void TitanData::StormGprops::initialize()
{

  layer_props_offset = 0;
  dbz_hist_offset = 0;
  runs_offset = 0;
  proj_runs_offset = 0;

  vol_centroid_x = 0.0;
  vol_centroid_y = 0.0;
  vol_centroid_z = 0.0;
  
  refl_centroid_x = 0.0;
  refl_centroid_y = 0.0;
  refl_centroid_z = 0.0;
  
  top = 0.0;
  base = 0.0;
  volume = 0.0;
  area_mean = 0.0;
  precip_flux = 0.0;
  mass = 0.0;

  tilt_angle = 0.0;
  tilt_dirn = 0.0;

  dbz_max = 0.0;
  dbz_mean = 0.0;
  dbz_max_gradient = 0.0;
  dbz_mean_gradient = 0.0;
  ht_of_dbz_max = 0.0;

  rad_vel_mean = 0.0;
  rad_vel_sd = 0.0;
  vorticity = 0.0;

  precip_area = 0.0;
  precip_area_centroid_x = 0.0;
  precip_area_centroid_y = 0.0;
  precip_area_orientation = 0.0;
  precip_area_minor_radius = 0.0;
  precip_area_major_radius = 0.0;
    
  proj_area = 0.0;
  proj_area_centroid_x = 0.0;
  proj_area_centroid_y = 0.0;
  proj_area_orientation = 0.0;
  proj_area_minor_radius = 0.0;
  proj_area_major_radius = 0.0;

  MEM_zero(proj_area_polygon);

  storm_num = 0;
  n_layers = 0;
  base_layer = 0;
  n_dbz_intervals = 0;
  n_runs = 0;
  n_proj_runs = 0;
  top_missing = 0;
  range_limited = 0;
  second_trip = 0;
  hail_present = 0;
  anom_prop = 0;
  bounding_min_ix = 0;
  bounding_min_iy = 0;
  bounding_max_ix = 0;
  bounding_max_iy = 0;
  
  vil_from_maxz = 0.0;
  ltg_count = 0.0;
  convectivity_median = 0.0;
  
  hailFOKRcategory = 0;
  hailWaldvogelProb = 0.0;
  hailMassAloft = 0.0;
  hailVertIntgMass = 0.0;
  
  hda_poh = 0.0;
  hda_shi = 0.0;
  hda_posh = 0.0;
  hda_mehs = 0.0;

}

void TitanData::StormGprops::setFromLegacy(const storm_file_params_t &params,
                                           const storm_file_global_props_t &gprops)
{

  layer_props_offset = gprops.layer_props_offset;
  dbz_hist_offset = gprops.dbz_hist_offset;
  runs_offset = gprops.runs_offset;
  proj_runs_offset = gprops.proj_runs_offset;

  vol_centroid_x = gprops.vol_centroid_x;
  vol_centroid_y = gprops.vol_centroid_y;
  vol_centroid_z = gprops.vol_centroid_z;
  
  refl_centroid_x = gprops.refl_centroid_x;
  refl_centroid_y = gprops.refl_centroid_y;
  refl_centroid_z = gprops.refl_centroid_z;
  
  top = gprops.top;
  base = gprops.base;
  volume = gprops.volume;
  area_mean = gprops.area_mean;
  precip_flux = gprops.precip_flux;
  mass = gprops.mass;

  tilt_angle = gprops.tilt_angle;
  tilt_dirn = gprops.tilt_dirn;

  dbz_max = gprops.dbz_max;
  dbz_mean = gprops.dbz_mean;
  dbz_max_gradient = gprops.dbz_max_gradient;
  dbz_mean_gradient = gprops.dbz_mean_gradient;
  ht_of_dbz_max = gprops.ht_of_dbz_max;

  rad_vel_mean = gprops.rad_vel_mean;
  rad_vel_sd = gprops.rad_vel_sd;
  vorticity = gprops.vorticity;

  precip_area = gprops.precip_area;
  precip_area_centroid_x = gprops.precip_area_centroid_x;
  precip_area_centroid_y = gprops.precip_area_centroid_y;
  precip_area_orientation = gprops.precip_area_orientation;
  precip_area_minor_radius = gprops.precip_area_minor_radius;
  precip_area_major_radius = gprops.precip_area_major_radius;
    
  proj_area = gprops.proj_area;
  proj_area_centroid_x = gprops.proj_area_centroid_x;
  proj_area_centroid_y = gprops.proj_area_centroid_y;
  proj_area_orientation = gprops.proj_area_orientation;
  proj_area_minor_radius = gprops.proj_area_minor_radius;
  proj_area_major_radius = gprops.proj_area_major_radius;

  assert(GPROPS_N_POLY_SIDES == N_POLY_SIDES);
  memcpy(proj_area_polygon, gprops.proj_area_polygon, N_POLY_SIDES * sizeof(si32));

  storm_num = gprops.storm_num;
  n_layers = gprops.n_layers;
  base_layer = gprops.base_layer;
  n_dbz_intervals = gprops.n_dbz_intervals;
  n_runs = gprops.n_runs;
  n_proj_runs = gprops.n_proj_runs;
  top_missing = gprops.top_missing;
  range_limited = gprops.range_limited;
  second_trip = gprops.second_trip;
  hail_present = gprops.hail_present;
  anom_prop = gprops.anom_prop;
  bounding_min_ix = gprops.bounding_min_ix;
  bounding_min_iy = gprops.bounding_min_iy;
  bounding_max_ix = gprops.bounding_max_ix;
  bounding_max_iy = gprops.bounding_max_iy;
  
  vil_from_maxz = gprops.vil_from_maxz;
  ltg_count = gprops.ltg_count;
  convectivity_median = gprops.convectivity_median;

  hailFOKRcategory = 0;
  hailWaldvogelProb = 0.0;
  hailMassAloft = 0.0;
  hailVertIntgMass = 0.0;
  hda_poh = 0.0;
  hda_shi = 0.0;
  hda_posh = 0.0;
  hda_mehs = 0.0;

  if (params.gprops_union_type == UNION_HAIL) {
    hailFOKRcategory = gprops.add_on.hail_metrics.FOKRcategory;
    hailWaldvogelProb = gprops.add_on.hail_metrics.waldvogelProbability;
    hailMassAloft = gprops.add_on.hail_metrics.hailMassAloft;
    hailVertIntgMass = gprops.add_on.hail_metrics.vihm;
  } else if (params.gprops_union_type == UNION_NEXRAD_HDA) {
    hda_poh = gprops.add_on.hda.poh;
    hda_shi = gprops.add_on.hda.shi;
    hda_posh = gprops.add_on.hda.posh;
    hda_mehs = gprops.add_on.hda.mehs;
  }

}

storm_file_global_props_t TitanData::StormGprops::convertToLegacy() const
{
  storm_file_global_props_t gprops;
  MEM_zero(gprops);
  convertToLegacy(gprops);
  return gprops;
}

void TitanData::StormGprops::convertToLegacy(storm_file_global_props_t &gprops) const
{

  gprops.layer_props_offset = layer_props_offset;
  gprops.dbz_hist_offset = dbz_hist_offset;
  gprops.runs_offset = runs_offset;
  gprops.proj_runs_offset = proj_runs_offset;

  gprops.vol_centroid_x = vol_centroid_x;
  gprops.vol_centroid_y = vol_centroid_y;
  gprops.vol_centroid_z = vol_centroid_z;
  
  gprops.refl_centroid_x = refl_centroid_x;
  gprops.refl_centroid_y = refl_centroid_y;
  gprops.refl_centroid_z = refl_centroid_z;
  
  gprops.top = top;
  gprops.base = base;
  gprops.volume = volume;
  gprops.area_mean = area_mean;
  gprops.precip_flux = precip_flux;
  gprops.mass = mass;

  gprops.tilt_angle = tilt_angle;
  gprops.tilt_dirn = tilt_dirn;

  gprops.dbz_max = dbz_max;
  gprops.dbz_mean = dbz_mean;
  gprops.dbz_max_gradient = dbz_max_gradient;
  gprops.dbz_mean_gradient = dbz_mean_gradient;
  gprops.ht_of_dbz_max = ht_of_dbz_max;

  gprops.rad_vel_mean = rad_vel_mean;
  gprops.rad_vel_sd = rad_vel_sd;
  gprops.vorticity = vorticity;

  gprops.precip_area = precip_area;
  gprops.precip_area_centroid_x = precip_area_centroid_x;
  gprops.precip_area_centroid_y = precip_area_centroid_y;
  gprops.precip_area_orientation = precip_area_orientation;
  gprops.precip_area_minor_radius = precip_area_minor_radius;
  gprops.precip_area_major_radius = precip_area_major_radius;
    
  gprops.proj_area = proj_area;
  gprops.proj_area_centroid_x = proj_area_centroid_x;
  gprops.proj_area_centroid_y = proj_area_centroid_y;
  gprops.proj_area_orientation = proj_area_orientation;
  gprops.proj_area_minor_radius = proj_area_minor_radius;
  gprops.proj_area_major_radius = proj_area_major_radius;

  assert(GPROPS_N_POLY_SIDES == N_POLY_SIDES);
  memcpy(gprops.proj_area_polygon, proj_area_polygon, N_POLY_SIDES * sizeof(si32));

  gprops.storm_num = storm_num;
  gprops.n_layers = n_layers;
  gprops.base_layer = base_layer;
  gprops.n_dbz_intervals = n_dbz_intervals;
  gprops.n_runs = n_runs;
  gprops.n_proj_runs = n_proj_runs;
  gprops.top_missing = top_missing;
  gprops.range_limited = range_limited;
  gprops.second_trip = second_trip;
  gprops.hail_present = hail_present;
  gprops.anom_prop = anom_prop;
  gprops.bounding_min_ix = bounding_min_ix;
  gprops.bounding_min_iy = bounding_min_iy;
  gprops.bounding_max_ix = bounding_max_ix;
  gprops.bounding_max_iy = bounding_max_iy;
  
  gprops.vil_from_maxz = vil_from_maxz;
  gprops.ltg_count = ltg_count;
  gprops.convectivity_median = convectivity_median;

  if (hailFOKRcategory != 0 ||
      hailWaldvogelProb != 0.0 ||
      hailMassAloft != 0.0 ||
      hailVertIntgMass != 0.0) {
    gprops.add_on.hail_metrics.FOKRcategory = hailFOKRcategory;
    gprops.add_on.hail_metrics.waldvogelProbability = hailWaldvogelProb;
    gprops.add_on.hail_metrics.hailMassAloft = hailMassAloft;
    gprops.add_on.hail_metrics.vihm = hailVertIntgMass;
  } else {
    gprops.add_on.hda.poh = hda_poh;
    gprops.add_on.hda.shi = hda_shi;
    gprops.add_on.hda.posh = hda_posh;
    gprops.add_on.hda.mehs = hda_mehs;
  }
  
}

void TitanData::StormGprops::setFromLegacy(const storm_file_params_t &params,
                                           const storm_file_global_props_t *legacyGprops,
                                           vector<TitanData::StormGprops> &gprops)
{
  for (size_t ii = 0; ii < gprops.size(); ii++) {
    gprops[ii].setFromLegacy(params, legacyGprops[ii]);
  }
}

void TitanData::StormGprops::convertToLegacy(const vector<TitanData::StormGprops> &gprops,
                                             storm_file_global_props_t *legacyGprops)
{
  for (size_t ii = 0; ii < gprops.size(); ii++) {
    gprops[ii].convertToLegacy(legacyGprops[ii]);
  }
}

void TitanData::StormGprops::print(FILE *out,
                                   const char *spacer,
                                   const StormParams &params,
                                   const ScanHeader &scan)
     
{

  const char *loc_label;
  si32 i;
  
  if (scan.grid.proj_type == TITAN_PROJ_FLAT) {
    loc_label = "(km) ";
  } else {
    loc_label = "(deg)";
  }

  fprintf(out, "%sGLOBAL STORM PROPERTIES - storm number %d\n", spacer, storm_num);
  
  fprintf(out, "%s  number of layers                : %d\n", spacer, n_layers);
  fprintf(out, "%s  base layer number               : %d\n", spacer, base_layer);
  fprintf(out, "%s  number of dbz intervals         : %d\n", spacer, n_dbz_intervals);
  fprintf(out, "%s  number of runs                  : %ld\n", spacer, n_runs);
  fprintf(out, "%s  number of proj runs             : %ld\n", spacer, n_proj_runs);
  
  fprintf(out, "%s  range_limited                   : %d\n", spacer, range_limited);
  fprintf(out, "%s  top_missing                     : %d\n", spacer, top_missing);
  fprintf(out, "%s  second_trip                     : %d\n", spacer, second_trip);
  fprintf(out, "%s  hail_present                    : %d\n", spacer, hail_present);
  fprintf(out, "%s  anom_prop                       : %d\n", spacer, anom_prop);
  
  fprintf(out, "%s  vol_centroid_x %s            : %g\n", spacer, loc_label, vol_centroid_x);
  fprintf(out, "%s  vol_centroid_y %s            : %g\n", spacer, loc_label, vol_centroid_y);
  fprintf(out, "%s  vol_centroid_z (km)             : %g\n", spacer, vol_centroid_z);
  
  fprintf(out, "%s  refl_centroid_x %s           : %g\n", spacer, loc_label, refl_centroid_x);
  fprintf(out, "%s  refl_centroid_y %s           : %g\n", spacer, loc_label, refl_centroid_y);
  fprintf(out, "%s  refl_centroid_z (km)            : %g\n", spacer, refl_centroid_z);
  
  fprintf(out, "%s  top (km)                        : %g\n", spacer, top);
  fprintf(out, "%s  base (km)                       : %g\n", spacer, base);
  fprintf(out, "%s  volume (km3)                    : %g\n", spacer, volume);
  fprintf(out, "%s  mean area (km2)                 : %g\n", spacer, area_mean);
  fprintf(out, "%s  precip flux (m3/s)              : %g\n", spacer, precip_flux);
  fprintf(out, "%s  mass (ktons)                    : %g\n", spacer, mass);
  fprintf(out, "%s  vil_from_maxz(kg/m2)            : %g\n", spacer, vil_from_maxz);
  
  fprintf(out, "%s  vihm_from_maxz (kg/m2)          : %g\n", spacer, hailVertIntgMass);
  fprintf(out, "%s  Hail mass aloft (ktons)         : %g\n", spacer, hailMassAloft);
  fprintf(out, "%s  Waldvogel probability of hail   : %g\n", spacer, hailWaldvogelProb);
  fprintf(out, "%s  FOKR storm category             : %d\n", spacer, hailFOKRcategory);
  fprintf(out, "%s  POH (%%)                        : %g\n", spacer, hda_poh);
  fprintf(out, "%s  SHI (Jm-1s-1)                   : %g\n", spacer, hda_shi);
  fprintf(out, "%s  POSH (%%)                       : %g\n", spacer, hda_posh);
  fprintf(out, "%s  MEHS (mm)                       : %g\n", spacer, hda_mehs);
  
  fprintf(out, "%s  tilt angle (deg)                : %g\n", spacer, tilt_angle);
  fprintf(out, "%s  tilt direction (deg)            : %g\n", spacer, tilt_dirn);
  
  fprintf(out, "%s  dbz max                         : %g\n", spacer, dbz_max);
  fprintf(out, "%s  dbz mean                        : %g\n", spacer, dbz_mean);
  fprintf(out, "%s  dbz max gradient                : %g\n", spacer, dbz_max_gradient);
  fprintf(out, "%s  dbz mean gradient               : %g\n", spacer, dbz_mean_gradient);
  fprintf(out, "%s  height of max dbz               : %g\n", spacer, ht_of_dbz_max);
  
  fprintf(out, "%s  rad_vel_mean (m/s)              : %g\n", spacer, rad_vel_mean);
  fprintf(out, "%s  rad_vel_sd (m/s)                : %g\n", spacer, rad_vel_sd);
  fprintf(out, "%s  vorticity (/s)                  : %.2e\n", spacer, vorticity);
  fprintf(out, "%s  convectivity_median             : %.2e\n", spacer, convectivity_median);

  fprintf(out, "%s  precip area (km2)               : %g\n", spacer, precip_area);
  fprintf(out, "%s  precip area centroid x %s       : %g\n",
          spacer, loc_label, precip_area_centroid_x);
  fprintf(out, "%s  precip area centroid y %s    : %g\n",
          spacer, loc_label, precip_area_centroid_y);
  fprintf(out, "%s  precip area orientation (deg)   : %g\n", spacer, precip_area_orientation);
  fprintf(out, "%s  precip area minor radius %s  : %g\n",
          spacer, loc_label, precip_area_minor_radius);
  fprintf(out, "%s  precip area major radius %s  : %g\n",
          spacer, loc_label, precip_area_major_radius);
  
  fprintf(out, "%s  proj. area (km2)                : %g\n", spacer, proj_area);
  fprintf(out, "%s  proj. area centroid x %s        : %g\n", spacer, loc_label, proj_area_centroid_x);
  fprintf(out, "%s  proj. area centroid y %s        : %g\n", spacer, loc_label, proj_area_centroid_y);
  fprintf(out, "%s  proj. area orientation (deg)    : %g\n", spacer, proj_area_orientation);
  fprintf(out, "%s  proj. area minor radius %s      : %g\n",
          spacer, loc_label, proj_area_minor_radius);
  fprintf(out, "%s  proj. area major radius %s      : %g\n",
          spacer, loc_label, proj_area_major_radius);
  
  fprintf(out, "%s  bounding_min_ix                 : %d\n", spacer, bounding_min_ix);
  fprintf(out, "%s  bounding_min_iy                 : %d\n", spacer, bounding_min_iy);
  fprintf(out, "%s  bounding_max_ix                 : %d\n", spacer, bounding_max_ix);
  fprintf(out, "%s  bounding_max_iy                 : %d\n", spacer, bounding_max_iy);
  
  fprintf(out, "%s\n    Proj. area polygon rays:\n", spacer);
  for (i = 0; i < N_POLY_SIDES; i++) {
    fprintf(out, "%s    side %d : %g\n", spacer, i, proj_area_polygon[i]);
  }
  
  fprintf(out, "\n");

}

////////////////////////////////////////////////////////////
// Storm Layer Properties

TitanData::StormLprops::StormLprops()
{
  initialize();
}

void TitanData::StormLprops::initialize()
{
  vol_centroid_x = 0.0;
  vol_centroid_y = 0.0;
  refl_centroid_x = 0.0;
  refl_centroid_y = 0.0;
  area = 0.0;
  dbz_max = 0.0;
  dbz_mean = 0.0;
  rad_vel_mean = 0.0;
  rad_vel_sd = 0.0;
  vorticity = 0.0;
  convectivity_median = 0.0;
}

void TitanData::StormLprops::setFromLegacy(const storm_file_layer_props_t &lprops)
{

  vol_centroid_x = lprops.vol_centroid_x;
  vol_centroid_y = lprops.vol_centroid_y;
  refl_centroid_x = lprops.refl_centroid_x;
  refl_centroid_y = lprops.refl_centroid_y;
  
  area = lprops.area;
  dbz_max = lprops.dbz_max;
  dbz_mean = lprops.dbz_mean;
  mass = lprops.mass;

  rad_vel_mean = lprops.rad_vel_mean;
  rad_vel_sd = lprops.rad_vel_sd;
  vorticity = lprops.vorticity;
  convectivity_median = lprops.convectivity_median;

}

storm_file_layer_props_t TitanData::StormLprops::convertToLegacy() const
{
  storm_file_layer_props_t lprops;
  MEM_zero(lprops);
  convertToLegacy(lprops);
  return lprops;
}

void TitanData::StormLprops::convertToLegacy(storm_file_layer_props_t &lprops) const
{

  lprops.vol_centroid_x = vol_centroid_x;
  lprops.vol_centroid_y = vol_centroid_y;
  lprops.refl_centroid_x = refl_centroid_x;
  lprops.refl_centroid_y = refl_centroid_y;
  
  lprops.area = area;
  lprops.dbz_max = dbz_max;
  lprops.dbz_mean = dbz_mean;
  lprops.mass = mass;

  lprops.rad_vel_mean = rad_vel_mean;
  lprops.rad_vel_sd = rad_vel_sd;
  lprops.vorticity = vorticity;
  lprops.convectivity_median = convectivity_median;

}

void TitanData::StormLprops::setFromLegacy(const storm_file_layer_props_t *legacyLprops,
                                           vector<TitanData::StormLprops> &lprops)
{
  for (size_t ii = 0; ii < lprops.size(); ii++) {
    lprops[ii].setFromLegacy(legacyLprops[ii]);
  }
}

void TitanData::StormLprops::convertToLegacy(const vector<TitanData::StormLprops> &lprops,
                                             storm_file_layer_props_t *legacyLprops)
{
  for (size_t ii = 0; ii < lprops.size(); ii++) {
    lprops[ii].convertToLegacy(legacyLprops[ii]);
  }
}
    
void TitanData::StormLprops::print(FILE *out,
                                   const char *spacer,
                                   const ScanHeader &scan,
                                   const StormGprops &gprops,
                                   const vector<StormLprops> &lprops)
  
{

  if (gprops.n_layers == 0) {
    return;
  }
  const char *loc_label;
  if (scan.grid.proj_type == TITAN_PROJ_FLAT) {
    loc_label = "(km) ";
  } else {
    loc_label = "(deg)";
  }

  /*
   * layer properties
   */
  
  fprintf(out, "%s%5s %5s %7s %7s %7s %7s %6s %6s %7s\n",
	  spacer,
	  "Layer", "z", "x-cent", "y-cent", "x-Zcent", "y-Zcent", "area",
	  "max Z", "mean Z");
  fprintf(out, "%s%5s %5s %7s %7s %7s %7s %6s %6s %7s\n",
	  spacer, " ", "(km)", loc_label, loc_label, loc_label, loc_label,
	  "(km2)", "(dbz)", "(dbz)");
  
  for (int ilayer = gprops.base_layer, ii = 0;
       ilayer < (gprops.base_layer + gprops.n_layers);
       ilayer++, ii++) {
    
    fprintf(out, "%s%5d %5g %7.1f %7.1f %7.1f %7.1f "
	    "%6.1f %6.1f %7.1f\n",
	    spacer, ilayer,
	    (scan.min_z + (double) ilayer * scan.delta_z),
	    lprops[ii].vol_centroid_x,
	    lprops[ii].vol_centroid_y,
	    lprops[ii].refl_centroid_x,
	    lprops[ii].refl_centroid_y,
	    lprops[ii].area,
	    lprops[ii].dbz_max,
	    lprops[ii].dbz_mean);
    
  } /* ilayer */
  
  fprintf(out, "\n");
  
  fprintf(out, "%s%5s %5s %7s %9s %7s %12s %15s\n", spacer,
	  "Layer", "z", "mass", "rvel_mean", "rvel_sd", "vorticity", "convectivity");
  fprintf(out, "%s%5s %5s %7s %9s %7s %12s %15s\n",
	  spacer, " ", "(km)", "(ktons)", "(m/s)", "(m/s)", "(/s)", " ");
  
  for (int ilayer = gprops.base_layer, ii = 0;
       ilayer < (gprops.base_layer + gprops.n_layers);
       ilayer++, ii++) {
    
    fprintf(out, "%s%5d %5g %7.1f %9.2f %7.2f %12.2e %15.2e\n",
	    spacer, ilayer,
	    (scan.min_z + (double) ilayer * scan.delta_z),
	    lprops[ii].mass,
	    lprops[ii].rad_vel_mean,
	    lprops[ii].rad_vel_sd,
	    lprops[ii].vorticity,
            lprops[ii].convectivity_median);
    
  } /* ilayer */
  
  fprintf(out, "\n");
  
  return;

}

////////////////////////////////////////////////////////////
// Storm Dbz Histograms

TitanData::StormDbzHist::StormDbzHist()
{
  initialize();
}

void TitanData::StormDbzHist::initialize()
{
  percent_volume = 0.0;
  percent_area = 0.0;
}

void TitanData::StormDbzHist::setFromLegacy(const storm_file_dbz_hist_t &hist)
{

  percent_volume = hist.percent_volume;
  percent_area = hist.percent_area;

}

storm_file_dbz_hist_t TitanData::StormDbzHist::convertToLegacy() const
{
  storm_file_dbz_hist_t hist;
  MEM_zero(hist);
  convertToLegacy(hist);
  return hist;
}

void TitanData::StormDbzHist::convertToLegacy(storm_file_dbz_hist_t &hist) const
{

  hist.percent_volume = percent_volume;
  hist.percent_area = percent_area;
  
}

void TitanData::StormDbzHist::setFromLegacy(const storm_file_dbz_hist_t *legacyHist,
                                            vector<TitanData::StormDbzHist> &hist)
{
  for (size_t ii = 0; ii < hist.size(); ii++) {
    hist[ii].setFromLegacy(legacyHist[ii]);
  }
}

void TitanData::StormDbzHist::convertToLegacy(const vector<TitanData::StormDbzHist> &hist,
                                              storm_file_dbz_hist_t *legacyHist)
{
  for (size_t ii = 0; ii < hist.size(); ii++) {
    hist[ii].convertToLegacy(legacyHist[ii]);
  }
}
    
/*-----------------------------
 */

void TitanData::StormDbzHist::print(FILE *out,
                                    const char *spacer,
                                    const StormParams &params,
                                    const StormGprops &gprops,
                                    const vector<StormDbzHist> &hist)
     
{

  if (hist.size() == 0) {
    return;
  }
    
  fprintf(out, "%sDbz histograms : \n", spacer);
  fprintf(out, "%s%10s %10s %12s %12s\n", spacer,
	  "Low dbz", "High Dbz", "% volume", "% area");
  
  for (size_t ii = 0; ii < hist.size(); ii++) {
    
    fprintf(out, "%s%10.1f %10.1f %12.2f %12.2f\n", spacer,
	    (params.low_dbz_threshold + (double) ii * params.dbz_hist_interval),
	    (params.low_dbz_threshold + (double) (ii + 1) * params.dbz_hist_interval),
	    hist[ii].percent_volume,
	    hist[ii].percent_area);
    
  } // ii

  fprintf(out, "\n");

  return;

}

////////////////////////////////////////////////////////////
// Storm Run - contiguous grid cells in a row

TitanData::StormRun::StormRun()
{
  initialize();
}
  
void TitanData::StormRun::initialize()
{
  run_ix = 0;
  run_iy = 0;
  run_iz = 0;
  run_len = 0;
}

void TitanData::StormRun::setFromLegacy(const storm_file_run_t &run)
{

  run_ix = run.ix;
  run_iy = run.iy;
  run_iz = run.iz;
  run_len = run.n;

}

storm_file_run_t TitanData::StormRun::convertToLegacy() const
{
  storm_file_run_t run;
  MEM_zero(run);
  convertToLegacy(run);
  return run;
}

void TitanData::StormRun::convertToLegacy(storm_file_run_t &run) const
{

  run.ix = run_ix;
  run.iy = run_iy;
  run.iz = run_iz;
  run.n = run_len;
  
}

void TitanData::StormRun::setFromLegacy(const storm_file_run_t *legacyRun,
                                        vector<TitanData::StormRun> &run)
{
  for (size_t ii = 0; ii < run.size(); ii++) {
    run[ii].setFromLegacy(legacyRun[ii]);
  }
}

void TitanData::StormRun::convertToLegacy(const vector<TitanData::StormRun> &run,
                                              storm_file_run_t *legacyRun)
{
  for (size_t ii = 0; ii < run.size(); ii++) {
    run[ii].convertToLegacy(legacyRun[ii]);
  }
}
    
void TitanData::StormRun::print(FILE *out,
                                const char *spacer,
                                const char *label,
                                const StormGprops &gprops,
                                const vector<StormRun> &runs)
  
{
  
  if (runs.size() == 0) {
    return;
  }
  
  int npts = 0;
  for (size_t ii = 0; ii < runs.size(); ii++) {
    npts += runs[ii].run_len;
  }

  fprintf(out, "%s%s - nGridPts: %d\n", spacer, label, npts);
  fprintf(out, "%s%8s %8s %8s %8s %8s\n", spacer,
	  "ix", "n", "maxx", "iy", "iz");
  
  for (size_t ii = 0; ii < runs.size(); ii++) {
    fprintf(out, "%s%8d %8d %8d %8d %8d",
	    spacer,
	    runs[ii].run_ix, runs[ii].run_len,
	    (runs[ii].run_ix + runs[ii].run_len - 1),
	    runs[ii].run_iy, runs[ii].run_iz);
    if (runs[ii].run_ix < gprops.bounding_min_ix ||
	(runs[ii].run_ix + runs[ii].run_len - 1) > gprops.bounding_max_ix ||
	runs[ii].run_iy < gprops.bounding_min_iy ||
	runs[ii].run_iy > gprops.bounding_max_iy) {
      fprintf(out, "\a *** Bounds exceeded");
    }
    fprintf(out, "\n");
  } // ii
  
  fprintf(out, "\n");

  return;
  
}

////////////////////////////////////////////////////////////
// track forecast properties

TitanData::TrackFcastProps::TrackFcastProps()
{
  initialize();
}
  
void TitanData::TrackFcastProps::initialize()
{
  proj_area_centroid_x = 0.0;
  proj_area_centroid_y = 0.0;
  vol_centroid_z = 0.0;
  refl_centroid_z = 0.0;
  top = 0.0;
  dbz_max = 0.0;
  volume = 0.0;
  precip_flux = 0.0;
  mass = 0.0;
  proj_area = 0.0;
  smoothed_proj_area_centroid_x = 0.0;
  smoothed_proj_area_centroid_y = 0.0;
  smoothed_speed = 0.0;
  smoothed_direction = 0.0;
}

void TitanData::TrackFcastProps::setFromLegacy(const track_file_forecast_props_t &fprops)
{

  proj_area_centroid_x = fprops.proj_area_centroid_x;
  proj_area_centroid_y = fprops.proj_area_centroid_y;
  vol_centroid_z = fprops.vol_centroid_z;
  refl_centroid_z = fprops.refl_centroid_z;
  top = fprops.top;
  dbz_max = fprops.dbz_max;
  volume = fprops.volume;
  precip_flux = fprops.precip_flux;
  mass = fprops.mass;
  proj_area = fprops.proj_area;
  smoothed_proj_area_centroid_x = fprops.smoothed_proj_area_centroid_x;
  smoothed_proj_area_centroid_y = fprops.smoothed_proj_area_centroid_y;
  smoothed_speed = fprops.smoothed_speed;
  smoothed_direction = fprops.smoothed_direction;

}

track_file_forecast_props_t TitanData::TrackFcastProps::convertToLegacy() const
{
  track_file_forecast_props_t fprops;
  MEM_zero(fprops);
  convertToLegacy(fprops);
  return fprops;
}

void TitanData::TrackFcastProps::convertToLegacy(track_file_forecast_props_t &fprops) const
{

  fprops.proj_area_centroid_x = proj_area_centroid_x;
  fprops.proj_area_centroid_y = proj_area_centroid_y;
  fprops.vol_centroid_z = vol_centroid_z;
  fprops.refl_centroid_z = refl_centroid_z;
  fprops.top = top;
  fprops.dbz_max = dbz_max;
  fprops.volume = volume;
  fprops.precip_flux = precip_flux;
  fprops.mass = mass;
  fprops.proj_area = proj_area;
  fprops.smoothed_proj_area_centroid_x = smoothed_proj_area_centroid_x;
  fprops.smoothed_proj_area_centroid_y = smoothed_proj_area_centroid_y;
  fprops.smoothed_speed = smoothed_speed;
  fprops.smoothed_direction = smoothed_direction;
  
}

void TitanData::TrackFcastProps::print(FILE *out,
                                       const char *label,
                                       const char *space)
  
{
  
  fprintf(out, "%s%s\n", space, label);
  fprintf(out, "  %sProj_Area_centroid_x : %g\n", space, proj_area_centroid_x);
  fprintf(out, "  %sProj_Area_centroid_y : %g\n", space, proj_area_centroid_y);
  fprintf(out, "  %sVol_centroid_z : %g\n", space, vol_centroid_z);
  fprintf(out, "  %sRefl_centroid_z : %g\n", space, refl_centroid_z);
  fprintf(out, "  %sDbz_max : %g\n", space, dbz_max);
  fprintf(out, "  %sTop : %g\n", space, top);
  fprintf(out, "  %sVolume : %g\n", space, volume);
  fprintf(out, "  %sPrecip_flux : %g\n", space, precip_flux);
  fprintf(out, "  %sMass : %g\n", space, mass);
  fprintf(out, "  %sProj_Area : %g\n", space, proj_area);
  fprintf(out, "  %sSmoothed_proj_area_centroid_x : %g\n", space, smoothed_proj_area_centroid_x);
  fprintf(out, "  %sSmoothed_proj_area_centroid_y : %g\n", space, smoothed_proj_area_centroid_y);
  fprintf(out, "  %sSpeed : %g\n", space, smoothed_speed);
  fprintf(out, "  %sDirection : %g\n", space, smoothed_direction);
  fprintf(out, "\n");

  return;

}

////////////////////////////////////////////////////////////
// track forecast verification

TitanData::TrackVerify::TrackVerify()
{
  initialize();
}
  
void TitanData::TrackVerify::initialize()
{
  end_time = 0;
  verification_performed = false;
  forecast_lead_time = 0;
  forecast_lead_time_margin = 0;
  forecast_min_history = 0;
  verify_before_forecast_time = false;
  verify_after_track_dies = false;
  MEM_zero(grid);
  grid.proj_type = Mdvx::PROJ_LATLON;
  grid.nx = 1;
  grid.ny = 1;
  grid.nz = 1;
  grid.dx = 1.0;
  grid.dy = 1.0;
  grid.dz = 1.0;
  grid.dz_constant = 1;
}

void TitanData::TrackVerify::setFromLegacy(const track_file_verify_t &verify)
{

  end_time = verify.end_time;
  verification_performed = verify.verification_performed;
  forecast_lead_time = verify.forecast_lead_time;
  forecast_lead_time_margin = verify.forecast_lead_time_margin;
  forecast_min_history = verify.forecast_min_history;
  verify_before_forecast_time = verify.verify_before_forecast_time;
  verify_after_track_dies = verify.verify_after_track_dies;
  
  // grid

  grid.proj_origin_lat = verify.grid.proj_origin_lat;
  grid.proj_origin_lon = verify.grid.proj_origin_lon;

  grid.minx = verify.grid.minx;
  grid.miny = verify.grid.miny;
  grid.minz = verify.grid.minz;

  grid.dx = verify.grid.dx;
  grid.dy = verify.grid.dy;
  grid.dz = verify.grid.dz;
  grid.dz_constant = verify.grid.dz_constant;
  
  grid.nx = verify.grid.nx;
  grid.ny = verify.grid.ny;
  grid.nz = verify.grid.nz;

  grid.sensor_x = verify.grid.sensor_x;
  grid.sensor_y = verify.grid.sensor_y;
  grid.sensor_z = verify.grid.sensor_z;
  grid.sensor_lat = verify.grid.sensor_lat;
  grid.sensor_lon = verify.grid.sensor_lon;

  STRncopy(grid.unitsx, verify.grid.unitsx, MDV64_COORD_UNITS_LEN);
  STRncopy(grid.unitsy, verify.grid.unitsy, MDV64_COORD_UNITS_LEN);
  STRncopy(grid.unitsz, verify.grid.unitsz, MDV64_COORD_UNITS_LEN);

  switch (verify.grid.proj_type) {
    case TITAN_PROJ_FLAT:
      grid.proj_type = Mdvx::PROJ_FLAT;
      grid.proj_params.flat.rotation = verify.grid.proj_params.flat.rotation;
      break;
    case TITAN_PROJ_LAMBERT_CONF:
      grid.proj_type = Mdvx::PROJ_LAMBERT_CONF;
      grid.proj_params.lc2.lat1 = verify.grid.proj_params.lc2.lat1;
      grid.proj_params.lc2.lat2 = verify.grid.proj_params.lc2.lat2;
      break;
    case TITAN_PROJ_LATLON:
    default:
      grid.proj_type = Mdvx::PROJ_LATLON;
  }
}

track_file_verify_t TitanData::TrackVerify::convertToLegacy() const
{
  track_file_verify_t verify;
  MEM_zero(verify);
  convertToLegacy(verify);
  return verify;
}

void TitanData::TrackVerify::convertToLegacy(track_file_verify_t &verify) const
{

  verify.end_time = end_time;
  verify.verification_performed = verification_performed;
  verify.forecast_lead_time = forecast_lead_time;
  verify.forecast_lead_time_margin = forecast_lead_time_margin;
  verify.forecast_min_history = forecast_min_history;
  verify.verify_before_forecast_time = verify_before_forecast_time;
  verify.verify_after_track_dies = verify_after_track_dies;
  
  // grid

  verify.grid.proj_origin_lat = grid.proj_origin_lat;
  verify.grid.proj_origin_lon = grid.proj_origin_lon;

  verify.grid.minx = grid.minx;
  verify.grid.miny = grid.miny;
  verify.grid.minz = grid.minz;

  verify.grid.dx = grid.dx;
  verify.grid.dy = grid.dy;
  verify.grid.dz = grid.dz;
  verify.grid.dz_constant = grid.dz_constant;
  
  verify.grid.nx = grid.nx;
  verify.grid.ny = grid.ny;
  verify.grid.nz = grid.nz;

  verify.grid.sensor_x = grid.sensor_x;
  verify.grid.sensor_y = grid.sensor_y;
  verify.grid.sensor_z = grid.sensor_z;
  verify.grid.sensor_lat = grid.sensor_lat;
  verify.grid.sensor_lon = grid.sensor_lon;

  STRncopy(verify.grid.unitsx, grid.unitsx, TITAN_GRID_UNITS_LEN);
  STRncopy(verify.grid.unitsy, grid.unitsy, TITAN_GRID_UNITS_LEN);
  STRncopy(verify.grid.unitsz, grid.unitsz, TITAN_GRID_UNITS_LEN);

  switch (grid.proj_type) {
    case Mdvx::PROJ_FLAT:
      verify.grid.proj_type = TITAN_PROJ_FLAT;
      verify.grid.proj_params.flat.rotation = grid.proj_params.flat.rotation;
      break;
    case Mdvx::PROJ_LAMBERT_CONF:
      verify.grid.proj_type = TITAN_PROJ_LAMBERT_CONF;
      verify.grid.proj_params.lc2.lat1 = grid.proj_params.lc2.lat1;
      verify.grid.proj_params.lc2.lat2 = grid.proj_params.lc2.lat2;
      break;
    case Mdvx::PROJ_LATLON:
    default:
      verify.grid.proj_type = TITAN_PROJ_LATLON;
  }
  
}

void TitanData::TrackVerify::print(FILE *out,
                                   const char *spacer,
                                   const char *label)
  
{
  
  fprintf(out, "%sVerification params : %s\n", spacer, label);
  fprintf(out, "%s  forecast_lead_time (secs) : %d\n", spacer, forecast_lead_time);
  fprintf(out, "%s  forecast_lead_time_margin (secs) : %d\n", spacer, forecast_lead_time_margin);
  fprintf(out, "%s  forecast_min_history (secs) : %d\n", spacer, forecast_min_history);
  fprintf(out, "%s  verify_before_forecast_time : %s\n", spacer,
          BOOL_STR(verify_before_forecast_time).c_str());
  fprintf(out, "%s  verify_after_track_dies : %s\n", spacer, BOOL_STR(verify_after_track_dies).c_str());
  
  char spacer2[128];
  snprintf(spacer2, 128, "%s  ", spacer);
  printMdvCoord(out, spacer2, grid);
  
}

////////////////////////////////////////////////////////////
// contingency results for verification

TitanData::TrackContingency::TrackContingency()
{
  initialize();
}
  
void TitanData::TrackContingency::initialize()
{
  n_success = 0.0;
  n_failure = 0.0;
  n_false_alarm = 0.0;
}

void TitanData::TrackContingency::setFromLegacy(const track_file_contingency_data_t &cont)
{

  n_success = cont.n_success;
  n_failure = cont.n_failure;
  n_false_alarm = cont.n_false_alarm;

}

track_file_contingency_data_t TitanData::TrackContingency::convertToLegacy() const
{
  track_file_contingency_data_t cont;
  MEM_zero(cont);
  convertToLegacy(cont);
  return cont;
}

void TitanData::TrackContingency::convertToLegacy(track_file_contingency_data_t &cont) const
{

  cont.n_success = n_success;
  cont.n_failure = n_failure;
  cont.n_false_alarm = n_false_alarm;

}

void TitanData::TrackContingency::print(FILE *out,
                                        const char *label,
                                        const char *spacer)
     
{

  fprintf(out, "\n%s%s\n\n", spacer, label);
  
  fprintf(out, "  %sn_success : %g\n", spacer, n_success);
  fprintf(out, "  %sn_failure : %g\n", spacer,n_failure);
  fprintf(out, "  %sn_false_alarm : %g\n", spacer, n_false_alarm);

  double denom = n_success + n_failure;

  if (denom == 0) {
    fprintf(out, "  %sPOD not computed\n", spacer);
  } else {
    fprintf(out, "  %sPOD = %g\n", spacer, n_success / denom);
  }
  
  denom = n_success + n_false_alarm;

  if (denom == 0) {
    fprintf(out, "  %sFAR not computed\n", spacer);
  } else {
    fprintf(out, "  %sFAR = %g\n", spacer,
	    n_false_alarm / denom);
  }

  denom = n_success + n_failure + n_false_alarm;

  if (denom == 0) {
    fprintf(out, "  %sCSI not computed\n", spacer);
  } else {
    fprintf(out, "  %sCSI = %g\n", spacer,
	    n_success / denom);
  }

}

////////////////////////////////////////////////////////////
// tracking parameters

TitanData::TrackingParams::TrackingParams()
{
  initialize();
}

void TitanData::TrackingParams::initialize()
{
  MEM_zero(forecast_weights);
  weight_distance = 0.0;
  weight_delta_cube_root_volume = 0.0;
  merge_split_search_ratio = 0.0;
  max_tracking_speed = 0.0;
  max_speed_for_valid_forecast = 0.0;
  parabolic_growth_period = 0.0;
  smoothing_radius = 0.0;
  min_fraction_overlap = 0.0;
  min_sum_fraction_overlap = 0.0;
  scale_forecasts_by_history = false;
  use_runs_for_overlaps = false;
  grid_type = 0;
  nweights_forecast = 0;
  forecast_type = 0;
  max_delta_time = 0;
  min_history_for_valid_forecast = 0;
  spatial_smoothing = false;
}

void TitanData::TrackingParams::setFromLegacy(const track_file_params_t &params)
{

  memcpy(forecast_weights, params.forecast_weights, MAX_NWEIGHTS_FCAST * sizeof(fl32));
  weight_distance = params.weight_distance;
  weight_delta_cube_root_volume = params.weight_delta_cube_root_volume;
  merge_split_search_ratio = params.merge_split_search_ratio;
  max_tracking_speed = params.max_tracking_speed;
  max_speed_for_valid_forecast = params.max_speed_for_valid_forecast;
  parabolic_growth_period = params.parabolic_growth_period;
  smoothing_radius = params.smoothing_radius;
  min_fraction_overlap = params.min_fraction_overlap;
  min_sum_fraction_overlap = params.min_sum_fraction_overlap;
  scale_forecasts_by_history = params.scale_forecasts_by_history;
  use_runs_for_overlaps = params.use_runs_for_overlaps;
  grid_type = params.grid_type;
  nweights_forecast = params.nweights_forecast;
  forecast_type = params.forecast_type;
  max_delta_time = params.max_delta_time;
  min_history_for_valid_forecast = params.min_history_for_valid_forecast;
  spatial_smoothing = params.spatial_smoothing;

}

track_file_params_t TitanData::TrackingParams::convertToLegacy() const
{
  track_file_params_t params;
  MEM_zero(params);
  convertToLegacy(params);
  return params;
}

void TitanData::TrackingParams::convertToLegacy(track_file_params_t &params) const
{

  memcpy(params.forecast_weights, forecast_weights, MAX_NWEIGHTS_FCAST * sizeof(fl32));
  params.weight_distance = weight_distance;
  params.weight_delta_cube_root_volume = weight_delta_cube_root_volume;
  params.merge_split_search_ratio = merge_split_search_ratio;
  params.max_tracking_speed = max_tracking_speed;
  params.max_speed_for_valid_forecast = max_speed_for_valid_forecast;
  params.parabolic_growth_period = parabolic_growth_period;
  params.smoothing_radius = smoothing_radius;
  params.min_fraction_overlap = min_fraction_overlap;
  params.min_sum_fraction_overlap = min_sum_fraction_overlap;
  params.scale_forecasts_by_history = scale_forecasts_by_history;
  params.use_runs_for_overlaps = use_runs_for_overlaps;
  params.grid_type = grid_type;
  params.nweights_forecast = nweights_forecast;
  params.forecast_type = forecast_type;
  params.max_delta_time = max_delta_time;
  params.min_history_for_valid_forecast = min_history_for_valid_forecast;
  params.spatial_smoothing = spatial_smoothing;

}

void TitanData::TrackingParams::print(FILE *out,
                                   const char *spacer)
  
{
  
  fprintf(out, "%sTRACKING PARAMETERS:\n", spacer);

  if (grid_type == Mdvx::PROJ_FLAT) {
    fprintf(out, "%s  Gridtype : flat\n", spacer);
  } else if (grid_type == Mdvx::PROJ_LATLON) {
    fprintf(out, "%s  Gridtype : latlon\n", spacer);
  }
  
  fprintf(out, "%s  Nweights_forecast : %d\n", spacer, nweights_forecast);
  fprintf(out, "%s  Forecast type : %s\n", spacer,
	  FORECAST_TYPE_STR((forecast_t) forecast_type).c_str());

  fprintf(out, "%s  Forecast weights :", spacer);
  for (int i = 0; i < nweights_forecast; i++) {
    fprintf(out, " %.2f",
	    forecast_weights[i]);
  }
  fprintf(out, "\n");
  fprintf(out, "%s  Parabolic_growth_period : %g\n", spacer, parabolic_growth_period);
  fprintf(out, "%s  Weight_distance : %g\n", spacer, weight_distance);
  fprintf(out, "%s  Weight_delta_cube_root_volume : %g\n", spacer, weight_delta_cube_root_volume);
  fprintf(out, "%s  Max tracking speed (km/hr) : %g\n", spacer, max_tracking_speed);
  fprintf(out, "%s  Max delta time (secs) : %d\n", spacer, max_delta_time);
  fprintf(out, "%s  Min history for valid forecast (secs) : %d\n", spacer, min_history_for_valid_forecast);
  fprintf(out, "%s  Max speed for valid forecast (km/hr) : %g\n", spacer, max_speed_for_valid_forecast);
  fprintf(out, "%s  Spatial smoothing : %s\n", spacer,
          (spatial_smoothing? "TRUE" : "FALSE"));
  fprintf(out, "%s  Use_runs_for_overlaps : %s\n", spacer,
          (use_runs_for_overlaps? "TRUE" : "FALSE"));
  fprintf(out, "%s  Scale_forecasts_by_history : %s\n", spacer,
          (scale_forecasts_by_history? "TRUE" : "FALSE"));
  fprintf(out, "%s  Smoothing radius (km) : %g\n", spacer, smoothing_radius);
  fprintf(out, "%s  Min_fraction_overlap : %g\n", spacer, min_fraction_overlap);
  fprintf(out, "%s  Min_sum_fraction_overlap : %g\n", spacer, min_sum_fraction_overlap);
  fprintf(out, "\n");

  return;
  
}

////////////////////////////////////////////////////////////
// TrackHeader

TitanData::TrackHeader::TrackHeader()
{
  initialize();
}

void TitanData::TrackHeader::initialize()
{
  file_valid = false;
  modify_code = 0;
  n_simple_tracks = 0;
  n_complex_tracks = 0;
  n_samples_for_forecast_stats = 0;
  n_scans = 0;
  last_scan_num = 0;
  max_simple_track_num = 0;
  max_complex_track_num = 0;
  max_parents = 0;
  max_children = 0;
  max_nweights_forecast = 0;
  params.initialize();
  verify.initialize();
  ellipse_verify.initialize();
  polygon_verify.initialize();
  forecast_bias.initialize();
  forecast_rmse.initialize();
}

void TitanData::TrackHeader::setFromLegacy(const track_file_header_t &hdr)
{

  file_valid = hdr.file_valid;
  modify_code = hdr.modify_code;
  n_simple_tracks = hdr.n_simple_tracks;
  n_complex_tracks = hdr.n_complex_tracks;
  n_samples_for_forecast_stats = hdr.n_samples_for_forecast_stats;
  n_scans = hdr.n_scans;
  last_scan_num = hdr.last_scan_num;
  max_simple_track_num = hdr.max_simple_track_num;
  max_complex_track_num = hdr.max_complex_track_num;
  max_parents = hdr.max_parents;
  max_children = hdr.max_children;
  max_nweights_forecast = hdr.max_nweights_forecast;

  params.setFromLegacy(hdr.params);
  verify.setFromLegacy(hdr.verify);
  ellipse_verify.setFromLegacy(hdr.ellipse_verify);
  polygon_verify.setFromLegacy(hdr.polygon_verify);
  forecast_bias.setFromLegacy(hdr.forecast_bias);
  forecast_rmse.setFromLegacy(hdr.forecast_rmse);
  
}

track_file_header_t TitanData::TrackHeader::convertToLegacy() const
{
  track_file_header_t hdr;
  MEM_zero(hdr);
  convertToLegacy(hdr);
  return hdr;
}

void TitanData::TrackHeader::convertToLegacy(track_file_header_t &hdr) const
{

  hdr.file_valid = file_valid;
  hdr.modify_code = modify_code;
  hdr.n_simple_tracks = n_simple_tracks;
  hdr.n_complex_tracks = n_complex_tracks;
  hdr.n_samples_for_forecast_stats = n_samples_for_forecast_stats;
  hdr.n_scans = n_scans;
  hdr.last_scan_num = last_scan_num;
  hdr.max_simple_track_num = max_simple_track_num;
  hdr.max_complex_track_num = max_complex_track_num;
  hdr.max_parents = max_parents;
  hdr.max_children = max_children;
  hdr.max_nweights_forecast = max_nweights_forecast;
  
  params.convertToLegacy(hdr.params);
  verify.convertToLegacy(hdr.verify);
  ellipse_verify.convertToLegacy(hdr.ellipse_verify);
  polygon_verify.convertToLegacy(hdr.polygon_verify);
  forecast_bias.convertToLegacy(hdr.forecast_bias);
  forecast_rmse.convertToLegacy(hdr.forecast_rmse);
  
}

void TitanData::TrackHeader::print(FILE *out,
                                   const char *spacer)
     
{

  char spacer2[128];
  snprintf(spacer2, 128, "%s  ", spacer);
  
  params.print(out, spacer);
  fprintf(out, "%sTRACK FILE HEADER :\n", spacer);
  fprintf(out, "%s  Number of simple tracks : %d\n", spacer, n_simple_tracks);
  fprintf(out, "%s  Number of complex tracks : %d\n", spacer, n_complex_tracks);
  fprintf(out, "%s  n_samples_for_forecast_stats : %d\n", spacer, n_samples_for_forecast_stats);
  fprintf(out, "%s  n_scans : %d\n", spacer, n_scans);
  fprintf(out, "%s  last_scan_num : %d\n", spacer, last_scan_num);
  fprintf(out, "%s  max_simple_track_num : %d\n", spacer, max_simple_track_num);
  fprintf(out, "%s  max_complex_track_num : %d\n", spacer, max_complex_track_num);
  fprintf(out, "%s  max_parents : %d\n", spacer, max_parents);
  fprintf(out, "%s  max_children : %d\n", spacer, max_children);
  fprintf(out, "%s  max_nweights_forecast : %d\n", spacer, max_nweights_forecast);
  fprintf(out, "\n");
  
  if (verify.verification_performed) {
    ellipse_verify.print(out, "Ellipses", spacer2);
    polygon_verify.print(out, "Polygons", spacer2);
    forecast_bias.print(out, "  Forecast bias", spacer2);
    forecast_rmse.print(out, "  Forecast RMSE", spacer2);
    fprintf(out, "\n");
  }
  
  return;

}
////////////////////////////////////////////////////////////
// simple track parameters

TitanData::SimpleTrackParams::SimpleTrackParams()
{
  initialize();
}

void TitanData::SimpleTrackParams::initialize()
{
  start_time = 0;
  end_time = 0;
  last_descendant_end_time = 0;
  time_origin = 0;
  first_entry_offset = 0;
  simple_track_num = 0;
  complex_track_num = 0;
  last_descendant_simple_track_num = 0;
  start_scan = 0;
  end_scan = 0;
  last_descendant_end_scan = 0;
  scan_origin = 0;
  history_in_scans = 0;
  history_in_secs = 0;
  duration_in_scans = 0;
  duration_in_secs = 0;
  nparents = 0;
  nchildren = 0;
  MEM_zero(parent);
  MEM_zero(child);
}

void TitanData::SimpleTrackParams::setFromLegacy(const simple_track_params_t &params)
{

  start_time = params.start_time;
  end_time = params.end_time;
  last_descendant_end_time = params.last_descendant_end_time;
  time_origin = params.time_origin;
  first_entry_offset = params.first_entry_offset;
  simple_track_num = params.simple_track_num;
  complex_track_num = params.complex_track_num;
  last_descendant_simple_track_num = params.last_descendant_simple_track_num;
  start_scan = params.start_scan;
  end_scan = params.end_scan;
  last_descendant_end_scan = params.last_descendant_end_scan;
  scan_origin = params.scan_origin;
  history_in_scans = params.history_in_scans;
  history_in_secs = params.history_in_secs;
  duration_in_scans = params.duration_in_scans;
  duration_in_secs = params.duration_in_secs;
  nparents = params.nparents;
  nchildren = params.nchildren;
  for (int ii = 0; ii < std::min(MAX_PARENTS, MAX_PARENTS_); ii++) {
    parent[ii] = params.parent[ii];
  }
  for (int ii = 0; ii < std::min(MAX_CHILDREN, MAX_CHILDREN_); ii++) {
    child[ii] = params.child[ii];
  }
  
}

simple_track_params_t TitanData::SimpleTrackParams::convertToLegacy() const
{
  simple_track_params_t params;
  MEM_zero(params);
  convertToLegacy(params);
  return params;
}

void TitanData::SimpleTrackParams::convertToLegacy(simple_track_params_t &params) const
{

  params.start_time = start_time;
  params.end_time = end_time;
  params.last_descendant_end_time = last_descendant_end_time;
  params.time_origin = time_origin;
  params.first_entry_offset = first_entry_offset;
  params.simple_track_num = simple_track_num;
  params.complex_track_num = complex_track_num;
  params.last_descendant_simple_track_num = last_descendant_simple_track_num;
  params.start_scan = start_scan;
  params.end_scan = end_scan;
  params.last_descendant_end_scan = last_descendant_end_scan;
  params.scan_origin = scan_origin;
  params.history_in_scans = history_in_scans;
  params.history_in_secs = history_in_secs;
  params.duration_in_scans = duration_in_scans;
  params.duration_in_secs = duration_in_secs;
  params.nparents = nparents;
  params.nchildren = nchildren;
  for (int ii = 0; ii < std::min(MAX_PARENTS, MAX_PARENTS_); ii++) {
    params.parent[ii] = parent[ii];
  }
  for (int ii = 0; ii < std::min(MAX_CHILDREN, MAX_CHILDREN_); ii++) {
    params.child[ii] = child[ii];
  }

}

void TitanData::SimpleTrackParams::setFromLegacy(const simple_track_params_t *legacyHdrs,
                                                 vector<TitanData::SimpleTrackParams> &scans)
{
  for (size_t ii = 0; ii < scans.size(); ii++) {
    scans[ii].setFromLegacy(legacyHdrs[ii]);
  }
}

void TitanData::SimpleTrackParams::convertToLegacy(const vector<TitanData::SimpleTrackParams> &scans,
                                                   simple_track_params_t *legacyHdrs)
{
  for (size_t ii = 0; ii < scans.size(); ii++) {
    scans[ii].convertToLegacy(legacyHdrs[ii]);
  }
}
    
void TitanData::SimpleTrackParams::print(FILE *out,
                                         const char *spacer)
     
{

  fprintf(out, "%sSIMPLE_TRACK_NUM : %d\n", spacer, simple_track_num);
  fprintf(out, "%s  last_descendant_simple_track_num : %d\n", spacer,
	  last_descendant_simple_track_num);
  fprintf(out, "%s  start_scan : %d\n", spacer, start_scan);
  fprintf(out, "%s  end_scan : %d\n", spacer, end_scan);
  fprintf(out, "%s  last_descendant_end_scan : %d\n", spacer, last_descendant_end_scan);
  fprintf(out, "%s  scan_origin : %d\n", spacer, scan_origin);
  fprintf(out, "%s  start_time : %s\n", spacer, utimstr(start_time));
  fprintf(out, "%s  end_time : %s\n", spacer, utimstr(end_time));
  fprintf(out, "%s  last_descendant_end_time : %s\n", spacer, utimstr(last_descendant_end_time));
  fprintf(out, "%s  time_origin : %s\n", spacer, utimstr(time_origin));
  fprintf(out, "%s  history_in_scans : %d\n", spacer, history_in_scans);
  fprintf(out, "%s  history_in_secs : %d\n", spacer, history_in_secs);
  fprintf(out, "%s  duration_in_scans : %d\n", spacer, duration_in_scans);
  fprintf(out, "%s  duration_in_secs : %d\n", spacer, duration_in_secs);
  fprintf(out, "%s  nparents : %d\n", spacer, nparents);
  fprintf(out, "%s  nchildren : %d\n", spacer, nchildren);
  
  if (nparents > 0) {
    fprintf(out, "%s  parents :", spacer);  
    for (int i = 0; i < nparents; i++) {
      if (i == nparents - 1)
	fprintf(out, " %d\n", parent[i]);
      else
	fprintf(out, " %d,", parent[i]);
    } /* i */
  } /* if */
  
  if (nchildren > 0) {
    fprintf(out, "%s  children :", spacer);  
    for (int i = 0; i < nchildren; i++) {
      if (i == nchildren - 1)
	fprintf(out, " %d\n", child[i]);
      else
	fprintf(out, " %d,", child[i]);
    } /* i */
  } /* if */
  
  fprintf(out, "%s  complex_track_num : %d\n", spacer, complex_track_num);
  fprintf(out, "%s  first_entry_offset : %ld\n", spacer, first_entry_offset);
  fprintf(out, "\n");

}

////////////////////////////////////////////////////////////
// complex track parameters

TitanData::ComplexTrackParams::ComplexTrackParams()
{
  initialize();
}
  
void TitanData::ComplexTrackParams::initialize()
{
  start_time = 0;
  end_time = 0;
  complex_track_num = 0;
  n_simple_tracks = 0;
  start_scan = 0;
  end_scan = 0;
  duration_in_scans = 0;
  duration_in_secs = 0;
  volume_at_start_of_sampling = 0.0;
  volume_at_end_of_sampling = 0.0;
  n_top_missing = 0;
  n_range_limited = 0;
  start_missing = 0;
  end_missing = 0;
  n_samples_for_forecast_stats = 0;
  ellipse_verify.initialize();
  polygon_verify.initialize();
  forecast_bias.initialize();
  forecast_rmse.initialize();
}

void TitanData::ComplexTrackParams::setFromLegacy(const complex_track_params_t &params)
{

  start_time = params.start_time;
  end_time = params.end_time;
  complex_track_num = params.complex_track_num;
  n_simple_tracks = params.n_simple_tracks;
  start_scan = params.start_scan;
  end_scan = params.end_scan;
  duration_in_scans = params.duration_in_scans;
  duration_in_secs = params.duration_in_secs;
  volume_at_start_of_sampling = params.volume_at_start_of_sampling;
  volume_at_end_of_sampling = params.volume_at_end_of_sampling;
  n_top_missing = params.n_top_missing;
  n_range_limited = params.n_range_limited;
  start_missing = params.start_missing;
  end_missing = params.end_missing;
  n_samples_for_forecast_stats = params.n_samples_for_forecast_stats;

  ellipse_verify.setFromLegacy(params.ellipse_verify);
  polygon_verify.setFromLegacy(params.polygon_verify);
  forecast_bias.setFromLegacy(params.forecast_bias);
  forecast_rmse.setFromLegacy(params.forecast_rmse);
  
}

complex_track_params_t TitanData::ComplexTrackParams::convertToLegacy() const
{
  complex_track_params_t params;
  MEM_zero(params);
  convertToLegacy(params);
  return params;
}

void TitanData::ComplexTrackParams::convertToLegacy(complex_track_params_t &params) const
{

  params.start_time = start_time;
  params.end_time = end_time;
  params.complex_track_num = complex_track_num;
  params.n_simple_tracks = n_simple_tracks;
  params.start_scan = start_scan;
  params.end_scan = end_scan;
  params.duration_in_scans = duration_in_scans;
  params.duration_in_secs = duration_in_secs;
  params.volume_at_start_of_sampling = volume_at_start_of_sampling;
  params.volume_at_end_of_sampling = volume_at_end_of_sampling;
  params.n_top_missing = n_top_missing;
  params.n_range_limited = n_range_limited;
  params.start_missing = start_missing;
  params.end_missing = end_missing;
  params.n_samples_for_forecast_stats = n_samples_for_forecast_stats;

  ellipse_verify.convertToLegacy(params.ellipse_verify);
  polygon_verify.convertToLegacy(params.polygon_verify);
  forecast_bias.convertToLegacy(params.forecast_bias);
  forecast_rmse.convertToLegacy(params.forecast_rmse);
  
}

void TitanData::ComplexTrackParams::setFromLegacy
  (const complex_track_params_t *legacyHdrs,
   vector<TitanData::ComplexTrackParams> &scans)
{
  for (size_t ii = 0; ii < scans.size(); ii++) {
    scans[ii].setFromLegacy(legacyHdrs[ii]);
  }
}

void TitanData::ComplexTrackParams::convertToLegacy
  (const vector<TitanData::ComplexTrackParams> &scans,
   complex_track_params_t *legacyHdrs)
{
  for (size_t ii = 0; ii < scans.size(); ii++) {
    scans[ii].convertToLegacy(legacyHdrs[ii]);
  }
}
    
void TitanData::ComplexTrackParams::print(FILE *out,
                                          const char *spacer,
                                          bool verification_performed,
                                          const vector<int> &simples_per_complex)
     
{
  
  char spacer2[128];
  snprintf(spacer2, 128, "%s  ", spacer);
  
  fprintf(out, "%sCOMPLEX_TRACK_NUM : %d\n", spacer, complex_track_num);
  fprintf(out, "%s  n_simple_tracks : %d\n", spacer, n_simple_tracks);
  
  if (simples_per_complex.size() > 0) {
    fprintf(out, "%s  simple_track_nums :", spacer);  
    for (size_t ii = 0; ii < simples_per_complex.size(); ii++) {
      if (ii == simples_per_complex.size() - 1) {
	fprintf(out, " %d\n", simples_per_complex[ii]);
      } else {
	fprintf(out, " %d,", simples_per_complex[ii]);
      }
    } /* ii */
  } /* if */
  
  fprintf(out, "%s  start_scan : %d\n", spacer, start_scan);
  fprintf(out, "%s  end_scan : %d\n", spacer, end_scan);
  fprintf(out, "%s  duration_in_scans : %d\n", spacer, duration_in_scans);
  fprintf(out, "%s  duration_in_secs : %d\n", spacer, duration_in_secs);
  fprintf(out, "%s  start_time : %s\n", spacer, utimstr(start_time));
  fprintf(out, "%s  end_time : %s\n", spacer, utimstr(end_time));
  
  if (verification_performed) {

    fprintf(out, "%s  n_top_missing : %d\n", spacer, n_top_missing);
    fprintf(out, "%s  n_range_limited : %d\n", spacer, n_range_limited);
    fprintf(out, "%s  start_missing : %d\n", spacer, start_missing);
    fprintf(out, "%s  end_missing : %d\n", spacer, end_missing);
    fprintf(out, "%s  volume_at_start_of_sampling : %g\n", spacer, volume_at_start_of_sampling);
    fprintf(out, "%s  volume_at_end_of_sampling : %g\n", spacer, volume_at_end_of_sampling);

    ellipse_verify.print(out, "Ellipses", spacer2);
    polygon_verify.print(out, "Polygons", spacer2);
    forecast_bias.print(out, "  Forecast bias", spacer2);
    forecast_rmse.print(out, "  Forecast RMSE", spacer2);

    fprintf(out, "\n");
  
  } // if (verification_performed)
  
  fprintf(out, "\n");
  
}

////////////////////////////////////////////////////////////
// track entry

TitanData::TrackEntry::TrackEntry()
{
  initialize();
}
  
void TitanData::TrackEntry::initialize()
{
  time = 0;
  time_origin = 0;
  prev_entry_offset = 0;
  this_entry_offset = 0;
  next_entry_offset = 0;
  next_scan_entry_offset = 0;
  scan_origin = 0;
  scan_num = 0;
  storm_num = 0;
  simple_track_num = 0;
  complex_track_num = 0;
  history_in_scans = 0;
  history_in_secs = 0;
  duration_in_scans = 0;
  duration_in_secs = 0;
  forecast_valid = false;
}

void TitanData::TrackEntry::setFromLegacy(const track_file_entry_t &entry)
{

  time = entry.time;
  time_origin = entry.time_origin;
  prev_entry_offset = entry.prev_entry_offset;
  this_entry_offset = entry.this_entry_offset;
  next_entry_offset = entry.next_entry_offset;
  next_scan_entry_offset = entry.next_scan_entry_offset;
  scan_origin = entry.scan_origin;
  scan_num = entry.scan_num;
  storm_num = entry.storm_num;
  simple_track_num = entry.simple_track_num;
  complex_track_num = entry.complex_track_num;
  history_in_scans = entry.history_in_scans;
  history_in_secs = entry.history_in_secs;
  duration_in_scans = entry.duration_in_scans;
  duration_in_secs = entry.duration_in_secs;
  forecast_valid = entry.forecast_valid;

  dval_dt.setFromLegacy(entry.dval_dt);
  
}

track_file_entry_t TitanData::TrackEntry::convertToLegacy() const
{
  track_file_entry_t entry;
  MEM_zero(entry);
  convertToLegacy(entry);
  return entry;
}

void TitanData::TrackEntry::convertToLegacy(track_file_entry_t &entry) const
{

  entry.time = time;
  entry.time_origin = time_origin;
  entry.prev_entry_offset = prev_entry_offset;
  entry.this_entry_offset = this_entry_offset;
  entry.next_entry_offset = next_entry_offset;
  entry.next_scan_entry_offset = next_scan_entry_offset;
  entry.scan_origin = scan_origin;
  entry.scan_num = scan_num;
  entry.storm_num = storm_num;
  entry.simple_track_num = simple_track_num;
  entry.complex_track_num = complex_track_num;
  entry.history_in_scans = history_in_scans;
  entry.history_in_secs = history_in_secs;
  entry.duration_in_scans = duration_in_scans;
  entry.duration_in_secs = duration_in_secs;
  entry.forecast_valid = forecast_valid;

  dval_dt.convertToLegacy(entry.dval_dt);
  
}

void TitanData::TrackEntry::setFromLegacy
  (const track_file_entry_t *legacyEntries,
   vector<TitanData::TrackEntry> &entries)
{
  for (size_t ii = 0; ii < entries.size(); ii++) {
    entries[ii].setFromLegacy(legacyEntries[ii]);
  }
}

void TitanData::TrackEntry::convertToLegacy
  (const vector<TitanData::TrackEntry> &entries,
   track_file_entry_t *legacyEntries)
{
  for (size_t ii = 0; ii < entries.size(); ii++) {
    entries[ii].convertToLegacy(legacyEntries[ii]);
  }
}
    
void TitanData::TrackEntry::print(FILE *out,
                                  const char *spacer,
                                  int entry_num)
     
{

  char spacer2[128];
  snprintf(spacer2, 128, "%s  ", spacer);
  
  fprintf(out, "%sENTRY NUMBER : %d\n", spacer, entry_num);
  fprintf(out, "%s  time : %s\n", spacer, utimstr(time));
  fprintf(out, "%s  time_origin : %s\n", spacer, utimstr(time_origin));
  fprintf(out, "%s  scan_origin in storm file : %d\n", spacer, scan_origin);
  fprintf(out, "%s  scan_num in storm file : %d\n", spacer, scan_num);
  fprintf(out, "%s  storm_num in storm file : %d\n", spacer, storm_num);
  fprintf(out, "%s  simple_track_num : %d\n", spacer, simple_track_num);
  fprintf(out, "%s  complex_track_num : %d\n", spacer, complex_track_num);
  fprintf(out, "%s  history_in_scans : %d\n", spacer, history_in_scans);
  fprintf(out, "%s  history_in_secs : %d\n", spacer, history_in_secs);
  fprintf(out, "%s  duration_in_scans : %d\n", spacer, duration_in_scans);
  fprintf(out, "%s  duration_in_secs : %d\n", spacer, duration_in_secs);
  fprintf(out, "%s  forecast_valid : %s\n", spacer, BOOL_STR(forecast_valid).c_str());
  fprintf(out, "%s  prev_entry_offset : %ld\n", spacer, prev_entry_offset);
  fprintf(out, "%s  this_entry_offset : %ld\n", spacer, this_entry_offset);
  fprintf(out, "%s  next_entry_offset : %ld\n", spacer, next_entry_offset);
  fprintf(out, "\n");

  dval_dt.print(out, "  Entry dval_dt", spacer2);

  return;
  
}

////////////////////////////////////////////////////////////
// index of track in scan

TitanData::TrackScanIndex::TrackScanIndex()
{
  initialize();
}
  
void TitanData::TrackScanIndex::initialize()
{
  utime = 0;
  first_entry_offset = 0;
  n_entries = 0;
}

void TitanData::TrackScanIndex::setFromLegacy(const track_file_scan_index_t &index)
{

  utime = index.utime;
  first_entry_offset = index.first_entry_offset;
  n_entries = index.n_entries;
  
}

track_file_scan_index_t TitanData::TrackScanIndex::convertToLegacy() const
{
  track_file_scan_index_t index;
  MEM_zero(index);
  convertToLegacy(index);
  return index;
}

void TitanData::TrackScanIndex::convertToLegacy(track_file_scan_index_t &index) const
{

  index.utime = utime;
  index.first_entry_offset = first_entry_offset;
  index.n_entries = n_entries;

}

void TitanData::TrackScanIndex::setFromLegacy
  (const track_file_scan_index_t *legacyScans,
   vector<TitanData::TrackScanIndex> &scans)
{
  for (size_t ii = 0; ii < scans.size(); ii++) {
    scans[ii].setFromLegacy(legacyScans[ii]);
  }
}

void TitanData::TrackScanIndex::convertToLegacy
  (const vector<TitanData::TrackScanIndex> &scans,
   track_file_scan_index_t *legacyScans)
{
  for (size_t ii = 0; ii < scans.size(); ii++) {
    scans[ii].convertToLegacy(legacyScans[ii]);
  }
}
    
void TitanData::TrackScanIndex::print(FILE *out, const char *spacer,
                                      const vector<TrackScanIndex> &indexes)

{

  fprintf(out, "\n");
  fprintf(out, "%sSCAN_ENTRY_OFFSETS (time, n_entries, offset):\n", spacer);
  for (size_t ii = 0; ii< indexes.size(); ii++) {
    fprintf(out, "%s  %s %d %ld\n", spacer,
	    utimstr(indexes[ii].utime),
	    indexes[ii].n_entries,
	    indexes[ii].first_entry_offset);
  }
  fprintf(out, "\n");

}
    
////////////////////////////////////////////////////
// utimes - start and end tracks

void TitanData::initialize(TrackUtime_t &utime)
{
  utime.start_simple = 0;
  utime.end_simple = 0;
  utime.start_simple = 0;
  utime.end_simple = 0;
}

void TitanData::initialize(vector<TrackUtime_t> &utimes)
{
  for (size_t ii = 0; ii < utimes.size(); ii++) {
    initialize(utimes[ii]);
  }
}

// copy from legacy to current utime

void TitanData::setUtimeFromLegacy(const track_utime_t &legacy, TrackUtime_t &current)
{
  current.start_simple = legacy.start_simple;
  current.end_simple = legacy.end_simple;
  current.start_simple = legacy.start_simple;
  current.end_simple = legacy.end_simple;
}

// copy from new to legacy utime

void TitanData::convertUtimeToLegacy(const TrackUtime_t &current, track_utime_t &legacy)
{
  legacy.start_simple = current.start_simple;
  legacy.end_simple = current.end_simple;
  legacy.start_simple = current.start_simple;
  legacy.end_simple = current.end_simple;
}

////////////////////////////////////////////////////
// print track arrays

void TitanData::printTrackArrays(FILE *out, const char *spacer,
                                 const TrackHeader &header,
                                 const vector<int> &complex_track_nums,
                                 const vector<int> &nsimples_per_complex,
                                 const vector<int> &simples_per_complex_offsets,
                                 const vector<vector<int> > &simples_per_complex,
                                 const vector<TrackScanIndex> &scan_index)

{

  fprintf(out, "%sTracking arrays:\n", spacer);
  fprintf(out, "%s----------------\n\n", spacer);
  
  /*
   * write out complex_track nums
   */

  fprintf(out, "\n");
  fprintf(out, "%sCOMPLEX_TRACK_NUMS: ", spacer);
  for (size_t ii = 0; ii < complex_track_nums.size(); ii++) {
    fprintf(out, "%d ", complex_track_nums[ii]);
  }
  fprintf(out, "\n");

  TrackScanIndex::print(out, spacer, scan_index);
  
  fprintf(out, "\n");
  fprintf(out, "%sSIMPLES_PER_COMPLEX_OFFSETS: ", spacer);
  for (size_t ii = 0; ii < complex_track_nums.size(); ii++) {
    int complex_num = complex_track_nums[ii];
    fprintf(out, "%d ", simples_per_complex_offsets[complex_num]);
  }
  fprintf(out, "\n");
  
  /*
   * write out nsimples_per_complex
   */
  
  fprintf(out, "\n");
  fprintf(out, "%sNSIMPLES_PER_COMPLEX: ", spacer);
  for (size_t ii = 0; ii < complex_track_nums.size(); ii++) {
    int complex_num = complex_track_nums[ii];
    fprintf(out, "%d ", nsimples_per_complex[complex_num]);
  }
  fprintf(out, "\n");
  fprintf(out, "\n");
  
  /*
   * write out nsimples_per_complex
   */
  
  for (size_t icomplex = 0; icomplex < complex_track_nums.size(); icomplex++) {
    int complex_num = complex_track_nums[icomplex];
    fprintf(out, "%sSIMPLES_PER_COMPLEX, track %d: ", spacer, complex_num);
    int nsimples = nsimples_per_complex[complex_num];
    for (int ii = 0; ii < nsimples; ii++) {
      fprintf(out, "%d ", simples_per_complex[complex_num][ii]);
    }
    fprintf(out, "\n");
  } /* icomplex */

  fprintf(out, "\n\n");

}

////////////////////////////////////////////////////
// printGrid()
//
// Print grid coord struct

void TitanData::printMdvCoord(FILE *out, const char *spacer, const Mdvx::coord_t &coord)
     
{
  
  fprintf(out, "%sMdvx::coord_t grid\n", spacer);
  fprintf(out, "%s------------------\n", spacer);

  fprintf(out, "%s  projType: %s\n", spacer, Mdvx::projType2Str(coord.proj_type));
  fprintf(out, "%s  proj_origin_lat: %g\n", spacer, coord.proj_origin_lat);
  fprintf(out, "%s  proj_origin_lon: %g\n", spacer, coord.proj_origin_lon);

  switch (coord.proj_type) {
    case Mdvx::PROJ_FLAT:
      fprintf(out, "%s  rotation: %g\n", spacer, coord.proj_params.flat.rotation);
      break;
    case Mdvx::PROJ_LAMBERT_CONF:
      fprintf(out, "%s  lat1: %g\n", spacer, coord.proj_params.lc2.lat1);
      fprintf(out, "%s  lat2: %g\n", spacer, coord.proj_params.lc2.lat2);
      break;
    case Mdvx::PROJ_POLAR_STEREO:
      fprintf(out, "%s  tan_lon: %g\n", spacer, coord.proj_params.ps.tan_lon);
      if (coord.proj_params.ps.pole_type == 0) {
        fprintf(out, "%s  pole: north\n", spacer);
      } else {
        fprintf(out, "%s  pole: south\n", spacer);
      }
      fprintf(out, "%s  central_scale: %g\n", spacer, coord.proj_params.ps.central_scale);
      break;
    case Mdvx::PROJ_OBLIQUE_STEREO:
      fprintf(out, "%s  tan_lat: %g\n", spacer, coord.proj_params.os.tan_lat);
      fprintf(out, "%s  tan_lon: %g\n", spacer, coord.proj_params.os.tan_lon);
      fprintf(out, "%s  central_scale: %g\n", spacer, coord.proj_params.os.central_scale);
      break;
    case Mdvx::PROJ_TRANS_MERCATOR:
      fprintf(out, "%s  central_scale: %g\n", spacer, coord.proj_params.tmerc.central_scale);
      break;
    case Mdvx::PROJ_ALBERS:
      fprintf(out, "%s  lat1: %g\n", spacer, coord.proj_params.albers.lat1);
      fprintf(out, "%s  lat2: %g\n", spacer, coord.proj_params.albers.lat2);
      break;
    default: {}
  }

  fprintf(out, "%s  nx, ny, nz : %d, %d, %d\n",
	  spacer,
	  coord.nx, coord.ny, coord.nz);

  fprintf(out, "%s  minx, miny, minz : %g, %g, %g\n",
	  spacer,
	  coord.minx, coord.miny, coord.minz);
  
  fprintf(out, "%s  dx, dy, dz : %g, %g, %g\n", spacer,
	  coord.dx, coord.dy, coord.dz);
  
  fprintf(out, "%s  sensor_x, sensor_y, sensor_z : %g, %g, %g\n",
	  spacer,
	  coord.sensor_x, coord.sensor_y, coord.sensor_z);
  
  fprintf(out, "%s  sensor_lat, sensor_lon : %g, %g\n",
	  spacer,
	  coord.sensor_lat, coord.sensor_lon);
  
  fprintf(out, "%s  dz_constant: %s\n", spacer,
	  BOOL_STR(coord.dz_constant).c_str());

  fprintf(out, "%s  x units : %s\n", spacer, coord.unitsx);
  fprintf(out, "%s  y units : %s\n", spacer, coord.unitsy);
  fprintf(out, "%s  z units : %s\n", spacer, coord.unitsz);
  
}

