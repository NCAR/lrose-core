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
// Titan2Xml.cc
//
// Convert titan objects to XML
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2021
//
////////////////////////////////////////////////////////////////
//
// Static methods that return strings
//
////////////////////////////////////////////////////////////////

#include <titan/Titan2Xml.hh>
#include <toolsa/TaXml.hh>
#include <algorithm>
using namespace std;

////////////////////////////////////////////////////////////
// storm file header

string Titan2Xml::stormFileHeader(int level,
                                  const storm_file_header_t &header)

{
  
  string xml;
  string tag("storm_file_header");

  xml += TaXml::writeStartTag(tag, level);
  
  xml += TaXml::writeInt("major_rev", level + 1, header.major_rev);
  xml += TaXml::writeInt("minor_rev", level + 1, header.minor_rev);
  xml += TaXml::writeInt("n_scans", level + 1, header.n_scans);
  xml += TaXml::writeInt("data_file_size", level + 1, header.data_file_size);
  xml += TaXml::writeTime("file_time", level + 1, header.file_time);
  xml += TaXml::writeTime("start_time", level + 1, header.start_time);
  xml += TaXml::writeTime("end_time", level + 1, header.end_time);
  xml += TaXml::writeString("header_file_name", level + 1, header.header_file_name);
  xml += TaXml::writeString("data_file_name", level + 1, header.data_file_name);

  xml += stormFileParams(level + 1, header.params);

  xml += TaXml::writeEndTag(tag, level);
  
  return xml;

}

////////////////////////////////////////////////////////////
// storm file params

string Titan2Xml::stormFileParams(int level,
                                  const storm_file_params_t &params)

{
  
  string xml;
  string tag("storm_file_params");
  
  xml += TaXml::writeStartTag(tag, level);

  xml += TaXml::writeDouble("low_dbz_threshold", level + 1, params.low_dbz_threshold);
  xml += TaXml::writeDouble("high_dbz_threshold", level + 1, params.high_dbz_threshold);
  xml += TaXml::writeDouble("dbz_hist_interval", level + 1, params.dbz_hist_interval);
  xml += TaXml::writeDouble("hail_dbz_threshold", level + 1, params.hail_dbz_threshold);
  xml += TaXml::writeDouble("base_threshold", level + 1, params.base_threshold);
  xml += TaXml::writeDouble("top_threshold", level + 1, params.top_threshold);
  xml += TaXml::writeDouble("min_storm_size", level + 1, params.min_storm_size);
  xml += TaXml::writeDouble("max_storm_size", level + 1, params.max_storm_size);
  xml += TaXml::writeDouble("morphology_erosion_threshold", level + 1, params.morphology_erosion_threshold);
  xml += TaXml::writeDouble("morphology_refl_divisor", level + 1, params.morphology_refl_divisor);
  xml += TaXml::writeDouble("min_radar_tops", level + 1, params.min_radar_tops);
  xml += TaXml::writeDouble("tops_edge_margin", level + 1, params.tops_edge_margin);
  xml += TaXml::writeDouble("z_p_coeff", level + 1, params.z_p_coeff);
  xml += TaXml::writeDouble("z_p_exponent", level + 1, params.z_p_exponent);
  xml += TaXml::writeDouble("z_m_coeff", level + 1, params.z_m_coeff);
  xml += TaXml::writeDouble("z_m_exponent", level + 1, params.z_m_exponent);
  xml += TaXml::writeDouble("sectrip_vert_aspect", level + 1, params.sectrip_vert_aspect);
  xml += TaXml::writeDouble("sectrip_horiz_aspect", level + 1, params.sectrip_horiz_aspect);
  xml += TaXml::writeDouble("sectrip_orientation_error", level + 1, params.sectrip_orientation_error);
  xml += TaXml::writeDouble("poly_start_az", level + 1, params.poly_start_az);
  xml += TaXml::writeDouble("poly_delta_az", level + 1, params.poly_delta_az);
  
  xml += TaXml::writeBoolean("check_morphology", level + 1, params.check_morphology);
  xml += TaXml::writeBoolean("check_tops", level + 1, params.check_tops);
  xml += TaXml::writeBoolean("vel_available", level + 1, params.vel_available);
  xml += TaXml::writeInt("n_poly_sides", level + 1, params.n_poly_sides);

  xml += TaXml::writeDouble("ltg_count_time", level + 1, params.ltg_count_time);
  xml += TaXml::writeDouble("ltg_count_margin_km", level + 1, params.ltg_count_margin_km);
  xml += TaXml::writeDouble("hail_z_m_coeff", level + 1, params.hail_z_m_coeff);
  xml += TaXml::writeDouble("hail_z_m_exponent", level + 1, params.hail_z_m_exponent);
  xml += TaXml::writeDouble("hail_mass_dbz_threshold", level + 1, params.hail_mass_dbz_threshold);

  xml += TaXml::writeString("gprops_union_type", level + 1, gpropsHailUnion(params.gprops_union_type));

  xml += TaXml::writeDouble("tops_dbz_threshold", level + 1, params.tops_dbz_threshold);

  xml += TaXml::writeString("precip_computation_mode", level + 1, precipMode(params.precip_computation_mode));

  xml += TaXml::writeDouble("precip_plane_ht", level + 1, params.precip_plane_ht);

  xml += TaXml::writeEndTag(tag, level);
  
  return xml;

}

