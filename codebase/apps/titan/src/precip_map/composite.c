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
/*********************************************************************
 * composite.c
 *
 * Computes and returns CAPPI indices for the composite
 *
 * RAP, NCAR, Boulder CO
 *
 * Jan 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "precip_map.h"

int cart_comp_base(cart_params_t *cart)

{

  double minz;
  double dz;
  int base_cappi;

  minz = (double) cart->minz / (double) cart->scalez;
  dz = (double) cart->dz / (double) cart->scalez;

  base_cappi =
    (int) ((Glob->params.composite_min_altitude - minz) / dz + 0.5);

  base_cappi = MAX(0, base_cappi);

  return (base_cappi);
  
}

int cart_comp_top(cart_params_t *cart)

{

  double minz;
  double dz;
  int top_cappi;

  minz = (double) cart->minz / (double) cart->scalez;
  dz = (double) cart->dz / (double) cart->scalez;

  top_cappi =
    (int) ((Glob->params.composite_max_altitude - minz) / dz + 0.5);

  top_cappi = MIN(cart->nz - 1, top_cappi);

  return (top_cappi);
  
}

int mdv_comp_base(mdv_grid_t *grid)

{

  int base_cappi;

  base_cappi =
    (int) ((Glob->params.composite_min_altitude - grid->minz) /
	   grid->dz + 0.5);

  base_cappi = MAX(0, base_cappi);

  return (base_cappi);
  
}

int mdv_comp_top(mdv_grid_t *grid)

{

  int top_cappi;

  top_cappi =
    (int) ((Glob->params.composite_max_altitude - grid->minz) /
	   grid->dz + 0.5);

  top_cappi = MIN(grid->nz - 1, top_cappi);

  return (top_cappi);
  
}

/*
 * create composite grid
 */

void create_composite(int npoints, int base, int top,
		      ui08 **vol, ui08 *comp)
     
{

  ui08 *c, *layer;
  int i, iz;

  if (base == top) {

    memcpy(comp, vol[base], npoints);

  } else {

    /* base != top */

    memset(comp, 0, npoints);
    
    for (iz = base; iz <= top; iz++) {
      
      c = comp;
      layer = vol[iz];
      
      for (i = 0; i < npoints; i++, c++, layer++) {
	if (*c < *layer) {
	  *c = *layer;
	}
      } /* i */
      
    } /* iz */

  }

}
    



