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
// HrdData.hh
//
// Support for Hurricane Research Division data format
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2011
//
///////////////////////////////////////////////////////////////

#include <Radx/Radx.hh>

#ifndef HRD_DATA_HH
#define HRD_DATA_HH

////////////////////////////////
// RADAR INFO

typedef struct hrd_radar_info {

  Radx::si16 sample_size;
  Radx::si16 DSP_flag;

  /*
   * 0: Range normalization 
   * 1: Doppler channel speckle remover 
   * 2: Log channel speckle remover 
   * 3: Pulse at end of ray 
   * 4: Pulse at beginning of ray 
   * 6: Use AGC (TA only) 
   * 8-9: 0:single PRF, 1:dual PRF 2/3, 2:dual PRF 3/4
   */
  Radx::si16 refl_slope_x4096;	/* dB per A/D count */
  Radx::si16 refl_noise_thr_x16;	/* dB above noise */
  Radx::si16 clutter_cor_thr_x16;	/* signed dB */
  Radx::si16 SQI_thr;		/* same units as DSP */
  Radx::si16 width_power_thr_x16;	/* dBZ */
  Radx::si16 calib_refl_x16;	/* dBZ */
  Radx::si16 AGC_decay_code;
  Radx::si16 dual_PRF_stabil_delay; /* pulses */
  Radx::si16 thr_flags_uncorr_refl;
  Radx::si16 word_12;
  Radx::si16 thr_flags_vel;
  Radx::si16 thr_flags_width;
  Radx::si16 data_mode;		/* 1:processed data, 2:time series */

  Radx::si16 words_16_to_40[25];

  /* Range Mask Configuration */
  Radx::si16 range_b1;		/* km. portion */
  Radx::si16 variable_spacing_flag; /* 1:var, 0:fixed */
  /*
   * if variable gate spacing then the gate spacing is
   *   75 m for the first 256 gates
   *  150 m for the next  128 gates
   *  300 m for the next  128 gates
   */
  Radx::si16 bin_spacing_xe3;
  Radx::si16 num_input_bins;
  Radx::si16 range_avg_state;	/* (1,2,3,4)(undefined in var mode) */
  Radx::si16 b1_adjust_xe4;
  Radx::si16 word_47;
  Radx::si16 word_48;
  Radx::si16 num_output_bins;
  Radx::si16 word_50;

  /* Noise sample information */
  Radx::si16 PRT_noise_sample;
  Radx::si16 range_noise_sample;	/* km. */
  Radx::si16 log_rec_noise_x64;	/* A2D units */
  Radx::si16 I_A2D_offset_x256;	/* A2D units */
  Radx::si16 Q_A2D_offset_x256;	/* A2D units */

  Radx::si16 words_56_to_71[16];

  /* Miscellaneous Information */

  Radx::si16 waveln_xe4;		/* m. */
  Radx::si16 pulse_width_xe8;	/* sec. */
  Radx::si16 PRF;
  Radx::si16 word_75;
  Radx::si16 DSS_flag;		/* 0:off, 1:on */
  Radx::si16 trans_recv_number;
  Radx::si16 transmit_power;
  Radx::si16 gain_control_flag;	/* 0:full, 1:STC, 2:AGC */

  Radx::si16 words_80_to_90[11];

  /* Antenna Scanning Information */
  Radx::si16 scan_mode;
  Radx::si16 sweep_speed_x10;	/* RPM */
  Radx::ui16 tilt_angle;	/* binary angle */
  Radx::si16 sector_center;	/* degrees */
  Radx::si16 sector_width;	/* degrees */

  Radx::si16 words_96_to_300[205];

} hrd_radar_info_t;

////////////////////////////////
// MAIN HEADER

typedef struct hrd_header {

  /* tape header general information */

  Radx::si16 header_flag;		/* should be a 0 */
  Radx::si16 sizeof_header;
  Radx::si16 tape_num;
  Radx::si16 hd_fmt_ver;
  Radx::si16 word_5;
  Radx::si16 year;		
  Radx::si16 month;	
  Radx::si16 day;		
  Radx::si16 hour;		
  Radx::si16 minute;	
  Radx::si16 second;	
  char LF_menu[16];
  char TA_menu[16];
  char Data_menu[16];
  Radx::si16 word_36;
  Radx::si16 nav_system;		/* 0=ONE,1=INE1,2=INE2 */
  Radx::si16 LU_tape_drive;
  Radx::si16 aircraft_id;		/* 42,43 (0 for ground) */
  char flight_id[8];
  Radx::si16 data_header_len;	/* # 16-bit words */
  Radx::si16 ray_header_len;	/* # 16-bit words */
  Radx::si16 time_zone_offset;	/* minutes ahead of GMT */
  Radx::si16 words_47_to_80[34];
  char project_id[8];
  Radx::si16 words_85_to_100[16];

  /* LF Radar Information words 101-400 */

  hrd_radar_info_t radar_lf;

  /* TA Radar Information words 401-700 */

  hrd_radar_info_t radar_ta;

  /* comments */

  char comments[648];

} hrd_header_t;

//////////////////////
// DATA RECORD HEADER

typedef struct hrd_data_rec_header {

  Radx::si16 data_record_flag; /* should be a 1 */
  Radx::ui16 sizeof_rec;
  Radx::si16 sweep_num;
  Radx::si16 rec_num;
  Radx::si08 radar_num; /* 1:LF, 2:TA */
  Radx::si08 rec_num_flag; /* 1:first, 0:middle, 2:last */

} hrd_data_rec_header_t;

//////////////////////
// RAY HEADER

typedef struct hrd_ray_header {

  Radx::ui16 sizeof_ray;

  // field code
  // 
  // bit 7 set: reflectivity
  // bit 6 set: velocity
  // bit 5 set: width
  // bit 4 set: data from TA DSP
  // bit 3 set: data from LF DSP
  // bit 2 set: time series

  Radx::ui08 field_code;
  
  Radx::si08 year;
  Radx::si08 month;
  Radx::si08 day;
  Radx::ui08 ray_code;
  Radx::si08 hour;
  Radx::si16 minute;
  Radx::si16 seconds_x100;
  Radx::si16 latitude;	/* (binary angle) */
  Radx::si16 longitude;	/* (binary angle) word_8 */
  Radx::si16 altitude_xe3;

  /*
   * executables of the translaters earlier than 1200 Apr.18, 1994
   * will produce files with ns and ew velocities and winds
   * swapped
   */

  Radx::si16 ac_vew_x10; /* east-west velocity */
  Radx::si16 ac_vns_x10; /* north-south velocity */
  Radx::si16 ac_vud_x10; /* vertical velocity */
  Radx::si16 ac_ui_x10;  /* east-west wind */
  Radx::si16 ac_vi_x10;  /* north-south wind */
  Radx::si16 ac_wi_x10;  /* vertical wind */
  Radx::si16 RCU_status;
  Radx::si16 elevation;  /* (binary angle) */
  Radx::si16 azimuth;    /* (binary angle) */
  Radx::si16 ac_pitch;   /* (binary angle) */
  Radx::si16 ac_roll;    /* (binary angle) */
  Radx::si16 ac_drift;   /* (binary angle) */
  Radx::si16 ac_heading; /* (binary angle) */

} hrd_ray_header_t;

#endif
