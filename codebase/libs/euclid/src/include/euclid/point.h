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

/* point.h - definitions for points */

#ifndef POINT_H
#define POINT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* #define abs(x) ((x) > 0 ? (x) : (-x)) */
#define sgn(x) ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))

/*
 * the following form is the equation of a line through x0,y0 and x1,y1 
 * the name of this macro should not be set to form because that conflicts
 * with other library macros 
 */
#define EG_form(x, y, x0, y0, x1, y1)  (((y)-(y0))*((x1)-(x0))-((x)-(x0))*((y1)-(y0)))


/* solving equations of lines for y or x */
#define x_sol(y, x0, y0, x1, y1)  ((x0) + ((y)-(y0))*((x1)-(x0))/((y1)-(y0)))
#define y_sol(x, x0, y0, x1, y1)  ((y0) + ((x)-(x0))*((y1)-(y0))/((x1)-(x0)))

/*
 * check_points:
 * if (a,b) and (c,d) are on the same side of the line segment determined
 * by (x0,y0) and (x1,y1) return 0
 */
#define check_points(a, b, c, d, x0, y0, x1, y1) \
  ((sgn(EG_form(a, b, x0, y0, x1, y1)) * sgn(EG_form(c, d, x0, y0, x1, y1))) < 0)


/* structure for integer points */
typedef struct 
{
  int x;
  int y;
} Point_i, pointi_t;

/* structure for double points */
typedef struct
{
  double x;
  double y;
} Point_d, pointd_t;

/* structure for double points about a ref pt (used by convex hull) */
typedef struct 
{
  Point_d pt;		/* x and y coordinates of point */
  double r;		/* distance from ref pt */
  double delta_x;	/* x dist from ref pt */
  double delta_y;	/* y dist from ref pt */
  double theta;		/* approx to angle of line from pt thru ref pt */
  int index;		/* index of point in a general array */
  int flag;		/* flag for optional use */
} Star_point;

/* structure for line segments */
typedef struct
{
  double x1;			/* x coord of first endpt */
  double y1;			/* y coord of first endpt */
  double x2;			/* x coord of second endpt */
  double y2;			/* y coord of first endpt */
  double delta_x;		/* x2 - x1 */
  double delta_y;		/* y2 - y1 */
  double length;		/* hypot(delta_x, delta_y) */
} Segment;

#ifdef __cplusplus
}
#endif

#endif
