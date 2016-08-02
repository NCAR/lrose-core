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
 * event_loop.c - time_hist routine
 *
 * process the event loop
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "time_hist.h"
#include <signal.h>
#include <time.h>

#define LEFT 1
#define MIDDLE 2
#define RIGHT 3

#define N_BUTTONS (N_TSCALE_BUTTONS + N_THIST_BUTTONS + \
		   N_TIMEHT_BUTTONS + N_RDIST_BUTTONS)

static int button, ibutton;

static si32 button_active[N_BUTTONS];
static si32 done = FALSE;

static Time prev_release_time = 0;
static double double_click_delta_time;

static XEvent event;

static void button_press_callback(void);
static void button_release_callback(void);
static void configure_callback(void);
static void enter_callback(void);
static void expose_callback(void);
static void leave_callback(void);
static int (*alarm_function)();

void event_loop(void)

{

  si32 i;

  /*
   * set all buttons to inactive
   */

  for (i = 0; i < N_BUTTONS; i++)
    button_active[i] = FALSE;

  /*
   * set the alarm function
   */
  
  alarm_function = respond_to_timer;

  /*
   * the event loop
   */

  while (done == FALSE) {

    /*
     * get next event
     */

    XNextEvent(Glob->rdisplay, &event);

    switch (event.type) {
     
      /*
       * window resized event
       */

    case ConfigureNotify:
      
      /*
       * disable the alarm signal while the event is being handled
       */

      PORTsignal(SIGALRM, SIG_IGN);

      configure_callback();

      /*
       * enable the alarm signal interrupt
       */

      PORTsignal(SIGALRM, (void(*)()) alarm_function);

      break;

      /*
       * repaint windows on expose events
       */

    case Expose:

      /*
       * disable the alarm signal while the event is being handled
       */

      PORTsignal(SIGALRM, SIG_IGN);

      expose_callback();

      /*
       * enable the alarm signal interrupt
       */

      PORTsignal(SIGALRM, (void(*)()) alarm_function);

      break;

    case ButtonPress:

      /*
       * disable the alarm signal until after a button release
       */

      PORTsignal(SIGALRM, SIG_IGN);

      button_press_callback();

      break;

    case ButtonRelease:

      if (Glob->help)
	Glob->help_status = NEEDS_REDRAW;
      else
	button_release_callback();

      /*
       * enable the alarm signal interrupt
       */

      PORTsignal(SIGALRM, (void(*)()) alarm_function);

      break;

      /*
       * process mouse enter events
       */

    case EnterNotify:

      enter_callback();
      break;

    case LeaveNotify:

      leave_callback();
      break;

      /*
       * termination
       */

    } /* switch(event.type) */

  } /* while(done == 0) */

}

/**********************************************************************
 * button_press_callback() - processes the mouse button presses
 */

static void button_press_callback(void)

{

  si32 i;
  button = event.xbutton.button;

  ibutton = 0;

  for (i = 0; i < N_TSCALE_BUTTONS; i++) {
    if (event.xbutton.window ==
	Glob->tscale_button_frame[i]->x->drawable) {
      button_active[ibutton] = TRUE;
      Glob->help_code = ibutton;
      return;
    }
    ibutton++;
  }

  for (i = 0; i < N_THIST_BUTTONS; i++) {
    if (event.xbutton.window ==
	Glob->thist_button_frame[i]->x->drawable) {
      button_active[ibutton] = TRUE;
      Glob->help_code = ibutton;
      return;
    }
    ibutton++;
  }

  for (i = 0; i < N_TIMEHT_BUTTONS; i++) {
    if (event.xbutton.window ==
	Glob->timeht_button_frame[i]->x->drawable) {
      button_active[ibutton] = TRUE;
      Glob->help_code = ibutton;
      return;
    }
    ibutton++;
  }

  for (i = 0; i < N_RDIST_BUTTONS; i++) {
    if (event.xbutton.window ==
	Glob->rdist_button_frame[i]->x->drawable) {
      button_active[ibutton] = TRUE;
      Glob->help_code = ibutton;
      return;
    }
    ibutton++;
  }

  for (i = 0; i < N_HELP_BUTTONS; i++) {
    if (event.xbutton.window ==
	Glob->help_button_frame[i]->x->drawable) {
      button_active[ibutton] = TRUE;
      Glob->help_code = ibutton;
      return;
    }
    ibutton++;
  }

  if(event.xbutton.window == Glob->tscale_plot_frame->x->drawable ||
     event.xbutton.window == Glob->thist_plot_frame->x->drawable ||
     event.xbutton.window == Glob->timeht_plot_frame->x->drawable ||
     event.xbutton.window == Glob->rdist_plot_frame->x->drawable) {

    Glob->help_code = HELP_PLOT;

  }

}

