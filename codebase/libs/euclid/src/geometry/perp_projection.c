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
 *   NAME
 *     perp_projection
 *
 *   PURPOSE
 *     Find the perpendicular projection point on a line from a given point.
 *
 *   NOTES
 *     
 *
 *   HISTORY
 *      wiener - May 8, 1995: Created.
 */

#include <euclid/copyright.h>
#include <euclid/geometry.h>

/*
 * perp_projection - Find the projection of a point to a line
 *
 * IN:  points c, d, e
 *
 * Here c, d determine a line segment and e is the point to be projected to
 * that line.
 *
 * OUT: the projection point f
 *
 * RETURNS: The distance from e to the line through c and d.
 *
 * If c == d then the function returns -1 and f is
 * set equal to c.
 *
 * If e is on the line from c through d, then f will be
 * set to e and the function will return 0.
 *
 * If e is not on the line from c through d, the function will return 1
 * if the perpendicular projection point is on the line segment cd
 * otherwise 2.  The point f will be set equal to the projected point.
 */
double EG_perp_projection(Point_d *c, Point_d *d, Point_d *e, Point_d *f)
{
  Point_d cd;
  Point_d ce;
  double length_cd;
  double length_ce;
  double prod;
  Point_d unit_vec;

  /* calculate the unit vector from c to d */
  EG_vect_sub(d, c, &cd);
  length_cd = EG_unit_vector(&cd, &unit_vec);
  if (length_cd < MACH_EPS)
    {
      /* c and d are identical */
      f->x = c->x;
      f->y = c->y;
      return(-1);
    }
  
  /* determine components of the vector from c to e */
  EG_vect_sub(e, c, &ce);
  length_ce = NORM(&ce);

  /* calculate the dot product of ce and unit_vec */
  prod = DOT(&ce, &unit_vec);
  if (ABS(length_ce - prod) < MACH_EPS)
    {
      /* e is on the line determined by cd so set f equal to e */
      f->x = e->x;
      f->y = e->y;
      return(0);
    }
  
  f->x = c->x + prod * unit_vec.x;
  f->y = c->y + prod * unit_vec.y;

  if (prod <= length_cd + MACH_EPS)
    return(1);
  else
    return(2);
}

