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
 *     half_plane
 *
 *   PURPOSE
 *     Assign a value to a point with regard to a given half-plane.  This
 * value will change in sign on opposing sides of the half-plane and will
 * be 0 if the point is on the line determining the half-plane.
 *
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
 * Given a line and a point P, return a value signifying the half plane P
 * is in.  More specifically, form the equation for the line through LP1
 * and LP2 by
 *
 * ((x,y) - LP1) dot N = 0
 *
 * making sure that the coefficient for x is greater than zero and if 0,
 * the coefficient for y should be greater than 0.
 *
 * Return the value when P is substituted for (x,y).
 *
 * Informational Note: Let (dx, dy) = ((x,y) - LP1).  Then the dot
 * product of the normal to the vector from LP1 to LP2 with (dx, dy) is
 * equal to the cross product of the vector from LP1 to LP2 with (dx,
 * dy).  Note that the normal to the vector from LP1 to LP2 can point in
 * two possible directions.  In order for the above equality to hold, the
 * normal to the vector from LP1 to LP2 should be chosen such that the
 * cross product of the vector from LP1 to LP2 with the normal is
 * positive.
 *
 * NOTE:  This function returns 0 if LP1 == LP2!
 */

/*
 * Given a line and a point P, return a value signifying the half plane P
 * is in.  More specifically, form the equation for the line through LP1
 * and LP2 by
 *
 * ((x,y) - LP1) dot N = 0
 *
 * making sure that the coefficient for x is greater than zero and if 0,
 * the coefficient for y should be greater than 0.
 *
 * Return the value when P is substituted for (x,y).
 * NOTE:  This function returns 0 if LP1 == LP2!
 */
double EG_half_plane(Point_d *P, Point_d *LP1, Point_d *LP2)
{
  double dx;
  double dy;
  double nx;
  double ny;
    
  /* calculate vector components */
  dx = P->x - LP1->x;
  dy = P->y - LP1->y;

  nx = LP2->x - LP1->x;
  ny = LP2->y - LP1->y;

  /* ny is the first component of the normal and -nx is the second */
  if (ny > 0)
    {
      return(ny * dx - nx * dy);
    }
  else if (ny == 0)
    {
      /* if the line is horizontal, the return value is indeterminate */
      return(fabs(nx) * dy);
    }
  else
    {
      return(-ny * dx + nx * dy);
    }
}



