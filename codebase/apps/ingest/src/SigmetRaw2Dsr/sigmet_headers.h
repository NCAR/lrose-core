/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */

#ifndef _sigmet_headers_h
#define _sigmet_headers_h

#include <dataport/port_types.h>
#define RAW_RECORD_LEN 6144

// field identifiers
// _2 indicates 2-byte fields, but in raw files it seems
// either the 1-byte or 2-byte identifier may be used

#define FIELD_DBZ_TOT       1
#define FIELD_DBZ           2
#define FIELD_VEL           3
#define FIELD_WIDTH         4
#define FIELD_ZDR           5
#define FIELD_DBZ_CORR      7
#define FIELD_DBZ_TOT_2     8
#define FIELD_DBZ_2         9
#define FIELD_VEL_2        10
#define FIELD_WIDTH_2      11
#define FIELD_ZDR_2        12
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
  si16 id;
  
  /*
   * Format version #
   * Task_Configuration: 5
   * ingest_header: 4
   * ingest_data_header: 3
   * Product_configuration: 6
   * Product_hdr: 8
   */
  si16 version;

  /* total size of struct, plus data */

  si32 nbytes;
  
  si16 reserved;

  /* Flag Bits - if 0x0001, incomplete struct */ 
  si16 flags;

} sigmet_id_hdr_t ;

// Time struct

typedef struct {
  ui32 sec;
  ui16 msecs;
  ui16 year, month, day;
} sigmet_time_t;

// product configuration

typedef struct {

  sigmet_id_hdr_t  id_hdr;	/* Generic Header */

  ui16 ptype;			/* Product Type Code */

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

  ui16 scheduling;
  si32 secs_between_runs;

  sigmet_time_t product_time;
  sigmet_time_t ingest_time;
  sigmet_time_t start_sched_time;
  
  char unised_1[6];
  char prod_name[12];
  char task_name[12];

  ui16 flags;

  si32 xscale, yscale, zscale;
  si32 xsize , ysize , zsize;
  si32 xrad,  yrad,  zrad;

  si32 range_last_bin_cm;
  char unused_2[2];
  ui16 data_type_out;

  char projection_name[12];
  ui16 data_type_in;
  ui08 projection_type;
  char unused_3;
  si16 radial_smoothing_range_km_100;
  ui16 n_runs;
  si32 zr_coeff;
  si32 zr_expon;
  si16 twod_smooth_x;
  si16 twod_smooth_y;

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
  si16 minutes_lst_west_of_gmt;
  char hardware_name[16];
  char ingest_site_name[16];
  si16 minutes_rec_west_of_gmt;
  ui32 latitude_center;
  ui32 longitude_center;
  si16 ground_ht_msl_meters;
  si16 radar_ht_agl_meters;
  si32 prf_hz;
  si32 pulse_width_us_100;
  ui16 dsp_type;
  ui16 trig_rate_scheme;
  si16 nsamples;
  char clut_filter_file_name[12];
  ui16 dop_filter_first_bin;
  si32 wavelength_cm_100;
  si32 trunc_ht_above_radar_cm;
  si32 range_first_bin_cm;
  si32 range_last_bin_cm;
  si32 n_gates;
  si16 input_file_count;
  ui16 polarization;
  si16 i0_cal_db_100;
  si16 cal_noise_db_100;
  si16 radar_cont_h_100;
  ui16 receiver_bandwidth;
  si16 noise_level_db_100;
  char unused_2[28];

  ui32 lambert_lat1;
  ui32 lambert_lat2;
  ui32 earth_radius_cm;
  ui32 earth_flattening_1000000;

  ui32 faults_bits;

  ui32 site_mask;
  ui16 log_filter_first;
  ui16 dsp_clutmap;

  ui32 proj_ref_lat;
  ui32 proj_ref_lon;

  ui16 sequence_num;

  char unused_3[32];

  si16 melting_ht_m_msl;
  si16 ht_radar_above_ref_m;

  si16 n_results_elements;

  ui08 wind_speed;
  ui08 wind_dirn;

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
  si16 num_data_files;
  si16 nsweeps_completed;
  si32 total_size_files;
  sigmet_time_t volume_time;
  char unused_1[12];

  si16 nbytes_in_ray_hdrs;
  si16 nbytes_in_ext_hdr;
  si16 nbytes_in_task_config;
  si16 playback_version;
  char unused_2[4];

  char iris_version[8];
  char hardware_name[16];
  si16 time_zone_local_mins_west_of_gmt;

  char site_name[16];
  si16 time_zone_rec_mins_west_of_gmt;
  si32 latitude;
  si32 longitude;

  si16 ground_ht_msl_meters;
  si16 radar_ht_agl_meters;

  si16 nrays_per_revolution;
  si16 index_of_first_ray;
  si16 n_rays_sweep;

  si16 nbytes_gparm;
  si32 radar_ht_msl_cm;
  si32 platform_vel[3]; // cm/s: east, north, up
  si32 ant_offset[3]; // cm: starboard, bow, up

  ui32 fault_bits;
  si16 ht_melting_m_msl;
  char unused_3[2] ;

  char timezone_name[8];
  ui32 flags;
  char conf_name[16];
  char unused_4[228];

} ingest_conf_t;

