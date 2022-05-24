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
 * setup_cart_volume.c
 *
 * set up the volume file handle for the cartesian volume
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include <string.h>
#include "cart_convert.h"

void setup_cart_volume(vol_file_handle_t *rv_handle,
		       vol_file_handle_t *cv_handle,
		       rc_table_file_handle_t *rc_handle)

{

  ui08 *cdata;
  char note[80];

  si32 nfields;
  si32 nplanes, npoints_per_plane;
  si32 ifield, iplane;

  /*
   * allocate vol params
   */
  
  if (RfAllocVolParams(cv_handle, "setup_cart_volume") != R_SUCCESS)
    exit(1);
  
  /*
   * copy over the vol params from the radar coords vol file
   */

  memcpy ((void *)  cv_handle->vol_params,
          (void *)  rv_handle->vol_params,
          (size_t) 
	sizeof(vol_params_t));

  /*
   * append to the note
   */

  sprintf(note,
	  "Converted to cartesian coords using prog '%s'.\n",
	  Glob->prog_name);

  strncat(cv_handle->vol_params->note, note, 
	  VOL_PARAMS_NOTE_LEN - strlen(cv_handle->vol_params->note) - 1);

  /*
   * copy in cart params from rc table index
   */

  memcpy ((void *)  &cv_handle->vol_params->cart,
          (void *)  &rc_handle->table_params->cart,
          (size_t) 
	sizeof(cart_params_t));

  /*
   * set constants
   */

  nfields = cv_handle->vol_params->nfields;
  nplanes = cv_handle->vol_params->cart.nz;
  npoints_per_plane =
    cv_handle->vol_params->cart.nx * cv_handle->vol_params->cart.ny;

  /*
   * allocate vol index arrays
   */

  if (RfAllocVolArrays(cv_handle, "setup_cart_volume") != R_SUCCESS)
    exit(1);
  
  /*
   * copy over field params
   */

  memcpy ((void *)  *cv_handle->field_params,
          (void *)  *rv_handle->field_params,
          (size_t)  (nfields * sizeof(field_params_t)));
  
  /*
   * copy elevation array
   */

  memcpy ((void *)  cv_handle->radar_elevations,
          (void *)  rv_handle->radar_elevations,
          (size_t)  (cv_handle->vol_params->radar.nelevations * sizeof(si32)));

  /*
   * copy plane_heights array
   */

  memcpy ((void *)  *cv_handle->plane_heights,
          (void *)  *rc_handle->plane_heights,
          (size_t)  (nplanes * N_PLANE_HEIGHT_VALUES * sizeof(si32)));
  /*
   * allocate memory for field data
   */
  
  cdata = (ui08 *) ucalloc
    ((ui32) (npoints_per_plane * nplanes * nfields),
     (ui32) sizeof(ui08));

  for (ifield = 0; ifield < nfields; ifield++) {

    for (iplane = 0; iplane < nplanes; iplane++) {
      
      cv_handle->field_plane[ifield][iplane] = cdata;

      cdata += npoints_per_plane;

    } /* iplane */

  } /* ifield */
      
}
