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

#ifndef _SigmetData_hh
#define _SigmetData_hh

#include <Radx/Radx.hh>
#define RAW_RECORD_LEN 6144

// field identifiers
// _2 indicates 2-byte fields, but in raw files it seems
// either the 1-byte or 2-byte identifier may be used

#define FIELD_EXT_HDR       0
#define FIELD_DBZ_TOT       1
#define FIELD_DBZ           2
#define FIELD_VEL           3
#define FIELD_WIDTH         4
#define FIELD_ZDR           5
#define FIELD_ORAIN         6
#define FIELD_DBZ_CORR      7
#define FIELD_DBZ_TOT_2     8
#define FIELD_DBZ_2         9
#define FIELD_VEL_2        10
#define FIELD_WIDTH_2      11
#define FIELD_ZDR_2        12
#define FIELD_RAINRATE_2   13
#define FIELD_KDP          14
#define FIELD_KDP_2        15
#define FIELD_PHIDP        16
#define FIELD_VEL_CORR     17
#define FIELD_SQI          18
#define FIELD_RHOHV        19
#define FIELD_RHOHV_2      20
#define FIELD_DBZ_CORR_2   21
#define FIELD_VEL_CORR_2   22
#define FIELD_SQI_2        23
#define FIELD_PHIDP_2      24
#define FIELD_LDRH         25
#define FIELD_LDRH_2       26
#define FIELD_LDRV         27
#define FIELD_LDRV_2       28
#define FIELD_FLAGS        29
#define FIELD_FLAGS_2      30
#define FIELD_FLOAT32      31
#define FIELD_ECHO_TOPS    32
#define FIELD_VIL_2        33
#define FIELD_RAW          34
#define FIELD_SHEAR        35
#define FIELD_DIVERGE_2    36
#define FIELD_FLIQUID_2    37
#define FIELD_USER         38
#define FIELD_OTHER        39
#define FIELD_DEFORM_2     40
#define FIELD_VVEL_2       41
#define FIELD_HVEL_2       42
#define FIELD_HDIR_2       43
#define FIELD_AXDIL_2      44
#define FIELD_TIME_2       45
#define FIELD_RHOH         46
#define FIELD_RHOH_2       47
#define FIELD_RHOV         48
#define FIELD_RHOV_2       49
#define FIELD_PHIH         50
#define FIELD_PHIH_2       51
#define FIELD_PHIV         52
#define FIELD_PHIV_2       53
#define FIELD_USER_2       54
#define FIELD_HCLASS       55
#define FIELD_HCLASS_2     56
#define FIELD_ZDRC         57
#define FIELD_ZDRC_2       58
#define FIELD_TEMP_2       59
#define FIELD_VIR_2        60
#define FIELD_DBTV         61
#define FIELD_DBTV_2       62
#define FIELD_DBZV         63
#define FIELD_DBZV_2       64
#define FIELD_SNR          65
#define FIELD_SNR_2        66
#define FIELD_ALBEDO       67
#define FIELD_ALBEDO_2     68
#define FIELD_VILD_2       69
#define FIELD_TURB_2       70
#define FIELD_DBTE         71
#define FIELD_DBTE_2       72
#define FIELD_DBZE         73
#define FIELD_DBZE_2       74

// PRF pulsing types

#define PRF_FIXED       0
#define PRF_DUAL_2_3    1
#define PRF_DUAL_3_4    2
#define PRF_DUAL_4_5    3

///////////////////////////////////
// id_header - id for structures

typedef struct {

  /*
   * Structure identifier
   * Task_Configuration: 22
   * ingest_header: 23
   * INGEST_data_header: 24
   * Product_configuration: 26
   * Product_hdr: 27
   */
  Radx::si16 id;
  
  /*
   * Format version #
   * Task_Configuration: 5
   * ingest_header: 4
   * ingest_data_header: 3
   * Product_configuration: 6
   * Product_hdr: 8
   */
  Radx::si16 version;

  /* total size of struct, plus data */

  Radx::si32 nbytes;
  
  Radx::si16 reserved;

  /* Flag Bits - if 0x0001, incomplete struct */ 
  Radx::si16 flags;

} sigmet_id_hdr_t ;

// Time struct

