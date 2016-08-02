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
/****************************************************************************
 * load_bounds.c
 *
 * Compute the bounding limits for the projected area of each
 * storm, either in it's present or forecast state
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * December 1995
 *
 ****************************************************************************/

#include "storm_track.h"

void load_bounds(storm_file_handle_t *s_handle,
		 track_file_handle_t *t_handle,
		 si32 nstorms1,
		 si32 nstorms2,
		 storm_status_t *storms1,
		 storm_status_t *storms2,
		 double d_hours)
     
{
  
  si32 istorm, jstorm;

  double current_x, current_y, current_area;
  double forecast_x, forecast_y, forecast_area;
  double dx_dt, dy_dt, darea_dt;
  double length_ratio;
  double current_minx, current_miny, current_maxx, current_maxy;

  titan_grid_t *grid = &s_handle->scan->grid;
  double fcast_minx, fcast_miny, fcast_maxx, fcast_maxy;
  storm_status_t *storm;
  
  for (istorm = 0; istorm < nstorms1; istorm++) {

    /*
     * for storms in prev scan, compute the bounding limits
     * based on the forecast as well as the current position
     */

    storm = storms1 + istorm;
	    
    dx_dt = storm->track->dval_dt.proj_area_centroid_x;
    dy_dt = storm->track->dval_dt.proj_area_centroid_y;
    darea_dt = storm->track->dval_dt.proj_area;
	    
    current_x = storm->current.proj_area_centroid_x;
    current_y = storm->current.proj_area_centroid_y;
    current_area = storm->current.proj_area;
	    
    forecast_x = current_x + dx_dt * d_hours;
    forecast_y = current_y + dy_dt * d_hours;
	    
    forecast_area = current_area + darea_dt * d_hours;
    if (forecast_area < 1.0) {
      forecast_area = 1.0;
    }

    if (current_area > 0) {
      length_ratio = pow((forecast_area / current_area), 0.5);
    } else {
      length_ratio = 1.0;
    }

    current_minx =
      grid->minx + (storm->current.bound.min_ix - 0.5) * grid->dx;
    current_miny =
      grid->miny + (storm->current.bound.min_iy - 0.5) * grid->dy;
    current_maxx =
      grid->minx + (storm->current.bound.max_ix + 0.5) * grid->dx;
    current_maxy =
      grid->miny + (storm->current.bound.max_iy + 0.5) * grid->dy;

    fcast_minx =
      forecast_x - (current_x - current_minx) * length_ratio;
    fcast_miny =
      forecast_y - (current_y - current_miny) * length_ratio;
    fcast_maxx =
      forecast_x - (current_x - current_maxx) * length_ratio;
    fcast_maxy =
      forecast_y - (current_y - current_maxy) * length_ratio;

    storm->box_for_overlap.min_ix =
      (si32) ((fcast_minx - grid->minx) / grid->dx + 0.5);
    storm->box_for_overlap.min_iy =
      (si32) ((fcast_miny - grid->miny) / grid->dy + 0.5);
    storm->box_for_overlap.max_ix =
      (si32) ((fcast_maxx - grid->minx) / grid->dx + 0.5);
    storm->box_for_overlap.max_iy =
      (si32) ((fcast_maxy - grid->miny) / grid->dy + 0.5);

    storm->track->forecast_x = forecast_x;
    storm->track->forecast_y = forecast_y;
    storm->track->forecast_area = forecast_area;
    storm->track->forecast_length_ratio = length_ratio;

  } /* istorm */

  for (jstorm = 0; jstorm < nstorms2; jstorm++) {

    /*
     * for storms in current scan, bounding box is based
     * on the current scan
     */

    storm = storms2 + jstorm;
    storm->box_for_overlap = storm->current.bound;

  } /* jstorm */

}

