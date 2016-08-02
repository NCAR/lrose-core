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
 * check_geom.c: check that the clutter data has the same geometry as
 *               that used to create the lookup table
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1990
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "clutter_table_generate.h"

static void print_cart_params(cart_params_t *cart);

void check_geom(vol_file_handle_t *clut_vol_index,
		rc_table_file_handle_t *rc_handle)

{
  
  /*
   * check that the cartesian parameters are the same for the
   * clutter data file and the radar-to-cart file
   */

  if (memcmp((void *) &clut_vol_index->vol_params->cart,
	     (void *) &rc_handle->table_params->cart,
	     (size_t) sizeof(cart_params_t))) {

    fprintf(stderr, "\nERROR - %s:check_geom.\n", Glob->prog_name);
    fprintf(stderr, "Clutter file cartesian params do not match\n");
    fprintf(stderr, "the radar-to-cart table params.\n\n");

    fprintf(stderr, "Clutter file '%s'\n\n", Glob->clutter_file_path);
    fprintf(stderr, "Cartesian params:\n");

    print_cart_params(&clut_vol_index->vol_params->cart);

    fprintf(stderr, "Radar-to-cart table '%s'\n\n", Glob->rc_table_path);
    fprintf(stderr, "Cartesian params:\n");

    print_cart_params(&rc_handle->table_params->cart);
    
    exit(1);
    
  } /* if (memcmp ... */

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
