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
/************************************************************************
 * NIDSDATA.H Structures & definitions for nids format files 
 * F. Hage Oct 1998.  From Jim Coowie Example - NCAR/COMET
 */

#ifndef NIDS_DATA_H
#define NIDS_DATA_H

#include <Radx/Radx.hh>
#include <cstdio>
#include <sys/types.h>
class RadxBuf;

/*
 * Maximums for NIDS products
*/

#define _NIDS_MXBINS          460
#define _NIDS_MXROWLEN        920
#define _NIDS_MXRLELEN        230
#define _NIDS_MXRADIALS       400

/* NIDS Messgae Header - This is Common to all NIDS PRODUCTS ?*/
typedef struct {
  Radx::si16   mcode;    /* Message Code */
  Radx::si16   mdate;    /* Message Date - Days since 12/31/69 */
  Radx::si32   mtime;    /* Seconds after 12AM GMT */
  Radx::si32   mlength;  /* Message Lengths - Bytes, including header */
  Radx::si16   msource;  /* Source ID of Sender */
  Radx::si16   mdest;    /* Destination ID */
  Radx::si16   nblocks;  /* Number of blocks in the messge */
  Radx::si16   divider;  /* Should be 0xffff */
  Radx::si32   lat;      /* Lat of Radar degrees * 1000 */
  Radx::si32   lon;      /* Long of Radar degrees * 1000 */
  Radx::si16   height;   /* Height of Radar - FEET MSL */
  Radx::si16   pcode;    /* Nids Product Code */
  Radx::si16   mode;     /*  0=Maint, 1 = Clear Air, 2=Precip */
  Radx::si16   vcp;      /* Volume Coverage Pattern */
  Radx::si16   seqnum;   /* Sequence Number of request */
  Radx::si16   vscan;    /* Volume scan number - Resets at 80 */
  Radx::si16   vsdate;   /* Scan Date - Days since 12/31/69 */
  
  /* Broken into 2 Radx::si16s to avoid struct size padding */
  Radx::si16   vstime;   /* Seconds since 12AM GMT - MSW */
  Radx::ui16   vstim2;  /* Seconds since 12AM GMT - LSW  */
  Radx::si16   pgdate;   /* Prod Generation Date - Days since 12/31/69 */
  Radx::si32   pgtime;   /* Prod Generation Time - Seconds since 12AM GMT */
  Radx::si16   pd1;      /* Product Dependant int 1 */
  Radx::si16   pd2;      /* Product Dependant int 2 */
  Radx::si16   elevnum;  /* Elevation Number */
  Radx::si16   pd[24];   /* Product Dependant Radx::si16s */
  Radx::si16   nmaps;    /* Number of map pieces */
  Radx::si32   soffset;  /* Offset to Symbology data - 2byte halfwords*/
  Radx::si32   goffset;  /* Offset to Graphic data - 2byte halfwords*/
  Radx::si32   toffset;  /* Offset to Tabular data - 2byte halfwords*/
  Radx::si16   bdivider; /* Block Divider = 0xffff */
  Radx::si16   blockid;  /* Block ID  */
  Radx::si32   blength;  /* Block Length  */
  Radx::si16   nlayers;  /* Number of layers */
  Radx::si16   ldivider; /* Layer Divider = 0xffff */
  Radx::si32   lendat;   /* Length of data layer */
} _NIDS_header_t; 

/* RASTER FORMAT NIDS FILES */
typedef struct { 
  Radx::si16 num_bytes; /* Number of bytes of RLE Data in this row */
  /* num_bytes number of RLE data bytes follow this in the file */
  /* Each RLE byte has 4 bit Run (MSB) and 4 bit value (LSB) */
} _NIDS_row_header_t;

typedef struct { 
  Radx::si16 packet_code1;  /* Packet ID - usually -0xBA0F or 0xBA07 */
  Radx::si16 packet_code2;  /* Packet ID - usually -0x8000 */
  Radx::si16 packet_code3;  /* Packet ID - usually -0x00C0 */
  Radx::si16 x_start;       /* Starting location of data (km/4) */
  Radx::si16 y_start;       /* Starting location of data (km/4) */
  Radx::si16 x_scale;       /* Scaling factor for grid (1 to 67) */
  Radx::si16 x_scale_fract; /* factor for grid - PUP-only */
  Radx::si16 y_scale;       /* Scaling factor for grid (1 to 67) */
  Radx::si16 y_scale_fract; /* factor for grid - PUP-only */
  Radx::si16 num_rows;      /* Number of Rows in this product (1 to 464) */
  Radx::si16 packing_descriptor;  /* "Defines packing format 2"  */
  /* num_rtows number of NIDS_beam_header_t follow this in the file */
} _NIDS_raster_header_t;

