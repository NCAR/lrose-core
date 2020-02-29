// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
////////////////////////////////////////////////
//
// Mdvx_enums.hh
//
// Enum typedefs for Mdvx class
//
////////////////////////////////////////////////

// This header file can only be included from within Mdvx.hh
#ifdef _in_Mdvx_hh

////////////////////////////////////////////
// Data format type
// How the data is represented, either in
// this object or in a file

typedef enum {

  FORMAT_MDV     =  0,  // normal mdvx representation
  FORMAT_XML     =  1,  // XML
  FORMAT_NCF     =  2,  // NETCDF CF format
  FORMAT_RADX    =  3   // Radial radar data types

} mdv_format_t;

////////////////////////////////////////////
// Data collection type
// How the data were collected or generated.

typedef enum {

  DATA_MEASURED =     0,  // from some instrument
  DATA_EXTRAPOLATED = 1,  // based on measured data
  DATA_FORECAST =     2,  // Numerically modeled
  DATA_SYNTHESIS =    3,  // A synthesis of modeled, measured data
  DATA_MIXED =        4,  // Different types of data in dataset
  DATA_IMAGE =        5,  // A Photograph - RGB Image
  DATA_GRAPHIC =      6,  // A Syntheticly Rendered RGB Image
  DATA_CLIMO_ANA =    7,  // A Climotology (aggregation of data) from Model Analyses.
  DATA_CLIMO_OBS =    8   // A Climotology (aggregation of data) from Observational Data.
} data_collection_type_t;

///////////////////////////////////////////
// Order of grid points on map.
// How the array is positioned on the earth. 
// Note: ORIENT_SN_WE is always supported

typedef enum {

  ORIENT_OTHER = 0,  // unsupported grid

  ORIENT_SN_WE = 1,  // south-north and west-east
                     // data varies faster west to east (x),
                     // slower north to south (y)
                     // This is the standard ordering

  ORIENT_NS_WE = 2,  // north-south and west-east
  ORIENT_SN_EW = 3,  // south-north and east-west
  ORIENT_NS_EW = 4   // north-south and east-west

} grid_order_direction_t;

////////////////////////
// grid projection type

typedef enum {

  PROJ_NATIVE =           -1,  // Same as grid from which data is derived.
                               // Not used much.

  PROJ_LATLON =            0,  // x,y in degrees.
                               // z defined by vert proj type

  PROJ_ARTCC =             1,  // x,y in km

  PROJ_DO_NOT_USE =        2,  // deprecated - used to be STEREOGRAPHIC

  PROJ_LAMBERT_CONF =      3,  // x,y in km

  PROJ_MERCATOR =          4,  // x,y in km

  PROJ_POLAR_STEREO =      5,  // x,y in km

  PROJ_POLAR_ST_ELLIP =    6,  // x,y in km

  PROJ_CYL_EQUIDIST =      7,  // x,y in km

  PROJ_FLAT =              8,  // Cartesian, x,y in km. Azimuthal Equidistant
                               // z defined by vert proj type

  PROJ_POLAR_RADAR =       9,  // Radial range, Azimuth angle, Elev angle
                               // x is gate spacing in km .
                               // y is azimuth in degrees, from
                               //   true north, + is clockwise
                               // z is elevation angle in degrees. 

  PROJ_RADIAL =           10,  // = Radius, Meters,
                               // y = azimuth in degrees
                               // z = Defined by VERT_TYPE...

  PROJ_VSECTION =         11,  // vertical section

  PROJ_OBLIQUE_STEREO =   12,  // x,y in km

  PROJ_RHI_RADAR =        13,  // Radial range, Elev angle, Az angle
                               // x is gate spacing in km .
                               // y is elevation in degrees.
                               // z is azimuth angle in degrees, from
                               //   true north. + is clockwise.

  PROJ_TIME_HEIGHT =      14,  // time-height profile
                               // x is time
                               // ny is 1
                               // z is height

  PROJ_TRANS_MERCATOR =   15,  // Transverse Mercator
                               // X (E-W) , y (N-S) in km.

  PROJ_ALBERS =           16,  // Albers Equal Area, x,y in km

  PROJ_LAMBERT_AZIM =     17,  // Lambert Azimuthal Equal Area, x,y in km

  PROJ_VERT_PERSP =       18,  // Vertical perspective (satellite view)

  PROJ_UNKNOWN =          99

} projection_type_t;

////////////////////////////////////////////////
// differentiates between polar stereographic
// with tangent point at North and South poles
//
typedef enum {
  POLE_NORTH = 0,
  POLE_SOUTH = 1
} pole_type_t;

//////////////////////////////////////////
// order of data points in storage arrays
// Note: ORDER_XYZ must be supported

typedef enum {

  ORDER_XYZ  =             0,
  ORDER_YXZ =              1,
  ORDER_XZY =              2,
  ORDER_YZX =              3,
  ORDER_ZXY =              4,
  ORDER_ZYX  =             5

} grid_order_indices_t;