////////////////////////////////////////////////////////////
// storm file scan header

string Titan2Xml::stormScanHeader(int level,
                                  const storm_file_scan_header_t &header)

{
  
  string xml;
  string tag("storm_file_scan_header");

  xml += TaXml::writeStartTag(tag, level);
  
  xml += TaXml::writeDouble("min_z", level + 1, header.min_z);
  xml += TaXml::writeDouble("delta_z", level + 1, header.delta_z);
  xml += TaXml::writeInt("scan_num", level + 1, header.scan_num);
  xml += TaXml::writeInt("nstorms", level + 1, header.nstorms);
  xml += TaXml::writeTime("time", level + 1, header.time);
  xml += TaXml::writeDouble("ht_of_freezing", level + 1, header.ht_of_freezing);
  xml += titanGrid("grid", level + 1, header.grid);

  xml += TaXml::writeEndTag(tag, level);
  
  return xml;

}

////////////////////////////////////////////////////////////
// storm global props

string Titan2Xml::stormGlobalProps(int level,
                                   const storm_file_params_t &params,
                                   const storm_file_global_props_t &gprops)

{
  
  string xml;
  string tag("storm_file_global_props");
  
  xml += TaXml::writeStartTag(tag, level);

  xml += TaXml::writeDouble("low_dbz_threshold", level + 1, params.low_dbz_threshold);
  xml += TaXml::writeBoolean("check_morphology", level + 1, params.check_morphology);
  xml += TaXml::writeInt("n_poly_sides", level + 1, params.n_poly_sides);
  xml += TaXml::writeString("precip_computation_mode", level + 1, precipMode(params.precip_computation_mode));

  xml += TaXml::writeDouble("vol_centroid_x", level + 1, gprops.vol_centroid_x);
  xml += TaXml::writeDouble("vol_centroid_y", level + 1, gprops.vol_centroid_y);
  xml += TaXml::writeDouble("vol_centroid_z", level + 1, gprops.vol_centroid_z);
  xml += TaXml::writeDouble("refl_centroid_x", level + 1, gprops.refl_centroid_x);
  xml += TaXml::writeDouble("refl_centroid_y", level + 1, gprops.refl_centroid_y);
  xml += TaXml::writeDouble("refl_centroid_z", level + 1, gprops.refl_centroid_z);
  xml += TaXml::writeDouble("top", level + 1, gprops.top);
  xml += TaXml::writeDouble("base", level + 1, gprops.base);
  xml += TaXml::writeDouble("volume", level + 1, gprops.volume);
  xml += TaXml::writeDouble("area_mean", level + 1, gprops.area_mean);
  xml += TaXml::writeDouble("precip_flux", level + 1, gprops.precip_flux);
  xml += TaXml::writeDouble("mass", level + 1, gprops.mass);
  xml += TaXml::writeDouble("tilt_angle", level + 1, gprops.tilt_angle);
  xml += TaXml::writeDouble("tilt_dirn", level + 1, gprops.tilt_dirn);
  xml += TaXml::writeDouble("dbz_max", level + 1, gprops.dbz_max);
  xml += TaXml::writeDouble("dbz_mean", level + 1, gprops.dbz_mean);
  xml += TaXml::writeDouble("dbz_max_gradient", level + 1, gprops.dbz_max_gradient);
  xml += TaXml::writeDouble("dbz_mean_gradient", level + 1, gprops.dbz_mean_gradient);
  xml += TaXml::writeDouble("ht_of_dbz_max", level + 1, gprops.ht_of_dbz_max);
  xml += TaXml::writeDouble("rad_vel_mean", level + 1, gprops.rad_vel_mean);
  xml += TaXml::writeDouble("rad_vel_sd", level + 1, gprops.rad_vel_sd);
  xml += TaXml::writeDouble("vorticity", level + 1, gprops.vorticity);
  xml += TaXml::writeDouble("precip_area", level + 1, gprops.precip_area);
  xml += TaXml::writeDouble("precip_area_centroid_x", level + 1, gprops.precip_area_centroid_x);
  xml += TaXml::writeDouble("precip_area_centroid_y", level + 1, gprops.precip_area_centroid_y);
  xml += TaXml::writeDouble("precip_area_orientation", level + 1, gprops.precip_area_orientation);
  xml += TaXml::writeDouble("precip_area_minor_radius", level + 1, gprops.precip_area_minor_radius);
  xml += TaXml::writeDouble("precip_area_major_radius", level + 1, gprops.precip_area_major_radius);
  xml += TaXml::writeDouble("proj_area", level + 1, gprops.proj_area);
  xml += TaXml::writeDouble("proj_area_centroid_x", level + 1, gprops.proj_area_centroid_x);
  xml += TaXml::writeDouble("proj_area_centroid_y", level + 1, gprops.proj_area_centroid_y);
  xml += TaXml::writeDouble("proj_area_orientation", level + 1, gprops.proj_area_orientation);
  xml += TaXml::writeDouble("proj_area_minor_radius", level + 1, gprops.proj_area_minor_radius);
  xml += TaXml::writeDouble("proj_area_major_radius", level + 1, gprops.proj_area_major_radius);
  xml += TaXml::writeStartTag("proj_area_polygon", level + 1);
  for (int ii = 0; ii < N_POLY_SIDES; ii++) {
    xml += TaXml::writeDouble("radius", level + 2, gprops.proj_area_polygon[ii]);
  }
  xml += TaXml::writeEndTag("proj_area_polygon", level + 1);
  xml += TaXml::writeInt("storm_num", level + 1, gprops.storm_num);
  xml += TaXml::writeInt("n_layers", level + 1, gprops.n_layers);
  xml += TaXml::writeInt("base_layer", level + 1, gprops.base_layer);
  xml += TaXml::writeInt("n_dbz_intervals", level + 1, gprops.n_dbz_intervals);
  xml += TaXml::writeInt("n_runs", level + 1, gprops.n_runs);
  xml += TaXml::writeInt("n_proj_runs", level + 1, gprops.n_proj_runs);
  xml += TaXml::writeBoolean("top_missing", level + 1, gprops.top_missing);
  xml += TaXml::writeBoolean("range_limited", level + 1, gprops.range_limited);
  xml += TaXml::writeBoolean("second_trip", level + 1, gprops.second_trip);
  xml += TaXml::writeBoolean("hail_present", level + 1, gprops.hail_present);
  xml += TaXml::writeBoolean("anom_prop", level + 1, gprops.anom_prop);
  xml += TaXml::writeInt("bounding_min_ix", level + 1, gprops.bounding_min_ix);
  xml += TaXml::writeInt("bounding_min_iy", level + 1, gprops.bounding_min_iy);
  xml += TaXml::writeInt("bounding_max_ix", level + 1, gprops.bounding_max_ix);
  xml += TaXml::writeInt("bounding_max_iy", level + 1, gprops.bounding_max_iy);
  xml += TaXml::writeDouble("vil_from_maxz", level + 1, gprops.vil_from_maxz);
  xml += TaXml::writeDouble("ltg_count", level + 1, gprops.ltg_count);

  if (params.gprops_union_type == UNION_HAIL) {

    xml += TaXml::writeInt("FOKRcategory", level + 1, gprops.add_on.hail_metrics.FOKRcategory);
    xml += TaXml::writeDouble("waldvogelProbability", level + 1, gprops.add_on.hail_metrics.waldvogelProbability);
    xml += TaXml::writeDouble("hailMassAloft", level + 1, gprops.add_on.hail_metrics.hailMassAloft);
    xml += TaXml::writeDouble("vihm", level + 1, gprops.add_on.hail_metrics.vihm);

  } else if (params.gprops_union_type == UNION_NEXRAD_HDA) {
    
    xml += TaXml::writeDouble("poh", level + 1, gprops.add_on.hda.poh);
    xml += TaXml::writeDouble("shi", level + 1, gprops.add_on.hda.shi);
    xml += TaXml::writeDouble("posh", level + 1, gprops.add_on.hda.posh);
    xml += TaXml::writeDouble("mehs", level + 1, gprops.add_on.hda.mehs);

  }

  xml += TaXml::writeEndTag(tag, level);

  return xml;

}

