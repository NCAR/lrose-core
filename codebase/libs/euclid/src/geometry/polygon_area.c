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
/* polygon_area.c - find the areas of arbitrary polygons */

/* includes */
#include <euclid/geometry.h>

/*
 * Given a polygon, determine its area.  The first and last points are
 * assumed to be identical.
 *
 * The standard formula for the area of a polygon is
 *
 * sum i = 0, n-1
 *
 * ((x[i] * y[(i+1)%n]) - (y[i] * x[(i+1)%n])) * 0.5
 */
double
EG_polygon_area_i(Point_i *points, int n)
{
  int i;
  double sum;

  sum = 0;
  for (i=0; i<n-1; i++)
    sum += points[i].x * points[i+1].y - points[i+1].x * points[i].y;
  return(0.5 * ABS(sum));
}


double
EG_polygon_area_d(Point_d *points, int n)
{
  int i;
  double sum;

  sum = 0;
  for (i=0; i<n-1; i++)
    {
      sum += points[i].x * points[i+1].y - points[i+1].x * points[i].y;
    }
  return(0.5 * ABS(sum));
}



