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
// RadxNcfStr.hh
//
// Base class containing
// strings for NetCDF CF-compliant radar data
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2018
//
///////////////////////////////////////////////////////////////
//
// Ncf File classes will inherit both RadxFile and this class
//
///////////////////////////////////////////////////////////////

#ifndef RadxNcfStr_HH
#define RadxNcfStr_HH

#include <string>
#include <set>
using namespace std;

class RadxNcfStr

{
  
public:

  /// Constructor
  
  RadxNcfStr();
  
  /// Destructor
  
  virtual ~RadxNcfStr();

  // is this a ray metadata variable name?

  bool isRayMetaName(const string &varName) const;

  // string constants
  
  string CfConvention;
  string BaseConvention;
  string CurrentVersion;

  string CfRadial2Conventions;
  string CfRadial2Version;

  // char * constants

  static constexpr const char* ADD_OFFSET = "add_offset";
  static constexpr const char* AIRBORNE = "airborne";
  static constexpr const char* ALTITUDE = "altitude";
  static constexpr const char* ALTITUDE_AGL = "altitude_agl";
  static constexpr const char* ALTITUDE_CORRECTION = "altitude_correction";
  static constexpr const char* ALTITUDE_OF_PROJECTION_ORIGIN = "altitude_of_projection_origin";
  static constexpr const char* ANCILLARY_VARIABLES = "ancillary_variables";
  static constexpr const char* ANTENNA_GAIN_H = "antenna_gain_h";
  static constexpr const char* ANTENNA_GAIN_V = "antenna_gain_v";
  static constexpr const char* ANTENNA_TRANSITION = "antenna_transition";
  static constexpr const char* AUTHOR = "author";
  static constexpr const char* AXIS = "axis";
  static constexpr const char* AZIMUTH = "azimuth";
  static constexpr const char* AZIMUTH_CORRECTION = "azimuth_correction";
  static constexpr const char* BASE_DBZ_1KM_HC = "base_dbz_1km_hc";
  static constexpr const char* BASE_DBZ_1KM_HX = "base_dbz_1km_hx";
  static constexpr const char* BASE_DBZ_1KM_VC = "base_dbz_1km_vc";
  static constexpr const char* BASE_DBZ_1KM_VX = "base_dbz_1km_vx";
  static constexpr const char* BLOCK_AVG_LENGTH = "block_avg_length";
  static constexpr const char* CALENDAR = "calendar";
  static constexpr const char* CALIBRATION_TIME = "calibration_time";
  static constexpr const char* CFRADIAL = "cfradial";
  static constexpr const char* CM = "cm";
  static constexpr const char* COMMENT = "comment";
  static constexpr const char* COMPRESS = "compress";
  static constexpr const char* CONVENTIONS = "Conventions";
  static constexpr const char* COORDINATES = "coordinates";
  static constexpr const char* COUPLER_FORWARD_LOSS_H = "coupler_forward_loss_h";
  static constexpr const char* COUPLER_FORWARD_LOSS_V = "coupler_forward_loss_v";
  static constexpr const char* CREATED = "created";
  static constexpr const char* DB = "dB";
  static constexpr const char* DBM = "dBm";
  static constexpr const char* DBZ = "dBZ";
  static constexpr const char* DBZ_CORRECTION = "dbz_correction";
  static constexpr const char* DEGREES = "degrees";
  static constexpr const char* DEGREES_EAST = "degrees_east";
  static constexpr const char* DEGREES_NORTH = "degrees_north";
  static constexpr const char* DEGREES_PER_SECOND = "degrees per second";
  static constexpr const char* DIELECTRIC_FACTOR_USED = "dielectric_factor_used";
  static constexpr const char* DORADE = "dorade";
  static constexpr const char* DOWN = "down";
  static constexpr const char* DRIFT = "drift";
  static constexpr const char* DRIFT_CORRECTION = "drift_correction";
  static constexpr const char* DRIVER = "driver";
  static constexpr const char* DRIVE_ANGLE_1 = "drive_angle_1";
  static constexpr const char* DRIVE_ANGLE_2 = "drive_angle_2";
  static constexpr const char* EASTWARD_VELOCITY = "eastward_velocity";
  static constexpr const char* EASTWARD_VELOCITY_CORRECTION = "eastward_velocity_correction";
  static constexpr const char* EASTWARD_WIND = "eastward_wind";
  static constexpr const char* ELEVATION = "elevation";
  static constexpr const char* ELEVATION_CORRECTION = "elevation_correction";
  static constexpr const char* END_DATETIME = "end_datetime";
  static constexpr const char* END_TIME = "end_time";
  static constexpr const char* FALSE_EASTING = "false_easting";
  static constexpr const char* FALSE_NORTHING = "false_northing";
  static constexpr const char* FFT_LENGTH = "fft_length";
  static constexpr const char* FIELD_FOLDS = "field_folds";
  static constexpr const char* FILL_VALUE = "_FillValue";
  static constexpr const char* FIXED_ANGLE = "fixed_angle";
  static constexpr const char* FLAG_MASKS = "flag_masks";
  static constexpr const char* FLAG_MEANINGS = "flag_meanings";
  static constexpr const char* FLAG_VALUES = "flag_values";
  static constexpr const char* FOLD_LIMIT_LOWER = "fold_limit_lower";
  static constexpr const char* FOLD_LIMIT_UPPER = "fold_limit_upper";
  static constexpr const char* FOLLOW_MODE = "follow_mode";
  static constexpr const char* FREQUENCY = "frequency";
  static constexpr const char* GATE_SPACING = "gate_spacing";
  static constexpr const char* GEOMETRY_CORRECTION = "geometry_correction";
  static constexpr const char* GEOREFERENCE = "georeference";
  static constexpr const char* GEOREFS_APPLIED = "georefs_applied";
  static constexpr const char* GEOREF_CORRECTION = "georef_correction";
  static constexpr const char* GEOREF_TIME = "georef_time";
  static constexpr const char* GEOREF_UNIT_ID = "georef_unit_id";
  static constexpr const char* GEOREF_UNIT_NUM = "georef_unit_num";
  static constexpr const char* GREGORIAN = "gregorian";
  static constexpr const char* GRID_MAPPING = "grid_mapping";
  static constexpr const char* GRID_MAPPING_NAME = "grid_mapping_name";
  static constexpr const char* HEADING = "heading";
  static constexpr const char* HEADING_CHANGE_RATE = "heading_change_rate";
  static constexpr const char* HEADING_CORRECTION = "heading_correction";
  static constexpr const char* HISTORY = "history";
  static constexpr const char* HZ = "s-1";
  static constexpr const char* INDEX_VAR_NAME = "index_var_name";
  static constexpr const char* INSTITUTION = "institution";
  static constexpr const char* INSTRUMENT_NAME = "instrument_name";
  static constexpr const char* INSTRUMENT_PARAMETERS = "instrument_parameters";
  static constexpr const char* INSTRUMENT_TYPE = "instrument_type";
  static constexpr const char* INTERMED_FREQ_HZ = "intermed_freq_hz";
  static constexpr const char* IS_DISCRETE = "is_discrete";
  static constexpr const char* IS_QUALITY = "is_quality";
  static constexpr const char* IS_SPECTRUM = "is_spectrum";
  static constexpr const char* JOULES = "joules";
  static constexpr const char* JULIAN = "julian";
  static constexpr const char* LATITUDE = "latitude";
  static constexpr const char* LATITUDE_CORRECTION = "latitude_correction";
  static constexpr const char* LATITUDE_OF_PROJECTION_ORIGIN = "latitude_of_projection_origin";
  static constexpr const char* LDR_CORRECTION_H = "ldr_correction_h";
  static constexpr const char* LDR_CORRECTION_V = "ldr_correction_v";
  static constexpr const char* LEGEND_XML = "legend_xml";
  static constexpr const char* LIDAR_APERTURE_DIAMETER = "lidar_aperture_diameter";
  static constexpr const char* LIDAR_APERTURE_EFFICIENCY = "lidar_aperture_efficiency";
  static constexpr const char* LIDAR_BEAM_DIVERGENCE = "lidar_beam_divergence";
  static constexpr const char* LIDAR_CALIBRATION = "lidar_calibration";
  static constexpr const char* LIDAR_CONSTANT = "lidar_constant";
  static constexpr const char* LIDAR_FIELD_OF_VIEW = "lidar_field_of_view";
  static constexpr const char* LIDAR_PARAMETERS = "lidar_parameters";
  static constexpr const char* LIDAR_PEAK_POWER = "lidar_peak_power";
  static constexpr const char* LIDAR_PULSE_ENERGY = "lidar_pulse_energy";
  static constexpr const char* LONGITUDE = "longitude";
  static constexpr const char* LONGITUDE_CORRECTION = "longitude_correction";
  static constexpr const char* LONGITUDE_OF_PROJECTION_ORIGIN = "longitude_of_projection_origin";
  static constexpr const char* LONG_NAME = "long_name";
  static constexpr const char* META_GROUP = "meta_group";
  static constexpr const char* METERS = "meters";
  static constexpr const char* METERS_BETWEEN_GATES = "meters_between_gates";
  static constexpr const char* METERS_PER_SECOND = "meters per second";
  static constexpr const char* METERS_TO_CENTER_OF_FIRST_GATE = "meters_to_center_of_first_gate";
  static constexpr const char* MISSING_VALUE = "missing_value";
  static constexpr const char* MONITORING = "monitoring";
  static constexpr const char* MOVING = "moving";
  static constexpr const char* MRAD = "mrad";
  static constexpr const char* NOISE_HC = "noise_hc";
  static constexpr const char* NOISE_HX = "noise_hx";
  static constexpr const char* NOISE_SOURCE_POWER_H = "noise_source_power_h";
  static constexpr const char* NOISE_SOURCE_POWER_V = "noise_source_power_v";
  static constexpr const char* NOISE_VC = "noise_vc";
  static constexpr const char* NOISE_VX = "noise_vx";
  static constexpr const char* NORTHWARD_VELOCITY = "northward_velocity";
  static constexpr const char* NORTHWARD_VELOCITY_CORRECTION = "northward_velocity_correction";
  static constexpr const char* NORTHWARD_WIND = "northward_wind";
  static constexpr const char* NYQUIST_VELOCITY = "nyquist_velocity";
  static constexpr const char* N_GATES_VARY = "n_gates_vary";
  static constexpr const char* N_POINTS = "n_points";
  static constexpr const char* N_PRTS = "n_prts";
  static constexpr const char* N_SAMPLES = "n_samples";
  static constexpr const char* N_SPECTRA = "n_spectra";
  static constexpr const char* OPTIONS = "options";
  static constexpr const char* ORIGINAL_FORMAT = "original_format";
  static constexpr const char* PERCENT = "percent";
  static constexpr const char* PITCH = "pitch";
  static constexpr const char* PITCH_CHANGE_RATE = "pitch_change_rate";
  static constexpr const char* PITCH_CORRECTION = "pitch_correction";
  static constexpr const char* PLATFORM_IS_MOBILE = "platform_is_mobile";
  static constexpr const char* PLATFORM_TYPE = "platform_type";
  static constexpr const char* PLATFORM_VELOCITY = "platform_velocity";
  static constexpr const char* POLARIZATION_MODE = "polarization_mode";
  static constexpr const char* POLARIZATION_SEQUENCE = "polarization_sequence";
  static constexpr const char* POSITIVE = "positive";
  static constexpr const char* POWER_MEASURE_LOSS_H = "power_measure_loss_h";
  static constexpr const char* POWER_MEASURE_LOSS_V = "power_measure_loss_v";
  static constexpr const char* PRESSURE_ALTITUDE_CORRECTION = "pressure_altitude_correction";
  static constexpr const char* PRIMARY_AXIS = "primary_axis";
  static constexpr const char* PROBERT_JONES_CORRECTION = "probert_jones_correction";
  static constexpr const char* PROPOSED_STANDARD_NAME = "proposed_standard_name";
  static constexpr const char* PRT = "prt";
  static constexpr const char* PRT_MODE = "prt_mode";
  static constexpr const char* PRT_RATIO = "prt_ratio";
  static constexpr const char* PRT_SEQUENCE = "prt_sequence";
  static constexpr const char* PULSE_WIDTH = "pulse_width";
  static constexpr const char* QC_PROCEDURES = "qc_procedures";
  static constexpr const char* QUALIFIED_VARIABLES = "qualified_variables";
  static constexpr const char* RADAR_ANTENNA_GAIN_H = "radar_antenna_gain_h";
  static constexpr const char* RADAR_ANTENNA_GAIN_V = "radar_antenna_gain_v";
  static constexpr const char* RADAR_BEAM_WIDTH_H = "radar_beam_width_h";
  static constexpr const char* RADAR_BEAM_WIDTH_V = "radar_beam_width_v";
  static constexpr const char* RADAR_CALIBRATION = "radar_calibration";
  static constexpr const char* RADAR_CONSTANT_H = "radar_constant_h";
  static constexpr const char* RADAR_CONSTANT_V = "radar_constant_v";
  static constexpr const char* RADAR_ESTIMATED_NOISE_DBM_HC = "estimated_noise_dbm_hc";
  static constexpr const char* RADAR_ESTIMATED_NOISE_DBM_HX = "estimated_noise_dbm_hx";
  static constexpr const char* RADAR_ESTIMATED_NOISE_DBM_VC = "estimated_noise_dbm_vc";
  static constexpr const char* RADAR_ESTIMATED_NOISE_DBM_VX = "estimated_noise_dbm_vx";
  static constexpr const char* RADAR_MEASURED_COLD_NOISE = "measured_transmit_cold_noise";
  static constexpr const char* RADAR_MEASURED_HOT_NOISE = "measured_transmit_hot_noise";
  static constexpr const char* RADAR_MEASURED_SKY_NOISE = "measured_transmit_sky_noise";
  static constexpr const char* RADAR_MEASURED_TRANSMIT_POWER_H = "measured_transmit_power_h";
  static constexpr const char* RADAR_MEASURED_TRANSMIT_POWER_V = "measured_transmit_power_v";
  static constexpr const char* RADAR_PARAMETERS = "radar_parameters";
  static constexpr const char* RADAR_RX_BANDWIDTH = "radar_rx_bandwidth";
  static constexpr const char* RANGE = "range";
  static constexpr const char* RANGE_CORRECTION = "range_correction";
  static constexpr const char* RAYS_ARE_INDEXED = "rays_are_indexed";
  static constexpr const char* RAY_ANGLE_RES = "ray_angle_res";
  static constexpr const char* RAY_ANGLE_RESOLUTION = "ray_angle_resolution";
  static constexpr const char* RAY_GATE_SPACING = "ray_gate_spacing";
  static constexpr const char* RAY_N_GATES = "ray_n_gates";
  static constexpr const char* RAY_START_INDEX = "ray_start_index";
  static constexpr const char* RAY_START_RANGE = "ray_start_range";
  static constexpr const char* RAY_TIMES_INCREASE = "ray_times_increase";
  static constexpr const char* RECEIVER_GAIN_HC = "receiver_gain_hc";
  static constexpr const char* RECEIVER_GAIN_HX = "receiver_gain_hx";
  static constexpr const char* RECEIVER_GAIN_VC = "receiver_gain_vc";
  static constexpr const char* RECEIVER_GAIN_VX = "receiver_gain_vx";
  static constexpr const char* RECEIVER_MISMATCH_LOSS = "receiver_mismatch_loss";
  static constexpr const char* RECEIVER_MISMATCH_LOSS_H = "receiver_mismatch_loss_h";
  static constexpr const char* RECEIVER_MISMATCH_LOSS_V = "receiver_mismatch_loss_v";
  static constexpr const char* RECEIVER_SLOPE_HC = "receiver_slope_hc";
  static constexpr const char* RECEIVER_SLOPE_HX = "receiver_slope_hx";
  static constexpr const char* RECEIVER_SLOPE_VC = "receiver_slope_vc";
  static constexpr const char* RECEIVER_SLOPE_VX = "receiver_slope_vx";
  static constexpr const char* REFERENCES = "references";
  static constexpr const char* ROLL = "roll";
  static constexpr const char* ROLL_CHANGE_RATE = "roll_change_rate";
  static constexpr const char* ROLL_CORRECTION = "roll_correction";
  static constexpr const char* ROTATION = "rotation";
  static constexpr const char* ROTATION_CORRECTION = "rotation_correction";
  static constexpr const char* RX_RANGE_RESOLUTION = "rx_range_resolution";
  static constexpr const char* R_CALIB = "r_calib";
  static constexpr const char* R_CALIB_ANTENNA_GAIN_H = "r_calib_antenna_gain_h";
  static constexpr const char* R_CALIB_ANTENNA_GAIN_V = "r_calib_antenna_gain_v";
  static constexpr const char* R_CALIB_BASE_DBZ_1KM_HC = "r_calib_base_dbz_1km_hc";
  static constexpr const char* R_CALIB_BASE_DBZ_1KM_HX = "r_calib_base_dbz_1km_hx";
  static constexpr const char* R_CALIB_BASE_DBZ_1KM_VC = "r_calib_base_dbz_1km_vc";
  static constexpr const char* R_CALIB_BASE_DBZ_1KM_VX = "r_calib_base_dbz_1km_vx";
  static constexpr const char* R_CALIB_COUPLER_FORWARD_LOSS_H = "r_calib_coupler_forward_loss_h";
  static constexpr const char* R_CALIB_COUPLER_FORWARD_LOSS_V = "r_calib_coupler_forward_loss_v";
  static constexpr const char* R_CALIB_DBZ_CORRECTION = "r_calib_dbz_correction";
  static constexpr const char* R_CALIB_DIELECTRIC_FACTOR_USED = "r_calib_dielectric_factor_used";
  static constexpr const char* R_CALIB_DYNAMIC_RANGE_DB_HC = "r_calib_dynamic_range_db_hc";
  static constexpr const char* R_CALIB_DYNAMIC_RANGE_DB_HX = "r_calib_dynamic_range_db_hx";
  static constexpr const char* R_CALIB_DYNAMIC_RANGE_DB_VC = "r_calib_dynamic_range_db_vc";
  static constexpr const char* R_CALIB_DYNAMIC_RANGE_DB_VX = "r_calib_dynamic_range_db_vx";
  static constexpr const char* R_CALIB_I0_DBM_HC = "r_calib_i0_dbm_hc";
  static constexpr const char* R_CALIB_I0_DBM_HX = "r_calib_i0_dbm_hx";
  static constexpr const char* R_CALIB_I0_DBM_VC = "r_calib_i0_dbm_vc";
  static constexpr const char* R_CALIB_I0_DBM_VX = "r_calib_i0_dbm_vx";
  static constexpr const char* R_CALIB_INDEX = "r_calib_index";
  static constexpr const char* R_CALIB_K_SQUARED_WATER = "r_calib_k_squared_water";
  static constexpr const char* R_CALIB_LDR_CORRECTION_H = "r_calib_ldr_correction_h";
  static constexpr const char* R_CALIB_LDR_CORRECTION_V = "r_calib_ldr_correction_v";
  static constexpr const char* R_CALIB_NOISE_HC = "r_calib_noise_hc";
  static constexpr const char* R_CALIB_NOISE_HX = "r_calib_noise_hx";
  static constexpr const char* R_CALIB_NOISE_SOURCE_POWER_H = "r_calib_noise_source_power_h";
  static constexpr const char* R_CALIB_NOISE_SOURCE_POWER_V = "r_calib_noise_source_power_v";
  static constexpr const char* R_CALIB_NOISE_VC = "r_calib_noise_vc";
  static constexpr const char* R_CALIB_NOISE_VX = "r_calib_noise_vx";
  static constexpr const char* R_CALIB_POWER_MEASURE_LOSS_H = "r_calib_power_measure_loss_h";
  static constexpr const char* R_CALIB_POWER_MEASURE_LOSS_V = "r_calib_power_measure_loss_v";
  static constexpr const char* R_CALIB_PROBERT_JONES_CORRECTION = "r_calib_probert_jones_correction";
  static constexpr const char* R_CALIB_PULSE_WIDTH = "r_calib_pulse_width";
  static constexpr const char* R_CALIB_RADAR_CONSTANT_H = "r_calib_radar_constant_h";
  static constexpr const char* R_CALIB_RADAR_CONSTANT_V = "r_calib_radar_constant_v";
  static constexpr const char* R_CALIB_RECEIVER_GAIN_HC = "r_calib_receiver_gain_hc";
  static constexpr const char* R_CALIB_RECEIVER_GAIN_HX = "r_calib_receiver_gain_hx";
  static constexpr const char* R_CALIB_RECEIVER_GAIN_VC = "r_calib_receiver_gain_vc";
  static constexpr const char* R_CALIB_RECEIVER_GAIN_VX = "r_calib_receiver_gain_vx";
  static constexpr const char* R_CALIB_RECEIVER_MISMATCH_LOSS = "r_calib_receiver_mismatch_loss";
  static constexpr const char* R_CALIB_RECEIVER_MISMATCH_LOSS_H = "r_calib_receiver_mismatch_loss_h";
  static constexpr const char* R_CALIB_RECEIVER_MISMATCH_LOSS_V = "r_calib_receiver_mismatch_loss_v";
  static constexpr const char* R_CALIB_RECEIVER_SLOPE_HC = "r_calib_receiver_slope_hc";
  static constexpr const char* R_CALIB_RECEIVER_SLOPE_HX = "r_calib_receiver_slope_hx";
  static constexpr const char* R_CALIB_RECEIVER_SLOPE_VC = "r_calib_receiver_slope_vc";
  static constexpr const char* R_CALIB_RECEIVER_SLOPE_VX = "r_calib_receiver_slope_vx";
  static constexpr const char* R_CALIB_SUN_POWER_HC = "r_calib_sun_power_hc";
  static constexpr const char* R_CALIB_SUN_POWER_HX = "r_calib_sun_power_hx";
  static constexpr const char* R_CALIB_SUN_POWER_VC = "r_calib_sun_power_vc";
  static constexpr const char* R_CALIB_SUN_POWER_VX = "r_calib_sun_power_vx";
  static constexpr const char* R_CALIB_SYSTEM_PHIDP = "r_calib_system_phidp";
  static constexpr const char* R_CALIB_TEST_POWER_H = "r_calib_test_power_h";
  static constexpr const char* R_CALIB_TEST_POWER_V = "r_calib_test_power_v";
  static constexpr const char* R_CALIB_TIME = "r_calib_time";
  static constexpr const char* R_CALIB_TIME_W3C_STR = "r_calib_time_w3c_str";
  static constexpr const char* R_CALIB_TWO_WAY_RADOME_LOSS_H = "r_calib_two_way_radome_loss_h";
  static constexpr const char* R_CALIB_TWO_WAY_RADOME_LOSS_V = "r_calib_two_way_radome_loss_v";
  static constexpr const char* R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_H = "r_calib_two_way_waveguide_loss_h";
  static constexpr const char* R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_V = "r_calib_two_way_waveguide_loss_v";
  static constexpr const char* R_CALIB_XMIT_POWER_H = "r_calib_xmit_power_h";
  static constexpr const char* R_CALIB_XMIT_POWER_V = "r_calib_xmit_power_v";
  static constexpr const char* R_CALIB_ZDR_CORRECTION = "r_calib_zdr_correction";
  static constexpr const char* SAMPLING_RATIO = "sampling_ratio";
  static constexpr const char* SCALE_FACTOR = "scale_factor";
  static constexpr const char* SCANNING = "scanning";
  static constexpr const char* SCANNING_RADIAL = "scanning_radial";
  static constexpr const char* SCAN_ID = "scan_id";
  static constexpr const char* SCAN_NAME = "scan_name";
  static constexpr const char* SCAN_RATE = "scan_rate";
  static constexpr const char* SECONDS = "seconds";
  static constexpr const char* SECS_SINCE_JAN1_1970 = "seconds since 1970-01-01T00:00:00Z";
  static constexpr const char* SITE_NAME = "site_name";
  static constexpr const char* SOURCE = "source";
  static constexpr const char* SPACING_IS_CONSTANT = "spacing_is_constant";
  static constexpr const char* SPECTRUM_N_SAMPLES = "spectrum_n_samples";
  static constexpr const char* STANDARD = "standard";
  static constexpr const char* STANDARD_NAME = "standard_name";
  static constexpr const char* STARING = "staring";
  static constexpr const char* START_DATETIME = "start_datetime";
  static constexpr const char* START_RANGE = "start_range";
  static constexpr const char* START_TIME = "start_time";
  static constexpr const char* STATIONARY = "stationary";
  static constexpr const char* STATUS_XML = "status_xml";
  static constexpr const char* STATUS_XML_LENGTH = "status_xml_length";
  static constexpr const char* STRING_LENGTH_256 = "string_length_256";
  static constexpr const char* STRING_LENGTH_32 = "string_length_32";
  static constexpr const char* STRING_LENGTH_64 = "string_length_64";
  static constexpr const char* STRING_LENGTH_8 = "string_length_8";
  static constexpr const char* SUB_CONVENTIONS = "Sub_conventions";
  static constexpr const char* SUN_POWER_HC = "sun_power_hc";
  static constexpr const char* SUN_POWER_HX = "sun_power_hx";
  static constexpr const char* SUN_POWER_VC = "sun_power_vc";
  static constexpr const char* SUN_POWER_VX = "sun_power_vx";
  static constexpr const char* SWEEP = "sweep";
  static constexpr const char* SWEEP_END_RAY_INDEX = "sweep_end_ray_index";
  static constexpr const char* SWEEP_FIXED_ANGLE = "sweep_fixed_angle";
  static constexpr const char* SWEEP_GROUP_NAME = "sweep_group_name";
  static constexpr const char* SWEEP_MODE = "sweep_mode";
  static constexpr const char* SWEEP_NUMBER = "sweep_number";
  static constexpr const char* SWEEP_START_RAY_INDEX = "sweep_start_ray_index";
  static constexpr const char* SYSTEM_PHIDP = "system_phidp";
  static constexpr const char* TARGET_SCAN_RATE = "target_scan_rate";
  static constexpr const char* TELESCOPE_ROLL_ANGLE_OFFSET = "telescope_roll_angle_offset";
  static constexpr const char* TEST_POWER_H = "test_power_h";
  static constexpr const char* TEST_POWER_V = "test_power_v";
  static constexpr const char* THRESHOLDING_XML = "thresholding_xml";
  static constexpr const char* TILT = "tilt";
  static constexpr const char* TILT_CORRECTION = "tilt_correction";
  static constexpr const char* TIME = "time";
  static constexpr const char* TIME_COVERAGE_END = "time_coverage_end";
  static constexpr const char* TIME_COVERAGE_START = "time_coverage_start";
  static constexpr const char* TIME_W3C_STR = "time_w3c_str";
  static constexpr const char* TITLE = "title";
  static constexpr const char* TRACK = "track";
  static constexpr const char* TRACK_REL_ROT = "track_rel_rot";
  static constexpr const char* TRACK_REL_TILT = "track_rel_tilt";
  static constexpr const char* TRACK_REL_AZ = "track_rel_az";
  static constexpr const char* TRACK_REL_EL = "track_rel_el";
  static constexpr const char* TWO_WAY_RADOME_LOSS_H = "two_way_radome_loss_h";
  static constexpr const char* TWO_WAY_RADOME_LOSS_V = "two_way_radome_loss_v";
  static constexpr const char* TWO_WAY_WAVEGUIDE_LOSS_H = "two_way_waveguide_loss_h";
  static constexpr const char* TWO_WAY_WAVEGUIDE_LOSS_V = "two_way_waveguide_loss_v";
  static constexpr const char* UNAMBIGUOUS_RANGE = "unambiguous_range";
  static constexpr const char* UNITS = "units";
  static constexpr const char* UP = "up";
  static constexpr const char* VALID_MAX = "valid_max";
  static constexpr const char* VALID_MIN = "valid_min";
  static constexpr const char* VALID_RANGE = "valid_range";
  static constexpr const char* VERSION = "version";
  static constexpr const char* VERTICAL_VELOCITY = "vertical_velocity";
  static constexpr const char* VERTICAL_VELOCITY_CORRECTION = "vertical_velocity_correction";
  static constexpr const char* VERTICAL_WIND = "vertical_wind";
  static constexpr const char* VOLUME = "volume";
  static constexpr const char* VOLUME_NUMBER = "volume_number";
  static constexpr const char* W3C_STR = "w3c_str";
  static constexpr const char* WATTS = "watts";
  static constexpr const char* XMIT_POWER_H = "xmit_power_h";
  static constexpr const char* XMIT_POWER_V = "xmit_power_v";
  static constexpr const char* ZDR_CORRECTION = "zdr_correction";

