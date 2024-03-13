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
#include <shapelib/shapefil.h>
#include <algorithm>
#include <toolsa/DateTime.hh>

static void _initGrids();
static void _initWinds();
static void _initWindComponent(met_record_t *wrec,
                               const Params::wind_t &windp,
                               bool isU, bool isV, bool isW);
static void _initTerrain();
static void _initDrawExport();
static void _initMaps();
static void _initRouteWinds();

static void _loadRapMap(Overlay_t *ov, const char *maps_url);
static void _loadShapeMap(Overlay_t *ov, const char    *maps_url);

static void _initZooms();
static void _initContours();
static void _initOverlayFields();

/*****************************************************************
 * INIT_DATA_SPACE : Init all globals and set up defaults
 */

void init_data_space()
{

  UTIMstruct temp_utime;

  if(!gd.quiet_mode) {
    fprintf(stderr,"Qucid: Version %s\n", CIDD_VERSION);
    fprintf(stderr,"copyright %s\n\n", CIDD_UCOPYRIGHT);
  }

  // debugging level
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    gd.debug = 1;
    gd.debug1 = 1;
    gd.debug2 = 1;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    gd.debug = 1;
    gd.debug1 = 1;
    gd.debug2 = 0;
  } else if (_params.debug >= Params::DEBUG_NORM) {
    gd.debug = 1;
    gd.debug1 = 0;
    gd.debug2 = 0;
  } else {
    gd.debug = 0;
    gd.debug1 = 0;
    gd.debug2 = 0;
  }
  
  // open shmem segment for interprocess comms
  
  gd.coord_key = _params.coord_key;
  if((gd.coord_expt = (coord_export_t *) ushm_create(gd.coord_key,
                                                     sizeof(coord_export_t),
                                                     0666)) == NULL) {
    fprintf(stderr, "Could not create shared mem for interprocess comms\n");
    exit(-1);
  }
  memset(gd.coord_expt, 0, sizeof(coord_export_t));

  // initialize shared memory
  
  PMU_auto_register("Initializing SHMEM");
  init_shared(); /* Initialize Shared memory based communications */
  
  
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
    _params.idle_reset_seconds = 1000000000; // 30+ years
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
  
  if (_params.proj_type == Params::PROJ_FLAT) {
    
    gd.display_projection = Mdvx::PROJ_FLAT;
    gd.proj_param[0] = _params.proj_rotation; // rotation rel to TN
    gd.proj.initFlat(_params.origin_latitude,
                     _params.origin_longitude,
                     _params.proj_rotation);
    if(gd.debug) {
      fprintf(stderr, "Cartesian projection\n");
      fprintf(stderr, "Origin at: %g, %g\n", 
              _params.origin_latitude,_params.origin_longitude);
    }
    
  } else if (_params.proj_type == Params::PROJ_LATLON) {

    gd.display_projection = Mdvx::PROJ_LATLON;
    gd.proj.initLatlon(_params.origin_longitude);
    if(gd.debug) {
      fprintf(stderr, "LATLON/Cylindrical projection\n");
      fprintf(stderr, "Origin at: %g, %g\n",
              _params.origin_latitude, _params.origin_longitude);
    }
    
  } else if (_params.proj_type == Params::PROJ_LAMBERT_CONF) {
    
    gd.display_projection = Mdvx::PROJ_LAMBERT_CONF;
    gd.proj_param[0] = _params.proj_lat1;
    gd.proj_param[1] = _params.proj_lat2;
    gd.proj.initLambertConf(_params.origin_latitude,
                            _params.origin_longitude,
                            _params.proj_lat1,
                            _params.proj_lat2);
    if(gd.debug) {
      fprintf(stderr, "LAMBERT projection\n");
      fprintf(stderr, "Origin at: %g, %g\n",
              _params.origin_latitude, _params.origin_longitude);
      fprintf(stderr, "Parallels at: %g, %g\n",
              _params.proj_lat1, _params.proj_lat2);
    }
    
  } else if (_params.proj_type == Params::PROJ_OBLIQUE_STEREO) {

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
      fprintf(stderr, "Oblique Stereographic projection\n");
      fprintf(stderr, "Origin at: %g, %g\n",
              _params.origin_latitude,_params.origin_longitude);
      fprintf(stderr, "Tangent at: %g, %g\n",
              _params.proj_tangent_lat, _params.proj_tangent_lon);
    }
    
  } else if (_params.proj_type == Params::PROJ_POLAR_STEREO) {
    
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
      fprintf(stderr, "Polar Stereographic projection\n");
      fprintf(stderr, "Origin at: %g, %g\n",
              _params.origin_latitude, _params.origin_longitude);
      fprintf(stderr, "Tangent at: %g, %g\n",
              _params.proj_tangent_lat, _params.proj_tangent_lon);
      fprintf(stderr, "Central scale: %g\n",
              _params.proj_central_scale);
    }

  } else if (_params.proj_type == Params::PROJ_MERCATOR) {
    
    gd.display_projection = Mdvx::PROJ_MERCATOR;
    gd.proj.initMercator(_params.origin_latitude,_params.origin_longitude);
    if(gd.debug) {
      fprintf(stderr, "MERCATOR projection\n");
      fprintf(stderr, "Origin at: %g, %g\n",
              _params.origin_latitude, _params.origin_longitude);
    }
      
  } else if (_params.proj_type == Params::PROJ_TRANS_MERCATOR) {
    
    gd.display_projection = Mdvx::PROJ_TRANS_MERCATOR;
    gd.proj_param[0] = _params.proj_central_scale;
    gd.proj.initTransMercator(_params.origin_latitude,
                              _params.origin_longitude,
                              _params.proj_central_scale);
    if(gd.debug) {
      fprintf(stderr, "TRANS_MERCATOR projection\n");
      fprintf(stderr, "Origin at: %g, %g\n",
              _params.origin_latitude, _params.origin_longitude);
      fprintf(stderr, "Central scale: %g\n",
              _params.proj_central_scale);
    }
      
  } else if (_params.proj_type == Params::PROJ_ALBERS) {
    
    gd.display_projection = Mdvx::PROJ_ALBERS;
    gd.proj_param[0] = _params.proj_lat1;
    gd.proj_param[1] = _params.proj_lat2;
    gd.proj.initAlbers(_params.origin_latitude,
                       _params.origin_longitude,
                       _params.proj_lat1,
                       _params.proj_lat2);
    if(gd.debug) {
      fprintf(stderr, "ALBERS projection\n");
      fprintf(stderr, "Origin at: %g, %g\n",
              _params.origin_latitude, _params.origin_longitude);
      fprintf(stderr, "Parallels at: %g, %g\n",
              _params.proj_lat1, _params.proj_lat2);
    }
    
  } else if (_params.proj_type == Params::PROJ_LAMBERT_AZIM) {
    
    gd.display_projection = Mdvx::PROJ_LAMBERT_AZIM;
    gd.proj.initLambertAzim(_params.origin_latitude,_params.origin_longitude);
    if(gd.debug) {
      fprintf(stderr, "LAMBERT_AZIM projection\n");
      fprintf(stderr, "Origin at: %g, %g\n",
              _params.origin_latitude, _params.origin_longitude);
    }
      
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
  gd.movie.time_interval_mins = _params.frame_duration_secs / 60.0;
  gd.movie.frame_span = _params.climo_frame_span_mins;
  gd.movie.num_frames = _params.n_movie_frames;
  gd.movie.reset_frames = _params.reset_frames;
  gd.movie.delay = _params.movie_delay;
  gd.movie.forecast_interval = _params.forecast_interval_hours;
  gd.movie.past_interval = _params.past_interval_hours;
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

  // Use cosine correction for computing range in polar data
  // check if set by Args

  if (gd.use_cosine_correction < 0) {
    // not set on command line
    gd.use_cosine_correction = _params.use_cosine_correction;
  }
  
  // IF demo_time is set in the params
  // Set into Archive Mode at the indicated time.
  // If demo time param is not set and command line option hasn't set archive mode

  if(_params.start_mode == Params::MODE_REALTIME) {
    
    /* REALTIME MODE */

    gd.movie.mode = REALTIME_MODE;
    gd.coord_expt->runtime_mode = RUNMODE_REALTIME;
    gd.coord_expt->time_seq_num++;
    gd.forecast_mode = 0;
    
    /* set the first index's time based on current time  */
    time_t clock = time(0);    /* get current time */
    gd.movie.start_time =
      (time_t) (clock - ((gd.movie.num_frames -1) * gd.movie.time_interval_mins * 60.0));
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
      
      DateTime startTime;
      startTime.set(_params.archive_start_time);
      gd.movie.start_time = startTime.utime();
      
    }
    gd.movie.demo_mode = 1;
	 
    /* Adjust the start time downward to the nearest round interval seconds */

    gd.movie.start_time -= (gd.movie.start_time % gd.movie.round_to_seconds);

    if(_params.gather_data_mode == CLOSEST_TO_FRAME_CENTER) {
      // Offset movie frame by 1/2 frame interval so that interest time
      // lies on frame mid point
      gd.movie.start_time -=  (time_t) (gd.movie.time_interval_mins * 30.0);
    }

    gd.movie.demo_time = gd.movie.start_time;
    UTIMunix_to_date(gd.movie.start_time,&temp_utime);
    
    gd.h_win.movie_page = gd.h_win.page;
    gd.v_win.movie_page = gd.v_win.page;
    gd.movie.cur_frame = 0;

  } // if(strlen(_params.demo_time) < 8 && (gd.movie.mode != ARCHIVE_MODE))

  reset_time_points(); // reset movie

