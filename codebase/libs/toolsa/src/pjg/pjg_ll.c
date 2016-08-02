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
/* pjg_ll.c : Projective Geometry "Latlon" Projection
 *
 * sets up the transformation as y = ykm_per_deg * (lat - lat0),
 *				 x = xkm_per_deg * (lon - lon0)
 */

#include "pjg_int.h"
#include <toolsa/pjg.h>
#include <toolsa/str.h>

typedef struct cs_t_ {
    int type;
    double Lat0, Lon0, xkm_per_deg, ykm_per_deg;
} cs_t;
static PJGstruct Cs;

PJGstruct *PJGs_ll_init( double lat0, double lon0, double xkm_per_deg, double ykm_per_deg)
    {
	PJGstruct *ps = (PJGstruct *) malloc(sizeof(PJGstruct));
	cs_t *cs = (cs_t *) ps;

	cs->Lat0 = lat0;
	cs->Lon0 = lon0;
	cs->xkm_per_deg = xkm_per_deg;
	cs->ykm_per_deg = ykm_per_deg;
	return ps;
    }

void PJGs_ll_latlon2xy(PJGstruct *ps, double lat, double lon, double *x, double *y)
/*
 *  Input:  lat, lon in degrees (lat N+, lon E+)
 *  Output: x, y in km
 *     
 */
    {
	cs_t *cs = (cs_t *) ps;

	*y = (lat - cs->Lat0) * cs->ykm_per_deg;
	*x = (lon - cs->Lon0) * cs->xkm_per_deg;
    }

void PJGs_ll_xy2latlon(PJGstruct *ps, double x, double y, double *lat, double *lon)
/*
 *  Input: x, y in km
 *  Output:  lat, lon in degrees (lat N+, lon E+)
 *     
 */
    {
	cs_t *cs = (cs_t *) ps;

	*lon = x / cs->xkm_per_deg + cs->Lon0;
	*lat = y / cs->ykm_per_deg + cs->Lat0;
    }


/* old way of doing it */
void PJGll_init( double lat0, double lon0, double xkm_per_deg, double ykm_per_deg)
    {
	PJGstruct *ps;
	ps = PJGs_ll_init( lat0, lon0, xkm_per_deg, ykm_per_deg);
	memcpy( &Cs, ps, sizeof(cs_t));
	free(ps);
    }

void PJGll_latlon2xy(double lat, double lon, double *x, double *y)
    {
	PJGs_ll_latlon2xy( &Cs, lat, lon, x, y);
    }

void PJGll_xy2latlon( double x, double y, double *lat, double *lon)
    {
	PJGs_ll_xy2latlon( &Cs, x, y, lat, lon);
    }

