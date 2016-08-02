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
 * check_second_trip.c
 *
 * Checks for second trip  - returns TRUE or FALSE
 *
 * Check is based on aspect ratio of the echo, and how closely the
 * major axis of the fitted ellipse lies to the radial from the
 * radar to the storm centroid.
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * December 1992
 *
 ****************************************************************************/

#include "storm_ident.h"
#include <math.h>

#define RAD_TO_DEG 57.29577951308092

int check_second_trip(double top,
		      double base,
		      double centroid_x,
		      double centroid_y,
		      double major_radius,
		      double minor_radius,
		      double orientation)

{

  int orientation_OK;
  int vert_aspect_OK;
  int horiz_aspect_OK;

  double vert_aspect;
  double horiz_aspect;
  double radial_azimuth;
  double delta_orientation;

  if (centroid_x == 0.0 && centroid_y == 0.0)
    radial_azimuth = 0.0;
  else
    radial_azimuth = atan2(centroid_x, centroid_y) * RAD_TO_DEG;

  if (radial_azimuth < 0.0)
    radial_azimuth += 180.0;

  delta_orientation = fabs(radial_azimuth - orientation);

  if (delta_orientation < Glob->params.sectrip_orientation_error ||
      180.0 - delta_orientation < Glob->params.sectrip_orientation_error)
    orientation_OK = TRUE;
  else
    orientation_OK = FALSE;

  vert_aspect = major_radius / (top - base);
  horiz_aspect = major_radius / minor_radius;

  if (vert_aspect > Glob->params.sectrip_vert_aspect)
    vert_aspect_OK = TRUE;
  else
    vert_aspect_OK = FALSE;

  if (horiz_aspect > Glob->params.sectrip_horiz_aspect)
    horiz_aspect_OK = TRUE;
  else
    horiz_aspect_OK = FALSE;


  if (Glob->params.debug >= DEBUG_VERBOSE &&
      orientation_OK && vert_aspect_OK && horiz_aspect_OK) {

    fprintf(stderr, "\n++++++++++ SECOND_TRIP ++++++++++++++\n");

    fprintf(stderr, "x,y : %g, %g\n", centroid_x, centroid_y);
    fprintf(stderr, "top, base : %g, %g\n", top, base);
    fprintf(stderr, "maj, min rad : %g, %g\n", major_radius,
	    minor_radius);
    fprintf(stderr, "orientation : %g\n", orientation);

    fprintf(stderr, "rad az : %g\n", radial_azimuth);
    fprintf(stderr, "delta orientation : %g\n", delta_orientation);
    fprintf(stderr, "orientation OK : %d\n", orientation_OK);
    fprintf(stderr, "vert, horiz aspect: %g, %g\n",
	    vert_aspect, horiz_aspect);
    fprintf(stderr, "vert_OK, horiz_OK : %d, %d\n\n",
	   vert_aspect_OK, horiz_aspect_OK);

  }

  if (orientation_OK && vert_aspect_OK && horiz_aspect_OK)
    return (TRUE);
  else
    return (FALSE);

}
