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
#ifndef EUCLID_GEOMETRY_H
#define EUCLID_GEOMETRY_H

#ifdef __cplusplus
 extern "C" {
#endif

/*
 * NAME
 *	geometry
 *
 * PURPOSE
 *	functions in the geometry package
 *
 * NOTES
 *	
 *
 * HISTORY
 *     wiener - Mar 27, 1995: Created.
 */

#include <math.h>
#include <euclid/point.h>

typedef struct {
  Point_d start;
  Point_d end;
} EG_poly_side_t;

typedef struct {
  EG_poly_side_t side;
  double x;
} EG_crossing_t;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define MACH_EPS 0.000000001

#define HYPOT(x, y) (sqrt((x) * (x) + (y) * (y)))
#define ABS(x) ((x) > 0 ? (x) : (-(x)))
#define CEILING(x) ((x) - (int)(x) > 0 ? (int)((x) + 1.0) : ((x) - (int)(x) < 0 ? (int)((x) - 1.0) : 0))
#define DOT(p, q) ((p)->x * (q)->x + (p)->y * (q)->y)
#define DIST(p,q) HYPOT((p)->x - (q)->x, (p)->y - (q)->y)
#define DIST_SQ(p,q) (((p)->x - (q)->x) * ((p)->x - (q)->x) + ((p)->y - (q)->y) * ((p)->y - (q)->y))
#define FLOOR(x) ((int)(x))
#define NORM(p) HYPOT((p)->x, (p)->y)
#define NORM1(p, q) (ABS((p)->x - (q)->x) + ABS((p)->y - (q)->y))
#define NORM_SQ(p) ((p)->x * (p)->x + (p)->y * (p)->y)
#define ROUND(x) ((x) > 0 ? (int)((x) + 0.5) : (int)((x) - 0.5))
#define SGN(x) ((x) > 0 ? 1 : ((x) < 0 ? (-1) : 0))		    
#define CROSS(p, q) ((p)->x * (q)->y - (p)->y * (q)->x)

/*
 * Computes twice the area of the triangle p, q, r.  Returns positive
 * value if triangle is oriented counter clockwise and negative value if
 * oriented clockwise.  
 */
#define TRI_AREA2(p, q, r) (((q)->x - (p)->x) * ((r)->y - (p)->y) \
			    - ((q)->y - (p)->y) * ((r)->x - (p)->x)) 

/*
 * return vals from polygon intersection routines
 */
   
#define POLY_NO_INTERSECTION 0 
#define POLY1_IN_POLY2  1 /* poly1 is inside poly2 */
#define POLY2_IN_POLY1  2 /* poly1 is inside poly2 */
#define POLY_INTERSECTION 3 /* poly1 and poly2 intersect in a polygon */

/* functions to be used externally */

extern void EG_vect_add(Point_d *u, Point_d *v, Point_d *w);
extern void EG_vect_sub(Point_d *u, Point_d *v, Point_d *w);

/*
 * Create a rectangle of total width 2*half_width where p1 and p2 are
 * midpoints on the top and bottom.  Return the height of the box.
 */

extern double EG_create_box(Point_d *p1, Point_d *p2, double width,
			    Point_d *box);

/*
 * Create equidistant points to or from pt in the direction vect.  If 
 * from != 0 the points are created away from pt.  If from == 0, the points are
 * created going toward pt.  The number of points created is determined
 * by num_pts and the points are stored in array.
 */
extern void EG_equidistant_pts(Point_d *pt, Point_d *vect, int from,
			       int num_pts, Point_d *array);

/*
 * Name:    
 *    convex_intersect
 *
 * Purpose:
 *    Find the convex polygon representing the intersection of two convex
 * polygons.  NOTE: We assume that each given polygon does not have two
 * collinear edges.
 *
 * In:
 *    poly1 - array of points in first polygon sorted in COUNTERCLOCKWISE!! order
 *    size1 - size of poly1
 *    poly2 - array of points in second polygon sorted in COUNTERCLOCKWISE!! order
 *    size2 - size of poly2
 *
 * Out:
 *    int_pt - array of points of intersection of polygon edges
 *    int_size - size of int_pt
 *    out_poly - array of points represent polygon of intersection
 *    (The arrayout_poly contains int_pt plus potentially additional vertices from
 *    poly1 and poly2 2.  It should have dimension at least size1+size2.)
 *    out_size - size of out_poly
 *
 * Returns:
 * -1 - error occured
 *  0 - no intersection 
 *  1 - poly1 is inside poly2
 *  2 - poly2 is inside poly1
 *  3 - there is at least one point of intersection of poly1 and poly2
 *
 * Notes:
 *
 *      1.  Iterate through the edges in poly1
 *      2.  Check for points of intersection from edges in poly2
 *      3.  Record points of intersection in order of occurence
 *
 *  See Computational Geometry by Shamos and Preparata for additional information.
 */

