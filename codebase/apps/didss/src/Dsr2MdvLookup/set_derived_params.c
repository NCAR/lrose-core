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
 * set_derived_params.c
 *
 * Set up derived parameters
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * April 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "Dsr2MdvLookup.h"
#include <toolsa/pjg.h>

#define DEG_TO_RAD 0.01745329251994372
#define TOLERANCE 1.e-8

void set_derived_params(radar_scan_table_t *scan_table)

{

  double cart_to_radar_range, cart_to_radar_theta;
  double radar_to_cart_range, radar_to_cart_theta;

  /*
   * malloc debug level
   */
  
  if (Glob->params.malloc_debug_level > 0) {
    umalloc_debug(Glob->params.malloc_debug_level);
  }

  /*
   * geometry type
   */

  if (Glob->params.geom_type == POLAR_GEOM) {
    Glob->geom = P2MDV_POLAR;
  } else if (Glob->params.geom_type == PPI_GEOM) {
    Glob->geom = P2MDV_PPI;
  } else {
    Glob->geom = P2MDV_CART;
  }
  
  /*
   * radar location on grid
   */
  
  if (Glob->geom == P2MDV_POLAR) {
    
    Glob->radarx = 0.0;
    Glob->radary = 0.0;
    Glob->radarz = Glob->params.radar_location.altitude;
    Glob->rotation_at_radar = Glob->params.output_grid.rotation;

  } else {

    /*
     * compute the range and theta from the cart grid origin
     * to the radar, and vice-versa
     */

    PJGLatLon2RTheta(Glob->params.output_grid.origin_lat,
		     Glob->params.output_grid.origin_lon,
		     Glob->params.radar_location.latitude,
		     Glob->params.radar_location.longitude,
		     &cart_to_radar_range,
		     &cart_to_radar_theta);
    
    PJGLatLon2RTheta(Glob->params.radar_location.latitude,
		     Glob->params.radar_location.longitude,
		     Glob->params.output_grid.origin_lat,
		     Glob->params.output_grid.origin_lon,
		     &radar_to_cart_range,
		     &radar_to_cart_theta);
    
    /*
     * compute the radar posn on the cart grid
     */

    Glob->radarz = Glob->params.radar_location.altitude;

    if (fabs(radar_to_cart_range) < TOLERANCE) {
      
      Glob->radarx = 0.0;
      Glob->radary = 0.0;
      Glob->rotation_at_radar = Glob->params.output_grid.rotation;;

    } else {
      
      Glob->radarx = cart_to_radar_range *
	sin((cart_to_radar_theta - Glob->params.output_grid.rotation) *
	    DEG_TO_RAD);
      
      Glob->radary = cart_to_radar_range *
	cos((cart_to_radar_theta - Glob->params.output_grid.rotation) *
	    DEG_TO_RAD);
      
      Glob->rotation_at_radar =
	(Glob->params.output_grid.rotation + radar_to_cart_theta -
	 cart_to_radar_theta - 180.0);
      
    } /* if (fabs(radar_to_cart_range) < TOLERANCE) */
    
    if (Glob->rotation_at_radar < -180.0)
      Glob->rotation_at_radar += 360.0;

    printf("radarx = %g\n", Glob->radarx);
    printf("radary = %g\n", Glob->radary);
    printf("rotation_at_radar = %g\n", Glob->rotation_at_radar);

  } /* if (Glob->geom == P2MDV_POLAR) */

  /*
   * Override grid params based on geometry
   */

  if (Glob->geom == P2MDV_PPI || Glob->geom == P2MDV_POLAR) {
    
    Glob->nz = scan_table->nelevations;
    Glob->minz = 0.0;
    Glob->dz = 1.0;

  } else {

    Glob->nz = Glob->params.output_grid.nz;
    Glob->minz = Glob->params.output_grid.minz;
    Glob->dz = Glob->params.output_grid.dz;

  }

  if (Glob->geom == P2MDV_POLAR) {

    if (scan_table->use_azimuth_table) {
      fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
      fprintf(stderr, "Cannot use scan table in Polar Mode\n");
      tidy_and_exit(-1);
    }
    
    Glob->ny = scan_table->nazimuths;
    Glob->miny = scan_table->start_azimuth;
    Glob->dy = scan_table->delta_azimuth;

    Glob->nx = scan_table->ngates;
    Glob->minx = scan_table->start_range;
    Glob->dx = scan_table->gate_spacing;
    
  } else {

    Glob->ny = Glob->params.output_grid.ny;
    Glob->miny = Glob->params.output_grid.miny;
    Glob->dy = Glob->params.output_grid.dy;

    Glob->nx = Glob->params.output_grid.nx;
    Glob->minx = Glob->params.output_grid.minx;
    Glob->dx = Glob->params.output_grid.dx;

  }

}