////////////////////////////////////////////////////////////
// storm layer props
// NOTE: layerNum is relative to grid minz

string Titan2Xml::stormLayerProps(int level,
                                  int layerNum,
                                  const titan_grid_t &grid,
                                  const storm_file_layer_props_t &lprops)

{
  
  double layerHtKm = grid.minz + layerNum * grid.dz;
  string xml;
  string tag("storm_file_layer_props");
  
  xml += TaXml::writeStartTag(tag, level);
  
  xml += TaXml::writeInt("layer_num", level + 1, layerNum);
  xml += TaXml::writeDouble("layer_ht_km", level + 1, layerHtKm);
  xml += TaXml::writeDouble("vol_centroid_x", level + 1, lprops.vol_centroid_x);
  xml += TaXml::writeDouble("vol_centroid_y", level + 1, lprops.vol_centroid_y);
  xml += TaXml::writeDouble("refl_centroid_x", level + 1, lprops.refl_centroid_x);
  xml += TaXml::writeDouble("refl_centroid_y", level + 1, lprops.refl_centroid_y);
  xml += TaXml::writeDouble("area", level + 1, lprops.area);
  xml += TaXml::writeDouble("dbz_max", level + 1, lprops.dbz_max);
  xml += TaXml::writeDouble("dbz_mean", level + 1, lprops.dbz_mean);
  xml += TaXml::writeDouble("mass", level + 1, lprops.mass);
  xml += TaXml::writeDouble("rad_vel_mean", level + 1, lprops.rad_vel_mean);
  xml += TaXml::writeDouble("rad_vel_sd", level + 1, lprops.rad_vel_sd);
  xml += TaXml::writeDouble("vorticity", level + 1, lprops.vorticity);
  
  xml += TaXml::writeEndTag(tag, level);

  return xml;

}

