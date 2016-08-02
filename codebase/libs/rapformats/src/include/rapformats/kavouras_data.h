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
*                    
* KAVOURAS_DATA.H : This file describes the format of Kavouras data 
*   tool
*                   
*                        16 February 1995
*************************************************************************/

#ifndef    KAVOURAS_DATA_H
#define    KAVOURAS_DATA_H

/*
 * Define constant values for file fields.
 */

#define     NUM_6H_COLS           640
#define     NUM_6H_ROWS           480
#define     COLOR_BAR_POS         520

/* 6 Bit low resolution NIDS product file definitions */
#define     NUM_6L_COLS           320
#define     NUM_6L_ROWS           240
#define     COLOR_BAR_POS_6L      260


/* NEXRAD Operational modes (Scanning strategy) */
#define     MAINT_MODE            0
#define     CLEAR_AIR_MODE        1
#define     PRECIP_MODE           2

/* DCMP6H_HEADER: This header is designed to be 128 bytes long */
/* These files contain one "high resolution" 6 bit image, in compressed */
/* form, followed by header information. */
typedef struct
{
    short    format_spec;               /* Format specifier for header block - always 0xFFFF */
    short    source_id;                 /* Site ID 1-999 */
    unsigned short      radar_lat_lo;   /* Radar Latitude - Deg * 1000 */
    short    radar_lat_hi;              /* Radar Latitude - Deg * 1000 */
    unsigned short      radar_lon_lo;   /* Radar Longitude - Deg * 1000 */
    short    radar_lon_hi;                 /* Radar Longitude - Deg * 1000 */
    short    radar_alt;                 /* Radar Altitude (feet) */
    short    product_code;              /* NEXRAD Product Code 16 - 131 */
    short    operational_mode;          /* 0= Maint, 1 = Clear Air, 2 = Precip */
    short    vol_cov_pat;               /* Volume Coverage Pattern -= RDA Scan strategy 1-767 */
    short    seq_no;                    /* N/A */
    short    scan_number;               /* Volume scan number 1 to 80 */
    short    scan_date;                 /* Days since Jan 1 1970 + 1 (1/1/70 = 1) */
    unsigned short  scan_time_lo;       /* Seconds in day (Unix time_t = (scan_date -1) * 86400 + scan_time) */
    short    scan_time_hi;              /* Seconds in day (Unix time_t = (scan_date -1) * 86400 + scan_time) */
    short    generate_date;             /* Product generation date,time - as above*/
    unsigned short   generate_time_lo;  /* */
    short    generate_time_hi;          /* */
    short    param1;                    /* Product dependant parameter 1 */
    short    param2;                    /* Product dependant parameter 2 */
    short    elev_number;               /* Elevation number of radar scan 1-20 */
    short    param3;                    /* Product dependant parameter 3 */
    short    data_level[16];            /* Data threshold values for each field  */
    short    param4;                    /* Product dependant parameter 4 */
    short    param5;                    /* Product dependant parameter 5 */
    short    param6;                    /* Product dependant parameter 6 */
    short    param7;                    /* Product dependant parameter 7 */
    short    param8;                    /* Product dependant parameter 8 */
    short    param9;                    /* Product dependant parameter 9 */
    short    param10;                   /* Product dependant parameter 10 */
/*  int      tab_data_len;*/              /* Length of tabular data  */
/*  int      reserved;    */             /* */
    short    level_pix_cnt[16];         /* Number of Pixels at data level 1 in image */

}  dcmp6h_header_t;

# endif     /* KAVOURAS_DATA_H */

