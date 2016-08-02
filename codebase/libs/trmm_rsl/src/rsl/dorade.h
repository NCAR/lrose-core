/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996-1999
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
#ifndef _dorade_
#define _dorade_
#include <stdio.h>

typedef struct {
  char code[4];
  int  len;
  char *comment; /* 0..len-1 of comments (N bytes ASCII, N%4==0 */
} Comment_block;

typedef struct {
  char  code[4];          /* Code identifier for the volume descriptor. (4 ASCII "VOLD") */
  int   len;              /* Length of the volume descriptor. (4 byte int) */
  short version;          /* Version number of the format. (2 byte int) */
  short volume_number;    /* Volume number from the begining of the data set. (2 byte int) */
  int   max_bytes;        /* Maximum number of bytes in a data record. (4 byte int) */
  char  project_name[20]; /* Project name. (20 ASCII) */
  short year;             /* Year YYYY e.g. 1999 (2 byte int) */
  short month;            /* Month 1-12 (2 byte int) */
  short day;              /* Day   1-31 (2 byte int) */
  short hour;             /* Hour  1-23 (2 byte int) */
  short minute;           /* Minute 0-59 (2 byte int) */
  short second;           /* Second 0-59 (2 byte int) */
  char  flight_num[8];    /* Flight number (8 ASCII) for airborne raar or IOP number
                             for ground based radars. */
  char  facility_name[8]; /* Generation facility. (8 ASCII) */
  short gen_year;         /* Generation year YYYY. (2 byte int) */
  short gen_month;        /* Generation month 1-12. (2 byte int) */
  short gen_day;          /* Generation day   1-31. (2 byte int) */
  short nsensors;         /* Number of sennsor descriptors to follow. (2 byte int) */
} Volume_desc;

typedef struct {
  char  code[4];       /* Code identifier for the radar descriptor. (4 ASCII "RADD") */
  int   len;           /* Length of the radar descriptor. (4 byte int) */
  char  radar_name[8]; /* Radar name. (8 ASCII) */
  float radar_constant;/* Radar constant. */
  float peak_power;    /* Nominal Peak power. [kw] */
  float noise_power;   /* Nominal Noise power. [dBm] */
  float rcvr_gain;     /* Receiver gain. [dB] */
  float ant_gain;      /* Antenna gain. [dB] */
  float radar_system_gain;     /* [dB] */
  float horizontal_beam_width; /* [deg] */
  float vertical_beam_width;   /* [deg] */
  short radar_type;            /* 0--Ground
                                  1--Airborne fore
                                  2--Airborne aft
                                  3--Airborne tail
                                  4--Airborne lower fuselage
                                  5--Shipborne
                               */
  short scan_mode;             /* 0--Calibration
                                  1--PPI (Constant elevation)
                                  2--Coplane
                                  3--RHI (Constant azimuth)
                                  4--Vertical pointing
                                  5--Target (Stationary, not vertical pointing)
                                  6--Manual
                                  7--Idle (out of control)
                                  8--Surveillance
                                  9--Vertical sweep (rotation axis parallels the fuselage)
                               */
  float scan_rate;             /* Nominal scan rate. [deg/sec] */
  float start_angle;           /* Nominal start angle. [deg] */
  float stop_angle;            /* Nominal stop angle. [deg] */
  short nparam_desc;           /* Total number of parameter descriptors for this radar. (2 byte int) */
  short ndesc;                 /* Total number of descriptors for this radar. (2 byte int) */
  short compress_code;         /* Data compression format code. (2 byte int):
                                  0--no compression
                                  1--data compression (compression algorithm described
                                     in the ASCII file athe the begining of the file.
                               */
  short compress_algo;         /* Data reduction algorithm:
                                  0--No data reduction.
                                  1--Data recorded between two rotation angles.
                                  2--Data recorded between two concentic angles.
                                  3--Data recorded between two altitudes.
                                  4-N--Other types of data reduction.
                               */
  float data_reduction_param1; /* Data reduction specific parameter #1
                                  1--Smallest positive angle [deg].
                                  2--Inner circle diameter [km].
                                  3--Minimum altitude [km].
                                  4-N--Will be defined if other types created.
                               */
  float data_reduction_param2; /* Data reduction specific parameter #2
                                  1--Largest positive angle [deg].
                                  2--Outer circle diameter [km].
                                  3--Maximum altitude [km].
                                  4-N--Will be defined if other types created.
                               */
  float longitude;             /* Radar longitude [deg].  If airborne, airport longitude. */
  float latitude;              /* Radar latitude [deg].   If airborne, airport latitude. */
  float altitude;              /* Radar altitude of mean sea level (msl) [km].
                                  If airborne, airport altitude. */
  float unambiguous_velocity;  /* Effective unambiguous velocity [m/s]. */
  float unambiguous_range;     /* Effective unambiguous range [km]. */
  short nfreq;                 /* Number of freqencies transmitted (2 byte int). */
  short npulse_periods;        /* Number of different inter-pulse periods (IPP's) transmitted.
                                  (2 byte int). */
  float freq[5];               /* Frequency 1..5 [GHz] (float) */
  float period[5];             /* Interpulse Period (IPP) 1..5 [ms] (float) */
} Radar_desc;

