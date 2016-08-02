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
/* pjg_art.c :   Projective Geometry
		 "denver artcc projection"
		This projection is the same as the one used by the 
		Denver Artcc. It uses a second order approximation of a
		oblique stereographic, ellipsoidal projection.

   7/20/92 : ANSI version
   10/15/93: add PJGstruct versions.
 */

#include "pjg_int.h"
#include <toolsa/pjg.h>
#include <toolsa/sincos.h>

static double Scale;
static double Clat = 41.312 * DEG_TO_RAD; /* Conformal lat */
static double Tsin = .660157, Tcos = .751127; /* Tangent */
static double Tlon = -106.7778 * DEG_TO_RAD;
static double X0 = 365.625 * KM_PER_NM;
static double Y0 = 495.625 * KM_PER_NM; 

PJGstruct *PJGs_art_init( void)
    {
	double sphi, cphi, t;
	
	ta_sincos( Clat, &sphi, &cphi);
	t = ECCENTRICITY * sphi;
	Scale = cphi/sqrt(1-t*t);

	return PJGs_pse_init( 33.012759, -113.990335, 41.503629, -106.7778, 6358.38, 1.0);
    }


void PJGs_art_latlon2xy( PJGstruct *ps, double lat, double lon, double *x, double *y)
    {
	double cd, sd, t, sinc, cosc, A = 0;

	lat *= DEG_TO_RAD;
	lon *= DEG_TO_RAD;

	t = sin( lat);
	sinc = .993277 * t + .0066625 * t * t * t;
	cosc = sqrt( 1 - sinc*sinc);

	ta_sincos(lon-Tlon, &sd, &cd);
	*y = A * (Tcos * sinc - Tsin * cosc * cd) + Y0;
	*x = A * cosc * sd + X0;
    }

void PJGs_art_xy2latlon(PJGstruct *ps, double x, double y, double *lat, double *lon)
/*
 *  Input: x, y in polar stereographic projection
 *  Output:  lat, lon in degrees (lat N+, lon E+)
 *     
 */
    {
	PJGs_pse_xy2latlon( ps, x, y, lat, lon);
    }


/* old way of doing it */
void PJGart_init(void)
    {
	PJGstruct *ps;
	ps = PJGs_art_init();
	free(ps);

	/* must initialize pse static stuff for backward compatability */
	PJGpse_init( 33.012759, -113.990335, 41.503629, -106.7778, 6358.38, 1.0);
    }

void PJGart_latlon2xy(double lat, double lon, double *x, double *y)
    {
	PJGs_art_latlon2xy( NULL, lat, lon, x, y);
    }

void PJGart_xy2latlon( double x, double y, double *lat, double *lon)
    {
	PJGpse_xy2latlon( x, y, lat, lon);
    }


