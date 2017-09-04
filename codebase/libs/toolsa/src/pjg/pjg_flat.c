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
/* pjg_flat.c : Projective Geometry "Flat Earth" Projection

    This projection surface is tangent at some point (lat0, lon0) and 
    has a y axis rotated from true North by some angle. 

    We call it "flat" because it should only be used where the spherical
    geometry of the earth is not significant. In actuallity, we use the simple
    "arclen" routine which computes dy along a meridian, and dx along a
    latitude circle.  We rotate the coordinate system to/from a true north system.
 */

#include <memory.h>
#include "pjg_int.h"
#include <toolsa/pjg.h>
#include <toolsa/sincos.h>

/*
 * routines for generic treatment of projection math
 */

typedef struct cs_t_ {
    int type;
    double lat0, lon0, rot_angle;
} cs_t;
static PJGstruct Cs;

/*
 * initialize flat earth projection
 */

PJGstruct *PJGs_flat_init(double lat0, double lon0, double rot_angle)

{
  PJGstruct *ps = (PJGstruct *) malloc(sizeof(PJGstruct));
  cs_t *cs = (cs_t *) ps;
  cs->lat0 = lat0;
  cs->lon0 = lon0;
  cs->rot_angle = rot_angle;
  return ps;	
}

/*
 *  Input:  lat, lon in degrees (lat N+, lon E+)
 *  Output: x, y in flat earth projection 
 *     
 * 	must call PJGflat_init() first
 */

void PJGs_flat_latlon2xy(PJGstruct *ps,
			 double lat, double lon,
			 double  *x, double *y)

{

  double r, theta;
  cs_t *cs = (cs_t *) ps;
  double dtheta, sin_dtheta, cos_dtheta;
  
  PJGLatLon2RTheta(cs->lat0, cs->lon0, lat, lon, &r, &theta);

  dtheta = (theta - cs->rot_angle) * DEG_TO_RAD;
  ta_sincos(dtheta, &sin_dtheta, &cos_dtheta);

  *x = r * sin_dtheta;
  *y = r * cos_dtheta;

}


/*
 *  Input: x, y in flat earth projection
 *  Output:  lat, lon in degrees (lat N+, lon E+)
 *     
 */
void PJGs_flat_xy2latlon(PJGstruct *ps, 
			 double x, double y,
			 double *lat, double *lon)

{
  double r, phi, theta;
  cs_t *cs = (cs_t *) ps;

  r = sqrt(x * x + y * y);
  if (x == 0.0 && y == 0.0) {
    phi = 0.0;
  } else {
    phi = atan2(x, y) * RAD_TO_DEG;
  }
  
  theta = phi + cs->rot_angle;

  PJGLatLonPlusRTheta(cs->lat0, cs->lon0,
		      r, theta,
		      lat, lon);

}

void PJGflat_init(double lat0, double lon0, double rot_angle)
    {
      PJGstruct *ps;
      ps = PJGs_flat_init(lat0, lon0, rot_angle);
      Cs = *ps;
      free(ps);
    }

void PJGflat_latlon2xy(double lat, double lon, double *x, double *y)
    {
      PJGs_flat_latlon2xy(&Cs, lat, lon, x, y);
    }

void PJGflat_xy2latlon(double x, double y, double *lat, double *lon)
    {
      PJGs_flat_xy2latlon(&Cs, x, y, lat, lon);
    }

/*********************************************************************
 * PJGLatLon2DxDy()
 *
 *  Input : lat1, lon1, lat2, lon2 in degrees (lat N+, lon E+)
 *  Output: Dx, Dy = the delta (x, y) in km,
 *          x being pos east, y pos north
 *
 ********************************************************************/

void PJGLatLon2DxDy(double lat1, double lon1,
		    double lat2, double lon2,
		    double *dx, double *dy)
{

  double r, theta;
  double theta_rad, sin_theta, cos_theta;
  
  PJGLatLon2RTheta(lat1, lon1, lat2, lon2, &r, &theta);
  
  theta_rad = theta * DEG_TO_RAD;
  ta_sincos(theta_rad, &sin_theta, &cos_theta);

  *dx = r * sin_theta;
  *dy = r * cos_theta;

}

/*********************************************************************
 * PJGLatLon2RTheta()
 *
 *  Input : lat1, lon1, lat2, lon2 in degrees (lat N+, lon E+)
 *  Output: r = the arc length from 1 to 2, in km 
 *	    theta =  angle with True North: positive if east of North,
 *          negative if west of North, 0 = North
 *
 *********************************************************************/

