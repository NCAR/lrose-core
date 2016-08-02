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
/* pjg_grid.c
 *
 * Grid manipulations
 *
 */

#include "pjg_int.h"
#include <toolsa/os_config.h>
#include <toolsa/pjg_types.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/sincos.h>
#include <stdio.h>
#include <math.h>

/*
 * check_flat() checks that the grid projection is flat and
 * that the units are km or nm.
 *
 * Relevant messages are printed.
 *
 * Exits on error, because this is fatal.
 */

static void check_flat(pjg_grid_geom_t *geom)

{
  
  if (geom->type != PJG_FLAT) {
    fprintf(stderr, "ERROR - pjg_grid\n");
    fprintf(stderr, "Grid not flat projection\n");
    exit (-1);
  }
  
  if (geom->units != PJG_UNITS_KM &&
      geom->units != PJG_UNITS_NM) {
    fprintf(stderr, "ERROR - pjg_grid\n");
    fprintf(stderr, "Grid units not km or nm\n");
    exit(-1);
  }

}

/***********************
 * PJG_grid_geom_print()
 *
 * Prints the grid geom
 *
 */

void PJG_grid_geom_print(pjg_grid_geom_t *geom, FILE *fp)

{

  fprintf(fp, "------------------------\n");

  fprintf(fp, "ref_lat, ref_lon = %g, %g\n",
	  geom->ref_lat, geom->ref_lon);

  fprintf(fp, "rotation = %g\n", geom->rotation);

  fprintf(fp, "min_x, min_y = %g, %g\n",
	  geom->min_x, geom->min_y);

  fprintf(fp, "dx, dy = %g, %g\n",
	  geom->dx, geom->dy);

  fprintf(fp, "nx, ny = %d, %d\n",
	  geom->nx, geom->ny);

  switch (geom->type) {
  case PJG_FLAT:
    fprintf(fp, "type: flat\n");
    break;
  case PJG_LATLON:
    fprintf(fp, "type: latlon\n");
    break;
  default:
    fprintf(fp, "type: unsupported\n");
  }

  fprintf(fp, "units: %s\n", pjg_units_str(geom->units));

  fprintf(fp, "------------------------\n");
  fflush(fp);

  return;

}

/*********************
 * PJG_fgrid_latloni()
 *
 * Returns the latlon for a given integer grid location
 *
 */

void PJG_fgrid_latloni(pjg_grid_geom_t *geom,
		       int ix, int iy,
		       double *lat, double *lon)

{

  PJG_fgrid_latlond(geom,
		    (double) ix, (double) iy,
		    lat, lon);

}

/*********************
 * PJG_fgrid_latlond()
 *
 * Returns the latlon for a given double grid location
 *
 */

void PJG_fgrid_latlond(pjg_grid_geom_t *geom,
		       double ixd, double iyd,
		       double *lat, double *lon)

{

  double scale;
  double x, y;
  double arclen, theta;
  
#ifndef NDEBUG
  check_flat(geom);
#endif

  if (geom->units == PJG_UNITS_NM) {
    scale = KM_PER_NM;
  } else {
    scale = 1.0;
  }

  x = (geom->min_x + ixd * geom->dx) * scale;
  y = (geom->min_y + iyd * geom->dy) * scale;
  arclen = sqrt(x * x + y * y);

  if (x == 0.0 && y == 0.0) {
    theta = geom->rotation;
  } else {
    theta = atan2(x, y) * RAD_TO_DEG + geom->rotation; /* rel to TN */
  }

  PJGLatLonPlusRTheta(geom->ref_lat, geom->ref_lon,
		      arclen, theta,
		      lat, lon);
  
/*  fprintf(stderr, "PJG_fgrid_latlond: arclen, theta: %g, %g\n",
	  arclen, theta); */

  return;
  
}

/*****************
 * PJG_fgrid_xyi()
 *
 * Returns the integer grid location for a given latlon.
 *
 */

void PJG_fgrid_xyi(pjg_grid_geom_t *geom,
		   double lat, double lon,
		   int *ix, int *iy)

{

  double ixd, iyd;

  PJG_fgrid_xyd(geom, lat, lon, &ixd, &iyd);

  *ix = (int) (ixd + 0.5);
  *iy = (int) (iyd + 0.5);

  return;
  
}

/*****************
 * PJG_fgrid_xyd()
 *
 * Returns the double grid location for a given latlon.
 *
 */

void PJG_fgrid_xyd(pjg_grid_geom_t *geom,
		   double lat, double lon,
		   double *ixd, double *iyd)

{

  double scale;
  double x, y;
  double arclen, theta;
  double grid_theta;
  double sin_grid_theta, cos_grid_theta;
  
#ifndef NDEBUG
  check_flat(geom);
#endif

  if (geom->units == PJG_UNITS_NM) {
    scale = KM_PER_NM;
  } else {
    scale = 1.0;
  }

  PJGLatLon2RTheta(geom->ref_lat, geom->ref_lon,
		   lat, lon,
		   &arclen, &theta);

  grid_theta = (theta - geom->rotation) * DEG_TO_RAD;
  ta_sincos(grid_theta, &sin_grid_theta, &cos_grid_theta);
  x = (arclen * sin_grid_theta) / scale;
  y = (arclen * cos_grid_theta) / scale;

  *ixd = (x - geom->min_x) / geom->dx;
  *iyd = (y - geom->min_y) / geom->dy;

  return;

}

