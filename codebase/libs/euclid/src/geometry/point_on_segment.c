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
 *     point_on_segment
 *
 *   PURPOSE
 *     Determine whether a point is on a line segment.
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
 * Return 1 if P is on the line segment formed by LP1 and LP2 and 0
 * otherwise.  Note that if P is on the line formed by LP1 and LP2 but
 * not on the line segment, this function returns 0.
 */
int EG_point_on_segment(Point_d *P, Point_d *LP1, Point_d *LP2)
{
  double dx1;
  double dy1;
  double dx2;
  double dy2;

  /* calculate vector components */
  dx1 = P->x - LP1->x;
  dy1 = P->y - LP1->y;

  dx2 = LP2->x - LP1->x;
  dy2 = LP2->y - LP1->y;

  /*
   * calculate cross product to determine whether the point P is even on
   * the line containing LP1 and LP2 (i.e., cross product should be zero
   */
  if (ABS(dx1 * dy2 - dy1 * dx2) < MACH_EPS)
    {
      /* check that the point is on the same side of LP1 as LP2 */
      if (dx1 * dx2 + dy1 * dy2 >= -MACH_EPS)
        {
          if (-MACH_EPS <= dx2 * dx2 + dy2 * dy2 - (dx1 * dx1 + dy1 * dy1))
            return(1);
          else
            return(0);
        }
      else
        return(0);
    }
  else
    return(0);
}