typedef struct {
  Radx::ui32 sec;
  Radx::ui16 msecs; /* lower 10 bits are msecs
                     * bit 10 is daylight savings time
                     * bit 11 is UTC
                     * bit 12 local time is daylight savings time
                     */
  Radx::ui16 year, month, day;
} sigmet_time_t;

// product configuration

typedef struct {

  sigmet_id_hdr_t id_hdr; /* Generic Header */

  Radx::ui16 ptype; /* Product Type Code */

  /*
   *  PPI  = 01 
   *  RHI  = 02 
   *  CAPPI  = 03 
   *  Cross section  = 04 
   *  Echo tops  = 05 
   *  Storm track  = 06 
   *  Precipitation 1 hour  = 07 
   *  Precipitation n hour  = 08 
   *  Velocity Volume processing  = 09 
   *  Vertically Integrated Liquid  = 10 
   *  Wind shear  = 11 
   *  Warning (overlay)  = 12 
   *  Rain catchments  = 13 
   *  Range-Time-Display  = 14 
   *  Raw data set (no display) = 15 
   *  Maximum with side panels  = 16 
   *  Earth projection user product  = 17 
   *  Section projection user product  = 18 
   *  Other user product (no display)  = 19 
   *  Status product (no display)  = 20 
   *  Shear Line Product  = 21 
   *  Horizontal wind field  = 22 
   *  Beam pattern  = 23 
   *  Text  = 24 
   *  Forecast  = 25 
   *  Multi-Doppler  = 26 
   *  Arbitrary graphics image  = 27 
   *  Composite  = 28 
   *  TDWR Wind Alert  = 29 
   *  Raingage product  = 30 
   *  Dwell and bird detection product  = 31 
   *  Surface Rainfall Intensity  = 32 
   *  Echo bottoms  = 33 
   *  Height of Max Reflectivity  = 34
   */

  Radx::ui16 scheduling;
  Radx::si32 secs_between_runs;

  sigmet_time_t product_time;
  sigmet_time_t ingest_time;
  sigmet_time_t start_sched_time;
  
  char unused_1[6];
  char prod_name[12];
  char task_name[12];

  Radx::ui16 flags;

  Radx::si32 xscale, yscale, zscale;
  Radx::si32 xsize , ysize , zsize;
  Radx::si32 xrad,  yrad,  zrad;

  Radx::si32 range_last_bin_cm;
  char unused_2[2];
  Radx::ui16 data_type_out;

  char projection_name[12];
  Radx::ui16 data_type_in;
  Radx::ui08 projection_type;
  char unused_3;
  Radx::si16 radial_smoothing_range_km_100;
  Radx::ui16 n_runs;
  Radx::si32 zr_coeff;
  Radx::si32 zr_expon;
  Radx::si16 twod_smooth_x;
  Radx::si16 twod_smooth_y;

  char unused[156];
  
} prod_conf_t;

// product end

// #define PRODUCT_END_SIZE  (308)

typedef struct {

  char prod_sitename[16];
  char prod_version[8];
  char iris_version[8];
  sigmet_time_t oldest_data_time;
  char unused_1[28];
  Radx::si16 minutes_lst_west_of_gmt;
  char hardware_name[16];
  char ingest_site_name[16];
  Radx::si16 minutes_rec_west_of_gmt;
  Radx::si32 latitude_center;
  Radx::si32 longitude_center;
  Radx::si16 ground_ht_msl_meters;
  Radx::si16 radar_ht_agl_meters;
  Radx::si32 prf_hz;
  Radx::si32 pulse_width_us_100;
  Radx::ui16 dsp_type;
  Radx::ui16 trig_rate_scheme;
  Radx::si16 nsamples;
  char clut_filter_file_name[12];
  Radx::ui16 dop_filter_first_bin;
  Radx::si32 wavelength_cm_100;
  Radx::si32 trunc_ht_above_radar_cm;
  Radx::si32 range_first_bin_cm;
  Radx::si32 range_last_bin_cm;
  Radx::si32 n_gates;
  Radx::si16 input_file_count;
  Radx::ui16 polarization;
  Radx::si16 i0_cal_db_100;
  Radx::si16 cal_noise_db_100;
  Radx::si16 radar_const_h_100;
  Radx::ui16 receiver_bandwidth;
  Radx::si16 noise_level_db_100;
  Radx::si16 ldr_offset_db_100;
  Radx::si16 zdr_offset_db_100;
  char unused_2[24];

  Radx::si32 lambert_lat1;
  Radx::si32 lambert_lat2;
  Radx::ui32 earth_radius_cm;
  Radx::ui32 earth_flattening_1000000;

  Radx::ui32 faults_bits;

  Radx::ui32 site_mask;
  Radx::ui16 log_filter_first;
  Radx::ui16 dsp_clutmap;

  Radx::si32 proj_ref_lat;
  Radx::si32 proj_ref_lon;

  Radx::ui16 sequence_num;

  char unused_3[32];

  Radx::si16 melting_ht_m_msl;
  Radx::si16 ht_radar_above_ref_m;

  Radx::si16 n_results_elements;

  Radx::ui08 wind_speed;
  Radx::ui08 wind_dirn;

  char  unused_4;
  char  local_tz[8];
  char  unused_5[8];

} prod_end_t;

