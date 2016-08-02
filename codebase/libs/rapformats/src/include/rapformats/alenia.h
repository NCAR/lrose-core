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
#ifndef alenia_h
#define alenia_h

#ifdef __cplusplus
extern "C" {
#endif

  /**********************************************************************
   * alenia.h
   *
   * structs and defines for alenia data
   *
   * Mike Dixon, RAP, NCAR, Boulder, CO, Oct 1995
   *
   */

#include <dataport/port_types.h>
#include <rapformats/gate_data.h>

#define N_ALENIA_FIELDS 4
#define ALENIA_ANGLE_CONV 0.043945313 

#define ALENIA_Z_RANGE_MED 0
#define ALENIA_Z_RANGE_HIGH 1
#define ALENIA_Z_RANGE_LOW 2

#define ALENIA_DBZ_SCALE 0.3125
#define ALENIA_DBZ_BIAS_MED -20.0
#define ALENIA_DBZ_BIAS_HIGH -10.0
#define ALENIA_DBZ_BIAS_LOW -30.0

#define ALENIA_ZDR_SCALE 0.0625
#define ALENIA_ZDR_BIAS -6.0

#define ALENIA_VEL_SCALE 0.125
#define ALENIA_VEL_BIAS -16.0

#define ALENIA_VEL_SCALE_DUAL 0.3828
#define ALENIA_VEL_BIAS_DUAL -49.0

#define ALENIA_SPW_SCALE 0.0391
#define ALENIA_SPW_BIAS 0.0

#define ALENIA_UDP_LEN   1024
#define ALENIA_UDP_START 117
#define ALENIA_UDP_DATA  118
#define ALENIA_UDP_STOP  119
#define ALENIA_UDP_ABORT 121

  typedef struct radardata {
    ui08 spare1[3];         /* 0 - 2 */
    ui08 azim_l;            /* 3 */
    ui08 azim_h;            /* 4 */
    ui08 elev_l;            /* 5 */
    ui08 elev_h;            /* 6 */
    ui08 init_raw;          /* 7 */
    ui08 scan_mode;         /* 8 */
    ui08 parameters;        /* 9 */
    ui08 num_bin_l;         /* 10 */
    ui08 avarie;            /* 11 */
    ui08 clutter;           /* 12 */
    ui08 num_pulses_l;      /* 13 */
    ui08 num_pulses_h;      /* 14 */
    ui08 real_power;        /* 15 */
    ui08 eval_power;        /* 16 */
    ui08 sec_hdrths;        /* 17 */
    ui08 sec;               /* 18 */
    ui08 min;               /* 19 */
    ui08 hour;              /* 20 */
    ui08 day_of_week;       /* 21 */
    ui08 day;               /* 22 */
    ui08 month;             /* 23 */
    ui08 year;              /* 24 */
    ui08 spare2;            /* 25 */
    ui08 ang_offset_l;      /* 26 */
    ui08 ang_offset_h;      /* 27 */
    ui08 spare3[12];        /* 28 - 39 */
  } alenia_header_t;

  typedef struct {
  
    double azimuth;		/* deg */
    double elevation;		/* deg */
    double elev_target;		/* deg */
    double gate_spacing;	/* km */
    double start_range;		/* km */
    double pulse_width;		/* us */

    double scale[N_ALENIA_FIELDS];
    double bias[N_ALENIA_FIELDS];

    int dbz_avail;
    int zdr_avail;
    int vel_avail;
    int width_avail;

    int dual_prf;
    int freq_num;
    int clutter_map;
    int rejected_by_filter;
    int filter_num;
    int clutter_filter;
    int clutter_correction;

    int scan_mode;		/* GATE_DATA_SURVEILLANCE_MODE or
				   GATE_DATE_SECTOR_MODE or
				   GATE_DATE_RHI_MODE or
				   GATE_DATA_UNKNOWN_MODE */

    si32 time;
    si32 prf;			/* /s */
    si32 tilt_num;
    si32 vol_num;
    si32 nfields;
    si32 ngates;
    si32 npulses;
    si32 ndata;

  } alenia_params_t;
        
#ifdef __cplusplus
}
#endif

#endif
