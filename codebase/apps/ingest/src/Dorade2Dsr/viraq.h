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
/* 	$Id: viraq.h,v 1.3 2016/03/07 01:23:00 dixon Exp $	 */

# ifndef INC_VIRAQ_HH
# define INC_VIRAQ_HH

#include <dataport/port_types.h>

# ifndef DD_BYTE_ARRAYS
# define DD_BYTE_ARRAYS

/* structure for four byte words */
typedef struct
   {
   unsigned char  zero;
   unsigned char  one;
   unsigned char  two;
   unsigned char  three;
   }
fourB;


typedef struct
   {
   char  one;
   char  two;
   char  three;
   char  four;
   }
fourb;

/* stucture for two byte words */
typedef struct
   {
   char one;
   char two;
   }
twob;

# endif /* DD_BYTE_ARRAYS */

/* definition of several different data formats */
#define DATA_SIMPLEPP    0 /* simple pulse pair ABP */
#define DATA_POLYPP      1 /* poly pulse pair ABPAB */
#define DATA_POL1        3 /* dual polarization pulse pair ABP,ABP */
#define DATA_POL2        4 /* more complex dual polarization ??????? */
#define DATA_POL3        5 /* almost full dual polarization with log integers */
#define DATA_SIMPLEPP16  6 /* simple pulse pair ABP (16-bit ints not floats) */
#define DATA_POL12       8 /* simple pulse pair ABP (16-bit ints not floats) */
#define DATA_POL_PLUS    9 /* full pol plus */
#define DATA_MAX_POL    10 /* same as full plus plus more gates */
#define DATA_HVSIMUL    11 /* simultaneous transmission of H and V */
#define DATA_SHRTPUL    12 /* same as MAX_POL with gate averaging */
#define DATA_DUALPP     15 /* DOW dual prt pulse pair ABP,ABP */
#define DATA_POL_PLUS_CMP 29	/* full pol plus */
#define DATA_MAX_POL_CMP  30	/* same as full plus plus more gates */
#define DATA_HVSIMUL_CMP  31	/* simultaneous transmission of H and V */
#define DATA_SHRTPUL_CMP  32	/* same as MAX_POL with gate averaging */

typedef twob    LeShort;
typedef fourB   LeLong;
typedef fourB   LeFloat;


/* header for each dwell describing parameters which might change dwell by dwell */
/* this structure will appear on tape before each abp set */
typedef struct  {
		char            desc[4];
		ui16            recordlen;
		si16            gates,hits;
		fl32            rcvr_pulsewidth,prt,delay; /* delay to first gate */
		char            clutterfilter,timeseries;
		si16            tsgate;
		ui32            time;      /* seconds since 1970 */
		si16            subsec;    /* fractional seconds (.1 mS) */
		fl32            az,el;
		fl32            radar_longitude; 
		fl32            radar_lattitude;
		fl32            radar_altitude;
		fl32            ew_velocity;
		fl32            ns_velocity;
		fl32            vert_velocity;
		char            dataformat;     /* 0 = abp, 1 = abpab (poly), 2 = abpab (dual prt) */
		fl32            prt2;
		fl32            fxd_angle;
		unsigned char   scan_type;
		unsigned char   scan_num;
		unsigned char   vol_num;
		ui32            ray_count;
		char            transition;
		fl32            hxmit_power;    /* on the fly hor power */
		fl32            vxmit_power;    /* on the fly ver power */
# ifdef obsolete
		char            spare[100];
# else
		fl32            yaw;            /* platform heading in degrees */
		fl32            pitch;          /* platform pitch in degrees */
		fl32            roll;           /* platform roll in degrees */
		char            spare[88];
# endif
		} HEADERV;

/* this structure gets recorded once per volume (when a parameter changes) */
typedef struct  {
		char    desc[4];
		ui16    recordlen;
		si16    rev;
		si16    year;           /* this is also in the dwell as sec from 1970 */
		char    radar_name[8];
		char    polarization;   /* H or V */
		fl32    test_pulse_pwr; /* power of test pulse (refered to antenna flange) */
		fl32    test_pulse_frq; /* test pulse frequency */
		fl32    frequency;      /* transmit frequency */
		fl32    peak_power;     /* typical xmit power (at antenna flange) read from config.rdr file */
		fl32    noise_figure;
		fl32    noise_power;    /* for subtracting from data */
		fl32    receiver_gain;  /* hor chan gain from antenna flange to VIRAQ input */
		fl32    data_sys_sat;   /* VIRAQ input power required for full scale */
		fl32    antenna_gain;
		fl32    horz_beam_width;
		fl32    vert_beam_width;
		fl32    xmit_pulsewidth; /* transmitted pulse width */
		fl32    rconst;         /* radar constant */
		fl32    phaseoffset;    /* offset for phi dp */
		fl32    vreceiver_gain; /* ver chan gain from antenna flange to VIRAQ */
		fl32    vtest_pulse_pwr; /* ver test pulse power refered to antenna flange */
		fl32    vantenna_gain;  
		fl32    vnoise_power;   /* for subtracting from data */
		fl32    zdr_fudge_factor; /* what else? */
# ifdef obsolete
		fl32    misc[4];        /* 4 more misc fl32s */
# else
                fl32    mismatch_loss;
		fl32    misc[3];        /* 3 more misc fl32s */
# endif
		char    text[960];
		} RADARV;

/* this is what the top of either the radar or dwell struct looks like */
/* it is used for recording on disk and tape */


