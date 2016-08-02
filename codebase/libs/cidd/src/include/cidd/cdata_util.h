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
/******************************************************************
 * CDATA_UTIL.H
 *
 * Structures and defines for cartesian data servers and clients.
 *
 * The command is sent to the server via the struct cdata_ieee_comm_t.
 * and cdata_ieee_comm2_t (carries the URL, field name and cross section
 * way  points
 * The primary command is coded as bits in the long primary_com.
 * The secondary command is set as a long in secondary_com.
 * 
 *
 * The reply is received via a number of structs and arrays in a
 * specified order, as follows:
 *
 * 1. cdata_ieee_reply_t - always
 *
 * 2. cdata_ieee_info_t - if (reply.status & INFO_FOLLOWS) is true.  This
 *    should always be sent when serving out lat/lon data since the
 *    projection information is given in this structure.
 *
 * 3. plane_heights as a long array if (reply.status & PLANE_HEIGHTS_FOLLOW)
 *    is true. If plane heights are sent, the info struct is always sent,
 *    and the info.divisor value applies to the plane heights. The number
 *    height values is (info.nz * 3), i.e. there are 3 height values for
 *    each plane. These are lower, mid and upper limits for each plane, and
 *    are sent consecutively, with the plane number changhing for each 3
 *    values sent.
 *   
 * 4. the byte data for the requested field and plane.
 *
 * Version 1 Notes:
 * The cdata_ class of data structs referred to above contain only
 * long and char data. Floating point values are sent as longs, and 
 * are to be scaled by the divisor value if marked ## in the
 * comment. These structs are used for data transmission, and the 
 * longs are transmitted in network-byte-order.
 *
 * The corresponding cd_ class of structs contain the data in the
 * native floating point format, and may be used by the server or the
 * client for convenience, but are not used for data transmission.
 *
 * Version 2 notes:
 * The Scaled longs proved to be problematic, so we now use 32bit ieee floats
 * instead. Additionally, we request fields by name , instead of using field
 * number index. Version 2 provides support for URL directory naming
 * conventions and support for multiple way point cross sections 
 *
 * Frank Hage, RAP, NCAR, Boulder, CO, USA, 1991
 * Updated by Mike Dixon, May 1992, August 1993
 * Updated for Version 2 IEEE protocols April 1999 - F. Hage.
 *
 ***********************************************************************/

#ifndef CDATA_UTIL_H
#define CDATA_UTIL_H

#ifdef IRIX5
#ifndef _BSD_TYPES
#define _BSD_TYPES
#endif
#include <sys/bsd_types.h>
#endif
   
#include <sys/types.h>
#include <toolsa/servmap.h>
#include <dataport/port_types.h>
   
#ifndef TRUE
#define TRUE 1
#endif
   
#ifndef FALSE
#define FALSE 0
#endif
   
#define CDATA_SERVER_PORT 65151   /* Default port CDATA service runs on */

#define CDATA_COMMAND 46464       /* ID cookie for Cart DATA command */
#define CDATA_REPLY 46465         /* ID cookie for Cart DATA reply */
#define CDATA_GRID_INFO 46466     /* ID cookie for Cart DATA info */
#define CDATA_GRID_DATA 46467     /* ID cookie for Cart DATA */
#define CDATA_DIVISOR 10000.0     /* Scaler to convert Longs to Doubles */
#define CDATA_LATLON_DIVISOR 10000000.0 /* ditto for PJG_LATLON */
#define CDATA_HIGHRES_DIVISOR 10000000

/*
 * PRIMARY COMMANDS - BIT MASK
 */

#define GET_INFO          0x00000001 /* Return Info on the data, grid, etc */
#define GET_DATA          0x00000002 /* Return some data */
#define GET_MOST_RECENT   0x00000004 /* Return the most recent data possible */

#define GET_NEW           0x00000008 /* Return the most recent data possible.
				      * In this mode, the request time
				      * centroid contains the time of
				      * the last data received. If there
				      * is new data (later that the 
				      * previous request), then the 
				      * new data is sent. Otherwise,
				      * no data is sent */

#define GET_PLANE_HEIGHTS 0x00000010 /* Return array of plane heights - 
				      * there are 3 heights for each
				      * plane: lower, mid, upper. The heights
				      * for each plane are sent
				      * consecutively */

#define GET_DATA_TIMES     0x00000020 /* Return array of data Times */ 
#define CMD_USE_IEEE_FLOAT 0x00001000 /* CMD Struct is Version 2 Float */
/*
 *  SECONDARY COMMANDS
 */

