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
 *     unit_vector
 *
 *   PURPOSE
 *     Create unit vectors
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
 * unit_vector
 *
 * unit_vector - Convert vector to unit vector
 *
 * IN:  vector c
 * OUT: vector d
 * RETURNS: norm of vector or 0 if vector is 0
 */
double EG_unit_vector(Point_d *c, Point_d *d)
{
  double length;

  length = NORM(c);
  if (length < MACH_EPS)
    {
      d->x = 0;
      d->y = 0;
      return(0);
    }
  else
    {
      d->x = c->x / length;
      d->y = c->y / length;
      return(length);
    }
}

/*
 * unit_vector_perp                        
 *
 * unit_vector_perp - Convert vector to perpendicular unit vector 
 *
 * IN:  vector c
 * OUT: point d
 * RETURNS: norm of vector or 0 if vector is 0
 */
double EG_unit_vector_perp(Point_d *c, Point_d *d)
{
  double length;
  
  length = NORM(c);
  
  if (length < MACH_EPS)
    {
      d->x = 0;
      d->y = 0;
      return(0);
    }
  else
    {
      d->x = -c->y / length;
      d->y = c->x / length;
      return(length);
    }
}

