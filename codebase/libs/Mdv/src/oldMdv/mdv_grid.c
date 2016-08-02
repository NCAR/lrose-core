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
/*************************************************************************
 *
 * mdv_grid.c
 *
 * mdv grid manipulation routines
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1998
 *
 **************************************************************************/

#include <Mdv/mdv/mdv_grid.h>
#include <Mdv/mdv/mdv_print.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/pjg_flat.h>
#include <string.h>

/***************************
 * MDV_load_grid_from_hdrs()
 *
 * Load up mdv_grid_t from
 * MDV_master_header_t and MDV_field_header_t
 */

void MDV_load_grid_from_hdrs(MDV_master_header_t *mhdr,
			     MDV_field_header_t *fhdr,
			     mdv_grid_t *grid)

{

  grid->proj_origin_lat = fhdr->proj_origin_lat;
  grid->proj_origin_lon = fhdr->proj_origin_lon;

  grid->proj_type = fhdr->proj_type;
  if (grid->proj_type == MDV_PROJ_FLAT) {
    grid->proj_params.flat.rotation = fhdr->proj_rotation;
  }
  else if (grid->proj_type == MDV_PROJ_LAMBERT_CONF) {
    grid->proj_params.lc2.lat1 = fhdr->proj_param[0];
    grid->proj_params.lc2.lat2 = fhdr->proj_param[1];
  }
  
  grid->minx = fhdr->grid_minx;
  grid->miny = fhdr->grid_miny;
  grid->minz = fhdr->grid_minz;

  grid->dx = fhdr->grid_dx;
  grid->dy = fhdr->grid_dy;
  grid->dz = fhdr->grid_dz;

  grid->nx = fhdr->nx;
  grid->ny = fhdr->ny;
  grid->nz = fhdr->nz;


  grid->sensor_lon = mhdr->sensor_lon;
  grid->sensor_lat = mhdr->sensor_lat;
  grid->sensor_z = mhdr->sensor_alt;

  if (grid->proj_type == MDV_PROJ_FLAT) {
    mdv_grid_comps_t comps;
    double xx, yy;
    MDV_init_flat(grid->proj_origin_lat,
		  grid->proj_origin_lon,
		  grid->proj_params.flat.rotation,
		  &comps);
    MDV_latlon2xy(&comps, grid->sensor_lat, grid->sensor_lon, &xx, &yy);
    grid->sensor_x = xx;
    grid->sensor_y = yy;
  } else if (grid->proj_type == MDV_PROJ_LATLON) {
    grid->sensor_x = grid->sensor_lon;
    grid->sensor_y = grid->sensor_lat;
  } else {
    grid->sensor_x = 0.0;
    grid->sensor_y = 0.0;
  }

  if (mhdr->vlevel_type == MDV_VERT_TYPE_Z) {
    grid->dz_constant = 1;
  } else {
    grid->dz_constant = 0;
  }

  grid->nbytes_char = MDV_N_GRID_LABELS * MDV_GRID_UNITS_LEN;
  
  if (grid->proj_type == MDV_PROJ_FLAT) {
    strcpy(grid->unitsx, "km");
    strcpy(grid->unitsy, "km");
  } else if (grid->proj_type == MDV_PROJ_LATLON) {
    strcpy(grid->unitsx, "deg");
    strcpy(grid->unitsy, "deg");
  } else {
    strcpy(grid->unitsx, "unknown");
    strcpy(grid->unitsy, "unknown");
  }

  if (mhdr->vlevel_type == MDV_VERT_TYPE_SURFACE) {
    strcpy(grid->unitsz, "");
  } else if (mhdr->vlevel_type == MDV_VERT_TYPE_SIGMA_P) {
    strcpy(grid->unitsz, "sigma_p");
  } else if (mhdr->vlevel_type == MDV_VERT_TYPE_PRESSURE) {
    strcpy(grid->unitsz, "mb");
  } else if (mhdr->vlevel_type == MDV_VERT_TYPE_Z) {
    strcpy(grid->unitsz, "km");
  } else if (mhdr->vlevel_type == MDV_VERT_TYPE_SIGMA_Z) {
    strcpy(grid->unitsz, "sigma_z");
  } else if (mhdr->vlevel_type == MDV_VERT_TYPE_ETA) {
    strcpy(grid->unitsz, "eta");
  } else if (mhdr->vlevel_type == MDV_VERT_TYPE_THETA) {
    strcpy(grid->unitsz, "K");
  } else if (mhdr->vlevel_type == MDV_VERT_TYPE_ELEV) {
    strcpy(grid->unitsz, "deg");
  } else if (mhdr->vlevel_type == MDV_VERT_VARIABLE_ELEV) {
    strcpy(grid->unitsz, "var_elev");
  } else if (mhdr->vlevel_type == MDV_VERT_FIELDS_VAR_ELEV) {
    strcpy(grid->unitsz, "var_elev");
  } else if (mhdr->vlevel_type == MDV_VERT_FLIGHT_LEVEL) {
    strcpy(grid->unitsz, "FL");
  } else {
    strcpy(grid->unitsz, "");
  }

  return;

}

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

