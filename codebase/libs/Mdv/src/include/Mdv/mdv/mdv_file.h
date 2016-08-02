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

/************************************************************************
*                    
* MDV_FILE.H: Meteorological Data Volume File Format Definitions
*                   
*                      August 1995 / February 1996
*************************************************************************/

/*************************************************************************
 * The MDV FILE STRUCTURE looks like:
 *
 *         MDV_master_header
 *         MDV_field_header 1 
 *         MDV_field_header 2
 *           ...
 *         MDV_field_header n
 *         MDV_vlevel_header 1  (optional, if one exists, then all must exist)
 *         MDV_vlevel_header 2
 *           ...
 *         MDV_vlevel_header n
 *         MDV_chunk_header 1   (optional)
 *         MDV_chunk_header 2
 *           ...
 *         MDV_chunk_header n
 *         field 1 data
 *         field 2 data
 *           ...
 *         field n data
 *         chunk 1 
 *           ...
 *         chunk n
 *
 *  The master header is followed by each field header.
 *  Then follow the optional vlevel headers and chunk headers.
 *
 *  Data records for each field follow the headers.
 *  Chunk data is written last.
 * 
 *  All headers and data fields have fortran compatible record length
 *  wrappers, and can be directly read/written using fortran unformatted I/O.
 *
 *  MDV files are stored in big-endian format. On little-endian machines,
 *  the values in the file must be swapped after input and before output.
 *  The MDV library routines handle that for the user, except for chunk
 *  data which is not recognized by the MDV library.
 *
 *  Field data is stored at the offset of the given in the field header.
 *
 *  If the field data is uncompressed, it is stored as a contiguous array,
 *  starting at the lowest plane and working upwards. 
 *  The order of the data in each plane is specified by grid_order_direction
 *  and grid_order_indices in the master header.
 *
 *  If the field data is compressed, there are two 32-bit integer arrays at
 *  the start of the field data buffer, followed by the data itself.
 *
 *  The first array gives the offsets of the compressed data in the buffer,
 *  relative to the first byte after plane_n_len. In other words the offsets
 *  are calculated relative to the end of the offset and length arrays.
 *  The first offset will always be 0.
 *  The second array gives the lengths of the compressed data in the buffer.
 *  
 *        plane_1_offset
 *        plane_2_offset
 *             .
 *             .
 *        plane_n_offset
 *
 *        plane_1_len
 *        plane_2_len
 *             .
 *             .
 *        plane_n_len
 * 
 *        plane_1_compressed_data
 *        plane_2_compressed_data
 *             .
 *             .
 *        plane_n_compressed_data
 *
 *  plane_x_offset gives the offset, in bytes, from the beginning of field
 *  volume buffer. plane_x_len gives the length, in bytes,
 *  of plane_x_encoded_data.
 *
 * Supported compression types are:
 *   MDV_COMPRESSION_NONE - uncompressed
 *   MDV_COMPRESSION_RLE - see <toolsa/compress.h>
 *   MDV_COMPRESSION_LZO - see <toolsa/compress.h>
 *   MDV_COMPRESSION_GZIP - see <toolsa/compress.h>
 *   MDV_COMPRESSION_BZIP - see <toolsa/compress.h>
 */

# ifndef    MDV_FILE_H
# define    MDV_FILE_H

#include <Mdv/mdv/mdv_macros.h>
#include <dataport/port_types.h>

/*
 * Header Parameter Sizes:
 *   These values were chosen as a reasonable compromise between
 *   flexibility and efficiency.
 */

#define     MDV_CHUNK_INFO_LEN      480
#define     MDV_INFO_LEN            512
#define     MDV_LONG_FIELD_LEN       64
#define     MDV_MAX_PROJ_PARAMS       8
#define     MDV_MAX_VLEVELS         122 
#define     MDV_NAME_LEN            128
#define     MDV_SHORT_FIELD_LEN      16
#define     MDV_TRANSFORM_LEN        16
#define     MDV_UNITS_LEN            16

