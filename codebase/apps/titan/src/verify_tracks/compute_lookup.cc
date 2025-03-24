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
/*****************************************************************************
 * compute_lookup.c
 *
 * compute the lookup arrays which relate the scan cartesian grid points
 * to their respective verification grid points
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * February 1992
 *
 *****************************************************************************/

#include "verify_tracks.h"

void compute_lookup(titan_grid_t *grid,
		    long *x_lookup,
		    long *y_lookup)

{

  long ix, iy;
  long jx, jy;

  double xx, yy;

  xx = grid->minx;

  for (ix = 0; ix < grid->nx; ix++) {

    jx = (long) ((xx - Glob->minx) / Glob->dx + 0.5);	
    
    if (jx >= 0 && jx < Glob->nx)
      *x_lookup = jx;
    else
      *x_lookup = -1;

    xx += grid->dx;
    x_lookup++;

  } /* ix */

  yy = grid->miny;

  for (iy = 0; iy < grid->ny; iy++) {

    jy = (long) ((yy - Glob->miny) / Glob->dy + 0.5);	
    
    if (jy >= 0 && jy < Glob->ny)
      *y_lookup = jy;
    else
      *y_lookup = -1;

    yy += grid->dy;
    y_lookup++;

  } /* iy */

}
