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
 * read_params.c - time_hist routine
 *
 * reads the paramteres or X data base to get parameters
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "time_hist.h"

/*
 * file scope prototypes
 */

static void _load_tf_option(char *param_name, char *def_str,
			    int *option_p);

static void _load_dual_option(char *param_name, char *def_str,
			      int *option_p,
			      char *option_str_1, int option_val_1,
			      char *option_str_2, int option_val_2);

static void _load_triple_option(char *param_name, char *def_str,
				int *option_p,
				char *option_str_1, int option_val_1,
				char *option_str_2, int option_val_2,
				char *option_str_3, int option_val_3);

static void _load_psgc_linestyle(psgc_t *psgc,
				 char *params_str,
				 char *def_str);

static void _load_quad_option(char *param_name, char *def_str,
			      int *option_p,
			      char *option_str_1, int option_val_1,
			      char *option_str_2, int option_val_2,
			      char *option_str_3, int option_val_3,
			      char *option_str_4, int option_val_4);

/**************
 * main routine
 */

void read_params(void)

{
  
  double ps_max_print_width, ps_min_print_width;
  char *resource_str;
  int flipped;
  
  Glob->track_shmem_key = xGetResLong(Glob->rdisplay, Glob->prog_name,
				      "track_shmem_key", TRACK_SHMEM_KEY);
  
  /*
   * instance track server
   */
  
  Glob->servmap_host1 = xGetResString(Glob->rdisplay, Glob->prog_name,
				      "servmap_host1", SERVMAP_HOST1_STR);
  
  Glob->servmap_host2 = xGetResString(Glob->rdisplay, Glob->prog_name,
				      "servmap_host2", SERVMAP_HOST2_STR);
  
  Glob->max_message_len =
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"max_message_len", MAX_MESSAGE_LEN);
  
  /*
   * starting thist plot options
   */

  _load_tf_option("plot_thist_vol", "true",
		  &Glob->thist_field_active[THIST_VOL_FIELD]);
  
  _load_tf_option("plot_thist_area", "true",
		  &Glob->thist_field_active[THIST_AREA_FIELD]);
  
  _load_tf_option("plot_thist_pflux", "true",
		  &Glob->thist_field_active[THIST_PFLUX_FIELD]);
  
  _load_tf_option("plot_thist_mass", "true",
		  &Glob->thist_field_active[THIST_MASS_FIELD]);
  
  _load_tf_option("plot_thist_vil", "true",
		  &Glob->thist_field_active[THIST_VIL_FIELD]);
  
  _load_tf_option("thist_plot_fit", "false", &Glob->thist_fit);
  
  _load_triple_option("plot_thist_forecast", "limited",
		      &Glob->thist_forecast,
		      "false", FALSE,
		      "limited", SELECTED_LIMITED,
		      "all", SELECTED_ALL);

  /*
   * starting timeht plot options
   */

  _load_quad_option("timeht_mode", "maxz",
		    &Glob->timeht_mode,
		    "maxz", TIMEHT_MAXZ,
		    "meanz", TIMEHT_MEANZ,
		    "mass", TIMEHT_MASS,
		    "vort", TIMEHT_VORTICITY);

  _load_triple_option("timeht_centroids", "limited",
		      &Glob->timeht_centroids,
		      "false", FALSE,
		      "limited", SELECTED_LIMITED,
		      "all", SELECTED_ALL);

  _load_triple_option("timeht_htmaxz", "limited",
		      &Glob->timeht_htmaxz,
		      "false", FALSE,
		      "limited", SELECTED_LIMITED,
		      "all", SELECTED_ALL);

  /*
   * starting rdist plot options
   */
  
  _load_dual_option("rdist_mode", "vol",
		    &Glob->rdist_mode,
		    "vol", RDIST_VOL,
		    "area", RDIST_AREA);

  _load_tf_option("rdist_flipped", "false", &flipped);

  if (flipped) {
    Glob->rdist_sign = -1.0;
  } else {
    Glob->rdist_sign = 1.0;
  }
  
  /*
   * use case tracks??
   */
  
  _load_tf_option("use_case_tracks", "false", &Glob->use_case_tracks);
  
  Glob->case_tracks_file_path =
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "case_tracks_file_path", "none");
  
  /*
   * partial track times
   */
  
  Glob->partial_track_past_period = (int) 
    (xGetResDouble(Glob->rdisplay, Glob->prog_name,
		   "partial_track_past_period", 20.0) * 60.0 + 0.5);
  
  Glob->partial_track_future_period = (int)
    (xGetResDouble(Glob->rdisplay, Glob->prog_name,
		   "partial_track_future_period", 60.0) * 60.0 + 0.5);
  
  /*
   * X variables 
   */
  
  Glob->foregroundstr = xGetResString(Glob->rdisplay, Glob->prog_name,
				      "x_foreground", X_FOREGROUND);
  Glob->backgroundstr = xGetResString(Glob->rdisplay, Glob->prog_name,
				      "x_background", X_BACKGROUND);
  
  Glob->double_click_delay = xGetResDouble(Glob->rdisplay, Glob->prog_name,
					   "double_click_delay",
					   DOUBLE_CLICK_DELAY) * 1000.0;
  
  Glob->x_tscale_x = xGetResLong(Glob->rdisplay, Glob->prog_name,
				"x_tscale_x", X_TSCALE_X);
  Glob->x_tscale_y = xGetResLong(Glob->rdisplay, Glob->prog_name,
				"x_tscale_y", X_TSCALE_Y);

  Glob->x_tscale_width =
    (ui32) xGetResLong(Glob->rdisplay, Glob->prog_name,
			"x_tscale_width", X_TSCALE_WIDTH);
  Glob->x_tscale_height =
    (ui32) xGetResLong(Glob->rdisplay, Glob->prog_name,
			"x_tscale_height", X_TSCALE_HEIGHT);
  
  Glob->x_thist_x = xGetResLong(Glob->rdisplay, Glob->prog_name,
				"x_thist_x", X_THIST_X);
  Glob->x_thist_y = xGetResLong(Glob->rdisplay, Glob->prog_name,
				"x_thist_y", X_THIST_Y);

  Glob->x_thist_width = (ui32) xGetResLong(Glob->rdisplay, Glob->prog_name,
					    "x_thist_width", X_THIST_WIDTH);
  Glob->x_thist_height = (ui32) xGetResLong(Glob->rdisplay, Glob->prog_name,
					     "x_thist_height", X_THIST_HEIGHT);
  
  Glob->x_timeht_x = xGetResLong(Glob->rdisplay, Glob->prog_name,
				 "x_timeht_x", X_TIMEHT_X);
  Glob->x_timeht_y = xGetResLong(Glob->rdisplay, Glob->prog_name,
				 "x_timeht_y", X_TIMEHT_Y);

  Glob->x_timeht_width =
    (ui32) xGetResLong(Glob->rdisplay, Glob->prog_name,
			"x_timeht_width", X_TIMEHT_WIDTH);
  Glob->x_timeht_height =
    (ui32) xGetResLong(Glob->rdisplay, Glob->prog_name,
			"x_timeht_height", X_TIMEHT_HEIGHT);
  
  Glob->x_rdist_x = xGetResLong(Glob->rdisplay, Glob->prog_name,
				"x_rdist_x", X_RDIST_X);
  Glob->x_rdist_y = xGetResLong(Glob->rdisplay, Glob->prog_name,
				"x_rdist_y", X_RDIST_Y);

  Glob->x_rdist_width =
    (ui32) xGetResLong(Glob->rdisplay, Glob->prog_name,
			"x_rdist_width", X_RDIST_WIDTH);
  Glob->x_rdist_height =
    (ui32) xGetResLong(Glob->rdisplay, Glob->prog_name,
			"x_rdist_height", X_RDIST_HEIGHT);
  
  Glob->x_help_x = xGetResLong(Glob->rdisplay, Glob->prog_name,
			       "x_help_x", X_HELP_X);
  Glob->x_help_y = xGetResLong(Glob->rdisplay, Glob->prog_name,
			       "x_help_y", X_HELP_Y);

  Glob->x_help_width =
    (ui32) xGetResLong(Glob->rdisplay, Glob->prog_name,
			"x_help_width", X_HELP_WIDTH);
  Glob->x_help_height =
    (ui32) xGetResLong(Glob->rdisplay, Glob->prog_name,
			"x_help_height", X_HELP_HEIGHT);
  
  Glob->x_timeht_scale_width = (ui32)
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"x_timeht_scale_width",	X_TIMEHT_SCALE_WIDTH);
  
  Glob->x_rdist_scale_width = (ui32)
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"x_rdist_scale_width", X_RDIST_SCALE_WIDTH);
  
  Glob->x_max_button_width = (ui32)
    xGetResLong(Glob->rdisplay, Glob->prog_name, "x_max_button_width",
		X_MAX_BUTTON_WIDTH);
  
  Glob->x_main_border_width = 
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"x_main_border_width", X_MAIN_BORDER_WIDTH);
  
  Glob->x_sub_border_width = 
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"x_sub_border_width", X_SUB_BORDER_WIDTH);
  
  /*
   * margins and clearances - pixels in X
   */
  
  Glob->x_xaxis_margin =
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"x_xaxis_margin", X_XAXIS_MARGIN);
  
  Glob->x_tscale_yaxis_margin =
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"x_tscale_yaxis_margin", X_TSCALE_YAXIS_MARGIN);
  
  Glob->x_thist_yaxis_margin =
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"x_thist_yaxis_margin", X_THIST_YAXIS_MARGIN);
  
  Glob->x_timeht_yaxis_margin =
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"x_timeht_yaxis_margin", X_TIMEHT_YAXIS_MARGIN);
  
  Glob->x_rdist_yaxis_margin =
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"x_rdist_yaxis_margin", X_RDIST_YAXIS_MARGIN);
  
  Glob->x_top_margin =
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"x_top_margin", X_TOP_MARGIN);
  
  Glob->x_text_margin =
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"x_text_margin", X_TEXT_MARGIN);
  
  Glob->x_xaxis_endmargin =
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"x_xaxis_endmargin", X_XAXIS_ENDMARGIN);
  
  Glob->x_yaxis_endmargin =
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"x_yaxis_endmargin", X_YAXIS_ENDMARGIN);
  
  Glob->x_tick_length =
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"x_tick_length", X_TICK_LENGTH);
  
  Glob->x_header_clearance =
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"x_header_clearance", X_HEADER_CLEARANCE);
  
  /*
   * set up title copy
   */
  
  _load_tf_option("draw_copy_title", "true", &Glob->draw_copy_title);
  
  /*
   * postscript variables
   */
  
  Glob->ps_unitscale = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				     "ps_unitscale", PS_UNITSCALE);
  
  Glob->ps_title_fontsize = xGetResDouble(Glob->rdisplay,
					  Glob->prog_name,
					  "ps_title_fontsize",
					  PS_TITLE_FONTSIZE);
  
  Glob->ps_scale_fontsize = xGetResDouble(Glob->rdisplay, Glob->prog_name,
					  "ps_scale_fontsize",
					  PS_SCALE_FONTSIZE);
  
  Glob->ps_ticklabel_fontsize = xGetResDouble(Glob->rdisplay, Glob->prog_name,
					      "ps_ticklabel_fontsize",
					      PS_TICKLABEL_FONTSIZE);
  
  Glob->ps_header_fontsize = xGetResDouble(Glob->rdisplay, Glob->prog_name,
					   "ps_header_fontsize",
					   PS_HEADER_FONTSIZE);
  
  Glob->ps_fontname = xGetResString(Glob->rdisplay, Glob->prog_name,
				    "ps_fontname", PS_FONTNAME);
  
  Glob->ps_page_width =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ps_page_width", PS_PAGE_WIDTH);
  
  Glob->ps_page_length =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ps_page_length", PS_PAGE_LENGTH);
  
  Glob->ps_width_margin =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ps_width_margin", PS_WIDTH_MARGIN);
  
  Glob->ps_length_margin =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ps_length_margin", PS_LENGTH_MARGIN);
  
  Glob->ps_plot_height = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				       "ps_plot_height",
				       PS_PLOT_HEIGHT) * Glob->ps_unitscale;
  
  Glob->ps_plot_width = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				      "ps_plot_width",
				      PS_PLOT_WIDTH) * Glob->ps_unitscale;
  
  Glob->ps_timeht_scale_width =
    xGetResDouble(Glob->rdisplay, Glob->prog_name, "ps_timeht_scale_width",
		  PS_TIMEHT_SCALE_WIDTH) * Glob->ps_unitscale;
  
  Glob->ps_rdist_scale_width =
    xGetResDouble(Glob->rdisplay, Glob->prog_name, "ps_rdist_scale_width",
		  PS_RDIST_SCALE_WIDTH) * Glob->ps_unitscale;
  
  Glob->ps_title_height =
    xGetResDouble(Glob->rdisplay, Glob->prog_name, "ps_title_height",
		  PS_TITLE_HEIGHT) * Glob->ps_unitscale;
  
  Glob->ps_title_to_plot_margin =
    xGetResDouble(Glob->rdisplay, Glob->prog_name, "ps_title_to_plot_margin",
		  PS_TITLE_TO_PLOT_MARGIN) * Glob->ps_unitscale;
  
  Glob->ps_plot_to_scale_margin =
    xGetResDouble(Glob->rdisplay, Glob->prog_name, "ps_plot_to_scale_margin",
		  PS_PLOT_TO_SCALE_MARGIN) * Glob->ps_unitscale;
  
  Glob->print_width = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				    "ps_print_width", PS_PRINT_WIDTH);
  
  ps_max_print_width = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				     "ps_max_print_width", PS_MAX_PRINT_WIDTH);
  ps_min_print_width = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				     "ps_min_print_width", PS_MIN_PRINT_WIDTH);
  
  Glob->ps_border_line_width = 
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ps_border_line_width", PS_BORDER_LINE_WIDTH);
  
  Glob->ps_tick_line_width = 
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ps_tick_line_width", PS_TICK_LINE_WIDTH);
  
  Glob->ps_divider_line_width = 
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ps_divider_line_width", PS_DIVIDER_LINE_WIDTH);
  
  Glob->ps_crosshair_line_width = 
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ps_crosshair_line_width", PS_CROSSHAIR_WIDTH);
  
  /*
   * margins and clearances - inches in postscript
   */
  
  Glob->ps_xaxis_margin =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ps_xaxis_margin", PS_XAXIS_MARGIN) *
		    Glob->ps_unitscale;
  
  Glob->ps_tscale_yaxis_margin =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ps_tscale_yaxis_margin", PS_TSCALE_YAXIS_MARGIN) *
		    Glob->ps_unitscale;
  
  Glob->ps_thist_yaxis_margin =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ps_thist_yaxis_margin", PS_THIST_YAXIS_MARGIN) *
		    Glob->ps_unitscale;
  
  Glob->ps_timeht_yaxis_margin =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ps_timeht_yaxis_margin", PS_TIMEHT_YAXIS_MARGIN) *
		    Glob->ps_unitscale;
  
  Glob->ps_rdist_yaxis_margin =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ps_rdist_yaxis_margin", PS_RDIST_YAXIS_MARGIN) *
		    Glob->ps_unitscale;
  
  Glob->ps_top_margin =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ps_top_margin", PS_TOP_MARGIN) *
		    Glob->ps_unitscale;
  
  Glob->ps_text_margin =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ps_text_margin", PS_TEXT_MARGIN) *
		    Glob->ps_unitscale;
  
  Glob->ps_xaxis_endmargin =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ps_xaxis_endmargin", PS_XAXIS_ENDMARGIN) *
		    Glob->ps_unitscale;
  
  Glob->ps_yaxis_endmargin =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ps_yaxis_endmargin", PS_YAXIS_ENDMARGIN) *
		    Glob->ps_unitscale;
  
  Glob->ps_tick_length =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ps_tick_length", PS_TICK_LENGTH) *
		    Glob->ps_unitscale;
  
  Glob->ps_header_clearance =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ps_header_clearance", PS_HEADER_CLEARANCE) *
		    Glob->ps_unitscale;
  
  /*
   * load up ps linestyles
   */
  
  _load_psgc_linestyle(&Glob->thist_graph_psgc[THIST_VOL_FIELD],
		       "ps_thist_vol_linestyle",
		       PS_THIST_VOL_LINESTYLE);
  
  _load_psgc_linestyle(&Glob->thist_graph_psgc[THIST_AREA_FIELD],
		       "ps_thist_area_linestyle",
		       PS_THIST_AREA_LINESTYLE);
  
  _load_psgc_linestyle(&Glob->thist_graph_psgc[THIST_PFLUX_FIELD],
		       "ps_thist_pflux_linestyle",
		       PS_THIST_PFLUX_LINESTYLE);
  
  _load_psgc_linestyle(&Glob->thist_graph_psgc[THIST_MASS_FIELD],
		       "ps_thist_mass_linestyle",
		       PS_THIST_MASS_LINESTYLE);
  
  _load_psgc_linestyle(&Glob->thist_graph_psgc[THIST_VIL_FIELD],
		       "ps_thist_vil_linestyle",
		       PS_THIST_VIL_LINESTYLE);
  
  _load_psgc_linestyle(&Glob->thist_forecast_psgc,
		       "ps_thist_forecast_linestyle",
		       PS_THIST_FORECAST_LINESTYLE);
  
  _load_psgc_linestyle(&Glob->ht_maxdbz_psgc,
		       "ps_ht_maxdbz_linestyle",
		       PS_HT_MAXDBZ_LINESTYLE);
  
  _load_psgc_linestyle(&Glob->ht_centroid_psgc,
		       "ps_ht_centroid_linestyle",
		       PS_HT_CENTROID_LINESTYLE);
  
  _load_psgc_linestyle(&Glob->top_base_psgc,
		       "ps_top_base_linestyle",
		       PS_TOP_BASE_LINESTYLE);
  
  _load_psgc_linestyle(&Glob->ht_refl_centroid_psgc,
		       "ps_ht_refl_centroid_linestyle",
		       PS_HT_REFL_CENTROID_LINESTYLE);
  
  _load_psgc_linestyle(&Glob->top_base_psgc,
		       "ps_top_base_linestyle",
		       PS_TOP_BASE_LINESTYLE);
  
  /*
   * check print width
   */
  
  if (Glob->print_width < ps_min_print_width) {
    
    fprintf(stderr, "WARNING - %s:read_params.\n", Glob->prog_name);
    fprintf(stderr, "Print width set out of range, to %f\n",
	    Glob->print_width);
    fprintf(stderr, "Check params file %s\n", Glob->params_path_name);
    fprintf(stderr, "Acceptable range is %f to %f.\n",
	    ps_min_print_width, ps_max_print_width);
    fprintf(stderr, "Setting print width to %f.\n", ps_min_print_width);
    Glob->print_width = ps_min_print_width;
    
  }
  
  if (Glob->print_width > ps_max_print_width) {
    
    fprintf(stderr, "WARNING - %s:read_params.\n", Glob->prog_name);
    fprintf(stderr, "Print width set out of range, to %f\n",
	    Glob->print_width);
    fprintf(stderr, "Check params file %s\n", Glob->params_path_name);
    fprintf(stderr, "Acceptable range is %f to %f.\n",
	    ps_max_print_width, ps_max_print_width);
    fprintf(stderr, "Setting print width to %f.\n", ps_max_print_width);
    Glob->print_width = ps_max_print_width;
    
  }
  
  /*
   * set up x synchronization option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			     "x_sync", X_SYNC);
  
  if (!strcmp(resource_str, "yes") || !strcmp(resource_str, "true")) {
    Glob->x_sync = TRUE;
  } else if (!strcmp(resource_str, "no") || !strcmp(resource_str, "false")) {
    Glob->x_sync = FALSE;
  } else {
    fprintf(stderr, "ERROR - %s:read_params\n", Glob->prog_name);
    fprintf(stderr, "'x_sync' option '%s' not recognized.\n",
	    resource_str);
    fprintf(stderr, "Valid options are 'true', 'yes', 'false' or 'no'\n");
    fprintf(stderr, "Check params file '%s'\n", Glob->params_path_name);
    tidy_and_exit(1);
  }
  
  /*
   * set up realtime_only option
   */
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			     "realtime_only", "true");
  
  if (!strcmp(resource_str, "yes") ||
      !strcmp(resource_str, "true")) {
    Glob->realtime_only = TRUE;
  } else if (!strcmp(resource_str, "no") ||
	     !strcmp(resource_str, "false")) {
    Glob->realtime_only = FALSE;
  } else {
    fprintf(stderr, "ERROR - %s:read_params\n", Glob->prog_name);
    fprintf(stderr, "'realtime_only' option '%s' not recognized.\n",
	    resource_str);
    fprintf(stderr, "Valid options are 'true', 'yes', 'false' or 'no'\n");
    fprintf(stderr, "Check params file '%s'\n", Glob->params_path_name);
    tidy_and_exit(1);
  }
  
}

