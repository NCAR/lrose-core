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
 * read_dobson()
 *
 * reads in the radar data, sorts out allocation of the arrays
 * whose size depends on the cartesian grid
 *
 * Returns 0 on success, -1 on failure
 *
 * RAP, NCAR, Boulder CO
 *
 * january 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "storm_ident.h"
#include <math.h>

int read_dobson(vol_file_handle_t *v_handle,
		date_time_t *data_time)

{
  
  si32 i;

  static double dbz_scale_prev = -1.0;
  static double dbz_bias_prev = -1.0;
  double dbz_scale, dbz_bias, dbz;
  cart_float_params_t fl_cart;
  cart_params_t *cart;

  v_handle->vol_file_path = (char *)
    umalloc ((ui32) (strlen(Glob->params.rdata_dir) +
		      2 * strlen(PATH_DELIM) +
		      strlen(Glob->params.rdata_file_ext) + 16));

  sprintf(v_handle->vol_file_path,
	  "%s%s%.4d%.2d%.2d%s%.2d%.2d%.2d.%s",
	  Glob->params.rdata_dir, PATH_DELIM,
	  data_time->year, data_time->month, data_time->day,
	  PATH_DELIM,
	  data_time->hour, data_time->min, data_time->sec,
	  Glob->params.rdata_file_ext);

  fprintf(stdout, "%s: processing file %s\n",
	  Glob->prog_name, v_handle->vol_file_path);
  fflush(stdout);

  /*
   * read in volume
   */

  if (RfReadVolume(v_handle, "read_dobson")) {
    if (Glob->params.debug >= DEBUG_NORM) {
      fprintf(stderr, "WARNING ONLY - %s:read_dobson.\n", Glob->prog_name);
      fprintf(stderr, "Reading in vol params.\n");
    }
    return(-1);
  }

  ufree((char *) v_handle->vol_file_path);
  v_handle->vol_file_path = NULL;

  /*
   * make sure dz is not zero - if it is, set to 1.0
   */

  cart = &v_handle->vol_params->cart;
  if (cart->dz <= 0) {
    cart->dz = 1 * cart->scalez;
  }

  /*
   * set cartesian coords and deltas
   */

  RfDecodeCartParams(cart, &fl_cart);

  Glob->nx = cart->nx;
  Glob->ny = cart->ny;
  Glob->nz = cart->nz;

  Glob->delta_x = fl_cart.dx;
  Glob->delta_y = fl_cart.dy;
  Glob->delta_z = fl_cart.dz;

  Glob->min_x = fl_cart.minx;
  Glob->min_y = fl_cart.miny;
  Glob->min_z = fl_cart.minz;

  Glob->radar_x = fl_cart.radarx;
  Glob->radar_y = fl_cart.radary;
  Glob->radar_z = fl_cart.radarz;
  
  /*
   * set number of fields
   */

  Glob->nfields = v_handle->vol_params->nfields;
  
  /*
   * set min cartesian layer at which data is considered valid
   */
  
  Glob->min_valid_layer =
    (si32) ((Glob->params.base_threshold - Glob->min_z) / Glob->delta_z + 0.5);
  
  if(Glob->min_valid_layer < 0) {
    Glob->min_valid_layer = 0;
  } else if(Glob->min_valid_layer > Glob->nz - 1) {
    Glob->min_valid_layer = Glob->nz - 1;
  }

  /*
   * set max cartesian layer at which data is considered valid
   */
  
  Glob->max_valid_layer =
    (si32) ((Glob->params.top_threshold - Glob->min_z) / Glob->delta_z + 0.5);
  
  if(Glob->max_valid_layer < 0) {
    Glob->max_valid_layer = 0;
  } else if(Glob->max_valid_layer > Glob->nz - 1) {
    Glob->max_valid_layer = Glob->nz - 1;
  }

  /*
   * check that field numbers requested do not exceed that available
   */
  
  if (Glob->params.dbz_field > Glob->nfields - 1) {
    fprintf(stderr, "ERROR - %s:read_dobson\n", Glob->prog_name);
    fprintf(stderr,
	    "Dbz field number %ld too great - only %ld fields available.\n",
	    (long) Glob->params.dbz_field, (long) Glob->nfields);
    fprintf(stderr, "Field numbers start at 0\n");
    return(-1);
  }
  
  if (Glob->params.vel_available && Glob->params.vel_field > Glob->nfields - 1) {
    fprintf(stderr, "ERROR - %s:read_dobson\n", Glob->prog_name);
    fprintf(stderr,
	    "Vel field number %ld too great - only %ld fields available.\n",
	    (long) Glob->params.vel_field, (long) Glob->nfields);
    fprintf(stderr, "Field numbers start at 0\n");
    return(-1);
  }
  
  /*
   * set Dbz_byte_data
   */
  
  dbz_scale = ((double) v_handle->field_params[Glob->params.dbz_field]->scale /
	       (double) v_handle->field_params[Glob->params.dbz_field]->factor);
  dbz_bias = ((double) v_handle->field_params[Glob->params.dbz_field]->bias /
	      (double) v_handle->field_params[Glob->params.dbz_field]->factor);
  
  Glob->low_dbz_byte =
    (si32) ((Glob->params.low_dbz_threshold - dbz_bias) / dbz_scale + 0.5);
  
  Glob->high_dbz_byte =
    (si32) ((Glob->params.high_dbz_threshold - dbz_bias) / dbz_scale + 0.5);
  
  /*
   * if scale or bias has changed, as will always be the case on the
   * first call, set the Glob->dbz_interval array. If the dbz val
   * is outside the histogram range, the index is set to -1
   */
  
  if (dbz_scale != dbz_scale_prev || dbz_bias != dbz_bias_prev) {
    
    for (i = 0; i < N_BYTE_DATA_VALS; i++) {
      
      dbz = (double) i * dbz_scale + dbz_bias;
      
      if (dbz < Glob->params.low_dbz_threshold ||
	  dbz >= Glob->params.high_dbz_threshold) {

	Glob->dbz_interval[i] = -1;

      } else {

	Glob->dbz_interval[i] =
	  (si32) ((dbz - Glob->params.low_dbz_threshold) / Glob->params.dbz_hist_interval);

      }

    } /* i */

  } /* if (dbz_scale != dbz_scale_prev .... */
  
  /*
   * save scale and bias values for checking next time
   */
  
  dbz_scale_prev = dbz_scale;
  dbz_bias_prev = dbz_bias;
  
  return(0);
  
}

