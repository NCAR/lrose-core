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
#ifndef PJG_FLAT_WAS_INCLUDED
#define PJG_FLAT_WAS_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>

#include <toolsa/pjg_types.h>

/********** "Flat earth" projection **************/

extern PJGstruct *PJGs_flat_init(double lat0, double lon0, double rot_angle);
extern void PJGs_flat_latlon2xy(PJGstruct *ps,
				double lat, double lon,
				double  *x, double *y);
extern void PJGs_flat_xy2latlon(PJGstruct *ps, 
				double x, double y,
				double *lat, double *lon);

extern void PJGflat_init( double lat0, double lon0, double rot_angle);
/* precompute constants for "flat earth, rotated" projection 

    This projection surface is tangent at some point (lat0, lon0) and 
    has a y axis rotated from true North by rot_angle. 
 
    lat0, lon0 : origin (point where x,y = 0,0)
    rot_angle: angle in degrees that the y axis makes with true North (positive in clockwise
	direction).

   you must call this before you call PJGflat_latlon2xy and PJGflat_xy2latlon
 */

extern void PJGflat_latlon2xy( double lat, double lon, double *x, double *y);
/*
 *  Input:  lat, lon in degrees (lat N+, lon E+)
 *  Output: x, y on flat earth projection plane 
 *  Call  PJGflat_init() first  
 */

extern void PJGflat_xy2latlon( double x, double y, double *lat, double *lon);
/*
 *  Input: x, y on flat earth projection plane 
 *  Output:  lat, lon in degrees (lat N+, lon E+)
 *  Call  PJGflat_init() first     
 */

extern void PJGLatLon2DxDy(double lat1, double lon1,
			   double lat2, double lon2,
			   double *dx, double *dy);
/*********************************************************************
 * PJGLatLon2DxDy()
 *
 *  Input : lat1, lon1, lat2, lon2 in degrees (lat N+, lon E+)
 *  Output: Dx, Dy = the delta (x, y) in km,
 *          x being pos east, y pos north
 *
 ********************************************************************/

extern void PJGLatLon2RTheta(double lat1, double lon1,
			     double lat2, double lon2,
			     double *r, double *theta);
/*********************************************************************
 * PJGLatLon2RTheta()
 *
 *  Input : lat1, lon1, lat2, lon2 in degrees (lat N+, lon E+)
 *  Output: r = the arc length from 1 to 2, in km 
 *	    theta =  angle with True North: positive if east of North,
 *          negative if west of North, 0 = North
 *
 *********************************************************************/

extern void PJGLatLonPlusDxDy(double lat1, double lon1,
			      double dx, double dy,
			      double *lat2, double *lon2);
/*******************************************************************
 * PJGLatLonPlusDxDy()
 *
 *  Starting from a given lat, lon, draw an arc (part of a great circle)
 *  which moves dx, dy from the input lat, lon
 *
 *  Input : Starting point lat1, lon1 in degrees (lat N+, lon E+)
 *	    arclengths dx, dy (km)
 *  Output: lat2, lon2, the ending point (degrees)
 *
 *******************************************************************/

extern void PJGLatLonPlusRTheta(double lat1, double lon1,
				double r, double theta,
				double *lat2, double *lon2);
/*******************************************************************
 * PJGLatLonPlusRTheta()
 *
 *  Starting from a given lat, lon, draw an arc (part of a great circle)
 *  of length r which makes an angle of theta from true North.
 *  Theta is positive if east of North, negative (or > PI) if west of North,
 *  0 = North
 *
 *  Input : Starting point lat1, lon1 in degrees (lat N+, lon E+)
 *	    arclength r (km), angle theta (degrees)
 *  Output: lat2, lon2, the ending point (degrees)
 *
 *******************************************************************/

extern void PJG_fgrid_latloni(pjg_grid_geom_t *geom,
			      int ix, int iy,
			      double *lat, double *lon);

/*********************
 * PJG_fgrid_latloni()
 *
 * Returns the latlon for a given integer grid location
 *
 */

extern void PJG_fgrid_latlond(pjg_grid_geom_t *geom,
			      double ixd, double iyd,
			      double *lat, double *lon);

/*********************
 * PJG_fgrid_latlond()
 *
 * Returns the latlon for a given double grid location
 *
 */

extern void PJG_fgrid_xyi(pjg_grid_geom_t *geom,
			  double lat, double lon,
			  int *ix, int *iy);

/*****************
 * PJG_fgrid_xyi()
 *
 * Returns the integer grid location for a given latlon.
 *
 */

extern void PJG_fgrid_xyd(pjg_grid_geom_t *geom,
			  double lat, double lon,
			  double *ixd, double *iyd);

/*****************
 * PJG_fgrid_xyd()
 *
 * Returns the double grid location for a given latlon.
 *
 */

extern void PJG_grid_geom_print(pjg_grid_geom_t *geom, FILE *fp);

/***********************
 * PJG_grid_geom_print()
 *
 * Prints the grid geom
 *
 */

#ifdef __cplusplus
}
#endif

#endif /* PJG_FLAT_WAS_INCLUDED */

