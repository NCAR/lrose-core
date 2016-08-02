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
 * init_file_header.c
 *
 * Set initial values in track file header
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * August 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "storm_track.h"

void init_file_header(storm_file_handle_t *s_handle,
		      track_file_header_t *theader)

{
  
  si32 iweight;
  
  memset (theader, 0, sizeof(storm_file_header_t));

  theader->params.grid_type = s_handle->scan->grid.proj_type;
  theader->params.nweights_forecast = Glob->params.forecast_weights.len;

  theader->params.forecast_type = Glob->forecast_type;

  for (iweight  = 0;
       iweight < Glob->params.forecast_weights.len; iweight++) {
    theader->params.forecast_weights[iweight] =
      Glob->params.forecast_weights.val[iweight];
  }

  theader->params.weight_distance = 
    Glob->params.weight_distance;
  
  theader->params.weight_delta_cube_root_volume = 
    Glob->params.weight_delta_cube_root_volume;
  
  theader->params.max_tracking_speed = 
    Glob->params.max_tracking_speed;
  
  theader->params.max_delta_time = Glob->params.max_delta_time;
  
  theader->params.min_history_for_valid_forecast =
    Glob->params.min_history_for_valid_forecast;

  theader->params.max_speed_for_valid_forecast = 
    Glob->params.max_speed_for_valid_forecast;
  
  theader->params.parabolic_growth_period = 
    Glob->params.parabolic_growth_period;
  
  theader->params.spatial_smoothing =
    Glob->params.spatial_smoothing;
  
  theader->params.smoothing_radius = 
    Glob->params.smoothing_radius;
  
  theader->params.use_runs_for_overlaps =
    Glob->params.use_runs_for_overlaps;

  theader->params.scale_forecasts_by_history =
    Glob->params.scale_forecasts_by_history;

  theader->params.min_fraction_overlap = 
    Glob->params.min_fraction_overlap;
  
  theader->params.min_sum_fraction_overlap = 
    Glob->params.min_sum_fraction_overlap;
  
  theader->n_simple_tracks = 0;
  theader->n_complex_tracks = 0;

  theader->max_simple_track_num = 0;
  theader->max_complex_track_num = 0;

  theader->data_file_size = 0;
  
  theader->max_parents = MAX_PARENTS;
  theader->max_children = MAX_CHILDREN;
  theader->max_nweights_forecast = MAX_NWEIGHTS_FORECAST;
  
}