////////////////////////////////
// Type for vertical coordinate

typedef enum {

  VERT_TYPE_UNKNOWN =      0,
  VERT_TYPE_SURFACE =      1,  // Earth surface field
  VERT_TYPE_SIGMA_P =      2,  // Sigma pressure levels
  VERT_TYPE_PRESSURE =     3,  // Pressure levels, units = mb
  VERT_TYPE_Z =            4,  // Constant altitude, units = Km MSL
  VERT_TYPE_SIGMA_Z =      5,  // Model sigma Z levels
  VERT_TYPE_ETA =          6,  // Model eta levels
  VERT_TYPE_THETA =        7,  // Isentropic surface, units = Kelvin
  VERT_TYPE_MIXED =        8,  // Any hybrid grid
  VERT_TYPE_ELEV =         9,  // Elevation angles - radar
  VERT_TYPE_COMPOSITE =   10,  // A Composite of a set of planes
  VERT_TYPE_CROSS_SEC =   11,  // Cross sectional view of a 
                               // set of planes
  VERT_SATELLITE_IMAGE =  12,  // Satelite Image
  VERT_VARIABLE_ELEV =    13,  // variable elevation scan
  VERT_FIELDS_VAR_ELEV =  14,  // variable elevation scan, field specific
  VERT_FLIGHT_LEVEL =     15,  // ICAO flight level (100's of ft)
  VERT_EARTH_IMAGE =      16,  // Image, conformal to the surface of the earth 
  VERT_TYPE_AZ =          17,  // Azimuth angles - radar RHI
  VERT_TYPE_TOPS =        18,  // Echo tops etc
  VERT_TYPE_ZAGL_FT =     19,  // Constant altitude above ground, units = ft
  VERT_SOIL =     	  20,  // Soil levels in negative KM 
  VERT_TYPE_WRF_ETA =     21,  // Eta levels as used in the WRF ARW model

  VERT_TYPE_VARIABLE =    99   // for master hdr if variable types in fields

} vlevel_type_t;

/////////////////////////////////////////////
// Binary encoding of the data in the arrays.

typedef enum {

  ENCODING_ASIS =      0,  // No change
                           // used in function args only,
                           // not in file storage
  ENCODING_INT8 =      1,  // unsigned 8 bit IEEE integer 
  ENCODING_INT16 =     2,  // unsigned 16 bit IEEE integer
  ENCODING_FLOAT32 =   5,  // 32 bit IEEE float
						   
  ENCODING_RGBA32 =    7,  // RGBA image (4 x 8 bits) - Exactly like TIFF RGBA
						   // See 'man TIFFReadRGBAImage'

  // The following encoding and compression format, MDV_PLANE_RLE8, is
  // equivalent to INT8 and COMPRESSION_RLE.
  // It's use is deprecated, but it is included for backward compatibility.
  // When the library finds this encoding type, it internally represents it
  // as INT8 and COMPRESSION_RLE.

  PLANE_RLE8 = 10 // INT8 with COMPRESSION_RLE

} encoding_type_t;

// Macros to Extract Red, Green Blue, Alpha channels from ENCODING_RGBA32
#define MdvGetR(abgr) ((abgr) & 0xff)
#define MdvGetG(abgr) (((abgr) >> 8) & 0xff)
#define MdvGetB(abgr) (((abgr) >> 16) & 0xff)
#define MdvGetA(abgr) (((abgr) >> 24) & 0xff)

// data compression types
//
// See <toolsa/compress for details on the compression types
//
// Most compression is done on a per-plane basis, with one compressed
// buffer per plane. GZIP_VOL compressed the entire volume in a single
// compressed buffer. This is especially suitable for vertical sections
// and time-height data.

typedef enum {

  COMPRESSION_ASIS = -1,  // not for file storage - only for
                          // args in functions

  COMPRESSION_NONE =  0,  // no compression
  COMPRESSION_RLE =   1,  // run-length encoding
  COMPRESSION_LZO =   2,  // Lempel-Ziv-Oberhaumer
  COMPRESSION_ZLIB =  3,  // Lempel-Ziv
  COMPRESSION_BZIP =  4,  // bzip2
  COMPRESSION_GZIP =  5,  // Lempel-Ziv in gzip format
  // Gzip compression using a single buffer for the volume
  // instead of one compressed buffer per plane
  COMPRESSION_GZIP_VOL =  6,
  COMPRESSION_TYPES_N = 7
  
} compression_type_t;

////////////////////////////////////////////////////////////////////
// data transform types
//
// If NONE, the data is interpreted as-is.
// If the data is transformed (eg LOG) the inverse transform must be
// applied before interpreting the floating point data.

