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
/*********************************************************************
 * update_ellipse_grid.c
 *
 * Flags regions in the forecast grid with 1's if the given
 * projected area ellipse crosses into or contains the region.
 *
 * The method uses 3 steps.
 *
 * 1) Flag the region containing the ellipse centroid.
 *
 * 2) Consider all vertical grid lines which intersect the ellipse.
 *    Flag all regions on either side of such a line for that
 *    line segment which crosses the ellipse.
 *
 * 3) Consider all horizontal grid lines which intersect the ellipse.
 *    Flag all regions on either side of such a line for that
 *    line segment which crosses the ellipse.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "verify_day.h"

#define SMALL_ANGLE 0.000001
#define PI_BY_TWO 1.570796327
#define ALMOST_PI_BY_TWO (PI_BY_TWO - SMALL_ANGLE)
#ifndef PI
#define PI 3.141592654
#endif
#define ALMOST_PI (PI - SMALL_ANGLE)

void update_ellipse_grid(double ellipse_x,
			 double ellipse_y,
			 double major_radius,
			 double minor_radius,
			 double axis_rotation,
			 ui08 **forecast_grid)

{

  si32 ix, iy, ix1, iy1, ix2, iy2;
  si32 start_ix, start_iy;
  si32 end_ix, end_iy;

  double eff_major_radius, eff_minor_radius;
  double grid_rotation, theta;
  double slope_prime, intercept_prime;
  double sin_rotation, cos_rotation, tan_rotation;

  double xprime1, yprime1, xprime2, yprime2;
  double x_1, y_1, x_2, y_2;
  double start_x, start_y;
  double end_x, end_y;
  double line_x, line_y;

  /*
   * compute the effective ellipse radii
   */

  eff_major_radius = major_radius * Glob->ellipse_radius_ratio;
  eff_minor_radius = minor_radius * Glob->ellipse_radius_ratio;

  /*
   * compute the grid_rotation, taking care to avoid 0, pi/2 and
   * pi, so that the trig functions will not fail. Remember that
   * the axis_rotation is relative to True North, and we need to
   * compute the grid rotation relative to the mathmatically
   * conventional axes
   */

  theta = 90.0 - axis_rotation;

  if (theta == 0.0)
    grid_rotation = SMALL_ANGLE;
  else if (theta == 90.0)
    grid_rotation = ALMOST_PI_BY_TWO;
  else if (theta == -90.0)
    grid_rotation = - ALMOST_PI_BY_TWO;
  else if (theta == 180.0 || theta == -180.0)
    grid_rotation = ALMOST_PI;
  else
    grid_rotation = theta * DEG_TO_RAD;

  sin_rotation = sin(grid_rotation);
  cos_rotation = cos(grid_rotation);
  tan_rotation = tan(grid_rotation);
  
  /*
   * compute the start and end x and y - these values are
   * chosen for a circle of radius eff_major_radius, which will
   * enclose the ellipse
   */

  start_x = ellipse_x - eff_major_radius;
  start_y = ellipse_y - eff_major_radius;

  end_x = ellipse_x + eff_major_radius;
  end_y = ellipse_y + eff_major_radius;

  /*
   * set the end and start grid indices
   */

  start_ix = (si32) ((start_x - Glob->minx) / Glob->dx + 0.49);
  start_ix = MAX(start_ix, 0);

  start_iy = (si32) ((start_y - Glob->miny) / Glob->dy + 0.49);
  start_iy = MAX(start_iy, 0);

  end_ix = (si32) ((end_x - Glob->minx) / Glob->dx + 0.51);
  end_ix = MIN(end_ix, Glob->nx - 1);

  end_iy = (si32) ((end_y - Glob->miny) / Glob->dy + 0.51);
  end_iy = MIN(end_iy, Glob->ny - 1);

  /*
   * flag the grid region which contains the ellipse centroid
   */

  ix = (si32) ((ellipse_x - Glob->minx) / Glob->dx + 0.5);
  iy = (si32) ((ellipse_y - Glob->miny) / Glob->dy + 0.5);

  if (ix >= start_ix && ix <= end_ix &&
      iy >= start_iy && iy <= end_iy)
    forecast_grid[iy][ix] = 1;

  /*
   * loop through the vertical lines which intersect the ellipse
   */

  for (ix = start_ix; ix < end_ix; ix++) {

    /*
     * compute the slope and intercept of this line in the
     * transformed coordinate system with ths origin at the
     * center of the ellipse and the x-axis along the major
     * axis. The prime values refer to the transformed
     * coord system.
     */

    
    line_x = Glob->minx + ((double) ix + 0.5) * Glob->dx;

    slope_prime = 1.0 / tan_rotation;

    intercept_prime  = - (line_x - ellipse_x) / sin_rotation;

    if (uline_through_ellipse(eff_major_radius, eff_minor_radius,
			      slope_prime, intercept_prime,
			      &xprime1, &yprime1,
			      &xprime2, &yprime2) == TRUE) {

      /*
       * transform the points back into grid coords
       */

      y_1 = ellipse_y + xprime1 * sin_rotation + yprime1 * cos_rotation;
      y_2 = ellipse_y + xprime2 * sin_rotation + yprime2 * cos_rotation;

      if  (y_1 <= y_2) {

	iy1 = (si32) ((y_1 - Glob->miny) / Glob->dy + 0.5);
	iy2 = (si32) ((y_2 - Glob->miny) / Glob->dy + 0.5);

      } else {

	iy1 = (si32) ((y_2 - Glob->miny) / Glob->dy + 0.5);
	iy2 = (si32) ((y_1 - Glob->miny) / Glob->dy + 0.5);

      }

      iy1 = MAX(iy1, 0);
      iy2 = MIN(iy2, Glob->ny - 1);

      for (iy = iy1; iy <= iy2; iy++) {

	forecast_grid[iy][ix] = 1;
	forecast_grid[iy][ix + 1] = 1;

      } /* iy */

    } /* if (uline_through_ellipse(eff_major_radius ... */

  } /* ix */

  /*
   * loop through the horizontal lines which intersect the ellipse
   */

  for (iy = start_iy; iy < end_iy; iy++) {

    /*
     * compute the slope and intercept of this line in the
     * transformed coordinate system with ths origin at the
     * center of the ellipse and the x-axis along the major
     * axis. The prime values refer to the transformed
     * coord system.
     */

    
    line_y = Glob->miny + ((double) iy + 0.5) * Glob->dy;

    slope_prime = - tan_rotation;

    intercept_prime  = (line_y - ellipse_y) / cos_rotation;

    if (uline_through_ellipse(eff_major_radius, eff_minor_radius,
			      slope_prime, intercept_prime,
			      &xprime1, &yprime1,
			      &xprime2, &yprime2) == TRUE) {

      /*
       * transform the points back into grid coords
       */

      x_1 = ellipse_x + xprime1 * cos_rotation - yprime1 * sin_rotation;
      x_2 = ellipse_x + xprime2 * cos_rotation - yprime2 * sin_rotation;

      if  (x_1 <= x_2) {

	ix1 = (si32) ((x_1 - Glob->minx) / Glob->dx + 0.5);
	ix2 = (si32) ((x_2 - Glob->minx) / Glob->dx + 0.5);

      } else {

	ix1 = (si32) ((x_2 - Glob->minx) / Glob->dx + 0.5);
	ix2 = (si32) ((x_1 - Glob->minx) / Glob->dx + 0.5);

      }

      ix1 = MAX(ix1, 0);
      ix2 = MIN(ix2, Glob->nx - 1);

      for (ix = ix1; ix <= ix2; ix++) {

	forecast_grid[iy][ix] = 1;
	forecast_grid[iy + 1][ix] = 1;

      } /* ix */

    } /* if (uline_through_ellipse(eff_major_radius ... */

  } /* iy */

}

