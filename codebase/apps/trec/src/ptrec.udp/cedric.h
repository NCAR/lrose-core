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
 */

#define NUM_FIELDS 25
#define	NUM_LANDMARKS	15

#define CED_MISSING_DATA -32768

struct	field_st {			/* 5 shorts TOTAL */
	char	field_name[8];		/* 4 shorts */
	short	field_sf;			/* Scale factor ?? */
};

struct	landmark_st {		/* 6 shorts TOTAL */
	char	name[6];			/* 3 shorts */
	short	x_position;			/* (km)  Divide by scale_factor */
	short	y_position;			/* (km)  Divide by scale_factor */
	short	z_position;			/* (km)  Multiply by 0.001 */
};
 
typedef struct {		/* Numbers start at 1! - FORTRAN CONVENTIONS */
	char	id[8];			/* 1-4 */
	char	program[6];		/* 5-7 */
	char	project[4];		/* 8-9 */
	char	scientist[6];	/* 10-12 */
	char	radar[6];		/* 13-15 */
	char	scan_mode[4];	/* 16-17 */
	char	tape[6];		/* 18-20 */

	short	tape_start_year;	/* 21-32 */
	short	tape_start_month;
	short	tape_start_day;
	short	tape_start_hour;
	short	tape_start_min;
	short	tape_start_sec;
	short	tape_end_year;
	short	tape_end_month;
	short	tape_end_day;
	short	tape_end_hour;
	short	tape_end_min;
	short	tape_end_sec;

	short	lat_deg;		/* 33 - Origin of data */
	short	lat_min;		/* 34 - Origin of data */
	short	lat_sec;		/* 35 - Divide by scale_factor */
	short	lon_deg;		/* 36 - Origin of data */
	short	lon_min;		/* 37 - Origin of data */
	short	lon_sec;		/* 38 - Divide by scale_factor */

	short	origin_height;	/* 39 */

	short	angle1;		/* 40 - Angle between north & X - Divide by cf */
	short	origin_x;	/* 41 - X coord of horiz axis -Divide by scale_factor */
	short	origin_y;	/* 42 - Y coord of horiz axis -Divide by scale_factor */

	char	time_zone[4];	/* 43,44 */

	char	sequence[6];	/* 45-47 */
	char	submitter[6];	/* 48-50 */
	char	date[8];		/* 51-54 */
	char	time_run[8];	/* 55-58 */

	short	sh59;
	short	tape_ed_number;			/* 60 */
	short	header_record_length;	/* 61 */

	char	computer[2];	/* 62 */
	short	bits_datum;		/* 63 */
	short	sh64;
	short	block_size;		/* 65 */
	short	sh66;
	short	bad_data;		/* 67 */

	short	scale_factor;	/* 68 Divisor for float values */
	short	cf;				/* 69 Divisor for float values */

	short	sh70;
	char	source[8];		/* 71-74 */
	char	tape_label2[8];	/* 75-94 */
	char	tape_label3[8];
	char	tape_label4[8];
	char	tape_label5[8];
	char	tape_label6[8];
	short	sh95;
	short	records_plane;	/* 96 */
	short	records_field;	/* 97 */
	short	records_volume;	/* 98 - Total data records */
	short	total_records;	/* 99 - Includes headers */
	short	tot_records;	/* 100 - Excludes level headers */
	char	vol_name[8];	/* 101-104 */
	short	sh105;
	short	num_planes;		/* 106 */
	short	cubic_km;		/* 107  Divide by scale_factor */
	short	total_points;	/* 108 Divide by scale_factor */
	short	sampling_density;	/* 109 - Mult by Scale_factor */
	short	num_pulses;		/* 110 */
	short	volume_number;	/* 111 */
	short	sh112;
	short	sh113;
	short	sh114;
	short	sh115;

	short	begin_year;			/* 116 */
	short	begin_month;		/* 117 */
	short	begin_day;			/* 118 */
	short	begin_hour;			/* 119 */
	short	begin_min;			/* 120 */
	short	begin_second;		/* 121 */
	short	end_year;			/* 122 */
	short	end_month;			/* 123 */
	short	end_day;			/* 124 */
	short	end_hour;			/* 125 */
	short	end_min;			/* 126 */
	short	end_second;			/* 127 */

	short	volume_time;		/* 128 - seconds */
	short	index_number_time;		/* 129 - time = 4 */
	short	sh130;
	short	sh131;
	short	min_range;			/* 132 - Mult by scale_factor */
	short	max_range;			/* 133 - Mult by scale_factor */
	short	num_gates_beam;		/* 134 */
	short	gate_spacing;		/* 134 */
	short	min_gates;			/* 136 */
	short	max_gates;			/* 137 */
	short	sh138;
	short	index_number_range;		/* 132 - Mult by scale_factor */
	short	sh140;
	short	sh141;
	short	min_azmith;			/* 142 - clockwise (deg) Mult by cf */
	short	max_azmith;			/* 143 - clockwise (deg) Mult by cf */
	short	num_beams_plane;	/* 144 */
	short	ave_angle;			/* 145 - Mult by cf */
	short	min_beams_plane;	/* 146 */
	short	max_beams_plane;	/* 147 */
	short	num_steps_beam;		/* 148 */
	short	index_number_azmith;	/* 149 */
	short	sh150;
	char	plane_type[2];		/* 151 -Coplane "CO" or PPI - "PI" */
	short	min_elev;			/* 152 - (deg) Mult by cf */
	short	max_elev;			/* 153 - (deg) Mult by cf */
	short	num_elevs;			/* 154 */
	short	ave_delta_elev;		/* 155 - (deg) Mult by cf */
	short	ave_elev;			/* 156 - (deg) Mult by cf */
	short	direction;			/* 157 - 1 = bottom to top */
	short	baseline_angle;		/* 158 - Mult by cf */
	short	index_number_coplane;	/* 159 */

	short	min_x;			/* 160 = (km) Divide by scale_factor */
	short	max_x;			/* 161 = (km) Divide by scale_factor */
	short	nx;				/* 162 - Number of points along x */
	short	dx;				/* 163 - distance along grid point * 1000 */
	short	fast_axis;		/* 164 - 1=X, 2=Y, 3=Z */

	short	min_y;			/* 165 = (km) Divide by scale_factor */
	short	max_y;			/* 166 = (km) Divide by scale_factor */
	short	ny;				/* 167 - Number of points along y */
	short	dy;				/* 168 - distance along grid point * 1000 */
	short	mid_axis;		/* 169 - 1=X, 2=Y, 3=Z */

	short	min_z;			/* 170 = (km) Divide by (scale_factor*10)*/
	short	max_z;			/* 171 = (km) Divide by (scale_factor*10)*/
	short	nz;				/* 172 - Number of points along z */
	short	dz;				/* 173 -  if < 0 - grids at surface (*1000) */
	short	slow_axis;		/* 174 - 1=X, 2=Y, 3=Z */

	short	num_fields;		/* 175 */

	struct	field_st field[NUM_FIELDS];	/* Field Descriptions shorts 176-300 */

	unsigned short	points_plane;		/* 301 points per field */

	short	num_landmarks;		/* 302 */
	short	num_radars;			/* 303 */
	short	nyquist_vel;		/* 304 - Divide by scale_factor */
	short	radar_const;		/* 305 - Divide by scale_factor */

	struct	landmark_st landmark[NUM_LANDMARKS]; /* Descriptions: shorts 306 - 509 */

	short	reserved[115];		/* 396-510 */
} cedric_header_t;