// product header

typedef struct {

  sigmet_id_hdr_t id_hdr;
  prod_conf_t conf;
  prod_end_t end;

} prod_header_t;

// ingest conf

typedef struct {

  char file_name[80];
  Radx::si16 num_data_files;
  Radx::si16 nsweeps_completed;
  Radx::si32 total_size_files;
  sigmet_time_t volume_time;
  char unused_1[12];

  Radx::si16 nbytes_in_ray_hdrs;
  Radx::si16 nbytes_in_ext_hdr;
  Radx::si16 nbytes_in_task_config;
  Radx::si16 playback_version;
  char unused_2[4];

  char iris_version[8];
  char hardware_name[16];
  Radx::si16 time_zone_local_mins_west_of_gmt;

  char site_name[16];
  Radx::si16 time_zone_rec_mins_west_of_gmt;
  Radx::si32 latitude;
  Radx::si32 longitude;

  Radx::si16 ground_ht_msl_meters;
  Radx::si16 radar_ht_agl_meters;

  Radx::si16 nrays_per_revolution;
  Radx::si16 index_of_first_ray;
  Radx::si16 n_rays_sweep;

  Radx::si16 nbytes_gparm;
  Radx::si32 radar_ht_msl_cm;
  Radx::si32 platform_vel[3]; // cm/s: east, north, up
  Radx::si32 ant_offset[3]; // cm: starboard, bow, up

  Radx::ui32 fault_bits;
  Radx::si16 ht_melting_m_msl;
  char unused_3[2] ;

  char timezone_name[8];
  Radx::ui32 flags;
  char conf_name[16];
  char unused_4[228];

} ingest_conf_t;

typedef struct
{
  Radx::si32 start_secs;
  Radx::si32 stop_secs;
  Radx::si32 skip_secs;
  Radx::si32 last_run_secs;
  Radx::si32 time_used_secs;
  Radx::si32 day_last_run;
  Radx::ui16 flag;
  char unused[94];
} task_sched_info_t;

// dsp info

typedef struct {
  Radx::ui32 word_0;
  Radx::ui32 xhdr_type;
  Radx::ui32 word_1;
  Radx::ui32 word_2;
  Radx::ui32 word_3;
  Radx::ui32 word_4;
} dsp_data_mask_t;

typedef struct {

  Radx::ui16 major_mode;
  Radx::ui16 dsp_type;
  dsp_data_mask_t data_mask;
  dsp_data_mask_t data_mask_orig;
  Radx::ui16 low_prf_hz;
  Radx::ui16 low_prf_frac_hz;
  Radx::si16 low_prf_sample_size;
  Radx::si16 low_prf_averaging;
  Radx::si16 refl_db_thresh_100;
  Radx::si16 velocity_db_thresh_100;
  Radx::si16 width_db_thresh_100;
  char batch_unused_end[18];
  char unused_1[52];
  Radx::si32 prf_hz;
  Radx::si32 pulse_width_usec_100;
  Radx::ui16 trig_rate_flag;
  Radx::si16 n_pulses_stabilization;
  Radx::ui16 agc_coeff;
  Radx::si16 nsamples;
  Radx::ui16 gain_code;
  char filter_name[12];
  Radx::ui08 dop_filter_first_bin;
  Radx::ui08 log_filter_unused;
  Radx::si16 fixed_gain_1000;
  Radx::ui16 gas_atten_10000;
  Radx::ui16 clutmap_num;
  Radx::ui16 xmit_phase_sequence;
  Radx::ui32 config_header_command_mask;
  Radx::ui16 ts_playback_flags;
  char unused2[2];
  char custom_ray_hdr[16];
  char unused3[120];

} task_dsp_info_t;

