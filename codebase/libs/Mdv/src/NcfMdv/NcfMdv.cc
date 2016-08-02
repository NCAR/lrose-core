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

/////////////////////////////////////////////////////////////
// NcfMdv.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2008
//
///////////////////////////////////////////////////////////////
//
// Utility class for NcfMdv conversions.
//
///////////////////////////////////////////////////////////////////////

#include <Mdv/NcfMdv.hh>

// standard strings used by CF NetCDF

const char* NcfMdv::_360_day = "360_day";
const char* NcfMdv::_365_day = "365_day";
const char* NcfMdv::_366_day = "366_day";
const char* NcfMdv::FillValue = "_FillValue";
const char* NcfMdv::add_offset = "add_offset";
const char* NcfMdv::albers_conical_equal_area = "albers_conical_equal_area";
const char* NcfMdv::all_leap = "all_leap";
const char* NcfMdv::ancillary_variables = "ancillary_variables";
const char* NcfMdv::area = "area";
const char* NcfMdv::axis = "axis";
const char* NcfMdv::azimuthal_equidistant = "azimuthal_equidistant";
const char* NcfMdv::bounds = "bounds";
const char* NcfMdv::cell_measures = "cell_measures";
const char* NcfMdv::cell_methods = "cell_methods";
const char* NcfMdv::comment = "comment";
const char* NcfMdv::compress = "compress";
const char* NcfMdv::conventions = "conventions";
const char* NcfMdv::coordinates = "coordinates";
const char* NcfMdv::degrees = "degrees";
const char* NcfMdv::degrees_east = "degrees_east";
const char* NcfMdv::degrees_north = "degrees_north";
const char* NcfMdv::detection_minimum = "detection_minimum";
const char* NcfMdv::down = "down";
const char* NcfMdv::earth_radius = "earth_radius";
const char* NcfMdv::false_easting = "false_easting";
const char* NcfMdv::false_northing = "false_northing";
const char* NcfMdv::flag_meanings = "flag_meanings";
const char* NcfMdv::flag_values = "flag_values";
const char* NcfMdv::forecast_period = "forecast_period";
const char* NcfMdv::forecast_reference_time = "forecast_reference_time";
const char* NcfMdv::formula_terms = "formula_terms";
const char* NcfMdv::gregorian = "gregorian";
const char* NcfMdv::grid_latitude = "grid_latitude";
const char* NcfMdv::grid_longitude = "grid_longitude";
const char* NcfMdv::grid_mapping = "grid_mapping";
const char* NcfMdv::grid_mapping_attribute = "grid_mapping_attribute";
const char* NcfMdv::grid_mapping_name = "grid_mapping_name";
const char* NcfMdv::grid_north_pole_latitude = "grid_north_pole_latitude";
const char* NcfMdv::grid_north_pole_longitude = "grid_north_pole_longitude";
const char* NcfMdv::history = "history";
const char* NcfMdv::institution = "institution";
const char* NcfMdv::inverse_flattening = "inverse_flattening";
const char* NcfMdv::julian = "julian";
const char* NcfMdv::lambert_azimuthal_equal_area = "lambert_azimuthal_equal_area";
const char* NcfMdv::lambert_conformal_conic = "lambert_conformal_conic";
const char* NcfMdv::latitude = "latitude";
const char* NcfMdv::latitude_longitude = "latitude_longitude";
const char* NcfMdv::latitude_of_projection_origin = "latitude_of_projection_origin";
const char* NcfMdv::layer = "layer";
const char* NcfMdv::leap_month = "leap_month";
const char* NcfMdv::leap_year = "leap_year";
const char* NcfMdv::level = "level";
const char* NcfMdv::long_name = "long_name";
const char* NcfMdv::longitude = "longitude";
const char* NcfMdv::longitude_of_central_meridian = "longitude_of_central_meridian";
const char* NcfMdv::longitude_of_prime_meridian = "longitude_of_prime_meridian";
const char* NcfMdv::longitude_of_projection_origin = "longitude_of_projection_origin";
const char* NcfMdv::maximum = "maximum";
const char* NcfMdv::mean = "mean";
const char* NcfMdv::median = "median";
const char* NcfMdv::mercator = "mercator";
const char* NcfMdv::mid_range = "mid_range";
const char* NcfMdv::minimum = "minimum";
const char* NcfMdv::missing_value = "missing_value";
const char* NcfMdv::mode = "mode";
const char* NcfMdv::month_lengths = "month_lengths";
const char* NcfMdv::noleap = "noleap";
const char* NcfMdv::none = "none";
const char* NcfMdv::number_of_observations = "number_of_observations";
const char* NcfMdv::perspective_point_height = "perspective_point_height";
const char* NcfMdv::point = "point";
const char* NcfMdv::polar_radar = "polar_radar";
const char* NcfMdv::polar_stereographic = "polar_stereographic";
const char* NcfMdv::positive = "positive";
const char* NcfMdv::projection_x_coordinate = "projection_x_coordinate";
const char* NcfMdv::projection_y_coordinate = "projection_y_coordinate";
const char* NcfMdv::proleptic_gregorian = "proleptic_gregorian";
const char* NcfMdv::references = "references";
const char* NcfMdv::region = "region";
const char* NcfMdv::rotated_latitude_longitude = "rotated_latitude_longitude";
const char* NcfMdv::scale_factor = "scale_factor";
const char* NcfMdv::scale_factor_at_central_meridian = "scale_factor_at_central_meridian";
const char* NcfMdv::scale_factor_at_projection_origin = "scale_factor_at_projection_origin";
const char* NcfMdv::seconds = "seconds";
const char* NcfMdv::secs_since_jan1_1970 = "seconds since 1970-01-01T00:00:00Z";
const char* NcfMdv::semi_major_axis = "semi_major_axis";
const char* NcfMdv::semi_minor_axis = "semi_minor_axis";
const char* NcfMdv::sigma_level = "sigma_level";
const char* NcfMdv::source = "source";
const char* NcfMdv::standard = "standard";
const char* NcfMdv::standard_deviation = "standard_deviation";
const char* NcfMdv::standard_error = "standard_error";
const char* NcfMdv::standard_name = "standard_name";
const char* NcfMdv::standard_parallel = "standard_parallel";
const char* NcfMdv::start_time = "start_time";
const char* NcfMdv::status_flag = "status_flag";
const char* NcfMdv::stereographic = "stereographic";
const char* NcfMdv::stop_time = "stop_time";
const char* NcfMdv::straight_vertical_longitude_from_pole = "straight_vertical_longitude_from_pole";
const char* NcfMdv::sum = "sum";
const char* NcfMdv::time = "time";
const char* NcfMdv::time_bounds = "time_bounds";
const char* NcfMdv::title = "title";
const char* NcfMdv::transverse_mercator = "transverse_mercator";
const char* NcfMdv::units = "units";
const char* NcfMdv::up = "up";
const char* NcfMdv::valid_max = "valid_max";
const char* NcfMdv::valid_min = "valid_min";
const char* NcfMdv::valid_range = "valid_range";
const char* NcfMdv::variance = "variance";
const char* NcfMdv::vertical = "vertical";
const char* NcfMdv::vertical_perspective = "vertical_perspective";
const char* NcfMdv::volume = "volume";

