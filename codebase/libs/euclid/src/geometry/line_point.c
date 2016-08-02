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
 *     line_point
 *
 *   PURPOSE
 *     Find a point on a line a given distance from a specific point on the line.
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
 * Given two points p1 and p2.  Find the point p3 on the line from p1 to
 * p2 that is a given distance from p1.  If the distance is negative, p1
 * will lie between p3 and p2.  Return the distance from p1 to p2 or 0 if
 * p1 == p2.
 */
double EG_line_point(Point_d *p1, Point_d *p2, double dist, Point_d *p3)
{
  double len;
  double quot;

  /* find the distance between p1 and p2 */
  len = DIST(p1, p2);

  if (len < MACH_EPS)
    {
      p3->x = 0;
      p3->y = 0;
      return(0);
    }

  quot = dist/len;

  /* the equation of a line from p1 to p2 is p1 + t * (p2 - p1) */
  p3->x = p1->x + quot * (p2->x - p1->x);
  p3->y = p1->y + quot * (p2->y - p1->y);

  return(len);
}