// calibration info

typedef struct {

  Radx::si16 log_slope;
  Radx::si16 log_noise_thresh;
  Radx::si16 clut_corr_thresh;
  Radx::si16 sqi_thresh_256;
  Radx::si16 sig_power_thresh_16;
  char unused_1[8];
  Radx::si16 z_calib_16;
  Radx::ui16 flags_z_uncorrected;
  Radx::ui16 flags_z_corrected;
  Radx::ui16 flags_vel;
  Radx::ui16 flags_width;
  Radx::ui16 flags_zdr;
  char unused_2[6];
  Radx::ui16 flags;
  char unused_3[2];
  Radx::si16 ldr_bias_100;
  Radx::si16 zdr_bias_16;
  Radx::si16 clut_thresh_100;
  Radx::ui16 clut_skip;
  Radx::si16 io_horiz_100;
  Radx::si16 io_vert_100;
  Radx::si16 cal_time_noise_100;
  Radx::si16 cal_noise_vert;
  Radx::si16 radar_const_h_100;
  Radx::si16 radar_const_v_100;
  Radx::si16 receiver_bandwidth_khz;
  char unused_4[258];

} task_calib_info_t;

// range info

typedef struct {

  Radx::si32 range_first_bin_cm;
  Radx::si32 range_last_bin_cm;
  Radx::si16 n_input_gates;
  Radx::si16 n_output_gates;
  Radx::si32 input_gate_spacing_cm;
  Radx::si32 output_gate_spacing_cm;
  Radx::ui16 bin_length_variable;
  Radx::si16 gate_averaging;
  Radx::si16 gate_smoothing;
  char unused[134];

} task_range_info_t;

// scan info
// rhi scan

#define MAX_SWEEPS 40

#define START_END_RHI_NEAREST 0
#define START_END_RHI_LOWER 1
#define START_END_RHI_UPPER 2

typedef struct {
  Radx::si16 start_el;
  Radx::si16 end_el;
  Radx::ui16 az_list[MAX_SWEEPS];
  char unused[115];
  Radx::ui08 start_end;
} rhi_scan_info_t;

// ppi scan info

#define START_END_PPI_NEAREST 0
#define START_END_PPI_LEFT 1
#define START_END_PPI_RIGHT 2

typedef struct {
  Radx::ui16 start_az;
  Radx::ui16 end_az;
  Radx::si16 el_list[MAX_SWEEPS];
  char ipad_end[115];
  Radx::ui08 start_end;  /* Which end of the sector to start at */
} ppi_scan_info_t;

typedef union {
  rhi_scan_info_t rhi;
  ppi_scan_info_t ppi;
} scan_info_union_t;

#define SCAN_MODE_PPI 1
#define SCAN_MODE_RHI 2
#define SCAN_MODE_SURVEILLANCE 4

typedef struct {

  Radx::ui16 scan_mode;
  Radx::si16 angular_res_deg_1000;
  Radx::ui16 scan_speed_bin_per_sec;
  Radx::si16 n_sweeps;
  scan_info_union_t u;
  char unused[112];

} task_scan_info_t;

// misc info

#define SINGLE_POL_H 0
#define SINGLE_POL_V 1
#define DUAL_POL_ALT 2
#define DUAL_POL_SIM 3

typedef struct {

  Radx::si32 wavelength_cm_100;
  char user_id[16];
  Radx::si32 xmit_power_watts;
  Radx::ui16 flags;
  Radx::ui16 polarization;
  Radx::si32 trunc_ht_above_radar_cm;
  char unused_1[18];
  char unused_2[12];
  Radx::si16 nbytes_comment;
  Radx::si32 beam_width_h;
  Radx::si32 beam_width_v;
  Radx::ui32 user[10];
  char ipad_end[208];

} task_misc_info_t;

// end info

typedef struct {

  Radx::si16 id_major;
  Radx::si16 id_minor;
  char task_name[12]; 
  char script[80];
  Radx::si32 ntasks_hybrid;
  Radx::ui16 task_state;
  char  unused_1[2];
  sigmet_time_t start_time;
  char ipad_end[204];

} task_end_info_t;

