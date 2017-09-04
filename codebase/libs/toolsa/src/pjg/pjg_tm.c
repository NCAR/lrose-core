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
/* pjg_tm.c :   Projective Geometry transverse mercator
		spherical earth

   see p 67, John P. Snyder, "Map Proj. Used by the USGS"
 */

#include "pjg_int.h"
#include <toolsa/pjg.h>
#include <toolsa/str.h>

/* atanh is not part of ANSO math.h; however it is typically
   available. We will explicitly declare it in order to
   avoid the possibility that it defaults to a function returning
   an int. */
extern double   atanh(double);

typedef struct cs_t_ {
    int type;
    double Tm_lat0, Tm_lon0, Tm_scale;
} cs_t;
static PJGstruct Cs;

PJGstruct *PJGs_tm_init(double lat0, double central_lon, double scale)
    {
	PJGstruct *ps = (PJGstruct *) malloc(sizeof(PJGstruct));
	cs_t *cs = (cs_t *) ps;

	cs->Tm_lat0 = lat0 * DEG_TO_RAD;
	cs->Tm_lon0 = central_lon * DEG_TO_RAD;
	cs->Tm_scale = PJG_get_earth_radius() * scale;

	return ps;
    }


void PJGs_tm_latlon2xy(PJGstruct *ps, double lat, double lon, double *x, double *y)
/*
 *  Input:  lat, lon in degrees (lat N+, lon E+)
 *  Output: x, y in transverse mercator projection
 *     
 */
    {
	cs_t *cs = (cs_t *) ps;
    	double  dlon, b, tr;

    	lon *= DEG_TO_RAD;
    	lat *= DEG_TO_RAD;
    	dlon = lon - cs->Tm_lon0;
    	b = cos(lat) * sin(dlon);
    	tr = tan(lat)/cos(dlon);
   	*y = cs->Tm_scale * (atan(tr) - cs->Tm_lat0);
    	*x = cs->Tm_scale * atanh(b);
    }


void PJGs_tm_xy2latlon(PJGstruct *ps, double x, double y, double *lat, double *lon)
/*
 *  Input: x, y in transverse mercator projection
 *  Output:  lat, lon in degrees (lat N+, lon E+)
 *     
 */
    {
	cs_t *cs = (cs_t *) ps;
    	double d, lat2, lon2;

    	x /= cs->Tm_scale;
    	d =  y/cs->Tm_scale + cs->Tm_lat0;
    	lon2 = cs->Tm_lon0 + atan(sinh(x)/cos(d));
    	*lon = lon2 * RAD_TO_DEG;
    	lat2 = asin( sin(d)/cosh(x));
    	*lat = lat2 * RAD_TO_DEG;        
    }

/* old way of doing it */
void PJGtm_init(double lat0, double central_lon, double scale)
    {
	PJGstruct *ps;
	ps = PJGs_tm_init( lat0, central_lon, scale);
	memcpy( &Cs, ps, sizeof(cs_t));
	free(ps);
    }

void PJGtm_latlon2xy(double lat, double lon, double *x, double *y)
    {
	PJGs_tm_latlon2xy( &Cs, lat, lon, x, y);
    }

void PJGtm_xy2latlon( double x, double y, double *lat, double *lon)
    {
	PJGs_tm_xy2latlon( &Cs, x, y, lat, lon);
    }