#ifdef CHECK_LATER
  if(_params.html_mode || gd.movie.num_frames < 3 ) {
    _params.bot_margin_render_style = gd.uparams->getLong("cidd.bot_margin_render_style", 1);
  } else {
    _params.bot_margin_render_style = gd.uparams->getLong("cidd.bot_margin_render_style", 2);
  }
#endif

  /////////////////////////////////////////////////
  // zooms
  
  _initZooms();
  
  //////////////////////////////////////////
  // heights
  
  gd.h_win.min_ht = _params.min_ht;
  gd.h_win.max_ht = _params.max_ht;

  // Otherwise set in the command line arguments

  if(gd.num_render_heights == 0) {
    gd.h_win.cur_ht = _params.start_ht;
  }

  /////////////////////////////////////////////
  // Fix limits as needed

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

  ///////////////////////////////////////////////
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
  
  // init current zoom
  
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

  // vertical section
  
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

  // Load the GRIDDED DATA FIELD parameters
  
  _initGrids();
  
  // Wind Rendering

  gd.layers.wind_vectors = _params.all_winds_on;
  gd.layers.init_state_wind_vectors = gd.layers.wind_vectors;
  gd.layers.wind_mode = _params.wind_mode;
  gd.layers.wind_time_scale_interval = _params.wind_time_scale_interval;
  gd.layers.wind_scaler = _params.wind_scaler;
  gd.legends.range = _params.range_rings;
  int plot_azimuths = _params.azimuth_lines;
  gd.legends.azimuths = plot_azimuths ? AZIMUTH_BIT : 0;

  // initialize wind data
  
  _initWinds();

  // initialize terrain

  _initTerrain();
  
  // initialize route winds
  
  _initRouteWinds();
  
  // Establish and initialize Draw-Export params 

  _initDrawExport();

  // initialize the map overlays

  _initMaps();
  
  // Instantiate the Station locator classes and params.
  
  if(strlen(_params.station_loc_url) > 1) {
    
    if(gd.debug || gd.debug1) {
      fprintf(stderr,"Loading Station data from %s ...",_params.station_loc_url);
    }
    gd.station_loc =  new StationLoc();
    if(gd.station_loc == NULL) {
      fprintf(stderr,"CIDD: Fatal alloc constructing new stationLoc()\n");
      exit(-1);
    }
    
    if(gd.station_loc->ReadData(_params.station_loc_url) < 0) {
      fprintf(stderr,"CIDD: Can't load Station Data from %s\n",_params.station_loc_url);
      exit(-1);
    }
    if (_params.debug >= Params::DEBUG_EXTRA) {
      gd.station_loc->PrintAll();
    }

  }

  /////////////////////
  // remote command queue
  
  if(strlen(_params.remote_ui_url) > 1) {
    gd.remote_ui = new RemoteUIQueue();
    // Create the FMQ with 4096 slots - Total size 1M
    bool compression = false;
    if (gd.remote_ui->initReadWrite( _params.remote_ui_url,
                                     (char *) "CIDD",
                                     (bool) gd.debug2,
                                     DsFmq::END, compression,
                                     4096, 4096*256 ) != 0 ) { 
      fprintf(stderr,
              "Problems initialising Remote Command Fmq: %s - aborting\n",
              _params.remote_ui_url);
    }
  }

  ////////////////////////////////////////////
  // contours

  _initContours();
  
  ////////////////////////////////////////////
  // overlay fields

  _initOverlayFields();

#ifdef NOTNOW
  
  /////////////////////
  // fonts

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

}

//////////////////////////////////
// initialize the gridded fields

static void _initGrids()
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

    mrec->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval_mins;

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

//////////////////////////////////
// initialize the wind grids

static void _initWinds()
{


  int default_marker_type = ARROWS;

  if (_params.winds_n == 0) {
    return;
  }

  gd.layers.num_wind_sets = _params.winds_n;
  if(gd.layers.num_wind_sets == 0) {
    gd.layers.wind_vectors = 0;
  }
  
  for(int ii = 0; ii < gd.layers.num_wind_sets; ii++) {
    
    const Params::wind_t &windp = _params._winds[ii];
    wind_data_t wind;
    gd.layers.wind.push_back(wind);
    wind_data_t &lwind = gd.layers.wind[ii];
    
    // marker type
    lwind.marker_type = default_marker_type;
    switch (windp.marker_type) {
      case Params::WIND_VECTOR:
        lwind.marker_type = VECTOR;
        break;
      case Params::WIND_BARB:
        lwind.marker_type = BARB;
        break;
      case Params::WIND_LABELEDBARB:
        lwind.marker_type = LABELEDBARB;
        break;
      case Params::WIND_TUFT:
        lwind.marker_type = TUFT;
        break;
      case Params::WIND_TICKVECTOR:
        lwind.marker_type = TICKVECTOR;
        break;
      case Params::WIND_METBARB:
        lwind.marker_type = METBARB;
        break;
      case Params::WIND_BARB_SH:
        lwind.marker_type = BARB_SH;
        break;
      case Params::WIND_LABELEDBARB_SH:
        lwind.marker_type = LABELEDBARB_SH;
        break;
      case Params::WIND_ARROW:
      default:
        lwind.marker_type = ARROWS;
        break;
    }
    STRcopy(lwind.color_name, windp.color, NAME_LENGTH);
    lwind.active = windp.on_at_startup;
    lwind.line_width = windp.line_width;
    // Sanity check
    if(lwind.line_width == 0 || lwind.line_width > 10) {
      lwind.line_width = 1;
    }

    // initialize the components

    lwind.wind_u = (met_record_t *) calloc(sizeof(met_record_t), 1);
    _initWindComponent(lwind.wind_u, windp, true, false, false);

    lwind.wind_v = (met_record_t *) calloc(sizeof(met_record_t), 1);
    _initWindComponent(lwind.wind_v, windp, false, true, false);

    if(strncasecmp(windp.w_field_name, "None", 4) != 0) {
      lwind.wind_w = (met_record_t *) calloc(sizeof(met_record_t), 1);
      _initWindComponent(lwind.wind_w, windp, false, false, true);
    } else {
      lwind.wind_w = NULL;
    }

    lwind.units_scale_factor = _params.wind_units_scale_factor;
    lwind.reference_speed = _params.wind_reference_speed;
    lwind.units_label = _params.wind_units_label;

  }

}

//////////////////////////////////
// initialize wind component

static void _initWindComponent(met_record_t *wrec,
                               const Params::wind_t &windp,
                               bool isU, bool isV, bool isW)
  
{
  
  wrec->h_data_valid = 0;
  wrec->v_data_valid = 0;
  wrec->h_vcm.nentries = 0;
  wrec->v_vcm.nentries = 0;
  wrec->h_fhdr.scale = -1.0;
  wrec->h_last_scale = 0.0;
  wrec->time_list.num_alloc_entries = 0;
  wrec->time_list.num_entries = 0;
  
  STRcopy(wrec->legend_name, windp.legend_label, NAME_LENGTH);
  if (isW) {
    sprintf(wrec->button_name, "%s_W ", windp.button_label);
  } else {
    STRcopy(wrec->button_name, windp.button_label, NAME_LENGTH);
  }
  STRcopy(wrec->url, windp.url, URL_LENGTH);
  
  /* Replace Underscores with spaces in names */
  if(_params.html_mode == 0 && _params.replace_underscores) {
    for(int jj = strlen(wrec->button_name) - 1; jj >= 0; jj--) {
      if(wrec->button_name[jj] == '_') {
        wrec->button_name[jj] = ' ';
      }
      if(wrec->legend_name[jj] == '_') {
        wrec->legend_name[jj] = ' ';
      }
    } // jj
  }
  
  // Append the field name

  if (isU) {
    strcat(wrec->url, windp.u_field_name);
  } else if (isV) {
    strcat(wrec->url, windp.v_field_name);
  } else {
    strcat(wrec->url, windp.w_field_name);
  }
  
  // units
  
  STRcopy(wrec->field_units, windp.units, LABEL_LENGTH);
  wrec->currently_displayed = 1;
  
  wrec->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval_mins;
  wrec->h_fhdr.proj_origin_lon = 0.0;
  wrec->h_fhdr.proj_origin_lat = 0.0;
  
  // instantiate classes for data retrieval
  
  wrec->h_mdvx = new DsMdvxThreaded;
  wrec->v_mdvx = new DsMdvxThreaded;
  wrec->h_mdvx_int16 = new MdvxField;
  wrec->v_mdvx_int16 = new MdvxField;

  // projection
  
  wrec->proj = new MdvxProj;

}

////////////////////////////////////////////////////////////////
// initialize terrain

