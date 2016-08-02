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
/*
 * NAME
 *	init_ray
 *
 * PURPOSE
 *	initialize array of rays
 *
 * NOTES
 *
 * HISTORY
 *     wiener - May 11, 1992: Created.
 */

#include <math.h>
#include <euclid/boundary.h>
#include <euclid/point.h>

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

/*
 * Divide a circle into "div" divisions about a reference point, ref_pt.
 * Store the ray endpoints in ray.
 */
double EG_init_ray(Point_d *ray, int div, Point_d *ref_pt)
{
  double angle;
  int i;
  double theta;

  angle = 0;
  theta = 2 * M_PI/div;

  /* initialize the ray array */
  for (i=0; i<div; i++)
    {
      ray[i].x = cos(angle) + ref_pt->x;
      ray[i].y = sin(angle) + ref_pt->y;
      angle += theta;
    }

  return(theta);
} /* init_ray */

/*
 * Divide a circle into "div" divisions about a reference point, ref_pt.
 * Store the ray endpoints in ray. This is done relative to True North,
 * instead of the usual Cartesian axes.
 */
double EG_init_ray_TN(Point_d *ray, int div, Point_d *ref_pt)
{
  double angle;
  int i;
  double theta;

  angle = 0;
  theta = (2 * M_PI)/div;

  /* initialize the ray array */
  for (i=0; i<div; i++)
    {
      ray[i].x = sin(angle) + ref_pt->x;
      ray[i].y = cos(angle) + ref_pt->y;
      angle += theta;
    }

  return(theta);
} /* init_ray */

