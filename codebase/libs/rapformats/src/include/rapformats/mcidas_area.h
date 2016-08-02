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
 * mcidas_area.h - Header file for the McIDAS AREA module of the DIDSS
 * library.
 *
 * Nancy Rehak
 * April 1997
 *                   
 *************************************************************************/

#include <dataport/port_types.h>

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef MCIDAS_AREA_H
#define MCIDAS_AREA_H


/* In McIDAS, images are stored in binary files called "areas".  Each
 * area file is a collection of information that defines the image and
 * its associated ancillary data.
 *
 * Area files consist of these six blocks.  Not all blocks are required
 * in all area files:
 *     - directory block (AREA_dir_block_t)
 *     - navigation (NAV) block
 *          (depends on first 4 letters in NAV block:
 *                 AREA_radr_nav_block_t,
 *                 AREA_lamb_nav_block_t)
 *     - calibration (CAL) block (not yet defined here)
 *     - supplemental block (not yet defined here)
 *     - data block (not yet defined here)
 *     - comment (AUDIT) block (not yet defined here)
 */

/*
 * Value of type field in directory block, used to determine if
 * swapping is needed
 */

#define AREA_VERSION_NUMBER     4

/*
 * Lengths of data fields in the file structures
 */

#define AREA_SENSOR_DATA_LEN    4
#define AREA_MEMO_LEN          32
#define AREA_PDL_LEN            8
#define AREA_SOURCE_TYPE_LEN    4
#define AREA_CALIB_TYPE_LEN     4
#define AREA_UNUSED3_LEN        6

/*
 * Legal values for satellite_id field
 */

#define AREA_SAT_ID_NON_IMAGE_DERIVED_DATA       0
#define AREA_SAT_ID_GRAPHICS                     2
#define AREA_SAT_ID_MDR_RADAR                    3
#define AREA_SAT_ID_METEOSAT_VISIBLE             4
#define AREA_SAT_ID_METEOSAT_INFRARED            5
#define AREA_SAT_ID_METEOSAT_WATER_VAPOR         6
#define AREA_SAT_ID_RADAR                        7
#define AREA_SAT_ID_MISC_AIRCRAFT_DATA           8
#define AREA_SAT_ID_RAW_METEOSAT                 9
#define AREA_SAT_ID_GMS_VISIBLE                 12
#define AREA_SAT_ID_GMS_INFRARED                13
#define AREA_SAT_ID_ATS_6_VISIBLE               14
#define AREA_SAT_ID_ATS_6_INFRARED              15
#define AREA_SAT_ID_SMS_1_VISIBLE               16
#define AREA_SAT_ID_SMS_1_INFRARED              17
#define AREA_SAT_ID_SMS_2_VISIBLE               18
#define AREA_SAT_ID_SMS_2_INFRARED              19
#define AREA_SAT_ID_GOES_1_VISIBLE              20
#define AREA_SAT_ID_GOES_1_INFRARED             21
#define AREA_SAT_ID_GOES_2_VISIBLE              22
#define AREA_SAT_ID_GOES_2_INFRARED             23
#define AREA_SAT_ID_GOES_3_VISIBLE              24
#define AREA_SAT_ID_GOES_3_INFRARED             25
#define AREA_SAT_ID_GOES_4_VISIBLE              26
#define AREA_SAT_ID_GOES_4_INFRARED_WATER_VAPOR 27
#define AREA_SAT_ID_GOES_5_VISIBLE              28
#define AREA_SAT_ID_GOES_5_INFRARED_WATER_VAPOR 29
#define AREA_SAT_ID_GOES_6_VISIBLE              30
#define AREA_SAT_ID_GOES_6_INFRARED             31
#define AREA_SAT_ID_GOES_7_VISIBLE              32
#define AREA_SAT_ID_GOES_7_INFRARED             33
#define AREA_SAT_ID_NOAA_1                      36
#define AREA_SAT_ID_NOAA_2                      37
#define AREA_SAT_ID_NOAA_3                      38
#define AREA_SAT_ID_NOAA_4                      39
#define AREA_SAT_ID_NOAA_5                      40
#define AREA_SAT_ID_TIROS_N                     41
#define AREA_SAT_ID_NOAA_6                      42
#define AREA_SAT_ID_NOAA_7                      43
#define AREA_SAT_ID_NOAA_8                      44
#define AREA_SAT_ID_NOAA_9                      45
#define AREA_SAT_ID_MARINER_1                   46
#define AREA_SAT_ID_MARINER_2                   47
#define AREA_SAT_ID_MARINER_3                   48
#define AREA_SAT_ID_MARINER_4                   49
#define AREA_SAT_ID_HUBBLE_SPACE_TELESCOPE      50
#define AREA_SAT_ID_METEOSAT_3                  54
#define AREA_SAT_ID_METEOSAT_4                  55
#define AREA_SAT_ID_METEOSAT_5                  56
#define AREA_SAT_ID_METEOSAT_6                  57
#define AREA_SAT_ID_NOAA_10                     60
#define AREA_SAT_ID_NOAA_11                     61
#define AREA_SAT_ID_NOAA_12                     62
#define AREA_SAT_ID_NOAA_13                     63
#define AREA_SAT_ID_NOAA_14                     64
#define AREA_SAT_ID_GOES_IMAGER                 70
#define AREA_SAT_ID_GOES_SOUNDER                71
#define AREA_SAT_ID_ERBE                        80
#define AREA_SAT_ID_DMSP_F_8                    87
#define AREA_SAT_ID_DMSP_F_9                    88
#define AREA_SAT_ID_DMSP_F_10                   89
#define AREA_SAT_ID_DMSP_F_11                   90
#define AREA_SAT_ID_DMSP_F_12                   91
#define AREA_SAT_ID_FY_1B                       95
#define AREA_SAT_ID_FY_1C                       96
#define AREA_SAT_ID_FY_1D                       97


