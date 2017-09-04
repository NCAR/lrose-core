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
/************************************************
 * umath.h : header file for math utility routines
 ************************************************/

#ifndef umath_h
#define umath_h

#ifdef __cplusplus
extern "C" {
#endif

#include <toolsa/toolsa_macros.h>
#include <stdio.h>

  /*
   * contingency table
   */

  typedef struct {
    
    double n_success;
    double n_failure;
    double n_false_alarm;
    double n_non_event;
    
    double pod;
    double far;
    double csi;
    double hss;
    double gss;
    
  } ucont_table_t;
  
  /*
   * prototypes
   */

  /*
   * uCurveFit.c
   */
  
  /*********************************************************************
   * uCurveFitDebug()
   *
   * Sets debugging status
   */

  extern void uCurveFitDebug(int dflag);
  
  /*********************************************************************
   * uExponentialFit()
   *
   * exponential fit to array of data
   *
   *  n: number of points in (x, y) data set
   *  x: array of x data
   *  y: array of y data
   *  a[] - exponential coefficients in equation :
   *               y = a[0] + a[1].exp(a[2].x)
   *  std_error - standard error of estimate
   *  r_squared - correlation coefficient squared
   *
   * Returns 0 on success, -1 on error.
   *
   */

  extern int uExponentialFit(long n, double *x, double *y, double *a,
			     double *std_error_est, double *r_squared);
  
  /**************************************************************************
   * uline_through_ellipse.c
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

  extern int uline_through_ellipse(double major_radius,
				   double minor_radius,
				   double slope,
				   double intercept,
				   double *xx1,
				   double *yy1,
				   double *xx2,
				   double *yy2);
  
  /***************************************************************************
   * uLinearFit.c : fit a line to a data series
   *
   * Mike Dixon  RAP, NCAR, Boulder, Colorado
   *
   * October 1990
   *
   *  n: number of points in (x, y) data set
   *  x: array of x data
   *  y: array of y data
   *  xmean, ymean: means
   *  xsdev, ysdev: standard deviations
   *  corr: correlation coefficient
   *  a[] - linear coefficients (a[0] - bias, a[1] - scale)
   *
   * Returns 0 on success, -1 on error.
   *
   ***************************************************************************/
 
  extern int uLinearFit(long n, double *x, double *y, double *a,
			double *xmean_p, double *ymean_p, double *xsdev_p,
			double *ysdev_p, double *corr_p,
			double *std_error_est_p,
			double *r_squared_p);

  /***************************************************************************
   * compute linear fit of line through x/y data, using principal components
   *
   * Compute a (slope) and b (intercept) so that: y = ax + b
   *
   * Returns 0 on success, -1 on error.
   *
   ***************************************************************************/
  
  extern int uLinearFitPC(int n, double *x, double *y,
                          double *a, double *b);
  
  /*********************************************************************
   * uPolyFit()
   *
   * polynomial fit to array of data
   *
   *  order: order of polynomial (linear = 1, quadratic = 2 etc. )
   *  a[] - polynomial coefficients
   *     y = a[0] + a[1] * x + a[2] * x**2 + ...
   *  n: number of points in (x, y) data set
   *  x: array of x data
   *  y: array of y data
   *  std_error - standard error of estimate
   *  r_squared - correlation coefficient squared
   *
   * Returns 0 on success, -1 on error.
   *
   */

  extern int uPolyFit(long n, double *x, double *y, double *a,
		      long order,
		      double *std_error_est, double *r_squared);
     
  /***************************************************************************
   * uQuadFit.c : fit a quadratic to a data series
   *
   * Mike Dixon  RAP, NCAR, Boulder, Colorado
   *
   * October 1990
   *
   *  n: number of points in (x, y) data set
   *  x: array of x data
   *  y: array of y data
   *  a[] - quadratic coefficients (a[0] - bias, a[1] - linear, a[2] - squared)
   *  std_error - standard error of estimate
   *  r_squared - correlation coefficient squared
   *
   * Returns 0 on success, -1 on error.
   *
   ***************************************************************************/

  extern int uQuadFit(long n, double *x, double *y, double *a,
		      double *std_error_est_p, double *r_squared_p);

  /****************************************************************************
   * uQuadWtFit.c : fit a quadratic to a data series, using weigths
   *
   *  n: number of points in (x, y) data set
   *  x: array of x data
   *  y: array of y data
   *  a[] - quadratic coefficients (a[0] - bias, a[1] - linear, a[2] - squared)
   *  std_error - standard error of estimate
   *  r_squared - correlation coefficient squared
   *
   **************************************************************************/

  extern int uQuadWtFit(long n, double *x, double *y, double *weight,
			double *a, double *std_error_est, double *r_squared);

  /*
   * uLatLon.c
   */

  /*********************************************************************
   * uLatLon2DxDy()
   *
   *  Input : lat1, lon1, lat2, lon2 in degrees (lat N+, lon E+)
   *  Output: Dx, Dy = the delta (x, y) in km,
   *          x being pos east, y pos north
   *
   ********************************************************************/

  extern void uLatLon2DxDy(double lat1, double lon1,
			   double lat2, double lon2,
			   double *dx, double *dy);

  /*********************************************************************
   * uLatLon2RTheta()
   *
   *  Input : lat1, lon1, lat2, lon2 in degrees (lat N+, lon E+)
   *  Output: r = the arc length from 1 to 2, in km 
   *	    theta =  angle with True North: positive if east of North,
   *          negative if west of North, 0 = North
   *
   *********************************************************************/

  extern void uLatLon2RTheta(double lat1, double lon1,
			     double lat2, double lon2,
			     double *r, double *theta);

  /*******************************************************************
   * uLatLonPlusDxDy()
   *
   *  Starting from a given lat, lon, draw an arc (part of a great circle)
   *  which moves dx, dy from the input lat, lon
   *
   *  Input : Starting point lat1, lon1 in degrees (lat N+, lon E+)
   *	    arclengths dx, dy (km)
   *  Output: lat2, lon2, the ending point (degrees)
   *
   *******************************************************************/

  extern void uLatLonPlusDxDy(double lat1, double lon1,
			      double dx, double dy,
			      double *lat2, double *lon2);

  /*******************************************************************
   * uLatLonPlusRTheta()
   *
   *  Starting from a given lat, lon, draw an arc (part of a great circle)
   *  of length r which makes an angle of theta from true North.
   *  Theta is positive if east of North, negative (or > PPI) if west of North,
   *  0 = North
   *
   *  Input : Starting point lat1, lon1 in degrees (lat N+, lon E+)
   *	    arclength r (km), angle theta (degrees)
   *  Output: lat2, lon2, the ending point (degrees)
   *
   *******************************************************************/

  extern void uLatLonPlusRTheta(double lat1, double lon1,
				double r, double theta,
				double *lat2, double *lon2);

  /*
   * uintersect.c
   */

  /*******************************************************************************
   * uintersect.c : tests whether two straight lines intersect
   *
   * utility routine
   *
   * args:
   *       xx1, yy1, xx2, yy2 - floats - end points of one line
   *       uu1, vv1, uu2, vv2 - floats - end points of the other line
   *
   * returns 1 if they intersect, 0 if not
   *
   * Mike Dixon, RAP, NCAR, August 1990
   ******************************************************************************/
  extern  int uintersect(double xx1, double yy1,
			 double xx2, double yy2,
			 double uu1, double vv1,
			 double uu2, double vv2);

  /*
   * ulong_scale.c
   */

  /*************************************************************************
   * ulong_scale.c - math utilities library
   *
   * returns a long for scaling a double into a long
   */

  extern long ulong_scale(double x);

  /*
   * umax_wt_bip.c
   */

  /*
   * Assume G = (S, T, E) is a bipartite graph with vertex sets S, T and
   * edges E.
   *
   * Inputs:
   *
   * umax_wt_bip(cost, sizeS, sizeT, no_edge_flag, match)
   *
   * cost - the cost matrix - the match should maximize the sum of entries
   *        from the cost matrix
   * sizeS - the number of vertices in S.
   * sizeT - the number of vertices in T
   *         NOTE: sizeS <= sizeT
   * flag - to indicate that an edge is invalid. Should be about half the
   *        max entry in the cost matrix
   *
   * Output: A maximum weighted matching of B.
   *
   * match - a maximum matching of B.
   */

  extern void umax_wt_bip(long **cost, long sizeS, long sizeT,
			  long no_edge_flag, long *match);

  /*
   * uPct.cc 
   */

  /**************************************************************************
   * uPct
   *
   * obtains the principal component transformation for the data
   * 
   * Args: long ndim - the dimension of the data vector.
   *       longg ndata - the number of data vectors.
   *       double **data - array for the data vectors, in the order
   *         data[ndata][ndim].
   *       double *means - address of a (double *) which points to
   *         the array of means. This array is allocated by this routine.
   *       double **eigenvectors - address of a (double **) which points
   *         to the eigenvector matrix array x. The eigenvectors are in the
   *         columns, where the columns are referred to by the second
   *         dereference, i.e. x[irow][icol].
   *       double *eigenvalues - address of a (double *) which points to
   *         the eigenvalue array lambda.
   *
   * Return value: 0 on success, -1 on failure
   *
   **************************************************************************/

  extern int upct(long ndim, long ndata,
		  double **data, double *means, double **eigenvectors,
		  double *eigenvalues);
  
  /*
   * upoint_in_ellipse.c
   */

  /**************************************************************************
   * upoint_in_ellipse.c
   *
   * Checks whether a given point lies inside a (rotated) ellipse. The search
   * radius is multiplied by a factor to allow the search to be extended
   * beyond the ellipse or restricted to a portion of the ellipse.
   *
   * If within, returns TRUE. Else, returns FALSE.
   */

  extern int upoint_in_ellipse(double ellipse_x, double ellipse_y,
			       double major_radius, double minor_radius,
			       double axis_rotation,
			       double search_x, double search_y,
			       double search_radius_ratio);

  /*
   * upoint_in_polygon.c
   */

  typedef struct {
    double x, y;
  } upoint_t;

  /*----------------
   * pt_in_polygon()
   *
   * Returns 1 if pt is inside polygon, 0 otherwise.
   * A pt lying on the polygon boundary is included in the polygon.
   */

  extern int upoint_in_polygon(upoint_t pt, upoint_t *vertex, int nvertices);

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

  extern int upoint_in_polygon2(upoint_t pt, upoint_t *vertex, int nvertices);

  /*
   * usign.c
   */

  /*************************************************************************
   * usign.c - math utilities library
   *
   * returns +1 if arg is non-negative, -1 if negative
   */

  extern int usign(double x);

  /*
   * ustat.c - statistical routines
   */

  /*****************
   * ucompute_cont()
   * 
   * compute contingency table
   */

  extern void ucompute_cont(ucont_table_t *cont);
  
  /***************
   * uprint_cont()
   * 
   * print contingency table
   */

  extern void uprint_cont(ucont_table_t *cont,
			  FILE *out,
			  char *label,
			  char *spacer,
			  int print_hss,
			  int print_gss);
     
  extern double usdev(double sum, double sumsq, double n);

  /*
   * uNewtRaph.c
   */

  typedef void (*NewtRaphFunc)(double x, double *val, double *deriv);

  /*************
   * uNewtRaph.cc
   *
   * User must provide function funcd, which returns the funcion value
   * and derivative at point x.
   *
   * funcd: method that computes the value and derivate of the function.
   *   typedef void (*NewtRaphFunc)(double x, double *val, double *deriv);
   *
   * xLower, xUpper: values bracketing the desired root.
   * accuracy: accuracy of test for root.
   * root_p: pointer to root to be returned.
   */

  extern int uNewtRaph(NewtRaphFunc myFunc,
		       double x1, double x2,
		       double xacc,
		       double *root_p);


  /*
   * dir_speed_2_uv.c
   */

  /*----------------------------------------------------------------*/
  /*
   * Convert from speed, dir (clockwise from true north, i.e. radar coords) 
   * to u, v.
   */ 
  /* speed - in km/h, m/s, whatever
   * dir   - in degrees
   * u, v  - same units as speed
   */

  extern void dir_speed_2_uv(float speed, float dir, float *u, float *v);

  /*----------------------------------------------------------------*/
  /*
   * Convert from speed, dir (counter-clockwise from true north, 
   *                          i.e. wind convention)
   * to u, v.
   */
  /* speed - in km/h, m/s, whatever
   * dir   - in degrees
   * u, v  - same units as speed
   */
  extern void wind_dir_speed_2_uv(float speed, float dir, float *u, float *v);

  /*
   * uv_2_dir_speed.c
   */

  /*----------------------------------------------------------------*/
  /*
   * Convert from u, v to speed, dir (clockwise from true north, i.e. radar 
   * coords).
   */
  /* u, v  - in km/h, m/s, whatever
   * dir   - in degrees
   * speed - same units as u and v
   */

  extern void uv_2_dir_speed(float u, float v, float *dir, float *speed);
  extern void uv_2_dir_speed_d(double u, double v, double *dir, double *speed);

  /*----------------------------------------------------------------*/
  /*
   * Convert from u, v to speed, dir (counter-clockwise from true north, 
   *                                  i.e. wind convention)
   */
  /* u, v  - in km/h, m/s, whatever
   * dir   - in degrees
   * speed - same units as u and v   
   */

  extern void uv_2_wind_dir_speed(float u, float v, float *dir, float *speed);

#ifdef __cplusplus
}
#endif

#endif
