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
 * bdry_typedefs.h - Constants and types used by the boundary SPDB databases.
 *
 * Nancy Rehak
 * March 2005
 *                   
 *************************************************************************/

#include <dataport/port_types.h>

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef BDRY_TYPEDEFS_H
#define BDRY_TYPEDEFS_H


#define BDRY_TYPE_LEN               16
#define BDRY_DESC_LEN               80
#define BDRY_LABEL_LEN              80
#define BDRY_LINE_TYPE_LEN          24

#define BDRY_SPARE_INT_LEN                   1
#define BDRY_SPARE_FLOAT_LEN                 4
#define BDRY_POLYLINE_SPARE_LEN              2
#define BDRY_POINT_SPARE_LEN                 2
#define BDRY_POINT_SHEAR_INFO_SPARE_LEN      2

/*
 * Define valid boundary types
 */

#define BDRY_TYPE_BDRY_TRUTH          1
#define BDRY_TYPE_BDRY_MIGFA          2
#define BDRY_TYPE_BDRY_COLIDE         3
#define BDRY_TYPE_COMB_NC_ISSUE       4
#define BDRY_TYPE_COMB_NC_VALID       5
#define BDRY_TYPE_EXTRAP_ISSUE_ANW    6
#define BDRY_TYPE_EXTRAP_VALID_ANW    7
#define BDRY_TYPE_FIRST_GUESS_ISSUE   8
#define BDRY_TYPE_FIRST_GUESS_VALID   9
#define BDRY_TYPE_MINMAX_NC_ISSUE    10
#define BDRY_TYPE_MINMAX_NC_VALID    11
#define BDRY_TYPE_EXTRAP_ISSUE_MIGFA 12
#define BDRY_TYPE_EXTRAP_VALID       13
#define BDRY_TYPE_EXTRAP_ISS_COLIDE  14
#define BDRY_TYPE_PREDICT_COLIDE     15

/*
 * Define valid boundary subtypes
 */

#define BDRY_SUBTYPE_ALL            100

/*
 * Define valid boundary line types
 */

#define BDRY_LINE_TYPE_COMBINED       1
#define BDRY_LINE_TYPE_EXTRAPOLATED   2
#define BDRY_LINE_TYPE_SHEAR          3
#define BDRY_LINE_TYPE_THIN           4
#define BDRY_LINE_TYPE_PREDICT        5
#define BDRY_LINE_TYPE_TRUTH          6
#define BDRY_LINE_TYPE_EMPTY          7
#define BDRY_LINE_TYPE_GENERIC        8

#define BDRY_VALUE_UNKNOWN           -1

/*
 * Define the mask values for the additional point information.
 */

#define BDRY_POINT_INFO_SHEAR         1

/*
 * The following structures define the boundary product as it
 * appears in the SPDB database.
 */

typedef struct
{
  si32 num_pts;               /* number of horizontal points going with */
                              /*   the leading edge point */
  fl32 zbar_cape;             /* CAPE value in shear layer */
  fl32 max_shear;             /* maximum shear in the layer */
  fl32 mean_shear;            /* mean shear in the layer */
  fl32 kmin;                  /* bottom level of shear layer */
  fl32 kmax;                  /* top level of shear layer */
  fl32 spare[BDRY_POINT_SHEAR_INFO_SPARE_LEN];
                              /* spare values for future expansion and to */
                              /*   keep the fields aligned */
} BDRY_spdb_point_shear_info_t;


typedef struct
{
  fl32 lat;                   /* latitude in degrees */
  fl32 lon;                   /* longitude in degrees */
  fl32 u_comp;                /* horizontal motion dir vector component */
                              /*   in m/s */
  fl32 v_comp;                /* vertical motion dir vector component */
                              /*   in m/s */
  fl32 value;                 /* bdry rel steering flow value for the */
                              /*   initiation zone */
  ui32 info_mask;             /* mask indicating which information is */
                              /*   included with this point */
  fl32 spare[BDRY_POINT_SPARE_LEN];
                              /* spare values for future expansion and to */
                              /*   keep the fields aligned */
} BDRY_spdb_point_t;


typedef struct
{
  si32 num_pts;               /* number of points */
  si32 num_secs;              /* number of seconds extrapolation it is */
  si32 spare[BDRY_POLYLINE_SPARE_LEN];
                              /* spare values for future expansion and to */
                              /*   keep the fields aligned */
  char object_label[BDRY_LABEL_LEN];
                              /* label associated with this polyline */
  BDRY_spdb_point_t points[1];
                              /* array of points in the polyline - occurs */
                              /*   num_pts times */
} BDRY_spdb_polyline_t;


typedef struct
{
  si32 type;                  /* product type value as defined above */
  si32 subtype;               /* product subtype value as defined above */
  si32 sequence_num;          /* product counter */
  si32 group_id;              /* group id number */
  si32 generate_time;         /* time of product generation */
  si32 data_time;             /* time of data used to create */
  si32 forecast_time;         /* time of forecast (extrapolation) */
  si32 expire_time;           /* time product becomes invalid */
  si32 line_type;             /* line type value as defined above (for */
                              /*   COLIDE bdrys, extraps) */
  si32 bdry_id;               /* boundary id number */
  si32 num_polylines;         /* number of polylines in this product */
  si32 spare_int[BDRY_SPARE_INT_LEN];
                              /* spare values for future expansion and to */
                              /*   keep the fields aligned */
  fl32 motion_direction;      /* motion direction in degrees (all objects */
                              /*   move together).  This value is given in */
                              /*   math coordinates (0 degrees along X axis, */
                              /*   increases in counterclockwise direction). */
  fl32 motion_speed;          /* motion speed in m/s (all objects move */
                              /*   together) */
  fl32 line_quality_value;    /* quality (confidence) value (for COLIDE) */
  fl32 line_quality_thresh;   /* quality threshold (for COLIDE) */
  fl32 spare_float[BDRY_SPARE_FLOAT_LEN];
                              /* spare values for future expansion and to */
                              /*   keep the fields aligned */
  char type_string[BDRY_TYPE_LEN];
                              /* product type in string format - used to */
                              /*   determine the type value above */
  char subtype_string[BDRY_TYPE_LEN];
                              /* product subtype in string format - used */
                              /*   to determine the subtype above */
  char line_type_string[BDRY_LINE_TYPE_LEN];
                              /* line type (for COLIDE bdrys, extraps) - */
                              /*   used to determine the line_type above */
  char desc[BDRY_DESC_LEN];   /* label associated with the product */
  BDRY_spdb_polyline_t polylines[1];
                              /* array of structures for all of the */
                              /*   polylines making up this product - */
                              /*   num_polylines of these */
} BDRY_spdb_product_t;


# endif     /* BDRY_TYPEDEFS_H */

#ifdef __cplusplus
}
#endif
