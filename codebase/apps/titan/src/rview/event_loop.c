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
 * event_loop.c: process the event loop
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1990   
 *
 * Mike Dixon
 *
 *********************************************************************/

#define X11R3

#include "rview.h"
#include <signal.h>

static void configure_callback(void);
static void expose_callback(void);
static void button_press_callback(void);
static void button_release_callback(void);
static void motion_callback(void);
static void enter_callback(void);
static void leave_callback(void);

#define LEFT 1
#define MIDDLE 2
#define RIGHT 3

#define N_BUTTONS (N_CAPPI_BUTTONS + N_VSECTION_BUTTONS)

#define TRIVIAL_MOVE 20

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#define UI_ABSDIFF(a, b) ((ui32) fabs((double)(a) - (double)(b)))

static int button, ibutton;
static int pan_in_progress = FALSE;
static int zoom_in_progress = FALSE;
static int vsection_in_progress = FALSE;
static int start_x, start_y;
static int end_x, end_y;
static int take_action;

static si32 button_active[N_BUTTONS];
static si32 done = FALSE;

static double dx, dy, wwidth, wheight;

static Time prev_release_time = 0;
static double double_click_delta_time;

static XEvent event;

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
   * set function to respond to alarm signals
   */

  alarm_function = respond_to_timer;

  /*
   * the event loop
   */

  while (!done) {

    /*
     * get next event
     */

    XNextEvent(Glob->rdisplay, &event);

    switch (event.type) {
     
      /*
       * if main window resized, resize & redraw all windows
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
	Glob->help_requires_text = TRUE;
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

    case MotionNotify:

      motion_callback();
      break;

    } /* switch(event.type) */

  } /* while(!done) */

}

/**********************************************************************
 * button_press_callback() - processes the mouse button presses
 */

static void button_press_callback(void)
{

  si32 i;
    
  button = event.xbutton.button;
  Glob->coord_export->button = button;

  if (event.xbutton.window ==
      Glob->cappi_plot_frame->x->drawable) {
    
    if (Glob->help) {
      
      Glob->help_code = HELP_CAPPI_PLOT;
      
    } else {
      
      start_x = event.xbutton.x;
      start_y = event.xbutton.y;
      end_x = event.xbutton.x;
      end_y = event.xbutton.y;
      
      if (button == LEFT) {
	
	/*
	 * start zoom sequence
	 */
	
	zoom_in_progress = TRUE;
	
	/*
	 * draw xor rectangle
	 */
	
	XDrawRectangle(Glob->rdisplay,
		       Glob->cappi_plot_frame->x->drawable,
		       Glob->xor_gc,
		       MIN(start_x, end_x),
		       MIN(start_y, end_y),
		       UI_ABSDIFF(start_x, end_x),
		       UI_ABSDIFF(start_y, end_y));
	
      } else if (button == MIDDLE) {
	
	if (Glob->zoom_level > 0) {
	  
	  /*
	   * start pan sequence
	   */
	  
	  pan_in_progress = TRUE;
	  
	  /*
	   * draw xor line
	   */
	  
	  XDrawLine(Glob->rdisplay, Glob->cappi_plot_frame->x->drawable,
		    Glob->xor_gc,
		    start_x, start_y, end_x, end_y);
	  
	}
	
      } else if (button == RIGHT) {
	
	/*
	 * start vsection sequence
	 */
	
	vsection_in_progress = TRUE;
	
	/*
	 * draw xor line
	 */
	
	XDrawLine(Glob->rdisplay, Glob->cappi_plot_frame->x->drawable,
		  Glob->xor_gc,
		  start_x, start_y, end_x, end_y);
	
      }

    } /* if (Glob->help) */
    
  } else if (event.xbutton.window ==
	     Glob->vsection_plot_frame->x->drawable) {
    
    if (Glob->help) 
      Glob->help_code = HELP_VSECTION_PLOT;
      
  } else {

    /*
     * menu button presses
     */

    ibutton = 0;

    for (i = 0; i < N_CAPPI_BUTTONS; i++) {
      if (event.xbutton.window ==
	  Glob->cappi_button_frame[i]->x->drawable) {
	button_active[ibutton] = TRUE;
	Glob->help_code = ibutton;
	return;
      }
      ibutton++;
    } /* i */

    for (i = 0; i < N_VSECTION_BUTTONS; i++) {
      if (event.xbutton.window ==
	  Glob->vsection_button_frame[i]->x->drawable) {
	button_active[ibutton] = TRUE;
	Glob->help_code = ibutton;
	return;
      }
      ibutton++;
    } /* i */

    for (i = 0; i < N_HELP_BUTTONS; i++) {
      if (event.xbutton.window ==
	  Glob->help_button_frame[i]->x->drawable) {
	button_active[ibutton] = TRUE;
	Glob->help_code = ibutton;
	return;
      }
      ibutton++;
    } /* i */

  } /*   if (event.xbutton.window ... */

}

/**********************************************************************
 * button_release_callback() - processes the mouse button releases
 */

static void button_release_callback(void)

