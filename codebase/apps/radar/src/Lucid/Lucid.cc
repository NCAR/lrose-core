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
// Lucid.cc
//
// Lucid display
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2023
//
///////////////////////////////////////////////////////////////
//
// Lucid is the Qt replacement for CIDD
//
///////////////////////////////////////////////////////////////

#include "Lucid.hh"
#include "GuiManager.hh"
#include "LegacyParams.hh"
#include "MdvReader.hh"
#include "RenderContext.hh"
#include "ProductMgr.hh"
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <toolsa/utim.h>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/TaStr.hh>
#include <toolsa/http.h>
#include <toolsa/HttpURL.hh>
#include <toolsa/pjg.h>       // Map projection geometry 
#include <shapelib/shapefil.h>
#include <qtplot/ColorMap.hh>
#include <Mdv/Mdvx_typedefs.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <Fmq/RemoteUIQueue.hh>
#include <Spdb/StationLoc.hh> // Station locator class 

#include <string>
#include <iostream>
#include <algorithm>
#include <QApplication>
#include <QErrorMessage>

// #define THIS_IS_MAIN 1 /* This is the main module */

using namespace std;

// Constructor

Lucid::Lucid(int argc, char **argv) :
        _params(Params::Instance()),
        _gd(GlobalData::Instance()),
        _progName("Lucid"),
        _args("Lucid")

{

  OK = true;

  // set programe name

  _progName = strdup("Lucid");

  // check for legacy params file
  // if found create a temporary tdrp file based on the legacy file

  string legacyParamsPath;
  char tdrpParamsPath[5000];
  bool usingLegacyParams = false;
  if (_args.getLegacyParamsPath(argc, (const char **) argv, legacyParamsPath) == 0) {
    // _gd.db_name = strdup(legacyParamsPath.c_str());
    Path lpPath(legacyParamsPath);
    snprintf(tdrpParamsPath, 4999,
             "/tmp/Lucid.%s.%d.tdrp", lpPath.getFile().c_str(), getpid());
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
    _paramsPathRequested = legacyParamsPath;
    _paramsPathUsed = tdrpParamsPath;
  } else {
    if (_params.loadFromArgs(argc, argv,
                             _args.override.list,
                             &paramsPath)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with TDRP parameters." << endl;
      OK = false;
      return;
    }
    _paramsPathRequested = paramsPath;
    _paramsPathUsed = paramsPath;
  }

  if (_params.debug) {
    cerr << "Using params path: " << _paramsPathUsed.getPath() << endl;
  }
  
  if (_params.fields_n < 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  0 fields specified" << endl;
    cerr << "  At least 1 field is required" << endl;
    OK = false;
    return;
  }
    
  // initialize globals, get/set defaults, establish data sources etc.

  if (_initDataSpace()) {
    OK = false;
    return;
  }

  // init process mapper registration
  
  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

}

// destructor

Lucid::~Lucid()

{

  if (_guiManager) {
    delete _guiManager;
  }

}

//////////////////////////////////////////////////
// Run

int Lucid::RunApp(QApplication &app)
{

  _gd.finished_init = 1;

  // create cartesian display
  
  _guiManager = new GuiManager;
  return _guiManager->run(app);
  
}

/*****************************************************************
 * Initialize the data variables
 */

int Lucid::_initDataSpace()
{

  UTIMstruct temp_utime;
  
  if(!_gd.quiet_mode) {
    fprintf(stderr,"Lucid: Version %s\n", Constants::LUCID_VERSION);
    fprintf(stderr,"Copyright: %s\n\n", Constants::LUCID_COPYRIGHT);
  }

  // debugging level
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _gd.debug = 1;
    _gd.debug1 = 1;
    _gd.debug2 = 1;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    _gd.debug = 1;
    _gd.debug1 = 1;
    _gd.debug2 = 0;
  } else if (_params.debug >= Params::DEBUG_NORM) {
    _gd.debug = 1;
    _gd.debug1 = 0;
    _gd.debug2 = 0;
  } else {
    _gd.debug = 0;
    _gd.debug1 = 0;
    _gd.debug2 = 0;
  }

  // create cache directories

  if (_createCacheDirs()) {
    return -1;
  }
  
  // open shmem segment for interprocess comms
  
  _gd.coord_key = _params.coord_key;
  if((_gd.coord_expt = (coord_export_t *) ushm_create(_gd.coord_key,
                                                      sizeof(coord_export_t),
                                                      0666)) == NULL) {
    fprintf(stderr, "Could not create shared mem for interprocess comms\n");
    exit(-1);
  }
  memset(_gd.coord_expt, 0, sizeof(coord_export_t));

  // initialize shared memory
  
  PMU_auto_register("Initializing SHMEM");
  _initShared(); /* Initialize Shared memory based communications */
  
  
  // html mode
  
  if(_params.run_once_and_exit) _params.html_mode = pTRUE;
  _gd.h_win.zoom_level = 0;
  
  // image generation
  
  STRcopy(_gd.h_win.image_dir, _params.image_dir, MAX_PATH_LEN);
  STRcopy(_gd.v_win.image_dir, _params.image_dir, MAX_PATH_LEN);
  
  STRcopy(_gd.h_win.image_fname, _params.horiz_image_fname, MAX_PATH_LEN);
  STRcopy(_gd.h_win.image_command, _params.horiz_image_command, MAX_PATH_LEN);

  STRcopy(_gd.v_win.image_fname, _params.vert_image_fname, MAX_PATH_LEN);
  STRcopy(_gd.v_win.image_command, _params.vert_image_command, MAX_PATH_LEN);

  // If individual horiz and vert scripts have not been set
  // use the general one.
  
  if(strlen(_gd.h_win.image_command) < 3) {
    STRcopy(_gd.h_win.image_command, _params.image_convert_script, MAX_PATH_LEN);
  }
  
  if(strlen(_gd.v_win.image_command) < 3) {
    STRcopy(_gd.v_win.image_command, _params.image_convert_script, MAX_PATH_LEN);
  }

  if(_params.idle_reset_seconds <= 0 || _params.html_mode == 1) {
    _params.idle_reset_seconds = 1000000000; // 30+ years
  }

  // layers
  
  _gd.layers.layer_legends_on = _params.layer_legends_on;
  _gd.layers.cont_legends_on = _params.cont_legends_on;
  _gd.layers.wind_legends_on = _params.wind_legends_on;
  _gd.layers.contour_line_width = _params.contour_line_width;
  _gd.layers.smooth_contours = _params.smooth_contours;
  _gd.layers.use_alt_contours = _params.use_alt_contours;
  _gd.layers.add_noise = _params.add_noise;
  _gd.layers.special_contour_value = _params.special_contour_value;
  _gd.layers.map_bad_to_min_value = _params.map_bad_to_min_value;
  _gd.layers.map_missing_to_min_value = _params.map_missing_to_min_value;

  // products
  
  _gd.prod.products_on = _params.products_on;
  _gd.prod.prod_line_width = _params.product_line_width;
  _gd.prod.prod_font_num = _params.product_font_size;
  
  for(int ii = 0; ii < _params.product_adjustments_n; ii++) {
    _gd.prod.detail[ii].threshold =
      _params._product_adjustments[ii].threshold;
    _gd.prod.detail[ii].adjustment =
      _params._product_adjustments[ii].font_index_adj;
  } // ii

  // if domain follows data, do not clip or decimate
  
  if (_params.zoom_domain_follows_data) {
    // _params.always_get_full_domain = pTRUE;
    _params.clip_to_current_zoom_on_mdv_request = pFALSE;
    _params.decimate_resolution_on_mdv_request = pFALSE;
  }

  // Bookmarks for a menu of URLS - Index starts at 1
  
  // if(_params.bookmarks_n > 0) {
  //   _gd.bookmark = new bookmark_t[_params.bookmarks_n];
  // }
  // for(int ii = 0; ii < _params.bookmarks_n; ii++) {
  //   _gd.bookmark[ii].url = strdup(_params._bookmarks[ii].url);
  //   _gd.bookmark[ii].label = strdup(_params._bookmarks[ii].label);
  // }

  // origin latitude and longitude
  
  _gd.h_win.origin_lat = _params.proj_origin_lat;
  _gd.h_win.origin_lon = _params.proj_origin_lon;

  // click location on reset
  
  _gd.h_win.reset_click_lat = _params.proj_origin_lat;
  _gd.h_win.reset_click_lon = _params.proj_origin_lon;

  // projection

  if (_params.proj_type == Params::PROJ_FLAT) {
    
    _gd.display_projection = Mdvx::PROJ_FLAT;
    _gd.proj_param[0] = _params.proj_rotation; // rotation rel to TN
    _gd.proj.initFlat(_params.proj_origin_lat,
                      _params.proj_origin_lon,
                      _params.proj_rotation);
    if(_gd.debug) {
      fprintf(stderr, "Cartesian projection\n");
      fprintf(stderr, "Origin at: %g, %g\n", 
              _params.proj_origin_lat,_params.proj_origin_lon);
    }
    
  } else if (_params.proj_type == Params::PROJ_LATLON) {

    _gd.display_projection = Mdvx::PROJ_LATLON;
    _gd.proj.initLatlon(_params.proj_origin_lon);
    if(_gd.debug) {
      fprintf(stderr, "LATLON/Cylindrical projection\n");
      fprintf(stderr, "Origin at: %g, %g\n",
              _params.proj_origin_lat, _params.proj_origin_lon);
    }
    
  } else if (_params.proj_type == Params::PROJ_LAMBERT_CONF) {
    
    _gd.display_projection = Mdvx::PROJ_LAMBERT_CONF;
    _gd.proj_param[0] = _params.proj_lat1;
    _gd.proj_param[1] = _params.proj_lat2;
    _gd.proj.initLambertConf(_params.proj_origin_lat,
                             _params.proj_origin_lon,
                             _params.proj_lat1,
                             _params.proj_lat2);
    if(_gd.debug) {
      fprintf(stderr, "LAMBERT projection\n");
      fprintf(stderr, "Origin at: %g, %g\n",
              _params.proj_origin_lat, _params.proj_origin_lon);
      fprintf(stderr, "Parallels at: %g, %g\n",
              _params.proj_lat1, _params.proj_lat2);
    }
    
  } else if (_params.proj_type == Params::PROJ_OBLIQUE_STEREO) {

    _gd.display_projection = Mdvx::PROJ_OBLIQUE_STEREO;
    _gd.proj_param[0] = _params.proj_tangent_lat;
    _gd.proj_param[1] = _params.proj_tangent_lon;
    _gd.proj_param[2] = _params.proj_central_scale;
    _gd.proj.initStereographic(_params.proj_tangent_lat,
                               _params.proj_tangent_lon,
                               _params.proj_central_scale);
    _gd.proj.setOffsetOrigin(_params.proj_origin_lat,
                             _params.proj_origin_lon);
    if(_gd.debug) {
      fprintf(stderr, "Oblique Stereographic projection\n");
      fprintf(stderr, "Origin at: %g, %g\n",
              _params.proj_origin_lat,_params.proj_origin_lon);
      fprintf(stderr, "Tangent at: %g, %g\n",
              _params.proj_tangent_lat, _params.proj_tangent_lon);
    }
    
  } else if (_params.proj_type == Params::PROJ_POLAR_STEREO) {
    
    _gd.display_projection = Mdvx::PROJ_POLAR_STEREO;
    _gd.proj_param[0] = _params.proj_tangent_lat;
    _gd.proj_param[1] = _params.proj_tangent_lon;
    _gd.proj_param[2] = _params.proj_central_scale;
    _gd.proj.initPolarStereo
      (_params.proj_tangent_lon,
       (Mdvx::pole_type_t) (_params.proj_tangent_lat < 0.0 ?
                            Mdvx::POLE_SOUTH : Mdvx::POLE_NORTH),
       _params.proj_central_scale);
    _gd.proj.setOffsetOrigin(_params.proj_origin_lat, _params.proj_origin_lon);
    if(_gd.debug) {
      fprintf(stderr, "Polar Stereographic projection\n");
      fprintf(stderr, "Origin at: %g, %g\n",
              _params.proj_origin_lat, _params.proj_origin_lon);
      fprintf(stderr, "Tangent at: %g, %g\n",
              _params.proj_tangent_lat, _params.proj_tangent_lon);
      fprintf(stderr, "Central scale: %g\n",
              _params.proj_central_scale);
    }

  } else if (_params.proj_type == Params::PROJ_MERCATOR) {
    
    _gd.display_projection = Mdvx::PROJ_MERCATOR;
    _gd.proj.initMercator(_params.proj_origin_lat, _params.proj_origin_lon);
    if(_gd.debug) {
      fprintf(stderr, "MERCATOR projection\n");
      fprintf(stderr, "Origin at: %g, %g\n",
              _params.proj_origin_lat, _params.proj_origin_lon);
    }
      
  } else if (_params.proj_type == Params::PROJ_TRANS_MERCATOR) {
    
    _gd.display_projection = Mdvx::PROJ_TRANS_MERCATOR;
    _gd.proj_param[0] = _params.proj_central_scale;
    _gd.proj.initTransMercator(_params.proj_origin_lat,
                               _params.proj_origin_lon,
                               _params.proj_central_scale);
    if(_gd.debug) {
      fprintf(stderr, "TRANS_MERCATOR projection\n");
      fprintf(stderr, "Origin at: %g, %g\n",
              _params.proj_origin_lat, _params.proj_origin_lon);
      fprintf(stderr, "Central scale: %g\n",
              _params.proj_central_scale);
    }
      
  } else if (_params.proj_type == Params::PROJ_ALBERS) {
    
    _gd.display_projection = Mdvx::PROJ_ALBERS;
    _gd.proj_param[0] = _params.proj_lat1;
    _gd.proj_param[1] = _params.proj_lat2;
    _gd.proj.initAlbers(_params.proj_origin_lat,
                        _params.proj_origin_lon,
                        _params.proj_lat1,
                        _params.proj_lat2);
    if(_gd.debug) {
      fprintf(stderr, "ALBERS projection\n");
      fprintf(stderr, "Origin at: %g, %g\n",
              _params.proj_origin_lat, _params.proj_origin_lon);
      fprintf(stderr, "Parallels at: %g, %g\n",
              _params.proj_lat1, _params.proj_lat2);
    }
    
  } else if (_params.proj_type == Params::PROJ_LAMBERT_AZIM) {
    
    _gd.display_projection = Mdvx::PROJ_LAMBERT_AZIM;
    _gd.proj.initLambertAzim(_params.proj_origin_lat,_params.proj_origin_lon);
    if(_gd.debug) {
      fprintf(stderr, "LAMBERT_AZIM projection\n");
      fprintf(stderr, "Origin at: %g, %g\n",
              _params.proj_origin_lat, _params.proj_origin_lon);
    }
      
  }
  
  _gd.h_win.prev_page = -1;
  _gd.v_win.prev_page = -1;

  // global redraw flags

  // _gd.redraw_horiz = true;
  _gd.redraw_vert = true;

  _gd.time_has_changed = true;
  _gd.field_has_changed = true;
  _gd.zoom_has_changed = true;
  _gd.vsect_has_changed = true;
  _gd.ht_has_changed = true;

  _gd.prev_time = -1;
  _gd.prev_field = -1;
  _gd.prev_ht = -9999.0;
  
  _gd.selected_time = -2;
  _gd.selected_field = -2;
  _gd.selected_ht = -9998.0;
  
  // movies

  int pid = getpid();
  for(int ii = 0; ii < Constants::MAX_FRAMES; ii++) {
    snprintf(_gd.movie.frame[ii].fname, Constants::NAME_LENGTH - 1,
             "%s/cidd_im%d_%d.",
             _params.image_dir, pid, ii);
    _gd.movie.frame[ii].h_pdev = 0;
    _gd.movie.frame[ii].v_pdev = 0;
    _gd.movie.frame[ii].redraw_horiz = 1;
    _gd.movie.frame[ii].redraw_vert = 1;
  }

  // copy movie info to other globals
  
  _gd.movie.movie_on = _params.movie_on;
  if(_params.html_mode) {
    _gd.movie.movie_on = 0;
  }
  _gd.movie.magnify_factor = _params.movie_magnify_factor;
  _gd.movie.time_interval_mins = _params.frame_interval_secs / 60.0;
  _gd.movie.frame_span = _params.climo_frame_span_mins;
  _gd.movie.num_frames = _params.n_movie_frames;
  _gd.movie.reset_frames = _params.reset_frames;
  _gd.movie.delay = _params.loop_delay_msecs;
  _gd.movie.forecast_interval = _params.forecast_interval_hours;
  _gd.movie.past_interval = _params.past_interval_hours;
  _gd.movie.mr_stretch_factor = _params.time_search_stretch_factor;
  _gd.movie.round_to_seconds = _params.temporal_rounding;
  _gd.movie.display_time_msec = _params.movie_dwell_msecs;

  _gd.movie.start_frame = 0;
  _gd.movie.sweep_on = 0;
  _gd.movie.sweep_dir = 1;
  _gd.movie.end_frame = _gd.movie.num_frames -1 ;
  _gd.movie.cur_frame = _gd.movie.num_frames -1;
  _gd.movie.last_frame = _gd.movie.cur_frame;

  // climatology mode for movies
  
  _gd.movie.climo_mode = (int) _params.climo_mode;

  // Use cosine correction for computing range in polar data
  // check if set by Args

  if (_gd.use_cosine_correction < 0) {
    // not set on command line
    _gd.use_cosine_correction = _params.use_cosine_correction;
  }
  
  // IF demo_time is set in the params
  // Set into Archive Mode at the indicated time.
  // If demo time param is not set and command line option hasn't set archive mode

  if(_params.start_mode == Params::MODE_REALTIME) {
    
    /* REALTIME MODE */

    _gd.movie.mode = REALTIME_MODE;
    _gd.coord_expt->runtime_mode = RUNMODE_REALTIME;
    _gd.coord_expt->time_seq_num++;
    _gd.forecast_mode = 0;
    
    /* set the first index's time based on current time  */
    time_t clock = time(0);    /* get current time */
    _gd.movie.start_time =
      (time_t) (clock - ((_gd.movie.num_frames -1) * _gd.movie.time_interval_mins * 60.0));
    _gd.movie.start_time -= (_gd.movie.start_time % _gd.movie.round_to_seconds);
    _gd.movie.demo_time = 0; // Indicated REAL-TIME is Native
    
    UTIMunix_to_date(_gd.movie.start_time,&temp_utime);
    _gd.movie.demo_mode = 0;
    _gd.selected_time = clock;
    
  } else {

    /* ARCHIVE MODE */
    
    if(_gd.movie.mode != ARCHIVE_MODE) { /* if not set by command line args */
      
      _gd.movie.mode = ARCHIVE_MODE;     /* time_series */
      _gd.coord_expt->runtime_mode = RUNMODE_ARCHIVE;
      _gd.coord_expt->time_seq_num++;
      
      DateTime startTime;
      startTime.set(_params.archive_start_time);
      _gd.movie.start_time = startTime.utime();
      _gd.selected_time = startTime.utime();
      
    }
    _gd.movie.demo_mode = 1;
	 
    /* Adjust the start time downward to the nearest round interval seconds */

    _gd.movie.start_time -= (_gd.movie.start_time % _gd.movie.round_to_seconds);

    if(_params.gather_data_mode == Params::CLOSEST_TO_FRAME_CENTER) {
      // Offset movie frame by 1/2 frame interval so that interest time
      // lies on frame mid point
      _gd.movie.start_time -=  (time_t) (_gd.movie.time_interval_mins * 30.0);
    }

    _gd.movie.demo_time = _gd.movie.start_time;
    UTIMunix_to_date(_gd.movie.start_time,&temp_utime);
    
    _gd.h_win.movie_page = _gd.h_win.page;
    _gd.v_win.movie_page = _gd.v_win.page;
    _gd.selected_field = _gd.h_win.page;
    
    _gd.movie.cur_frame = 0;

  } // if(strlen(_params.demo_time) < 8 && (_gd.movie.mode != ARCHIVE_MODE))