////////////////////////////////////////////////////////////
// storm dbz histogram

string Titan2Xml::stormDbzHistEntry(int level,
                                    int binNum,
                                    const storm_file_dbz_hist_t &hist)
  
{
  
  string xml;
  string tag("dbz_hist_entry");
  
  xml += TaXml::writeStartTag(tag, level);
  
  xml += TaXml::writeInt("bin_num", level + 1, binNum);
  xml += TaXml::writeDouble("percent_volume", level + 1, hist.percent_volume);
  xml += TaXml::writeDouble("percent_area", level + 1, hist.percent_area);
  
  xml += TaXml::writeEndTag(tag, level);

  return xml;

}

////////////////////////////////////////////////////////////
// track file header

string Titan2Xml::trackFileHeader(int level,
                                  const track_file_header_t &header)

{
  
  string xml;
  string tag("track_file_header");
  
  xml += TaXml::writeStartTag(tag, level);
  
  xml += TaXml::writeInt("major_rev", level + 1, header.major_rev);
  xml += TaXml::writeInt("minor_rev", level + 1, header.minor_rev);
  xml += TaXml::writeBoolean("file_valid", level + 1, header.file_valid);
  xml += TaXml::writeInt("modify_code", level + 1, header.modify_code);

  xml += TaXml::writeInt("n_simple_tracks", level + 1, header.n_simple_tracks);
  xml += TaXml::writeInt("n_complex_tracks", level + 1, header.n_complex_tracks);

  xml += contingencyData("ellipse_verify", level + 1, header.ellipse_verify);
  xml += contingencyData("polygon_verify", level + 1, header.polygon_verify);

  xml += forecastProps("forecast_bias", level + 1, header.forecast_bias);
  xml += forecastProps("forecast_rmse", level + 1, header.forecast_rmse);
    
  xml += TaXml::writeInt("n_samples_for_forecast_stats", level + 1, header.n_samples_for_forecast_stats);
  xml += TaXml::writeInt("n_scans", level + 1, header.n_scans);
  xml += TaXml::writeInt("last_scan_num", level + 1, header.last_scan_num);
  xml += TaXml::writeInt("max_simple_track_num", level + 1, header.max_simple_track_num);
  xml += TaXml::writeInt("max_complex_track_num", level + 1, header.max_complex_track_num);
  xml += TaXml::writeInt("data_file_size", level + 1, header.data_file_size);
  xml += TaXml::writeTime("file_time", level + 1, header.file_time);
  xml += TaXml::writeInt("max_parents", level + 1, header.max_parents);
  xml += TaXml::writeInt("max_children", level + 1, header.max_children);
  xml += TaXml::writeInt("max_nweights_forecast", level + 1, header.max_nweights_forecast);

  xml += trackVerify("verify", level + 1, header.verify);

  xml += TaXml::writeString("header_file_name", level + 1, header.header_file_name);
  xml += TaXml::writeString("data_file_name", level + 1, header.data_file_name);
  xml += TaXml::writeString("storm_header_file_name", level + 1, header.storm_header_file_name);

  xml += trackFileParams(level + 1, header.params);

  xml += TaXml::writeEndTag(tag, level);
  
  return xml;

}

