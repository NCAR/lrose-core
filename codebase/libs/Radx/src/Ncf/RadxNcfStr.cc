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
// RadxNcfStr.cc
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

#include <Radx/RadxNcfStr.hh>

//////////////
// Constructor

RadxNcfStr::RadxNcfStr()
  
{

  // conventions

  CfConvention = "CF-1.7";
  BaseConvention = "CF-Radial";
  CurrentVersion = "CF-Radial-1.4";
  CurrentVersion2 = "CF-Radial-2.0";

  // short names

  ADD_OFFSET = "add_offset";
  AIRBORNE = "airborne";
  ALTITUDE = "altitude";
  ALTITUDE_AGL = "altitude_agl";
  ALTITUDE_CORRECTION = "altitude_correction";
  ALTITUDE_OF_PROJECTION_ORIGIN = "altitude_of_projection_origin";
  ANCILLARY_VARIABLES = "ancillary_variables";
  ANTENNA_GAIN_H = "antenna_gain_h";
  ANTENNA_GAIN_V = "antenna_gain_v";
  ANTENNA_TRANSITION = "antenna_transition";
  AUTHOR = "author";
  AXIS = "axis";
  AZIMUTH = "azimuth";
  AZIMUTH_CORRECTION = "azimuth_correction";
  BASE_DBZ_1KM_HC = "base_dbz_1km_hc";
  BASE_DBZ_1KM_HX = "base_dbz_1km_hx";
  BASE_DBZ_1KM_VC = "base_dbz_1km_vc";
  BASE_DBZ_1KM_VX = "base_dbz_1km_vx";
  BLOCK_AVG_LENGTH = "block_avg_length";
  CALENDAR = "calendar";
  CALIBRATION_TIME = "calibration_time";
  CFRADIAL = "cfradial";
  CM = "cm";
  COMMENT = "comment";
  COMPRESS = "compress";
  CONVENTIONS = "Conventions";
  COORDINATES = "coordinates";
  COUPLER_FORWARD_LOSS_H = "coupler_forward_loss_h";
  COUPLER_FORWARD_LOSS_V = "coupler_forward_loss_v";
  CREATED = "created";
  DB = "db";
  DBM = "dBm";
  DBZ = "dBZ";
  DBZ_CORRECTION = "dbz_correction";
  DEGREES = "degrees";
  DEGREES_EAST = "degrees_east";
  DEGREES_NORTH = "degrees_north";
  DEGREES_PER_SECOND = "degrees per second";
  DIELECTRIC_FACTOR_USED = "dielectric_factor_used";
  DORADE = "dorade";
  DOWN = "down";
  DRIFT = "drift";
  DRIFT_CORRECTION = "drift_correction";
  DRIVER = "driver";
  DRIVE_ANGLE_1 = "drive_angle_1";
  DRIVE_ANGLE_2 = "drive_angle_2";
  EASTWARD_VELOCITY = "eastward_velocity";
  EASTWARD_VELOCITY_CORRECTION = "eastward_velocity_correction";
  EASTWARD_WIND = "eastward_wind";
  ELEVATION = "elevation";
  ELEVATION_CORRECTION = "elevation_correction";
  END_DATETIME = "end_datetime";
  END_TIME = "end_time";
  FALSE_EASTING = "false_easting";
  FALSE_NORTHING = "false_northing";
  FFT_LENGTH = "fft_length";
  FIELD_FOLDS = "field_folds";
  FILL_VALUE = "_FillValue";
  FIXED_ANGLE = "fixed_angle";
  FLAG_MASKS = "flag_masks";
  FLAG_MEANINGS = "flag_meanings";
  FLAG_VALUES = "flag_values";
  FOLD_LIMIT_LOWER = "fold_limit_lower";
  FOLD_LIMIT_UPPER = "fold_limit_upper";
  FOLLOW_MODE = "follow_mode";
  FREQUENCY = "frequency";
  GATE_SPACING = "gate_spacing";
  GEOMETRY_CORRECTION = "geometry_correction";
  GEOREFERENCE = "georeference";
  GEOREFS_APPLIED = "georefs_applied";
  GEOREF_CORRECTION = "georef_correction";
  GEOREF_TIME = "georef_time";
  GEOREF_UNIT_ID = "georef_unit_id";
  GEOREF_UNIT_NUM = "georef_unit_num";
  GREGORIAN = "gregorian";
  GRID_MAPPING = "grid_mapping";
  GRID_MAPPING_NAME = "grid_mapping_name";
  HEADING = "heading";
  HEADING_CHANGE_RATE = "heading_change_rate";
  HEADING_CORRECTION = "heading_correction";
  HISTORY = "history";
  HZ = "s-1";
  INDEX_VAR_NAME = "index_var_name";
  INSTITUTION = "institution";
  INSTRUMENT_NAME = "instrument_name";
  INSTRUMENT_PARAMETERS = "instrument_parameters";
  INSTRUMENT_TYPE = "instrument_type";
  INTERMED_FREQ_HZ = "intermed_freq_hz";
  IS_DISCRETE = "is_discrete";
  IS_QUALITY = "is_quality";
  IS_SPECTRUM = "is_spectrum";
  JOULES = "joules";
  JULIAN = "julian";
  LATITUDE = "latitude";
  LATITUDE_CORRECTION = "latitude_correction";
  LATITUDE_OF_PROJECTION_ORIGIN = "latitude_of_projection_origin";
  LDR_CORRECTION_H = "ldr_correction_h";
  LDR_CORRECTION_V = "ldr_correction_v";
  LEGEND_XML = "legend_xml";
  LIDAR_APERTURE_DIAMETER = "lidar_aperture_diameter";
  LIDAR_APERTURE_EFFICIENCY = "lidar_aperture_efficiency";
  LIDAR_BEAM_DIVERGENCE = "lidar_beam_divergence";
  LIDAR_CALIBRATION = "lidar_calibration";
  LIDAR_CONSTANT = "lidar_constant";
  LIDAR_FIELD_OF_VIEW = "lidar_field_of_view";
  LIDAR_PARAMETERS = "lidar_parameters";
  LIDAR_PEAK_POWER = "lidar_peak_power";
  LIDAR_PULSE_ENERGY = "lidar_pulse_energy";
  LONGITUDE = "longitude";
  LONGITUDE_CORRECTION = "longitude_correction";
  LONGITUDE_OF_PROJECTION_ORIGIN = "longitude_of_projection_origin";
  LONG_NAME = "long_name";
  META_GROUP = "meta_group";
  METERS = "meters";
  METERS_BETWEEN_GATES = "meters_between_gates";
  METERS_PER_SECOND = "meters per second";
  METERS_TO_CENTER_OF_FIRST_GATE = "meters_to_center_of_first_gate";
  MISSING_VALUE = "missing_value";
  MONITORING = "monitoring";
  MOVING = "moving";
  MRAD = "mrad";
  NOISE_HC = "noise_hc";
  NOISE_HX = "noise_hx";
  NOISE_SOURCE_POWER_H = "noise_source_power_h";
  NOISE_SOURCE_POWER_V = "noise_source_power_v";
  NOISE_VC = "noise_vc";
  NOISE_VX = "noise_vx";
  NORTHWARD_VELOCITY = "northward_velocity";
  NORTHWARD_VELOCITY_CORRECTION = "northward_velocity_correction";
  NORTHWARD_WIND = "northward_wind";
  NYQUIST_VELOCITY = "nyquist_velocity";
  N_GATES_VARY = "n_gates_vary";
  N_POINTS = "n_points";
  N_PRTS = "n_prts";
  N_SAMPLES = "n_samples";
  N_SPECTRA = "n_spectra";
  OPTIONS = "options";
  ORIGINAL_FORMAT = "original_format";
  PERCENT = "percent";
  PITCH = "pitch";
  PITCH_CHANGE_RATE = "pitch_change_rate";
  PITCH_CORRECTION = "pitch_correction";
  PLATFORM_IS_MOBILE = "platform_is_mobile";
  PLATFORM_TYPE = "platform_type";
  PLATFORM_VELOCITY = "platform_velocity";
  POLARIZATION_MODE = "polarization_mode";
  POLARIZATION_SEQUENCE = "polarization_sequence";
  POSITIVE = "positive";
  POWER_MEASURE_LOSS_H = "power_measure_loss_h";
  POWER_MEASURE_LOSS_V = "power_measure_loss_v";
  PRESSURE_ALTITUDE_CORRECTION = "pressure_altitude_correction";
  PRIMARY_AXIS = "primary_axis";
  PROBERT_JONES_CORRECTION = "probert_jones_correction";
  PROPOSED_STANDARD_NAME = "proposed_standard_name";
  PRT = "prt";
  PRT_MODE = "prt_mode";
  PRT_RATIO = "prt_ratio";
  PRT_SEQUENCE = "prt_sequence";
  PULSE_WIDTH = "pulse_width";
  QC_PROCEDURES = "qc_procedures";
  QUALIFIED_VARIABLES = "qualified_variables";
  RADAR_ANTENNA_GAIN_H = "radar_antenna_gain_h";
  RADAR_ANTENNA_GAIN_V = "radar_antenna_gain_v";
  RADAR_BEAM_WIDTH_H = "radar_beam_width_h";
  RADAR_BEAM_WIDTH_V = "radar_beam_width_v";
  RADAR_CALIBRATION = "radar_calibration";
  RADAR_CONSTANT_H = "radar_constant_h";
  RADAR_CONSTANT_V = "radar_constant_v";
  RADAR_ESTIMATED_NOISE_DBM_HC = "estimated_noise_dbm_hc";
  RADAR_ESTIMATED_NOISE_DBM_HX = "estimated_noise_dbm_hx";
  RADAR_ESTIMATED_NOISE_DBM_VC = "estimated_noise_dbm_vc";
  RADAR_ESTIMATED_NOISE_DBM_VX = "estimated_noise_dbm_vx";
  RADAR_MEASURED_COLD_NOISE = "measured_transmit_cold_noise";
  RADAR_MEASURED_HOT_NOISE = "measured_transmit_hot_noise";
  RADAR_MEASURED_SKY_NOISE = "measured_transmit_sky_noise";
  RADAR_MEASURED_TRANSMIT_POWER_H = "measured_transmit_power_h";
  RADAR_MEASURED_TRANSMIT_POWER_V = "measured_transmit_power_v";
  RADAR_PARAMETERS = "radar_parameters";
  RADAR_RX_BANDWIDTH = "radar_rx_bandwidth";
  RANGE = "range";
  RANGE_CORRECTION = "range_correction";
  RAYS_ARE_INDEXED = "rays_are_indexed";
  RAY_ANGLE_RES = "ray_angle_res";
  RAY_ANGLE_RESOLUTION = "ray_angle_resolution";
  RAY_GATE_SPACING = "ray_gate_spacing";
  RAY_N_GATES = "ray_n_gates";
  RAY_START_INDEX = "ray_start_index";
  RAY_START_RANGE = "ray_start_range";
  RAY_TIMES_INCREASE = "ray_times_increase";
  RECEIVER_GAIN_HC = "receiver_gain_hc";
  RECEIVER_GAIN_HX = "receiver_gain_hx";
  RECEIVER_GAIN_VC = "receiver_gain_vc";
  RECEIVER_GAIN_VX = "receiver_gain_vx";
  RECEIVER_MISMATCH_LOSS = "receiver_mismatch_loss";
  RECEIVER_MISMATCH_LOSS_H = "receiver_mismatch_loss_h";
  RECEIVER_MISMATCH_LOSS_V = "receiver_mismatch_loss_v";
  RECEIVER_SLOPE_HC = "receiver_slope_hc";
  RECEIVER_SLOPE_HX = "receiver_slope_hx";
  RECEIVER_SLOPE_VC = "receiver_slope_vc";
  RECEIVER_SLOPE_VX = "receiver_slope_vx";
  REFERENCES = "references";
  ROLL = "roll";
  ROLL_CHANGE_RATE = "roll_change_rate";
  ROLL_CORRECTION = "roll_correction";
  ROTATION = "rotation";
  ROTATION_CORRECTION = "rotation_correction";
  RX_RANGE_RESOLUTION = "rx_range_resolution";
  R_CALIB = "r_calib";
  R_CALIB_ANTENNA_GAIN_H = "r_calib_antenna_gain_h";
  R_CALIB_ANTENNA_GAIN_V = "r_calib_antenna_gain_v";
  R_CALIB_BASE_DBZ_1KM_HC = "r_calib_base_dbz_1km_hc";
  R_CALIB_BASE_DBZ_1KM_HX = "r_calib_base_dbz_1km_hx";
  R_CALIB_BASE_DBZ_1KM_VC = "r_calib_base_dbz_1km_vc";
  R_CALIB_BASE_DBZ_1KM_VX = "r_calib_base_dbz_1km_vx";
  R_CALIB_COUPLER_FORWARD_LOSS_H = "r_calib_coupler_forward_loss_h";
  R_CALIB_COUPLER_FORWARD_LOSS_V = "r_calib_coupler_forward_loss_v";
  R_CALIB_DBZ_CORRECTION = "r_calib_dbz_correction";
  R_CALIB_DIELECTRIC_FACTOR_USED = "r_calib_dielectric_factor_used";
  R_CALIB_DYNAMIC_RANGE_DB_HC = "r_calib_dynamic_range_db_hc";
  R_CALIB_DYNAMIC_RANGE_DB_HX = "r_calib_dynamic_range_db_hx";
  R_CALIB_DYNAMIC_RANGE_DB_VC = "r_calib_dynamic_range_db_vc";
  R_CALIB_DYNAMIC_RANGE_DB_VX = "r_calib_dynamic_range_db_vx";
  R_CALIB_I0_DBM_HC = "r_calib_i0_dbm_hc";
  R_CALIB_I0_DBM_HX = "r_calib_i0_dbm_hx";
  R_CALIB_I0_DBM_VC = "r_calib_i0_dbm_vc";
  R_CALIB_I0_DBM_VX = "r_calib_i0_dbm_vx";
  R_CALIB_INDEX = "r_calib_index";
  R_CALIB_INDEX = "r_calib_index";
  R_CALIB_K_SQUARED_WATER = "r_calib_k_squared_water";
  R_CALIB_LDR_CORRECTION_H = "r_calib_ldr_correction_h";
  R_CALIB_LDR_CORRECTION_V = "r_calib_ldr_correction_v";
  R_CALIB_NOISE_HC = "r_calib_noise_hc";
  R_CALIB_NOISE_HX = "r_calib_noise_hx";
  R_CALIB_NOISE_SOURCE_POWER_H = "r_calib_noise_source_power_h";
  R_CALIB_NOISE_SOURCE_POWER_V = "r_calib_noise_source_power_v";
  R_CALIB_NOISE_VC = "r_calib_noise_vc";
  R_CALIB_NOISE_VX = "r_calib_noise_vx";
  R_CALIB_POWER_MEASURE_LOSS_H = "r_calib_power_measure_loss_h";
  R_CALIB_POWER_MEASURE_LOSS_V = "r_calib_power_measure_loss_v";
  R_CALIB_PROBERT_JONES_CORRECTION = "r_calib_probert_jones_correction";
  R_CALIB_PULSE_WIDTH = "r_calib_pulse_width";
  R_CALIB_RADAR_CONSTANT_H = "r_calib_radar_constant_h";
  R_CALIB_RADAR_CONSTANT_V = "r_calib_radar_constant_v";
  R_CALIB_RECEIVER_GAIN_HC = "r_calib_receiver_gain_hc";
  R_CALIB_RECEIVER_GAIN_HX = "r_calib_receiver_gain_hx";
  R_CALIB_RECEIVER_GAIN_VC = "r_calib_receiver_gain_vc";
  R_CALIB_RECEIVER_GAIN_VX = "r_calib_receiver_gain_vx";
  R_CALIB_RECEIVER_MISMATCH_LOSS = "r_calib_receiver_mismatch_loss";
  R_CALIB_RECEIVER_MISMATCH_LOSS_H = "r_calib_receiver_mismatch_loss_h";
  R_CALIB_RECEIVER_MISMATCH_LOSS_V = "r_calib_receiver_mismatch_loss_v";
  R_CALIB_RECEIVER_SLOPE_HC = "r_calib_receiver_slope_hc";
  R_CALIB_RECEIVER_SLOPE_HX = "r_calib_receiver_slope_hx";
  R_CALIB_RECEIVER_SLOPE_VC = "r_calib_receiver_slope_vc";
  R_CALIB_RECEIVER_SLOPE_VX = "r_calib_receiver_slope_vx";
  R_CALIB_SUN_POWER_HC = "r_calib_sun_power_hc";
  R_CALIB_SUN_POWER_HX = "r_calib_sun_power_hx";
  R_CALIB_SUN_POWER_VC = "r_calib_sun_power_vc";
  R_CALIB_SUN_POWER_VX = "r_calib_sun_power_vx";
  R_CALIB_SYSTEM_PHIDP = "r_calib_system_phidp";
  R_CALIB_TEST_POWER_H = "r_calib_test_power_h";
  R_CALIB_TEST_POWER_V = "r_calib_test_power_v";
  R_CALIB_TIME = "r_calib_time";
  R_CALIB_TIME_W3C_STR = "r_calib_time_w3c_str";
  R_CALIB_TWO_WAY_RADOME_LOSS_H = "r_calib_two_way_radome_loss_h";
  R_CALIB_TWO_WAY_RADOME_LOSS_V = "r_calib_two_way_radome_loss_v";
  R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_H = "r_calib_two_way_waveguide_loss_h";
  R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_V = "r_calib_two_way_waveguide_loss_v";
  R_CALIB_XMIT_POWER_H = "r_calib_xmit_power_h";
  R_CALIB_XMIT_POWER_V = "r_calib_xmit_power_v";
  R_CALIB_ZDR_CORRECTION = "r_calib_zdr_correction";
  SAMPLING_RATIO = "sampling_ratio";
  SCALE_FACTOR = "scale_factor";
  SCANNING = "scanning";
  SCANNING_RADIAL = "scanning_radial";
  SCAN_ID = "scan_id";
  SCAN_NAME = "scan_name";
  SCAN_RATE = "scan_rate";
  SECONDS = "seconds";
  SECS_SINCE_JAN1_1970 = "seconds since 1970-01-01T00:00:00Z";
  SITE_NAME = "site_name";
  SOURCE = "source";
  SPACING_IS_CONSTANT = "spacing_is_constant";
  SPECTRUM_N_SAMPLES = "spectrum_n_samples";
  STANDARD = "standard";
  STANDARD_NAME = "standard_name";
  STARING = "staring";
  START_DATETIME = "start_datetime";
  START_RANGE = "start_range";
  START_TIME = "start_time";
  STATIONARY = "stationary";
  STATUS_XML = "status_xml";
  STATUS_XML_LENGTH = "status_xml_length";
  STRING_LENGTH_256 = "string_length_256";
  STRING_LENGTH_32 = "string_length_32";
  STRING_LENGTH_64 = "string_length_64";
  STRING_LENGTH_8 = "string_length_8";
  SUB_CONVENTIONS = "Sub_conventions";
  SUN_POWER_HC = "sun_power_hc";
  SUN_POWER_HX = "sun_power_hx";
  SUN_POWER_VC = "sun_power_vc";
  SUN_POWER_VX = "sun_power_vx";
  SWEEP = "sweep";
  SWEEP_END_RAY_INDEX = "sweep_end_ray_index";
  SWEEP_FIXED_ANGLE = "sweep_fixed_angle";
  SWEEP_GROUP_NAME = "sweep_group_name";
  SWEEP_MODE = "sweep_mode";
  SWEEP_NUMBER = "sweep_number";
  SWEEP_START_RAY_INDEX = "sweep_start_ray_index";
  SYSTEM_PHIDP = "system_phidp";
  TARGET_SCAN_RATE = "target_scan_rate";
  TEST_POWER_H = "test_power_h";
  TEST_POWER_V = "test_power_v";
  THRESHOLDING_XML = "thresholding_xml";
  TILT = "tilt";
  TILT_CORRECTION = "tilt_correction";
  TIME = "time";
  TIME_COVERAGE_END = "time_coverage_end";
  TIME_COVERAGE_START = "time_coverage_start";
  TIME_W3C_STR = "time_w3c_str";
  TITLE = "title";
  TRACK = "track";
  TWO_WAY_RADOME_LOSS_H = "two_way_radome_loss_h";
  TWO_WAY_RADOME_LOSS_V = "two_way_radome_loss_v";
  TWO_WAY_WAVEGUIDE_LOSS_H = "two_way_waveguide_loss_h";
  TWO_WAY_WAVEGUIDE_LOSS_V = "two_way_waveguide_loss_v";
  UNAMBIGUOUS_RANGE = "unambiguous_range";
  UNITS = "units";
  UP = "up";
  VALID_MAX = "valid_max";
  VALID_MIN = "valid_min";
  VALID_RANGE = "valid_range";
  VERSION = "version";
  VERTICAL_VELOCITY = "vertical_velocity";
  VERTICAL_VELOCITY_CORRECTION = "vertical_velocity_correction";
  VERTICAL_WIND = "vertical_wind";
  VOLUME = "volume";
  VOLUME_NUMBER = "volume_number";
  W3C_STR = "w3c_str";
  WATTS = "watts";
  XMIT_POWER_H = "xmit_power_h";
  XMIT_POWER_V = "xmit_power_v";
  ZDR_CORRECTION = "zdr_correction";
  
  // long names

  ALTITUDE_AGL_LONG = "altitude_above_ground_level";
  ALTITUDE_CORRECTION_LONG = "altitude_correction";
  ALTITUDE_LONG = "altitude";
  ANTENNA_GAIN_H_LONG = "calibrated_radar_antenna_gain_h_channel";
  ANTENNA_GAIN_V_LONG = "calibrated_radar_antenna_gain_v_channel";
  ANTENNA_TRANSITION_LONG = "antenna_is_in_transition_between_sweeps";
  AZIMUTH_CORRECTION_LONG = "azimuth_angle_correction";
  AZIMUTH_LONG = "ray_azimuth_angle";
  BASE_DBZ_1KM_HC_LONG = "radar_reflectivity_at_1km_at_zero_snr_h_co_polar_channel";
  BASE_DBZ_1KM_HX_LONG = "radar_reflectivity_at_1km_at_zero_snr_h_cross_polar_channel";
  BASE_DBZ_1KM_VC_LONG = "radar_reflectivity_at_1km_at_zero_snr_v_co_polar_channel";
  BASE_DBZ_1KM_VX_LONG = "radar_reflectivity_at_1km_at_zero_snr_v_cross_polar_channel";
  COUPLER_FORWARD_LOSS_H_LONG = "radar_calibration_coupler_forward_loss_h_channel";
  COUPLER_FORWARD_LOSS_V_LONG = "radar_calibration_coupler_forward_loss_v_channel";
  CO_TO_CROSS_POLAR_CORRELATION_RATIO_H = "co_to_cross_polar_correlation_ratio_h";
  CO_TO_CROSS_POLAR_CORRELATION_RATIO_V = "co_to_cross_polar_correlation_ratio_v";
  CROSS_POLAR_DIFFERENTIAL_PHASE = "cross_polar_differential_phase";
  CROSS_SPECTRUM_OF_COPOLAR_HORIZONTAL = "cross_spectrum_of_copolar_horizontal";
  CROSS_SPECTRUM_OF_COPOLAR_VERTICAL = "cross_spectrum_of_copolar_vertical";
  CROSS_SPECTRUM_OF_CROSSPOLAR_HORIZONTAL = "cross_spectrum_of_crosspolar_horizontal";
  CROSS_SPECTRUM_OF_CROSSPOLAR_VERTICAL = "cross_spectrum_of_crosspolar_vertical";
  DBZ_CORRECTION_LONG = "calibrated_radar_dbz_correction";
  DRIFT_CORRECTION_LONG = "platform_drift_angle_correction";
  DRIFT_LONG = "platform_drift_angle";
  DRIVE_ANGLE_1_LONG = "antenna_drive_angle_1";
  DRIVE_ANGLE_2_LONG = "antenna_drive_angle_2";
  EASTWARD_VELOCITY_CORRECTION_LONG = "platform_eastward_velocity_correction";
  EASTWARD_VELOCITY_LONG = "platform_eastward_velocity";
  EASTWARD_WIND_LONG = "eastward_wind_speed";
  ELEVATION_CORRECTION_LONG = "ray_elevation_angle_correction";
  ELEVATION_LONG = "ray_elevation_angle";
  FIXED_ANGLE_LONG = "ray_target_fixed_angle";
  FOLLOW_MODE_LONG = "follow_mode_for_scan_strategy";
  FREQUENCY_LONG = "transmission_frequency";
  GEOREF_TIME_LONG = "georef time in seconds since volume start";
  GEOREF_UNIT_ID_LONG = "georef hardware id or serial number";
  GEOREF_UNIT_NUM_LONG = "georef hardware unit number";
  HEADING_CHANGE_RATE_LONG = "platform_heading_angle_rate_of_change";
  HEADING_CORRECTION_LONG = "platform_heading_angle_correction";
  HEADING_LONG = "platform_heading_angle";
  INDEX_LONG = "calibration_data_array_index_per_ray";
  INSTRUMENT_NAME_LONG = "name_of_instrument";
  INSTRUMENT_TYPE_LONG = "type_of_instrument";
  INTERMED_FREQ_HZ_LONG = "intermediate_freqency_hz";
  LATITUDE_CORRECTION_LONG = "latitude_correction";
  LATITUDE_LONG = "latitude";
  LDR_CORRECTION_H_LONG = "calibrated_radar_ldr_correction_h_channel";
  LDR_CORRECTION_V_LONG = "calibrated_radar_ldr_correction_v_channel";
  LIDAR_APERTURE_DIAMETER_LONG = "lidar_aperture_diameter";
  LIDAR_APERTURE_EFFICIENCY_LONG = "lidar_aperture_efficiency";
  LIDAR_BEAM_DIVERGENCE_LONG = "lidar_beam_divergence";
  LIDAR_CONSTANT_LONG = "lidar_calibration_constant";
  LIDAR_FIELD_OF_VIEW_LONG = "lidar_field_of_view";
  LIDAR_PEAK_POWER_LONG = "lidar_peak_power";
  LIDAR_PULSE_ENERGY_LONG = "lidar_pulse_energy";
  LONGITUDE_CORRECTION_LONG = "longitude_correction";
  LONGITUDE_LONG = "longitude";
  NOISE_HC_LONG = "calibrated_radar_receiver_noise_h_co_polar_channel";
  NOISE_HX_LONG = "calibrated_radar_receiver_noise_h_cross_polar_channel";
  NOISE_SOURCE_POWER_H_LONG = "radar_calibration_noise_source_power_h_channel";
  NOISE_SOURCE_POWER_V_LONG = "radar_calibration_noise_source_power_v_channel";
  NOISE_VC_LONG = "calibrated_radar_receiver_noise_v_co_polar_channel";
  NOISE_VX_LONG = "calibrated_radar_receiver_noise_v_cross_polar_channel";
  NORTHWARD_VELOCITY_CORRECTION_LONG = "platform_northward_velocity_correction";
  NORTHWARD_VELOCITY_LONG = "platform_northward_velocity";
  NORTHWARD_WIND_LONG = "northward_wind";
  NYQUIST_VELOCITY_LONG = "unambiguous_doppler_velocity";
  N_SAMPLES_LONG = "number_of_samples_used_to_compute_moments";
  PITCH_CHANGE_RATE_LONG = "platform_pitch_angle_rate_of_change";
  PITCH_CORRECTION_LONG = "platform_pitch_angle_correction";
  PITCH_LONG = "platform_pitch_angle";
  PLATFORM_IS_MOBILE_LONG = "platform_is_mobile";
  PLATFORM_TYPE_LONG = "platform_type";
  POLARIZATION_MODE_LONG = "polarization_mode_for_sweep";
  POWER_MEASURE_LOSS_H_LONG = "radar_calibration_power_measurement_loss_h_channel";
  POWER_MEASURE_LOSS_V_LONG = "radar_calibration_power_measurement_loss_v_channel";
  PRESSURE_ALTITUDE_CORRECTION_LONG = "pressure_altitude_correction";
  PRIMARY_AXIS_LONG = "primary_axis_of_rotation";
  PRT_LONG = "pulse_repetition_time";
  PRT_MODE_LONG = "transmit_pulse_mode";
  PRT_RATIO_LONG = "pulse_repetition_frequency_ratio";
  PULSE_WIDTH_LONG = "transmitter_pulse_width";
  RADAR_ANTENNA_GAIN_H_LONG = "nominal_radar_antenna_gain_h_channel";
  RADAR_ANTENNA_GAIN_V_LONG = "nominal_radar_antenna_gain_v_channel";
  RADAR_BEAM_WIDTH_H_LONG = "half_power_radar_beam_width_h_channel";
  RADAR_BEAM_WIDTH_V_LONG = "half_power_radar_beam_width_v_channel";
  RADAR_CONSTANT_H_LONG = "calibrated_radar_constant_h_channel";
  RADAR_CONSTANT_V_LONG = "calibrated_radar_constant_v_channel";
  RADAR_ESTIMATED_NOISE_DBM_HC_LONG = "estimated_noise_dbm_hc";
  RADAR_ESTIMATED_NOISE_DBM_HX_LONG = "estimated_noise_dbm_hx";
  RADAR_ESTIMATED_NOISE_DBM_VC_LONG = "estimated_noise_dbm_vc";
  RADAR_ESTIMATED_NOISE_DBM_VX_LONG = "estimated_noise_dbm_vx";
  RADAR_MEASURED_TRANSMIT_POWER_H_LONG = "measured_radar_transmit_power_h_channel";
  RADAR_MEASURED_TRANSMIT_POWER_V_LONG = "measured_radar_transmit_power_v_channel";
  RADAR_RX_BANDWIDTH_LONG = "radar_receiver_bandwidth";
  RANGE_CORRECTION_LONG = "range_to_center_of_measurement_volume_correction";
  RANGE_LONG = "range_to_center_of_measurement_volume";
  RAYS_ARE_INDEXED_LONG = "flag_for_indexed_rays";
  RAY_ANGLE_RES_LONG = "angular_resolution_between_rays";
  RAY_ANGLE_RESOLUTION_LONG = "angular_resolution_between_rays";
  RECEIVER_GAIN_HC_LONG = "calibrated_radar_receiver_gain_h_co_polar_channel";
  RECEIVER_GAIN_HX_LONG = "calibrated_radar_receiver_gain_h_cross_polar_channel";
  RECEIVER_GAIN_VC_LONG = "calibrated_radar_receiver_gain_v_co_polar_channel";
  RECEIVER_GAIN_VX_LONG = "calibrated_radar_receiver_gain_v_cross_polar_channel";
  RECEIVER_MISMATCH_LOSS_LONG = "radar_calibration_receiver_mismatch_loss";
  RECEIVER_SLOPE_HC_LONG = "calibrated_radar_receiver_slope_h_co_polar_channel";
  RECEIVER_SLOPE_HX_LONG = "calibrated_radar_receiver_slope_h_cross_polar_channel";
  RECEIVER_SLOPE_VC_LONG = "calibrated_radar_receiver_slope_v_co_polar_channel";
  RECEIVER_SLOPE_VX_LONG = "calibrated_radar_receiver_slope_v_cross_polar_channel";
  ROLL_CHANGE_RATE_LONG = "platform_roll_angle_rate_of_change";
  ROLL_CORRECTION_LONG = "platform_roll_angle_correction";
  ROLL_LONG = "platform_roll_angle";
  ROTATION_CORRECTION_LONG = "ray_rotation_angle_relative_to_platform_correction";
  ROTATION_LONG = "ray_rotation_angle_relative_to_platform";
  R_CALIB_ANTENNA_GAIN_H_LONG = "calibrated_radar_antenna_gain_h_channel";
  R_CALIB_ANTENNA_GAIN_V_LONG = "calibrated_radar_antenna_gain_v_channel";
  R_CALIB_BASE_DBZ_1KM_HC_LONG = "radar_reflectivity_at_1km_at_zero_snr_h_co_polar_channel";
  R_CALIB_BASE_DBZ_1KM_HX_LONG = "radar_reflectivity_at_1km_at_zero_snr_h_cross_polar_channel";
  R_CALIB_BASE_DBZ_1KM_VC_LONG = "radar_reflectivity_at_1km_at_zero_snr_v_co_polar_channel";
  R_CALIB_BASE_DBZ_1KM_VX_LONG = "radar_reflectivity_at_1km_at_zero_snr_v_cross_polar_channel";
  R_CALIB_COUPLER_FORWARD_LOSS_H_LONG = "radar_calibration_coupler_forward_loss_h_channel";
  R_CALIB_COUPLER_FORWARD_LOSS_V_LONG = "radar_calibration_coupler_forward_loss_v_channel";
  R_CALIB_DBZ_CORRECTION_LONG = "calibrated_radar_dbz_correction";
  R_CALIB_INDEX_LONG = "calibration_data_array_index_per_ray";
  R_CALIB_LDR_CORRECTION_H_LONG = "calibrated_radar_ldr_correction_h_channel";
  R_CALIB_LDR_CORRECTION_V_LONG = "calibrated_radar_ldr_correction_v_channel";
  R_CALIB_NOISE_HC_LONG = "calibrated_radar_receiver_noise_h_co_polar_channel";
  R_CALIB_NOISE_HX_LONG = "calibrated_radar_receiver_noise_h_cross_polar_channel";
  R_CALIB_NOISE_SOURCE_POWER_H_LONG = "radar_calibration_noise_source_power_h_channel";
  R_CALIB_NOISE_SOURCE_POWER_V_LONG = "radar_calibration_noise_source_power_v_channel";
  R_CALIB_NOISE_VC_LONG = "calibrated_radar_receiver_noise_v_co_polar_channel";
  R_CALIB_NOISE_VX_LONG = "calibrated_radar_receiver_noise_v_cross_polar_channel";
  R_CALIB_POWER_MEASURE_LOSS_H_LONG = "radar_calibration_power_measurement_loss_h_channel";
  R_CALIB_POWER_MEASURE_LOSS_V_LONG = "radar_calibration_power_measurement_loss_v_channel";
  R_CALIB_PULSE_WIDTH_LONG = "radar_calibration_pulse_width";
  R_CALIB_RADAR_CONSTANT_H_LONG = "calibrated_radar_constant_h_channel";
  R_CALIB_RADAR_CONSTANT_V_LONG = "calibrated_radar_constant_v_channel";
  R_CALIB_RECEIVER_GAIN_HC_LONG = "calibrated_radar_receiver_gain_h_co_polar_channel";
  R_CALIB_RECEIVER_GAIN_HX_LONG = "calibrated_radar_receiver_gain_h_cross_polar_channel";
  R_CALIB_RECEIVER_GAIN_VC_LONG = "calibrated_radar_receiver_gain_v_co_polar_channel";
  R_CALIB_RECEIVER_GAIN_VX_LONG = "calibrated_radar_receiver_gain_v_cross_polar_channel";
  R_CALIB_RECEIVER_MISMATCH_LOSS_LONG = "radar_calibration_receiver_mismatch_loss";
  R_CALIB_RECEIVER_SLOPE_HC_LONG = "calibrated_radar_receiver_slope_h_co_polar_channel";
  R_CALIB_RECEIVER_SLOPE_HX_LONG = "calibrated_radar_receiver_slope_h_cross_polar_channel";
  R_CALIB_RECEIVER_SLOPE_VC_LONG = "calibrated_radar_receiver_slope_v_co_polar_channel";
  R_CALIB_RECEIVER_SLOPE_VX_LONG = "calibrated_radar_receiver_slope_v_cross_polar_channel";
  R_CALIB_SUN_POWER_HC_LONG = "calibrated_radar_sun_power_h_co_polar_channel";
  R_CALIB_SUN_POWER_HX_LONG = "calibrated_radar_sun_power_h_cross_polar_channel";
  R_CALIB_SUN_POWER_VC_LONG = "calibrated_radar_sun_power_v_co_polar_channel";
  R_CALIB_SUN_POWER_VX_LONG = "calibrated_radar_sun_power_v_cross_polar_channel";
  R_CALIB_SYSTEM_PHIDP_LONG = "calibrated_radar_system_phidp";
  R_CALIB_TEST_POWER_H_LONG = "radar_calibration_test_power_h_channel";
  R_CALIB_TEST_POWER_V_LONG = "radar_calibration_test_power_v_channel";
  R_CALIB_TIME_LONG = "radar_calibration_time_utc";
  R_CALIB_TWO_WAY_RADOME_LOSS_H_LONG = "radar_calibration_two_way_radome_loss_h_channel";
  R_CALIB_TWO_WAY_RADOME_LOSS_V_LONG = "radar_calibration_two_way_radome_loss_v_channel";
  R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_H_LONG = "radar_calibration_two_way_waveguide_loss_h_channel";
  R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_V_LONG = "radar_calibration_two_way_waveguide_loss_v_channel";
  R_CALIB_XMIT_POWER_H_LONG = "calibrated_radar_xmit_power_h_channel";
  R_CALIB_XMIT_POWER_V_LONG = "calibrated_radar_xmit_power_v_channel";
  R_CALIB_ZDR_CORRECTION_LONG = "calibrated_radar_zdr_correction";
  SCAN_ID_LONG = "volume_coverage_pattern";
  SCAN_NAME_LONG = "name_of_antenna_scan_strategy";
  SCAN_RATE_LONG = "antenna_angle_scan_rate";
  SITE_NAME_LONG = "name_of_instrument_site";
  SPACING_IS_CONSTANT_LONG = "spacing_between_range_gates_is_constant";
  SPECTRUM_COPOLAR_HORIZONTAL = "spectrum_copolar_horizontal";
  SPECTRUM_COPOLAR_VERTICAL = "spectrum_copolar_vertical";
  SPECTRUM_CROSSPOLAR_HORIZONTAL = "spectrum_crosspolar_horizontal";
  SPECTRUM_CROSSPOLAR_VERTICAL = "spectrum_crosspolar_vertical";
  SUN_POWER_HC_LONG = "calibrated_radar_sun_power_h_co_polar_channel";
  SUN_POWER_HX_LONG = "calibrated_radar_sun_power_h_cross_polar_channel";
  SUN_POWER_VC_LONG = "calibrated_radar_sun_power_v_co_polar_channel";
  SUN_POWER_VX_LONG = "calibrated_radar_sun_power_v_cross_polar_channel";
  SWEEP_END_RAY_INDEX_LONG = "index_of_last_ray_in_sweep";
  SWEEP_FIXED_ANGLE_LONG = "fixed_angle_for_sweep";
  SWEEP_GROUP_NAME_LONG = "group_name_for_sweep";
  SWEEP_MODE_LONG = "scan_mode_for_sweep";
  SWEEP_NUMBER_LONG = "sweep_index_number_0_based";
  SWEEP_START_RAY_INDEX_LONG = "index_of_first_ray_in_sweep";
  SYSTEM_PHIDP_LONG = "calibrated_radar_system_phidp";
  TARGET_SCAN_RATE_LONG = "target_scan_rate_for_sweep";
  TEST_POWER_H_LONG = "radar_calibration_test_power_h_channel";
  TEST_POWER_V_LONG = "radar_calibration_test_power_v_channel";
  TILT_CORRECTION_LONG = "ray_tilt_angle_relative_to_platform_correction";
  TILT_LONG = "ray_tilt_angle_relative_to_platform";
  TIME_COVERAGE_END_LONG = "data_volume_end_time_utc";
  TIME_COVERAGE_START_LONG = "data_volume_start_time_utc";
  TRACK_LONG = "platform_track_over_the_ground";
  TWO_WAY_RADOME_LOSS_H_LONG = "radar_calibration_two_way_radome_loss_h_channel";
  TWO_WAY_RADOME_LOSS_V_LONG = "radar_calibration_two_way_radome_loss_v_channel";
  TWO_WAY_WAVEGUIDE_LOSS_H_LONG = "radar_calibration_two_way_waveguide_loss_h_channel";
  TWO_WAY_WAVEGUIDE_LOSS_V_LONG = "radar_calibration_two_way_waveguide_loss_v_channel";
  UNAMBIGUOUS_RANGE_LONG = "unambiguous_range";
  VERTICAL_VELOCITY_CORRECTION_LONG = "platform_vertical_velocity_correction";
  VERTICAL_VELOCITY_LONG = "platform_vertical_velocity";
  VERTICAL_WIND_LONG = "upward_air_velocity";
  VOLUME_NUMBER_LONG = "data_volume_index_number";
  XMIT_POWER_H_LONG = "calibrated_radar_xmit_power_h_channel";
  XMIT_POWER_V_LONG = "calibrated_radar_xmit_power_v_channel";
  ZDR_CORRECTION_LONG = "calibrated_radar_zdr_correction";

}

/////////////
// destructor

RadxNcfStr::~RadxNcfStr()

{

}

