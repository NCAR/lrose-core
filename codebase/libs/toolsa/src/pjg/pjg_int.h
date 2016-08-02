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
/* pjg_int.h : common includes and defiles for 
		Projective Geometry routines
 */

#ifndef PJG_INT_WAS_INCLUDED
#define PJG_INT_WAS_INCLUDED

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#ifndef M_PI

#define M_E		2.7182818284590452354
#define M_LOG2E		1.4426950408889634074
#define M_LOG10E	0.43429448190325182765
#define M_LN2		0.69314718055994530942
#define M_LN10		2.30258509299404568402
#define M_PI		3.14159265358979323846
#define M_PI_2		1.57079632679489661923
#define M_PI_4		0.78539816339744830962
#define M_1_PI		0.31830988618379067154
#define M_2_PI		0.63661977236758134308
#define M_2_SQRTPI	1.12837916709551257390
#define M_SQRT2		1.41421356237309504880
#define M_SQRT1_2	0.70710678118654752440
#ifndef HUGE
#define	HUGE		3.40282347e+38	/*max decimal value of a "float" */
#endif /* !HUGE */

#endif

#define RAD_TO_DEG 57.29577951308092
#define DEG_TO_RAD 0.01745329251994372
#define TINY_ANGLE 1.e-4
#define TINY_DIST 1.e-2
#define TINY_FLOAT 1.e-10
#define KM_PER_NM 1.85196
#define KM_PER_DEG_AT_EQ 111.198487

#define LARGE_DOUBLE 1.0e99
#define SMALL_DOUBLE 1.0e-99
#define LARGE_LONG 2000000000L

/* constants for spherical earth projection */
#define EARTH_RADIUS 6371.204 /* kilometers */
#define KM_PER_DEG 2 * PI * EARTH_RADIUS / 360.

/* constants for ellipsoidal earth */
#define SEMI_MAJOR_AXIS 6378.388  /* km */
#define ECCENTRICITY .081991890	
#define ECC_SQUARED .006722670    /* = (1 - (296/297)**2)
				    official 1 part in 297 flattening */


#endif
#ifdef __cplusplus
}
#endif
