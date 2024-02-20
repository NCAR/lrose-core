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
///////////////////////////////////////////////////////////////
// Qucid.cc
//
// Qucid display
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2023
//
///////////////////////////////////////////////////////////////
//
// Qucid is the Qt replacement for CIDD
//
///////////////////////////////////////////////////////////////

#include "Qucid.hh"
#include "CartManager.hh"
#include "DisplayField.hh"
#include "LegacyParams.hh"
#include <qtplot/ColorMap.hh>
#include "Params.hh"
#include "SoloDefaultColorWrapper.hh"
#include <toolsa/mem.h>
#include <toolsa/Path.hh>
#include <toolsa/LogStream.hh>

#include <string>
#include <iostream>
#include <QApplication>
#include <QErrorMessage>

#define CIDD_MAIN 1 /* This is the main module */
#include "cidd.h"

extern void init_xview(int *argc_ptr, char *argv[]);

using namespace std;

// Constructor

Qucid::Qucid(int argc, char **argv) :
        _args("Qucid")

{

  OK = true;
  _cartManager = NULL;

  // set programe name

  _progName = strdup("Qucid");

  // initialize legacy CIDD structs
  
  _initGlobals();

  // initialize signal handling
  
  init_signal_handlers();  

  // check for legacy params file

  string legacyParamsPath;
  char tdrpParamsPath[5000];
  bool usingLegacyParams = false;
  if (_args.getLegacyParamsPath(argc, (const char **) argv, legacyParamsPath) == 0) {
    // gd.db_name = strdup(legacyParamsPath.c_str());
    snprintf(tdrpParamsPath, 4999,
             "/tmp/Qucid.%s.%d.tdrp", legacyParamsPath.c_str(), getpid());
    LegacyParams lParams;
    lParams.translateToTdrp(legacyParamsPath, tdrpParamsPath);
    usingLegacyParams = true;
  }
  
  // get command line args
  
  if (_args.parse(argc, (const char **) argv)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = false;
    return;
  }
  
  // load TDRP params from command line
  
  char *paramsPath = (char *) "unknown";
  if (usingLegacyParams) {
    if (_params.loadApplyArgs(tdrpParamsPath,
                              argc, argv,
                              _args.override.list)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with TDRP parameters." << endl;
      OK = false;
      return;
    }
  } else {
    if (_params.loadFromArgs(argc, argv,
                             _args.override.list,
                             &paramsPath)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with TDRP parameters." << endl;
      OK = false;
      return;
    }
  }
  
  if (_params.fields_n < 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  0 fields specified" << endl;
    cerr << "  At least 1 field is required" << endl;
    OK = false;
    return;
  }

  // initialize globals, get/set defaults, establish data sources etc.
  
  if (_args.usingLegacyParams()) {
    init_data_space();
  }
  
  exit(0);
  
  // set params on alloc checker

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    LOG_STREAM_INIT(true, true, true, true);
  } else if (_params.debug) {
    LOG_STREAM_INIT(true, false, true, true);
  } else {
    LOG_STREAM_INIT(false, false, false, false);
    LOG_STREAM_TO_CERR();
  }

  // print color scales if debugging
  if (_params.debug) {
    SoloDefaultColorWrapper sd = SoloDefaultColorWrapper::getInstance();
    sd.PrintColorScales();
  } 

  // set up display fields

  if (_setupDisplayFields()) {
    OK = false;
    return;
  }

  // create reader

  /* Ref: https://bugs.launchpad.net/ubuntu/+source/xview/+bug/1059988
   * Xview libs Segfault if RLIMIT_NOFILE > 3232
   */
  struct rlimit rlim;
  getrlimit(RLIMIT_NOFILE, &rlim);
  if (rlim.rlim_cur >  3200) 
    rlim.rlim_cur = 3200;
  setrlimit(RLIMIT_NOFILE, &rlim);

  // get the display

  if (_setupXDisplay(argc, argv)) {
    cerr << "Cannot set up X display" << endl;
    OK = false;
  }
  init_xview(&argc, argv);
  
  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

}

// destructor

Qucid::~Qucid()

{

  if (_cartManager) {
    delete _cartManager;
  }

  // if (_bscanManager) {
  //   delete _bscanManager;
  // }

  for (size_t ii = 0; ii < _displayFields.size(); ii++) {
    delete _displayFields[ii];
  }
  _displayFields.clear();
  // logger.closeFile();

}

//////////////////////////////////////////////////
// Run