////////////////////////////////////////////////////////////
// track file params

string Titan2Xml::trackFileParams(int level,
                                  const track_file_params_t &params)

{
  
  string xml;
  string tag("track_file_params");
  
  xml += TaXml::writeStartTag(tag, level);

  xml += TaXml::writeInt("nweights_forecast", level + 1, params.nweights_forecast);
  xml += TaXml::writeStartTag("forecast_weights", level + 1);
  for (int ii = 0; ii < params.nweights_forecast; ii++) {
    xml += TaXml::writeInt("weight", level + 2, params.forecast_weights[ii]);
  }
  xml += TaXml::writeEndTag("forecast_weights", level + 1);

  xml += TaXml::writeDouble("weight_distance", level + 1, params.weight_distance);
  xml += TaXml::writeDouble("weight_delta_cube_root_volume", level + 1, params.weight_delta_cube_root_volume);
  xml += TaXml::writeDouble("merge_split_search_ratio", level + 1, params.merge_split_search_ratio);
  xml += TaXml::writeDouble("max_tracking_speed", level + 1, params.max_tracking_speed);
  xml += TaXml::writeDouble("max_speed_for_valid_forecast", level + 1, params.max_speed_for_valid_forecast);
  xml += TaXml::writeDouble("parabolic_growth_period", level + 1, params.parabolic_growth_period);
  xml += TaXml::writeDouble("smoothing_radius", level + 1, params.smoothing_radius);
  xml += TaXml::writeDouble("min_fraction_overlap", level + 1, params.min_fraction_overlap);
  xml += TaXml::writeDouble("min_sum_fraction_overlap", level + 1, params.min_sum_fraction_overlap);
  xml += TaXml::writeBoolean("scale_forecasts_by_history", level + 1, params.scale_forecasts_by_history);
  xml += TaXml::writeBoolean("use_runs_for_overlaps", level + 1, params.use_runs_for_overlaps);
  xml += TaXml::writeString("grid_type", level + 1, gridType(params.grid_type));
  xml += TaXml::writeString("forecast_type", level + 1, forecastType(params.forecast_type));
  xml += TaXml::writeInt("max_delta_time", level + 1, params.max_delta_time);
  xml += TaXml::writeInt("min_history_for_valid_forecast", level + 1, params.min_history_for_valid_forecast);
  xml += TaXml::writeBoolean("spatial_smoothing", level + 1, params.spatial_smoothing);

  xml += TaXml::writeEndTag(tag, level);
  
  return xml;

}

////////////////////////////////////////////////////////////
// simple params

string Titan2Xml::simpleTrackParams(int level, 
                                    const simple_track_params_t &params)

{

  string xml;
  string tag("simple_track_params");
  
  xml += TaXml::writeStartTag(tag, level);

  xml += TaXml::writeInt("simple_track_num", level + 1, params.simple_track_num);
  xml += TaXml::writeInt("last_descendant_simple_track_num", level + 1, params.last_descendant_simple_track_num);
  xml += TaXml::writeInt("start_scan", level + 1, params.start_scan);
  xml += TaXml::writeInt("end_scan", level + 1, params.end_scan);
  xml += TaXml::writeInt("last_descendant_end_scan", level + 1, params.last_descendant_end_scan);
  xml += TaXml::writeInt("scan_origin", level + 1, params.scan_origin);
  xml += TaXml::writeInt("start_time", level + 1, params.start_time);
  xml += TaXml::writeInt("end_time", level + 1, params.end_time);
  xml += TaXml::writeInt("last_descendant_end_time", level + 1, params.last_descendant_end_time);
  xml += TaXml::writeInt("time_origin", level + 1, params.time_origin);
  xml += TaXml::writeInt("history_in_scans", level + 1, params.history_in_scans);
  xml += TaXml::writeInt("history_in_secs", level + 1, params.history_in_secs);
  
  xml += TaXml::writeInt("duration_in_scans", level + 1, params.duration_in_scans);
  xml += TaXml::writeInt("duration_in_secs", level + 1, params.duration_in_secs);
  int nparents = max(params.nparents, MAX_PARENTS_V5);
  xml += TaXml::writeInt("nparents", level + 1, nparents);
  int nchildren = max(params.nchildren, MAX_CHILDREN_V5);
  xml += TaXml::writeInt("nchildren", level + 1, nchildren);

  xml += TaXml::writeStartTag("parents", level + 1);
  for (int ii = 0; ii < nparents; ii++) {
    xml += TaXml::writeInt("parent", level + 2, params.parent[ii]);
  }
  xml += TaXml::writeEndTag("parents", level + 1);

  xml += TaXml::writeStartTag("children", level + 1);
  for (int ii = 0; ii < nchildren; ii++) {
    xml += TaXml::writeInt("child", level + 2, params.child[ii]);

  }
  xml += TaXml::writeEndTag("children", level + 1);

  xml += TaXml::writeInt("complex_track_num", level + 1, params.complex_track_num);
  xml += TaXml::writeInt("first_entry_offset", level + 1, params.first_entry_offset);

  xml += TaXml::writeEndTag(tag, level);

  return xml;

}

