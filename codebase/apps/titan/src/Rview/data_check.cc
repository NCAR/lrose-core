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
 * data_check.cc
 *
 * Check if data has changed, redraw appropriately
 *
 * RAP, NCAR, Boulder CO
 *
 * Nov 2008
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "Rview.hh"
using namespace std;

#define UPDATE_COUNT_DEFAULT 10000
#define TRACK_CHECK_COUNT_DEFAULT 10000

// file scope variables (globals)

static bool initDone = false;
static double lastCheckTime = 0;
static int update_count = 0;
static int update_count_target;
static int track_check_count = 0;
static int track_check_count_target;
static time_t prev_track_data_time = 0;
static int prev_time_hist_active = FALSE;
static time_t ttime = 0;
static int prev_track_seq_num = -1;
static time_hist_shmem_t *tshmem;
static int output_cappi_web_files;
static int trigger_cappi_web_output_on_new_data;
static int output_vsection_web_files;
static int cappi_web_output_interval;
static int cappi_web_output_timestamped_files;
static int vsection_web_output_interval;
static int trigger_vsection_web_output_on_new_data;
static time_t next_cappi_web_time;
static time_t next_vsection_web_time;
static double update_interval;
static double track_check_interval;
static double base_timer_interval;

/*
 * Prototypes for static functions
 */

static void initialize();

static time_t get_next_web_time(time_t prev_output_time,
				time_t current_time,
				int web_output_interval);

/******************************************************************************
 * initialize the data check module
 */

static void initialize()

{

  date_time_t zero_time;
    
  if (Glob->mode == REALTIME) {
    
    base_timer_interval =
      uGetParamDouble(Glob->prog_name,
		      "base_timer_interval",
		      BASE_TIMER_INTERVAL);
    
    if (base_timer_interval == 0.0)
      base_timer_interval = BASE_TIMER_INTERVAL;
    
    update_interval = uGetParamDouble(Glob->prog_name,
				      "update_interval",
				      UPDATE_INTERVAL);
    
    update_count_target =
      (int) ((update_interval / base_timer_interval) + 0.5);
    
    track_check_interval = uGetParamDouble(Glob->prog_name,
					   "track_check_interval",
					   TRACK_CHECK_INTERVAL);
    
    track_check_count_target = (int) ((track_check_interval /
				       base_timer_interval) + 0.5);
    
  } else {
    
    update_count_target = UPDATE_COUNT_DEFAULT;
    track_check_count_target = TRACK_CHECK_COUNT_DEFAULT;
    
  }
  
  tshmem = Glob->track_shmem;
  
  /*
   * Get Web file creation intervals and flags
   */
  
  time_t current_time = time(NULL);
  zero_time.unix_time = current_time;
  uconvert_from_utime(&zero_time);
  zero_time.min = 0;
  zero_time.sec = 0;
  uconvert_to_utime(&zero_time);

  // set relevant parameters
  
  char *resource_str = uGetParamString(Glob->prog_name,
                                       "output_cappi_web_files",
                                       OUTPUT_CAPPI_WEB_FILES);
  if (uset_true_false_param(Glob->prog_name,
                            "data_check::initialize",
                            Glob->params_path_name,
                            resource_str,
                            &output_cappi_web_files,
                            "output_cappi_web_files")) {
    tidy_and_exit(-1);
  }
  ufree(resource_str);

  if (output_cappi_web_files)  {

    cappi_web_output_interval =
      uGetParamLong(Glob->prog_name,
		    "cappi_web_output_interval",
                    CAPPI_WEB_OUTPUT_INTERVAL) * 60;
    
    next_cappi_web_time = get_next_web_time(zero_time.unix_time,
					    current_time,
					    cappi_web_output_interval);
    resource_str = uGetParamString(Glob->prog_name,
				   "cappi_web_output_timestamped_files",
				   CAPPI_WEB_OUTPUT_TIMESTAMPED_FILES);
    
    if (uset_true_false_param(Glob->prog_name,
			      "data_check::initialize",
			      Glob->params_path_name,
			      resource_str,
			      &cappi_web_output_timestamped_files,
			      "cappi_web_output_timestamped_files"))
      tidy_and_exit(-1);
    
    ufree(resource_str);
    
    resource_str =
      uGetParamString(Glob->prog_name,
                      "trigger_cappi_web_output_on_new_data",
                      TRIGGER_CAPPI_WEB_OUTPUT_ON_NEW_DATA);
    if (uset_true_false_param(Glob->prog_name,
                              "data_check::initialize",
                              Glob->params_path_name,
                              resource_str,
                              &trigger_cappi_web_output_on_new_data,
                              "trigger_cappi_web_output_on_new_data")) {
      tidy_and_exit(-1);
    }
    ufree(resource_str);
  
  }
    
  resource_str = uGetParamString(Glob->prog_name,
				 "output_vsection_web_files",
				 OUTPUT_VSECTION_WEB_FILES);
  
  if (uset_true_false_param(Glob->prog_name,
			    "data_check::initialize",
			    Glob->params_path_name,
			    resource_str,
			    &output_vsection_web_files,
			    "output_vsection_web_files")) {
    tidy_and_exit(-1);
  }
  
  ufree(resource_str);
  
  if (output_vsection_web_files) {

    vsection_web_output_interval =
      uGetParamLong(Glob->prog_name,
		    "vsection_web_output_interval",
		    VSECTION_WEB_OUTPUT_INTERVAL) * 60;
    
    next_vsection_web_time = get_next_web_time(zero_time.unix_time,
					       current_time,
					       vsection_web_output_interval);

    resource_str =
      uGetParamString(Glob->prog_name,
                      "trigger_vsection_web_output_on_new_data",
                      TRIGGER_VSECTION_WEB_OUTPUT_ON_NEW_DATA);
    if (uset_true_false_param(Glob->prog_name,
                              "data_check::initialize",
                              Glob->params_path_name,
                              resource_str,
                              &trigger_vsection_web_output_on_new_data,
                              "trigger_vsection_web_output_on_new_data")) {
      tidy_and_exit(-1);
    }
    ufree(resource_str);

  }
    
} 

