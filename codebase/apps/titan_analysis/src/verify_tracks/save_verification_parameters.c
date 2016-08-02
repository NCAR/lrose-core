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
/*******************************************************************************
 * load_verification_parameters.c
 *
 * Saves the parameters used in this analysis in the track file header
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * January 1992
 *
 *******************************************************************************/

#include "verify_tracks.h"

#define SCALE_FACTOR 1000.0

void save_verification_parameters(storm_file_handle_t *s_handle,
				  track_file_handle_t *t_handle)

{

  titan_grid_t *grid;

  grid = &t_handle->header->verify.grid;
  *grid = s_handle->scan->grid;

  grid->proj_params.flat.rotation = 0.0;

  grid->nx = Glob->nx;
  grid->ny = Glob->ny;
  grid->nz = 1;
  
  grid->dz_constant = TRUE;
  
  grid->minx = Glob->minx;
  grid->miny = Glob->miny;
  
  grid->dx = Glob->dx;
  grid->dy = Glob->dy;
  grid->dz = 1.0;

  strcpy(grid->unitsx, "km");
  strcpy(grid->unitsy, "km");
  strcpy(grid->unitsz, "km");

  t_handle->header->verify.verification_performed = TRUE;

  t_handle->header->verify.end_time = s_handle->header->end_time;

  t_handle->header->verify.forecast_lead_time =
    (long) floor(Glob->forecast_lead_time + 0.5);

  t_handle->header->verify.forecast_lead_time_margin =
    (long) floor(Glob->forecast_lead_time_margin + 0.5);

  t_handle->header->verify.forecast_min_history =
    Glob->forecast_min_history;

  if (RfWriteTrackHeader(t_handle,
			 "save_verification_parameters"))
    tidy_and_exit(-1);

}
