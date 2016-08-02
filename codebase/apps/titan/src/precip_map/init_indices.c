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

#include "precip_map.h"

void init_indices(storm_file_handle_t *s_handle,
		  track_file_handle_t *t_handle,
		  vol_file_handle_t *v_handle,
		  vol_file_handle_t *map_v_handle)

{

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

  /*
   * initialize map file handle
   */

  RfInitVolFileHandle(map_v_handle,
		      Glob->prog_name,
		      (char *) NULL,
		      (FILE *) NULL);
  
  if (RfAllocVolParams(map_v_handle, "init_indices"))
    tidy_and_exit(-1);
  
}

/************************************************************************
 * init_map_index()
 *
 */

void init_map_index(vol_file_handle_t *map_v_handle,
		    vol_file_handle_t *radar_v_handle,
		    date_time_t *file_time)

{

  field_params_t *fparams;
  vol_params_t *vol_params;
  date_time_t ttime;
  si32 dt;

  memcpy((void *) map_v_handle->vol_params,
	 (void *) radar_v_handle->vol_params,
	 (size_t) sizeof(vol_params_t));
  
  map_v_handle->vol_params->nfields = 1;
  map_v_handle->vol_params->cart.nz = 1;
  map_v_handle->vol_params->cart.scalez = 10000;
  map_v_handle->vol_params->cart.km_scalez = 10000;
  map_v_handle->vol_params->cart.dz = 10000;
  
  /*
   * set times - test the input file mid time against the 
   * file time. If they differ, offset the data times by
   * that amount.
   */

  vol_params = map_v_handle->vol_params;
  
  Rfrtime2dtime(&vol_params->mid_time, &ttime);
  dt = file_time->unix_time - ttime.unix_time;

  if (dt != 0) {

    ttime.unix_time += dt;
    uconvert_from_utime(&ttime);
    Rfdtime2rtime(&ttime, &vol_params->mid_time);
  
    Rfrtime2dtime(&vol_params->start_time, &ttime);
    ttime.unix_time += dt;
    uconvert_from_utime(&ttime);
    Rfdtime2rtime(&ttime, &vol_params->start_time);

    Rfrtime2dtime(&vol_params->end_time, &ttime);
    ttime.unix_time += dt;
    uconvert_from_utime(&ttime);
    Rfdtime2rtime(&ttime, &vol_params->end_time);

  }
  
  if (RfAllocVolArrays(map_v_handle, "initialize")) {
    tidy_and_exit(-1);
  }
  
  memcpy((void *) map_v_handle->radar_elevations,
	 (void *) radar_v_handle->radar_elevations,
	 (int) (map_v_handle->vol_params->radar.nelevations * sizeof(si32)));
  
  map_v_handle->plane_heights[0][PLANE_BASE_INDEX] = 0;
  map_v_handle->plane_heights[0][PLANE_MIDDLE_INDEX] = 5000;
  map_v_handle->plane_heights[0][PLANE_TOP_INDEX] = 10000;

  /*
   * init field params for precip output grid
   */

  fparams = map_v_handle->field_params[0];
  
  switch (Glob->params.map_type) {

  case FORECAST:
    memset((void *) fparams,
	   (int) 0, sizeof(field_params_t));
    fparams->encoded = TRUE;
    fparams->factor = 10000;
    fparams->scale = 1;
    fparams->bias = 0;
    strcpy(fparams->transform, "none");
    strcpy(fparams->units, "mm");
    break;

  case VERIFY:
    memset((void *) fparams,
	   (int) 0, sizeof(field_params_t));
    fparams->encoded = TRUE;
    fparams->factor = 10000;
    fparams->scale = 1;
    fparams->bias = 0;
    strcpy(fparams->transform, "none");
    strcpy(fparams->units, "mm");
    break;

  case PERSISTENCE:
    memset((void *) fparams,
	   (int) 0, sizeof(field_params_t));
    fparams->encoded = TRUE;
    fparams->factor = 10000;
    fparams->scale = 1;
    fparams->bias = 0;
    strcpy(fparams->transform, "none");
    strcpy(fparams->units, "mm");
    break;

  case REFL_FORECAST:
    memcpy((void *) fparams,
	   (void *) radar_v_handle->field_params[Glob->params.dbz_field],
	   sizeof(field_params_t));
    break;
    
  case ACCUM_PERIOD:
    memset((void *) fparams,
	   (int) 0, sizeof(field_params_t));
    fparams->encoded = TRUE;
    fparams->factor = 10000;
    fparams->scale = 1;
    fparams->bias = 0;
    strcpy(fparams->transform, "none");
    strcpy(fparams->units, "mm");
    break;

  case ACCUM_FROM_START:
    memset((void *) fparams,
	   (int) 0, sizeof(field_params_t));
    fparams->encoded = TRUE;
    fparams->factor = 10000;
    fparams->scale = 1;
    fparams->bias = 0;
    strcpy(fparams->transform, "none");
    strcpy(fparams->units, "mm");
    break;

  default:
    strcpy(fparams->units, "mm");
    break;

  } /* switch */

}







