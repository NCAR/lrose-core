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
 * respond_to_timer.c - time_hist routine
 *
 * Called in response to interval timer alarm signal.
 * Checks is the requested track may have changed,
 * and if so displays track based on shared memory data.
 *
 * RAP, NCAR, Boulder CO
 *
 * Jan 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "time_hist.h"
#include <signal.h>

static int alarm_in_progress = FALSE;

int respond_to_timer(int sig)

{

  static int first_call = TRUE;
  static si32 ttime = 0;
  static si32 mode = -1;
  static si32 track_set;
  static si32 plot_forecast;
  static si32 past_plot_period;
  static si32 future_plot_period;
  static si32 track_check_count = 0;
  static si32 track_check_count_target;

  static time_hist_shmem_t *tshmem;

  double track_check_interval;
  double base_timer_interval;

  int new_track_data_available;
  int main_display_must_update = FALSE;
  si32 i;

  PORTsignal(SIGALRM, (void (*)()) SIG_IGN);
  alarm_in_progress = TRUE;

  PMU_auto_register("In event loop (OK)");

  /*
   * on first call, compute track check parameters
   */

  if (first_call) {

    tshmem = Glob->track_shmem;

    if (tshmem->mode == TDATA_REALTIME) {

      base_timer_interval =
	xGetResDouble(Glob->rdisplay, Glob->prog_name,
		      "base_timer_interval",
		      BASE_TIMER_INTERVAL);

      if (base_timer_interval == 0.0)
	base_timer_interval = BASE_TIMER_INTERVAL;
      
      track_check_interval = xGetResDouble(Glob->rdisplay, Glob->prog_name,
					   "track_check_interval",
					   TRACK_CHECK_INTERVAL);
      
      track_check_count_target = (int) ((track_check_interval /
					 base_timer_interval) + 0.5);

    } else {
      
      track_check_count_target = 1000;

    }

  } /* if (first_call) */

  /*
   * check the main display - if it is not active, quit
   */

  if (!tshmem->main_display_active) {
    tidy_and_exit(0);
  }

  /*
   * every now and again, check for new track data
   */

  new_track_data_available = FALSE;

  track_check_count++;

  if (track_check_count >= track_check_count_target) {

    if(tserver_check_notify(&Glob->tdata_index))
      new_track_data_available = TRUE;
    
    track_check_count = 0;

  } /* if (track_check_count >= track_check_count_target) */

  /*
   * if time has changed, or track_set has changed, or new data
   * available, get track data
   */

  if (new_track_data_available ||
      ttime != tshmem->time ||
      mode != tshmem->mode ||
      track_set != tshmem->track_set ||
      Glob->scan_delta != 0) {

    get_track_data();

    Glob->tscale_status = NEEDS_REDRAW;
    Glob->thist_status = NEEDS_DATA;
    Glob->timeht_status = NEEDS_DATA;
    Glob->rdist_status = NEEDS_DATA;
    Glob->rdist_scale_status = NEEDS_EXPOSE;
    Glob->plot_data_status = NOT_CURRENT;
    
    /*
     * set flag to indicate that the main display is invalid
     */

    main_display_must_update = TRUE;

  }

  /*
   * if a new track selection has been made, get the track
   * number(s)
   */

  if (tshmem->select_track_type != NO_TRACK_TYPE) {

    compute_track_num();

    /*
     * raise plot windows
     */

    XRaiseWindow(Glob->rdisplay, Glob->thist_window);
    XRaiseWindow(Glob->rdisplay, Glob->timeht_window);
    XRaiseWindow(Glob->rdisplay, Glob->rdist_window);
    safe_XFlush(Glob->rdisplay);

    Glob->thist_status = NEEDS_DATA;
    Glob->timeht_status = NEEDS_DATA;
    Glob->rdist_status = NEEDS_DATA;
    Glob->plot_data_status = NOT_CURRENT;

  }

  /*
   * put main display invalid flag into shared memory
   */
  
  if (main_display_must_update) {
    tshmem->main_display_must_update = TRUE;
  }

  /*
   * redraw time scale if plot selections on main display have changed
   */

  if (plot_forecast != tshmem->plot_forecast ||
      past_plot_period != tshmem->past_plot_period ||
      future_plot_period != tshmem->future_plot_period)
    Glob->tscale_status = NEEDS_REDRAW;

  if (Glob->thist_status != CURRENT)
    draw_thist_plot(XDEV);

  if (Glob->timeht_status != CURRENT) {
    draw_timeht_plot(XDEV, Glob->x_timeht_cscale);
    Glob->timeht_scale_status = NEEDS_EXPOSE;
  }

  if(Glob->rdist_status != CURRENT) {
    draw_rdist_plot(XDEV, Glob->x_rdist_cscale);
  }

  if(Glob->rdist_scale_status != CURRENT) {
    draw_scale (XDEV, Glob->rdist_scale_frame, Glob->x_rdist_cscale,
		SCALE_LABEL_FORMAT_G, SCALE_NO_LEGENDS);
    Glob->rdist_scale_status = CURRENT;
  }

  if (Glob->tscale_status != CURRENT) {
    draw_tscale_plot(XDEV);
    draw_tscale_title(XDEV);
  }

  if (Glob->help_status != CURRENT)
    draw_help_text();

  if (Glob->timeht_scale_status != CURRENT) {

    if (Glob->timeht_mode == TIMEHT_VORTICITY) {
      draw_scale (XDEV, Glob->timeht_scale_frame, Glob->x_timeht_cscale,
		  SCALE_LABEL_FORMAT_E, SCALE_PLOT_LEGENDS);
    } else {
      draw_scale (XDEV, Glob->timeht_scale_frame, Glob->x_timeht_cscale,
		  SCALE_LABEL_FORMAT_G, SCALE_PLOT_LEGENDS);
    }

    Glob->timeht_scale_status = CURRENT;

  }

  if (Glob->rdist_scale_status != CURRENT) {

    draw_scale (XDEV, Glob->rdist_scale_frame, Glob->x_rdist_cscale,
		SCALE_LABEL_FORMAT_G, SCALE_NO_LEGENDS);

    Glob->rdist_scale_status = CURRENT;

  }

  ttime = tshmem->time;
  mode = tshmem->mode;
  track_set = tshmem->track_set;
  tshmem->time_hist_active = TRUE;
  plot_forecast = tshmem->plot_forecast;
  past_plot_period = tshmem->past_plot_period;
  future_plot_period = tshmem->future_plot_period;

  /*
   * on first call, draw buttons and titles
   */

  if (first_call) {

    draw_tscale_title (XDEV);
    draw_thist_title (XDEV);
    draw_timeht_title (XDEV);
    draw_rdist_title (XDEV);

    for (i = 0; i < N_TSCALE_BUTTONS; i++)
      draw_tscale_button(i, Glob->background);
	  
    for (i = 0; i < N_THIST_BUTTONS; i++)
      draw_thist_button(i, Glob->background);
	  
    for (i = 0; i < N_TIMEHT_BUTTONS; i++)
      draw_timeht_button(i, Glob->background);
	  
    for (i = 0; i < N_RDIST_BUTTONS; i++)
      draw_rdist_button(i, Glob->background);

    first_call = FALSE;

  }

  alarm_in_progress = FALSE;
  PORTsignal(SIGALRM, (void(*)()) respond_to_timer);
  
  return(sig);
  
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