// standard strings used by NcfMdv transformations

const char* NcfMdv::mdv_master_header = "mdv_master_header";
const char* NcfMdv::mdv_revision_number = "mdv_revision_number";
const char* NcfMdv::mdv_epoch = "mdv_epoch";
const char* NcfMdv::mdv_time_centroid = "mdv_time_centroid";
const char* NcfMdv::mdv_time_gen = "mdv_time_gen";
const char* NcfMdv::mdv_time_begin = "mdv_time_begin";
const char* NcfMdv::mdv_time_end = "mdv_time_end";
const char* NcfMdv::mdv_user_time = "mdv_user_time";
const char* NcfMdv::mdv_time_expire = "mdv_time_expire";
const char* NcfMdv::mdv_time_written = "mdv_time_written";
const char* NcfMdv::mdv_data_collection_type = "mdv_data_collection_type";
const char* NcfMdv::mdv_forecast_time = "mdv_forecast_time";
const char* NcfMdv::mdv_forecast_delta = "mdv_forecast_delta";
const char* NcfMdv::mdv_user_data = "mdv_user_data";
const char* NcfMdv::mdv_user_data_si32_0 = "mdv_user_data_si32_0";
const char* NcfMdv::mdv_user_data_si32_1 = "mdv_user_data_si32_1";
const char* NcfMdv::mdv_user_data_si32_2 = "mdv_user_data_si32_2";
const char* NcfMdv::mdv_user_data_si32_3 = "mdv_user_data_si32_3";
const char* NcfMdv::mdv_user_data_si32_4 = "mdv_user_data_si32_4";
const char* NcfMdv::mdv_user_data_si32_5 = "mdv_user_data_si32_5";
const char* NcfMdv::mdv_user_data_si32_6 = "mdv_user_data_si32_6";
const char* NcfMdv::mdv_user_data_si32_7 = "mdv_user_data_si32_7";
const char* NcfMdv::mdv_user_data_si32_8 = "mdv_user_data_si32_8";
const char* NcfMdv::mdv_user_data_si32_9 = "mdv_user_data_si32_9";
const char* NcfMdv::mdv_user_data_fl32_0 = "mdv_user_data_fl32_0";
const char* NcfMdv::mdv_user_data_fl32_1 = "mdv_user_data_fl32_1";
const char* NcfMdv::mdv_user_data_fl32_2 = "mdv_user_data_fl32_2";
const char* NcfMdv::mdv_user_data_fl32_3 = "mdv_user_data_fl32_3";
const char* NcfMdv::mdv_user_data_fl32_4 = "mdv_user_data_fl32_4";
const char* NcfMdv::mdv_user_data_fl32_5 = "mdv_user_data_fl32_5";
const char* NcfMdv::mdv_sensor_lon = "mdv_sensor_lon";
const char* NcfMdv::mdv_sensor_lat = "mdv_sensor_lat";
const char* NcfMdv::mdv_sensor_alt = "mdv_sensor_alt";
const char* NcfMdv::mdv_field_code = "mdv_field_code";
const char* NcfMdv::mdv_user_time_1 = "mdv_user_time_1";
const char* NcfMdv::mdv_user_time_2 = "mdv_user_time_2";
const char* NcfMdv::mdv_user_time_3 = "mdv_user_time_3";
const char* NcfMdv::mdv_user_time_4 = "mdv_user_time_4";
const char* NcfMdv::mdv_transform_type = "mdv_transform_type";
const char* NcfMdv::mdv_vlevel_type = "mdv_vlevel_type";
const char* NcfMdv::mdv_native_vlevel_type = "mdv_native_vlevel_type";
const char* NcfMdv::mdv_transform = "mdv_transform";
const char* NcfMdv::mdv_proj_type = "mdv_proj_type";
const char* NcfMdv::mdv_proj_origin_lat = "mdv_proj_origin_lat";
const char* NcfMdv::mdv_proj_origin_lon = "mdv_proj_origin_lon";
const char* NcfMdv::vertical_section = "vertical_section";
const char* NcfMdv::id = "id";
const char* NcfMdv::size = "size";
const char* NcfMdv::info = "info";
const char* NcfMdv::mdv_chunk = "mdv_chunk";
const char* NcfMdv::nbytes_mdv_chunk = "nbytes_mdv_chunk";
