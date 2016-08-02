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
 * perform_integration()
 *
 * Integrate over height for each grid point.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1993
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "mdv2vil.h"
#include <physics/vil.h>

void perform_integration(vol_file_handle_t *v_handle)

{

  ui08 *data_plane;
  
  long iz, ipoint;
  si32 field_num;
  si32 npoints;

  double *filtered, *fil_p;
  double scale, bias;
  double dbz_scale, dbz_bias;
  double dbz;
  double z_scale;
  double top_ht, base_ht, dht;

  cart_params_t *cart;
  field_params_t *fparams;

  double minVil,maxVil;

  PMU_auto_register("In perform_integration");

  /*
   * initialize and allocate
   */

  cart = &v_handle->vol_params->cart;
  npoints = cart->ny * cart->nx;
  z_scale = (double) cart->km_scalez;
  filtered = (double *) umalloc ((ui32) npoints * sizeof(double));

  if ( filtered == NULL )
  {
       fprintf(stderr,"Alloc failed in perform_integration for filtered\n");
       tidy_and_exit(-1);
  }
  /*
   * set dbz scale and bias
   */

  fparams = v_handle->field_params[Glob->params.dbz_field];
  dbz_scale = (double) fparams->scale / (double) fparams->factor;
  dbz_bias = (double) fparams->bias / (double) fparams->factor;

  /*
   * loop through fields
   */

  field_num = Glob->params.dbz_field;

  /*
   * clear filtered array
   */
  
  memset((void *) filtered, (int) 0,
	 (int) (npoints * sizeof(double)));
	 
  /*
   * loop through points
   */
  
  minVil =  9999999.0;
  maxVil = -9999999.0;
  for (ipoint = 0; ipoint < npoints; ipoint++) {
    
    fil_p = filtered + ipoint;
    vil_init();

    /*
     * loop through planes
     */
    
    for (iz = 0; iz <  cart->nz; iz++) {
    
      data_plane = v_handle->field_plane[field_num][iz];

      base_ht =
	(double )(v_handle->plane_heights[iz][PLANE_BASE_INDEX]) / z_scale; 

      top_ht =
	(double )(v_handle->plane_heights[iz][PLANE_TOP_INDEX]) / z_scale;

      dht = ( top_ht - base_ht );

      dbz = (double) (data_plane[ipoint] * dbz_scale + dbz_bias);
      
      vil_add(dbz, dht);

    } /* iz */

    *fil_p = vil_compute();

    minVil = MIN(*fil_p, minVil);
    maxVil = MAX(*fil_p, maxVil);

  } /* ipoint */
  
  /*
   * get max and min of filtered array
   */

  /*
   * find scale and bias
   */

  scale = ( maxVil - minVil ) / 250.0;
  bias =  minVil;
  
  /*
   * copy the filtered array into lowest plane of first field
   */
  
  data_plane = v_handle->field_plane[0][0];

  for (ipoint = 0; ipoint < npoints; ipoint++) {
    
    data_plane[ipoint] =
      (ui08) ((filtered[ipoint] - bias) / scale + 0.5);
    
  } /* ipoint */

  /*
   * set field params for first field
   */

  fparams = v_handle->field_params[0];

  fparams->scale = (si32) (scale * fparams->factor + 0.5);
  fparams->bias = (si32) (bias * fparams->factor + 0.5);
  fparams->noise = 0;
  strncpy(fparams->transform, "none", R_LABEL_LEN);
  strncpy(fparams->name, "vil", R_LABEL_LEN);
  strncpy(fparams->units, "Kg/m2", R_LABEL_LEN);

  /*
   * free resources
   */

  ufree((char *) filtered);

  return;

}






