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
 * Set up and serve out composite grid
 *
 * RAP, NCAR, Boulder CO
 *
 * February 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "storm_ident.h"
#include <assert.h>

static ui08 *CompGrid = NULL;

ui08 *get_comp_grid(void)
{
  assert (CompGrid);
  return (CompGrid);
}

void create_comp_grid(vol_file_handle_t *v_handle)

{

  static ui08 *comp_grid = NULL;
  static int n_comp_alloc = 0;

  int i, iz;
  int nbytes;
  ui08 *comp, *dbz;
  cart_params_t *cart = &v_handle->vol_params->cart;

  /*
   * if only one plane, that plane is the comp grid
   */
  
  if (cart->nz == 1) {
    CompGrid = v_handle->field_plane[Glob->params.dbz_field][0];
    return;
  }

  /*
   * allocate the composite grid
   */
  
  nbytes = cart->ny * cart->nx;

  if (nbytes > n_comp_alloc) {
    if (comp_grid == NULL) {
      comp_grid = (ui08 *) umalloc (nbytes);
    } else {
      comp_grid = (ui08 *) urealloc (comp_grid, nbytes);
    }
    n_comp_alloc = nbytes;
  }
  memset(comp_grid, 0, nbytes);
  
  /*
   * compute the composite reflectivity grid - this is the
   * max reflectivity at any height
   */

  for (iz = 0; iz < cart->nz; iz++) {

    dbz = v_handle->field_plane[Glob->params.dbz_field][iz];
    comp = comp_grid;

    for (i = 0; i < cart->nx * cart->ny; i++, dbz++, comp++) {
      if (*dbz > *comp) {
	*comp = *dbz;
      }
    } /* i */

  } /* iz */

  CompGrid = comp_grid;
  return;

}