/*
 * Magic Cookie values and label
 */

#define MDV_FILE_LABEL "NCAR RAP MDV FILE"

#define MDV_MASTER_HEAD_MAGIC_COOKIE    14142
#define MDV_FIELD_HEAD_MAGIC_COOKIE     14143
#define MDV_VLEVEL_HEAD_MAGIC_COOKIE    14144
#define MDV_CHUNK_HEAD_MAGIC_COOKIE     14145

#define MDV_REVISION_NUMBER 1

/*
 * Number of 32 bit elements in headers (before the
 * first character).  The _32 values are used for
 * swapping the header fields and the _SI32 and_FL32
 * values are used for for copying the fields in FORTRAN
 * read and write routines.
 * Note that the _32 values should equal the corresponding
 * _SI32 value + the corresponding _FL32 value + 1. The
 * extra 1 in this formula is the FORTRAN record length
 * field at the beginning of each header.  This field is
 * swapped in an array with the rest of the header fields,
 * but is not copied into the FORTRAN arrays on reads and
 * writes. */

#define MDV_NUM_MASTER_HEADER_SI32    41
#define MDV_NUM_MASTER_HEADER_FL32    21
#define MDV_NUM_MASTER_HEADER_32      63

#define MDV_NUM_FIELD_HEADER_SI32     39
#define MDV_NUM_FIELD_HEADER_FL32     31
#define MDV_NUM_FIELD_HEADER_32       71

#define MDV_NUM_VLEVEL_HEADER_SI32   127
#define MDV_NUM_VLEVEL_HEADER_FL32   127
#define MDV_NUM_VLEVEL_HEADER_32     255

#define MDV_NUM_CHUNK_HEADER_SI32      6
#define MDV_NUM_CHUNK_HEADER_32        7

/* NOTE on FORTRAN record lengths:
 *
 * The various items which make up the MDV file are all wrapped in
 * 32-bit integers, indicating the length of the item in bytes. This
 * allows FORTRAN programs to read them directly, though it is better
 * for FORTRAN to access the files through a C/FORTRAN wrapper.
 */

/*
 * Master header Definition.  Each data file contains one master header
 * at the beginning of the file. This header is designed to be 1024 bytes
 * in total length, including the fortran record length pads. Adjust the
 * spare space if new data elements are added. Total size is 1 Kbyte.
 *
 * The FORTRAN interpretation of this record is:
 *  INTEGER DATASET_INTS(MDV_NUM_MASTER_HEADER_SI32)
 *  REAL DATASET_FLOATS(MDV_NUM_MASTER_HEADER_FL32)
 *  CHARACTER*512 DATASET_INFO
 *  CHARACTER*128 DATASET_CHARS(2)
 */

/*  Note: 
 *  The number in the comment refers to the array element number 
 *  in a FORTRAN array.
 *  All enumerated types defined in <Mdv/mdv/mdv_macros.h>. */

/*****************************************************************
 * MDV_master_header_t
 */