int Qucid::Run(QApplication &app)
{

  // init_xview(&argc,argv); /* create all Xview objects */    
  
  setup_colorscales(gd.dpy);    /* Establish color table & mappings  */
  
  // Instantiate Symbolic products
  init_symprods();
  
  /* make changes to xview objects not available from DevGuide */
  modify_gui_objects();
  
  gd.finished_init = 1;
     
  if (_params.display_mode == Params::POLAR_DISPLAY) {

    _cartManager = new CartManager(_params, _displayFields, false);
    return _cartManager->run(app);

  } else if (_params.display_mode == Params::BSCAN_DISPLAY) {

    // _bscanManager = new BscanManager(_params, _reader, 
    //                                  _displayFields, _haveFilteredFields);
    // return _bscanManager->run(app);

  }
  
  return -1;

}

//////////////////////////////////////////////////
// set up the display variable
  
int Qucid::_setupXDisplay(int argc, char **argv)
{

  /*
   * search for display name on command line
   */

  gd.dpyName = NULL;
  
  for (int i =  1; i < argc; i++) {
    if (!strcmp(argv[i], "-display") || !strcmp(argv[i], "-d")) {
      if (i < argc - 1) {
	gd.dpyName = new char[strlen(argv[i+1]) + 1];
	strcpy(gd.dpyName, argv[i+1]);
      }
    }
  } /* i */
	
  if((gd.dpy = XOpenDisplay(gd.dpyName)) == NULL) {
    fprintf(stderr, "ERROR - Qucid::_setupXDisplay\n");
    fprintf(stderr,
	    "Cannot open display '%s' or '%s'\n",
	    gd.dpyName, getenv("DISPLAY"));
    return -1;
  }
  
  /*
   * register x error handler
   */
  
  // XSetErrorHandler(xerror_handler);

  return 0;

}

//////////////////////////////////////////////////
// set up field objects, with their color maps
// use same map for raw and unfiltered fields
// returns 0 on success, -1 on failure
  
int Qucid::_setupDisplayFields()
{

  // check for color map location
  
  string colorMapDir = _params.color_scale_dir;
  Path mapDir(_params.color_scale_dir);
  if (!mapDir.dirExists()) {
    colorMapDir = Path::getPathRelToExec(_params.color_scale_dir);
    mapDir.setPath(colorMapDir);
    if (!mapDir.dirExists()) {
      cerr << "ERROR - Qucid" << endl;
      cerr << "  Cannot find color scale directory" << endl;
      cerr << "  Primary is: " << _params.color_scale_dir << endl;
      cerr << "  Secondary is relative to binary: " << colorMapDir << endl;
      return -1;
    }
    if (_params.debug) {
      cerr << "NOTE - using color scales relative to executable location" << endl;
      cerr << "  Exec path: " << Path::getExecPath() << endl;
      cerr << "  Color scale dir:: " << colorMapDir << endl;
    }
  }

  // we interleave unfiltered fields and filtered fields

  for (int ifield = 0; ifield < _params.fields_n; ifield++) {

    const Params::field_t &pfld = _params._fields[ifield];

    // check we have a valid label
    
    if (strlen(pfld.legend_label) == 0) {
      cerr << "WARNING - Qucid::_setupDisplayFields()" << endl;
      cerr << "  Empty field legend_label, ifield: " << ifield << endl;
      cerr << "  Ignoring" << endl;
      continue;
    }
    
    // check we have a raw field name
    
    if (strlen(pfld.field_name) == 0) {
      cerr << "WARNING - Qucid::_setupDisplayFields()" << endl;
      cerr << "  Empty raw field name, ifield: " << ifield << endl;
      cerr << "  Ignoring" << endl;
      continue;
    }

    // create color map
    
    string colorMapPath = colorMapDir;
    colorMapPath += PATH_DELIM;
    colorMapPath += pfld.color_map;
    ColorMap map;
    map.setName(pfld.legend_label);
    map.setUnits(pfld.field_units);
    // TODO: the logic here is a little weird ... the legend_label and units have been set, but are we throwing them away?

    bool noColorMap = false;

    if (map.readMap(colorMapPath)) {
      cerr << "WARNING - Qucid::_setupDisplayFields()" << endl;
      cerr << "  Cannot read in color map file: " << colorMapPath << endl;
      cerr << "  Looking for default color map for field " << pfld.legend_label << endl; 

      try {
        // check here for smart color scale; look up by field name/legend_label and
        // see if the name is a usual parameter for a known color map
        SoloDefaultColorWrapper sd = SoloDefaultColorWrapper::getInstance();
        ColorMap colorMap = sd.ColorMapForUsualParm.at(pfld.legend_label);
        cerr << "  found default color map for " <<  pfld.legend_label  << endl;
        // if (_params.debug) colorMap.print(cout); // LOG(DEBUG_VERBOSE)); // cout);
        map = colorMap;
        // HERE: What is missing from the ColorMap object??? 
      } catch (std::out_of_range &ex) {
        cerr << "WARNING - did not find default color map for field; using rainbow colors" << endl;
        // Just set the colormap to a generic color map
        // use range to indicate it needs update; update when we have access to the actual data values
        map = ColorMap(0.0, 1.0);
        noColorMap = true; 
        // return -1
      }
    }

    // unfiltered field

    DisplayField *field =
      new DisplayField(pfld.legend_label, pfld.field_name, pfld.field_units, 
                       "a", map, ifield, false);
    if (noColorMap)
      field->setNoColorMap();

    _displayFields.push_back(field);

    // filtered field

    if (strlen(pfld.field_name) > 0) {
      string filtField_Label = string(pfld.legend_label) + "-filt";
      DisplayField *filt =
        new DisplayField(pfld.legend_label, pfld.field_name, pfld.field_units, "a", 
                         map, ifield, true);
      _displayFields.push_back(filt);
    }

  } // ifield

  if (_displayFields.size() < 1) {
    cerr << "ERROR - Qucid::_setupDisplayFields()" << endl;
    cerr << "  No fields found" << endl;
    return -1;
  }

  return 0;

}


