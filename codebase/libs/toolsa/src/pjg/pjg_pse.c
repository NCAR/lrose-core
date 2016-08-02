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
/* pjg_pse.c :   Projective Geometry
		 oblique stereographic routines
		 using ellipsoid earth

		Currently does not support tangent point at the poles 

   7/20/92 : ANSI version
 */

#include "pjg_int.h"
#include <toolsa/pjg.h>
#include <toolsa/str.h>
#include <toolsa/sincos.h>

typedef struct cs_t_ {
    int type;
    double Ps_lat, Ps_lon, Ps_sin, Ps_cos, Ps_chi, Ps_x0, Ps_y0, Ps_scale, Ps_axis;
} cs_t;
static PJGstruct Cs;

static double Conformal_lat( double geodetic_lat)
   /*  double geodetic_lat; in radians */

    /* calc conformal latitude from geodetic lat (see eq 3-1)
	return conformal lat in radians  */
    {
	double esl, t, p, chi;

	esl = ECCENTRICITY * sin( geodetic_lat);
	t = (1.0 - esl) / (1.0 + esl);
	p = pow( t, 0.5 * ECCENTRICITY);
	t = tan( .25 * M_PI + .5 * geodetic_lat);
	chi = 2.0 * atan( p * t) - M_PI_2;

	return chi;
    }

PJGstruct *PJGs_pse_init(double lat0, double lon0, double latt, double lont, 
		double semimajor_axis, double scale)
/*  double lat0, lon0;   	origin of the coordinate system 
    double latt, lont;   	tangent point of plane
    double semimajor_axis;	semimajor axis of earth in km 
    double scale;		scale factor at tangent point, usually = 1.0 
 */

    {
	PJGstruct *ps = (PJGstruct *) malloc(sizeof(PJGstruct));
	cs_t *cs = (cs_t *) ps;
	double t, x, y;

	cs->Ps_lat = latt * DEG_TO_RAD;
	cs->Ps_lon = lont * DEG_TO_RAD;
	cs->Ps_axis = semimajor_axis;

	cs->Ps_chi = Conformal_lat( cs->Ps_lat);
	ta_sincos( cs->Ps_chi, &cs->Ps_sin, &cs->Ps_cos);

	t = ECCENTRICITY * cs->Ps_sin;
	cs->Ps_scale = cs->Ps_cos * scale / sqrt( 1.0 - t*t);

	/* calculate x,y of origin */
	cs->Ps_x0 = 0.0;
	cs->Ps_y0 = 0.0;
	PJGs_pse_latlon2xy( ps, lat0, lon0, &x, &y);
	cs->Ps_x0 = x;
	cs->Ps_y0 = y;

	return ps;
    }


void PJGs_pse_latlon2xy(PJGstruct *ps, double lat, double lon, double *x, double *y)
/*
 *  Input:  lat, lon in degrees (lat N+, lon E+)
 *  Output: x, y in polar stereographic projection
 *     
 */
    {
	cs_t *cs = (cs_t *) ps;
	double cd, sd, sinc, cosc, A, chi;

	lat *= DEG_TO_RAD;
	lon *= DEG_TO_RAD;

	/* degenerate cases */
	if ((fabs(lat + cs->Ps_lat) <= TINY_ANGLE) &&
	    ((fabs(lon - cs->Ps_lon - M_PI) <= TINY_ANGLE) ||
	     (fabs(lon - cs->Ps_lon + M_PI) <= TINY_ANGLE)))
		{
		*x = 0.0;
		*y = 0.0;
		return;
		}

	chi = Conformal_lat( lat);
	ta_sincos( chi, &sinc, &cosc);
	ta_sincos(lon - cs->Ps_lon, &sd, &cd);

	A = 2.0 * cs->Ps_axis * cs->Ps_scale / 
	    (cs->Ps_cos * (1.0 + cs->Ps_sin * sinc + cs->Ps_cos * cosc * cd));

	*y = A * (cs->Ps_cos * sinc - cs->Ps_sin * cosc * cd) - cs->Ps_y0;
	*x = A * cosc * sd - cs->Ps_x0;
    }


static double InvConformal_lat( double chi, double lat)
    /* double chi, lat;    in radians */

    {
	double esl, t, p;

	esl = ECCENTRICITY * sin( lat);
	t = (1.0 + esl) / (1.0 - esl);
	p = pow( t, 0.5 * ECCENTRICITY);
	t = tan( .25 * M_PI + .5 * chi);
	lat = 2.0 * atan( p * t) - M_PI_2;

	return lat;
    }

void PJGs_pse_xy2latlon(PJGstruct *ps, double x, double y, double *lat, double *lon)
/*
 *  Input: x, y in polar stereographic projection
 *  Output:  lat, lon in degrees (lat N+, lon E+)
 *     
 */

    {
	cs_t *cs = (cs_t *) ps;
	double rho, c, chi, lam, sinc, cosc, phi, old_phi;

	/* first translate into coord system with origin at tangent point */
	x += cs->Ps_x0;
	y += cs->Ps_y0;

	rho = sqrt( x*x + y*y);
	c = 2.0 * atan2( rho * cs->Ps_cos, 2.0*cs->Ps_axis*cs->Ps_scale);	
	ta_sincos( c, &sinc, &cosc);

	if (fabs(rho) < TINY_FLOAT)
	    chi = cs->Ps_chi;
	else
	    chi = asin( cosc * cs->Ps_sin + y * sinc * cs->Ps_cos / rho);

	lam = cs->Ps_lon + atan2( x * sinc, rho * cs->Ps_cos * cosc - y * sinc * cs->Ps_sin);

	/* iterate until phi converges */
	phi = chi;
	do
	    {
	    old_phi = phi;
	    phi = InvConformal_lat( chi, old_phi);
	    /* printf("old = %9.6f, new = %9.6f, diff = %10.8f\n",
		old_phi, phi, fabs( old_phi - phi)); */
	    }
	while (fabs( old_phi - phi) > TINY_ANGLE);
	
	*lon = PJGrange180( lam * RAD_TO_DEG);
	*lat = phi * RAD_TO_DEG;
    }


/* old way of doing it */
void PJGpse_init(double lat0, double lon0, double latt, double lont, 
		double semimajor_axis, double scale)
    {
	PJGstruct *ps;
	ps = PJGs_pse_init( lat0, lon0, latt, lont, semimajor_axis, scale);
	memcpy( &Cs, ps, sizeof(cs_t));
	free(ps);
    }

void PJGpse_latlon2xy(double lat, double lon, double *x, double *y)
    {
	PJGs_pse_latlon2xy( &Cs, lat, lon, x, y);
    }

void PJGpse_xy2latlon( double x, double y, double *lat, double *lon)
    {
	PJGs_pse_xy2latlon( &Cs, x, y, lat, lon);
    }