typedef struct {

  si32  record_len1;               /* Fortran record length field (bytes) */
  si32  struct_id;                 /* 1 Magic cookie ID for this struct */
  si32  revision_number;           /* 2 Minor revision number.  Major revisions
				      are documented by Magic cookie ID */
  
  si32  time_gen;                  /* 3 Time data generated.
				      Seconds since 1970 */
  si32  user_time;                 /* 4 Time value specific to the dataset.
				      This time value is not used by any
				      of the general MDV code, but can be
				      used by users of specific datasets.
				      This field was left here as a place-
				      holder after removing an obsolete
				      field from the header. */
  si32  time_begin;                /* 5 Begin time of dataset.  For forecast
				      datasets, this is the begin time of
				      the data used to generate the
				      forecast.
				      Seconds since 1970 */
  si32  time_end;                  /* 6 End time of dataset.  For forecast
                                         datasets, this is the end time of the
                                         data used to generate the forecast.
                                         Seconds since 1970 */
  si32  time_centroid;             /* 7 Midpoint time of dataset.  This time
				      is not necessarily directly in the
				      middle of time_begin and time_end
				      since some datasets have data skewed
				      more towards the beginning or ending
				      data time.  For forecast datasets,
				      this is the midpoint time of the data
				      used to generate the forecast.
				      Seconds since 1970*/
  si32  time_expire;               /* 8 Time dataset is no longer valid.
                                         Seconds since 1970 */
  si32  num_data_times;            /* 9 Number of data times in 
				      associated data set */
  si32  index_number;              /* 10 Index of data within 
				      associated data set */

  si32  data_dimension;            /* 11 Dimension of data 
				      (2=flat plane, 3=volume) */
  si32  data_collection_type;      /* 12 Data type (e.g. MDV_DATA_MEASURED) */
  si32  user_data;                 /* 13 Data specific to the dataset.  This
                                         value is not used by any of the
                                         general MDV code, but can be used by
                                         users of specific datasets.  This
                                         field was left here as a placeholder
                                         after removing an obsolete field from
                                         the header. */
  si32  native_vlevel_type;        /* 14 Native vertical coordinate type 
				      (e.g. MDV_VERT_TYPE_SURFACE) */
  si32  vlevel_type;               /* 15 Current vertical coordinate type 
				      (e.g. MDV_VERT_TYPE_SURFACE) */
  si32  vlevel_included;           /* 16 Flag indicating if specific vlevel 
				      info is included
				      YES(1): each field has a vlevel_hdr,
				      NO(0): no vlevel_hdrs are included */

  si32  grid_order_direction;      /* 17 Ordering of cells on map 
                                         (e.g. MDV_ORIENT_SN_WE) */
  si32  grid_order_indices;        /* 18 Ordering of cells in array 
				      (e.g. MDV_ORDER_XYZ) */
   
  si32  n_fields;                  /* 19 Number of fields in this data set */

  si32  max_nx;                    /* 20 Max number of array elements in 
                                      x direction */
  si32  max_ny;                    /* 21 Max number of array elements in 
				      y direction */
  si32  max_nz;                    /* 22 Max number of array elements in 
				      z direction */

  si32  n_chunks;                  /* 23 Number of chunks in this file */
    
  /* The following offsets are all in bytes from beginning of file */

  si32  field_hdr_offset;          /* 24 fseek offset to first field header */
  si32  vlevel_hdr_offset;         /* 25 fseek offset to first vlevel header */
  si32  chunk_hdr_offset;          /* 26 fseek offset to first chunk header */

  si32  field_grids_differ;        /* 27 Flag indicating if the data grids for
                                         all of the fields differ in any way.
                                         YES(1): the field grids differ
                                         NO(0): the field grids are all the
                                                same (so some processing may be
                                                simplified */
 
  si32  user_data_si32[8];        /* 28-35 User defined data particular to a
				     dataset.  These values are not used
				     by any general MDV processing but
				     are guaranteed to be available to
				     users for their own purposes. */

  si32  unused_si32[6];           /* 36-41 To fill out record to 42 si32's
				     (including record_len1) */

  /* Note about Grids: */
  /* Points are located at center of grid box */ 
  /* Units defined by projection) */

  fl32  user_data_fl32[6];         /* 1-6 User defined data particular to a
				      dataset.  These values are not used
				      by any general MDV processing but
				      are guaranteed to be available to
				      users for their own purposes. */

  fl32  sensor_lon;                /* 7 origin of sensor (degrees) */
  fl32  sensor_lat;                /* 8 origin of sensor (degrees) */
  fl32  sensor_alt;                /* 9 origin of sensor (degrees) */
    
  fl32  unused_fl32[12];           /* 10-21 to fill out record to 21 fl32's */

  char  data_set_info[MDV_INFO_LEN];   /* (512 bytes) */
  char  data_set_name[MDV_NAME_LEN];   /* (128 bytes)*/

  char  data_set_source[MDV_NAME_LEN]; /* Where the data came from */

  si32  record_len2;               /* Fortran record length field (in bytes) */

}  MDV_master_header_t;

