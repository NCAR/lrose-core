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
 *     line_intersect
 *
 *   PURPOSE
 *     Find the intersection point of two lines.
 *
 *   NOTES
 *     
 *
 *   HISTORY
 *      wiener - May 8, 1995: Created.
 */

#include <euclid/copyright.h>
#include <euclid/geometry.h>
#include <toolsa/toolsa_macros.h>
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
int EG_line_intersect(Point_d *a, Point_d *b, Point_d *c, Point_d *d, Point_d *e, double *t)
{
  Point_d cd, ca, perp;
  double denom;

  /* initialize */
  *t = 0.0;
  
  /* find the vector normal to ab */
  perp.x = -(b->y - a->y);
  perp.y = b->x - a->x;
  cd.x = d->x - c->x;
  cd.y = d->y - c->y;
  ca.x = a->x - c->x;
  ca.y = a->y - c->y;

  denom = DOT(&perp, &cd);

  if (ABS(denom) < MACH_EPS)
    {
      if (ABS(cd.x) + ABS(cd.y) < MACH_EPS || ABS(perp.x) + ABS(perp.y) < MACH_EPS)
        return(-1);
      
      if (ABS(DOT(&perp, &ca)) < MACH_EPS)
        return(2);
      else
        return(0);              /* lines are parallel */
    }
  
  *t = DOT(&perp, &ca)/denom;
  e->x = c->x + *t * cd.x;
  e->y = c->y + *t * cd.y;

  return(1);
} /* line_intersect */

/*
 * Given a line, a point and an angle find the intersection of the ray with the line. Return values follow the paradigm of EG_line_intersect().
 */
int EG_ray_line_intersect
(
 Point_d *a,			/* first point on line */
 Point_d *b,			/* second point on line */
 Point_d *c,			/* point */
 double angle,			/* mathematical angle in radians (use M_PI/2.0 - true_north_angle if angle is being measured from true north) */
 Point_d *e,			/* intersection point */
 double *t			/* e = a + t * (b - a) */
)
{
  Point_d *d;
  Point_d tmp;
  double delta;
  double ret;

  d = &tmp;			/* to promote uniform notation */

  /* generate another point on the ray */
  delta =  fabs(angle - M_PI/2.0);
  if (delta < MACH_EPS)
    {
      d->x = c->x;
      d->y = c->y + 1.0;
    }
  else
    {
      d->x = c->x + 1.0;
      d->y = c->y + tan(angle);
    }
  
  ret = EG_line_intersect(c, d, a, b, e, t);
  return(ret);
}

#ifdef TEST_MAIN
int main(int argc, char **argv)
{
  int ret;
  Point_d a;
  Point_d b;
  Point_d c;
  Point_d e;
  double angle;
  double t;

  a.x = 0.0;
  a.y = 10.0;
  b.x = 10.0;
  b.y = 0.0;
  c.x = 0;
  c.y = 0;
  angle = M_PI/4.0;
  
  ret = EG_ray_line_intersect(&a, &b, &c, angle, &e, &t);
  printf("ret = %d, e = %g, %g, t = %g\n", ret, e.x, e.y, t);

  a.x = 0.0;
  a.y = 10.0;
  b.x = 10.0;
  b.y = 10.0;
  c.x = 0;
  c.y = 0;
  angle = M_PI/4.0;
  
  ret = EG_ray_line_intersect(&a, &b, &c, angle, &e, &t);
  printf("ret = %d, e = %g, %g, t = %g\n", ret, e.x, e.y, t);

  a.x = 0.0;
  a.y = 10.0;
  b.x = 10.0;
  b.y = 10.0;
  c.x = 0;
  c.y = 0;
  angle = M_PI/2.0;
  
  ret = EG_ray_line_intersect(&a, &b, &c, angle, &e, &t);
  printf("ret = %d, e = %g, %g, t = %g\n", ret, e.x, e.y, t);
  
  a.x = 0.0;
  a.y = 10.0;
  b.x = 10.0;
  b.y = 10.0;
  c.x = 0;
  c.y = 0;
  angle = 0;
  
  ret = EG_ray_line_intersect(&a, &b, &c, angle, &e, &t);
  printf("ret = %d, e = %g, %g, t = %g\n", ret, e.x, e.y, t);

}
#endif