/**********************************************************************
 * button_release_callback() - processes the mouse button releases
 */

static void button_release_callback(void)

{
  
  si32 i;
  time_t now;
  date_time_t now_dt;
  struct tm *gmt;
  si32 prev_seq_num;
  coord_export_t *coord;
  time_hist_shmem_t *tshmem;

  double_click_delta_time = fabs ((double) event.xbutton.time -
				  (double) prev_release_time);

  prev_release_time = event.xbutton.time;
  button = event.xbutton.button;
  coord = Glob->coord_export;
  tshmem = Glob->track_shmem;

  /*
   * tscale window mouse actions
   */

  if (event.xbutton.window == Glob->tscale_plot_frame->x->drawable) {
    
    /*
     * check for double click - if there is a double click, it
     * means that the user wants to change the scan time to the
     * time indicated by the mouse position
     */

    if (double_click_delta_time < Glob->double_click_delay) {

      tshmem->time =
	(si32) GXWorldx(Glob->tscale_plot_frame, event.xbutton.x);

      if (tshmem->time >
	  Glob->tdata_index.complete.header.data_end_time)
	tshmem->time = Glob->tdata_index.complete.header.data_end_time;

      tshmem->mode = TDATA_ARCHIVE;

    } /* if (double_click_delta_time < Glob->double_click_delay) */

  /*
   * thist window mouse actions
   */

  } else if (event.xbutton.window == Glob->thist_plot_frame->x->drawable) {
    
    /*
     * check for double click - if there is a double click, it
     * means that the user wants to change the scan time to the
     * time indicated by the mouse position
     */

    if (double_click_delta_time < Glob->double_click_delay) {

      tshmem->time =
	(si32) GXWorldx(Glob->thist_plot_frame, event.xbutton.x);

      if (tshmem->time >
	  Glob->tdata_index.complete.header.data_end_time)
	tshmem->time = Glob->tdata_index.complete.header.data_end_time;

      tshmem->mode = TDATA_ARCHIVE;

      Glob->thist_cursor_active = FALSE;
      
    } else {
      
      /*
       * process the single click
       */

      Glob->thist_cursor_active = TRUE;
      Glob->thist_cursor_x =
	GXWorldx(Glob->thist_plot_frame, event.xbutton.x);
      Glob->thist_cursor_y =
	GXWorldy(Glob->thist_plot_frame, event.xbutton.y);
      Glob->thist_status = NEEDS_REDRAW;
      
    } /* if (double_click_delta_time < Glob->double_click_delay) */

  /*
   * timeht window mouse actions
   */

  } else if (event.xbutton.window == Glob->timeht_plot_frame->x->drawable) {
    
    /*
     * check for double click - if there is a double click, it
     * means that the user wants to change the scan time to the
     * time indicated by the mouse position
     */

    if (double_click_delta_time < Glob->double_click_delay) {
  
      tshmem->time =
	(si32) GXWorldx(Glob->timeht_plot_frame, event.xbutton.x);

      if (tshmem->time >
	  Glob->tdata_index.complete.header.data_end_time)
	tshmem->time = Glob->tdata_index.complete.header.data_end_time;

      tshmem->mode = TDATA_ARCHIVE;

      Glob->timeht_cursor_active = FALSE;
      
    } else {
      
      /*
       * process the single click
       */

      Glob->timeht_cursor_active = TRUE;
      Glob->timeht_cursor_x =
	GXWorldx(Glob->timeht_plot_frame, event.xbutton.x);
      Glob->timeht_cursor_y =
	GXWorldy(Glob->timeht_plot_frame, event.xbutton.y);
      Glob->timeht_status = NEEDS_REDRAW;

    } /* if (double_click_delta_time < Glob->double_click_delay) */

  /*
   * rdist window mouse actions
   */

  } else if (event.xbutton.window == Glob->rdist_plot_frame->x->drawable) {
    
    /*
     * check for double click - if there is a double click, it
     * means that the user wants to change the scan time to the
     * time indicated by the mouse position
     */

    if (double_click_delta_time < Glob->double_click_delay) {

      tshmem->time =
	(si32) GXWorldx(Glob->rdist_plot_frame, event.xbutton.x);
  
      if (tshmem->time >
	  Glob->tdata_index.complete.header.data_end_time)
	tshmem->time = Glob->tdata_index.complete.header.data_end_time;

      tshmem->mode = TDATA_ARCHIVE;

      Glob->rdist_cursor_active = FALSE;
      
    } else {
      
      /*
       * process the single click
       */

      Glob->rdist_cursor_active = TRUE;
      Glob->rdist_cursor_x =
	GXWorldx(Glob->rdist_plot_frame, event.xbutton.x);
      Glob->rdist_cursor_y =
	GXWorldy(Glob->rdist_plot_frame, event.xbutton.y);
      Glob->rdist_status = NEEDS_REDRAW;

    } /* if (double_click_delta_time < Glob->double_click_delay) */

  } else {
    
    /*
     * menu button action
     */
    
    if(button_active[ibutton] == TRUE) {
      
      switch(ibutton) {
	
      case TSCALE_HELP:

	Glob->help = TRUE;
	Glob->help_status = NEEDS_REDRAW;
	setup_help_windows();
	set_help_sens();
	XMapRaised (Glob->rdisplay, Glob->help_window);
	XMapSubwindows (Glob->rdisplay, Glob->help_window);
	safe_XFlush(Glob->rdisplay);
    
	break;
	
      case TSCALE_SELECT:
	
	prev_seq_num = coord->pointer_seq_num;

	while (coord->pointer_seq_num == prev_seq_num)
	  uusleep(100000);

	coord->focus_x = coord->pointer_x;
	coord->focus_y = coord->pointer_y;

	if (Glob->debug)
	  fprintf(stderr, "focus_x, focus_y = %g, %g\n",
		  coord->focus_x, coord->focus_y);

	switch (coord->button) {

	case LEFT:

	  tshmem->select_track_type = SIMPLE_TRACK;
	  if (Glob->debug)
	    fprintf(stderr, "select simple track\n");
	  break;

	case MIDDLE:

	  tshmem->select_track_type = PARTIAL_TRACK;
	  if (Glob->debug)
	    fprintf(stderr, "select partial track\n");
	  break;

	case RIGHT:

	  tshmem->select_track_type = COMPLEX_TRACK;
	  if (Glob->debug)
	    fprintf(stderr, "select complex track\n");
	  break;
	  
	}
	break;
	
      case TSCALE_TRACK_SET:

	switch (button) {
	      
	case LEFT:
	      
	  /*
	   * tracks at present time only
	   */

	  if (tshmem->track_set != TDATA_ALL_AT_TIME) {
	    tshmem->track_set = TDATA_ALL_AT_TIME;
	  }
	  break;
	    
	case RIGHT:
	      
	  /*
	   * all tracks in ops period
	   */
	      
	  if (tshmem->track_set != TDATA_ALL_IN_FILE) {
	    tshmem->track_set = TDATA_ALL_IN_FILE;
	  }
	  break;
	  
	} /* switch (button) */

	break;
	
      case TSCALE_DATE:

	/*
	 * increment or decrement date, depending upon which button
	 * was pressed.
	 */

	switch (button) {
	      
	case LEFT:

	  tshmem->time -= 86400;
	  tshmem->mode = TDATA_ARCHIVE;
	  break;
	      
	case RIGHT:
	  
	  if (tshmem->mode != TDATA_REALTIME)
	    tshmem->time += 86400;
	  break;
	  
	} /*switch */
	      
	break;
	
      case TSCALE_TIME:

	/*
	 * increment or decrement time, depending upon which button
	 * was pressed.
	 */

	switch (button) {
	      
	case LEFT:

	  Glob->scan_delta = -1;
	  tshmem->mode = TDATA_ARCHIVE;
	  break;
	      
	case RIGHT:
	  
	  if (tshmem->mode != TDATA_REALTIME) {
	    Glob->scan_delta = 1;
	  }
	  break;

	} /*switch */
	      
	break;
	
      case TSCALE_NOW:
	
	now = time((time_t *) NULL);
	gmt = gmtime(&now);
	now_dt.year = gmt->tm_year + 1900;
	now_dt.month = gmt->tm_mon + 1;
	now_dt.day = gmt->tm_mday;
	now_dt.hour = gmt->tm_hour;
	now_dt.month = gmt->tm_min;
	now_dt.sec = gmt->tm_sec;
	uconvert_to_utime(&now_dt);
	
	tshmem->time = now_dt.unix_time;
	tshmem->mode = TDATA_REALTIME;

	break;
	
      case TSCALE_QUIT:
	
	done = TRUE;
	break;
	
      case THIST_VOL:
	
	switch (button) {
	case LEFT:
	  if(Glob->thist_field_active[THIST_VOL_FIELD]) {
	    Glob->thist_field_active[THIST_VOL_FIELD] = FALSE;
	    Glob->thist_status = NEEDS_REDRAW;
	  }
	  break;
	case MIDDLE:
	  break;
	case RIGHT:
	  if(!Glob->thist_field_active[THIST_VOL_FIELD]) {
	    Glob->thist_field_active[THIST_VOL_FIELD] = TRUE;
	    Glob->thist_status = NEEDS_REDRAW;
	  }
	  break;
	}
	break;
	
      case THIST_AREA:
	
	switch (button) {
	case LEFT:
	  if(Glob->thist_field_active[THIST_AREA_FIELD]) {
	    Glob->thist_field_active[THIST_AREA_FIELD] = FALSE;
	    Glob->thist_status = NEEDS_REDRAW;
	  }
	  break;
	case MIDDLE:
	  break;
	case RIGHT:
	  if(!Glob->thist_field_active[THIST_AREA_FIELD]) {
	    Glob->thist_field_active[THIST_AREA_FIELD] = TRUE;
	    Glob->thist_status = NEEDS_REDRAW;
	  }
	  break;
	}
	break;
	
      case THIST_PFLUX:
	
	switch (button) {
	case LEFT:
	  if(Glob->thist_field_active[THIST_PFLUX_FIELD]) {
	    Glob->thist_field_active[THIST_PFLUX_FIELD] = FALSE;
	    Glob->thist_status = NEEDS_REDRAW;
	  }
	  break;
	case MIDDLE:
	  break;
	case RIGHT:
	  if(!Glob->thist_field_active[THIST_PFLUX_FIELD]) {
	    Glob->thist_status = NEEDS_REDRAW;
	    Glob->thist_field_active[THIST_PFLUX_FIELD] = TRUE;
	  }
	  break;
	}
	break;
	
      case THIST_MASS:
	
	switch (button) {
	case LEFT:
	  if(Glob->thist_field_active[THIST_MASS_FIELD]) {
	    Glob->thist_field_active[THIST_MASS_FIELD] = FALSE;
	    Glob->thist_status = NEEDS_REDRAW;
	  }
	  break;
	case MIDDLE:
	  break;
	case RIGHT:
	  if(!Glob->thist_field_active[THIST_MASS_FIELD]) {
	    Glob->thist_field_active[THIST_MASS_FIELD] = TRUE;
	    Glob->thist_status = NEEDS_REDRAW;
	  }
	  break;
	}
	break;
	
      case THIST_VIL:
	
	switch (button) {
	case LEFT:
	  if(Glob->thist_field_active[THIST_VIL_FIELD]) {
	    Glob->thist_field_active[THIST_VIL_FIELD] = FALSE;
	    Glob->thist_status = NEEDS_REDRAW;
	  }
	  break;
	case MIDDLE:
	  break;
	case RIGHT:
	  if(!Glob->thist_field_active[THIST_VIL_FIELD]) {
	    Glob->thist_field_active[THIST_VIL_FIELD] = TRUE;
	    Glob->thist_status = NEEDS_REDRAW;
	  }
	  break;
	}
	break;
	
      case THIST_FORECAST:

	switch (button) {

	case LEFT:

	  if(Glob->thist_forecast != FALSE) {
	    Glob->thist_forecast = FALSE;
	    Glob->thist_status = NEEDS_DATA;
	  }
	  break;

	case MIDDLE:

	  if(Glob->thist_forecast != SELECTED_LIMITED) {
	    Glob->thist_forecast = SELECTED_LIMITED;
	    Glob->thist_status = NEEDS_DATA;
	  }
	  break;

	case RIGHT:

	  if(Glob->thist_forecast != SELECTED_ALL) {
	    Glob->thist_forecast = SELECTED_ALL;
	    Glob->thist_status = NEEDS_DATA;
	  }
	  break;

	}

	break;
	
      case THIST_FIT:

	switch (button) {
	case LEFT:
	  if(Glob->thist_fit != FALSE) {
	    Glob->thist_fit = FALSE;
	    Glob->thist_status = NEEDS_REDRAW;
	  }
	  break;
	case RIGHT:
	  if(Glob->thist_fit != TRUE) {
	    Glob->thist_fit = TRUE;
	    Glob->thist_status = NEEDS_REDRAW;
	  }
	  break;
	}
	break;

      case THIST_COPY:
	
	copy_thist();
	
	break;
	
      case TIMEHT_MAXZ:
	
	if (button != RIGHT) {
	  if (Glob->timeht_mode != FALSE) {
	    Glob->timeht_mode = FALSE;
	    Glob->timeht_status = NEEDS_DATA;
	  }
	} else if (Glob->timeht_mode != TIMEHT_MAXZ) {
	  Glob->timeht_mode = TIMEHT_MAXZ;
	  Glob->timeht_status = NEEDS_DATA;
	  Glob->x_timeht_cscale = Glob->x_dbz_cscale;
	  Glob->ps_timeht_cscale = Glob->ps_dbz_cscale;
	}
	break;
	
      case TIMEHT_MEANZ:
	
	if (button != RIGHT) {
	  if (Glob->timeht_mode != FALSE) {
	    Glob->timeht_mode = FALSE;
	    Glob->timeht_status = NEEDS_DATA;
	  }
	} else if (Glob->timeht_mode != TIMEHT_MEANZ) {
	  Glob->timeht_mode = TIMEHT_MEANZ;
	  Glob->timeht_status = NEEDS_DATA;
	  Glob->x_timeht_cscale = Glob->x_dbz_cscale;
	  Glob->ps_timeht_cscale = Glob->ps_dbz_cscale;
	}
	
	break;
	
      case TIMEHT_MASS:
	
	if (button != RIGHT) {
	  if (Glob->timeht_mode != FALSE) {
	    Glob->timeht_mode = FALSE;
	    Glob->timeht_status = NEEDS_DATA;
	  }
	} else if (Glob->timeht_mode != TIMEHT_MASS) {
	  Glob->timeht_mode = TIMEHT_MASS;
	  Glob->timeht_status = NEEDS_DATA;
	  Glob->x_timeht_cscale = Glob->x_percent_cscale;
	  Glob->ps_timeht_cscale = Glob->ps_percent_cscale;
	}
	break;
	
      case TIMEHT_VORTICITY:
	
	if (button != RIGHT) {
	  if (Glob->timeht_mode != FALSE) {
	    Glob->timeht_mode = FALSE;
	    Glob->timeht_status = NEEDS_DATA;
	  }
	} else if (Glob->timeht_mode != TIMEHT_VORTICITY) {
	  Glob->timeht_mode = TIMEHT_VORTICITY;
	  Glob->timeht_status = NEEDS_DATA;
	  Glob->x_timeht_cscale = Glob->x_vorticity_cscale;
	  Glob->ps_timeht_cscale = Glob->ps_vorticity_cscale;
	}
	break;
	
      case TIMEHT_CENTROIDS:
	
	switch (button) {

	case LEFT:

	  if(Glob->timeht_centroids != FALSE) {
	    Glob->timeht_centroids = FALSE;
	    Glob->timeht_status = NEEDS_EXPOSE;
	  }
	  break;

	case MIDDLE:

	  if(Glob->timeht_centroids != SELECTED_LIMITED) {
	    Glob->timeht_centroids = SELECTED_LIMITED;
	    Glob->timeht_status = NEEDS_EXPOSE;
	  }
	  break;

	case RIGHT:

	  if(Glob->timeht_centroids != SELECTED_ALL) {
	    Glob->timeht_centroids = SELECTED_ALL;
	    Glob->timeht_status = NEEDS_EXPOSE;
	  }
	  break;

	}

	break;

      case TIMEHT_HTMAXZ:
	
	switch (button) {

	case LEFT:

	  if(Glob->timeht_htmaxz != FALSE) {
	    Glob->timeht_htmaxz = FALSE;
	    Glob->timeht_status = NEEDS_EXPOSE;
	  }
	  break;

	case MIDDLE:
	  if(Glob->timeht_htmaxz != SELECTED_LIMITED) {
	    Glob->timeht_htmaxz = SELECTED_LIMITED;
	    Glob->timeht_status = NEEDS_EXPOSE;
	  }
	  break;

	case RIGHT:

	  if(Glob->timeht_htmaxz != SELECTED_ALL) {
	    Glob->timeht_htmaxz = SELECTED_ALL;
	    Glob->timeht_status = NEEDS_EXPOSE;
	  }
	  break;

	}

	break;
	
      case TIMEHT_COPY:
	
	copy_timeht(Glob->ps_timeht_cscale);
	break;
	
      case RDIST_VOL:

	if (Glob->rdist_mode != RDIST_VOL) {
	  Glob->rdist_mode = RDIST_VOL;
	  Glob->rdist_status = NEEDS_DATA;
	}
	break;
	
      case RDIST_AREA:
	
	if (Glob->rdist_mode != RDIST_AREA) {
	  Glob->rdist_mode = RDIST_AREA;
	  Glob->rdist_status = NEEDS_DATA;
	}
	break;
	
      case RDIST_FLIP:
	
	Glob->rdist_sign *= -1.0;
	Glob->rdist_status = NEEDS_EXPOSE;
	break;
	
      case RDIST_FIT:

	switch (button) {
	case LEFT:
	  if(Glob->rdist_fit != FALSE) {
	    Glob->rdist_fit = FALSE;
	    Glob->rdist_status = NEEDS_EXPOSE;
	  }
	  break;
	case RIGHT:
	  if(Glob->rdist_fit != TRUE) {
	    Glob->rdist_fit = TRUE;
	    Glob->rdist_status = NEEDS_EXPOSE;
	  }
	  break;
	}
	break;

      case RDIST_COPY:
	
	copy_rdist(Glob->ps_rdist_cscale);
	break;
	
      }
      
      /*
       * set window back to background color
       */
      
      if (ibutton < N_TSCALE_BUTTONS) {
	
	i = ibutton;
	draw_tscale_button(i, Glob->background);
	
      } else if (ibutton < (N_TSCALE_BUTTONS +
			    N_THIST_BUTTONS)) {
	
	i = ibutton - N_TSCALE_BUTTONS;
	draw_thist_button(i, Glob->background);
	
      } else if (ibutton < (N_TSCALE_BUTTONS +
			    N_THIST_BUTTONS +
			    N_TIMEHT_BUTTONS)) {
	
	i = ibutton - N_TSCALE_BUTTONS - N_THIST_BUTTONS;
	draw_timeht_button(i, Glob->background);
	
      } else {
	
	i = ibutton - N_TSCALE_BUTTONS - N_THIST_BUTTONS - N_TIMEHT_BUTTONS;
	draw_rdist_button(i, Glob->background);
	
      }
      
      button_active[ibutton] = FALSE;
      
    } /* if(button_active[ibutton] == TRUE) */
  
  } /* if(event.xbutton.window == Glob->thist_plot_frame->x->drawable .. */
    
}