////////////////////////////////////////////////////////////
// simple params

string Titan2Xml::complexTrackParams(int level,
                                     const complex_track_params_t &params)

{

  string xml;
  string tag("complex_track_params");
  
  xml += TaXml::writeStartTag(tag, level);
  
  xml += TaXml::writeDouble("volume_at_start_of_sampling",
                            level + 1, params.volume_at_start_of_sampling);
  xml += TaXml::writeDouble("volume_at_end_of_sampling",
                            level + 1, params.volume_at_end_of_sampling);

  xml += TaXml::writeInt("complex_track_num", level + 1, params.complex_track_num);

  xml += TaXml::writeInt("start_scan", level + 1, params.start_scan);
  xml += TaXml::writeInt("end_scan", level + 1, params.end_scan);

  xml += TaXml::writeInt("duration_in_scans", level + 1, params.duration_in_scans);
  xml += TaXml::writeInt("duration_in_secs", level + 1, params.duration_in_secs);

  xml += TaXml::writeTime("start_time", level + 1, params.start_time);
  xml += TaXml::writeTime("end_time", level + 1, params.end_time);

  xml += TaXml::writeInt("n_simple_tracks", level + 1, params.n_simple_tracks);

  xml += TaXml::writeInt("n_top_missing", level + 1, params.n_top_missing);
  xml += TaXml::writeInt("n_range_limited", level + 1, params.n_range_limited);
  xml += TaXml::writeInt("start_missing", level + 1, params.start_missing);
  xml += TaXml::writeInt("end_missing", level + 1, params.end_missing);
  xml += TaXml::writeInt("n_samples_for_forecast_stats", level + 1, params.n_samples_for_forecast_stats);

  xml += contingencyData("ellipse_verify", level + 1, params.ellipse_verify);
  xml += contingencyData("polygon_verify", level + 1, params.polygon_verify);
  xml += forecastProps("forecast_bias", level + 1, params.forecast_bias);
  xml += forecastProps("forecast_rmse", level + 1, params.forecast_rmse);

  xml += TaXml::writeEndTag(tag, level);
  
  return xml;

}

////////////////////////////////////////////////////////////
// track entry

string Titan2Xml::trackEntry(int level,
                             const track_file_entry_t &entry,
                             int entry_num /* = -1 */)

{

  string xml;
  string tag("track_entry");
  
  if (entry_num >= 0) {
    vector<TaXml::attribute> attrs;
    attrs.push_back(TaXml::attribute("entry_number", entry_num));
    xml += TaXml::writeStartTag(tag, level, attrs, true);
  } else {
    xml += TaXml::writeStartTag(tag, level);
  }

  xml += TaXml::writeTime("time", level + 1, entry.time);
  xml += TaXml::writeTime("time_origin", level + 1, entry.time_origin);
  xml += TaXml::writeInt("scan_origin", level + 1, entry.scan_origin);
  xml += TaXml::writeInt("scan_num", level + 1, entry.scan_num);
  xml += TaXml::writeInt("storm_num", level + 1, entry.storm_num);
  xml += TaXml::writeInt("simple_track_num", level + 1, entry.simple_track_num);
  xml += TaXml::writeInt("complex_track_num", level + 1, entry.complex_track_num);
  xml += TaXml::writeInt("history_in_scans", level + 1, entry.history_in_scans);
  xml += TaXml::writeInt("history_in_secs", level + 1, entry.history_in_secs);
  xml += TaXml::writeInt("duration_in_scans", level + 1, entry.duration_in_scans);
  xml += TaXml::writeInt("duration_in_secs", level + 1, entry.duration_in_secs);
  xml += TaXml::writeBoolean("forecast_valid", level + 1, entry.forecast_valid);

  xml += forecastProps("dval_dt", level + 1, entry.dval_dt);

  xml += TaXml::writeEndTag(tag, level);
  
  return xml;

}

////////////////////////////////////////////////////////////
// forecast props

string Titan2Xml::forecastProps(const string &tag,
                                int level,
                                const track_file_forecast_props_t &props)
  