/******************************************************************************
 * check for new data, respond appropriately
 */

void check_for_new_data()

{

  // has enough time elapsed since we checked last?
  
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double checkTime = (double) tv.tv_sec + tv.tv_usec / 1000000.0;
  double timeSinceLastCheck = checkTime - lastCheckTime;
  if (timeSinceLastCheck < 0.1) {
    return;
  }
  lastCheckTime = checkTime;

  time_t current_time = time(NULL);
  PMU_auto_register("check_for_new_data");
  
  bool main_display_must_update = FALSE;
  if (!initDone) {
    initialize();
    initDone = true;
    main_display_must_update = true;
  }
  
  /*
   * set local flags
   */

  int time_hist_active = tshmem->time_hist_active;
  int track_seq_num = tshmem->track_seq_num;

  /*
   * every now and again, check for new track data
   */

  
  if (Glob->use_time_hist) {

    if (tshmem->main_display_must_update) {
      main_display_must_update = TRUE;
      tshmem->main_display_must_update = FALSE;
    }

  } else {

    if (Glob->use_track_data &&
	track_check_count >= track_check_count_target) {

      if (!Glob->use_time_hist) {
        if (Glob->_titanLdata.read() == 0) {
          if (prev_track_data_time != Glob->_titanLdata.getLatestTime()) {
            main_display_must_update = TRUE;
            prev_track_data_time = Glob->_titanLdata.getLatestTime();
          }
        }
      }
      
      track_check_count = 0;

    } /* if (Glob->use_track_data && ... */
    
  } /* if (Glob->use_time_hist) */

  /*
   * get new data if applicable
   */

  bool new_track_data = FALSE;

  if (Glob->use_track_data) {

    if (main_display_must_update || (ttime != Glob->time)) {
      
      /*
       * if time_hist has set the main display invalid,
       * set the mode and the time to match time_hist
       */

      if (Glob->use_time_hist) {
	Glob->time = tshmem->time;
	Glob->mode = tshmem->mode;
      }

      new_track_data = TRUE;

    } /* if (first_call ... */

  } /* if ((Glob->use_track_data) */

  bool new_product_data = FALSE;
  
  if (time_hist_active != prev_time_hist_active ||
      track_seq_num != prev_track_seq_num ||
      ttime != Glob->time ||
      update_count >= update_count_target ||
      new_track_data ||
      new_product_data) {

    /*
     * time hist activity has changed, or
     * track selection has been changed, or
     * time has been changed, or
     * update time has been reached, or
     * new track data has been read in, or
     * there is new product data available,
     * so set all plots invalid
     */

    if (Glob->use_track_data) {
      get_titan_data();
    }
    set_cappi_invalid();
    set_vsection_invalid();
    update_count = 0;
    
  } /* if (time_hist_active != .... */
  
  /*
   * render the vsection if required
   */
  
  if (Glob->vsection.active && !Glob->vsection.current) {
      
    draw_vsection_plot(XDEV, Glob->fcontrol[Glob->field].xcolors);
    Glob->vsection.current = TRUE;
    expose_vsection_pixmap();
    Glob->vsection_requires_expose = FALSE;
    set_cappi_invalid();
	
  } /* if (Glob->vsection.active ... */

  /*
   * render the cappi if required
   */

  zoom_t *zoom = Glob->zoom + Glob->zoom_level;

  bool cappi_drawn = false;
  if (!zoom->current && !gui_active()) {

    draw_cappi_plot(XDEV, Glob->fcontrol[Glob->field].xcolors);
    zoom->current = TRUE;
    expose_cappi_pixmap();
    Glob->cappi_requires_expose = FALSE;
    cappi_drawn = true;
    
  } /* if (!zoom->current) */

  /*
   * render other windows if required
   */
  
  if (Glob->main_scale_invalid) {
    
    if (Glob->field >= 0) {
      draw_main_scale(XDEV, Glob->fcontrol[Glob->field].xcolors,
		      SCALE_PLOT_LEGENDS);
    } else {
      draw_main_scale(XDEV,
		      (g_color_scale_t *) NULL,
		      SCALE_PLOT_LEGENDS);
    }
      
    Glob->main_scale_invalid = FALSE;

  }

  if (Glob->cappi_title_invalid) {

    draw_cappi_title(XDEV);
    Glob->cappi_title_invalid = FALSE;

  } 

  if (Glob->cappi_requires_expose) {

    expose_cappi_pixmap();
    Glob->cappi_requires_expose = FALSE;

  }

  if (Glob->vsection_title_invalid) {

    draw_vsection_title(XDEV);
    Glob->vsection_title_invalid = FALSE;

  } 

  if (Glob->vsection.active && Glob->vsection_requires_expose) {

    expose_vsection_pixmap();
    Glob->vsection_requires_expose = FALSE;

  }

  if (Glob->help_title_invalid) {

    draw_help_title();
    Glob->help_title_invalid = FALSE;

  } 

  if (Glob->help_requires_text) {

    draw_help_text();
    Glob->help_requires_text = FALSE;

  } 

  /*
   * Output Web files, if required
   */

  if (output_cappi_web_files) {
    bool do_output = false;
    if (trigger_cappi_web_output_on_new_data) {
      if (new_track_data) {
        do_output = true;
      }
    } else {
      if (next_cappi_web_time < current_time) {
        do_output = true;
      }
    }
    if (do_output) {
      if (Glob->debug) {
        cerr << "Making cappi images for web" << endl;
      }
      if (cappi_web_output_timestamped_files) {
        copy_cappi_named_gif();
      } else {
        copy_cappi_web();
      }
    }
    if (next_cappi_web_time < current_time) {
      next_cappi_web_time =
        get_next_web_time(next_cappi_web_time,
                          current_time,
                          cappi_web_output_interval);
    }
  }
  
  if (Glob->vsection.active && output_vsection_web_files) {
    bool do_output = false;
    if (trigger_vsection_web_output_on_new_data) {
      if (new_track_data) {
        do_output = true;
      }
    } else {
      if (next_vsection_web_time < current_time) {
        do_output = true;
      }
    }
    if (do_output) {
      copy_vsection_web();
    }
    if (next_vsection_web_time < current_time) {
      next_vsection_web_time =
        get_next_web_time(next_vsection_web_time,
                          current_time,
                          vsection_web_output_interval);
    }
  }
  
  /*
   * set static variables
   */

  if (Glob->mode == REALTIME) {
    update_count++;
    track_check_count++;
  }

  ttime = Glob->time;
  prev_time_hist_active = time_hist_active;
  prev_track_seq_num = track_seq_num;

  // auto_advance

  if (cappi_drawn && Glob->auto_advance) {

    bool doSave = true;
    if (!Glob->save_if_no_data
	&& !Glob->cappi_plotted
	&& !Glob->tracks_plotted) {
      doSave = false;
    }

    if (Glob->save_copy_on_auto_advance && doSave) {
      if (Glob->debug) {
	cerr << "Saving cappi hardcopy before advancing" << endl;
      }
      copy_cappi();
    }

    if (Glob->save_gif_on_auto_advance && doSave) {
      if (Glob->debug) {
	cerr << "Saving cappi gif before advancing" << endl;
      }
      copy_cappi_named_gif();
    }
    
    Glob->time += Glob->auto_advance_delta_time;
    if (Glob->debug) {
      cerr << "Auto-advance to time: " << utimstr(Glob->time) << endl;
    }

    if (Glob->time > Glob->auto_advance_end_time) {
      if (Glob->debug) {
	cerr << "Auto-advance - reached end time, quitting" << endl;
      }
      tidy_and_exit(0);
    }

  }

}

/******************************************************************************
 * set_cappi_invalid()
 */

void set_cappi_invalid()

{

  int i;

  for (i = 0; i < NZOOM; i++) {
    Glob->zoom[i].current = FALSE;
  }

}

/******************************************************************************
 * set_vsection_invalid()
 */

void set_vsection_invalid()

{

  Glob->vsection.current = FALSE;

}

/******************************************************************************
 * get_next_web_time()
 */

static time_t get_next_web_time(time_t prev_output_time,
				time_t current_time,
				int web_output_interval)
{

  time_t next_web_time = prev_output_time + web_output_interval;
  
  while (next_web_time < current_time)
    next_web_time += web_output_interval;
  
  if (Glob->debug) {
    cerr << "For image output, next_web_time: "
         << DateTime::strm(next_web_time) << endl;
  }

  return(next_web_time);
}
