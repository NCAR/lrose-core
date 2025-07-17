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

  low_dbz_threshold = missingFl32;
  high_dbz_threshold = missingFl32;
  dbz_hist_interval = missingFl32;
  hail_dbz_threshold = missingFl32;
  base_threshold = missingFl32;
  top_threshold = missingFl32;
  min_storm_size = missingFl32;
  max_storm_size = missingFl32;
  morphology_erosion_threshold = missingFl32;
  morphology_refl_divisor = missingFl32;
  min_radar_tops = missingFl32;
  tops_edge_margin = missingFl32;
  z_p_coeff = missingFl32;
  z_p_exponent = missingFl32;
  z_m_coeff = missingFl32;
  z_m_exponent = missingFl32;
  sectrip_vert_aspect = missingFl32;
  sectrip_horiz_aspect = missingFl32;
  sectrip_orientation_error = missingFl32;
  poly_start_az = missingFl32;
  poly_delta_az = missingFl32;
  ltg_count_time = missingFl32;
  ltg_count_margin_km = missingFl32;
  hail_z_m_coeff = missingFl32;
  hail_z_m_exponent = missingFl32;
  hail_mass_dbz_threshold = missingFl32;
  tops_dbz_threshold = missingFl32;
  precip_plane_ht = missingFl32;
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

void TitanData::StormParams::convertToLegacy(storm_file_params_t &params)
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

void TitanData::StormHeader::convertToLegacy(storm_file_header_t &hdr)
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
    