#ifdef NOTYET
extern int EG_convex_intersect(Point_d *poly1, int poly1_size,
			       Point_d *poly2, int poly2_size,
			       Point_d *out_poly, int *out_size);
#else
extern int EG_convex_intersect(Point_d *poly1, int poly1_size,
			       Point_d *poly2, int poly2_size,
			       Point_d *out_poly, int *out_size,
			       Point_d *int_pt, int *int_size);
#endif

extern void EG_create_hash_marks(Point_d *array, int num_pts, double scale,
				 Point_d *segments);

extern double EG_create_runway_pts(Point_d *p1, Point_d *p2,
				   double scale, int num_ext, Point_d *array);

/*
 * Create a trapezoid where p1 and p2 are midpoints on two opposing
 * parallel sides (top and bottom) and half_width1 corresponds to half of
 * the width at p1 and half_width2 corresponds to half of the width at
 * p2.  Return the height of the box.
 */

extern double EG_create_trapezoid(Point_d *p1, Point_d *p2,
				  double half_width1, double half_width2,
				  Point_d *box);

extern int EG_polygon_crossing_compare(const void *v1, const void *v2);

/**************************************************************************
 * EG_fill_polygon()
 *
 * For a given polygon, fills in the points on a grid by adding a given
 * value to each point in the grid which is inside the polygon.
 *
 * The polygon must close - i.e. the last point duplicates the first.
 *
 * This is an approximate routine. As a preprocessing step, the
 * points in the polygon are checked to see if they lie exaclty on
 * the grid lines in y. If they do, a small number is added to them
 * to move them off the line.
 *
 * Returns number of points filled
 */

extern long EG_fill_polygon(Point_d *vertices, int nvertices,
			    long nx, long ny,
			    double minx, double miny,
			    double dx, double dy,
			    unsigned char *grid_array,
			    int add_val);
     
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

extern double EG_half_plane(Point_d *P, Point_d *LP1, Point_d *LP2);

/*
 * Determine whether P is inside the polygon 'poly'.  IMPORTANT: This
 * algorithm requires that poly has dimension num_pts + 1 since
 * poly[num_pts] is set to poly[0].
 */

extern int EG_inside_poly(Point_d *P, Point_d *poly, int num_pts);

/*
 * Check whether array is rectangle - assume points are arranged
 * sequentially around boundary
 */

extern int EG_is_rectangle(Point_d *array);

/**************************************************************************
 * EG_line_through_ellipse.c
 *
 * Finds the two intersection points between a line and a non-rotated
 * ellipse centered at (0,0).
 *
 * If line intersects ellipse, returns TRUE, and places the coords
 * of the intersection points as (xx1, yy1) and (xx2, yy2), xx1 <= xx2;
 *
 * If no intersection, returns FALSE.
 *
 * The points are found by the solution of a quadratic. If the quadratic
 * does not have real roots, there is no intersection.
 */

extern int EG_line_through_ellipse(double major_radius,
				   double minor_radius,
				   double slope,
				   double intercept,
				   double *xx1,
				   double *yy1,
				   double *xx2,
				   double *yy2);

/*
 * Given two points p1 and p2.  Find the point p3 on the line from p1 to
 * p2 that is a given distance from p1.  If the distance is negative, p1
 * will lie between p3 and p2.  Return the distance from p1 to p2 or 0 if
 * p1 == p2.
 */

extern double EG_line_point(Point_d *p1, Point_d *p2, double dist,
			    Point_d *p3);

/*
 * Given the four points a, b, c, d, determine the intersection of the
 * lines determined by ab with cd.  If one point intersection exists,
 * return 1.  If the lines are collinear return 2.  If one of the input
 * segments is not a line but a point, return -1.  Otherwise return 0.
 * The point of intersection is returned in e.
 *
 * Since the point of intersection e is on the line determined by c,d we
 * have e = c + t * (d - c).  Since e is also on the line determined by
 * a,b we have perp(ab) dot (e - a) = 0 or
 * perp(ab) dot e = perp(ab) dot a.  This implies
 *
 * perp(ab) dot (c + t * (d - c)) = perp(ab) dot a
 *
 * or
 *
 * t = (perp(ab) dot (a - c)/(perp(ab) dot (d - c))
 *
 * Adapted from Graphics Gems.
 */

