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

#ifndef NEXRAD_PORT_TYPES_H
#define NEXRAD_PORT_TYPES_H

#include <sys/types.h>

/*
 * Types that are fixed in bit size -- C does not assume that a character
 * is 8 bits in length.  These types must be set on each machine and the
 * appropriate C type must be set to correspond to the fixed length types
 * such as si08, si16 and so forth.  Note that be.c assumes there is
 * a type in C that is 8 bits.  Since sizeof(char) is 1 in C, this means
 * that be.c assumes that a char is an 8 bit quantity.
 */

#if defined(IRIX5) || defined(LINUX) || \
defined(SUNOS4) || defined(SUNOS5) || \
defined(AIX) || defined(HPUX) || defined(ULTRIX) || \
defined(LINUX_ALPHA) || defined(LINUX_IL6)

typedef signed char     si08;
typedef unsigned char   ui08;
typedef signed short    si16;
typedef unsigned short  ui16;
typedef signed int      si32;
typedef unsigned int    ui32;
typedef unsigned int    ti32; /* time */
typedef float           fl32; 
typedef double          fl64; 

#endif

#if defined(IRIX6) || defined(DECOSF1) || defined(SUNOS5_64)
typedef signed char     si08;
typedef unsigned char   ui08;
typedef signed short    si16;
typedef unsigned short  ui16;
typedef signed int      si32;
typedef unsigned int    ui32;
typedef unsigned int    ti32; /* time */
typedef float           fl32; 
typedef double          fl64; 
typedef signed long long int si64;
typedef unsigned long long int ui64;
typedef signed long long int ti64; /* time */
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

#endif /* NEXRAD_PORT_TYPES_H */
