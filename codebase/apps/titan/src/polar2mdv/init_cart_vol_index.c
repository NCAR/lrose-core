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
/**************************************************************************
 * init_cart_vol_index.c
 *
 * initialize the file handle for a volume in cart coords
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * December 1991
 *
 **************************************************************************/

#include "polar2mdv.h"

void init_cart_vol_index(vol_file_handle_t *v_handle,
			 rc_table_file_handle_t *rc_handle,
			 clutter_table_file_handle_t *clutter_handle)

{

  char notebuf[VOL_PARAMS_NOTE_LEN];
  ui08 *cdata;

  long ielev, ifield, iplane;
  si32 nelev;
  si32 nfields;
  si32 ncdata, nplanes;
  si32 npoints_plane;
  si32 field_pos;

  double dbz_margin;

  radar_params_t *rparams;
  field_params_t *fparams;
  cart_params_t *cart;

  rdata_shmem_header_t *shmem_header = get_shmem_header();
  field_params_t *field_params = get_field_params();

  /*
   * initialize volume file handle
   */
  
  RfInitVolFileHandle(v_handle,
		      Glob->prog_name,
		      (char *) NULL,
		      (FILE *) NULL);
  
  /*
   * allocate vol params
   */
  
  if (RfAllocVolParams(v_handle, "init_cart_vol_index") != R_SUCCESS)
    tidy_and_exit(-1);
  
  /*
   * amend note
   */
  
  memset ((void *)  v_handle->vol_params->note,
          (int) 0, (size_t) VOL_PARAMS_NOTE_LEN);
  
  if (Glob->remove_clutter) {
    
    dbz_margin = (double) clutter_handle->table_params->dbz_margin /
      (double) clutter_handle->table_params->factor;
    
    sprintf(notebuf, "%s%s%g%s%s%g%s%s%s%s%s",
	    shmem_header->note,
	    "\nSignal/noise threshold = ", Glob->sn_threshold,
	    " db. \nCartesian conversion.",
	    "\nClutter removal, dbz margin = ", dbz_margin,
	    " db. \nClutter map from ",
	    utimstr(clutter_handle->table_params->start_time),
	    " to ",
	    utimstr(clutter_handle->table_params->end_time),
	    ".");
    
  }  else {
    
    sprintf(notebuf, "%s%s%g%s%s",
	    shmem_header->note,
	    "\nSignal/noise threshold = ",
	    Glob->sn_threshold, " db.\n",
	    "Cartesian conversion.\n");
    
  }
  
  strncpy(v_handle->vol_params->note,
	  notebuf, (int) VOL_PARAMS_NOTE_LEN);

  /*
   * copy in radar params
   */

  rparams = &shmem_header->radar;
  nelev  = rparams->nelevations;

  memcpy ((void *) &v_handle->vol_params->radar,
          (void *) rparams,
          (size_t) sizeof(radar_params_t));

  /*
   * copy in the cartesian params from the lookup table
   */

  cart = &rc_handle->table_params->cart;

  memcpy ((void *) &v_handle->vol_params->cart,
          (void *) cart,
          (size_t) sizeof(cart_params_t));
  
  nplanes = cart->nz;

  nfields = Glob->nfields_processed;
  v_handle->vol_params->nfields = nfields;
  
  /*
   * allocate vol file handle arrays
   */
  
  if (RfAllocVolArrays(v_handle, "init_cart_vol_index") != R_SUCCESS)
    tidy_and_exit(-1);
  
  /*
   * copy radar elevation angles
   */
  
  for (ielev = 0; ielev < nelev; ielev++)
    v_handle->radar_elevations[ielev] =
      (si32) (rc_handle->scan_table->elev_angles[ielev] * DEG_FACTOR + 0.5);

  /*
   * copy in field params
   *
   */
  
  for (ifield = 0; ifield < Glob->nfields_processed; ifield++) {

    field_pos = Glob->field_positions[ifield];
    fparams = v_handle->field_params[ifield];
    *fparams = field_params[field_pos];
    fparams->encoded = Glob->run_length_encode;
    fparams->missing_val = MISSING_DATA_VAL;

  } /* ifield */
  
  /*
   * copy plane heights from radar-to-cart table
   */
  
  memcpy ((void *) *v_handle->plane_heights,
          (void *) *rc_handle->plane_heights,
          (size_t) (nplanes * N_PLANE_HEIGHT_VALUES * sizeof(si32)));

  /*
   * set up cdata array and point field plane array to it
   */

  npoints_plane = cart->nx * cart->ny;
  ncdata = npoints_plane * cart->nz;

  cdata = (ui08 *) ucalloc
    ((ui32) (nfields * ncdata), (ui32) sizeof(ui08));
  
  for (ifield = 0; ifield < nfields; ifield++) {

    for (iplane = 0; iplane < nplanes; iplane++) {
      v_handle->field_plane[ifield][iplane] = cdata;
      cdata += npoints_plane;
    } /* iplane */

  } /* ifield */
  
}