#define GET_XY_PLANE 1       /* Retrieve a Horizontal plane of data */
#define GET_XZ_PLANE 2       /* Retrieve a Horizontal plane of data */
#define GET_YZ_PLANE 3       /* Retrieve a Horizontal plane of data */
#define GET_V_PLANE 4        /* Retrieve a Vertical plane of data */
#define GET_VOLUME 5         /* Retrieve a volume of data */
#define GET_MAX_XY_PLANE 6   /* Retrieve The Maxvalue in H orient */ 
#define GET_MAX_XZ_PLANE 7   /* Retrieve The Maxvalue in H orient */
#define GET_MAX_YZ_PLANE 8   /* Retrieve The Maxvalue in H orient */

/*
 * DATA SERVER DATA TYPES
 */
				      
#define CDATA_CHAR 1
#define CDATA_SHORT 2
#define CDATA_LONG 3
#define CDATA_FLOAT 4
#define CDATA_DOUBLE 5

/*
 * DATA ORIENTATIONS
 */

#define XY_PLANE 1  /* Top View */
#define XZ_PLANE 2  /* Longitude along X Height along Y */
#define YZ_PLANE 3  /* Latitude along X, Height along Y */
#define V_PLANE 4   /* Arbitrary Vertical plane   */

/*
 * DATA PROJECTIONS
 *
 * see "toolsa/pjg.h"
 */

/*
 * SERVER RETURNED  STATUS MESSAGES - BITS
 */

#define REQUEST_SATISFIED    0x00000001 /* Indicates success */
#define INFO_FOLLOWS         0x00000002 /* Information Message follows reply */
#define DATA_FOLLOWS         0x00000004 /* Data Message(s) follows reply */
#define MOST_RECENT          0x00000008 /* Data is the most recent available */
#define NEW_DATA             0x00000010 /* Data changed since last request */
#define NO_NEW_DATA          0x00000020 /* Data changed since last request */
#define NO_DATA_AVAILABLE    0x00000040 /* No data available */
#define NO_INFO              0x00000080 /* Data info is not availible */
#define VOLUME_LIMITS        0x00000100 /* No data in desired volume */
#define TIME_LIMITS          0x00000200 /* No data within the time frame */
#define PLANE_HEIGHTS_FOLLOW 0x00000400 /* A Height message struct fllows */
#define DATA_TIMES_FOLLOW    0x00000800 /* A Data times message follows */
#define REPLY_USE_IEEE_FLOAT 0x00001000 /* Struct is Version 2 Float */
#define NO_SUCH_FILE         0x00010000 /* File or directory not found */
#define NO_SUCH_FIELD        0x00020000 /* File Does not contain the requested field */


#define LAB_LEN 32  /* Character label lengths */

/*
 * The following structures are passed across the network in
 * network-byte-order
 */

/*
 * The client command request - Version 1.
 *
 * If the comment is marked ##, divide by 'divisor' to get the
 * floating point value
 */

typedef struct {

  si32 primary_com; /* The primary command */
  si32 second_com;  /* The secondary command */
  si32 divisor;     /* Scaling quanity to convert longs to double */

  si32 lat_origin;  /* ## Degrees latitude of the origin of
		     * the coord system */
  si32 lon_origin;  /* ## Degrees longitude of the origin of
		     * the coord system */
  si32 ht_origin;   /* ## Km */

                    /* times are secs since 1/1/1970 0:00:00) */

  si32 time_min;    /* start time */
  si32 time_cent;   /* time centroid */
  si32 time_max;    /* end time */

  si32 min_x,max_x; /* ## coord limits (Km for flat, deg for lat/lon) */
  si32 min_y,max_y; /* ## coord limits (Km for flat, deg for lat/lon) */
  si32 min_z,max_z; /* ## coord limits (Km for flat, deg for lat/lon) */

  si32 data_field;  /* Which data field to return data/info on */
  si32 data_type;   /* Type of data to return. i.e.Float, int, char */
  si32 add_data_len;/* Additional data bytes to read for more request info */

} cdata_comm_t;

/*
 * The client IEEE FLOAT command request - VERSION 2 Protocol
 */
