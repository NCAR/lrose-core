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
/* pjg_ps.c :   Projective Geometry polar stereographic routines
		General, oblique case 
		Can handle polar case also

	Reference : "Map Projections used by the US Geological Survey"
		Bulletin 1532, 2nd Edition, John P. Snyder
	
 */

#include "pjg_int.h"
#include <toolsa/pjg.h>
#include <toolsa/str.h>
#include <toolsa/sincos.h>

typedef struct cs_t_ {
    int type;
    double Ps_lat, Ps_lon, Ps_sin, Ps_cos, Ps_x0, Ps_y0, Ps_scale;
} cs_t;
static PJGstruct Cs;


PJGstruct *PJGs_ps_init(double lat0, double lon0, double latt, double lont, double scale)
/* precompute constants for polar stereographic projection

   lat0, lon0: origin of the coordinate system
   latt, lont: tangent point of projection
   scale: scale factor at tangent point, usually = 1.0
 
   you must call this before you call other PJGps_xxx routines
 */
    {
	PJGstruct *ps = (PJGstruct *) malloc(sizeof(PJGstruct));
	cs_t *cs = (cs_t *) ps;
	double x, y;

	cs->Ps_lat = latt * DEG_TO_RAD;
	cs->Ps_lon = lont * DEG_TO_RAD;
	cs->Ps_scale = scale;

	ta_sincos( cs->Ps_lat, &cs->Ps_sin, &cs->Ps_cos);

	/* calculate x,y of origin */
	cs->Ps_x0 = 0.0;
	cs->Ps_y0 = 0.0;
	PJGs_ps_latlon2xy( ps, lat0, lon0, &x, &y);
	cs->Ps_x0 = x;
	cs->Ps_y0 = y;

	return ps;
    }


void PJGs_ps_latlon2xy(PJGstruct *ps, double lat, double lon, double *x, double *y)
/*
 *  Input:  lat, lon in degrees (lat N+, lon E+)
 *  Output: x, y in polar stereographic projection
 *     
 */

    {
	cs_t *cs = (cs_t *) ps;
	double cd, sd, sin1, cos1, k;

	lat *= DEG_TO_RAD;
	lon *= DEG_TO_RAD;

	/* degenerate cases */
	if ((fabs(lat + cs->Ps_lat) <= TINY_ANGLE) ||
	    (fabs(lon - cs->Ps_lon - M_PI) <= TINY_ANGLE) ||
	    (fabs(lon - cs->Ps_lon + M_PI) <= TINY_ANGLE))
		{
		*x = 0.0;
		*y = 0.0;
		return;
		}

	ta_sincos(lon - cs->Ps_lon, &sd, &cd);
	ta_sincos( lat, &sin1, &cos1);

	k = 2.0 * cs->Ps_scale / (1.0 + cs->Ps_sin * sin1 + cs->Ps_cos * cos1 * cd);
	*x = PJG_get_earth_radius() * k * cos1 * sd - cs->Ps_x0;
	*y = PJG_get_earth_radius() * k * ( cs->Ps_cos * sin1 - cs->Ps_sin * cos1 * cd) - cs->Ps_y0;
    }


void PJGs_ps_xy2latlon(PJGstruct *ps, double x, double y, double *lat, double *lon)
/*
 *  Input: x, y in polar stereographic projection
 *  Output:  lat, lon in degrees (lat N+, lon E+)
 *     
 */
    {
	cs_t *cs = (cs_t *) ps;
	double rho, c, phi, lam, sinc, cosc;

	/* first translate into coord system with origin at tangent point */
	x += cs->Ps_x0;
	y += cs->Ps_y0;

	rho = sqrt( x*x + y*y);
	c = 2.0 * atan2( rho, 2.0 * PJG_get_earth_radius() * cs->Ps_scale);	
	ta_sincos( c, &sinc, &cosc);

	if (fabs(rho) < TINY_FLOAT)
	    phi = cs->Ps_lat;
	else
	    phi = asin( cosc * cs->Ps_sin + y * sinc * cs->Ps_cos / rho);
	*lat = phi * RAD_TO_DEG;

	if ((fabs(x) < TINY_FLOAT) && (fabs(y) < TINY_FLOAT))
	    lam = cs->Ps_lon;
	else if (fabs(cs->Ps_cos) < TINY_FLOAT)
	    lam = cs->Ps_lon + atan2( x, ((cs->Ps_lat > 0) ? -y : y) );
	else
	    lam = cs->Ps_lon + atan2( x*sinc, rho*cs->Ps_cos*cosc - y*sinc*cs->Ps_sin);
	
	*lon = lam * RAD_TO_DEG;
	*lon = PJGrange180( *lon);
    }



/* old way of doing it */
void PJGps_init(double lat0, double lon0, double latt, double lont, double scale)
    {
	PJGstruct *ps;
	ps = PJGs_ps_init( lat0, lon0, latt, lont, scale);
	memcpy( &Cs, ps, sizeof(cs_t));
	free(ps);
    }

void PJGps_latlon2xy(double lat, double lon, double *x, double *y)
    {
	PJGs_ps_latlon2xy( &Cs, lat, lon, x, y);
    }

void PJGps_xy2latlon( double x, double y, double *lat, double *lon)
    {
	PJGs_ps_xy2latlon( &Cs, x, y, lat, lon);
    }

