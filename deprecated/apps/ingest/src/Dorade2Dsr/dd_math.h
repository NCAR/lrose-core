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
/* 	$Id: dd_math.h,v 1.3 2016/03/07 01:23:00 dixon Exp $	 */

# ifndef DDMATHH
# define DDMATHH

#include <math.h>

#define dd_isnanf(x)       (((*(si32 *)&(x) & 0x7f800000L) == 0x7f800000L) && \
                            ((*(si32 *)&(x) & 0x007fffffL) != 0x00000000L))

# define FMOD360(x) (fmod((double)((x)+720.), (double)360.))
# define EXP2(x) (pow((double)2.0, (double)(x)))
# define EXPN(x) (pow((double)2.718281828, (double)(x)))
# define EXP10(x) (pow((double)10.0, (double)(x)))

# define WATTZ(x) (pow((double)10.0, (double)((x) * .1)))
# define W_TO_DBM(x) (10. * log10((double)(x)))

# define LOG10(x) (log10((double)(x)))
# define LOGN(x) (log((double)(x)))
# define SQRT(x) (sqrt((double)(x)))
# define FABS(x) (fabs((double)(x)))
# define SIN(x) (sin((double)(x)))
# define COS(x) (cos((double)(x)))
# define TAN(x) (tan((double)(x)))
# define ATAN2(y,x) (atan2((double)(y), (double)(x)))
# define ASIN(x) (asin((double)(x)))
# define ACOS(x) (acos((double)(x)))
# define ATAN(x) (atan((double)(x)))

# define M_TO_KM(x) ((x) * .001)
# define KM_TO_M(x) ((x) * 1000.)

# endif
