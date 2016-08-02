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
/****************************************************************************
 * rp7.h : header for RP7 processor radar format
 *
 * modified from the RAP mh.h file
 *
 * Mike Dixon, RAP, NCAR, October 1990
 *
 ****************************************************************************/

/*
 * The Mile High radar data from 1989 - 1991 has 4 fields:
 *
 *            1) reflectivity
 *            2) velocity
 *            3) velocity spectrum width
 *            4) signal to noise ratio
 *
 *		Ideal		Ideal
 * Parameter   scale factor	Bias factor
 *   Z        .5dB/bit  	-30.0dBZ
 *   V	      .25m/s/bit	-32.0 m/s
 *   W	      .25m/s/bit	-32.0 m/s
 *   SN	      .5dB/bit          -30.0dBZ
 *
 * actual calculations are:
 *	Z (dBZ) = reflect  * lscale[0].scale / 100.0 + lscale[0].bias / 100.0
 *	V (m/s) = velocity * lscale[1].scale / 100.0 + lscale[1].bias / 100.0
 *	W (m/s) = width    * lscale[2].scale / 100.0 + lscale[2].bias / 100.0
 *	SN (dB) = s_n      * lscale[3].scale / 100.0 + lscale[3].bias / 100.0
 *
 *
 * Starting Dec 1992, two extra flag fields were added.
 */

#ifndef _rp7_h
#define _rp7_h

#include <dataport/port_types.h>
#include <sys/types.h>

/*
 * max number of fields
 */

#define RP7_MAX_NFIELDS 6

/*
 * factor by which scale and bias values have been multiplied
 * before storage
 */

#define RP7_SCALE_AND_BIAS_MULT 100

/*
 * scan modes
 */

#define RP7_SURVEILLANCE_MODE 8
#define RP7_RHI_MODE 3
#define RP7_SECTOR_MODE 1

/*
 * packet header code
 */

#define RP7_BEAM_PACKET_CODE 0x6006

/*
 * structures
 */

/*
 * scale for linear relationship
 */

typedef struct {
  ui16 scale;
  si16 bias;
} l_scale_t;

/*
 * mile high header
 */

typedef struct	{
  
  ui16 rec_num;         /* logical record # this beam */
  ui16 field_tape_seq;  /* field tape sequence # =0 */
  ui16 rec_type;	   /* record type 0 = data */
  ui16 year;		   /* last two digits */
  ui16 month;
  ui16 day;
  ui16 hour;
  ui16 min;
  ui16 sec;
  ui16 azimuth;          /* degrees * CF */
  ui16 elevation;	    /* degrees * CF */
  ui16 rhozero1;         /* range to leading edge of */
  ui16 rhozero2;         /* 1st gate = rhozero1
			     * + rhozero2 / 1000 (in km) */
  ui16 gate_spacing;     /* gate spacing (m) =
			     * 225 for Batch or Doppler
			     * 450 for Reflectivity mode */
  ui16 gates_per_beam;   /* gates per beam */
  ui16 samples_per_beam;
  ui16 test_pulse_level; /* 0 */
  ui16 avg_xmit_pwr;
  ui16 pulse_width;	    /* 1.67 us */
  ui16 prfx10;	    /* PRF (Hz * 10), typ. 1235 Hz */
  ui16 wavelength;	    /* wavelength (cm * 100)
			     * = 10.44 cm */
  ui16 tilt_seq;	    /* running counter of elevation
			     * scans since last start of
			     * operations */
  ui16 tilt_num;         /* identifies the sweep (scan)
			     * in volume scan (#'s 1 - 16) */
  ui16 unused1[2];
  ui16 scan_mode;	    /* 8 = Surveillance */
  ui16 cw_az_lim;	    /* azim angle of first dwell */
  ui16 ccw_az_lim;	    /* azim angle of last dwell */
  ui16 up_elev_lim;	    /* 0 */
  ui16 lo_elev_lim;	    /* 0 */
  ui16 target_elev;	    /* the elevation angle from the
			     * scan strategy table */
  ui16 sig_source;	    /*  0 = radar */
  ui16 coupler_loss;
  ui16 tp_strt;
  ui16 tp_width;
  ui16 pri_co_bl;
  ui16 scnd_co_bl;
  ui16 tp_atten;
  ui16 sys_gain;
  ui16 fix_tape;
  ui16 tp_freq_off;
  ui16 log_bw;
  ui16 lin_bw;
  ui16 beamwidth;
  ui16 scan_rate;
  ui16 unused2[2];
  ui16 vol_num; 	     /* running count of full or
			      * partial volume scans since
			      * last start of operations */
  ui16 unused3;
  ui16 polarization;	     /* 0 = horizontal */
  ui16 prf1;
  ui16 prf2;
  ui16 prf3;
  ui16 prf4;
  ui16 prf5;
  ui16 unused4;
  ui16 rec_cnt_overflow;  /* record count overflow */
  ui16 altitude;
  ui16 latitude;
  ui16 longitude;
  ui16 transit;	      /* 0 = in a scan */
  ui16 ds_id;	      /* -1 */
  ui16 rs_id;	      /* 0x4d48 - 'MH' */
  ui16 proj_num;	
  ui16 size_rp7_params;    /* # of words of params = 100 */
  ui16 rec_size;           /* size of current logical record */
  ui16 num_log_rcd;
  ui16 nfields;
  ui16 field1_desc;
  ui16 field2_desc;
  ui16 field3_desc;
  ui16 field4_desc;
  ui16 field5_desc;
  ui16 field6_desc;
  ui16 tp_max;
  ui16 tp_min;
  ui16 tp_step;
  ui16 vol_scan_prg;
  l_scale_t lscale[RP7_MAX_NFIELDS];
  ui16 rnoise_bdcal;
  ui16 rsolar;
  ui16 rgcc;
  ui16 rvtime;
  ui16 rcrec;
  ui16 unused5[4];
  ui16 live_or_sim;
  
} rp7_params_t;

