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
/***************************************************************************
 * compute_grid.c
 *
 * Performs the grid computations
 *
 * Mike Dixon
 * Marion Mittermaier
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * Aug 1998
 *
 ****************************************************************************/

#include "dva_mkgrid.h"

static int _pos_compare(const void *v1, const void *v2);

void compute_grid(int iz, int npts_cappi, dva_position_t *pos)

{

  double angle;
  double cappi_ht;
  double yy, xx, zz;
  double zcor, zheight;
  short ix, iy;
  dva_position_t *pp;

  double rad_to_deg = 180.0 / PI;

  pp = pos;
      
  for ( iy = 1; iy < Glob->params.output_grid.ny + 1; iy ++ )
    {

      yy = iy + Glob->params.output_grid.miny;
      
      for ( ix = 1; ix < Glob->params.output_grid.nx + 1; ix++ , pp++)
	{

	  xx = ix + Glob->params.output_grid.minx;
	  pp->x = ix;
	  pp->y = iy;

	  cappi_ht =  (Glob->params.output_grid.minz +
		       Glob->params.output_grid.dz * iz);

	  zz  = cappi_ht - Glob->params.radar_altitude;

	  pp->z = zz;
	  
	  if ( yy == 0. && xx == 0. ) 
	    {
	      angle = 360.;
	    } 
	  else
	    {
	      angle = atan2 (yy, xx) * rad_to_deg;
	    }
	  
	  if ( angle < 0. )
	    {
	      pp->azim = 90. - angle;
	    } 
	  else if ( angle >= 0. && angle < 90.) 
	    {
	      pp->azim = 90. - angle;
	    } 
	  else 
	    {
	      pp->azim = 360. - (angle - 90.);
	    }
	  
	  zcor = ((xx*xx) + (yy*yy))/17080.;
	  zheight = zz - zcor;
	  
	  pp->range = sqrt ((xx*xx) + (yy*yy) + (zz*zz));
	  pp->elev = asin (zheight/pp->range) * rad_to_deg;
	  
	  
	} /* ix */
      
    } /* iy */
  
  /*
   * sort the array on azimuth
   */
  
  qsort(pos, npts_cappi, sizeof(dva_position_t), _pos_compare);
  
  if (Glob->params.debug) {
    int i;
    pp = pos;
    for (i = 0; i < npts_cappi; i++, pp++) {
      fprintf(stderr, "azim, elev, range, z, y, x: %g, %g, %g, %g, %d, %d\n",
	      pp->azim, pp->elev, pp->range, pp->z, pp->y, pp->x);
    }
  }
  
  return;

}

/*
 * define function to be used for sorting
 */

static int _pos_compare(const void *v1, const void *v2)

{

  dva_position_t *pp1 = (dva_position_t *) v1;
  dva_position_t *pp2 = (dva_position_t *) v2;
  double diff = pp1->azim - pp2->azim;
  
  if (diff > 0) {
    return (1);
  } else if (diff < 0) {
    return (-1);
  } else {
    return(0);
  }

}



