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
using namespace std;

class RadxNcfStr

{
  
public:

  /// Constructor
  
  RadxNcfStr();
  
  /// Destructor
  
  virtual ~RadxNcfStr();
  
protected:

  // string constants
  
  string CfConvention;
  string BaseConvention;
  string CurrentVersion;
  string CurrentVersion2;

  // char * constants

  const char* ADD_OFFSET;
  const char* AIRBORNE;
  const char* ALTITUDE;
  const char* ALTITUDE_AGL;
  const char* ALTITUDE_CORRECTION;
  const char* ALTITUDE_OF_PROJECTION_ORIGIN;
  const char* ANCILLARY_VARIABLES;
  const char* ANTENNA_GAIN_H;
  const char* ANTENNA_GAIN_V;
  const char* ANTENNA_TRANSITION;
  const char* AUTHOR;
  const char* AXIS;
  const char* AZIMUTH;
  const char* AZIMUTH_CORRECTION;
  const char* BASE_DBZ_1KM_HC;
  const char* BASE_DBZ_1KM_HX;
  const char* BASE_DBZ_1KM_VC;
  const char* BASE_DBZ_1KM_VX;
  const char* BLOCK_AVG_LENGTH;
  const char* CALENDAR;
  const char* CALIBRATION_TIME;
  const char* CFRADIAL;
  const char* CM;
  const char* COMMENT;
  const char* COMPRESS;
  const char* CONVENTIONS;
  const char* COORDINATES;
  const char* COUPLER_FORWARD_LOSS_H;
  const char* COUPLER_FORWARD_LOSS_V;
  const char* CREATED;
  const char* DB;
  const char* DBM;
  const char* DBZ;
  const char* DBZ_CORRECTION;
  const char* DEGREES;
  const char* DEGREES_EAST;
  const char* DEGREES_NORTH;
  const char* DEGREES_PER_SECOND;
  const char* DIELECTRIC_FACTOR_USED;
  const char* DORADE;
  const char* DOWN;
  const char* DRIFT;
  const char* DRIFT_CORRECTION;
  const char* DRIVER;
  const char* DRIVE_ANGLE_1;
  const char* DRIVE_ANGLE_2;
  const char* EASTWARD_VELOCITY;
  const char* EASTWARD_VELOCITY_CORRECTION;
  const char* EASTWARD_WIND;
  const char* ELEVATION;
  const char* ELEVATION_CORRECTION;
  const char* END_DATETIME;
  const char* END_TIME;
  const char* FALSE_EASTING;
  const char* FALSE_NORTHING;
  const char* FFT_LENGTH;
  const char* FIELD_FOLDS;
  const char* FILL_VALUE;
  const char* FIXED_ANGLE;
  const char* FLAG_MASKS;
  const char* FLAG_MEANINGS;
  const char* FLAG_VALUES;
  const char* FOLD_LIMIT_LOWER;
  const char* FOLD_LIMIT_UPPER;
  const char* FOLLOW_MODE;
  const char* FREQUENCY;
  const char* GATE_SPACING;
  const char* GEOMETRY_CORRECTION;
  const char* GEOREFERENCE;
  const char* GEOREFS_APPLIED;
  const char* GEOREF_CORRECTION;
  const char* GEOREF_TIME;
  const char* GEOREF_UNIT_ID;
  const char* GEOREF_UNIT_NUM;
  const char* GREGORIAN;
  const char* GRID_MAPPING;
  const char* GRID_MAPPING_NAME;
  const char* HEADING;
  const char* HEADING_CHANGE_RATE;
  const char* HEADING_CORRECTION;
  const char* HISTORY;
  const char* HZ;
  const char* INDEX_VAR_NAME;
  const char* INSTITUTION;
  const char* INSTRUMENT_NAME;
  const char* INSTRUMENT_PARAMETERS;
  const char* INSTRUMENT_TYPE;
  const char* INTERMED_FREQ_HZ;
  const char* IS_DISCRETE;
  const char* IS_QUALITY;
  const char* IS_SPECTRUM;
  const char* JOULES;
  const char* JULIAN;
  const char* LATITUDE;
  const char* LATITUDE_CORRECTION;
  const char* LATITUDE_OF_PROJECTION_ORIGIN;
  const char* LDR_CORRECTION_H;
  const char* LDR_CORRECTION_V;
  const char* LEGEND_XML;
  const char* LIDAR_APERTURE_DIAMETER;
  const char* LIDAR_APERTURE_EFFICIENCY;
  const char* LIDAR_BEAM_DIVERGENCE;
  const char* LIDAR_CALIBRATION;
  const char* LIDAR_CONSTANT;
  const char* LIDAR_FIELD_OF_VIEW;
  const char* LIDAR_PARAMETERS;
  const char* LIDAR_PEAK_POWER;
  const char* LIDAR_PULSE_ENERGY;
  const char* LONGITUDE;
  const char* LONGITUDE_CORRECTION;
  const char* LONGITUDE_OF_PROJECTION_ORIGIN;
  const char* LONG_NAME;
  const char* META_GROUP;
  const char* METERS;
  const char* METERS_BETWEEN_GATES;
  const char* METERS_PER_SECOND;
  const char* METERS_TO_CENTER_OF_FIRST_GATE;
  const char* MISSING_VALUE;
  const char* MONITORING;
  const char* MOVING;
  const char* MRAD;
  const char* NOISE_HC;
  const char* NOISE_HX;
  const char* NOISE_SOURCE_POWER_H;
  const char* NOISE_SOURCE_POWER_V;
  const char* NOISE_VC;
  const char* NOISE_VX;
  const char* NORTHWARD_VELOCITY;
  const char* NORTHWARD_VELOCITY_CORRECTION;
  const char* NORTHWARD_WIND;
  const char* NYQUIST_VELOCITY;
  const char* N_GATES_VARY;
  const char* N_POINTS;
  const char* N_PRTS;
  const char* N_SAMPLES;
  const char* N_SPECTRA;
  const char* OPTIONS;
  const char* ORIGINAL_FORMAT;
  const char* PERCENT;
  const char* PITCH;
  const char* PITCH_CHANGE_RATE;
  const char* PITCH_CORRECTION;
  const char* PLATFORM_IS_MOBILE;
  const char* PLATFORM_TYPE;
  const char* PLATFORM_VELOCITY;
  const char* POLARIZATION_MODE;
  const char* POLARIZATION_SEQUENCE;
  const char* POSITIVE;
  const char* POWER_MEASURE_LOSS_H;
  const char* POWER_MEASURE_LOSS_V;
  const char* PRESSURE_ALTITUDE_CORRECTION;
  const char* PRIMARY_AXIS;
  const char* PROBERT_JONES_CORRECTION;
  const char* PROPOSED_STANDARD_NAME;
  const char* PRT;
  const char* PRT_MODE;
  const char* PRT_RATIO;
  const char* PRT_SEQUENCE;
  const char* PULSE_WIDTH;
  const char* QC_PROCEDURES;
  const char* QUALIFIED_VARIABLES;
  const char* RADAR_ANTENNA_GAIN_H;
  const char* RADAR_ANTENNA_GAIN_V;
  const char* RADAR_BEAM_WIDTH_H;
  const char* RADAR_BEAM_WIDTH_V;
  const char* RADAR_CALIBRATION;
  const char* RADAR_CONSTANT_H;
  const char* RADAR_CONSTANT_V;
  const char* RADAR_ESTIMATED_NOISE_DBM_HC;
  const char* RADAR_ESTIMATED_NOISE_DBM_HX;
  const char* RADAR_ESTIMATED_NOISE_DBM_VC;
  const char* RADAR_ESTIMATED_NOISE_DBM_VX;
  const char* RADAR_MEASURED_COLD_NOISE;
  const char* RADAR_MEASURED_HOT_NOISE;
  const char* RADAR_MEASURED_SKY_NOISE;
  const char* RADAR_MEASURED_TRANSMIT_POWER_H;
  const char* RADAR_MEASURED_TRANSMIT_POWER_V;
  const char* RADAR_PARAMETERS;
  const char* RADAR_RX_BANDWIDTH;
  const char* RANGE;
  const char* RANGE_CORRECTION;
  const char* RAYS_ARE_INDEXED;
  const char* RAY_ANGLE_RES;
  const char* RAY_ANGLE_RESOLUTION;
  const char* RAY_GATE_SPACING;
  const char* RAY_N_GATES;
  const char* RAY_START_INDEX;
  const char* RAY_START_RANGE;
  const char* RAY_TIMES_INCREASE;
  const char* RECEIVER_GAIN_HC;
  const char* RECEIVER_GAIN_HX;
  const char* RECEIVER_GAIN_VC;
  const char* RECEIVER_GAIN_VX;
  const char* RECEIVER_MISMATCH_LOSS;
  const char* RECEIVER_MISMATCH_LOSS_H;
  const char* RECEIVER_MISMATCH_LOSS_V;
  const char* RECEIVER_SLOPE_HC;
  const char* RECEIVER_SLOPE_HX;
  const char* RECEIVER_SLOPE_VC;
  const char* RECEIVER_SLOPE_VX;
  const char* REFERENCES;
  const char* ROLL;
  const char* ROLL_CHANGE_RATE;
  const char* ROLL_CORRECTION;
  const char* ROTATION;
  const char* ROTATION_CORRECTION;
  const char* RX_RANGE_RESOLUTION;
  const char* R_CALIB;
  const char* R_CALIB_ANTENNA_GAIN_H;
  const char* R_CALIB_ANTENNA_GAIN_V;
  const char* R_CALIB_BASE_DBZ_1KM_HC;
  const char* R_CALIB_BASE_DBZ_1KM_HX;
  const char* R_CALIB_BASE_DBZ_1KM_VC;
  const char* R_CALIB_BASE_DBZ_1KM_VX;
  const char* R_CALIB_COUPLER_FORWARD_LOSS_H;
  const char* R_CALIB_COUPLER_FORWARD_LOSS_V;
  const char* R_CALIB_DBZ_CORRECTION;
  const char* R_CALIB_DIELECTRIC_FACTOR_USED;
  const char* R_CALIB_DYNAMIC_RANGE_DB_HC;
  const char* R_CALIB_DYNAMIC_RANGE_DB_HX;
  const char* R_CALIB_DYNAMIC_RANGE_DB_VC;
  const char* R_CALIB_DYNAMIC_RANGE_DB_VX;
  const char* R_CALIB_I0_DBM_HC;
  const char* R_CALIB_I0_DBM_HX;
  const char* R_CALIB_I0_DBM_VC;
  const char* R_CALIB_I0_DBM_VX;
  const char* R_CALIB_INDEX;
  const char* R_CALIB_K_SQUARED_WATER;
  const char* R_CALIB_LDR_CORRECTION_H;
  const char* R_CALIB_LDR_CORRECTION_V;
  const char* R_CALIB_NOISE_HC;
  const char* R_CALIB_NOISE_HX;
  const char* R_CALIB_NOISE_SOURCE_POWER_H;
  const char* R_CALIB_NOISE_SOURCE_POWER_V;
  const char* R_CALIB_NOISE_VC;
  const char* R_CALIB_NOISE_VX;
  const char* R_CALIB_POWER_MEASURE_LOSS_H;
  const char* R_CALIB_POWER_MEASURE_LOSS_V;
  const char* R_CALIB_PROBERT_JONES_CORRECTION;
  const char* R_CALIB_PULSE_WIDTH;
  const char* R_CALIB_RADAR_CONSTANT_H;
  const char* R_CALIB_RADAR_CONSTANT_V;
  const char* R_CALIB_RECEIVER_GAIN_HC;
  const char* R_CALIB_RECEIVER_GAIN_HX;
  const char* R_CALIB_RECEIVER_GAIN_VC;
  const char* R_CALIB_RECEIVER_GAIN_VX;
  const char* R_CALIB_RECEIVER_MISMATCH_LOSS;
  const char* R_CALIB_RECEIVER_MISMATCH_LOSS_H;
  const char* R_CALIB_RECEIVER_MISMATCH_LOSS_V;
  const char* R_CALIB_RECEIVER_SLOPE_HC;
  const char* R_CALIB_RECEIVER_SLOPE_HX;
  const char* R_CALIB_RECEIVER_SLOPE_VC;
  const char* R_CALIB_RECEIVER_SLOPE_VX;
  const char* R_CALIB_SUN_POWER_HC;
  const char* R_CALIB_SUN_POWER_HX;
  const char* R_CALIB_SUN_POWER_VC;
  const char* R_CALIB_SUN_POWER_VX;
  const char* R_CALIB_SYSTEM_PHIDP;
  const char* R_CALIB_TEST_POWER_H;
  const char* R_CALIB_TEST_POWER_V;
  const char* R_CALIB_TIME;
  const char* R_CALIB_TIME_W3C_STR;
  const char* R_CALIB_TWO_WAY_RADOME_LOSS_H;
  const char* R_CALIB_TWO_WAY_RADOME_LOSS_V;
  const char* R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_H;
  const char* R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_V;
  const char* R_CALIB_XMIT_POWER_H;
  const char* R_CALIB_XMIT_POWER_V;
  const char* R_CALIB_ZDR_CORRECTION;
  const char* SAMPLING_RATIO;
  const char* SCALE_FACTOR;
  const char* SCANNING;
  const char* SCANNING_RADIAL;
  const char* SCAN_ID;
  const char* SCAN_NAME;
  const char* SCAN_RATE;
  const char* SECONDS;
  const char* SECS_SINCE_JAN1_1970;
  const char* SITE_NAME;
  const char* SOURCE;
  const char* SPACING_IS_CONSTANT;
  const char* SPECTRUM_N_SAMPLES;
  const char* STANDARD;
  const char* STANDARD_NAME;
  const char* STARING;
  const char* START_DATETIME;
  const char* START_RANGE;
  const char* START_TIME;
  const char* STATIONARY;
  const char* STATUS_XML;
  const char* STATUS_XML_LENGTH;
  const char* STRING_LENGTH_256;
  const char* STRING_LENGTH_32;
  const char* STRING_LENGTH_64;
  const char* STRING_LENGTH_8;
  const char* SUB_CONVENTIONS;
  const char* SUN_POWER_HC;
  const char* SUN_POWER_HX;
  const char* SUN_POWER_VC;
  const char* SUN_POWER_VX;
  const char* SWEEP;
  const char* SWEEP_END_RAY_INDEX;
  const char* SWEEP_FIXED_ANGLE;
  const char* SWEEP_GROUP_NAME;
  const char* SWEEP_MODE;
  const char* SWEEP_NUMBER;
  const char* SWEEP_START_RAY_INDEX;
  const char* SYSTEM_PHIDP;
  const char* TARGET_SCAN_RATE;
  const char* TEST_POWER_H;
  const char* TEST_POWER_V;
  const char* THRESHOLDING_XML;
  const char* TILT;
  const char* TILT_CORRECTION;
  const char* TIME;
  const char* TIME_COVERAGE_END;
  const char* TIME_COVERAGE_START;
  const char* TIME_W3C_STR;
  const char* TITLE;
  const char* TRACK;
  const char* TWO_WAY_RADOME_LOSS_H;
  const char* TWO_WAY_RADOME_LOSS_V;
  const char* TWO_WAY_WAVEGUIDE_LOSS_H;
  const char* TWO_WAY_WAVEGUIDE_LOSS_V;
  const char* UNAMBIGUOUS_RANGE;
  const char* UNITS;
  const char* UP;
  const char* VALID_MAX;
  const char* VALID_MIN;
  const char* VALID_RANGE;
  const char* VERSION;
  const char* VERTICAL_VELOCITY;
  const char* VERTICAL_VELOCITY_CORRECTION;
  const char* VERTICAL_WIND;
  const char* VOLUME;
  const char* VOLUME_NUMBER;
  const char* W3C_STR;
  const char* WATTS;
  const char* XMIT_POWER_H;
  const char* XMIT_POWER_V;
  const char* ZDR_CORRECTION;

