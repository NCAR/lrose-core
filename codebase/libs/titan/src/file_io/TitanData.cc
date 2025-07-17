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

  // initialize to 0
  
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
  ltg_count_time = 0.0;
  ltg_count_margin_km = 0.0;
  hail_z_m_coeff = 0.0;
  hail_z_m_exponent = 0.0;
  hail_mass_dbz_threshold = 0.0;
  tops_dbz_threshold = 0.0;
  precip_plane_ht = 0.0;
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
  ltg_count_time = params.ltg_count_time;
  ltg_count_margin_km = params.ltg_count_margin_km;
  hail_z_m_coeff = params.hail_z_m_coeff;
  hail_z_m_exponent = params.hail_z_m_exponent;
  hail_mass_dbz_threshold = params.hail_mass_dbz_threshold;
  tops_dbz_threshold = params.tops_dbz_threshold;
  precip_plane_ht = params.precip_plane_ht;

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
  params.ltg_count_time = ltg_count_time;
  params.ltg_count_margin_km = ltg_count_margin_km;
  params.hail_z_m_coeff = hail_z_m_coeff;
  params.hail_z_m_exponent = hail_z_m_exponent;
  params.hail_mass_dbz_threshold = hail_mass_dbz_threshold;
  params.tops_dbz_threshold = tops_dbz_threshold;
  params.precip_plane_ht = precip_plane_ht;

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

////////////////////////////////////////////////////////////
// StormHeader

TitanData::StormHeader::StormHeader()

{

  // initialize to 0
  
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

////////////////////////////////////////////////////////////
// ScanHeader

TitanData::ScanHeader::ScanHeader()

{

  // initialize to 0
  
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
    
////////////////////////////////////////////////////////////
// Storm Global Properties

TitanData::StormGprops::StormGprops()

{

  // initialize to 0
  
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
  
  FOKRcategory = 0;
  waldvogelProbability = 0.0;
  hailMassAloft = 0.0;
  vihm = 0.0;
  
  poh = 0.0;
  shi = 0.0;
  posh = 0.0;
  mehs = 0.0;

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

  FOKRcategory = 0;
  waldvogelProbability = 0.0;
  hailMassAloft = 0.0;
  vihm = 0.0;
  poh = 0.0;
  shi = 0.0;
  posh = 0.0;
  mehs = 0.0;

  if (params.gprops_union_type == UNION_HAIL) {
    FOKRcategory = gprops.add_on.hail_metrics.FOKRcategory;
    waldvogelProbability = gprops.add_on.hail_metrics.waldvogelProbability;
    hailMassAloft = gprops.add_on.hail_metrics.hailMassAloft;
    vihm = gprops.add_on.hail_metrics.vihm;
  } else if (params.gprops_union_type == UNION_NEXRAD_HDA) {
    poh = gprops.add_on.hda.poh;
    shi = gprops.add_on.hda.shi;
    posh = gprops.add_on.hda.posh;
    mehs = gprops.add_on.hda.mehs;
  }

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

  if (FOKRcategory != 0 ||
      waldvogelProbability != 0.0 ||
      hailMassAloft != 0.0 ||
      vihm != 0.0) {
    gprops.add_on.hail_metrics.FOKRcategory = FOKRcategory;
    gprops.add_on.hail_metrics.waldvogelProbability = waldvogelProbability;
    gprops.add_on.hail_metrics.hailMassAloft = hailMassAloft;
    gprops.add_on.hail_metrics.vihm = vihm;
  } else {
    gprops.add_on.hda.poh = poh;
    gprops.add_on.hda.shi = shi;
    gprops.add_on.hda.posh = posh;
    gprops.add_on.hda.mehs = mehs;
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
    
////////////////////////////////////////////////////////////
// Storm Layer Properties

TitanData::StormLprops::StormLprops()

{

  // initialize to 0
  
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
    
////////////////////////////////////////////////////////////
// Storm Dbz Histograms

TitanData::StormDbzHist::StormDbzHist()

{

  // initialize to 0
  
  percent_volume = 0;
  percent_area = 0;

}

void TitanData::StormDbzHist::setFromLegacy(const storm_file_dbz_hist_t &hist)
{

  percent_volume = hist.percent_volume;
  percent_area = hist.percent_area;

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
    
////////////////////////////////////////////////////////////
// Storm Run - contiguous grid cells in a row

TitanData::StormRun::StormRun()
  
{

  // initialize to 0
  
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
    
