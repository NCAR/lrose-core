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
/**************************************************************************
 * upoint_in_ellipse.c
 *
 * Checks whether a given point lies inside a (rotated) ellipse. The search
 * radius is multiplied by a factor to allow the search to be extended
 * beyond the ellipse or restricted to a portion of the ellipse.
 *
 * If within, returns TRUE. Else, returns FALSE.
 *
 * Mike Dixon    RAP NCAR Boulder Colorado USA
 *
 * August 1991
 *
 *************************************************************************/

#include <math.h>

#include <rapmath/umath.h>

int upoint_in_ellipse(double ellipse_x, double ellipse_y,
		      double major_radius, double minor_radius,
		      double axis_rotation,
		      double search_x, double search_y,
		      double search_radius_ratio)
{

  double dx, dy;
  double distance_from_centroid, azimuth_from_centroid;
  double relative_azimuth;
  double ellipse_radius, search_radius;
  double ratio_factor;

  dx = search_x - ellipse_x;
  dy = search_y - ellipse_y;

  if (dx == 0.0 && dy == 0.0) {

    return (TRUE);

  } else {

    distance_from_centroid = sqrt(dx * dx + dy * dy);

    azimuth_from_centroid = atan2(dy, dx);

    relative_azimuth = azimuth_from_centroid -
      axis_rotation * DEG_TO_RAD;

    ratio_factor = pow(minor_radius / major_radius, 2.0) - 1.0;

    ellipse_radius = minor_radius /
      sqrt(1.0 + pow(cos(relative_azimuth), 2.0) * ratio_factor);

    search_radius = ellipse_radius * search_radius_ratio;

    if (distance_from_centroid <= search_radius)
      return (TRUE);
    else
      return (FALSE);

  }

}
