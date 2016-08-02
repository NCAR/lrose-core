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
/************************************************************************
 * draw_tscale.c
 *
 * time scale
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "time_hist.h"

#define TICK_CLEARANCE_FRACTION 0.5
#define APPROX_N_TICKS (si32) 10

#define TIME_BAR_MIN_Y 0.0
#define TIME_BAR_ONE_THIRD_Y 0.2
#define TIME_BAR_MID_Y 0.4
#define TIME_BAR_TWO_THIRDS_Y 0.6
#define TIME_BAR_MAX_Y 0.8

/************************************************************************
 * draw_tscale_title()
 *
 * draw title for time scale
 */

void draw_tscale_title(int dev)

{

  gframe_t *frame;
  char tstring[128];
  double header_clearance_x, header_clearance_y;

  if (Glob->debug) {
    fprintf(stderr, "** draw_tscale_title **\n");
  }

  frame = Glob->tscale_title_frame;

  switch (dev) {

  case XDEV:
    GXSetGeomFrame(frame, frame->x->width, frame->x->height);
    header_clearance_x = Glob->x_header_clearance / frame->x->xscale;
    header_clearance_y = Glob->x_header_clearance / frame->x->yscale;
    break;

  case PSDEV:
    GPsSetGeomFrame(frame, frame->ps->xmin, frame->ps->ymin,
		    frame->ps->width, frame->ps->height);
    header_clearance_x = Glob->ps_header_clearance / frame->x->xscale;
    header_clearance_y = Glob->ps_header_clearance / frame->x->yscale;
    break;

  }

  /*
   * clear window
   */

  if (dev == XDEV) {
    XSetWindowBackground(Glob->rdisplay, frame->x->drawable,
			 Glob->background);
    safe_XClearWindow(Glob->rdisplay, frame->x->drawable);
  }

  sprintf(tstring, "TRACK DATA TIME SCALE");

  GDrawString(dev, frame, frame->x->gc, frame->x->font,
	      frame->psgc, XJ_CENTER, YJ_CENTER,
	      (double) 0.5, (double) 0.5, tstring);

  sprintf(tstring, "%s",
	  utimstr(Glob->track_shmem->time));

  GDrawString(dev, frame, Glob->header_gc, Glob->x_header_font,
	      frame->psgc, XJ_LEFT, YJ_BELOW,
	      frame->w_xmin + header_clearance_x,
	      frame->w_ymax - header_clearance_y,
	      tstring);
    
  safe_XFlush(Glob->rdisplay);

}

/************************************************************************
 * draw_tscale_button()
 *
 * Draws a button in the time scale window
 */

void draw_tscale_button(si32 n,
			ui32 background)

{

  static si32 first_call = TRUE;
  static char **button_label;
  char *label_str;
  int i;
  gframe_t *frame;

  if (Glob->debug) {
    fprintf(stderr, "** draw_tscale_button **\n");
  }

  frame = Glob->tscale_button_frame[n];

  /*
   * if first call, load up the label strings
   */

  if (first_call == TRUE) {

    button_label = (char **) umalloc (N_TSCALE_BUTTONS * sizeof(char *));

    label_str = (char *) umalloc((ui32) (strlen(TSCALE_BUTTON_LABELS) + 1));
    strcpy(label_str, TSCALE_BUTTON_LABELS);

    for (i = 0; i < N_TSCALE_BUTTONS; i++) {

      if (i == 0)
	button_label[i] = strtok(label_str, " ");
      else
	button_label[i] = strtok((char *) NULL, " ");

    }

    first_call = FALSE;

  }

  /*
   * clear window and set background
   */

  XSetWindowBackground(Glob->rdisplay, frame->x->drawable,
		       background);
  safe_XClearWindow(Glob->rdisplay, frame->x->drawable);

  /*
   * write the label to the window
   */

  GDrawString(XDEV, frame, frame->x->gc, frame->x->font,
	      frame->psgc, XJ_CENTER, YJ_CENTER,
	      (double) 0.5, (double) 0.5, button_label[n]);

  safe_XFlush(Glob->rdisplay);

}

/************************************************************************
 * draw_tscale_plot()
 *
 * Draws the track data time scale
 */

void draw_tscale_plot(int dev)

