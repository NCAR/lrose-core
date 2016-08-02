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
* MDVT_DATA.H : This file describes the format of data intended for the MDVT
*   tool
*                   
*************************************************************************/
#ifdef __cplusplus
 extern "C" {
#endif

# ifndef    MDVT_DISPLAY_DATA_H
# define    MDVT_DISPLAY_DATA_H


/************************************************************************
*                    
* MDVT_DATA.H : This file describes the format of data intended for the MDVT
*   tool
                        10 February 1993
*************************************************************************/

/*
 * Define constant values for file fields.
 */

#define     DATA_MEASURED         0
#define     DATA_EXTRAPOLATED     1
#define     DATA_FORECAST         2
#define     DATA_SYNTHESIS        3

#define     GRID_DOT              0
#define     GRID_CROSS            1

#define     ORIENT_NS_WE_LAT      0
#define     ORIENT_NS_EW_LAT      4
#define     ORIENT_SN_WE_LAT      2
#define     ORIENT_SN_EW_LAT      6
#define     ORIENT_NS_WE_LON      1
#define     ORIENT_NS_EW_LON      5
#define     ORIENT_SN_WE_LON      3
#define     ORIENT_SN_EW_LON      7

#define     PROJ_LAT_LONG_GRID    0
#define     PROJ_ARTCC            1
#define     PROJ_STEREOGRAPHIC    2
#define     PROJ_LAMBERT_CONF     3
#define     PROJ_MERCATOR         4
#define     PROJ_POLAR_STEREO     5
#define     PROJ_POLAR_ST_ELLIP   6
#define     PROJ_CYL_EQUIDIST     7
#define     PROJ_FLAT             8

#define     ORDER_XYZ             0
#define     ORDER_YXZ             1
#define     ORDER_XZY             2
#define     ORDER_YZX             3
#define     ORDER_ZXY             4
#define     ORDER_ZYX             5

#define     VERT_TYPE_SURFACE     1  
#define     VERT_TYPE_SIGMA_P     2
#define     VERT_TYPE_PRESSURE    3    /* Units = mb  */
#define     VERT_TYPE_Z           4    /* Constant altitude; units = Km MSL */
#define     VERT_TYPE_SIGMA_Z     5
#define     VERT_TYPE_ETA         6
#define     VERT_TYPE_THETA       7    /* Isentropic surface, units = Kelvin */
#define     VERT_TYPE_MIXED       8    /* Any hybrid grid */
#define     VERT_TYPE_OTHER       9    /* Any other undefined */

#define CHAR             1        /* For data typing */
#define SHORT            2
#define INT              3
#define LONG             3
#define FLOAT            4
#define DOUBLE           5


/*
 * Define the constants used in the structures below.
 */

#define     MAX_PROJ_PARAMS       8
#define     MAX_V_LEVELS        192
#define     INFO_LEN            512
#define     NAME_LEN            128
#define     LABEL_LEN            64
#define     UNITS_LEN            16


/*
 * This is the Master header in the file.  Each data file contains one
 * master header at the beginning of the file. This header is designed to be 2048 bytes
 * in total length, including the fortran record length pads. Adjust the
 * spare space if other data elements are added. Total size for this structure is
 * 3 Kbytes.
 *
 * The basic FORTRAN structure of this record is:
 *  INTEGER DATASET_INTS(62)
 *  INTEGER DATASET_VERT_TYPE(MAX_V_LEVELS)
 *  REAL    DATASET_FLOATS(64)
 *  REAL    VERT_LEVELS(MAX_V_LEVELS)
 *  CHARACTER*512 DATASET_INFO
 *  CHARACTER*128 DATASET_CHARS(4)
 */

typedef struct
{
    int      record_len1;               /* fortran record length field */

    long     n_fields;                  /* number of fields */
    long     num_data_times;            /* Number of data times in "data set" if known */
    long     data_number;               /* index of data within "data set" if known */

    long     grid_order_indices;        /* ORDER_XYZ, etc. */
    long     grid_order_direction ;      /* ORIENT_NS_WE_LAT, etc. ? */
    
    long     time_of_generation;        /* Data generated at time */
    long     time_delta;                /* Difference from generation time this data repesents */
    long     time_begin;
    long     time_end;
    long     time_centroid;
    long     time_expire;

    long     max_nx;
    long     max_ny;
    long     max_nz;

    long     data_collection_type;      /* Measured, Modeled, Calculated, etc */
    long     proj_type;                 /* PROJ_LAT_LONG_GRID, etc. */
    long     vlevel_type;               /* Native Vertical coordinate type */
    long     unused_long[45];           /* To fill out record to 62 longs */

    long     v_level_type[MAX_V_LEVELS]; /* Enumerated type from above list */

    float    proj_origin_lat;
    float    proj_origin_long;
    float    proj_param[MAX_PROJ_PARAMS]; 

    float    grid_dx;                  /*  Defauly grid spacing in Km */
    float    grid_dy;
    float    grid_minx;                /* Starting point in KM of data within the projection */
    float    grid_miny;
    float    grid_minz;
    float    grid_maxx;                /* Ending point in KM of data within the projection */
    float    grid_maxy;
    float    grid_maxz;
     
    float    bad_data_value;
    float    missing_data_value;

    float    vert_reference;          /* Vertical coordinate reference value */

    float    unused_float[43];          /* to  fill out record to 64 floats */

    float    vlevel_params[MAX_V_LEVELS];   /* MAX_V_LEVELS vertical levels */

    char     data_info[INFO_LEN];           /* 512 bytes of data set info */
    char     data_set_name[NAME_LEN];       /* 128 bytes: Data set ID */
    char     unused_char[3][NAME_LEN];      /* to fill out 1024 byte character area */

    int      record_len2;                 /* fortran record length field */
}  master_header_t;


/*
 * THis is the Field Header.  The data file will contain one field
 * header for each field in the file.  All of the field headers follow the
 * master header in the file. This header is designed to be 1024 bytes
 * in total length, including the fortran record length pads. Adjust the
 * spare space if other data elements are added.
 * Total size for this structure is 2 Kbytes.
 *
 * The basic FORTRAN structure of this record is:
 *  INTEGER FIELD_INTS(30)
 *  INTEGER FIELD_VERT_TYPE(MAX_V_LEVELS)
 *  REAL    FIELD_REALS(64)
 *  REAL    VERT_LEVELS(MAX_V_LEVELS)
 *  CHARACTER*64 LONG_FIELD_NAME
 *  CHARACTER*16 FIELD_CHARS(4)
 */

typedef struct
{
    int        record_len1;           /* fortran record length field */
     
    long       nx;    /* Number of points in each direction */
    long       ny;
    long       nz;

    long       field_code;       /* GRIB FIELD CODE */
    long       proj_type;        /* Projection Type */
    long       encoding_type;    /* type of data encoding */
    long       num_pts;          /* total points in grid */
    long       vlevel_type;      /* Native Vertical coordinate type */
    long       file_index;       /* fseek offset in file for field data */
    long       centering_flag;   /* GRID_DOT or GRID_CROSS */

    long      time_begin;
    long      time_end;
    long      time_centroid;
    long      time_expire;

    long      unused_long[16];  /* to fill out array to 30 longs */

    long     v_level_type[MAX_V_LEVELS]; /* Enumerated type from above list */

    float    scale;    /* Values are to be scaled by these quantities */
    float    bias;

    float    proj_origin_lat;
    float    proj_origin_long;
    float    proj_param[MAX_PROJ_PARAMS];

    float    grid_dx;    /* Km */
    float    grid_dy;
    float    grid_minx;  /* Start of grid in projection */
    float    grid_miny;
     
    float    bad_data_value;
    float    missing_data_value;

    float    vert_reference;          /* Vertical coordinate reference value */

    float    unused_float[45];     /* to fill out array to 64 floats */

    float    vlevel_params[MAX_V_LEVELS];  /* 128 vertical levels */
     
    char      field_name_long[LABEL_LEN];  /* Long Names for margins, etc: 64 bytes */
    char      field_name[UNITS_LEN];       /* Short name for controls: 16 bytes*/
    char      units[UNITS_LEN];            /* Units label: 16 bytes */
    char      unused_char[2][UNITS_LEN];   /* Spare 32 bytes */

    int        record_len2;                /* fortran record length field */
} field_header_t;


/* GENERAL FILE STRUCTURE LOOKS LIKE :
 * After all of the field headers, the MDVT display data file will contain
 * all of the data for all of the fields. 
 * Thus, the MDVT display data file will look like the following:
 *
 *         master header
 *         field 1 header
 *         field 2 header
 *              .
 *              .
 *              .
 *         field n header
 *         field 1 data
 *         field 2 data
 *              .
 *              .
 *              .
 *         field n data
 *
 * (Note that the above shows the field data being in the same order as the
 *  field headers.  This is not necessary.  The only requirement is that the
 *  data for a single field be contiguous in the file and that the
 *  file_index in the field header polong to the appropriate data location.)
 */


typedef struct
{
    master_header_t    master_hdr;
    field_header_t     *field_hdrs;
    float              *data;
} mdvt_display_data_t;



/*
 * Define the structure for the field configuration file information.
 */

typedef struct
{
    long         code;
    char        name[NAME_LEN];
    char        units[UNITS_LEN];
    char        colorscale[NAME_LEN];
    double      scale;
    double      bias;
    double      low;
    double      high;
    double      interval;
    double      thresh1;
    double      thresh2;
} field_config_t;
# endif     /* MDVT_DISPLAY_DATA_H */


#ifdef __cplusplus
}
#endif