void PJGLatLon2RTheta(double lat1, double lon1,
		      double lat2, double lon2,
		      double *r, double *theta)
{

  double darc, colat1, colat2, delon, denom, therad;
  double cos_colat1, sin_colat1;
  double cos_colat2, sin_colat2;
  double xx;
  double sin_darc, cos_darc;

  colat1 = (90.0 - lat1) * DEG_TO_RAD;
  colat2 = (90.0 - lat2) * DEG_TO_RAD;
  delon = (lon2 - lon1) * DEG_TO_RAD;

  if (delon < -M_PI) {
    delon += 2.0 * M_PI;
  }
  
  if (delon > M_PI) {
    delon -= 2.0 * M_PI;
  }

  ta_sincos(colat1, &sin_colat1, &cos_colat1);
  ta_sincos(colat2, &sin_colat2, &cos_colat2);

  xx = cos_colat1 * cos_colat2 + sin_colat1 * sin_colat2 * cos(delon);
  if (xx < -1.0) xx = -1.0;
  if (xx > 1.0) xx = 1.0;
  darc = acos(xx);
  ta_sincos(darc, &sin_darc, &cos_darc);
  
  *r = darc * PJG_get_earth_radius();
  
  denom = sin_colat1 * sin_darc;

  if ((fabs(colat1) <= TINY_ANGLE) || (fabs(denom) <= TINY_FLOAT)) {
    therad = 0.0;
  } else {
    xx = (cos_colat2 - cos_colat1 * cos_darc) / denom;
    if (xx < -1.0) xx = -1.0;
    if (xx > 1.0) xx = 1.0;
    therad = acos(xx);
  }
  
  if ((delon < 0.0) || (delon > M_PI))
    therad *= -1.0;
  
  *theta = therad * RAD_TO_DEG;

}

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

void PJGLatLonPlusDxDy(double lat1, double lon1,
		       double dx, double dy,
		       double *lat2, double *lon2)
{
  
  double r, theta;

  r = sqrt(dx * dx + dy * dy);

  if (dx == 0.0 && dy == 0.0)
    theta = 0.0;
  else
    theta = atan2(dx, dy) * RAD_TO_DEG;
  
  PJGLatLonPlusRTheta(lat1, lon1,
		      r, theta,
		      lat2, lon2);

}

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

void PJGLatLonPlusRTheta(double lat1, double lon1,
			 double r, double theta,
			 double *lat2, double *lon2)
{
  
  double darc, colat1, colat2, denom, delta_lon, cost, therad;
  double xx;
  double sin_therad, cos_therad;
  double cos_colat1, sin_colat1;
  double cos_colat2, sin_colat2;
  double sin_darc, cos_darc;
  
  darc = r / PJG_get_earth_radius();
  ta_sincos(darc, &sin_darc, &cos_darc);

  therad = theta * DEG_TO_RAD;
  ta_sincos(therad, &sin_therad, &cos_therad);
  cost = cos_therad;

  colat1 = (90.0 - lat1) * DEG_TO_RAD;
  ta_sincos(colat1, &sin_colat1, &cos_colat1);
  lon1 *= DEG_TO_RAD;
  
  xx = cos_colat1 * cos_darc + sin_colat1 * sin_darc * cost;
  if (xx < -1.0) xx = -1.0;
  if (xx > 1.0) xx = 1.0;
  colat2 = acos(xx);
  *lat2 = 90.0 - colat2 * RAD_TO_DEG;
  ta_sincos(colat2, &sin_colat2, &cos_colat2);
  
  denom = sin_colat1 * sin_colat2;
  if ( fabs(denom) <= TINY_FLOAT) {
    delta_lon = 0.0;
  } else {
    xx = (cos_darc - cos_colat1 * cos_colat2)/denom;
    if (xx < -1.0) xx = -1.0;
    if (xx > 1.0) xx = 1.0;
    delta_lon = acos(xx);
  }

  /*
   * reverse sign if theta is west
   */

  if (sin_therad < 0.0)
    delta_lon *= -1.0;
  
  *lon2 = (lon1 + delta_lon) * RAD_TO_DEG;

  if (*lon2 < -180.0)
    *lon2 += 360.0;
  if (*lon2 > 180.0)
    *lon2 -= 360.0;

  return;
  
}