static void _initTerrain()
{

  if (strlen(_params.terrain_url) > 0) {
    
    gd.layers.earth.terrain_active = 1;
    gd.layers.earth.terr = (met_record_t *) calloc(sizeof(met_record_t), 1);
    if(gd.layers.earth.terr == NULL) {
      fprintf(stderr,"Cannot allocate space for terrain data\n");
      exit(-1);
    }
    
    gd.layers.earth.terr->time_allowance = 1000000000; // 30+ years
    STRcopy(gd.layers.earth.terr->color_file,
            _params.landuse_colorscale,NAME_LENGTH);
    STRcopy(gd.layers.earth.terr->button_name,
            _params.terrain_id_label,NAME_LENGTH);
    STRcopy(gd.layers.earth.terr->legend_name,
            _params.terrain_id_label,NAME_LENGTH);
    STRcopy(gd.layers.earth.terr->url,
            _params.terrain_url,URL_LENGTH);
    
    gd.layers.earth.terr->h_mdvx = new DsMdvxThreaded;
    gd.layers.earth.terr->v_mdvx = new DsMdvxThreaded;
    gd.layers.earth.terr->h_mdvx_int16 = new MdvxField;
    gd.layers.earth.terr->v_mdvx_int16 = new MdvxField;
    gd.layers.earth.terr->proj =  new MdvxProj;

  }
  
  if (strlen(_params.landuse_url) > 0) {

    gd.layers.earth.landuse_active = (_params.landuse_active == true)? 1: 0;
    gd.layers.earth.land_use = (met_record_t *) calloc(sizeof(met_record_t), 1);
    if(gd.layers.earth.land_use == NULL) {
      fprintf(stderr,"Cannot allocate space for land_use data\n");
      exit(-1);
    }
    
    gd.layers.earth.land_use->time_allowance = 1000000000; // 30+ years
    STRcopy(gd.layers.earth.land_use->color_file,
            _params.landuse_colorscale, NAME_LENGTH);
    STRcopy(gd.layers.earth.land_use->button_name,
            _params.terrain_id_label,  NAME_LENGTH);
    STRcopy(gd.layers.earth.land_use->legend_name,
            _params.terrain_id_label, NAME_LENGTH);
    STRcopy(gd.layers.earth.land_use->url,
            _params.landuse_url, URL_LENGTH);
    
    gd.layers.earth.land_use->h_mdvx = new DsMdvxThreaded;
    gd.layers.earth.land_use->v_mdvx = new DsMdvxThreaded;
    gd.layers.earth.land_use->h_mdvx_int16 = new MdvxField;
    gd.layers.earth.land_use->v_mdvx_int16 = new MdvxField;
    gd.layers.earth.land_use->proj =  new MdvxProj;
    
    switch(_params.landuse_render_method) {
      default:
      case Params::TERRAIN_RENDER_RECTANGLES:
        gd.layers.earth.land_use->render_method = POLYGONS;
        break;
        
      case Params::TERRAIN_RENDER_FILLED_CONT:
        gd.layers.earth.land_use->render_method = FILLED_CONTOURS;
        break;
        
      case Params::TERRAIN_RENDER_DYNAMIC_CONTOURS:
        gd.layers.earth.land_use->render_method = DYNAMIC_CONTOURS;
        break;
    }
    
  }

}

////////////////////////////////////////////////////////////////
// INIT_DRAW_EXPORT_LINKS:  Scan param file and setup links to
//  for drawn and exported points 

static void _initDrawExport()
{

  gd.draw.num_draw_products = _params.draw_export_info_n;
  gd.draw.dexport = (draw_export_info_t *)
    calloc(gd.draw.num_draw_products, sizeof(draw_export_info_t));
  if (gd.draw.dexport == NULL) {
    fprintf(stderr,"Unable to allocate space for %d draw.dexport sets\n",
            gd.draw.num_draw_products);
    perror("Qucid");
    exit(-1);
  }
  
  for(int ii = 0; ii < _params.draw_export_info_n;  ii++) {
    
    Params::draw_export_t &dinfo = _params._draw_export_info[ii];
    draw_export_info_t &dexp = gd.draw.dexport[ii];

    // Product ID label
    int len = strlen(dinfo.id_label) + 1;
    dexp.product_id_label = (char *) calloc(1, len);
    STRcopy(dexp.product_id_label, dinfo.id_label, len);
    
    // product_label_text
    dexp.product_label_text = (char *) calloc(1,TITLE_LENGTH);
    STRcopy(dexp.product_label_text, dinfo.default_label, TITLE_LENGTH);
    
    // FMQ URL 
    len = strlen(dinfo.url) + 1;
    if(len > NAME_LENGTH) {
      fprintf(stderr,"URL: %s too long - Must be less than %d chars. Sorry.",
              dinfo.url,URL_LENGTH);
      exit(-1);
    }
    dexp.product_fmq_url = (char *) calloc(1, URL_LENGTH);
    STRcopy(dexp.product_fmq_url, dinfo.url, URL_LENGTH);
    
    // Get the Default valid time  
    dexp.default_valid_minutes = dinfo.valid_minutes;
    
    // Get the Default ID   
    dexp.default_serial_no = dinfo.default_id_no;
    
  } // ii

}

////////////////////////////////////////////////////////////////
// Initialize route winds

#define NUM_PARSE_FIELDS (MAX_ROUTE_SEGMENTS +2) * 3
#define PARSE_FIELD_SIZE 1024