/**********************************************************************
 * MDV_field_header definition.  
 *
 * Each field has its own header, 416 bytes in length.
 * If you want to add items, replace a spare. Do not change the length.
 *
 * If a field requires additional vlevel information, then each field must 
 * have an associated vlevel header as well.
 *
 * Supported encoding types are:
 *   MDV_NATIVE  - no change
 *   MDV_INT8
 *   MDV_INT16
 *   MDV_FLOAT32
 *
 * Supported compression types are:
 *   MDV_COMPRESSION_ASIS - no change
 *   MDV_COMPRESSION_NONE - uncompressed
 *   MDV_COMPRESSION_RLE - see <toolsa/compress.h>
 *   MDV_COMPRESSION_LZO - see <toolsa/compress.h>
 *   MDV_COMPRESSION_GZIP - see <toolsa/compress.h>
 *   MDV_COMPRESSION_BZIP - see <toolsa/compress.h>
 *
 * Scaling types apply only to int types (INT8 and INT16)
 *
 * Supported scaling types are:
 *   MDV_SCALING_DYNAMIC
 *   MDV_SCALING_ROUNDED
 *   MDV_SCALING_INTEGRAL
 *   MDV_SCALING_SPECIFIED
 *
 * For MDV_SCALING_DYNAMIC, the scale and bias is determined from the
 * dynamic range of the data.
 *
 * For MDV_SCALING_ROUNDED, the operation is similar to MDV_SCALING_DYNAMIC,
 * except that the scale and bias are constrained to round to 0.2, 0.5 or
 * 1.0 multiplied by a power of 10.
 * 
 * For MDV_SCALING_INTEGRAL, the operation is similar to MDV_SCALING_DYNAMIC,
 * except that the scale and bias are constrained to integral values.
 * 
 * For MDV_SCALING_SPECIFIED, the specified scale and bias are used.
 *
 * Output scale and bias are ignored for conversions to float, and
 * for MDV_SCALING_DYNAMIC and MDV_SCALING_INTEGRAL.
 *
 * The FORTRAN interpretation of this record is:
 *  INTEGER FIELD_INTS(39)
 *  REAL    FIELD_REALS(31)
 *  CHARACTER*64 LONG_FIELD_NAME
 *  CHARACTER*16 FIELD_CHARS(4)
 */

/* MDV_field_header_t */

