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
/* polygon_centroid.c - find the centroid of an arbitrary polygon */

/* includes */
#include <euclid/geometry.h>

/*
 * Given a polygon, calculate its centroid, which is just the simple
 * average of the X coordinates of the vertices and the simple average
 * of the Y coordinates of the vertices.
 */
void
EG_polygon_centroid_i(const Point_i *points, const int n,
		      Point_d *centroid)
{
  double x_sum = 0.0;
  double y_sum = 0.0;
  
  int i;
  
  if (n <= 0)
  {
    centroid->x = 0.0;
    centroid->y = 0.0;
    
    return;
  }
  
  for (i = 0; i < n; ++i)
  {
    x_sum += (double)points[i].x;
    y_sum += (double)points[i].y;
  }
  
  centroid->x = x_sum / (double)n;
  centroid->y = y_sum / (double)n;
}


void
EG_polygon_centroid_d(const Point_d *points, const int n,
		      Point_d *centroid)
{
  double x_sum = 0.0;
  double y_sum = 0.0;
  
  int i;
  
  if (n <= 0)
  {
    centroid->x = 0.0;
    centroid->y = 0.0;
    
    return;
  }
  
  for (i = 0; i < n; ++i)
  {
    x_sum += points[i].x;
    y_sum += points[i].y;
  }
  
  centroid->x = x_sum / (double)n;
  centroid->y = y_sum / (double)n;
}



