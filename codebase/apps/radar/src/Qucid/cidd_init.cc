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

#define NUM_PARSE_FIELDS    64
#define PARSE_FIELD_SIZE    1024
#define INPUT_LINE_LEN      512
/*****************************************************************
 * INIT_DATA_SPACE : Init all globals and set up defaults
 */

void init_data_space()
{

  int i,j,pid;
  int num_fields;
  int err_flag;
  int default_setting;
  long param_text_len;
  long param_text_line_no;
  time_t    clock;
  const char *param_text;
  const char *demo_time;
  const char *resource;
  const char *field_str;
  char str_buf[128];   /* Space to build resource strings */
  char *cfield[3];     /* Space to collect sub strings */
  double delta_x,delta_y;
  double lat1 = 0, lat2 = 0;

  UTIMstruct    temp_utime;

  // INSTANCE = xv_unique_key(); /* get keys for retrieving data */
  // MENU_KEY = xv_unique_key();

  // load up the params buffer from file or http

  load_db_data(gd.db_name);  // Retrieve the parameter text file

  // create uparams object, read params into it from buffer

  gd.uparams = new Uparams();
  if (gd.uparams->read(gd.db_data, gd.db_data_len, "cidd")) {
    fprintf(stderr,"init_data_space: could not read params buffer\n");
    exit(-1);
  }
  gd.uparams->setPrintTdrp(true);

  // Load the Main parameters
  param_text_line_no = 0;
  param_text_len = 0;
  param_text = find_tag_text(gd.db_data,"MAIN_PARAMS",
                             &param_text_len, &param_text_line_no);
    
  if(param_text == NULL || param_text_len <=0 ) {
    fprintf(stderr,"Could'nt Find MAIN_PARAMS SECTION\n");
    exit(-1);
  }
    
  // set_X_parameter_database(param_text); /* load the main parameter database*/


  if(!gd.quiet_mode)
    fprintf(stderr,"\n\nCIDD: Version %s\nUcopyright %s\n\n", CIDD_VERSION, CIDD_UCOPYRIGHT);

  gd.html_mode = gd.uparams->getLong( "cidd.html_mode", 0);
  if(gd.html_mode) gd.movie.movie_on = 0;

  gd.run_once_and_exit = gd.uparams->getLong( "cidd.run_once_and_exit",0);
  if(gd.run_once_and_exit) gd.html_mode = 1; 

  gd.transparent_images =
    gd.uparams->getLong( "cidd.transparent_images", 0);

  if(!gd.run_once_and_exit)  PMU_auto_register("Initializing SHMEM");

  init_shared();            /* Initialize Shared memory based communications */
  if(!gd.run_once_and_exit)  PMU_auto_register("Parsing Config Sections");
     
  /* Initialize tha Process Mapper Functions */
  if(!gd.run_once_and_exit)  PMU_auto_init(gd.app_name,gd.app_instance,PROCMAP_REGISTER_INTERVAL);

  if(!gd.run_once_and_exit)  PMU_auto_register("Initializing data");


  gd.debug |= gd.uparams->getLong( "cidd.debug_flag", 0);
  gd.debug1 |= gd.uparams->getLong( "cidd.debug1_flag", 0);
  gd.debug2 |= gd.uparams->getLong( "cidd.debug2_flag", 0);

  // How many idle seconds can elapse before resetting the display
  gd.idle_reset_seconds = gd.uparams->getLong( "cidd.idle_reset_seconds",0);

  gd.model_run_list_hours = gd.uparams->getLong( "cidd.model_run_list_hours",24);

  gd.movie.magnify_factor = gd.uparams->getDouble("cidd.movie_magnify_factor",1.0);

  gd.model_run_list_hours = gd.uparams->getLong( "cidd.model_run_list_hours",24);

  // Establish whether CIDD puts out HTML imagery - Yes if set
  // Look for this for backward compatibility

  resource = gd.uparams->getString( "cidd.html_image_dir", "");
  if(resource != NULL && strlen(resource) > 1) {
    fprintf(stderr,"NOTE: cidd.html_image_dir is a deprecated parameter.\n");
    fprintf(stderr,"     Use cidd.image_dir: instead to avoid this message\n");
    gd.image_dir = resource;
    gd.html_mode = 1;
    gd.h_win.zoom_level = 0;
  } else {
    gd.image_dir = NULL;
  }

  gd.image_horiz_prefix = gd.uparams->getString( "cidd.image_horiz_prefix", "CP");
  gd.image_vert_prefix = gd.uparams->getString( "cidd.image_vert_prefix", "CV");
  gd.image_name_separator = gd.uparams->getString( "cidd.image_name_separator", "_");

  gd.close_popups =   gd.uparams->getLong( "cidd.close_popups", 0);
  gd.disable_pick_mode =   gd.uparams->getLong( "cidd.disable_pick_mode", 1);
  gd.clip_overlay_fields =   gd.uparams->getLong( "cidd.clip_overlay_fields", 0);
  gd.output_geo_xml =   gd.uparams->getLong( "cidd.output_geo_xml", 0);
  gd.use_latlon_in_geo_xml =   gd.uparams->getLong( "cidd.use_latlon_in_geo_xml", 0);
  gd.replace_underscores =   gd.uparams->getLong( "cidd.replace_underscores", 1);
	
  char *image_dir_default;
  if (resource == NULL) {
    image_dir_default = NULL;
  } else {
    image_dir_default = strdup(resource);
  }

  // For backward compatibility

  resource = gd.uparams->getString( "cidd.html_convert_script", "");
  if(strlen(resource) > 1) {
    fprintf(stderr,"NOTE: cidd.html_convert_script is a deprecated parameter.\n");
    fprintf(stderr,"      Use cidd.image_convert_script: instead to avoid this message\n");
    gd.image_convert_script = resource;
  }

  resource = gd.uparams->getString( "cidd.image_convert_script", "");
  gd.image_convert_script = resource;

  resource = gd.uparams->getString( "cidd.image_dir", image_dir_default);
  if(resource != NULL && strlen(resource) > 1) { 
    gd.image_dir = resource;
    STRcopy(gd.h_win.image_dir,resource,MAX_PATH_LEN);
    STRcopy(gd.v_win.image_dir,resource,MAX_PATH_LEN);
  } else {
    gd.image_dir = NULL;
    STRcopy(gd.h_win.image_dir,"/tmp",MAX_PATH_LEN);
    STRcopy(gd.v_win.image_dir,"/tmp",MAX_PATH_LEN);
  }

  // Set up resources for dumping output images
    
  gd.save_images_to_day_subdir = gd.uparams->getLong( "cidd.save_images_to_day_subdir",0);
  STRcopy(gd.h_win.image_dir,
          gd.uparams->getString( "cidd.horiz_image_dir", gd.h_win.image_dir ),
          MAX_PATH_LEN);
  STRcopy(gd.h_win.image_fname,
          gd.uparams->getString( "cidd.horiz_image_fname", "cidd_planview.png"),
          MAX_PATH_LEN);
  STRcopy(gd.h_win.image_command,
          gd.uparams->getString( "cidd.horiz_image_command", ""),
          MAX_PATH_LEN);

  STRcopy(gd.v_win.image_dir,
          gd.uparams->getString( "cidd.vert_image_dir", gd.v_win.image_dir ),
          MAX_PATH_LEN);
  STRcopy(gd.v_win.image_fname,
          gd.uparams->getString( "cidd.vert_image_fname", "cidd_xsect.png"),
          MAX_PATH_LEN);
  STRcopy(gd.v_win.image_command,
          gd.uparams->getString( "cidd.vert_image_command", ""),
          MAX_PATH_LEN);

  // If individual scripts have not been set - Use the General one.

  if(strlen(gd.v_win.image_command) < 3) 
    STRcopy(gd.v_win.image_command,gd.image_convert_script,MAX_PATH_LEN);

  if(strlen(gd.h_win.image_command) < 3) 
    STRcopy(gd.h_win.image_command,gd.image_convert_script,MAX_PATH_LEN);

  // Set up our default image type.

  resource = gd.uparams->getString( "cidd.image_ext", "png");
  if(resource != NULL && strlen(resource) > 1) { 
    gd.image_ext = resource;
  } else {
    gd.image_ext = "png";
  }

#ifndef USE_IMLIB2
  // force to png if not using IMLIB2
  gd.image_ext = "png";
#endif

  if(gd.idle_reset_seconds <= 0 || gd.html_mode == 1) {
    gd.idle_reset_seconds = 259200000; // 3000 days.
  }

  gd.complex_command_timeout_secs =  gd.uparams->getLong( "cidd.complex_command_timeout_secs",180);
  resource = gd.uparams->getString( "cidd.print_script", "");
  if(strlen(resource) > 1) {
    gd.print_script = resource;
  }

  resource = gd.uparams->getString( "cidd.series_convert_script", "");
  if(strlen(resource) > 1) {
    gd.series_convert_script = resource;
  }

  gd.simple_command_timeout_secs =  gd.uparams->getLong( "cidd.simple_command_timeout_secs",30);

  gd.label_time_format = gd.uparams->getString(
	  "cidd.label_time_format", "%m/%d/%y %H:%M:%S");

  gd.moviestart_time_format = gd.uparams->getString(
	  "cidd.moviestart_time_format", "%H:%M %m/%d/%Y");

  gd.frame_range_time_format = gd.uparams->getString(
	  "cidd.frame_range_time_format", "%H:%M");

  gd.movieframe_time_format = gd.uparams->getString(
	  "cidd.movieframe_time_format", "%H%M");

  gd.movieframe_time_mode = gd.uparams->getLong(
          "cidd.movieframe_time_mode", 0);

  // Get the on/off state of the extra legend plotting - Force to either 0 or 1
  gd.layers.layer_legends_on = (gd.uparams->getLong( "cidd.layer_legends_on", 1) & 1);
  gd.layers.cont_legends_on = (gd.uparams->getLong( "cidd.cont_legends_on", 1) & 1);
  gd.layers.wind_legends_on = (gd.uparams->getLong( "cidd.wind_legends_on", 1) & 1);

  gd.layers.contour_line_width = gd.uparams->getLong( "cidd.contour_line_width", 1);
  gd.layers.smooth_contours = gd.uparams->getLong( "cidd.smooth_contours", 0);
  gd.layers.use_alt_contours = gd.uparams->getLong( "cidd.use_alt_contours", 0);
  gd.layers.add_noise = gd.uparams->getLong( "cidd.add_noise", 0);
  gd.layers.special_contour_value = gd.uparams->getDouble( "cidd.special_contour_value", 0.0);

  gd.layers.map_bad_to_min_value =  gd.uparams->getLong( "cidd.map_bad_to_min_value", 0);
  gd.layers.map_missing_to_min_value =  gd.uparams->getLong( "cidd.map_missing_to_min_value", 0);

  gd.draw_main_on_top = (gd.uparams->getLong( "cidd.draw_main_on_top", 0) & 1);
  gd.mark_latest_click_location =
    gd.uparams->getLong( "cidd.mark_latest_click_location", 0);
  gd.mark_latest_client_location =
    gd.uparams->getLong( "cidd.mark_latest_client_location", 0);

  gd.drawing_mode = 0;

  gd.prod.products_on = 1;
  gd.prod.prod_line_width = gd.uparams->getLong( "cidd.product_line_width", 1);

  gd.prod.prod_font_num = gd.uparams->getLong( "cidd.product_font_size", 1);;

  for(i=0; i < NUM_PRODUCT_DETAIL_THRESHOLDS; i++) {
    sprintf(str_buf,"cidd.product_detail_threshold%d",i+1);
    gd.prod.detail[i].threshold = gd.uparams->getDouble(str_buf,0.0);

    sprintf(str_buf,"cidd.product_detail_adjustment%d",i+1);
    gd.prod.detail[i].adjustment = gd.uparams->getLong(str_buf,0);
  }
	

  gd.always_get_full_domain = gd.uparams->getLong( "cidd.always_get_full_domain", 0);
     
  gd.do_not_clip_on_mdv_request = gd.uparams->getLong( "cidd.do_not_clip_on_mdv_request", 0);
     
  gd.do_not_decimate_on_mdv_request = gd.uparams->getLong( "cidd.do_not_decimate_on_mdv_request", 0);
     
  // Toggle for displaying range rings at the data's origin - Useful for mobile units.
  gd.range_ring_follows_data = gd.uparams->getLong("cidd.range_ring_follows_data", 0);
  gd.range_ring_for_radar_only = gd.uparams->getLong("cidd.range_ring_for_radar_only", 0);

  // Toggle for shifting the display origin - Useful for mobile units.
  gd.domain_follows_data = gd.uparams->getLong("cidd.domain_follows_data", 0);

  // if domain follows data, do not clip or decimate

  if (gd.domain_follows_data) {
    gd.always_get_full_domain = 1;
    gd.do_not_clip_on_mdv_request = 1;
    gd.do_not_decimate_on_mdv_request = 1;
  }

  gd.help_command = gd.uparams->getString( "cidd.help_command", "");

  gd.http_tunnel_url = gd.uparams->getString( "cidd.http_tunnel_url", "");

  gd.datamap_host = gd.uparams->getString( "cidd.datamap_host", "");

  gd.http_proxy_url = gd.uparams->getString( "cidd.http_proxy_url", "");

  // Bookmarks for a menu of URLS - Index starts at 1
  gd.bookmark_command = gd.uparams->getString( "cidd.bookmark_command", "");
  gd.num_bookmarks = gd.uparams->getLong( "cidd.num_bookmarks", 0);

  if(gd.num_bookmarks > 0) {
    gd.bookmark = (bookmark_t *)  calloc(sizeof(bookmark_t),gd.num_bookmarks);
  }

  err_flag = 0;
  for(i=0; i < gd.num_bookmarks; i++) {
    sprintf(str_buf,"cidd.bookmark%d",i+1);
    gd.bookmark[i].url = gd.uparams->getString(str_buf,"");

    if(strlen(gd.bookmark[i].url) < 4) {
      fprintf(stderr,"Error: Parameter %s undefined\n",str_buf);
      err_flag++;
    }

    sprintf(str_buf,"cidd.bookmark_label%d",i+1);
    gd.bookmark[i].label = gd.uparams->getString(str_buf,"");

    if(strlen(gd.bookmark[i].label) < 1) {
      fprintf(stderr,"Error: Parameter %s undefined\n",str_buf);
      err_flag++;
    }

  }
  if(err_flag) {
    fprintf(stderr,"Correct the cidd.bookmark section of the parameter file\n");
    exit(-1);
  }

  gd.h_win.origin_lat = gd.uparams->getDouble( "cidd.origin_latitude", 39.8783);
  gd.h_win.origin_lon = gd.uparams->getDouble( "cidd.origin_longitude", -104.7568);

  double north_angle = gd.uparams->getDouble( "cidd.north_angle",0.0);
  double lambert_lat1 = gd.uparams->getDouble( "cidd.lambert_lat1",20.0);
  double lambert_lat2 = gd.uparams->getDouble( "cidd.lambert_lat2",60.0);
  double tangent_lat = gd.uparams->getDouble( "cidd.tangent_lat",90.0);
  double tangent_lon = gd.uparams->getDouble( "cidd.tangent_lon",0.0);
  double central_scale = gd.uparams->getDouble( "cidd.central_scale",1.0);

  gd.proj_param[0] = north_angle; // flat projection is default

  gd.aspect_ratio = gd.uparams->getDouble( "cidd.aspect_ratio",1.0);
  if (gd.aspect_ratio <= 0.0 && gd.debug) {
    cerr << "WARNING - Using First domain to set aspect ratio: " << endl;
  }

  gd.scale_units_per_km = gd.uparams->getDouble( "cidd.units_per_km",1.0);
  gd.scale_units_label = gd.uparams->getString( "cidd.scale_units_label", "km");

  gd.h_win.reset_click_lon = gd.uparams->getDouble( "cidd.reset_click_longitude",
                                                    gd.h_win.origin_lon);
  gd.h_win.reset_click_lat = gd.uparams->getDouble( "cidd.reset_click_latitude", 
                                                    gd.h_win.origin_lat);


  /* Establish the native projection type */

  resource = gd.uparams->getString( "cidd.projection_type", "CARTESIAN");

  if (strncasecmp(resource,"CARTESIAN",9) == 0) {

    gd.display_projection = Mdvx::PROJ_FLAT;
    if(gd.debug) {
      printf("Cartesian Projection - Origin at: %g, %g\n", 
             gd.h_win.origin_lat,gd.h_win.origin_lon);
    }
    gd.proj.initFlat(gd.h_win.origin_lat,gd.h_win.origin_lon,north_angle);

  } else if (strncasecmp(resource,"LAT_LON",7) == 0) {

    gd.display_projection = Mdvx::PROJ_LATLON;
    if(gd.debug) {
      printf("LATLON/ Cylindrical Projection - Origin at: %g, %g\n",
             gd.h_win.origin_lat,gd.h_win.origin_lon);
    }

  } else if (strncasecmp(resource,"LAMBERT",7) == 0) {

    gd.display_projection = Mdvx::PROJ_LAMBERT_CONF;
    gd.proj_param[0] = lambert_lat1;
    gd.proj_param[1] = lambert_lat2;
    if(lat1 == -90.0 || lat2 == -90.0) {
      fprintf(stderr,
              "Must set cidd.lambert_lat1 and cidd.lambert_lat2 parameters for LAMBERT projections\n");
      exit(-1);
    }
    if(gd.debug) {
      printf("LAMBERT Projection - Origin at: %g, %g Parallels at: %g, %g\n",
             gd.h_win.origin_lat,gd.h_win.origin_lon,
             lambert_lat1, lambert_lat2);
      gd.proj.initLambertConf(gd.h_win.origin_lat,gd.h_win.origin_lon,
                              lambert_lat1, lambert_lat2);
    }

  } else if (strncasecmp(resource,"STEREOGRAPHIC",13) == 0) {

    gd.display_projection = Mdvx::PROJ_OBLIQUE_STEREO;
    gd.proj_param[0] = tangent_lat;
    gd.proj_param[1] = tangent_lon;
    gd.proj_param[2] = central_scale;
    if(gd.debug) {
      printf("Oblique Stereographic Projection - Origin at: %g, %g Tangent at: %g, %g\n",
             gd.h_win.origin_lat,gd.h_win.origin_lon,
             tangent_lat,tangent_lon);
    }
    gd.proj.initStereographic(tangent_lat, tangent_lon, central_scale);
    gd.proj.setOffsetOrigin(gd.h_win.origin_lat,gd.h_win.origin_lon);

  } else if (strncasecmp(resource,"POLAR_STEREO",12) == 0) {

    gd.display_projection = Mdvx::PROJ_POLAR_STEREO;
    gd.proj_param[0] = tangent_lat;
    gd.proj_param[1] = tangent_lon;
    gd.proj_param[2] = central_scale;
    if(gd.debug) {
      printf("Polar Stereographic Projection - Origin at: %g, %g Tangent at: %g, %g\n",
             gd.h_win.origin_lat,gd.h_win.origin_lon,
             tangent_lat,tangent_lon);
    }
    gd.proj.initPolarStereo
      (tangent_lon,
       (Mdvx::pole_type_t) (tangent_lat < 0.0 ? Mdvx::POLE_SOUTH : Mdvx::POLE_NORTH),
       central_scale);
    gd.proj.setOffsetOrigin(gd.h_win.origin_lat,gd.h_win.origin_lon);

  } else if (strncasecmp(resource,"MERCATOR",8) == 0) {

    gd.display_projection = Mdvx::PROJ_MERCATOR;
    if(gd.debug) {
      printf("MERCATOR Projection - Origin at: %g, %g\n",
             gd.h_win.origin_lat,gd.h_win.origin_lon);
    }
    gd.proj.initMercator(gd.h_win.origin_lat,gd.h_win.origin_lon);

  } else {

    fprintf(stderr,"Unknown projection type for resource: cidd.projection_type! \n");
    fprintf(stderr," Current valid types are: CARTESIAN, LAT_LON, LAMBERT, STEREOGRAPHIC, MERCATOR\n");
    exit(-1);

  }

  gd.h_win.last_page = -1;
  gd.v_win.last_page = -1;

  /* fill in movie frame names & other info */
  gd.movie_frame_dir = gd.uparams->getString( "cidd.movie_frame_dir", "/tmp");

  if(strncmp(gd.movie_frame_dir,"/tmp",4) != 0) {
    fprintf(stderr,"NOTE: cidd.movie_frame_dir is a deprecated parameter.\n");
    fprintf(stderr,"      Set cidd.image_dir instead.\n"); 
  }

  pid = getpid();
  for(i=0; i < MAX_FRAMES; i++) {
    sprintf(gd.movie.frame[i].fname,
            "%s/cidd_im%d_%d.",
            gd.movie_frame_dir, pid, i);
    gd.movie.frame[i].h_xid = 0;
    gd.movie.frame[i].v_xid = 0;
    gd.movie.frame[i].redraw_horiz = 1;
    gd.movie.frame[i].redraw_vert = 1;
  }

  gd.movie.time_interval = gd.uparams->getDouble( "cidd.time_interval",10.0);
  gd.movie.frame_span = gd.uparams->getDouble( "cidd.frame_span",gd.movie.time_interval);

  gd.movie.forecast_interval = gd.uparams->getDouble( "cidd.forecast_interval", 0.0);
  gd.movie.past_interval = gd.uparams->getDouble( "cidd.past_interval", 0.0);

  gd.movie.reset_frames = gd.uparams->getLong( "cidd.reset_frames", 0);
  gd.movie.mr_stretch_factor = gd.uparams->getDouble( "cidd.stretch_factor", 1.5);

  // Handle legacy parameter for starting number of movie frames
  gd.movie.num_frames = gd.uparams->getLong( "cidd.starting_movie_frames",
                                             gd.uparams->getLong( "cidd.num_pixmaps", 5));

  gd.movie.delay = gd.uparams->getLong( "cidd.movie_delay",3000);

  gd.movie.start_frame = 0;
  gd.movie.sweep_on = 0;
  gd.movie.sweep_dir = 1;
  gd.movie.end_frame = gd.movie.num_frames -1 ;
  gd.movie.cur_frame = gd.movie.num_frames -1;
  gd.movie.last_frame = gd.movie.cur_frame;

  gd.movie.climo_mode = REGULAR_INTERVAL;
  resource = gd.uparams->getString( "cidd.climo_mode", "regular");
  if(strncmp(resource,"daily",5) == 0 ) gd.movie.climo_mode = DAILY_INTERVAL;
  if(strncmp(resource,"yearly",6) == 0 ) gd.movie.climo_mode = YEARLY_INTERVAL;
  gd.check_clipping = gd.uparams->getLong( "cidd.check_clipping", 0);

  /* Toggle for displaying the analog clock */
  gd.max_time_list_span = gd.uparams->getLong("cidd.max_time_list_span", 365);

  /* Toggle for displaying the analog clock */
  gd.show_clock = gd.uparams->getLong("cidd.show_clock", 0);

  /* Toggle for displaying data access and rendering messages */
  gd.show_data_messages = gd.uparams->getLong("cidd.show_data_messages", 1);

  /* Toggle for displaying data labels */
  gd.display_labels = gd.uparams->getLong("cidd.display_labels", 1);

  /* Toggle for displaying the analog clock */
  gd.display_ref_lines = gd.uparams->getLong("cidd.display_ref_lines", 1);

  /* Toggle for enabling a status report window */
  gd.enable_status_window = gd.uparams->getLong("cidd.enable_status_window", 0);

  /* Toggle for enabling a Save Image Panel */
  // WARNING - ALLOWS USERS SHELL ACCESS
  gd.enable_save_image_panel = gd.uparams->getLong("cidd.enable_save_image_panel", 0);

  /* Set the time to display on the analog clock */
  gd.draw_clock_local = gd.uparams->getLong("cidd.draw_clock_local", 0);

  /* Use local times for Product timestamps and user input widgets. */
  gd.use_local_timestamps = gd.uparams->getLong("cidd.use_local_timestamps", 0);

  /* Toggle for displaying the height Selector in Right Margin */
  gd.show_height_sel = gd.uparams->getLong("cidd.show_height_sel", 1);

  /* Use cosine correction for computing range in polar data */
  if (gd.use_cosine_correction < 0) {
    // not set on command line
    gd.use_cosine_correction = gd.uparams->getLong("cidd.use_cosine", 1);
  }

  clock = time(0);    /* get current time */
  gd.movie.round_to_seconds = gd.uparams->getLong("cidd.temporal_rounding", 300);

  /* IF this string is present in the params - Set into Archive Mode at the indicated time. */
  demo_time = gd.uparams->getString("cidd.demo_time","");

  gd.gather_data_mode = gd.uparams->getLong( "cidd.gather_data_mode",CLOSEST_TO_FRAME_CENTER);
	 
  /* If demo time param is not set and command line option hasn't set archive mode */
  if((int)strlen(demo_time) < 8 && (gd.movie.mode != ARCHIVE_MODE) ) { /* LIVE MODE */
    gd.movie.mode = REALTIME_MODE;
    gd.coord_expt->runtime_mode = RUNMODE_REALTIME;
    gd.coord_expt->time_seq_num++;
    gd.forecast_mode = 0;

    /* set the first index's time based on current time  */
    gd.movie.start_time = (time_t) (clock - ((gd.movie.num_frames -1) * gd.movie.time_interval * 60.0));
    gd.movie.start_time -= (gd.movie.start_time % gd.movie.round_to_seconds);
    gd.movie.demo_time = 0; // Indicated REAL-TIME is Native

    UTIMunix_to_date(gd.movie.start_time,&temp_utime);
    gd.movie.demo_mode = 0;

  } else {   /* DEMO MODE */

    if(gd.movie.mode != ARCHIVE_MODE) { /* if not set by command line args */
      gd.movie.mode = ARCHIVE_MODE;     /* time_series */
      gd.coord_expt->runtime_mode = RUNMODE_ARCHIVE;
      gd.coord_expt->time_seq_num++;

      parse_string_into_time(demo_time,&temp_utime);
      UTIMdate_to_unix(&temp_utime);
      /* set the first index's time  based on indicated time */
      gd.movie.start_time = temp_utime.unix_time;
    }
    gd.movie.demo_mode = 1;
	 
    /* Adjust the start time downward to the nearest round interval seconds */
    gd.movie.start_time -= (gd.movie.start_time % gd.movie.round_to_seconds);


    if(gd.gather_data_mode == CLOSEST_TO_FRAME_CENTER) {
      // Offset movie frame by 1/2 frame interval so that interest time
      // lies on frame mid point
      gd.movie.start_time -=  (time_t) (gd.movie.time_interval * 30.0);
    }

    gd.movie.demo_time = gd.movie.start_time;

    UTIMunix_to_date(gd.movie.start_time,&temp_utime);

    gd.h_win.movie_page = gd.h_win.page;
    gd.v_win.movie_page = gd.v_win.page;
    gd.movie.cur_frame = 0;
  }

  gd.movie.movie_on = gd.uparams->getLong("cidd.movie_on",0);

  gd.movie.display_time_msec = gd.uparams->getLong( "cidd.movie_speed_msec", 75);

  reset_time_points();

  gd.image_fill_threshold = gd.uparams->getLong( "cidd.image_fill_threshold",    120000);

  gd.dynamic_contour_threshold = gd.uparams->getLong( "cidd.dynamic_contour_threshold",    160000);

  gd.image_inten = gd.uparams->getDouble( "cidd.image_inten", 0.8);
  gd.inten_levels = gd.uparams->getLong( "cidd.inten_levels", 32);
  gd.data_inten = gd.uparams->getDouble( "cidd.data_inten", 1.0);

  gd.data_timeout_secs = gd.uparams->getLong( "cidd.data_timeout_secs", 10);

  gd.latlon_mode = gd.uparams->getLong( "cidd.latlon_mode",0);
     
  gd.request_compressed_data = gd.uparams->getLong( "cidd.request_compressed_data",0);
  gd.request_gzip_vol_compression = gd.uparams->getLong( "cidd.request_gzip_vol_compression",0);
     
  gd.add_frame_num_to_filename = gd.uparams->getLong( "cidd.add_frame_num_to_filename",1);
  gd.add_button_name_to_filename = gd.uparams->getLong( "cidd.add_button_name_to_filename",0);
     
  gd.add_height_to_filename = gd.uparams->getLong( "cidd.add_height_to_filename",0);
     
  gd.add_frame_time_to_filename = gd.uparams->getLong( "cidd.add_frame_time_to_filename",1);
     
  gd.add_gen_time_to_filename = gd.uparams->getLong( "cidd.add_gen_time_to_filename",0);
     
  gd.add_valid_time_to_filename = gd.uparams->getLong( "cidd.add_valid_time_to_filename",0);
     
  gd.font_display_mode = gd.uparams->getLong( "cidd.font_display_mode",1);

  gd.label_contours = gd.uparams->getLong( "cidd.label_contours",1);

  default_setting =  1;
  gd.top_margin_render_style = gd.uparams->getLong( "cidd.top_margin_render_style",default_setting);

  default_setting =  2;
  if(gd.html_mode || gd.movie.num_frames < 3 ) default_setting =  1;
  gd.bot_margin_render_style = gd.uparams->getLong( "cidd.bot_margin_render_style",default_setting);

  gd.num_field_menu_cols = gd.uparams->getLong( "cidd.num_field_menu_cols",0);

  gd.num_cache_zooms = gd.uparams->getLong( "cidd.num_cache_zooms",1);
  if(gd.num_cache_zooms > MAX_CACHE_PIXMAPS)
    gd.num_cache_zooms = MAX_CACHE_PIXMAPS ;
  if(gd.num_cache_zooms < 1) gd.num_cache_zooms = 1 ;
  gd.h_win.can_xid = (Drawable *) calloc(sizeof(Drawable *),gd.num_cache_zooms);
  gd.v_win.can_xid = (Drawable *) calloc(sizeof(Drawable *),gd.num_cache_zooms);

  gd.h_win.num_zoom_levels =  gd.uparams->getLong( "cidd.num_zoom_levels",1);
  if(gd.html_mode ==0) {
    gd.h_win.zoom_level =  gd.uparams->getLong( "cidd.start_zoom_level",1) -1;

    if(gd.h_win.zoom_level < 0) gd.h_win.zoom_level = 0;
    if(gd.h_win.zoom_level > gd.h_win.num_zoom_levels) gd.h_win.zoom_level = gd.h_win.num_zoom_levels -1;
    gd.h_win.start_zoom_level = gd.h_win.zoom_level;
  }

  gd.h_win.zmin_x = (double *)  calloc(sizeof(double),gd.h_win.num_zoom_levels+NUM_CUSTOM_ZOOMS + 1);
  gd.h_win.zmax_x = (double *)  calloc(sizeof(double),gd.h_win.num_zoom_levels+NUM_CUSTOM_ZOOMS + 1);
  gd.h_win.zmin_y = (double *)  calloc(sizeof(double),gd.h_win.num_zoom_levels+NUM_CUSTOM_ZOOMS + 1);
  gd.h_win.zmax_y = (double *)  calloc(sizeof(double),gd.h_win.num_zoom_levels+NUM_CUSTOM_ZOOMS + 1);

  gd.zoom_limits_in_latlon =  gd.uparams->getLong( "cidd.zoom_limits_in_latlon",0);

  switch(gd.display_projection) {
    case Mdvx::PROJ_FLAT:
      gd.h_win.min_x = gd.uparams->getDouble( "cidd.domain_limit_min_x",-1000);
      gd.h_win.max_x = gd.uparams->getDouble( "cidd.domain_limit_max_x",1000);
      gd.h_win.min_y = gd.uparams->getDouble( "cidd.domain_limit_min_y",-1000);
      gd.h_win.max_y = gd.uparams->getDouble( "cidd.domain_limit_max_y",1000);
      break;

    case Mdvx::PROJ_LATLON:
      gd.h_win.min_x = gd.uparams->getDouble( "cidd.domain_limit_min_x",-360);
      gd.h_win.max_x = gd.uparams->getDouble( "cidd.domain_limit_max_x",360);
      gd.h_win.min_y = gd.uparams->getDouble( "cidd.domain_limit_min_y",-90);
      gd.h_win.max_y = gd.uparams->getDouble( "cidd.domain_limit_max_y",90);
      break;

    case Mdvx::PROJ_LAMBERT_CONF:
    default:
      gd.h_win.min_x = gd.uparams->getDouble( "cidd.domain_limit_min_x",-10000);
      gd.h_win.max_x = gd.uparams->getDouble( "cidd.domain_limit_max_x",10000);
      gd.h_win.min_y = gd.uparams->getDouble( "cidd.domain_limit_min_y",-10000);
      gd.h_win.max_y = gd.uparams->getDouble( "cidd.domain_limit_max_y",10000);

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
      
    if( gd.h_win.min_x == gd.h_win.max_x) {
      gd.h_win.min_x = gd.h_win.min_x - 180.0;
      gd.h_win.max_x = gd.h_win.max_x + 180.0;
    }
      
    if( gd.h_win.min_x < -360.0) gd.h_win.min_x = -360.0;
    if( gd.h_win.max_x > 360.0) gd.h_win.max_x = 360.0;
    if( gd.h_win.min_y < -180.0) gd.h_win.min_y = -180.0;
    if( gd.h_win.max_y > 180.0) gd.h_win.max_y = 180.0;
      
    if((gd.h_win.max_x - gd.h_win.min_x) > 360.0)  { 
      gd.h_win.min_x = ((gd.h_win.max_x + gd.h_win.min_x) / 2.0) - 180.0;
      gd.h_win.max_x = gd.h_win.min_x + 360;
    }
    double originLon = (gd.h_win.min_x + gd.h_win.max_x) / 2.0;
    gd.proj.initLatlon(originLon);
  }
    
  if(gd.debug) {
    fprintf(stderr,
            " LIMITS:  X: %g,%g   Y: %g,%g \n",
            gd.h_win.min_x,gd.h_win.max_x,
            gd.h_win.min_y,gd.h_win.max_y );
  }
    
  double max_delta_x = gd.h_win.max_x - gd.h_win.min_x;
  double max_delta_y = gd.h_win.max_y - gd.h_win.min_y;

  for(i=0; i < gd.h_win.num_zoom_levels; i++) {

    sprintf(str_buf, "cidd.level%d_min_xkm", i+1);
    double minx = gd.uparams->getDouble(str_buf,-200.0/(i+1));

    sprintf(str_buf, "cidd.level%d_min_ykm", i+1);
    double miny = gd.uparams->getDouble(str_buf,-200.0/(i+1));

    sprintf(str_buf, "cidd.level%d_max_xkm", i+1);
    double maxx = gd.uparams->getDouble(str_buf,200.0/(i+1));

    sprintf(str_buf, "cidd.level%d_max_ykm", i+1);
    double maxy = gd.uparams->getDouble(str_buf,200.0/(i+1));

    // convert from latlon if needed

    if (gd.zoom_limits_in_latlon) {

      double minLon = minx;
      double maxLon = maxx;
      double minLat = miny;
      double maxLat = maxy;

      gd.proj.latlon2xy(minLat, minLon, minx, miny);
      gd.proj.latlon2xy(maxLat, maxLon, maxx, maxy);
      
      if(gd.debug) {
        cerr << "Zoom number: " << (i + 1) << endl;
        cerr << "  converting lat/lon to km" << endl;
        cerr << "  minLon, minLat: " << minLon << ", " << minLat << endl;
        cerr << "  maxLon, maxLat: " << maxLon << ", " << maxLat << endl;
        cerr << "  minXkm, minYkm: " << minx << ", " << miny << endl;
        cerr << "  maxXkm, maxYkm: " << maxx << ", " << maxy << endl;
      }

    }
    
    gd.h_win.zmin_x[i] = minx;
    gd.h_win.zmin_y[i] = miny;
    gd.h_win.zmax_x[i] = maxx;
    gd.h_win.zmax_y[i] = maxy;

    delta_x = gd.h_win.zmax_x[i] - gd.h_win.zmin_x[i];
    delta_y = gd.h_win.zmax_y[i] - gd.h_win.zmin_y[i];

    if(delta_x > max_delta_x) delta_x = max_delta_x;
    if(delta_y > max_delta_y) delta_y = max_delta_y;

    // trap bogus values
    if(gd.h_win.zmin_x[i] < gd.h_win.min_x) {
      gd.h_win.zmin_x[i] = gd.h_win.min_x;
      gd.h_win.zmax_x[i] =  gd.h_win.min_x + delta_x;
    }

    if(gd.h_win.zmin_y[i] < gd.h_win.min_y) {
      gd.h_win.zmin_y[i] = gd.h_win.min_y;
      gd.h_win.zmax_y[i] =  gd.h_win.min_y + delta_y;
    }

    if(gd.h_win.zmax_x[i] > gd.h_win.max_x) {
      gd.h_win.zmax_x[i] = gd.h_win.max_x;
      gd.h_win.zmin_x[i] =  gd.h_win.max_x - delta_x;
    }

    if(gd.h_win.zmax_y[i] > gd.h_win.max_y) {
      gd.h_win.zmax_y[i] = gd.h_win.max_y;
      gd.h_win.zmin_y[i] =  gd.h_win.max_y - delta_y;
    }


    if(gd.aspect_ratio <= 0.0) gd.aspect_ratio = fabs(delta_x/delta_y);

    gd.aspect_correction =
      cos(((gd.h_win.zmax_y[i] + gd.h_win.zmin_y[i])/2.0) * DEG_TO_RAD);

    /* Make sure domains are consistant with the window aspect ratio */
    switch(gd.display_projection) {
      case Mdvx::PROJ_LATLON:
        /* forshorten the Y coords to make things look better */
        delta_y /= gd.aspect_correction;
        break;
    }

    delta_x /= gd.aspect_ratio;

    if(delta_x > delta_y) {
      gd.h_win.zmax_y[i] += ((delta_x - delta_y) /2.0) ;
      gd.h_win.zmin_y[i] -= ((delta_x - delta_y) /2.0) ;
    } else {
      gd.h_win.zmax_x[i] += ((delta_y - delta_x) /2.0) ;
      gd.h_win.zmin_x[i] -= ((delta_y - delta_x) /2.0) ;
    }

    if(gd.debug) {
      printf(" ZOOM: %d --  X: %g,%g   Y: %g,%g,  Delta: %g,%g\n",i,
             gd.h_win.zmin_x[i],gd.h_win.zmax_x[i],
             gd.h_win.zmin_y[i],gd.h_win.zmax_y[i],
             delta_x,delta_y);
    }

  } // i

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

  gd.h_win.route.y_world[0] =  gd.h_win.cmin_y + ((gd.h_win.cmax_y - gd.h_win.cmin_y) / 4);
  gd.h_win.route.y_world[1] =  gd.h_win.cmax_y - ((gd.h_win.cmax_y - gd.h_win.cmin_y) / 4);

  gd.h_win.route.seg_length[0] = disp_proj_dist(gd.h_win.route.x_world[0],gd.h_win.route.y_world[0],
                                                gd.h_win.route.x_world[1],gd.h_win.route.y_world[1]);
  gd.h_win.route.total_length = gd.h_win.route.seg_length[0];

  /* Automatically define the Custom Zoom levels */
  for(i=0; i <= NUM_CUSTOM_ZOOMS; i++) {
    gd.h_win.zmin_x[gd.h_win.num_zoom_levels] = gd.h_win.zmin_x[0] + 
      ((gd.h_win.zmax_x[0] -gd.h_win.zmin_x[0]) / ( NUM_CUSTOM_ZOOMS - i  + 2.0));

    gd.h_win.zmax_x[gd.h_win.num_zoom_levels] = gd.h_win.zmax_x[0] - 
      ((gd.h_win.zmax_x[0] -gd.h_win.zmin_x[0]) / ( NUM_CUSTOM_ZOOMS - i  + 2.0));

    gd.h_win.zmin_y[gd.h_win.num_zoom_levels] = gd.h_win.zmin_y[0] + 
      ((gd.h_win.zmax_y[0] -gd.h_win.zmin_y[0]) / ( NUM_CUSTOM_ZOOMS - i  + 2.0));

    gd.h_win.zmax_y[gd.h_win.num_zoom_levels] = gd.h_win.zmax_y[0] - 
      ((gd.h_win.zmax_y[0] -gd.h_win.zmin_y[0]) / ( NUM_CUSTOM_ZOOMS - i  + 2.0));

    gd.h_win.num_zoom_levels++;
  }

  ZERO_STRUCT(&gd.menu_bar);

  // Establish what each Menu Bar Cell Does.
  gd.menu_bar.num_menu_bar_cells = gd.uparams->getLong( "cidd.num_menu_bar_cells",0);
  if(gd.menu_bar.num_menu_bar_cells > 0) {
    for(i=1; i <= gd.menu_bar.num_menu_bar_cells; i++) {
      sprintf(str_buf,"cidd.menu_bar_funct%d",i);
      resource = gd.uparams->getString(str_buf,"Not Defined");
      if(strcmp("LOOP_ONOFF",resource) == 0) {
        gd.menu_bar.loop_onoff_bit = 1 << (i-1) ;
      } else if( strcmp("WINDS_ONOFF",resource) == 0) {
        gd.menu_bar.winds_onoff_bit = 1 << (i-1) ;
      } else if( strcmp("SHOW_FORECAST_MENU",resource) == 0) {
        gd.menu_bar.show_forecast_menu_bit = 1 << (i-1) ;
      } else if( strcmp("SHOW_PAST_MENU",resource) == 0) {
        gd.menu_bar.show_past_menu_bit = 1 << (i-1) ;
      } else if( strcmp("SHOW_GENTIME_MENU",resource) == 0) {
        gd.menu_bar.show_gen_time_win_bit = 1 << (i-1) ;
      } else if( strcmp("SYMPRODS_ONOFF",resource) == 0) {
        gd.menu_bar.symprods_onoff_bit = 1 << (i-1) ;
      } else if( strcmp("PRINT_BUTTON",resource) == 0) {
        gd.menu_bar.print_button_bit = 1 << (i-1) ;
      } else if( strcmp("HELP_BUTTON",resource) == 0) {
        gd.menu_bar.help_button_bit = 1 << (i-1) ;
      } else if( strcmp("CLONE_CIDD",resource) == 0) {
        gd.menu_bar.clone_button_bit = 1 << (i-1) ;
      } else if( strcmp("EXIT_BUTTON",resource) == 0) {
        gd.menu_bar.exit_button_bit = 1 << (i-1) ;
      } else if( strcmp("SHOW_VIEW_MENU",resource) == 0) {
        gd.menu_bar.show_view_menu_bit = 1 << (i-1) ;
      } else if( strcmp("SHOW_PRODUCT_MENU",resource) == 0) {
        gd.menu_bar.show_prod_menu_bit = 1 << (i-1) ;
      } else if( strcmp("SHOW_MAP_MENU",resource) == 0) {
        gd.menu_bar.show_map_menu_bit = 1 << (i-1) ;
      } else if( strcmp("SHOW_BOOKMARK_MENU",resource) == 0) {
        gd.menu_bar.show_bookmark_menu_bit = 1 << (i-1) ;
      } else if( strcmp("SHOW_STATUS_PANEL",resource) == 0) {
        gd.menu_bar.show_status_win_bit = 1 << (i-1) ;
      } else if( strcmp("SHOW_TIME_PANEL",resource) == 0) {
        gd.menu_bar.show_time_panel_bit = 1 << (i-1) ;
      } else if( strcmp("SHOW_DPD_MENU",resource) == 0) {
        gd.menu_bar.show_dpd_menu_bit = 1 << (i-1) ;
      } else if( strcmp("SHOW_DPD_PANEL",resource) == 0) {
        gd.menu_bar.show_dpd_panel_bit = 1 << (i-1) ;
      } else if( strcmp("SET_DRAW_MODE",resource) == 0) {
        gd.menu_bar.set_draw_mode_bit = 1 << (i-1) ;
      } else if( strcmp("SET_PICK_MODE",resource) == 0) {
        gd.menu_bar.set_pick_mode_bit = 1 << (i-1) ;
      } else if( strcmp("SET_ROUTE_MODE",resource) == 0) {
        gd.menu_bar.set_route_mode_bit = 1 << (i-1) ;
      } else if( strcmp("SHOW_XSECT_PANEL",resource) == 0) {
        gd.menu_bar.show_xsect_panel_bit = 1 << (i-1) ;
      } else if( strcmp("SHOW_GRID_PANEL",resource) == 0) {
        gd.menu_bar.show_grid_panel_bit = 1 << (i-1) ;
      } else if( strcmp("RELOAD",resource) == 0) {
        gd.menu_bar.reload_bit = 1 << (i-1) ;
      } else if( strcmp("RESET",resource) == 0) {
        gd.menu_bar.reset_bit = 1 << (i-1) ;
      } else if( strcmp("SET_TO_NOW",resource) == 0) {
        gd.menu_bar.set_to_now_bit = 1 << (i-1) ;
      } else if( strcmp("CLOSE_POPUPS",resource) == 0) {
        gd.menu_bar.close_popups_bit = 1 << (i-1) ;
      } else if( strcmp("REPORT_MODE_ONOFF",resource) == 0) {
        gd.menu_bar.report_mode_bit = 1 << (i-1) ;
      } else if( strcmp("LANDUSE_ONOFF",resource) == 0) {
        gd.menu_bar.landuse_onoff_bit = 1 << (i-1) ;
      } else if( strcmp("SHOW_CMD_MENU",resource) == 0) {
        gd.menu_bar.show_cmd_menu_bit = 1 << (i-1) ;
      } else if( strcmp("SNAP_IMAGE",resource) == 0) {
        gd.menu_bar.snapshot_bit = 1 << (i-1) ;
      } else if( strcmp("ZOOM_BACK",resource) == 0) {
        gd.menu_bar.zoom_back_bit = 1 << (i-1) ;
      } else {
        fprintf(stderr,"Unrecognized Menu Bar Cell Function %d: %s\n",i,resource);
        exit(-1);
      }
    }
  } else {
    fprintf(stderr,"Menu Bar cells must be defined in this version\n");
    exit(-1);
  } 

  gd.h_win.min_ht = gd.uparams->getDouble( "cidd.min_ht", 0.0);
  gd.h_win.max_ht = gd.uparams->getDouble( "cidd.max_ht", 30.0);

  // Otherwise set in the command line arguments
  if( gd.num_render_heights == 0) {
    gd.h_win.cur_ht = gd.uparams->getDouble( "cidd.start_ht", 0.5);
  }

  gd.v_win.zmin_x = (double *)  calloc(sizeof(double), 1);
  gd.v_win.zmax_x = (double *)  calloc(sizeof(double), 1);
  gd.v_win.zmin_y = (double *)  calloc(sizeof(double), 1);
  gd.v_win.zmax_y = (double *)  calloc(sizeof(double), 1);

  gd.v_win.origin_lat = gd.h_win.origin_lat;
  gd.v_win.origin_lon = gd.h_win.origin_lon;
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

  // Load Wind Rendering preferences.
  gd.ideal_x_vects = gd.uparams->getLong( "cidd.ideal_x_vectors", 20);
  gd.ideal_y_vects = gd.uparams->getLong( "cidd.ideal_y_vectors", 20);
  gd.head_size = gd.uparams->getLong( "cidd.wind_head_size", 5);
  gd.shaft_len = gd.uparams->getLong( "cidd.barb_shaft_len", 25);
  gd.head_angle = gd.uparams->getDouble( "cidd.wind_head_angle", 45.0);

  /* Initialize Extra features data */
  gd.layers.wind_vectors = gd.uparams->getLong( "cidd.all_winds_on", 1);
  gd.layers.init_state_wind_vectors = gd.layers.wind_vectors;

  gd.layers.wind_mode = gd.uparams->getLong( "cidd.wind_mode", 0);
  gd.layers.wind_time_scale_interval = gd.uparams->getDouble( "cidd.wind_time_scale_interval", 10.0);

  gd.layers.wind_scaler = gd.uparams->getLong( "cidd.wind_scaler", 3);

  gd.legends.range = gd.uparams->getLong( "cidd.range_rings", 0) ? RANGE_BIT : 0;
  int plot_azimuths = gd.uparams->getLong( "cidd.azmith_lines", 0);
  plot_azimuths = gd.uparams->getLong( "cidd.azimuth_lines", plot_azimuths);

  plot_azimuths = gd.uparams->getLong( "cidd.azimuth_lines", plot_azimuths);
  gd.legends.azimuths = plot_azimuths ? AZIMUTH_BIT : 0;
  init_signal_handlers();  

  // Load the GRID / DATA FIELD parameters
  param_text_line_no = 0;
  param_text_len = 0;
  param_text = find_tag_text(gd.db_data,"GRIDS",
                             &param_text_len, &param_text_line_no);

  if(param_text == NULL || param_text_len <=0 ) {
    fprintf(stderr,"Couldn't Find GRIDS Section\n");
    exit(-1);
  }
  // establish and initialize sources of data 
  init_data_links(param_text, param_text_len, param_text_line_no);

  // Load the Wind Data Field  parameters
  param_text_line_no = 0;
  param_text_len = 0;
  param_text = find_tag_text(gd.db_data,"WINDS",
                             &param_text_len, &param_text_line_no);

  if(param_text == NULL || param_text_len <=0 ) {
    if(gd.debug)fprintf(stderr,"Couldn't Find WINDS Section\n");
  } else {
    /* Establish and initialize connections to wind fields */
    init_wind_data_links(param_text, param_text_len, param_text_line_no);
  }
    
  if(gd.layers.num_wind_sets == 0) gd.layers.wind_vectors = 0;

  // Instantiate and load the SYMPROD TDRP Parameter section
  gd.syprod_P = new Csyprod_P();

  param_text_line_no = 0;
  param_text_len = 0;
  param_text = find_tag_text(gd.db_data,"SYMPRODS",
                             &param_text_len, &param_text_line_no); 
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
  param_text = find_tag_text(gd.db_data,"TERRAIN",
                             &param_text_len, &param_text_line_no); 
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
  param_text = find_tag_text(gd.db_data,"ROUTE_WINDS",
                             &param_text_len, &param_text_line_no); 
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
  param_text = find_tag_text(gd.db_data,"GUI_CONFIG",
                             &param_text_len, &param_text_line_no);

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
  param_text = find_tag_text(gd.db_data,"IMAGE_GENERATION",
                             &param_text_len, &param_text_line_no);

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
  param_text = find_tag_text(gd.db_data,"DRAW_EXPORT",
                             &param_text_len, &param_text_line_no);

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
  param_text = find_tag_text(gd.db_data,"MAPS",
                             &param_text_len, &param_text_line_no);

  if(param_text == NULL || param_text_len <=0 ) {
    fprintf(stderr,"Could'nt Find MAPS SECTION\n");
    exit(-1);
  }
  /* establish and initialize Map overlay data */
  init_over_data_links(param_text, param_text_len, param_text_line_no);


  // Instantiate the Station locator classes and params.
  gd.locator_margin_km = gd.uparams->getDouble( "cidd.locator_margin_km", 50.0);
  gd.station_loc_url = gd.uparams->getString("cidd.station_loc_url", "");

  if(strlen(gd.station_loc_url) > 1) {
    if(gd.debug || gd.debug1) {
      fprintf(stderr,"Loading Station data from %s ...",gd.station_loc_url);
    }
    gd.station_loc =  new StationLoc();
    if(gd.station_loc == NULL) {
      fprintf(stderr,"CIDD: Fatal Alloc constructing new StationLoc()\n");
      exit(-1);
    }

    if(gd.station_loc->ReadData(gd.station_loc_url) < 0) {
      fprintf(stderr,"CIDD: Can't load Station Data from %s\n",gd.station_loc_url);
      exit(-1);
    }
    // gd.station_loc->PrintAll();  // DEBUG

    if(gd.debug || gd.debug1) {
      fprintf(stderr,"Done\n");
    }
  }

  gd.remote_ui_url = gd.uparams->getString("cidd.remote_ui_url", "");
  if(strlen(gd.remote_ui_url) > 1) {

    gd.remote_ui = new RemoteUIQueue();

    // Create the FMW with 4096 slots - Total size 1M
    bool compression = false;
    if (gd.remote_ui->initReadWrite( gd.remote_ui_url,
                                     (char *) "CIDD",
                                     (bool) gd.debug2,
                                     DsFmq::END, compression,
                                     4096, 4096*256 ) != 0 ) { 
      fprintf(stderr,"Problems initialising Remote Command Fmq: %s - aborting\n",gd.remote_ui_url);
    }
  }

  for(i=0; i < NUM_CONT_LAYERS; i++) {
    gd.layers.cont[i].field = 0;
    gd.layers.cont[i].min = gd.mrec[0]->cont_low;
    gd.layers.cont[i].max = gd.mrec[0]->cont_high;
    gd.layers.cont[i].interval = gd.mrec[0]->cont_interv;
    gd.layers.cont[i].labels_on  = gd.label_contours;
  }
  for(i=0; i < NUM_GRID_LAYERS; i++) { gd.layers.overlay_field[i] = 0; }

  for(j=0;j < gd.num_datafields; j++) {
    gd.h_win.redraw[j] = 1;
    gd.v_win.redraw[j] = 1;
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
  for(i = 1; i <= NUM_CONT_LAYERS; i++ ) {

    sprintf(str_buf,"cidd.contour%d_field",i);
    field_str = gd.uparams->getString(str_buf, "NoMaTcH");

    num_fields = STRparse(field_str, cfield, NAME_LENGTH, 3, NAME_LENGTH); 

    strncpy(gd.layers.cont[i-1].color_name,"white",NAME_LENGTH);

    if(gd.html_mode == 0) {
      /* Replace Underscores with spaces in field names */
      for(j=strlen(cfield[0])-1 ; j >= 0; j--) 
        if(gd.replace_underscores && cfield[0][j] == '_') cfield[0][j] = ' ';
    }
    for(j=0;j < gd.num_datafields; j++) {
      if(strcmp(gd.mrec[j]->button_name,cfield[0]) == 0) {
        gd.layers.cont[i-1].field = j;
        if(num_fields >  2 && (strncasecmp(cfield[2],"off",3) == 0) ) {
          gd.layers.cont[i-1].active = 0;
        } else {
          gd.layers.cont[i-1].active = 1;
        }

        gd.layers.cont[i-1].min = gd.mrec[j]->cont_low;
        gd.layers.cont[i-1].max = gd.mrec[j]->cont_high;
        gd.layers.cont[i-1].interval = gd.mrec[j]->cont_interv;

        if(num_fields > 1)
          strncpy(gd.layers.cont[i-1].color_name,cfield[1],NAME_LENGTH);

      }
    }
  }

  /* Set up default OVERLAY FIELDS */
  for(i = 1; i <= NUM_GRID_LAYERS; i++ ) {
    sprintf(str_buf,"cidd.layer%d_field",i);
    field_str =  gd.uparams->getString( str_buf, "NoMaTcH");

    num_fields = STRparse(field_str, cfield, NAME_LENGTH, 3, NAME_LENGTH); 

    if(gd.html_mode == 0) {
      /* Replace Underscores with spaces in field names */
      for(j=strlen(cfield[0])-1 ; j >= 0; j--) 
        if(gd.replace_underscores && field_str[j] == '_') cfield[0][j] = ' ';
    }
    for(j=0; j <  gd.num_datafields; j++) {
      if(strcmp(gd.mrec[j]->button_name,cfield[0]) == 0) {  
        if(num_fields >  1 && (strncasecmp(cfield[1],"off",3) == 0) ) {
          gd.layers.overlay_field_on[i-1] = 0;
        } else {
          gd.layers.overlay_field_on[i-1] = 1;
        }

        gd.layers.overlay_field[i-1] = j;
      }
    }
  }

  free(cfield[0]);
  free(cfield[1]);
  free(cfield[2]);

  // fonts

  gd.num_fonts = gd.uparams->getLong( "cidd.num_fonts", 1);
  
  if(gd.num_fonts > MAX_FONTS) {
    gd.num_fonts = MAX_FONTS;
    fprintf(stderr,"Cidd: Warning. Too Many Fonts. Limited to %d Fonts\n",MAX_FONTS);
  }

#ifdef NOTNOW
  
  // Make sure specified font for Winds, Contours and Products are within range.
  if(gd.prod.prod_font_num < 0) gd.prod.prod_font_num = 0;
  if(gd.prod.prod_font_num >= gd.num_fonts) gd.prod.prod_font_num = gd.num_fonts -1;
  
  for(i=0;i < gd.num_fonts; i++) {
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

  gd.contour_font_num = gd.uparams->getLong("contour_font_num", 6);
  gd.n_ideal_contour_labels = gd.uparams->getLong("n_ideal_contour_labels", 5);

  // canvas events

  gd.rotate_coarse_adjust = gd.uparams->getDouble( "cidd.rotate_coarse_adjust",6.0);
  gd.rotate_medium_adjust = gd.uparams->getDouble( "cidd.rotate_medium_adjust",2.0);
  gd.rotate_fine_adjust = gd.uparams->getDouble( "cidd.rotate_fine_adjust", 0.5);

  // zoom
  
  gd.min_zoom_threshold = gd.uparams->getDouble( "cidd.min_zoom_threshold", 5.0);

  // shmem
  
  gd.coord_key = gd.uparams->getLong( "cidd.coord_key", 63500);

  // gui

  gd.no_data_message = gd.uparams->getString
    ("cidd.no_data_message",
     "NO DATA FOUND (in this area at the selected time)");

  
  gd.h_win.margin.top =  gd.uparams->getLong( "cidd.horiz_top_margin", 20);
  gd.h_win.margin.bot =  gd.uparams->getLong( "cidd.horiz_bot_margin", 20);
  gd.h_win.margin.left = gd.uparams->getLong( "cidd.horiz_left_margin", 20);
  gd.h_win.margin.right = gd.uparams->getLong( "cidd.horiz_right_margin", 80);
  
  gd.h_win.legends_start_x =
    gd.uparams->getLong( "cidd.horiz_legends_start_x",
                         gd.h_win.margin.left + 5);
  
  gd.h_win.legends_start_y =
    gd.uparams->getLong( "cidd.horiz_legends_start_y",
                         gd.h_win.margin.top * 2);
  
  gd.h_win.legends_delta_y =
    gd.uparams->getLong( "cidd.horiz_legends_delta_y",
                         gd.h_win.margin.top);
  
  gd.h_win.min_height = gd.uparams->getLong( "cidd.horiz_min_height", 440);
  gd.h_win.min_width = gd.uparams->getLong( "cidd.horiz_min_width", 580);
  gd.h_win.active = 1;
  
  gd.wsddm_mode  = gd.uparams->getLong( "cidd.wsddm_mode", 0);
  gd.one_click_rhi  = gd.uparams->getLong( "cidd.one_click_rhi", 0);
  gd.click_posn_rel_to_origin  = gd.uparams->getLong( "cidd.click_posn_rel_to_origin", 0);
  gd.report_clicks_in_status_window = gd.uparams->getLong("cidd.report_clicks_in_status_window", 0);
  gd.report_clicks_in_degM_and_nm = gd.uparams->getLong( "cidd.report_clicks_in_degM_and_nm", 0);
  gd.magnetic_variation_deg = gd.uparams->getLong( "cidd.magnetic_variation_deg", 0);
  gd.check_data_times = gd.uparams->getLong("cidd.check_data_times", 0);

  gd.frame_label = gd.uparams->getString("cidd.horiz_frame_label", "Qucid");
  
  gd.status_info_file = gd.uparams->getString("cidd.status_info_file", "");

  // h_win_proc
  
  gd.horiz_default_y_pos = gd.uparams->getLong("cidd.horiz_default_y_pos",0);
  gd.horiz_default_x_pos = gd.uparams->getLong("cidd.horiz_default_x_pos",0);

  // overlays

  gd.map_file_sub_dir =  gd.uparams->getString( "cidd.map_file_subdir", "maps");

  // page_pu_proc

  gd.ideal_x_vectors = gd.uparams->getLong("cidd.ideal_x_vectors", 20);
  gd.ideal_y_vectors = gd.uparams->getLong("cidd.ideal_y_vectors", 20);
  gd.azimuth_interval = gd.uparams->getDouble( "cidd.azmith_interval", 30.0);
  gd.azimuth_radius = gd.uparams->getDouble( "cidd.azmith_radius", 200.0);
  gd.latest_click_mark_size = gd.uparams->getLong("cidd.latest_click_mark_size", 11);
  gd.range_ring_x_space = gd.uparams->getLong("cidd.range_ring_x_space", 50);
  gd.range_ring_y_space = gd.uparams->getLong("cidd.range_ring_y_space", 15);
  gd.range_ring_spacing = gd.uparams->getDouble("cidd.range_ring_spacing", -1.0);
  gd.max_ring_range = gd.uparams->getDouble("cidd.max_ring_range", 1000.0);
  gd.range_ring_labels = gd.uparams->getLong("cidd.range_ring_labels", 1);
  gd.wind_units_scale_factor = gd.uparams->getDouble("cidd.wind_units_scale_factor", 1.0);
  gd.wind_units_label = gd.uparams->getString("cidd.wind_units_label", "m/sec");
  gd.wind_w_scale_factor = gd.uparams->getDouble( "cidd.wind_w_scale_factor", 10.0);

  // symprods
  
  gd.scale_constant = gd.uparams->getDouble("cidd.scale_constant", 300.0);

  // timer control

  gd.redraw_interval = gd.uparams->getLong("cidd.redraw_interval", 1000);
  gd.update_interval = gd.uparams->getLong("cidd.update_interval", 120);

  // winds init
  
  gd.wind_marker_type = gd.uparams->getString( "cidd.wind_marker_type", "arrow");
  gd.wind_reference_speed = gd.uparams->getDouble( "cidd.wind_reference_speed", 10.0);

}