/**********************************************************************
 * enter_callback() - processes the mouse enter events
 */

static void enter_callback(void)

{

  si32 i;

  /*
   * highlight buttons on enter
   */

  for (i = 0; i < N_TSCALE_BUTTONS; i++) {
    if (event.xcrossing.window ==
	Glob->tscale_button_frame[i]->x->drawable) {
      draw_tscale_button(i, Glob->hlight_background);
      return;
    }
  }

  for (i = 0; i < N_THIST_BUTTONS; i++) {
    if (event.xcrossing.window ==
	Glob->thist_button_frame[i]->x->drawable) {
      draw_thist_button(i, Glob->hlight_background);
      return;
    }
  }

  for (i = 0; i < N_TIMEHT_BUTTONS; i++) {
    if (event.xcrossing.window ==
	Glob->timeht_button_frame[i]->x->drawable) {
      draw_timeht_button(i, Glob->hlight_background);
      return;
    }
  }

  for (i = 0; i < N_RDIST_BUTTONS; i++) {
    if (event.xcrossing.window ==
	Glob->rdist_button_frame[i]->x->drawable) {
      draw_rdist_button(i, Glob->hlight_background);
      return;
    }
  }

  for (i = 0; i < N_HELP_BUTTONS; i++) {
    if (event.xcrossing.window ==
	Glob->help_button_frame[i]->x->drawable) {
      draw_help_button(i, Glob->hlight_background);
      return;
    }
  }

}