typedef struct
{
  si32 rel_position;    /*   1 - relative position of the image object in    */
                        /*       the ADDE dataset                            */
  si32 type;            /*   2 - type = 4, area version number               */
  si32 satellite_id;    /*   3 - SSEC sensor source number; see defines      */
                        /*       above                                       */
  si32 image_date;      /*   4 - year and Julian date of the image, YYDDD    */
  si32 image_time;      /*   5 - time of the image, HHMMSS                   */
  si32 y_coord;         /*   6 - Y-COOR, upper-left line in the satellite    */
                        /*       coordinates                                 */
  si32 x_coord;         /*   7 - X-COOR, upper_left element in the satellite */
                        /*       coordinates                                 */
  si32 unused1;         /*   8 - reserved                                    */
  si32 y_size;          /*   9 - Y-SIZE, number of lines in the image        */
  si32 x_size;          /*  10 - X-SIZE, number of elements (data points) in */
                        /*       each line of the image                      */
  si32 z_size;          /*  11 - Z-SIZE, number of bytes per band (data      */
                        /*       element)                                    */
  si32 y_res;           /*  12 - Y-RES, line resolution                      */
  si32 x_res;           /*  13 - X-RES, element resolution                   */
  si32 z_res;           /*  14 - Z-RES, z resolution; number of bands or     */
                        /*       channels                                    */
  si32 line_prefix;     /*  15 - line prefix, number of bytes; must be       */
                        /*       multiple of 4                               */
  si32 proj_num;        /*  16 - SSEC file creation project number           */
  si32 create_date;     /*  17 - creation date (year and Julian date), YYDDD */
  si32 create_time;     /*  18 - creation time, HHMMSS                       */
  si32 filter_map;      /*  19 - filter map for soundings; a 32-bit vector;  */
                        /*       if bit = 1, data for that band exists in    */
                        /*       the file; the right-most bit is for band 1  */
  si32 image_id;        /*  20 - image ID number                             */
  si32 sensor_data[AREA_SENSOR_DATA_LEN];
                        /*  21-24 - reserved for sensor-specific data); for  */
                        /*          RADAR 21,22 = call letters, 23 = range,  */
                        /*          24 = number of bad lines; for METEOSTAT  */
                        /*          21-24 = calibration coefficients; for    */
                        /*          GMS 21 = IR calibration ID               */
  ui08 memo[AREA_MEMO_LEN];
                        /*  25-32 - up to 32 characters for comments         */
  si32 unused2;         /*  33 - reserved                                    */
  si32 data_offset;     /*  34 - byte offset to the start of the data block  */
  si32 nav_offset;      /*  35 - byte offset to the start of the navigation  */
                        /*       block                                       */
  si32 validity_code;   /*  36 - validity code; must match the line prefix   */
  si32 pdl[AREA_PDL_LEN];
                        /*  37-44 - PDL; in packed-byte format if Mode AA    */
                        /*          image; used for pre-GOES-8 satellites    */
  si32 band_8_source;   /*  45 - where band 8 came from if Mode AA-DS image  */
  si32 start_date;      /*  46 - actual image start day (year and Julian     */
                        /*       date), YYDDD                                */
  si32 start_time;      /*  47 - actual image start time, HHMMSS; in milli-  */
                        /*       seconds for POES data                       */
  si32 start_scan;      /*  48 - actual starting scan                        */
  si32 prefix_doc_len;  /*  49 - length of the prefix documentation (DOC)    */
                        /*       section, multiple of 4 bytes                */
  si32 prefix_cal_len;  /*  50 - length of the prefix calibration (CAL)      */
                        /*       section, multiple of 4 bytes                */
  si32 prefix_lev_len;  /*  51 - length of the prefix level (LEV) section    */
                        /*       (band list), multiple of 4 bytes            */
  ui08 source_type[AREA_SOURCE_TYPE_LEN];
                        /*  52 - source type: VISR, VAS, AAA, TIRO, etc.     */
  ui08 calib_type[AREA_CALIB_TYPE_LEN];
                        /*  53 - calibration type: BRIT, RAW, TEMP, RAD,     */
                        /*       etc.                                        */
  si32 unused3[AREA_UNUSED3_LEN];
                        /*  54-59 - reserved                                 */
  si32 suppl_offset;    /*  60 - byte offset of the supplemental block       */
  si32 suppl_len;       /*  61 - number of bytes in the supplemental block   */
  si32 unused4;         /*  62 - reserved                                    */
  si32 cal_offset;      /*  63 - byte offset to the start of the calibration */
                        /*       (CAL) block                                 */
  si32 num_comments;    /*  64 - number of comments appearing after the data */
                        /*       block; each comment is 80 characters long   */
} AREA_dir_block_t;