#ifdef CHECK_LATER

  reset_time_points(); // reset movie

  if(_params.html_mode || _gd.movie.num_frames < 3 ) {
    _params.bot_margin_render_style = _gd.uparams->getLong("cidd.bot_margin_render_style", 1);
  } else {
    _params.bot_margin_render_style = _gd.uparams->getLong("cidd.bot_margin_render_style", 2);
  }
#endif

  /////////////////////////////////////////////////
  // zooms
  
  if (_initZooms()) {
    return -1;
  }
  
  //////////////////////////////////////////
  // heights
  
  _gd.h_win.min_ht = _params.min_ht;
  _gd.h_win.max_ht = _params.max_ht;

  // Otherwise set in the command line arguments

  if(_gd.num_render_heights == 0) {
    _gd.h_win.cur_ht = _params.start_ht;
    _gd.selected_ht = _params.start_ht;
  }

  /////////////////////////////////////////////
  // Fix limits as needed

  if(_gd.h_win.min_x > _gd.h_win.max_x) {
    double tmp = _gd.h_win.min_x;
    _gd.h_win.min_x = _gd.h_win.max_x;
    _gd.h_win.max_x = tmp;
  }
    
  if(_gd.h_win.min_y > _gd.h_win.max_y) {
    double tmp = _gd.h_win.min_y;
    _gd.h_win.min_y = _gd.h_win.max_y;
    _gd.h_win.max_y = tmp;
  }

  ///////////////////////////////////////////////
  // Sanitize Full earth domain limits.
  
  if (_gd.display_projection == Mdvx::PROJ_LATLON) {
    
    if(_gd.h_win.min_x == _gd.h_win.max_x) {
      _gd.h_win.min_x = _gd.h_win.min_x - 180.0;
      _gd.h_win.max_x = _gd.h_win.max_x + 180.0;
    }
    
    if(_gd.h_win.min_x < -360.0) _gd.h_win.min_x = -360.0;
    if(_gd.h_win.max_x > 360.0) _gd.h_win.max_x = 360.0;
    if(_gd.h_win.min_y < -180.0) _gd.h_win.min_y = -180.0;
    if(_gd.h_win.max_y > 180.0) _gd.h_win.max_y = 180.0;
      
    if((_gd.h_win.max_x - _gd.h_win.min_x) > 360.0)  { 
      _gd.h_win.min_x = ((_gd.h_win.max_x + _gd.h_win.min_x) / 2.0) - 180.0;
      _gd.h_win.max_x = _gd.h_win.min_x + 360;
    }
    double originLon = (_gd.h_win.min_x + _gd.h_win.max_x) / 2.0;
    _gd.proj.initLatlon(originLon);

  }
  
  if(_gd.debug) {
    fprintf(stderr,
            " GRID LIMITS:  X: %g,%g   Y: %g,%g \n",
            _gd.h_win.min_x,_gd.h_win.max_x,
            _gd.h_win.min_y,_gd.h_win.max_y );
  }
  
  // init current zoom
  
  _gd.h_win.cmin_x = _gd.h_win.zmin_x[_gd.h_win.zoom_level];
  _gd.h_win.cmax_x = _gd.h_win.zmax_x[_gd.h_win.zoom_level];
  _gd.h_win.cmin_y = _gd.h_win.zmin_y[_gd.h_win.zoom_level];
  _gd.h_win.cmax_y = _gd.h_win.zmax_y[_gd.h_win.zoom_level];

  if(_gd.debug) {
    printf("CUR X: %g,%g   Y: %g,%g,\n",
           _gd.h_win.cmin_x,_gd.h_win.cmax_x,
           _gd.h_win.cmin_y,_gd.h_win.cmax_y);
  }

  // Define a very simple vertical oriented route - In current domain
  
  _gd.h_win.route.num_segments = 1;
  _gd.h_win.route.x_world[0] = (_gd.h_win.cmin_x + _gd.h_win.cmax_x) / 2;
  _gd.h_win.route.x_world[1] = _gd.h_win.route.x_world[0];
  
  _gd.h_win.route.y_world[0] =
    _gd.h_win.cmin_y + ((_gd.h_win.cmax_y - _gd.h_win.cmin_y) / 4);
  _gd.h_win.route.y_world[1] =
    _gd.h_win.cmax_y - ((_gd.h_win.cmax_y - _gd.h_win.cmin_y) / 4);

  _gd.h_win.route.seg_length[0] =
    _dispProjDist(_gd.h_win.route.x_world[0],_gd.h_win.route.y_world[0],
                  _gd.h_win.route.x_world[1],_gd.h_win.route.y_world[1]);

  _gd.h_win.route.total_length = _gd.h_win.route.seg_length[0];

  /* Automatically define the Custom Zoom levels */

  for(int ii = 0; ii <= Constants::NUM_CUSTOM_ZOOMS; ii++) {
    _gd.h_win.zmin_x[_gd.h_win.num_zoom_levels] = _gd.h_win.zmin_x[0] + 
      ((_gd.h_win.zmax_x[0] -_gd.h_win.zmin_x[0]) / ( Constants::NUM_CUSTOM_ZOOMS - ii  + 2.0));
    _gd.h_win.zmax_x[_gd.h_win.num_zoom_levels] = _gd.h_win.zmax_x[0] - 
      ((_gd.h_win.zmax_x[0] -_gd.h_win.zmin_x[0]) / ( Constants::NUM_CUSTOM_ZOOMS - ii  + 2.0));
    _gd.h_win.zmin_y[_gd.h_win.num_zoom_levels] = _gd.h_win.zmin_y[0] + 
      ((_gd.h_win.zmax_y[0] -_gd.h_win.zmin_y[0]) / ( Constants::NUM_CUSTOM_ZOOMS - ii  + 2.0));
    _gd.h_win.zmax_y[_gd.h_win.num_zoom_levels] = _gd.h_win.zmax_y[0] - 
      ((_gd.h_win.zmax_y[0] -_gd.h_win.zmin_y[0]) / ( Constants::NUM_CUSTOM_ZOOMS - ii  + 2.0));
    _gd.h_win.num_zoom_levels++;
  }

  // vertical section
  
  _gd.v_win.zmin_x = (double *) calloc(sizeof(double), 1);
  _gd.v_win.zmax_x = (double *) calloc(sizeof(double), 1);
  _gd.v_win.zmin_y = (double *) calloc(sizeof(double), 1);
  _gd.v_win.zmax_y = (double *) calloc(sizeof(double), 1);

  _gd.v_win.origin_lat = _params.proj_origin_lat;
  _gd.v_win.origin_lon = _params.proj_origin_lon;
  _gd.v_win.min_x = _gd.h_win.min_x;
  _gd.v_win.max_x = _gd.h_win.max_x;
  _gd.v_win.min_y = _gd.h_win.min_y;
  _gd.v_win.max_y = _gd.h_win.max_y;
  _gd.v_win.min_ht = _gd.h_win.min_ht;
  _gd.v_win.max_ht = _gd.h_win.max_ht;

  // Set Vertical window route  params

  _gd.v_win.cmin_x = _gd.h_win.route.x_world[0];
  _gd.v_win.cmin_y = _gd.h_win.route.y_world[0];
  _gd.v_win.cmax_x = _gd.h_win.route.x_world[1];
  _gd.v_win.cmax_y = _gd.h_win.route.y_world[1];

  // Load the GRIDDED DATA FIELD parameters

  int iret = 0;
  if (_initGrids()) {
    iret = -1;
  }

  // initialize wind Rendering

  _gd.layers.wind_vectors = _params.winds_on_at_startup;
  _gd.layers.init_state_wind_vectors = _gd.layers.wind_vectors;
  _gd.layers.wind_mode = WIND_MODE_ON;
  _gd.layers.wind_time_scale_interval = _params.wind_time_scale_interval;
  _gd.layers.wind_scaler = _params.wind_scaler;
  // _gd.legends.range = _params.range_rings;
  // int plot_azimuths = _params.azimuth_lines;
  // _gd.legends.azimuths = plot_azimuths ? AZIMUTH_BIT : 0;

  _initWinds();

  // initialize terrain

  _initTerrain();
  
  // initialize route winds
  
  _initRouteWinds();
  
  // Establish and initialize Draw-Export params 

  _initDrawExport();

  // initialize the map overlays

  if (_initMaps()) {
    iret = -1;
  }
  
  // Instantiate the Station locator classes and params.

  if (_initStationLoc()) {
    iret = -1;
  }

  /////////////////////
  // remote command queue
  
  if(strlen(_params.remote_ui_url) > 1) {
    _gd.remote_ui = new RemoteUIQueue();
    // Create the FMQ with 4096 slots - Total size 1M
    bool compression = false;
    if (_gd.remote_ui->initReadWrite( _params.remote_ui_url,
                                      (char *) "CIDD",
                                      (bool) _gd.debug2,
                                      DsFmq::END, compression,
                                      4096, 4096*256 ) != 0 ) { 
      fprintf(stderr,
              "Problems initialising Remote Command Fmq: %s - aborting\n",
              _params.remote_ui_url);
    }
  }

  // initialize contours

  _initContours();
  
  // initialize overlay fields

  _initOverlayFields();

  // initialize symbolic products

  _initSymprods();

