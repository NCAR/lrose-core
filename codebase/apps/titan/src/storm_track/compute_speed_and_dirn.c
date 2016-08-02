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
 * compute_speed_and_dirn.c
 *
 * compute the speed and direction from dx and dy
 *
 * RAP, NCAR, Boulder CO
 *
 * Oct 1994
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "storm_track.h"
#include <toolsa/pjg.h>

#define KM_PER_DEG_AT_EQUATOR 111.12

void compute_speed_and_dirn(storm_file_handle_t *s_handle,
			    storm_status_t *storms,
			    si32 nstorms)

{

  int i;
  int latlon;
  double start_lat, start_lon;
  double end_lat, end_lon;
  double dx_km, dy_km;
  double speed, direction;

  storm_file_global_props_t *gprops;
  storm_file_params_t *sparams;
  track_status_t *track;

  sparams = &s_handle->header->params;

  if (s_handle->scan->grid.proj_type == TITAN_PROJ_LATLON) {
    latlon = TRUE;
  } else {
    latlon = FALSE;
  }

  for (i = 0; i < nstorms; i++) {

    track = storms[i].track;

    if (latlon) {
      
      gprops = s_handle->gprops + i;

      start_lon = gprops->proj_area_centroid_x;
      start_lat = gprops->proj_area_centroid_y;

      end_lon = start_lon + track->dval_dt.smoothed_proj_area_centroid_x;
      end_lat = start_lat + track->dval_dt.smoothed_proj_area_centroid_y;

      PJGLatLon2DxDy(start_lat, start_lon, end_lat, end_lon,
		     &dx_km, &dy_km);

    } else {
      
      dx_km = track->dval_dt.smoothed_proj_area_centroid_x;
      dy_km = track->dval_dt.smoothed_proj_area_centroid_y;
      
    } /* if (latlon) */
  
    speed = sqrt(dx_km * dx_km + dy_km * dy_km);

    if (dx_km == 0.0 && dy_km == 0.0)
      direction = 0.0;
    else
      direction = atan2(dx_km, dy_km) * RAD_TO_DEG;

    if (direction < 0.0)
      direction += 360.0;
    
    track->dval_dt.smoothed_speed = speed;
    track->dval_dt.smoothed_direction = direction;

  } /* i */

}

