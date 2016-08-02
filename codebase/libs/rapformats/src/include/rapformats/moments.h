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
#include <dataport/port_types.h>

/*
 * Lincoln labs moments format
 */

typedef struct {
	si16 cpi_seq_num;      /* signal processor radial seq num */
	si16 rad_seq_num;      /* DAA radial seq number */
	si16 tilt_num;         /* 19 */
	si16 vol_count;		/* 20 */
	si16 scan_mode;		/* 21 */
	si16 target_elev;      /* 22 */
	si16 scan_dir;			/* 23 */
	si16 range_to_first_gate;	/* 24 */
	si16 num_spacing_segs;		/* 25 */ 
	si16 gates_per_beam;		/* 26 */
	si16 gate_spacing;		/* 27 */
	ui08 rgs_1[2];	/* due to SUN-4 OS 4.0 alignment problems */
	ui08 rgs_2[32];
	ui08 rgs_3[2];
	si16 prf;			/* 46 */
	si16 month;			/* 47 */
	si16 day;			/* 48 */
	si16 year;			/* 49 */
	si16 hour;			/* 50 */
	si16 min;			/* 51 */
	si16 sec;			/* 52 */
	ui16 azimuth;		/* 53 */
	si16 elevation;		/* 54 */
	ui08 radar_name[16];		/* 55 */
	ui08 site_name[16];		/* 63 */
	ui08 proj_name[16];		/* 71 */
	si32 latitude;			/* 79 */
	si32 longitude;			/* 81 */
	si16 altitude;			/* 83 */
	si16 ant_beamwidth;		/* 84 */
	si16 polarization;		/* 85 */
	si16 power_trans;		/* 86 */
	si16 freq;			/* 87 */
	si16 pulse_width;		/* 88 */
	ui08 prod_name[4][8];		/* 89 */
	si16 scale[4];			/* 105 */
	si16 bias[4];			/* 109 */
	si16 vel_field_size;		/* 113 */
	si16 min_behind_uct;		/* 114 */
	si16 end_tilt_flag;		/* 115 - 1 for last radial in tilt else 0 */
	si16 vcp;
	ui08 unused_2[24];		/* 116 */
	si16 start_gate_first_pkt;	/* 129 */
	si16 gates_first_pkt;
	} Beam_hdr;

typedef struct
{
	Beam_hdr        header;
	unsigned char   data[6144];
} LL_beam_rec;
