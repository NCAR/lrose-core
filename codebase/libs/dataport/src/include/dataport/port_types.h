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
/*
 * Module: port_types.h
 *
 * Author: Gerry Wiener, mods by Mike Dixon
 *	
 * Date:   Jan 26, 1995
 * mods:   Jan 12, 1996
 *
 * Description:
 *   Declarations and macros for the Big Endian library.
 */

#ifndef PORT_TYPES_H
#define PORT_TYPES_H

#include <sys/types.h>

/*
 * Types that are fixed in bit size -- C does not assume that a character
 * is 8 bits in length.  These types must be set on each machine and the
 * appropriate C type must be set to correspond to the fixed length types
 * such as si08, si16 and so forth.  Note that be.c assumes there is
 * a type in C that is 8 bits.  Since sizeof(char) is 1 in C, this means
 * that be.c assumes that a char is an 8 bit quantity.
 */

#if defined(__linux)

typedef signed char     si08;
typedef unsigned char   ui08;
typedef int16_t         si16;
typedef u_int16_t       ui16;
typedef int32_t         si32;
typedef u_int32_t       ui32;
typedef int32_t         ti32; /* time secs since 1970 */
typedef float	        fl32; 
typedef double	        fl64; 
typedef int64_t         si64;
typedef u_int64_t       ui64;
typedef int64_t         ti64; /* time secs since 1970 */

#elif defined(IRIX5) || defined(SUNOS4) || defined(SUNOS5) || defined(SUNOS5_ETG) || defined(AIX) || defined(HPUX) || defined(ULTRIX) 

typedef signed char	si08;
typedef unsigned char	ui08;
typedef signed short	si16;
typedef unsigned short	ui16;
typedef signed int	si32;
typedef unsigned int	ui32;
typedef signed int	ti32; /* time */
typedef float		fl32; 
typedef double		fl64; 

#elif defined(IRIX6) || defined(DECOSF1) || defined(SUNOS5_64) || defined(SUNOS5_ETG) || defined(LINUX_ALPHA) || defined(SUNOS5_INTEL) || defined(__APPLE__) || defined(CYGWIN)

typedef signed char	si08;
typedef unsigned char	ui08;
typedef signed short	si16;
typedef unsigned short	ui16;
typedef signed int	si32;
typedef unsigned int	ui32;
typedef signed int	ti32; /* time */
typedef float		fl32; 
typedef double		fl64; 
typedef signed long long int si64;
typedef unsigned long long int ui64;
typedef signed long long int ti64; /* time */

#else

typedef signed char     si08;
typedef unsigned char   ui08;
typedef int16_t         si16;
typedef u_int16_t       ui16;
typedef int32_t         si32;
typedef u_int32_t       ui32;
typedef int32_t         ti32; /* time secs since 1970 */
typedef float	        fl32; 
typedef double	        fl64; 
typedef int64_t         si64;
typedef u_int64_t       ui64;
typedef int64_t         ti64; /* time secs since 1970 */

#endif

/*
 * number of bytes in various types
 */

#define INT8_SIZE  1
#define INT16_SIZE 2
#define INT32_SIZE 4
#define INT64_SIZE 8
#define FLOAT32_SIZE 4
#define FLOAT64_SIZE 8

/*
 * constant multiplier for storing degrees as longs
 */
  
#define DEG2LONG 1000000.0

#endif /* PORT_TYPES_H */

