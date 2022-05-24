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
/***************************************************************************
 * init_indices.c
 *
 * initializes the track verification file handle
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * November 1991
 *
 ***************************************************************************/

#include "track_grid_stats.h"

void init_indices(storm_file_handle_t *s_handle,
		  track_file_handle_t *t_handle,
		  vol_file_handle_t *v_handle)

{

  char note[2048];
  si32 ifield;
  cart_params_t *cart;
  field_params_t *fparams;

  /*
   * initialize storm file handle
   */
  
  RfInitStormFileHandle(s_handle, Glob->prog_name);
      
  /*
   * initialize track file handle
   */

  RfInitTrackFileHandle(t_handle, Glob->prog_name);
      
  /*
   * initialize volume file handle
   */

  RfInitVolFileHandle(v_handle,
		      Glob->prog_name,
		      (char *) NULL,
		      (FILE *) NULL);

  if (RfAllocVolParams(v_handle,
		       "init_indices"))
    tidy_and_exit(-1);

  memset((void *) v_handle->vol_params,
	 (int) 0, sizeof(vol_params_t));

  /*
   * note
   */

  sprintf(note, "%s\n%s : %ld\n%s : %g\n%s : %g\n",
	  Glob->params.note,
	  "n_seasons", Glob->params.n_seasons,
	  "scan_interval", Glob->params.scan_interval,
	  "min_duration", Glob->params.min_duration); 

  ustrncpy(v_handle->vol_params->note, note, VOL_PARAMS_NOTE_LEN);

  /*
   * init radar params
   */
  
  strcpy(v_handle->vol_params->radar.name, "TRACK_STATS");

  /*
   * init cart params
   */

  cart = &v_handle->vol_params->cart;

  cart->latitude = (si32) (Glob->params.grid_lat * 1000000.0 + 0.5);
  cart->longitude = (si32) (Glob->params.grid_lon * 1000000.0 + 0.5);
  cart->nx = Glob->params.grid.nx;
  cart->ny = Glob->params.grid.ny;
  cart->nz = 1;
  cart->scalex = 1000;
  cart->scaley = 1000;
  cart->scalez = 1000;
  cart->km_scalex = 1000;
  cart->km_scaley = 1000;
  cart->km_scalez = 1000;
  cart->minx = (si32) (Glob->params.grid.minx * cart->km_scalex + 0.5);
  cart->miny = (si32) (Glob->params.grid.miny * cart->km_scaley + 0.5);
  cart->minz = 0;
  cart->dx = (si32) (Glob->params.grid.dx * cart->km_scalex + 0.5);
  cart->dy = (si32) (Glob->params.grid.dy * cart->km_scaley + 0.5);
  cart->dz = cart->km_scalez;
  cart->dz_constant = TRUE;
  strcpy(cart->unitsx, "km");
  strcpy(cart->unitsy, "km");
  strcpy(cart->unitsz, "km");

  /*
   * number of fields
   */
  
  v_handle->vol_params->nfields = N_STATS_FIELDS;

  /*
   * allocate arrays
   */
  
  if (RfAllocVolArrays(v_handle, "init_indices"))
    tidy_and_exit(-1);

  /*
   * plane heights
   */

  v_handle->plane_heights[0][PLANE_BASE_INDEX] = cart->minz;
  v_handle->plane_heights[0][PLANE_MIDDLE_INDEX] = cart->minz + cart->dz / 2;
  v_handle->plane_heights[0][PLANE_TOP_INDEX] = cart->minz + cart->dz;

  /*
   * field parameters
   */

  for (ifield = 0; ifield < N_STATS_FIELDS; ifield++) {

    fparams = v_handle->field_params[ifield];
    memset((void *) fparams, (int) 0, (size_t) sizeof(field_params_t));
    
    fparams->encoded = TRUE;
    fparams->factor = 10000;
    strcpy(fparams->transform, "none");

    switch (ifield) {

    case N_EVENTS_POS:
      fparams->factor = 1000000;
      strcpy(fparams->name, "N_events");
      strcpy(fparams->units, "count");
      break;
    
    case N_WEIGHTED_POS:
      fparams->factor = 1000000;
      strcpy(fparams->name, "N_weighted");
      strcpy(fparams->units, "count");
      break;
    
    case N_COMPLEX_POS:
      fparams->factor = 1000000;
      strcpy(fparams->name, "N_complex");
      strcpy(fparams->units, "count");
      break;
    
    case PERCENT_ACTIVITY_POS:
      strcpy(fparams->name, "Activity");
      strcpy(fparams->units, "%");
      break;
    
    case N_START_POS:
      fparams->factor = 10000000;
      strcpy(fparams->name, "N_start");
      strcpy(fparams->units, "count/km2");
      break;
    
    case N_MID_POS:
      fparams->factor = 10000000;
      strcpy(fparams->name, "N_mid");
      strcpy(fparams->units, "count/km2");
      break;
    
    case PRECIP_POS:
      strcpy(fparams->name, "Precip");
      strcpy(fparams->units, "mm");
      break;
      
    case VOLUME_POS:
      strcpy(fparams->name, "Mean volume");
      strcpy(fparams->units, "km3");
      break;
    
    case DBZ_MAX_POS:
      strcpy(fparams->name, "Mean max_dbz");
      strcpy(fparams->units, "dBZ");
      break;
    
    case TOPS_POS:
      strcpy(fparams->name, "Mean tops");
      strcpy(fparams->units, "km");
      break;
    
    case SPEED_POS:
      strcpy(fparams->name, "Mean speed");
      strcpy(fparams->units, "km/hr");
      break;
    
    case U_POS:
      strcpy(fparams->name, "Mean U");
      strcpy(fparams->units, "km/hr");
      break;
    
    case V_POS:
      strcpy(fparams->name, "Mean V");
      strcpy(fparams->units, "km/hr");
      break;
    
    case DISTANCE_POS:
      strcpy(fparams->name, "Mean distance moved");
      strcpy(fparams->units, "km");
      break;
    
    case DX_POS:
      strcpy(fparams->name, "Mean dx moved");
      strcpy(fparams->units, "km");
      break;
    
    case DY_POS:
      strcpy(fparams->name, "Mean dy moved");
      strcpy(fparams->units, "km");
      break;
    
    case DUR_MAX_PRECIP_POS:
      sprintf(fparams->name, "Max %g min precip",
	      (double) Glob->params.dur_for_max_precip / 60.0);
      strcpy(fparams->units, "mm");
      break;
    
    case AREA_POS:
      strcpy(fparams->name, "Mean area");
      strcpy(fparams->units, "km2");
      break;
    
    case DURATION_POS:
      strcpy(fparams->name, "Mean duration");
      strcpy(fparams->units, "hr");
      break;
    
    case LN_AREA_POS:
      strcpy(fparams->name, "ln(area)");
      strcpy(fparams->units, "ln(km2)");
      break;
    
    } /* switch */

    /*
     * allocate data plane
     */

    v_handle->field_plane[ifield][0] = (ui08 *) ucalloc
      ((ui32) (Glob->params.grid.nx *
		Glob->params.grid.ny), (ui32) sizeof(ui08));
    
  } /* ifield */

}