/**********************************************************************
 * leave_callback() - processes the mouse leave events
 */

static void leave_callback(void)

{

  si32 i;

  /*
   * un_highlight window on mouse leave events
   */

  for (i = 0; i < N_TSCALE_BUTTONS; i++) {
    if (event.xcrossing.window ==
	Glob->tscale_button_frame[i]->x->drawable) {
      draw_tscale_button(i, Glob->background);
      return;
    }
  }

  for (i = 0; i < N_THIST_BUTTONS; i++) {
    if (event.xcrossing.window ==
	Glob->thist_button_frame[i]->x->drawable) {
      draw_thist_button(i, Glob->background);
      return;
    }
  }

  for (i = 0; i < N_TIMEHT_BUTTONS; i++) {
    if (event.xcrossing.window ==
	Glob->timeht_button_frame[i]->x->drawable) {
      draw_timeht_button(i, Glob->background);
      return;
    }
  }

  for (i = 0; i < N_RDIST_BUTTONS; i++) {
    if (event.xcrossing.window ==
	Glob->rdist_button_frame[i]->x->drawable) {
      draw_rdist_button(i, Glob->background);
      return;
    }
  }

  for (i = 0; i < N_HELP_BUTTONS; i++) {
    if (event.xcrossing.window ==
	Glob->help_button_frame[i]->x->drawable) {
      draw_help_button(i, Glob->background);
      return;
    }
  }

  button_active[ibutton] = FALSE;
      
}

