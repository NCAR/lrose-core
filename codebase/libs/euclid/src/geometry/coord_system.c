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
 *     coord_system
 *
 *   PURPOSE
 *     Create unit vector cartesian coordinate system from two points
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
 * Create coordinate system unit vectors given two points.  Origin is
 * assumed to be p1.  The vector u1 will have the same direction as the
 * vector from p1 to p2.  The unit vector u2 will be perpendicular to u2.
 * Returns the length of the vector from p1 to p2.
 */
double EG_coord_system(Point_d *p1, Point_d *p2, Point_d *u1, Point_d *u2)
{
  double ret;
  Point_d vect;
  
  EG_vect_sub(p2, p1, &vect);
  ret = EG_unit_vector(&vect, u1);
  u2->x = -u1->y;
  u2->y = u1->x;
  return(ret);
}

