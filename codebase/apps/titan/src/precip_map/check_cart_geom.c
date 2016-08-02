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
/****************************************************************************
 * check_cart_geom.c
 *
 * Check the cart geometry between storm file scan and mdv file
 * to make sure they are identical.
 *
 * Returns 0 on success, -1 on failure
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * August 1995
 *
 ****************************************************************************/

#include "precip_map.h"

int check_cart_geom(cart_params_t *radar_cart,
		    titan_grid_t *storm_grid)

{
  
  static int First_call = TRUE;
  static titan_grid_t Radar_grid;

  int retval = 0;

  if (First_call) {
    RfCartParams2TITANGrid(radar_cart, &Radar_grid, TITAN_PROJ_UNKNOWN);
    First_call = FALSE;
  }

  if (Radar_grid.nx != storm_grid->nx)
    retval = -1;
  if (Radar_grid.ny != storm_grid->ny)
    retval = -1;
  if (Radar_grid.nz != storm_grid->nz)
    retval = -1;

  if (Radar_grid.dx != storm_grid->dx)
    retval = -1;
  if (Radar_grid.dy != storm_grid->dy)
    retval = -1;
  if (Radar_grid.dz != storm_grid->dz)
    retval = -1;

  if (Radar_grid.minx != storm_grid->minx)
    retval = -1;
  if (Radar_grid.miny != storm_grid->miny)
    retval = -1;
  if (Radar_grid.minz != storm_grid->minz)
    retval = -1;

  return (retval);

}

