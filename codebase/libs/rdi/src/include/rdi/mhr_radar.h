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
From blackbur @ rap.ucar.EDU Wed Mar 16 16:48:53 1994
  Received:from profile.rap.ucar.EDU by rap.rap.ucar.
EDU (8.6 .4 / NCAR Mail Server 04 / 10 / 90)
     id JAA08933;
Wed, 16 Mar 1994 09: 48:52 - 0700
Date: Wed, 16 Mar 1994 09: 48:50 - 0700
From:blackbur @ rap.ucar.EDU (Gary Blackburn)
Message - Id:<199403161648. JAA14772 @ profile.rap.ucar.EDU >
Received:from localhost by profile.rap.ucar.EDU (8.6 .4 / NCAR Mail Server 04 / 10 / 90)
     id JAA14772;
Wed, 16 Mar 1994 09: 48:50 - 0700
To:jing
Subject:header file
Status:R

#include       <sys/types.h>
#include       <sys/socket.h>
#include       <sys/time.h>
#include       <net/if.h>
#include       <netinet/in.h>
#include       <netinet/if_ether.h>
#include       <net/packetfilt.h>
#include       <net/nit_if.h>
#include       <net/nit_pf.h>
#include       <stropts.h>

/*----------------------------------------------------------------------------*/

#define	MH
#define	CFT

/*----------------------------------------------------------------------------*/

#ifdef	CFT

#define MH_TAPE_DESCRIPTOR    1
#define MH_VOLUME_SCAN_HEADER 2
#define MH_COMMENT            3
#define MH_TILT_HEADER        4
#define MH_RANGE_GATE_SPACING 5
#define MH_OPTIONAL_INFO      6
#define MH_RADIAL_HEADER      7
#define MH_RADIAL_DATA        8
#define MH_TILT_TRAILER       9
#define MH_VOLUME_SCAN_TRAILER 10

#define CFT_TAPE_DESCRIPTOR    1
#define CFT_VOLUME_SCAN_HEADER 2
#define CFT_COMMENT            3
#define CFT_TILT_HEADER        4
#define CFT_RANGE_GATE_SPACING 5
#define CFT_OPTIONAL_INFO      6
#define CFT_RADIAL_HEADER      7
#define CFT_RADIAL_DATA        8
#define CFT_TILT_TRAILER       9
#define CFT_VOLUME_SCAN_TRAILER 10

#define BUFF_LIMIT 8192
#define MAX_FRAMES_PER_RAD 4
#define ETHERNET_FRAME_SIZE 756	/* largest frame to be sent (words) */
#define MAX_NUM_GATES 1024

#define LINK_HDR_LEN (0)

#define LONG_HEADER_LEN (130+LINK_HDR_LEN)
#define SHORT_HEADER_LEN (18+LINK_HDR_LEN)
#define TRAILER_LEN 0

#define	MAX_DATA_BUFFER		 (ETHERNET_FRAME_SIZE * MAX_FRAMES_PER_RAD)
#define ENET_BUFFER_SIZE (ETHERNET_FRAME_SIZE*MAX_FRAMES_PER_RAD+LONG_HEADER_LEN)

#define GATES_IN_FRAME_1      (2*(ETHERNET_FRAME_SIZE-LONG_HEADER_LEN-TRAILER_LEN)/4)
#define GATES_IN_LATER_FRAMES (2*(ETHERNET_FRAME_SIZE-SHORT_HEADER_LEN-TRAILER_LEN)/4)

#define MAX_PRODS     100