/************************************************************************
 * _load_psgc_linestyle
 *
 * loads up the width, dash length and space length into a
 * postscript graphics context
 */

void _load_psgc_linestyle(psgc_t *psgc,
			  char *params_str,
			  char *def_str)

{
  
  int error_flag = FALSE;
  char *linestyle_str;
  char *start_pt, *end_pt;
  
  linestyle_str = xGetResString(Glob->rdisplay, Glob->prog_name,
				params_str, def_str);
  
  start_pt = linestyle_str;
  errno = 0;
  psgc->line_width = strtod(start_pt, &end_pt);
  if (errno != 0)
    error_flag = TRUE;
  
  start_pt = end_pt;
  errno = 0;
  psgc->dash_length = strtod(start_pt, &end_pt);
  if (errno != 0)
    error_flag = TRUE;
  
  start_pt = end_pt;
  errno = 0;
  psgc->space_length = strtod(start_pt, &end_pt);
  if (errno != 0)
    error_flag = TRUE;
  
  if (error_flag == TRUE) {
    
    fprintf(stderr, "ERROR - %s:read_params:_load_psgc_linestyle\n",
	    Glob->prog_name);
    fprintf(stderr, "Reading linestyle descriptor '%s'\n", linestyle_str);
    fprintf(stderr, "Check params file '%s'\n", Glob->params_path_name);
    tidy_and_exit(1);
    
  }
  
}