{

  char label[80];

  int i;
  si32 n_time_ticks;
  si32 tick_time_interval;

  double plot_min_x, plot_max_x, plot_min_y, plot_max_y;
  double plot_x_range, window_x_range;
  double window_min_x, window_max_x;
  double plot_y_range;
  double window_min_y, window_max_y;
  double xaxis_margin, yaxis_margin, top_margin;
  double xtick_length, xtick_margin;
  double xaxis_endmargin;
  double tick_x;
  double text_y;
  double header_clearance_x;
  double delta_mins;
  double current_time, current_start, current_end;
  double past_start, future_end, forecast_end;

  date_time_t tick_time;
  gframe_t *frame;
  thist_track_data_t *tdata;
  time_hist_shmem_t *tshmem;

  if (Glob->debug) {
    fprintf(stderr, "** draw_tscale_plot **\n");
  }

  /*
   * set pointers
   */

  frame = Glob->tscale_plot_frame;
  tdata = &Glob->tdata;
  tshmem = Glob->track_shmem;

  /*
   * clear window
   */

  if (dev == XDEV)
    safe_XClearWindow(Glob->rdisplay, frame->x->drawable);

  /*
   * get margins depending on X or postscript
   */

  switch (dev) {

  case XDEV:

    yaxis_margin = Glob->x_tscale_yaxis_margin / (double) (frame->x->width);
    xaxis_margin = Glob->x_xaxis_margin / (double) (frame->x->height);
    top_margin = 0.0;
    break;

  case PSDEV:

    yaxis_margin = Glob->ps_tscale_yaxis_margin / (double) (frame->ps->width);
    xaxis_margin = Glob->ps_xaxis_margin / (double) (frame->ps->height);
    top_margin = 0.0;
    break;

  } /* switch */

  /*
   * compute world coordinate window limits based on the data limits
   * and the margins - time scale has extra 60 secs added to either end
   */

  plot_min_x =
    (double) Glob->tdata_index.complete.header.data_start_time;
  plot_max_x =
    (double) (Glob->tdata_index.complete.header.data_end_time +
	      Glob->track_shmem->n_forecast_steps *
	      Glob->track_shmem->forecast_interval);

  plot_min_y = 0.0;
  plot_max_y = 1.0;

  window_min_x = plot_min_x;
  plot_x_range = plot_max_x - plot_min_x;
  window_x_range = plot_x_range * (1.0 / (1.0 - yaxis_margin));
  window_max_x = window_min_x + window_x_range;

  plot_y_range = plot_max_y - plot_min_y;

  window_min_y = plot_min_y -
    (xaxis_margin / (1.0 - xaxis_margin - top_margin)) * plot_y_range;

  window_max_y = plot_max_y +
    (top_margin / (1.0 - xaxis_margin - top_margin)) * plot_y_range;

  GMoveFrameWorld(frame,
		  window_min_x,
		  window_min_y,
		  window_max_x,
		  window_max_y);

  switch (dev) {

  case XDEV:
    GXSetGeomFrame(frame, frame->x->width, frame->x->height);
    xtick_length = Glob->x_tick_length / frame->x->yscale;
    xtick_margin = xtick_length * TICK_CLEARANCE_FRACTION;
    xaxis_endmargin = Glob->x_xaxis_endmargin / frame->x->xscale;
    header_clearance_x = Glob->x_header_clearance / frame->x->xscale;
    break;

  case PSDEV:
    GPsSetGeomFrame(frame, frame->ps->xmin, frame->ps->ymin,
		    frame->ps->width, frame->ps->height);
    xtick_length = Glob->ps_tick_length / frame->ps->yscale;
    xtick_margin = xtick_length * TICK_CLEARANCE_FRACTION;
    xaxis_endmargin = Glob->ps_xaxis_endmargin / frame->x->xscale;
    header_clearance_x = Glob->ps_header_clearance / frame->x->xscale;
    break;

  }

  /*
   * plot time scale data
   */
  
  if (dev == PSDEV)
    PsGsave(frame->psgc->file);

  /*
   * plotting stuff here - to be added
   */

  if (dev == PSDEV)
    PsGrestore(frame->psgc->file);
  
  /*
   * draw in divider between tick marks and data
   */

  if (dev == PSDEV)
    PsSetLineWidth(frame->psgc->file, Glob->ps_divider_line_width);

  /*
   * if printer, set tick line width and ticklabel fontsize
   */

  if (dev == PSDEV) {

    PsSetLineWidth(frame->psgc->file,
		   Glob->ps_tick_line_width);

    PsSetFont(frame->psgc->file, frame->psgc->fontname,
	      Glob->ps_ticklabel_fontsize);

  }

  /*
   * x axis tick marks and labels
   */

  delta_mins = (plot_max_x - plot_min_x) / 60.0;

  if (delta_mins > 1440)
    tick_time_interval = 240 * 60;
  else if (delta_mins > 720)
    tick_time_interval = 120 * 60;
  else if (delta_mins > 360)
    tick_time_interval = 60 * 60;
  else if (delta_mins > 120)
    tick_time_interval = 30 * 60;
  else if (delta_mins > 90)
    tick_time_interval = 20 * 60;
  else if (delta_mins > 60)
    tick_time_interval = 15 * 60;
  else if (delta_mins > 30)
    tick_time_interval = 10 * 60;
  else if (delta_mins > 10)
    tick_time_interval = 5 * 60;
  else if (delta_mins > 5)
    tick_time_interval = 2 * 60;
  else
    tick_time_interval = 1 * 60;

  n_time_ticks =
    (si32) ((delta_mins * 60.0) / tick_time_interval) + 1;

  tick_time.unix_time =
    ((((si32) plot_min_x / tick_time_interval) + 1) * tick_time_interval);
  uconvert_from_utime(&tick_time);

  text_y = (frame->w_ymin + xtick_length + xtick_margin);

  for (i = 0; i < n_time_ticks; i++) {

    tick_x = tick_time.unix_time;

    if (tick_x > frame->w_xmin + xaxis_endmargin &&
	tick_x < frame->w_xmax - xaxis_endmargin) {

      GDrawLine(dev, frame, Glob->tick_gc, frame->psgc, 
		tick_x, frame->w_ymin,
		tick_x, frame->w_ymin + xtick_length);

      sprintf(label, "%.2d%.2d", tick_time.hour, tick_time.min);

      GDrawString(dev, frame, Glob->ticklabel_gc, frame->x->font,
		  frame->psgc,
		  XJ_CENTER, YJ_ABOVE, tick_x, text_y, label);
    }

    tick_time.unix_time += tick_time_interval;
    uconvert_from_utime(&tick_time);

  }

  if (dev == XDEV) {

    GDrawString(dev, frame,
		Glob->ticklabel_gc,
		Glob->x_ticklabel_font,
		frame->psgc,
		XJ_LEFT, YJ_ABOVE,
		frame->w_xmin + header_clearance_x, text_y,
		"UCT");

  } else {

    GDrawString(dev, frame,
		Glob->ticklabel_gc,
		Glob->x_ticklabel_font,
		frame->psgc,
		XJ_LEFT, YJ_ABOVE,
		frame->w_xmin + header_clearance_x, text_y,
		"Time (UCT)");

  }

  /*
   * plot in time bars and image time as a tick mark if X
   */

  if (dev == XDEV) {

    current_time = (double) Glob->track_shmem->time;
    current_start = (current_time -
		     (double) tdata->scan_duration / 2.0);
    current_end = current_start + tdata->scan_duration;
    past_start = current_time - tshmem->past_plot_period;
    future_end = current_time + tshmem->future_plot_period;
    forecast_end = current_time +
      (tshmem->forecast_interval * tshmem->n_forecast_steps);

    if (current_start > past_start)
      GFillRectangle(dev, frame, Glob->past_gc, frame->psgc,
		     past_start,
		     TIME_BAR_ONE_THIRD_Y,
		     current_start - past_start,
		     TIME_BAR_TWO_THIRDS_Y - TIME_BAR_ONE_THIRD_Y);

    if (future_end > current_end)
      GFillRectangle(dev, frame, Glob->future_gc, frame->psgc,
		     current_end,
		     TIME_BAR_MID_Y,
		     future_end - current_end,
		     TIME_BAR_MAX_Y - TIME_BAR_MID_Y);

    if (Glob->track_shmem->plot_forecast && forecast_end > current_end)
      GFillRectangle(dev, frame, Glob->forecast_gc, frame->psgc,
		     current_end,
		     TIME_BAR_MIN_Y,
		     forecast_end - current_end,
		     TIME_BAR_MID_Y - TIME_BAR_MIN_Y);
    
    GFillRectangle(dev, frame, Glob->current_gc, frame->psgc,
		   current_start,
		   TIME_BAR_MIN_Y,
		   current_end - current_start,
		   TIME_BAR_MAX_Y - TIME_BAR_MIN_Y);
  }

  /*
   * draw frame border if postscript
   */

  if (dev == PSDEV) {
    GDrawPsFrameBorder(frame, Glob->ps_border_line_width);
  }

  XMapSubwindows(Glob->rdisplay, frame->x->drawable);
  safe_XFlush(Glob->rdisplay);
  
  Glob->tscale_status = CURRENT;

}