#ifdef NOTNOW
  
  /////////////////////
  // fonts

  if(_params.num_fonts > MAX_FONTS) {
    _params.num_fonts = MAX_FONTS;
    fprintf(stderr,"Cidd: Warning. Too Many Fonts. Limited to %d Fonts\n",MAX_FONTS);
  }

  // Make sure specified font for Winds, Contours and Products are within range.
  if(_gd.prod.prod_font_num < 0) _gd.prod.prod_font_num = 0;
  if(_gd.prod.prod_font_num >= _params.num_fonts) _gd.prod.prod_font_num = _params.num_fonts -1;
  
  for(i=0;i < _params.num_fonts; i++) {
    snprintf(p_name,"cidd.font%d",i+1);
    f_name = _gd.uparams->getString(
            p_name, "fixed");
    _gd.fontst[i] = (XFontStruct *) XLoadQueryFont(dpy,f_name);
    if(_gd.fontst[i] != NULL) {
      _gd.ciddfont[i]  = _gd.fontst[i]->fid;
    } else {
      fprintf(stderr,"Can't load font: %s\n",f_name);
      fprintf(stderr,"Using 'fixed' instead\n");
      _gd.fontst[i]  = (XFontStruct *) XLoadQueryFont(dpy,"fixed");
      _gd.ciddfont[i]  = _gd.fontst[i]->fid;
    }
  }    

#endif

  return iret;
  
}

//////////////////////////////////
// initialize the gridded fields

int Lucid::_initGrids()
{

  int iret = 0;
  
  _gd.num_datafields = _params.fields_n;
  if(_gd.num_datafields <=0) {
    fprintf(stderr,"Lucid requires at least one valid gridded data field to be defined\n");
    exit(-1);
  }
  
  for (int ifld = 0; ifld < _params.fields_n; ifld++) {

    Params::field_t &fld = _params._fields[ifld];
    
    /* get space for data info */
    
    _gd.mread[ifld] = new MdvReader;
    MdvReader *mread = _gd.mread[ifld]; 
    
    mread->legend_name = fld.legend_label;
    mread->button_name = fld.button_label;
    
    if(_params.html_mode == 0 && _params.replace_underscores) {
      /* Replace Underscores with spaces in names */
      for(int jj = (int) mread->button_name.size() -1 ; jj >= 0; jj--) {
        if(mread->button_name[jj] == '_') {
          mread->button_name[jj] = ' ';
        }
      } // jj
      for(int jj = (int) mread->legend_name.size() -1 ; jj >= 0; jj--) {
        if(mread->legend_name[jj] == '_') {
          mread->legend_name[jj] = ' ';
        }
      } // jj
    }
    
    mread->url = fld.url;
    mread->field_label = fld.field_name;
    mread->color_file = fld.color_map;

    // if units are "" or --, set to zero-length string
    if (!strcmp(fld.field_units, "\"\"") || !strcmp(fld.field_units, "--")) {
      mread->field_units.clear();
    } else {
      mread->field_units = fld.field_units;
    }

    mread->cont_low = fld.contour_low;
    mread->cont_high = fld.contour_high;
    mread->cont_interv = fld.contour_interval;

    mread->time_allowance = _gd.movie.mr_stretch_factor * _gd.movie.time_interval_mins;

    if (fld.render_mode == Params::POLYGONS) {
      mread->render_method = POLYGONS;
    } else if (fld.render_mode == Params::FILLED_CONTOURS) {
      mread->render_method = FILLED_CONTOURS;
    } else if (fld.render_mode == Params::DYNAMIC_CONTOURS) {
      mread->render_method = DYNAMIC_CONTOURS;
    } else if (fld.render_mode == Params::LINE_CONTOURS) {
      mread->render_method = LINE_CONTOURS;
    }

    if (fld.composite_mode) {
      mread->composite_mode = TRUE;
    }

    if (fld.auto_scale) {
      mread->auto_scale = TRUE;
    }

    if (fld.display_in_menu) {
      mread->currently_displayed = 1;
    } else {
      mread->currently_displayed = 0;
    }
    
    if(_params.run_once_and_exit) {
      mread->auto_render = 1;
    } else {
      mread->auto_render = fld.auto_render;
    }
    
    mread->last_elev = (char *)NULL;
    mread->elev_size = 0;
    
    mread->plane = 0;
    mread->h_data_valid = 0;
    mread->v_data_valid = 0;
    mread->h_last_scale  = -1.0;
    mread->h_last_bias  = -1.0;
    mread->h_last_missing  = -1.0;
    mread->h_last_bad  = -1.0;
    mread->h_last_transform  = -1;
    mread->v_last_scale  = -1.0;
    mread->v_last_bias  = -1.0;
    mread->v_last_missing  = -1.0;
    mread->v_last_bad  = -1.0;
    mread->v_last_transform  = -1;
    mread->h_fhdr.proj_origin_lat  = 0.0;
    mread->h_fhdr.proj_origin_lon  = 0.0;
    mread->time_list.num_alloc_entries = 0;
    mread->time_list.num_entries = 0;
    
    STRcopy(mread->units_label_cols,"KM",Constants::LABEL_LENGTH);
    STRcopy(mread->units_label_rows,"KM",Constants::LABEL_LENGTH);
    STRcopy(mread->units_label_sects,"KM",Constants::LABEL_LENGTH);

    // instantiate classes
    mread->h_mdvx = new DsMdvx;
    mread->v_mdvx = new DsMdvx;
    mread->h_mdvx_int16 = new MdvxField;
    mread->v_mdvx_int16 = new MdvxField;
    mread->proj = new MdvxProj;

    mread->colorMap = NULL;
    mread->color_file = fld.color_map;
    string colorscaleCachePath;
    if (_getColorscaleCachePath(mread->color_file, colorscaleCachePath)) {
      iret = -1;
    }
    mread->colorMap = new ColorMap(colorscaleCachePath.c_str(),
                                   _params.debug >= Params::DEBUG_EXTRA);
    mread->colorMap->setName(fld.color_map);
    mread->colorMap->setUnits(mread->field_units);
    
  } // ifld
  
  /* Make sure the first field is always on */
  _gd.mread[0]->currently_displayed = 1;

  return iret;

}

//////////////////////////////////
// initialize the wind grids