static void _initRouteWinds()
{

  // U WINDS Met Record

  if(strlen(_params.route_u_url) > 1) {

    met_record_t *mr = (met_record_t *) calloc(sizeof(met_record_t), 1);
    if(mr == NULL) {
      fprintf(stderr,"Unable to allocate space for Route U Wind\n");
      perror("cidd_init::_initRouteWinds");
      exit(-1);
    }
    gd.layers.route_wind.u_wind = mr;
    mr->h_data_valid = 0;
    mr->v_data_valid = 0;
    mr->v_vcm.nentries = 0;
    mr->h_vcm.nentries = 0;
    mr->h_fhdr.scale = -1.0;
    mr->h_last_scale = 0.0;
    STRcopy(mr->legend_name, "ROUTE_U_WIND", NAME_LENGTH);
    STRcopy(mr->button_name, "ROUTE_U_WIND", NAME_LENGTH);
    STRcopy(mr->url, _params.route_u_url, URL_LENGTH);

    STRcopy(mr->field_units,"unknown",LABEL_LENGTH);
    mr->currently_displayed = 1;
    mr->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval_mins;
    mr->h_fhdr.proj_origin_lon = 0.0;
    mr->h_fhdr.proj_origin_lat = 0.0;

    // instantiate DsMdvxThreaded class
    mr->v_mdvx = new DsMdvxThreaded;
    mr->v_mdvx_int16 = new MdvxField;

  } // U WINDS

  // V WINDS Met Record
  
  if(strlen(_params.route_v_url) > 1) {
    
    met_record_t *mr = (met_record_t *) calloc(sizeof(met_record_t), 1);
    if(mr == NULL) {
      fprintf(stderr,"Unable to allocate space for Route V Wind\n");
      perror("cidd_init::_initRouteWinds");
      exit(-1);
    }
    gd.layers.route_wind.v_wind = mr;
    mr->h_data_valid = 0;
    mr->v_data_valid = 0;
    mr->v_vcm.nentries = 0;
    mr->h_vcm.nentries = 0;
    mr->h_fhdr.scale = -1.0;
    mr->h_last_scale = 0.0;
    STRcopy(mr->legend_name, "ROUTE_V_WIND", NAME_LENGTH);
    STRcopy(mr->button_name, "ROUTE_V_WIND", NAME_LENGTH);
    STRcopy(mr->url, _params.route_v_url, URL_LENGTH);
    
    STRcopy(mr->field_units, "unknown", LABEL_LENGTH);
    mr->currently_displayed = 1;
    mr->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval_mins;
    mr->h_fhdr.proj_origin_lon = 0.0;
    mr->h_fhdr.proj_origin_lat = 0.0;
    
    // instantiate DsMdvxThreaded class
    mr->v_mdvx = new DsMdvxThreaded;
    mr->v_mdvx_int16 = new MdvxField;

  } // V WINDS

  // TURB Met Record

  if(strlen(_params.route_turb_url) > 1) {

    met_record_t *mr = (met_record_t *) calloc(sizeof(met_record_t), 1);
    if(mr == NULL) {
      fprintf(stderr,"Unable to allocate space for Route TURB\n");
      perror("cidd_init::_initRouteWinds");
      exit(-1);
    }
    gd.layers.route_wind.turb = mr;
    mr->h_data_valid = 0;
    mr->v_data_valid = 0;
    mr->v_vcm.nentries = 0;
    mr->h_vcm.nentries = 0;
    mr->h_fhdr.scale = -1.0;
    mr->h_last_scale = 0.0;
    STRcopy(mr->legend_name, "ROUTE_TURB", NAME_LENGTH);
    STRcopy(mr->button_name, "ROUTE_TURB", NAME_LENGTH);
    STRcopy(mr->url, _params.route_turb_url, URL_LENGTH);
    
    STRcopy(mr->field_units, "unknown", LABEL_LENGTH);
    mr->currently_displayed = 1;
    mr->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval_mins;
    mr->h_fhdr.proj_origin_lon = 0.0;
    mr->h_fhdr.proj_origin_lat = 0.0;

    // instantiate DsMdvxThreaded class
    mr->v_mdvx = new DsMdvxThreaded;
    mr->v_mdvx_int16 = new MdvxField;
    
  } // TURB

  // ICING met Record
  
  if(strlen(_params.route_icing_url) > 1) {

    met_record_t *mr = (met_record_t *) calloc(sizeof(met_record_t), 1);
    if(mr == NULL) {
      fprintf(stderr,"Unable to allocate space for Route ICING\n");
      perror("cidd_init::_initRouteWinds");
      exit(-1);
    }
    gd.layers.route_wind.icing = mr;
    mr->h_data_valid = 0;
    mr->v_data_valid = 0;
    mr->v_vcm.nentries = 0;
    mr->h_vcm.nentries = 0;
    mr->h_fhdr.scale = -1.0;
    mr->h_last_scale = 0.0;
    STRcopy(mr->legend_name, "ROUTE_ICING", NAME_LENGTH);
    STRcopy(mr->button_name, "ROUTE_ICING", NAME_LENGTH);
    STRcopy(mr->url, _params.route_icing_url, URL_LENGTH);

    STRcopy(mr->field_units,"unknown",LABEL_LENGTH);
    mr->currently_displayed = 1;
    mr->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval_mins;
    mr->h_fhdr.proj_origin_lon = 0.0;
    mr->h_fhdr.proj_origin_lat = 0.0;

    // instantiate DsMdvxThreaded class
    mr->v_mdvx = new DsMdvxThreaded;
    mr->v_mdvx_int16 = new MdvxField;

  } // ICING

  // How many are route are defined in the file
  gd.layers.route_wind.num_predef_routes = _params.route_paths_n;

  // Allocate space for num_predef_routes + 1 for the custom/user defined route

  if((gd.layers.route_wind.route =(route_track_t *) 
      calloc(gd.layers.route_wind.num_predef_routes + 1, sizeof(route_track_t))) == NULL) {
    fprintf(stderr,"Unable to allocate space for %d Routes\n",
            gd.layers.route_wind.num_predef_routes + 1);
    perror("CIDD route_winds_init");
    exit(-1);
  }
  
  char *cfield[NUM_PARSE_FIELDS];
  for(int ii = 0; ii < NUM_PARSE_FIELDS; ii++) {
    cfield[ii] =(char *) calloc(PARSE_FIELD_SIZE, 1);
  }
  
  for(int ii = 0; ii < gd.layers.route_wind.num_predef_routes; ii++) {
    
    int num_fields = STRparse(_params._route_paths[ii], cfield,
                              strlen(_params._route_paths[ii]),
                              NUM_PARSE_FIELDS, PARSE_FIELD_SIZE);
    if(num_fields == NUM_PARSE_FIELDS) {
      fprintf(stderr,"Warning: Route path: %s\n Too long. Only %d segments allowed \n",
              _params._route_paths[ii], MAX_ROUTE_SEGMENTS);
    }
    
    // Collect Label
    strncpy(gd.layers.route_wind.route[ii].route_label, cfield[0], 62);
    
    // Collect the number of points  & segments 
    gd.layers.route_wind.route[ii].num_segments = atoi(cfield[1]) -1;
    if(_params.route_debug) {
      fprintf(stderr,"\nRoute: %s - %d segments\n",
              gd.layers.route_wind.route[ii].route_label,
              gd.layers.route_wind.route[ii].num_segments);
    }

    // Sanity check
    if(gd.layers.route_wind.route[ii].num_segments <= 0 || 
       gd.layers.route_wind.route[ii].num_segments >  MAX_ROUTE_SEGMENTS) {
      fprintf(stderr,"Warning: Route path: %s\n Error Only 1-%d segments allowed \n",
              _params._route_paths[ii], MAX_ROUTE_SEGMENTS);
      continue;
    }
    
    int index = 2; // The first triplet.
    // Pick up each triplet
    for(int kk = 0; kk <= gd.layers.route_wind.route[ii].num_segments; kk++, index+=3 ) {

      strncpy(gd.layers.route_wind.route[ii].navaid_id[kk], cfield[index], 16);
      gd.layers.route_wind.route[ii].y_world[kk] = atof(cfield[index +1]);
      gd.layers.route_wind.route[ii].x_world[kk] = atof(cfield[index +2]);

      switch (gd.display_projection) {
        case  Mdvx::PROJ_LATLON:
          normalize_longitude(gd.h_win.min_x, gd.h_win.max_x,
                              &gd.layers.route_wind.route[ii].x_world[kk]);
          break;

        default :
          normalize_longitude(-180.0, 180.0,
                              &gd.layers.route_wind.route[ii].x_world[kk]);
          break;

      } 

      gd.proj.latlon2xy(gd.layers.route_wind.route[ii].y_world[kk],
                        gd.layers.route_wind.route[ii].x_world[kk],
                        gd.layers.route_wind.route[ii].x_world[kk],
                        gd.layers.route_wind.route[ii].y_world[kk]);

      if(_params.route_debug) {
        fprintf(stderr,"%s:  %g,    %g\n",
                gd.layers.route_wind.route[ii].navaid_id[kk],
                gd.layers.route_wind.route[ii].x_world[kk],
                gd.layers.route_wind.route[ii].y_world[kk]);
      }
    }
    
    // Compute the segment lengths
    gd.layers.route_wind.route[ii].total_length = 0.0;
    for(int kk = 0; kk < gd.layers.route_wind.route[ii].num_segments; kk++) {
      gd.layers.route_wind.route[ii].seg_length[kk] = 
        disp_proj_dist(gd.layers.route_wind.route[ii].x_world[kk],
                       gd.layers.route_wind.route[ii].y_world[kk],
                       gd.layers.route_wind.route[ii].x_world[kk+1],
                       gd.layers.route_wind.route[ii].y_world[kk+1]);
      gd.layers.route_wind.route[ii].total_length += gd.layers.route_wind.route[ii].seg_length[kk];
    }
    
  } // ii

  // Copy the initial route definition into the space reserved for the Custom route
  memcpy(gd.layers.route_wind.route + gd.layers.route_wind.num_predef_routes,
         &gd.h_win.route,sizeof(route_track_t));

  /* free temp space */
  for(int ii = 0; ii < NUM_PARSE_FIELDS; ii++) {
    free(cfield[ii]);
  }
  
  if (_params.route_winds_active) {
    gd.layers.route_wind.has_params = 1;
    // Use the first route as the default.
    memcpy(&gd.h_win.route, gd.layers.route_wind.route, sizeof(route_track_t)); 
  } else {
    gd.layers.route_wind.has_params = 0;
  }

}

/************************************************************************
 * LOAD_RAP_MAP - load map in RAP format
 */