typedef struct {

  si32 primary_com; /* The primary command */
  si32 second_com;  /* The secondary command */
  si32 reserved1;   /* */

  fl32 lat_origin;  /* Degrees latitude of the origin of * the coord system */
  fl32 lon_origin;  /* Degrees longitude of the origin of  the coord system */
  fl32 ht_origin;   /* Km */

                    /* times are secs since 1/1/1970 0:00:00) */
  si32 time_min;    /* start time */
  si32 time_cent;   /* time centroid */
  si32 time_max;    /* end time */

  fl32 min_x,max_x; /* coord limits (Km for flat, deg for lat/lon) */
  fl32 min_y,max_y; /* coord limits (Km for flat, deg for lat/lon) */
  fl32 min_z,max_z; /* coord limits (Km for most, deg for radial, etc) */

  si32 data_field;  /* Which data field to return data/info on */
  si32 data_type;   /* Type of data to return. i.e.Float, int, char */
  si32 add_data_len;/* Additional data bytes to read for more request info - 
		     * see cdata_ieee_comm2_t below
		     */
} cdata_ieee_comm_t;

/*
 * The client IEEE FLOAT command2 request - VERSION 2 Protocol
 */
#define COM2_SI32_BYTES 8
typedef struct {
  si32 url_len;     /* Length of null terminated url string in message */
  si32 num_way_points; /* Number of points requested - (N Segments + 1) */
  char url[1];      /* Actually url_len url[] char array + */
		    /* struct way_point[] of cross section points -  */
		    /* pad url[] to make (url_len)%4 == 0
		     */
} cdata_ieee_comm2_t;


typedef struct {
  fl32 lat;
  fl32 lon;
} way_point_t;

/*
 * The server reply - Version 1.
 *
 * If the comment is marked ##, divide by 'divisor' to get the
 * floating point value
 *
 * If the comment is marked %%, then
 *   for projection == PJG_FLAT,  divide by divisor to get floating
 *                                point value
 *   for projection == PJG_LATLON,  divide by CDATA_HIGHRES_DIVISOR
 *                                  to get floating point value
 */

typedef struct {

  si32 status;       /* Status message bit mask */
  si32 divisor;      /* Scaling quanity to convert si32s to doubles */
  si32 orient;       /* The data's orientation */
  si32 nx,ny,nz;     /* number of grid points returned in each direction */

  si32 dx,dy;        /* %% Size of grid points in the x and y directions -
		      * (Km for flat, deg for lat/lon) */

  si32 dz;           /* ## Size of grid points in the Z direction - (Km)
		      * If Plane heights are returned, dz is not
		      * applicable. Therefore, it is set to 0 
		      * in this struct */

  si32 x1,x2;        /* Grid limits in returned data */
  si32 y1,y2;        /* Grid limits in returned data */
  si32 z1,z2;        /* Grid limits in returned data */
  si32 scale;        /* ## Multiplier to reconstruct orig value */
  si32 bias;         /* ## Bias value to reconstruct orig value */
  si32 time_begin;   /* Time when data began to accumulate */
  si32 time_end;     /* Time when data finished accumulating */
  si32 time_cent;    /* Time Centroid (Weighted) */
  si32 bad_data_val; /* Value that represents bad or missing data */
  si32 data_type;    /* Type of data in data plane */
  si32 data_field;   /* Which field info is about */
  si32 expire_time;  /* Unix time at which data becomes invalid */
  si32 n_points;     /* number of points in data plane */
  si32 data_length;  /* bytes of remaining data to be read  - info,
		      * plane_heights, data plane */

} cdata_reply_t;


/*
 * The client IEEE FLOAT reply message - VERSION 2 Protocol
 */
typedef struct {

  si32 status;       /* Status message bit mask */
  si32 unused1;      /* */
  si32 orient;       /* The data's orientation - Enum */
  si32 nx,ny,nz;     /* number of grid points returned in each direction */

  fl32 dx,dy;        /* Size of grid points in the x and y directions -
		      * (Km for flat, deg for lat/lon) */

  fl32 dz;           /* Size of grid points in the Z direction - (Km)
		      * If Plane heights are returned, dz is not
		      * applicable. Therefore, it is set to 0 
		      * in this struct */

  si32 x1,x2;        /* Grid limits in returned data */
  si32 y1,y2;        /* Grid limits in returned data */
  si32 z1,z2;        /* Grid limits in returned data */
  fl32 scale;        /* Multiplier to reconstruct orig value */
  fl32 bias;         /* Bias value to reconstruct orig value */
  si32 time_begin;   /* Time when data began to accumulate */
  si32 time_end;     /* Time when data finished accumulating */
  si32 time_cent;    /* Time Centroid (Weighted) */
  si32 bad_data_val; /* Value that represents bad or missing data */
  si32 data_type;    /* Type of data in data plane */
  si32 data_field;   /* Which field info is about */
  si32 expire_time;  /* Unix time at which data becomes invalid */
  si32 n_points;     /* number of points in data plane */
  si32 data_length;  /* bytes of remaining data to be read  - info,
		      * plane_heights, data plane, data_times */

} cdata_ieee_reply_t;

