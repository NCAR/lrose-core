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
 * read_params.c
 *
 * reads the parameters or X data base to get parameters
 *
 * RAP, NCAR, Boulder CO
 *
 * April 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "rview.h"

static void load_psgc_linestyle(psgc_t *psgc,
				char *params_str,
				char *def_str);

void read_params(void)

{
  
  double ps_maxprintwidth, ps_minprintwidth;
  char *resource_str;

  Glob->time_hist_command_line =
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "time_hist_command_line",
		  TIME_HIST_COMMAND_LINE);
  
  Glob->track_shmem_key = xGetResLong(Glob->rdisplay, Glob->prog_name,
				      "track_shmem_key", TRACK_SHMEM_KEY);
  
  Glob->foregroundstr = xGetResString(Glob->rdisplay, Glob->prog_name,
				      "x_foreground", X_FOREGROUND);
  Glob->backgroundstr = xGetResString(Glob->rdisplay, Glob->prog_name,
				      "x_background", X_BACKGROUND);
  
  Glob->double_click_delay = xGetResDouble(Glob->rdisplay, Glob->prog_name,
					   "double_click_delay",
					   DOUBLE_CLICK_DELAY) * 1000.0;
  
  Glob->field = xGetResLong(Glob->rdisplay, Glob->prog_name,
			    "field_requested", FIELD_REQUESTED);
  
  Glob->z_requested = xGetResDouble(Glob->rdisplay,
				    Glob->prog_name,
				    "z_requested",
				    Z_REQUESTED);
  
  Glob->z_cappi = Glob->z_requested;
  
  Glob->print_width = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				    "ps_printwidth", PS_PRINTWIDTH);
  
  ps_maxprintwidth = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				   "ps_maxprintwidth", PS_MAXPRINTWIDTH);
  ps_minprintwidth = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				   "ps_minprintwidth", PS_MINPRINTWIDTH);
  
  /*
   * main x window dimensions
   */
  
  Glob->mainx = xGetResLong(Glob->rdisplay, Glob->prog_name,
			    "x_mainx", X_MAINX);
  Glob->mainy = xGetResLong(Glob->rdisplay, Glob->prog_name, 
			    "x_mainy", X_MAINY);
  Glob->mainx_sign = xGetResLong(Glob->rdisplay, Glob->prog_name,
				 "x_mainx_sign", X_MAINX_SIGN);
  Glob->mainy_sign = xGetResLong(Glob->rdisplay, Glob->prog_name,
				 "x_mainy_sign", X_MAINY_SIGN);
  Glob->mainwidth = (ui32) xGetResLong(Glob->rdisplay,
					Glob->prog_name,
					"x_mainwidth",
					X_MAINWIDTH);
  Glob->mainheight = (ui32) xGetResLong(Glob->rdisplay,
					 Glob->prog_name,
					 "x_mainheight",
					 X_MAINHEIGHT);
  
  Glob->vsection_width = (ui32)
    xGetResLong(Glob->rdisplay, Glob->prog_name, "x_vsection_width",
		X_VSECTION_WIDTH);
  
  Glob->vsection_height = (ui32)
    xGetResLong(Glob->rdisplay, Glob->prog_name, "x_vsection_height",
		X_VSECTION_HEIGHT);
  
  Glob->help_width = (ui32)
    xGetResLong(Glob->rdisplay, Glob->prog_name, "x_help_width",
		X_HELP_WIDTH);
  
  Glob->help_height = (ui32)
    xGetResLong(Glob->rdisplay, Glob->prog_name, "x_help_height",
		X_HELP_HEIGHT);
  
  Glob->max_button_width = (ui32)
    xGetResLong(Glob->rdisplay, Glob->prog_name, "x_max_button_width",
		X_MAX_BUTTON_WIDTH);
  
  /*
   * full-grid limits and position
   */
  
  Glob->full_min_x = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				   "full_min_x",
				   FULL_MIN_X);
  
  Glob->full_min_y = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				   "full_min_y",
				   FULL_MIN_Y);
  
  Glob->full_max_x = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				   "full_max_x",
				   FULL_MAX_X);
  
  Glob->full_max_y = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				   "full_max_y",
				   FULL_MAX_Y);
  
  Glob->full_range_x = Glob->full_max_x - Glob->full_min_x;
  Glob->full_range_y = Glob->full_max_y - Glob->full_min_y;
  Glob->full_aspect = Glob->full_range_y / Glob->full_range_x;
  
  Glob->delta_z = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				"delta_z", DELTA_Z);
  
  /*
   * set up display projection
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			       "projection", PROJECTION);
  
  if (uset_double_param(Glob->prog_name,
			"read_params",
			Glob->params_path_name,
			resource_str, &Glob->grid.proj_type,
			"flat", TITAN_PROJ_FLAT,
			"latlon", TITAN_PROJ_LATLON,
			"projection"))
    tidy_and_exit(-1);

  ufree(resource_str);
  
  Glob->grid.proj_origin_lat =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "grid_lat", GRID_LAT);
  
  Glob->grid.proj_origin_lon =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "grid_lon", GRID_LON);

  if (Glob->grid.proj_type == TITAN_PROJ_LATLON) {
    Glob->grid.proj_params.flat.rotation = 0.0;
  } else {
    Glob->grid.proj_params.flat.rotation =
      xGetResDouble(Glob->rdisplay, Glob->prog_name,
		    "grid_rot", GRID_ROT);
  }

  TITAN_init_proj(&Glob->grid, &Glob->grid_comps);
  
  /*
   * time between scans - secs
   */
  
  Glob->scan_delta_t = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				     "scan_delta_t",
				     SCAN_DELTA_T);
  
  /*
   * linestyles
   */
  
  load_psgc_linestyle(&Glob->pos_contour_psgc,
		      "ps_pos_contour_linestyle",
		      PS_POS_CONTOUR_LINESTYLE);
  
  load_psgc_linestyle(&Glob->zero_contour_psgc,
		      "ps_zero_contour_linestyle",
		      PS_ZERO_CONTOUR_LINESTYLE);
  
  load_psgc_linestyle(&Glob->neg_contour_psgc,
		      "ps_neg_contour_linestyle",
		      PS_NEG_CONTOUR_LINESTYLE);
  
  load_psgc_linestyle(&Glob->current_storm_psgc,
		      "ps_current_storm_linestyle",
		      PS_CURRENT_STORM_LINESTYLE);
  
  load_psgc_linestyle(&Glob->past_storm_psgc[0],
		      "ps_past_storm_linestyle",
		      PS_PAST_STORM_LINESTYLE);
  
  load_psgc_linestyle(&Glob->past_storm_psgc[1],
		      "ps_past_storm_linestyle_2",
		      PS_PAST_STORM_LINESTYLE_2);
  
  load_psgc_linestyle(&Glob->future_storm_psgc[0],
		      "ps_future_storm_linestyle",
		      PS_FUTURE_STORM_LINESTYLE);
  
  load_psgc_linestyle(&Glob->future_storm_psgc[1],
		      "ps_future_storm_linestyle_2",
		      PS_FUTURE_STORM_LINESTYLE_2);
  
  load_psgc_linestyle(&Glob->forecast_storm_psgc[0],
		      "ps_forecast_storm_linestyle",
		      PS_FORECAST_STORM_LINESTYLE);
  
  load_psgc_linestyle(&Glob->forecast_storm_psgc[1],
		      "ps_forecast_storm_linestyle_2",
		      PS_FORECAST_STORM_LINESTYLE_2);
  
  load_psgc_linestyle(&Glob->past_vector_psgc,
		      "ps_past_vector_linestyle",
		      PS_PAST_VECTOR_LINESTYLE);
  
  load_psgc_linestyle(&Glob->future_vector_psgc,
		      "ps_future_vector_linestyle",
		      PS_FUTURE_VECTOR_LINESTYLE);
  
  load_psgc_linestyle(&Glob->forecast_vector_psgc,
		      "ps_forecast_vector_linestyle",
		      PS_FORECAST_VECTOR_LINESTYLE);
  
  /*
   * dim percentage for dimmed colors
   */
  
  Glob->color_dim_percent = xGetResDouble(Glob->rdisplay, Glob->prog_name,
					  "color_dim_percent",
					  COLOR_DIM_PERCENT);
  
  /*
   * check print width
   */
  
  if (Glob->print_width < ps_minprintwidth) {
    
    fprintf(stderr, "WARNING - %s:read_params.\n", Glob->prog_name);
    fprintf(stderr, "Print width set out of range, to %f\n",
	    Glob->print_width);
    fprintf(stderr, "Check params file %s\n", Glob->params_path_name);
    fprintf(stderr, "Acceptable range is %f to %f.\n",
	    ps_minprintwidth, ps_maxprintwidth);
    fprintf(stderr, "Setting print width to %f.\n", ps_minprintwidth);
    Glob->print_width = ps_minprintwidth;
    
  }
  
  if (Glob->print_width > ps_maxprintwidth) {
    
    fprintf(stderr, "WARNING - %s:read_params.\n", Glob->prog_name);
    fprintf(stderr, "Print width set out of range, to %f\n",
	    Glob->print_width);
    fprintf(stderr, "Check params file %s\n", Glob->params_path_name);
    fprintf(stderr, "Acceptable range is %f to %f.\n",
	    ps_maxprintwidth, ps_maxprintwidth);
    fprintf(stderr, "Setting print width to %f.\n", ps_maxprintwidth);
    Glob->print_width = ps_maxprintwidth;
    
  }
  
  /*
   * set up title copy
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			     "draw_copy_title", DRAW_COPY_TITLE);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->draw_copy_title, "draw_copy_title"))
    tidy_and_exit(-1);

  ufree(resource_str);
  
  /*
   * set up header copy
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			     "draw_copy_header", DRAW_COPY_HEADER);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->draw_copy_header, "draw_copy_header"))
    tidy_and_exit(-1);

  ufree(resource_str);
  
  /*
   * set up x synchronization option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			     "x_sync", X_SYNC);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str, &Glob->x_sync, "x_sync"))
    tidy_and_exit(-1);

  ufree(resource_str);
  
  /*
   * set up use_track_data option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
				     "use_track_data", USE_TRACK_DATA);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->use_track_data,
			    "use_track_data"))
    tidy_and_exit(-1);

  ufree(resource_str);
  
  /*
   * set up use_time_hist option
   */
  
  if (Glob->use_track_data) {
    
    resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
				      "use_time_hist", USE_TIME_HIST);
    
    if (uset_true_false_param(Glob->prog_name,
			      "read_params",
			      Glob->params_path_name,
			      resource_str,
			      &Glob->use_time_hist,
			      "use_time_hist"))
      tidy_and_exit(-1);

    ufree(resource_str);
    
  } else {
    
    Glob->use_time_hist = FALSE;
    
  }

  /*
   * if we are not using time_hist, do not dim tracks
   */
  
  if (!Glob->use_time_hist) {
    Glob->color_dim_percent = 100.0;
  }
  
  /*
   * set up plot_image option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
				 "plot_image", PLOT_IMAGE);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str, &Glob->plot_image, "plot_image"))
    tidy_and_exit(-1);

  ufree(resource_str);

  /*
   * set up plot_composite option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
				 "plot_composite", PLOT_COMPOSITE);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str, &Glob->plot_composite,
			    "plot_composite"))
    tidy_and_exit(-1);

  ufree(resource_str);

  /*
   * set up plot_rings option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
				 "plot_rings", PLOT_RINGS);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str, &Glob->plot_rings, "plot_rings"))
    tidy_and_exit(-1);

  ufree(resource_str);
  
  /*
   * set up plot_map option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			       "plot_maps", PLOT_MAPS);
  
  if (uset_triple_param(Glob->prog_name,
			"read_params",
			Glob->params_path_name,
			resource_str, &Glob->plot_maps,
			"false", FALSE,
			"limited", MAPS_LIMITED,
			"all", MAPS_ALL,
			"plot_maps"))
    tidy_and_exit(-1);

  ufree(resource_str);
  
  /*
   * set up annotate_tracks option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			       "annotate_tracks", ANNOTATE_TRACKS);
  
  if (!strcmp(resource_str, "false")) {
    Glob->annotate_tracks = FALSE;
  } else if (!strcmp(resource_str, "speed")) {
    Glob->annotate_tracks = SPEED_ANNOTATION;
  } else if (!strcmp(resource_str, "max_dbz")) {
    Glob->annotate_tracks = MAX_DBZ_ANNOTATION;
  } else if (!strcmp(resource_str, "vil")) {
    Glob->annotate_tracks = VIL_ANNOTATION;
  } else if (!strcmp(resource_str, "tops")) {
    Glob->annotate_tracks = TOPS_ANNOTATION;
  } else if (!strcmp(resource_str, "numbers")) {
    Glob->annotate_tracks = NUMBER_ANNOTATION;
  } else {
    fprintf(stderr, "ERROR - %s:parse_args\n", Glob->prog_name);
    fprintf(stderr,
	    "Annotate_tracks option '%s' not recognized.\n", resource_str);
    fprintf(stderr,
	    "Valid options: 'false', 'speed', 'max_dbz', "
	    "'vil', 'tops' or 'numbers'\n");
    fprintf(stderr, "Check params file '%s'\n", Glob->params_path_name);
    tidy_and_exit(-1);
  }

  ufree(resource_str);

  /*
   * set up plot_tracks option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
				  "plot_tracks", PLOT_TRACKS);
  
  if (uset_triple_param(Glob->prog_name,
			"read_params",
			Glob->params_path_name,
			resource_str, &Glob->plot_tracks,
			"false", FALSE,
			"selected_track", SELECTED_TRACK,
			"all_tracks", ALL_TRACKS,
			"plot_tracks"))
    tidy_and_exit(-1);

  ufree(resource_str);

  /*
   * set up plot_forecast option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
				    "plot_forecast", PLOT_FORECAST);
  
  if (uset_triple_param(Glob->prog_name,
			"read_params",
			Glob->params_path_name,
			resource_str, &Glob->plot_forecast,
			"false", FALSE,
			"limited", FORECAST_LIMITED,
			"all", FORECAST_ALL,
			"plot_forecast"))
    tidy_and_exit(-1);

  ufree(resource_str);
  
  /*
   * set up plot_vectors option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			       "plot_vectors",
			       PLOT_VECTORS);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->plot_vectors,
			    "plot_vectors"))
    tidy_and_exit(-1);
  
  ufree(resource_str);

  /*
   * set up track_graphic option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
				    "track_graphic", TRACK_GRAPHIC);
  
  if (uset_triple_param(Glob->prog_name,
			"read_params",
			Glob->params_path_name,
			resource_str, &Glob->track_graphic,
			"false", FALSE,
			"ellipses", ELLIPSES,
			"polygons", POLYGONS,
			"track_graphic"))
    tidy_and_exit(-1);

  ufree(resource_str);
  
  /*
   * set up fill_graphic option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			       "fill_graphic",
			       FILL_GRAPHIC);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->fill_graphic,
			    "fill_graphic"))
    tidy_and_exit(-1);
  
  ufree(resource_str);

  /*
   * set up plot_runs option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			       "plot_runs",
			       PLOT_RUNS);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->plot_runs,
			    "plot_runs"))
    tidy_and_exit(-1);

  ufree(resource_str);

  /*
   * set up runs_included option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
				 "runs_included", RUNS_INCLUDED);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str, &Glob->runs_included,
			    "runs_included"))
    tidy_and_exit(-1);

  ufree(resource_str);
  
  /*
   * set up fill_runs option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			       "fill_runs",
			       FILL_RUNS);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->fill_runs,
			    "fill_runs"))
    tidy_and_exit(-1);

  ufree(resource_str);

  /*
   * forecast steps
   */
  
  Glob->n_forecast_steps = xGetResLong(Glob->rdisplay, Glob->prog_name,
				       "n_forecast_steps",
				       N_FORECAST_STEPS);
  
  Glob->forecast_interval =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "forecast_interval",
		  FORECAST_INTERVAL) * 60.0;
  
  /*
   * set up plot_current option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			       "plot_current",
			       PLOT_CURRENT);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->plot_current,
			    "plot_current"))
    tidy_and_exit(-1);
  
  ufree(resource_str);

  /*
   * set up plot_past option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			       "plot_past", PLOT_PAST);
  if (uset_triple_param(Glob->prog_name,
			"read_params",
			Glob->params_path_name,
			resource_str, &Glob->plot_past,
			"false", FALSE,
			"limited", PAST_LIMITED,
			"all", PAST_ALL,
			"plot_past"))
    tidy_and_exit(-1);
  
  ufree(resource_str);
  
  Glob->past_plot_period = xGetResDouble(Glob->rdisplay,
					 Glob->prog_name,
					 "past_plot_period",
					 PAST_PLOT_PERIOD) * 60.0;
  
  /*
   * set up plot_future option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			       "plot_future", PLOT_FUTURE);
  
  if (uset_triple_param(Glob->prog_name,
			"read_params",
			Glob->params_path_name,
			resource_str, &Glob->plot_future,
			"false", FALSE,
			"limited", FUTURE_LIMITED,
			"all", FUTURE_ALL,
			"plot_future"))
    tidy_and_exit(-1);
  
  ufree(resource_str);
  
  Glob->future_plot_period = xGetResDouble(Glob->rdisplay,
					   Glob->prog_name,
					   "future_plot_period",
					   FUTURE_PLOT_PERIOD) * 60.0;
  
  /*
   * set up plot_cappi_contours option
   */
  
  resource_str = xGetResString(Glob->rdisplay,
					  Glob->prog_name,
					  "plot_cappi_contours",
					  PLOT_CAPPI_CONTOURS);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->plot_cappi_contours,
			    "plot_cappi_contours"))
    tidy_and_exit(-1);

  ufree(resource_str);

  /*
   * set up plot_vsection_contours option
   */
  
  resource_str = xGetResString(Glob->rdisplay,
					     Glob->prog_name,
					     "plot_vsection_contours",
					     PLOT_VSECTION_CONTOURS);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->plot_vsection_contours,
			    "plot_vsection_contours"))
    tidy_and_exit(-1);

  ufree(resource_str);
  
  /*
   * set up vsection_interp option
   */
  
  resource_str = xGetResString(Glob->rdisplay,
				      Glob->prog_name,
				      "interpolate_vsection",
				      INTERPOLATE_VSECTION);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->vsection_interp, "vsection_interp"))
    tidy_and_exit(-1);

  ufree(resource_str);
  
  Glob->vsection_interp_res = xGetResDouble(Glob->rdisplay,
					    Glob->prog_name,
					    "vsection_interp_res",
					    VSECTION_INTERP_RES);
  
  /*
   * instance track server
   */
  
  Glob->servmap_host1 = xGetResString(Glob->rdisplay, Glob->prog_name,
				      "servmap_host1", SERVMAP_HOST1_STR);
  
  Glob->servmap_host2 = xGetResString(Glob->rdisplay, Glob->prog_name,
				      "servmap_host2", SERVMAP_HOST2_STR);
  
  Glob->track_server_subtype =
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "track_server_subtype", TRACK_SERVER_SUBTYPE);
  
  Glob->track_server_instance =
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "track_server_instance", TRACK_SERVER_INSTANCE);
  
  Glob->track_server_default_host =
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "track_server_default_host", TRACK_SERVER_DEFAULT_HOST);

  if (!strcmp(Glob->track_server_default_host, "local")) {
    Glob->track_server_default_host = PORThostname();
  }

  Glob->track_server_default_port =
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"track_server_default_port", TRACK_SERVER_DEFAULT_PORT);
  
  Glob->max_message_len =
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"max_message_len", MAX_MESSAGE_LEN);
  
  /*
   * set up cursor bearing options
   */
  
  resource_str = xGetResString(Glob->rdisplay,
			       Glob->prog_name,
			       "cursor_magnetic",
			       "true");
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->cursor_magnetic, "cursor_magnetic"))
    tidy_and_exit(-1);
  
  ufree(resource_str);

  Glob->magnetic_variation =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "magnetic_variation", 0.0);

  resource_str = xGetResString(Glob->rdisplay,
			       Glob->prog_name,
			       "cursor_dist_nm",
			       "false");
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    &Glob->cursor_dist_nm, "cursor_dist_nm"))
    tidy_and_exit(-1);
  
  ufree(resource_str);

  /*
   * if track data is not used, override setting of plot_tracks
   */
  
  if (Glob->use_track_data == FALSE)
    Glob->plot_tracks = FALSE;
  
}