/* RADIAL FORMAT NIDS FILES */
typedef struct { 
  Radx::si16 num_halfwords; /* Number of bytes of RLE Data / 2 in this beam */
  Radx::si16 radial_start_angle; /* Degrees * 10 - (0 to 3599) */
  Radx::si16 radial_delta_angle; /* Degrees * 10 - (0 to 20)   */
  /* num_halfwords * 2 number of RLE data bytes follow this in the file */
  /* Each RLE byte has 4 bit Run (MSB) and 4 bit value (LSB) */
} _NIDS_beam_header_t;

typedef struct { 
  Radx::si16 packet_code;  /* Packet ID - Usually 0xAF1F */
  Radx::si16 first_r_bin;  /* Index number of the first range bin */
  Radx::si16 num_r_bin;    /* Number of range bins in each radial beam */
  Radx::si16 i_center;     /* I coordinate of center of sweep (km/4) */
  Radx::si16 j_center;     /* J coordinate of center of sweep (km/4) */
  Radx::si16 scale_factor; /* Number of pixels per range bin * 1000 */
  Radx::si16 num_radials;  /* Number of radials in this product */
  /* num_radials number of NIDS_beam_header_t follow this in the file */
} _NIDS_radial_header_t;

/*
 * Byte swapping
 */

/* BE_to_mess_header - convert to native from BE */
extern void _NIDS_BE_to_mess_header(_NIDS_header_t *nhead);

/* BE_from_mess_header - convert from native to BE */
extern void _NIDS_BE_from_mess_header(_NIDS_header_t *nhead);

/* BE_to_raster_header - convert to native from BE  */
extern void _NIDS_BE_to_raster_header(_NIDS_raster_header_t *rhead);

/* BE_from_raster_header - convert from native to BE  */
extern void _NIDS_BE_from_raster_header(_NIDS_raster_header_t *rhead);

/* BE_to_beam_header - convert to native from BE  */
extern void _NIDS_BE_to_beam_header(_NIDS_beam_header_t *bhead);

/* BE_from_beam_header - convert from native to BE  */
extern void _NIDS_BE_from_beam_header(_NIDS_beam_header_t *bhead);

/* BE_to_radial_header - convert to native from BE  */
extern void _NIDS_BE_to_radial_header(_NIDS_radial_header_t *rhead);

/* BE_from_radial_header - convert from native to BE  */
extern void _NIDS_BE_from_radial_header(_NIDS_radial_header_t *rhead);

/* BE_to_row_header - convert to native from BE */
void _NIDS_BE_to_row_header(_NIDS_row_header_t *rowhead);

/* BE_from_row_header - convert from native to BE */
void _NIDS_BE_from_row_header(_NIDS_row_header_t *rowhead);

/*
 * printing
 */

/*
 * print message header
 */
extern void _NIDS_print_mess_hdr(FILE *out, const char *spacer,
				_NIDS_header_t *mhead);

/*
 * print raster header
 */
extern void _NIDS_print_raster_hdr(FILE *out, const char *spacer,
				  _NIDS_raster_header_t *rhead);

/*
 * print beam header
 */
extern void _NIDS_print_beam_hdr(FILE *out, const char *spacer,
				_NIDS_beam_header_t *bhead);

/*
 * print row header
 */

void _NIDS_print_row_hdr(FILE *out, const char *spacer,
			_NIDS_row_header_t *rowhead);

/*
 * print radial header
 */
extern void _NIDS_print_radial_hdr(FILE *out, const char *spacer,
				  _NIDS_radial_header_t *rhead);


/* non-standard byte swapping routines */

extern int _NIDS_host_is_big_endian(void);
extern void _NIDS_Reverse_4byte_vals(Radx::ui32* array, int num);
extern void _NIDS_Reverse_2byte_vals(Radx::ui16* array, int num);

extern void swap_nids_header(_NIDS_header_t *head);
extern void swap_nids_row_header(_NIDS_row_header_t *head);
extern void swap_nids_beam_header(_NIDS_beam_header_t *head);
extern void swap_nids_raster_header(_NIDS_raster_header_t *head);
extern void swap_nids_radial_header(_NIDS_radial_header_t *head);  