/***************************
 * MDV_load_hdrs_from_grid()
 *
 * Load up MDV_master_header_t and MDV_field_header_t
 * from mdv_grid_t
 */

void MDV_load_hdrs_from_grid(mdv_grid_t *grid,
			     MDV_master_header_t *mhdr,
			     MDV_field_header_t *fhdr)

{

  fhdr->proj_origin_lat = grid->proj_origin_lat;
  fhdr->proj_origin_lon = grid->proj_origin_lon;

  fhdr->proj_type = grid->proj_type;
  if (grid->proj_type == MDV_PROJ_FLAT) {
    fhdr->proj_rotation = grid->proj_params.flat.rotation;
  }
  
  fhdr->grid_minx = grid->minx;
  fhdr->grid_miny = grid->miny;
  fhdr->grid_minz = grid->minz;

  fhdr->grid_dx = grid->dx;
  fhdr->grid_dy = grid->dy;
  fhdr->grid_dz = grid->dz;

  fhdr->nx = grid->nx;
  fhdr->ny = grid->ny;
  fhdr->nz = grid->nz;

  mhdr->max_nx = MAX(mhdr->max_nx, fhdr->nx);
  mhdr->max_ny = MAX(mhdr->max_nx, fhdr->ny);
  mhdr->max_nz = MAX(mhdr->max_nx, fhdr->nz);

  mhdr->sensor_lon = grid->sensor_lon;
  mhdr->sensor_lat = grid->sensor_lat;
  mhdr->sensor_alt = grid->sensor_z;

  return;

}


/***************************
 * MDV_latlon2index_xy()
 *
 * Returns the data x, y indices for the given lat/lon location.
 * Returns -1 and prints an error message on error.  The current
 # errors are:
 *    - data is compressed
 *    - projection not yet handled
 *
 * Currently only handles the MDV_PROJ_LATLON projection.
 */

int MDV_latlon2index_xy(MDV_master_header_t *master_hdr,
			MDV_field_header_t *field_hdr,
			double lat, double lon,
			int *x_index, int *y_index)
{
  /*
   * Check the MDV projection so we can determine how to find
   * the lat/lon location in the data
   */

  switch (field_hdr->proj_type)
  {
  case MDV_PROJ_LATLON :
    *x_index = (int)((lon - field_hdr->grid_minx) / field_hdr->grid_dx + 0.5);
    *y_index = (int)((lat - field_hdr->grid_miny) / field_hdr->grid_dy + 0.5);
    
    break;
    
  case MDV_PROJ_FLAT :
  {
    double dx, dy;
    
    PJGLatLon2DxDy(field_hdr->proj_origin_lat, field_hdr->proj_origin_lon,
		   lat, lon,
		   &dx, &dy);
    
    *x_index = (int)((dx - field_hdr->grid_minx) / field_hdr->grid_dx + 0.5);
    *y_index = (int)((dy - field_hdr->grid_miny) / field_hdr->grid_dy + 0.5);
    
    break;
  }
  
  default:
    fprintf(stderr,
	    "ERROR: MDV_latlon2index_xy() does not yet handle %s projection data\n",
	    MDV_proj2string(field_hdr->proj_type));
    
    *x_index = -1;
    *y_index = -1;
    
    return(-1);
    
  } /* endswitch - field_hdr->proj_type */
  
  if (*x_index < 0)
    *x_index = 0;
  
  if (*x_index >= field_hdr->nx)
    *x_index = field_hdr->nx - 1;
  
  if (*y_index < 0)
    *y_index = 0;
  
  if (*y_index >= field_hdr->ny)
    *y_index = field_hdr->ny - 1;
  
  return(0);
}


/***************************
 * MDV_latlon2index()
 *
 * Returns the data index for the given lat/lon location.
 * Returns -1 and prints an error message on error.  The
 * current errors are:
 *    - data is compressed
 *    - projection not yet handled
 *    - location outside of grid (no error message for this one)
 *
 * Currently only handles the MDV_PROJ_LATLON projection.
 */

int MDV_latlon2index(MDV_master_header_t *master_hdr,
		     MDV_field_header_t *field_hdr,
		     double lat, double lon)
{
  int x_index, y_index;
  
  if (MDV_latlon2index_xy(master_hdr, field_hdr,
			  lat, lon,
			  &x_index, &y_index) < 0)
    return(-1);

  if (x_index < 0 || x_index >= field_hdr->nx ||
      y_index < 0 || y_index >= field_hdr->ny)
    return(-1);
    
  return((field_hdr->nx * y_index) + x_index);
}
