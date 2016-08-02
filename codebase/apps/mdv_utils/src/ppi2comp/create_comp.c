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
 * create_comp()
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1994
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "ppi2comp.h"

/*
 * prototypes
 */

static void init_comp_v_handle(vol_file_handle_t *comp_v_handle, 
			     vol_file_handle_t *trigger_v_handle,
			     si32 npoints, int nfields);

static void set_comp(vol_file_handle_t *comp_v_handle,
		    ppi_t *ppi, ppi2comp_input *input,
		    si32 npoints, int nfields);

static void set_mask(ui08 *mask, cart_params_t *ref_cart,
		     double min_range, double max_range);
     
/*
 * main
 */

void create_comp(int nppis, ppi2comp_input *input, ppi_t *ppi)

{

  static int first_call = TRUE;
  static int nfields;
  static si32 npoints;
  static cart_params_t ref_cart;
  static vol_file_handle_t comp_v_handle;

  int i, ifield;
  int write_current_index;

  if (first_call) {

    /*
     * set reference cart params and nfields
     */

    ref_cart = ppi[0].v_handle.vol_params->cart;
    nfields =  ppi[0].v_handle.vol_params->nfields;

    /*
     * alloc and set the mask grids
     */

    npoints = ref_cart.nx * ref_cart.ny;
    
    for (i = 0; i < nppis; i++) {
      ppi[i].mask = (ui08 *) ucalloc (npoints, sizeof(ui08));
      set_mask(ppi[i].mask, &ref_cart,
	       input[i].min_range, input[i].max_range);
    } /* i */

    init_comp_v_handle(&comp_v_handle, &ppi[0].v_handle,
		     npoints, nfields);
    
    first_call = FALSE;
    
  } 
  
  /*
   * initialize the field planes
   */
  
  for (ifield = 0; ifield < nfields; ifield++) {
    memset((void *) comp_v_handle.field_plane[ifield][0],
	   0, npoints * sizeof(ui08));
  }
  
  for (i = 0; i < nppis; i++) {
    
    /*
     * check the cart params
     */

    if (memcmp((void *) &ref_cart,
	       (void *) &ppi[i].v_handle.vol_params->cart,
	       sizeof(cart_params_t))) {
      fprintf(stderr, "WARNING - %s:creat_comp\n", Glob->prog_name);
      fprintf(stderr, "Cart params have changed.\n");
      fprintf(stderr, "File '%s'\n", ppi[i].file_path);
      return;
    }
    
    /*
     * check the number of fields
     */

    if (nfields != ppi[i].v_handle.vol_params->nfields) {
      fprintf(stderr, "WARNING - %s:creat_comp\n", Glob->prog_name);
      fprintf(stderr, "Number of fields has changed.\n");
      fprintf(stderr, "File '%s'\n", ppi[i].file_path);
      return;
    }

    /*
     * set comp values
     */

    set_comp(&comp_v_handle, &ppi[i], &input[i], npoints, nfields);
    
  } /* i */

  /*
   * write out the comp volume
   */

  comp_v_handle.vol_params->start_time = ppi[0].v_handle.vol_params->start_time;
  comp_v_handle.vol_params->mid_time = ppi[0].v_handle.vol_params->mid_time;
  comp_v_handle.vol_params->end_time = ppi[0].v_handle.vol_params->end_time;
  
  if (Glob->params.mode == REALTIME) {
    write_current_index = TRUE;
  } else {
    write_current_index = FALSE;
  }
    
  if (RfWriteDobson(&comp_v_handle,
		    write_current_index,
		    Glob->params.debug,
		    Glob->params.output_dir,
		    Glob->params.output_file_ext,
		    Glob->prog_name,
		    "create_comp") != R_SUCCESS) {
    fprintf(stderr, "ERROR - %s:create_comp\n", Glob->prog_name);
    fprintf(stderr, "Cannot write output file\n");
  }

  return;

}

static void init_comp_v_handle(vol_file_handle_t *comp_v_handle, 
			     vol_file_handle_t *trigger_v_handle,
			     si32 npoints, int nfields)

{

  int ifield;
  
  RfInitVolFileHandle(comp_v_handle,
		      Glob->prog_name,
		      (char *) NULL,
		      (FILE *) NULL);

  if (RfAllocVolParams(comp_v_handle, "init_comp_v_handle")) {
    tidy_and_exit(-1);
  }
  
  *comp_v_handle->vol_params = *trigger_v_handle->vol_params;

  sprintf(comp_v_handle->vol_params->note,
	  "Composite formed from PPI files.\n");
  
  ustr_concat(comp_v_handle->vol_params->note,
	      trigger_v_handle->vol_params->note,
	      VOL_PARAMS_NOTE_LEN);
  
  comp_v_handle->vol_params->radar.nelevations = 0;
  comp_v_handle->vol_params->cart.nz = 1;
  
  if (RfAllocVolArrays(comp_v_handle, "init_comp_v_handle")) {
    tidy_and_exit(-1);
  }

  /*
   * plane heights
   */
  
  comp_v_handle->plane_heights[0][PLANE_BASE_INDEX] =
    trigger_v_handle->plane_heights[0][PLANE_BASE_INDEX];

  comp_v_handle->plane_heights[0][PLANE_MIDDLE_INDEX] =
    trigger_v_handle->plane_heights[0][PLANE_MIDDLE_INDEX];

  comp_v_handle->plane_heights[0][PLANE_TOP_INDEX] =
    trigger_v_handle->plane_heights[0][PLANE_TOP_INDEX];

  /*
   * field params
   */
  
  for (ifield = 0; ifield < nfields; ifield++) {
    *(comp_v_handle->field_params[ifield]) =
      *(trigger_v_handle->field_params[ifield]);
    ustr_concat(comp_v_handle->field_params[ifield]->name,
		" Comp", R_LABEL_LEN);
  }

  /*
   * field plane
   */

  for (ifield = 0; ifield < nfields; ifield++) {
    comp_v_handle->field_plane[ifield][0] =
      (ui08 *) umalloc (npoints * sizeof(ui08));
  }

  return;
  
}

static void set_comp(vol_file_handle_t *comp_v_handle,
		     ppi_t *ppi, ppi2comp_input *input,
		     si32 npoints, int nfields)

{

  ui08 *in, *comp, *mask;
  int ifield;
  si32 ipoint;

  for (ifield = 0; ifield < nfields; ifield++) {

    in = ppi->v_handle.field_plane[ifield][0];
    comp = comp_v_handle->field_plane[ifield][0];
    mask = ppi->mask;
    
    for (ipoint = 0; ipoint < npoints;
	 ipoint++, in++, comp++, mask++) {

      if (*mask) {
	*comp = MAX(*comp, *in);
      }

    } /* ipoint */

  } /* ifield */

  return;

}

static void set_mask(ui08 *mask, cart_params_t *ref_cart,
		     double min_range, double max_range)
     
{

  int ix, iy;
  double x, y, range;
  cart_float_params_t fl_cart;

  RfDecodeCartParams(ref_cart, &fl_cart);

  y = fl_cart.miny;
  
  for (iy = 0; iy < fl_cart.ny; iy++, y += fl_cart.dy) {

    x = fl_cart.minx;
    
    for (ix = 0; ix < fl_cart.nx; ix++, x += fl_cart.dx, mask++) {

      range = sqrt(x * x + y * y);

      if (range >= min_range && range <= max_range) {
	*mask = 1;
      }

    } /* ix */

  } /* iy */

}