/*
 * Defines for lengths of fields in the navigation blocks.
 */

#define AREA_NAV_TYPE_LEN             4
#define AREA_NAV_LAMB_UNUSED_LEN    109
#define AREA_NAV_LAMB_MEMO_LEN        8

/*
 * Defines for navigation block types.  These are used internally.
 */

#define AREA_NAV_UNKNOWN        -1
#define AREA_NAV_NONE            0
#define AREA_NAV_GOES            1
#define AREA_NAV_TIRO            2
#define AREA_NAV_GVAR            3
#define AREA_NAV_PS              4
#define AREA_NAV_LAMB            5
#define AREA_NAV_RADR            6
#define AREA_NAV_RECT            7
#define AREA_NAV_MERC            8
#define AREA_NAV_MET             9


/*
 * Navigation blocks
 */

typedef struct
{
  ui08 nav_type[AREA_NAV_TYPE_LEN];
                        /*   1 - navigation type, 4-byte ASCII; should be   */
                        /*       LAMB                                       */
  si32 np_line;         /*   2 - image line of the North Pole               */
  si32 np_element;      /*   3 - image element of the North Pole            */
  si32 latitude_1;      /*   4 - standard latitude 1, DDDMMSS               */
  si32 latitude_2;      /*   5 - standard latitude 2, DDDMMSS               */
  si32 lat_spacing;     /*   6 - spacing at standard latitude, M            */
  si32 longitude;       /*   7 - normal longitude, DDDMMSS                  */
  si32 planet_radius;   /*   8 - radius of the planet, M                    */
  si32 planet_ecc;      /*   9 - eccentricity of the planet, * 1000000      */
  si32 coord_type;      /*  10 - coordinate type, >= 0 planetodetic, < 0    */
                        /*       planetocentric                             */
  si32 long_conv;       /*  11 - longitude convention, >= 0 west positive,  */
                        /*       < 0 west negative                          */
  si32 unused[AREA_NAV_LAMB_UNUSED_LEN];
                        /*  12-120 - reserved                               */
  ui08 memo[AREA_NAV_LAMB_MEMO_LEN];
                        /* 121-128 - memo entry; up to 32 characters of     */
  /*           comments                               */
} AREA_lamb_nav_block_t;

  
typedef struct
{
  ui08 nav_type[AREA_NAV_TYPE_LEN];
                        /*   1 - navigation type, 4-byte ASCII; should be    */
                        /*       RADR                                        */
  si32 row;             /*   2 - row (image coordinates) of the radar site   */
  si32 column;          /*   3 - column (image coordinates) of the radar     */
                        /*       site                                        */
  si32 latitude;        /*   4 - latitude of the radar site, DDDMMSS         */
  si32 longitude;       /*   5 - longitude of the radar site, DDDMMSS        */
  si32 resolution;      /*   6 - pixel resolution, meters                    */
  si32 rotation;        /*   7 - rotation of north from vertical, degrees *  */
                        /*       1000                                        */
  si32 lon_resolution;  /*   8 - if present, same as resolution, but only    */
                        /*       for the longitude direction                 */
} AREA_radr_nav_block_t;

