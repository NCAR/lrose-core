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
 * print_grid.c
 *
 * Print mdv grids
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * Feb 1996
 *
 **************************************************************************/

#include <Mdv/mdv/mdv_grid.h>

#define BOOL_STR(a) (a == 0 ? "false" : "true")

/******************
 * MDV_print_grid()
 *
 * Print mdv grid struct
 */

void  MDV_print_grid(FILE *out, char *spacer, mdv_grid_t *grid)
     
{
  
  fprintf(out, "%sMDV grid parameters\n", spacer);
  fprintf(out, "%s-------------------\n", spacer);
  
  fprintf(out, "%s  nbytes_char : %ld\n",
	  spacer, (long) grid->nbytes_char);
  
  if (grid->proj_type == MDV_PROJ_FLAT) {
    fprintf(out, "%s  gridtype : flat\n", spacer);
  } else if (grid->proj_type == MDV_PROJ_LATLON) {
    fprintf(out, "%s  gridtype : latlon\n", spacer);
  } else {
    fprintf(out, "%s  gridtype : UNKNOWN\n", spacer);
  }

  fprintf(out, "%s  origin latitude : %g\n",
	  spacer, grid->proj_origin_lat);
  fprintf(out, "%s  origin longitude : %g\n",
	  spacer, grid->proj_origin_lon);
  fprintf(out, "%s  grid rotation : %g\n",
	  spacer, grid->proj_params.flat.rotation);

  fprintf(out, "%s  nx, ny, nz : %d, %d, %d\n",
	  spacer,
	  grid->nx, grid->ny, grid->nz);

  fprintf(out, "%s  minx, miny, minz : %g, %g, %g\n",
	  spacer,
	  grid->minx, grid->miny, grid->minz);
  
  fprintf(out, "%s  dx, dy, dz : %g, %g, %g\n", spacer,
	  grid->dx, grid->dy, grid->dz);
  
  fprintf(out, "%s  sensor_x, sensor_y, sensor_z : %g, %g, %g\n",
	  spacer,
	  grid->sensor_x, grid->sensor_y, grid->sensor_z);
  
  fprintf(out, "%s  sensor_lat, sensor_lon : %g, %g\n",
	  spacer,
	  grid->sensor_lat, grid->sensor_lon);
  
  fprintf(out, "%s  dz_constant: %s\n", spacer,
	  BOOL_STR(grid->dz_constant));

  fprintf(out, "%s  x units : %s\n", spacer, grid->unitsx);
  fprintf(out, "%s  y units : %s\n", spacer, grid->unitsy);
  fprintf(out, "%s  z units : %s\n", spacer, grid->unitsz);
  
}

