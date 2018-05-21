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
/* lat_long.c - a program to convert from lat-longs to cartesian 
 * and vice-versa */

#include <stdio.h>
#include <math.h>
#include <euclid/geometry.h>
#include <toolsa/toolsa_macros.h>

#define TOLERANCE 1.e-8

static double _earthRadius = EARTH_RADIUS; /* from toolsa_macros.h */

/*********************************************************************
 * override the earth radius
 ********************************************************************/

void EG_set_earth_radius_km(double earth_radius_km)
{
  _earthRadius = earth_radius_km;
}

/*********************************************************************
 * EG_lat_lon_to_dx_dy()
 *
 *  Given the latitude and longitude of a start point in degrees as well
 *  as the latitude and longitude of a second point in degrees
 *  calculate the x, y kilometer coordinates of the second point.
 *
 *  Input : lat1, lon1, lat2, lon2 in degrees (lat N+, lon E+)
 *  Output: Dx, Dy = the delta (x, y) in km,
 *          x being pos east, y pos north
 *
 ********************************************************************/

void EG_lat_lon_to_dx_dy(double lat1, double lon1,
                         double lat2, double lon2,
                         double *dx, double *dy)
{

  double r;
  double darc, colat1, colat2, delon, denom, therad;
  
  colat1 = (90.0 - lat1) * DEG_TO_RAD;
  colat2 = (90.0 - lat2) * DEG_TO_RAD;
  delon = (lon2 - lon1) * DEG_TO_RAD;
  
  /*
   * same longitude : 0 or 180 deg
   */

  if (fabs(delon) <= TOLERANCE) {
    r = fabs(colat2 - colat1) * _earthRadius;
    *dx = 0.0;
    *dy = ((colat2 < colat1) ? r : -r);
    return;
  }
  
  /*
   * longitude differs by 180
   */

  if (fabs(180.0*DEG_TO_RAD - delon) <= TOLERANCE) {
    r = fabs(colat2 + colat1) * _earthRadius;
    *dx = 0.0;
    *dy = r;
    return;
  }
  
  darc = acos(cos(colat1)*cos(colat2) +
              sin(colat1)*sin(colat2)*cos( delon));
  
  r = darc* _earthRadius;
  
  denom = sin(colat1) * sin (darc);

  if ((fabs(colat1) <= TOLERANCE) || (fabs(denom) <= TOLERANCE)) 
    therad = 0.0;
  else
    therad = acos( (cos(colat2) - cos(colat1)*cos(darc)) / denom);
  
  if ((delon < 0.0) || (delon > 180.0 * DEG_TO_RAD))
    therad *= -1.0;
  
  *dx = r * sin (therad);
  *dy = r * cos (therad);

}

/*********************************************************************
 * EG_lat_lon_to_r_theta()
 *
 *  Input : lat1, lon1, lat2, lon2 in degrees (lat N+, lon E+)
 *  Output: r = the arc length from 1 to 2, in km 
 *            theta =  angle with True North: positive if east of North,
 *          negative if west of North, 0 = North
 *
 *********************************************************************/

void EG_lat_lon_to_r_theta(double lat1, double lon1,
                           double lat2, double lon2,
                           double *r, double *theta)
{

  double darc, colat1, colat2, delon, denom, therad;
  
  colat1 = (90.0 - lat1) * DEG_TO_RAD;
  colat2 = (90.0 - lat2) * DEG_TO_RAD;
  delon = (lon2 - lon1) * DEG_TO_RAD;
  
  /*
   * same longitude : 0 or 180 deg
   */

  if (fabs(delon) <= TOLERANCE) {
    *r = fabs(colat2 - colat1) * _earthRadius;
    *theta = ((colat2 < colat1) ? 0.0 : 180.0);
    return;
  }
  
  /*
   * longitude differs by 180
   */

  if (fabs(180.0*DEG_TO_RAD - delon) <= TOLERANCE) {
    *r = fabs(colat2 + colat1) * _earthRadius;
    *theta = 0.0;
    return;
  }
  
  darc = acos(cos(colat1)*cos(colat2) +
              sin(colat1)*sin(colat2)*cos( delon));
  
  *r = darc* _earthRadius;
  
  denom = sin(colat1) * sin (darc);

  if ((fabs(colat1) <= TOLERANCE) || (fabs(denom) <= TOLERANCE)) 
    therad = 0.0;
  else
    therad = acos( (cos(colat2) - cos(colat1)*cos(darc)) / denom);
  
  if ((delon < 0.0) || (delon > 180.0 * DEG_TO_RAD))
    therad *= -1.0;
  
  *theta = therad * RAD_TO_DEG;

}