  // long names for metadata

  const char* ALTITUDE_AGL_LONG;
  const char* ALTITUDE_CORRECTION_LONG;
  const char* ALTITUDE_LONG;
  const char* ANTENNA_GAIN_H_LONG;
  const char* ANTENNA_GAIN_V_LONG;
  const char* ANTENNA_TRANSITION_LONG;
  const char* AZIMUTH_CORRECTION_LONG;
  const char* AZIMUTH_LONG;
  const char* BASE_DBZ_1KM_HC_LONG;
  const char* BASE_DBZ_1KM_HX_LONG;
  const char* BASE_DBZ_1KM_VC_LONG;
  const char* BASE_DBZ_1KM_VX_LONG;
  const char* COUPLER_FORWARD_LOSS_H_LONG;
  const char* COUPLER_FORWARD_LOSS_V_LONG;
  const char* CO_TO_CROSS_POLAR_CORRELATION_RATIO_H;
  const char* CO_TO_CROSS_POLAR_CORRELATION_RATIO_V;
  const char* CROSS_POLAR_DIFFERENTIAL_PHASE;
  const char* CROSS_SPECTRUM_OF_COPOLAR_HORIZONTAL;
  const char* CROSS_SPECTRUM_OF_COPOLAR_VERTICAL;
  const char* CROSS_SPECTRUM_OF_CROSSPOLAR_HORIZONTAL;
  const char* CROSS_SPECTRUM_OF_CROSSPOLAR_VERTICAL;
  const char* DBZ_CORRECTION_LONG;
  const char* DRIFT_CORRECTION_LONG;
  const char* DRIFT_LONG;
  const char* DRIVE_ANGLE_1_LONG;
  const char* DRIVE_ANGLE_2_LONG;
  const char* EASTWARD_VELOCITY_CORRECTION_LONG;
  const char* EASTWARD_VELOCITY_LONG;
  const char* EASTWARD_WIND_LONG;
  const char* ELEVATION_CORRECTION_LONG;
  const char* ELEVATION_LONG;
  const char* FIXED_ANGLE_LONG;
  const char* FOLLOW_MODE_LONG;
  const char* FREQUENCY_LONG;
  const char* GEOREF_TIME_LONG;
  const char* GEOREF_UNIT_ID_LONG;
  const char* GEOREF_UNIT_NUM_LONG;
  const char* HEADING_CHANGE_RATE_LONG;
  const char* HEADING_CORRECTION_LONG;
  const char* HEADING_LONG;
  const char* INDEX_LONG;
  const char* INSTRUMENT_NAME_LONG;
  const char* INSTRUMENT_TYPE_LONG;
  const char* INTERMED_FREQ_HZ_LONG;
  const char* LATITUDE_CORRECTION_LONG;
  const char* LATITUDE_LONG;
  const char* LDR_CORRECTION_H_LONG;
  const char* LDR_CORRECTION_V_LONG;
  const char* LIDAR_APERTURE_DIAMETER_LONG;
  const char* LIDAR_APERTURE_EFFICIENCY_LONG;
  const char* LIDAR_BEAM_DIVERGENCE_LONG;
  const char* LIDAR_CONSTANT_LONG;
  const char* LIDAR_FIELD_OF_VIEW_LONG;
  const char* LIDAR_PEAK_POWER_LONG;
  const char* LIDAR_PULSE_ENERGY_LONG;
  const char* LONGITUDE_CORRECTION_LONG;
  const char* LONGITUDE_LONG;
  const char* NOISE_HC_LONG;
  const char* NOISE_HX_LONG;
  const char* NOISE_SOURCE_POWER_H_LONG;
  const char* NOISE_SOURCE_POWER_V_LONG;
  const char* NOISE_VC_LONG;
  const char* NOISE_VX_LONG;
  const char* NORTHWARD_VELOCITY_CORRECTION_LONG;
  const char* NORTHWARD_VELOCITY_LONG;
  const char* NORTHWARD_WIND_LONG;
  const char* NYQUIST_VELOCITY_LONG;
  const char* N_SAMPLES_LONG;
  const char* PITCH_CHANGE_RATE_LONG;
  const char* PITCH_CORRECTION_LONG;
  const char* PITCH_LONG;
  const char* PLATFORM_IS_MOBILE_LONG;
  const char* PLATFORM_TYPE_LONG;
  const char* POLARIZATION_MODE_LONG;
  const char* POWER_MEASURE_LOSS_H_LONG;
  const char* POWER_MEASURE_LOSS_V_LONG;
  const char* PRESSURE_ALTITUDE_CORRECTION_LONG;
  const char* PRIMARY_AXIS_LONG;
  const char* PRT_LONG;
  const char* PRT_MODE_LONG;
  const char* PRT_RATIO_LONG;
  const char* PULSE_WIDTH_LONG;
  const char* RADAR_ANTENNA_GAIN_H_LONG;
  const char* RADAR_ANTENNA_GAIN_V_LONG;
  const char* RADAR_BEAM_WIDTH_H_LONG;
  const char* RADAR_BEAM_WIDTH_V_LONG;
  const char* RADAR_CONSTANT_H_LONG;
  const char* RADAR_CONSTANT_V_LONG;
  const char* RADAR_ESTIMATED_NOISE_DBM_HC_LONG;
  const char* RADAR_ESTIMATED_NOISE_DBM_HX_LONG;
  const char* RADAR_ESTIMATED_NOISE_DBM_VC_LONG;
  const char* RADAR_ESTIMATED_NOISE_DBM_VX_LONG;
  const char* RADAR_MEASURED_TRANSMIT_POWER_H_LONG;
  const char* RADAR_MEASURED_TRANSMIT_POWER_V_LONG;
  const char* RADAR_RX_BANDWIDTH_LONG;
  const char* RANGE_CORRECTION_LONG;
  const char* RANGE_LONG;
  const char* RAYS_ARE_INDEXED_LONG;
  const char* RAY_ANGLE_RES_LONG;
  const char* RAY_ANGLE_RESOLUTION_LONG;
  const char* RECEIVER_GAIN_HC_LONG;
  const char* RECEIVER_GAIN_HX_LONG;
  const char* RECEIVER_GAIN_VC_LONG;
  const char* RECEIVER_GAIN_VX_LONG;
  const char* RECEIVER_MISMATCH_LOSS_LONG;
  const char* RECEIVER_SLOPE_HC_LONG;
  const char* RECEIVER_SLOPE_HX_LONG;
  const char* RECEIVER_SLOPE_VC_LONG;
  const char* RECEIVER_SLOPE_VX_LONG;
  const char* ROLL_CHANGE_RATE_LONG;
  const char* ROLL_CORRECTION_LONG;
  const char* ROLL_LONG;
  const char* ROTATION_CORRECTION_LONG;
  const char* ROTATION_LONG;
  const char* R_CALIB_ANTENNA_GAIN_H_LONG;
  const char* R_CALIB_ANTENNA_GAIN_V_LONG;
  const char* R_CALIB_BASE_DBZ_1KM_HC_LONG;
  const char* R_CALIB_BASE_DBZ_1KM_HX_LONG;
  const char* R_CALIB_BASE_DBZ_1KM_VC_LONG;
  const char* R_CALIB_BASE_DBZ_1KM_VX_LONG;
  const char* R_CALIB_COUPLER_FORWARD_LOSS_H_LONG;
  const char* R_CALIB_COUPLER_FORWARD_LOSS_V_LONG;
  const char* R_CALIB_DBZ_CORRECTION_LONG;
  const char* R_CALIB_INDEX_LONG;
  const char* R_CALIB_LDR_CORRECTION_H_LONG;
  const char* R_CALIB_LDR_CORRECTION_V_LONG;
  const char* R_CALIB_NOISE_HC_LONG;
  const char* R_CALIB_NOISE_HX_LONG;
  const char* R_CALIB_NOISE_SOURCE_POWER_H_LONG;
  const char* R_CALIB_NOISE_SOURCE_POWER_V_LONG;
  const char* R_CALIB_NOISE_VC_LONG;
  const char* R_CALIB_NOISE_VX_LONG;
  const char* R_CALIB_POWER_MEASURE_LOSS_H_LONG;
  const char* R_CALIB_POWER_MEASURE_LOSS_V_LONG;
  const char* R_CALIB_PULSE_WIDTH_LONG;
  const char* R_CALIB_RADAR_CONSTANT_H_LONG;
  const char* R_CALIB_RADAR_CONSTANT_V_LONG;
  const char* R_CALIB_RECEIVER_GAIN_HC_LONG;
  const char* R_CALIB_RECEIVER_GAIN_HX_LONG;
  const char* R_CALIB_RECEIVER_GAIN_VC_LONG;
  const char* R_CALIB_RECEIVER_GAIN_VX_LONG;
  const char* R_CALIB_RECEIVER_MISMATCH_LOSS_LONG;
  const char* R_CALIB_RECEIVER_SLOPE_HC_LONG;
  const char* R_CALIB_RECEIVER_SLOPE_HX_LONG;
  const char* R_CALIB_RECEIVER_SLOPE_VC_LONG;
  const char* R_CALIB_RECEIVER_SLOPE_VX_LONG;
  const char* R_CALIB_SUN_POWER_HC_LONG;
  const char* R_CALIB_SUN_POWER_HX_LONG;
  const char* R_CALIB_SUN_POWER_VC_LONG;
  const char* R_CALIB_SUN_POWER_VX_LONG;
  const char* R_CALIB_SYSTEM_PHIDP_LONG;
  const char* R_CALIB_TEST_POWER_H_LONG;
  const char* R_CALIB_TEST_POWER_V_LONG;
  const char* R_CALIB_TIME_LONG;
  const char* R_CALIB_TWO_WAY_RADOME_LOSS_H_LONG;
  const char* R_CALIB_TWO_WAY_RADOME_LOSS_V_LONG;
  const char* R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_H_LONG;
  const char* R_CALIB_TWO_WAY_WAVEGUIDE_LOSS_V_LONG;
  const char* R_CALIB_XMIT_POWER_H_LONG;
  const char* R_CALIB_XMIT_POWER_V_LONG;
  const char* R_CALIB_ZDR_CORRECTION_LONG;
  const char* SCAN_ID_LONG;
  const char* SCAN_NAME_LONG;
  const char* SCAN_RATE_LONG;
  const char* SITE_NAME_LONG;
  const char* SPACING_IS_CONSTANT_LONG;
  const char* SPECTRUM_COPOLAR_HORIZONTAL;
  const char* SPECTRUM_COPOLAR_VERTICAL;
  const char* SPECTRUM_CROSSPOLAR_HORIZONTAL;
  const char* SPECTRUM_CROSSPOLAR_VERTICAL;
  const char* SUN_POWER_HC_LONG;
  const char* SUN_POWER_HX_LONG;
  const char* SUN_POWER_VC_LONG;
  const char* SUN_POWER_VX_LONG;
  const char* SWEEP_END_RAY_INDEX_LONG;
  const char* SWEEP_FIXED_ANGLE_LONG;
  const char* SWEEP_GROUP_NAME_LONG;
  const char* SWEEP_MODE_LONG;
  const char* SWEEP_NUMBER_LONG;
  const char* SWEEP_START_RAY_INDEX_LONG;
  const char* SYSTEM_PHIDP_LONG;
  const char* TARGET_SCAN_RATE_LONG;
  const char* TEST_POWER_H_LONG;
  const char* TEST_POWER_V_LONG;
  const char* TILT_CORRECTION_LONG;
  const char* TILT_LONG;
  const char* TIME_COVERAGE_END_LONG;
  const char* TIME_COVERAGE_START_LONG;
  const char* TRACK_LONG;
  const char* TWO_WAY_RADOME_LOSS_H_LONG;
  const char* TWO_WAY_RADOME_LOSS_V_LONG;
  const char* TWO_WAY_WAVEGUIDE_LOSS_H_LONG;
  const char* TWO_WAY_WAVEGUIDE_LOSS_V_LONG;
  const char* UNAMBIGUOUS_RANGE_LONG;
  const char* VERTICAL_VELOCITY_CORRECTION_LONG;
  const char* VERTICAL_VELOCITY_LONG;
  const char* VERTICAL_WIND_LONG;
  const char* VOLUME_NUMBER_LONG;
  const char* XMIT_POWER_H_LONG;
  const char* XMIT_POWER_V_LONG;
  const char* ZDR_CORRECTION_LONG;

private:

};

#endif
