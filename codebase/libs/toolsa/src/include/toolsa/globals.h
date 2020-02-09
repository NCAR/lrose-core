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

#ifndef TOOLSA_GLOBALS_WAS_INCLUDED
#define TOOLSA_GLOBALS_WAS_INCLUDED

#include <toolsa/os_config.h>

#ifndef NULL
#define NULL (void *) 0
#endif

   /* #define hidden char */

#ifndef EOS
#define EOS ((char) 0)
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define BIT0 1
#define BIT1 2
#define BIT2 4
#define BIT3 8
#define BIT4 16
#define BIT5 32
#define BIT6 64 
#define BIT7 128 

/* useful for defining library/module name only once */
#define LibName(s) static char Library[] =s
#define ModName(s) static char Module[] =s

/* useful macros */
#ifndef MAX
#define MAX(a,b)	(((a) > (b)) ?  (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b)	(((a) < (b)) ?  (a) : (b))
#endif
#ifndef ABS
#define ABS(x) ((x) > 0 ? (x) : (-(x)))
#endif
#ifndef FABS
#define FABS(a)	(((a) < (float) 0.0) ? -(a) : (a))
#endif
#ifndef DABS
#define DABS(a)	(((a) < (double) 0.0) ? -(a) : (a))
#endif

/* round up s to align on m-byte boundry: s += BYTE_ALIGN(s,m) */
#define BYTE_BDRY_ALIGN( s, m) (((s)%(m) == 0) ? 0 : ((m) - (s)%(m)))


/********************************************************************/
/* Conversion Macros:  word = int16, long = int32, byte = 8 bits,   */
/*     nibble = 4 bits                                              */
/********************************************************************/

#define uthiword(a)   (((a)>>16)&0xffffL)   /* High word of long a. */
#define utloword(a)   ((a)&0xffffL)	    /* Low  word of long a. */

/* Combine high word a, low  word b.*/
#define utwdlong(a,b) ((((0xffffL&(long)(a)))<<16)|		     \
		       (0xffffL&(long)(b)))

#define uthibyte(a)   (((a)>>8)&0x00ff) /* High byte of word a.     */
#define utlobyte(a)   ((a)&0x00ff)	/* Low	byte of word a.     */

/* Combine high byte a, low byte b. */
#define utbyword(a,b) ((((a)&0x00ff)<<8)|((b)&0x00ff))

#define uthinyb(a)    (((a)>>4)&0x000f) /* High nybble of byte a.   */
#define utlonyb(a)    ((a)&0x000f)	/* Low nybble  of byte a.   */

/* Combine high nybble a, low	    */
/* nybble b.			    */
#define utnybbyt(a,b) ((((a)&0x000f)<<4)|((b)&0x000f))

/* test,set bit n */
#define utbitest(w,n) ((((w) >> (n)) & 1) == 1)
#define utbitset(w,n) (w |= (1 << (n)))

#ifndef PI 
#define PI 3.14159265358987
#endif

#ifndef RAD_PER_DEG 
#define RAD_PER_DEG 0.017453293	/* radians per degree */
#endif

#ifndef DEG_PER_RAD 
#define DEG_PER_RAD 57.29577951 /* degrees per radian */
#endif

#ifndef FT_PER_KM
#define FT_PER_KM 3280	    	/* feet per kilometer */
#endif

#ifndef NMH_PER_MS
#define NMH_PER_MS  1.9438445   /* Nautical Miles per hour per meters/sec */
#endif

#ifndef KMH_PER_MPS
#define KMH_PER_MPS 3.6 /* Kilometers/hour per meters/sec */
#endif

#ifndef LOC_MHR_LON
#define LOC_MHR_LON -104.75900	/* location of Mile High Radar */
#endif

#ifndef LOC_MHR_LAT
#define LOC_MHR_LAT 39.87823	
#endif

#endif /* GLOBALS_WAS_INCLUDED */
#ifdef __cplusplus
}
#endif
