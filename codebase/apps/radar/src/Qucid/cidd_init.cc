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
/************************************************************************
 * CIDD_INIT.CC: Routines that read initialization files, set up
 *         communications between processes, etc.,
 *
 * For the Configurable Interactive Data Display (CIDD)
 * Frank Hage   July 1991 NCAR, Research Applications Program
 *  Special thanks to Mike Dixon. Without his guidance, support and
 * friendship, CIDD would not have been possible.
 */

#define CIDD_INIT    1
#include "cidd.h"
#include <algorithm>
static void _init_grids();

// #define NUM_PARSE_FIELDS    64
// #define PARSE_FIELD_SIZE    1024
// #define INPUT_LINE_LEN      512

/*****************************************************************
 * INIT_DATA_SPACE : Init all globals and set up defaults
 */

void init_data_space()
{

  int num_fields;
  long param_text_len;
  long param_text_line_no;
  const char *param_text;
  const char *resource;
  const char *field_str;
  char str_buf[128];   /* Space to build resource strings */
  char *cfield[3];     /* Space to collect sub strings */
  double delta_x,delta_y;
  UTIMstruct temp_utime;

  if(!gd.quiet_mode) {
    fprintf(stderr,"Qucid: Version %s\n", CIDD_VERSION);
    fprintf(stderr,"copyright %s\n\n", CIDD_UCOPYRIGHT);
  }
  
  // gd.debug |= gd.uparams->getLong("cidd.debug_flag", 0);
  // gd.debug1 |= gd.uparams->getLong("cidd.debug1_flag", 0);
  // gd.debug2 |= gd.uparams->getLong("cidd.debug2_flag", 0);

  gd.debug |= _params.debug;
  gd.debug1 |= _params.debug1_flag;
  gd.debug2 |= _params.debug2_flag;
  
  // open shmem segment for interprocess comms
  
  gd.coord_key = _params.coord_key;
  if((gd.coord_expt = (coord_export_t *) ushm_create(gd.coord_key,
                                                     sizeof(coord_export_t),
                                                     0666)) == NULL) {
    fprintf(stderr, "Could not create shared mem for interprocess comms\n");
    exit(-1);
  }
  memset(gd.coord_expt, 0, sizeof(coord_export_t));

  // html mode
  
  if(_params.run_once_and_exit) _params.html_mode = pTRUE;
  if (_params.html_mode) {
    gd.h_win.zoom_level = 0;
  }
  
  // image generation
  
  STRcopy(gd.h_win.image_dir, _params.image_dir, MAX_PATH_LEN);
  STRcopy(gd.v_win.image_dir, _params.image_dir, MAX_PATH_LEN);

  
  STRcopy(gd.h_win.image_fname, _params.horiz_image_fname, MAX_PATH_LEN);
  STRcopy(gd.h_win.image_command, _params.horiz_image_command, MAX_PATH_LEN);

  STRcopy(gd.v_win.image_fname, _params.vert_image_fname, MAX_PATH_LEN);
  STRcopy(gd.v_win.image_command, _params.vert_image_command, MAX_PATH_LEN);

  // If individual horiz and vert scripts have not been set
  // use the general one.
  
  if(strlen(gd.h_win.image_command) < 3) {
    STRcopy(gd.h_win.image_command, _params.image_convert_script, MAX_PATH_LEN);
  }
  
  if(strlen(gd.v_win.image_command) < 3) {
    STRcopy(gd.v_win.image_command, _params.image_convert_script, MAX_PATH_LEN);
  }

  if(_params.idle_reset_seconds <= 0 || _params.html_mode == 1) {
    _params.idle_reset_seconds = 1000000000; // a very long time
  }

  // layers
  
  gd.layers.layer_legends_on = _params.layer_legends_on;
  gd.layers.cont_legends_on = _params.cont_legends_on;
  gd.layers.wind_legends_on = _params.wind_legends_on;
  gd.layers.contour_line_width = _params.contour_line_width;
  gd.layers.smooth_contours = _params.smooth_contours;
  gd.layers.use_alt_contours = _params.use_alt_contours;
  gd.layers.add_noise = _params.add_noise;
  gd.layers.special_contour_value = _params.special_contour_value;
  gd.layers.map_bad_to_min_value = _params.map_bad_to_min_value;
  gd.layers.map_missing_to_min_value = _params.map_missing_to_min_value;

  // products
  
  gd.prod.products_on = _params.products_on;
  gd.prod.prod_line_width = _params.product_line_width;
  gd.prod.prod_font_num = _params.product_font_size;
  
  for(int ii = 0; ii < _params.product_adjustments_n; ii++) {
    gd.prod.detail[ii].threshold =
      _params._product_adjustments[ii].threshold;
    gd.prod.detail[ii].adjustment =
      _params._product_adjustments[ii].font_index_adj;
  } // ii


  // if domain follows data, do not clip or decimate
  
  if (_params.domain_follows_data) {
    _params.always_get_full_domain = pTRUE;
    _params.do_not_clip_on_mdv_request = pTRUE;
    _params.do_not_decimate_on_mdv_request = pTRUE;
  }

  // Bookmarks for a menu of URLS - Index starts at 1
  
  if(_params.bookmarks_n > 0) {
    gd.bookmark = (bookmark_t *)  calloc(sizeof(bookmark_t),_params.bookmarks_n);
  }
  for(int ii = 0; ii < _params.bookmarks_n; ii++) {
    gd.bookmark[ii].url = strdup(_params._bookmarks[ii].url);
    gd.bookmark[ii].label = strdup(_params._bookmarks[ii].label);
  }

  // origin latitude and longitude
  
  gd.h_win.origin_lat = _params.origin_latitude;
  gd.h_win.origin_lon = _params.origin_longitude;

  // click location on reset
  
  gd.h_win.reset_click_lat = _params.reset_click_latitude;
  gd.h_win.reset_click_lon = _params.reset_click_longitude;

  // projection
  

  // _params.aspect_ratio = gd.uparams->getDouble("cidd.aspect_ratio", 1.0);
  // if (_params.aspect_ratio <= 0.0 && gd.debug) {
  //   cerr << "WARNING - Using first domain to set aspect ratio: " << endl;
  // }

  // _params.scale_units_per_km = gd.uparams->getDouble("cidd.scale_units_per_km",1.0);
  // _params.scale_units_label = gd.uparams->getString("cidd.scale_units_label", "km");
  
  /* Establish the native projection type */

  //_params.projection_type = gd.uparams->getString("cidd.projection_type", "CARTESIAN");
  
  if (strncasecmp(_params.proj_type_str, "CARTESIAN", 9) == 0) {

    _params.proj_type = Params::PROJ_FLAT;
    gd.display_projection = Mdvx::PROJ_FLAT;
    gd.proj_param[0] = _params.proj_rotation; // rotation rel to TN
    gd.proj.initFlat(_params.origin_latitude,
                     _params.origin_longitude,
                     _params.proj_rotation);
    if(gd.debug) {
      printf("Cartesian Projection\n");
      printf("Origin at: %g, %g\n", 
             _params.origin_latitude,_params.origin_longitude);
    }
    
  } else if (strncasecmp(_params.proj_type_str, "LAT_LON", 7) == 0) {

    _params.proj_type = Params::PROJ_LATLON;
    gd.display_projection = Mdvx::PROJ_LATLON;
    gd.proj.initLatlon(_params.origin_longitude);
    if(gd.debug) {
      printf("LATLON/Cylindrical Projection\n");
      printf("Origin at: %g, %g\n",
             _params.origin_latitude, _params.origin_longitude);
    }
    
  } else if (strncasecmp(_params.proj_type_str, "LAMBERT", 7) == 0) {

    _params.proj_type = Params::PROJ_LAMBERT_CONF;
    gd.display_projection = Mdvx::PROJ_LAMBERT_CONF;
    gd.proj_param[0] = _params.proj_lat1;
    gd.proj_param[1] = _params.proj_lat2;
    gd.proj.initLambertConf(_params.origin_latitude,
                            _params.origin_longitude,
                            _params.proj_lat1,
                            _params.proj_lat2);
    if(gd.debug) {
      printf("LAMBERT Projection\n");
      printf("Origin at: %g, %g\n",
             _params.origin_latitude, _params.origin_longitude);
      printf("Parallels at: %g, %g\n",
             _params.proj_lat1, _params.proj_lat2);
    }
    
  } else if (strncasecmp(_params.proj_type_str, "STEREOGRAPHIC", 13) == 0) {

    _params.proj_type = Params::PROJ_OBLIQUE_STEREO;
    gd.display_projection = Mdvx::PROJ_OBLIQUE_STEREO;
    gd.proj_param[0] = _params.proj_tangent_lat;
    gd.proj_param[1] = _params.proj_tangent_lon;
    gd.proj_param[2] = _params.proj_central_scale;
    gd.proj.initStereographic(_params.proj_tangent_lat,
                              _params.proj_tangent_lon,
                              _params.proj_central_scale);
    gd.proj.setOffsetOrigin(_params.origin_latitude,
                            _params.origin_longitude);
    if(gd.debug) {
      printf("Oblique Stereographic Projection\n");
      printf("Origin at: %g, %g\n",
             _params.origin_latitude,_params.origin_longitude);
      printf("Tangent at: %g, %g\n",
             _params.proj_tangent_lat, _params.proj_tangent_lon);
    }
    
  } else if (strncasecmp(_params.proj_type_str, "POLAR_STEREO", 12) == 0) {
    
    _params.proj_type = Params::PROJ_POLAR_STEREO;
    gd.display_projection = Mdvx::PROJ_POLAR_STEREO;
    gd.proj_param[0] = _params.proj_tangent_lat;
    gd.proj_param[1] = _params.proj_tangent_lon;
    gd.proj_param[2] = _params.proj_central_scale;
    gd.proj.initPolarStereo
      (_params.proj_tangent_lon,
       (Mdvx::pole_type_t) (_params.proj_tangent_lat < 0.0 ?
                            Mdvx::POLE_SOUTH : Mdvx::POLE_NORTH),
       _params.proj_central_scale);
    gd.proj.setOffsetOrigin(_params.origin_latitude, _params.origin_longitude);
    if(gd.debug) {
      printf("Polar Stereographic Projection\n");
      printf("Origin at: %g, %g\n",
             _params.origin_latitude, _params.origin_longitude);
      printf("Tangent at: %g, %g\n",
             _params.proj_tangent_lat, _params.proj_tangent_lon);
    }

  } else if (strncasecmp(_params.proj_type_str,"MERCATOR",8) == 0) {
    
    _params.proj_type = Params::PROJ_MERCATOR;
    gd.display_projection = Mdvx::PROJ_MERCATOR;
    gd.proj.initMercator(_params.origin_latitude,_params.origin_longitude);
    if(gd.debug) {
      printf("MERCATOR Projection\n");
      printf("Origin at: %g, %g\n",
             _params.origin_latitude, _params.origin_longitude);
    }
      
  } else {

    fprintf(stderr,"Unknown projection type: cidd.proj_type_str = %s !\n",
            _params.proj_type_str);
    fprintf(stderr,"Current valid types are:\n");
    fprintf(stderr,"CARTESIAN, LAT_LON, LAMBERT, STEREOGRAPHIC, POLAR_STEREO, MERCATOR\n");
    exit(-1);

  }
  
  gd.h_win.last_page = -1;
  gd.v_win.last_page = -1;

  // movies

  int pid = getpid();
  for(int ii = 0; ii < MAX_FRAMES; ii++) {
    sprintf(gd.movie.frame[ii].fname,
            "%s/cidd_im%d_%d.",
            _params.image_dir, pid, ii);
    gd.movie.frame[ii].h_xid = 0;
    gd.movie.frame[ii].v_xid = 0;
    gd.movie.frame[ii].redraw_horiz = 1;
    gd.movie.frame[ii].redraw_vert = 1;
  }

  // copy movie info to other globals

  gd.movie.movie_on = _params.movie_on;
  if(_params.html_mode) {
    gd.movie.movie_on = 0;
  }
  gd.movie.magnify_factor = _params.movie_magnify_factor;
  gd.movie.time_interval = _params.time_interval;
  gd.movie.frame_span = _params.frame_span;
  gd.movie.num_frames = _params.starting_movie_frames;
  gd.movie.reset_frames = _params.reset_frames;
  gd.movie.delay = _params.movie_delay;
  gd.movie.forecast_interval = _params.forecast_interval;
  gd.movie.past_interval = _params.past_interval;
  gd.movie.mr_stretch_factor = _params.time_search_stretch_factor;
  gd.movie.round_to_seconds = _params.temporal_rounding;
  gd.movie.display_time_msec = _params.movie_speed_msec;

  gd.movie.start_frame = 0;
  gd.movie.sweep_on = 0;
  gd.movie.sweep_dir = 1;
  gd.movie.end_frame = gd.movie.num_frames -1 ;
  gd.movie.cur_frame = gd.movie.num_frames -1;
  gd.movie.last_frame = gd.movie.cur_frame;

  // climatology mode for movies
  
  gd.movie.climo_mode = REGULAR_INTERVAL;
  if(strncmp(_params.climo_mode, "daily", 5) == 0) {
    gd.movie.climo_mode = DAILY_INTERVAL;
  }
  if(strncmp(_params.climo_mode, "yearly",6) == 0) {
    gd.movie.climo_mode = YEARLY_INTERVAL;
  }

  /* Use cosine correction for computing range in polar data */

  if (gd.use_cosine_correction < 0) {
    // not set on command line
    gd.use_cosine_correction = _params.use_cosine_correction;
  }
  
  // IF demo_time is set in the params
  // Set into Archive Mode at the indicated time.

  /* If demo time param is not set and command line option hasn't set archive mode */

  if(strlen(_params.demo_time) < 8 && (gd.movie.mode != ARCHIVE_MODE)) {
    
    /* REALTIME MODE */

    gd.movie.mode = REALTIME_MODE;
    gd.coord_expt->runtime_mode = RUNMODE_REALTIME;
    gd.coord_expt->time_seq_num++;
    gd.forecast_mode = 0;
    
    /* set the first index's time based on current time  */
    time_t clock = time(0);    /* get current time */
    gd.movie.start_time =
      (time_t) (clock - ((gd.movie.num_frames -1) * gd.movie.time_interval * 60.0));
    gd.movie.start_time -= (gd.movie.start_time % gd.movie.round_to_seconds);
    gd.movie.demo_time = 0; // Indicated REAL-TIME is Native
    
    UTIMunix_to_date(gd.movie.start_time,&temp_utime);
    gd.movie.demo_mode = 0;

  } else {

    /* ARCHIVE MODE */
    
    if(gd.movie.mode != ARCHIVE_MODE) { /* if not set by command line args */
      
      gd.movie.mode = ARCHIVE_MODE;     /* time_series */
      gd.coord_expt->runtime_mode = RUNMODE_ARCHIVE;
      gd.coord_expt->time_seq_num++;
      
      parse_string_into_time(_params.demo_time,&temp_utime);
      UTIMdate_to_unix(&temp_utime);
      /* set the first index's time  based on indicated time */
      gd.movie.start_time = temp_utime.unix_time;
      
    }

    gd.movie.demo_mode = 1;
	 
    /* Adjust the start time downward to the nearest round interval seconds */

    gd.movie.start_time -= (gd.movie.start_time % gd.movie.round_to_seconds);

    if(_params.gather_data_mode == CLOSEST_TO_FRAME_CENTER) {
      // Offset movie frame by 1/2 frame interval so that interest time
      // lies on frame mid point
      gd.movie.start_time -=  (time_t) (gd.movie.time_interval * 30.0);
    }

    gd.movie.demo_time = gd.movie.start_time;
    UTIMunix_to_date(gd.movie.start_time,&temp_utime);

    gd.h_win.movie_page = gd.h_win.page;
    gd.v_win.movie_page = gd.v_win.page;
    gd.movie.cur_frame = 0;

  } // if(strlen(_params.demo_time) < 8 && (gd.movie.mode != ARCHIVE_MODE))

  reset_time_points(); // reset movie

  // _params.image_fill_threshold =
  //   gd.uparams->getLong("cidd.image_fill_threshold", 120000);

  // _params.dynamic_contour_threshold =
  //   gd.uparams->getLong("cidd.dynamic_contour_threshold", 160000);

  // _params.image_inten = gd.uparams->getDouble("cidd.image_inten", 0.8);
  // _params.inten_levels = gd.uparams->getLong("cidd.inten_levels", 32);
  // _params.data_inten = gd.uparams->getDouble("cidd.data_inten", 1.0);

  // _params.data_timeout_secs = gd.uparams->getLong("cidd.data_timeout_secs", 10);

  // data compression from server
  
  // _params.request_compressed_data = gd.uparams->getLong("cidd.request_compressed_data",0);
  // _params.request_gzip_vol_compression = gd.uparams->getLong("cidd.request_gzip_vol_compression",0);

  // output image file names
  
  // _params.add_frame_num_to_filename = gd.uparams->getLong("cidd.add_frame_num_to_filename",1);
  // _params.add_button_name_to_filename = gd.uparams->getLong("cidd.add_button_name_to_filename",0);
  // _params.add_height_to_filename = gd.uparams->getLong("cidd.add_height_to_filename",0);
  // _params.add_frame_time_to_filename = gd.uparams->getLong("cidd.add_frame_time_to_filename",1);
  // _params.add_gen_time_to_filename = gd.uparams->getLong("cidd.add_gen_time_to_filename",0);
  // _params.add_valid_time_to_filename = gd.uparams->getLong("cidd.add_valid_time_to_filename",0);
  // _params.font_display_mode = gd.uparams->getLong("cidd.font_display_mode",1);
  // _params.label_contours = gd.uparams->getLong("cidd.label_contours",1);

  // margins
  
  // _params.top_margin_render_style = gd.uparams->getLong("cidd.top_margin_render_style", 1);


#ifdef CHECK_LATER
  if(_params.html_mode || gd.movie.num_frames < 3 ) {
    _params.bot_margin_render_style = gd.uparams->getLong("cidd.bot_margin_render_style", 1);
  } else {
    _params.bot_margin_render_style = gd.uparams->getLong("cidd.bot_margin_render_style", 2);
  }
#endif

  // caching zooms to go back to - sanity check

  if(_params.num_cache_zooms > MAX_CACHE_PIXMAPS) {
    _params.num_cache_zooms = MAX_CACHE_PIXMAPS;
  }
  if(_params.num_cache_zooms < 1) {
    _params.num_cache_zooms = 1 ;
  }
  
  gd.h_win.can_xid = (Drawable *) calloc(sizeof(Drawable *),_params.num_cache_zooms);
  gd.v_win.can_xid = (Drawable *) calloc(sizeof(Drawable *),_params.num_cache_zooms);
  
  // zooms
  
  if(!_params.html_mode) {
    gd.h_win.zoom_level = _params.start_zoom_level;
    gd.h_win.num_zoom_levels = _params.zoom_levels_n;
    if(gd.h_win.zoom_level < 0) gd.h_win.zoom_level = 0;
    if(gd.h_win.zoom_level > gd.h_win.num_zoom_levels) {
      gd.h_win.zoom_level = gd.h_win.num_zoom_levels -1;
    }
    gd.h_win.start_zoom_level = gd.h_win.zoom_level;
  }
  
  gd.h_win.zmin_x =
    (double *) calloc(sizeof(double),gd.h_win.num_zoom_levels+NUM_CUSTOM_ZOOMS + 1);
  gd.h_win.zmax_x =
    (double *) calloc(sizeof(double),gd.h_win.num_zoom_levels+NUM_CUSTOM_ZOOMS + 1);
  gd.h_win.zmin_y =
    (double *) calloc(sizeof(double),gd.h_win.num_zoom_levels+NUM_CUSTOM_ZOOMS + 1);
  gd.h_win.zmax_y =
    (double *) calloc(sizeof(double),gd.h_win.num_zoom_levels+NUM_CUSTOM_ZOOMS + 1);

  // _params.zoom_limits_in_latlon =  gd.uparams->getLong("cidd.zoom_limits_in_latlon",0);
  // _params.domain_limit_min_x = gd.uparams->getDouble("cidd.domain_limit_min_x",-10000);
  // _params.domain_limit_max_x = gd.uparams->getDouble("cidd.domain_limit_max_x",10000);
  // _params.domain_limit_min_y = gd.uparams->getDouble("cidd.domain_limit_min_y",-10000);
  // _params.domain_limit_max_y = gd.uparams->getDouble("cidd.domain_limit_max_y",10000);
  
  if (gd.display_projection == Mdvx::PROJ_LATLON) {
    gd.h_win.min_x = max(_params.domain_limit_min_x, -360.0);
    gd.h_win.max_x = min(_params.domain_limit_max_x, 360.0);
    gd.h_win.min_y = max(_params.domain_limit_min_y, -90.0);
    gd.h_win.max_y = min(_params.domain_limit_max_y, 90.0);
  } else {
    gd.h_win.min_x = _params.domain_limit_min_x;
    gd.h_win.max_x = _params.domain_limit_max_x;
    gd.h_win.min_y = _params.domain_limit_min_y;
    gd.h_win.max_y = _params.domain_limit_max_y;
  }
  
  // _params.min_ht = gd.uparams->getDouble("cidd.min_ht", 0.0);
  // _params.max_ht = gd.uparams->getDouble("cidd.max_ht", 30.0);
  // _params.start_ht = gd.uparams->getDouble("cidd.start_ht", 0.0);
  // _params.planview_start_page = gd.uparams->getLong("cidd.planview_start_page", 1) -1;
  // _params.xsect_start_page = gd.uparams->getLong("cidd.xsect_start_page", 1) -1;

  gd.h_win.min_ht = _params.min_ht;
  gd.h_win.max_ht = _params.max_ht;

  // Otherwise set in the command line arguments

  if(gd.num_render_heights == 0) {
    gd.h_win.cur_ht = _params.start_ht;
  }
  
  // Fix out of order limits.
  if(gd.h_win.min_x > gd.h_win.max_x) {
    double tmp = gd.h_win.min_x;
    gd.h_win.min_x = gd.h_win.max_x;
    gd.h_win.max_x = tmp;
  }
    
  if(gd.h_win.min_y > gd.h_win.max_y) {
    double tmp = gd.h_win.min_y;
    gd.h_win.min_y = gd.h_win.max_y;
    gd.h_win.max_y = tmp;
  }
    
  // Sanitize Full earth domain limits.

  if (gd.display_projection == Mdvx::PROJ_LATLON) {
      
    if(gd.h_win.min_x == gd.h_win.max_x) {
      gd.h_win.min_x = gd.h_win.min_x - 180.0;
      gd.h_win.max_x = gd.h_win.max_x + 180.0;
    }
    
    if(gd.h_win.min_x < -360.0) gd.h_win.min_x = -360.0;
    if(gd.h_win.max_x > 360.0) gd.h_win.max_x = 360.0;
    if(gd.h_win.min_y < -180.0) gd.h_win.min_y = -180.0;
    if(gd.h_win.max_y > 180.0) gd.h_win.max_y = 180.0;
      
    if((gd.h_win.max_x - gd.h_win.min_x) > 360.0)  { 
      gd.h_win.min_x = ((gd.h_win.max_x + gd.h_win.min_x) / 2.0) - 180.0;
      gd.h_win.max_x = gd.h_win.min_x + 360;
    }
    double originLon = (gd.h_win.min_x + gd.h_win.max_x) / 2.0;
    gd.proj.initLatlon(originLon);
  }
    
  if(gd.debug) {
    fprintf(stderr,
            " GRID LIMITS:  X: %g,%g   Y: %g,%g \n",
            gd.h_win.min_x,gd.h_win.max_x,
            gd.h_win.min_y,gd.h_win.max_y );
  }
    
  double max_delta_x = gd.h_win.max_x - gd.h_win.min_x;
  double max_delta_y = gd.h_win.max_y - gd.h_win.min_y;

  for(int izoom = 0; izoom < gd.h_win.num_zoom_levels; izoom++) {
    
    double minx = _params._zoom_levels[izoom].min_x;
    double miny = _params._zoom_levels[izoom].min_y;
    double maxx = _params._zoom_levels[izoom].max_x;
    double maxy = _params._zoom_levels[izoom].max_y;
    
    // convert from latlon if needed

    if (_params.zoom_limits_in_latlon) {
      
      double minLon = minx;
      double maxLon = maxx;
      double minLat = miny;
      double maxLat = maxy;

      gd.proj.latlon2xy(minLat, minLon, minx, miny);
      gd.proj.latlon2xy(maxLat, maxLon, maxx, maxy);
      
      if(gd.debug) {
        cerr << "Zoom number: " << (izoom + 1) << endl;
        cerr << "  converting lat/lon to km" << endl;
        cerr << "  minLon, minLat: " << minLon << ", " << minLat << endl;
        cerr << "  maxLon, maxLat: " << maxLon << ", " << maxLat << endl;
        cerr << "  minXkm, minYkm: " << minx << ", " << miny << endl;
        cerr << "  maxXkm, maxYkm: " << maxx << ", " << maxy << endl;
      }

    }
    
    gd.h_win.zmin_x[izoom] = minx;
    gd.h_win.zmin_y[izoom] = miny;
    gd.h_win.zmax_x[izoom] = maxx;
    gd.h_win.zmax_y[izoom] = maxy;
    
    delta_x = gd.h_win.zmax_x[izoom] - gd.h_win.zmin_x[izoom];
    delta_y = gd.h_win.zmax_y[izoom] - gd.h_win.zmin_y[izoom];

    if(delta_x > max_delta_x) delta_x = max_delta_x;
    if(delta_y > max_delta_y) delta_y = max_delta_y;

    // trap bogus values
    if(gd.h_win.zmin_x[izoom] < gd.h_win.min_x) {
      gd.h_win.zmin_x[izoom] = gd.h_win.min_x;
      gd.h_win.zmax_x[izoom] =  gd.h_win.min_x + delta_x;
    }

    if(gd.h_win.zmin_y[izoom] < gd.h_win.min_y) {
      gd.h_win.zmin_y[izoom] = gd.h_win.min_y;
      gd.h_win.zmax_y[izoom] =  gd.h_win.min_y + delta_y;
    }

    if(gd.h_win.zmax_x[izoom] > gd.h_win.max_x) {
      gd.h_win.zmax_x[izoom] = gd.h_win.max_x;
      gd.h_win.zmin_x[izoom] =  gd.h_win.max_x - delta_x;
    }

    if(gd.h_win.zmax_y[izoom] > gd.h_win.max_y) {
      gd.h_win.zmax_y[izoom] = gd.h_win.max_y;
      gd.h_win.zmin_y[izoom] =  gd.h_win.max_y - delta_y;
    }

    if(_params.aspect_ratio <= 0.0) _params.aspect_ratio = fabs(delta_x/delta_y);

    gd.aspect_correction =
      cos(((gd.h_win.zmax_y[izoom] + gd.h_win.zmin_y[izoom])/2.0) * DEG_TO_RAD);

    /* Make sure domains are consistant with the window aspect ratio */
    switch(gd.display_projection) {
      case Mdvx::PROJ_LATLON:
        /* forshorten the Y coords to make things look better */
        delta_y /= gd.aspect_correction;
        break;
    }

    delta_x /= _params.aspect_ratio;

    if(delta_x > delta_y) {
      gd.h_win.zmax_y[izoom] += ((delta_x - delta_y) /2.0) ;
      gd.h_win.zmin_y[izoom] -= ((delta_x - delta_y) /2.0) ;
    } else {
      gd.h_win.zmax_x[izoom] += ((delta_y - delta_x) /2.0) ;
      gd.h_win.zmin_x[izoom] -= ((delta_y - delta_x) /2.0) ;
    }

    if(gd.debug) {
      printf(" ZOOM: %d --  X: %g,%g   Y: %g,%g,  Delta: %g,%g\n", izoom,
             gd.h_win.zmin_x[izoom],gd.h_win.zmax_x[izoom],
             gd.h_win.zmin_y[izoom],gd.h_win.zmax_y[izoom],
             delta_x,delta_y);
    }

  } // izoom

  gd.h_win.cmin_x = gd.h_win.zmin_x[gd.h_win.zoom_level];
  gd.h_win.cmax_x = gd.h_win.zmax_x[gd.h_win.zoom_level];
  gd.h_win.cmin_y = gd.h_win.zmin_y[gd.h_win.zoom_level];
  gd.h_win.cmax_y = gd.h_win.zmax_y[gd.h_win.zoom_level];

  if(gd.debug) {
    printf("CUR X: %g,%g   Y: %g,%g,\n",
           gd.h_win.cmin_x,gd.h_win.cmax_x,
           gd.h_win.cmin_y,gd.h_win.cmax_y);
  }

  // Define a very simple vertical oriented  route - In current domain
  
  gd.h_win.route.num_segments = 1;
  gd.h_win.route.x_world[0] = (gd.h_win.cmin_x + gd.h_win.cmax_x) / 2;
  gd.h_win.route.x_world[1] = gd.h_win.route.x_world[0];

  gd.h_win.route.y_world[0] =
    gd.h_win.cmin_y + ((gd.h_win.cmax_y - gd.h_win.cmin_y) / 4);
  gd.h_win.route.y_world[1] =
    gd.h_win.cmax_y - ((gd.h_win.cmax_y - gd.h_win.cmin_y) / 4);

  gd.h_win.route.seg_length[0] =
    disp_proj_dist(gd.h_win.route.x_world[0],gd.h_win.route.y_world[0],
                   gd.h_win.route.x_world[1],gd.h_win.route.y_world[1]);

  gd.h_win.route.total_length = gd.h_win.route.seg_length[0];

  /* Automatically define the Custom Zoom levels */
  for(int ii = 0; ii <= NUM_CUSTOM_ZOOMS; ii++) {
    gd.h_win.zmin_x[gd.h_win.num_zoom_levels] = gd.h_win.zmin_x[0] + 
      ((gd.h_win.zmax_x[0] -gd.h_win.zmin_x[0]) / ( NUM_CUSTOM_ZOOMS - ii  + 2.0));

    gd.h_win.zmax_x[gd.h_win.num_zoom_levels] = gd.h_win.zmax_x[0] - 
      ((gd.h_win.zmax_x[0] -gd.h_win.zmin_x[0]) / ( NUM_CUSTOM_ZOOMS - ii  + 2.0));

    gd.h_win.zmin_y[gd.h_win.num_zoom_levels] = gd.h_win.zmin_y[0] + 
      ((gd.h_win.zmax_y[0] -gd.h_win.zmin_y[0]) / ( NUM_CUSTOM_ZOOMS - ii  + 2.0));

    gd.h_win.zmax_y[gd.h_win.num_zoom_levels] = gd.h_win.zmax_y[0] - 
      ((gd.h_win.zmax_y[0] -gd.h_win.zmin_y[0]) / ( NUM_CUSTOM_ZOOMS - ii  + 2.0));

    gd.h_win.num_zoom_levels++;
  }

  ZERO_STRUCT(&gd.menu_bar);

  // Establish what each Menu Bar Cell Does.
  gd.menu_bar.num_menu_bar_cells = gd.uparams->getLong("cidd.num_menu_bar_cells",0);
  if(gd.menu_bar.num_menu_bar_cells > 0) {
    for(int ii = 1; ii <= gd.menu_bar.num_menu_bar_cells; ii++) {
      sprintf(str_buf,"cidd.menu_bar_funct%d",ii);
      resource = gd.uparams->getString(str_buf,"Not Defined");
      if(strcmp("LOOP_ONOFF",resource) == 0) {
        gd.menu_bar.loop_onoff_bit = 1 << (ii - 1) ;
      } else if( strcmp("WINDS_ONOFF",resource) == 0) {
        gd.menu_bar.winds_onoff_bit = 1 << (ii - 1) ;
      } else if( strcmp("SHOW_FORECAST_MENU",resource) == 0) {
        gd.menu_bar.show_forecast_menu_bit = 1 << (ii - 1) ;
      } else if( strcmp("SHOW_PAST_MENU",resource) == 0) {
        gd.menu_bar.show_past_menu_bit = 1 << (ii - 1) ;
      } else if( strcmp("SHOW_GENTIME_MENU",resource) == 0) {
        gd.menu_bar.show_gen_time_win_bit = 1 << (ii - 1) ;
      } else if( strcmp("SYMPRODS_ONOFF",resource) == 0) {
        gd.menu_bar.symprods_onoff_bit = 1 << (ii - 1) ;
      } else if( strcmp("PRINT_BUTTON",resource) == 0) {
        gd.menu_bar.print_button_bit = 1 << (ii - 1) ;
      } else if( strcmp("HELP_BUTTON",resource) == 0) {
        gd.menu_bar.help_button_bit = 1 << (ii - 1) ;
      } else if( strcmp("CLONE_CIDD",resource) == 0) {
        gd.menu_bar.clone_button_bit = 1 << (ii - 1) ;
      } else if( strcmp("EXIT_BUTTON",resource) == 0) {
        gd.menu_bar.exit_button_bit = 1 << (ii - 1) ;
      } else if( strcmp("SHOW_VIEW_MENU",resource) == 0) {
        gd.menu_bar.show_view_menu_bit = 1 << (ii - 1) ;
      } else if( strcmp("SHOW_PRODUCT_MENU",resource) == 0) {
        gd.menu_bar.show_prod_menu_bit = 1 << (ii - 1) ;
      } else if( strcmp("SHOW_MAP_MENU",resource) == 0) {
        gd.menu_bar.show_map_menu_bit = 1 << (ii - 1) ;
      } else if( strcmp("SHOW_BOOKMARK_MENU",resource) == 0) {
        gd.menu_bar.show_bookmark_menu_bit = 1 << (ii - 1) ;
      } else if( strcmp("SHOW_STATUS_PANEL",resource) == 0) {
        gd.menu_bar.show_status_win_bit = 1 << (ii - 1) ;
      } else if( strcmp("SHOW_TIME_PANEL",resource) == 0) {
        gd.menu_bar.show_time_panel_bit = 1 << (ii - 1) ;
      } else if( strcmp("SHOW_DPD_MENU",resource) == 0) {
        gd.menu_bar.show_dpd_menu_bit = 1 << (ii - 1) ;
      } else if( strcmp("SHOW_DPD_PANEL",resource) == 0) {
        gd.menu_bar.show_dpd_panel_bit = 1 << (ii - 1) ;
      } else if( strcmp("SET_DRAW_MODE",resource) == 0) {
        gd.menu_bar.set_draw_mode_bit = 1 << (ii - 1) ;
      } else if( strcmp("SET_PICK_MODE",resource) == 0) {
        gd.menu_bar.set_pick_mode_bit = 1 << (ii - 1) ;
      } else if( strcmp("SET_ROUTE_MODE",resource) == 0) {
        gd.menu_bar.set_route_mode_bit = 1 << (ii - 1) ;
      } else if( strcmp("SHOW_XSECT_PANEL",resource) == 0) {
        gd.menu_bar.show_xsect_panel_bit = 1 << (ii - 1) ;
      } else if( strcmp("SHOW_GRID_PANEL",resource) == 0) {
        gd.menu_bar.show_grid_panel_bit = 1 << (ii - 1) ;
      } else if( strcmp("RELOAD",resource) == 0) {
        gd.menu_bar.reload_bit = 1 << (ii - 1) ;
      } else if( strcmp("RESET",resource) == 0) {
        gd.menu_bar.reset_bit = 1 << (ii - 1) ;
      } else if( strcmp("SET_TO_NOW",resource) == 0) {
        gd.menu_bar.set_to_now_bit = 1 << (ii - 1) ;
      } else if( strcmp("CLOSE_POPUPS",resource) == 0) {
        gd.menu_bar.close_popups_bit = 1 << (ii - 1) ;
      } else if( strcmp("REPORT_MODE_ONOFF",resource) == 0) {
        gd.menu_bar.report_mode_bit = 1 << (ii - 1) ;
      } else if( strcmp("LANDUSE_ONOFF",resource) == 0) {
        gd.menu_bar.landuse_onoff_bit = 1 << (ii - 1) ;
      } else if( strcmp("SHOW_CMD_MENU",resource) == 0) {
        gd.menu_bar.show_cmd_menu_bit = 1 << (ii - 1) ;
      } else if( strcmp("SNAP_IMAGE",resource) == 0) {
        gd.menu_bar.snapshot_bit = 1 << (ii - 1) ;
      } else if( strcmp("ZOOM_BACK",resource) == 0) {
        gd.menu_bar.zoom_back_bit = 1 << (ii - 1) ;
      } else {
        fprintf(stderr,"Unrecognized Menu Bar Cell Function %d: %s\n",ii,resource);
        exit(-1);
      }
    }
  } else {
    fprintf(stderr,"Menu Bar cells must be defined in this version\n");
    exit(-1);
  } 

  gd.v_win.zmin_x = (double *) calloc(sizeof(double), 1);
  gd.v_win.zmax_x = (double *) calloc(sizeof(double), 1);
  gd.v_win.zmin_y = (double *) calloc(sizeof(double), 1);
  gd.v_win.zmax_y = (double *) calloc(sizeof(double), 1);

  gd.v_win.origin_lat = _params.origin_latitude;
  gd.v_win.origin_lon = _params.origin_longitude;
  gd.v_win.min_x = gd.h_win.min_x;
  gd.v_win.max_x = gd.h_win.max_x;
  gd.v_win.min_y = gd.h_win.min_y;
  gd.v_win.max_y = gd.h_win.max_y;
  gd.v_win.min_ht = gd.h_win.min_ht;
  gd.v_win.max_ht = gd.h_win.max_ht;

  // Set Vertical window route  params
  gd.v_win.cmin_x = gd.h_win.route.x_world[0];
  gd.v_win.cmin_y = gd.h_win.route.y_world[0];
  gd.v_win.cmax_x = gd.h_win.route.x_world[1];
  gd.v_win.cmax_y = gd.h_win.route.y_world[1];

  // Wind Rendering

  // _params.ideal_x_vects = gd.uparams->getLong("cidd.ideal_x_vectors", 20);
  // _params.ideal_y_vects = gd.uparams->getLong("cidd.ideal_y_vectors", 20);
  // _params.wind_head_size = gd.uparams->getLong("cidd.wind_head_size", 5);
  // _params.wind_head_angle = gd.uparams->getDouble("cidd.wind_head_angle", 45.0);
  // _params.barb_shaft_len = gd.uparams->getLong("cidd.barb_shaft_len", 33);
  
  /* Initialize Extra features data */
  // _params.all_winds_on = gd.uparams->getLong("cidd.all_winds_on", 1);

  gd.layers.wind_vectors = _params.all_winds_on;
  gd.layers.init_state_wind_vectors = gd.layers.wind_vectors;
  
  // _params.wind_mode = gd.uparams->getLong("cidd.wind_mode", 0);
  gd.layers.wind_mode = _params.wind_mode;
  
  // _params.wind_time_scale_interval =
  //   gd.uparams->getDouble("cidd.wind_time_scale_interval", 10.0);
  gd.layers.wind_time_scale_interval = _params.wind_time_scale_interval;

  // _params.wind_scaler = gd.uparams->getLong("cidd.wind_scaler", 3);
  gd.layers.wind_scaler = _params.wind_scaler;

  gd.legends.range = _params.range_rings;
  int plot_azimuths = _params.azimuth_lines;
  gd.legends.azimuths = plot_azimuths ? AZIMUTH_BIT : 0;

  // Load the GRID / DATA FIELD parameters
  // establish and initialize sources of data 

  _init_grids();
  
  // winds init
  
  // _params.wind_marker_type = gd.uparams->getString("cidd.wind_marker_type", "arrow");
  // _params.wind_reference_speed = gd.uparams->getDouble("cidd.wind_reference_speed", 10.0);
  // _params.wind_units_label = gd.uparams->getString("cidd.wind_units_label", "m/sec");
  // _params.wind_w_scale_factor = gd.uparams->getDouble("cidd.wind_w_scale_factor", 10.0);
  // _params.wind_units_scale_factor = gd.uparams->getDouble("cidd.wind_units_scale_factor", 1.0);

#ifdef JUNK
  if(param_text == NULL || param_text_len <=0 ) {
    if(gd.debug)fprintf(stderr,"Couldn't Find WINDS Section\n");
  } else {
    /* Establish and initialize connections to wind fields */
    init_wind_data_links(param_text, param_text_len, param_text_line_no);
  }
#endif
  
  if(gd.layers.num_wind_sets == 0) gd.layers.wind_vectors = 0;

  // Instantiate and load the SYMPROD TDRP Parameter section
  gd.syprod_P = new Csyprod_P();

  param_text_line_no = 0;
  param_text_len = 0;
  // param_text = find_tag_text(gd.db_data,"SYMPRODS",
  //                            &param_text_len, &param_text_line_no); 
  if(param_text == NULL || param_text_len <=0 ) {
    if(gd.debug) fprintf(stderr," Warning: No SYMPRODS Section in params\n");
  } else {
    /* Establish and initialize params */

    if(gd.syprod_P->loadFromBuf("SYMPRODS TDRP Section",
                                NULL,param_text,
                                param_text_len,
                                param_text_line_no,
                                TRUE,gd.debug2) < 0) {
      fprintf(stderr,"Please fix the <SYMPRODS> parameter section\n");
      exit(-1);
    }
  }

  // Instantiate and load the TERRAIN TDRP Parameter 
  gd.layers.earth._P = new Cterrain_P();

  param_text_line_no = 0;
  param_text_len = 0;
  // param_text = find_tag_text(gd.db_data,"TERRAIN",
  //                            &param_text_len, &param_text_line_no); 
  if(param_text == NULL || param_text_len <=0 ) {
    if(gd.debug) fprintf(stderr," Warning: No TERRAIN Section in params\n");
  } else {
    if(gd.layers.earth._P->loadFromBuf("TERRAIN TDRP Section",
                                       NULL,param_text,
                                       param_text_len,
                                       param_text_line_no,
                                       TRUE,gd.debug2) < 0) {
      fprintf(stderr,"Please fix the <TERRAIN> parameter section\n");
      exit(-1);
    }
    if(strlen(gd.layers.earth._P->terrain_url) >0) {
      gd.layers.earth.terrain_active = 1;
      gd.layers.earth.terr = (met_record_t *) calloc(sizeof(met_record_t), 1);

      if(gd.layers.earth.terr == NULL) {
        fprintf(stderr,"Cannot allocate space for terrain data\n");
        exit(-1);
      }
      gd.layers.earth.terr->time_allowance = 5270400; // 10 years
      STRcopy(gd.layers.earth.terr->color_file,
              gd.layers.earth._P->landuse_colorscale,NAME_LENGTH);
      STRcopy(gd.layers.earth.terr->button_name,
              gd.layers.earth._P->id_label,NAME_LENGTH);
      STRcopy(gd.layers.earth.terr->legend_name,
              gd.layers.earth._P->id_label,NAME_LENGTH);
      STRcopy(gd.layers.earth.terr->url,
              gd.layers.earth._P->terrain_url,URL_LENGTH);

      gd.layers.earth.terr->h_mdvx = new DsMdvxThreaded;
      gd.layers.earth.terr->v_mdvx = new DsMdvxThreaded;
      gd.layers.earth.terr->h_mdvx_int16 = new MdvxField;
      gd.layers.earth.terr->v_mdvx_int16 = new MdvxField;
      gd.layers.earth.terr->proj =  new MdvxProj;
    }

    if(strlen(gd.layers.earth._P->landuse_url) >0) {
      gd.layers.earth.landuse_active = (gd.layers.earth._P->landuse_active == true)? 1: 0;
      gd.layers.earth.land_use = (met_record_t *) calloc(sizeof(met_record_t), 1);

      if(gd.layers.earth.land_use == NULL) {
        fprintf(stderr,"Cannot allocate space for land_use data\n");
        exit(-1);
      }
      gd.layers.earth.land_use->time_allowance = 5270400; // 10 years
      STRcopy(gd.layers.earth.land_use->color_file,
              gd.layers.earth._P->landuse_colorscale,NAME_LENGTH);
      STRcopy(gd.layers.earth.land_use->button_name,
              gd.layers.earth._P->id_label,NAME_LENGTH);
      STRcopy(gd.layers.earth.land_use->legend_name,
              gd.layers.earth._P->id_label,NAME_LENGTH);
      STRcopy(gd.layers.earth.land_use->url,
              gd.layers.earth._P->landuse_url,URL_LENGTH);

      gd.layers.earth.land_use->h_mdvx = new DsMdvxThreaded;
      gd.layers.earth.land_use->v_mdvx = new DsMdvxThreaded;
      gd.layers.earth.land_use->h_mdvx_int16 = new MdvxField;
      gd.layers.earth.land_use->v_mdvx_int16 = new MdvxField;
      gd.layers.earth.land_use->proj =  new MdvxProj;

      switch(gd.layers.earth._P->land_use_render_method) {
        default:
        case Cterrain_P::RENDER_RECTANGLES:
          gd.layers.earth.land_use->render_method = POLYGONS;
          break;

        case Cterrain_P::RENDER_FILLED_CONT:
          gd.layers.earth.land_use->render_method = FILLED_CONTOURS;
          break;

        case Cterrain_P::RENDER_DYNAMIC_CONTOURS:
          gd.layers.earth.land_use->render_method = DYNAMIC_CONTOURS;
          break;
      }

    }

  }

  // Instantiate and load the ROUTE_WINDS TDRP Parameter 
  gd.layers.route_wind._P = new Croutes_P();

  param_text_line_no = 0;
  param_text_len = 0;
  // param_text = find_tag_text(gd.db_data,"ROUTE_WINDS",
  //                            &param_text_len, &param_text_line_no); 
  if(param_text == NULL || param_text_len <=0 ) {
    if(gd.debug) fprintf(stderr," Warning: No ROUTE_WINDS Section in params\n");
    gd.layers.route_wind.has_params = 0;
  } else {
    if(gd.layers.route_wind._P->loadFromBuf("ROUTE_WINDS TDRP Section",
                                            NULL,param_text,
                                            param_text_len,
                                            param_text_line_no,
                                            TRUE,gd.debug2) < 0) {
      fprintf(stderr,"Please fix the <ROUTE_WINDS> parameter section\n");
      exit(-1);
    }

    gd.layers.route_wind.has_params = 1;
    route_winds_init();

    // Use the first route as the default.
    memcpy(&gd.h_win.route,gd.layers.route_wind.route,sizeof(route_track_t)); 
  }

  // Instantiate the GUI Config TDRP 
  gd.gui_P = new Cgui_P();

  // Load the GUI_CONFIG parameters
  param_text_line_no = 0;
  param_text_len = 0;
  // param_text = find_tag_text(gd.db_data,"GUI_CONFIG",
  //                            &param_text_len, &param_text_line_no);

  if(param_text == NULL || param_text_len <=0 ) {
  } else {
    if(gd.gui_P->loadFromBuf("GUI_CONFIG TDRP Section",
                             NULL,param_text,
                             param_text_len,
                             param_text_line_no,
                             TRUE,gd.debug2)  < 0) { 
      fprintf(stderr,"Please fix the <GUI_CONFIG> parameter section\n");
      exit(-1);
    }
  }

  // Instantiate the IMAGES Config TDRP 
  gd.images_P = new Cimages_P();

  // Load the IMAGES_CONFIG parameters
  param_text_line_no = 0;
  param_text_len = 0;
  // param_text = find_tag_text(gd.db_data,"IMAGE_GENERATION",
  //                            &param_text_len, &param_text_line_no);

  if(param_text == NULL || param_text_len <=0 ) {
  } else {
    if(gd.images_P->loadFromBuf("IMAGE_GENERATION TDRP Section",
                                NULL,param_text,
                                param_text_len,
                                param_text_line_no,
                                TRUE,gd.debug2)  < 0) { 
      fprintf(stderr,"Please fix the <IMAGE_GENERATION> parameter section\n");
      exit(-1);
    }
  }

  // Instantiate the Draw TDRP 
  gd.draw_P = new Cdraw_P();

  // Load the Draw_Export parameters
  param_text_line_no = 0;
  param_text_len = 0;
  // param_text = find_tag_text(gd.db_data,"DRAW_EXPORT",
  //                            &param_text_len, &param_text_line_no);

  if(param_text == NULL || param_text_len <=0 ) {
    if(gd.debug) fprintf(stderr," Warning: No DRAW_EXPORT Section in params\n");
    gd.draw.num_draw_products = 0;
  } else {
    if(gd.draw_P->loadFromBuf("DRAW EXPORT TDRP Section",
			      NULL,param_text,
			      param_text_len,
			      param_text_line_no,
			      TRUE,gd.debug2)  < 0) { 
      fprintf(stderr,"Please fix the <DRAW_EXPORT> parameter section\n");
      exit(-1);
    }

    /* Establish and initialize Draw-Export params */
    init_draw_export_links();
  }
    
  if(gd.draw.num_draw_products == 0 && gd.menu_bar.set_draw_mode_bit >0) {
    fprintf(stderr,
	    "Fatal Error: DRAW Button Enabled, without any DRAW_EXPORT Products defined\n"); 
    fprintf(stderr,
	    "Either remove SET_DRAW_MODE button or define products in DRAW_EXPORT \n"); 
    fprintf(stderr,
	    "Section of the parameter file \n"); 
    exit(-1);
  }


  // Load the Map Overlay parameters
  param_text_line_no = 0;
  param_text_len = 0;
  // param_text = find_tag_text(gd.db_data,"MAPS",
  //                            &param_text_len, &param_text_line_no);

  if(param_text == NULL || param_text_len <=0 ) {
    fprintf(stderr,"Could'nt Find MAPS SECTION\n");
    exit(-1);
  }

  // overlays

  // _params.map_file_subdir =  gd.uparams->getString("cidd.map_file_subdir", "maps");
  init_over_data_links(param_text, param_text_len, param_text_line_no);
  
  // Instantiate the Station locator classes and params.
  // _params.locator_margin_km = gd.uparams->getDouble("cidd.locator_margin_km", 50.0);
  // _params.station_loc_url = gd.uparams->getString("cidd.station_loc_url", "");

#ifdef JUNK
  
  if(strlen(_params.station_loc_url) > 1) {
    if(gd.debug || gd.debug1) {
      fprintf(stderr,"Loading Station data from %s ...",_params.station_loc_url);
    }
    gd.station_loc =  new StationLoc();
    if(gd.station_loc == NULL) {
      fprintf(stderr,"CIDD: Fatal Alloc constructing new StationLoc()\n");
      exit(-1);
    }

    if(gd.station_loc->ReadData(_params.station_loc_url) < 0) {
      fprintf(stderr,"CIDD: Can't load Station Data from %s\n",_params.station_loc_url);
      exit(-1);
    }
    // gd.station_loc->PrintAll();  // DEBUG

    if(gd.debug || gd.debug1) {
      fprintf(stderr,"Done\n");
    }
  }

#endif

  // _params.remote_ui_url = gd.uparams->getString("cidd.remote_ui_url", "");
  if(strlen(_params.remote_ui_url) > 1) {

    gd.remote_ui = new RemoteUIQueue();

    // Create the FMW with 4096 slots - Total size 1M
    bool compression = false;
    if (gd.remote_ui->initReadWrite( _params.remote_ui_url,
                                     (char *) "CIDD",
                                     (bool) gd.debug2,
                                     DsFmq::END, compression,
                                     4096, 4096*256 ) != 0 ) { 
      fprintf(stderr,"Problems initialising Remote Command Fmq: %s - aborting\n",_params.remote_ui_url);
    }
  }

  for(int ii = 0; ii < NUM_CONT_LAYERS; ii++) {
    gd.layers.cont[ii].field = 0;
    gd.layers.cont[ii].min = gd.mrec[0]->cont_low;
    gd.layers.cont[ii].max = gd.mrec[0]->cont_high;
    gd.layers.cont[ii].interval = gd.mrec[0]->cont_interv;
    gd.layers.cont[ii].labels_on  = _params.label_contours;
  }
  for(int ii = 0; ii < NUM_GRID_LAYERS; ii++) {
    gd.layers.overlay_field[ii] = 0;
  }

  for(int ii = 0; ii < gd.num_datafields; ii++) {
    gd.h_win.redraw[ii] = 1;
    gd.v_win.redraw[ii] = 1;
  }

  /* Get space for string parsing sub-fields */
  cfield[0] = (char *)  calloc(1,NAME_LENGTH);
  cfield[1] = (char *)  calloc(1,NAME_LENGTH);
  cfield[2] = (char *)  calloc(1,NAME_LENGTH);

  if(cfield[0]  == NULL || cfield[1] == NULL || cfield[2] == NULL) {
    fprintf(stderr,"Cidd: Fatal Alloc failure of %d bytes\n", NAME_LENGTH);
    exit(-1);
  }

  /* Setup default CONTOUR FIELDS */
  for(int ii = 1; ii <= NUM_CONT_LAYERS; ii++ ) {

    sprintf(str_buf,"cidd.contour%d_field", ii);
    field_str = gd.uparams->getString(str_buf, "NoMaTcH");

    num_fields = STRparse(field_str, cfield, NAME_LENGTH, 3, NAME_LENGTH); 

    strncpy(gd.layers.cont[ii-1].color_name,"white",NAME_LENGTH);

    if(_params.html_mode == 0) {
      /* Replace Underscores with spaces in field names */
      for(int jj = strlen(cfield[0])-1 ; jj >= 0; jj--) {
        if(_params.replace_underscores && cfield[0][jj] == '_') {
          cfield[0][jj] = ' ';
        }
      }
    }
    for(int jj = 0; jj < gd.num_datafields; jj++) {
      if(strcmp(gd.mrec[jj]->button_name,cfield[0]) == 0) {
        gd.layers.cont[ii-1].field = jj;
        if(num_fields >  2 && (strncasecmp(cfield[2],"off",3) == 0) ) {
          gd.layers.cont[ii-1].active = 0;
        } else {
          gd.layers.cont[ii-1].active = 1;
        }

        gd.layers.cont[ii-1].min = gd.mrec[jj]->cont_low;
        gd.layers.cont[ii-1].max = gd.mrec[jj]->cont_high;
        gd.layers.cont[ii-1].interval = gd.mrec[jj]->cont_interv;

        if(num_fields > 1) {
          strncpy(gd.layers.cont[ii-1].color_name,cfield[1],NAME_LENGTH);
        }
      }
    } // jj
  } // ii

  /* Set up default OVERLAY FIELDS */
  for(int ii = 1; ii <= NUM_GRID_LAYERS; ii++ ) {
    sprintf(str_buf,"cidd.layer%d_field",ii);
    field_str =  gd.uparams->getString( str_buf, "NoMaTcH");
    
    num_fields = STRparse(field_str, cfield, NAME_LENGTH, 3, NAME_LENGTH); 

    if(_params.html_mode == 0) {
      /* Replace Underscores with spaces in field names */
      for(int jj = strlen(cfield[0])-1 ; jj >= 0; jj--) {
        if(_params.replace_underscores && field_str[jj] == '_') {
          cfield[0][jj] = ' ';
        }
      } // jj
    }
    for(int jj = 0; jj <  gd.num_datafields; jj++) {
      if(strcmp(gd.mrec[jj]->button_name,cfield[0]) == 0) {  
        if(num_fields > 1 && (strncasecmp(cfield[1],"off",3) == 0) ) {
          gd.layers.overlay_field_on[ii-1] = 0;
        } else {
          gd.layers.overlay_field_on[ii-1] = 1;
        }

        gd.layers.overlay_field[ii-1] = jj;
      }
    } // jj
  } // ii

  free(cfield[0]);
  free(cfield[1]);
  free(cfield[2]);

  // fonts

  // _params.num_fonts = gd.uparams->getLong("cidd.num_fonts", 1);
  
#ifdef NOTNOW
  
  if(_params.num_fonts > MAX_FONTS) {
    _params.num_fonts = MAX_FONTS;
    fprintf(stderr,"Cidd: Warning. Too Many Fonts. Limited to %d Fonts\n",MAX_FONTS);
  }

  // Make sure specified font for Winds, Contours and Products are within range.
  if(gd.prod.prod_font_num < 0) gd.prod.prod_font_num = 0;
  if(gd.prod.prod_font_num >= _params.num_fonts) gd.prod.prod_font_num = _params.num_fonts -1;
  
  for(i=0;i < _params.num_fonts; i++) {
    sprintf(p_name,"cidd.font%d",i+1);
    f_name = gd.uparams->getString(
            p_name, "fixed");
    gd.fontst[i] = (XFontStruct *) XLoadQueryFont(dpy,f_name);
    if(gd.fontst[i] != NULL) {
      gd.ciddfont[i]  = gd.fontst[i]->fid;
    } else {
      fprintf(stderr,"Can't load font: %s\n",f_name);
      fprintf(stderr,"Using 'fixed' instead\n");
      gd.fontst[i]  = (XFontStruct *) XLoadQueryFont(dpy,"fixed");
      gd.ciddfont[i]  = gd.fontst[i]->fid;
    }
  }    

#endif

  // contours

  // _params.contour_font_num = gd.uparams->getLong("contour_font_num", 6);
  // _params.n_ideal_contour_labels = gd.uparams->getLong("n_ideal_contour_labels", 5);

  // canvas events

  // _params.rotate_coarse_adjust = gd.uparams->getDouble("cidd.rotate_coarse_adjust",6.0);
  // _params.rotate_medium_adjust = gd.uparams->getDouble("cidd.rotate_medium_adjust",2.0);
  // _params.rotate_fine_adjust = gd.uparams->getDouble("cidd.rotate_fine_adjust", 0.5);

  // zoom
  
  // _params.min_zoom_threshold = gd.uparams->getDouble("cidd.min_zoom_threshold", 5.0);

  // gui

  // _params.no_data_message = gd.uparams->getString("cidd.no_data_message", "NO DATA FOUND (in this area at the selected time)");
  // _params.horiz_top_margin =  gd.uparams->getLong("cidd.horiz_top_margin", 20);
  // _params.horiz_bot_margin =  gd.uparams->getLong("cidd.horiz_bot_margin", 20);
  // _params.horiz_left_margin = gd.uparams->getLong("cidd.horiz_left_margin", 20);
  // _params.horiz_right_margin = gd.uparams->getLong("cidd.horiz_right_margin", 80);
  
  // _params.vert_top_margin =  gd.uparams->getLong("cidd.vert_top_margin", 20);
  // _params.vert_bot_margin =  gd.uparams->getLong("cidd.vert_bot_margin", 20);
  // _params.vert_left_margin = gd.uparams->getLong("cidd.vert_left_margin", 20);
  // _params.vert_right_margin = gd.uparams->getLong("cidd.vert_right_margin", 80);
  
  // _params.horiz_legends_start_x = gd.uparams->getLong("cidd.horiz_legends_start_x", 0);
  // _params.horiz_legends_start_y = gd.uparams->getLong("cidd.horiz_legends_start_y", 0);
  // _params.horiz_legends_delta_y = gd.uparams->getLong("cidd.horiz_legends_delta_y", 0);
  
  // _params.vert_legends_start_x = gd.uparams->getLong("cidd.vert_legends_start_x", 0);
  // _params.vert_legends_start_y = gd.uparams->getLong("cidd.vert_legends_start_y", 0);
  // _params.vert_legends_delta_y = gd.uparams->getLong("cidd.vert_legends_delta_y", 0);
  
  // _params.horiz_min_height = gd.uparams->getLong("cidd.horiz_min_height", 400);
  // _params.horiz_min_width = gd.uparams->getLong("cidd.horiz_min_width", 600);
  // _params.horiz_default_height = gd.uparams->getLong("cidd.horiz_default_height", 600);
  // _params.horiz_default_width = gd.uparams->getLong("cidd.horiz_default_width", 800);

  // _params.vert_min_height = gd.uparams->getLong("cidd.vert_min_height", 400);
  // _params.vert_min_width = gd.uparams->getLong("cidd.vert_min_width", 600);
  // _params.vert_default_height = gd.uparams->getLong("cidd.vert_default_height", 400);
  // _params.vert_default_width = gd.uparams->getLong("cidd.vert_default_width", 600);

  // _params.wsddm_mode  = gd.uparams->getLong("cidd.wsddm_mode", 0);
  // _params.one_click_rhi  = gd.uparams->getLong("cidd.one_click_rhi", 0);
  // _params.click_posn_rel_to_origin  = gd.uparams->getLong("cidd.click_posn_rel_to_origin", 0);
  // _params.report_clicks_in_status_window = gd.uparams->getLong("cidd.report_clicks_in_status_window", 0);
  // _params.report_clicks_in_degM_and_nm = gd.uparams->getLong("cidd.report_clicks_in_degM_and_nm", 0);
  // _params.magnetic_variation_deg = gd.uparams->getLong("cidd.magnetic_variation_deg", 0);
  // _params.check_data_times = gd.uparams->getLong("cidd.check_data_times", 0);

  // _params.frame_label = gd.uparams->getString("cidd.horiz_frame_label", "Qucid");
  // _params.status_info_file = gd.uparams->getString("cidd.status_info_file", "");

  // h_win_proc
  
  // _params.horiz_default_y_pos = gd.uparams->getLong("cidd.horiz_default_y_pos",0);
  // _params.horiz_default_x_pos = gd.uparams->getLong("cidd.horiz_default_x_pos",0);

  // _params.vert_default_x_pos = gd.uparams->getLong("cidd.vert_default_x_pos", 0);
  // _params.vert_default_y_pos = gd.uparams->getLong("cidd.vert_default_y_pos", 0);
  
  // page_pu_proc

#ifdef JUNK
  _params.ideal_x_vectors = gd.uparams->getLong("cidd.ideal_x_vectors", 20);
  _params.ideal_y_vectors = gd.uparams->getLong("cidd.ideal_y_vectors", 20);
  gd.azimuth_interval = gd.uparams->getDouble("cidd.azmith_interval", 30.0);
  gd.azimuth_interval = gd.uparams->getDouble("cidd.azimuth_interval", gd.azimuth_interval);
  gd.azimuth_radius = gd.uparams->getDouble("cidd.azmith_radius", 200.0);
  gd.azimuth_radius = gd.uparams->getDouble("cidd.azimuth_radius", gd.azimuth_radius);
  _params.latest_click_mark_size = gd.uparams->getLong("cidd.latest_click_mark_size", 11);
  _params.range_ring_x_space = gd.uparams->getLong("cidd.range_ring_x_space", 50);
  _params.range_ring_y_space = gd.uparams->getLong("cidd.range_ring_y_space", 15);
  _params.range_ring_spacing = gd.uparams->getDouble("cidd.range_ring_spacing", -1.0);
  _params.max_ring_range = gd.uparams->getDouble("cidd.max_ring_range", 1000.0);
  _params.range_ring_labels = gd.uparams->getLong("cidd.range_ring_labels", 1);
#endif
  
  // symprods
  
  // _params.scale_constant = gd.uparams->getDouble("cidd.scale_constant", 300.0);

  // timer control

  // _params.redraw_interval = gd.uparams->getLong("cidd.redraw_interval", REDRAW_INTERVAL);
  // _params.update_interval = gd.uparams->getLong("cidd.update_interval", UPDATE_INTERVAL);

  gd.uparams->setPrintTdrp(false);

  // initialize procmap
  
  if(!_params.run_once_and_exit) {
    PMU_auto_init(gd.app_name, gd.app_instance, PROCMAP_REGISTER_INTERVAL);
  }

  // initialize shared memory
  
  if(!_params.run_once_and_exit) {
    PMU_auto_register("Initializing SHMEM");
  }
    
  init_shared(); /* Initialize Shared memory based communications */
  
}