typedef struct
{

  sigmet_id_hdr_t id_hdr; 
  task_sched_info_t sched_info;
  task_dsp_info_t dsp_info;
  task_calib_info_t calib_info;
  task_range_info_t range_info;
  task_scan_info_t scan_info;
  task_misc_info_t misc_info;
  task_end_info_t  end_info;
  char comments[720];

} task_conf_t;

typedef struct {

  sigmet_id_hdr_t id_hdr;
  ingest_conf_t ingest_conf;
  task_conf_t task_conf;
  char unused[1780];
  
} ingest_header_t;

// raw product header

typedef struct {
  Radx::si16 record_num;
  Radx::si16 sweep_num;
  Radx::si16 byte_offset;
  Radx::si16 ray_num;
  Radx::ui16 flags; // should not be 0x0001
  Radx::ui08 unused[2];
} raw_product_header_t;

// ingest data header

typedef struct {

  sigmet_id_hdr_t id_hdr ;
  sigmet_time_t time ;	// for start of sweep
  Radx::si16 sweep_num; // starts at 1, not 0
  Radx::si16 nrays_per_revolution;
  Radx::si16 angle_of_first_pointer;
  Radx::si16 n_rays_total;
  Radx::si16 n_rays_written;
  Radx::si16 fixed_angle;
  Radx::si16 bits_per_bin;
  Radx::ui16 data_code; // from task_dsp_info.idata
  char unused[36];

} ingest_data_header_t;

// sweep data header
//
// This is found 2 bytes from the start of the sweep data
// preceded by a 2-byte code word

typedef struct {
  Radx::ui16 start_az;
  Radx::si16 start_el;
  Radx::ui16 end_az;
  Radx::si16 end_el;
  Radx::si16 n_gates;
  Radx::ui16 seconds;
} ray_header_t;

// Extended Header version 0, 3data.pdf, section 3.2.8

typedef struct {
  Radx::si32 msecs_since_sweep_start;
  Radx::si16 calib_signal_level;
  Radx::si16 spare[7];
} ext_header_ver0;

// Extended Header version 1, 3data.pdf, section 3.2.9

typedef struct {
  Radx::si32 msecs_since_sweep_start;
  Radx::si16 calib_signal_level;
  Radx::ui16 az;           /* Azimuth (binary angle) */
  Radx::si16 elev;         /* Elevation (binary angle) */
  Radx::si16 train_order;  /* Train order (binary angle) */
  Radx::si16 elev_order;   /* Elevation order (binary angle) */
  Radx::si16 pitch;        /* Pitch   (binary angle) */
  Radx::si16 roll;         /* Roll    (binary angle) */
  Radx::si16 heading;      /* Heading (binary angle) */
  Radx::si16 az_rate;      /* Azimuth Rate (binary angle/sec) */
  Radx::si16 elev_rate;    /* Elevation Rate (binary angle/sec) */
  Radx::si16 pitch_rate;   /* Pitch Rate (binary angle/sec) */
  Radx::si16 roll_rate;    /* Roll Rate (binary angle/sec) */
  Radx::si32 lat; /* Latitude (binary angle) */
  Radx::si32 lon; /* Longitude (binary angle) */
  Radx::si16 heading_rate; /* Heading Rate (binary angle/sec) */
  Radx::si16 alt_m_msl;   /* Altitude (meters) */
  Radx::si16 vel_e_cm_per_sec; /* Velocity East (cm/sec) */
  Radx::si16 vel_n_cm_per_sec; /* Velocity North (cm/sec) */
  Radx::si32 msecs_since_update; /* Time since last update (msecs) */
  Radx::si16 vel_up_cm_per_sec; /* Velocity Up (cm/sec) */
  Radx::si16 nav_sys_ok_flag;  /* Navigation system OK flag */
  Radx::si16 radial_vel_corr;  /* Radial velocity correction (velocity units) */
  Radx::si16 spare;   /* unused */
} ext_header_ver1;

// Extended Header version 2, 3data.pdf, section 3.2.10

typedef struct {
  Radx::si32 msecs_since_sweep_start;
  Radx::si16 calib_signal_level;
  Radx::si16 spare;
  Radx::si32 nbytes_in_header;
  // remainder is customer-specified
} ext_header_ver2;