/*
 * the grid info - Version 1.
 *
 * If the comment is marked ##, divide by 'divisor' to get the
 * floating point value
 *
 * If the comment is marked %%, then
 *   for projection == PJG_FLAT,  divide by divisor to get floating
 *                                point value
 *   for projection == PJG_LATLON,  divide by CDATA_HIGHRES_DIVISOR
 *                                  to get floating point value
 */

typedef struct { /* ## = APPLY SCALING QUANTITY */

  si32 divisor;      /* Scaling quanity to convert si32s to doubles */
  si32 order;        /* 0 = right handed coord system, 1 = left */
  si32 data_field;   /* Which field info is about */
  si32 projection;   /* See "toolsa/pjg.h" */
  si32 lat_origin;   /* ## Degrees lat of the origin of the coord system,
		      * for projection == PJG_FLAT */
  si32 lon_origin;   /* ## Degrees long of the origin of the coord system,
		      * for projection == PJG_FLAT */
  si32 source_x;     /* Data source X location in coordinate system */
  si32 source_y;     /* Data source Y location in coordinate system */
  si32 source_z;     /* Data source Z location in coordinate system */
  si32 ht_origin;    /* ## Km */
  si32 nx,ny,nz;     /* number of grid points in each direction */
  si32 dx,dy;        /* %% Size of grid in (x,y) direction */
  si32 dz;           /* ## Size of grid in z dirn - (Km) */
  si32 min_x,max_x;  /* %% limits of grid in x direction */
  si32 min_y,max_y;  /* %% limits of grid in y direction */
  si32 min_z,max_z;  /* ## limits of grid in z direction */
  si32 north_angle;  /* ## Angle of Y axis relative to true north */
  si32 gate_spacing; /* ## meters */
  si32 wavelength;   /* ## micro-meters */
  si32 frequency;    /* ## kHz */
  si32 min_range;    /* ## km */
  si32 max_range;    /* ## km */
  si32 num_gates;    /*  */
  si32 min_gates;    /*  */
  si32 max_gates;    /*  */
  si32 num_tilts;    /* The number of unique radar tilts */
  si32 min_elev;     /* ## Minimum  elevation of scans - Deg */
  si32 max_elev;     /* ## Maximum elevation */
  si32 radar_const;  /* ## */
  si32 highres_divisor;  /* ##  */
  si32 delta_azmith; /* ## degrees between each beam */
  si32 start_azmith; /* ## degrees */
  si32 beam_width;   /* ## degrees */
  si32 pulse_width;  /* ## nano-seconds */
  si32 data_length;  /* bytes of remaining data to read */
  si32 noise_thresh; /* ## Signal/Noise threshold for data rejection */
  si32 nfields;      /* number of data fields */
  char units_label_x[LAB_LEN];  /* Units eg. km, meters etc */
  char units_label_y[LAB_LEN];  /* Units eg. km, meters etc */
  char units_label_z[LAB_LEN];  /* Units eg. mbar, meters etc */
  char field_units[LAB_LEN];    /* Units eg. dbz, m/sec etc */
  char field_name[LAB_LEN]; 
  char source_name[LAB_LEN];    /* eg. radar name, algorithm name etc */

} cdata_info_t;


/*
 * IEEE  Grid Info - Version 2 protocol
 */
