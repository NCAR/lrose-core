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
 *     segment_intersect
 *
 *   PURPOSE
 *     Determine whether two line segments intersect.
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
 * Determine whether the line segments LP1, LP2 and LP3, LP4 intersect.
 * If they intersect in one point return 1.  If they overlap return 2.
 * If one of the segments is not a line but a point, return -1.  Otherwise
 * return 0.
 */
int EG_segment_intersect(Point_d *LP1, Point_d *LP2, Point_d *LP3, Point_d *LP4, Point_d *intersect, double *pos)
{
  double dot1;
  double dot2;
  double dot3;
  double dx1;
  double dy1;
  double dx2;
  double dy2;
  double dx3;
  double dy3;
  int ret;

  *pos = 0;
  
  ret = EG_line_intersect(LP1, LP2, LP3, LP4, intersect, pos);
/*  printf("intersection is %f, %f\n", intersect->x, intersect->y); */
  if (ret == 1)
    {
      /* check whether intersection point is on both line segments */
      if (EG_point_on_segment(intersect, LP1, LP2) == 1 && EG_point_on_segment(intersect, LP3, LP4) == 1)
        return(1);
      else
        return(0);
    }
  else if (ret == 2)
    {
      /* The line segments overlap.  Calculate vector components. */
      dx1 = LP2->x - LP1->x;
      dy1 = LP2->y - LP1->y;
      dx2 = LP3->x - LP1->x;
      dy2 = LP3->y - LP1->y;
      dx3 = LP4->x - LP1->x;
      dy3 = LP4->y - LP1->y;

      dot1 = dx1 * dx2 + dy1 * dy2;
      dot2 = dx1 * dx3 + dy1 * dy3;
        
      if (dot1 < 0)
        {
          if (dot2 < 0)
            return(0);                /* LP3 and LP4 are not on the same side of LP1 as LP2  */
          else
            return(2);                /* the segment LP3, LP4 contains LP1 */
        }
      else                        /* dot1 >= 0 */
        {
          if (dot2 < 0)
            return(2);
          else                        /* dot2 >= 0 */
            {
              dot3 = dx1 * dx1 + dy1 * dy1;
              if (dot1 <= dot3 || dot2 <= dot3)
                return(2);
              else
                return(0);
            }
        }
    }
  else if (ret == -1)
    return(-1);
  else
    return(0);
}