{

  int window_y;
  si32 i;
  int do_move;
  zoom_t *zoom;

  zoom = Glob->zoom + Glob->zoom_level;

  double_click_delta_time = fabs ((double) event.xbutton.time -
				  (double) prev_release_time);

  prev_release_time = event.xbutton.time;
  
  button = event.xbutton.button;

  /*
   * plot window button actions
   */

  if(event.xbutton.window == Glob->cappi_plot_frame->x->drawable) {

    /*
     * set shmem to indicate mouse pointer location, and increment
     * the pointer sequence number
     */
    
    Glob->coord_export->pointer_x =
      GXWorldx(Glob->cappi_plot_frame, end_x);
    Glob->coord_export->pointer_y =
      GXWorldy(Glob->cappi_plot_frame, end_y);
    Glob->coord_export->pointer_seq_num++;

    /*
     * check for trivial move
     */
    
    if (abs(start_x - end_x) > TRIVIAL_MOVE ||
	abs(start_y - end_y) > TRIVIAL_MOVE) {
      do_move = TRUE;
    } else {
      do_move = FALSE;
    }

    if (button == LEFT && zoom_in_progress) {
      
      /*
       * complete zoom sequence
       */
      
      zoom_in_progress = FALSE;
      
      /*
       * erase rectangle
       */
      
      XDrawRectangle(Glob->rdisplay,
		     Glob->cappi_plot_frame->x->drawable,
		     Glob->xor_gc,
		     MIN(start_x, end_x),
		     MIN(start_y, end_y),
		     UI_ABSDIFF(start_x, end_x),
		     UI_ABSDIFF(start_y, end_y));
      
      if (do_move) {

	/*
	 * perform zoom
	 */
	
	if (Glob->zoom_level < NZOOM - 1)
	  Glob->zoom_level++;
	
	for (i = Glob->zoom_level + 1; i < NZOOM; i++)
	  Glob->zoom[i].active = FALSE;

	zoom = Glob->zoom + Glob->zoom_level;

	zoom->min_x =
	  GXWorldx(Glob->cappi_plot_frame, MIN(start_x, end_x));

	zoom->max_x =
	  GXWorldx(Glob->cappi_plot_frame, MAX(start_x, end_x));
	
	zoom->min_y =
	  GXWorldy(Glob->cappi_plot_frame, MAX(start_y, end_y));

	zoom->max_y =
	  GXWorldy(Glob->cappi_plot_frame, MIN(start_y, end_y));
	
	zoom->active = TRUE;
	Glob->cursor_active = FALSE;

      } /* if (do_move) */

      zoom->current = FALSE;
    
    } else if (button == MIDDLE && pan_in_progress) {
	
      /*
       * complete pan sequence
       */
	
      pan_in_progress = FALSE;
      
      /*
       * erase line
       */
      
      XDrawLine(Glob->rdisplay, Glob->cappi_plot_frame->x->drawable,
		Glob->xor_gc,
		start_x, start_y, end_x, end_y);
      
      if (do_move) {

	/*
	 * perform pan
	 */
	
	dx = (GXWorldx(Glob->cappi_plot_frame, start_x)
	      - GXWorldx(Glob->cappi_plot_frame, end_x));
	dy = (GXWorldy(Glob->cappi_plot_frame, start_y)
	      - GXWorldy(Glob->cappi_plot_frame, end_y));
	
	wwidth = zoom->max_x - zoom->min_x;
	wheight = zoom->max_y - zoom->min_y;
	
	zoom->min_x += dx;
	zoom->max_x += dx;
	zoom->min_y += dy;
	zoom->max_y += dy;
	
	if (dx > 0 && zoom->max_x > Glob->full_max_x) {
	  zoom->max_x = Glob->full_max_x;
	  zoom->min_x = Glob->full_max_x - wwidth;
	  XBell(Glob->rdisplay, 0);
	}
	
	if (dy > 0 && zoom->max_y > Glob->full_max_y) {
	  zoom->max_y = Glob->full_max_y;
	  zoom->min_y = Glob->full_max_y - wheight;
	  XBell(Glob->rdisplay, 0);
	}
	
	if (dx < 0 && zoom->min_x < Glob->full_min_x) {
	  zoom->min_x = Glob->full_min_x;
	  zoom->max_x = Glob->full_min_x + wwidth;
	  XBell(Glob->rdisplay, 0);
	}
	
	if (dy < 0 && zoom->min_y < Glob->full_min_y) {
	  zoom->min_y = Glob->full_min_y;
	  zoom->max_y = Glob->full_min_y + wheight;
	  XBell(Glob->rdisplay, 0);
	}

      } /* if (do_move) */
	
      zoom->current = FALSE;
      Glob->cursor_active = FALSE;
	
    } else if (button == RIGHT && vsection_in_progress) {
	
      /*
       * complete vsection sequence
       */
      
      vsection_in_progress = FALSE;
      
      /*
       * erase line
       */
      
      XDrawLine(Glob->rdisplay, Glob->cappi_plot_frame->x->drawable,
		Glob->xor_gc,
		start_x, start_y, end_x, end_y);
      
      if (do_move) {

	/*
	 * perform vsection if non-trivial section chosen
	 */
	
	if (abs(start_x - end_x) > TRIVIAL_MOVE ||
	    abs(start_y - end_y) > TRIVIAL_MOVE) {
	  
	  Glob->vsection.req_start_x =
	    GXWorldx(Glob->cappi_plot_frame, start_x);
	  Glob->vsection.req_end_x =
	    GXWorldx(Glob->cappi_plot_frame, end_x);
	  Glob->vsection.req_start_y =
	    GXWorldy(Glob->cappi_plot_frame, start_y);
	  Glob->vsection.req_end_y =
	    GXWorldy(Glob->cappi_plot_frame, end_y);
	  
	  if (!Glob->vsection.active) {
	    
	    setup_vsection_windows();
	    XMapRaised (Glob->rdisplay, Glob->vsection_window);
	    XMapSubwindows (Glob->rdisplay, Glob->vsection_window);
	    set_vsection_sens();
	    safe_XFlush(Glob->rdisplay);
	    Glob->vsection.active = TRUE;
	    
	  } else {
	    
	    XRaiseWindow(Glob->rdisplay, Glob->vsection_window);
	    safe_XFlush(Glob->rdisplay);
	    
	  } /* if (!Glob->vsection.active) */
	  
	  Glob->vsection.current = FALSE;
	  Glob->main_scale_invalid = TRUE;
	  Glob->cursor_active = FALSE;
	  
	} /* if (abs( ... */

      } /* if (do_move) */
	
      zoom->current = FALSE;
      Glob->cursor_active = FALSE;

    }
      
    if (!do_move) {

      /*
       * for left or right buttons, display cursor posn info
       */
      
      if (button == LEFT || button == RIGHT) {

	static double prev_x = 0.0;
	static double prev_y = 0.0;
	double bearing;
	
	/*
	 * display info from cursor posn in window header
	 */
	
	Glob->cursor_active = TRUE;
	Glob->cursor_x = GXWorldx(Glob->cappi_plot_frame, end_x);
	Glob->cursor_y = GXWorldy(Glob->cappi_plot_frame, end_y);

	/*
	 * LEFT button - range and bearing from radar.
	 * RIGHT button - range and bearing from prev point
	 */
	
	if (button == LEFT) {
	  dx = Glob->cursor_x;
	  dy = Glob->cursor_y;
	} else {
      	  dx = Glob->cursor_x - prev_x;
	  dy = Glob->cursor_y - prev_y;
	}
	
	Glob->cursor_dist = sqrt(dx * dx + dy * dy);
      
	if (Glob->cursor_dist_nm) {
	  /* convert from km to nm */
	  Glob->cursor_dist /= KM_PER_NM;
	}
	
	if (dx == 0.0 && dy == 0.0) {
	  bearing = 0.0;
	} else {
	  bearing = atan2(dx, dy) * RAD_TO_DEG;
	}
	
	if (Glob->cursor_magnetic) {
	  Glob->cursor_bearing = bearing + Glob->magnetic_variation;
	} else {
	  Glob->cursor_bearing = bearing;
	}
	if (Glob->cursor_bearing > 360.0) {
	  Glob->cursor_bearing -= 360.0;
	} else if (Glob->cursor_bearing < 0.0) {
	  Glob->cursor_bearing += 360.0;
	}
	
	prev_x = Glob->cursor_x;
	prev_y = Glob->cursor_y;
	
      } else {

	Glob->cursor_active = FALSE;

      }
      
      zoom->current = FALSE;

    } /* if (!do_move) */
      
    /*
     * Check for double click - if there is a double click, it
     * means that the user wants to select or add a track.
     */

    if (double_click_delta_time < Glob->double_click_delay &&
	Glob->use_track_data) {

      switch (button) {

      case LEFT:
	Glob->track_shmem->select_track_type = SIMPLE_TRACK;
	break;
	
      case MIDDLE:
	Glob->track_shmem->select_track_type = PARTIAL_TRACK;
	break;
	
      case RIGHT:
	Glob->track_shmem->select_track_type = COMPLEX_TRACK;
	break;
	
      }
	
      Glob->coord_export->focus_x = Glob->coord_export->pointer_x;
      Glob->coord_export->focus_y = Glob->coord_export->pointer_y;

    } /* if (double_click_delta_time ...... */
    
  } else if(event.xbutton.window == Glob->vsection_plot_frame->x->drawable) {
    
    /*
     * check for double click - if there is a double click, it
     * means that the user wants to change cappi ht
     */

    if (double_click_delta_time < Glob->double_click_delay) {

      window_y = event.xbutton.y;

      Glob->z_requested = 
	GXWorldy(Glob->vsection_plot_frame, window_y);

      set_cappi_invalid();
      set_vsection_invalid();

    }
      
  } else {
    
    /*
     * menu button action
     */

    if(button_active[ibutton]) {

      take_action = FALSE;

      switch(ibutton) {

      case CAPPI_HELP:

	Glob->help = TRUE;
	Glob->help_requires_text = TRUE;
	setup_help_windows();
	set_help_sens();
	XMapRaised (Glob->rdisplay, Glob->help_window);
	XMapSubwindows (Glob->rdisplay, Glob->help_window);
	safe_XFlush(Glob->rdisplay);
    
	break;
	
      case CAPPI_LEVEL:

	/*
	 * move up, down or to bottom, for right, left, middle buttons
	 */

	switch(button) {

	case RIGHT:
	
	  /*
	   * move plot altitude up
	   */
	
	  if (Glob->plot_composite) {

	    /*
	     * was composite, reset
	     */

	    Glob->plot_composite = FALSE;
	    take_action = TRUE;

	  } else {

	    Glob->z_requested += Glob->delta_z;
	    take_action = TRUE;

	  }
	
	  break;
	
	case LEFT:
	
	  /*
	   * move plot altitude down
	   */
	
	  if (Glob->plot_composite) {

	    /*
	     * was composite, reset
	     */

	    Glob->plot_composite = FALSE;
	    take_action = TRUE;

	  } else {

	    Glob->z_requested -= Glob->delta_z;
	    take_action = TRUE;

	  }
	    
	  break;
	
	case MIDDLE:
	
	  /*
	   * get composite plot
	   */
	
	  Glob->plot_composite = TRUE;
	  take_action = TRUE;
	  
	  break;

	} /* switch (button) */

	if (take_action) {
	  set_cappi_invalid();
	  set_vsection_invalid();
	}
	  
	break;  /* end of level change actions */
	
      case CAPPI_FIELD:

	/*
	 * move up, down or to first field, depending upon which button
	 * was pressed
	 */

	switch (button) {
	
	case RIGHT:

	  /*
	   * move up by one field
	   */
	
	  if (Glob->field != Glob->nfields - 1) {
	    Glob->field++;
	    take_action = TRUE;
	  }
	
	  break;
	
	case LEFT:
	
	  /*
	   * move down by one field
	   */
	
	  if (Glob->field != 0) {
	    Glob->field--;
	    take_action = TRUE;
	  }
	
	  break;

	case MIDDLE:
	
	  /*
	   * move to field 0
	   */
	
	  if (Glob->field != 0) {
	    Glob->field = 0;
	    take_action = TRUE;
	  }
	
	  break;

	} /* switch (button) */

	if (take_action) {
	  set_cappi_invalid();
	  set_vsection_invalid();
	  Glob->cappi_title_invalid = TRUE;
	  Glob->vsection_title_invalid = TRUE;
	  Glob->main_scale_invalid = TRUE;
	}
	  
	break;  /* end of field change actions */

      case CAPPI_ZOOM:

	/*
	 * set zoom state - left unzoomed, right zoomed
	 */

	switch (button) {

	case LEFT:

	  /*
	   * zoom down
	   */

	  if (Glob->zoom_level > 0) {
	    Glob->zoom_level--;
	    Glob->cappi_requires_expose = TRUE;
	  }

	  break;

	case MIDDLE:

	  /*
	   * cancel all zooms, set zoom level to 0
	   */

	  Glob->zoom_level = 0;
	  for (i = 1; i < NZOOM; i++)
	    Glob->zoom[i].active = FALSE;
	  Glob->zoom[0].current = FALSE;

	  break;

	case RIGHT:

	  /*
	   * zoom up
	   */

	  if (Glob->zoom_level < NZOOM - 1 &&
	      Glob->zoom[Glob->zoom_level + 1].active) {

	    Glob->zoom_level++;
	    Glob->cappi_requires_expose = TRUE;

	  }

	  break;

	} /* switch */

	break;
	
      case CAPPI_CONT:

	/*
	 * set the plot_cappi_contours state - left off, right on
	 */

	if (button == LEFT) {
	  if (Glob->plot_cappi_contours) {
	    Glob->plot_cappi_contours = FALSE;
	    take_action = TRUE;
	  }
	} else if (button == RIGHT) {
	  if (Glob->plot_cappi_contours == FALSE) {
	    Glob->plot_cappi_contours = TRUE;
	    take_action = TRUE;
	  }
	} /* if */

	if (take_action) {
	  set_cappi_invalid();
	}

	break;

      case CAPPI_IMAGE:

	/*
	 * set the plot_image state
	 *
	 * left toggles image on/off
	 * middle toggles runs on/off
	 * right toggles run fill on/off
	 */

	if (button == LEFT) {
	  Glob->plot_image = !Glob->plot_image;
	  take_action = TRUE;
	} else if (button == MIDDLE) {
	  Glob->fill_runs = !Glob->fill_runs;
	  take_action = TRUE;
	} else if (button == RIGHT) {
	  Glob->plot_runs = !Glob->plot_runs;
	  take_action = TRUE;
	} /* if */

	if (take_action) {
	  set_cappi_invalid();
	  set_vsection_invalid();
	  Glob->main_scale_invalid = TRUE;
	}

	break;

      case CAPPI_RINGS:

	/*
	 * set the plot_rings state - left off, right on
	 */

	if (button == LEFT) {
	  if (Glob->plot_rings) {
	    Glob->plot_rings = FALSE;
	    take_action = TRUE;
	  }
	} else if (button == RIGHT) {
	  if (Glob->plot_rings == FALSE) {
	    Glob->plot_rings = TRUE;
	    take_action = TRUE;
	  }
	} /* if */

	if (take_action) {
	  set_cappi_invalid();
	}

	break;

      case CAPPI_MAPS:

	/*
	 * set the plot_maps state - left off, middle limited, right all
	 */

	if (button == LEFT) {
	  if (Glob->plot_maps != FALSE) {
	    Glob->plot_maps = FALSE;
	    take_action = TRUE;
	  }
	} else if (button == MIDDLE) {
	  if (Glob->plot_maps != MAPS_LIMITED) {
	    Glob->plot_maps = MAPS_LIMITED;
	    take_action = TRUE;
	  }
	} else if (button == RIGHT) {
	  if (Glob->plot_maps != MAPS_ALL) {
	    Glob->plot_maps = MAPS_ALL;
	    take_action = TRUE;
	  }
	} /* if */

	if (take_action) {
	  set_cappi_invalid();
	}

	break;

      case CAPPI_TRACKS:

	switch (button) {
	      
	case LEFT:
	      
	  /*
	   * plot no tracks
	   */
	  
	  if (Glob->plot_tracks) {
	    Glob->plot_tracks = FALSE;
	    take_action = TRUE;
	  }
	  break;
	  
	case MIDDLE:
	      
	  /*
	   * plot selected track
	   */
	      
	  if (Glob->plot_tracks != SELECTED_TRACK) {
	    Glob->plot_tracks = SELECTED_TRACK;
	    take_action = TRUE;
	  }
	  break;
	      
	case RIGHT:
	  
	  /*
	   * plot entire tracks
	   */
	  
	  if (Glob->plot_tracks != ALL_TRACKS) {
	    Glob->plot_tracks = ALL_TRACKS;
	    take_action = TRUE;
	  }
	    
	  break;
	      
	} /* switch (button) */

	if (take_action) {
	  set_cappi_sens();
	  set_cappi_invalid();
	  Glob->main_scale_invalid = TRUE;
	}
	    
	break;
	
      case CAPPI_TRACK_GRAPHIC:

	/*
	 * check that there is track data available
	 */
	  
	switch (button) {
	      
	case LEFT:
	      
	  /*
	   * togle plot_vectors
	   */

	  Glob->plot_vectors = !Glob->plot_vectors;
	  take_action = TRUE;
	  break;
	    
	case MIDDLE:
	  
	  /*
	   * graphic shape - vectors, ellipses or none
	   */
	      
	  if (Glob->track_graphic == FALSE) {
	    Glob->track_graphic = ELLIPSES;
	  } else if (Glob->track_graphic == ELLIPSES) {
	    Glob->track_graphic = POLYGONS;
	  } else if (Glob->track_graphic == POLYGONS) {
	    Glob->track_graphic = FALSE;
	  }
	  take_action = TRUE;
	  break;
	      
	case RIGHT:
	      
	  /*
	   * toggle graphic shape fill
	   */
	      
	  Glob->fill_graphic = !Glob->fill_graphic;
	  take_action = TRUE;
	  break;
	      
	} /* switch (button) */

	if (take_action) {
	  set_cappi_invalid();
	  Glob->main_scale_invalid = TRUE;
	}
	    
	break;
	
      case CAPPI_TRACK_ANNOTATE:

	switch (button) {
	  
	case LEFT:
	      
	  /*
	   * decrease annotation selection
	   */

	  if (Glob->annotate_tracks > 0) {
	    Glob->annotate_tracks--;
	    take_action = TRUE;
	  }
	  break;
	    
	case MIDDLE:
	      
	  /*
	   * annotation off
	   */

	  if (Glob->annotate_tracks != FALSE) {
	    Glob->annotate_tracks = FALSE;
	    take_action = TRUE;
	  }
	  break;
	    
	case RIGHT:
	      
	  /*
	   * increase annotation selection
	   */

	  if (Glob->annotate_tracks < N_ANNOTATIONS - 1) {
	    Glob->annotate_tracks++;
	    take_action = TRUE;
	  }
	  break;
	    
	} /* switch (button) */

	if (take_action) {
	  set_cappi_invalid();
	}
	    
	break;
	
      case CAPPI_PAST:

	/*
	 * set the plot_past state - left off, middle limited, right all
	 */

	switch (button) {

	case LEFT:
	  
	  if (Glob->plot_past != FALSE) {
	    Glob->plot_past = FALSE;
	    take_action = TRUE;
	  }
	  break;

	case MIDDLE:

	  if (Glob->plot_past != PAST_LIMITED) {
	    Glob->plot_past = PAST_LIMITED;
	    take_action = TRUE;
	  }
	  break;

	case RIGHT:

	  if (Glob->plot_past != PAST_ALL) {
	    Glob->plot_past = PAST_ALL;
	    take_action = TRUE;
	  }
	  break;

	} /* switch */

	if (take_action) {
	  set_cappi_invalid();
	  Glob->main_scale_invalid = TRUE;
	}

	break;

      case CAPPI_FORECAST:

	switch (button) {
	  
	case LEFT:
	    
	  /*
	   * plot no forecast
	   */
	    
	  if (Glob->plot_forecast) {
	    Glob->plot_forecast = FALSE;
	    take_action = TRUE;
	  }
	  break;
	      
	case MIDDLE:
	    
	  /*
	   * plot limited forecast - from current time only
	   */
	    
	  if (Glob->plot_forecast != FORECAST_LIMITED) {
	    Glob->plot_forecast = FORECAST_LIMITED;
	    take_action = TRUE;
	  }
	  break;
	  
	case RIGHT:
	      
	  /*
	   * plot all track forecasts
	   */
	  
	  if (Glob->plot_forecast != FORECAST_ALL) {
	    Glob->plot_forecast = FORECAST_ALL;
	    take_action = TRUE;
	  }
	  break;
	  
	} /* switch (button) */
	    
	if(take_action) {
	  set_cappi_invalid();
	  Glob->main_scale_invalid = TRUE;
	  Glob->track_shmem->plot_forecast = Glob->plot_forecast;
	}
	    
	break;
	
      case CAPPI_FUTURE:
	
	/*
	 * set the plot_future state - left off, middle limited, right all
	 */

	switch (button) {

	case LEFT:

	  if (Glob->plot_future != FALSE) {
	    Glob->plot_future = FALSE;
	    take_action = TRUE;
	  }
	  break;

	case MIDDLE:

	  if (Glob->plot_future != FUTURE_LIMITED) {
	    Glob->plot_future = FUTURE_LIMITED;
	    take_action = TRUE;
	  }
	  break;

	case RIGHT:

	  if (Glob->plot_future != FUTURE_ALL) {
	    Glob->plot_future = FUTURE_ALL;
	    take_action = TRUE;
	  }
	  break;

	} /* switch */
	
	if (take_action) {
	  set_cappi_invalid();
	  Glob->main_scale_invalid = TRUE;
	}
	
	break;

      case CAPPI_TIME:

	/*
	 * increment or decrement time, depending upon which button
	 * was pressed.
	 */

	switch (button) {
	      
	case LEFT:

	  Glob->time -= Glob->scan_delta_t;
	  take_action = TRUE;
	  break;
	      
	case RIGHT:
	  
	  Glob->time += Glob->scan_delta_t;
	  take_action = TRUE;
	  break;

	} /*switch */
	      
	if (take_action) {
	  set_cappi_invalid();
	  set_vsection_invalid();
	}

	break;
	
      case CAPPI_COPY:
	/*
	 * Copy the CAPPI to a PostScript file or to an xwd file
	 * depending upon which button was pressed.
	 */

	switch (button) {
	      
	case LEFT:

	  /*
	   * Copy the CAPPI to a printer.
	   */

	  copy_cappi();
	  break;
	      
	case RIGHT:
	  
	  /*
	   * Copy the CAPPI to a WEB file.
	   */

	  copy_cappi_web();
	  
	  break;

	} /*switch */
	      
	break;
	
      case CAPPI_QUIT:

	done = TRUE;

	break;

      case VSECTION_INTERP:

	/*
	 * set the vsection interp state - left off, right on
	 */

	if (button == LEFT) {
	  if (Glob->vsection_interp) {
	    Glob->vsection_interp = FALSE;
	    take_action = TRUE;
	  }
	} else if (button == RIGHT) {
	  if (Glob->vsection_interp == FALSE) {
	    Glob->vsection_interp = TRUE;
	    take_action = TRUE;
	  }
	} /* if */

	if (take_action) {
	  set_vsection_invalid();
	}

	break;

      case VSECTION_CONT:
	
	/*
	 * set the plot_vsection_contours state - left off, right on
	 */

	if (button == LEFT) {
	  if (Glob->plot_vsection_contours) {
	    Glob->plot_vsection_contours = FALSE;
	    take_action = TRUE;
	  }
	} else if (button == RIGHT) {
	  if (Glob->plot_vsection_contours == FALSE) {
	    Glob->plot_vsection_contours = TRUE;
	    take_action = TRUE;
	  }
	} /* if */

	if (take_action) {
	  set_vsection_invalid();
	}

	break;

      case VSECTION_COPY:

	/*
	 * Copy the vertical cross-section to a PostScript file or
	 * to an xwd file depending upon which button was pressed.
	 */

	switch (button) {
	      
	case LEFT:

	  /*
	   * Copy the vertical cross-section to a PostScript file.
	   */

	  copy_vsection();
	  break;
	      
	case RIGHT:
	  
	  /*
	   * Copy the vertical cross-section to a WEB file.
	   */

	  copy_vsection_web();
	  
	  break;

	} /*switch */
	      
	break;

      case VSECTION_CLOSE:
	
	XSelectInput(Glob->rdisplay, Glob->vsection_window, NoEventMask);

#ifdef X11R3
	XUnmapWindow(Glob->rdisplay, Glob->vsection_window);
#else
	XIconifyWindow(Glob->rdisplay, Glob->vsection_window,
		       Glob->rscreen);
#endif

	Glob->vsection.active = FALSE;
	Glob->main_scale_invalid = TRUE;

      }
      
      /*
       * set window back to background color
       */
     
      if (ibutton < N_CAPPI_BUTTONS) {

	i = ibutton;

	draw_cappi_button(i, Glob->background);

      } else {

	i = ibutton - N_CAPPI_BUTTONS;

	draw_vsection_button(i, Glob->background);

      }

      button_active[ibutton] = FALSE;
      
    }
    
  }
  
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

  for (i = 0; i < N_CAPPI_BUTTONS; i++) {
    if (event.xcrossing.window ==
	Glob->cappi_button_frame[i]->x->drawable) {
      draw_cappi_button(i, Glob->hlight_background);
      return;
    }
  }

  for (i = 0; i < N_VSECTION_BUTTONS; i++) {
    if (event.xcrossing.window ==
	Glob->vsection_button_frame[i]->x->drawable) {
      draw_vsection_button(i, Glob->hlight_background);
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

  if (event.xcrossing.window == Glob->cappi_plot_frame->x->drawable) {
  
    if (pan_in_progress) {
      
      /*
       * abort pan sequence
       */
      
      /*
       * erase line
       */
      
      XDrawLine(Glob->rdisplay, Glob->cappi_plot_frame->x->drawable,
		Glob->xor_gc,
		start_x, start_y, end_x, end_y);
      
      pan_in_progress = FALSE;
      
      /*
       * enable the alarm signal interrupt
       */

      PORTsignal(SIGALRM, (void(*)()) alarm_function);

    } else if (zoom_in_progress) {
      
      /*
       * abort zoom sequence
       */
      
      /*
       * erase rectangle
       */
      
      XDrawRectangle(Glob->rdisplay, Glob->cappi_plot_frame->x->drawable,
		     Glob->xor_gc,
		     MIN(start_x, end_x),
		     MIN(start_y, end_y),
		     UI_ABSDIFF(start_x, end_x),
		     UI_ABSDIFF(start_y, end_y));
      
      
      zoom_in_progress = FALSE;
      
      /*
       * enable the alarm signal interrupt
       */

      PORTsignal(SIGALRM, (void(*)()) alarm_function);

    } else if (vsection_in_progress) {
      
      /*
       * abort vsection sequence
       */
      
      /*
       * erase line
       */
      
      XDrawLine(Glob->rdisplay, Glob->cappi_plot_frame->x->drawable,
		Glob->xor_gc,
		start_x, start_y, end_x, end_y);
      
      vsection_in_progress = FALSE;
      
      /*
       * enable the alarm signal interrupt
       */

      PORTsignal(SIGALRM, (void(*)()) alarm_function);

    }
    
  } else {

    /*
     * un_highlight window on mouse leave events
     */

    button_active[ibutton] = FALSE;
      
    for (i = 0; i < N_CAPPI_BUTTONS; i++) {
      if (event.xcrossing.window ==
	  Glob->cappi_button_frame[i]->x->drawable) {
	draw_cappi_button(i, Glob->background);
	return;
      }
    }

    for (i = 0; i < N_VSECTION_BUTTONS; i++) {
      if (event.xcrossing.window ==
	  Glob->vsection_button_frame[i]->x->drawable) {
	draw_vsection_button(i, Glob->background);
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

  } /* if (event.xcrossing.window ... */

}

/**********************************************************************
 * motion_callback() - processes the mouse motion events
 */

static void motion_callback(void)

{

  static gframe_t *frame;
  
  frame = Glob->cappi_plot_frame;

  while (XCheckMaskEvent(Glob->rdisplay, ButtonMotionMask, &event));
  
  if (event.xmotion.window == frame->x->drawable) {
    
    if (pan_in_progress) {
      
      /*
       * move pan position line
       */
      
      /*
       * erase old line
       */
      
      XDrawLine(Glob->rdisplay,
		frame->x->drawable,
		Glob->xor_gc,
		start_x, start_y, end_x, end_y);
      
      /*
       * get new position
       */
      
      end_x = event.xmotion.x;
      end_y = event.xmotion.y;
      
      /*
       * draw line in new position
       */
      
      XDrawLine(Glob->rdisplay,
		frame->x->drawable,
		Glob->xor_gc,
		start_x, start_y, end_x, end_y);
      
    } else if (zoom_in_progress) {
      
      /*
       * move zoom rectangle
       */
      
      /*
       * erase old rectangle
       */
      
      XDrawRectangle(Glob->rdisplay,
		     frame->x->drawable,
		     Glob->xor_gc,
		     MIN(start_x, end_x),
		     MIN(start_y, end_y),
		     UI_ABSDIFF(start_x, end_x),
		     UI_ABSDIFF(start_y, end_y));
      
      /*
       * get new position
       */
      
      end_x = event.xmotion.x;
      end_y = event.xmotion.y;
      
      /*
       * draw rectangle in new position
       */
      
      XDrawRectangle(Glob->rdisplay,
		     frame->x->drawable,
		     Glob->xor_gc,
		     MIN(start_x, end_x),
		     MIN(start_y, end_y),
		     UI_ABSDIFF(start_x, end_x),
		     UI_ABSDIFF(start_y, end_y));
      
    } else if (vsection_in_progress) {
      
      /*
       * move vsection position line
       */
      
      /*
       * erase old line
       */
      
      XDrawLine(Glob->rdisplay,
		frame->x->drawable,
		Glob->xor_gc,
		start_x, start_y, end_x, end_y);
      
      /*
       * get new position
       */
      
      end_x = event.xmotion.x;
      end_y = event.xmotion.y;
      
      /*
       * draw line in new position
       */
      
      XDrawLine(Glob->rdisplay,
		frame->x->drawable,
		Glob->xor_gc,
		start_x, start_y, end_x, end_y);
      
    }

  }

}

/**********************************************************************
 * configure_callback() - processes the configure notify events
 */

static void configure_callback(void)
{

  if(event.xconfigure.window == Glob->main_window) {
    
    if (Glob->mainwidth != event.xconfigure.width ||
	Glob->mainheight != event.xconfigure.height) {

      Glob->mainx = event.xconfigure.x;
      Glob->mainy = event.xconfigure.y;
      Glob->mainwidth = event.xconfigure.width;
      Glob->mainheight = event.xconfigure.height;
      
      XSelectInput(Glob->rdisplay, Glob->main_window, NoEventMask);
    
      setup_cappi_windows();
      set_cappi_sens();
      set_cappi_invalid();

    }
    
  } else if(event.xconfigure.window == Glob->vsection_window) {

    
    if (Glob->vsection_width != event.xconfigure.width ||
	Glob->vsection_height != event.xconfigure.height) {

      Glob->vsection_x = event.xconfigure.x;
      Glob->vsection_y = event.xconfigure.y;
      Glob->vsection_width = event.xconfigure.width;
      Glob->vsection_height = event.xconfigure.height;
      
      XSelectInput(Glob->rdisplay, Glob->vsection_window, NoEventMask);
      
      setup_vsection_windows();
      set_vsection_sens();
      set_vsection_invalid();

    }

  } else if(event.xconfigure.window == Glob->help_window) {

    if (Glob->help_width != event.xconfigure.width ||
	Glob->help_height != event.xconfigure.height) {
      
      Glob->help_x = event.xconfigure.x;
      Glob->help_y = event.xconfigure.y;
      Glob->help_width = event.xconfigure.width;
      Glob->help_height = event.xconfigure.height;
      
      XSelectInput(Glob->rdisplay, Glob->help_window, NoEventMask);
      
      setup_help_windows();
      set_help_sens();

    }

  }
  
}

/**********************************************************************
 * expose_callback() - processes the expose events
 */

static void expose_callback(void)
{

  si32 i;
  
  if (event.xexpose.count == 0) {
    
    if (event.xexpose.window == Glob->cappi_plot_frame->x->drawable) {
      
      Glob->cappi_requires_expose = TRUE;
      
    } else if (event.xexpose.window ==
	       Glob->cappi_title_frame->x->drawable) {
      
      Glob->cappi_title_invalid = TRUE;
      
    } else if (event.xexpose.window ==
	       Glob->vsection_plot_frame->x->drawable) {
	
      Glob->vsection_requires_expose = TRUE;
	
    } else if (event.xexpose.window ==
	       Glob->vsection_title_frame->x->drawable) {
	
      Glob->vsection_title_invalid = TRUE;
	
    } else if (event.xexpose.window ==
	       Glob->main_scale_frame->x->drawable)  {
	
      Glob->main_scale_invalid = TRUE;
	
    } else if (event.xexpose.window ==
	       Glob->help_title_frame->x->drawable) {
	
      Glob->help_title_invalid = TRUE;
	
    } else if (event.xexpose.window ==
	       Glob->help_text_frame->x->drawable) {
	
      Glob->help_requires_text = TRUE;
	
    } else {
	
      for (i = 0; i < N_CAPPI_BUTTONS; i++)
	if (event.xexpose.window ==
	    Glob->cappi_button_frame[i]->x->drawable) {
	  draw_cappi_button(i, Glob->background);
	  return;
	}
	
      for (i = 0; i < N_VSECTION_BUTTONS; i++)
	if (event.xexpose.window ==
	    Glob->vsection_button_frame[i]->x->drawable) {
	  draw_vsection_button(i, Glob->background);
	  return;
	}
	
      for (i = 0; i < N_HELP_BUTTONS; i++)
	if (event.xexpose.window ==
	    Glob->help_button_frame[i]->x->drawable) {
	  draw_help_button(i, Glob->background);
	  return;
	}
	
    }  /* if (event.xexpose.count == 0) */
    
  } /* if (event.xexpose.window ... */
    
}

