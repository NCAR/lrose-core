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
 *
 * Mike Dixon RAP NCAR Boulder Colorado USA
 *
 * March 1996
 *
 *************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <euclid/geometry.h>
#include <euclid/alloc.h>

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

/*
 * main routine
 */

long EG_fill_polygon(Point_d *vertices, int nvertices,
                     long nx, long ny,
                     double minx, double miny,
                     double dx, double dy,
                     unsigned char *grid_array,
                     int add_val)
     
{

  unsigned char *g;
  long nfilled = 0;
  long i, ix, ix1, ix2, iy;
  long index;
  long nsides = nvertices - 1;
  long ncrossings;
  double y;
  double poly_y_min, poly_y_max;
  double slope;
  Point_d *pt, *pt1, *pt2;
  EG_poly_side_t *sides, *sd;
  EG_crossing_t *crossings, *cr, *cr1, *cr2;
  
  /*
   * ensure that no vertex lies on a horizintal grid line
   */

  pt = vertices;

  for (i = 0; i < nvertices; i++, pt++) {
    y = (pt->y - miny) / dy;
    if (fabs(fmod(y, 1.0)) < 0.0001) {
      pt->y += dy / 1000.0;
    }
  } /* i */

  /*
   * alloc mem for sides and crossings
   */

  sides = (EG_poly_side_t *) EG_malloc((nsides) * sizeof(EG_poly_side_t));
  crossings = (EG_crossing_t *) EG_malloc((nsides) * sizeof(EG_crossing_t));

  /*
   * load sides, ensuring start has lesser y
   */

  poly_y_min = 1.0e99;
  poly_y_max = -1.0e99;
  sd = sides;
  pt1 = vertices;
  pt2 = pt1 + 1;
  
  for (i = 0; i < nsides; i++, sd++, pt1++, pt2++) {
    if (pt1->y < pt2->y) {
      sd->start = *pt1;
      sd->end = *pt2;
    } else {
      sd->start = *pt2;
      sd->end = *pt1;
    }
    poly_y_min = MIN(poly_y_min, sd->start.y);
    poly_y_max = MAX(poly_y_max, sd->end.y);
  }

  /*
   * loop through the y values in the grid
   */

  for (iy = 0; iy < ny; iy++) {

    y = miny + (double) iy * dy;

    if (y < poly_y_min || y > poly_y_max) {
      continue;
    }
    
    /*
     * find the segments which straddle the y value, and load
     * these into the crossings array
     */

    ncrossings = 0;
    sd = sides;
    for (i = 0; i < nsides; i++, sd++) {
      if (sd->start.y < y && sd->end.y > y) {
        crossings[ncrossings].side = *sd;
        ncrossings++;
      }
    } /* i */

    if ((ncrossings % 2) != 0) {
      fprintf(stderr, "ERROR - fill_polygon\n");
      fprintf(stderr, "ncrossings should always be even\n");
      continue;
    }
    
    /*
     * for each crossing, compute the x val of the crossing
     */

    cr = crossings;
    for (i = 0; i < ncrossings; i++, cr++) {
      slope = ((cr->side.end.y - cr->side.start.y) /
               (cr->side.end.x - cr->side.start.x));
      cr->x = cr->side.start.x + (y - cr->side.start.y) / slope;
    } /* i */
    
    /*
     * sort the crossings in ascending order of x
     */
    
    qsort((void *) crossings, ncrossings,
          sizeof(EG_crossing_t), EG_polygon_crossing_compare);
    
    /*
     * for each pair of crossings, compute the range in x and
     * set the relevant grid points
     */

    cr1 = crossings;
    cr2 = cr1 + 1;
    for (i = 0; i < ncrossings; i += 2, cr1 += 2, cr2 += 2) {
      
      ix1 = (int) ((cr1->x - minx) / dx + 1.0);
      ix2 = (int) ((cr2->x - minx) / dx);

      ix1 = MAX(ix1, 0);
      ix1 = MIN(ix1, nx - 1);
      
      ix2 = MAX(ix2, 0);
      ix2 = MIN(ix2, nx - 1);

      index = iy * nx + ix1;
      
      g = grid_array + index;

      for (ix = ix1; ix <= ix2; ix++, g++) {
        *g = *g + add_val;
        nfilled++;
      }
      
    } /* i */
    
  } /* iy */

  /*
   * free up sides
   */
    
  EG_free((void *) sides);
  EG_free((void *) crossings);

  return (nfilled);

}

/*---------------------------------------------
 * define function to be used for sorting sides
 */

int EG_polygon_crossing_compare(const void *v1, const void *v2)

{

  EG_crossing_t *c1 = (EG_crossing_t *) v1;
  EG_crossing_t *c2 = (EG_crossing_t *) v2;

  return (c1->x - c2->x);

}

