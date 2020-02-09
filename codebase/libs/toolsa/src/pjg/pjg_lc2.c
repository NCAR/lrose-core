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
/* pjg_lc.c : Projective Geometry lambert conformal routines */


#include "pjg_int.h"
#include <toolsa/pjg.h>
#include <toolsa/str.h>
#include <toolsa/sincos.h>

typedef struct cs_t_ {
    int type;
    double Lc2_F, Lc2_lon0, Lc2_n, Lc2_rho, Lc2_tan0, Lc2_sin0;
} cs_t;
static PJGstruct Cs;

int Lc2_2tan_line;

PJGstruct *PJGs_lc2_init(double lat0, double lon0, double lat1, double lat2)
/* precompute constants for lambert conformal, two standard parellels
   spherical Earth

   lon0, lat0 =  origin of the projection's coord. system
   lat1, lat2: 	 standard parellels; 
 
   you must call this before you call other PJGlc2_xxx routines
   return FALSE if any of the following are true:
	lat0, lat1, lat2 = +/- 90 deg
	lat1 = lat2
 */
    {
	PJGstruct *ps;
	cs_t *cs;
	double	t1, t2, t0n, t1n;

	/* check illegal values */
	if (fabs(lat0 - 90.0) < TINY_ANGLE)
	    return NULL;
	if (fabs(lat0 + 90.0) < TINY_ANGLE)
	    return NULL;
	if (fabs(lat1 - 90.0) < TINY_ANGLE)
	    return NULL;
	if (fabs(lat1 + 90.0) < TINY_ANGLE)
	    return NULL;
	if (fabs(lat2 - 90.0) < TINY_ANGLE)
	    return NULL;
	if (fabs(lat2 + 90.0) < TINY_ANGLE)
	    return NULL;

	Lc2_2tan_line = 1;
	if (fabs(lat2-lat1) < TINY_ANGLE){
	    Lc2_2tan_line = 0;
	}
	ps = (PJGstruct *) malloc(sizeof(PJGstruct));
	cs = (cs_t *) ps;

	lat0 *= DEG_TO_RAD;
	lat1 *= DEG_TO_RAD;
	lat2 *= DEG_TO_RAD;

	if (Lc2_2tan_line){
	    t1 = tan( M_PI_4 + lat1/2);
	    t2 = tan( M_PI_4 + lat2/2);
	    cs->Lc2_n = log( cos(lat1)/cos(lat2)) / log(t2/t1);

	    t1n = pow( t1, cs->Lc2_n);
	    cs->Lc2_F = cos(lat1) * t1n / cs->Lc2_n;
	
	    t0n = pow( tan(M_PI_4 + lat0/2), cs->Lc2_n);
	    cs->Lc2_rho = PJG_get_earth_radius() * cs->Lc2_F / t0n;

	    cs->Lc2_lon0 = lon0 * DEG_TO_RAD;
	}
	else {
	    cs->Lc2_sin0 = sin(lat0);
	    cs->Lc2_tan0 = tan(M_PI_4 - lat0/2.);
	    cs->Lc2_rho = PJG_get_earth_radius() / tan(lat0);
	    cs->Lc2_lon0 = lon0 * DEG_TO_RAD;
	}
	return ps;
    }