typedef struct { /* ## = APPLY SCALING QUANTITY */

  si32 divisor;      /* Scaling quanity to convert si32s to doubles */
  si32 order;        /* 0 = right handed coord system, 1 = left */
  si32 data_field;   /* Which field info is about */
  si32 projection;   /* See "toolsa/pjg.h" */
  fl32 lat_origin;   /* Degrees lat of the origin of the coord system,
		      * for projection == PJG_FLAT */
  fl32 lon_origin;   /* Degrees long of the origin of the coord system,
		      * for projection == PJG_FLAT */
  fl32 source_x;     /* Data source X location in coordinate system */
  fl32 source_y;     /* Data source Y location in coordinate system */
  fl32 source_z;     /* Data source Z location in coordinate system */
  fl32 ht_origin;    /* Km */
  si32 nx,ny,nz;     /* number of grid points in each direction */
  fl32 dx,dy;        /* Size of grid in (x,y) direction */
  fl32 dz;           /* Size of grid in z dirn - (Km) */
  fl32 min_x,max_x;  /* %% limits of grid in x direction */
  fl32 min_y,max_y;  /* %% limits of grid in y direction */
  fl32 min_z,max_z;  /* limits of grid in z direction */
  fl32 north_angle;  /* Angle of Y axis relative to true north */
  fl32 gate_spacing; /* meters */
  fl32 wavelength;   /* micro-meters */
  fl32 frequency;    /* kHz */
  fl32 min_range;    /* km */
  fl32 max_range;    /* km */
  si32 num_gates;    /*  */
  si32 min_gates;    /*  */
  si32 max_gates;    /*  */
  si32 num_tilts;    /* The number of unique radar tilts */
  fl32 min_elev;     /* Minimum  elevation of scans - Deg */
  fl32 max_elev;     /* Maximum elevation */
  fl32 radar_const;  /* */
  fl32 nyquist_vel;  /* */
  fl32 delta_azmith; /* degrees between each beam */
  fl32 start_azmith; /* degrees */
  fl32 beam_width;   /* degrees */
  fl32 pulse_width;  /* nano-seconds */
  si32 data_length;  /* bytes of remaining data to read */
  fl32 noise_thresh; /* Signal/Noise threshold for data rejection */
  si32 nfields;      /* number of data fields */
  char units_label_x[LAB_LEN];  /* Units eg. km, meters etc */
  char units_label_y[LAB_LEN];  /* Units eg. km, meters etc */
  char units_label_z[LAB_LEN];  /* Units eg. mbar, meters etc */
  char field_units[LAB_LEN];    /* Units eg. dbz, m/sec etc */
  char field_name[LAB_LEN]; 
  char source_name[LAB_LEN];    /* eg. radar name, algorithm name etc */

} cdata_ieee_info_t;

#define NUM_INFO_LONGS 43  /* the number of longd in the info struct */

/*
 * The following are convenience structs in native floating point
 * format where applicable - they are not used for data transmission
 * Note: FOr Version 2 protocols Floats are used - See the _ieee_ named 
 *  structs defined above.
 */

/*
 * The Client command request
 */

typedef struct { /* Native Floating point format */

  si32 primary_com;   /* The primary command */
  si32 second_com;    /* The secondary command */

                      /* times are secs since 1/1/1970 0:00:00) */

  si32 time_min;      /* start time */
  si32 time_cent;     /* time centroid */
  si32 time_max;      /* end time */

  si32 data_field;    /* Which data field to return data/info on */
  si32 data_type;     /* Type of data to return. i.e.Float, int, char */
  si32 add_data_len;  /* Additional data bytes to read for more request info */

  double lat_origin;  /* Degrees lat of the origin of the coord system */
  double lon_origin;  /* Degrees long of the origin of the coord system */
  double ht_origin;   /* Km */

  double min_x,max_x; /* coord limits (Km) */
  double min_y,max_y; /* coord limits (Km) */
  double min_z,max_z; /* coord limits (Km) */

} cd_command_t;

/*
 * the server reply
 */

typedef struct { /* Native Floating Point format */

  si32 status;        /* Status message bit mask */
  si32 orient;        /* The data's orientation */
  si32 nx,ny,nz;      /* number of grid points returned in each direction */

  double dx,dy,dz;    /* Size of grid points in each direction - (Km)
		       * If dz is 0, then plane heights are included */

  si32 x1,x2;         /* Grid limits in returned data (grid units) */
  si32 y1,y2;         /* Grid limits in returned data (grid_units) */
  si32 z1,z2;         /* Grid limits in returned data (grid_units) */
  double scale;       /* Multiplier to reconstruct orig value */
  double bias;        /* Bias value to reconstruct orig value */
  si32 time_begin;    /* Time when data began to accumulate */
  si32 time_end;      /* Time when data finished accumulating */
  si32 time_cent;     /* Time Centroid (Weighted) */
  si32 bad_data_val;  /* Value that represents bad or missing data */
  si32 data_type;     /* Type of data in data plane */
  si32 data_field;    /* Which field info is about */
  si32 expire_time;   /* Unix time at which data becomes invalid */
  si32 n_points;      /* number of points in data plane */
  si32 data_length;   /* bytes of remaining data to be read */

} cd_reply_t;