typedef struct  {
		char            desc[4];
		LeShort           recordlen;
		LeShort           gates,hits;
		LeFloat           rcvr_pulsewidth,prt,delay; /* delay to first gate */
		char            clutterfilter,timeseries;
		LeShort           tsgate;
		LeLong          time;      /* seconds since 1970 */
		LeShort           subsec;    /* fractional seconds (.1 mS) */
		LeFloat           az,el;
		LeFloat           radar_longitude; 
		LeFloat           radar_lattitude;
		LeFloat           radar_altitude;
		LeFloat           ew_velocity;
		LeFloat           ns_velocity;
		LeFloat           vert_velocity;
		char              dataformat; /* 0 = abp, 1 = abpab (poly),
					       * 2 = abpab (dual prt) */
		LeFloat           prt2;
		LeFloat           fxd_angle;
		unsigned char     scan_type; 
		unsigned char     scan_num; /* bumped by one for each new scan */
		unsigned char     vol_num; /* bumped by one for each new vol */
		LeLong            ray_count;
		char              transition;
		LeFloat           hxmit_power;    /* on the fly hor power */
		LeFloat           vxmit_power;    /* on the fly ver power */
		char              spare[100];
		} LeHEADERV;


/* this structure gets recorded once per volume (when a parameter changes) */
typedef struct  {
		char    desc[4];
		LeShort   recordlen;
		LeShort   rev;
		LeShort   year;           /* this is also in the dwell as sec from 1970 */
		char    radar_name[8];
		char    polarization;   /* H or V */
		LeFloat   test_pulse_pwr; /* power of test pulse (refered to antenna flange) */
		LeFloat   test_pulse_frq; /* test pulse frequency */
		LeFloat   frequency;      /* transmit frequency */
		LeFloat   peak_power;     /* typical xmit power (at antenna flange) */
		LeFloat   noise_figure;
		LeFloat   noise_power;    /* for subtracting from data */
		LeFloat   receiver_gain;  /* gain from antenna flange to PIRAQ input */
		LeFloat   data_sys_sat;   /* PIRAQ input power required for full scale */
		LeFloat   antenna_gain;
		LeFloat   horz_beam_width;
		LeFloat   vert_beam_width;
		LeFloat   xmit_pulsewidth; /* transmitted pulse width */
		LeFloat   rconst;         /* radar constant */
		LeFloat   phaseoffset;    /* offset for phi dp */
		LeFloat   vreceiver_gain; /* ver chan gain from antenna flange to VIRAQ */
		LeFloat   vtest_pulse_pwr; /* ver test pulse power refered to antenna flange */
		LeFloat   vantenna_gain;  
		LeFloat   misc[6];        /* 7 more misc floats */
		char    text[960];
		} LeRADARV;

/* c------------------------------------------------------------------------ */

typedef si32 int4;
typedef fl32 float4;

struct piraqX_header {		/* /code/oye/solo/translate/piraq.h
				 * all elements start on 4-byte boundaries
				 * 8-byte elements start on 8-byte boundaries
				 * character arrays that are a multiple of 4
				 * are welcome
				 */
  char desc[4];			/* "DWLX" */
  int4 recordlen;
  int4 one;			/* always set to the value 1 (endian flag) */
  int4 byte_offset_to_data;

  int4 typeof_compression;	/*  */
  int4 rays_in_chunk;		/* a chunk is some number of compressed rays */
  int4 ray_in_chunk;				
  int4 ray_in_sweep;				

  int4 gates;
  int4 hits;
  float4 rcvr_pulsewidth;
  float4 prt;

  float4 delay;
  int4 clutterfilter;
  int4 timeseries;
  int4 tsgate;

  int4 time;
  int4 subsec;
  float4 az;
  float4 el;

  float4 radar_longitude;
  float4 radar_lattitude;
  float4 radar_altitude;
  float4 ew_velocity;

  float4 ns_velocity;
  float4 vert_velocity;
  int4 dataformat;
  float4 prt2;

  float4 fxd_angle;		/* in degrees instead of counts */
  int4 scan_type;
  int4 scan_num;
  int4 vol_num;

  int4 ray_count;
  int4 transition;
  float4 hxmit_power;
  float4 vxmit_power;

  float4 yaw;
  float4 pitch;
  float4 roll;
  float4 spare1;

  /* items from the depricated radar "RHDR" header */
  /* do not set "radar->recordlen" */
  int4 rev;
  int4 year;
  char radar_name[8];

  int4 polarization;
  float4 test_pulse_pwr;
  float4 test_pulse_frq;
  float4 frequency;

  float4 peak_power;
  float4 noise_figure;
  float4 noise_power;
  float4 receiver_gain;

  float4 data_sys_sat;
  float4 antenna_gain;
  float4 horz_beam_width;
  float4 vert_beam_width;

  float4 xmit_pulsewidth;
  float4 rconst;
  float4 phaseoffset;
  float4 vreceiver_gain;

  float4 vtest_pulse_pwr;
  float4 vantenna_gain;
  float4 vnoise_power;
  float4 zdr_fudge_factor;

  float4 mismatch_loss;
  float4 h_rconst;
  float4 v_rconst;
  float4 spare2;

  float4 test_pulse_rngs_km[2];
  float4 misc[6];

  /* always append new items so the alignment of legacy variables
   * won't change */

};

typedef struct piraqX_header PIRAQX;

/* c------------------------------------------------------------------------ */


# endif /* INC_VIRAQ_HH */



