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
* MDV_MACROS.H : Meteorological Data Volume Format macro definitions
*                   
*************************************************************************/
#ifndef    MDV_MACROS_H
#define    MDV_MACROS_H

/* General defines used by the MDV library. */

#define MDV_SUCCESS 0
#define MDV_FAILURE -1

#define MDV_ENCODE_KEY 255

/* How the data were generated -- Collection Type*/
#define     MDV_DATA_MEASURED         0     /* from some instrument */
#define     MDV_DATA_EXTRAPOLATED     1     /* based on measured data */
#define     MDV_DATA_FORECAST         2     /* Numerically modeled */
#define     MDV_DATA_SYNTHESIS        3     /* A synthesis of modeled, 
                                               measured data */
#define     MDV_DATA_MIXED            4     /* Different types of data 
                                               in dataset */

/* How the array is positioned on the earth. 
   Note: ORIENT_SN_WE is always supported */
#define     MDV_ORIENT_OTHER          0     /* Not a typical grid 
                                               spherical coords, etc */
#define     MDV_ORIENT_SN_WE          1     /* This is the standard ordering */
#define     MDV_ORIENT_NS_WE          2
#define     MDV_ORIENT_SN_EW          3
#define     MDV_ORIENT_NS_EW          4

/* Projection type */
#define     MDV_PROJ_NATIVE           -1
#define     MDV_PROJ_LATLON           0   /* x,y in degrees. 
                                             z defined by vert proj type */
#define     MDV_PROJ_ARTCC            1   /* x,y in km */
#define     MDV_PROJ_STEREOGRAPHIC    2   /* x,y in km */
#define     MDV_PROJ_LAMBERT_CONF     3   /* x,y in km */
#define     MDV_PROJ_MERCATOR         4   /* x,y in km */
#define     MDV_PROJ_POLAR_STEREO     5   /* x,y in km */
#define     MDV_PROJ_POLAR_ST_ELLIP   6   /* x,y in km */
#define     MDV_PROJ_CYL_EQUIDIST     7   /* x,y in km */
#define     MDV_PROJ_FLAT             8   /* Cartesian, x,y in km. 
                                             z defined by vert proj type*/
#define     MDV_PROJ_POLAR_RADAR      9   /* Radial range, Azimuth angle,
                                           * x is gate spacing in km .
                                           * y is azimuth in degrees. from
					   * true north + is clockwise
					   * z is elevation angle in degrees. 
                                           */
#define     MDV_PROJ_RADIAL          10   /* x = Radius, Meters,
					   * y = azimuth in degrees
					   * z = Defined by MDV_VERT_TYPE...
					   */
#define     MDV_PROJ_UNKNOWN         99

/* Describes the order of the data in the arrays 
   Note: ORDER_XYZ must be supported */
#define     MDV_ORDER_XYZ             0
#define     MDV_ORDER_YXZ             1
#define     MDV_ORDER_XZY             2
#define     MDV_ORDER_YZX             3
#define     MDV_ORDER_ZXY             4
#define     MDV_ORDER_ZYX             5

/* Vertical Type */
#define     MDV_VERT_TYPE_SURFACE     1  
#define     MDV_VERT_TYPE_SIGMA_P     2
#define     MDV_VERT_TYPE_PRESSURE    3    /* Units = mb  */
#define     MDV_VERT_TYPE_Z           4    /* Constant altitude; 
                                              units = Km MSL */
#define     MDV_VERT_TYPE_SIGMA_Z     5
#define     MDV_VERT_TYPE_ETA         6
#define     MDV_VERT_TYPE_THETA       7    /* Isentropic surface, 
                                              units = Kelvin */
#define     MDV_VERT_TYPE_MIXED       8    /* Any hybrid grid */
#define     MDV_VERT_TYPE_ELEV        9    /* Elevation angles */
#define     MDV_VERT_TYPE_COMPOSITE  10    /* A Composite of a set of planes */
#define     MDV_VERT_TYPE_CROSS_SEC  11    /* Cross sectional view of a 
                                              set of planes */
