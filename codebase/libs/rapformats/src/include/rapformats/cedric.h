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
/************************************************************************
 * CEDRIC.H Structure & definitions for cedric format files 
 *
 * F. Hage May 1991.
 * Keyword: CEDRIC File Header Structure 
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <dataport/port_types.h>
#include <stdio.h>

#define CED_MAX_FIELDS 25
#define CED_MAX_LANDMARKS 15
#define CED_MISSING_DATA -32768

  struct field_st {  /* 5 si16s TOTAL */
    char field_name[8]; /* 4 si16s */
    si16 field_sf;  /* Scale factor ?? */
  };

  struct landmark_st { /* 6 si16s TOTAL */
    char name[6];  /* 3 si16s */
    si16 x_position;  /* (km) Divide by scale_factor */
    si16 y_position;  /* (km) Divide by scale_factor */
    si16 z_position;  /* (km) Multiply by 0.001 */
  };
   
  /* VOLUME HEADER */
  typedef struct cedric_header {  /* Numbers start at 1! - FORTRAN CONVENTIONS */

    char id[8];  /* 1-4 */
    char program[6]; /* 5-7 */
    char project[4]; /* 8-9 */
    char scientist[6]; /* 10-12 */
    char radar[6]; /* 13-15 */
    char scan_mode[4]; /* 16-17 */
    char tape[6]; /* 18-20 */
  
    si16 tape_start_year; /* 21-32 */
    si16 tape_start_month;
    si16 tape_start_day;
    si16 tape_start_hour;
    si16 tape_start_min;
    si16 tape_start_sec;
    si16 tape_end_year;
    si16 tape_end_month;
    si16 tape_end_day;
    si16 tape_end_hour;
    si16 tape_end_min;
    si16 tape_end_sec;
  
    si16 lat_deg; /* 33 - Origin of data */
    si16 lat_min; /* 34 - Origin of data */
    si16 lat_sec; /* 35 - Divide by scale_factor */
    si16 lon_deg; /* 36 - Origin of data */
    si16 lon_min; /* 37 - Origin of data */
    si16 lon_sec; /* 38 - Divide by scale_factor */
  
    si16 origin_height; /* 39 */
  
    si16 angle1; /* 40 - Angle between north & X - Divide by cf */
    si16 origin_x; /* 41 - X coord of horiz axis -Divide by scale_factor */
    si16 origin_y; /* 42 - Y coord of horiz axis -Divide by scale_factor */
  
    char time_zone[4]; /* 43,44 */
  
    char sequence[6]; /* 45-47 */
    char submitter[6]; /* 48-50 */
    char date_run[8]; /* 51-54 */
    char time_run[8]; /* 55-58 */
  
    si16 sh59;
    si16 tape_ed_number;  /* 60 */
    si16 header_record_length; /* 61 */
  
    char computer[2]; /* 62 */
    si16 bits_datum; /* 63 */
    si16 blocking_mode;
    si16 block_size; /* 65 */
    si16 sh66;
    si16 missing_val; /* 67 */

    si16 scale_factor; /* 68 Scale factor for header meta data */
    si16 angle_factor; /* 69 Scale factor for angles */

    si16 sh70;
    char source[8]; /* 71-74 */
    char tape_label2[8]; /* 75-94 */
    char tape_label3[8];
    char tape_label4[8];
    char tape_label5[8];
    char tape_label6[8];
    si16 sh95;
    si16 records_plane; /* 96 */
    si16 records_field; /* 97 */
    si16 records_volume; /* 98 - Total data records */
    si16 total_records; /* 99 - Includes headers */
    si16 tot_records; /* 100 - Excludes level headers */
    char vol_name[8]; /* 101-104 */
    si16 sh105;
    si16 num_planes; /* 106 */
    si16 cubic_km; /* 107 Divide by scale_factor */
    si16 total_points; /* 108 Divide by scale_factor */
    si16 sampling_density; /* 109 - Mult by Scale_factor */
    si16 num_pulses; /* 110 */
    si16 volume_number; /* 111 */
    si16 sh112;
    si16 sh113;
    si16 sh114;
    si16 sh115;

    si16 vol_start_year;  /* 116 */
    si16 vol_start_month; /* 117 */
    si16 vol_start_day;  /* 118 */
    si16 vol_start_hour;  /* 119 */
    si16 vol_start_min;  /* 120 */
    si16 vol_start_second; /* 121 */
    si16 vol_end_year;  /* 122 */
    si16 vol_end_month;  /* 123 */
    si16 vol_end_day;  /* 124 */
    si16 vol_end_hour;  /* 125 */
    si16 vol_end_min;  /* 126 */
    si16 vol_end_second;  /* 127 */
  
    si16 volume_time; /* 128 - seconds */
    si16 index_number_time; /* 129 - time = 4 */
    si16 sh130;
    si16 sh131;
    si16 min_range;  /* 132 - Mult by scale_factor */
    si16 max_range;  /* 133 - Mult by scale_factor */
    si16 num_gates_beam; /* 134 */
    si16 gate_spacing; /* 134 */
    si16 min_gates;  /* 136 */
    si16 max_gates;  /* 137 */
    si16 sh138;
    si16 index_number_range; /* 132 - Mult by scale_factor */
    si16 sh140;
    si16 sh141;
    si16 min_azimuth;  /* 142 - clockwise (deg) Mult by cf */
    si16 max_azimuth;  /* 143 - clockwise (deg) Mult by cf */
    si16 num_beams_plane; /* 144 */
    si16 ave_angle;  /* 145 - Mult by cf */
    si16 min_beams_plane; /* 146 */
    si16 max_beams_plane; /* 147 */
    si16 num_steps_beam; /* 148 */
    si16 index_number_azimuth; /* 149 */
    si16 sh150;
    char plane_type[2]; /* 151 -Coplane "CO" or PPI - "PI" */
    si16 min_elev;  /* 152 - (deg) Mult by cf */
    si16 max_elev;  /* 153 - (deg) Mult by cf */
    si16 num_elevs;  /* 154 */
    si16 ave_delta_elev; /* 155 - (deg) Mult by cf */
    si16 ave_elev;  /* 156 - (deg) Mult by cf */
    si16 direction;  /* 157 - 1 = bottom to top */
    si16 baseline_angle; /* 158 - Mult by cf */
    si16 index_number_coplane; /* 159 */

    si16 min_x;  /* 160 = (km) Divide by scale_factor */
    si16 max_x;  /* 161 = (km) Divide by scale_factor */
    si16 nx;  /* 162 - Number of points along x */
    si16 dx;  /* 163 - distance along grid point * 1000 */
    si16 fast_axis; /* 164 - 1=X, 2=Y, 3=Z */
  
    si16 min_y;  /* 165 = (km) Divide by scale_factor */
    si16 max_y;  /* 166 = (km) Divide by scale_factor */
    si16 ny;  /* 167 - Number of points along y */
    si16 dy;  /* 168 - distance along grid point * 1000 */
    si16 mid_axis; /* 169 - 1=X, 2=Y, 3=Z */

    si16 min_z;  /* 170 = (km) Divide by (scale_factor*10)*/
    si16 max_z;  /* 171 = (km) Divide by (scale_factor*10)*/
    si16 nz;  /* 172 - Number of points along z */
    si16 dz;  /* 173 - if < 0 - grids at surface (*1000) */
    si16 slow_axis; /* 174 - 1=X, 2=Y, 3=Z */

    si16 num_fields; /* 175 */

    struct field_st field[CED_MAX_FIELDS]; /* Field Descriptions si16s 176-300 */

    ui16 points_plane; /* 301 points per field */

    si16 num_landmarks; /* 302 */
    si16 num_radars;  /* 303 */
    si16 nyquist_vel; /* 304 - Divide by scale_factor */
    si16 radar_const; /* 305 - Divide by scale_factor */
  
    struct landmark_st landmark[CED_MAX_LANDMARKS]; /* Descriptions: si16s 306 - 509 */
  
    si16 reserved[115]; /* 396-510 */
  } CED_vol_head_t;

  /* NEW 1994 FILE FORMAT HEADERS */

  typedef struct ced_file_head {
    char id[4];   /* Always "CED1" */
    si32 byte_order;  /* 0 is big endian */
    si32 file_size;  /* Size of this file */
    si32 reserved1; 
    si32 vol_index[25];  /* Seek locations in file for each volume */
    char vol_label[25][56]; /* A String for each volume */
    si32 reserved2; 
    si32 reserved3; 
    si32 reserved4; 
    si32 reserved5; 
    si32 reserved6; 
    si32 reserved7; 
  } CED_file_head_t;

  typedef struct ced_level_head {
    char id[6];   /* Always "LEVEL " */
    si16 coord;  /* Coordinate of the plane or level (X,Y,or Zkm) */
    si16 level_number;
    si16 number_fields;
    ui16 points_per_plane;
    si16 records_per_field;
    si16 records_per_plane;
    si16 nyquist_vel;
  } CED_level_head_t;

#ifdef __cplusplus
}
#endif