typedef struct {

  /* ints */

  si32  record_len1;               /* Fortran record length field (in bytes) */
  si32  struct_id;                 /* 1 Magic cookie ID for this struct */
     
  si32  field_code;                /* 2 MDV Field Code (taken from GRIB,
				      defined in <Mdv/mdv/mdv_field_codes.h>) */

  si32  user_time1;                /* 3 Time value specific to the field.
				      This time value is not used by any
				      of the general MDV code, but can be
				      used by users of specific datasets.
				      This field was left here as a place-
				      holder after removing an obsolete
				      field from the header. */
  si32  forecast_delta;            /* 4 Time interval, in seconds, of
				      forecast.  This is the number of
				      seconds since the data midpoint
				      (time_centroid in the master header)
				      this forecast field represents.  This
				      value should be the same as
				      forecast_time - time_centroid. If this
				      field is not a forecast field, this
				      value should be 0. */
  si32  user_time2;                /* 5 Time value specific to the field.
				      This time value is not used by any
				      of the general MDV code, but can be
				      used by users of specific datasets.
				      This field was left here as a place-
				      holder after removing an obsolete
				      field from the header. */
  si32  user_time3;                /* 6 Time value specific to the field.
				      This time value is not used by any
				      of the general MDV code, but can be
				      used by users of specific datasets.
				      This field was left here as a place-
				      holder after removing an obsolete
				      field from the header. */
  si32  forecast_time;             /* 7 Time of forecast the data in this
				      field represents.  If this field does
				      not represent forecast data, this time
				      should be the same as time_centroid
				      in the master header. */
  si32  user_time4;                /* 8 Time value specific to the field.
				      This time value is not used by any
				      of the general MDV code, but can be
				      used by users of specific datasets.
				      This field was left here as a place-
				      holder after removing an obsolete
				      field from the header. */

  si32   nx;                       /* 9 Number of points in X direction */
  si32   ny;                       /* 10 Number of points in Y direction */
  si32   nz;                       /* 11 Number of points in Z direction */
 
  si32   proj_type;                /* 12 Projection Type (e.g.
				      MDV_PROJ_LATLON) */

  si32   encoding_type;            /* 13 Type of data encoding (e.g.
				      MDV_INT8, MDV_FLOAT32) */

  si32   data_element_nbytes;      /* 14 Size of each data element 
				     (in bytes) */

  si32   field_data_offset;        /* 15 fseek offset to start of field data
				      Field data may be encoded and
				      stored in the MDV compression format.
				      In this case the offset is to the 
				      beginning of nplane offset's and 
				      nplanes length's.  Each offset and 
				      plane element is a ui32 */

  si32   volume_size;              /* 16 Size of data volume in bytes.  Does
				      not include Fortran record length
				      values that surround the field data.
				      If the data is compressed (encoding
				      type is MDV_PLANE_RLE8 or
				      MDV_ROW_RLE8), the volume size
				      includes the length and offset arrays
				      included at the front of the encoded
				      data. */

  si32   user_data_si32[10];       /* 17-26 User defined data particular to a
				      field.  These values are not used
				      by any general MDV processing but
				      are guaranteed to be available to
				      users for their own purposes. */
  
  si32   compression_type;         /* 27: data compression type used
				      e.g. MDV_COMPRESSION_NONE,
				      MDV_COMPRESSION_RLE, etc */

  si32   transform_type;           /* 28: data transform type used
				      e.g. MDV_TRANSFORM_NONE,
				      MDV_TRANSFORM_NATLOG, etc */

  si32   scaling_type;             /* 29: data scaling method used
				      e.g. MDV_SCALING_DYNAMIC,
				      MDV_SCALING_INTEGRAL, etc
				      Only applicable for integral
				      encoding types */
  
  si32  native_vlevel_type;       /* 30 Native vertical coordinate type 
				   * (e.g. VERT_TYPE_SURFACE) */

  si32  vlevel_type;              /* 31 Current vertical coordinate type 
				   * (e.g. VERT_TYPE_SURFACE) */

  si32 dz_constant;               /* 32 - flag to indicate constant dz
				   * not in use yet */

  si32 unused_si32[7];            /* 33-39 Spare, set to 0 */

  /* floats */
    
  fl32   proj_origin_lat;          /* 1 Projection origin 
				      (deg, south lat's negative)*/

  fl32   proj_origin_lon;          /* 2 Projection origin 
				      (deg, west lon's negative)*/

  fl32   proj_param[MDV_MAX_PROJ_PARAMS];/* 3-10 Projection information, these
					    values are specific to the
					    proj_type specified above. */
  /* Note: proj_rotation follows later */

  fl32   vert_reference;           /* 11 Vertical coordinate reference value */

  fl32   grid_dx;                  /* 12 X dimension grid spacing 
				     (units determined by proj_type)*/
  fl32   grid_dy;                  /* 13 Y dimension grid spacing 
				     (units determined by proj_type)*/
  fl32   grid_dz;                  /* 14 Z dimension grid spacing 
				     (units determined by vlevel_type)*/
  fl32   grid_minx;                /* 15 Starting point of grid 
				     (units determined by proj_type)  */
  fl32   grid_miny;                /* 16 Starting point of grid 
				     (units determined by proj_type)  */
  fl32   grid_minz;                /* 17 Starting point of grid 
				     (units determined by vlevel_type)  */

  fl32   scale;                    /* 18 Scale factor for data values
				    * Applies to INT8 and INT16 only */ 
   
  fl32   bias;                     /* 19 Bias applied to data values  
				    * Applies to INT8 and INT16 only */

  /* NOTE: To get float data from INT8 and INT16,
     float_value = scale * integer_value + bias */

  fl32   bad_data_value;           /* 20 Data with this value (BEFORE applying
				      scale and bias) not valid */
  fl32   missing_data_value;       /* 21 Data with this value (BEFORE applying
				      scale and bias) not measured */

  fl32   proj_rotation;            /* 22 Projection rotation in degrees */

  fl32   user_data_fl32[4];        /* 23-26 User defined data particular to a
				      field.  These values are not used
				      by any general MDV processing but
				      are guaranteed to be available to
				      users for their own purposes. */

  fl32   min_value;                /* 27 - min val in data set */
  fl32   max_value;                /* 28 - max val in data set */
  fl32   unused_fl32[3];           /* 29-31 Spare, to fill out array 
				      to 31 fl32's */

  /* chars */

  char   field_name_long[MDV_LONG_FIELD_LEN]; /* Long field name (64 bytes) */
  char   field_name[MDV_SHORT_FIELD_LEN];     /* Short field name (16 bytes) */
  char   units[MDV_UNITS_LEN];                /* Units label (16 bytes) */
  char   transform[MDV_TRANSFORM_LEN];        /* Data transformation type 
						 (16 bytes) */
  char   unused_char[MDV_UNITS_LEN];          /* Spare 16 bytes */

  si32   record_len2;                         /* Fortran record length field */

} MDV_field_header_t;

