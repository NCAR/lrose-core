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
# include       <sys/types.h> 
# include       <sys/socket.h>
# include       <sys/time.h>

typedef struct {
	short cpi_seq_num;      /* signal processor radial seq num */
	short rad_seq_num;      /* DAA radial seq number */
	short tilt_num;         /* 19 */
	short vol_count;		/* 20 */
	short scan_mode;		/* 21 */
	short target_elev;      /* 22 */
	short scan_dir;			/* 23 */
	short range_to_first_gate;	/* 24 */
	short num_spacing_segs;		/* 25 */ 
	short gates_per_beam;		/* 26 */
	short gate_spacing;		/* 27 */
	char rgs_1[2];		   /* due to SUN-4 OS 4.0 alignment problems */
	char rgs_2[32];
	char rgs_3[2];
	short prf;			/* 46 */
	short month;			/* 47 */
	short day;			/* 48 */
	short year;			/* 49 */
	short hour;			/* 50 */
	short min;			/* 51 */
	short sec;			/* 52 */
	unsigned short azimuth;		/* 53 */
	short elevation;		/* 54 */
	char radar_name[16];		/* 55 */
	char site_name[16];		/* 63 */
	char proj_name[16];		/* 71 */
	long latitude;			/* 79 */
	long longitude;			/* 81 */
	short altitude;			/* 83 */
	short ant_beamwidth;		/* 84 */
	short polarization;		/* 85 */
	short power_trans;		/* 86 */
	short freq;			/* 87 */
	short pulse_width;		/* 88 */
	char prod_name[4][8];		/* 89 */
	short scale[4];			/* 105 */
	short bias[4];			/* 109 */
	short vel_field_size;		/* 113 */
	short min_behind_uct;		/* 114 */
	short end_tilt_flag;		/* 115 - 1 for last radial in tilt else 0 */
	short vcp;
	short orig_beamwidth;
	char unused_2[22];		/* 116 */
	short start_gate_first_pkt;	/* 129 */
	short gates_first_pkt;
} Beam_hdr;

typedef struct {
	Beam_hdr        header;
	unsigned char   data[6144];
} LL_beam_rec;

typedef struct
{
	int beam_length;
	LL_beam_rec beam;
} RADAR_elevation;

