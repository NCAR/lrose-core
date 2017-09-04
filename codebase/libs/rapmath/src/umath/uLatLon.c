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
/***************************************************************************
 * uLatLon.c
 *
 * Routines for lat, lon computations
 *
 * All latitudes in range (90, -90)
 * All longitudes in the range [180, -180]
 *
 * use cosine law: cos a = cos b cos c + sin b sin c cos A
 *
 * Code developed by John Caron
 *
 * RAP NCAR Boulder Colorado USA
 *
 * November 1991
 *
 ***************************************************************************/

#include <math.h>
#include <rapmath/trig.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pjg.h>

#define TOLERANCE 1.e-8
#define TINY_FLOAT 1.e-10
#define TINY_ANGLE 1.e-4
#define TINY_DIST 1.e-2

void uLatLon2DxDy(double lat1, double lon1,
		  double lat2, double lon2,
		  double *dx, double *dy);

void uLatLon2RTheta(double lat1, double lon1,
		    double lat2, double lon2,
		    double *r, double *theta);

void uLatLonPlusDxDy(double lat1, double lon1,
		     double dx, double dy,
		     double *lat2, double *lon2);

void uLatLonPlusRTheta(double lat1, double lon1,
		       double r, double theta,
		       double *lat2, double *lon2);

/*********************************************************************
 * uLatLon2DxDy()
 *
 *  Input : lat1, lon1, lat2, lon2 in degrees (lat N+, lon E+)
 *  Output: Dx, Dy = the delta (x, y) in km,
 *          x being pos east, y pos north
 *
 ********************************************************************/

void uLatLon2DxDy(double lat1, double lon1,
		  double lat2, double lon2,
		  double *dx, double *dy)
{

  double r, theta;
  double theta_rad, sin_theta, cos_theta;
  
  uLatLon2RTheta(lat1, lon1, lat2, lon2, &r, &theta);
  
  theta_rad = theta * DEG_TO_RAD;
  rap_sincos(theta_rad, &sin_theta, &cos_theta);

  *dx = r * sin_theta;
  *dy = r * cos_theta;

}

/*********************************************************************
 * uLatLon2RTheta()
 *
 *  Input : lat1, lon1, lat2, lon2 in degrees (lat N+, lon E+)
 *  Output: r = the arc length from 1 to 2, in km 
 *	    theta =  angle with True North: positive if east of North,
 *          negative if west of North, 0 = North
 *
 *********************************************************************/

void uLatLon2RTheta(double lat1, double lon1,
		    double lat2, double lon2,
		    double *r, double *theta)
{

  double darc, colat1, colat2, delon, denom, therad;
  double cos_colat1, sin_colat1;
  double cos_colat2, sin_colat2;
  double sin_darc, cos_darc;
  double xx;

  colat1 = (90.0 - lat1) * DEG_TO_RAD;
  colat2 = (90.0 - lat2) * DEG_TO_RAD;
  delon = (lon2 - lon1) * DEG_TO_RAD;

  if (delon < -M_PI) {
    delon += 2.0 * M_PI;
  }
  
  if (delon > M_PI) {
    delon -= 2.0 * M_PI;
  }

  rap_sincos(colat1, &sin_colat1, &cos_colat1);
  rap_sincos(colat2, &sin_colat2, &cos_colat2);

  xx = cos_colat1 * cos_colat2 + sin_colat1 * sin_colat2 * cos(delon);
  if (xx < -1.0) xx = -1.0;
  if (xx > 1.0) xx = 1.0;
  darc = acos(xx);
  rap_sincos(darc, &sin_darc, &cos_darc);
  
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
 * uLatLonPlusDxDy()
 *
 *  Starting from a given lat, lon, draw an arc (part of a great circle)
 *  which moves dx, dy from the input lat, lon
 *
 *  Input : Starting point lat1, lon1 in degrees (lat N+, lon E+)
 *	    arclengths dx, dy (km)
 *  Output: lat2, lon2, the ending point (degrees)
 *
 *******************************************************************/

void uLatLonPlusDxDy(double lat1, double lon1,
		     double dx, double dy,
		     double *lat2, double *lon2)
{
  
/*  double darc, colat1, colat2, denom, delta_lon, cost; */
  double r, theta;

  r = sqrt(dx * dx + dy * dy);

  if (dx == 0.0 && dy == 0.0)
    theta = 0.0;
  else
    theta = atan2(dx, dy) * RAD_TO_DEG;
  
  uLatLonPlusRTheta(lat1, lon1,
		    r, theta,
		    lat2, lon2);

}

/*******************************************************************
 * uLatLonPlusRTheta()
 *
 *  Starting from a given lat, lon, draw an arc (part of a great circle)
 *  of length r which makes an angle of theta from true North.
 *  Theta is positive if east of North, negative (or > PPI) if west of North,
 *  0 = North
 *
 *  Input : Starting point lat1, lon1 in degrees (lat N+, lon E+)
 *	    arclength r (km), angle theta (degrees)
 *  Output: lat2, lon2, the ending point (degrees)
 *
 *******************************************************************/

void uLatLonPlusRTheta(double lat1, double lon1,
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
  rap_sincos(darc, &sin_darc, &cos_darc);

  therad = theta * DEG_TO_RAD;
  rap_sincos(therad, &sin_therad, &cos_therad);
  cost = cos_therad;

  colat1 = (90.0 - lat1) * DEG_TO_RAD;
  rap_sincos(colat1, &sin_colat1, &cos_colat1);
  lon1 *= DEG_TO_RAD;
  
  xx = cos_colat1 * cos_darc + sin_colat1 * sin_darc * cost;
  if (xx < -1.0) xx = -1.0;
  if (xx > 1.0) xx = 1.0;
  colat2 = acos(xx);
  *lat2 = 90.0 - colat2 * RAD_TO_DEG;
  rap_sincos(colat2, &sin_colat2, &cos_colat2);
  
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