/**********************************************************************
 * MDV_vlevel_header
 *
 * A vlevel_header exists when more information about vertical levels 
 * are needed.  If it is used, the master header's vlevel_included 
 * flag must be set to 1 and a vlevel_header must be included for every
 * field in the file. This structure is 1 kbyte and all vlevel 
 * headers should follow the field headers.
 *
 * The FORTRAN interpretation of this record is:
 *  INTEGER STRUCT_ID
 *  INTEGER VLEVEL_TYPE(MDV_MAX_VLEVELS)
 *  INTEGER UNUSED(4)
 *  REAL    VLEVEL_PARAMS(MDV_MAX_VLEVELS)
 *  REAL RUNUSED(5)
 */

typedef struct {

  si32  record_len1;                    /* Fortran record length field 
					   (in bytes) */
  si32  struct_id;                      /* 1 Magic cookie ID for this struct */
  
  si32   vlevel_type[MDV_MAX_VLEVELS];  /* 2-123 Enumerated type of vertical 
					   coordinate system */
  
  si32   unused_si32[4];                /* 124-127 */

  fl32   vlevel_params[MDV_MAX_VLEVELS];/* 1-122 Additional information about 
					   vertical level */

  fl32   unused_fl32[5];                /* 123-127 */

  si32   record_len2;                   /*  Fortran record length field */

} MDV_vlevel_header_t;

/*************************************************************************
 * MDV_chunk_header
 *
 * A chunk header provides information about a "chunk" of data.  Chunk data 
 * is not designed to conform to any standard so the writer of "chunk" data is 
 * responsible for knowing how it was written, and therefore, how it should be 
 * read. Note that there is space for ascii text about chunk data.
 * A chunk header is 512 bytes.
 *
 * The MDV library has access to routines which can interpret certain
 * chunk data, identified through the chunk ID.
 *
 * The FORTRAN interpretation of this record is:
 *  INTEGER STRUCT_ID
 *  INTEGER CHUNK_ID
 *  INTEGER CHUNK_DATA_OFFSET
 *  INTEGER SIZE
 *  INTEGER UNUSED(2)
 *  CHARACTER*480 CHUNK_INFO
 */