// Extended header for Hurricane Research Division Tail Doppler Radar

typedef Radx::si16 bam16_t;
typedef Radx::si32 bam32_t;

typedef struct {

  Radx::si32 msecs_since_sweep_start;
  Radx::si16 calib_signal_level;
  Radx::si16 nbytes_in_header;
  Radx::ui16 __pad_1;
  Radx::ui16 gps_age_msecs; /**< Time in milliseconds since last GPS Input */
  Radx::ui16 irs_age_msecs; /**< Time in milliseconds since last IRS Input */
  Radx::ui16 aamps_age_msecs; /**< Time in milliseconds since last AAMPS Input */
  bam32_t gps_lat; /**< GPS latitude (BAM) */
  bam32_t gps_long; /**< GPS Longitude (BAM) */
  Radx::si32 gps_alt_cm; /**< GPS Altitude (cm) */
  Radx::si32 gps_vel_e_cm_per_sec; /**< GPS Ground Speed East (cm/second) */
  Radx::si32 gps_vel_n_cm_per_sec; /**< GPS Ground Speed North (cm/second) */
  Radx::si32 gps_vel_v_cm_per_sec; /**< GPS Ground Speed Up (cm/second) */
  bam32_t irs_lat; /**< IRS latitude (BAM) */
  bam32_t irs_long; /**< IRS Longitude (BAM) */
  Radx::si32 irs_vel_e_cm_per_sec; /**< IRS Ground Speed East (cm/second) */
  Radx::si32 irs_vel_n_cm_per_sec; /**< IRS Ground Speed North (cm/second) */
  Radx::si32 irs_vel_v_cm_per_sec; /**< IRS Ground Speed Up (cm/second) */
  bam16_t irs_pitch; /**< IRS Pitch (BAM) */
  bam16_t irs_roll; /**< IRS Roll (BAM) */
  bam16_t irs_heading; /**< IRS Heading (BAM) */
  bam16_t irs_drift; /**< IRS Drift (BAM) */
  bam16_t irs_tru_track; /**< IRS True Track (BAM) */
  bam16_t irs_pitch_r; /**< IRS Pitch rate (BAM/sec) */
  bam16_t irs_roll_r; /**< IRS Roll rate (BAM/sec) */
  bam16_t irs_yaw_r; /**< IRS Yaw rate (BAM/sec) */
  Radx::si32 irs_wind_vel_cm_per_sec; /**< IRS Wind speed (cm/second) */
  bam16_t irs_wind_dir; /**< IRS Wind direction (BAM) */
  Radx::ui16 __pad_2;
  bam32_t aamps_lat; /**< AAMPT latitude (BAM) */
  bam32_t aamps_long; /**< AAMPT Longitude (BAM) */
  Radx::si32 aamps_alt_cm; /**< AAMPT Altitude (cm) */
  Radx::si32 aamps_ground_vel_cm_per_sec; /**< AAMPS Ground Speed East (cm/second) */
  Radx::si32 time_stamp_secs_utc; /**< AAMPS Timestamp in UTC (seconds since the epoch) */
  Radx::si32 aamps_vel_v_cm_per_sec; /**< AAMPT Ground Speed Up (cm/second) */
  bam16_t aamps_pitch; /**< AAMPT Pitch (BAM) */
  bam16_t aamps_roll; /**< AAMPT Roll (BAM) */
  bam16_t aamps_heading; /**< AAMPT Heading (BAM) */
  bam16_t aamps_drift; /**< AAMPT Drift (BAM) */
  bam16_t aamps_track; /**< AAMPT Track (BAM) */
  Radx::ui16 __pad_4;
  Radx::si32 aamps_radar_alt_cm; /**< AAMPT altitude (cm) */
  Radx::si32 aamps_wind_vel_cm_per_sec; /**< AAMPS Wind Speed (cm/second) */
  bam16_t aamps_wind_dir; /**< AAMPS Wind direction (BAM) */
  Radx::ui16 __pad_5;
  Radx::si32 aamps_wind_vel_v_cm_per_sec; /**< AAMPS Wind Speed Up (cm/second) */
 
} hrd_tdr_ext_header_t;

#endif /* #ifndef _sigmet_headers_h */


