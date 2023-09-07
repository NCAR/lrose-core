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
#include <qtplot/ColorMap.hh>
#include "Params.hh"
#include "Reader.hh"
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
  // _bscanManager = NULL;
  _reader = NULL;

  // set programe name

  _progName = strdup("Qucid");

  // get command line args
  
  if (_args.parse(argc, (const char **) argv)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = false;
    return;
  }

  // load TDRP params from command line
  
  char *paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = false;
    return;
  }

  if (_params.fields_n < 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  0 fields specified" << endl;
    cerr << "  At least 1 field is required" << endl;
    OK = false;
    return;
  }

  // check for any filtered fields

  _haveFilteredFields = false;
  for (int ifield = 0; ifield < _params.fields_n; ifield++) {
    if (strlen(_params._fields[ifield].filtered_name) > 0) {
      _haveFilteredFields = true;
    }
  }

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

  if (_setupReader()) {
    OK = false;
    return;
  }

  // initialize legacy CIDD structs
  
  _initGlobals();
  gd.use_cosine_correction = -1;
  process_args(argc,argv);    /* process command line arguments */
  
  /* initialize globals, get/set defaults, establish data sources etc. */
  init_data_space();
  
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

  if (_reader) {
    delete _reader;
  }

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
  
  // setup_colorscales(gd.dpy);    /* Establish color table & mappings  */
  
  // Instantiate Symbolic products
  init_symprods();
  
  /* make changes to xview objects not available from DevGuide */
  modify_gui_objects();
  
  gd.finished_init = 1;
     
  // start the reader thread

  _reader->signalRunToStart();
  
  if (_params.display_mode == Params::POLAR_DISPLAY) {

    _cartManager = new CartManager(_params, _reader,
                                    _displayFields, _haveFilteredFields);
    
    if (_args.inputFileList.size() > 0) {
      _cartManager->setArchiveFileList(_args.inputFileList);
      // override archive data url from input file
      string url = _getArchiveUrl(_args.inputFileList[0]);
      TDRP_str_replace(&_params.archive_data_url, url.c_str());
    } else if (_params.begin_in_archive_mode) {
      if (_cartManager->loadArchiveFileList()) {
        
        string errMsg = "WARNING\n";
        errMsg.append("<p>Qucid cannot find archive data files. </p>");
        errMsg.append("<p> Choose a file to open or change the time limits. </p>");
        //errMsg.append(" in startup location. </p>");
        //errMsg.append(_params.archive_data_url);
        //errMsg.append(")</p>");
        //errMsg.append("<p> Click OK to continue to use Qucid.</p>");
        QErrorMessage errorDialog;
        errorDialog.setMinimumSize(400, 250);
        errorDialog.showMessage(errMsg.c_str());
        errorDialog.exec();

        // return -1;
      }
    }

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
// set up reader thread
// returns 0 on success, -1 on failure
  
int Qucid::_setupReader()
{
  
  switch (_params.input_mode) {
    
    case Params::DSR_FMQ_INPUT:
    case Params::IWRF_FMQ_INPUT:
    case Params::IWRF_TCP_INPUT: {
      IwrfReader *iwrfReader = new IwrfReader(_params);
      _reader = iwrfReader;
      break;
    }
      
    case Params::SIMULATED_INPUT:
    default: {
      
      SimReader *simReader = new SimReader(_params);
      _reader = simReader;
      
      vector<SimReader::Field> simFields;
      for (size_t ii = 0; ii < _displayFields.size(); ii++) {
        SimReader::Field simField;
        simField.name = _displayFields[ii]->getName();
        simField.units = _displayFields[ii]->getUnits();
        simField.minVal = _displayFields[ii]->getColorMap().rangeMin();
        simField.maxVal = _displayFields[ii]->getColorMap().rangeMax();
        simFields.push_back(simField);
      }
      simReader->setFields(simFields);

      _params.begin_in_archive_mode = pFALSE;

    }

  } // switch

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
    
    if (strlen(pfld.label) == 0) {
      cerr << "WARNING - Qucid::_setupDisplayFields()" << endl;
      cerr << "  Empty field label, ifield: " << ifield << endl;
      cerr << "  Ignoring" << endl;
      continue;
    }
    
    // check we have a raw field name
    
    if (strlen(pfld.raw_name) == 0) {
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
    map.setName(pfld.label);
    map.setUnits(pfld.units);
    // TODO: the logic here is a little weird ... the label and units have been set, but are we throwing them away?

    bool noColorMap = false;

    if (map.readMap(colorMapPath)) {
        cerr << "WARNING - Qucid::_setupDisplayFields()" << endl;
        cerr << "  Cannot read in color map file: " << colorMapPath << endl;
        cerr << "  Looking for default color map for field " << pfld.label << endl; 

        try {
          // check here for smart color scale; look up by field name/label and
          // see if the name is a usual parameter for a known color map
          SoloDefaultColorWrapper sd = SoloDefaultColorWrapper::getInstance();
          ColorMap colorMap = sd.ColorMapForUsualParm.at(pfld.label);
          cerr << "  found default color map for " <<  pfld.label  << endl;
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
      new DisplayField(pfld.label, pfld.raw_name, pfld.units, 
                       pfld.shortcut, map, ifield, false);
    if (noColorMap)
      field->setNoColorMap();

    _displayFields.push_back(field);

    // filtered field

    if (strlen(pfld.filtered_name) > 0) {
      string filtLabel = string(pfld.label) + "-filt";
      DisplayField *filt =
        new DisplayField(filtLabel, pfld.filtered_name, pfld.units, pfld.shortcut, 
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

string Qucid::_getArchiveUrl(const string &filePath)
  
{

  // find first digit in path - if no digits, return now
  
  const char *start = NULL;
  for (size_t ii = 0; ii < filePath.size(); ii++) {
    if (isdigit(filePath[ii])) {
      start = filePath.c_str() + ii;
      break;
    }
  }
  if (!start) {
    return "";
  }

  const char *end = start + strlen(start);

  // get day dir
  
  int year, month, day;
  while (start < end - 6) {
    if (sscanf(start, "%4d%2d%2d/", &year, &month, &day) == 3) {
      int urlLen = start - filePath.c_str() - 1;
      string url(filePath.substr(0, urlLen));
      if (_params.debug) {
        cerr << "===>> Setting archive url to: " << url << endl;
      }
      return url;
    }
    start++;
  }

  return "";

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
    gd.always_get_full_domain = 0; 
    gd.do_not_clip_on_mdv_request = 0;
    gd.do_not_decimate_on_mdv_request = 0;
    gd.enable_status_window = 0;   
    gd.wsddm_mode = 0;   
    gd.quiet_mode = 0;   
    gd.report_mode = 0;   
    gd.close_popups = 0;   
    gd.one_click_rhi = 0;  
    gd.click_posn_rel_to_origin = 0; 
    gd.report_clicks_in_status_window = 0; 
    gd.report_clicks_in_degM_and_nm = 0;
    gd.magnetic_variation_deg = 0; 
    gd.display_labels = 0; 
    gd.display_ref_lines = 0; 
    gd.show_clock = 0;     
    gd.show_data_messages = 0;
    gd.draw_clock_local = 0; 
    gd.use_local_timestamps = 0; 
    gd.run_unmapped = 0;   
    gd.html_mode = 0;      
    gd.transparent_images = 0;
    gd.range_ring_follows_data = 0; 
    gd.range_ring_for_radar_only = 0;
    gd.domain_follows_data = 0; 
    gd.show_height_sel = 0;
    gd.use_cosine_correction = 0; 
    gd.latlon_mode = 0;    
    gd.zoom_limits_in_latlon = 0; 
    gd.label_contours = 0; 
    gd.top_margin_render_style = 0; 
    gd.bot_margin_render_style = 0; 
    gd.drawing_mode = 0;   
    gd.draw_main_on_top = 0;  
    gd.mark_latest_click_location = 0; 
    gd.mark_latest_client_location = 0; 
    gd.check_data_times = 0;  
    gd.check_clipping = 0;    
    gd.run_once_and_exit = 0; 
    gd.request_compressed_data = 0;  
    gd.request_gzip_vol_compression = 0;  
    gd.add_height_to_filename = 0;   
    gd.add_frame_time_to_filename = 0; 
    gd.add_gen_time_to_filename = 0; 
    gd.add_valid_time_to_filename = 0; 
    gd.add_frame_num_to_filename = 0; 
    gd.add_button_name_to_filename = 0;
    gd.save_images_to_day_subdir = 0;
    gd.simple_command_timeout_secs = 0; 
    gd.complex_command_timeout_secs = 0; 

    gd.font_display_mode = 0; 
    gd.forecast_mode = 0;     
    gd.gather_data_mode = 0;  
    gd.enable_save_image_panel = 0; 
    gd.disable_pick_mode = 0;  
    gd.clip_overlay_fields = 0; 
    gd.output_geo_xml = 0;       
    gd.use_latlon_in_geo_xml = 0;       
    gd.replace_underscores = 0;  

    gd.data_format = 0; 
    gd.image_fill_threshold = 0; 
    gd.dynamic_contour_threshold = 0; 
    gd.inten_levels = 0;    
    gd.idle_reset_seconds = 0;   
    gd.model_run_list_hours = 0; 
				                 
    gd.ideal_x_vects = 0;
    gd.ideal_y_vects = 0;
    gd.head_size = 0;     
    gd.shaft_len = 0;     

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
    gd.num_field_menu_cols = 0; 
    gd.num_map_overlays = 0; 
    gd.num_bookmarks = 0;    
    gd.num_render_heights = 0;
    gd.num_cache_zooms = 0;   
    gd.cur_render_height = 0; 
    gd.cur_field_set = 0;     
    gd.save_im_win = 0;       
    gd.image_needs_saved = 0; 
    gd.generate_filename = 0; 
    gd.max_time_list_span = 0; 

    gd.pan_in_progress = 0;    
    gd.zoom_in_progress = 0;   
    gd.route_in_progress = 0;  
    gd.data_timeout_secs = 0;  
    gd.data_status_changed = 0;
    gd.series_save_active = 0; 

    gd.num_field_labels = 0;  
    gd.db_data_len = 0;       
    MEM_zero(gd.field_index);
    gd.movieframe_time_mode = 0;  
                               
    gd.head_angle = 0;     
    gd.image_inten = 0;    
    gd.data_inten = 0;     
    gd.aspect_correction = 0; 
    gd.aspect_ratio = 0;  
    gd.scale_units_per_km = 0; 
    gd.locator_margin_km = 0;  
    MEM_zero(gd.height_array);
    MEM_zero(gd.proj_param);

    gd.argv = NULL;             
    gd.orig_wd = NULL;           
    gd.db_data = NULL;           
    gd.db_name = NULL;           
    gd.movie_frame_dir = NULL;   
    gd.frame_label = NULL;       
    gd.no_data_message = NULL;   
    gd.help_command = NULL;      
    gd.bookmark_command = NULL;  
    gd.app_name = NULL;          
    gd.app_instance = NULL;      
    gd.scale_units_label = NULL; 

    gd.image_dir = NULL;     
    gd.image_ext = NULL;     
    gd.image_convert_script = NULL; 
    gd.series_convert_script = NULL; 
    gd.print_script = NULL;          
    gd.image_horiz_prefix = NULL;   
    gd.image_vert_prefix = NULL;   
    gd.image_name_separator = NULL; 

    gd.http_tunnel_url = NULL;   
    gd.http_proxy_url = NULL;    
    gd.station_loc_url = NULL;   
    gd.remote_ui_url = NULL;     
    gd.datamap_host = NULL;      

    gd.label_time_format = NULL;  
    gd.moviestart_time_format = NULL;   
    gd.frame_range_time_format = NULL;   
    gd.movieframe_time_format = NULL;  
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
    gd.num_fonts = 0;
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
    gd.syprod_P = NULL;  
    gd.draw_P = NULL;      
    gd.gui_P = NULL;        
    gd.images_P = NULL;  
    gd.uparams = NULL;

}