void PJGs_lc2_latlon2xy(PJGstruct *ps, double lat, double lon, double *x, double *y)
/*
 *  Input:  lat, lon in degrees (lat N+, lon E+)
 *  Output: x, y in lambert conformal projection 
 *     
 * 	must call PJGlc2_init() first
 */
    {
	cs_t *cs = (cs_t *) ps;
	double r, theta, tn;
	double del_lon, f1, tan_phi;
	double sin_lon, cos_lon;
	double sin_theta, cos_theta;

	lat *= DEG_TO_RAD;
	lon *= DEG_TO_RAD;

	if (Lc2_2tan_line){
	    theta = cs->Lc2_n * (lon - cs->Lc2_lon0);

	    tn = pow( tan(M_PI_4 + lat/2), cs->Lc2_n);
	    r = PJG_get_earth_radius() * cs->Lc2_F / tn;

	    ta_sincos(theta, &sin_theta, &cos_theta);
	    *x = r * sin_theta;
	    *y = cs->Lc2_rho - r * cos_theta;
	}
	else {
	    tan_phi = tan(M_PI_4 - lat/2.);
	    f1 = pow((tan_phi/cs->Lc2_tan0), cs->Lc2_sin0);
	    del_lon = lon - cs->Lc2_lon0;
	    ta_sincos(del_lon * cs->Lc2_sin0, &sin_lon, &cos_lon);

	    *x = cs->Lc2_rho * f1 * sin_lon;
	    *y = cs->Lc2_rho * (1 - f1 * cos_lon);
	}
    }

void PJGs_lc2_xy2latlon(PJGstruct *ps, double x, double y, double *lat, double *lon)
/*
 *  Input: x, y in lambert conformal projection
 *  Output:  lat, lon in degrees (lat N+, lon E+)
 *     
 */
    {
	cs_t *cs = (cs_t *) ps;
	double r, theta, rn, yd;
	double del_lon, sin_lon, f1;
	double inv_sin0, to_sin0, loc_x;

	if (Lc2_2tan_line){
          /* See Bottom of Pg 105 - Snyder, Projections used by the USGS */
		if (cs->Lc2_n < 0.0) {
	      yd = (-cs->Lc2_rho + y);
	      theta = atan2(-x, yd);
	      r = sqrt( x*x + yd*yd);
	      r *= -1.0;
		} else {
	      yd = (cs->Lc2_rho - y);
	      theta = atan2( x, yd);
	      r = sqrt( x*x + yd*yd);
		}

	    *lon = (theta/cs->Lc2_n + cs->Lc2_lon0) * RAD_TO_DEG;
	    *lon = PJGrange180( *lon);

	    if (fabs(r) < TINY_FLOAT){  
	        *lat = ((cs->Lc2_n < 0.0) ? -90.0 : 90.0);
	    }
	    else {
	        rn = pow(PJG_get_earth_radius() * cs->Lc2_F / r, 1/cs->Lc2_n);
	        *lat = (2.0 * atan( rn) - M_PI_2) * RAD_TO_DEG;
	    }
	    *lat = PJGrange180( *lat);
	}
	else {
	    loc_x = x;
	    inv_sin0 = 1/cs->Lc2_sin0;

	    if (fabs(loc_x) < TINY_FLOAT) {
	        loc_x = 0.001;
	    }

	    del_lon = inv_sin0*atan2(loc_x,(cs->Lc2_rho - y));

	    *lon = cs->Lc2_lon0 + del_lon;
  
	    sin_lon = sin(del_lon * cs->Lc2_sin0);
	    r = cs->Lc2_rho * sin_lon;
	    to_sin0 = pow((loc_x/r), inv_sin0);
	    f1 = 2*atan(cs->Lc2_tan0 * to_sin0);

	    *lon = PJGrange180(((*lon)*RAD_TO_DEG));
	    *lat = PJGrange180(((M_PI_2 - f1) * RAD_TO_DEG));
	}
    }

/* old way of doing it */
int PJGlc2_init(double lat0, double lon0, double lat1, double lat2)
    {
	PJGstruct *ps;
	ps = PJGs_lc2_init( lat0, lon0, lat1, lat2);
	if (NULL == ps)
	    return 0;
	else
	    {
	    memcpy( &Cs, ps, sizeof(cs_t));
	    free(ps);
	    return 1;
	    }
    }

void PJGlc2_latlon2xy(double lat, double lon, double *x, double *y)
    {
	PJGs_lc2_latlon2xy( &Cs, lat, lon, x, y);
    }

void PJGlc2_xy2latlon( double x, double y, double *lat, double *lon)
    {
	PJGs_lc2_xy2latlon( &Cs, x, y, lat, lon);
    }