static void _loadRapMap(Overlay_t *ov, const char *maps_url)
{

  int    i,j;
  int    index,found;
  int    len,point;
  int    num_points;        
  int    num_fields;  /* number of fields (tokens) found in input line */
  int    map_len;
  int    ret_stat;
  char   *str_ptr;
  char   *map_buf;         // Buffer to hold map file
  char    name_buf[2048];  /* Buffer for input lines */
  char    dirname[2048];   /* Buffer for directories to search */
  FILE    *mapfile;
  char    *cfield[32];
  struct stat sbuf;
  char *lasts;

  for(i=0; i < 32; i++)  cfield[i] = (char *) calloc(1,64);  /* get space for sub strings */

  // Add . to list to start.
  strncpy(dirname, ".,", 1024);
  strncat(dirname, maps_url, 1024);

  str_ptr = strtok(dirname,","); // Prime strtok

  do {  // Try each comma delimited subdir
    
    while(*str_ptr == ' ') str_ptr++; //skip any leading space
    snprintf(name_buf, 2047, "%s/%s", str_ptr, ov->map_file_name.c_str());

    // Check if it's a HTTP URL
    if(strncasecmp(name_buf,"http:",5) == 0) {
      if(strlen(_params.http_proxy_url)  > URL_MIN_SIZE) {
        ret_stat =  HTTPgetURL_via_proxy(_params.http_proxy_url,
                                         name_buf,_params.data_timeout_secs * 1000,
                                         &map_buf, &map_len);
      } else {
        ret_stat =  HTTPgetURL(name_buf,
                               _params.data_timeout_secs * 1000,
                               &map_buf, &map_len);
      }
      if(ret_stat <=0 ) {
        map_len = 0;
        map_buf = NULL;
      }
      if(gd.debug) fprintf(stderr,"Map: %s: Len: %d\n",name_buf,map_len);
    } else {
      if(stat(name_buf,&sbuf) < 0) { // Stat to find the file's size
        map_len = 0;
        map_buf = NULL;
      }
      if((mapfile = fopen(name_buf,"r")) == NULL) {
        map_len = 0;
        map_buf = NULL;
      } else {
        if((map_buf = (char *)  calloc(sbuf.st_size + 1 ,1)) == NULL) {
          fprintf(stderr,"Problems allocating %ld bytes for map file\n",
                  (long) sbuf.st_size);
          exit(-1);
        }

        // Read
        if((map_len = fread(map_buf,1,sbuf.st_size,mapfile)) != sbuf.st_size) {
          fprintf(stderr,"Problems reading RAP map: %s\n", name_buf);
          exit(-1);
        }
        map_buf[sbuf.st_size] = '\0'; // Make sure to null terminate
        fclose(mapfile);
      }
    }
  } while ((str_ptr = strtok(NULL,",")) != NULL && map_len == 0 );

  if(map_len == 0 || map_buf == NULL) {
    fprintf(stderr,"Warning!: Unable to load map file: %s\n", ov->map_file_name.c_str());
    for(i=0; i < 32; i++)  free(cfield[i]);
    return;
  }

  // Prime strtok_r;
  str_ptr = strtok_r(map_buf,"\n",&lasts);

  while (str_ptr != NULL) {        /* read all lines in buffer */
    if(*str_ptr != '#') {

      if(strncasecmp(str_ptr,"MAP_NAME",8) == 0) {    /* Currently Ignore */
        str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
        continue;
      }

      if(strncasecmp(str_ptr,"TRANSFORM",9) == 0) {    /* Currently Ignore */
        str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
        continue;
      }

      if(strncasecmp(str_ptr,"PROJECTION",10) == 0) {    /* Currently Ignore */
        str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
        continue;
      }

      if(strncasecmp(str_ptr,"ICONDEF",7) == 0) {        /* describes an icon's coordinates in pixels */
        index = ov->num_icondefs;
        if(index >= ov->num_alloc_icondefs) {
          if(ov->num_alloc_icondefs == 0) { /* start with space for 2 */
            ov->geo_icondef = (Geo_feat_icondef_t **)
              calloc(2,sizeof(Geo_feat_icondef_t *));
            ov->num_alloc_icondefs = 2;
          } else { /* Double the space */
            ov->num_alloc_icondefs *= 2;
            ov->geo_icondef = (Geo_feat_icondef_t **) 
              realloc(ov->geo_icondef, ov->num_alloc_icondefs * sizeof(Geo_feat_icondef_t *)); 
          }
        }
        if(ov->geo_icondef == NULL) {
          fprintf(stderr,"Unable to allocate memory for Icon definition pointer array!\n");
          exit(-1);
        }

        if(STRparse(str_ptr,cfield,256,32,64) != 3) {
          fprintf(stderr,"Error in ICONDEF line: %s\n",str_ptr);
          exit(-1);
        }
        /* get space for the icon definition */
        ov->geo_icondef[index] = (Geo_feat_icondef_t *) calloc(1,sizeof(Geo_feat_icondef_t));
        ZERO_STRUCT(ov->geo_icondef[index]);

        if(ov->geo_icondef[index] == NULL) {
          fprintf(stderr,"Unable to allocate memory for Icon definition!\n");
          exit(-1);
        }
        STRcopy(ov->geo_icondef[index]->name,cfield[1],NAME_LENGTH);
        num_points = atoi(cfield[2]);

        /* Get space for points in the icon */
        ov->geo_icondef[index]->x = (short *) calloc(1,num_points * sizeof(short));
        ov->geo_icondef[index]->y = (short *) calloc(1,num_points * sizeof(short));

        if(ov->geo_icondef[index]->x == NULL || ov->geo_icondef[index]->y == NULL) {
          fprintf(stderr,"Error!: Unable to allocate space for icon points in file %s, num points: %d\n",
                  ov->map_file_name.c_str(),num_points);
          exit(-1);
        }

        /* Read in all of the points */
        for(j=0,point = 0; j < num_points; j++) {

          str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line

          if(str_ptr != NULL && STRparse(str_ptr,cfield,256,32,64) == 2) {
            ov->geo_icondef[index]->x[point] = atoi(cfield[0]);
            ov->geo_icondef[index]->y[point] = atoi(cfield[1]);
            point++;
          }
        }
        ov->geo_icondef[index]->num_points = point;
        ov->num_icondefs++;
        str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
        continue;
      }

      if(strncasecmp(str_ptr,"ICON ",5) == 0) {    
        index = ov->num_icons;
        if(index >= ov->num_alloc_icons) {
          if(ov->num_alloc_icons == 0) { /* start with space for 2 */
            ov->geo_icon = (Geo_feat_icon_t **)
              calloc(2,sizeof(Geo_feat_icon_t *));
            ov->num_alloc_icons = 2;
          } else {  /* Double the space */
            ov->num_alloc_icons *= 2;
            ov->geo_icon = (Geo_feat_icon_t **) 
              realloc(ov->geo_icon, ov->num_alloc_icons * sizeof(Geo_feat_icon_t *)); 
          }
        }
        if(ov->geo_icon == NULL) {
          fprintf(stderr,"Unable to allocate memory for Icon pointer array!\n");
          exit(-1);
        }

        /* get space for the Icon */
        ov->geo_icon[index] = (Geo_feat_icon_t *) calloc(1,sizeof(Geo_feat_icon_t));
        ZERO_STRUCT(ov->geo_icon[index]);

        if(ov->geo_icon[index] == NULL) {
          fprintf(stderr,"Unable to allocate memory for Icon definition!\n");
          exit(-1);
        }

        if((num_fields = STRparse(str_ptr,cfield,256,32,64)) < 6) {
          fprintf(stderr,"Error in ICON line: %s\n",str_ptr);
          exit(-1);
        }

        /* find the definition for the line segments that make up the icon */
        ov->geo_icon[index]->icon = NULL;
        found = 0;
        for(j=0; j < ov->num_icondefs && found == 0; j++) {
          if(strcmp(ov->geo_icondef[j]->name,cfield[1]) == 0) {
            ov->geo_icon[index]->icon = ov->geo_icondef[j];
            found = 1;
          }
        }

        if(found == 0) {    
          fprintf(stderr,"No Icon definition: %s found in file %s!\n",cfield[1],ov->map_file_name.c_str());
          str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
          continue;
        }

        /* record its position */
        ov->geo_icon[index]->lat = atof(cfield[2]);
        ov->geo_icon[index]->lon = atof(cfield[3]);
        ov->geo_icon[index]->text_x = atoi(cfield[4]);
        ov->geo_icon[index]->text_y = atoi(cfield[5]);

        /* gather up remaining text fields */
        ov->geo_icon[index]->label[0] = '\0';
        len = 2;
        for(j = 6; j < num_fields && len < LABEL_LENGTH; j++ ) {
          strncat(ov->geo_icon[index]->label,cfield[j],LABEL_LENGTH - len);
          len = strlen(ov->geo_icon[index]->label) +1;

          // Separate multiple text label fiedds with spaces.
          if( j < num_fields -1) {
            strncat(ov->geo_icon[index]->label," ",LABEL_LENGTH - len);
            len = strlen(ov->geo_icon[index]->label) +1;
          }
        }
        {
          int labellen = strlen(ov->geo_icon[index]->label);
          if (labellen > 1) {
            if (ov->geo_icon[index]->label[labellen-1] == ' ') {
              ov->geo_icon[index]->label[labellen-1] = '\0';
            }
          }
        }

        ov->num_icons++;
        str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
        continue;
      }

      if(strncasecmp(str_ptr,"POLYLINE",8) == 0) {    
        index = ov->num_polylines;
        if(index >= ov->num_alloc_polylines) {
          if(ov->num_alloc_polylines == 0) { /* start with space for 2 */
            ov->geo_polyline = (Geo_feat_polyline_t **)
              calloc(2,sizeof(Geo_feat_polyline_t *));
            ov->num_alloc_polylines = 2;
          } else {  /* Double the space */
            ov->num_alloc_polylines *= 2;
            ov->geo_polyline = (Geo_feat_polyline_t **) 
              realloc(ov->geo_polyline, ov->num_alloc_polylines * sizeof(Geo_feat_polyline_t *)); 
          }
        }
        if(ov->geo_polyline == NULL) {
          fprintf(stderr,"Unable to allocate memory for Polyline pointer array!\n");
          exit(-1);
        }

        if((STRparse(str_ptr,cfield,256,32,64)) != 3) {
          fprintf(stderr,"Error in POLYLINE line: %s\n",str_ptr);
          exit(-1);
        }
        /* get space for the Polyline definition */
        ov->geo_polyline[index] = (Geo_feat_polyline_t *) calloc(1,sizeof(Geo_feat_polyline_t));
        ZERO_STRUCT(ov->geo_polyline[index]);

        if(ov->geo_polyline[index] == NULL) {
          fprintf(stderr,"Unable to allocate memory for Polyline definition!\n");
          exit(-1);
        }
        STRcopy(ov->geo_polyline[index]->label,cfield[1],LABEL_LENGTH);
        num_points = atoi(cfield[2]);
        if(num_points <=0 ) {
          fprintf(stderr,"Warning!: Bad POLYLINE Definition. File: %s, Line: %s\n",name_buf,str_ptr);
          fprintf(stderr,"        : Format should be:    POLYLINE Label #points\n");
          fprintf(stderr,"        : Skipping \n");
          str_ptr = strtok_r(NULL,"\n",&lasts); // move to next line
          continue;
        }

        /* Get space for points in the polyline */
        ov->geo_polyline[index]->lat = (double *) calloc(1,num_points * sizeof(double));
        ov->geo_polyline[index]->lon = (double *) calloc(1,num_points * sizeof(double));
        ov->geo_polyline[index]->local_x = (double *) calloc(1,num_points * sizeof(double));
        ov->geo_polyline[index]->local_y = (double *) calloc(1,num_points * sizeof(double));

        if(ov->geo_polyline[index]->lat == NULL || ov->geo_polyline[index]->lon == NULL) {
          fprintf(stderr,"Error!: Unable to allocate space for polyline points in file %s, num points: %d\n",
                  ov->map_file_name.c_str(),num_points);
          exit(-1);
        }

        /* Read in all of the points */
        for(j=0,point = 0; j < num_points; j++) {
          str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
          if(str_ptr != NULL && STRparse(str_ptr,cfield,256,32,64) >= 2) {
            ov->geo_polyline[index]->lat[point] = atof(cfield[0]);
            ov->geo_polyline[index]->lon[point] = atof(cfield[1]);
            point++;
          }
        }
        ov->geo_polyline[index]->num_points = point;
        ov->num_polylines++;
        str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
        continue;
      }

      if(strncasecmp(str_ptr,"LABEL",5) == 0) {    
        index = ov->num_labels;
        if(index >= ov->num_alloc_labels) {
          if(ov->num_alloc_labels == 0) { /* start with space for 2 */
            ov->geo_label = (Geo_feat_label_t **)
              calloc(2,sizeof(Geo_feat_label_t *));
            ov->num_alloc_labels = 2;
          } else {  /* Double the space */
            ov->num_alloc_labels *=2;
            ov->geo_label = (Geo_feat_label_t **) 
              realloc(ov->geo_label, ov->num_alloc_labels * sizeof(Geo_feat_label_t *)); 
          }
        }
        if(ov->geo_label == NULL) {
          fprintf(stderr,"Unable to allocate memory for Label pointer array!\n");
          exit(-1);
        }

        ov->num_labels++;
                     
        /* get space for the Label definition */
        ov->geo_label[index] = (Geo_feat_label_t *) calloc(1,sizeof(Geo_feat_label_t));
        ZERO_STRUCT(ov->geo_label[index]);

        if(ov->geo_label[index] == NULL) {
          fprintf(stderr,"Unable to allocate memory for Label definition!\n");
          exit(-1);
        }

        if((num_fields = STRparse(str_ptr,cfield,256,32,64)) < 8) {
          fprintf(stderr,"Too few fields in LABEL line: %s\n",str_ptr);
          str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
          continue;
        } 
        ov->geo_label[index]->min_lat = atof(cfield[1]);
        ov->geo_label[index]->min_lon = atof(cfield[2]);
        ov->geo_label[index]->max_lat = atof(cfield[3]);
        ov->geo_label[index]->max_lon = atof(cfield[4]);
        ov->geo_label[index]->rotation = atof(cfield[5]);
        ov->geo_label[index]->attach_lat = atof(cfield[6]);
        ov->geo_label[index]->attach_lon = atof(cfield[7]);

        ov->geo_label[index]->display_string[0] = '\0';
        len = 2;
        for(j = 8; j < num_fields && len < NAME_LENGTH; j++) {
          strncat(ov->geo_label[index]->display_string,cfield[j],NAME_LENGTH - len);
          len = strlen(ov->geo_label[index]->display_string) +1;
          if(j < num_fields -1)
            strncat(ov->geo_label[index]->display_string," ",NAME_LENGTH - len);
          len = strlen(ov->geo_label[index]->display_string) +1;
        }
        str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
        continue;
      }


      if(strncasecmp(str_ptr,"SIMPLELABEL",11) == 0) {    
        index = ov->num_labels;
        if(index >= ov->num_alloc_labels) {
          if(ov->num_alloc_labels == 0) { /* start with space for 2 */
            ov->geo_label = (Geo_feat_label_t **)
              calloc(2,sizeof(Geo_feat_label_t *));
            ov->num_alloc_labels = 2;
          } else {  /* Double the space */
            ov->num_alloc_labels *=2;
            ov->geo_label = (Geo_feat_label_t **) 
              realloc(ov->geo_label, ov->num_alloc_labels * sizeof(Geo_feat_label_t *)); 
          }
        }
        if(ov->geo_label == NULL) {
          fprintf(stderr,"Unable to allocate memory for Label pointer array!\n");
          exit(-1);
        }
        ov->num_labels++;

        /* get space for the Label definition */
        ov->geo_label[index] = (Geo_feat_label_t *) calloc(1,sizeof(Geo_feat_label_t));
        ZERO_STRUCT(ov->geo_label[index]);

        if(ov->geo_label[index] == NULL) {
          fprintf(stderr,"Unable to allocate memory for Label definition!\n");
          exit(-1);
        }

        if((num_fields = STRparse(str_ptr,cfield,256,32,64)) < 4) {
          fprintf(stderr,"Too few fields in SIMPLELABEL line: %s\n",str_ptr);
          str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
          continue;
        } 
        ov->geo_label[index]->min_lat = atof(cfield[1]);
        ov->geo_label[index]->min_lon = atof(cfield[2]);
        ov->geo_label[index]->max_lat = atof(cfield[1]);
        ov->geo_label[index]->max_lon = atof(cfield[2]);
        ov->geo_label[index]->rotation = 0;
        ov->geo_label[index]->attach_lat = atof(cfield[1]);
        ov->geo_label[index]->attach_lon = atof(cfield[2]);

        ov->geo_label[index]->display_string[0] = '\0';
        len = 2;
        for(j = 3; j < num_fields && len < NAME_LENGTH; j++) {
          strncat(ov->geo_label[index]->display_string,cfield[j],NAME_LENGTH - len);
          len = strlen(ov->geo_label[index]->display_string) +1;
          if(j < num_fields -1)
            strncat(ov->geo_label[index]->display_string," ",NAME_LENGTH - len);
          len = strlen(ov->geo_label[index]->display_string) +1;
        }
        str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
        continue;
      }

    } 

    // Nothing matches
    str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line

  }  // End of while additional lines exist in buffer

  if(map_buf!= NULL) {
    free(map_buf);
    map_buf = NULL;
  }

  for(i=0; i < 32; i++)  free(cfield[i]);         /* free space for sub strings */
  return;

}

