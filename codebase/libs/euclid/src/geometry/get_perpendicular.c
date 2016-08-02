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
 * 	get_perp
 *
 * PURPOSE
 * 	Given a point P and a line L, find the point on the line L closest
 * in distance to P.
 *
 * NOTES
 * 	
 *
 * HISTORY
 *     wiener - Feb 24, 1993: Created.
 *
 */

#include <euclid/point.h>

/*
 * Given the point a and the line segment bc, find point d on the line
 * determined by bc such that ad is perpendicular to bc (pg 10 Graphics Gems)
 */
void EG_get_perp(Point_d *a, Point_d *b, Point_d *c, Point_d *d)
{
  double ba_x;
  double ba_y;
  double cb_x;
  double cb_y;
  double dot;
  double normal_x;
  double normal_y;
  double quot;
  double sq_normal;

  ba_y = b->y - a->y;
  ba_x = b->x - a->x;
  cb_x = c->x - b->x;
  cb_y = c->y - b->y;

  normal_x = -cb_y;
  normal_y = cb_x;
  dot = normal_x * ba_x + normal_y * ba_y;
  sq_normal = normal_x * normal_x + normal_y * normal_y;
  quot = dot / sq_normal;
  d->x = a->x + quot * normal_x;
  d->y = a->y + quot * normal_y;
}

/*
 * Given the point a and the line segment bc, find the square of the
 * distance to the line determined by bc
 */
double EG_get_perp_dist(Point_d *a, Point_d *b, Point_d *c)
{
  double ba_x;
  double ba_y;
  double cb_x;
  double cb_y;
  double dot;
  double sq_ab;
  double sq_dot;
  double sq_cb;

  ba_y = b->y - a->y;
  ba_x = b->x - a->x;
  cb_x = c->x - b->x;
  cb_y = c->y - b->y;

  sq_ab = ba_x * ba_x + ba_y * ba_y;
  sq_cb = cb_x * cb_x + cb_y * cb_y;
  dot = ba_x * cb_x + ba_y * cb_y;
  sq_dot = dot * dot;
  return(sq_ab - sq_dot / sq_cb);
}

/*
 * Given the point a and the line segment bc, find the square of the
 * distance to the line determined by bc.  Also return a sign
 * corresponding to the halfplane where a is located.
 */
double EG_get_perp_sign_dist(Point_d *a, Point_d *b, Point_d *c, int *sign)
{
  double ba_x;
  double ba_y;
  double cb_x;
  double cb_y;
  double dot;
  double sq_ab;
  double sq_dot;
  double sq_cb;

  ba_y = b->y - a->y;
  ba_x = b->x - a->x;
  cb_x = c->x - b->x;
  cb_y = c->y - b->y;

  sq_ab = ba_x * ba_x + ba_y * ba_y;
  sq_cb = cb_x * cb_x + cb_y * cb_y;
  dot = ba_x * cb_x + ba_y * cb_y;
  sq_dot = dot * dot;
  *sign = -cb_y * ba_x + cb_x * ba_y;
  return(sq_ab - sq_dot / sq_cb);
}


