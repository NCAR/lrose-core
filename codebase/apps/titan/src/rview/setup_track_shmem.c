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
 * setup_track_shmem.c
 *
 * Sets up shared memory communication for track data.
 * If time hist will not be run, the memory is allocated locally.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "rview.h"

#define S_PERMISSIONS 0666

void setup_track_shmem(void)

{

  char call_str[BUFSIZ];
  time_hist_shmem_t *tshmem;
  coord_export_t *coord;

  if (Glob->debug) {
    fprintf(stderr, "** setup_track_shmem **\n");
  }

  /*
   * if time hist will be used, get shared memory.
   * Otherwise, malloc local mem for shmem struct
   */

  if (Glob->use_time_hist) {

    if ((Glob->coord_export = (coord_export_t *)
	 ushm_create(Glob->track_shmem_key,
		     sizeof(coord_export_t),
		     S_PERMISSIONS)) == NULL) {
      fprintf(stderr, "ERROR - %s:setup_track_shmem.\n", Glob->prog_name);
      fprintf(stderr,
	      "Cannot create shared memory for coord_export, key = %d\n",
	      Glob->track_shmem_key);
      Glob->use_time_hist = FALSE;
      tidy_and_exit(-1);
    }

    if ((Glob->track_shmem = (time_hist_shmem_t *)
	 ushm_create(Glob->track_shmem_key + 1,
		     sizeof(time_hist_shmem_t),
		     S_PERMISSIONS)) == NULL) {
      fprintf(stderr, "ERROR - %s:setup_track_shmem.\n", Glob->prog_name);
      fprintf(stderr,
	      "Cannot create shared memory for track data, key = %d\n",
	      Glob->track_shmem_key + 1);
      Glob->use_time_hist = FALSE;
      tidy_and_exit(-1);
    }

  } else {

    Glob->track_shmem = (time_hist_shmem_t *)
      umalloc((ui32) sizeof(time_hist_shmem_t));

    Glob->coord_export = (coord_export_t *)
      umalloc((ui32) sizeof(coord_export_t));

  }

  /*
   * initialize coord export
   */
  
  coord = Glob->coord_export;

  memset ((void *) coord,
          (int) 0, (size_t) sizeof(coord_export_t));
  
  coord->datum_longitude = Glob->grid.proj_origin_lon;
  coord->datum_latitude = Glob->grid.proj_origin_lat;
  coord->shmem_ready = TRUE;

  /*
   * initialize time_hist shmem
   */
  
  tshmem = Glob->track_shmem;

  memset ((void *)  tshmem,
          (int) 0, (size_t) sizeof(time_hist_shmem_t));

  tshmem->mode = Glob->mode;
  tshmem->track_set = TDATA_ALL_AT_TIME;
  tshmem->time = Glob->time;
  tshmem->n_forecast_steps = Glob->n_forecast_steps;
  tshmem->forecast_interval = Glob->forecast_interval;
  tshmem->plot_forecast = Glob->plot_forecast;
  tshmem->select_track_type = COMPLEX_TRACK;
  tshmem->main_display_must_update = TRUE;

  ustrncpy(tshmem->track_server_subtype,
	   Glob->track_server_subtype,
	   SERVMAP_NAME_MAX);

  ustrncpy(tshmem->track_server_instance,
	   Glob->track_server_instance,
	   SERVMAP_INSTANCE_MAX);

  ustrncpy(tshmem->track_server_default_host,
	   Glob->track_server_default_host,
	   TIME_HIST_SHMEM_HOST_NAME_LEN);

  tshmem->track_server_default_port = 
    Glob->track_server_default_port;

  tshmem->track_data_time_margin = (si32)
    (xGetResDouble(Glob->rdisplay, Glob->prog_name,
		   "track_data_time_margin",
		   TRACK_DATA_TIME_MARGIN) * 60.0 + 0.5);

  ustrncpy(tshmem->past_color,
	   xGetResString(Glob->rdisplay, Glob->prog_name,
			 "x_past_storm_color",
			 X_PAST_STORM_COLOR),
	   TIME_HIST_SHMEM_COLOR_NAME_LEN);

  ustrncpy(tshmem->current_color,
	   xGetResString(Glob->rdisplay, Glob->prog_name,
			 "x_current_storm_color",
			 X_CURRENT_STORM_COLOR),
	   TIME_HIST_SHMEM_COLOR_NAME_LEN);

  ustrncpy(tshmem->future_color,
	   xGetResString(Glob->rdisplay, Glob->prog_name,
			 "x_future_storm_color",
			 X_FUTURE_STORM_COLOR),
	   TIME_HIST_SHMEM_COLOR_NAME_LEN);

  ustrncpy(tshmem->forecast_color,
	   xGetResString(Glob->rdisplay, Glob->prog_name,
			 "x_forecast_storm_color",
			 X_FORECAST_STORM_COLOR),
	   TIME_HIST_SHMEM_COLOR_NAME_LEN);

  tshmem->grid = Glob->grid;

  tshmem->main_display_active = TRUE;
  tshmem->shmem_ready = TRUE;

  /*
   * start time_hist if needed
   */

  if (Glob->use_time_hist) {

    if (Glob->display_name != NULL) {

      sprintf(call_str, "%s -d %s",
	      Glob->time_hist_command_line,
	      Glob->display_name);

    } else {

      sprintf(call_str, "%s", Glob->time_hist_command_line);
    
    } /* if (Glob->display_name != NULL) */

    system(call_str);

  } /* if (Glob->use_time_hist) */
		    
}