/************************************************************************
 * LOAD_SHAPE_OVERLAY_DATA: This version reads Shape files
 */

static void _loadShapeMap(Overlay_t *ov, const char *maps_url)
{
  int    i,j;
  int    index,found,is_http;
  int    point;
  int    num_points;        
  int    ret_stat;
  char   *str_ptr;
  char    name_base[1024];  /* Buffer for input names */
  char    dirname[4096];   /* Buffer for directories to search */
  char    name_buf[2048];  /* Buffer for input names */
  char    name_buf2[2048]; /* Buffer for input names */
  char    *map_buf;
  int     map_len;

  SHPHandle SH;
  SHPObject *SO;
  FILE    *map_file;

  int pid = getpid();

  // Add . to list of dirs to search  to start.
  strncpy(dirname,".,",2048);
  strncat(dirname,maps_url,2048);


  found = 0;
  is_http = 0;

  // Search each subdir
  str_ptr = strtok(dirname,","); // Prime strtok
  do{  //  Search 

    while(*str_ptr == ' ') str_ptr++; //skip any leading space

    sprintf(name_buf,"%s/%s,",str_ptr,ov->map_file_name.c_str());

    // Check if it's a HTTP URL
    if(strncasecmp(name_buf,"http:",5) == 0) {

      // Extract name base
      strncpy(name_base,ov->map_file_name.c_str(),1023);
      char *ptr = strrchr(name_base,'.');
      if(ptr != NULL) *ptr = '\0';

      // Download  SHP Part of shapefile
      sprintf(name_buf,"%s/%s.shp",str_ptr,name_base);
      if(strlen(_params.http_proxy_url)  > URL_MIN_SIZE) {
        ret_stat = HTTPgetURL_via_proxy(_params.http_proxy_url,
                                        name_buf,_params.data_timeout_secs * 1000,
                                        &map_buf, &map_len);
      } else {
        ret_stat =  HTTPgetURL(name_buf,
                               _params.data_timeout_secs * 1000,
                               &map_buf, &map_len);
      }
      if(ret_stat > 0 && map_len > 0 ) { // Succeeded
        is_http = 1;

        if(gd.debug) fprintf(stderr,"Read Shape File: %s: Len: %d\n",name_buf,map_len);

        sprintf(name_buf2,"/tmp/%d_%s.shp",pid,name_base);
        if((map_file = fopen(name_buf2,"w")) == NULL) {
          fprintf(stderr,"Problems Opening %s for writing\n",name_buf2);
          perror("CIDD ");
          exit(-1);
        }
        if(fwrite(map_buf,map_len,1,map_file) != 1) {
          fprintf(stderr,"Problems Writing to %s \n",name_buf2);
          perror("CIDD ");
          exit(-1);
        }
        fclose(map_file);
        if(map_buf != NULL) free(map_buf);
      }

      // Download  SHX Part of shapefile
      sprintf(name_buf,"%s/%s.shx",str_ptr,name_base);
      if(strlen(_params.http_proxy_url)  > URL_MIN_SIZE) {
        ret_stat = HTTPgetURL_via_proxy(_params.http_proxy_url,
                                        name_buf,_params.data_timeout_secs * 1000,
                                        &map_buf, &map_len);

      } else {
        ret_stat =  HTTPgetURL(name_buf,
                               _params.data_timeout_secs * 1000,
                               &map_buf, &map_len);
      }
      if(ret_stat > 0  && map_len > 0) { // Succeeded

        if(gd.debug) fprintf(stderr,"Read Shape File: %s: Len: %d\n",name_buf,map_len);

        sprintf(name_buf2,"/tmp/%d_%s.shx",pid,name_base);
        if((map_file = fopen(name_buf2,"w")) == NULL) {
          fprintf(stderr,"Problems Opening %s for writing\n",name_buf2);
          perror("CIDD ");
          exit(-1);
        }
        if(fwrite(map_buf,map_len,1,map_file) != 1) {
          fprintf(stderr,"Problems Writing to %s \n",name_buf2);
          perror("CIDD ");
          exit(-1);
        }
        fclose(map_file);
        if(map_buf != NULL) free(map_buf);
      }

      sprintf(name_buf,"/tmp/%d_%s",pid,name_base);
      if((SH = SHPOpen(name_buf,"rb")) != NULL) {
        found = 1;
      } else {
        fprintf(stderr,"Problems with SHPOpen on %s \n",name_buf);
      }

    } else {  // Looks like a regular file

      sprintf(name_buf,"%s/%s,",str_ptr,ov->map_file_name.c_str());
      if((SH = SHPOpen(name_buf,"rb")) != NULL) {
        found = 1;
      }
    }

  } while ((str_ptr = strtok(NULL,",")) != NULL && found == 0 );

  if( found == 0) {
    fprintf(stderr,"Warning!: Unable to load map file: %s\n",ov->map_file_name.c_str());
    if(is_http) {  // Unlink temporary files
      sprintf(name_buf2,"/tmp/%d_%s.shp",pid,name_base);
      unlink(name_buf2);
      sprintf(name_buf2,"/tmp/%d_%s.shx",pid,name_base);
      unlink(name_buf2);
    }
		
    return;
  }


  // Shape File is Found and Open

  int n_objects;
  int shape_type;
  int part_num;

  SHPGetInfo(SH, &n_objects, &shape_type, NULL, NULL);

  if(gd.debug) {
    fprintf(stderr,"Found %d objects, type %d  in %s\n",n_objects, shape_type, ov->map_file_name.c_str());
  }

  for(i=0; i < n_objects; i++ ) {  // Loop through each object

    SO = SHPReadObject(SH,i);    // Load the shape object

    switch(SO->nSHPType) {

      case SHPT_POLYGON:  // Polyline
      case SHPT_ARC:
      case SHPT_ARCM:
      case SHPT_ARCZ:
        index = ov->num_polylines;
        if(index >= ov->num_alloc_polylines) {
          if(ov->num_alloc_polylines == 0) { /* start with space for 2 */
            ov->geo_polyline = (Geo_feat_polyline_t **)
              calloc(2,sizeof(Geo_feat_polyline_t *));
            ov->num_alloc_polylines = 2;
          } else {  /* Double the space */
            ov->num_alloc_polylines *= 2;
            ov->geo_polyline = (Geo_feat_polyline_t **) 
              realloc(ov->geo_polyline, ov->num_alloc_polylines * sizeof(Geo_feat_polyline_t *)); 
          }
        }
        if(ov->geo_polyline == NULL) {
          fprintf(stderr,"Unable to allocate memory for Polyline pointer array!\n");
          exit(-1);
        }

        /* get space for the Polyline definition */
        ov->geo_polyline[index] = (Geo_feat_polyline_t *) calloc(1,sizeof(Geo_feat_polyline_t));
        ZERO_STRUCT(ov->geo_polyline[index]);
        if(ov->geo_polyline[index] == NULL) {
          fprintf(stderr,"Unable to allocate memory for Polyline definition!\n");
          exit(-1);
        }

        STRcopy(ov->geo_polyline[index]->label,"Shape",LABEL_LENGTH);

        /* Get space for points in the polyline */
        ov->geo_polyline[index]->lat = (double *) calloc(1,(SO->nVertices + SO->nParts) * sizeof(double));
        ov->geo_polyline[index]->lon = (double *) calloc(1,(SO->nVertices + SO->nParts) * sizeof(double));
        ov->geo_polyline[index]->local_x = (double *) calloc(1,(SO->nVertices + SO->nParts) * sizeof(double));
        ov->geo_polyline[index]->local_y = (double *) calloc(1,(SO->nVertices + SO->nParts) * sizeof(double));

        if(ov->geo_polyline[index]->lat == NULL || ov->geo_polyline[index]->lon == NULL) {
          fprintf(stderr,"Error!: Unable to allocate space for polyline points in file %s, num points: %d\n",
                  ov->map_file_name.c_str(),SO->nVertices);
          exit(-1);
        }

        /* Read in all of the points */
        part_num = 1;
        for(j=0,point = 0; j < SO->nVertices; j++) {
          ov->geo_polyline[index]->lat[point] = SO->padfY[j];
          ov->geo_polyline[index]->lon[point] = SO->padfX[j];
          if(j+1 == SO->panPartStart[part_num]) {     // Insert a pen up in the data stream.
            point++;
            ov->geo_polyline[index]->lat[point] = -1000.0;
            ov->geo_polyline[index]->lon[point] = -1000.0;
            part_num++;
          }
          point++;
        }
        ov->geo_polyline[index]->num_points = point;
        ov->num_polylines++;

        break;

      case SHPT_POINT :  // Icon Instance
      case SHPT_POINTZ:
      case SHPT_POINTM:
        if(ov->num_icondefs == 0) {  // No Icon definition yet.
          if(ov->num_alloc_icondefs == 0) { /* start with space for 2 */
            ov->geo_icondef = (Geo_feat_icondef_t **) calloc(1,sizeof(Geo_feat_icondef_t *));
            ov->num_icondefs = 1;
            ov->num_alloc_icondefs = 1;
          }
                
          if(ov->geo_icondef == NULL) {
            fprintf(stderr,"Unable to allocate memory for Icon definition pointer array!\n");
            exit(-1);
          }

          /* get space for the icon definition */
          ov->geo_icondef[0] = (Geo_feat_icondef_t *) calloc(1,sizeof(Geo_feat_icondef_t));
          ZERO_STRUCT(ov->geo_icondef[0]);

          if(ov->geo_icondef[0] == NULL) {
            fprintf(stderr,"Unable to allocate memory for Icon definition!\n");
            exit(-1);
          }
          num_points = 6;  // A Predefined Box.

          /* Get space for points in the icon */
          ov->geo_icondef[0]->x = (short *) calloc(1,num_points * sizeof(short));
          ov->geo_icondef[0]->y = (short *) calloc(1,num_points * sizeof(short));

          if(ov->geo_icondef[0]->x == NULL || ov->geo_icondef[0]->y == NULL) {
            fprintf(stderr,"Error!: Unable to allocate space for icon points in file %s, num points: %d\n",
                    ov->map_file_name.c_str(),num_points);
            exit(-1);
          }

          // Set all of the points - Draws a Small Box
          ov->geo_icondef[0]->x[0] = -1;
          ov->geo_icondef[0]->y[0] = -1;

          ov->geo_icondef[0]->x[1] = 1;
          ov->geo_icondef[0]->y[1] = -1;

          ov->geo_icondef[0]->x[2] = 1;
          ov->geo_icondef[0]->y[2] = 1;

          ov->geo_icondef[0]->x[3] = -1;
          ov->geo_icondef[0]->y[3] = 1;

          ov->geo_icondef[0]->x[4] = -1;
          ov->geo_icondef[0]->y[4] = -1;

          ov->geo_icondef[0]->x[5] = 32767;
          ov->geo_icondef[0]->y[5] = 32767;

          ov->geo_icondef[0]->num_points = num_points;
        }

        index = ov->num_icons;
        if(index >= ov->num_alloc_icons) {
          if(ov->num_alloc_icons == 0) { /* start with space for 2 */
            ov->geo_icon = (Geo_feat_icon_t **) calloc(2,sizeof(Geo_feat_icon_t *));
            ov->num_alloc_icons = 2;
          } else {  /* Double the space */
            ov->num_alloc_icons *= 2;
            ov->geo_icon = (Geo_feat_icon_t **) 
              realloc(ov->geo_icon, ov->num_alloc_icons * sizeof(Geo_feat_icon_t *)); 
          }
        }
        if(ov->geo_icon == NULL) {
          fprintf(stderr,"Unable to allocate memory for Icon pointer array!\n");
          exit(-1);
        }

        /* get space for the Icon */
        ov->geo_icon[index] = (Geo_feat_icon_t *) calloc(1,sizeof(Geo_feat_icon_t));
        ZERO_STRUCT(ov->geo_icon[index]);

        if(ov->geo_icon[index] == NULL) {
          fprintf(stderr,"Unable to allocate memory for Icon definition!\n");
          exit(-1);
        }

        // The definition for the Icon is fixed 
        ov->geo_icon[index]->icon = ov->geo_icondef[0];

        /* record its position */
        ov->geo_icon[index]->lat = SO->padfY[0];
        ov->geo_icon[index]->lon = SO->padfX[0];

        ov->num_icons++;
        break;

      default:
      case SHPT_NULL:
      case SHPT_MULTIPOINT:
      case SHPT_POLYGONZ:
      case SHPT_MULTIPOINTZ:
      case SHPT_POLYGONM:
      case SHPT_MULTIPOINTM:
      case SHPT_MULTIPATCH:
        if(gd.debug) {
          fprintf(stderr,"Encountered Unsupported Shape type %d\n",SO->nSHPType);
        }
        break;
    }

    if(SO != NULL) SHPDestroyObject(SO);
  }  // End of each object

  if(is_http) {  // Unlink temporary files
    sprintf(name_buf2,"/tmp/%d_%s.shp",pid,name_base);
    unlink(name_buf2);
    sprintf(name_buf2,"/tmp/%d_%s.shx",pid,name_base);
    unlink(name_buf2);
	
  }

}

