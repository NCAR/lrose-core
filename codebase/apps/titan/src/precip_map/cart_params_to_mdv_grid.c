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

#include "precip_map.h"

/*************************************************************************
 *
 * cart_params_to_mdv_grid()
 *
 * Copies a cart_params_t struct to mdv_grid_t struct
 *
 **************************************************************************/

void cart_params_to_mdv_grid(cart_params_t *cart,
			     mdv_grid_t *grid,
			     si32 grid_type)
     
{

  double range, azimuth;
  double lat, lon;

  memset((void *) grid, 0, sizeof(mdv_grid_t));
  
  grid->nbytes_char = MDV_N_GRID_LABELS * MDV_GRID_UNITS_LEN;
  grid->proj_type = grid_type;
  grid->proj_origin_lat = (double) cart->latitude / DEG2LONG;
  grid->proj_origin_lon = (double) cart->longitude / DEG2LONG;
  grid->proj_params.flat.rotation = (double) cart->rotation / DEG2LONG;
  grid->nx = cart->nx;
  grid->ny = cart->ny;
  grid->nz = cart->nz;
  grid->minx = (double) cart->minx / (double) cart->scalex;
  grid->miny = (double) cart->miny / (double) cart->scaley;
  grid->minz = (double) cart->minz / (double) cart->scalez;
  grid->dx = (double) cart->dx / (double) cart->scalex;
  grid->dy = (double) cart->dy / (double) cart->scaley;
  grid->dz = (double) cart->dz / (double) cart->scalez;
  grid->sensor_x = (double) cart->radarx / (double) cart->scalex;
  grid->sensor_y = (double) cart->radary / (double) cart->scaley;
  grid->sensor_z = (double) cart->radarz / (double) cart->scalez;
  grid->dz_constant = cart->dz_constant;
  
  if (grid_type == MDV_PROJ_LATLON) {
    
    grid->sensor_lat = grid->sensor_y;
    grid->sensor_lon = grid->sensor_x;
    
  } else if (grid_type == MDV_PROJ_FLAT) {
    
    if (grid->sensor_x == 0.0 && grid->sensor_y == 0.0) {
      
      grid->sensor_lat = grid->proj_origin_lat;
      grid->sensor_lon = grid->proj_origin_lon;
      
    } else {
      
      range = sqrt(grid->sensor_x * grid->sensor_x +
		   grid->sensor_y * grid->sensor_y);
      
      azimuth = (atan2(grid->sensor_x, grid->sensor_y) +
		 grid->proj_params.flat.rotation * DEG_TO_RAD);

      PJGLatLonPlusRTheta(grid->proj_origin_lat, grid->proj_origin_lon,
			  range, azimuth,
			  &lat, &lon);
      
      grid->sensor_lat = lat;
      grid->sensor_lon = lon;
      
    } /* if (grid->sensor_x == 0.0 ...*/

  } else {

    grid->sensor_lat = 0.0;
    grid->sensor_lon = 0.0;
    
  } /* if (grid_type == MDV_PROJ_LATLON) */
      
  strncpy(grid->unitsx, cart->unitsx, MDV_GRID_UNITS_LEN);
  strncpy(grid->unitsy, cart->unitsy, MDV_GRID_UNITS_LEN);
  strncpy(grid->unitsz, cart->unitsz, MDV_GRID_UNITS_LEN);

  return;

}

