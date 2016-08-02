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
#ifndef BIGEND_H
#define BIGEND_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Module: bigend.h
 *
 * Author: Gerry Wiener
 *	
 * Date:   Jan 26, 1995
 *
 * Modified by Mike Dixon, Jan 1996.
 * 64 Bit support - F. Hage 1999
 *
 */

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

#include <dataport/port_types.h>

/*
 * determine whether the underlying machine is big endian or not
 */

extern int BE_is_big_endian(void);

/*
 * BE_reverse()
 *
 * Reverses the sense of this library. Therefore,
 * is called once, SmallEndian values are set.
 * If called twice, goes back to BigEndian.
 */

extern void BE_reverse(void);

/*
 * in-place array converting
 */

/**********************************************************************
 * BE_swap_array_64()
 *
 * Performs an in-place 64-bit word byte swap, if necessary, to produce
 * BE representation from machine representation, or vice-versa.
 *
 * Array must be aligned.
 *
 * Returns the number of bytes converted.  Note that this number is 0
 * if the local machine is big-endian.
 */

extern si32 BE_swap_array_64(void *array, ui32 nbytes);


/**********************************************************************
 * BE_swap_array_32()
 *
 * Performs an in-place 32-bit word byte swap, if necessary, to produce
 * BE representation from machine representation, or vice-versa.
 *
 * Array must be aligned.
 *
 * Returns the number of bytes converted.  Note that this number is 0
 * if the local machine is big-endian.
 *
 */

extern si32 BE_swap_array_32(void *array, ui32 nbytes);

/**********************************************************************
 * BE_swap_array_16()
 *
 * Performs an in-place 16-bit word byte swap, if necessary, to produce
 * BE representation from machine representation, or vice-versa.
 *
 * Array must be aligned.
 *
 * Returns the number of bytes converted.  Note that this number is 0
 * if the local machine is big-endian.
 *
 */

extern si32 BE_swap_array_16(void *array, ui32 nbytes);

/********************
 * BE_from_array_64()
 * Converts an array of 64's
 * Returns the number of bytes converted.  Note that this number is 0
 * if the local machine is big-endian.
 */

#define BE_from_array_64(array,nbytes) BE_swap_array_64((array),(nbytes))
     
/******************
 * BE_to_array_64()
 * Converts an array of 64's
 *
 * Returns the number of bytes converted.  Note that this number is 0
 * if the local machine is big-endian.
 */

#define BE_to_array_64(array,nbytes) BE_swap_array_64((array),(nbytes))
     
/********************************
 *  BE_from_fl64
 *  Converts from a single fl64
 */

extern void BE_from_fl64(fl64 *dst, fl64 *src);

/********************************
 *  BE_to_fl64 
 *  Converts to a single fl64
 */

extern void BE_to_fl64(fl64 *src, fl64 *dst);

/********************************
 *  BE_from_si64 
 *  Converts a single si64
 *
 *  Returns the converted number.
 */

extern si64 BE_from_si64(si64 x);

/******************************
 *  BE_to_si64 
 *  Converts a single si64
 *
 *  Returns the converted number.
 */

extern si64 BE_to_si64(si64 x);

/********************
 * BE_from_array_32()
 * Converts an array of 64's
 * Returns the number of bytes converted.  Note that this number is 0
 * if the local machine is big-endian.
 */

#define BE_from_array_32(array,nbytes) BE_swap_array_32((array),(nbytes))
     
/******************
 * BE_to_array_32()
 * Converts an array of 32's
 *
 * Returns the number of bytes converted.  Note that this number is 0
 * if the local machine is big-endian.
 */

#define BE_to_array_32(array,nbytes) BE_swap_array_32((array),(nbytes))
     
/********************
 * BE_from_array_16()
 * Converts an array of 16's
 *
 * Returns the number of bytes converted.  Note that this number is 0
 * if the local machine is big-endian.
 */

#define BE_from_array_16(array,nbytes) BE_swap_array_16((array),(nbytes))

/******************
 * BE_to_array_16()
 * Converts an array of 16's
 *
 * Returns the number of bytes converted.  Note that this number is 0
 * if the local machine is big-endian.
 */

#define BE_to_array_16(array,nbytes) BE_swap_array_16((array),(nbytes))

/********************************
 *  BE_from_fl32
 *  Converts from a single fl32
 */

extern void BE_from_fl32(fl32 *dst, fl32 *src);

/********************************
 *  BE_to_fl32 
 *  Converts to a single fl32
 */

extern void BE_to_fl32(fl32 *src, fl32 *dst);

/********************************
 *  BE_from_si32 replaces htonl()
 *  Converts a single si32
 *
 *  Returns the converted number.
 */

extern si32 BE_from_si32(si32 x);

/******************************
 *  BE_to_si32 replaces ntohl()
 *  Converts a single si32
 *
 *  Returns the converted number.
 */

extern si32 BE_to_si32(si32 x);

/********************************
 *  BE_from_ti32
 *  Converts a single ti32
 */

extern ti32 BE_from_ti32(ti32 x);

/******************************
 *  BE_to_ti32
 *  Converts a single ti32
 */

extern ti32 BE_to_ti32(ti32 x);

/********************************
 *  BE_from_si16 replaces htons()
 *  Converts a single si16
 *
 *  Returns the converted number.
 */

extern si16 BE_from_si16(si16 x);

/******************************
 *  BE_to_si16 replaces ntohs()
 *  Converts a single si16
 *
 *  Returns the converted number.
 */

extern si16 BE_to_si16(si16 x);

/********************************
 *  BE_from_ui32
 *  Converts a single ui32
 *
 *  Returns the converted number.
 */

extern ui32 BE_from_ui32(ui32 x);

/******************************
 *  BE_to_ui32
 *  Converts a single ui32
 *
 *  Returns the converted number.
 */

extern ui32 BE_to_ui32(ui32 x);

/********************************
 *  BE_from_ui16
 *  Converts a single ui16
 *
 *  Returns the converted number.
 */

extern ui16 BE_from_ui16(ui16 x);

/******************************
 *  BE_to_ui16
 *  Converts a single ui16
 *
 *  Returns the converted number.
 */

extern ui16 BE_to_ui16(ui16 x);

#ifdef __cplusplus
}
#endif

#endif /* BIGEND_H */