/************************************************************************
 * load_psgc_linestyle
 *
 * loads up the width, dash length, space length and graylevel
 * into a postscript graphics context
 */

static void load_psgc_linestyle(psgc_t *psgc,
				char *params_str,
				char *def_str)

{
  
  char *resource_str;
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			       params_str, def_str);

  /*
   * If 4 fields, read in all 4
   */
  
  if (sscanf(resource_str, "%lg %lg %lg %lg",
	     &psgc->line_width,
	     &psgc->dash_length,
	     &psgc->space_length,
	     &psgc->graylevel) != 4) {

    /*
     * else if 3 fields, read in 3 and set graylevel to 0
     */

    if (sscanf(resource_str, "%lg %lg %lg",
	       &psgc->line_width,
	       &psgc->dash_length,
	       &psgc->space_length) != 3) {

      /*
       * else error
       */

      fprintf(stderr, "ERROR - %s:read_params:load_psgc_linestyle\n",
	      Glob->prog_name);
      fprintf(stderr, "Reading linestyle descriptor '%s'\n", resource_str);
      fprintf(stderr, "Check params file '%s'\n", Glob->params_path_name);
      tidy_and_exit(-1);

    } else {

      psgc->graylevel = 0.0;

    }

  }

  ufree(resource_str);

}
