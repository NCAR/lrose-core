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
/****************************************************************************
 * bprp.h : header for Enterprize processor radar format
 *
 ****************************************************************************/


#ifndef _bprp_h
#define _bprp_h

#include <dataport/port_types.h>

/*
 * max number of fields
 */

#define BPRP_MAX_NFIELDS 1

/*
 * scan modes
 */

#define BPRP_SURVEILLANCE_MODE 8
#define BPRP_RHI_MODE 0
#define BPRP_SECTOR_MODE 0

/*
 * packet header code
 */

#define BPRP_BEAM_PACKET_CODE 0x6006

/*
 * radar params
 */

#define BPRP_GATES_PER_BEAM 224         
#define BPRP_GATE_SPACING 0.600            
#define BPRP_ALTITUDE 1.687              
#define BPRP_LATITUDE -28.2500              
#define BPRP_LONGITUDE 28.3333              
#define BPRP_START_RANGE 14.400               
#define BPRP_BEAM_WIDTH 1.0                  
#define BPRP_SAMPLES_PER_BEAM 8                    
#define BPRP_PULSE_WIDTH 2.0                       
#define BPRP_PRF_NOMINAL 250                       
#define BPRP_WAVELENGTH 5.0                       
#define BPRP_NIFF 8

/*
 * scale and bias for DBZ field
 */

#define BPRP_DBZ_SCALE 0.5
#define BPRP_DBZ_BIAS -30.0

/*
 * distance scaling
 */

#define	NM_PER_US 0.0809319
#define KM_PER_US 0.1500000

/*
 * codes
 */

#define RADAR_VERSION 0x0100
#define RADAR_MAGIK 0x9d4f28cb

#define	RADAR_NULL 0
#define	RADAR_STATE 1
#define	RADAR_DATA 2
#define	RADAR_INFO 3
#define	RADAR_ERROR 4
#define	RADAR_CONTROL 5

#define	RADAR_ENONE 0
#define	RADAR_EBADREQ 1
#define	RADAR_ENODATA 2

/*
 * the following are used for reading and converting bprp data
 */

typedef struct {
  ui32		length;
  ui32		magik;
  ui32		reference;
  ui32		version;
  ui32		request;
  ui08		data [12];
} bprp_request_t;

#define BPRP_NBYTES_UI32_RESPONSE 20

typedef struct {
  ui32		length;
  ui32		magik;
  ui32		reference;
  ui32		version;
  ui32		response;
  ui08		filler [12];
  /* End of header */
  ui08		data [512];
} bprp_response_t;

typedef struct 	{

  ui16 range;
  ui16 code;

} bprp_iff_t;

typedef struct {

  ui16 date;			/* Year * 512 + Julian day */
  ui16 hour;
  ui16 min;			/* Minute * 60 + seconds */
  ui16 start_azimuth;		/* Code for start of azimuth */
  ui16 end_azimuth;		/* Code for end of azimuth */
  ui16 raycount;		/* Raycount * elstep * 256 - set to zero
				 * for ray of elevation */
  ui16 azimuth;			/* Code for azimuth */
  ui16 elevation;		/* Code for elevation */
  ui16 mds;			/* MDS of system */
  ui16 noise;
  ui16 viphi;			/* VIP high level, in VIP units */
  ui16 viplo;			/* VIP low level, in VIP units */
  si16 phi;			/* VIP high level in dbm*32 */
  si16 plo;			/* VIP low level in dbm*32 */
  si16 xmt;			/* Transmitter power * 32 + site */
  ui16 site_blk;                /* Site block : skip + bin_width * 256 +
				 * ints * 4096 */

} bprp_beam_hdr_t;


typedef struct {

  bprp_beam_hdr_t hdr;
  ui16 vip[BPRP_GATES_PER_BEAM];
  bprp_iff_t iff[BPRP_NIFF];

} bprp_beam_t;

/*
 * the following are used by rdata_to_shmem
 */

typedef struct {

  double scale;
  double bias;

} bprp_scale_t;

typedef struct {

  double start_azimuth;
  double end_azimuth;
  double azimuth;
  double elevation;
  double target_elevation;
  double mds;
  double noise;
  double viphi;
  double viplo;
  double phi;
  double plo;
  double xmt;
  double rec_slope;

  si32 year;
  si32 month;
  si32 day;
  si32 hour;
  si32 min;   
  si32 sec;   
  si32 scan_mode;   
  si32 vol_num;   
  si32 tilt_num;   
  si32 raycount;
  si32 azcwlim;
  si32 azccwlim;
  si32 site;
  si32 skip;
  si32 binwidth;
  si32 ints;

  bprp_scale_t lscale[BPRP_MAX_NFIELDS];

} bprp_params_t;

typedef struct {

  ui08  dbz[BPRP_GATES_PER_BEAM];
  bprp_iff_t iff[BPRP_NIFF];

} bprp_data_t;

typedef struct {

  ui16 id; /* always 0 */
  ui16 count; /* always 0 */
  ui16 ad[4];
  ui08 flags[4];
  ui16 non_rcc0;
  ui16 non_rcc1;
  ui16 non_rcc2;
  ui16 non_rcc3;

} bprp_status_t;
  
#endif



