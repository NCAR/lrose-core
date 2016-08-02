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
 *     linear_comb
 *
 *   PURPOSE
 *     Form a linear combination of two vectors
 *
 *   NOTES
 *     
 *
 *   HISTORY
 *      wiener - May 8, 1995: Created.
 */

#include <euclid/copyright.h>
#include <euclid/geometry.h>

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
void EG_linear_comb(Point_d *u, Point_d *v, Point_d *w, Point_d *z)
{
  z->x = u->x * w->x + v->x * w->y;
  z->y = u->y * w->x + v->y * w->y;
}