typedef struct
{
  si32 start_secs;
  si32 stop_secs;
  si32 skip_secs;
  si32 last_run_secs;
  si32 time_used_secs;
  si32 day_last_run;
  ui16 flag;
  char unused[94];
} task_sched_info_t;

// dsp info

typedef struct {
  ui32 word_0;
  ui32 xhdr_type;
  ui32 word_1;
  ui32 word_2;
  ui32 word_3;
  ui32 word_4;
} dsp_data_mask_t;

typedef struct {

  ui16 major_mode;
  ui16 dsp_type;
  dsp_data_mask_t data_mask;
  dsp_data_mask_t data_mask_orig;
  ui16 low_prf_hz;
  ui16 low_prf_frac_hz;
  si16 low_prf_sample_size;
  si16 low_prf_averaging;
  si16 refl_db_thresh_100;
  si16 velocity_db_thresh_100;
  si16 width_db_thresh_100;
  char batch_unused_end[18];
  char unused_1[52];
  si32 prf_hz;
  si32 pulse_width_usec_100;
  ui16 trig_rate_flag;
  si16 n_pulses_stabilization;
  ui16 agc_coeff;
  si16 nsamples;
  ui16 gain_code;
  char filter_name[12];
  ui08 dop_filter_first_bin;
  ui08 log_filter_unused;
  si16 fixed_gain_1000;
  ui16 gas_atten_10000;
  ui16 clutmap_num;
  ui16 xmit_phase_sequence;
  ui32 config_header_command_mask;
  ui16 ts_playback_flags;
  char unused2[2];
  char custom_ray_hdr[16];
  char unused3[120];

} task_dsp_info_t;

// calibration info

typedef struct {

  si16 log_slope;
  si16 log_noise_thresh;
  si16 clut_corr_thresh;
  si16 sqi_thresh_256;
  si16 sig_power_thresh_16;
  char unused_1[8];
  si16 z_calib_16;
  ui16 flags_z_uncorrected;
  ui16 flags_z_corrected;
  ui16 flags_vel;
  ui16 flags_width;
  ui16 flags_zdr;
  char unused_2[6];
  ui16 flags;
  char unused_3[2];
  si16 ldr_bias_100;
  si16 zdr_bias_16;
  si16 clut_thresh_100;
  ui16 clut_skip;
  si16 io_horiz_100;
  si16 io_vert_100;
  si16 cal_time_noise_100;
  si16 cal_noise_vert;
  si16 radar_cont_h_100;
  si16 radar_cont_v_100;
  si16 receiver_bandwidth_khz;
  char unused_4[258];

} task_calib_info_t;

// range info

typedef struct {

  si32 range_first_bin_cm;
  si32 range_last_bin_cm;
  si16 n_input_gates;
  si16 n_output_gates;
  si32 input_gate_spacing_cm;
  si32 output_gate_spacing_cm;
  ui16 bin_length_variable;
  si16 gate_averaging;
  si16 gate_smoothing;
  char unused[134];

} task_range_info_t;

// scan info
// rhi scan

#define MAX_SWEEPS 40

#define START_END_RHI_NEAREST 0
#define START_END_RHI_LOWER 1
#define START_END_RHI_UPPER 2

typedef struct {
  ui16 start_el;
  ui16 end_el;
  ui16 az_list[MAX_SWEEPS];
  char unused[115];
  ui08 start_end;
} rhi_scan_info_t;

// ppi scan info

#define START_END_PPI_NEAREST 0
#define START_END_PPI_LEFT 1
#define START_END_PPI_RIGHT 2

typedef struct {
  ui16 start_az;
  ui16 end_az;
  ui16 el_list[MAX_SWEEPS];
  char ipad_end[115];
  ui08 start_end;  /* Which end of the sector to start at */
} ppi_scan_info_t;

