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
 * 	make_star
 *
 * PURPOSE
 * 	Find points of intersection of rays at equal angles theta with
 * a given boundary.
 *
 * NOTES
 *
 *
 * HISTORY
 * 	wiener - Mar 25, 1993: Created.
 */

/* includes */
#include <string.h>
#include <math.h>
#include <euclid/boundary.h>
#include <euclid/point.h>
#include <euclid/geometry.h>

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

/* defines */
#define MIN_DIV 3

/* file scope function declarations */
static int eval_intersect(Point_d *ray, int j, Point_d bdry_pts[], int i, Point_d *ref_pt, Star_point star_pts[]);
static double atan2_adj(double y, double x);

/*
 * DESCRIPTION:    
 * 	make_star takes a boundary consisting of a sequence of line
 * segments perpendicular to the x or y axes, a number of rays, num_rays,
 * and a reference point and then outputs a star-shaped set consisting of
 * the points of intersection of the given boundary with rays emanating
 * from the reference point at equidistant angles.  The initial ray is
 * identical with the positive x axis and sweeps counterclockwise.
 *
 * INPUTS:
 * 	bdry_pts - array of points determining the boundary, 
 * 		   the initial and ending boundary points are identical
 * 	dim_bdry_pts - dimension of array of boundary points
 * 	ray - array of sines and cosines for each ray endpoint
 * 	div - dimension of ray (short for divisions)
 * 	theta - angle used for producing ray
 * 	ref_pt - reference point
 *
 * OUTPUTS:
 * 	star_pts - output array of points
 *
 * RETURNS:
 *     	dimension of star_pts or -1 on failure
 *
 * METHOD:
 * 	Iterate through the boundary segments doing the following:
 *
 * 1.  Utilizing the endpoints of a segment, determine the range of
 * rays that the segment intersects.
 *
 * For each ray found in 1) do
 *
 * a.  Determine the point of intersection of the segment with the ray
 * and then calculate the length of the segment from the point of
 * intersection to ref_pt.  If this length is greater than the current
 * maximal length for the ray, replace the point of intersection stored
 * for the ray with the new point of intersection.
 */
int EG_make_star(Point_d bdry_pts[], int dim_bdry_pts, Point_d *ray, int div, double theta, Point_d *ref_pt, Star_point star_pts[])
{
  double angle0;
  double angle1;
  double bdry_theta0;
  double bdry_theta1;
  double delta;
  double dx;
  double dy;
  int i;
  int j;
  int ray_begin;		/* beginning angle of ray */
  int ray_end;			/* ending angle of ray */

  /*
   * the number of divisions of the circle should be reasonably large
   * otherwise error return
   */
  if (div < MIN_DIV)
    return(-1);

  /*
   * Initialize to 0.  This prepares for spokes of zero length
   */
  memset ((void *) star_pts,
          (int) 0, (size_t) (sizeof(Star_point) * (div + 1)));

  /*
   * for each boundary segment, determine the range of rays it
   * covers
   */
  dy = bdry_pts[0].y - ref_pt->y;
  dx = bdry_pts[0].x - ref_pt->x;
  bdry_theta0 = atan2_adj(dy, dx); /* first angle of bdry segment */

  for (i=1; i<dim_bdry_pts; i++)
    {
      dy = bdry_pts[i].y - ref_pt->y;
      dx = bdry_pts[i].x - ref_pt->x;
      bdry_theta1 = atan2_adj(dy, dx); /* second angle of bdry segment */
      delta = bdry_theta1 - bdry_theta0;
      
      /* orient the angle of the segment */
      if (delta > 0)
	{
	  angle0 = bdry_theta0;
	  angle1 = bdry_theta1;
	}
      else
	{
	  angle0 = bdry_theta1;
	  angle1 = bdry_theta0;
	  delta = -delta;
	}
	
      if (delta < M_PI)
	{
	  ray_begin = ceil(angle0/theta);
	  ray_end = floor(angle1/theta);
	}
      else
	{
	  ray_begin = ceil(angle1/theta);
	  ray_end = floor(angle0/theta);
	}	      

      /*
       * For each ray the segment covers, determine whether the
       * point of intersection of the segment with the ray is
       * further from ref_pt than the current intersection point.
       */
      if (ray_begin <= ray_end)
	{
	  for (j=ray_begin; j<=ray_end; j++)
	    {
	      eval_intersect(ray, j, bdry_pts, i, ref_pt, star_pts);
	    }
	}
      else
	{
	  /*
	   * rule out the cases where the boundary segment actually lies
	   * in between two rays (delta < theta and ray_end < ray_begin)
	   */
	  if (delta >= theta)
	    {
	      for (j=ray_begin; j<div; j++)
		{
		  eval_intersect(ray, j, bdry_pts, i, ref_pt, star_pts);
		}

	      for (j=0; j<=ray_end; j++)
		{
		  eval_intersect(ray, j, bdry_pts, i, ref_pt, star_pts);
		}
	    }
	}

      bdry_theta0 = bdry_theta1;
    }

  /* set the last star point to the initial star point */
  star_pts[div] = star_pts[0];

  return(div);
} /* make_star */

/*
 * DESCRIPTION:    
 * 	make_star_TN is similar to make_star, except that the star
 *      starts at True North and moves clockwise, as opposed to the
 *      usual Cartesian axes which start at the X axis and move
 *      counter-clockwise
 */