extern int EG_line_intersect(Point_d *a, Point_d *b, Point_d *c,
			     Point_d *d, Point_d *e, double *t);

/*
 * Given a line, a point and an angle find the intersection of the ray with the line. Return values follow the paradigm of EG_line_intersect().
 */
extern int EG_ray_line_intersect
(
 Point_d *a,			/* first point on line */
 Point_d *b,			/* second point on line */
 Point_d *c,			/* point */
 double angle,			/* mathematical angle in radians (use M_PI/2.0 - true_north_angle if angle is being measured from true north) */
 Point_d *e,			/* intersection point */
 double *t			/* e = a + t * (b - a) */
 );

/*
 * Assume that u, v are vectors defining a coordinate system and that w
 * is a vector having coordinates with respect to u and v.  Specifically,
 *
 * u = (u.x, u.y) with respect to some standard basis
 * v = (v.x, v.y) with respect to some standard basis
 * w = (w.x, w.y) with respect to u, v
 *
 * Express w with regard to the standard basis.  I.e., find z = w.x (u) +
 * w.y (v).  Note that this is simply matrix multiplication:
 *
 * Matrix  Vector
 *
 * z   = (u v)   w
 *
 * z   = u   * w.x + v   * w.y
 *
 * z.x = u.x * w.x + v.x * w.y
 * z.y = u.y * w.x + v.y * w.y 
 */

extern void EG_linear_comb(Point_d *u, Point_d *v, Point_d *w, Point_d *z);

/*
 * perp_projection - Find the projection of a point to a line
 *
 * IN:  points c, d, e
 *
 * Here c, d determine a line segment and e is the point to be projected to
 * that line.
 *
 * OUT: the projection point f
 *
 * RETURNS: The distance from e to the line through c and d.
 *
 * If c == d then the function returns -1 and f is
 * set equal to c.
 *
 * If e is on the line from c through d, then f will be
 * set to e and the function will return 0.
 *
 * If e is not on the line from c through d, the function will return 1
 * if the perpendicular projection point is on the line segment cd
 * otherwise 2.  The point f will be set equal to the projected point.
 */

extern double EG_perp_projection(Point_d *c, Point_d *d,
				 Point_d *e, Point_d *f);

/*
 * Return 1 if P is on the line segment formed by LP1 and LP2 and 0
 * otherwise.  Note that if P is on the line formed by LP1 and LP2 but
 * not on the line segment, this function returns 0.
 */

extern int EG_point_on_segment(Point_d *P, Point_d *LP1, Point_d *LP2);

/**************************************************************************
 * EG_point_in_ellipse.c
 *
 * Checks whether a given point lies inside a (rotated) ellipse. The search
 * radius is multiplied by a factor to allow the search to be extended
 * beyond the ellipse or restricted to a portion of the ellipse.
 *
 * If within, returns TRUE. Else, returns FALSE.
 */

extern int EG_point_in_ellipse(double ellipse_x, double ellipse_y,
			       double major_radius, double minor_radius,
			       double axis_rotation,
			       double search_x, double search_y,
			       double search_radius_ratio);

/*----------------
 * EG_pt_in_polygon()
 *
 * Returns 1 if pt is inside polygon, 0 otherwise.
 * A pt lying on the polygon boundary is included in the polygon.
 */

extern int EG_point_in_polygon(Point_d pt, Point_d *vertex, int nvertices);

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

extern int EG_point_in_polygon2(Point_d pt, Point_d *vertex, int nvertices);
     
/*
 * Given a polygon, determine its area.  The first and last points are
 * assumed to be identical.
 *
 * The standard formula for the area of a polygon is
 *
 * sum i = 0, n-1
 *
 * ((x[i] * y[(i+1)%n]) - (y[i] * x[(i+1)%n])) * 0.5
 */

extern double EG_polygon_area_d(Point_d *points, int n);
extern double EG_polygon_area_i(Point_i *points, int n);

/*
 * Given a polygon, calculate its centroid, which is just the simple
 * average of the X coordinates of the vertices and the simple average
 * of the Y coordinates of the vertices.
 */

extern void EG_polygon_centroid_i(const Point_i *points, const int n,
				  Point_d *centroid);
extern void EG_polygon_centroid_d(const Point_d *points, const int n,
				  Point_d *centroid);

