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
 * check_geom.c: check that the incoming packets have the same
 *               geometry as that used to create the
 *               slave lookup table
 *
 * RAP, NCAR, Boulder CO
 *
 * january 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "cart_slave.h"

static void print_cart_params(cart_params_t *cart);

void check_geom(void)

{

  int error_flag = FALSE;
  int i;

  si32 *shmem_elevations;
  double *slave_elevations;

  radar_params_t *rparams;
  rc_table_params_t *slave_params;

  rparams = &Glob->shmem_header->vol_params.radar;
  slave_params = Glob->slave_handle.table_params;
  
  if(rparams->altitude != slave_params->cart.radarz) {
    
    fprintf(stderr, "\nERROR - %s:check_geom.\n", Glob->prog_name);
    fprintf(stderr, "Radar altitude does not match.\n");
    fprintf(stderr, "Data radar altitude = %d\n",
	    rparams->altitude);
    fprintf(stderr, "Radar to cart. table radar altitude = %d\n",
	    slave_params->cart.radarz);
    error_flag = TRUE;
    
  }
  
  if(rparams->nazimuths != slave_params->nazimuths) {
    
    fprintf(stderr, "\nERROR - %s:check_geom.\n", Glob->prog_name);
    fprintf(stderr, "No of azimuths do not match.\n");
    fprintf(stderr, "Radar volume nazimuths = %d\n",
	    rparams->nazimuths);
    fprintf(stderr, "Radar to cart. table nazimuths = %d\n",
	    slave_params->nazimuths);
    error_flag = TRUE;
    
  }
  
  if(rparams->nelevations != slave_params->nelevations) {
    
    fprintf(stderr, "\nERROR - %s:check_geom.\n", Glob->prog_name);
    fprintf(stderr, "No of elevations do not match.\n");
    fprintf(stderr, "Radar volume nelevations = %d\n",
	    rparams->nelevations);
    fprintf(stderr, "Radar to cart. table nelevations = %d\n",
	    slave_params->nelevations);
    error_flag = TRUE;
    
  }
  
  if(rparams->ngates != slave_params->ngates) {
    
    fprintf(stderr, "\nERROR - %s:check_geom.\n", Glob->prog_name);
    fprintf(stderr, "No of gates do not match.\n");
    fprintf(stderr, "Radar volume ngates = %d\n",
	    rparams->ngates);
    fprintf(stderr, "Radar to cart. table ngates = %d\n",
	    slave_params->ngates);
    error_flag = TRUE;
    
  }
  
  if(rparams->gate_spacing != slave_params->gate_spacing) {
    
    fprintf(stderr, "\nERROR - %s:check_geom.\n", Glob->prog_name);
    fprintf(stderr, "Gate spacing does not match.\n");
    fprintf(stderr, "Radar volume gate_spacing = %g\n",
	    rparams->gate_spacing / 1000.0);
    fprintf(stderr, "Radar to cart. table gate_spacing = %g\n",
	    slave_params->gate_spacing / 1000.0);
    error_flag = TRUE;
    
  }
  
  if(rparams->start_range != slave_params->start_range) {
    
    fprintf(stderr, "\nERROR - %s:check_geom.\n", Glob->prog_name);
    fprintf(stderr, "Start range does not match.\n");
    fprintf(stderr, "Radar volume start_range = %g\n",
	    rparams->start_range / 1000.0);
    fprintf(stderr, "Radar to cart. table start_range = %g\n",
	    slave_params->start_range / 1000.0);
    error_flag = TRUE;
    
  }
  
  if(rparams->start_azimuth != slave_params->start_azimuth) {
    
    fprintf(stderr, "\nERROR - %s:check_geom.\n", Glob->prog_name);
    fprintf(stderr, "Start azimuth does not match.\n");
    fprintf(stderr, "Radar volume start_azimuth = %g\n",
	    (double) rparams->start_azimuth / DEG_FACTOR);
    fprintf(stderr, "Radar to cart. table start_azimuth = %g\n",
	    (double) slave_params->start_azimuth / DEG_FACTOR);
    error_flag = TRUE;
    
  }
  
  if(rparams->delta_azimuth != slave_params->delta_azimuth) {
    
    fprintf(stderr, "\nERROR - %s:check_geom.\n", Glob->prog_name);
    fprintf(stderr, "Delta does not match.\n");
    fprintf(stderr, "Radar volume delta_azimuth = %g\n",
	    (double) rparams->delta_azimuth / DEG_FACTOR);
    fprintf(stderr, "Radar to cart. table delta_azimuth = %g\n",
	    (double) slave_params->delta_azimuth / DEG_FACTOR);
    error_flag = TRUE;
    
  }
  
  if(rparams->beam_width != slave_params->beam_width) {
    
    fprintf(stderr, "\nERROR - %s:check_geom.\n", Glob->prog_name);
    fprintf(stderr, "Beam width does not match.\n");
    fprintf(stderr, "Radar volume beam_width = %g\n",
	    (double) rparams->beam_width / DEG_FACTOR);
    fprintf(stderr, "Radar to cart. table beam_width = %g\n",
	    (double) slave_params->beam_width / DEG_FACTOR);
    error_flag = TRUE;
    
  }
  
  /*
   * check elevations
   */

  shmem_elevations = (si32 *)
    ((char *) Glob->shmem_aux_header +
     Glob->shmem_header->radar_elevations_offset);
  
  slave_elevations = Glob->slave_handle.scan_table->elev_angles;

  if (rparams->nelevations == slave_params->nelevations) {
    
    for (i = 0; i < rparams->nelevations; i++) {
      
      if(shmem_elevations[i] / DEG_FACTOR != slave_elevations[i]) {
	
	fprintf(stderr, "\nERROR - %s:check_geom.\n", Glob->prog_name);
	fprintf(stderr, "Elevation %d does not match.\n", i);
	fprintf(stderr, "Incoming data radar elevation[%d] = %g\n", i,
		(double) shmem_elevations[i] / DEG_FACTOR);
	fprintf(stderr, "Radar to cart. slave table elevation[%d] = %g\n", i,
		slave_elevations[i]);
	error_flag = TRUE;
	
      } /* if */
      
    } /* i */
    
  } /* if */
  
  /*
   * check that the cartesian parameters are the same for the
   * slave table and the incoming data
   */
  
  if (memcmp((void *) &slave_params->cart,
	     (void *) &Glob->shmem_header->vol_params.cart,
	     (size_t) sizeof(cart_params_t))) {

    fprintf(stderr, "\nERROR - %s:check_geom.\n", Glob->prog_name);
    fprintf(stderr, "Incoming data cartesian params do not match\n");
    fprintf(stderr, "the slave table params.\n\n");

    fprintf(stderr, "Incoming data cartesian params:\n");
    
    print_cart_params(&Glob->shmem_header->vol_params.cart);

    fprintf(stderr, "Slave radar-to-cart table '%s'\n\n",
	    Glob->slave_table_path);
    fprintf(stderr, "Cartesian params:\n");

    print_cart_params(&slave_params->cart);
    
    error_flag = TRUE;
    
  }
  
  if (error_flag == TRUE)
    tidy_and_exit(-1);
  
}


