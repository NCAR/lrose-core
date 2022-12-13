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
 * Module: PortTypes.hh
 *
 * Copied from grib2/PortTypes.hh
 *	
 * Date:   Dec 2022
 *
 * Description:
 *   Declarations and macros for portable types.
 */

#ifndef PORTTYPES_HH
#define PORTTYPES_HH

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

typedef signed char     g2_si08;
typedef unsigned char   g2_ui08;
typedef int16_t         g2_si16;
typedef u_int16_t       g2_ui16;
typedef int32_t         g2_si32;
typedef u_int32_t       g2_ui32;
typedef int32_t         g2_ti32; /* time secs since 1970 */
typedef float	        g2_fl32; 
typedef double	        g2_fl64; 
typedef int64_t         g2_si64;
typedef u_int64_t       g2_ui64;
typedef int64_t         g2_ti64; /* time secs since 1970 */

#elif defined(IRIX5) || defined(SUNOS4) || defined(SUNOS5) || defined(SUNOS5_ETG) || defined(AIX) || defined(HPUX) || defined(ULTRIX) 

typedef signed char	g2_si08;
typedef unsigned char	g2_ui08;
typedef signed short	g2_si16;
typedef unsigned short	g2_ui16;
typedef signed int	g2_si32;
typedef unsigned int	g2_ui32;
typedef signed int	g2_ti32; /* time */
typedef float		g2_fl32; 
typedef double		g2_fl64; 

#elif defined(IRIX6) || defined(DECOSF1) || defined(SUNOS5_64) || defined(SUNOS5_ETG) || defined(LINUX_ALPHA) || defined(SUNOS5_INTEL) || defined(__APPLE__) || defined(CYGWIN)

typedef signed char	g2_si08;
typedef unsigned char	g2_ui08;
typedef signed short	g2_si16;
typedef unsigned short	g2_ui16;
typedef signed int	g2_si32;
typedef unsigned int	g2_ui32;
typedef signed int	g2_ti32; /* time */
typedef float		g2_fl32; 
typedef double		g2_fl64; 
typedef signed long long int g2_g2_si64;
typedef unsigned long long int g2_ui64;
typedef signed long long int g2_ti64; /* time */

#else

typedef signed char     g2_si08;
typedef unsigned char   g2_ui08;
typedef int16_t         g2_si16;
typedef u_int16_t       g2_ui16;
typedef int32_t         g2_si32;
typedef u_int32_t       g2_ui32;
typedef int32_t         g2_ti32; /* time secs since 1970 */
typedef float	        g2_fl32; 
typedef double	        g2_fl64; 
typedef int64_t         g2_si64;
typedef u_int64_t       g2_ui64;
typedef int64_t         g2_ti64; /* time secs since 1970 */

#endif

/*
 * number of bytes in various types
 */

#define G2_INT8_SIZE  1
#define G2_INT16_SIZE 2
#define G2_INT32_SIZE 4
#define G2_INT64_SIZE 8
#define G2_FLOAT32_SIZE 4
#define G2_FLOAT64_SIZE 8

/*
 * constant multiplier for storing degrees as longs
 */
  
#define G2_DEG2LONG 1000000.0

#endif /* PORT_TYPES_H */

