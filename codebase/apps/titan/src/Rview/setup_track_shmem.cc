// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
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

#include "Rview.hh"
#include <toolsa/str.h>
using namespace std;

#define S_PERMISSIONS 0666

void setup_track_shmem()

{

  char call_str[BUFSIZ];
  time_hist_shmem_t *tshmem;
  coord_export_t *coord;

  if (Glob->verbose) {
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
  
  coord->datum_longitude = Glob->proj.getCoord().proj_origin_lon;
  coord->datum_latitude = Glob->proj.getCoord().proj_origin_lat;
  coord->shmem_ready = TRUE;

  /*
   * initialize time_hist shmem
   */
  
  tshmem = Glob->track_shmem;
  memset(tshmem, 0, sizeof(time_hist_shmem_t));
  
  char *time_label = uGetParamString(Glob->prog_name,
                                     "time_label", "UTC");
  STRncopy(tshmem->time_label, time_label,
           TIME_HIST_SHMEM_TIME_LABEL_LEN);
  tshmem->localtime = Glob->localtime;
  tshmem->timeOffsetSecs = 0;

  tshmem->mode = Glob->mode;
  tshmem->track_set = TDATA_ALL_AT_TIME;
  tshmem->time = Glob->time;
  tshmem->scan_delta_t = Glob->scan_delta_t;
  if (tshmem->time == 0) {
    tshmem->time = time(NULL);
  }
  tshmem->n_forecast_steps = Glob->n_forecast_steps;
  tshmem->forecast_interval = Glob->forecast_interval;
  tshmem->plot_forecast = Glob->plot_forecast;
  tshmem->select_track_type = COMPLEX_TRACK;
  tshmem->main_display_must_update = TRUE;

  ustrncpy(tshmem->titan_server_url,
	   Glob->titan_server_url,
	   TIME_HIST_SHMEM_URL_LEN);

  tshmem->track_data_time_margin = (si32)
    (uGetParamDouble(Glob->prog_name,
		   "track_data_time_margin",
		   TRACK_DATA_TIME_MARGIN) * 60.0 + 0.5);

  ustrncpy(tshmem->past_color,
	   uGetParamString(Glob->prog_name,
                           "x_past_storm_color",
                           X_PAST_STORM_COLOR),
	   TIME_HIST_SHMEM_COLOR_NAME_LEN);
  
  ustrncpy(tshmem->current_color,
	   uGetParamString(Glob->prog_name,
                           "x_current_storm_color",
                           X_CURRENT_STORM_COLOR),
	   TIME_HIST_SHMEM_COLOR_NAME_LEN);

  ustrncpy(tshmem->future_color,
	   uGetParamString(Glob->prog_name,
			 "x_future_storm_color",
			 X_FUTURE_STORM_COLOR),
	   TIME_HIST_SHMEM_COLOR_NAME_LEN);

  ustrncpy(tshmem->forecast_color,
	   uGetParamString(Glob->prog_name,
			 "x_forecast_storm_color",
			 X_FORECAST_STORM_COLOR),
	   TIME_HIST_SHMEM_COLOR_NAME_LEN);

  // set the grid
  
  titan_grid_t *tgrid = &tshmem->grid;
  const Mdvx::coord_t *mgrid = &Glob->proj.getCoord();
  
  tgrid->proj_origin_lat = mgrid->proj_origin_lat;
  tgrid->proj_origin_lon = mgrid->proj_origin_lon;
  tgrid->minx = mgrid->minx;
  tgrid->miny = mgrid->miny;
  tgrid->minz = mgrid->minz;
  tgrid->dx = mgrid->dx;
  tgrid->dy = mgrid->dy;
  tgrid->dz = mgrid->dz;
  tgrid->sensor_x = mgrid->sensor_x;
  tgrid->sensor_y = mgrid->sensor_y;
  tgrid->sensor_z = mgrid->sensor_z;
  tgrid->sensor_lat = mgrid->sensor_lat;
  tgrid->sensor_lon = mgrid->sensor_lon;
  tgrid->proj_type = mgrid->proj_type;
  tgrid->dz_constant = mgrid->dz_constant;
  tgrid->nx = mgrid->nx;
  tgrid->ny = mgrid->ny;
  tgrid->nz = mgrid->nz;
  
  memcpy(tgrid->unitsx, mgrid->unitsx, TITAN_GRID_UNITS_LEN);
  memcpy(tgrid->unitsy, mgrid->unitsy, TITAN_GRID_UNITS_LEN);
  memcpy(tgrid->unitsz, mgrid->unitsz, TITAN_GRID_UNITS_LEN);
  
  tshmem->main_display_active = TRUE;
  tshmem->shmem_ready = TRUE;

  if (Glob->debug) {
    cerr << "Set up time_hist_shmem segment" << endl;
    cerr << "titan_server_url: " << tshmem->titan_server_url << endl;
    cerr << "time_label: " << tshmem->time_label << endl;
    cerr << "time: " << DateTime::strm(tshmem->time) << endl;
    cerr << "shmem_ready: " << tshmem->shmem_ready << endl;
    cerr << "localtime: " << tshmem->localtime << endl;
    cerr << "timeOffsetSecs: " << tshmem->timeOffsetSecs << endl;
    cerr << "time_hist_active: " << tshmem->time_hist_active << endl;
    cerr << "main_display_active: " << tshmem->main_display_active << endl;
    cerr << "main_display_must_update: " << tshmem->main_display_must_update << endl;
    cerr << "mode: " << tshmem->mode << endl;
    cerr << "plot_forecast: " << tshmem->plot_forecast << endl;
    cerr << "track_set: " << tshmem->track_set << endl;
    cerr << "scan_delta_t: " << tshmem->scan_delta_t << endl;
    cerr << "complex_track_num: " << tshmem->complex_track_num << endl;
    cerr << "simple_track_num: " << tshmem->simple_track_num << endl;
    cerr << "track_type: " << tshmem->track_type << endl;
    cerr << "track_seq_num: " << tshmem->track_seq_num << endl;
    cerr << "select_track_type: " << tshmem->select_track_type << endl;
    cerr << "track_data_time_margin: " << tshmem->track_data_time_margin << endl;
    cerr << "past_plot_period: " << tshmem->past_plot_period << endl;
    cerr << "future_plot_period: " << tshmem->future_plot_period << endl;
    cerr << "case_num: " << tshmem->case_num << endl;
    cerr << "partial_track_ref_time: " << tshmem->partial_track_ref_time << endl;
    cerr << "partial_track_past_period: " << tshmem->partial_track_past_period << endl;
    cerr << "partial_track_future_period: " << tshmem->partial_track_future_period << endl;
    cerr << "n_forecast_steps: " << tshmem->n_forecast_steps << endl;
    cerr << "forecast_interval: " << tshmem->forecast_interval << endl;
    cerr << "past_color: " << tshmem->past_color << endl;
    cerr << "current_color: " << tshmem->current_color << endl;
    cerr << "future_color: " << tshmem->future_color << endl;
    cerr << "forecast_color: " << tshmem->forecast_color << endl;
  }

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