/************************************************************************
 * INIT_OVER_DATA_LINKS:  Scan cidd_overlays.info file and setup
 *
 */ 

static void _initMaps()

{
  
  gd.num_map_overlays = _params.maps_n;
  
  for (int ii = 0; ii < _params.maps_n; ii++) {
    
    Params::map_t &omap = _params._maps[ii];
    string mapFileName = omap.map_file_name;
    Overlay_t *over = new Overlay_t;
    gd.over.push_back(over);

    over->mapParams = &omap;
    
    over->map_code = omap.map_code;
    over->control_label = omap.control_label;
    over->map_file_name = mapFileName;
    over->default_on_state = omap.on_at_startup;
    over->line_width = omap.line_width;
    if(over->line_width <=0) {
      over->line_width = 1;
    }
    over->detail_thresh_min = omap.detail_thresh_min;
    over->detail_thresh_max = omap.detail_thresh_max;
    over->active = over->default_on_state;
    
    over->color_name = omap.color;

    if (mapFileName.find(".shp") != string::npos &&
        mapFileName.find(".shx") != string::npos) {
      
      _loadShapeMap(over, _params.maps_url);

    } else {  // Assume RAP Map Format 
      
      _loadRapMap(over, _params.maps_url);

    }
    
    if(gd.debug) {
      printf("Overlay file %s contains %ld polylines, %ld icon_defns, %ld icons, %ld labels\n",
             over->map_file_name.c_str(),
             over->num_polylines,
             over->num_icondefs,
             over->num_icons,
             over->num_labels);
    }
    
  } // ii
  
  calc_local_over_coords();
  
}