/**********************************************************************
 * configure_callback() - processes the configure notify events
 */

static void configure_callback(void)

{

  if(event.xconfigure.window == Glob->tscale_window) {

    Glob->x_tscale_x = event.xconfigure.x;
    Glob->x_tscale_y = event.xconfigure.y;
    Glob->x_tscale_width = event.xconfigure.width;
    Glob->x_tscale_height = event.xconfigure.height;

    XSelectInput(Glob->rdisplay, Glob->tscale_window, NoEventMask);

    setup_tscale_windows();
    set_tscale_sens();

  } else if(event.xconfigure.window == Glob->thist_window) {

    Glob->x_thist_x = event.xconfigure.x;
    Glob->x_thist_y = event.xconfigure.y;
    Glob->x_thist_width = event.xconfigure.width;
    Glob->x_thist_height = event.xconfigure.height;

    XSelectInput(Glob->rdisplay, Glob->thist_window, NoEventMask);

    setup_thist_windows();
    set_thist_sens();

  } else if(event.xconfigure.window == Glob->timeht_window) {

    Glob->x_timeht_x = event.xconfigure.x;
    Glob->x_timeht_y = event.xconfigure.y;
    Glob->x_timeht_width = event.xconfigure.width;
    Glob->x_timeht_height = event.xconfigure.height;

    XSelectInput(Glob->rdisplay, Glob->timeht_window, NoEventMask);

    setup_timeht_windows();
    set_timeht_sens();

  } else if(event.xconfigure.window == Glob->rdist_window) {

    Glob->x_rdist_x = event.xconfigure.x;
    Glob->x_rdist_y = event.xconfigure.y;
    Glob->x_rdist_width = event.xconfigure.width;
    Glob->x_rdist_height = event.xconfigure.height;

    XSelectInput(Glob->rdisplay, Glob->rdist_window, NoEventMask);

    setup_rdist_windows();
    set_rdist_sens();

  } else if(event.xconfigure.window == Glob->help_window) {

    Glob->x_help_x = event.xconfigure.x;
    Glob->x_help_y = event.xconfigure.y;
    Glob->x_help_width = event.xconfigure.width;
    Glob->x_help_height = event.xconfigure.height;

    XSelectInput(Glob->rdisplay, Glob->help_window, NoEventMask);

    setup_help_windows();
    set_help_sens();

  }
  
}

