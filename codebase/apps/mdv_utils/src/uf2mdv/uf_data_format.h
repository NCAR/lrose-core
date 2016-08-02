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
#ifndef UF_DATA_FORMAT_H
#define UF_DATA_FORMAT_H
/*
 * uf_data_format.h
 *
 * header file for univeral format specification.
 *
 * Created:	Carol L. Stimmel	
 * 		July 1993
 *
 * Upgraded by: Dave Albo
 *              January 1998
 *=============================================================== 
 * Universal Format:
 *	a.  1600 bpi, 9 track tape
 *	b.  16-bit words, signed integers, 2's complement
 *	c.  Physical records, length <= 4095 words
 *	d.  File marks between volume scans
 *	e.  ASCII words are left justified, blank filled
 */

#include <stdio.h>

#define PRF(uf_pulse_rep) ((uf_pulse_rep)!=0?(1.0/(float)(uf_pulse_rep)*1000000.0):-9999.0)
#define FHPTR(f) ((Field_header *)(f))
#define RNG0(f) (float)((FHPTR(f))->rnge_frst_gate*1000 + (FHPTR(f))->adjustment)
#define RNG1(f) (float)(RNG0(f) + (FHPTR(f))->vol_space*(FHPTR(f))->num_samples-1)

#define POSITION(b, x) ((char *)(b) + 2*(x) - 2)
#define F_FLAG -99.9
#define PPI_MODE 1

#ifndef __linux
/* this is defined as nothing because of K&R incompatibilty */
#define signed
#endif

/* 
 * globals 
 */

/*
 * Mandatory Header Block
 */

typedef struct {
  char   	UF[2];		/* ASCII */
  signed short	Rec_len;	/* 16-bit words */
  signed short	non_mandatory;  /* position of nonmandatory header block */
  signed short	local_use;	/* position of local user header block */
  signed short	data_header;	/* position of data header block */
  signed short	physical_rec;   /* physical record # relative to beginning of file */
  signed short	volume_scan;	/* voulume scan number on tape */
  signed short	ray_number;	/* ray number within volume scan */
  signed short	ray_record;	/* record number within ray */
  signed short	sweep_number;	/* sweep number within this volume scan */
  char    	radar_name[8];	/* 8 ASCII characters w/processor id */
  char          site_name[8];	/* 8 ASCII characters */
  signed short	degrees_lat;	/* North is positive, South is negative */
  signed short	minutes_lat;	 
  signed short	seconds_lat;	/* X 64 of latitude */
  signed short	degrees_lon;	/* East is positive, West is negative */
  signed short	minutes_lon;
  signed short	seconds_lon;	/* X 64 of longitude */
  signed short	height_antenna;	/* above sea level (meters) */
  signed short	year;		/* last 2 digits */
  signed short	month;
  signed short	day;
  signed short	hour;
  signed short	minute;
  signed short	second;
  char      	time_zone[2];	/* 2 ASCII - UT, CS, MS, etc */
  signed short  azimuth;	/* degrees X 64 to midpoint of sample */
  signed short  elevation;	/* degrees X 64 */
  signed short  sweep_mode;   	/* 0 - Calibration
				   1 - PPI (Constant Elevation)
				   2 - Coplane
				   3 - RHI (Constant Azimuth)
				   4 - Vertical
				   5 - Target (stationary)
				   6 - Manual
				   7 - Idle (out of control) */
  signed short  fixed_angle;	/* degrees X 64...elevation of PPI, etc */
  signed short  sweep_rate;	/* (degrees/second) X 64 */
  signed short  gen_year;	/* generation */
  signed short  gen_month;
  signed short  gen_day;
  char          facility[8];	/* 8 char ASCII tape generator facility */
  signed short  missing;	/* missing flag value */
}  Mandatory_header_block;

/*
 * Data_header is a variable length structure  
 * based on the number of fields in record and position(s)
 */

typedef struct {
  signed short  num_flds_ray;	/* # of fields this ray */
  signed short  num_rec_ray;	/* # of records this ray */
  signed short  num_flds_rec;	/* # of fields this record */
  /*
  signed short  field_name;	VE, DZ, etc...
  signed short  position;	postion of 1st word on n field 
  */
} Data_header;

/*
 * Field Header  -- fixed length, not all fields are used by
 * all types of field names
 */

typedef struct {
  signed short  position;	/* postion of first data word */
  signed short  scale_factor;	/* tape value / scale factor */
  signed short  rnge_frst_gate; /* range to first gate (km) */
  signed short  adjustment;	/* adjustment to center of gate (m) */
  signed short  vol_space;	/* sample volume spacing (m) */
  signed short  num_samples;	/* number of sample volumes */
  signed short  vol_depth;	/* sample volume depth (m) */
  signed short  hor_bm_wdth;	/* horizontal beam width (degrees X 64) */
  signed short  ver_bm_wdth;	/* vertical beam width */
  signed short  receiver_wdth;	/* receiver bandwidth (MHz) */
  signed short  polarization;	/* polarization transmitted:
					0 == horizontal
					1 == vertical
					2 == circular
					>2 == elliptical */
  signed short	wavelength;	/* cm X 64 */
  signed short	num_sam_fld;	/* number of samples in field estimate */
  signed short  thresh_fld;	/* threshold field (ASCII) e.g. DM */
  signed short	thresh_val;	/* threshold value */
  signed short  scale;
  signed short  edit_code;	/* 2 ASCII */
  signed short  pulse_rep;	/* pulse repetition time (microseconds) */
  signed short  bits_volume;	/* bits per sample volume */
  signed short	Nyquist;	/* used by VE: scaled */
  signed short	FL;		/* used by VE: FL if flagged,
				   0 == bad, 1 == good (least significant bit) */
  signed short	Radar_const;	/* used by DM: 
				   RC, such that dB(Z) = [(RC + DATA)/SCALE] +
						 20log (range in km) */
  signed short  noise_pwr;	/* DM:  noise power ( db(mW) X scale ) */
  signed short  receiver_gn;	/* DM:  receiver gain ( db X scale ) */
  signed short	peak_pwr;	/* DM:  peak power ( db(mW) X scale ) */
  signed short  ant_gain;	/* DM:  antenna gain ( db X scale ) */ 
  signed short  pulse_dur;	/* DM:  pulse durations (microsecs X 64) */
} Field_header;

#endif
