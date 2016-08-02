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
/****************************************************************************
 * wind.c - Routines for calculating different wind values.
 *
 *********************************************************************/

#include <math.h>
#include <stdio.h>

#include <physics/physics.h>
#include <physics/physics_macros.h>

/****************************************************************************
 * PHYwind_speed - Calculate the wind speed given the U and V values.
 *
 * Expects the u and v values to be given in m/sec, returns the wind
 * speed value in m/sec.
 *
 * Returns -9999.0 if the wind speed cannot be calculated from the
 * given vectors.
 *********************************************************************/

double PHYwind_speed(const double u, const double v)
{
  if (u < -999.0 || v < -999.0)
    return -9999.0;
  
  return sqrt(u*u + v*v);
}


/****************************************************************************
 * PHYwind_dir - Calculate the wind direction given the U and V components.
 *
 * Expects the U and V components in m/sec, returns the wind direction in
 * degrees.  The returned wind direction is normalized to be between
 * 0.0 and 360.0.
 *
 * Returns -9999.0 if the V component cannot be calculated from the
 * given data.
 *********************************************************************/

double PHYwind_dir(const double u, const double v)
{
  double wind_dir;
  
  if (u < -999.0 || v < -999.0)
    return -9999.0;

  /* Calculate the wind direction */
  
  if (v == 0.0 && u == 0.0)
    wind_dir = 0.0;
  else
    wind_dir = atan2(-u, -v) / DEG_TO_RAD;

  /* Normalize the direction value */

  if (wind_dir < 0)
    wind_dir += 360.0;

  return wind_dir;
}


/****************************************************************************
 * PHYwind_u - Calculate the U component of the wind given the wind
 *             speed and direction.
 *
 * Expects the wind_speed value in m/sec and the wind_dir value in
 * degrees, returns the U component in m/sec.
 *
 * Returns -9999.0 if the U component cannot be calculated from the
 * given data.
 *********************************************************************/

double PHYwind_u(const double wind_speed, const double wind_dir)
{
  if (wind_speed < -999.0 || wind_dir < -999.0)
    return -9999.0;
  
  return -1.0 * (sin(wind_dir * DEG_TO_RAD) * wind_speed);
}


/****************************************************************************
 * PHYwind_v - Calculate the V component of the wind given the wind
 *             speed and direction.
 *
 * Expects the wind_speed value in m/sec and the wind_dir value in
 * degrees, returns the V component in m/sec.
 *
 * Returns -9999.0 if the V component cannot be calculated from the
 * given data.
 *********************************************************************/

double PHYwind_v(const double wind_speed, const double wind_dir)
{
  if (wind_speed < -999.0 || wind_dir < -999.0)
    return -9999.0;
  
  return -1.0 * (cos(wind_dir * DEG_TO_RAD) * wind_speed);
}