/*
 * Determine whether the line segments LP1, LP2 and LP3, LP4 intersect.
 * If they intersect in one point return 1.  If they overlap return 2.
 * If one of the segments is not a line but a point, return -1.  Otherwise
 * return 0.
 */

extern int EG_segment_intersect(Point_d *LP1, Point_d *LP2,
				Point_d *LP3, Point_d *LP4,
				Point_d *intersect, double *pos);

/*
 * unit_vector
 *
 * unit_vector - Convert vector to unit vector
 *
 * IN:  vector c
 * OUT: vector d
 * RETURNS: norm of vector or 0 if vector is 0
 */

extern double EG_unit_vector(Point_d *c, Point_d *d);

/*
 * unit_vector_perp                        
 *
 * unit_vector_perp - Convert vector to perpendicular unit vector 
 *
 * IN:  vector c
 * OUT: point d
 * RETURNS: norm of vector or 0 if vector is 0
 */

extern double EG_unit_vector_perp(Point_d *c, Point_d *d);

extern double EG_hypot(double dx, double dy);

/*********************************************************************
 * override the earth radius
 ********************************************************************/

extern void EG_set_earth_radius_km(double earth_radius_km);

/*********************************************************************
 * EG_lat_lon_to_dx_dy()
 *
 *  Given the latitude and longitude of a start point in degrees as well
 *  as the latitude and longitude of a second point in degrees
 *  calculate the x, y kilometer coordinates of the second point.
 *
 *  Input : lat1, lon1, lat2, lon2 in degrees (lat N+, lon E+)
 *  Output: Dx, Dy = the delta (x, y) in km,
 *          x being pos east, y pos north
 *
 ********************************************************************/

extern void EG_lat_lon_to_dx_dy(double lat1, double lon1,
				double lat2, double lon2,
				double *dx, double *dy);

/*********************************************************************
 * EG_lat_lon_to_r_theta()
 *
 *  Input : lat1, lon1, lat2, lon2 in degrees (lat N+, lon E+)
 *  Output: r = the arc length from 1 to 2, in km 
 *            theta =  angle with True North: positive if east of North,
 *          negative if west of North, 0 = North
 *
 *********************************************************************/

extern void EG_lat_lon_to_r_theta(double lat1, double lon1,
				  double lat2, double lon2,
				  double *r, double *theta);

/*******************************************************************
 * EG_lat_lon_plus_dx_dy()
 *
 *  Starting from a given lat, lon, draw an arc (part of a great circle)
 *  which moves dx, dy from the input lat, lon
 *
 *  Input : Starting point lat1, lon1 in degrees (lat N+, lon E+)
 *            arclengths dx, dy (km)
 *  Output: lat2, lon2, the ending point (degrees)
 *
 *******************************************************************/

extern void EG_lat_lon_plus_dx_dy(double lat1, double lon1,
				  double dx, double dy,
				  double *lat2, double *lon2);

/*******************************************************************
 * EG_lat_lon_plus_r_theta()
 *
 *  Starting from a given lat, lon, draw an arc (part of a great circle)
 *  of length r which makes an angle of theta from true North.
 *  Theta is positive if east of North, negative (or > PI) if west of North,
 *  0 = North
 *
 *  Input : Starting point lat1, lon1 in degrees (lat N+, lon E+)
 *            arclength r (km), angle theta (degrees)
 *  Output: lat2, lon2, the ending point (degrees)
 *
 *******************************************************************/

extern void EG_lat_lon_plus_r_theta(double lat1, double lon1,
				    double r, double theta,
				    double *lat2, double *lon2);



/*
     Generate a well-ordered boundary from a random set of input
     boundary points. If the input point set is star-shaped, i.e., a
     line segment from the centroid to a point P in the point set
     never intersects the unknown boundary except at P this function
     should correctly order the boundary points.  Returns -1 if num <=
     0. Otherwise returns 0.
*/
extern int EG_random_bdry
(
 Point_d *points,		/* I - input array of points */
 int num,			/* I - size of points array */
 Point_d *ctr,			/* O - center of mass */
 int *indices			/* O - indices of ordered output points */
 );

/* convex hull code */
extern void EG_dump_hull(double **P, double *start, int m);
extern int EG_ch2d(double **P, int n);
extern void EG_print_hull(double **P, double *start, int m);
extern int EG_read_points(double **P, double points[][2], int N);
extern int EG_chull(double **P, double *start, int n, int *indices);

#ifdef __cplusplus
}
#endif

#endif /* EUCLID_GEOMETRY_H */


