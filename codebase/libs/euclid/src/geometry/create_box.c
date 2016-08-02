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
 *     create_box
 *
 *   PURPOSE
 *     
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
 * Create a rectangle of total width 2*half_width where p1 and p2 are
 * midpoints on the top and bottom.  Return the height of the box.
 */
double EG_create_box(Point_d *p1, Point_d *p2, double half_width, Point_d *box)
{
  double len;
  Point_d unit_vect;
  Point_d vect;

  
  EG_vect_sub(p2, p1, &vect);
  len = EG_unit_vector_perp(&vect, &unit_vect);
  box[0].x = p1->x + half_width * unit_vect.x;
  box[0].y = p1->y + half_width * unit_vect.y;
  box[1].x = p1->x - half_width * unit_vect.x;
  box[1].y = p1->y - half_width * unit_vect.y;
  box[2].x = p2->x - half_width * unit_vect.x;
  box[2].y = p2->y - half_width * unit_vect.y;
  box[3].x = p2->x + half_width * unit_vect.x;
  box[3].y = p2->y + half_width * unit_vect.y;

  return(len);
}