/*
 * Bigend module
 *
 * Description:
 *   This module consists of library routines to convert integers and
 * floating point values to and from big endian format.  Big endian
 * format assigns lower order bytes to storage with larger addresses.
 * For example, if the number 1 is to be stored using a two byte big
 * endian integer, it would be represented as
 *
 * address n+1    address n
 * 00000001       00000000
 *
 * The same number would be stored as
 *
 * address n+1    address n
 * 00000000       00000001
 *
 * in little endian format.  In the following code, we assume that the
 * underlying machine uses either big endian or little endian addressing.
 * If this is not the case, the module has to be rewritten to support
 * the underlying addressing scheme.
 *   The strategy of this module is to provide tools for machine
 * independent byte storage.  The routines in this module were designed
 * for efficiency, utility and portability .  The routines do not pad but
 * assume that different types will be converted to appropriate fixed
 * storage sizes.  In order to use these routines, one needs to determine
 * which C integer types on the underlying machine have 8 bits, 16 bits
 * and 32 bits.  On current machines one would use unsigned char,
 * unsigned short and unsigned int.  One would then reset ui08,
 * ui16, ui32, in bigend.h appropriately.
 *
 * Note that as of this date, Jan 24, 1995, Unix workstations such as
 * Sun, SGI, Digital, IBM, HP implement the following sizes:
 *
 * char   -> 1 byte
 * short  -> 2 bytes
 * int    -> 4 bytes
 * long   -> 4 bytes or 8 bytes (Digital)
 * float  -> 4 bytes
 * double -> 8 bytes
 *
 * MSDOS/WINDOWS typically assign 2 bytes to integers and 4 bytes to longs.
 *
 * IMPORTANT NOTE: The software assumes that the floating point
 * implementation is identical (IEEE) on all machines in question except for
 * byte ordering.
 *
 * In the future, it may be the case that types are assigned larger byte
 * lengths perhaps 12 bytes or 16 bytes.  In such cases, these routines
 * could be extended.
 */

/*
 * determine whether the underlying machine is big endian or not
 */

extern int _NIDS_BE_is_big_endian(void);

/*
 * _NIDS_BE_reverse()
 *
 * Reverses the sense of this library. Therefore,
 * is called once, SmallEndian values are set.
 * If called twice, goes back to BigEndian.
 */

extern void _NIDS_BE_reverse(void);

/*
 * in-place array converting
 */

/**********************************************************************
 * _NIDS_BE_swap_array_64()
 *
 * Performs an in-place 64-bit word byte swap, if necessary, to produce
 * _NIDS_BE representation from machine representation, or vice-versa.
 *
 * Array must be aligned.
 *
 * Returns the number of bytes converted.  Note that this number is 0
 * if the local machine is big-endian.
 */

extern Radx::si32 _NIDS_BE_swap_array_64(void *array, Radx::ui32 nbytes);


/**********************************************************************
 * _NIDS_BE_swap_array_32()
 *
 * Performs an in-place 32-bit word byte swap, if necessary, to produce
 * _NIDS_BE representation from machine representation, or vice-versa.
 *
 * Array must be aligned.
 *
 * Returns the number of bytes converted.  Note that this number is 0
 * if the local machine is big-endian.
 *
 */

extern Radx::si32 _NIDS_BE_swap_array_32(void *array, Radx::ui32 nbytes);

/**********************************************************************
 * _NIDS_BE_swap_array_16()
 *
 * Performs an in-place 16-bit word byte swap, if necessary, to produce
 * _NIDS_BE representation from machine representation, or vice-versa.
 *
 * Array must be aligned.
 *
 * Returns the number of bytes converted.  Note that this number is 0
 * if the local machine is big-endian.
 *
 */

extern Radx::si32 _NIDS_BE_swap_array_16(void *array, Radx::ui32 nbytes);

/********************
 * _NIDS_BE_from_array_64()
 * Converts an array of 64's
 * Returns the number of bytes converted.  Note that this number is 0
 * if the local machine is big-endian.
 */

#define _NIDS_BE_from_array_64(array,nbytes) _NIDS_BE_swap_array_64((array),(nbytes))
     
/******************
 * _NIDS_BE_to_array_64()
 * Converts an array of 64's
 *
 * Returns the number of bytes converted.  Note that this number is 0
 * if the local machine is big-endian.
 */

#define _NIDS_BE_to_array_64(array,nbytes) _NIDS_BE_swap_array_64((array),(nbytes))
     
/********************************
 *  _NIDS_BE_from_fl64
 *  Converts from a single fl64
 */