/*
 * the grid info
 */

typedef struct { /* Native Floating Point format */

  si32 order;          /* 0 = left handed coord system, 1 = right */
  si32 data_field;     /* Which field info is about */
  si32 projection;     /* see "toolsa/pjg.h" */
  double lat_origin;   /* Degrees lat of the origin of the coord system,
			* for projection == PJG_FLAT*/
  double lon_origin;   /* Degrees long of the origin of the coord system,
			* for projection == PJG_FLAT */
  double source_x;     /* Data source X location in coordinate system */
  double source_y;     /* Data source Y location in coordinate system */
  double source_z;     /* Data source Z location in coordinate system */
  double ht_origin;    /* Km */
  si32 nx,ny,nz;       /* number of grid points in each direction */
  double dx,dy,dz;     /* Size of grid points in each direction - (Km) */
  double min_x,max_x;  /* limits of grid in x direction */
  double min_y,max_y;  /* limits of grid in x direction */
  double min_z,max_z;  /* limits of grid in x direction */
  double north_angle;  /* Angle of Y axis relitive to true north */
  double gate_spacing; /* meters */
  double wavelength;   /* micro-meters */
  double frequency;    /* kHz */
  double min_range;    /* km */
  double max_range;    /* km */
  si32 num_gates;
  si32 min_gates;
  si32 max_gates;
  si32 num_tilts;      /* The number of unique radar tilts */
  double min_elev;     /* Minimum  elevation of scans - Deg */
  double max_elev;     /* Maximum elevation */
  double radar_const;
  double nyquist_vel;
  double delta_azmith; /* degrees between each beam */
  double start_azmith; /* degrees */
  double beam_width;   /* degrees */
  double pulse_width;  /* nano-seconds */
  si32 data_length;    /* bytes of remaining data to read */
  double noise_thresh; /* Signal/Noise threshold for data rejection */
  si32 nfields;
  si32 spare;
  char units_label_x[LAB_LEN];  /* Units eg. km, meters etc */
  char units_label_y[LAB_LEN];  /* Units eg. km, meters etc */
  char units_label_z[LAB_LEN];  /* Units eg. mbar, meters etc */
  char field_units[LAB_LEN];    /* Units eg. dbz, m/sec etc */
  char field_name[LAB_LEN]; 
  char source_name[LAB_LEN];    /* eg. radar name, algorithm name etc */

} cd_grid_info_t;

/*
 * client index struct
 */

typedef struct {
  
  ui08 *data;

  char *prog_name;
  char *server_type;
  char *server_subtype;
  char *server_instance;
  char *default_host;
  char *servmap_host1;
  char *servmap_host2;

  int default_port;
  int messages_flag;            /* TRUE or FALSE */
  int debug_flag;               /* TRUE or FALSE */
  int last_request_successful;  /* TRUE or FALSE */
  int want_realtime;            /* TRUE or FALSE */
  
  si32 n_servers;
  si32 current_server_num;

  double *plane_heights;

  cd_command_t command;
  cd_reply_t reply;
  cd_grid_info_t grid_info;

  SERVMAP_info_t *server_info;

} cdata_index_t;

/*
 * client index struct
 */

typedef struct {
  
  ui08 *data;

  char *prog_name;
  char *url;

  int messages_flag;            /* TRUE or FALSE */
  int debug_flag;               /* TRUE or FALSE */
  int last_request_successful;  /* TRUE or FALSE */
  int want_realtime;            /* TRUE or FALSE */
  
  double *plane_heights;

  cd_command_t command;
  cd_reply_t reply;
  cd_grid_info_t grid_info;

} cdata2_index_t;

/*
 * CDATA_CURRENT_FILE_INDEX - this file indicates
 * the time etc. for the latest file written.
 *
 * The file is named "current_file_index" and resides in the
 * top level directory for a gridded data set.
 */

#define CDATA_CURRENT_FILE_INDEX "current_file_index"
#define CDATA_PREV_FILE_INDEX "prev_file_index"
#define CDATA_MAX_INDEX_STR 32

/*
 * the current_file_index consists of one cdata_current_index_t
 * followed by n_fcasts * cdata_current_fcast_t.
 *
 * The file is in ASCII, with one element per line.
 */