/***************************
 * set up true-false option
 */
  
static void _load_tf_option(char *param_name, char *def_str,
			    int *option_p)

{
  
  char *resource_str;
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			       param_name, def_str);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str,
			    option_p,
			    param_name)) {
    tidy_and_exit(-1);
  }
  
  ufree(resource_str);

}

/***********************
 * set up dual option
 */
  
static void _load_dual_option(char *param_name, char *def_str,
			      int *option_p,
			      char *option_str_1, int option_val_1,
			      char *option_str_2, int option_val_2)

{

  char *resource_str;

  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			       param_name, def_str);
  
  if (uset_double_param(Glob->prog_name,
			"read_params",
			Glob->params_path_name,
			resource_str, option_p,
			option_str_1, option_val_1,
			option_str_2, option_val_2,
			param_name)) {
    tidy_and_exit(-1);
  }

  ufree(resource_str);

}

/***********************
 * set up triple option
 */
  
static void _load_triple_option(char *param_name, char *def_str,
				int *option_p,
				char *option_str_1, int option_val_1,
				char *option_str_2, int option_val_2,
				char *option_str_3, int option_val_3)

{

  char *resource_str;

  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			       param_name, def_str);
  
  if (uset_triple_param(Glob->prog_name,
			"read_params",
			Glob->params_path_name,
			resource_str, option_p,
			option_str_1, option_val_1,
			option_str_2, option_val_2,
			option_str_3, option_val_3,
			param_name)) {
    tidy_and_exit(-1);
  }

  ufree(resource_str);

}

/***********************
 * set up quad option
 */
  
static void _load_quad_option(char *param_name, char *def_str,
			      int *option_p,
			      char *option_str_1, int option_val_1,
			      char *option_str_2, int option_val_2,
			      char *option_str_3, int option_val_3,
			      char *option_str_4, int option_val_4)
     
{
  
  char *resource_str;
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
			       param_name, def_str);
  
  if (uset_quad_param(Glob->prog_name,
		      "read_params",
		      Glob->params_path_name,
		      resource_str, option_p,
		      option_str_1, option_val_1,
		      option_str_2, option_val_2,
		      option_str_3, option_val_3,
		      option_str_4, option_val_4,
		      param_name)) {
    tidy_and_exit(-1);
  }

  ufree(resource_str);

}

  
  

