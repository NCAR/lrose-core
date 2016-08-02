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
 * 	find_extreme_pts
 *
 * PURPOSE
 * 	Find points in a boundary which are extrema relative to a
 * particular vector.  This function assumes that the line corresponding
 * to the input vector does not intersect the boundary.
 *
 * NOTES
 * 	
 *
 * HISTORY
 *     wiener - Mar 1, 1993: Created.
 */
#include <limits.h>
#include <euclid/boundary.h>
#include <euclid/point.h>
#include <euclid/geometry.h>

/*
 * DESCRIPTION:    
 * 	find_extreme_pts - find the extreme points of a boundary
 * corresponding to a given input vector.
 *
 * INPUTS:
 * 	in_bdry - input array of boundary points
 * 	in_bdry_size - dimension of in_bdry
 * 	v0 - begin point of vector
 * 	v1 - end point of vector
 *
 * OUTPUTS:
 * 	max - array index of furthest boundary point 
 * 	min - array index of closest boundary point 
 *
 * RETURNS:
 *       void
 *
 * NOTES:
 * 	Assumes that line corresponding to input vector does not
 * intersect boundary.
 */

#ifndef HUGE_VAL
#define HUGE_VAL 1.0e50
#endif

void EG_find_extreme_pts(Point_d *in_bdry, int in_bdry_size, Point_d *v0, Point_d *v1, int *max, int *min)
{
  double bot_dist = HUGE_VAL;
  int bot_ind = -1;
  double dist;
  int i;
  double top_dist = -1;
  int top_ind = -1;

  /*
   * find the points of in_bdry which are closest and furthest to the
   * line corresponding to v0, v1
   */
  for (i=0; i<in_bdry_size; i++)
    {
      dist = EG_get_perp_dist(&in_bdry[i], v0, v1);
      if (dist > top_dist)
	{
	  top_ind = i;
	  top_dist = dist;
	}
      if (dist < bot_dist)
	{
	  bot_ind = i;
	  bot_dist = dist;
	}
    }

  *max = top_ind;
  *min = bot_ind;
}