/////////////////////////////////////////////////
// zooms

static void _initZooms()

{
  
  if(_params.num_cache_zooms > MAX_CACHE_PIXMAPS) {
    _params.num_cache_zooms = MAX_CACHE_PIXMAPS;
  }
  if(_params.num_cache_zooms < 1) {
    _params.num_cache_zooms = 1 ;
  }
  
  gd.h_win.can_xid = (Drawable *) calloc(sizeof(Drawable *), _params.num_cache_zooms);
  gd.v_win.can_xid = (Drawable *) calloc(sizeof(Drawable *), _params.num_cache_zooms);
  
  gd.h_win.num_zoom_levels = _params.zoom_levels_n;

  if(!_params.html_mode) {
    gd.h_win.zoom_level = _params.start_zoom_level;
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
    
    double delta_x = gd.h_win.zmax_x[izoom] - gd.h_win.zmin_x[izoom];
    double delta_y = gd.h_win.zmax_y[izoom] - gd.h_win.zmin_y[izoom];

    double max_delta_x = gd.h_win.max_x - gd.h_win.min_x;
    double max_delta_y = gd.h_win.max_y - gd.h_win.min_y;
  
    if (delta_x > max_delta_x) {
      delta_x = max_delta_x;
    }
    if (delta_y > max_delta_y) {
      delta_y = max_delta_y;
    }

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

    if(_params.aspect_ratio <= 0.0) {
      _params.aspect_ratio = fabs(delta_x/delta_y);
    }
    
    gd.aspect_correction =
      cos(((gd.h_win.zmax_y[izoom] + gd.h_win.zmin_y[izoom])/2.0) * DEG_TO_RAD);

    /* Make sure domains are consistant with the window aspect ratio */

    if (gd.display_projection == Mdvx::PROJ_LATLON) {
      /* forshorten the Y coords to make things look better */
      delta_y /= gd.aspect_correction;
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

}

/////////////////////////////
// initialize contours

static void _initContours()

{

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

  for(int ii = 0; ii < _params.contour_fields_n; ii++ ) {
    
    Params::contour_field_t &cfield = _params._contour_fields[ii];
    
    strncpy(gd.layers.cont[ii].color_name, cfield.color, NAME_LENGTH);
    
    /* Replace underscores with spaces in contour field names */
    char *contourFieldName = cfield.field_name;
    if(_params.html_mode == 0 && _params.replace_underscores) {
      for(int jj = strlen(contourFieldName) - 1; jj >= 0; jj--) {
        if (contourFieldName[jj] == '_') {
          contourFieldName[jj] = ' ';
        }
      } // jj
    }
    
    for (int jj = 0; jj < gd.num_datafields; jj++) {
      if (strcmp(gd.mrec[jj]->button_name, contourFieldName) == 0) {
        gd.layers.cont[ii].field = jj;
        if(cfield.on_at_startup) {
          gd.layers.cont[ii].active = 1;
        } else {
          gd.layers.cont[ii].active = 0;
        }
        gd.layers.cont[ii].min = gd.mrec[jj]->cont_low;
        gd.layers.cont[ii].max = gd.mrec[jj]->cont_high;
        gd.layers.cont[ii].interval = gd.mrec[jj]->cont_interv;
        strncpy(gd.layers.cont[ii].color_name, cfield.color, NAME_LENGTH);
        break;
      }
      
    } // jj

  } // ii

}

/////////////////////////////
// initialize overlay fields

static void _initOverlayFields()

{

  for (int ii = 0; ii < _params.layer_fields_n; ii++) {

    Params::layer_field_t &lfield = _params._layer_fields[ii];

    char *fieldName = lfield.field_name;
    
    /* Replace Underscores with spaces in field names */
    if(_params.html_mode == 0 && _params.replace_underscores) {
      for(int jj = strlen(fieldName) - 1; jj >= 0; jj--) {
        if(fieldName[jj] == '_') {
          fieldName[jj] = ' ';
        }
      } // jj
    }
    
    for(int jj = 0; jj <  gd.num_datafields; jj++) {
      if(strcmp(gd.mrec[jj]->button_name, fieldName) == 0) {  
        if(lfield.on_at_startup) {
          gd.layers.overlay_field_on[ii] = 1;
        } else {
          gd.layers.overlay_field_on[ii] = 0;
        }
        gd.layers.overlay_field[ii] = jj;
      }
    } // jj
  } // ii

}

///////////////////////////////////////////////////
// get the archive url

void init_globals()
  
{

  gd.hcan_xid = 0;
  gd.vcan_xid = 0;
    
  gd.debug = 0;
  gd.debug1 = 0;
  gd.debug2 = 0;
    
  gd.argc = 0;
  gd.display_projection = 0;
  gd.quiet_mode = 0;   
  gd.report_mode = 0;   
  gd.run_unmapped = 0;   
  gd.use_cosine_correction = -1;
  MEM_zero(gd.product_detail_threshold);
  MEM_zero(gd.product_detail_adjustment);

  gd.mark_latest_client_location = 0; 
  gd.forecast_mode = 0;     
  gd.data_format = 0; 

  gd.num_colors = 0;       
  gd.num_draw_colors = 0;  
  gd.map_overlay_color_index_start = 0;
  gd.last_event_time = 0;  
  gd.epoch_start = 0;      
  gd.epoch_end  = 0;       
  gd.model_run_time = 0;  
  gd.data_request_time = 0; 
  gd.finished_init = 0;    

  gd.num_datafields = 0;   
  gd.num_menu_fields = 0;  
  gd.num_map_overlays = 0; 
  gd.num_render_heights = 0;
  gd.cur_render_height = 0; 
  gd.cur_field_set = 0;     
  gd.save_im_win = 0;       
  gd.image_needs_saved = 0; 
  gd.generate_filename = 0; 

  gd.pan_in_progress = 0;    
  gd.zoom_in_progress = 0;   
  gd.route_in_progress = 0;  
  gd.data_status_changed = 0;
  gd.series_save_active = 0; 

  gd.num_field_labels = 0;  
  MEM_zero(gd.field_index);
  gd.aspect_correction = 0; 
  MEM_zero(gd.height_array);
  MEM_zero(gd.proj_param);

  gd.argv = NULL;             
  gd.orig_wd = NULL;           
  gd.app_name = NULL;          
  gd.app_instance = NULL;      

  MEM_zero(gd.data_info);

  MEM_zero(gd.gen_time_list);

  MEM_zero(gd.h_win);
  MEM_zero(gd.v_win);

  gd.def_gc = 0;
  gd.ol_gc = 0;
  gd.clear_ol_gc = 0;
  gd.dpyName = NULL;
  gd.dpy = NULL;

  MEM_zero(gd.color);
  MEM_zero(gd.null_color);

  MEM_zero(gd.ciddfont);
  MEM_zero(gd.fontst);
    
  MEM_zero(gd.prod);
  // MEM_zero(gd.over);
  MEM_zero(gd.mrec);
  // MEM_zero(gd.layers);
  MEM_zero(gd.legends);
  gd.bookmark = NULL;
  MEM_zero(gd.movie);
  MEM_zero(gd.io_info);
  MEM_zero(gd.status);
  MEM_zero(gd.draw);

  gd.coord_expt = NULL;
  gd.prod_mgr = NULL;
  gd.time_plot = NULL;         
  gd.r_context = NULL;    
  gd.station_loc = NULL;    
  gd.remote_ui = NULL;   

  gd.coord_key = 0;

}

