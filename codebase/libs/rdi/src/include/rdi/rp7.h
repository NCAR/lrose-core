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
 *              Ideal           Ideal
 * Parameter   scale factor     Bias factor
 *   Z        .5dB/bit          -30.0dBZ
 *   V        .25m/s/bit        -32.0 m/s
 *   W        .25m/s/bit        -32.0 m/s
 *   SN       .5dB/bit          -30.0dBZ
 *
 * actual calculations are:
 *      Z (dBZ) = reflect  * lscale[0].scale / 100.0 + lscale[0].bias / 100.0
 *      V (m/s) = velocity * lscale[1].scale / 100.0 + lscale[1].bias / 100.0
 *      W (m/s) = width    * lscale[2].scale / 100.0 + lscale[2].bias / 100.0
 *      SN (dB) = s_n      * lscale[3].scale / 100.0 + lscale[3].bias / 100.0
 *
 *
 * Starting Dec 1992, two extra flag fields were added.
 */

#ifndef _rp7_h
#define _rp7_h

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
    u_short scale;
    short bias;
} l_scale_t;

/*
 * mile high header
 */

typedef struct {

    u_short rec_num;		/* logical record # this beam */
    u_short field_tape_seq;	/* field tape sequence # =0 */
    u_short rec_type;		/* record type 0 = data */
    u_short year;		/* last two digits */
    u_short month;
    u_short day;
    u_short hour;
    u_short min;
    u_short sec;
    u_short azimuth;		/* degrees * CF */
    u_short elevation;		/* degrees * CF */
    u_short rhozero1;		/* range to leading edge of */
    u_short rhozero2;		/* 1st gate = rhozero1 * + rhozero2 / 1000
				   (in km) */
    u_short gate_spacing;	/* gate spacing (m) = * 225 for Batch or
				   Doppler * 450 for Reflectivity mode */
    u_short gates_per_beam;	/* gates per beam */
    u_short samples_per_beam;
    u_short test_pulse_level;	/* 0 */
    u_short avg_xmit_pwr;
    u_short pulse_width;	/* 1.67 us */
    u_short prfx10;		/* PRF (Hz * 10), typ. 1235 Hz */
    u_short wavelength;		/* wavelength (cm * 100) * = 10.44 cm */
    u_short tilt_seq;		/* running counter of elevation * scans since 

				   last start of * operations */
    u_short tilt_num;		/* identifies the sweep (scan) * in volume
				   scan (#'s 1 - 16) */
    u_short unused1[2];
    u_short scan_mode;		/* 8 = Surveillance */
    u_short cw_az_lim;		/* azim angle of first dwell */
    u_short ccw_az_lim;		/* azim angle of last dwell */
    u_short up_elev_lim;	/* 0 */
    u_short lo_elev_lim;	/* 0 */
    u_short target_elev;	/* the elevation angle from the * scan
				   strategy table */
    u_short sig_source;		/* 0 = radar */
    u_short coupler_loss;
    u_short tp_strt;
    u_short tp_width;
    u_short pri_co_bl;
    u_short scnd_co_bl;
    u_short tp_atten;
    u_short sys_gain;
    u_short fix_tape;
    u_short tp_freq_off;
    u_short log_bw;
    u_short lin_bw;
    u_short beamwidth;
    u_short scan_rate;
    u_short unused2[2];
    u_short vol_num;		/* running count of full or * partial volume
				   scans since * last start of operations */
    u_short unused3;
    u_short polarization;	/* 0 = horizontal */
    u_short prf1;
    u_short prf2;
    u_short prf3;
    u_short prf4;
    u_short prf5;
    u_short unused4;
    u_short rec_cnt_overflow;	/* record count overflow */
    u_short altitude;
    u_short latitude;
    u_short longitude;
    u_short transit;		/* 0 = in a scan */
    u_short ds_id;		/* -1 */
    u_short rs_id;		/* 0x4d48 - 'MH' */
    u_short proj_num;
    u_short size_rp7_params;	/* # of words of params = 100 */
    u_short rec_size;		/* size of current logical record */
    u_short num_log_rcd;
    u_short nfields;
    u_short field1_desc;
    u_short field2_desc;
    u_short field3_desc;
    u_short field4_desc;
    u_short field5_desc;
    u_short field6_desc;
    u_short tp_max;
    u_short tp_min;
    u_short tp_step;
    u_short vol_scan_prg;
    l_scale_t lscale[RP7_MAX_NFIELDS];
    u_short rnoise_bdcal;
    u_short rsolar;
    u_short rgcc;
    u_short rvtime;
    u_short rcrec;
    u_short unused5[4];
    u_short live_or_sim;

} rp7_params_t;

typedef struct {

    u_char edst[6];		/* destination address */
    u_char esrc[6];		/* source address */
    short enet_frame_type;
    short frame_format;
    long frame_seq_num;
    short frame_per_rad;
    short frame_num;
    char unused[8];
    short pkt_start_gate;
    short pkt_ngates;

} rp7_frame_header_t;

#endif

#ifdef __cplusplus             
}
#endif