#define PROD_CODE_OFFSET    (2*(100-1))
#define CODE_LEN            (2)
#define PROD_NAME_OFFSET    (2*(200-1))
#define NAME_LEN            (8)
#define PROD_SCALING_OFFSET (2*(600-1))
#define SCALE_LEN            (16)

     struct time12 {
	 short month;
	 short day;
	 short year;
	 short hour;
	 short minute;
	 short second;
     };

     struct record_header {
	 short rec_type;
	 short rec_len;
     };

     typedef struct record_header Rec_head;


     typedef struct time12 Time12;

     struct comment {
	 Rec_head header;
	 char text_line[8192];
     };

     typedef struct comment Comment;

     struct tilt_hdr {
	 Rec_head header;
	 Time12 tilt_start_time;
	 short tilt_type;	/* 1=ppi,2=rhi,3=coplane,4=fixed,5=manual */
	 short fixed_angle;	/* deg*100 */
	 short direction;	/* +1, 0, or -1 */
     };

     typedef struct tilt_hdr Tilt_hdr;

     struct spacing_info {
	 short num_gates;
	 short spacing;
     };

     typedef struct spacing_info Spacing_info;

     struct vol_scan_hdr {
	 Rec_head header;
	 int vol_scan_id_code;
	 Time12 scan_start_time;
	 short local_time_zone;
	 unsigned short latitude_hw;	/* latitude high word */
	 unsigned short latitude_lw;	/* latitude low word */
	 unsigned short longitude_hw;	/* longitude high word */
	 unsigned short longitude_lw;	/* longitude low word */
	 short altitude;
	 char radar_name[16];
	 char site_name[16];
	 char project_name[16];
	 Time12 tape_creation_time;
	 short beamwidth;	/* two way deg*100 */
	 short polarization;	/* 1=hor, 2=ver, 3=cir, 4=ellip */
	 short peak_power;	/* dBm*256   */
	 short trans_freq;	/* MHz */
	 short pulse_width;	/* nanosec */

     };

     typedef struct vol_scan_hdr Vol_scan_hdr;

     struct range_gate_spacing {
	 Rec_head header;
	 short prod_code;	/* <=0 means for ALL products */
	 short range_to_1st_gate;	/* m */
	 Spacing_info gate_spacing[1000];
     };

     typedef struct range_gate_spacing Range_gate_spacing;

     struct radial_hdr {
	 Rec_head header;
	 Time12 radial_time;
	 short azimuth;		/* deg*100 */
	 short elev;		/* deg*100 */
	 short prf;		/* pulse/sec */
	 short rec_noise;	/* dBm */
     };

     typedef struct radial_hdr Radial_hdr;

     struct radial_data {
	 Rec_head header;
	 short prod_code;
	 short index;
	 short rad_data[4096];
     };

     typedef struct radial_data Radial_data;

     typedef struct {
	 short cpi_seq_num;	/* signal processor radial seq num */
	 short rad_seq_num;	/* DAA radial seq number */
	 short tilt_num;	/* 19 */
	 short vol_count;	/* 20 */
	 short scan_mode;	/* 21 */
	 short target_elev;	/* 22 */
	 short scan_dir;	/* 23 */
	 short range_to_first_gate;	/* 24 */
	 short num_spacing_segs;	/* 25 */
	 short gates_per_beam;	/* 26 */
	 short gate_spacing;	/* 27 */
	 char rgs_1[2];		/* due to SUN-4 OS 4.0 alignment problems */
	 char rgs_2[32];
	 char rgs_3[2];
	 short prf;		/* 46 */
	 short month;		/* 47 */
	 short day;		/* 48 */
	 short year;		/* 49 */
	 short hour;		/* 50 */
	 short min;		/* 51 */
	 short sec;		/* 52 */
	 unsigned short azimuth;	/* 53 */
	 short elevation;	/* 54 */
	 char radar_name[16];	/* 55 */
	 char site_name[16];	/* 63 */
	 char proj_name[16];	/* 71 */
	 long latitude;		/* 79 */
	 long longitude;	/* 81 */
	 short altitude;	/* 83 */
	 short ant_beamwidth;	/* 84 */
	 short polarization;	/* 85 */
	 short power_trans;	/* 86 */
	 short freq;		/* 87 */
	 short pulse_width;	/* 88 */
	 char prod_name[4][8];	/* 89 */
	 short scale[4];	/* 105 */
	 short bias[4];		/* 109 */
	 short vel_field_size;	/* 113 */
	 short min_behind_uct;	/* 114 */
	 short end_tilt_flag;	/* 115 - 1 for last radial in tilt else 0 */
	 char unused_2[26];	/* 116 */
	 short start_gate_first_pkt;	/* 129 */
	 short gates_first_pkt;
     } Beam_hdr;

     struct long_header {
	 char addresses[12];
	 short enet_frame_type;
	 short frame_format;
	 int frame_seq_num;
	 short frame_per_rad;
	 short frame_num;
	 char junk[8];
	 short CPI_seq_num;
	 short rad_seq_num;
	 short tilt_num;
	 short scan_num;
	 short tilt_type;
	 short fixed_angle;
	 short scan_dir;
	 short range_to_first_gate;
	 short num_spacing_segs;
	 short num_gates;
	 short gate_spacing;
	 char rgs_1[2];		/* due to SUN-4 OS 4.0 alignment problems */
	 char rgs_2[32];
	 char rgs_3[2];
	 short prf;
	 short mon;
	 short day;
	 short year;
	 short hour;
	 short min;
	 short sec;
	 short azi;
	 short ele;
	 char radar_name[16];
	 char site_name[16];
	 char proj_name[16];
	 int lat;
	 int lon;
	 short alt;
	 short bw;
	 short polar;
	 short power_trans;
	 short freq;
	 short pw;
	 char prod_name[4][8];
	 short resolution[4];
	 short min_val[4];
	 char junk1[32];
	 short stg;
	 short ngat;
     };

     typedef struct long_header Long_header;