typedef struct  {

  ui08 edst[6];              /* destination address */
  ui08 esrc[6];              /* source address */
  si16 nit_frame_type;
  si16 frame_format;
  si32 frame_seq_num;
  si16 frame_per_rad;
  si16 frame_num;
  char  unused[8];
  si16 pkt_start_gate;
  si16 pkt_ngates;

} rp7_frame_header_t;

/* flag byte 0 */

/* flag definitions used for clutter removal */
#define CLUTTER_FLAG    (0x08)

/* flag definitions used for point target (spike) removal */
#define SPIKE_FLAG      (0x80)

/* flag definitions used for range de-aliasing */
#define AMBIGUOUS_TRIP  ( 0 )  /* cannot tell accurately */
#define FIRST_TRIP      ( 1 )  /* first trip echo */
#define SECOND_TRIP     ( 2 )  /* second trip echo */
#define THIRD_TRIP      ( 3 )  /* third trip echo */
#define FOURTH_TRIP     ( 4 )  /* third trip echo */
#define FIFTH_TRIP      ( 5 )  /* third trip echo */
#define SIXTH_TRIP      ( 6 )  /* third trip echo */
#define OBSCURED_TRIP   ( 7 )  /* third trip echo */
#define RANGE_FLAG_MASK (0x07)


/* flag definitions used for velocity de-aliasing */
#define VEL_FLAG_MASK   (0x70)
#define VEL_FLAG_OFFSET (4)

/* flag byte 1 */

/* reserved for future use */

/* houskeeping post_proc_alg flags */
#define VEL_DEAL_ENABLED        (0x8000)
#define RANGE_DEAL_ENABLED      (0x4000)
#define SPIKE_REM_ENABLED       (0x2000)
#define CLUT_FILT_ENABLED       (0x1000)

#define VEL_DEAL_COPIED         (0x0800)
#define RANGE_DEAL_COPIED       (0x0400)
#define SPIKE_REM_COPIED        (0x0200)
#define CLUT_FILT_COPIED        (0x0100)

#define COPIED_MASK             (0xf0ff)
#define CLUT_MAP_MASK           (0x00ff)

#endif
#ifdef __cplusplus
}
#endif