typedef enum {

  DATA_TRANSFORM_NONE =    0,  // Depicts an area or volume in space.
  DATA_TRANSFORM_LOG =     1,   // natural log
                               // forward transform log()
                               // inverse transform exp()
  DATA_TRANSFORM_POINT =    2, // Depicts a point in space.
  DATA_TRANSFORM_SUM =		3, // Sum Of values 
  DATA_TRANSFORM_DIFF =		4, // Difference Of values 
  DATA_TRANSFORM_PROD =		5, // Product Of values 
  DATA_TRANSFORM_RATIO =	6, // Ratio Of values 
  DATA_TRANSFORM_MAXIMUM =	7, // Maximum Of values 
  DATA_TRANSFORM_MINIMUM =	8, // Minimum Of values 
  DATA_TRANSFORM_MEAN =		9, // Mean (Average) 
  DATA_TRANSFORM_MEDIAN =	10, // Median 
  DATA_TRANSFORM_MODE =		11, // Mode 
  DATA_TRANSFORM_MID_RANGE= 12, // Average of maximum and minimum
  DATA_TRANSFORM_STDDEV =	13, // Standard deviation
  DATA_TRANSFORM_VAR =		14, // Variance 
  DATA_TRANSFORM_COVAR =	15, // Covariance
  DATA_TRANSFORM_NORM =		16 // Normalized data
} transform_type_t;

///////////////////////////////////////////////////////////////////
// scaling control (when converting from float to int types)
//
// When converting from float to an int type, data scaling must
// be performed.
//
// The following scaling schemes are supported:
//
//  (a) dynamic - scale and bias computed from the data range
//  (b) dynamic_integral - scale and bias computed from the data range,
//      but scale and bias constrained to be integral values
//  (c) specified - scale and bias specified.

typedef enum {

  SCALING_NONE =       0,
  SCALING_ROUNDED =    1,
  SCALING_INTEGRAL =   2,
  SCALING_DYNAMIC =    3,
  SCALING_SPECIFIED =  4

} scaling_type_t;

//////////////////////////////////////////////////////////////
// chunk IDs - each chunk type is identified by an integer ID
//
// NOTE: DATA_SET_INFO chunk is used if the info string exceeds
// a length of 512 bytes. In that case, the data_set_info in
// the master header is truncated, and the full string is
// added as a chunk.

typedef enum {

  CHUNK_DOBSON_VOL_PARAMS =         0,
  CHUNK_DOBSON_ELEVATIONS =         1,
  CHUNK_NOWCAST_DATA_TIMES  =       2,
  CHUNK_DSRADAR_PARAMS =            3,
  CHUNK_DSRADAR_ELEVATIONS =        4,
  CHUNK_VARIABLE_ELEV  =            5,
  CHUNK_SOUNDING_DATA  =            6,
  CHUNK_DSRADAR_AZIMUTHS =          7,
  CHUNK_TEXT_DATA =                 8,
  CHUNK_CLIMO_INFO =                9,
  CHUNK_DSRADAR_CALIB =             10,
  CHUNK_COMMENT =                   11,
  CHUNK_DATA_SET_INFO =            100,
  CHUNK_VSECT_WAY_PTS_32 =       50200,
  CHUNK_VSECT_SAMPLE_PTS_32 =    50700,
  CHUNK_VSECT_SEGMENTS_32 =      50710,
  CHUNK_VSECT_WAY_PTS_64 =       54200,
  CHUNK_VSECT_SAMPLE_PTS_64 =    54700,
  CHUNK_VSECT_SEGMENTS_64 =      54710
 
} chunk_id_t;

//////////////////////////////////////////////////////
// Specifying read search mode in the time domain
//
// Transient - not stored in any file.

typedef enum {

  READ_LAST               = 0,
  READ_CLOSEST            = 1,
  READ_FIRST_BEFORE       = 2,
  READ_FIRST_AFTER        = 3,
  READ_BEST_FORECAST      = 4,
  READ_SPECIFIED_FORECAST = 5

} read_search_mode_t;
  
//////////////////////////////////////////////////////
// Specifying mode for time lists
//
// Transient - not stored in any file.

typedef MdvxTimeList::time_list_mode_t time_list_mode_t;
  
typedef enum {
  CLIMO_TYPE_MEAN,
  CLIMO_TYPE_STD_DEV,
  CLIMO_TYPE_MAX,
  CLIMO_TYPE_MIN,
  CLIMO_TYPE_NUM_OBS,
  CLIMO_TYPE_NUM_OBS_GT,
  CLIMO_TYPE_NUM_OBS_GE,
  CLIMO_TYPE_NUM_OBS_LT,
  CLIMO_TYPE_NUM_OBS_LE,
  CLIMO_TYPE_MAX_DATE,
  CLIMO_TYPE_MIN_DATE,
  CLIMO_TYPE_NUM_TIMES,
  CLIMO_TYPE_PERCENT,
  CLIMO_TYPE_PERCENT_GT,
  CLIMO_TYPE_PERCENT_GE,
  CLIMO_TYPE_PERCENT_LT,
  CLIMO_TYPE_PERCENT_LE
} climo_type_t;
  
#endif