typedef struct {

  si32  record_len1;                    /* Fortran record length field 
					   (in bytes)*/
  si32  struct_id;                      /* 1 Magic cookie ID for this struct */
     
  si32   chunk_id;                      /* 2 A value identifying the data
					   in this chunk.  Chunk data that
					   we know about are defined in
					   <Mdv/mdv/mdv_macros.h as
					   MDV_CHUNK_xxx */
  si32   chunk_data_offset;             /* 3 fseek offset to start of 
					   chunk data */ 
  si32   size;                          /* 4 Chunk size (in bytes) */

  si32   unused_si32[2];                /* 5-6 Unused ints */

  char   info[MDV_CHUNK_INFO_LEN];      /* ascii info about chunk data */

  si32   record_len2;                   /* Fortran record length field */

} MDV_chunk_header_t;

/************************************************************************
 * The MDV_field_vlevel_header_t is a container.
 * It contains pointers to a field header and a vlevel header 
 */

typedef struct {
  MDV_field_header_t   *fld_hdr;       /* A field header struct */
  MDV_vlevel_header_t  *vlv_hdr;       /* A vlevel header struct */
} MDV_field_vlevel_header_t;


/**************************************************************************
 * MDV_plane_info parameters are used for message passing in data requests.
 *
 * Size is 64 bytes
 */
 
typedef struct {

  si32   struct_id;          /* 1 Magic cookie ID for this struct */
  
  si32   plane_no;           /* 2 Current vertical level (plane) number */
  si32   plane_type;         /* 3 Current vertical level type 
				(enumerated type) */
  si32   plane_size;         /* 4 Size of this vertical level (in bytes) */
 
  si32   plane_x1;           /* 5 Current vertical level starting x location */
  si32   plane_y1;           /* 6 Current vertical level starting y location */
  si32   plane_z1;           /* 7 Current vertical level starting z location */
 
  si32   plane_x2;           /* 8 Current vertical level ending x location */
  si32   plane_y2;           /* 9 Current vertical level ending y location */
  si32   plane_z2;           /*10 Current vertical level ending z location */
 
  si32   unused_si32[5];     /*11-15 Spare, to fill out array to 15 si32's */
    
  /* floats */
 
  fl32   plane_param;        /* 1 Current vertical level parameter 
				information */

} MDV_plane_info_t;


/*************************************************************************
 * The MDV_field_message_t is a utility structure for message passing.  It 
 * contains pointers to a field header struct and a plane_info struct.
 */

typedef struct {
  MDV_field_header_t          *fld_hdr;
  MDV_plane_info_t            *plane_info;
} MDV_field_message_t;


/*
 * The MDV_dataset_t is a container class for an entire dataset. It
 * contains a pointer to a master header, a pointer to the beginning of
 * n_fields field headers, a pointer to the beginning of n_fields vlevel
 * headers (or NULL if no vlevel headers exist), a pointer to the
 * beginning of n_chunks chunk headers, and then pointers to each plane
 * in each field and to chunck data.  Also contained is the record
 * keeping variables nfields_alloc and nchunks_alloc so that space may be
 * realloced if necessary.  The datafile_buf pointer points to beginning
 * of the datafile.
 */

typedef struct {
  MDV_master_header_t         *master_hdr; 	
  MDV_field_header_t         **fld_hdrs;    
  MDV_vlevel_header_t        **vlv_hdrs;    
  MDV_chunk_header_t         **chunk_hdrs;  
  void                      ***field_plane;
  void                       **chunk_data;
  int                          nfields_alloc;		               
  int                          nchunks_alloc;		               
  char                        *datafile_buf;
} MDV_dataset_t;


# endif     /* MDV_FILE_H */

#ifdef __cplusplus
}
#endif
