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
 * respond_to_timer.c
 *
 * respond to the timer interrupt
 *
 * RAP, NCAR, Boulder CO
 *
 * May 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "rview.h"
#include <signal.h>

#define UPDATE_COUNT_DEFAULT 10000
#define TRACK_CHECK_COUNT_DEFAULT 10000

static int alarm_in_progress = FALSE;

/*
 * Prototypes for static functions
 */

static time_t get_next_web_time(time_t last_output_time,
				time_t current_time,
				int web_output_interval);

/******************************************************************************
 * respond_to_timer()
 */

int respond_to_timer(int sig)

{

  static int first_call = TRUE;
  static si32 update_count = 0;
  static si32 update_count_target;
  static si32 track_check_count = 0;
  static si32 track_check_count_target;
  static int prev_time_hist_active = FALSE;
  static si32 ttime = 0;
  static si32 prev_track_seq_num = -1;
  static time_hist_shmem_t *tshmem;
  static int output_cappi_web_files;
  static int output_vsection_web_files;
  static int cappi_web_output_interval;
  static int vsection_web_output_interval;
  static time_t next_cappi_web_time;
  static time_t next_vsection_web_time;
  
  static double update_interval;
  static double track_check_interval;
  static double base_timer_interval;

  char *resource_str;
  
  int new_track_data;
  int main_display_must_update;
  int time_hist_active;

  si32 track_seq_num;

  zoom_t *zoom;

  time_t current_time = time(NULL);
  
  PORTsignal(SIGALRM, (void (*)())SIG_IGN);
  alarm_in_progress = TRUE;

  PMU_auto_register("In event loop (OK)");

  /*
   * on first call, get plot update interval and Web file creation
   * times from the parameters
   */

  if (first_call) {

    date_time_t zero_time;
    
    if (Glob->mode == REALTIME) {

      base_timer_interval =
	xGetResDouble(Glob->rdisplay, Glob->prog_name,
		      "base_timer_interval",
		      BASE_TIMER_INTERVAL);

      if (base_timer_interval == 0.0)
	base_timer_interval = BASE_TIMER_INTERVAL;

      update_interval = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				      "update_interval",
				      UPDATE_INTERVAL);
      
      update_count_target =
	(int) ((update_interval / base_timer_interval) + 0.5);

      track_check_interval = xGetResDouble(Glob->rdisplay, Glob->prog_name,
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

    zero_time.unix_time = current_time;
    uconvert_from_utime(&zero_time);
    zero_time.min = 0;
    zero_time.sec = 0;
    uconvert_to_utime(&zero_time);
    
    resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
				 "output_cappi_web_files",
				 OUTPUT_CAPPI_WEB_FILES);
    
    if (uset_true_false_param(Glob->prog_name,
			      "respond_to_timer",
			      Glob->params_path_name,
			      resource_str,
			      &output_cappi_web_files,
			      "output_cappi_web_files"))
      tidy_and_exit(-1);
    
    ufree(resource_str);
    
    if (output_cappi_web_files)
    {
      cappi_web_output_interval =
	xGetResLong(Glob->rdisplay, Glob->prog_name,
		    "cappi_web_output_interval", CAPPI_WEB_OUTPUT_INTERVAL)
	  * 60;
      
      next_cappi_web_time = get_next_web_time(zero_time.unix_time,
					      current_time,
					      cappi_web_output_interval);
    }
    
    resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
				 "output_vsection_web_files",
				 OUTPUT_VSECTION_WEB_FILES);
    
    if (uset_true_false_param(Glob->prog_name,
			      "respond_to_timer",
			      Glob->params_path_name,
			      resource_str,
			      &output_vsection_web_files,
			      "output_vsection_web_files"))
      tidy_and_exit(-1);
    
    ufree(resource_str);
    
    if (output_vsection_web_files)
    {
      vsection_web_output_interval =
	xGetResLong(Glob->rdisplay, Glob->prog_name,
		    "vsection_web_output_interval",
		    VSECTION_WEB_OUTPUT_INTERVAL)
	  * 60;
      
      next_vsection_web_time = get_next_web_time(zero_time.unix_time,
						 current_time,
						 vsection_web_output_interval);
    }
    
  }

  /*
   * set local flags
   */

  time_hist_active = tshmem->time_hist_active;
  track_seq_num = tshmem->track_seq_num;

  /*
   * every now and again, check for new track data
   */

  main_display_must_update = FALSE;
  
  if (Glob->use_time_hist) {

    if (tshmem->main_display_must_update) {
      main_display_must_update = TRUE;
      tshmem->main_display_must_update = FALSE;
    }

  } else {

    if (Glob->use_track_data &&
	track_check_count >= track_check_count_target) {

      if(tserver_check_notify(&Glob->tdata_index)) {
	main_display_must_update = TRUE;
      }
    
      track_check_count = 0;

    } /* if (Glob->use_track_data && ... */
    
  } /* if (Glob->use_time_hist) */

  /*
   * get new data if applicable
   */

  new_track_data = FALSE;

  if (Glob->use_track_data) {

    if (first_call || main_display_must_update ||
	(ttime != Glob->time)) {
      
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

  if (time_hist_active != prev_time_hist_active ||
      track_seq_num != prev_track_seq_num ||
      ttime != Glob->time ||
      update_count >= update_count_target ||
      new_track_data) {

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
      get_track_data();
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

  zoom = Glob->zoom + Glob->zoom_level;
  
  if (!zoom->current) {

    draw_cappi_plot(XDEV, Glob->fcontrol[Glob->field].xcolors);
    zoom->current = TRUE;
    expose_cappi_pixmap();
    Glob->cappi_requires_expose = FALSE;

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

  if (output_cappi_web_files &&
      next_cappi_web_time < current_time)
  {
    copy_cappi_web();
    next_cappi_web_time = get_next_web_time(next_cappi_web_time,
					    current_time,
					    cappi_web_output_interval);
  }
  
  if (Glob->vsection.active &&
      output_vsection_web_files &&
      next_vsection_web_time < current_time)
  {
    copy_vsection_web();
    next_vsection_web_time = get_next_web_time(next_vsection_web_time,
					       current_time,
					       vsection_web_output_interval);
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

  alarm_in_progress = FALSE;
  PORTsignal(SIGALRM, (void(*)()) respond_to_timer);
  
  first_call = FALSE;

  return(sig);

}

/******************************************************************************
 * set_cappi_invalid()
 */

void set_cappi_invalid(void)

{

  si32 i;

  for (i = 0; i < NZOOM; i++)
    Glob->zoom[i].current = FALSE;

}

/******************************************************************************
 * set_vsection_invalid()
 */

void set_vsection_invalid(void)

{

  Glob->vsection.current = FALSE;

}

/******************************************************************************
 * get_next_web_time()
 */

static time_t get_next_web_time(time_t last_output_time,
				time_t current_time,
				int web_output_interval)
{
  time_t next_web_time = last_output_time + web_output_interval;
  
  while (next_web_time < current_time)
    next_web_time += web_output_interval;
  
  return(next_web_time);
}

/*******************************************************
 * routines to avoid potential problems from the timer
 * interrupting an XFlush() or XClearWindow()
 */

void safe_XFlush(Display *display)

{
  
  if (alarm_in_progress) {
    XFlush(display);
  } else {
    PORTsignal(SIGALRM, (void (*)(int)) SIG_IGN);
    XFlush(display);
    PORTsignal(SIGALRM, (void(*)(int)) respond_to_timer);
  }
  
}

void safe_XClearWindow(Display *display, Window w)

{

  if (alarm_in_progress) {
    XClearWindow(display, w);
  } else {
    PORTsignal(SIGALRM, (void (*)(int)) SIG_IGN);
    XClearWindow(display, w);
    PORTsignal(SIGALRM, (void(*)(int)) respond_to_timer);
  }
  
}

