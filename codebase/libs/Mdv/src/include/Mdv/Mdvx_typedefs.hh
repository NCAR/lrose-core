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
// Mdvx_typedefs.hh
//
// Typedefs for Mdvx class
//
////////////////////////////////////////////////

// This header file can only be included from within Mdvx.hh
#ifdef _in_Mdvx_hh

#include <Mdv/Mdvx_typedefs_32.hh>
#include <Mdv/Mdvx_typedefs_64.hh>

// Defines for header string and array sizes.
// These should not be changed since that would alter the file format.

#define     MDV_CHUNK_INFO_LEN      MDV64_CHUNK_INFO_LEN
#define     MDV_INFO_LEN            MDV64_INFO_LEN
#define     MDV_LONG_FIELD_LEN      MDV64_LONG_FIELD_LEN
#define     MDV_MAX_PROJ_PARAMS     MDV64_MAX_PROJ_PARAMS
#define     MDV_MAX_VLEVELS         MDV64_MAX_VLEVELS
#define     MDV_NAME_LEN            MDV64_NAME_LEN
#define     MDV_SHORT_FIELD_LEN     MDV64_SHORT_FIELD_LEN
#define     MDV_TRANSFORM_LEN       MDV64_TRANSFORM_LEN
#define     MDV_UNITS_LEN           MDV64_UNITS_LEN
#define     MDV_N_COORD_LABELS      MDV64_N_COORD_LABELS
#define     MDV_COORD_UNITS_LEN     MDV64_COORD_UNITS_LEN


/////////////////////
// heartbeat function

typedef void (*heartbeat_t)(const char *label);


//////////////////
// master_header_t

typedef master_header_64_t master_header_t;

///////////////////////////////////////////////////////////////////////////
// field_header_t
//
// Each field has its own header, 416 bytes in length.
// If you want to add items, replace a spare. Do not change the length.
//
// If a field requires additional vlevel information, then each field must 
// have an associated vlevel header as well.
//
// Supported encoding types are:
//  INT8
//   INT16
//   FLOAT32
//
// Supported compression types are:
//   COMPRESSION_ASIS - no change
//   COMPRESSION_NONE - uncompressed
//   COMPRESSION_RLE - see <toolsa/compress.h>
//   COMPRESSION_LZO - see <toolsa/compress.h>
//   COMPRESSION_GZIP - see <toolsa/compress.h>
//   COMPRESSION_BZIP - see <toolsa/compress.h>
//
// Scaling types apply only to int types (INT8 and INT16)
//
// Supported scaling types are:
//   SCALING_DYNAMIC
//   SCALING_ROUNDED
//   SCALING_INTEGRAL
//   SCALING_SPECIFIED
//
// For SCALING_DYNAMIC, the scale and bias is determined from the
// dynamic range of the data.
//
// For SCALING_ROUNDED, the operation is similar to SCALING_DYNAMIC,
// except that the scale and bias are constrained to round to 0.2, 0.5 or
// 1.0 multiplied by a power of 10.
// 
// For SCALING_INTEGRAL, the operation is similar to SCALING_DYNAMIC,
// except that the scale and bias are constrained to integral values.
// 
// For SCALING_SPECIFIED, the specified scale and bias are used.
//
// Output scale and bias are ignored for conversions to float, and
// for SCALING_DYNAMIC and SCALING_INTEGRAL.
//

typedef field_header_64_t field_header_t;

///////////////////////////////////////////////////////////////////////
// MDV_vlevel_header
//
// A vlevel_header exists when more information about vertical levels 
// are needed.  If it is used, the master header's vlevel_included 
// flag must be set to 1 and a vlevel_header must be included for every
// field in the file. This structure is 1 kbyte and all vlevel 
// headers should follow the field headers.
//

typedef vlevel_header_64_t vlevel_header_t;

//////////////////////////////////////////////////////////////////////////////
// MDV_chunk_header
//
// A chunk header provides information about a "chunk" of data.  Chunk
// data is not designed to conform to any standard so the writer of
// "chunk" data is responsible for knowing how it was written, and
// therefore, how it should be read. Note that there is space for
// ascii text about chunk data.
//
// A chunk header is 512 bytes in length.
//
// The MDV library has access to routines which can interpret certain
// chunk data, identified through the chunk ID.
//

typedef chunk_header_64_t chunk_header_t;

///////////////////////////////
// Vertical sections

// structures used in Mdvx class vectors for storing vertical section
// information

typedef struct {
  double lat;
  double lon;
} vsect_waypt_t;

typedef struct {
  double lat;
  double lon;
  int segNum;
} vsect_samplept_t;

typedef struct {
  double length;
  double azimuth;
} vsect_segment_t;

// structures used in Mdvx chunks and DsMdvMsg for storing vertical section
// information 

typedef chunkVsectWayPtHdr_64_t chunkVsectWayPtHdr_t;
typedef chunkVsectWayPt_64_t chunkVsectWayPt_t;
typedef chunkVsectSamplePtHdr_64_t chunkVsectSamplePtHdr_t;
typedef chunkVsectSamplePt_64_t chunkVsectSamplePt_t;
typedef chunkVsectSegmentHdr_64_t chunkVsectSegmentHdr_t;
typedef chunkVsectSegment_64_t chunkVsectSegment_t;

//////////////////////////////////////////////////////////////////
// projection and coordinates struct
//
// This is an alternative representation of the coord information
// in the field headers.
//
// This struct is not stored in the Mdv files directly.
// It is used to store mdv grid information in related data files,
// which make use of Mdv data but for which it does not make
// sense to store the entire field header.
//
// This struct is also used to pass projection information
// between the DsMdvServer and clients. Its size should be kept
// constant for backwards compatibility.

typedef flat_params_64_t flat_params_t;
typedef albers_params_64_t albers_params_t;
typedef lc2_params_64_t lc2_params_t;
typedef os_params_64_t os_params_t;
typedef ps_params_64_t ps_params_t;
typedef trans_merc_params_64_t trans_merc_params_t;
typedef vert_persp_params_64_t vert_persp_params_t;

// coordinate structure
    
typedef coord_64_t coord_t;
  
#endif
