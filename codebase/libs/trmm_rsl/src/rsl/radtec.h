/*
 * RADTEC Version 3.20 data structures.
 *
 * By John H. Merritt
 *
 * May 21, 1998
 */
/*
 
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1998
            John H. Merritt
            Space Applications Corporation
            Vienna, Virginia

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


/*
 * An integer word in RADTEC is 16 bits, however, floats and doubles
 * are 32 and 64 bits respectively.
 */

typedef struct {
  double rcvr_slope;
  short  isolation;
  short  noise_level;
  short  counts_hi;
  short  counts_lo;
  short  power_hi;
  short  power_lo;
} Video_calibration; /* 16 bytes */

typedef struct {
  short  average_power;
  short  reference_range;
  double zconstant;
  double zcorrection;
} Radar_calibration; /* 20 bytes */

typedef struct {
  Video_calibration video_cal;
  Radar_calibration radar_cal;
} Log_video_transmitter; /* 36 bytes */

typedef Log_video_transmitter Doppler_transmitter;

typedef struct {
  short  doppler_velocity_range;      /* 0=0-16, 1=0-32 */
  short  ground_clutter_filter_status;
  short  adtnl_log_gc_filter_status;
  short  adtnl_lin_gc_filter_status;
} Model_info;

typedef struct {
  short  version;       /* Version number multiplied by 100.  320 means 3.20 */
  short  scan_type;     /* 1=PPI, 2=RHI */
  short  scan_mode;     /* Recording mode.  0=Log Video, 1=Doppler */
  short  seqno;         /* Sequence number of this data file. */
  short  month;         /* 1-12 */
  short  day;           /* 1-31 */
  short  year;          /* YYYY */
  short  hour;          /* 0-23 */
  short  min;           /* 0-59 */
  short  sec;           /* 0-59 */
  float  az_el;         /* RHI azimuth or PPI elevation in degrees. */
  double azim_resolution;
  double azim_offset;
  double elev_resolution;
  double elev_offset;
  double site_elevation;
  double site_latitude;
  double site_longitude;
  float  skip;             /* In microseconds, 1-1024. */
  float  range_bin_size;   /* In microseconds, 1-1024. */
  short  num_range_bins;   /* 16-240. */
  short  num_integrations; /* # of integrations comprising each ray. 1-1024. */
  short  num_rays;         /* Number of rays. */
  Log_video_transmitter log_video;
  Doppler_transmitter doppler;
  short  model;            /* Processor model. 0=none, 1=750, 2=940, 3=950. */
  Model_info model_info;
  char   spare[12];        /* Fill to 200 bytes. */
} Radtec_header;

typedef struct {
  short   ray_num;       /* Ray number.  0-n */
  float  azim_angle;    /* Azimuth angle in degrees. */
  float  elev_angle;    /* Elevation angle in degrees. */
  short  hour;          /* 0-23 */
  short  min;           /* 0-59 */
  short  sec;           /* 0-59 */
} Radtec_ray_header;

typedef float Radtec_dbz[240];
typedef struct {
  Radtec_ray_header *h;   /* May be set during reading of data.  Can
						   * be safely ignored.  However, you'd have
						   * to maintain two arrays -- headers and rays. 
						   */
  Radtec_dbz dbz;
} Radtec_ray;


typedef struct {
  Radtec_header h;
  Radtec_ray *ray;
} Radtec_file;

void radtec_free_file(Radtec_file *rfile);
Radtec_file *radtec_read_file(char *infile);
void radtec_print_header(Radtec_header *h);
void radtec_print_ray_header(Radtec_ray_header *h);

   unsigned int _explode(
      unsigned int (*read_buf)(char *buf, unsigned int *size, void *param),
      void         (*write_buf)(char *buf, unsigned int *size, void *param),
      char         *work_buf,
      void         *param);
   
   unsigned long _crc32(
      char          *buffer, 
      unsigned int  *size, 
      unsigned long *old_crc);