void Lucid::_initWinds()
{


  if (_params.winds_n == 0) {
    return;
  }

  _gd.layers.num_wind_sets = _params.winds_n;
  if(_gd.layers.num_wind_sets == 0) {
    _gd.layers.wind_vectors = 0;
  }
  
  for(int ii = 0; ii < _gd.layers.num_wind_sets; ii++) {
    
    const Params::wind_t &windp = _params._winds[ii];
    wind_data_t wind;
    _gd.layers.wind.push_back(wind);
    wind_data_t &lwind = _gd.layers.wind[ii];
    
    // marker type
    lwind.marker_type = ARROWS;
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
    STRcopy(lwind.color_name, windp.color, Constants::NAME_LENGTH);
    lwind.active = windp.on_at_startup;
    lwind.line_width = windp.line_width;
    // Sanity check
    if(lwind.line_width == 0 || lwind.line_width > 10) {
      lwind.line_width = 1;
    }

    // initialize the components

    lwind.wind_u = new MdvReader;
    _initWindComponent(lwind.wind_u, windp, true, false, false);

    lwind.wind_v = new MdvReader;
    _initWindComponent(lwind.wind_v, windp, false, true, false);

    if(strncasecmp(windp.w_field_name, "None", 4) != 0) {
      lwind.wind_w = new MdvReader;
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

void Lucid::_initWindComponent(MdvReader *wrec,
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
  
  wrec->legend_name = windp.legend_label;
  if (isW) {
    char text[Constants::NAME_LENGTH];
    snprintf(text, Constants::NAME_LENGTH - 1, "%s_W ", windp.button_label);
    wrec->button_name = text;
  } else {
    wrec->button_name = windp.button_label;
  }
  wrec->url = windp.url;
  
  /* Replace Underscores with spaces in names */
  if(_params.html_mode == 0 && _params.replace_underscores) {
    for(int jj = (int) wrec->button_name.size() - 1; jj >= 0; jj--) {
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
    wrec->url = windp.u_field_name;
  } else if (isV) {
    wrec->url = windp.v_field_name;
  } else {
    wrec->url = windp.w_field_name;
  }
  
  // units
  
  wrec->field_units = windp.units;
  wrec->currently_displayed = 1;
  
  wrec->time_allowance = _gd.movie.mr_stretch_factor * _gd.movie.time_interval_mins;
  wrec->h_fhdr.proj_origin_lon = 0.0;
  wrec->h_fhdr.proj_origin_lat = 0.0;
  
  // instantiate classes for data retrieval
  
  wrec->h_mdvx = new DsMdvx;
  wrec->v_mdvx = new DsMdvx;
  wrec->h_mdvx_int16 = new MdvxField;
  wrec->v_mdvx_int16 = new MdvxField;

  // projection
  
  wrec->proj = new MdvxProj;

}

////////////////////////////////////////////////////////////////
// initialize terrain

void Lucid::_initTerrain()
{

  if (strlen(_params.terrain_url) > 0) {
    
    _gd.layers.earth.terrain_active = 1;
    _gd.layers.earth.terr = new MdvReader;
    if(_gd.layers.earth.terr == NULL) {
      fprintf(stderr,"Cannot allocate space for terrain data\n");
      exit(-1);
    }
    
    _gd.layers.earth.terr->time_allowance = 1000000000; // 30+ years
    _gd.layers.earth.terr->color_file = _params.landuse_colorscale;
    _gd.layers.earth.terr->button_name = _params.terrain_id_label;
    _gd.layers.earth.terr->legend_name = _params.terrain_id_label;
    _gd.layers.earth.terr->url = _params.terrain_url;
    
    _gd.layers.earth.terr->h_mdvx = new DsMdvx;
    _gd.layers.earth.terr->v_mdvx = new DsMdvx;
    _gd.layers.earth.terr->h_mdvx_int16 = new MdvxField;
    _gd.layers.earth.terr->v_mdvx_int16 = new MdvxField;
    _gd.layers.earth.terr->proj =  new MdvxProj;

  }
  
  if (strlen(_params.landuse_url) > 0) {

    _gd.layers.earth.landuse_active = (_params.landuse_active == true)? 1: 0;
    _gd.layers.earth.land_use = new MdvReader;
    if(_gd.layers.earth.land_use == NULL) {
      fprintf(stderr,"Cannot allocate space for land_use data\n");
      exit(-1);
    }
    
    _gd.layers.earth.land_use->time_allowance = 1000000000; // 30+ years
    _gd.layers.earth.land_use->color_file = _params.landuse_colorscale;
    _gd.layers.earth.land_use->button_name = _params.terrain_id_label;
    _gd.layers.earth.land_use->legend_name = _params.terrain_id_label;
    _gd.layers.earth.land_use->url = _params.landuse_url;
    
    _gd.layers.earth.land_use->h_mdvx = new DsMdvx;
    _gd.layers.earth.land_use->v_mdvx = new DsMdvx;
    _gd.layers.earth.land_use->h_mdvx_int16 = new MdvxField;
    _gd.layers.earth.land_use->v_mdvx_int16 = new MdvxField;
    _gd.layers.earth.land_use->proj =  new MdvxProj;
    
    switch(_params.landuse_render_method) {
      default:
      case Params::TERRAIN_RENDER_RECTANGLES:
        _gd.layers.earth.land_use->render_method = POLYGONS;
        break;
        
      case Params::TERRAIN_RENDER_FILLED_CONT:
        _gd.layers.earth.land_use->render_method = FILLED_CONTOURS;
        break;
        
      case Params::TERRAIN_RENDER_DYNAMIC_CONTOURS:
        _gd.layers.earth.land_use->render_method = DYNAMIC_CONTOURS;
        break;
    }
    
  }

}

////////////////////////////////////////////////////////////////
// INIT_DRAW_EXPORT_LINKS:  Scan param file and setup links to
//  for drawn and exported points 

void Lucid::_initDrawExport()
{

  _gd.draw.num_draw_products = _params.draw_export_info_n;
  _gd.draw.dexport = new draw_export_info_t[_gd.draw.num_draw_products];
  if (_gd.draw.dexport == NULL) {
    fprintf(stderr,"Unable to allocate space for %d draw.dexport sets\n",
            _gd.draw.num_draw_products);
    perror("Lucid");
    exit(-1);
  }
  
  for(int ii = 0; ii < _params.draw_export_info_n;  ii++) {
    
    Params::draw_export_t &dinfo = _params._draw_export_info[ii];
    draw_export_info_t &dexp = _gd.draw.dexport[ii];

    // Product ID label
    int len = strlen(dinfo.id_label) + 1;
    dexp.product_id_label = (char *) calloc(1, len);
    STRcopy(dexp.product_id_label, dinfo.id_label, len);
    
    // product_label_text
    dexp.product_label_text = (char *) calloc(1,Constants::TITLE_LENGTH);
    STRcopy(dexp.product_label_text, dinfo.default_label, Constants::TITLE_LENGTH);
    
    // FMQ URL 
    len = strlen(dinfo.url) + 1;
    if(len > Constants::NAME_LENGTH) {
      fprintf(stderr,"URL: %s too long - Must be less than %d chars. Sorry.",
              dinfo.url,Constants::URL_LENGTH);
      exit(-1);
    }
    dexp.product_fmq_url = (char *) calloc(1, Constants::URL_LENGTH);
    STRcopy(dexp.product_fmq_url, dinfo.url, Constants::URL_LENGTH);
    
    // Get the Default valid time  
    dexp.default_valid_minutes = dinfo.valid_minutes;
    
    // Get the Default ID   
    dexp.default_serial_no = dinfo.default_id_no;
    
  } // ii

}

////////////////////////////////////////////////////////////////
// Initialize route winds

void Lucid::_initRouteWinds()
{

  // U WINDS Met Record

  if(strlen(_params.route_u_url) > 1) {

    MdvReader *mr = new MdvReader;
    if(mr == NULL) {
      fprintf(stderr,"Unable to allocate space for Route U Wind\n");
      perror("cidd_init::_initRouteWinds");
      exit(-1);
    }
    _gd.layers.route_wind.u_wind = mr;
    mr->h_data_valid = 0;
    mr->v_data_valid = 0;
    mr->v_vcm.nentries = 0;
    mr->h_vcm.nentries = 0;
    mr->h_fhdr.scale = -1.0;
    mr->h_last_scale = 0.0;
    mr->legend_name = "ROUTE_U_WIND";
    mr->button_name = "ROUTE_U_WIND";
    mr->url = _params.route_u_url;

    mr->field_units = "unknown";
    mr->currently_displayed = 1;
    mr->time_allowance = _gd.movie.mr_stretch_factor * _gd.movie.time_interval_mins;
    mr->h_fhdr.proj_origin_lon = 0.0;
    mr->h_fhdr.proj_origin_lat = 0.0;

    // instantiate DsMdvx class
    mr->v_mdvx = new DsMdvx;
    mr->v_mdvx_int16 = new MdvxField;

  } // U WINDS

  // V WINDS Met Record
  
  if(strlen(_params.route_v_url) > 1) {
    
    MdvReader *mr = new MdvReader;
    if(mr == NULL) {
      fprintf(stderr,"Unable to allocate space for Route V Wind\n");
      perror("cidd_init::_initRouteWinds");
      exit(-1);
    }
    _gd.layers.route_wind.v_wind = mr;
    mr->h_data_valid = 0;
    mr->v_data_valid = 0;
    mr->v_vcm.nentries = 0;
    mr->h_vcm.nentries = 0;
    mr->h_fhdr.scale = -1.0;
    mr->h_last_scale = 0.0;
    mr->legend_name = "ROUTE_V_WIND";
    mr->button_name = "ROUTE_V_WIND";
    mr->url = _params.route_v_url;
    
    mr->field_units = "unknown";
    mr->currently_displayed = 1;
    mr->time_allowance = _gd.movie.mr_stretch_factor * _gd.movie.time_interval_mins;
    mr->h_fhdr.proj_origin_lon = 0.0;
    mr->h_fhdr.proj_origin_lat = 0.0;
    
    // instantiate DsMdvx class
    mr->v_mdvx = new DsMdvx;
    mr->v_mdvx_int16 = new MdvxField;

  } // V WINDS

#ifdef NOTNOW
  
  // TURB Met Record

  if(strlen(_params.route_turb_url) > 1) {

    MdvReader *mr = new MdvReader;
    if(mr == NULL) {
      fprintf(stderr,"Unable to allocate space for Route TURB\n");
      perror("cidd_init::_initRouteWinds");
      exit(-1);
    }
    _gd.layers.route_wind.turb = mr;
    mr->h_data_valid = 0;
    mr->v_data_valid = 0;
    mr->v_vcm.nentries = 0;
    mr->h_vcm.nentries = 0;
    mr->h_fhdr.scale = -1.0;
    mr->h_last_scale = 0.0;
    mr->legend_name = "ROUTE_TURB";
    mr->button_name  = "ROUTE_TURB";
    mr->url = _params.route_turb_url;
    
    mr->field_units = "unknown";
    mr->currently_displayed = 1;
    mr->time_allowance = _gd.movie.mr_stretch_factor * _gd.movie.time_interval_mins;
    mr->h_fhdr.proj_origin_lon = 0.0;
    mr->h_fhdr.proj_origin_lat = 0.0;

    // instantiate DsMdvx class
    mr->v_mdvx = new DsMdvx;
    mr->v_mdvx_int16 = new MdvxField;
    
  } // TURB

  // ICING met Record
  
  if(strlen(_params.route_icing_url) > 1) {

    MdvReader *mr = new MdvReader;
    if(mr == NULL) {
      fprintf(stderr,"Unable to allocate space for Route ICING\n");
      perror("cidd_init::_initRouteWinds");
      exit(-1);
    }
    _gd.layers.route_wind.icing = mr;
    mr->h_data_valid = 0;
    mr->v_data_valid = 0;
    mr->v_vcm.nentries = 0;
    mr->h_vcm.nentries = 0;
    mr->h_fhdr.scale = -1.0;
    mr->h_last_scale = 0.0;
    mr->legend_name = "ROUTE_ICING";
    mr->button_name = "ROUTE_ICING";
    mr->url = _params.route_icing_url;
    mr->field_units = "unknown";
    mr->currently_displayed = 1;
    mr->time_allowance = _gd.movie.mr_stretch_factor * _gd.movie.time_interval_mins;
    mr->h_fhdr.proj_origin_lon = 0.0;
    mr->h_fhdr.proj_origin_lat = 0.0;

    // instantiate DsMdvx class
    mr->v_mdvx = new DsMdvx;
    mr->v_mdvx_int16 = new MdvxField;

  } // ICING

#endif

  // How many are route are defined in the file
  _gd.layers.route_wind.num_predef_routes = _params.route_paths_n;

  // Allocate space for num_predef_routes + 1 for the custom/user defined route

  if((_gd.layers.route_wind.route =(route_track_t *) 
      calloc(_gd.layers.route_wind.num_predef_routes + 1, sizeof(route_track_t))) == NULL) {
    fprintf(stderr,"Unable to allocate space for %d Routes\n",
            _gd.layers.route_wind.num_predef_routes + 1);
    perror("CIDD route_winds_init");
    exit(-1);
  }
  
  char *cfield[Constants::NUM_PARSE_FIELDS];
  for(int ii = 0; ii < Constants::NUM_PARSE_FIELDS; ii++) {
    cfield[ii] =(char *) calloc(Constants::PARSE_FIELD_SIZE, 1);
  }
  
  for(int ii = 0; ii < _gd.layers.route_wind.num_predef_routes; ii++) {
    
    int num_fields = STRparse(_params._route_paths[ii], cfield,
                              strlen(_params._route_paths[ii]),
                              Constants::NUM_PARSE_FIELDS, Constants::PARSE_FIELD_SIZE);
    if(num_fields == Constants::NUM_PARSE_FIELDS) {
      fprintf(stderr,"Warning: Route path: %s\n Too long. Only %d segments allowed \n",
              _params._route_paths[ii], Constants::MAX_ROUTE);
    }
    
    // Collect Label
    strncpy(_gd.layers.route_wind.route[ii].route_label, cfield[0], 62);
    
    // Collect the number of points  & segments 
    _gd.layers.route_wind.route[ii].num_segments = atoi(cfield[1]) -1;
    if(_params.route_debug) {
      fprintf(stderr,"\nRoute: %s - %d segments\n",
              _gd.layers.route_wind.route[ii].route_label,
              _gd.layers.route_wind.route[ii].num_segments);
    }

    // Sanity check
    if(_gd.layers.route_wind.route[ii].num_segments <= 0 || 
       _gd.layers.route_wind.route[ii].num_segments >  Constants::MAX_ROUTE) {
      fprintf(stderr,"Warning: Route path: %s\n Error Only 1-%d segments allowed \n",
              _params._route_paths[ii], Constants::MAX_ROUTE);
      continue;
    }
    
    int index = 2; // The first triplet.
    // Pick up each triplet
    for(int kk = 0; kk <= _gd.layers.route_wind.route[ii].num_segments; kk++, index+=3 ) {

      strncpy(_gd.layers.route_wind.route[ii].navaid_id[kk], cfield[index], 16);
      _gd.layers.route_wind.route[ii].y_world[kk] = atof(cfield[index +1]);
      _gd.layers.route_wind.route[ii].x_world[kk] = atof(cfield[index +2]);

      switch (_gd.display_projection) {
        case  Mdvx::PROJ_LATLON:
          _normalizeLongitude(_gd.h_win.min_x, _gd.h_win.max_x,
                              &_gd.layers.route_wind.route[ii].x_world[kk]);
          break;

        default :
          _normalizeLongitude(-180.0, 180.0,
                              &_gd.layers.route_wind.route[ii].x_world[kk]);
          break;

      } 

      _gd.proj.latlon2xy(_gd.layers.route_wind.route[ii].y_world[kk],
                         _gd.layers.route_wind.route[ii].x_world[kk],
                         _gd.layers.route_wind.route[ii].x_world[kk],
                         _gd.layers.route_wind.route[ii].y_world[kk]);

      if(_params.route_debug) {
        fprintf(stderr,"%s:  %g,    %g\n",
                _gd.layers.route_wind.route[ii].navaid_id[kk],
                _gd.layers.route_wind.route[ii].x_world[kk],
                _gd.layers.route_wind.route[ii].y_world[kk]);
      }
    }
    
    // Compute the segment lengths
    _gd.layers.route_wind.route[ii].total_length = 0.0;
    for(int kk = 0; kk < _gd.layers.route_wind.route[ii].num_segments; kk++) {
      _gd.layers.route_wind.route[ii].seg_length[kk] = 
        _dispProjDist(_gd.layers.route_wind.route[ii].x_world[kk],
                      _gd.layers.route_wind.route[ii].y_world[kk],
                      _gd.layers.route_wind.route[ii].x_world[kk+1],
                      _gd.layers.route_wind.route[ii].y_world[kk+1]);
      _gd.layers.route_wind.route[ii].total_length +=
        _gd.layers.route_wind.route[ii].seg_length[kk];
    }
    
  } // ii

  // Copy the initial route definition into the space reserved for the Custom route
  memcpy(_gd.layers.route_wind.route + _gd.layers.route_wind.num_predef_routes,
         &_gd.h_win.route,sizeof(route_track_t));

  /* free temp space */
  for(int ii = 0; ii < Constants::NUM_PARSE_FIELDS; ii++) {
    free(cfield[ii]);
  }
  
  if (_params.route_winds_active) {
    _gd.layers.route_wind.has_params = 1;
    // Use the first route as the default.
    memcpy(&_gd.h_win.route, _gd.layers.route_wind.route, sizeof(route_track_t)); 
  } else {
    _gd.layers.route_wind.has_params = 0;
  }

}

/************************************************************************
 * INIT_MAPS - read in map files
 */ 

int Lucid::_initMaps()

{

  int iret = 0;
  _gd.num_map_overlays = _params.maps_n;
  
  for (int ii = 0; ii < _params.maps_n; ii++) {
    
    Params::map_t &omap = _params._maps[ii];
    string mapFileName = omap.map_file_name;
    MapOverlay_t *over = new MapOverlay_t;
    _gd.overlays.push_back(over);

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

      string cachePathShp, cachePathShx;
      if (_getMapCachePath(mapFileName, cachePathShp, cachePathShx)) {
        iret = -1;
      } else if (_loadShapeMap(over, cachePathShp, cachePathShx)) {
        iret = -1;
      }
      
    } else {  // Assume RAP Map Format 
      
      string cachePath, dummy;
      if (_getMapCachePath(mapFileName, cachePath, dummy)) {
        iret = -1;
      } else if (_loadRapMap(over, cachePath)) {
        iret = -1;
      }
      
    }
    
    if(_gd.debug) {
      printf("Overlay file %s contains %ld polylines, %ld icon_defns, %ld icons, %ld labels\n",
             over->map_file_name.c_str(),
             over->num_polylines,
             over->num_icondefs,
             over->num_icons,
             over->num_labels);
    }
    
  } // ii

  // calculate the map coords for specified projection
  
  if (_gd.display_projection == Mdvx::PROJ_LATLON) {
    _calcMapCoordsLatLon();
  } else {
    _calcMapCoordsProj();
  }

  return iret;
  
}

/************************************************************************
 * LOAD_RAP_MAP - load map in RAP format
 */

int Lucid::_loadRapMap(MapOverlay_t *ov, const string &mapFilePath)
{

  int i,j;
  int index,found;
  int len,point;
  int num_points;        
  int num_fields;  /* number of fields (tokens) found in input line */
  // int ret_stat;
  char name_buf[2048];  /* Buffer for input lines */
  char *lasts;

  // read map file into buffer

  char *map_buf;
  int map_len;
  if (_readFileIntoBuffer(mapFilePath, map_buf, map_len)) {
    cerr << "ERROR - _loadRapMap, cannot load map file: " << mapFilePath << endl;
    return -1;
  }

  // alloc space for sub strings
  
  char *cfield[32];
  for(i=0; i < 32; i++) {
    cfield[i] = (char *) calloc(1,64);  /* get space for sub strings */
  }
  
  // Prime strtok_r;

  char *str_ptr = strtok_r(map_buf, "\n", &lasts);

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
        MEM_zero_ptr(ov->geo_icondef[index]);

        if(ov->geo_icondef[index] == NULL) {
          fprintf(stderr,"Unable to allocate memory for Icon definition!\n");
          exit(-1);
        }
        STRcopy(ov->geo_icondef[index]->name,cfield[1],Constants::NAME_LENGTH);
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
        MEM_zero_ptr(ov->geo_icon[index]);

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
        for(j = 6; j < num_fields && len < Constants::LABEL_LENGTH; j++ ) {
          strncat(ov->geo_icon[index]->label,cfield[j],Constants::LABEL_LENGTH - len);
          len = strlen(ov->geo_icon[index]->label) +1;

          // Separate multiple text label fiedds with spaces.
          if( j < num_fields -1) {
            strncat(ov->geo_icon[index]->label," ",Constants::LABEL_LENGTH - len);
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
        MEM_zero_ptr(ov->geo_polyline[index]);

        if(ov->geo_polyline[index] == NULL) {
          fprintf(stderr,"Unable to allocate memory for Polyline definition!\n");
          exit(-1);
        }
        STRcopy(ov->geo_polyline[index]->label,cfield[1],Constants::LABEL_LENGTH);
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
        ov->geo_polyline[index]->proj_x = (double *) calloc(1,num_points * sizeof(double));
        ov->geo_polyline[index]->proj_y = (double *) calloc(1,num_points * sizeof(double));

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
        MEM_zero_ptr(ov->geo_label[index]);

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
        for(j = 8; j < num_fields && len < Constants::NAME_LENGTH; j++) {
          strncat(ov->geo_label[index]->display_string,cfield[j],Constants::NAME_LENGTH - len);
          len = strlen(ov->geo_label[index]->display_string) +1;
          if(j < num_fields -1)
            strncat(ov->geo_label[index]->display_string," ",Constants::NAME_LENGTH - len);
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
        MEM_zero_ptr(ov->geo_label[index]);

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
        for(j = 3; j < num_fields && len < Constants::NAME_LENGTH; j++) {
          strncat(ov->geo_label[index]->display_string,cfield[j],Constants::NAME_LENGTH - len);
          len = strlen(ov->geo_label[index]->display_string) +1;
          if(j < num_fields -1)
            strncat(ov->geo_label[index]->display_string," ",Constants::NAME_LENGTH - len);
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

  return 0;

}

/************************************************************************
 * LOAD_SHAPE_OVERLAY_DATA: This version reads Shape files
 */

int Lucid::_loadShapeMap(MapOverlay_t *ov, const string &shpFilePath, const string &shxFilePath)
{

  // open shape file

  SHPHandle SH;
  if((SH = SHPOpen(shpFilePath.c_str(), "rb")) == NULL) {
    cerr << "ERROR - _loadShapeMap, cannot open shape file: " << shpFilePath << endl;
    return -1;
  }

  int i,j;
  int index;
  int point;
  int num_points;        
  
  int n_objects = 0;
  int shape_type = 0;
  int part_num = 0;
  
  SHPGetInfo(SH, &n_objects, &shape_type, NULL, NULL);

  if(_gd.debug) {
    fprintf(stderr,"Found %d objects, type %d  in %s\n",n_objects, shape_type, ov->map_file_name.c_str());
  }

  for(i=0; i < n_objects; i++ ) {  // Loop through each object

    SHPObject *SO = SHPReadObject(SH,i);    // Load the shape object

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
        MEM_zero_ptr(ov->geo_polyline[index]);
        if(ov->geo_polyline[index] == NULL) {
          fprintf(stderr,"Unable to allocate memory for Polyline definition!\n");
          exit(-1);
        }

        STRcopy(ov->geo_polyline[index]->label,"Shape",Constants::LABEL_LENGTH);

        /* Get space for points in the polyline */
        ov->geo_polyline[index]->lat = (double *) calloc(1,(SO->nVertices + SO->nParts) * sizeof(double));
        ov->geo_polyline[index]->lon = (double *) calloc(1,(SO->nVertices + SO->nParts) * sizeof(double));
        ov->geo_polyline[index]->proj_x = (double *) calloc(1,(SO->nVertices + SO->nParts) * sizeof(double));
        ov->geo_polyline[index]->proj_y = (double *) calloc(1,(SO->nVertices + SO->nParts) * sizeof(double));

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
          MEM_zero_ptr(ov->geo_icondef[0]);

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
        MEM_zero_ptr(ov->geo_icon[index]);

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
        if(_gd.debug) {
          fprintf(stderr,"Encountered Unsupported Shape type %d\n",SO->nSHPType);
        }
        break;
    }

    if(SO != NULL) SHPDestroyObject(SO);
  }  // End of each object

  return 0;
  
}

#define KM_CLIP_BUFFER 25.0
#define LATLON_CLIP_BUFFER 0.4

///////////////////////////////////////////////////////////////////
// calculate map coords in XY space for the specified projection

void Lucid::_calcMapCoordsProj()
{

  // condition longitudes to be in the same hemisphere as the origin
  _gd.proj.setConditionLon2Origin(true);
  
  // Compute the bounding box
  double max_lon = -360.0;
  double min_lon = 360.0;
  double max_lat = -180.0;
  double min_lat = 180.0;
  
  // Check each corner of the projection + 2 center points, top, bottom
  // Lower left
  double lat,lon;
  _gd.proj.xy2latlon(_gd.h_win.min_x - KM_CLIP_BUFFER ,
                     _gd.h_win.min_y - KM_CLIP_BUFFER, lat, lon);
  if(lon > max_lon) max_lon = lon;
  if(lon < min_lon) min_lon = lon;
  if(lat > max_lat) max_lat = lat;
  if(lat < min_lat) min_lat = lat;
  
  // Lower midpoint
  _gd.proj.xy2latlon((_gd.h_win.min_x +_gd.h_win.max_x)/2 - KM_CLIP_BUFFER,
                     _gd.h_win.min_y - KM_CLIP_BUFFER, lat, lon);
  if(lon > max_lon) max_lon = lon;
  if(lon < min_lon) min_lon = lon;
  if(lat > max_lat) max_lat = lat;
  if(lat < min_lat) min_lat = lat;
  
  // Lower right
  _gd.proj.xy2latlon(_gd.h_win.max_x - KM_CLIP_BUFFER,
                     _gd.h_win.min_y - KM_CLIP_BUFFER, lat, lon);
  if(lon > max_lon) max_lon = lon;
  if(lon < min_lon) min_lon = lon;
  if(lat > max_lat) max_lat = lat;
  if(lat < min_lat) min_lat = lat;
  
  // Upper right
  _gd.proj.xy2latlon(_gd.h_win.max_x + KM_CLIP_BUFFER,
                     _gd.h_win.max_y + KM_CLIP_BUFFER, lat, lon);
  if(lon > max_lon) max_lon = lon;
  if(lon < min_lon) min_lon = lon;
  if(lat > max_lat) max_lat = lat;
  if(lat < min_lat) min_lat = lat;
  
  // Upper midpoint
  _gd.proj.xy2latlon((_gd.h_win.min_x + _gd.h_win.max_x)/2 - KM_CLIP_BUFFER,
                     _gd.h_win.max_y - KM_CLIP_BUFFER, lat, lon);
  if(lon > max_lon) max_lon = lon;
  if(lon < min_lon) min_lon = lon;
  if(lat > max_lat) max_lat = lat;
  if(lat < min_lat) min_lat = lat;
  
  // Upper left
  _gd.proj.xy2latlon(_gd.h_win.min_x + KM_CLIP_BUFFER,
                     _gd.h_win.max_y + KM_CLIP_BUFFER, lat, lon);
  if(lon > max_lon) max_lon = lon;
  if(lon < min_lon) min_lon = lon;
  if(lat > max_lat) max_lat = lat;
  if(lat < min_lat) min_lat = lat;
  
  //  Handle pathalogical cases where edges extend around the world.
  if(min_lat >= max_lat ||
     min_lon >= max_lon ||
     _gd.proj.getProjType() == Mdvx::PROJ_POLAR_STEREO ||
     _gd.proj.getProjType() == Mdvx::PROJ_OBLIQUE_STEREO) {
    min_lon = -360.0;
    min_lat = -180.0;
    max_lon = 360.0;
    max_lat = 180.0;
  }
  
  if(_gd.debug) {
    printf("----> Overlay lon,lat Clip box: %g, %g to  %g, %g\n",
           min_lon,min_lat,max_lon,max_lat);
  }
  
  for(int ii=0; ii < _gd.num_map_overlays; ii++) {        /* For Each Overlay */
    MapOverlay_t *ov =  _gd.overlays[ii];
    if(_gd.debug)  printf("Converting map %s\n", ov->control_label.c_str());
    
    /* Convert all labels   */
    for(int jj=0; jj < ov->num_labels; jj++) { 
      int clip_flag = 0;
      if(ov->geo_label[jj]->min_lat < min_lat ||
         ov->geo_label[jj]->min_lat > max_lat) clip_flag = 1;
      if(ov->geo_label[jj]->min_lon < min_lon ||
         ov->geo_label[jj]->min_lon > max_lon) clip_flag = 1;
      
      if(clip_flag) {
        ov->geo_label[jj]->proj_x = -32768.0;
      } else {
        /* Current rendering only uses min_lat, min_lon to position text */
        _gd.proj.latlon2xy(ov->geo_label[jj]->min_lat, ov->geo_label[jj]->min_lon,
                           ov->geo_label[jj]->proj_x, ov->geo_label[jj]->proj_y);
      }
      
    }
    
    /* Convert all Iconic Objects */
    for(int jj=0; jj < ov->num_icons; jj++) {
      Geo_feat_icon_t *ic = ov->geo_icon[jj];
      int clip_flag = 0;
      if(ic->lat < min_lat || ic->lat > max_lat) clip_flag = 1;
      if(ic->lon < min_lon || ic->lon > max_lon) clip_flag = 1;
      
      if(clip_flag) {
        ic->proj_x = -32768.0;
      } else {
        _gd.proj.latlon2xy( ic->lat,ic->lon, ic->proj_x,ic->proj_y);
      }
    }
    
    /* Convert all Poly Line Objects */

    double min_loc_x, max_loc_x;
    double min_loc_y, max_loc_y;

    for(int jj=0; jj < ov->num_polylines; jj++) { 
      Geo_feat_polyline_t *poly = ov->geo_polyline[jj];
      /* Reset the bounding box limits */
      min_loc_x = DBL_MAX;
      max_loc_x = DBL_MIN;
      min_loc_y = DBL_MAX;
      max_loc_y = DBL_MIN;
      // npoints = 0;
      for(int ll=0; ll < poly->num_points; ll++) {
        int clip_flag = 0;
        if(poly->lat[ll] < min_lat || poly->lat[ll] > max_lat)
          clip_flag = 1;
        if(poly->lon[ll] < min_lon || poly->lon[ll] > max_lon)
          clip_flag = 1;
        
        if(poly->lon[ll] > -360.0 && clip_flag == 0 ) {
          _gd.proj.latlon2xy(poly->lat[ll], poly->lon[ll],
                             poly->proj_x[ll], poly->proj_y[ll]);
          
          /* printf("LAT,LON: %10.6f, %10.6f   LOCAL XY: %10.6f, %10.6f\n",
             poly->lat[l],poly->lon[l], poly->proj_x[l],poly->proj_y[l]); */
          
          /* Gather the bounding box for this polyline in local coords */
          if(poly->proj_x[ll] < min_loc_x) min_loc_x = poly->proj_x[ll];
          if(poly->proj_y[ll] < min_loc_y) min_loc_y = poly->proj_y[ll];
          if(poly->proj_x[ll] > max_loc_x) max_loc_x = poly->proj_x[ll];
          if(poly->proj_y[ll] > max_loc_y) max_loc_y = poly->proj_y[ll];
          
        } else {
          poly->proj_x[ll] = -32768.0;
          poly->proj_y[ll] = -32768.0;
        }
      }
      /* Set the bounding box */
      poly->min_x = min_loc_x;
      poly->min_y = min_loc_y;
      poly->max_x = max_loc_x;
      poly->max_y = max_loc_y;
    }
  }

}

///////////////////////////////////////////////////////////////////
// calculate map coords in XY space for latlon projection

void Lucid::_calcMapCoordsLatLon()
{

  double min_lon = _gd.h_win.min_x - LATLON_CLIP_BUFFER;
  double min_lat = _gd.h_win.min_y - LATLON_CLIP_BUFFER;
  double max_lon = _gd.h_win.max_x + LATLON_CLIP_BUFFER;
  double max_lat = _gd.h_win.max_y + LATLON_CLIP_BUFFER;
  
  for(int ii=0; ii < _gd.num_map_overlays; ii++) {        /* For Each Overlay */
    MapOverlay_t *ov =  _gd.overlays[ii];
    if(_gd.debug)  printf("Converting map %s\n", ov->control_label.c_str());
    
    for(int jj=0; jj < ov->num_labels; jj++) {        /* Convert all labels   */
      int clip_flag = 0;
      
      _normalizeLongitude(_gd.h_win.min_x, _gd.h_win.max_x, &ov->geo_label[jj]->min_lon);
      
      if(ov->geo_label[jj]->min_lat < min_lat ||
         ov->geo_label[jj]->min_lat > max_lat) clip_flag = 1;
      
      if(ov->geo_label[jj]->min_lon < min_lon ||
         ov->geo_label[jj]->min_lon > max_lon) clip_flag = 1;
      
      if(clip_flag) {
        ov->geo_label[jj]->proj_x = -32768.0;
      } else {
        ov->geo_label[jj]->proj_x = ov->geo_label[jj]->min_lon;
        ov->geo_label[jj]->proj_y = ov->geo_label[jj]->min_lat;
      }
      
    }
    
    /* Convert all Iconic Objects */
    for(int jj=0; jj < ov->num_icons; jj++) {
      Geo_feat_icon_t *ic = ov->geo_icon[jj];
      int clip_flag = 0;
      
      _normalizeLongitude(_gd.h_win.min_x, _gd.h_win.max_x, &ic->lon);
      
      if(ic->lat < min_lat || ic->lat > max_lat) clip_flag = 1;
      if(ic->lon < min_lon || ic->lon > max_lon) clip_flag = 1;
      
      if(clip_flag) {
        ic->proj_x = -32768.0;
      } else {
        ic->proj_x = ic->lon;
        ic->proj_y = ic->lat;
      }
    }
    
    /* Convert all Poly Line Objects */

    double min_loc_x, max_loc_x;
    double min_loc_y, max_loc_y;

    for(int jj=0; jj < ov->num_polylines; jj++) {
      Geo_feat_polyline_t *poly = ov->geo_polyline[jj];
      // npoints = 0;
      /* Reset the bounding box limits */
      min_loc_x = DBL_MAX;
      max_loc_x = DBL_MIN;
      min_loc_y = DBL_MAX;
      max_loc_y = DBL_MIN;
      for(int l=0; l < poly->num_points; l++) {
        int clip_flag = 0;
        
        _normalizeLongitude(_gd.h_win.min_x, _gd.h_win.max_x, &poly->lon[l]);
	
        if(poly->lat[l] < min_lat || poly->lat[l] > max_lat) clip_flag = 1;
        if(poly->lon[l] < min_lon || poly->lon[l] > max_lon) clip_flag = 1;
        
        if (l > 0 && fabs(poly->lon[l] - poly->lon[l-1]) > 330) {
          clip_flag = 1;
        }
        
        if(poly->lon[l] > -360.0 && clip_flag == 0 ) {
          
          poly->proj_x[l] = poly->lon[l];
          poly->proj_y[l] = poly->lat[l];
          
          /* Gather the bounding box for this polyline in local coords */
          if(poly->proj_x[l] < min_loc_x) min_loc_x = poly->proj_x[l];
          if(poly->proj_y[l] < min_loc_y) min_loc_y = poly->proj_y[l];
          if(poly->proj_x[l] > max_loc_x) max_loc_x = poly->proj_x[l];
          if(poly->proj_y[l] > max_loc_y) max_loc_y = poly->proj_y[l];
          
        } else {
          poly->proj_x[l] = -32768.0;
          poly->proj_y[l] = -32768.0;
        }
      }
      /* Set the bounding box */
      poly->min_x = min_loc_x;
      poly->min_y = min_loc_y;
      poly->max_x = max_loc_x;
      poly->max_y = max_loc_y;
    }
  }

}

///////////////////////////////////////////////////////
// Instantiate the Station locator classes and params.
  
int Lucid::_initStationLoc()

{

  if (strlen(_params.station_loc_url) < 1) {
    return 0;
  }
    
  if(_gd.debug || _gd.debug1) {
    fprintf(stderr,"Loading Station data from: %s\n", _params.station_loc_url);
  }
  _gd.station_loc =  new StationLoc();
  if(_gd.station_loc == NULL) {
    fprintf(stderr,"CIDD: Fatal alloc constructing new stationLoc()\n");
    exit(-1);
  }

  // download station location file into cache
  
  Path locPath(_params.station_loc_url);
  string stationlocCachePath;
  if (_getResourceCachePath(_gd.stationlocCacheDir, locPath.getDirectory(),
                            locPath.getFile(), stationlocCachePath)) {
    fprintf(stderr, "CIDD: Can't find Station Data from %s\n", _params.station_loc_url);
    return -1;
  }
  
  if(_gd.station_loc->ReadData(stationlocCachePath.c_str()) < 0) {
    fprintf(stderr, "CIDD: Can't load Station Data from cache path %s\n", stationlocCachePath.c_str());
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _gd.station_loc->PrintAll();
  }

  return 0;
  
}

/////////////////////////////////////////////////
// zooms

int Lucid::_initZooms()

{
  
  if (_params.zoom_levels_n < 1) {
    cerr << "ERROR - no zoom levels specified" << endl;
    cerr << "  This must be corrected in the params file: " << _paramsPathUsed.getPath() << endl;
    return -1;
  }

  if(_params.num_cache_zooms > Constants::MAX_CACHE_PIXMAPS) {
    _params.num_cache_zooms = Constants::MAX_CACHE_PIXMAPS;
  }
  if(_params.num_cache_zooms < 1) {
    _params.num_cache_zooms = 1 ;
  }
  
  _gd.h_win.can_pdev = new QPixmap*[_params.num_cache_zooms];
  _gd.v_win.can_pdev = new QPixmap*[_params.num_cache_zooms];
  
  _gd.h_win.num_zoom_levels = _params.zoom_levels_n;

  _gd.h_win.start_zoom_level = 0;
  if (!_params.html_mode && _params.zoom_levels_n > 0) {
    bool zoomLabelFound = false;
    for (int ii = 0; ii < _params.zoom_levels_n; ii++) {
      if (strcmp(_params._zoom_levels[ii].label, _params.start_zoom_label) == 0) {
        _gd.h_win.start_zoom_level = ii;
        zoomLabelFound = true;
      }
    } // ii
    if (!zoomLabelFound) {
      cerr << "WARNING - start zoom label not found: " << _params.start_zoom_label << endl;
      cerr << "  Using first zoom level instead: " << _params._zoom_levels[0].label << endl;
    }
  } // if (!_params.html_mode)
  _gd.h_win.zoom_level = _gd.h_win.start_zoom_level;
  _gd.h_win.prev_zoom_level = _gd.h_win.zoom_level;
  
  _gd.h_win.zmin_x =
    (double *) calloc(sizeof(double),_gd.h_win.num_zoom_levels+Constants::NUM_CUSTOM_ZOOMS + 1);
  _gd.h_win.zmax_x =
    (double *) calloc(sizeof(double),_gd.h_win.num_zoom_levels+Constants::NUM_CUSTOM_ZOOMS + 1);
  _gd.h_win.zmin_y =
    (double *) calloc(sizeof(double),_gd.h_win.num_zoom_levels+Constants::NUM_CUSTOM_ZOOMS + 1);
  _gd.h_win.zmax_y =
    (double *) calloc(sizeof(double),_gd.h_win.num_zoom_levels+Constants::NUM_CUSTOM_ZOOMS + 1);

  if (_gd.display_projection == Mdvx::PROJ_LATLON) {
    _gd.h_win.min_x = max(_params.domain_limit_min_x, -360.0);
    _gd.h_win.max_x = min(_params.domain_limit_max_x, 360.0);
    _gd.h_win.min_y = max(_params.domain_limit_min_y, -90.0);
    _gd.h_win.max_y = min(_params.domain_limit_max_y, 90.0);
  } else {
    _gd.h_win.min_x = _params.domain_limit_min_x;
    _gd.h_win.max_x = _params.domain_limit_max_x;
    _gd.h_win.min_y = _params.domain_limit_min_y;
    _gd.h_win.max_y = _params.domain_limit_max_y;
  }

  for(int izoom = 0; izoom < _gd.h_win.num_zoom_levels; izoom++) {
    
    double minx = _params._zoom_levels[izoom].min_x;
    double miny = _params._zoom_levels[izoom].min_y;
    double maxx = _params._zoom_levels[izoom].max_x;
    double maxy = _params._zoom_levels[izoom].max_y;
    
    // convert from latlon if needed

    if (_params.zoom_limits_in_latlon && _gd.proj.getProjType() != Mdvx::PROJ_LATLON) {
      
      double minLon = minx;
      double maxLon = maxx;
      double minLat = miny;
      double maxLat = maxy;

      _gd.proj.latlon2xy(minLat, minLon, minx, miny);
      _gd.proj.latlon2xy(maxLat, maxLon, maxx, maxy);
      
      if(_gd.debug) {
        cerr << "Zoom number: " << (izoom + 1) << endl;
        cerr << "  converting lat/lon to km" << endl;
        cerr << "  minLon, minLat: " << minLon << ", " << minLat << endl;
        cerr << "  maxLon, maxLat: " << maxLon << ", " << maxLat << endl;
        cerr << "  minXkm, minYkm: " << minx << ", " << miny << endl;
        cerr << "  maxXkm, maxYkm: " << maxx << ", " << maxy << endl;
      }

    }
    
    _gd.h_win.zmin_x[izoom] = minx;
    _gd.h_win.zmin_y[izoom] = miny;
    _gd.h_win.zmax_x[izoom] = maxx;
    _gd.h_win.zmax_y[izoom] = maxy;
    
    double delta_x = _gd.h_win.zmax_x[izoom] - _gd.h_win.zmin_x[izoom];
    double delta_y = _gd.h_win.zmax_y[izoom] - _gd.h_win.zmin_y[izoom];

    double max_delta_x = _gd.h_win.max_x - _gd.h_win.min_x;
    double max_delta_y = _gd.h_win.max_y - _gd.h_win.min_y;
  
    if (delta_x > max_delta_x) {
      delta_x = max_delta_x;
    }
    if (delta_y > max_delta_y) {
      delta_y = max_delta_y;
    }

    // trap bogus values

    if(_gd.h_win.zmin_x[izoom] < _gd.h_win.min_x) {
      _gd.h_win.zmin_x[izoom] = _gd.h_win.min_x;
      _gd.h_win.zmax_x[izoom] =  _gd.h_win.min_x + delta_x;
    }

    if(_gd.h_win.zmin_y[izoom] < _gd.h_win.min_y) {
      _gd.h_win.zmin_y[izoom] = _gd.h_win.min_y;
      _gd.h_win.zmax_y[izoom] =  _gd.h_win.min_y + delta_y;
    }

    if(_gd.h_win.zmax_x[izoom] > _gd.h_win.max_x) {
      _gd.h_win.zmax_x[izoom] = _gd.h_win.max_x;
      _gd.h_win.zmin_x[izoom] =  _gd.h_win.max_x - delta_x;
    }

    if(_gd.h_win.zmax_y[izoom] > _gd.h_win.max_y) {
      _gd.h_win.zmax_y[izoom] = _gd.h_win.max_y;
      _gd.h_win.zmin_y[izoom] =  _gd.h_win.max_y - delta_y;
    }

    // if(_params.horiz_aspect_ratio <= 0.0) {
    //   _params.horiz_aspect_ratio = fabs(delta_x/delta_y);
    // }
    
    _gd.aspect_correction =
      cos(((_gd.h_win.zmax_y[izoom] + _gd.h_win.zmin_y[izoom])/2.0) * Constants::LUCID_DEG_TO_RAD);

    /* Make sure domains are consistant with the window aspect ratio */

    if (_gd.display_projection == Mdvx::PROJ_LATLON) {
      /* forshorten the Y coords to make things look better */
      delta_y /= _gd.aspect_correction;
    }
    // delta_x /= _params.aspect_ratio;

    if(delta_x > delta_y) {
      _gd.h_win.zmax_y[izoom] += ((delta_x - delta_y) /2.0) ;
      _gd.h_win.zmin_y[izoom] -= ((delta_x - delta_y) /2.0) ;
    } else {
      _gd.h_win.zmax_x[izoom] += ((delta_y - delta_x) /2.0) ;
      _gd.h_win.zmin_x[izoom] -= ((delta_y - delta_x) /2.0) ;
    }
    
    if(_gd.debug) {
      printf(" ZOOM: %d --  X: %g,%g   Y: %g,%g,  Delta: %g,%g\n", izoom,
             _gd.h_win.zmin_x[izoom],_gd.h_win.zmax_x[izoom],
             _gd.h_win.zmin_y[izoom],_gd.h_win.zmax_y[izoom],
             delta_x,delta_y);
    }
    
  } // izoom

  return 0;

}

/////////////////////////////
// initialize contours

void Lucid::_initContours()

{

  for(int ii = 0; ii < Constants::NUM_CONT_LAYERS; ii++) {
    _gd.layers.cont[ii].field = 0;
    _gd.layers.cont[ii].min = _gd.mread[0]->cont_low;
    _gd.layers.cont[ii].max = _gd.mread[0]->cont_high;
    _gd.layers.cont[ii].interval = _gd.mread[0]->cont_interv;
    _gd.layers.cont[ii].labels_on  = _params.label_contours;
  }
  for(int ii = 0; ii < Constants::NUM_GRID_LAYERS; ii++) {
    _gd.layers.overlay_field[ii] = 0;
  }

  for(int ii = 0; ii < _gd.num_datafields; ii++) {
    _gd.h_win.redraw_flag[ii] = 1;
    _gd.v_win.redraw_flag[ii] = 1;
  }

  for(int ii = 0; ii < _params.contour_fields_n; ii++ ) {
    
    Params::contour_field_t &cfield = _params._contour_fields[ii];
    
    strncpy(_gd.layers.cont[ii].color_name, cfield.color, Constants::NAME_LENGTH);
    
    /* Replace underscores with spaces in contour field names */
    char *contourFieldName = cfield.field_name;
    if(_params.html_mode == 0 && _params.replace_underscores) {
      for(int jj = strlen(contourFieldName) - 1; jj >= 0; jj--) {
        if (contourFieldName[jj] == '_') {
          contourFieldName[jj] = ' ';
        }
      } // jj
    }
    
    for (int jj = 0; jj < _gd.num_datafields; jj++) {
      if (_gd.mread[jj]->button_name.compare(contourFieldName) == 0) {
        _gd.layers.cont[ii].field = jj;
        if(cfield.on_at_startup) {
          _gd.layers.cont[ii].active = 1;
        } else {
          _gd.layers.cont[ii].active = 0;
        }
        _gd.layers.cont[ii].min = _gd.mread[jj]->cont_low;
        _gd.layers.cont[ii].max = _gd.mread[jj]->cont_high;
        _gd.layers.cont[ii].interval = _gd.mread[jj]->cont_interv;
        strncpy(_gd.layers.cont[ii].color_name, cfield.color, Constants::NAME_LENGTH);
        break;
      }
      
    } // jj

  } // ii

}

/////////////////////////////
// initialize overlay fields

void Lucid::_initOverlayFields()

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
    
    for(int jj = 0; jj <  _gd.num_datafields; jj++) {
      if(_gd.mread[jj]->button_name.compare(fieldName) == 0) {  
        if(lfield.on_at_startup) {
          _gd.layers.overlay_field_on[ii] = 1;
        } else {
          _gd.layers.overlay_field_on[ii] = 0;
        }
        _gd.layers.overlay_field[ii] = jj;
      }
    } // jj
  } // ii

}

/////////////////////////////////
// initialize sympbolic products

void Lucid::_initSymprods()
{

  _gd.r_context = new RenderContext(_gd.h_win.vis_pdev,
                                    _gd.def_brush, _gd.cmap, _gd.proj);
  
  _gd.prod_mgr = new ProductMgr(*_gd.r_context, (_gd.debug1 | _gd.debug2));
  
  _gd.r_context->set_scale_constant(_params.scale_constant);
  
  double min_lat, max_lat, min_lon, max_lon; 
  _getBoundingBox(min_lat,max_lat,min_lon,max_lon);
  _gd.r_context->set_clip_limits(min_lat, min_lon, max_lat, max_lon);
  
  if(_params.symprod_prod_info_n <= 32) {
    int value = 0;
    for (int ii = 0; ii < _params.symprod_prod_info_n && ii < 32; ii++) {
      if(_params._symprod_prod_info[ii].on_by_default == TRUE) {
        value |= 1 << ii;
      }
      _gd.prod_mgr->set_product_active
        (ii, (int) _params._symprod_prod_info[ii].on_by_default);
    } // ii

    // Set the widget's value and size the panel to fit the widget
    
    if(!_gd.run_unmapped) {
      // xv_set(_gd.prod_pu->prod_st, PANEL_VALUE, value,XV_SHOW,TRUE,XV_X,0,XV_Y,0, NULL);
      // xv_set(_gd.prod_pu->prod_pu,XV_HEIGHT,xv_get(_gd.prod_pu->prod_st,XV_HEIGHT),NULL);
      // xv_set(_gd.prod_pu->prod_pu,XV_WIDTH,xv_get(_gd.prod_pu->prod_st,XV_WIDTH),NULL);
    }
    
  } else { // Use a  scrolling list when over 32 products are configured in
    
    for (int i = 0; i < _params.symprod_prod_info_n ; i++) {
      if(!_gd.run_unmapped) {
        // xv_set(_gd.prod_pu->prod_lst,
        //      PANEL_LIST_INSERT, i,
        //      PANEL_LIST_STRING, i, _params._symprod_prod_info[i].menu_label,
        //      PANEL_LIST_CLIENT_DATA, i, i,
        //      NULL);
        
        if(_params._symprod_prod_info[i].on_by_default == TRUE) {
          // xv_set(_gd.prod_pu->prod_lst,
          //      PANEL_LIST_SELECT, i, TRUE,
          //      NULL);
        }
      }

      _gd.prod_mgr->set_product_active(i,(int) _params._symprod_prod_info[i].on_by_default);
    }

    if(!_gd.run_unmapped) {
      // Set the widget's value and size the panel to fit the widget
      // xv_set(_gd.prod_pu->prod_st,XV_SHOW,FALSE, NULL);
      // xv_set(_gd.prod_pu->prod_lst, XV_SHOW,TRUE,XV_X,0,XV_Y,0, NULL);
      // xv_set(_gd.prod_pu->prod_pu,XV_HEIGHT,xv_get(_gd.prod_pu->prod_lst,XV_HEIGHT),NULL);
      // xv_set(_gd.prod_pu->prod_pu,XV_WIDTH,xv_get(_gd.prod_pu->prod_lst,XV_WIDTH),NULL);
    }
  }

}

/************************************************************************
 * INIT_SHARED:  Initialize the shared memory communications
 *
 */

void Lucid::_initShared()
{

  /* Initialize shared memory area for coordinate/selection communications */

  _gd.coord_expt->button =  0;
  _gd.coord_expt->selection_sec = 0;
  _gd.coord_expt->selection_usec = 0;
  
  _gd.epoch_start = (time_t)
    (_gd.movie.start_time - (_gd.movie.time_interval_mins * 30.0));   
  _gd.epoch_end = (time_t)
    (_gd.movie.start_time +
     (_gd.movie.num_frames * _gd.movie.time_interval_mins * 60.0) -
     (_gd.movie.time_interval_mins * 30.0)); 

  _gd.coord_expt->epoch_start = _gd.epoch_start;
  _gd.coord_expt->epoch_end = _gd.epoch_end;
  
  _gd.coord_expt->time_min = _gd.movie.frame[_gd.movie.num_frames -1].time_start;
  _gd.coord_expt->time_max = _gd.movie.frame[_gd.movie.num_frames -1].time_end;
  if(_gd.movie.movie_on) {
    _gd.coord_expt->time_cent = _gd.epoch_end; 
  } else {
    _gd.coord_expt->time_cent = _gd.coord_expt->time_min +
      (_gd.coord_expt->time_max - _gd.coord_expt->time_min) / 2;
  } 
  _gd.coord_expt->pointer_seq_num = 0;

  _gd.coord_expt->datum_latitude = _params.proj_origin_lat;
  _gd.coord_expt->datum_longitude = _params.proj_origin_lon;

  _gd.coord_expt->pointer_x = 0.0;
  _gd.coord_expt->pointer_y = 0.0;
  _gd.coord_expt->pointer_lon = _gd.coord_expt->datum_longitude;
  _gd.coord_expt->pointer_lat = _gd.coord_expt->datum_latitude;

  _gd.coord_expt->focus_x = 0.0;
  _gd.coord_expt->focus_y = 0.0;
  _gd.coord_expt->focus_lat = _gd.coord_expt->datum_latitude;
  _gd.coord_expt->focus_lon = _gd.coord_expt->datum_longitude;

  _gd.coord_expt->click_type = CIDD_RESET_CLICK;
}

////////////////////////////////////////////////////
// create cache directories

int Lucid::_createCacheDirs()

{

  _gd.cacheDir = _params.top_level_cache_dir;
  _gd.cacheDir += PATH_DELIM;
  _gd.cacheDir += "Lucid";
  _gd.cacheDir += PATH_DELIM;
  _gd.cacheDir += _paramsPathRequested.getFile(); // add params file name

  // create it

  if (ta_makedir_recurse(_gd.cacheDir.c_str())) {
    int err = errno;
    cerr << "ERROR - Lucid" << endl;
    cerr << "Cannot make cache dir: " << _gd.cacheDir << endl;
    cerr << "  " << strerror(err) << endl;
    return -1;
  }

  _gd.mapCacheDir = _gd.cacheDir + PATH_DELIM + "maps";
  if (ta_makedir_recurse(_gd.mapCacheDir.c_str())) {
    int err = errno;
    cerr << "ERROR - Lucid" << endl;
    cerr << "Cannot make maps cache dir: " << _gd.mapCacheDir << endl;
    cerr << "  " << strerror(err) << endl;
    return -1;
  }

  _gd.colorscaleCacheDir = _gd.cacheDir + PATH_DELIM + "color_scales";
  if (ta_makedir_recurse(_gd.colorscaleCacheDir.c_str())) {
    int err = errno;
    cerr << "ERROR - Lucid" << endl;
    cerr << "Cannot make color scales cache dir: " << _gd.colorscaleCacheDir << endl;
    cerr << "  " << strerror(err) << endl;
    return -1;
  }

  _gd.stationlocCacheDir = _gd.cacheDir + PATH_DELIM + "stationloc";
  if (ta_makedir_recurse(_gd.stationlocCacheDir.c_str())) {
    int err = errno;
    cerr << "ERROR - Lucid" << endl;
    cerr << "Cannot make station locator cache dir: " << _gd.stationlocCacheDir << endl;
    cerr << "  " << strerror(err) << endl;
    return -1;
  }

  return 0;

}

/**********************************************************************
 * Get path to cached file.
 * We pass in the file name.
 * The cached path is filled in.
 * Returns 0 on success, -1 on error.
 */

int Lucid::_getResourceCachePath(const string &cacheDir,
                                 const string &resourceUrl,
                                 const string &resourceName,
                                 string &cachePath)
  
{

  // check if resource file exists in cache

  cachePath = cacheDir + PATH_DELIM + resourceName;
  if (!_params.clear_cache) {
    if (ta_stat_is_file(cachePath.c_str())) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Success - found resource file in cache: " << cachePath << endl;
      }
      return 0;
    }
  }
  
  // check if file exists locally? if so copy to the cache

  bool fileIsLocal = true;
  if (resourceUrl.find("http") == 0) {
    fileIsLocal = false;
  }
  
  if (fileIsLocal) {
    string localPath = resourceUrl;
    localPath += PATH_DELIM;
    localPath += resourceName;
    if (ta_stat_is_file(localPath.c_str())) {
      // copy to cache
      if (filecopy_by_name(cachePath.c_str(), localPath.c_str()) == 0) {
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "Success - copied local resource file to cache: " << cachePath << endl;
        }
        return 0;
      } else {
        int err = errno;
        cerr << "ERROR - cidd_init::_getResourceCachePath" << endl;
        cerr << "  Cannot copy local resource file: " << localPath << endl;
        cerr << "    to cache: " << cachePath << endl;
        cerr << "  " << strerror(err) << endl;
        return -1;
      }
    }
  }

  // file is remote, retrieve it using curl or http

  string remoteUrl = resourceUrl;
  remoteUrl += "/";
  remoteUrl += resourceName;

  if (_params.use_curl_for_downloads) {
    
    string cmd = "curl -s -o ";
    cmd += cachePath;
    cmd += " ";
    cmd += remoteUrl;
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Retrieving remote resource file, cmd is: " << endl;
      cerr << "  " << cmd << endl;
    }
    
    int timeoutSecs = _params.data_timeout_secs;
    if (safe_system(cmd.c_str(), timeoutSecs)) {
      cerr << "ERROR - cidd_init::_getResourceCachePath" << endl;
      cerr << "  Cannot get remote resource, cmd: " << cmd << endl;
      return -1;
    }

  } else {

    // use http
    
    char *cs_buf;
    int cs_len;
    int ret_stat = HTTPgetURL(remoteUrl.c_str(), _params.data_timeout_secs * 1000, &cs_buf, &cs_len);
    if(ret_stat <= 0) {
      cerr << "ERROR - cidd_init::_getResourceCachePath" << endl;
      cerr << "  Cannot get remote resource, url: " << remoteUrl << endl;
      return -1;
    }

    // write buffer to cache file

    FILE *cacheFile = fopen(cachePath.c_str(), "w");
    if (cacheFile == NULL) {
      int err = errno;
      cerr << "ERROR - cidd_init::_getResourceCachePath" << endl;
      cerr << "  Cannot open cache file for writing: " << cachePath << endl;
      cerr << "  " << strerror(err) << endl;
      return -1;
    }
    if ((int) fwrite(cs_buf, 1, cs_len, cacheFile) != cs_len) {
      int err = errno;
      cerr << "ERROR - cidd_init::_getResourceCachePath" << endl;
      cerr << "  Cannot write to cache file: " << cachePath << endl;
      cerr << "  " << strerror(err) << endl;
      fclose(cacheFile);
      return -1;
    }
    fclose(cacheFile);
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Success - copied resource file to cache: " << cachePath << endl;
      cerr << "Source url: " << remoteUrl << endl;
    }

  } // if (_params.use_curl_for_downloads)
    
  return 0;

}