///////////////////////////////////////////////////
// get the archive url

void Qucid::_initGlobals()
  
{

  gd.hcan_xid = 0;
  gd.vcan_xid = 0;
    
  gd.debug = 0;
  gd.debug1 = 0;
  gd.debug2 = 0;
    
  gd.argc = 0;
  gd.display_projection = 0;
  // _params.always_get_full_domain = 0; 
  // _params.do_not_clip_on_mdv_request = 0;
  // _params.do_not_decimate_on_mdv_request = 0;
  // _params.enable_status_window = 0;   
  // _params.wsddm_mode = 0;   
  gd.quiet_mode = 0;   
  gd.report_mode = 0;   
  // _params.close_popups = 0;   
  // _params.one_click_rhi = 0;  
  // _params.click_posn_rel_to_origin = 0; 
  // _params.report_clicks_in_status_window = 0; 
  // _params.report_clicks_in_degM_and_nm = 0;
  // _params.magnetic_variation_deg = 0; 
  // _params.display_labels = 0; 
  // _params.display_ref_lines = 0; 
  // _params.show_clock = 0;     
  // _params.show_data_messages = 0;
  // _params.draw_clock_local = 0; 
  // _params.use_local_timestamps = 0; 
  gd.run_unmapped = 0;   
  // _params.html_mode = 0;      
  // _params.transparent_images = 0;
  // _params.range_ring_follows_data = 0; 
  // _params.range_ring_for_radar_only = 0;
  // _params.domain_follows_data = 0; 
  // _params.show_height_sel = 0;
  
  gd.use_cosine_correction = -1;
  // _params.latlon_mode = 0;    
  // _params.zoom_limits_in_latlon = 0; 

  // _params.horiz_top_margin = 0;
  // _params.horiz_bot_margin = 0;
  // _params.horiz_left_margin = 0;
  // _params.horiz_right_margin = 0;
  
  // _params.vert_top_margin = 0;
  // _params.vert_bot_margin = 0;
  // _params.vert_left_margin = 0;
  // _params.vert_right_margin = 0;
  
  // _params.horiz_min_height = 0;
  // _params.horiz_min_width = 0;
  // _params.horiz_default_height = 0;
  // _params.horiz_default_width = 0;
  
  // _params.vert_min_height = 0;
  // _params.vert_min_width = 0;
  // _params.vert_default_height = 0;
  // _params.vert_default_width = 0;
  
  // _params.horiz_legends_start_x = 0;
  // _params.horiz_legends_start_y = 0;
  // _params.horiz_legends_delta_y = 0;

  // _params.vert_legends_start_x = 0;
  // _params.vert_legends_start_y = 0;
  // _params.vert_legends_delta_y = 0;

  // _params.domain_limit_min_x = 0;
  // _params.domain_limit_max_x = 0;
  // _params.domain_limit_min_y = 0;
  // _params.domain_limit_max_y = 0;
  
  // _params.min_ht = 0;
  // _params.max_ht = 0;
  // _params.start_ht = 0;
  // _params.planview_start_page = 0;
  // _params.xsect_start_page = 0;
  
  // _params.label_contours = 0; 
  // _params.top_margin_render_style = 0; 
  // _params.bot_margin_render_style = 0; 
  // _params.drawing_mode = 0;   

  // _params.layer_legends_on = 0;  // Control variables for plotting legend/Titles
  // _params.cont_legends_on = 0;
  // _params.wind_legends_on = 0;
  // _params.contour_line_width = 0;
  // _params.smooth_contours = 0;  // Apply smoothing before contouring  = 0;  0-2 are valid values 
  // _params.use_alt_contours = 0; // 1 =  Use the Alternate Contouring routines 
  // _params.add_noise = 0;        // Add 1 part in 250 of noise to contours 1 == true
  // _params.special_contour_value = 0;    // which value to draw wider 
  // _params.map_bad_to_min_value = 0;     // 1 == true
  // _params.map_missing_to_min_value = 0; // 1 == true

  // _params.products_on = 0;
  // _params.product_line_width = 0;
  // _params.product_font_size = 0;
  MEM_zero(gd.product_detail_threshold);
  MEM_zero(gd.product_detail_adjustment);

  // _params.draw_main_on_top = 0;  
  // _params.mark_latest_click_location = 0; 
  gd.mark_latest_client_location = 0; 
  // _params.check_data_times = 0;  
  // _params.check_clipping = 0;    
  // _params.run_once_and_exit = 0; 
  // _params.request_compressed_data = 0;  
  // _params.request_gzip_vol_compression = 0;  
  // _params.add_height_to_filename = 0;   
  // _params.add_frame_time_to_filename = 0; 
  // _params.add_gen_time_to_filename = 0; 
  // _params.add_valid_time_to_filename = 0; 
  // _params.add_frame_num_to_filename = 0; 
  // _params.add_button_name_to_filename = 0;
  // _params.save_images_to_day_subdir = 0;
  // _params.simple_command_timeout_secs = 0; 
  // _params.complex_command_timeout_secs = 0; 

  // _params.font_display_mode = 0; 
  gd.forecast_mode = 0;     
  // _params.gather_data_mode = 0;  
  // _params.enable_save_image_panel = 0; 
  // _params.disable_pick_mode = 0;  
  // _params.clip_overlay_fields = 0; 
  // _params.output_geo_xml = 0;       
  // _params.use_latlon_in_geo_xml = 0;       
  // _params.replace_underscores = 0;  

  gd.data_format = 0; 
  // _params.image_fill_threshold = 0; 
  // _params.dynamic_contour_threshold = 0; 
  // _params.inten_levels = 0;    
  // _params.idle_reset_seconds = 0;   
  // _params.model_run_list_hours = 0; 
				                 
  // _params.ideal_x_vects = 0;
  // _params.ideal_y_vects = 0;
  // _params.wind_head_size = 0;     
  // _params.wind_head_angle = 0;     
  // _params.barb_shaft_len = 0;
  // _params.all_winds_on = 0;
  // _params.wind_mode = 0;
  // _params.wind_time_scale_interval = 0;
  // _params.wind_scaler = 0;

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
  // _params.num_field_menu_cols = 0; 
  gd.num_map_overlays = 0; 
  // _params.num_bookmarks = 0;    
  gd.num_render_heights = 0;
  // _params.num_cache_zooms = 0;   
  gd.cur_render_height = 0; 
  gd.cur_field_set = 0;     
  gd.save_im_win = 0;       
  gd.image_needs_saved = 0; 
  gd.generate_filename = 0; 
  // _params.max_time_list_span = 0; 

  gd.pan_in_progress = 0;    
  gd.zoom_in_progress = 0;   
  gd.route_in_progress = 0;  
  // _params.data_timeout_secs = 0;  
  gd.data_status_changed = 0;
  gd.series_save_active = 0; 

  gd.num_field_labels = 0;  
  // gd.db_data_len = 0;       
  MEM_zero(gd.field_index);
  // _params.movieframe_time_mode = 0;  
                               
  // _params.image_inten = 0;    
  // _params.data_inten = 0;     
  gd.aspect_correction = 0; 
  // _params.aspect_ratio = 0;  
  // _params.scale_units_per_km = 0; 
  // _params.locator_margin_km = 0;  
  MEM_zero(gd.height_array);
  MEM_zero(gd.proj_param);

  // _params.origin_latitude = 0.0;
  // _params.origin_longitude = 0.0;

  // _params.reset_click_latitude = 0.0;
  // _params.reset_click_longitude = 0.0;

  // _params.projection_type = NULL;
  // _params.north_angle = 0.0;      // radar cart
  // _params.lambert_lat1 = 0.0;     // lambert
  // _params.lambert_lat2 = 0.0;     // lambert
  // _params.tangent_lat = 0.0;      // stereographic
  // _params.tangent_lon = 0.0;      // stereographic
  // _params.central_scale = 0.0;    // stereographic

  // _params.movie_on = 0;
  // _params.movie_magnify_factor = 0;
  // _params.time_interval = 0;
  // _params.frame_span = 0;
  // _params.starting_movie_frames = 0;
  // _params.reset_frames = 0;
  // _params.movie_delay = 0;
  // _params.forecast_interval = 0;
  // _params.past_interval = 0;
  // _params.time_search_stretch_factor = 0;
  // _params.climo_mode = NULL;
  // _params.temporal_rounding = 0;
  // _params.movie_speed_msec = 0;
  
  gd.argv = NULL;             
  gd.orig_wd = NULL;           
  // gd.db_data = NULL;           
  // gd.db_name = NULL;           
  // _params.frame_label = NULL;       
  // _params.no_data_message = NULL;   
  // _params.help_command = NULL;      
  // _params.bookmark_command = NULL;  
  gd.app_name = NULL;          
  gd.app_instance = NULL;      
  // _params.scale_units_label = NULL; 

  // _params.image_dir = NULL;     
  // gd.horiz_image_dir = NULL;     
  // gd.vert_image_dir = NULL;     
  // _params.horiz_image_fname = NULL;     
  // _params.vert_image_fname = NULL;     
  // _params.horiz_image_command = NULL;     
  // _params.vert_image_command = NULL;     

  // _params.image_ext = NULL;     
  // _params.image_convert_script = NULL; 
  // _params.series_convert_script = NULL; 

  // _params.image_horiz_prefix = NULL;   
  // _params.image_vert_prefix = NULL;   
  // _params.image_name_separator = NULL; 

  // _params.print_script = NULL;          

  // _params.http_tunnel_url = NULL;   
  // _params.http_proxy_url = NULL;    
  // _params.station_loc_url = NULL;   
  // _params.remote_ui_url = NULL;     
  // _params.datamap_host = NULL;      

  // _params.label_time_format = NULL;  
  // _params.moviestart_time_format = NULL;   
  // _params.frame_range_time_format = NULL;   
  // _params.movieframe_time_format = NULL;  
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

  // FONTS
  // _params.fonts_n = 0;
  MEM_zero(gd.ciddfont);
  MEM_zero(gd.fontst);
    
  MEM_zero(gd.prod);
  MEM_zero(gd.over);
  MEM_zero(gd.mrec);
  MEM_zero(gd.layers);
  MEM_zero(gd.legends);
  MEM_zero(gd.menu_bar);
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
  // gd.syprod_P = NULL;  
  // gd.draw_P = NULL;      
  // gd.gui_P = NULL;        
  // gd.images_P = NULL;  
  // gd.uparams = NULL;

  // _params.contour_font_num = 0;
  // _params.n_ideal_contour_labels = 0;
  
  // _params.rotate_coarse_adjust = 0.0;
  // _params.rotate_medium_adjust = 0.0;
  // _params.rotate_fine_adjust = 0.0;
  
  // _params.min_zoom_threshold = 0.0;

  gd.coord_key = 0;

  // _params.status_info_file = NULL;

  // _params.horiz_default_x_pos = 0;
  // _params.horiz_default_y_pos = 0;
  // _params.vert_default_x_pos = 0;
  // _params.vert_default_y_pos = 0;

  // _params.map_file_subdir = NULL;

  // _params.ideal_x_vectors = 0;
  // _params.ideal_y_vectors = 0;

  // gd.azimuth_interval = 0;
  // gd.azimuth_radius = 0;

  // _params.latest_click_mark_size = 0;
  // _params.range_ring_x_space = 0;
  // _params.range_ring_y_space = 0;
  // _params.range_ring_spacing = 0;
  // _params.max_ring_range = 0;
  // _params.range_ring_labels = 0;
  // _params.wind_units_scale_factor = 0;
  // _params.wind_units_label = NULL;
  // _params.wind_w_scale_factor = 0;

  // _params.scale_constant = 0.0;
    
  // _params.redraw_interval = 0;
  // _params.update_interval = 0;
    
  // _params.wind_marker_type = NULL;
  // _params.wind_reference_speed = 0.0;

}