typedef struct {
  char  code[4];       /* Code identifier. (4 ASCII "PARM") */
  int   len;           /* Length of this descriptor. */
  char  name[8];       /* Name of the parameter. */
  char  description[40]; /* Description of the parameter. */
  char  units[8];      /* Units (8 ASCII) */
  short ipp;           /* Inter-pulse periods.
                          Bit 0 set to 1 indicates IPP#1 is used in this parameter.
                          Similiarly for bits 1,2,3 and 4 and IPP's 2,3,4 and 5.
                       */
  short xmit_freq;     /* Transmittd frequencies.
                          Bit 0 set to 1 indicates Frequency#1 is used in this parameter.
                          Similiarly for bits 1,2,3 and 4 and Frequencies 2,3,4 and 5.
                       */
  float rcvr_bandwidth;/* [MHz] */
  short pulse_width;   /* [m] */
  short polarization;  /* 0--Horizontal
                          1--Vertical
                          2--Circular, Right handed
                          3--Elliptical
                          4--Circular, Left handed
                          5--Dual polarization
                       */
  short nsamp_in_dwell_time; /* Number of samples in dwell time. */
  short parameter_type;    /* 1--8  bit integer
                              2--16 bit integer
                              3--32 bit integer
                              4--floating point (32 bit IEEE)
                           */
  char  threshold_field[8];
  float threshold_value;   /* Units depend on the threshold field. */
  float scale_factor;      /* Scale factor. */
  float offset_factor;     /* meteorological val = (recorded val - offset factor) / scale factor */
  int   missing_data_flag; /* Deleted or missing data flag.  256for bytes, -999 for all others. */
} Parameter_desc;

typedef struct {
  char  code[4];     /* Code identifier. (4 ASCII "CELV") */
  int   len;         /* Length of this descriptor. */
  int   ncells;      /* Number of cells definced in this vector. */
  float *range_cell; /* Range to cell n [m] (0..ncells-1) */
} Cell_range_vector;

typedef struct {
  char  code[4];           /* Code identifier. (4 ASCII "CFAC") */
  int   len;               /* Length of this descriptor. */
  float azimuth;           /* Correction for azimuth [deg]. */
  float elevation;         /* Correction for elevation [deg]. */
  float range;             /* Correction for range delay [m]. */
  float longitude;         /* Correction for radar longitude [deg]. */
  float latitude;          /* Correction for radar latitude  [deg]. */
  float altitude;          /* Correction for radar pressure altitude (msl) [km]. */
  float height;            /* Correction for radar altitude above ground (agl) [km]. */
  float speed_east_west;   /* Correction for radar platform ground speed E->W [m/s]. */
  float speed_north_south; /* Correction for radar platform ground spedd N->S [m/s]. */
  float vertical_velocity; /* Correction for radar platform vertical velocity [m/s]. */
  float heading;           /* Correction for radar platform heading [deg]. */
  float roll;              /* Correction for radar platform roll [deg]. */
  float pitch;             /* Correction for radar platform pitch [deg]. */
  float drift;             /* Correction for radar platform drift [deg]. */
  float rotation_angle;    /* Correction for radar rotation angle [deg]. */
  float tilt_angle;        /* Correction for radar tilt angle [deg]. */
} Correction_factor_desc;

typedef struct {
  char  code[4];       /* Code identifier. (4 ASCII "SWIB") */
  int   len;           /* Length of this descriptor. */
  char  radar_name[8]; /* Radar name. */
  int   sweep_num;     /* Sweep number from beginning of volume. */
  int   nrays;         /* Number of rays recorded in this sweep. */
  float start_angle;   /* True start angle [deg]. */
  float stop_angle;    /* True stop  angle [deg]. */
  float fixed_angle;   /* Fixed angle [deg]. */
  int   filter_flag;   /* Filter flag:
                          0--No filtering in use.
                          1--ON (Algorithm described in ASCII file at beginning of the file.)
                       */
} Sweep_info;

typedef struct {
  char  code[4];   /* Code identifier. (4 ASCII "RYIB") */
  int   len;       /* Length of this descriptor. */
  int   sweep_num;
  int   jday;      /* Julian day.  (from beginning of year :-) */
  short hour;      /* Hour 0-23. */
  short minute;    /* Minute 0-59. */
  short second;    /* Second 0-59. */
  short msec;      /* Millisecond. */
  float azimuth;   /* [deg] */
  float elevation; /* [deg] */
  float peak_power;/* Peak transmitted power [kw] */
  float scan_rate; /* [deg/sec]. */
  int   status;    /* Ray status:
                      0--Normal
                      1--Transition (antenna repositioning)
                      2--Bad
                      3--Questionable
                   */
} Ray_info;

