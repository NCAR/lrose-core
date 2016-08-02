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
 * 	adj_star
 *
 * PURPOSE
 * 	Adjust spokes of zero length to length eps to avoid degenerate
 * star shaped polygons
 *
 *
 * NOTES
 * 	
 *
 * HISTORY
 *     wiener - May 13, 1992: Created.
 */

#include <euclid/boundary.h>
#include <euclid/point.h>

int EG_adj_star(Star_point *star_pts, int dim_star, Point_d *ref_pt, Point_d *ray, double eps)
{
  int ct;
  int i;

  ct = 0;
  for (i=0; i<dim_star; i++)
    {
      if (star_pts[i].r == 0)
	{
	  star_pts[i].pt.x = ref_pt->x + eps * ray[i].x;
	  star_pts[i].pt.y = ref_pt->y + eps * ray[i].y;
	  ct++;
	}
    }

  return(ct);
} /* adj_star */