/**********************************************************************
 * Loop through the color scale URLs, searching for the
 * requested color scale.
 */

int Lucid::_getColorscaleCachePath(const string &colorscaleName,
                                   string &cachePath)

{

  // get the color scale url from the parameter file

  vector<string> urlList;
  TaStr::tokenize(_params.color_scale_urls, ",", urlList);
  for (size_t ii = 0; ii < urlList.size(); ii++) {
    if (_getResourceCachePath(_gd.colorscaleCacheDir, urlList[ii], colorscaleName, cachePath) == 0) {
      return 0;
    }
  }
  return -1;

}
  
/**********************************************************************
 * Loop through the map URLs, searching for the
 * requested map.
 */

int Lucid::_getMapCachePath(const string &mapName,
                            string &cachePath,
                            string &cachePathX)

{

  // get the map url from the parameter file

  vector<string> urlList;
  TaStr::tokenize(_params.map_urls, ",", urlList);

  // for shape file, we need to download both the .shp and .shx files
  // for other (RAL-style) files, we just download the map file

  if (mapName.find(".shp") != string::npos ||
      mapName.find(".shx") != string::npos) {

    // shape file

    Path mapPath(mapName);
    string shpFileName = mapPath.getFile() + ".shp";
    string shxFileName = mapPath.getFile() + ".shx";
    
    for (size_t ii = 0; ii < urlList.size(); ii++) {
      if ((_getResourceCachePath(_gd.mapCacheDir, urlList[ii], shpFileName, cachePath) == 0) &&
          (_getResourceCachePath(_gd.mapCacheDir, urlList[ii], shxFileName, cachePathX) == 0)) {
        return 0;
      } else {
        cerr << "ERROR - _getMapCachePath" << endl;
        cerr << "  Cannot load shape mapName: " << mapName << endl;
      }
    }

  } else {

    // RAL style file
    
    for (size_t ii = 0; ii < urlList.size(); ii++) {
      if (_getResourceCachePath(_gd.mapCacheDir, urlList[ii], mapName, cachePath) == 0) {
        return 0;
      } else {
        cerr << "ERROR - _getMapCachePath" << endl;
        cerr << "  Cannot load RAL mapName: " << mapName << endl;
      }
    }

  }
  
  return -1;

}

