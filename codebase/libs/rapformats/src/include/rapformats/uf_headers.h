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
#ifdef __cplusplus
extern "C" {
#endif

  /*********************************************************************
   * uf_headers.h
   *
   * Defines the header structures used in universal format radar
   * files. The information in this file comes from the "Bulletin of
   * the American Meteorological Society", vol 61, no 11, pgs 1401-1404,
   * "SESAME News, No. 5".
   *
   * Nancy Rehak
   *
   * RAP, NCAR, Boulder, CO, USA
   *
   * March 1995
   *
   * The following UF file description is taken from the above reference:
   *
   * Each physical record in the file contains the header blocks and data.
   * Each header block is described by the typedefs below.
   *
   * Mandatory header - The first 45 words of each record have the same
   * meaning for all tapes generated in this format. The first few words
   * of this header are pointers to the start of subsequent headers.
   *
   * Optional header - This header immediately follows the mandatory
   * header. It may be of any length, including zero. The words within this
   * header, if they exist, must have the contents indicated in the format,
   * or must be flagged as bad data. (The flag is indicated in the
   * missing_data_val field of the mandatory header.) The length of the 
   * header may be expanded from time to time, and the contents of the
   * added words will be determined by mutual agreement within the
   * Doppler radar community. Such changes will not require tape reading
   * software to be changed.
   *
   * Local use header - This header immediately follows the optional header,
   * may be of any length, including zero, and the words may have any
   * meaning.
   *
   * Data header - This header, whose starting location is given by the
   * data_header_pos field of the mandatory header, tells what data fields
   * are associated with this ray and whether the data field is a part of
   * this physical record.
   *
   * Field header - This header precedes each data field and gives information
   * identifying the field. This header may have any length, but we have
   * specified the contents of the first 19 words for all fields and of
   * a few additional words for the velocity and reflected power fields.
   * 
   * Data - Data are grouped by field, in 16-bit words, integers, 2's
   * complement. The first word gives data for the sample volume nearest the
   * radar, etc.
   *
   *********************************************************************/

#ifndef UF_HEADERS_INCLUDED
#define UF_HEADERS_INCLUDED

#include <dataport/port_types.h>

  /*
   * Define values for the sweep_mode field in the mandatory header.
   */

#define UF_SWEEP_CALIBRATION          0
#define UF_SWEEP_PPI                  1
#define UF_SWEEP_COPLANE              2
#define UF_SWEEP_RHI                  3
#define UF_SWEEP_VERTICAL             4
#define UF_SWEEP_TARGET               5
#define UF_SWEEP_MANUAL               6
#define UF_SWEEP_IDLE                 7

  /*
   * Mandatory header.
   */
  
  typedef struct
{

  char  uf_string[2];	/* 1 UF (ASCII)                       */
  si16  record_length;	/* 2 Record length (16-bit words)     */
  si16  optional_header_pos; /* 3 Position of first word of        
			      *   nonmandatory header block.     
			      *   (If no nonmandatory header     
			      *   block exists, this points to   
			      *   the first existing header      
			      *   block following the mandatory. 
			      *   In this way, this value always 
			      *   gives 1 + the length of the    
			      *   mandatory header.)             */
  si16  local_use_header_pos; /* 4 Position of first word of local  
			       *   use header block. (If no local 
			       *   use headers exist, this points 
			       *   to the start of the data       
			       *   header block.)                 */
  si16  data_header_pos;	/* 5 Position of first word of data   
				 *   header block                   */
  si16  record_num;		/* 6 Physical record number relative  
				 *   to beginning of file           */
  si16  volume_scan_num;	/* 7 Volume scan number relative to   
				 *   beginning of tape              */
  si16  ray_num;		/* 8 Ray number within volume scan    */
  si16  ray_record_num;	/* 9 Physical record number within    
			 *   the ray (1 for the first       
			 *   phusical record of each ray)   */
  si16  sweep_num;	/* 10 Sweep number within this volume  
			 *   scan                           */
  char  radar_name[8];	/* 11 - 14 Radar name (8 ASCII characters,  
			 *   includes processor ID.)        */
  char  site_name[8];	/* 15 - 18 Site name (8 ASCII characters)   */
  si16  lat_degrees;	/* 19 Degrees of latitude (North is    
			 *   positive, South is negative)   */
  si16  lat_minutes;	/* 20 Minutes of latitude              */
  si16  lat_seconds;	/* 21 Seconds (x 64) of latitude       */
  si16  lon_degrees;	/* 22 Degrees of longitude (East is    
			 *   positive, West is negative)    */
  si16  lon_minutes;	/* 23 Minutes of longitude             */
  si16  lon_seconds;	/* 24 Seconds (x 64) of longitude      
			 *   (Note: minutes and seconds     
			      *   have same sign as degrees.)    */
  si16  antenna_height;	/* 25 Height of antenna above sea      
			 *   level (meters)                 */
  si16  year;		/* 26 Year (of data) (last 2 digits)   */
  si16  month;		/* 27 Month                            */
  si16  day;		/* 28 Day                              */
  si16  hour;		/* 29 Hour                             */
  si16  minute;		/* 30 Minute                           */
  si16  second;		/* 31 Second                           */
  char  time_zone[2];	/* 32 Time zone (2 ASCII -- UT, CS,    
			 *   MS, etc.)                      */
  si16  azimuth;	/* 33 Azimuth (degrees x 64) to        
			 *   to midpoint of sample          */
  si16  elevation;	/* 34 Elevation (degrees x 64)         */
  si16  sweep_mode;	/* 35 Sweep mode:                      
			 *   0 = Calibration                
			 *   1 = PPI (Constant elevation)   
			 *   2 = Coplane                    
			 *   3 = RHI (Constant azimuth)     
			 *   4 = Vertical                   
			 *   5 = Target (stationary)        
			 *   6 = Manual                     
			 *   7 = Idle (out of control)      */
  si16  fixed_angle;	/* 36 Fixed angle (degrees x 64)       
			 *   (e.g., elevation of PPI;       
			 *   azimuth of RHI; coplane angle) */
  si16  sweep_rate;	/* 37 Sweep rate ((degrees/second)x64) */
  si16  gen_year;	/* 38 Generation date of common format 
			 *   -- Year                        */
  si16  gen_month;	/* 39 Month                            */
  si16  gen_day;	/* 40 Day                              */
  char  gen_facility[8];/* 41- 44 : 8 char ASCII tape generator      
			  *   facility name                  */
  si16  missing_data_val;/* 45 Deleted or missing data flag     
			  *   (Suggest 100000 octal)         */
} uf_mandatory_header_t;


/*
 * Optional header.
 */

typedef struct 
{
  char  project_name[8];	/* 1 - 4 Project name (8 ASCII)           */
  si16  baseline_azimuth;	/* 5 Baseline azimuth (degrees x 64)  */
  si16  baseline_elevation;	/* 6 Baseline elevation (degrees x 64)*/
  si16  hour;			/* 7 Hour (start of current volume    
				 *   scan)                          */
  si16  minute;			/* 8 Minute                           */
  si16  second;			/* 9 Second                           */
  char  tape_name[8];		/* 10 - 13 Field tape name (8 ASCII)        */
  si16  flag;			/* 14 Flag (= 0 if number of range     
				 *   gates, R min, and spacing are  
				 *   the same for all data within   
				 *   this volume scan; = 1 if these 
				 *   are the same only within each  
				 *   sweep; = 2 if these are the    
				 *   same only within each ray)     */
} uf_optional_header_t;


/*
 * Repeating field information in data header.
 */

typedef struct
{
  char  field_name[2];		/* 1 Field name: e.g.                 
				 *   VE = velocity (m/s)            
				 *   SW = spectral width (m/s)      
				 *   DM = reflected poser dB (mW)   
				 *   DZ = dB(Z)                     */
  si16  field_pos;		/* 2 Position of first word in field  
				 *   header                         */
} uf_field_info_t;


/*
 * Data header.
 */

typedef struct 
{
  si16  num_ray_fields;		/* 1 Total number of fields this ray  */
  si16  num_ray_records;	/* 2 Total number of records this ray */
  si16  num_record_fields;	/* 3 Total number of fields this      
				 *   record                         */
} uf_data_header_t;

/*
 * convenience struct for referencing a data header and related
 * array of field info structs
 */

typedef struct 
{
  uf_data_header_t hdr;         /* header */

  uf_field_info_t		/* Variable length array of field   */
   field_info_array[1];	        /*   information                    */

} uf_data_hdr_info_t;

/*
 * Field header.
 */

typedef struct
{
  si16  data_pos;		/* 1 Position of first data word      */
  si16  scale_factor;		/* 2 Scale factor (meteorological     
				 *   units = tape value divided by  
				 *   scale factor)                  */
  si16  start_range;		/* 3 Range to first gate (km)         */
  si16  start_center;		/* 4 Adjustment to center of first    
				 *   gate (m)                       */
  si16  volume_spacing;		/* 5 Sample volume spacing (m)        */
  si16  num_volumes;		/* 6 Number of sample volumes         */
  si16  volume_depth;		/* 7 Sample volume depth (m)          */
  si16  horiz_beam_width;	/* 8 Horizontal beam width (degrees   
				 *   x 64)                          */
  si16  vert_beam_width;	/* 9 Vertical beam width (degrees     
				 *   x 64)                          */
  si16  receiver_bandwidth;	/* 10 Receiver bandwidth (MHz)         */
  si16  polarization;		/* 11 Polarization transmitted:        
				 *   0 = horizontal                 
				 *   1 = vertical                   
				 *   2 = circular                   
				 *   >2 = elliptical                */
  si16  wavelength;		/* 12 Wavelength (cm x 64)             */
  si16  num_samples;		/* 13 Number of samples used in field  
				 *   estimate                       */
  char  threshold_field[2];	/* 14 Threshold field (e.g. DM)        
				 *   (2 ASCII)                      */
  si16  threshold_val;		/* 15 Threshold value                  */
  si16  scale;			/* 16 Scale                            */
  char  edit_code[2];		/* 17 Edit code (2 ASCII)              */
  si16  pulse_rep_time;		/* 18 Pulse repetition time            
				 *   (microseconds)                 */
  si16  volume_bits;		/* 19 Bits per sample volume (16 for   
				 *   exchanged tape)                */
  si16  field_info[6];		/* 20 - 25 Words for individual fields      */
} uf_field_header_t;


/*
 * Special VR field information.
 */

typedef struct
{
  si16  nyquist_vel;		/* 1 Nyquist velocity (scaled)        */
  char  fl_string[2];		/* 2 FL (2 ASCII) if flagged in least 
				 *   significant bit with NCAR bad  
				 *   velocity flag (1 = good,       
				 *   0 = bad)                       */
} uf_VE_field_info_t;


/*
 * Special DM field information.
 */

typedef struct
{
  si16  radar_const;		/* 1 Radar constant = RC, such that
				 *   dB(Z) = [(RC + DATA)/SCALE] +
				 *   20 log(range in km)            */
  si16  noise_power;		/* 2 Noise power (dB(mW) x scale)     */
  si16  receiver_gain;		/* 3 Receiver gain (dB x scale)       */
  si16  peak_power;		/* 4 Peak power (dB(mW) x scale)      */
  si16  antenna_gain;		/* 5 Antenna gain (dB x scale)        */
  si16  pulse_duration;		/* 6 Pulse duration (microsec x 64)   */
} uf_DM_field_info_t;


#endif

#ifdef __cplusplus
}
#endif