/*******************************************************************
 * EG_lat_lon_plus_dx_dy()
 *
 *  Starting from a given lat, lon, draw an arc (part of a great circle)
 *  which moves dx, dy from the input lat, lon
 *
 *  Input : Starting point lat1, lon1 in degrees (lat N+, lon E+)
 *            arclengths dx, dy (km)
 *  Output: lat2, lon2, the ending point (degrees)
 *
 *******************************************************************/

void EG_lat_lon_plus_dx_dy(double lat1, double lon1,
                           double dx, double dy,
                           double *lat2, double *lon2)
{
  
  double r, theta;

  r = sqrt(dx * dx + dy * dy);

  if (dx == 0.0 && dy == 0.0)
    theta = 0.0;
  else
    theta = atan2(dx, dy) * RAD_TO_DEG;
  
  EG_lat_lon_plus_r_theta(lat1, lon1,
                          r, theta,
                          lat2, lon2);

}

/*******************************************************************
 * EG_lat_lon_plus_r_theta()
 *
 *  Starting from a given lat, lon, draw an arc (part of a great circle)
 *  of length r which makes an angle of theta from true North.
 *  Theta is positive if east of North, negative (or > PI) if west of North,
 *  0 = North
 *
 *  Input : Starting point lat1, lon1 in degrees (lat N+, lon E+)
 *            arclength r (km), angle theta (degrees)
 *  Output: lat2, lon2, the ending point (degrees)
 *
 *******************************************************************/

void EG_lat_lon_plus_r_theta(double lat1, double lon1,
                             double r, double theta,
                             double *lat2, double *lon2)
{
  
  double darc, colat1, colat2, denom, delta_lon, cost;
  
  darc = r / _earthRadius;
  
  if (fabs(darc) < TOLERANCE) {
    *lon2 = lon1;
    *lat2 = lat1;
    return;
  }
  
  /*
   * 180 degree lon case
   */

  if ((fabs(theta) < TOLERANCE) && (lat1 + darc*RAD_TO_DEG > 90.0)) {
    *lon2 = lon1 + 180.0;
    if (*lon2 > 180)
      *lon2 -= 360.0;
    
    (*lat2) = 180.0 - lat1 - darc*RAD_TO_DEG;
    return;
  }
  
  /*
   * 0 degree case
   */

  if ((fabs(theta) < TOLERANCE) || (fabs(180.0-theta) < TOLERANCE)) {
    *lon2 = lon1;
    (*lat2) = lat1*DEG_TO_RAD + ((fabs(theta) < 1.0) ? (darc) : (-darc));
    (*lat2) *= RAD_TO_DEG;
    return;
  }
  
  /*
   * 90 degree case
   */

  theta *= DEG_TO_RAD;
  cost = cos(theta);
  if (fabs(cost) < TOLERANCE) {
    *lat2 = lat1;
    (*lon2) = lon1*DEG_TO_RAD + ((theta > 0.0) ? (darc) : (-darc));
    (*lon2) *= RAD_TO_DEG;
    if (*lon2 < -180.0)
      *lon2 += 360.0;
    return;
  }

  colat1 = (90.0 - lat1) * DEG_TO_RAD;
  lon1 *= DEG_TO_RAD;
  
  colat2 = acos( cos( colat1)*cos( darc) + sin(colat1)*sin(darc)*cost);
  *lat2 = 90.0 - colat2 * RAD_TO_DEG;
  
  denom = sin (colat1) * sin(colat2);
  if ( fabs(denom) <= TOLERANCE)
    delta_lon = 0.0;
  else
    delta_lon = acos((cos(darc) - cos(colat1)*cos(colat2))/denom);
  
  /*
   * reverse sign if theta is west
   */

  if (sin(theta) < 0.0)
    delta_lon *= -1.0;
  
  *lon2 = (lon1 + delta_lon) * RAD_TO_DEG;
  if (*lon2 < -180.0)
    *lon2 += 360.0;
  
}