typedef struct {       /* Especially for moving radars. */
  char  code[4];       /* Code identifier. (4 ASCII "ASIB") */
  int   len;           /* Length of this descriptor. */
  float longitude;     /* Radar longitude [deg]. */
  float latitude;      /* Radar latitude [deg]. */
  float altitude;      /* Radar pressure altitude (msl) [km]. */
  float height;        /* Radar above ground altitude (agl) [km]. */
  float ew_speed;      /* Platform ground speed (E (positive) or W) [m/s]. */
  float ns_speed;      /* Platform ground speed (N (positive) or S) [m/s]. */
  float v_speed;       /* Platform vertical velocity [m/s]. */
  float heading;       /* Platform heading [deg]. */
  float roll;          /* Platform roll    [deg]. */
  float pitch;         /* Platform pitch   [deg]. */
  float drift;         /* Platform drift   [deg]. */
  float rotation;      /* Platform rotation angle [deg]. */
  float tilt;          /* Platform tilt    [deg]. */
  float ew_wind_speed; /* Horizontal wind speed at radar (toward East positive) [m/s]. */
  float ns_wind_speed; /* Horizontal wind speed at radar (toward North positive) [m/s]. */
  float v_wind_speed;  /* Vertical wind speed at radar (up is positive) [m/s]. */
  float heading_rate;  /* Heading change rate [deg/sec]. */
  float pitch_rate;    /* Pitch change rate [deg/sec]. */
} Platform_info;

typedef struct {
  char  code[4];   /* Code identifier. (4 ASCII "RDAT") */
  int   len;       /* Length of this descriptor. */
  char  name[8];   /* Name of parameter.  (See name in  'parameter descriptor'). */
  char *data;      /* Length as described in Parameter_desc. */
} Parameter_data;

/* Higher level objects */
typedef struct {
  Radar_desc              *radar_desc;
  int                      nparam;
  Parameter_desc         **p_desc;     /* 0..nparam-1 */
  Cell_range_vector       *cell_range_vector;
  Correction_factor_desc  *correction_factor_desc;
} Sensor_desc;

typedef struct {
  Ray_info        *ray_info;
  Platform_info   *platform_info;
  int             nparam;
  int            *data_len;        /* 0..nparam-1
                                    * Length of *parameter_data[i] in bytes.
                                    * This is length of data portion.
                                    */
  int            *word_size;       /* 0..nparam-1
                                    * Size of each word in *parameter_data[i].
                                    */
  Parameter_data **parameter_data; /* 0..nparam-1 */
} Data_ray;

typedef struct {
  Sweep_info *s_info;
  int        nrays;
  Data_ray  **data_ray; /* 0..nrays-1 */
} Sweep_record;

/* PROTOTYPES */
Comment_block *dorade_read_comment_block(FILE *in);

Volume_desc    *dorade_read_volume_desc    (FILE *in);

/* Sensor descriptor routines. */
Radar_desc     *dorade_read_radar_desc     (FILE *in);
Parameter_desc *dorade_read_parameter_desc (FILE *in);
Cell_range_vector      *dorade_read_cell_range_vector     (FILE *in);
Correction_factor_desc *dorade_read_correction_factor_desc(FILE *in);
Sensor_desc            *dorade_read_sensor (FILE *in);

Sweep_info *dorade_read_sweep_info(FILE *in);
Sweep_record *dorade_read_sweep(FILE *fp, Sensor_desc **sd);

/* Data Ray routines. */

Ray_info       *dorade_read_ray_info      (FILE *in);
Platform_info  *dorade_read_platform_info (FILE *in);
Parameter_data *dorade_read_parameter_data(FILE *in);
Data_ray       *dorade_read_ray           (FILE *in);

/* Memory management routines. */
void dorade_free_sweep(Sweep_record *s);
void dorade_free_data_ray(Data_ray *r);

/* Print routines. */
void dorade_print_sweep_info(Sweep_info *d);
void dorade_print_ray_info(Ray_info *d);
void dorade_print_platform_info(Platform_info *d);
void dorade_print_correction_factor_desc(Correction_factor_desc *d);
void dorade_print_cell_range_vector(Cell_range_vector *d);
void dorade_print_parameter_desc(Parameter_desc *d);
void dorade_print_radar_desc(Radar_desc *d);
void dorade_print_volume_desc(Volume_desc *d);
void dorade_print_comment_block(Comment_block *cb);
void dorade_print_sensor(Sensor_desc *s);

#endif
