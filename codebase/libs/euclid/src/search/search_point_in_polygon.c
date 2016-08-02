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
 * point_in_polygon algorithm
 *
 * Mike Dixon, Sept 1994
 */

#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <euclid/search.h>

#define LARGE_INT 1000000
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define PI 3.14159265358979323846

/*-------------------
 * pt_left_of_line()
 *
 * pt is test point
 * p1 is start of line
 * p2 is end of line
 * p1.y < p2.y
 *
 * Returns 1 if pt is to left of line
 *        -1 if pt is to right of line
 *        0 if pt is on line
 */

static int pt_left_of_line(Point_d pt,
			   Point_d p1,
			   Point_d p2)

{

  double intercept, slope;
  double f;

  /*
   * check for vertical line
   */
  
  if (p1.x == p2.x) {

    if (pt.x > p1.x) {
      return (1);
    } else if (pt.x < p1.x) {
      return (-1);
    } else {
      return (0);
    }

  } /* if (p1.x == p2.x) */

  slope = (p2.y - p1.y) / (p2.x - p1.x);
  intercept = p1.y - slope * p1.x;
  f = pt.x + intercept / slope - pt.y / slope;

  if (f > 0.0) {
    return (1);
  } else if (f < 0.0) {
    return (-1);
  } else {
    return (0);
  }

}

/*---------------
 * pt_is_vertex()
 *
 * Returns 1 if point is one of the vertices of the polygon,
 * 0 otherwise.
 */

static int pt_is_vertex(Point_d pt, Point_d *vertex, int nvertices)

{

  int i;

  for (i = 0; i < nvertices; i++) {
    if (!memcmp((void *) &pt,
		(void *) vertex,
		sizeof(Point_d))) {
      return (1);
    }
    vertex++;
  } /* i */

  return (0);

}

/*-------------------
 * pt_inside_limits()
 *
 * Returns 1 if point is inside rectangle which encloses the polygon,
 * 0 otherwise
 */

static int pt_outside_bounding_rect(Point_d pt,
				    Point_d *vertex,
				    int nvertices)

{

  double min_x = 1.0e100;
  double min_y = 1.0e100;
  double max_x = -1.0e100;
  double max_y = -1.0e100;
  int i;

  for (i = 0; i < nvertices; i++) {
    min_y = MIN(min_y, vertex->y);
    max_y = MAX(max_y, vertex->y);
    min_x = MIN(min_x, vertex->x);
    max_x = MAX(max_x, vertex->x);
    vertex++;
  } /* i */

  if (pt.x >= min_x && pt.x <= max_x &&
      pt.y >= min_y && pt.y <= max_y) {
    return (0);
  } else {
    return (1);
  }

}

/*----------------
 * pt_in_polygon()
 *
 * Returns 1 if pt is inside polygon, 0 otherwise.
 * A pt lying on the polygon boundary is included in the polygon.
 */

int EGS_point_in_polygon(Point_d pt, Point_d *vertex, int nvertices)

{

  int i;
  int nleft;
  int retval;
  Point_d p1, p2, tmp_pt;

  /*
   * if point lies outside bounding rectangle of polygon,
   * return false
   */

  if (pt_outside_bounding_rect(pt, vertex, nvertices)) {
    return (0);
  }

  /*
   * if pt is a vertex, return true
   */

  if (pt_is_vertex(pt, vertex, nvertices)) {
    return (1);
  }

  /*
   * enter loop to determine how many segments of the polygon
   * are to the left of the point
   */

  nleft = 0;

  for (i = 0; i < nvertices; i++) {

    /*
     * set points of segment from vertex array
     */
    
    p1 = vertex[i];
    if (i < nvertices - 1) {
      p2 = vertex[i+1];
    } else {
      p2 = vertex[0];
    }

    /*
     * make sure p1 is below p2
     */
    
    if (p1.y > p2.y) {
      tmp_pt = p1;
      p1 = p2;
      p2 = tmp_pt;
    }
    
    /*
     * check if this segment crosses the level of the pt
     */
    
    if (p1.y < pt.y && p2.y >= pt.y) {
      
      if (p1.y == p2.y) {

	/*
	 * if line is horizontal, check is pt is on line
	 */
	
	if ((pt.x >= p1.x && pt.x <= p2.x) ||
	    (pt.x >= p2.x && pt.x <= p1.x)) {
	  
	  return (1);
	  
	}
	
      } else {
	
	/*
	 * increment counter if pt is to the left
	 */
	
	retval = pt_left_of_line(pt, p1, p2);
	
	if (retval == 0) {
	  
	  /*
	   * point on line, therefore include
	   */
	  
	  return (1);
	  
	}
	
	if (retval == 1) {
	  nleft++;
	}
	
      } /* if (p1.y == p2.y) */
      
    } /* if (p1.y < pt.y &&	p2.y >= pt.y) */

  } /* i */

  /*
   * if nleft is odd, pt is inside
   */

  return (nleft % 2);

}


/*
 *
 * This routine tests whether or not the point
 * ( x_test, y_test ) is inside the region determined
 * by the points ( x[i], y[i] ),  0 <= i < n.
 *
 * If the test point is not inside the region, a zero value is 
 * returned, otherwise the value tells how many times the region
 * wraps around the test point, and the sign tells the direction
 * in which the winding occurs: + CCW, - CW.
 *
 * This routine does not test to see if the point is exactly
 * on the boundary of the region.
 *
 * Author: Randy Bullock
 *
 * This routine is simpler and more elegant than the
 * one above. It should yield the same answer. It is
 * somewhat slower.
 */

int EGS_point_in_polygon2(Point_d pt, Point_d *vertex, int nvertices)

{

  int j, k;
  double angle, angle0, a, b, c, d, e, f;
  double n_rot;

  a = vertex[0].x - pt.x;
  b = vertex[0].y - pt.y;

  if (b == 0.0 && a == 0.0) {
    angle = angle0 = 0.0;
  } else {
    angle = angle0 = atan2(b, a) / PI;
  }

  for (j=0; j<nvertices; ++j)  {
    
    k = (j + 1)%nvertices;
    
    c = vertex[k].x - pt.x;
    d = vertex[k].y - pt.y;
    e = a*d - b*c;
    f = a*c + b*d;

    if (e != 0.0 || f != 0.0) {
      angle += atan2(e, f) / PI;
    }
    
    a = c;
    b = d;
    
  }


  n_rot = (angle - angle0)/2;

  if (n_rot > 0.0) {
    return ((int) floor(n_rot + 0.5));
  } else {
    return ((int) ceil(n_rot - 0.5));
  }

}