extern void _NIDS_BE_from_fl64(Radx::fl64 *dst, Radx::fl64 *src);

/********************************
 *  _NIDS_BE_to_fl64 
 *  Converts to a single fl64
 */

extern void _NIDS_BE_to_fl64(Radx::fl64 *src, Radx::fl64 *dst);

/********************************
 *  _NIDS_BE_from_si64 
 *  Converts a single si64
 *
 *  Returns the converted number.
 */

extern Radx::si64 _NIDS_BE_from_si64(Radx::si64 x);

/******************************
 *  _NIDS_BE_to_si64 
 *  Converts a single si64
 *
 *  Returns the converted number.
 */

extern Radx::si64 _NIDS_BE_to_si64(Radx::si64 x);

/********************
 * _NIDS_BE_from_array_32()
 * Converts an array of 64's
 * Returns the number of bytes converted.  Note that this number is 0
 * if the local machine is big-endian.
 */

#define _NIDS_BE_from_array_32(array,nbytes) _NIDS_BE_swap_array_32((array),(nbytes))
     
/******************
 * _NIDS_BE_to_array_32()
 * Converts an array of 32's
 *
 * Returns the number of bytes converted.  Note that this number is 0
 * if the local machine is big-endian.
 */

#define _NIDS_BE_to_array_32(array,nbytes) _NIDS_BE_swap_array_32((array),(nbytes))
     
/********************
 * _NIDS_BE_from_array_16()
 * Converts an array of 16's
 *
 * Returns the number of bytes converted.  Note that this number is 0
 * if the local machine is big-endian.
 */

#define _NIDS_BE_from_array_16(array,nbytes) _NIDS_BE_swap_array_16((array),(nbytes))

/******************
 * _NIDS_BE_to_array_16()
 * Converts an array of 16's
 *
 * Returns the number of bytes converted.  Note that this number is 0
 * if the local machine is big-endian.
 */

#define _NIDS_BE_to_array_16(array,nbytes) _NIDS_BE_swap_array_16((array),(nbytes))

/********************************
 *  _NIDS_BE_from_fl32
 *  Converts from a single fl32
 */

extern void _NIDS_BE_from_fl32(Radx::fl32 *dst, Radx::fl32 *src);

/********************************
 *  _NIDS_BE_to_fl32 
 *  Converts to a single fl32
 */

extern void _NIDS_BE_to_fl32(Radx::fl32 *src, Radx::fl32 *dst);

/********************************
 *  _NIDS_BE_from_si32 replaces htonl()
 *  Converts a single si32
 *
 *  Returns the converted number.
 */

extern Radx::si32 _NIDS_BE_from_si32(Radx::si32 x);

/******************************
 *  _NIDS_BE_to_si32 replaces ntohl()
 *  Converts a single si32
 *
 *  Returns the converted number.
 */

extern Radx::si32 _NIDS_BE_to_si32(Radx::si32 x);

/********************************
 *  _NIDS_BE_from_si16 replaces htons()
 *  Converts a single si16
 *
 *  Returns the converted number.
 */

extern Radx::si16 _NIDS_BE_from_si16(Radx::si16 x);

/******************************
 *  _NIDS_BE_to_si16 replaces ntohs()
 *  Converts a single si16
 *
 *  Returns the converted number.
 */

extern Radx::si16 _NIDS_BE_to_si16(Radx::si16 x);

/********************************
 *  _NIDS_BE_from_ui32
 *  Converts a single ui32
 *
 *  Returns the converted number.
 */

extern Radx::ui32 _NIDS_BE_from_ui32(Radx::ui32 x);

/******************************
 *  _NIDS_BE_to_ui32
 *  Converts a single ui32
 *
 *  Returns the converted number.
 */

extern Radx::ui32 _NIDS_BE_to_ui32(Radx::ui32 x);

/********************************
 *  _NIDS_BE_from_ui16
 *  Converts a single ui16
 *
 *  Returns the converted number.
 */

extern Radx::ui16 _NIDS_BE_from_ui16(Radx::ui16 x);

/******************************
 *  _NIDS_BE_to_ui16
 *  Converts a single ui16
 *
 *  Returns the converted number.
 */

extern Radx::ui16 _NIDS_BE_to_ui16(Radx::ui16 x);

/********************************************************************
* NIDS decompression utility for data compressed with zlib
* output stripping header and leaving raw NIDS data
********************************************************************/

extern int _NIDS_uncompress(RadxBuf &inBuf,
                            RadxBuf &outBuf,
                            string &radarName);

#endif
