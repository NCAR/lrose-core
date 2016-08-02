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
 * EG_line_through_ellipse.c
 *
 * Finds the two intersection points between a line and a non-rotated
 * ellipse centered at (0,0).
 *
 * If line intersects ellipse, returns TRUE, and places the coords
 * of the intersection points as (xx1, yy1) and (xx2, yy2), xx1 <= xx2;
 *
 * If no intersection, returns FALSE.
 *
 * The points are found by the solution of a quadratic. If the quadratic
 * does not have real roots, there is no intersection.
 * 
 * Mike Dixon    RAP NCAR Boulder Colorado USA
 *
 * August 1991
 *
 *************************************************************************/

#include <euclid/geometry.h>
#include <math.h>

int EG_line_through_ellipse(double major_radius,
                            double minor_radius,
                            double slope,
                            double intercept,
                            double *xx1,
                            double *yy1,
                            double *xx2,
                            double *yy2)

{

  double a, b, c;                 /* quadratic coefficients */
  double discrim, sqrt_discrim;   /* discriminant */
  double minor_radius_sq, major_radius_sq;
  double intercept_sq, slope_sq;

  /*
   * compute the square of the args
   */

  minor_radius_sq = minor_radius * minor_radius;
  major_radius_sq = major_radius * major_radius;
  slope_sq = slope * slope;
  intercept_sq = intercept * intercept;

  /*
   * compute quadratic coefficients
   */

  a = minor_radius_sq / major_radius_sq + slope_sq;
  b = 2.0 * slope * intercept;
  c = intercept_sq - minor_radius_sq;

  discrim = b * b - 4.0 * a * c;

  if (discrim < 0.0) {

    /*
     * no real roots, so no intersection
     */

    return (FALSE);

  } else {

    sqrt_discrim = sqrt(discrim);

    *xx1 = (-b - sqrt_discrim) / (2.0 * a);
    *xx2 = (-b + sqrt_discrim) / (2.0 * a);

    *yy1 = *xx1 * slope + intercept;
    *yy2 = *xx2 * slope + intercept;

    return (TRUE);

  }

}