  // long names for metadata

  static constexpr const char* ALTITUDE_AGL_LONG = "altitude_above_ground_level";
  static constexpr const char* ALTITUDE_CORRECTION_LONG = "altitude_correction";
  static constexpr const char* ALTITUDE_LONG = "altitude";
  static constexpr const char* ANTENNA_GAIN_H_LONG = "calibrated_radar_antenna_gain_h_channel";
  static constexpr const char* ANTENNA_GAIN_V_LONG = "calibrated_radar_antenna_gain_v_channel";
  static constexpr const char* ANTENNA_TRANSITION_LONG = "antenna_is_in_transition_between_sweeps";
  static constexpr const char* AZIMUTH_CORRECTION_LONG = "azimuth_angle_correction";
  static constexpr const char* AZIMUTH_LONG = "azimuth_angle_from_true_north";
  static constexpr const char* AZIMUTH_STANDARD = "ray_azimuth_angle";
  static constexpr const char* BASE_DBZ_1KM_HC_LONG = "radar_reflectivity_at_1km_at_zero_snr_h_co_polar_channel";
  static constexpr const char* BASE_DBZ_1KM_HX_LONG = "radar_reflectivity_at_1km_at_zero_snr_h_cross_polar_channel";
  static constexpr const char* BASE_DBZ_1KM_VC_LONG = "radar_reflectivity_at_1km_at_zero_snr_v_co_polar_channel";
  static constexpr const char* BASE_DBZ_1KM_VX_LONG = "radar_reflectivity_at_1km_at_zero_snr_v_cross_polar_channel";
  static constexpr const char* COUPLER_FORWARD_LOSS_H_LONG = "radar_calibration_coupler_forward_loss_h_channel";
  static constexpr const char* COUPLER_FORWARD_LOSS_V_LONG = "radar_calibration_coupler_forward_loss_v_channel";
  static constexpr const char* CO_TO_CROSS_POLAR_CORRELATION_RATIO_H = "co_to_cross_polar_correlation_ratio_h";
  static constexpr const char* CO_TO_CROSS_POLAR_CORRELATION_RATIO_V = "co_to_cross_polar_correlation_ratio_v";
  static constexpr const char* CROSS_POLAR_DIFFERENTIAL_PHASE = "cross_polar_differential_phase";
  static constexpr const char* CROSS_SPECTRUM_OF_COPOLAR_HORIZONTAL = "cross_spectrum_of_copolar_horizontal";
  static constexpr const char* CROSS_SPECTRUM_OF_COPOLAR_VERTICAL = "cross_spectrum_of_copolar_vertical";
  static constexpr const char* CROSS_SPECTRUM_OF_CROSSPOLAR_HORIZONTAL = "cross_spectrum_of_crosspolar_horizontal";
  static constexpr const char* CROSS_SPECTRUM_OF_CROSSPOLAR_VERTICAL = "cross_spectrum_of_crosspolar_vertical";
  static constexpr const char* DBZ_CORRECTION_LONG = "calibrated_radar_dbz_correction";
  static constexpr const char* DRIFT_CORRECTION_LONG = "platform_drift_angle_correction";
  static constexpr const char* DRIFT_LONG = "platform_drift_angle";
  static constexpr const char* DRIVE_ANGLE_1_LONG = "antenna_drive_angle_1";
  static constexpr const char* DRIVE_ANGLE_2_LONG = "antenna_drive_angle_2";
  static constexpr const char* EASTWARD_VELOCITY_CORRECTION_LONG = "platform_eastward_velocity_correction";
  static constexpr const char* EASTWARD_VELOCITY_LONG = "platform_eastward_velocity";
  static constexpr const char* EASTWARD_WIND_LONG = "eastward_wind_speed";
  static constexpr const char* ELEVATION_CORRECTION_LONG = "ray_elevation_angle_correction";
  static constexpr const char* ELEVATION_LONG = "elevation_angle_from_horizontal_plane";
  static constexpr const char* ELEVATION_STANDARD = "ray_elevation_angle";
  static constexpr const char* FIXED_ANGLE_LONG = "ray_target_fixed_angle";
  static constexpr const char* FOLLOW_MODE_LONG = "follow_mode_for_scan_strategy";
  static constexpr const char* FREQUENCY_LONG = "transmission_frequency";
  static constexpr const char* GEOREF_TIME_LONG = "georef time in seconds since volume start";
  static constexpr const char* GEOREF_UNIT_ID_LONG = "georef hardware id or serial number";
  static constexpr const char* GEOREF_UNIT_NUM_LONG = "georef hardware unit number";
  static constexpr const char* HEADING_CHANGE_RATE_LONG = "platform_heading_angle_rate_of_change";
  static constexpr const char* HEADING_CORRECTION_LONG = "platform_heading_angle_correction";
  static constexpr const char* HEADING_LONG = "platform_heading_angle";
  static constexpr const char* INDEX_LONG = "calibration_data_array_index_per_ray";
  static constexpr const char* INSTRUMENT_NAME_LONG = "name_of_instrument";
  static constexpr const char* INSTRUMENT_TYPE_LONG = "type_of_instrument";
  static constexpr const char* INTERMED_FREQ_HZ_LONG = "intermediate_freqency_hz";
  static constexpr const char* LATITUDE_CORRECTION_LONG = "latitude_correction";
  static constexpr const char* LATITUDE_LONG = "latitude";
  static constexpr const char* LDR_CORRECTION_H_LONG = "calibrated_radar_ldr_correction_h_channel";
  static constexpr const char* LDR_CORRECTION_V_LONG = "calibrated_radar_ldr_correction_v_channel";
  static constexpr const char* LIDAR_APERTURE_DIAMETER_LONG = "lidar_aperture_diameter";
  static constexpr const char* LIDAR_APERTURE_EFFICIENCY_LONG = "lidar_aperture_efficiency";
  static constexpr const char* LIDAR_BEAM_DIVERGENCE_LONG = "lidar_beam_divergence";
  static constexpr const char* LIDAR_CONSTANT_LONG = "lidar_calibration_constant";
  static constexpr const char* LIDAR_FIELD_OF_VIEW_LONG = "lidar_field_of_view";
  static constexpr const char* LIDAR_PEAK_POWER_LONG = "lidar_peak_power";
  static constexpr const char* LIDAR_PULSE_ENERGY_LONG = "lidar_pulse_energy";
  static constexpr const char* LONGITUDE_CORRECTION_LONG = "longitude_correction";
  static constexpr const char* LONGITUDE_LONG = "longitude";
  static constexpr const char* NOISE_HC_LONG = "calibrated_radar_receiver_noise_h_co_polar_channel";
  static constexpr const char* NOISE_HX_LONG = "calibrated_radar_receiver_noise_h_cross_polar_channel";
  static constexpr const char* NOISE_SOURCE_POWER_H_LONG = "radar_calibration_noise_source_power_h_channel";
  static constexpr const char* NOISE_SOURCE_POWER_V_LONG = "radar_calibration_noise_source_power_v_channel";
  static constexpr const char* NOISE_VC_LONG = "calibrated_radar_receiver_noise_v_co_polar_channel";
  static constexpr const char* NOISE_VX_LONG = "calibrated_radar_receiver_noise_v_cross_polar_channel";
  static constexpr const char* NORTHWARD_VELOCITY_CORRECTION_LONG = "platform_northward_velocity_correction";
  static constexpr const char* NORTHWARD_VELOCITY_LONG = "platform_northward_velocity";
  static constexpr const char* NORTHWARD_WIND_LONG = "northward_wind";
  static constexpr const char* NYQUIST_VELOCITY_LONG = "unambiguous_doppler_velocity";
  static constexpr const char* N_SAMPLES_LONG = "number_of_samples_used_to_compute_moments";
  static constexpr const char* PITCH_CHANGE_RATE_LONG = "platform_pitch_angle_rate_of_change";
  static constexpr const char* PITCH_CORRECTION_LONG = "platform_pitch_angle_correction";
  static constexpr const char* PITCH_LONG = "platform_pitch_angle";
  static constexpr const char* PLATFORM_IS_MOBILE_LONG = "platform_is_mobile";
  static constexpr const char* PLATFORM_TYPE_LONG = "platform_type";
  static constexpr const char* POLARIZATION_MODE_LONG = "polarization_mode_for_sweep";
  static constexpr const char* POWER_MEASURE_LOSS_H_LONG = "radar_calibration_power_measurement_loss_h_channel";
  static constexpr const char* POWER_MEASURE_LOSS_V_LONG = "radar_calibration_power_measurement_loss_v_channel";
  static constexpr const char* PRESSURE_ALTITUDE_CORRECTION_LONG = "pressure_altitude_correction";
  static constexpr const char* PRIMARY_AXIS_LONG = "primary_axis_of_rotation";
  static constexpr const char* PRT_LONG = "pulse_repetition_time";
  static constexpr const char* PRT_MODE_LONG = "transmit_pulse_mode";
  static constexpr const char* PRT_RATIO_LONG = "pulse_repetition_frequency_ratio";
  static constexpr const char* PULSE_WIDTH_LONG = "transmitter_pulse_width";
  static constexpr const char* RADAR_ANTENNA_GAIN_H_LONG = "nominal_radar_antenna_gain_h_channel";
  static constexpr const char* RADAR_ANTENNA_GAIN_V_LONG = "nominal_radar_antenna_gain_v_channel";
  static constexpr const char* RADAR_BEAM_WIDTH_H_LONG = "half_power_radar_beam_width_h_channel";
  static constexpr const char* RADAR_BEAM_WIDTH_V_LONG = "half_power_radar_beam_width_v_channel";
  static constexpr const char* RADAR_CONSTANT_H_LONG = "calibrated_radar_constant_h_channel";
  static constexpr const char* RADAR_CONSTANT_V_LONG = "calibrated_radar_constant_v_channel";
  static constexpr const char* RADAR_ESTIMATED_NOISE_DBM_HC_LONG = "estimated_noise_dbm_hc";
  static constexpr const char* RADAR_ESTIMATED_NOISE_DBM_HX_LONG = "estimated_noise_dbm_hx";
  static constexpr const char* RADAR_ESTIMATED_NOISE_DBM_VC_LONG = "estimated_noise_dbm_vc";
  static constexpr const char* RADAR_ESTIMATED_NOISE_DBM_VX_LONG = "estimated_noise_dbm_vx";
  static constexpr const char* RADAR_MEASURED_TRANSMIT_POWER_H_LONG = "measured_radar_transmit_power_h_channel";
  static constexpr const char* RADAR_MEASURED_TRANSMIT_POWER_V_LONG = "measured_radar_transmit_power_v_channel";
  static constexpr const char* RADAR_RX_BANDWIDTH_LONG = "radar_receiver_bandwidth";
  static constexpr const char* RANGE_CORRECTION_LONG = "range_to_center_of_measurement_volume_correction";
  static constexpr const char* RANGE_LONG = "range_to_center_of_measurement_volume";
  static constexpr const char* RAYS_ARE_INDEXED_LONG = "flag_for_indexed_rays";
  static constexpr const char* RAY_ANGLE_RES_LONG = "angular_resolution_between_rays";
  static constexpr const char* RAY_ANGLE_RESOLUTION_LONG = "angular_resolution_between_rays";
  static constexpr const char* RECEIVER_GAIN_HC_LONG = "calibrated_radar_receiver_gain_h_co_polar_channel";
  static constexpr const char* RECEIVER_GAIN_HX_LONG = "calibrated_radar_receiver_gain_h_cross_polar_channel";
  static constexpr const char* RECEIVER_GAIN_VC_LONG = "calibrated_radar_receiver_gain_v_co_polar_channel";
  static constexpr const char* RECEIVER_GAIN_VX_LONG = "calibrated_radar_receiver_gain_v_cross_polar_channel";
  static constexpr const char* RECEIVER_MISMATCH_LOSS_LONG = "radar_calibration_receiver_mismatch_loss";
  static constexpr const char* RECEIVER_SLOPE_HC_LONG = "calibrated_radar_receiver_slope_h_co_polar_channel";
  static constexpr const char* RECEIVER_SLOPE_HX_LONG = "calibrated_radar_receiver_slope_h_cross_polar_channel";
  static constexpr const char* RECEIVER_SLOPE_VC_LONG = "calibrated_radar_receiver_slope_v_co_polar_channel";
  static constexpr const char* RECEIVER_SLOPE_VX_LONG = "calibrated_radar_receiver_slope_v_cross_polar_channel";
  static constexpr const char* ROLL_CHANGE_RATE_LONG = "platform_roll_angle_rate_of_change";
  static constexpr const char* ROLL_CORRECTION_LONG = "platform_roll_angle_correction";
  static constexpr const char* ROLL_LONG = "platform_roll_angle";
  static constexpr const char* ROTATION_CORRECTION_LONG = "ray_rotation_angle_relative_to_platform_correction";
  static constexpr const char* ROTATION_LONG = "ray_rotation_angle_relative_to_platform";
  static constexpr const char* R_CALIB_ANTENNA_GAIN_H_LONG = "calibrated_radar_antenna_gain_h_channel";
  static constexpr const char* R_CALIB_ANTENNA_GAIN_V_LONG = "calibrated_radar_antenna_gain_v_channel";
  static constexpr const char* R_CALIB_BASE_DBZ_1KM_HC_LONG = "radar_reflectivity_at_1km_at_zero_snr_h_co_polar_channel";
  static constexpr const char* R_CALIB_BASE_DBZ_1KM_HX_LONG = "radar_reflectivity_at_1km_at_zero_snr_h_cross_polar_channel";
  static constexpr const char* R_CALIB_BASE_DBZ_1KM_VC_LONG = "radar_reflectivity_at_1km_at_zero_snr_v_co_polar_channel";
  static constexpr const char* R_CALIB_BASE_DBZ_1KM_VX_LONG = "radar_reflectivity_at_1km_at_zero_snr_v_cross_polar_channel";
  static constexpr const char* R_CALIB_COUPLER_FORWARD_LOSS_H_LONG = "radar_calibration_coupler_forward_loss_h_channel";
  static constexpr const char* R_CALIB_COUPLER_FORWARD_LOSS_V_LONG = "radar_calibration_coupler_forward_loss_v_channel";
  static constexpr const char* R_CALIB_DBZ_CORRECTION_LONG = "calibrated_radar_dbz_correction";
  static constexpr const char* R_CALIB_INDEX_LONG = "calibration_data_array_index_per_ray";
  static constexpr const char* R_CALIB_LDR_CORRECTION_H_LONG = "calibrated_radar_ldr_correction_h_channel";
  static constexpr const char* R_CALIB_LDR_CORRECTION_V_LONG = "calibrated_radar_ldr_correction_v_channel";
  static constexpr const char* R_CALIB_NOISE_HC_LONG = "calibrated_radar_receiver_noise_h_co_polar_channel";
  static constexpr const char* R_CALIB_NOISE_HX_LONG = "calibrated_radar_receiver_noise_h_cross_polar_channel";
  static constexpr const char* R_CALIB_NOISE_SOURCE_POWER_H_LONG = "radar_calibration_noise_source_power_h_channel";
  static constexpr const char* R_CALIB_NOISE_SOURCE_POWER_V_LONG = "radar_calibration_noise_source_power_v_channel";
  static constexpr const char* R_CALIB_NOISE_VC_LONG = "calibrated_radar_receiver_noise_v_co_polar_channel";
  static constexpr const char* R_CALIB_NOISE_VX_LONG = "calibrated_radar_receiver_noise_v_cross_polar_channel";
  static constexpr const char* R_CALIB_POWER_MEASURE_LOSS_H_LONG = "radar_calibration_power_measurement_loss_h_channel";
  static constexpr const char* R_CALIB_POWER_MEASURE_LOSS_V_LONG = "radar_calibration_power_measurement_loss_v_channel";
  static constexpr const char* R_CALIB_PULSE_WIDTH_LONG = "radar_calibration_pulse_width";
  static constexpr const char* R_CALIB_RADAR_CONSTANT_H_LONG = "calibrated_radar_constant_h_channel";
  static constexpr const char* R_CALIB_RADAR_CONSTANT_V_LONG = "calibrated_radar_constant_v_channel";
  static constexpr const char* R_CALIB_RECEIVER_GAIN_HC_LONG = "calibrated_radar_receiver_gain_h_co_polar_channel";
  static constexpr const char* R_CALIB_RECEIVER_GAIN_HX_LONG = "calibrated_radar_receiver_gain_h_cross_polar_channel";
  static constexpr const char* R_CALIB_RECEIVER_GAIN_VC_LONG = "calibrated_radar_receiver_gain_v_co_polar_channel";
  static constexpr const char* R_CALIB_RECEIVER_GAIN_VX_LONG = "calibrated_radar_receiver_gain_v_cross_polar_channel";
  static constexpr const char* R_CALIB_RECEIVER_MISMATCH_LOSS_LONG = "radar_calibration_receiver_mismatch_loss";
  static constexpr const char* R_CALIB_RECEIVER_SLOPE_HC_LONG = "calibrated_radar_receiver_slope_h_co_polar_channel";
  static constexpr const char* R_CALIB_RECEIVER_SLOPE_HX_LONG = "calibrated_radar_receiver_slope_h_cross_polar_channel";
  static constexpr const char* R_CALIB_RECEIVER_SLOPE_VC_LONG = "calibrated_radar_receiver_slope_v_co_polar_channel";
  static constexpr const char* R_CALIB_RECEIVER_SLOPE_VX_LONG = "calibrated_radar_receiver_slope_v_cross_polar_channel";
  static constexpr const char* R_CALIB_SUN_POWER_HC_LONG = "calibrated_radar_sun_power_h_co_polar_channel";
  static constexpr const char* R_CALIB_SUN_POWER_HX_LONG = "calibrated_radar_sun_power_h_cross_polar_channel";
  static constexpr const char* R_CALIB_SUN_POWER_VC_LONG = "calibrated_radar_sun_power_v_co_polar_channel";
  static constexpr const char* R_CALIB_SUN_POWER_VX_LONG = "calibrated_radar_sun_power_v_cross_polar_channel";
  static constexpr const char* R_CALIB_SYSTEM_PHIDP_LONG = "calibrated_radar_system_phidp";
  static constexpr const char* R_CALIB_TEST_POWER_H_LONG = "radar_calibration_test_power_h_channel";
  static constexpr const char* R_CALIB_TEST_POWER_V_LONG = "radar_calibration_test_power_v_channel";
  static constexpr const char* R_CALIB_TIME_LONG = "radar_calibration_time_utc";
  static constexpr const char* R_CALIB_TWO_WAY_RADOME_LOSS_H_LONG = "radar_calibration_two_way_radome_loss_h_channel";
  static constexpr const char* R_CALIB_TWO_WAY_RADOME_LOSS_V_LONG = "radar_calibration_two_way_radome_loss_v_channel";
  static constexpr const char* R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_H_LONG = "radar_calibration_two_way_waveguide_loss_h_channel";
  static constexpr const char* R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_V_LONG = "radar_calibration_two_way_waveguide_loss_v_channel";
  static constexpr const char* R_CALIB_XMIT_POWER_H_LONG = "calibrated_radar_xmit_power_h_channel";
  static constexpr const char* R_CALIB_XMIT_POWER_V_LONG = "calibrated_radar_xmit_power_v_channel";
  static constexpr const char* R_CALIB_ZDR_CORRECTION_LONG = "calibrated_radar_zdr_correction";
  static constexpr const char* SCAN_ID_LONG = "volume_coverage_pattern";
  static constexpr const char* SCAN_NAME_LONG = "name_of_antenna_scan_strategy";
  static constexpr const char* SCAN_RATE_LONG = "antenna_angle_scan_rate";
  static constexpr const char* SITE_NAME_LONG = "name_of_instrument_site";
  static constexpr const char* SPACING_IS_CONSTANT_LONG = "spacing_between_range_gates_is_constant";
  static constexpr const char* SPECTRUM_COPOLAR_HORIZONTAL = "spectrum_copolar_horizontal";
  static constexpr const char* SPECTRUM_COPOLAR_VERTICAL = "spectrum_copolar_vertical";
  static constexpr const char* SPECTRUM_CROSSPOLAR_HORIZONTAL = "spectrum_crosspolar_horizontal";
  static constexpr const char* SPECTRUM_CROSSPOLAR_VERTICAL = "spectrum_crosspolar_vertical";
  static constexpr const char* SUN_POWER_HC_LONG = "calibrated_radar_sun_power_h_co_polar_channel";
  static constexpr const char* SUN_POWER_HX_LONG = "calibrated_radar_sun_power_h_cross_polar_channel";
  static constexpr const char* SUN_POWER_VC_LONG = "calibrated_radar_sun_power_v_co_polar_channel";
  static constexpr const char* SUN_POWER_VX_LONG = "calibrated_radar_sun_power_v_cross_polar_channel";
  static constexpr const char* SWEEP_END_RAY_INDEX_LONG = "index_of_last_ray_in_sweep";
  static constexpr const char* SWEEP_FIXED_ANGLE_LONG = "fixed_angle_for_sweep";
  static constexpr const char* SWEEP_GROUP_NAME_LONG = "group_name_for_sweep";
  static constexpr const char* SWEEP_MODE_LONG = "scan_mode_for_sweep";
  static constexpr const char* SWEEP_NUMBER_LONG = "sweep_index_number_0_based";
  static constexpr const char* SWEEP_START_RAY_INDEX_LONG = "index_of_first_ray_in_sweep";
  static constexpr const char* SYSTEM_PHIDP_LONG = "calibrated_radar_system_phidp";
  static constexpr const char* TARGET_SCAN_RATE_LONG = "target_scan_rate_for_sweep";
  static constexpr const char* TEST_POWER_H_LONG = "radar_calibration_test_power_h_channel";
  static constexpr const char* TEST_POWER_V_LONG = "radar_calibration_test_power_v_channel";
  static constexpr const char* TILT_CORRECTION_LONG = "ray_tilt_angle_relative_to_platform_correction";
  static constexpr const char* TILT_LONG = "ray_tilt_angle_relative_to_platform";
  static constexpr const char* TIME_COVERAGE_END_LONG = "data_volume_end_time_utc";
  static constexpr const char* TIME_COVERAGE_START_LONG = "data_volume_start_time_utc";
  static constexpr const char* TRACK_LONG = "platform_track_over_the_ground";
  static constexpr const char* TRACK_REL_ROT_LONG = "track_relative_rotation_angle";
  static constexpr const char* TRACK_REL_TILT_LONG = "track_relative_tilt_angle";
  static constexpr const char* TRACK_REL_AZ_LONG = "track_relative_azimith_angle";
  static constexpr const char* TRACK_REL_EL_LONG = "track_relative_elevation_angle";
  static constexpr const char* TWO_WAY_RADOME_LOSS_H_LONG = "radar_calibration_two_way_radome_loss_h_channel";
  static constexpr const char* TWO_WAY_RADOME_LOSS_V_LONG = "radar_calibration_two_way_radome_loss_v_channel";
  static constexpr const char* TWO_WAY_WAVEGUIDE_LOSS_H_LONG = "radar_calibration_two_way_waveguide_loss_h_channel";
  static constexpr const char* TWO_WAY_WAVEGUIDE_LOSS_V_LONG = "radar_calibration_two_way_waveguide_loss_v_channel";
  static constexpr const char* UNAMBIGUOUS_RANGE_LONG = "unambiguous_range";
  static constexpr const char* VERTICAL_VELOCITY_CORRECTION_LONG = "platform_vertical_velocity_correction";
  static constexpr const char* VERTICAL_VELOCITY_LONG = "platform_vertical_velocity";
  static constexpr const char* VERTICAL_WIND_LONG = "upward_air_velocity";
  static constexpr const char* VOLUME_NUMBER_LONG = "data_volume_index_number";
  static constexpr const char* XMIT_POWER_H_LONG = "calibrated_radar_xmit_power_h_channel";
  static constexpr const char* XMIT_POWER_V_LONG = "calibrated_radar_xmit_power_v_channel";
  static constexpr const char* ZDR_CORRECTION_LONG = "calibrated_radar_zdr_correction";

protected:

  // create set of ray variable names

  void _createRayMetaNameSet();
  set<string> _rayMetaNames;

private:

};

#endif
