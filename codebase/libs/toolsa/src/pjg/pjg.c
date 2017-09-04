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
/* pjg.c :   Projective Geometry interface routines

	Reference : "Map Projections used by the US Geological Survey"
		Bulletin 1532, 2nd Edition, John P. Snyder
	
 */

#include <stdio.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/err.h>

#include <toolsa/pjg.h>
#include "pjg_int.h"

/*****************************************************************************
 * earth radius
 */

static double _earthRadiusKm = EARTH_RADIUS;

void PJG_set_earth_radius(double val)
{
  _earthRadiusKm = val;
}

double PJG_get_earth_radius()
{
  return _earthRadiusKm;
}

/*****************************************************************************
 * projection names
 */

#define MAX_PROJ 10
static char *ProjName[MAX_PROJ] = 	{
					"Not Supported",
					"Oblique Stereographic",
					"Lambert Conformal",
					"Transverse Mercator",
					"Flat Earth",
					"Ellipsoid Stereographic",
					"Denver ARTCC",
					"Latlon",
					"Cylindrical Equidistant"
					};

/*****************************************************************************
 * get projection name
 */

char *PJGname( int proj_type)
    {
	if ((proj_type < 2) || (proj_type > MAX_PROJ))
	    return "Unknown";

	return ProjName[ proj_type-1];
    }
	

/*****************************************************************************
 * Initialize projection, allocates struct
 * Returns that struct.
 */

PJGstruct *PJGinit( int proj_type, double *p)
    {
	if ((proj_type < 2) || (proj_type > MAX_PROJ))
	    {
	    ERRprintf(ERR_PROGRAM, "PJGinit: Unknown projection type %d\n", proj_type);
	    return NULL;
	    }
	
	switch (proj_type) {
	    case PJG_OBLIQUE_STEREOGRAPHIC:
		return PJGs_ps_init( p[0], p[1], p[2], p[3], p[4]);
	    case PJG_LAMBERT_CONFORMAL2:
		return PJGs_lc2_init( p[0], p[1], p[2], p[3]);
	    case PJG_TRANSVERSE_MERCATOR:
		return PJGs_tm_init( p[0], p[1], p[2]);
	    case PJG_FLAT:
		return PJGs_flat_init( p[0], p[1], p[2]);
	    case PJG_STEREOGRAPHIC_ELLIPSOID:
		return PJGs_pse_init( p[0], p[1], p[2], p[3], p[4], p[5]);
	    case PJG_DENVER_ARTCC:
		return PJGs_art_init();
	    case PJG_LATLON:
		return PJGs_ll_init(p[0], p[1], p[2], p[3]);

	    /* case PJG_CYLINDICAL_EQUIDISTANT: */
	    /*     return PJGs_ce_init(p[0], p[1]); */

	    default:
		{
		ERRprintf( ERR_PROGRAM, "Projection %d = %s not available through PJGinit()",
			proj_type, PJGname( proj_type));
		return (PJGstruct *) NULL;
		}
		
	    }
    }



/*****************************************************************************
 * Free struct created by init()
 */

void PJG_free_struct(PJGstruct *ps)

{
  free(ps);
}

/*****************************************************************************
 * convert latlon to XY in current projection
 */

void PJGlatlon2xy(PJGstruct *proj, double lat, double lon, double *x, double *y)
/*
 *  Input:  lat, lon in degrees (lat N+, lon E+)
 *  Output: x, y in polar stereographic projection
 *     
 */
    {
	if (proj == NULL)
	    {
	    ERRprintf( ERR_PROGRAM, "PJGlatlon2xy: PJGstruct is NULL");
	    return;
	    }

	if ((proj->type < 2) || (proj->type > MAX_PROJ))
	    {
	    ERRprintf(ERR_PROGRAM, "PJGlatlon2xy: Unknown projection type %d\n", proj->type);
	    return;
	    }
	
	switch (proj->type) {
	    case PJG_OBLIQUE_STEREOGRAPHIC:
		PJGs_ps_latlon2xy( proj, lat, lon, x, y);
		return;
	    case PJG_LAMBERT_CONFORMAL2:
		PJGs_lc2_latlon2xy(  proj, lat, lon, x, y);
		return;
	    case PJG_TRANSVERSE_MERCATOR:
		PJGs_tm_latlon2xy(  proj, lat, lon, x, y);
		return;
	    case PJG_FLAT:
		PJGs_flat_latlon2xy(  proj, lat, lon, x, y);
		return;
	    case PJG_STEREOGRAPHIC_ELLIPSOID:
		PJGs_pse_latlon2xy(  proj, lat, lon, x, y);
		return;
	    case PJG_DENVER_ARTCC:
		PJGs_art_latlon2xy(  proj, lat, lon, x, y);
		return;
	    case PJG_LATLON:
		PJGs_ll_latlon2xy( proj, lat, lon, x, y);
		return;
/*
	    case PJG_CYLINDICAL_EQUIDISTANT:
		PJGs_ce_latlon2xy( proj, lat, lon, x, y);
		return;
 */
	    }
	return;

    }

/*****************************************************************************
 * convert XY in current projection to latlon
 */

void PJGxy2latlon(PJGstruct *proj, double x, double y, double *lat, double *lon)
/*
 *  Input:  lat, lon in degrees (lat N+, lon E+)
 *  Output: x, y in polar stereographic projection
 *     
 */
    {
	if (proj == NULL)
	    {
	    ERRprintf( ERR_PROGRAM, "PJGxy2latlon: PJGstruct is NULL");
	    return;
	    }
	if ((proj->type < 2) || (proj->type > MAX_PROJ))
	    {
	    ERRprintf(ERR_PROGRAM, "PJGxy2latlon: Unknown projection type %d\n", proj->type);
	    return;
	    }

	switch (proj->type) {
	    case PJG_OBLIQUE_STEREOGRAPHIC:
		PJGs_ps_xy2latlon( proj, x, y, lat, lon);
		return;
	    case PJG_LAMBERT_CONFORMAL2:
		PJGs_lc2_xy2latlon( proj, x, y, lat, lon);
		return;
	    case PJG_TRANSVERSE_MERCATOR:
		PJGs_tm_xy2latlon( proj, x, y, lat, lon);
		return;
	    case PJG_FLAT:
		PJGs_flat_xy2latlon( proj, x, y, lat, lon);
		return;
	    case PJG_STEREOGRAPHIC_ELLIPSOID:
		PJGs_pse_xy2latlon( proj, x, y, lat, lon);
		return;
	    case PJG_DENVER_ARTCC:
		PJGs_art_xy2latlon( proj, x, y, lat, lon);
		return;
	    case PJG_LATLON:
		PJGs_ll_xy2latlon( proj, x, y, lat, lon);
		return;

/*	    case PJG_CYLINDICAL_EQUIDISTANT:
		PJGs_ce_xy2latlon( proj, x, y, lat, lon);
		return;
 */
	    }
	return;
    }