/**********************************************************************
 * expose_callback() - processes the expose events
 */

static void expose_callback(void)

{

  si32 i;
  
  if (event.xexpose.count == 0) {
    
    if (event.xexpose.window ==
	Glob->tscale_plot_frame->x->drawable) {

      Glob->tscale_status = NEEDS_EXPOSE;
      
    } else if (event.xexpose.window ==
	       Glob->tscale_title_frame->x->drawable) {
      
      draw_tscale_title (XDEV);
      
    } else if (event.xexpose.window ==
	       Glob->thist_plot_frame->x->drawable) {

      Glob->thist_status = NEEDS_EXPOSE;
      
    } else if (event.xexpose.window ==
	       Glob->thist_title_frame->x->drawable) {

      draw_thist_title (XDEV);
      
    } else if (event.xexpose.window ==
	       Glob->timeht_plot_frame->x->drawable) {

      Glob->timeht_status = NEEDS_EXPOSE;
	
    } else if (event.xexpose.window ==
	       Glob->timeht_title_frame->x->drawable) {
	
      draw_timeht_title (XDEV);
	
    } else if (event.xexpose.window ==
	       Glob->timeht_scale_frame->x->drawable)  {

      Glob->timeht_scale_status = NEEDS_EXPOSE;
	
    } else if (event.xexpose.window ==
	       Glob->rdist_plot_frame->x->drawable) {
	
      Glob->rdist_status = NEEDS_EXPOSE;
	
    } else if (event.xexpose.window ==
	       Glob->rdist_title_frame->x->drawable) {
	
      draw_rdist_title (XDEV);
	
    } else if (event.xexpose.window ==
	       Glob->rdist_scale_frame->x->drawable)  {

      Glob->rdist_scale_status = NEEDS_EXPOSE;

    } else if (event.xexpose.window ==
	       Glob->help_title_frame->x->drawable) {
	
      draw_help_title ();
	
    } else if (event.xexpose.window ==
	       Glob->help_text_frame->x->drawable) {
	
      Glob->help_status = NEEDS_REDRAW;
	
    } else {
	
      for (i = 0; i < N_TSCALE_BUTTONS; i++) 
	if (event.xexpose.window ==
	    Glob->tscale_button_frame[i]->x->drawable) {
	  draw_tscale_button(i, Glob->background);
	  return;
	}
	
      for (i = 0; i < N_THIST_BUTTONS; i++)
	if (event.xexpose.window ==
	    Glob->thist_button_frame[i]->x->drawable) {
	  draw_thist_button(i, Glob->background);
	  return;
	}
	
      for (i = 0; i < N_TIMEHT_BUTTONS; i++)
	if (event.xexpose.window ==
	    Glob->timeht_button_frame[i]->x->drawable) {
	  draw_timeht_button(i, Glob->background);
	  return;
	}
	
      for (i = 0; i < N_RDIST_BUTTONS; i++)
	if (event.xexpose.window ==
	    Glob->rdist_button_frame[i]->x->drawable) {
	  draw_rdist_button(i, Glob->background);
	  return;
	}

      for (i = 0; i < N_HELP_BUTTONS; i++)
	if (event.xexpose.window ==
	    Glob->help_button_frame[i]->x->drawable) {
	  draw_help_button(i, Glob->background);
	  return;
	}

    }
      
  }
    
}
  