{

  string xml;

  xml += TaXml::writeStartTag(tag, level);
  
  xml += TaXml::writeDouble("proj_area_centroid_x", level + 1,
                            props.proj_area_centroid_x);
  xml += TaXml::writeDouble("proj_area_centroid_y", level + 1,
                            props.proj_area_centroid_y);
  xml += TaXml::writeDouble("vol_centroid_z", level + 1,
                            props.vol_centroid_z);
  xml += TaXml::writeDouble("refl_centroid_z", level + 1,
                            props.refl_centroid_z);
  xml += TaXml::writeDouble("top", level + 1,
                            props.top);
  xml += TaXml::writeDouble("dbz_max", level + 1,
                            props.dbz_max);
  xml += TaXml::writeDouble("volume", level + 1,
                            props.volume);
  xml += TaXml::writeDouble("precip_flux", level + 1,
                            props.precip_flux);
  xml += TaXml::writeDouble("mass", level + 1,
                            props.mass);
  xml += TaXml::writeDouble("proj_area", level + 1,
                            props.proj_area);
  xml += TaXml::writeDouble("smoothed_proj_area_centroid_x", level + 1,
                            props.smoothed_proj_area_centroid_x);
  xml += TaXml::writeDouble("smoothed_proj_area_centroid_y", level + 1,
                            props.smoothed_proj_area_centroid_y);
  xml += TaXml::writeDouble("smoothed_speed", level + 1,
                            props.smoothed_speed);
  xml += TaXml::writeDouble("smoothed_direction", level + 1,
                            props.smoothed_direction);
  
  xml += TaXml::writeEndTag(tag, level);
  
  return xml;

}

////////////////////////////////////////////////////////////
// titan grid

string Titan2Xml::titanGrid(const string &tag,
                            int level,
                            const titan_grid_t &grid)
  
{
  
  string xml;

  xml += TaXml::writeStartTag(tag, level);

  xml += TaXml::writeDouble("proj_origin_lat", level + 1, grid.proj_origin_lat);
  xml += TaXml::writeDouble("proj_origin_lon", level + 1, grid.proj_origin_lon);

  if (grid.proj_type == TITAN_PROJ_FLAT) {
    xml += TaXml::writeDouble("flat.rotation", level + 1, grid.proj_params.flat.rotation);
  } else if (grid.proj_type == TITAN_PROJ_LAMBERT_CONF) {
    xml += TaXml::writeDouble("lc2.lat1", level + 1, grid.proj_params.lc2.lat1);
    xml += TaXml::writeDouble("lc2.lat2", level + 1, grid.proj_params.lc2.lat2);
    xml += TaXml::writeDouble("lc2.SW_lat", level + 1, grid.proj_params.lc2.SW_lat);
    xml += TaXml::writeDouble("lc2.SW_lon", level + 1, grid.proj_params.lc2.SW_lon);
    xml += TaXml::writeDouble("lc2.origin_x", level + 1, grid.proj_params.lc2.origin_x);
    xml += TaXml::writeDouble("lc2.origin_y", level + 1, grid.proj_params.lc2.origin_y);
  }

  xml += TaXml::writeInt("nx", level + 1, grid.nx);
  xml += TaXml::writeInt("ny", level + 1, grid.ny);
  xml += TaXml::writeInt("nz", level + 1, grid.nz);

  xml += TaXml::writeDouble("minx", level + 1, grid.minx);
  xml += TaXml::writeDouble("miny", level + 1, grid.miny);
  xml += TaXml::writeDouble("minz", level + 1, grid.minz);

  xml += TaXml::writeDouble("dx", level + 1, grid.dx);
  xml += TaXml::writeDouble("dy", level + 1, grid.dy);
  xml += TaXml::writeDouble("dz", level + 1, grid.dz);

  xml += TaXml::writeDouble("sensor_x", level + 1, grid.sensor_x);
  xml += TaXml::writeDouble("sensor_y", level + 1, grid.sensor_y);
  xml += TaXml::writeDouble("sensor_z", level + 1, grid.sensor_z);

  xml += TaXml::writeDouble("sensor_lat", level + 1, grid.sensor_lat);
  xml += TaXml::writeDouble("sensor_lon", level + 1, grid.sensor_lon);

  xml += TaXml::writeString("proj_type", level + 1, gridType(grid.proj_type));

  xml += TaXml::writeBoolean("dz_constant", level + 1, grid.dz_constant);
  xml += TaXml::writeString("unitsx", level + 1, grid.unitsx);
  xml += TaXml::writeString("unitsy", level + 1, grid.unitsy);
  xml += TaXml::writeString("unitsz", level + 1, grid.unitsz);
  
  xml += TaXml::writeEndTag(tag, level);
  
  return xml;

}

