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
/******************************************************************
    the header file defining the volume data structure
    File: v_data.h
    Notes: .
******************************************************************/
#ifdef __cplusplus
 extern "C" {
#endif

#define     PROJ_CARTESIAN       -1
#define     PROJ_LAT_LONG_GRID    0
#define     PROJ_ARTCC            1
#define     PROJ_STEREOGRAPHIC    2
#define     PROJ_LAMBERT_CONF     3
#define     PROJ_MERCATOR         4
#define     PROJ_POLAR_STEREO     5
#define     PROJ_POLAR_ST_ELLIP   6
#define     PROJ_CYL_EQUIDIST     7
#define     PROJ_FLAT             8    /* Equivilent to PROJ_CARTESIAN */


/* shared memory segments */
struct volume_header {        /* communication area for output */
    char data_id[16];         /* Usually "gint" */
    char flag0;               /* 1: a volume dat received */
    char flag1;               /* flags - not used yet */
    char snr_clip_value;      /* the threshold  */
    char encode;              /* encoding index. See the defines later */
    long v_cnt;
    long xstt;                /* the coordinate system: origin is the radar
                               location; x: points to east; y: south; z: upward,
                               unit: in meters */
    long ystt;                /* ?stt: starting coordinates of the volume - meters */
    long xss;                 /* ?ss: step size */
    long yss;

    long nx;                  /* n?: number of grid points */
    long ny;
    long nz;
    long n_fields;            /* Number of fields in the volume */
    long l_time;              /* Ending time of last volume - Start time of this one */
    long time;                /* Ending time of this volume */
    long ray_life_time;       /* life time of the data in the interpolation */
    char file_name[80];       /* name of this file */
    long name_time;           /* unix time for this volume */
    long proj_type;           /* Grid Projection Type (CART=  -1 / LATLON = 0) */
    long origin_lat;          /* Grid Projection Origin - Latitude * 1,000,000 */
    long origin_lon;          /* Grid Projection Origin - Longitude * 1,000,000  */
    long proj_param[4];       /* Paramaters used by the various projections */
    char note[260];           /* This is the size of Beam_struct */
};
typedef struct volume_header Volume_header;

struct field_infor {
    long scale;               /* shifted int (<<16) */
    long offset;              /* shifted int (<<16) */
    char field_names[8];
    char unit_names[16];
    unsigned char bad_data_value;
    char res[3];
};
typedef struct field_infor Field_infor;

struct altitude_infor {
    long z;                   /* z coordinate of the cappis */
};
typedef struct altitude_infor Altitude_infor;

struct location_infor {      /* for each cappi of each field */
    long off;                /* offset in the data area */
    long len;                /* length in bytes */
};
typedef struct location_infor Location_infor;

/* the file header contains the following:
    a volume header;
    n_fields Field_infor;
    nz Altitude_infor;
    n_fields*nz Location_infor;
    for field f and cappi c the index is: c*n_fields+f;
*/

    
/* the encoding */
#define NOT_ENCODED 0
#define RL7_ENCODED 1
#define RL8_ENCODED 2

#ifdef __cplusplus
}
#endif
