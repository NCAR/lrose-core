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
 * Module: random_bdry.c
 *
 * Author: Gerry Wiener
 *
 * Date:   4/22/03
 *
 * Description:
 *
 *     Generate a boundary from a random set of input points. If the input point set is star-shaped, i.e., the line segment from the centroid
 *     to a point in the point set never crosses another line from the centroid to a point in the point set, this function should correctly list out the boundary.
 *     the boundary generated 
 *
 */

/* Include files */
#include <math.h>
#include <stdlib.h>
#include <euclid/point.h>
#include <toolsa/toolsa_macros.h>

/* Constant, macro and type definitions */

/* Global variables */

/* Functions */


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



int cmp( const void *p1, const void *p2)
{
  return(**(double **)p2 - **(double **)p1);
}

/*
     Generate a well-ordered boundary from a random set of input
     boundary points. If the input point set is star-shaped, i.e., a
     line segment from the centroid to a point P in the point set
     never intersects the unknown boundary except at P this function
     should correctly order the boundary points.  Returns -1 if num <=
     0. Otherwise returns 0.
*/
int EG_random_bdry
(
 Point_d *points,		/* I - input array of points */
 int num,			/* I - size of points array */
 Point_d *ctr,			/* O - center of mass */
 int *indices			/* O - indices of ordered output points */
)
{
  double dx;
  double dy;
  int i;
  double *theta;
  double **theta_ptr;

  if (num <= 0)
    return -1;

  theta = (double *) malloc(num * sizeof(double));
  theta_ptr = (double **) malloc(num * sizeof(double *));

  ctr->x = 0;
  ctr->y = 0;

  /* find centroid of points */
  for (i=0; i<num; i++)
    {
      ctr->x += points[i].x;
      ctr->y += points[i].y;
    }

  ctr->x /= num;
  ctr->y /= num;

  /* calculate the angle of each ray */  
  for (i=0; i<num; i++)
    {
      dx = points[i].x - ctr->x;
      dy = points[i].y - ctr->y;
      theta[i] = atan2_adj(dy, dx); 
      theta_ptr[i] = &theta[i];
    }


  /* sort the angles into ascending order */
  qsort(theta_ptr, num, sizeof(double *), cmp);
  
  for (i=0; i<num; i++)
    {
      indices[i] = theta_ptr[i] - &theta[0];
    }
      
  free(theta);
  free(theta_ptr);

  return 0;
} /* random_bdry */


#ifdef TEST_MAIN
int main(int argc, char **argv)
{
  Point_d ctr;
  int i;
  int indices[100];
  Point_d pts[100];

  // Set up points array
  pts[0].x = 0;
  pts[0].y = 0;

  pts[1].x = 1;
  pts[1].y = 0;

  pts[2].x = 1;
  pts[2].y = 1;


  pts[3].x = 0;
  pts[3].y = 1.0;

  pts[4].x = 0.5;
  pts[4].y = 2.0;


  EG_random_bdry(pts, 5, &ctr, indices);


  printf("ctr = %g, %g\n", ctr.x, ctr.y);

  printf("Original points:\n");
  for (i=0; i<5; i++)
    {
      printf("pts[%d] = %g, %g\n", i, pts[i].x, pts[i].y);
    }

  printf("Ordered points:\n");
  for (i=0; i<5; i++)
    {
      printf("ordered[%d] = %g, %g\n", i, pts[indices[i]].x, pts[indices[i]].y);
    }

  return 0;
}
#endif
