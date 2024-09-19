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
/*******************************************************************
 * triangle_points.c
 *
 * PURPOSE
 *	Find the points inside a triangle and map to a grid
 *
 * DESCRIPTION:    
 * 	Given 3 points determining a triangle, set the grid points 
 * that are covered by the triangle.  Before calling the function, make
 * sure that vertices of the triangle are mapped to grid coordinates
 *
 * The grid origin is lower-left, with x changing the fastest. The 
 * (0, 0) point is the middle of the lower-left square.
 *
 * INPUTS:
 * 	p0, p1, p2 - vertices of the triangle
 *
 * OUTPUTS:
 * 	grid - grid to be filled
 * 	
 *
 * RETURNS:
 * 	void      
 *
 * METHOD:
 * 	
 * HISTORY
 *     wiener - May 12, 1992: Created.
 *     dixon -  December 1992: Modified.
 *******************************************************************/

#include "verify_tracks.h"

static int compare_x_coords(const void *v1, const void *v2);
static int compare_y_coords(const void *v1, const void *v2);

/*
 * the following form is the equation of a line through x0,y0 and x1,y1
 */

#define FORM(x, y, x0, y0, x1, y1)  (((y)-(y0))*((x1)-(x0)) - \
                                     ((x)-(x0))*((y1)-(y0)))

/*
 * solving equations of lines for y or x
 */

#define X_SOL(y, x0, y0, x1, y1)  ((x0) + ((y)-(y0))*((x1)-(x0))/((y1)-(y0)))
#define Y_SOL(x, x0, y0, x1, y1)  ((y0) + ((x)-(x0))*((y1)-(y0))/((x1)-(x0)))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/*
 * macro to constrain value to given limits
 */

#define DO_CONSTRAIN(x, low, high) if ((x) < (low)) (x) = (low); \
                                else if ((x) > (high)) (x) = (high)

void triangle_pts(point_t *pt0,
		  point_t *pt1,
		  point_t *pt2,
		  ui08 **grid,
		  long nx,
		  long ny,
		  double minx,
		  double miny,
		  double dx,
		  double dy)