#define     MDV_VERT_SATELLITE_IMAGE 12    /* Satelite Image */
#define     MDV_VERT_VARIABLE_ELEV   13    /* variable elevation scan */
#define     MDV_VERT_FIELDS_VAR_ELEV 14    /* variable elevation scan,
					    * field specific */
#define     MDV_VERT_FLIGHT_LEVEL    15    /* standard atmosphere flight level.
					    * Flight level is alt (ft) / 100. */


/*
 * Binary encoding of the data in the arrays.
 *
 * Only NATIVE (ASIS), INT8, INT16 and FLOAT32 supported
 */

#define MDV_NATIVE                    0    /* Whatever units the data 
					    * is already in - not for file
					    * storage, only function args */

#define MDV_ASIS                      0    /* same as MDV_NATIVE */

#define MDV_INT8                      1    /* Uncompressed 8 bit 
                                              unsigned integers */

#define MDV_INT16                     2    /* Uncompressed 16 bit 
                                              unsigned integers */

#define MDV_FLOAT32                   5    /* Uncompressed 32 bit 
                                              signed IEEE Floats */


/* The following encoding and compression format, MDV_PLANE_RLE8, is
 * equivalent to MDV_INT8 and MDV_COMPRESSION_RLE.
 * It's use is deprecated, but it is included for backward compatibility.
 * When the library finds this encoding type, it internally represents it
 * as MDV_INT8 and MDV_COMPRESSION_RLE.
 */

#define MDV_PLANE_RLE8                10   /* Each plane encoded/decoded 
					    * using run-length encoding on
					    * the bytes in the data. */

/* Composite algorithms.  These reflect the types of compositing of
 * data that can be done by the MDV library.
 */

#define MDV_COMPOSITE_NONE            0
#define MDV_COMPOSITE_MAX             1

/* Chunk ids */

#define MDV_CHUNK_DOBSON_VOL_PARAMS        0
#define MDV_CHUNK_DOBSON_ELEVATIONS        1
#define MDV_CHUNK_NOWCAST_DATA_TIMES       2
#define MDV_CHUNK_DSRADAR_PARAMS           3
#define MDV_CHUNK_DSRADAR_ELEVATIONS       4
#define MDV_CHUNK_VARIABLE_ELEV            5
#define MDV_CHUNK_SOUNDING_DATA            6

#define MDV_CHUNK_TEXT                     8

/*
 * compression types
 */

#define MDV_COMPRESSION_ASIS -1 /* not for file storage - only for
				 * arg in functions */

#define MDV_COMPRESSION_NONE 0
#define MDV_COMPRESSION_RLE  1
#define MDV_COMPRESSION_LZO  2
#define MDV_COMPRESSION_ZLIB 3
#define MDV_COMPRESSION_BZIP 4
#define MDV_COMPRESSION_GZIP 5

/*
 * data transform types
 *
 * If NONE, the data is interpreted as-is.
 * If the data is transformed (eg LOG) the inverse transform must be
 * applied before interpreting the floating point data.
 */

#define MDV_TRANSFORM_NONE   0
#define MDV_TRANSFORM_LOG    1 /* forward transform log()
				* inverse transform exp() */

/*
 * scaling procedures
 *
 * When converting from float to an int type, data scaling must be performed.
 * The following scaling schemes are supported:
 *
 *  (a) dynamic - scale and bias computed from the data range
 *  (b) dynamic_integral - scale and bias computed from the data range,
 *      but scale and bias constrained to be integral values
 *  (c) specified - scale and bias specified.
 */

#define MDV_SCALING_NONE      0
#define MDV_SCALING_ROUNDED   1
#define MDV_SCALING_INTEGRAL  2
#define MDV_SCALING_DYNAMIC   3
#define MDV_SCALING_SPECIFIED 4

#endif /*    MDV_MACROS_H */