#endif

/*----------------------------------------------------------------------------*/

#ifdef		MH

#define	FIRST_PKT	1
#define	CONTINUE_PKT	2
#define	FR_TYPE		0x6006
#define	LIVE		0xAAAA
#define	SIM		0x5555
#define	CORR_FACT	182.04657

     typedef struct {
	 unsigned char edst[6];	/* destination */
	 unsigned char esrc[6];	/* source addr */
	 unsigned short etype;	/* frame type code 'FR_TYPE' */
	 unsigned short fr_fmt;	/* frame format FIRST_PKT or * CONTINUE_PKT */
	 unsigned long fr_seq;	/* frame sequence # */
	 unsigned short fr_frames_per_rad;	/* frames per radial (3) */
	 unsigned short fr_frame_num_in_rad;	/* frame in radial (1,2,3) */
	 unsigned short fr_reserved[4];		/* for future use */
	 unsigned short fr_strt_gate;	/* starting gate this frame * 1-1024 */
	 unsigned short fr_end_gate;	/* ending gate this frame */
     } FR_HEADER;		/* frame header */

/* house keeping header */

     typedef struct {
	 /* 1 */ unsigned short log_rec_num;
	 /* logical record # this beam  */
	 /* 2 */ unsigned short field_tape_seq;
	 /* field tape sequence # =0 */
	 /* 3 */ unsigned short rec_type;
	 /* record type 0 = data */
	 /* 4 */ unsigned short year;
	 /* last two digits */
/* 5 */ unsigned short month;
/* 6 */ unsigned short day;
/* 7 */ unsigned short hour;
/* 8 */ unsigned short minute;
/* 9 */ unsigned short second;
	 /* 10 */ unsigned short azimuth;
	 /* degrees * CF */
	 /* 11 */ unsigned short elevation;
	 /* degrees * CF */
	 /* 12 */ unsigned short rhozero1;
	 /* range to leading edge of */
	 /* 13 */ unsigned short rhozero2;
	 /* 1st gate = rhozero1 */
	 /* + rhozero2 / 1000 (in km) */
	 /* 14 */ unsigned short gate_spacing;
	 /* gate spacing (m) = */
	 /* 225 for Batch or Doppler, */
	 /* 450 for Reflectivity mode */
	 /* 15 */ unsigned short gates_per_beam;
	 /* gates per beam = 1024 */
/*16 */ unsigned short samples_per_beam;
	 /* 17 */ unsigned short test_pulse_level;
	 /* 0 */
/*18 */ unsigned short avg_xmit_pwr;
	 /* 19 */ unsigned short pulse_width;
	 /* 1.67 us */
	 /* 20 */ unsigned short prfx10;
	 /* PRF (Hz * 10), typ. 1235 Hz */
	 /* 21 */ unsigned short wavelength;
	 /* wavelength (cm * 100) * = 10.44 cm */
	 /* 22 */ unsigned short seq_sweep;
	 /* running counter of elevation * scans since last start of *
	    operations  */
	 /* 23 */ unsigned short sweep_index;
	 /* identifies the sweep (scan) * in volume scan (#'s 1 - 16) */
/*24 */ unsigned short unused1[2];
	 /* 26 */ unsigned short scan_mode;
	 /* 8 = Surveillance */
	 /* 27 */ unsigned short cw_az_lim;
	 /* azim angle of first dwell */
	 /* 28 */ unsigned short ccw_az_lim;
	 /* azim angle of last dwell */
	 /* 29 */ unsigned short up_elev_lim;
	 /* 0 */
	 /* 30 */ unsigned short lo_elev_lim;
	 /* 0 */
	 /* 31 */ unsigned short target_elev;
	 /* the elevation angle from the * scan strategy table */
	 /* 32 */ unsigned short sig_source;
	 /* 0 = radar */
/*33 */ unsigned short coupler_loss;
/*34 */ unsigned short tp_strt;
/*35 */ unsigned short tp_width;
/*36 */ unsigned short pri_co_bl;
/*37 */ unsigned short scnd_co_bl;
/*38 */ unsigned short tp_atten;
/*39 */ unsigned short sys_gain;
/*40 */ unsigned short fix_tape;
/*41 */ unsigned short tp_freq_off;
/*42 */ unsigned short log_bw;
/*43 */ unsigned short lin_bw;
/*44 */ unsigned short ant_bw;
/*45 */ unsigned short ant_scan_rate;
/*46 */ unsigned short unused2[2];
	 /* 48 */ unsigned short vol_count;
	 /* running count of full or *  partial volume scans since * last
	    start of operations */
/*49 */ unsigned short unused3;
	 /* 50 */ unsigned short polarization;
	 /* 0 = horizontal */
/*51 */ unsigned short prf1;
/*52 */ unsigned short prf2;
/*53 */ unsigned short prf3;
/*54 */ unsigned short prf4;
/*55 */ unsigned short prf5;
/*56 */ unsigned short unused4;
	 /* 57 */ unsigned short rec_cnt_of;
	 /* record count overflow */
/*58 */ unsigned short altitude;
/*59 */ unsigned short latitude;
/*60 */ unsigned short longitude;
	 /* 61 */ unsigned short transit;
	 /* 0 = in a scan */
	 /* 62 */ unsigned short ds_id;
	 /* -1 */
	 /* 63 */ unsigned short rs_id;
	 /* 0x4d48 - 'MH' */
/*64 */ unsigned short proj_num;
	 /* 65 */ unsigned short sz_hsk;
	 /* # of words of housekeeping * = 100 */
/*66 */ unsigned short sz_cur_log;
/*67 */ unsigned short num_log_rcd;
/*68 */ unsigned short parm_per_gate;
/*69 */ unsigned short parm1_desc;
/*70 */ unsigned short parm2_desc;
/*71 */ unsigned short parm3_desc;
/*72 */ unsigned short parm4_desc;
/*73 */ unsigned short parm5_desc;
/*74 */ unsigned short parm6_desc;
/*75 */ unsigned short tp_max;
/*76 */ unsigned short tp_min;
/*77 */ unsigned short tp_step;
/*78 */ unsigned short vol_scan_prg;
	 /* 79 */ unsigned short parm1_scale;
	 /* scale * 100 for Z */
	 /* 80 */ short parm1_bias;
	 /* bias  * 100 for Z */
	 /* 81 */ unsigned short parm2_scale;
	 /* scale * 100 for V */
	 /* 82 */ short parm2_bias;
	 /* bias  * 100 for V */
	 /* 83 */ unsigned short parm3_scale;
	 /* scale * 100 for W */
	 /* 84 */ short parm3_bias;
	 /* bias  * 100 for W */
/*85 */ unsigned short parm4_scale;
/*86 */ short parm4_bias;
/*87 */ unsigned short parm5_scale;
/*88 */ short parm5_bias;
/*89 */ unsigned short parm6_scale;
/*90 */ short parm6_bias;
/*91 */ unsigned short rnoise_bdcal;
/*92 */ unsigned short rsolar;
/*93 */ unsigned short rgcc;
/*94 */ unsigned short rvtime;
/*95 */ unsigned short rcrec;
/*96 */ unsigned short unused5[4];
	 /* 100 */ unsigned short live_or_sim;
	 /* LIVE or SIM */
     } HSK_HEADER;