typedef struct {

  long seq_num;                         /* incrementing */

  long year;                            /* file time */
  long month;
  long day;
  long hour;
  long min;
  long sec;
  long unix_time;

  long exact_time;                      /* true or false (for approx time) */

  char file_ext[CDATA_MAX_INDEX_STR];   /* file extension or "unknown" */

  char id1[CDATA_MAX_INDEX_STR];        /* optional extra ID or "unknown" */
  char id2[CDATA_MAX_INDEX_STR];        /* optional extra ID or "unknown" */

  long n_fcasts;                        /* number of forecasts in data set -
					 * usually 0. There are n_fcasts
                                         * f_times following this struct
					 */

  long n_fcasts_alloc;
  long *fcast_times;

} cdata_current_index_t;

/*
 * function prototypes
 */

/*****************************************************************
 * CDATA_INIT()
 *
 * initialize cidd server index
 *
 *   prog_name : program name - used for error messages
 *
 *   servmap_host1, servmap_host2 : server mapper hosts
 *
 *   server_type, server_subtype, server_instance : server details
 *
 *   default_host, default_port : defaults for server location to be
 *   used if server mapper fails
 *
 *   messages_flag - if set, warnings and failure messages will be
 *     printed
 *
 *   debug_flag - if set, debug messages will be printed
 */

extern void cdata_init(char *prog_name,
		       char *servmap_host1,
		       char *servmap_host2,
		       char *server_type,
		       char *server_subtype,
		       char *server_instance,
		       char *default_host,
		       int default_port,
		       int messages_flag,
		       int debug_flag,
		       cdata_index_t *index);

/*************************************************************************
 * CDATA_READ()
 *
 * Reads cidd data using index struct. cdata_init must be
 * called first.
 *
 * returns 0 on success, -1 on failure
 */

extern int cdata_read(cdata_index_t *index);


/*****************************************************************
 * CDATA2_INIT()
 *
 * initialize cidd server index V2 Protocol
 *
 *   prog_name : program name - used for error messages
 *
 *   url: Resource locator for particulat grid
 *
 *   messages_flag - if set, warnings and failure messages will be
 *     printed
 *
 *   debug_flag - if set, debug messages will be printed
 */

extern void cdata2_init(char *prog_name,
		       char *url,
		       int messages_flag,
		       int debug_flag,
		       cdata2_index_t *index);

/*************************************************************************
 * CDATA2_READ()
 *
 * Reads cidd data using index struct. cdata2_init must be
 * called first.
 *
 * returns 0 on success, -1 on failure
 */

extern int cdata2_read(cdata2_index_t *index,char *field_name);

/**************************************************************************
 * CDATA2_GET - Version 2 Protocol
 * Get a Plane of data. Returns 0 if successful, -1 otherwise.
 * After use the user must free up plane data
 */

extern ui08 *cdata2_get(cd_command_t *command,
			 cd_reply_t *reply,
			 cd_grid_info_t *grid_info,
			 char *url);

/**************************************************************************
 * CDATA_GET - Version 1 Protocol
 * Get a Plane of data. Returns 0 if successful, -1 otherwise.
 * After use the user must free up plane data
 */

extern ui08 *cdata_get(cd_command_t *command,
			 cd_reply_t *reply,
			 cd_grid_info_t *grid_info,
			 char *host_name,
			 int port);

/**************************************************************************
 * CDATA_GET_WITH_HEIGHTS - Version 1 Protocol
 * Get a plane of data with heights.
 * Returns 0 if successful, -1 otherwise.
 * After use the user must free up :
 *                         1) the plane heights
 *                         2) plane data
 */

extern int cdata_get_with_heights(cd_command_t *command,
				  cd_reply_t *reply,
				  cd_grid_info_t *grid_info,
				  double **plane_heights,
				  ui08 **plane_data,
				  char *host_name,
				  int port);


/**************************************************************************
 * CDATA2_GET_WITH_HEIGHTS() Version 2 Protocol
 *
 * Get a plane of data with heights.
 * Returns 0 if successful, -1 otherwise.
 * The data and plane heights will be overwritten on subsequent calls
 * to this routine - the client routine should copy the data if this
 * is a problem.
 */

int cdata2_get_with_heights(cd_command_t *command,
                           cd_reply_t *reply,
                           cd_grid_info_t *grid_info,
                           double **plane_heights,
                           ui08 **data,
                           char *url);

/*******************************************************************
 * cdata_index_init()
 * Initialize the index struct for malloc'ing
 */