typedef struct
{
  ui08 nav_type[AREA_NAV_TYPE_LEN];
                        /*   1 - navigation type, 4-byte ASCII; should be    */
                        /*       RECT                                        */
  si32 row_num;         /*   2 - a particular image row number               */
  si32 row_latitude;    /*   3 - latitude corresponding to 2, degrees *      */
                        /*       10000                                       */
  si32 column_num;      /*   4 - a particular image column number            */
  si32 column_longitude;/*   5 - longitude corresponding to 4, degrees *     */
                        /*       10000                                       */
  si32 degrees_lat;     /*   6 - latitude degrees/image line, degrees *      */
                        /*       10000                                       */
  si32 degrees_lon;     /*   7 - longitude degrees/image line, degrees *     */
                        /*       10000                                       */
  si32 planet_radius;   /*   8 - radius of the planet, meters                */
  si32 planet_ecc;      /*   9 - eccentricity of the planet, * 1000000       */
  si32 coord_type;      /*  10 - coordinate type, >= 0 planetodetic, < 0     */
                        /*       planetocentric                              */
  si32 long_conv;       /*  11 - longitude convention, >= 0 west positive,   */
                        /*       < 0 west negative                           */
} AREA_rect_nav_block_t;


  
/************************************************************************
 * Function prototypes.
 ************************************************************************/

/************************************************************************
 * AREA_nav_type_to_id(): Converts the navigation type string (note that
 *                        this string is NOT null-terminated) into an
 *                        internal id, defined as AREA_NAV_xxx.
 *
 * Returns the appropriate id (AREA_NAV_xxxx) for the string.  If the
 * navigation type was unknown, returns AREA_NAV_UNKNOWN.
 */

int AREA_nav_type_to_id(char *nav_type);

/************************************************************************
 * AREA_needs_swap(): Determines whether the data read in from a McIDAS
 *                    AREA file needs to be swapped based on the type
 *                    value in the directory block.  This value should
 *                    always be equal to the AREA_VERSION_NUMBER.
 *
 * Note that this routine must only be called before the directory
 * block is swapped and the returned flag should be saved by the calling
 * routine to determine if the other parts of the AREA file need to be
 * swapped, also.
 *
 * Returns TRUE if the data needs to be swapped, FALSE otherwise.
 */

int AREA_needs_swap(AREA_dir_block_t *dir_block);

/************************************************************************
 * AREA_print_dir_block(): Print the contents of an AREA file directory
 *                         block in ASCII format to the given stream.
 */

void AREA_print_dir_block(FILE *stream, AREA_dir_block_t *dir_block);

/************************************************************************
 * AREA_print_lamb_nav_block(): Print the contents of an AREA file LAMB
 *                              navigation block in ASCII format to the
 *                              given stream.
 */

void AREA_print_lamb_nav_block(FILE *stream,
			       AREA_lamb_nav_block_t *nav_block);

/************************************************************************
 * AREA_print_radr_nav_block(): Print the contents of an AREA file radar
 *                              navigation block in ASCII format to the
 *                              given stream.
 */

void AREA_print_radr_nav_block(FILE *stream,
			       AREA_radr_nav_block_t *nav_block);

/************************************************************************
 * AREA_print_rect_nav_block(): Print the contents of an AREA file RECT
 *                              navigation block in ASCII format to the
 *                              given stream.
 */

void AREA_print_rect_nav_block(FILE *stream,
			       AREA_rect_nav_block_t *nav_block);

/************************************************************************
 * AREA_swap_dir_block(): Swap the bytes in all of the integers in an
 *                        AREA file directory block.
 */

void AREA_swap_dir_block(AREA_dir_block_t *dir_block);

/************************************************************************
 * AREA_swap_lamb_nav_block(): Swap the bytes in all of the integers in an
 *                             AREA file LAMB navigation block.
 */

void AREA_swap_lamb_nav_block(AREA_lamb_nav_block_t *nav_block);

/************************************************************************
 * AREA_swap_radr_nav_block(): Swap the bytes in all of the integers in an
 *                             AREA file radar navigation block.
 */

void AREA_swap_radr_nav_block(AREA_radr_nav_block_t *nav_block);

/************************************************************************
 * AREA_swap_rect_nav_block(): Swap the bytes in all of the integers in an
 *                             AREA file RECT navigation block.
 */

void AREA_swap_rect_nav_block(AREA_rect_nav_block_t *nav_block);



# endif     /* MCIDAS_AREA_H */

#ifdef __cplusplus
}
#endif



