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
// Utilities for Ncf / MDV conversions
// 
///////////////////////////////////////////////////////////////////////

#ifndef NCF_MDV_HH
#define NCF_MDV_HH

#include <string>
#include <Ncxx/Nc3xFile.hh>

using namespace std;

////////////////////////
// 
// Mdv2NcfTrans object extracts data from DsMdvx object and stores
// in objects including NcfGridInfo, NcfFieldData, NcfVlevelInfo.
//
// NcfGridInfo stores projection meta data for a netCDF projection information
// variable, as well as 2D grid information for field datasets.
// and is used to define netCDF dimensions,  coordinate variables, and auxiliary 
// coordinate variables for field datasets.
//
// NcfVlevelInfo objects contain vertical 
// level information and are used to define dimensions and coordinate variables 
// for vertical coordinates.
//
// NcfFieldData objects contain the mdv field data and associate 
// the data to relevant grid,vertical level, and projection information. 

class NcfMdv {
  
public:
  
  // Ncf strings

  const static char* _360_day;
  const static char* _365_day;
  const static char* _366_day;
  const static char* FillValue;
  const static char* add_offset;
  const static char* albers_conical_equal_area;
  const static char* all_leap;
  const static char* ancillary_variables;
  const static char* area;
  const static char* axis;
  const static char* azimuthal_equidistant;
  const static char* bounds;
  const static char* cell_measures;
  const static char* cell_methods;
  const static char* comment;
  const static char* compress;
  const static char* conventions;
  const static char* coordinates;
  const static char* degrees;
  const static char* degrees_east;
  const static char* degrees_north;
  const static char* detection_minimum;
  const static char* down;
  const static char* earth_radius;
  const static char* false_easting;
  const static char* false_northing;
  const static char* flag_meanings;
  const static char* flag_values;
  const static char* formula_terms;
  const static char* forecast_period;
  const static char* forecast_reference_time;
  const static char* gregorian;
  const static char* grid_latitude;
  const static char* grid_longitude;
  const static char* grid_mapping;
  const static char* grid_mapping_attribute;
  const static char* grid_mapping_name;
  const static char* grid_north_pole_latitude;
  const static char* grid_north_pole_longitude;
  const static char* history;
  const static char* institution;
  const static char* inverse_flattening;
  const static char* julian;
  const static char* lambert_azimuthal_equal_area;
  const static char* lambert_conformal_conic;
  const static char* latitude;
  const static char* latitude_longitude;
  const static char* latitude_of_projection_origin;
  const static char* layer;
  const static char* leap_month;
  const static char* leap_year;
  const static char* level;
  const static char* long_name;
  const static char* longitude;
  const static char* longitude_of_central_meridian;
  const static char* longitude_of_prime_meridian;
  const static char* longitude_of_projection_origin;
  const static char* maximum;
  const static char* mean;
  const static char* median;
  const static char* mercator;
  const static char* mid_range;
  const static char* minimum;
  const static char* missing_value;
  const static char* mode;
  const static char* month_lengths;
  const static char* noleap;
  const static char* none;
  const static char* number_of_observations;
  const static char* perspective_point_height;
  const static char* point;
  const static char* polar_radar;
  const static char* polar_stereographic;
  const static char* positive;
  const static char* projection_x_coordinate;
  const static char* projection_y_coordinate;
  const static char* proleptic_gregorian;
  const static char* references;
  const static char* region;
  const static char* rotated_latitude_longitude;
  const static char* scale_factor;
  const static char* scale_factor_at_central_meridian;
  const static char* scale_factor_at_projection_origin;
  const static char* seconds;
  const static char* secs_since_jan1_1970;
  const static char* semi_major_axis;
  const static char* semi_minor_axis;
  const static char* sigma_level;
  const static char* source;
  const static char* standard;
  const static char* standard_deviation;
  const static char* standard_error;
  const static char* standard_name;
  const static char* standard_parallel;
  const static char* start_time;
  const static char* status_flag;
  const static char* stereographic;
  const static char* stop_time;
  const static char* straight_vertical_longitude_from_pole;
  const static char* sum;
  const static char* time;
  const static char* time_bounds;
  const static char* title;
  const static char* transverse_mercator;
  const static char* units;
  const static char* up;
  const static char* valid_max;
  const static char* valid_min;
  const static char* valid_range;
  const static char* variance;
  const static char* vertical;
  const static char* vertical_perspective;
  const static char* volume;

  // MDV strings

  const static char *mdv_master_header;
  const static char *mdv_revision_number;
  const static char *mdv_epoch;
  const static char *mdv_time_centroid;
  const static char *mdv_time_gen;
  const static char *mdv_time_begin;
  const static char *mdv_time_end;
  const static char *mdv_user_time;
  const static char *mdv_time_expire;
  const static char *mdv_time_written;
  const static char *mdv_data_collection_type;
  const static char *mdv_forecast_time;
  const static char *mdv_forecast_delta;
  const static char *mdv_user_data;
  const static char *mdv_user_data_si32_0;
  const static char *mdv_user_data_si32_1;
  const static char *mdv_user_data_si32_2;
  const static char *mdv_user_data_si32_3;
  const static char *mdv_user_data_si32_4;
  const static char *mdv_user_data_si32_5;
  const static char *mdv_user_data_si32_6;
  const static char *mdv_user_data_si32_7;
  const static char *mdv_user_data_si32_8;
  const static char *mdv_user_data_si32_9;
  const static char *mdv_user_data_fl32_0;
  const static char *mdv_user_data_fl32_1;
  const static char *mdv_user_data_fl32_2;
  const static char *mdv_user_data_fl32_3;
  const static char *mdv_user_data_fl32_4;
  const static char *mdv_user_data_fl32_5;
  const static char *mdv_sensor_lon;
  const static char *mdv_sensor_lat;
  const static char *mdv_sensor_alt;
  const static char *mdv_field_code;
  const static char *mdv_user_time_1;
  const static char *mdv_user_time_2;
  const static char *mdv_user_time_3;
  const static char *mdv_user_time_4;
  const static char *mdv_transform_type;
  const static char *mdv_vlevel_type;
  const static char *mdv_native_vlevel_type;
  const static char *mdv_transform;
  const static char *mdv_proj_type;
  const static char *mdv_proj_origin_lat;
  const static char *mdv_proj_origin_lon;
  const static char *vertical_section;
  const static char *id;
  const static char *size;
  const static char *info;
  const static char *mdv_chunk;
  const static char *nbytes_mdv_chunk;

protected:  

private:

};

#endif