//////////////////////////////////
// initialize the gridded fields

static void _init_grids()
{

  gd.num_datafields = _params.fields_n;
  if(gd.num_datafields <=0) {
    fprintf(stderr,"Qucid requires at least one valid gridded data field to be defined\n");
    exit(-1);
  }
  
  for (int ifld = 0; ifld < _params.fields_n; ifld++) {
    
    Params::field_t &fld = _params._fields[ifld];
    
    /* get space for data info */
    
    gd.mrec[ifld] = (met_record_t *) calloc(sizeof(met_record_t), 1);
    met_record_t *mrec = gd.mrec[ifld]; 
    
    STRcopy(mrec->legend_name, fld.legend_label, NAME_LENGTH);
    STRcopy(mrec->button_name, fld.button_label, NAME_LENGTH);
    
    if(_params.html_mode == 0 && _params.replace_underscores) {
      /* Replace Underscores with spaces in names */
      for(int jj = strlen(mrec->button_name)-1 ; jj >= 0; jj--) {
        if(mrec->button_name[jj] == '_') {
          mrec->button_name[jj] = ' ';
        }
        if(mrec->legend_name[jj] == '_') {
          mrec->legend_name[jj] = ' ';
        }
      } // jj
    }
    
    STRcopy(mrec->url, fld.url, URL_LENGTH);
    STRcopy(mrec->field_label, fld.field_name, NAME_LENGTH);
    STRcopy(mrec->color_file, fld.color_map, NAME_LENGTH);

    // if units are "" or --, set to zero-length string
    if (!strcmp(fld.field_units, "\"\"") || !strcmp(fld.field_units, "--")) {
      STRcopy(mrec->field_units, "", LABEL_LENGTH);
    } else {
      STRcopy(mrec->field_units, fld.field_units, LABEL_LENGTH);
    }

    mrec->cont_low = fld.contour_low;
    mrec->cont_high = fld.contour_high;
    mrec->cont_interv = fld.contour_interval;

    mrec->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval;

    if (fld.render_mode == Params::POLYGONS) {
      mrec->render_method = POLYGONS;
    } else if (fld.render_mode == Params::FILLED_CONTOURS) {
      mrec->render_method = FILLED_CONTOURS;
    } else if (fld.render_mode == Params::DYNAMIC_CONTOURS) {
      mrec->render_method = DYNAMIC_CONTOURS;
    } else if (fld.render_mode == Params::LINE_CONTOURS) {
      mrec->render_method = LINE_CONTOURS;
    }

    if (fld.composite_mode) {
      mrec->composite_mode = TRUE;
    }

    if (fld.auto_scale) {
      mrec->auto_scale = TRUE;
    }

    if (fld.display_in_menu) {
      mrec->currently_displayed = 1;
    } else {
      mrec->currently_displayed = 0;
    }
    
    if(_params.run_once_and_exit) {
      mrec->auto_render = 1;
    } else {
      mrec->auto_render = fld.auto_render;
    }
    
    mrec->last_elev = (char *)NULL;
    mrec->elev_size = 0;
    
    mrec->plane = 0;
    mrec->h_data_valid = 0;
    mrec->v_data_valid = 0;
    mrec->h_last_scale  = -1.0;
    mrec->h_last_bias  = -1.0;
    mrec->h_last_missing  = -1.0;
    mrec->h_last_bad  = -1.0;
    mrec->h_last_transform  = -1;
    mrec->v_last_scale  = -1.0;
    mrec->v_last_bias  = -1.0;
    mrec->v_last_missing  = -1.0;
    mrec->v_last_bad  = -1.0;
    mrec->v_last_transform  = -1;
    mrec->h_fhdr.proj_origin_lat  = 0.0;
    mrec->h_fhdr.proj_origin_lon  = 0.0;
    mrec->time_list.num_alloc_entries = 0;
    mrec->time_list.num_entries = 0;
    
    STRcopy(mrec->units_label_cols,"KM",LABEL_LENGTH);
    STRcopy(mrec->units_label_rows,"KM",LABEL_LENGTH);
    STRcopy(mrec->units_label_sects,"KM",LABEL_LENGTH);

    // instantiate classes
    mrec->h_mdvx = new DsMdvxThreaded;
    mrec->v_mdvx = new DsMdvxThreaded;
    mrec->h_mdvx_int16 = new MdvxField;
    mrec->v_mdvx_int16 = new MdvxField;
    mrec->proj = new MdvxProj;
    
  } // ifld
  
  /* Make sure the first field is always on */
  gd.mrec[0]->currently_displayed = 1;

}
