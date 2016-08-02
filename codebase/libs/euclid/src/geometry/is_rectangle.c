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
 *     is_rectangle, is_parallelogram
 *
 *   PURPOSE
 *     Check whether an array of points is a rectangle or parallelogram
 *
 *   NOTES
 *     If all four points are identical, the array of points is not
 *     considered to be either.
 *
 *   HISTORY
 *      wiener - May 8, 1995: Created.
 */

#include <euclid/copyright.h>
#include <euclid/geometry.h>

/*
 * Check whether array is rectangle - assume points are arranged
 * sequentially around boundary
 */
int EG_is_rectangle(Point_d *array)
{
  int i;
  Point_d delta[4];

  
  /* check distances */
  for (i=0; i<3; i++)
    {
      delta[i].x = array[i+1].x - array[i].x;
      delta[i].y = array[i+1].y - array[i].y;
    }

  delta[3].x = array[0].x - array[3].x;
  delta[3].y = array[0].y - array[3].y;
  
  if (ABS(DOT(&delta[0], &delta[3])) > MACH_EPS)
    return(0);

  if (ABS(DOT(&delta[1], &delta[2])) > MACH_EPS)
    return(0);

  if (NORM_SQ(&delta[0]) < MACH_EPS)
    return(0);

  return(1);
}

/*
 * Check whether array is parallelogram - assume points are arranged
 * sequentially around boundary
 */
int EG_is_parallelogram(Point_d *array)
{
  int i;
  double dist[4];
  
  for (i=0; i<3; i++)
    dist[i] = DIST_SQ(&array[i+1], &array[i]);

  dist[3] = DIST_SQ(&array[0], &array[3]);

  if (ABS(dist[0] - dist[2]) > MACH_EPS)
    return(0);

  if (ABS(dist[1] - dist[3]) > MACH_EPS)
    return(0);

  if (dist[0] < MACH_EPS)
    return(0);

  return(1);
}