/* MH - for each gate, 4 bytes of base data */

     typedef struct {
	 unsigned char reflect;
	 unsigned char velocity;
	 unsigned char width;
	 unsigned char flags;
     } MH_GATE_DATA;

/* CP3 - for each gate, 4 bytes of base data */

     typedef struct {
	 unsigned char power;
	 unsigned char ncp;
	 unsigned char velocity;
	 unsigned char width;
	 unsigned char reflect;
	 unsigned char snr;
     } CP3_GATE_DATA;

/*
 * DON'T use sizeof on these structures! 
 * these are only to be overlaid on existing buffers
 */
     typedef struct {
	 char spacer[2];
	 unsigned short fr_fmt;	/* FIRST_PKT or CONTINUE_PKT */
	 unsigned long fr_seq;	/* sequence # */
	 unsigned short fr_frames_per_rad;	/* frames required per radial 

						 */
	 unsigned short fr_frame_num_in_rad;	/* frame # in radial seq */
	 unsigned short fr_reserved[2];		/* for future use */
	 unsigned short fr_strt_gate;	/* starting gate this frame */
	 unsigned short fr_gates;	/* number of gates this frame */
     } Packet_hdr;

     typedef struct {
	 unsigned short fr_strt_gate;	/* starting gate this frame */
	 unsigned short fr_gates;	/* number of gates this frame */
     } Subseq_beam_hdr;

     typedef struct {
	 Packet_hdr frh;
	 Beam_hdr beam_hdr;
	 MH_GATE_DATA gates[1];	/* place holder for variable size array */
     } First_ll_pkt;

     typedef struct {
	 Packet_hdr frh;
	 Subseq_beam_hdr beam_hdr;
	 MH_GATE_DATA gates[1];	/* place holder for variable size array */
     } Subseq_ll_pkts;

     typedef struct {
	 FR_HEADER frh;
	 HSK_HEADER hsk;
	 MH_GATE_DATA gates[1];	/* place holder for variable * size array */
     } FIRST_FRAME;

     typedef struct {
	 FR_HEADER frh;
	 MH_GATE_DATA gates[1];	/* place holder for variable * size array */
     } CONT_FRAME;

     typedef struct {
	 Packet_hdr pkt;
	 Beam_hdr beam;
     } LL_beam_hdr;

     typedef struct {
	 struct sockaddr sock_addr;
	 LL_beam_hdr header;
	 unsigned char data[4 * 1500];
	 struct strbuf ctlptr, dataptr;
     } LL_beam_rec;

#endif
