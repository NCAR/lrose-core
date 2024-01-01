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
////////////////////////////////////////////////////////////
// LegacyParams.hh
//
// Read legacy params, write out tdrp-compatible param file
//
// Mike Dixon, EOL, NCAR, Boulder, CO, USA
// Dec 2023
//
/////////////////////////////////////////////////////////////

#include "LegacyParams.hh"
#include "cidd.h"
#include <cerrno>
#include <iostream>
#include <cstdio>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/utim.h>

// constructor

LegacyParams::LegacyParams()
{
  _printTdrp = false;
  _paramsBuf = NULL;
  _paramsBufLen = 0;
}

// destructor

LegacyParams::~LegacyParams()
{
  clear();
}

// Clear the data base

void LegacyParams::clear()
{
  _plist.clear();
  _paramsBuf = NULL;
  _paramsBufLen = 0;
}

////////////////////////////////////////
// read in from param file
//
// returns 0 on success, -1 on failure

int LegacyParams::readFromPath(const char *file_path,
                               const char *prog_name)

{

  // open file

  FILE *params_file;
  if ((params_file = fopen(file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - LegacyParams::read" << endl;
    cerr << "  Cannot read params from file: " << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // loop through file
  
  char line[BUFSIZ];

  while (!feof(params_file)) {
    
    // read a line
    
    if (fgets(line, BUFSIZ, params_file) == NULL) {
      break;
    }
    
    if (feof(params_file))
      break;

    // substitute in any environment variables
    
    usubstitute_env(line, BUFSIZ);

    // delete past any hash-bang

    char *sptr;
    if ((sptr = strstr(line, "#!")) != NULL) {
      *sptr = '\0';
    }
    
    // process only if the line has the program name followed by a period.
    
    char *name = line;
    if (strlen(name) < strlen(prog_name + 1)) {
      continue;
    }
    if (strncmp(prog_name, name, strlen(prog_name)) ||
	name[strlen(prog_name)] != '.') {
      continue;
    }

    // check that there is a colon

    char *colon = strchr(name, ':');
    if (!colon) {
      continue;
    }
    
    // back up past any white space
    
    char *end_of_name = colon - 1;
    while (*end_of_name == ' ' || *end_of_name == '\t') {
      end_of_name--;
    }

    // place null at end of name

    *(end_of_name + 1) = '\0';

    // get entry string

    char *entry = colon + 1;

    // advance past white space
    
    while (*entry == ' ' || *entry == '\t') {
      entry++;
    }

    // back up past white space
    
    char *end_of_entry = entry + strlen(entry);
    while (*end_of_entry == ' ' || *end_of_entry == '\t' ||
	   *end_of_entry == '\r' || *end_of_entry == '\n' ||
	   *end_of_entry == '\0') {
      end_of_entry--;
    }

    // place null at end of entry
    
    *(end_of_entry + 1) = '\0';

    // check that we do not already have this param
    
    bool previous_entry_found = false;
    for (size_t ii = 0; ii < _plist.size(); ii++) {
      if (_plist[ii].name == name) {
	_plist[ii].entry = entry;
	previous_entry_found = true;
	break;
      }
    } // ii

    // if previous entry was not found,
    // store name and entry pointers in params list

    if (!previous_entry_found) {
      param_list_t ll;
      ll.name = name;
      ll.entry = entry;
      _plist.push_back(ll);
    }
      
  } /* while (!feof(params_file)) */

  // close file

  fclose(params_file);

  // debug print

  // for (size_t ii = 0; ii < _plist.size(); ii++) {
  //   cerr << "name, val: " << _plist[ii].name << ", " << _plist[ii].entry << endl;
  // } // ii
  // cerr << "Param list size: " << _plist.size() << endl;

  return 0;

}

////////////////////////////////////////
// read from a param buffer
//
// returns 0 on success, -1 on failure

int LegacyParams::readFromBuf(const char *buf,
                              int buf_len,
                              const char *prog_name)

{

  // loop through lines in buffer
  
  char line[BUFSIZ];
  const char *ptr = buf;
  
  while (ptr < buf + buf_len) {
  
    // find a line

    const char *eol = strchr(ptr, '\n');
    if (eol == NULL) {
      break;
    }
    
    int lineLen = eol - ptr + 1;
    int copyLen;
    if (lineLen > BUFSIZ) {
      copyLen = BUFSIZ;
    } else {
      copyLen = lineLen;
    }
    STRncopy(line, ptr, copyLen);
    ptr += lineLen;

    // substitute in any environment variables
    
    usubstitute_env(line, BUFSIZ);
    
    // delete past any hash-bang

    char *sptr;
    if ((sptr = strstr(line, "#!")) != NULL) {
      *sptr = '\0';
    }
    
    // process only if the line has the program name followed by a period.
    
    char *name = line;
    if (strlen(name) < strlen(prog_name + 1)) {
      continue;
    }
    if (strncmp(prog_name, name, strlen(prog_name)) ||
	name[strlen(prog_name)] != '.') {
      continue;
    }

    // check that there is a colon

    char *colon = strchr(name, ':');
    if (!colon) {
      continue;
    }
    
    // back up past any white space
    
    char *end_of_name = colon - 1;
    while (*end_of_name == ' ' || *end_of_name == '\t') {
      end_of_name--;
    }

    // place null at end of name

    *(end_of_name + 1) = '\0';

    // get entry string

    char *entry = colon + 1;

    // advance past white space
    
    while (*entry == ' ' || *entry == '\t') {
      entry++;
    }

    // back up past white space
    
    char *end_of_entry = entry + strlen(entry);
    while (*end_of_entry == ' ' || *end_of_entry == '\t' ||
	   *end_of_entry == '\r' || *end_of_entry == '\n' ||
	   *end_of_entry == '\0') {
      end_of_entry--;
    }

    // place null at end of entry
    
    *(end_of_entry + 1) = '\0';

    // check that we do not already have this param
    
    bool previous_entry_found = false;
    for (size_t ii = 0; ii < _plist.size(); ii++) {
      if (_plist[ii].name == name) {
	_plist[ii].entry = entry;
	previous_entry_found = true;
	break;
      }
    } // ii

    // if previous entry was not found,
    // store name and entry pointers in params list

    if (!previous_entry_found) {
      param_list_t ll;
      ll.name = name;
      ll.entry = entry;
      _plist.push_back(ll);
    }
      
  } /* while (!feof(params_file)) */

  // debug print

  // for (size_t ii = 0; ii < _plist.size(); ii++) {
  //   cerr << "name, val: " << _plist[ii].name << ", " << _plist[ii].entry << endl;
  // } // ii
  // cerr << "Param list size: " << _plist.size() << endl;

  return 0;

}

///////////////////////////////////////////////////////////////
// read in the legacy params
// write out tdrp params file
///////////////////////////////////////////////////////////////

int LegacyParams::translateToTdrp(const string &legacyParamsPath,
                                  const string &tdrpParamsPath)
  
{

  int i,j,pid;
  int num_fields;
  int err_flag;
  long param_text_len;
  long param_text_line_no;
  const char *param_text;
  const char *resource;
  const char *field_str;
  char str_buf[128];   /* Space to build resource strings */
  char *cfield[3];     /* Space to collect sub strings */
  double delta_x,delta_y;
  double lat1 = 0, lat2 = 0;

  UTIMstruct temp_utime;

  // load up the params buffer from file or http
  
  if (_loadDbData(legacyParamsPath)) {
    fprintf(stderr,"LegacyParams::translateToTdrp()\n");
    fprintf(stderr,"  Could not load params from file\n");
    return -1;
  }
  
  // Retrieve the parameters from text file
  // read params in from buffer
  
  if (readFromBuf(_paramsBuf, _paramsBufLen, "cidd")) {
    fprintf(stderr,"LegacyParams::translateToTdrp()\n");
    fprintf(stderr,"  Could not read params buffer\n");
    return -1;
  }

  setPrintTdrp(true);

  // Load the Main parameters

  param_text_line_no = 0;
  param_text_len = 0;
  param_text = _findTagText(_paramsBuf,"MAIN_PARAMS",
                            &param_text_len, &param_text_line_no);
    
  if(param_text == NULL || param_text_len <=0 ) {
    fprintf(stderr,"Could'nt Find MAIN_PARAMS SECTION\n");
    return -1;
  }
    
  // set_X_parameter_database(param_text); /* load the main parameter database*/
  
  // if(!gd.quiet_mode) {
  //   fprintf(stderr,"\n\nCIDD: Version %s\nUcopyright %s\n\n",
  //           CIDD_VERSION, CIDD_UCOPYRIGHT);
  // }

  gd.debug |= getBoolean("cidd.debug_flag", 0);
  gd.debug1 |= getBoolean("cidd.debug1_flag", 0);
  gd.debug2 |= getBoolean("cidd.debug2_flag", 0);

  // IF demo_time is set in the params
  // Set into Archive Mode at the indicated time.

  gd.demo_time = getString("cidd.demo_time", "");

  gd.temporal_rounding = getLong("cidd.temporal_rounding", 300);
  
  gd.climo_mode = getString("cidd.climo_mode", "regular");

  /* Toggle for displaying the analog clock */
  gd.max_time_list_span = getLong("cidd.max_time_list_span", 365);

  // movies
  
  gd.starting_movie_frames = getLong("cidd.starting_movie_frames", 12);
  gd.time_interval = getDouble("cidd.time_interval",10.0);
  gd.frame_span = getDouble("cidd.frame_span", gd.time_interval);
  gd.forecast_interval = getDouble("cidd.forecast_interval", 0.0);
  gd.past_interval = getDouble("cidd.past_interval", 0.0);
  gd.movie_magnify_factor = getDouble("cidd.movie_magnify_factor",1.0);
  gd.check_data_times = getBoolean("cidd.check_data_times", 0);

  // clipping for rendering
  gd.check_clipping = getBoolean("cidd.check_clipping", 0);
  
  gd.stretch_factor = getDouble("cidd.stretch_factor", 1.5);
  gd.gather_data_mode = getLong("cidd.gather_data_mode",
                                            CLOSEST_TO_FRAME_CENTER);
  gd.redraw_interval =
    getLong("cidd.redraw_interval", REDRAW_INTERVAL);
  gd.update_interval =
    getLong("cidd.update_interval", UPDATE_INTERVAL);

  gd.datamap_host = getString("cidd.datamap_host", "");
  gd.data_timeout_secs = getLong("cidd.data_timeout_secs", 10);
  gd.simple_command_timeout_secs =  getLong("cidd.simple_command_timeout_secs",30);
  gd.complex_command_timeout_secs =  getLong("cidd.complex_command_timeout_secs",180);
  
  gd.movie_on = getBoolean("cidd.movie_on", 0);
  gd.movie_delay = getLong("cidd.movie_delay",3000);
  gd.movie_speed_msec = getLong("cidd.movie_speed_msec", 75);
  gd.reset_frames = getBoolean("cidd.reset_frames", 0);

  gd.model_run_list_hours = getLong("cidd.model_run_list_hours",24);

  // How many idle seconds can elapse before resetting the display
  gd.idle_reset_seconds = getLong("cidd.idle_reset_seconds",0);

  gd.html_mode = getBoolean("cidd.html_mode", 0);
  gd.run_once_and_exit = getBoolean("cidd.run_once_and_exit",0);

  gd.transparent_images = getBoolean("cidd.transparent_images", 0);
  
  // Image dir - for output images
  gd.image_dir = getString("cidd.image_dir", "/tmp/image_dir");
  
  gd.save_images_to_day_subdir =
    getBoolean("cidd.save_images_to_day_subdir", 0);
  
  // Set up our default image type.
  // force png for now
  gd.image_ext = getString("cidd.image_ext", "png");
  gd.image_ext = "png";

  gd.image_horiz_prefix = getString("cidd.image_horiz_prefix", "CP");
  gd.image_vert_prefix = getString("cidd.image_vert_prefix", "CV");
  gd.image_name_separator = getString("cidd.image_name_separator", "_");

  // output image file names
  
  gd.add_height_to_filename =
    getBoolean("cidd.add_height_to_filename",0);
     
  gd.add_frame_time_to_filename =
    getBoolean("cidd.add_frame_time_to_filename",1);
     
  gd.add_button_name_to_filename =
    getBoolean("cidd.add_button_name_to_filename",0);

  gd.add_frame_num_to_filename =
    getBoolean("cidd.add_frame_num_to_filename",1);

  gd.add_gen_time_to_filename =
    getBoolean("cidd.add_gen_time_to_filename",0);
  
  gd.add_valid_time_to_filename =
    getBoolean("cidd.add_valid_time_to_filename",0);
     
  gd.horiz_image_dir =
    getString("cidd.horiz_image_dir", "/tmp/cidd_horiz_image_dir");
  gd.horiz_image_fname =
    getString("cidd.horiz_image_fname", "cidd_horiz_view.png");
  gd.horiz_image_command =
    getString("cidd.horiz_image_command", "");
  
  gd.vert_image_dir =
    getString("cidd.vert_image_dir", "/tmp/cidd_vert_image_dir");
  gd.vert_image_fname =
    getString("cidd.vert_image_fname", "cidd_vert_view.png");
  gd.vert_image_command =
    getString("cidd.vert_image_command", "");
  
  gd.output_geo_xml =   getBoolean("cidd.output_geo_xml", 0);
  gd.use_latlon_in_geo_xml =   getBoolean("cidd.use_latlon_in_geo_xml", 0);

  gd.movieframe_time_format = getString(
	  "cidd.movieframe_time_format", "%H%M");

  gd.movieframe_time_mode = getLong(
          "cidd.movieframe_time_mode", 0);

  // script to run after generating image
  
  gd.image_convert_script =
    getString("cidd.image_convert_script", "convert_image.csh");
  
  resource = getString("cidd.print_script", "");
  if(strlen(resource) > 1) {
    gd.print_script = resource;
  }

  gd.series_convert_script =
    getString("cidd.series_convert_script", "make_anim.csh");

  // data compression from server
  
  gd.request_compressed_data =
    getBoolean("cidd.request_compressed_data",0);
  gd.request_gzip_vol_compression =
    getBoolean("cidd.request_gzip_vol_compression",0);

  /* Establish the projection type */

  gd.projection_type = getString("cidd.projection_type", "CARTESIAN");

  // projections
  
  gd.lambert_lat1 = getDouble("cidd.lambert_lat1",20.0);
  gd.lambert_lat2 = getDouble("cidd.lambert_lat2",60.0);
  gd.tangent_lat = getDouble("cidd.tangent_lat",90.0);
  gd.tangent_lon = getDouble("cidd.tangent_lon",0.0);
  gd.central_scale = getDouble("cidd.central_scale",1.0);

  gd.north_angle = getDouble("cidd.north_angle",0.0);

  /* Use cosine correction for computing range in polar data */
  if (gd.use_cosine_correction < 0) {
    // not set on command line
    int use_cosine = getBoolean("cidd.use_cosine", 1); // legacy
    gd.use_cosine_correction =
      getLong("cidd.use_cosine_correction", use_cosine);
  }

  gd.scale_units_per_km = getDouble("cidd.scale_units_per_km",1.0);
  gd.scale_units_label = getString("cidd.scale_units_label", "km");
  
  gd.always_get_full_domain = getBoolean("cidd.always_get_full_domain", 0);
  gd.do_not_clip_on_mdv_request = getBoolean("cidd.do_not_clip_on_mdv_request", 0);
  gd.do_not_decimate_on_mdv_request = getBoolean("cidd.do_not_decimate_on_mdv_request", 0);
     
  // zoom
  
  gd.min_zoom_threshold = getDouble("cidd.min_zoom_threshold", 5.0);

  gd.aspect_ratio = getDouble("cidd.aspect_ratio", 1.0);
  if (gd.aspect_ratio <= 0.0 && gd.debug) {
    cerr << "WARNING - Using first domain to set aspect ratio: " << endl;
  }

  /* Toggle for enabling a status report window */
  gd.enable_status_window = getBoolean("cidd.enable_status_window", 0);

  gd.report_clicks_in_status_window = getBoolean("cidd.report_clicks_in_status_window", 0);
  gd.report_clicks_in_degM_and_nm = getBoolean("cidd.report_clicks_in_degM_and_nm", 0);
  gd.magnetic_variation_deg = getDouble("cidd.magnetic_variation_deg", 0);
  
  /* Toggle for enabling a Save Image Panel */
  // WARNING - ALLOWS USERS SHELL ACCESS
  gd.enable_save_image_panel =
    getBoolean("cidd.enable_save_image_panel", 0);
  
  gd.domain_limit_min_x = getDouble("cidd.domain_limit_min_x",-10000);
  gd.domain_limit_max_x = getDouble("cidd.domain_limit_max_x",10000);
  gd.domain_limit_min_y = getDouble("cidd.domain_limit_min_y",-10000);
  gd.domain_limit_max_y = getDouble("cidd.domain_limit_max_y",10000);
  
  // origin latitude and longitude
  
  gd.origin_latitude = getDouble("cidd.origin_latitude", 0.0);
  gd.origin_longitude = getDouble("cidd.origin_longitude", 0.0);

  // click location on reset
  
  gd.reset_click_latitude =
    getDouble("cidd.reset_click_latitude", gd.origin_latitude);
  gd.reset_click_longitude =
    getDouble("cidd.reset_click_longitude", gd.origin_longitude);

  gd.planview_start_page = getLong("cidd.planview_start_page", 1) -1;
  gd.xsect_start_page = getLong("cidd.xsect_start_page", 1) -1;
  
  gd.num_zoom_levels =  getLong("cidd.num_zoom_levels",1);
  gd.start_zoom_level =  getLong("cidd.start_zoom_level",1);
  gd.zoom_limits_in_latlon =  getBoolean("cidd.zoom_limits_in_latlon",0);
  gd.num_cache_zooms = getLong("cidd.num_cache_zooms",1);

  gd.min_ht = getDouble("cidd.min_ht", 0.0);
  gd.max_ht = getDouble("cidd.max_ht", 30.0);
  gd.start_ht = getDouble("cidd.start_ht", 0.0);

  gd.map_file_subdir =  getString("cidd.map_file_subdir", "maps");
  gd.uparams->getString( "cidd.color_file_subdir", "colorscales");
     
  // Instantiate the Station locator classes and params.
  gd.locator_margin_km = getDouble("cidd.locator_margin_km", 50.0);
  gd.station_loc_url = getString("cidd.station_loc_url", "");

  gd.remote_ui_url = getString("cidd.remote_ui_url", "");

  gd.http_tunnel_url = getString("cidd.http_tunnel_url", "");
  gd.http_proxy_url = getString("cidd.http_proxy_url", "");
  
  const char *color_name = gd.uparams->getString("cidd.foreground_color", "White");
  color_name = gd.uparams->getString("cidd.background_color", "Black");
  color_name = gd.uparams->getString("cidd.margin_color", color_name);
  color_name = gd.uparams->getString("cidd.out_of_range_color", "transparent");
  color_name = gd.uparams->getString("cidd.route_path_color", "yellow");
  color_name = gd.uparams->getString("cidd.time_axis_color", "cyan");
  color_name = gd.uparams->getString("cidd.time_frame_color", "yellow");
  color_name = gd.uparams->getString("cidd.height_axis_color", "cyan");
  color_name = gd.uparams->getString("cidd.height_indicator_color", "red");
  color_name = gd.uparams->getString("cidd.range_ring_color", "grey");
  color_name = gd.uparams->getString("cidd.missing_data_color","transparent");
  color_name = gd.uparams->getString("cidd.bad_data_color","transparent");
  color_name = gd.uparams->getString("cidd.epoch_indicator_color", "yellow");
  color_name = gd.uparams->getString("cidd.now_time_color", "red");

  // need multiple time ticks - i.e. array
  
  color_name = gd.uparams->getString("cidd.time_tick_color", "yellow");
  color_name = gd.uparams->getString("cidd.latest_click_mark_color", "red");
  color_name = gd.uparams->getString( "cidd.latest_client_mark_color", "yellow");
  
  /* Toggle for displaying the height Selector in Right Margin */
  gd.show_height_sel = getBoolean("cidd.show_height_sel", 1);

  /* Toggle for displaying data access and rendering messages */
  gd.show_data_messages = getBoolean("cidd.show_data_messages", 1);

  gd.latlon_mode = getLong("cidd.latlon_mode",0);

  gd.label_time_format = getString(
	  "cidd.label_time_format", "%m/%d/%y %H:%M:%S");

  gd.moviestart_time_format = getString(
	  "cidd.moviestart_time_format", "%H:%M %m/%d/%Y");

  gd.frame_range_time_format = getString(
	  "cidd.frame_range_time_format", "%H:%M");

  // Get the on/off state of the extra legend plotting - Force to either 0 or 1

  gd.layer_legends_on = (getBoolean("cidd.layer_legends_on", 1) & 1);
  gd.cont_legends_on = (getBoolean("cidd.cont_legends_on", 1) & 1);
  gd.wind_legends_on = (getBoolean("cidd.wind_legends_on", 1) & 1);

  /* Toggle for displaying data labels */
  gd.display_labels = getBoolean("cidd.display_labels", 1);

  /* Toggle for displaying the analog clock */
  gd.display_ref_lines = getBoolean("cidd.display_ref_lines", 1);

  // margins
  
  gd.top_margin_render_style =
    getLong("cidd.top_margin_render_style", 1);
  
  gd.bot_margin_render_style =
    getLong("cidd.bot_margin_render_style", 1);
  
  // h_win_proc
  
  gd.horiz_default_y_pos = getLong("cidd.horiz_default_y_pos",0);
  gd.horiz_default_x_pos = getLong("cidd.horiz_default_x_pos",0);

  gd.horiz_default_height = getLong("cidd.horiz_default_height", 600);
  gd.horiz_default_width = getLong("cidd.horiz_default_width", 800);

  gd.horiz_min_height = getLong("cidd.horiz_min_height", 400);
  gd.horiz_min_width = getLong("cidd.horiz_min_width", 600);

  gd.horiz_top_margin =  getLong("cidd.horiz_top_margin", 20);
  gd.horiz_bot_margin =  getLong("cidd.horiz_bot_margin", 20);
  gd.horiz_left_margin = getLong("cidd.horiz_left_margin", 20);
  gd.horiz_right_margin = getLong("cidd.horiz_right_margin", 80);
  
  gd.horiz_legends_start_x = getLong("cidd.horiz_legends_start_x", 0);
  gd.horiz_legends_start_y = getLong("cidd.horiz_legends_start_y", 0);
  gd.horiz_legends_delta_y = getLong("cidd.horiz_legends_delta_y", 0);
  
  // h_win_proc
  
  gd.vert_default_x_pos = getLong("cidd.vert_default_x_pos", 0);
  gd.vert_default_y_pos = getLong("cidd.vert_default_y_pos", 0);
  
  gd.vert_default_height = getLong("cidd.vert_default_height", 400);
  gd.vert_default_width = getLong("cidd.vert_default_width", 600);

  gd.vert_min_height = getLong("cidd.vert_min_height", 400);
  gd.vert_min_width = getLong("cidd.vert_min_width", 600);
  
  gd.vert_top_margin =  getLong("cidd.vert_top_margin", 20);
  gd.vert_bot_margin =  getLong("cidd.vert_bot_margin", 20);
  gd.vert_left_margin = getLong("cidd.vert_left_margin", 20);
  gd.vert_right_margin = getLong("cidd.vert_right_margin", 80);
  
  gd.vert_legends_start_x = getLong("cidd.vert_legends_start_x", 0);
  gd.vert_legends_start_y = getLong("cidd.vert_legends_start_y", 0);
  gd.vert_legends_delta_y = getLong("cidd.vert_legends_delta_y", 0);

  // range rings
  
  gd.range_ring_spacing = getDouble("cidd.range_ring_spacing", -1.0);
  gd.max_ring_range = getDouble("cidd.max_ring_range", 1000.0);
  
  // Toggle for displaying range rings at the data's origin - Useful for mobile units.
  gd.range_ring_follows_data = getBoolean("cidd.range_ring_follows_data", 0);
  gd.range_ring_for_radar_only = getBoolean("cidd.range_ring_for_radar_only", 0);

  // Toggle for shifting the display origin - Useful for mobile units.
  gd.domain_follows_data = getBoolean("cidd.domain_follows_data", 0);

  int plot_range_rings = getBoolean("cidd.range_rings", 0);
  gd.range_ring_x_space = getLong("cidd.range_ring_x_space", 50);
  gd.range_ring_y_space = getLong("cidd.range_ring_y_space", 15);
  gd.range_ring_labels = getBoolean("cidd.range_ring_labels", 1);
  
  gd.azimuth_interval = getDouble("cidd.azmith_interval", 30.0);
  gd.azimuth_radius = getDouble("cidd.azmith_radius", 200.0);
  int plot_azimuth_lines = getBoolean("cidd.azmith_lines", 0);
  
  gd.all_winds_on = getBoolean("cidd.all_winds_on", 1);

  // Load Wind Rendering preferences.
  gd.barb_shaft_len = getLong("cidd.barb_shaft_len", 33);
  gd.ideal_x_vects = getLong("cidd.ideal_x_vectors", 20);
  gd.ideal_y_vects = getLong("cidd.ideal_y_vectors", 20);
  gd.wind_head_size = getLong("cidd.wind_head_size", 5);
  gd.wind_head_angle = getDouble("cidd.wind_head_angle", 45.0);
  
  gd.wind_scaler = getLong("cidd.wind_scaler", 3);
  gd.wind_time_scale_interval = getDouble("cidd.wind_time_scale_interval", 10.0);

  gd.wind_marker_type = getString("cidd.wind_marker_type", "arrow");
  gd.wind_w_scale_factor = getDouble("cidd.wind_w_scale_factor", 10.0);
  gd.wind_units_scale_factor = getDouble("cidd.wind_units_scale_factor", 1.0);
  gd.wind_reference_speed = getDouble("cidd.wind_reference_speed", 10.0);
  gd.wind_units_label = getString("cidd.wind_units_label", "m/sec");

  gd.label_contours = getBoolean("cidd.label_contours",1);
  gd.contour_line_width = getLong("cidd.contour_line_width", 1);
  gd.smooth_contours = getBoolean("cidd.smooth_contours", 0);
  gd.use_alt_contours = getBoolean("cidd.use_alt_contours", 0);
  gd.add_noise = getBoolean("cidd.add_noise", 0);
  gd.special_contour_value = getDouble("cidd.special_contour_value", 0.0);

  gd.map_bad_to_min_value =  getBoolean("cidd.map_bad_to_min_value", 0);
  gd.map_missing_to_min_value =  getBoolean("cidd.map_missing_to_min_value", 0);


  // main field on top?
  
  gd.draw_main_on_top = (getBoolean("cidd.draw_main_on_top", 0) & 1);

  // latest click location
  
  gd.mark_latest_click_location =
    getBoolean("cidd.mark_latest_click_location", 0);

  gd.latest_click_mark_size = getLong("cidd.latest_click_mark_size", 11);

  gd.num_fonts = getLong("cidd.num_fonts", 1);
  gd.font_display_mode = getLong("cidd.font_display_mode",1);

  
  /* Toggle for displaying the analog clock */
  gd.show_clock = getBoolean("cidd.show_clock", 0);

  /* Set the time to display on the analog clock */
  gd.draw_clock_local = getBoolean("cidd.draw_clock_local", 0);
  
  /* Use local times for Product timestamps and user input widgets. */
  gd.use_local_timestamps = getBoolean("cidd.use_local_timestamps", 0);
  
  // field menu - number of columns
  
  gd.num_field_menu_cols = getLong("cidd.num_field_menu_cols",0);
  
  
  gd.wsddm_mode  = getBoolean("cidd.wsddm_mode", 0);
  gd.one_click_rhi  = getBoolean("cidd.one_click_rhi", 0);

  // canvas events

  gd.rotate_coarse_adjust = getDouble("cidd.rotate_coarse_adjust",6.0);
  gd.rotate_medium_adjust = getDouble("cidd.rotate_medium_adjust",2.0);
  gd.rotate_fine_adjust = getDouble("cidd.rotate_fine_adjust", 0.5);

  gd.disable_pick_mode =   getBoolean("cidd.disable_pick_mode", 1);
  gd.replace_underscores =   getBoolean("cidd.replace_underscores", 1);
  gd.close_popups =   getBoolean("cidd.close_popups", 0);
  gd.clip_overlay_fields =   getBoolean("cidd.clip_overlay_fields", 0);
  
  gd.frame_label = getString("cidd.horiz_frame_label", "Qucid");

  gd.no_data_message = getString
    ("cidd.no_data_message",
     "NO DATA FOUND (in this area at the selected time)");
  
  gd.status_info_file = getString("cidd.status_info_file", "");

  gd.help_command = getString("cidd.help_command", "");

  // Bookmarks for a menu of URLS - Index starts at 1
  gd.bookmark_command = getString("cidd.bookmark_command", "");
  gd.num_bookmarks = getLong("cidd.num_bookmarks", 0);
  
  gd.image_inten = getDouble("cidd.image_inten", 0.8);
  gd.inten_levels = getLong("cidd.inten_levels", 32);
  gd.data_inten = getDouble("cidd.data_inten", 1.0);

  gd.image_fill_threshold =
    getLong("cidd.image_fill_threshold", 120000);

  gd.dynamic_contour_threshold =
    getLong("cidd.dynamic_contour_threshold", 160000);

  // shmem
  
  gd.coord_key = getLong("cidd.coord_key", 63500);

  gd.products_on = getBoolean("cidd.products_on", 1);
  gd.product_line_width = getLong("cidd.product_line_width", 1);
  gd.product_font_size = getLong("cidd.product_font_size", 1);
  // symprods
  
  gd.scale_constant = getDouble("cidd.scale_constant", 300.0);

  
  // copy to structs
  
  STRcopy(gd.h_win.image_dir, gd.image_dir, MAX_PATH_LEN);
  STRcopy(gd.v_win.image_dir, gd.image_dir, MAX_PATH_LEN);

  
  STRcopy(gd.h_win.image_fname, gd.horiz_image_fname, MAX_PATH_LEN);
  STRcopy(gd.h_win.image_command, gd.horiz_image_command, MAX_PATH_LEN);

  STRcopy(gd.v_win.image_fname, gd.vert_image_fname, MAX_PATH_LEN);
  STRcopy(gd.v_win.image_command, gd.vert_image_command, MAX_PATH_LEN);

  // If individual horiz and vert scripts have not been set
  // use the General one.
  
  if(strlen(gd.v_win.image_command) < 3) {
    STRcopy(gd.v_win.image_command,gd.image_convert_script,MAX_PATH_LEN);
  }

  if(strlen(gd.h_win.image_command) < 3) {
    STRcopy(gd.h_win.image_command,gd.image_convert_script,MAX_PATH_LEN);
  }

  if(gd.idle_reset_seconds <= 0 || gd.html_mode == 1) {
    gd.idle_reset_seconds = 259200000; // 3000 days.
  }



  // contours etc

  gd.layers.layer_legends_on = gd.layer_legends_on;
  gd.layers.cont_legends_on = gd.cont_legends_on;
  gd.layers.wind_legends_on = gd.wind_legends_on;
  gd.layers.contour_line_width = gd.contour_line_width;
  gd.layers.smooth_contours = gd.smooth_contours;
  gd.layers.use_alt_contours = gd.use_alt_contours;
  gd.layers.add_noise = gd.add_noise;
  gd.layers.special_contour_value = gd.special_contour_value;
  gd.layers.map_bad_to_min_value = gd.map_bad_to_min_value;
  gd.layers.map_missing_to_min_value = gd.map_missing_to_min_value;

  gd.contour_font_num = getLong("contour_font_num", 6);
  gd.n_ideal_contour_labels = getLong("n_ideal_contour_labels", 5);

  
  // drawing
  
  gd.drawing_mode = 0;

  // products
  
  gd.prod.products_on = gd.products_on;
  gd.prod.prod_line_width = gd.product_line_width;
  gd.prod.prod_font_num = gd.product_font_size;
  
  for(i=0; i < NUM_PRODUCT_DETAIL_THRESHOLDS; i++) {

    sprintf(str_buf,"cidd.product_detail_threshold%d",i+1);
    gd.product_detail_threshold[i] = getDouble(str_buf,0.0);
    gd.prod.detail[i].threshold = gd.product_detail_threshold[i];
    
    sprintf(str_buf,"cidd.product_detail_adjustment%d",i+1);
    gd.product_detail_adjustment[i] = getLong(str_buf,0);
    gd.prod.detail[i].adjustment = gd.product_detail_adjustment[i];

  }
	

  // if domain follows data, do not clip or decimate

  if (gd.domain_follows_data) {
    gd.always_get_full_domain = 1;
    gd.do_not_clip_on_mdv_request = 1;
    gd.do_not_decimate_on_mdv_request = 1;
  }

  if(gd.num_bookmarks > 0) {
    gd.bookmark = (bookmark_t *)  calloc(sizeof(bookmark_t),gd.num_bookmarks);
  }

  err_flag = 0;
  for(i=0; i < gd.num_bookmarks; i++) {
    sprintf(str_buf,"cidd.bookmark%d",i+1);
    gd.bookmark[i].url = getString(str_buf,"");

    if(strlen(gd.bookmark[i].url) < 4) {
      fprintf(stderr,"Error: Parameter %s undefined\n",str_buf);
      err_flag++;
    }

    sprintf(str_buf,"cidd.bookmark_label%d",i+1);
    gd.bookmark[i].label = getString(str_buf,"");

    if(strlen(gd.bookmark[i].label) < 1) {
      fprintf(stderr,"Error: Parameter %s undefined\n",str_buf);
      err_flag++;
    }

  }
  if(err_flag) {
    fprintf(stderr,"Correct the cidd.bookmark section of the parameter file\n");
    return -1;
  }

  gd.h_win.origin_lat = gd.origin_latitude;
  gd.h_win.origin_lon = gd.origin_longitude;

  gd.h_win.reset_click_lat = gd.reset_click_latitude;
  gd.h_win.reset_click_lon = gd.reset_click_longitude;

  gd.proj_param[0] = gd.north_angle; // flat projection is default

  /* Establish the native projection type */

  gd.projection_type = getString("cidd.projection_type", "CARTESIAN");

  if (strncasecmp(gd.projection_type,"CARTESIAN",9) == 0) {

    gd.display_projection = Mdvx::PROJ_FLAT;
    if(gd.debug) {
      printf("Cartesian Projection - Origin at: %g, %g\n", 
             gd.origin_latitude,gd.origin_longitude);
    }
    gd.proj.initFlat(gd.origin_latitude,gd.origin_longitude,gd.north_angle);

  } else if (strncasecmp(gd.projection_type,"LAT_LON",7) == 0) {

    gd.display_projection = Mdvx::PROJ_LATLON;
    if(gd.debug) {
      printf("LATLON/ Cylindrical Projection - Origin at: %g, %g\n",
             gd.origin_latitude,gd.origin_longitude);
    }

  } else if (strncasecmp(gd.projection_type,"LAMBERT",7) == 0) {

    gd.display_projection = Mdvx::PROJ_LAMBERT_CONF;
    gd.proj_param[0] = gd.lambert_lat1;
    gd.proj_param[1] = gd.lambert_lat2;
    if(lat1 == -90.0 || lat2 == -90.0) {
      fprintf(stderr,
              "Must set cidd.lambert_lat1 and cidd.lambert_lat2 parameters for LAMBERT projections\n");
      return -1;
    }
    if(gd.debug) {
      printf("LAMBERT Projection - Origin at: %g, %g Parallels at: %g, %g\n",
             gd.origin_latitude,gd.origin_longitude,
             gd.lambert_lat1, gd.lambert_lat2);
      gd.proj.initLambertConf(gd.origin_latitude,gd.origin_longitude,
                              gd.lambert_lat1, gd.lambert_lat2);
    }

  } else if (strncasecmp(gd.projection_type,"STEREOGRAPHIC",13) == 0) {

    gd.display_projection = Mdvx::PROJ_OBLIQUE_STEREO;
    gd.proj_param[0] = gd.tangent_lat;
    gd.proj_param[1] = gd.tangent_lon;
    gd.proj_param[2] = gd.central_scale;
    if(gd.debug) {
      printf("Oblique Stereographic Projection - Origin at: %g, %g Tangent at: %g, %g\n",
             gd.origin_latitude,gd.origin_longitude,
             gd.tangent_lat,gd.tangent_lon);
    }
    gd.proj.initStereographic(gd.tangent_lat, gd.tangent_lon, gd.central_scale);
    gd.proj.setOffsetOrigin(gd.origin_latitude,gd.origin_longitude);

  } else if (strncasecmp(gd.projection_type,"POLAR_STEREO",12) == 0) {

    gd.display_projection = Mdvx::PROJ_POLAR_STEREO;
    gd.proj_param[0] = gd.tangent_lat;
    gd.proj_param[1] = gd.tangent_lon;
    gd.proj_param[2] = gd.central_scale;
    if(gd.debug) {
      printf("Polar Stereographic Projection - Origin at: %g, %g Tangent at: %g, %g\n",
             gd.origin_latitude,gd.origin_longitude,
             gd.tangent_lat,gd.tangent_lon);
    }
    gd.proj.initPolarStereo
      (gd.tangent_lon,
       (Mdvx::pole_type_t) (gd.tangent_lat < 0.0 ? Mdvx::POLE_SOUTH : Mdvx::POLE_NORTH),
       gd.central_scale);
    gd.proj.setOffsetOrigin(gd.origin_latitude,gd.origin_longitude);

  } else if (strncasecmp(gd.projection_type,"MERCATOR",8) == 0) {

    gd.display_projection = Mdvx::PROJ_MERCATOR;
    if(gd.debug) {
      printf("MERCATOR Projection - Origin at: %g, %g\n",
             gd.origin_latitude,gd.origin_longitude);
    }
    gd.proj.initMercator(gd.origin_latitude,gd.origin_longitude);

  } else {

    fprintf(stderr,"Unknown projection type: cidd.projection_type = %s !\n", gd.projection_type);
    fprintf(stderr," Current valid types are: CARTESIAN, LAT_LON, LAMBERT, STEREOGRAPHIC, MERCATOR\n");
    return -1;

  }

  gd.h_win.last_page = -1;
  gd.v_win.last_page = -1;

  // movies

  pid = getpid();
  for(i=0; i < MAX_FRAMES; i++) {
    sprintf(gd.movie.frame[i].fname,
            "%s/cidd_im%d_%d.",
            gd.image_dir, pid, i);
    gd.movie.frame[i].h_xid = 0;
    gd.movie.frame[i].v_xid = 0;
    gd.movie.frame[i].redraw_horiz = 1;
    gd.movie.frame[i].redraw_vert = 1;
  }


  // copy movie info to other globals

  gd.movie.movie_on = gd.movie_on;
  if(gd.html_mode) gd.movie.movie_on = 0;
  gd.movie.magnify_factor = gd.movie_magnify_factor;
  gd.movie.time_interval = gd.time_interval;
  gd.movie.frame_span = gd.frame_span;
  gd.movie.num_frames = gd.starting_movie_frames;
  gd.movie.reset_frames = gd.reset_frames;
  gd.movie.delay = gd.movie_delay;
  gd.movie.forecast_interval = gd.forecast_interval;
  gd.movie.past_interval = gd.past_interval;
  gd.movie.mr_stretch_factor = gd.stretch_factor;
  gd.movie.round_to_seconds = gd.temporal_rounding;
  gd.movie.display_time_msec = gd.movie_speed_msec;

  gd.movie.start_frame = 0;
  gd.movie.sweep_on = 0;
  gd.movie.sweep_dir = 1;
  gd.movie.end_frame = gd.movie.num_frames -1 ;
  gd.movie.cur_frame = gd.movie.num_frames -1;
  gd.movie.last_frame = gd.movie.cur_frame;

  gd.movie.climo_mode = REGULAR_INTERVAL;
  if(strncmp(gd.climo_mode,"daily", 5) == 0) gd.movie.climo_mode = DAILY_INTERVAL;
  if(strncmp(gd.climo_mode,"yearly",6) == 0) gd.movie.climo_mode = YEARLY_INTERVAL;

  
  /* If demo time param is not set and command line option hasn't set archive mode */

  if(strlen(gd.demo_time) < 8 &&
     (gd.movie.mode != ARCHIVE_MODE) ) { /* REALTIME MODE */
    gd.movie.mode = REALTIME_MODE;
    gd.coord_expt->runtime_mode = RUNMODE_REALTIME;
    gd.coord_expt->time_seq_num++;
    gd.forecast_mode = 0;
    
    /* set the first index's time based on current time  */
    time_t clock = time(0);    /* get current time */
    gd.movie.start_time = (time_t) (clock - ((gd.movie.num_frames -1) * gd.movie.time_interval * 60.0));
    gd.movie.start_time -= (gd.movie.start_time % gd.movie.round_to_seconds);
    gd.movie.demo_time = 0; // Indicated REAL-TIME is Native

    UTIMunix_to_date(gd.movie.start_time,&temp_utime);
    gd.movie.demo_mode = 0;

  } else {   /* ARCHIVE MODE */
    
    if(gd.movie.mode != ARCHIVE_MODE) { /* if not set by command line args */
      gd.movie.mode = ARCHIVE_MODE;     /* time_series */
      gd.coord_expt->runtime_mode = RUNMODE_ARCHIVE;
      gd.coord_expt->time_seq_num++;

      parse_string_into_time(gd.demo_time,&temp_utime);
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

  reset_time_points(); // reset movie


  // caching zooms to go back to
  
  if(gd.num_cache_zooms > MAX_CACHE_PIXMAPS) {
    gd.num_cache_zooms = MAX_CACHE_PIXMAPS;
  }
  if(gd.num_cache_zooms < 1) {
    gd.num_cache_zooms = 1 ;
  }

  gd.h_win.can_xid = (Drawable *) calloc(sizeof(Drawable *),gd.num_cache_zooms);
  gd.v_win.can_xid = (Drawable *) calloc(sizeof(Drawable *),gd.num_cache_zooms);
  
  if(gd.html_mode ==0) {
    gd.h_win.zoom_level = gd.start_zoom_level;
    gd.h_win.num_zoom_levels = gd.num_zoom_levels;
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
    gd.h_win.min_x = max(gd.domain_limit_min_x, -360.0);
    gd.h_win.max_x = min(gd.domain_limit_max_x, 360.0);
    gd.h_win.min_y = max(gd.domain_limit_min_y, -90.0);
    gd.h_win.max_y = min(gd.domain_limit_max_y, 90.0);
  } else {
    gd.h_win.min_x = gd.domain_limit_min_x;
    gd.h_win.max_x = gd.domain_limit_max_x;
    gd.h_win.min_y = gd.domain_limit_min_y;
    gd.h_win.max_y = gd.domain_limit_max_y;
  }
  
  gd.h_win.min_ht = gd.min_ht;
  gd.h_win.max_ht = gd.max_ht;

  // Otherwise set in the command line arguments
  if(gd.num_render_heights == 0) {
    gd.h_win.cur_ht = gd.start_ht;
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
    double minx = getDouble(str_buf,-200.0/(i+1));

    sprintf(str_buf, "cidd.level%d_min_ykm", i+1);
    double miny = getDouble(str_buf,-200.0/(i+1));

    sprintf(str_buf, "cidd.level%d_max_xkm", i+1);
    double maxx = getDouble(str_buf,200.0/(i+1));

    sprintf(str_buf, "cidd.level%d_max_ykm", i+1);
    double maxy = getDouble(str_buf,200.0/(i+1));

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
  gd.menu_bar.num_menu_bar_cells = getLong("cidd.num_menu_bar_cells",0);
  if(gd.menu_bar.num_menu_bar_cells > 0) {
    for(i=1; i <= gd.menu_bar.num_menu_bar_cells; i++) {
      sprintf(str_buf,"cidd.menu_bar_funct%d",i);
      resource = getString(str_buf,"Not Defined");
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
        return -1;
      }
    }
  } else {
    fprintf(stderr,"Menu Bar cells must be defined in this version\n");
    return -1;
  } 

  gd.v_win.zmin_x = (double *)  calloc(sizeof(double), 1);
  gd.v_win.zmax_x = (double *)  calloc(sizeof(double), 1);
  gd.v_win.zmin_y = (double *)  calloc(sizeof(double), 1);
  gd.v_win.zmax_y = (double *)  calloc(sizeof(double), 1);

  gd.v_win.origin_lat = gd.origin_latitude;
  gd.v_win.origin_lon = gd.origin_longitude;
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

  /* Initialize Extra features data */
  gd.layers.wind_vectors = gd.all_winds_on;
  gd.layers.init_state_wind_vectors = gd.layers.wind_vectors;
  
  gd.wind_mode = getBoolean("cidd.wind_mode", 0);
  gd.layers.wind_mode = gd.wind_mode;
  
  gd.layers.wind_time_scale_interval = gd.wind_time_scale_interval;

  gd.layers.wind_scaler = gd.wind_scaler;

  // int plot_azimuths = getLong("cidd.azmith_lines", 0);
  // plot_azimuths = getLong("cidd.azimuth_lines", plot_azimuths);

  // plot_azimuths = getLong("cidd.azimuth_lines", plot_azimuths);
  // gd.legends.azimuths = plot_azimuths ? AZIMUTH_BIT : 0;

  // Load the GRID / DATA FIELD parameters
  param_text_line_no = 0;
  param_text_len = 0;
  param_text = find_tag_text(_paramsBuf,"GRIDS",
                             &param_text_len, &param_text_line_no);

  if(param_text == NULL || param_text_len <=0 ) {
    fprintf(stderr,"Couldn't Find GRIDS Section\n");
    return -1;
  }
  // establish and initialize sources of data 
  if (_initDataFields(param_text, param_text_len, param_text_line_no)) {
    return -1;
  }

  // copy legacy params to tdrp

  // Load the Wind Data Field  parameters
  param_text_line_no = 0;
  param_text_len = 0;
  param_text = find_tag_text(_paramsBuf,"WINDS",
                             &param_text_len, &param_text_line_no);


  if(param_text == NULL || param_text_len <=0 ) {
    if(gd.debug)fprintf(stderr,"Couldn't Find WINDS Section\n");
  } else {
    /* Establish and initialize connections to wind fields */
    if (_initWindFields(param_text, param_text_len, param_text_line_no)) {
      return -1;
    }
  }
    
  if(gd.layers.num_wind_sets == 0) gd.layers.wind_vectors = 0;

  // Instantiate and load the SYMPROD TDRP Parameter section
  gd.syprod_P = new Csyprod_P();

  param_text_line_no = 0;
  param_text_len = 0;
  param_text = find_tag_text(_paramsBuf,"SYMPRODS",
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
      return -1;
    }
  }

  // Instantiate and load the TERRAIN TDRP Parameter 
  gd.layers.earth._P = new Cterrain_P();

  param_text_line_no = 0;
  param_text_len = 0;
  param_text = find_tag_text(_paramsBuf,"TERRAIN",
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
      return -1;
    }
    if(strlen(gd.layers.earth._P->terrain_url) >0) {
      gd.layers.earth.terrain_active = 1;
      gd.layers.earth.terr = (met_record_t *) calloc(sizeof(met_record_t), 1);

      if(gd.layers.earth.terr == NULL) {
        fprintf(stderr,"Cannot allocate space for terrain data\n");
        return -1;
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
        return -1;
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
  param_text = find_tag_text(_paramsBuf,"ROUTE_WINDS",
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
      return -1;
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
  param_text = find_tag_text(_paramsBuf,"GUI_CONFIG",
                             &param_text_len, &param_text_line_no);

  if(param_text == NULL || param_text_len <=0 ) {
  } else {
    if(gd.gui_P->loadFromBuf("GUI_CONFIG TDRP Section",
                             NULL,param_text,
                             param_text_len,
                             param_text_line_no,
                             TRUE,gd.debug2)  < 0) { 
      fprintf(stderr,"Please fix the <GUI_CONFIG> parameter section\n");
      return -1;
    }
  }

  // Instantiate the IMAGES Config TDRP 
  gd.images_P = new Cimages_P();

  // Load the IMAGES_CONFIG parameters
  param_text_line_no = 0;
  param_text_len = 0;
  param_text = find_tag_text(_paramsBuf,"IMAGE_GENERATION",
                             &param_text_len, &param_text_line_no);

  if(param_text == NULL || param_text_len <=0 ) {
  } else {
    if(gd.images_P->loadFromBuf("IMAGE_GENERATION TDRP Section",
                                NULL,param_text,
                                param_text_len,
                                param_text_line_no,
                                TRUE,gd.debug2)  < 0) { 
      fprintf(stderr,"Please fix the <IMAGE_GENERATION> parameter section\n");
      return -1;
    }
  }

  // Instantiate the Draw TDRP 
  gd.draw_P = new Cdraw_P();

  // Load the Draw_Export parameters
  param_text_line_no = 0;
  param_text_len = 0;
  param_text = find_tag_text(_paramsBuf,"DRAW_EXPORT",
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
      return -1;
    }

    /* Establish and initialize Draw-Export params */
    if (_initDrawExportLinks()) {
      return -1;
    }
  }
    
  if(gd.draw.num_draw_products == 0 && gd.menu_bar.set_draw_mode_bit >0) {
    fprintf(stderr,
	    "Fatal Error: DRAW Button Enabled, without any DRAW_EXPORT Products defined\n"); 
    fprintf(stderr,
	    "Either remove SET_DRAW_MODE button or define products in DRAW_EXPORT \n"); 
    fprintf(stderr,
	    "Section of the parameter file \n"); 
    return -1;
  }


  // Load the Map Overlay parameters
  param_text_line_no = 0;
  param_text_len = 0;
  param_text = find_tag_text(_paramsBuf,"MAPS",
                             &param_text_len, &param_text_line_no);

  if(param_text == NULL || param_text_len <=0 ) {
    fprintf(stderr,"Could'nt Find MAPS SECTION\n");
    return -1;
  }

  // overlays

  if (_initOverlays(param_text, param_text_len, param_text_line_no)) {
    return -1;
  }
  

  if(strlen(gd.station_loc_url) > 1) {
    if(gd.debug || gd.debug1) {
      fprintf(stderr,"Loading Station data from %s ...",gd.station_loc_url);
    }
    gd.station_loc =  new StationLoc();
    if(gd.station_loc == NULL) {
      fprintf(stderr,"CIDD: Fatal Alloc constructing new StationLoc()\n");
      return -1;
    }

    if(gd.station_loc->ReadData(gd.station_loc_url) < 0) {
      fprintf(stderr,"CIDD: Can't load Station Data from %s\n",gd.station_loc_url);
      return -1;
    }
    // gd.station_loc->PrintAll();  // DEBUG

    if(gd.debug || gd.debug1) {
      fprintf(stderr,"Done\n");
    }
  }

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
    return -1;
  }

  /* Setup default CONTOUR FIELDS */
  for(i = 1; i <= NUM_CONT_LAYERS; i++ ) {

    sprintf(str_buf,"cidd.contour%d_field",i);
    field_str = getString(str_buf, "NoMaTcH");

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
    field_str =  getString( str_buf, "NoMaTcH");

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
    f_name = getString(
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

  // gui

  gd.click_posn_rel_to_origin  = getBoolean("cidd.click_posn_rel_to_origin", 0);

  // timer control


  setPrintTdrp(false);

  return 0;

}


///////////////////////////////////////////////////////////////
// gets an entry from the param list
// returns NULL if this fails
///////////////////////////////////////////////////////////////

const char *LegacyParams::_get(const char *search_name) const

{
  
  for (size_t ii =  0; ii < _plist.size(); ii++) {
    if (_plist[ii].name == search_name) {
      return (_plist[ii].entry.c_str());
    }
  }

  return NULL;

}

///////////////////////////////////////////////////////////////
// returns the value of a double parameter
// If it cannot find the parameter, returns the default
///////////////////////////////////////////////////////////////

double LegacyParams::getDouble(const char *name, double default_val)
{

  if (_printTdrp) {
    cout << endl;
    cout << "paramdef double {" << endl;
    cout << "  p_default = " << default_val << ";" << endl;
    cout << "  p_descr = \"\"" << ";" << endl;
    cout << "  p_help = \"\"" << ";" << endl;
    cout << "} " << name << ";" << endl;
    cout << endl;
  }
  
  const char *entryStr = _get(name);
  if (entryStr == NULL) {
    return default_val;
  } else {
    double val;
    if (sscanf(entryStr, "%lg", &val) == 1) {
      return val;
    } else {
      return default_val;
    }
  }

  return default_val;

}

///////////////////////////////////////////////////////////////
// returns the value of a float parameter
// If it cannot find the parameter, returns the default
///////////////////////////////////////////////////////////////

float LegacyParams::getFloat(const char *name, float default_val)
{

  if (_printTdrp) {
    cout << endl;
    cout << "paramdef double {" << endl;
    cout << "  p_default = " << default_val << ";" << endl;
    cout << "  p_descr = \"\"" << ";" << endl;
    cout << "  p_help = \"\"" << ";" << endl;
    cout << "} " << name << ";" << endl;
    cout << endl;
  }
  
  const char *entryStr = _get(name);
  if (entryStr == NULL) {
    return default_val;
  } else {
    float val;
    if (sscanf(entryStr, "%g", &val) == 1) {
      return val;
    } else {
      return default_val;
    }
  }
  
  return default_val;

}

//////////////////////////////////////////////////////////////
// remove cidd. from start of parameter name

const char *LegacyParams::_removeCidd(const char *name) const

{

  if (strncmp(name, "cidd.", 5) == 0) {
    return (name + 5);
  } else {
    return name;
  }
  
}


///////////////////////////////////////////////////////////////
// returns the value of a int parameter
// If it cannot find the parameter, returns the default
///////////////////////////////////////////////////////////////

int LegacyParams::getInt(const char *name, int default_val)

{

  if (_printTdrp) {
    cout << endl;
    cout << "paramdef int {" << endl;
    cout << "  p_default = " << default_val << ";" << endl;
    cout << "  p_descr = \"\"" << ";" << endl;
    cout << "  p_help = \"\"" << ";" << endl;
    cout << "} " << _removeCidd(name) << ";" << endl;
    cout << endl;
  }
  
  const char *entryStr = _get(name);
  if (entryStr == NULL) {
    return default_val;
  } else {
    int val;
    if (sscanf(entryStr, "%d", &val) == 1) {
      return val;
    } else {
      return default_val;
    }
  }
  
  return default_val;

}

///////////////////////////////////////////////////////////////
// returns the value of a boolean parameter
// If it cannot find the parameter, returns the default
///////////////////////////////////////////////////////////////

bool LegacyParams::getBoolean(const char *name, int default_val)

{

  if (_printTdrp) {
    cout << endl;
    cout << "paramdef boolean {" << endl;
    if (default_val == 0) {
      cout << "  p_default = FALSE;" << endl;
    } else {
      cout << "  p_default = TRUE;" << endl;
    }
    cout << "  p_descr = \"\"" << ";" << endl;
    cout << "  p_help = \"\"" << ";" << endl;
    cout << "} " << _removeCidd(name) << ";" << endl;
    cout << endl;
  }
  
  const char *entryStr = _get(name);
  if (entryStr == NULL) {
    return default_val;
  } else {
    int val;
    if (sscanf(entryStr, "%d", &val) == 1) {
      return val;
    } else {
      return default_val;
    }
  }
  
  return default_val;

}

///////////////////////////////////////////////////////////////
// returns the value of a long parameter
// If it cannot find the parameter, returns the default
///////////////////////////////////////////////////////////////

long LegacyParams::getLong(const char *name, long default_val)

{

  if (_printTdrp) {
    cout << endl;
    cout << "paramdef int {" << endl;
    cout << "  p_default = " << default_val << ";" << endl;
    cout << "  p_descr = \"\"" << ";" << endl;
    cout << "  p_help = \"\"" << ";" << endl;
    cout << "} " << _removeCidd(name) << ";" << endl;
    cout << endl;
  }
  
  const char *entryStr = _get(name);
  if (entryStr == NULL) {
    return default_val;
  } else {
    long val;
    if (sscanf(entryStr, "%ld", &val) == 1) {
      return val;
    } else {
      return default_val;
    }
  }
  
  return default_val;

}

///////////////////////////////////////////////////////////////
// returns the value of a string parameter
// If it cannot find the parameter, returns the default
///////////////////////////////////////////////////////////////

const char *LegacyParams::getString(const char *name, const char *default_val)

{

  if (_printTdrp) {
    cout << endl;
    cout << "paramdef string {" << endl;
    cout << "  p_default = \"" << default_val << "\";" << endl;
    cout << "  p_descr = \"\"" << ";" << endl;
    cout << "  p_help = \"\"" << ";" << endl;
    cout << "} " << _removeCidd(name) << ";" << endl;
    cout << endl;
  }
  
  const char *entryStr = _get(name);
  if (entryStr == NULL) {
    return default_val;
  } else {
    return (char *) entryStr;
  }
}

/////////////////////////////////////////////////////////////////////////////
// FIND_TAG_TEXT Search a null terminated string for the text between tags
//
// Searches through input_buf for text between tags of the form <TAG>...Text...</TAG>
// Returns a pointer to the beginning of the text and its length if found.
// text_line_no is used on input to begin counting and is set on output to the starting
// line number of the tagged text

#define TAG_BUF_LEN 256
const char *LegacyParams::_findTagText(const char *input_buf,
                                       const char * tag,
                                       long *text_len,
                                       long *text_line_no)
{
  int start_line_no;
  const char *start_ptr;
  const char *end_ptr;
  const char *ptr;
  char end_tag[TAG_BUF_LEN];
  char start_tag[TAG_BUF_LEN];
  
  // Reasonable tag check - Give up
  if(strlen(tag) > TAG_BUF_LEN - 5) {
    fprintf(stderr,"Unreasonable tag: %s - TOO long!\n",tag);
    *text_len = 0;
    return NULL;
  }
  
  // Clear the string buffers
  memset(start_tag,0,TAG_BUF_LEN);
  memset(end_tag,0,TAG_BUF_LEN);
  
  start_line_no = *text_line_no;
  
  sprintf(start_tag,"<%s>",tag);
  sprintf(end_tag,"</%s>",tag);
  
  // Search for Start tag
  if((start_ptr = strstr(input_buf,start_tag)) == NULL) {
    *text_len = 0;
    *text_line_no = start_line_no;
    return NULL;
  }
  start_ptr +=  strlen(start_tag); // Skip ofer tag to get to the text
  
  // Search for end tag after the start tag
  if((end_ptr = strstr(start_ptr,end_tag)) == NULL) {
    *text_len = 0;
    *text_line_no = start_line_no;
    return NULL;
  }
  end_ptr--;  // Skip back one character to last text character
  
  // Compute the length of the text_tag
  *text_len = (long) (end_ptr - start_ptr);
  
  // Count the lines before the starting tag
  ptr = input_buf;
  while(((ptr = strchr(ptr,'\n')) != NULL) && (ptr < start_ptr)) {
    ptr++; // Move past the found NL
    start_line_no++;
  }
  
  *text_line_no = start_line_no;
  return start_ptr;
}

/////////////////////////////////////////////////////////////////////////////
// LOAD_DB_DATA_DEFAULT : Allocate and load the parameter data base using the
//                        default parameter settings and set Global Struct
//                        members
// 

int LegacyParams::_loadDbDataDefault(char* &db_buf, int &db_len)
{

  // Generate the full params string.  The non-TDRP portions are kept in
  // static strings while the TDRP portions are loaded from the default
  // parameters

  gd.gui_P = new Cgui_P;
  gd.syprod_P = new Csyprod_P;
  gd.draw_P = new Cdraw_P;
  gd.images_P = new Cimages_P;
  gd.layers.earth._P = new Cterrain_P;
  gd.layers.route_wind._P = new Croutes_P;
  
  string params_text = ParamsTextMasterHeader;
  params_text += get_default_tdrp_params("GUI_CONFIG", gd.gui_P);
  params_text += ParamsTextGrids;
  params_text += ParamsTextWinds;
  params_text += ParamsTextMaps;
  params_text += ParamsTextMainParams;
  params_text += get_default_tdrp_params("DRAW_EXPORT", gd.draw_P);
  params_text += get_default_tdrp_params("IMAGE_GENERATION", gd.images_P);
  params_text += get_default_tdrp_params("SYMPRODS", gd.syprod_P);
  params_text += get_default_tdrp_params("TERRAIN",
					 gd.layers.earth._P);
  params_text += get_default_tdrp_params("ROUTE_WINDS",
					 gd.layers.route_wind._P);
  

  // Allocate space for the buffer copy
  
  db_len = params_text.size() + 1;
  
  if ((db_buf = (char *)calloc(db_len, 1)) == NULL) {
    fprintf(stderr,"Problems allocating %ld bytes for parameter file\n",
	    (long)db_len);
    return -1;
  }

  // Copy the parameters into the buffer

  memcpy(db_buf, params_text.c_str(), db_len - 1);
  db_buf[db_len-1] = '\0';

  return 0;
  
}


/////////////////////////////////////////////////////////////////////////////
// LOAD_DB_DATA_FILE : Allocate and load the parameter data base from a file
//                     and set Global Struct members
// 

int LegacyParams::_loadDbDataFile(const string &fname,
                                  char* &db_buf,
                                  int &db_len)
{

  FILE *infile;
  
  // create temp buffer
  
  int tmpLen = 1000000;
  char *tmpBuf = new char[tmpLen];
  
  // Open DB file
  
  if((infile = fopen(fname.c_str(),"r")) == NULL) {
    perror(fname.c_str());
    fprintf(stderr,"Problems Opening %s\n",fname.c_str());
    return -1;
  }
  
  // Read into tmp buf
  db_len = fread(tmpBuf,1,tmpLen,infile);
  if(db_len <= 0) {
    perror(fname.c_str());
    fprintf(stderr,"Problems Reading %s\n",fname.c_str());
    return -1;
  }
  
  // Allocate space for the whole file plus a null
  if((db_buf = (char *)  calloc(db_len + 1, 1)) == NULL) {
    fprintf(stderr,"Problems allocating %d bytes for parameter file\n",
            db_len);
    return -1;
  }
  
  // copy in
  
  memcpy(db_buf, tmpBuf, db_len);
  db_buf[db_len] = '\0'; // Make sure to null terminate
  delete[] tmpBuf;
  
  // Close DB file
  if(fclose(infile) != 0 )  {
    fprintf(stderr,"Problems Closing %s\n",fname.c_str());
    return -1;
  }

  return 0;
  
}


/////////////////////////////////////////////////////////////////////////////
// LOAD_DB_DATA_HTTP : Allocate and load the parameter data base from a Web
//                     server and set Global Struct members
// 

int LegacyParams::_loadDbDataHttp(const string &fname,
                                  char* &db_buf,
                                  int &db_len)
{

  int ret_stat;
  
  // Allow 5 seconds to retrieve the data 
  
  if(strlen(gd.http_proxy_url)  > URL_MIN_SIZE) {
    ret_stat = HTTPgetURL_via_proxy(gd.http_proxy_url, fname.c_str(), 5000,
				    &db_buf, &db_len);
  } else {
    ret_stat = HTTPgetURL(fname.c_str(), 5000, &db_buf, &db_len);
  }
  
  if(ret_stat <= 0 || db_len <= 0) {
    fprintf(stderr,"Could'nt Load Parameter Database from URL: %s,  %d\n",
	    fname.c_str(), ret_stat);
    if(ret_stat < 0) {
      fprintf(stderr,"Failed to successfully trasnact with the http server\n");
    } else {
      fprintf(stderr,
	      "HTTP server couldn't retreive the file - Returned  Stat: %d\n",
	      ret_stat);
    }
    fprintf(stderr,
	    "Make sure URL looks like: http://host.domain/dir/filename\n");
    fprintf(stderr,
	    "The most common problem is usually missing  the :// part \n");
    fprintf(stderr,"or a misspelled/incorrect host, directory or filename\n");
    if(strlen(gd.http_proxy_url)  > URL_MIN_SIZE)
      fprintf(stderr,"Also Check Proxy URL:%s\n",gd.http_proxy_url);
    return -1;
  }

  return 0;
  
}


/////////////////////////////////////////////////////////////////////////////
// LOAD_DB_DATA : Allocate and load the data base file - Set Global
// Struct members
// 

int LegacyParams::_loadDbData(const string &fname)
{
  
  char *db_buf = NULL;
  int db_len = 0;
  int iret = 0;
  
  if (fname == "") {
    // Default parameters
    iret = _loadDbDataDefault(db_buf, db_len);
  } else if(strncasecmp(fname.c_str(), "http:", 5) == 0) {
    // HTTP Based retrieve 
    iret = _loadDbDataHttp(fname, db_buf, db_len);
  } else {
    // FILE based retrieve
    iret = _loadDbDataFile(fname, db_buf, db_len);
  }

  if (iret == 0) {
    _paramsBuf = db_buf;
    _paramsBufLen = db_len;
    return 0;
  } else {
    return -1;
  }
  
}

/////////////////////////////////////////////////////////////////////////////
// initialize data field structs

#define NUM_PARSE_FIELDS    32
#define PARSE_FIELD_SIZE    1024
#define INPUT_LINE_LEN      2048

int LegacyParams::_initDataFields(const char *param_buf,
                                  long param_buf_len,
                                  long line_no)
{
  int  i,j;
  int  len,total_len;
  const char *start_ptr;
  const char *end_ptr;
  char *cfield[NUM_PARSE_FIELDS];

  gd.num_datafields = 0;
  total_len = 0;
  start_ptr = param_buf;

  cerr << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << endl;
  cerr << "fields_n: " << gParams.fields_n << endl;
  for (int ii = 0; ii < gParams.fields_n; ii++) {
    Params::field_t &fld = gParams._fields[ii];
    cerr << "  button label: " << fld.button_label << endl;
    cerr << "  legend label: " << fld.legend_label << endl;
    cerr << "  contour_low: " << fld.contour_low << endl;
  }
  cerr << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << endl;
    

  // read all the lines in the data information buffer
  while((end_ptr = strchr(start_ptr,'\n')) != NULL && (total_len < param_buf_len)) {
    // Skip over blank, short or commented lines
    len = (end_ptr - start_ptr)+1;
    if( (len < 20)  || (*start_ptr == '#')) {
      total_len += len  +1;
      start_ptr = end_ptr +1; // Skip past the newline
      line_no++;
      continue;
    }

    if(gd.num_datafields < MAX_DATA_FIELDS -1) {
      // Ask for 128 extra bytes for the null and potential env var  expansion
      gd.data_info[gd.num_datafields] = (char *)  calloc(len+128, 1);
      STRcopy(gd.data_info[gd.num_datafields],start_ptr,len);

      /* Do Environment variable substitution */
      usubstitute_env(gd.data_info[gd.num_datafields], len+128);
      gd.num_datafields++;
    } else {
      fprintf(stderr,
              "Cidd: Warning. Too many Data Fields. Data field not processed\n");
      fprintf(stderr,"Line %ld \n",line_no);
	 
    }

    total_len += len + 1;   // Count characters processed 
    start_ptr = end_ptr +1; // Skip past the newline
    line_no++;
  }

  if(gd.num_datafields <=0) {
    fprintf(stderr,"CIDD requires at least one valid gridded data field to be defined\n");
    return -1;
  }

  /* get temp space for substrings */
  for(i = 0; i < NUM_PARSE_FIELDS; i++) {
    cfield[i] = (char *)  calloc(PARSE_FIELD_SIZE, 1);
  }

  /* scan through each of the data information lines */
  for(i = 0; i < gd.num_datafields; i++) {

    /* get space for data info */
    gd.mrec[i] =  (met_record_t *) calloc(sizeof(met_record_t), 1);

    /* separate into substrings */
    STRparse(gd.data_info[i], cfield, INPUT_LINE_LEN, NUM_PARSE_FIELDS, PARSE_FIELD_SIZE);
    STRcopy(gd.mrec[i]->legend_name,cfield[0],NAME_LENGTH);
    STRcopy(gd.mrec[i]->button_name,cfield[1],NAME_LENGTH);

    if(gd.html_mode == 0) {
      /* Replace Underscores with spaces in names */
      for(j=strlen(gd.mrec[i]->button_name)-1 ; j >= 0; j--) {
        if(gd.replace_underscores && gd.mrec[i]->button_name[j] == '_') gd.mrec[i]->button_name[j] = ' ';
        if(gd.replace_underscores && gd.mrec[i]->legend_name[j] == '_') gd.mrec[i]->legend_name[j] = ' ';
      }
    }
    STRcopy(gd.mrec[i]->url,cfield[2],URL_LENGTH);

    STRcopy(gd.mrec[i]->color_file,cfield[3],NAME_LENGTH);

    // if units are "" or --, set to zero-length string
    if (!strcmp(cfield[4], "\"\"") || !strcmp(cfield[4], "--")) {
      STRcopy(gd.mrec[i]->field_units,"",LABEL_LENGTH);
    } else {
      STRcopy(gd.mrec[i]->field_units,cfield[4],LABEL_LENGTH);
    }

    gd.mrec[i]->cont_low = atof(cfield[5]);
    gd.mrec[i]->cont_high = atof(cfield[6]);
    gd.mrec[i]->cont_interv = atof(cfield[7]);

    gd.mrec[i]->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval;

    if (strncasecmp(cfield[8],"rad",3) == 0) {
      gd.mrec[i]->render_method = POLYGONS;
    } else {
      gd.mrec[i]->render_method = POLYGONS;
    }

    if (strncasecmp(cfield[8],"cont",4) == 0) {
      gd.mrec[i]->render_method = FILLED_CONTOURS;
    }

    if (strncasecmp(cfield[8],"lcont",4) == 0) {
      gd.mrec[i]->render_method = LINE_CONTOURS;
    }

    if (strncasecmp(cfield[8],"dcont",4) == 0) {
      gd.mrec[i]->render_method = DYNAMIC_CONTOURS;
    }

    if (strstr(cfield[8],"comp") != NULL) {
      gd.mrec[i]->composite_mode = TRUE;
    }

    if (strstr(cfield[8],"autoscale") != NULL) {
      gd.mrec[i]->auto_scale = TRUE;
    }

    gd.mrec[i]->currently_displayed = atoi(cfield[9]);

    if(gd.run_once_and_exit) {
      gd.mrec[i]->auto_render = 1;
    } else {
      gd.mrec[i]->auto_render = atoi(cfield[10]);
    }

    gd.mrec[i]->last_elev = (char *)NULL;
    gd.mrec[i]->elev_size = 0;

    gd.mrec[i]->plane = 0;
    gd.mrec[i]->h_data_valid = 0;
    gd.mrec[i]->v_data_valid = 0;
    gd.mrec[i]->h_last_scale  = -1.0;
    gd.mrec[i]->h_last_bias  = -1.0;
    gd.mrec[i]->h_last_missing  = -1.0;
    gd.mrec[i]->h_last_bad  = -1.0;
    gd.mrec[i]->h_last_transform  = -1;
    gd.mrec[i]->v_last_scale  = -1.0;
    gd.mrec[i]->v_last_bias  = -1.0;
    gd.mrec[i]->v_last_missing  = -1.0;
    gd.mrec[i]->v_last_bad  = -1.0;
    gd.mrec[i]->v_last_transform  = -1;
    gd.mrec[i]->h_fhdr.proj_origin_lat  = 0.0;
    gd.mrec[i]->h_fhdr.proj_origin_lon  = 0.0;
    gd.mrec[i]->time_list.num_alloc_entries = 0;
    gd.mrec[i]->time_list.num_entries = 0;

    STRcopy(gd.mrec[i]->units_label_cols,"KM",LABEL_LENGTH);
    STRcopy(gd.mrec[i]->units_label_rows,"KM",LABEL_LENGTH);
    STRcopy(gd.mrec[i]->units_label_sects,"KM",LABEL_LENGTH);

    // instantiate classes
    gd.mrec[i]->h_mdvx = new DsMdvxThreaded;
    gd.mrec[i]->v_mdvx = new DsMdvxThreaded;
    gd.mrec[i]->h_mdvx_int16 = new MdvxField;
    gd.mrec[i]->v_mdvx_int16 = new MdvxField;
    gd.mrec[i]->proj = new MdvxProj;

  }
  /* Make sure the first field is always on */
  gd.mrec[0]->currently_displayed = 1;
    
  /* free up temp storage for substrings */
  for(i = 0; i < NUM_PARSE_FIELDS; i++) {
    free(cfield[i]);
  }

  // set the tdrp params for the fields

  cerr << "aaaaaaaaaaaaaaaaaaaaa num_datafields: " << gd.num_datafields << endl;
  cerr << "bbbbbbbbbbbbbbbbbbbbbb fields_n: " << gParams.fields_n << endl;
    
  gParams.arrayRealloc("fields", gd.num_datafields);

  cerr << "cccccccccccccccccccccc fields_n: " << gParams.fields_n << endl;
  cerr << "dddddddddddddddddddddd fields ptr: " << gParams._fields << endl;
  for(i = 0; i < gd.num_datafields; i++) {

    met_record_t &record = *(gd.mrec[i]);
    Params::field_t *field = gParams._fields + i;

    cerr << "111111111111 i, button_name, legend_name: " << record.button_name << ", " << record.legend_name << endl;

    TDRP_str_replace(&field->button_label, record.button_name);
    TDRP_str_replace(&field->legend_label, record.legend_name);
    TDRP_str_replace(&field->url, record.url);
    TDRP_str_replace(&field->field_name, record.field_label);
    TDRP_str_replace(&field->color_map, record.color_file);
    TDRP_str_replace(&field->units, record.field_units);
    field->contour_low = record.cont_low;
    field->contour_low = -9999;
    field->contour_high = record.cont_high;
    field->contour_interval = record.cont_interv;
    field->render_mode = (Params::render_mode_t) record.render_method;
    field->display_in_menu = (tdrp_bool_t) record.currently_displayed;
    field->background_render = (tdrp_bool_t) record.auto_render;

  }

  cerr << "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy" << endl;
  for (int ii = 0; ii < gParams.fields_n; ii++) {
    Params::field_t &fld = gParams._fields[ii];
    cerr << "  button label: " << fld.button_label << endl;
    cerr << "  legend label: " << fld.legend_label << endl;
    cerr << "  contour_low: " << fld.contour_low << endl;
  }
  cerr << "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy" << endl;

  return 0;

}
/************************************************************************
 * INIT_WIND_DATA_LINKS:  Scan cidd_wind_data.info file and setup link to
 *         data source for the wind fields.
 *
 */

int LegacyParams::_initWindFields(const char *param_buf,
                                  long param_buf_len,
                                  long line_no)
{
  int    i;
  int    len,total_len;
  int    num_sets;    /* number of sets of wind data */
  int    num_fields;
  const char   *start_ptr;
  const char   *end_ptr;
  char   *cfield[NUM_PARSE_FIELDS];
  
  // initialize pointers to NULL
  
  for(i = 0; i < NUM_PARSE_FIELDS; i++) {
    cfield[i] = NULL;
  }
  
  /* PASS 1 - Count the wind set lines */
  num_sets = 0;
  total_len = 0;
  start_ptr = param_buf;
  while((end_ptr = strchr(start_ptr,'\n')) != NULL && 
        (total_len < param_buf_len)) {
    // Skip over blank, short or commented lines
    len = (end_ptr - start_ptr)+1;
    if( len > 15  && *start_ptr != '#') {
      num_sets++;
    }
    start_ptr = end_ptr +1; // Skip past the newline
    total_len += len  +1;
  }

   
  int default_marker_type = ARROWS;  
  if(num_sets > 0) {  /* ALLOCATE Space */
    /* get temp space for substrings */
    for(i = 0; i < NUM_PARSE_FIELDS; i++) {
      cfield[i] =(char *)  calloc(PARSE_FIELD_SIZE, 1);
    }
    
    if((gd.layers.wind =(wind_data_t*) calloc(num_sets,sizeof(wind_data_t))) == NULL) {
      fprintf(stderr,"Unable to allocate space for %d wind sets\n",num_sets);
      perror("Cidd");
      return -1;
    }
    
    // Set up global barb preferences
    const char *type_ptr = gd.wind_marker_type;
    
    if(strncasecmp(type_ptr, "tuft", 4) == 0)  default_marker_type = TUFT;
    if(strncasecmp(type_ptr, "barb", 4) == 0)  default_marker_type = BARB;
    if(strncasecmp(type_ptr, "vector", 6) == 0)  default_marker_type = VECTOR;
    if(strncasecmp(type_ptr, "tickvector", 10) == 0)  default_marker_type = TICKVECTOR; 
    if(strncasecmp(type_ptr, "labeledbarb", 11) == 0)  default_marker_type = LABELEDBARB;
    if(strncasecmp(type_ptr, "metbarb", 7) == 0)  default_marker_type = METBARB;
    if(strncasecmp(type_ptr, "barb_sh", 7) == 0)  default_marker_type = BARB_SH;
    if(strncasecmp(type_ptr, "labeledbarb_sh", 14) == 0)  default_marker_type = LABELEDBARB_SH;
    
    /* PASS 2 - fill in the params in the wind sets */
    num_sets = 0;
    
    total_len = 0;
    start_ptr = param_buf;
    while((end_ptr = strchr(start_ptr,'\n')) != NULL && 
          (total_len < param_buf_len)) {
      len = (end_ptr - start_ptr)+1; 
      // Skip over blank, short or commented lines
      if( len > 15  && *start_ptr != '#') {
        num_fields = STRparse(start_ptr, cfield, len, NUM_PARSE_FIELDS, PARSE_FIELD_SIZE); 
        if( *start_ptr != '#' && num_fields >= 7) {
          
          // Ask for 128 extra bytes for the null and potential env var  expansion
          gd.layers.wind[num_sets].data_info = (char *) calloc(len+128, 1);
          if(gd.layers.wind[num_sets].data_info == NULL) {
            fprintf(stderr,"Unable to allocate %d bytes for wind info\n",len+128);
            perror("Cidd");
            return -1;
          }
          STRcopy(gd.layers.wind[num_sets].data_info,start_ptr,len);
          
          /* DO Environment variable substitution */
          usubstitute_env(gd.layers.wind[num_sets].data_info, len+128);
          num_sets++;
        }
      }
      start_ptr = end_ptr +1; // Skip past the newline
      total_len += len  +1;
    }
  }
  
  gd.layers.num_wind_sets = num_sets;
  
  for(i=0; i < gd.layers.num_wind_sets; i++) {
    num_fields = STRparse(gd.layers.wind[i].data_info, cfield,
                          INPUT_LINE_LEN,
                          NUM_PARSE_FIELDS, PARSE_FIELD_SIZE);
    if(num_fields < 7) {
      fprintf(stderr,
              "Error in wind field line. Wrong number of parameters,  -Line: \n %s"
              ,gd.layers.wind[i].data_info);
    }
    
    /* Allocate Space for U record and initialize */
    gd.layers.wind[i].wind_u = (met_record_t *)
      calloc(sizeof(met_record_t), 1);
    gd.layers.wind[i].wind_u->h_data_valid = 0;
    gd.layers.wind[i].wind_u->v_data_valid = 0;
    gd.layers.wind[i].wind_u->h_vcm.nentries = 0;
    gd.layers.wind[i].wind_u->v_vcm.nentries = 0;
    gd.layers.wind[i].wind_u->h_fhdr.scale = -1.0;
    gd.layers.wind[i].wind_u->h_last_scale = 0.0;
    gd.layers.wind[i].wind_u->time_list.num_alloc_entries = 0;
    gd.layers.wind[i].wind_u->time_list.num_entries = 0;
    STRcopy(gd.layers.wind[i].wind_u->legend_name,cfield[0],NAME_LENGTH);
    STRcopy(gd.layers.wind[i].wind_u->button_name,cfield[0],NAME_LENGTH);
    STRcopy(gd.layers.wind[i].wind_u->url,cfield[1],URL_LENGTH);
    
    if(gd.html_mode == 0) { /* Replace Underscores with spaces in names */
      for(int j=strlen(gd.layers.wind[i].wind_u->button_name)-1 ; j >= 0; j--) {
        if(gd.replace_underscores && gd.layers.wind[i].wind_u->button_name[j] == '_') gd.layers.wind[i].wind_u->button_name[j] = ' ';
        if(gd.replace_underscores && gd.layers.wind[i].wind_u->legend_name[j] == '_') gd.layers.wind[i].wind_u->legend_name[j] = ' ';
      }
    }
    
    // Append the field name
    strcat(gd.layers.wind[i].wind_u->url,cfield[2]);
    
    STRcopy(gd.layers.wind[i].wind_u->field_units,cfield[5],LABEL_LENGTH);
    gd.layers.wind[i].wind_u->currently_displayed = atoi(cfield[6]);
    gd.layers.wind[i].active = (atoi(cfield[6]) > 0)? 1: 0;
    gd.layers.wind[i].line_width = abs(atoi(cfield[6]));
    // Sanity check
    if(gd.layers.wind[i].line_width == 0 ||
       gd.layers.wind[i].line_width > 10) gd.layers.wind[i].line_width = 1;
    
    // Pick out Optional Marker type fields
    gd.layers.wind[i].marker_type = default_marker_type;
    if(strstr(cfield[6], ",tuft") != NULL)  gd.layers.wind[i].marker_type = TUFT;
    if(strstr(cfield[6], ",barb") != NULL)  gd.layers.wind[i].marker_type = BARB;
    if(strstr(cfield[6], ",vector") != NULL)  gd.layers.wind[i].marker_type = VECTOR;
    if(strstr(cfield[6], ",tickvector") != NULL)  gd.layers.wind[i].marker_type = TICKVECTOR; 
    if(strstr(cfield[6], ",labeledbarb") != NULL)  gd.layers.wind[i].marker_type = LABELEDBARB;
    if(strstr(cfield[6], ",metbarb") != NULL)  gd.layers.wind[i].marker_type = METBARB;
    if(strstr(cfield[6], ",barb_sh") != NULL)  gd.layers.wind[i].marker_type = BARB_SH;
    if(strstr(cfield[6], ",labeledbarb_sh") != NULL)  gd.layers.wind[i].marker_type = LABELEDBARB_SH;
    
    
    // Pick out Optional Color
    if(num_fields > 7) {
      STRcopy(gd.layers.wind[i].color_name, cfield[7], NAME_LENGTH);
    } else {
      STRcopy(gd.layers.wind[i].color_name, "white", NAME_LENGTH);
    }
    gd.layers.wind[i].wind_u->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval;
    gd.layers.wind[i].wind_u->h_fhdr.proj_origin_lon = 0.0;
    gd.layers.wind[i].wind_u->h_fhdr.proj_origin_lat = 0.0;
    
    // instantiate classes
    gd.layers.wind[i].wind_u->h_mdvx = new DsMdvxThreaded;
    gd.layers.wind[i].wind_u->v_mdvx = new DsMdvxThreaded;
    gd.layers.wind[i].wind_u->h_mdvx_int16 = new MdvxField;
    gd.layers.wind[i].wind_u->v_mdvx_int16 = new MdvxField;
    gd.layers.wind[i].wind_u->proj = new MdvxProj;
    
    /* Allocate Space for V record and initialize */
    gd.layers.wind[i].wind_v = (met_record_t *)
      calloc(sizeof(met_record_t), 1);
    gd.layers.wind[i].wind_v->h_data_valid = 0;
    gd.layers.wind[i].wind_v->v_data_valid = 0;
    gd.layers.wind[i].wind_v->h_vcm.nentries = 0;
    gd.layers.wind[i].wind_v->v_vcm.nentries = 0;
    gd.layers.wind[i].wind_v->h_fhdr.scale = -1.0;
    gd.layers.wind[i].wind_v->h_last_scale = 0.0;
    gd.layers.wind[i].wind_v->time_list.num_alloc_entries = 0;
    gd.layers.wind[i].wind_v->time_list.num_entries = 0;
    STRcopy(gd.layers.wind[i].wind_v->legend_name,cfield[0],NAME_LENGTH);
    STRcopy(gd.layers.wind[i].wind_v->button_name,cfield[0],NAME_LENGTH);
    STRcopy(gd.layers.wind[i].wind_v->url,cfield[1],URL_LENGTH);
    
    
    if(gd.html_mode == 0) { /* Replace Underscores with spaces in names */
      for(int j=strlen(gd.layers.wind[i].wind_v->button_name)-1 ; j >= 0; j--) {
        if(gd.replace_underscores && gd.layers.wind[i].wind_v->button_name[j] == '_') gd.layers.wind[i].wind_v->button_name[j] = ' ';
        if(gd.replace_underscores && gd.layers.wind[i].wind_v->legend_name[j] == '_') gd.layers.wind[i].wind_v->legend_name[j] = ' ';
      }
    }
    // Append the field name
    strcat(gd.layers.wind[i].wind_v->url,cfield[3]);
    
    STRcopy(gd.layers.wind[i].wind_v->field_units,cfield[5],LABEL_LENGTH);
    gd.layers.wind[i].wind_v->currently_displayed = atoi(cfield[6]);
    gd.layers.wind[i].wind_v->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval;
    gd.layers.wind[i].wind_v->h_fhdr.proj_origin_lon = 0.0;
    gd.layers.wind[i].wind_v->h_fhdr.proj_origin_lat = 0.0;
    
    // instantiate classes
    gd.layers.wind[i].wind_v->h_mdvx = new DsMdvxThreaded();
    gd.layers.wind[i].wind_v->v_mdvx = new DsMdvxThreaded();
    gd.layers.wind[i].wind_v->h_mdvx_int16 = new MdvxField;
    gd.layers.wind[i].wind_v->v_mdvx_int16 = new MdvxField;
    gd.layers.wind[i].wind_v->proj = new MdvxProj;
    
    /* Allocate Space for W  record (If necessary)  and initialize */
    if(strncasecmp(cfield[4],"None",4) != 0) {
      gd.layers.wind[i].wind_w = (met_record_t *) calloc(sizeof(met_record_t), 1);
      gd.layers.wind[i].wind_w->h_data_valid = 0;
      gd.layers.wind[i].wind_w->v_data_valid = 0;
      gd.layers.wind[i].wind_w->v_vcm.nentries = 0;
      gd.layers.wind[i].wind_w->h_vcm.nentries = 0;
      gd.layers.wind[i].wind_w->h_fhdr.scale = -1.0;
      gd.layers.wind[i].wind_w->h_last_scale = 0.0;
      gd.layers.wind[i].wind_w->time_list.num_alloc_entries = 0;
      gd.layers.wind[i].wind_w->time_list.num_entries = 0;
      
      STRcopy(gd.layers.wind[i].wind_w->legend_name,cfield[0],NAME_LENGTH);
      sprintf(gd.layers.wind[i].wind_w->button_name,"%s_W ",cfield[0]);
      STRcopy(gd.layers.wind[i].wind_w->url,cfield[1],URL_LENGTH);
      
      if(gd.html_mode == 0) { /* Replace Underscores with spaces in names */
        for(int j=strlen(gd.layers.wind[i].wind_w->button_name)-1 ; j >= 0; j--) {
          if(gd.replace_underscores && gd.layers.wind[i].wind_w->button_name[j] == '_') gd.layers.wind[i].wind_w->button_name[j] = ' ';
          if(gd.layers.wind[i].wind_w->legend_name[j] == '_') gd.layers.wind[i].wind_w->legend_name[j] = ' ';
        }
      }
      
      // Append the field name
      strcat(gd.layers.wind[i].wind_w->url,cfield[4]);
      
      STRcopy(gd.layers.wind[i].wind_w->field_units,cfield[5],LABEL_LENGTH);
      gd.layers.wind[i].wind_w->currently_displayed = atoi(cfield[6]);
      gd.layers.wind[i].wind_w->time_allowance = gd.movie.mr_stretch_factor * gd.movie.time_interval;
      gd.layers.wind[i].wind_w->h_fhdr.proj_origin_lon = 0.0;
      gd.layers.wind[i].wind_w->h_fhdr.proj_origin_lat = 0.0;
      
      // instantiate classes
      gd.layers.wind[i].wind_w->h_mdvx = new DsMdvxThreaded();
      gd.layers.wind[i].wind_w->v_mdvx = new DsMdvxThreaded();
      gd.layers.wind[i].wind_w->h_mdvx_int16 = new MdvxField;
      gd.layers.wind[i].wind_w->v_mdvx_int16 = new MdvxField;
      gd.layers.wind[i].wind_w->proj = new MdvxProj;
    } else {
      gd.layers.wind[i].wind_w =  (met_record_t *) NULL;
    }
    
    gd.layers.wind[i].units_scale_factor = gd.wind_units_scale_factor;
    gd.layers.wind[i].reference_speed = gd.wind_reference_speed;
    gd.layers.wind[i].units_label = gd.wind_units_label;
    
  }
  
  /* free temp space */
  for(i = 0; i < NUM_PARSE_FIELDS; i++) {
    if (cfield[i] != NULL) {
      free(cfield[i]);
    }
  }

  return 0;
  
}
/************************************************************************
 * INIT_DRAW_EXPORT_LINKS:  Scan param file and setup links to
 *  for drawn and exported points 
 *
 */

int LegacyParams::_initDrawExportLinks()
{
  int  i,len;
  
  gd.draw.num_draw_products = gd.draw_P->dexport_info_n;
  
  if((gd.draw.dexport =(draw_export_info_t*)
      calloc(gd.draw.num_draw_products,sizeof(draw_export_info_t))) == NULL) {
    fprintf(stderr,"Unable to allocate space for %d draw.dexport sets\n",gd.draw.num_draw_products);
    perror("CIDD");
    return -1;
  }

  
  // Allocate space for control struct and record starting values for each product.
  for(i=0; i < gd.draw.num_draw_products;  i++) {
    
    // Product ID  
    len = strlen(gd.draw_P->_dexport_info[i].id_label) +1;
    gd.draw.dexport[i].product_id_label = (char *)  calloc(1,len);
    STRcopy(gd.draw.dexport[i].product_id_label,gd.draw_P->_dexport_info[i].id_label,len);
    
    // Allocate space for product_label_text (annotations) and set to nulls
    gd.draw.dexport[i].product_label_text =  (char *) calloc(1,TITLE_LENGTH);
    
    strncpy(gd.draw.dexport[i].product_label_text, gd.draw_P->_dexport_info[i].default_label,TITLE_LENGTH-1);
    
    // FMQ URL 
    len = strlen(gd.draw_P->_dexport_info[i].url) +1;
    if(len > NAME_LENGTH) {
      fprintf(stderr,"URL: %s too long - Must be less than %d chars. Sorry.",
              gd.draw_P->_dexport_info[i].url,URL_LENGTH);
      return -1;
    }
    gd.draw.dexport[i].product_fmq_url =  (char *) calloc(1,URL_LENGTH);
    STRcopy(gd.draw.dexport[i].product_fmq_url,gd.draw_P->_dexport_info[i].url,URL_LENGTH);
    
    // Get the Default valid time  
    gd.draw.dexport[i].default_valid_minutes = gd.draw_P->_dexport_info[i].valid_minutes;
    
    // Get the Default ID   
    gd.draw.dexport[i].default_serial_no = gd.draw_P->_dexport_info[i].default_id_no;
  }

  return 0;
  
}
/************************************************************************
 * LOAD_OVERLAY_INFO:  Scan info file and record info about overlay
 *         sources. Returns number of overlay info lines found &
 *         filled;
 *
 */

int LegacyParams::_loadOverlayInfo(const char *param_buf, long param_buf_len,
                                   long line_no,
                                   int  max_overlays)
{
  int i,num_overlays;
  int  len,total_len;
  int num_fields;
  const char *start_ptr;
  const char *end_ptr;
  char    *cfield[32];
   
  char full_line[BUFSIZ];
    
  for(i=0; i < 32; i++)  cfield[i] = (char *) calloc(1,64);  /* get space for sub strings */

  /* read all the lines in the information file */
  num_overlays = 0;
  total_len = 0;
  start_ptr = param_buf;
  while((end_ptr = strchr(start_ptr,'\n')) != NULL &&
        (total_len < param_buf_len) && 
        (num_overlays < max_overlays)) {
    len = (end_ptr - start_ptr)+1;
    // Skip over blank, short or commented lines
    if((len > 20)  && *start_ptr != '#') {
      
      STRcopy(full_line, start_ptr, len);
      usubstitute_env(full_line, BUFSIZ);
      
      // over[num_overlays] = (Overlay_t *) calloc(1,sizeof(Overlay_t));
      
      num_fields = STRparse(full_line,cfield,BUFSIZ,32,64);  /* separate into substrings */
      
      if(num_fields >= 7) {    /* Is a correctly formatted line */
        // STRcopy(over[num_overlays]->map_code,cfield[0],LABEL_LENGTH);
        // STRcopy(over[num_overlays]->control_label,cfield[1],LABEL_LENGTH);
        // STRcopy(over[num_overlays]->map_file_name,cfield[2],NAME_LENGTH);
        // over[num_overlays]->default_on_state = atoi(cfield[3]);
        // over[num_overlays]->line_width = atoi(cfield[3]);
        // if(over[num_overlays]->line_width <=0) 
        //   over[num_overlays]->line_width = 1;
        // over[num_overlays]->detail_thresh_min = atof(cfield[4]);
        // over[num_overlays]->detail_thresh_max = atof(cfield[5]);
        
        // over[num_overlays]->active = over[num_overlays]->default_on_state;
        
        // over[num_overlays]->color_name[0] = '\0';
        // for(i=6; i < num_fields; i++) {
        //   strncat(over[num_overlays]->color_name,cfield[i],NAME_LENGTH-1);
        //   strncat(over[num_overlays]->color_name," ",NAME_LENGTH-1);
        // }
        // over[num_overlays]->color_name[strlen(over[num_overlays]->color_name) -1] = '\0';
        
        /* strip underscores out of control label */
        // for(i = strlen(over[num_overlays]->control_label)-1;i >0 ; i--) {
        //   if (gd.replace_underscores && over[num_overlays]->control_label[i] == '_') 
        //     over[num_overlays]->control_label[i] = ' ';
        // }
        
        num_overlays++;
      }
    }

    total_len += len  +1;
    start_ptr = end_ptr +1; // Skip past the newline
    line_no++;
  }

  for(i=0; i < 32; i++)  free(cfield[i]);         /* free space for sub strings */
     
  return num_overlays;
}


/************************************************************************
 * LOAD_OVERLAY_DATA: Load each Map
 */

int LegacyParams::_loadOverlayData(int  num_overlays)
{

  // int i;

#ifdef JUNK
  Overlay_t    *ov;    /* pointer to the current overlay structure */
  const char *map_file_subdir = gd.map_file_subdir;
  
  /* Read in each overlay file */
  for(i=0; i < num_overlays; i++) {
    ov = over[i];
    ov->num_polylines = 0;
    ov->num_labels = 0;
    ov->num_icons = 0;
    
    if(strstr(ov->map_file_name,".shp") != NULL  ||
       strstr(ov->map_file_name,".shx") != NULL) {
      
      load_shape_map(ov,map_file_subdir);

    } else {  // Assume RAP Map Format 
      load_rap_map(ov,map_file_subdir);
    }
    
    if(gd.debug)
      printf("Overlay File %s contains %ld Polylines, %ld Icon_defns, %ld Icons, %ld Labels\n",
             ov->map_file_name,ov->num_polylines,ov->num_icondefs,ov->num_icons,ov->num_labels);
    
  }  // End for(i=0; i < num_overlays ...
#endif
  
  return 0;
}

/************************************************************************
 * INIT_OVER_DATA_LINKS:  Scan cidd_overlays.info file and setup
 *
 */ 

int LegacyParams::_initOverlays(const char *param_buf,
                                long param_buf_len,
                                long line_no)
  
{

  gd.num_map_overlays =
    _loadOverlayInfo(param_buf, param_buf_len, line_no, MAX_OVERLAYS);
  
  if(_loadOverlayData(gd.num_map_overlays) != 0) {
    fprintf(stderr,"Problem loading overlay data\n");
    return -1;
  }
  
  // calc_local_over_coords();

  return 0;
  
}