int EG_make_star_TN(Point_d bdry_pts[], int dim_bdry_pts, Point_d *ray, int div, double theta, Point_d *ref_pt, Star_point star_pts[])
{
  double angle0;
  double angle1;
  double bdry_theta0;
  double bdry_theta1;
  double delta;
  double dx;
  double dy;
  int i;
  int j;
  int ray_begin;		/* beginning angle of ray */
  int ray_end;			/* ending angle of ray */

  /*
   * the number of divisions of the circle should be reasonably large
   * otherwise error return
   */
  if (div < MIN_DIV)
    return(-1);

  /*
   * Initialize to 0.  This prepares for spokes of zero length
   */
  
  memset ((void *)  star_pts,
          (int) 0, (size_t) (sizeof(Star_point) * (div + 1)));


  /*
   * for each boundary segment, determine the range of rays it
   * covers
   */

  dy = bdry_pts[0].y - ref_pt->y;
  dx = bdry_pts[0].x - ref_pt->x;
  bdry_theta0 = atan2_adj(dx, dy); /* first angle of bdry segment */

  for (i=1; i<dim_bdry_pts; i++)
    {
      dy = bdry_pts[i].y - ref_pt->y;
      dx = bdry_pts[i].x - ref_pt->x;
      bdry_theta1 = atan2_adj(dx, dy); /* second angle of bdry segment */
      delta = bdry_theta1 - bdry_theta0;
      
      /* orient the angle of the segment */
      if (delta > 0)
	{
	  angle0 = bdry_theta0;
	  angle1 = bdry_theta1;
	}
      else
	{
	  angle0 = bdry_theta1;
	  angle1 = bdry_theta0;
	  delta = -delta;
	}
	
      if (delta < M_PI)
	{
	  ray_begin = ceil(angle0/theta);
	  ray_end = floor(angle1/theta);
	}
      else
	{
	  ray_begin = ceil(angle1/theta);
	  ray_end = floor(angle0/theta);
	}	      

      /*
       * For each ray the segment covers, determine whether the
       * point of intersection of the segment with the ray is
       * further from ref_pt than the current intersection point.
       */
      if (ray_begin <= ray_end)
	{
	  for (j=ray_begin; j<=ray_end; j++)
	    {
	      eval_intersect(ray, j, bdry_pts, i, ref_pt, star_pts);
	    }
	}
      else
	{
	  /*
	   * rule out the cases where the boundary segment actually lies
	   * in between two rays (delta < theta and ray_end < ray_begin)
	   */
	  if (delta >= theta)
	    {
	      for (j=ray_begin; j<div; j++)
		{
		  eval_intersect(ray, j, bdry_pts, i, ref_pt, star_pts);
		}

	      for (j=0; j<=ray_end; j++)
		{
		  eval_intersect(ray, j, bdry_pts, i, ref_pt, star_pts);
		}
	    }
	}

      bdry_theta0 = bdry_theta1;
    }

  /* set the last star point to the initial star point */
  star_pts[div] = star_pts[0];

  return(div);
} /* make_star */

/*
 * DESCRIPTION:    
 * 	Find the intersection of the segment ref_pt, ray[j] with the
 * segment formed by bdry_pts[i-1], bdry_pts[i].  If the length of the
 * segment from ref_pt to this intersection is greater than the length
 * currently stored in ray[j], reset the length stored in star_pts[j] as
 * well as the index stored in star_pts[j].
 *
 * INPUTS:
 * 	ray - array of endpoints.  The point ray[i] corresponds to the ray
 * formed by ref_pt, ray[i].
 * 	j - current ray index
 * 	bdry_pts - array of boundary points
 * 	i - current bdry_pts index
 * 	ref_pt - reference point
 *
 * OUTPUTS:
 * 	star_pts - the values star_pts[j].length and star_pts[j].index
 * may be adjusted
 *
 * RETURNS:
 *     	0 on success, -1 on failure
 */

static int eval_intersect(Point_d *ray, int j, Point_d bdry_pts[], int i, Point_d *ref_pt, Star_point star_pts[])
{
  Point_d intersect;
  double length;
  int ret;
  double t;

  /* find the intersection of ray[j] with bdry_pts[i] */
  ret = EG_line_intersect(ref_pt, &ray[j], &bdry_pts[i-1], &bdry_pts[i], &intersect, &t);
  
  if (ret == 0 || t < -0.000001 || t > 1.000001) {

    /*
     * no intersection
     */

    return(0);

  }

  if (ret != 1) {

    /*
     * failure - make intersection center of line
     */

    intersect.x = (bdry_pts[i-1].x + bdry_pts[i].x) / 2.0;
    intersect.y = (bdry_pts[i-1].y + bdry_pts[i].y) / 2.0;
    
  }

  length = EG_hypot(intersect.x - ref_pt->x, intersect.y - ref_pt->y);
    
  if (star_pts[j].r < length)
    {
      star_pts[j].r = length;
      star_pts[j].index = i;
      star_pts[j].pt = intersect;
    }
  
  return(1);

} /* eval_intersect */


/* adjust the atan2 range to 0 .. 2PI */

static double atan2_adj(double y, double x)
{

  double result;

  if (x == 0.0 && y == 0.0)
    result = 0.0;
  else
    result = atan2(y, x);

  if (result < 0)
    result += 2 * M_PI;
  return(result);

} /* atan2_adj */