/***********************************************************************
 * print_cart_params()
 *
 ***********************************************************************/

static void print_cart_params(cart_params_t *cart)

{

  fprintf(stderr, "latitude : %g\n",
	  (double) cart->latitude / DEG_FACTOR);
  
  fprintf(stderr, "longitude : %g\n",
	  (double) cart->longitude / DEG_FACTOR);
  
  fprintf(stderr, "rotation : %g\n",
	  (double) cart->rotation / DEG_FACTOR);
  
  fprintf(stderr, "nx, ny, nz : %ld, %ld, %ld\n",
	  (long) cart->nx, (long) cart->ny, (long) cart->nz);

  fprintf(stderr, "minx, miny, minz : %g, %g, %g\n",
	  (double) cart->minx / (double) cart->scalex,
	  (double) cart->miny / (double) cart->scaley,
	  (double) cart->minz / (double) cart->scalez);

  fprintf(stderr, "dx, dy, dz : %g, %g, %g\n",
	  (double) cart->dx / (double) cart->scalex,
	  (double) cart->dy / (double) cart->scaley,
	  (double) cart->dz / (double) cart->scalez);

  fprintf(stderr, "radarx, radary, radarz : %g, %g, %g\n",
	  (double) cart->radarx / (double) cart->scalex,
	  (double) cart->radary / (double) cart->scaley,
	  (double) cart->radarz / (double) cart->scalez);

  fprintf(stderr, "x units : %s\n", cart->unitsx);

  fprintf(stderr, "y units : %s\n", cart->unitsy);

  fprintf(stderr, "z units : %s\n\n", cart->unitsz);
  
}