////////////////////////////////////////////
// read file into buffer

int Lucid::_readFileIntoBuffer(const string &path,
                               char* &buf, int &len)

{
  
  struct stat sbuf;
  if(stat(path.c_str(), &sbuf) < 0) { // Stat to find the file's size
    int errNum = errno;
    cerr << "ERROR - cidd_init::_readFileIntoBuffer, cannot load file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  buf = (char *) calloc(sbuf.st_size + 1 ,1);
  if (!buf) {
    cerr << "ERROR - cidd_init::_readFileIntoBuffer - out of memory" << endl;
    return -1;
  }

  len = sbuf.st_size;
  FILE *ff = fopen(path.c_str(), "r");
  if (!ff) {
    int errNum = errno;
    cerr << "ERROR - cidd_init::_readFileIntoBuffer, cannot open file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if((ssize_t) fread(buf, 1, sbuf.st_size, ff) != sbuf.st_size) {
    int errNum = errno;
    cerr << "ERROR - cidd_init::_readFileIntoBuffer, cannot read file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    fclose(ff);
    return -1;
  }
  
  fclose(ff);

  return 0;

}

/************************************************************************
 * NORMALIZE_LONGITUDE: Normalize the given longitude value to fall within
 *                      min_lon and min_lon + 360.0.
 *
 */

void Lucid::_normalizeLongitude(double min_lon, double max_lon, double *normal_lon)
{

  if ((*normal_lon < min_lon || *normal_lon > max_lon) &&
      (min_lon >= -540.0 && max_lon < 540.0)) {
    
    while (*normal_lon < min_lon) {
      if (max_lon < *normal_lon + 360.0)
	break;
      *normal_lon += 360.0;
    }
    
    while (*normal_lon >= min_lon + 360.0) {
      *normal_lon -= 360.0;
    }

  }

}
/**************************************************************************
 *  GET_BOUNDING_BOX: Return the current lat,lon bounding box of data on the display
 */

void Lucid::_getBoundingBox(double &min_lat,
                            double &max_lat,
                            double &min_lon,
                            double &max_lon)
{

  // condition the longitudes for this zoom

  double meanx = (_gd.h_win.cmin_x + _gd.h_win.cmax_x) / 2.0;
  double meany = (_gd.h_win.cmin_y + _gd.h_win.cmax_y) / 2.0;
  double meanLat, meanLon;
  double lon1,lon2,lat1,lat2;

  _gd.proj.xy2latlon(meanx,meany,meanLat,meanLon);
  // Make sure meanLon makes since.
  if (meanLon > _gd.h_win.max_x) {
    meanLon -= 360.0;
  } else if (meanLon < _gd.h_win.min_x) {
    meanLon += 360.0;
  }
  _gd.proj.setConditionLon2Ref(true, meanLon);
  
  if(!_params.clip_to_current_zoom_on_mdv_request) {
    _gd.proj.xy2latlon(_gd.h_win.min_x,_gd.h_win.min_y,min_lat,min_lon);
    _gd.proj.xy2latlon(_gd.h_win.max_x,_gd.h_win.max_y,max_lat,max_lon);
  } else {
    switch(_gd.display_projection) {
      default:
        lon1 = _gd.h_win.cmin_x;
        lon2 = _gd.h_win.cmax_x;

        if((lon2 - lon1) > 360.0) {
          lon1 = _gd.h_win.min_x;
          lon2 = _gd.h_win.max_x; 
        }


        lat1 = _gd.h_win.cmin_y;
        lat2 = _gd.h_win.cmax_y;
        if((lat2 - lat1) > 360.0) {
          lat1 = _gd.h_win.min_y;
          lat2 = _gd.h_win.max_y; 
        }

        _gd.proj.xy2latlon(lon1, lat1,min_lat,min_lon);
        _gd.proj.xy2latlon(lon2, lat2,max_lat,max_lon);

        break;
 
      case Mdvx::PROJ_FLAT :
      case Mdvx::PROJ_LAMBERT_CONF:
      case Mdvx::PROJ_POLAR_STEREO:
      case Mdvx::PROJ_OBLIQUE_STEREO:
      case Mdvx::PROJ_MERCATOR:
        double lat,lon;
 
        // Compute the bounding box
        max_lon = -360.0;
        min_lon = 360.0;
        max_lat = -180.0;
        min_lat = 180.0;
 
        // Check each corner of the projection + 4 mid points, top, bottom
        // Left and right 

        // Lower left
        _gd.proj.xy2latlon(_gd.h_win.cmin_x , _gd.h_win.cmin_y ,lat,lon);
        if(lon > max_lon) max_lon = lon;
        if(lon < min_lon) min_lon = lon;
        if(lat > max_lat) max_lat = lat;
        if(lat < min_lat) min_lat = lat;
 
        // Lower midpoint
        _gd.proj.xy2latlon((_gd.h_win.cmin_x +_gd.h_win.cmax_x)/2 , _gd.h_win.cmin_y,lat,lon);
        if(lon > max_lon) max_lon = lon;
        if(lon < min_lon) min_lon = lon;
        if(lat > max_lat) max_lat = lat;
        if(lat < min_lat) min_lat = lat;
 
        // Lower right
        _gd.proj.xy2latlon(_gd.h_win.cmax_x , _gd.h_win.cmin_y ,lat,lon);
        if(lon > max_lon) max_lon = lon;
        if(lon < min_lon) min_lon = lon;
        if(lat > max_lat) max_lat = lat;
        if(lat < min_lat) min_lat = lat;
 
        // Right midpoint
        _gd.proj.xy2latlon(_gd.h_win.cmax_x , (_gd.h_win.cmin_y + _gd.h_win.cmax_y)/2,lat,lon);
        if(lon > max_lon) max_lon = lon;
        if(lon < min_lon) min_lon = lon;
        if(lat > max_lat) max_lat = lat;
        if(lat < min_lat) min_lat = lat;

        // Upper right
        _gd.proj.xy2latlon(_gd.h_win.cmax_x , _gd.h_win.cmax_y ,lat,lon);
        if(lon > max_lon) max_lon = lon;
        if(lon < min_lon) min_lon = lon;
        if(lat > max_lat) max_lat = lat;
        if(lat < min_lat) min_lat = lat;

        // Upper midpoint
        _gd.proj.xy2latlon((_gd.h_win.cmin_x +_gd.h_win.cmax_x)/2 , _gd.h_win.cmax_y,lat,lon);
        if(lon > max_lon) max_lon = lon;
        if(lon < min_lon) min_lon = lon;
        if(lat > max_lat) max_lat = lat;
        if(lat < min_lat) min_lat = lat;
 
        // Upper left
        _gd.proj.xy2latlon(_gd.h_win.cmin_x , _gd.h_win.cmax_y ,lat,lon);
        if(lon > max_lon) max_lon = lon;
        if(lon < min_lon) min_lon = lon;
        if(lat > max_lat) max_lat = lat;
        if(lat < min_lat) min_lat = lat;

        // Left midpoint
        _gd.proj.xy2latlon(_gd.h_win.cmin_x , (_gd.h_win.cmin_y + _gd.h_win.cmax_y)/2,lat,lon);
        if(lon > max_lon) max_lon = lon;
        if(lon < min_lon) min_lon = lon;
        if(lat > max_lat) max_lat = lat;
        if(lat < min_lat) min_lat = lat;
			 
        break;
    }
  }

  if(_gd.display_projection == Mdvx::PROJ_LATLON ) {
    double originLon = (min_lon + max_lon) / 2.0;
    _gd.proj.initLatlon(originLon);
  }
}

/**************************************************************************
 *  DISP_PROJ_DIST; Compute Distance between two display projection coordinates
 */

double Lucid::_dispProjDist(double x1, double y1, double x2, double y2)
{
  double dist,theta;
  double diff_x;
  double diff_y;
  
  switch(_gd.display_projection) {
    default:
      diff_x = (x2 - x1);
      diff_y = (y2 - y1);
      dist =  sqrt((diff_y * diff_y) + (diff_x * diff_x));
      break;
      
    case Mdvx::PROJ_LATLON:
      PJGLatLon2RTheta(y1,x1, y2, x2, &dist, &theta);
      break;
  }
  return dist;
  
}