////////////////////////////////////////////////////////////
// track verify

string Titan2Xml::trackVerify(const string &tag,
                              int level,
                              const track_file_verify_t &verify)

{
  
  string xml;
  
  xml += TaXml::writeStartTag(tag, level);
  xml += TaXml::writeBoolean("verification_performed", level + 1, verify.verification_performed);
  if (verify.verification_performed) {
    xml += TaXml::writeInt("forecast_lead_time", level + 1, verify.forecast_lead_time);
    xml += TaXml::writeTime("end_time", level + 1, verify.end_time);
    xml += TaXml::writeInt("forecast_lead_time_margin", level + 1, verify.forecast_lead_time_margin);
    xml += TaXml::writeInt("forecast_min_history", level + 1, verify.forecast_min_history);
    xml += TaXml::writeBoolean("verify_before_forecast_time", level + 1, verify.verify_before_forecast_time);
    xml += TaXml::writeBoolean("verify_after_track_dies", level + 1, verify.verify_after_track_dies);
    xml += titanGrid("grid", level + 1, verify.grid);
  }
  xml += TaXml::writeEndTag(tag, level);
  
  return xml;

}


////////////////////////////////////////////////////////////
// contingency data

string Titan2Xml::contingencyData(const string &tag,
                                  int level,
                                  const track_file_contingency_data_t &cont)
  
{

  string xml;

  xml += TaXml::writeStartTag(tag, level);
  xml += TaXml::writeDouble("n_success", level + 1, cont.n_success);
  xml += TaXml::writeDouble("n_failure", level + 1, cont.n_failure);
  xml += TaXml::writeDouble("n_false_alarm", level + 1, cont.n_false_alarm);

  xml += TaXml::writeEndTag(tag, level);
  
  return xml;

}

////////////////////////////////////////////////////////////
// grid type

string Titan2Xml::gridType(int grid_type)

{
  
  switch (grid_type) {
    case TITAN_PROJ_LATLON:
      return "LATLON";
    case TITAN_PROJ_STEREOGRAPHIC:
      return "STEREOGRAPHIC";
    case TITAN_PROJ_LAMBERT_CONF:
      return "LAMBERT_CONF";
    case TITAN_PROJ_MERCATOR:
      return "MERCATOR";
    case TITAN_PROJ_POLAR_STEREO:
      return "POLAR_STEREO";
    case TITAN_PROJ_POLAR_ST_ELLIP:
      return "POLAR_ST_ELLIP";
    case TITAN_PROJ_CYL_EQUIDIST:
      return "CYL_EQUIDIST";
    case TITAN_PROJ_FLAT:
      return "FLAT";
    case TITAN_PROJ_POLAR_RADAR:
      return "POLAR_RADAR";
    case TITAN_PROJ_RADIAL:
      return "RADIAL";
    case TITAN_PROJ_OBLIQUE_STEREO:
      return "OBLIQUE_STEREO";
    case TITAN_PROJ_TRANS_MERCATOR:
      return "TRANS_MERCATOR";
    case TITAN_PROJ_ALBERS:
      return "ALBERS";
    case TITAN_PROJ_LAMBERT_AZIM:
      return "LAMBERT_AZIM";
  }

  return "UNKNOWN";
  
}

////////////////////////////////////////////////////////////
// forecast type

string Titan2Xml::forecastType(int forecast_type)

{
  
  switch (forecast_type) {
    case FORECAST_BY_TREND:
      return "TREND";
    case FORECAST_BY_PARABOLA:
      return "PARABOLA";
    case FORECAST_BY_REGRESSION:
      return "REGRESSION";
  }

  return "UNKNOWN";

}

////////////////////////////////////////////////////////////
// precip mode

string Titan2Xml::precipMode(int precip_mode)

{
  
  switch (precip_mode) {
    case TITAN_PRECIP_FROM_COLUMN_MAX:
      return "COLUMN_MAX";
    case TITAN_PRECIP_AT_SPECIFIED_HT:
      return "SPECIFIED_HT";
    case TITAN_PRECIP_AT_LOWEST_VALID_HT:
      return "LOWEST_VALID_HT";
    case TITAN_PRECIP_FROM_LOWEST_AVAILABLE_REFL:
      return "LOWEST_AVAILABLE_REFL";
  }

  return "UNKNOWN";

}

////////////////////////////////////////////////////////////
// global props hail union type

string Titan2Xml::gpropsHailUnion(int gprops_union_type)

{
  
  switch (gprops_union_type) {
    case UNION_HAIL:
      return "HAIL";
    case UNION_NEXRAD_HDA:
      return "NEXRAD_HDA";
  }

  return "NONE";
  
}