{

  long i;
  long ix, iy;
  long iy1, iy2;
  long ix1, ix2;
  long min_ix, max_ix, min_iy, max_iy;

  double hp_val_y, hp_val_x;  /* halfplane value */
  double x, y;
  double xx1, xx2;
  double yy1, yy2;

  point_t tri[3];
  point_t a, b, c, d;

  /*
   * get the bounding grid locations
   */
  
  min_ix = LARGE_LONG;
  max_ix = -LARGE_LONG;
  min_iy = LARGE_LONG;
  max_iy = -LARGE_LONG;

  for (i = 0; i < 3; i++) {

    ix = (long) ((tri[i].x - minx) / dx + 0.5);
    iy = (long) ((tri[i].y - miny) / dy + 0.5);
    
    min_ix = MIN(min_ix, ix);
    min_iy = MIN(min_iy, iy);

    max_ix = MAX(max_ix, ix);
    max_iy = MAX(max_iy, iy);

  } /* i */
  
  DO_CONSTRAIN(min_ix, 0, nx - 1);
  DO_CONSTRAIN(min_iy, 0, ny - 1);
  DO_CONSTRAIN(max_ix, 0, nx - 1);
  DO_CONSTRAIN(max_iy, 0, ny - 1);

  /*
   * fill in the grid my moving up through the
   * relevant y coords
   */

  /*
   * sort the triangle vertices into vertical order
   */

  tri[0] = *pt0;
  tri[1] = *pt1;
  tri[2] = *pt2;

  qsort((char *) tri, 3, sizeof(point_t), compare_y_coords);

  hp_val_y = FORM(tri[1].x, tri[1].y,
		  tri[0].x, tri[0].y,
		  tri[2].x, tri[2].y);
  
  /*
   * fill in the grid
   */

  if (hp_val_y < 0) {
    a = tri[2];
    b = tri[1];
    c = tri[0];
    d = tri[1];
  } else {
    a = tri[1];
    b = tri[2];
    c = tri[1];
    d = tri[0];
  }

  iy1 = (long) ceil((tri[0].y - miny) / dy + 0.5);
  iy2 = (long) ceil((tri[1].y - miny) / dy + 0.5);

  DO_CONSTRAIN(iy1, min_iy, max_iy);
  DO_CONSTRAIN(iy2, min_iy, max_iy + 1);

  for (iy = iy1; iy < iy2; iy++) {

    y = (double) iy * dy + miny;

    xx1 = X_SOL((double) y, tri[0].x, tri[0].y, a.x, a.y);
    xx2 = X_SOL((double) y, tri[0].x, tri[0].y, b.x, b.y);

    ix1 = (long) floor ((xx1 - minx) / dx + 0.5);
    ix2 = (long) ceil ((xx2 - minx) / dx + 0.5);

/*    if (ix1 < 0 || ix2 < 0)
      Glob->debug = TRUE; */

    DO_CONSTRAIN(ix1, min_ix, max_ix);
    DO_CONSTRAIN(ix2, min_ix, max_ix + 1);

    for (ix = ix1; ix < ix2; ix++)
      grid[iy][ix] = 1;

  }

  iy1 = (long) ceil((tri[1].y - miny) / dy);
  iy2 = (long) ceil((tri[2].y - miny) / dy);

  DO_CONSTRAIN(iy1, min_iy, max_iy);
  DO_CONSTRAIN(iy2, min_iy, max_iy + 1);

  for (iy = iy1; iy < iy2; iy++) {

    y = (double) iy * dy + miny;

    xx1 = X_SOL((double) y, c.x, c.y, tri[2].x, tri[2].y);
    xx2 = X_SOL((double) y, d.x, d.y, tri[2].x, tri[2].y);
    
    ix1 = (long) floor ((xx1 - minx) / dx + 0.5);
    ix2 = (long) ceil ((xx2 - minx) / dx + 0.5);

/*    if (ix1 < 0 || ix2 < 0)
      Glob->debug = TRUE; */

    DO_CONSTRAIN(ix1, min_ix, max_ix);
    DO_CONSTRAIN(ix2, min_ix, max_ix + 1);

    for (ix = ix1; ix < ix2; ix++)
      grid[iy][ix] = 1;

  }

  /*
   * fill in the grid my moving across through the
   * relevant x coords
   */

  /*
   * sort the triangle vertices into horizontal order
   */

  qsort((char *) tri, 3, sizeof(point_t), compare_x_coords);

  hp_val_x = FORM(tri[1].y, tri[1].x,
                  tri[0].y, tri[0].x,
                  tri[2].y, tri[2].x);
  
  /*
   * fill in the grid
   */

  if (hp_val_x < 0) {
    a = tri[2];
    b = tri[1];
    c = tri[0];
    d = tri[1];
  } else {
    a = tri[1];
    b = tri[2];
    c = tri[1];
    d = tri[0];
  }

  ix1 = (long) ceil((tri[0].x - minx) / dx);
  ix2 = (long) ceil((tri[1].x - minx) / dx);
  
/*  if (ix1 < 0 || ix2 < 0)
    Glob->debug = TRUE; */

  DO_CONSTRAIN(ix1, min_ix, max_ix);
  DO_CONSTRAIN(ix2, min_ix, max_ix + 1);

  for (ix = ix1; ix < ix2; ix++) {

    x = (double) ix * dx + minx;

    yy1 = Y_SOL(x, tri[0].x, tri[0].y, a.x, a.y);
    yy2 = Y_SOL(x, tri[0].x, tri[0].y, b.x, b.y);

    iy1 = (long) floor ((yy1 - miny) / dy + 0.5);
    iy2 = (long) ceil ((yy2 - miny) / dy + 0.5);

    DO_CONSTRAIN(iy1, min_iy, max_iy);
    DO_CONSTRAIN(iy2, min_iy, max_iy + 1);

    for (iy = iy1; iy < iy2; iy++)
      grid[iy][ix] = 1;

  } /* ix */

  ix1 = (long) ceil((tri[1].x - minx) / dx);
  ix2 = (long) ceil((tri[2].x - minx) / dx);
  
/*  if (ix1 < 0 || ix2 < 0)
    Glob->debug = TRUE; */

  DO_CONSTRAIN(ix1, min_ix, max_ix);
  DO_CONSTRAIN(ix2, min_ix, max_ix + 1);

  for (ix = ix1; ix < ix2; ix++) {

    x = (double) ix * dx + minx;

    yy1 = Y_SOL((double) x, c.x, c.y, tri[2].x, tri[2].y);
    yy2 = Y_SOL((double) x, d.x, d.y, tri[2].x, tri[2].y);
    
    iy1 = (long) floor ((yy1 - miny) / dy + 0.5);
    iy2 = (long) ceil ((yy2 - miny) / dy + 0.5);

    DO_CONSTRAIN(iy1, min_iy, max_iy);
    DO_CONSTRAIN(iy2, min_iy, max_iy + 1);

    for (iy = iy1; iy < iy2; iy++)
      grid[iy][ix] = 1;

  } /* ix */

} /* triangle_pts */

/*****************************************************************************
 * define function to be used for sorting triangle points
 * based on the y coordinate (lowest to highest)
 */

static int compare_y_coords(const void *v1, const void *v2)

{
  
  double dy;

  point_t *p1 = (point_t *) v1;
  point_t *p2 = (point_t *) v2;

  dy = p1->y - p2->y;

  if (dy > 0)
    return (1);
  else if (dy < 0)
    return (-1);
  else
    return (0);

}

/*****************************************************************************
 * define function to be used for sorting triangle points
 * based on the x coordinate (lowest to highest)
 */

static int compare_x_coords(const void *v1, const void *v2)

{
  
  double dx;

  point_t *p1 = (point_t *) v1;
  point_t *p2 = (point_t *) v2;

  dx = p1->x - p2->x;

  if (dx > 0)
    return (1);
  else if (dx < 0)
    return (-1);
  else
    return (0);

}