extern void cdata_index_init(cdata_current_index_t *index);

/*******************************************************************
 * cdata_index_alloc()
 * Alloc space for forecasts
 * Returns 0 on success, -1 on malloc error
 */

extern int cdata_index_alloc(cdata_current_index_t *index,
			     int n_fcasts,
			     char *prog_name,
			     char *calling_routine);
  
/*****************************************************************
 * FILL_CURRENT_INDEX: Fill in structure members of a 
 * cdata_current_index_t data struct.
 * If suffix, id1 or id2 are NULL, the relevant strings are
 * set empty.
 */

extern void fill_current_index(cdata_current_index_t* index, time_t tsec,
			       char* suffix, char* id1, char* id2);

/*********************************************************************
  * cdata_read_index_simple()
  *
  * Read the struct data from the current file index.
  * Wait a maximum of wait_msec number of milliseconds before
  * Returning.
  *
  * Returns -1 on time out.
  * Returns -2 on system or file format errors;
  * Returns 0 when file exists, is newer that max_valid_file_age,
  * and has a sequence number != prev_seq_num.
  *
  * Sets prev_seq_number to current seq_num when returning 0
  *
  * Typically, the calling routine declares prev_seq_num as static,
  * initialized to 0 before the initial call
  *
  *********************************************************************/

extern int cdata_read_index_simple(char *dir,
				   int wait_msec,
				   long *prev_seq_num,
				   long max_valid_file_age,
				   cdata_current_index_t *index,
				   char *prog_name,
				   char *calling_routine,
				   int debug);
     
/*********************************************************************
  * cdata_read_index_fcasts()
  *
  *
  * Read the struct data from the current file index.
  * Wait a maximum of wait_msec number of milliseconds before
  * Returning.
  *
  * Returns -1 on time out.
  * Returns -2 on system or file format errors;
  * Returns 0 when file exists, is newer that max_valid_file_age,
  * and has a sequence number != prev_seq_num.
  *
  * Sets prev_seq_number to current seq_num when returning 0
  *
  * Typically, the calling routine declares prev_seq_num as static,
  * initialized to 0 before the initial call
  *
  *********************************************************************/
  
extern int cdata_read_index_fcasts(char *dir,
				   int wait_msec,
				   long *prev_seq_num,
				   long max_valid_file_age,
				   cdata_current_index_t *index,
				   char *prog_name,
				   char *calling_routine,
				   int debug);
     
/*********************************************************************
  * cdata_remove_index()
  *
  * Removes the index file from a directory.
  *
  * Returns 0 on success, -1 on failure.
  *
  *********************************************************************/
  
extern int cdata_remove_index(char *dir,
			      char *prog_name,
			      char *calling_routine);
     
/*****************************************************************
 * cdata_write_index()
 *
 * Writes a cdata_file_index
 *
 * Writes to a tmp file first, then moves the tmp file to
 * the final file name when done.
 *
 * On success, returns a pointer to the path of the index file,
 * on failure return NULL
 */

extern char *cdata_write_index(char *dir,
			       cdata_current_index_t *index,
			       char *prog_name,
			       char *calling_routine);

/*****************************************************************
 * cdata_write_index_simple()
 *
 * Writes a cdata_file_index which does not contain
 * any forecasts.
 *
 * Writes to a tmp file first, then moves the tmp file to
 * the final file name when done.
 *
 * On success, returns a pointer to the path of the index file,
 * on failure return NULL
 */

extern char *cdata_write_index_simple(char *dir,
				      cdata_current_index_t *index,
				      char *prog_name,
				      char *calling_routine);

/*****************************************************************
 * cdata_write_index_simple2()
 *  
 * A variation on cdata_write_index_simple which fills out
 * the cdata_current_index_t based on the unix time argument
 *
 * (optional id fields are set to "unknown")
 * (exact_time is set to TRUE)   
 *
 * On success, returns a pointer to the path of the index file,
 * on failure return NULL       
 */                                                        
extern char *cdata_write_index_simple2(char *dir,
                                       char *ext,
                                       time_t indexTime,
                                       char *prog_name,
                                       char *calling_routine);

/*****************************************************************
 * COM2_STRUCT_TO_BE() make sure struct elements are in correct
 * byte order 
 */

void  com2_struct_to_BE(cdata_ieee_comm2_t *com2);
void  com2_struct_from_BE(void * buf);
#endif


#ifdef __cplusplus
}
#endif