typedef union {
  rhi_scan_info_t rhi;
  ppi_scan_info_t ppi;
} scan_info_union_t;

#define SCAN_MODE_PPI 1
#define SCAN_MODE_RHI 2
#define SCAN_MODE_SURVEILLANCE 4

typedef struct {

  ui16 scan_mode;
  si16 angular_res_deg_1000;
  ui16 scan_speed_bin_per_sec;
  si16 n_sweeps;
  scan_info_union_t u;
  char unused[112];

} task_scan_info_t;

// misc info

#define SINGLE_POL_H 0
#define SINGLE_POL_V 1
#define DUAL_POL_ALT 2
#define DUAL_POL_SIM 3

typedef struct {

  si32 wavelength_cm_100;
  char user_id[16];
  si32 xmit_power_watts;
  ui16 flags;
  ui16 polarization;
  si32 trunc_ht_above_radar_cm;
  char unused_1[18];
  char unused_2[12];
  si16 nbytes_comment;
  si32 beam_width_h;
  si32 beam_width_v;
  ui32 user[10];
  char ipad_end[208];

} task_misc_info_t;

// end info

typedef struct {

  si16 id_major;
  si16 id_minor;
  char task_name[12]; 
  char script[80];
  si32 ntasks_hybrid;
  ui16 task_state;
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
  si16 record_num;
  si16 sweep_num;
  si16 byte_offset;
  si16 ray_num;
  ui16 flags; // should not be 0x0001
  ui08 unused[2];
} raw_product_header_t;

// ingest data header

typedef struct {

  sigmet_id_hdr_t id_hdr ;
  sigmet_time_t time ;	// for start of sweep
  si16 sweep_num; // starts at 1, not 0
  si16 nrays_per_revolution;
  si16 angle_of_first_pointer;
  si16 n_rays_total;
  si16 n_rays_written;
  ui16 fixed_angle;
  si16 bits_per_bin;
  ui16 data_code; // from task_dsp_info.idata
  char unused[36];

} ingest_data_header_t;

// sweep data header
//
// This is found 2 bytes from the start of the sweep data
// preceded by a 2-byte code word

typedef struct {
  ui16 start_az;
  ui16 start_el;
  ui16 end_az;
  ui16 end_el;
  si16 n_gates;
  ui16 seconds;
} ray_header_t;

// Extended Header version 0, section 3.4.3, page III-29
// Based on trmm_rsl library

typedef struct {
  si32 msec;
  si16 cal_sig;
  si16 spare[7];
} ext_header_ver0;


// Extended Header version 1, section 3.4.3, page III-29
// Based on trmm_rsl library

typedef struct {
  si16 msec_msb;   /* Time in milliseconds from the sweep starting time. */
  si16 msec_lsb;   /* Time in milliseconds from the sweep starting time. */
  si16 cal_sig;    /* Calibration Signal level. */
  ui16 azm;        /* Azimuth (binary angle) */
  ui16 elev;       /* Elevation (binary angle) */
  ui16 train_ord;  /* Train order (binary angle) */
  ui16 elev_ord;   /* Elevation order (binary angle) */
  ui16 pitch;      /* Pitch   (binary angle) */
  ui16 roll;       /* Roll    (binary angle) */
  ui16 heading;    /* Heading (binary angle) */
  ui16 azm_rate;   /* Azimuth Rate (binary angle/sec) */
  ui16 elev_rate;  /* Elevation Rate (binary angle/sec) */
  ui16 pitch_rate; /* Pitch Rate (binary angle/sec) */
  ui16 roll_rate;  /* Roll Rate (binary angle/sec) */
  si16 lat_msb; /* Latitude (binary angle) */
  si16 lat_lsb; /* Latitude (binary angle) */
  si16 lon_msb; /* Longitude (binary angle) */
  si16 lon_lsb; /* Longitude (binary angle) */
  ui16 heading_rate; /* Heading Rate (binary angle/sec) */
  si16 alt;   /* Altitude (meters) */
  si16 vel_e; /* Velocity East (cm/sec) */
  si16 vel_n; /* Velocity North (cm/sec) */
  si16 time_update_msb; /* Time since last update (msecs) - most sig byte */
  si16 time_update_lsb; /* Time since last update (msecs) - least sig byte */
  si16 vel_u; /* Velocity Up (cm/sec) */
  si16 nav_sys_flag;  /* Navigation system OK flag */
  si16 rad_vel_cor;   /* Radial velocity correction (velocity units) */
} ext_header_ver1;

#endif /* #ifndef _sigmet_headers_h */


