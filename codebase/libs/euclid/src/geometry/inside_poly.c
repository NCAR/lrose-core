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
 *     inside_poly
 *
 *   PURPOSE
 *     Determine whether a point is inside a polygon.
 *
 *   NOTES
 *     
 *
 *   HISTORY
 *      wiener - May 8, 1995: Created.
 */

#include <euclid/copyright.h>
#include <euclid/geometry.h>

#define S1 0 /* state 1 */
#define S2 1 /* state 2 */

/*
 * Determine whether pt is inside the polygon 'poly'.  Note that if
 * the pt is inside the polygon 'poly', EG_inside_poly will return 1
 * and if the pt is outside 'poly', 0 will be returned. If the point
 * is on the boundary, 0 or 1 may be returned.
 */
int EG_inside_poly(Point_d *pt, Point_d *poly, int n)
{
  int	i, i1;		/* point index; i1 = i-1 mod n */
  double	x;		/* x intersection of e with ray */
  int	crossings = 0;	/* number of edge/ray crossings */
  Point_d q;

  q.x = pt->x;
  q.y = pt->y;

  /* For each edge e=(i-1,i), see if crosses ray. */
  for( i = 0; i < n; i++ )
    {
      i1 = ( i + n - 1 ) % n;

      /* if e straddles the x-axis... */
      if((( poly[i].y > q.y ) && ( poly[i1].y <= q.y)) ||
	 (( poly[i1].y > q.y ) && ( poly[i].y <= q.y ) ) )
	{
	  /* e straddles ray, so compute intersection with ray. */
	  x = poly[i1].x + ((q.y - poly[i1].y) * (poly[i].x - poly[i1].x)) / (poly[i].y - poly[i1].y);

	  /* crosses ray if strictly positive intersection. */
	  if (x > q.x) crossings++;
	}
    }

  /* q inside if an odd number of crossings. */
  if( (crossings % 2) == 1 )
    return 1;
  else
    return 0;
}

/*
 * Determine whether P is inside the polygon 'poly'.  IMPORTANT: This
 * algorithm requires that poly has dimension num_pts + 1 since
 * poly[num_pts] is set to poly[0].
 *
 * This routine is less efficient than the one above.
 */
int EG_inside_poly_deprecated(Point_d *P, Point_d *poly, int num_pts)
{
  int count;
  int i;
  double ret;
  int state;

  count = 0;
  poly[num_pts] = poly[0];

  if (poly[0].y <= P->y)
    state = S1;
  else
    state = S2;

  for (i=1; i<=num_pts; i++)
    {
      switch (state)
	{
	case S1:
	  if (poly[i].y > P->y)
	    {
	      /* check half-plane */
	      state = S2;
	      ret = EG_half_plane(P, &poly[i-1], &poly[i]);
	      if (ret > 0)
		count++;
	      else if (ret == 0)
		return(1);
	    }
	  else
	    {
	      state = S1;
	    }
	  break;

	case S2:
	  if (poly[i].y <= P->y)
	    {
	      /* check half-plane */
	      state = S1;
	      ret = EG_half_plane(P, &poly[i-1], &poly[i]);
	      if (ret > 0)
		count++;
	      else if (ret == 0)
		return(1);
	    }
	  else
	    {
	      state = S2;
	    }
	  break;
	}

    }

  return(count % 2);
}
