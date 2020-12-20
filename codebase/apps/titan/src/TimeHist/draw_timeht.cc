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
/************************************************************************
 * draw_timeht.c
 *
 * time history plot
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "TimeHist.hh"
#include <toolsa/DateTime.hh>
using namespace std;

#define TICK_CLEARANCE_FRACTION 0.5
#define APPROX_N_TICKS (si32) 5

/************************************************************************
 * draw_timeht_title()
 *
 * draw title for time history plot
 */

void draw_timeht_title(int dev)

{

  gframe_t *frame;
  char tstring[128];

  if (Glob->verbose) {
    fprintf(stderr, "** draw_timeht_title **\n");
  }

  frame = Glob->timeht_title_frame;

  /*
   * clear window
   */

  if (dev == XDEV) {
    XSetWindowBackground(Glob->rdisplay, frame->x->drawable,
			 Glob->background);
    XClearWindow(Glob->rdisplay, frame->x->drawable);
  }

  sprintf(tstring, "STORM TIME HEIGHT PROFILE");

  GDrawString(dev, frame, frame->x->gc, frame->x->font,
	      frame->psgc, XJ_CENTER, YJ_CENTER,
	      (double) 0.5, (double) 0.5, tstring);

  XFlush(Glob->rdisplay);

}

/************************************************************************
 * draw_timeht_button()
 *
 * Draws a button in the time history window
 */

void draw_timeht_button(si32 n,
			ui32 background)

{

  static si32 first_call = TRUE;
  static char **button_label;
  char *label_str;
  int i;
  gframe_t *frame;

  if (Glob->verbose) {
    fprintf(stderr, "** draw_timeht_button **\n");
  }

  frame = Glob->timeht_button_frame[n];

  /*
   * if first call, load up the label strings
   */

  if (first_call == TRUE) {

    button_label = (char **) umalloc (N_TIMEHT_BUTTONS * sizeof(char *));

    label_str = (char *) umalloc((ui32) (strlen(TIMEHT_BUTTON_LABELS) + 1));
    strcpy(label_str, TIMEHT_BUTTON_LABELS);

    for (i = 0; i < N_TIMEHT_BUTTONS; i++) {

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
  XClearWindow(Glob->rdisplay, frame->x->drawable);

  /*
   * write the label to the window
   */

  GDrawString(XDEV, frame, frame->x->gc, frame->x->font,
	      frame->psgc, XJ_CENTER, YJ_CENTER,
	      (double) 0.5, (double) 0.5, button_label[n]);

  XFlush(Glob->rdisplay);
  
}

/*********************************************************************
 * draw_ht_trace()
 */

static void draw_ht_trace(int dev,
			  gframe_t *frame,
			  GC gc,
			  psgc_t *psgc,
			  thist_track_data_t *tdata,
			  si32 offset)

{

  si32 iscan;
  double time1, time2;
  double ht1, ht2;

  if (Glob->verbose) {
    fprintf(stderr, "** draw_ht_trace **\n");
  }

  if (dev == PSDEV) {
      
    PsGsave(frame->psgc->file);
    PsSetLineWidth(frame->psgc->file,
		   psgc->line_width);
    PsSetLineDash(frame->psgc->file,
		  psgc->dash_length,
		  psgc->space_length);

  }
  
  for (iscan = 1; iscan < tdata->nscans; iscan++) {

    time1 = (double) tdata->scan[iscan - 1].time;
    ht1 = *((double *) ((char *) (tdata->scan + iscan - 1) + offset));
    time2 = (double) tdata->scan[iscan].time;
    ht2 = *((double *) ((char *) (tdata->scan + iscan) + offset));

    GDrawLine(dev, frame, gc, frame->psgc, time1, ht1, time2, ht2);

  } /* iscan */

  if (dev == PSDEV)
    PsGrestore(frame->psgc->file);

}

/*********************************************************************
 * draw_trace_vals()
 */

static void draw_trace_vals(int dev,
			    gframe_t *frame,
			    GC gc,
			    psgc_t *psgc,
			    double fontsize,
			    thist_track_data_t *tdata,
			    si32 ht_offset,
			    si32 val_offset)

{

  char text[32];
  si32 iscan;
  double time, ht, val;

  if (Glob->verbose) {
    fprintf(stderr, "** draw_trace_vals **\n");
  }

  if (dev == PSDEV) {
      
    PsGsave(frame->psgc->file);
    PsSetFont(frame->psgc->file, frame->psgc->fontname, fontsize);

  }
  
  for (iscan = 0; iscan < tdata->nscans; iscan++) {

    time = (double) tdata->scan[iscan].time;
    ht = *((double *) ((char *) (tdata->scan + iscan) + ht_offset));
    val = *((double *) ((char *) (tdata->scan + iscan) + val_offset));

    sprintf(text, "%.0f", val);

    GDrawImageString(dev, frame, gc,
		     frame->x->font, frame->psgc,
		     XJ_CENTER, YJ_CENTER, time, ht, text);

  } /* iscan */

  if (dev == PSDEV)
    PsGrestore(frame->psgc->file);

}

/************************************************************************
 * draw_timeht_plot()
 *
 * Draws the time height profile
 */

void draw_timeht_plot(int dev,
		      g_color_scale_t *colors)

{

  int i;
  int valid_color_found;
  long iscan, ilayer, icolor;
  long nticksy;
  long nticksx, first_tick_time, tick_interval;
  long offset, val_offset;
  
  double tickmin_y, delta_tick_y;
  double plot_min_x, plot_max_x, plot_min_y, plot_max_y;
  double plot_x_range, window_x_range;
  double window_min_x, window_max_x;
  double plot_y_range;
  double window_min_y, window_max_y;
  double xaxis_margin = 0, yaxis_margin = 0, top_margin = 0;
  double xtick_length = 0, ytick_length = 0;
  double xtick_margin = 0, ytick_margin = 0;
  double xaxis_endmargin = 0, yaxis_endmargin = 0;
  double tick_x, tick_y;
  double text_x, text_y;
  double rect_min_x, rect_min_y, rect_max_x;
  double header_clearance_x = 0, header_clearance_y = 0;
  double text_margin_x = 0, text_margin_y = 0;
  double xaxis_margin_d = 0;
  double data_value;

  char hstring[BUFSIZ];
  char label[80];
  char track_num_str[32];
  char case_str[32];
  char date_str[32];

  date_time_t tick_time;
  date_time_t plot_start, plot_end;
  gframe_t *frame;
  thist_track_data_t *tdata;

  if (Glob->verbose) {
    fprintf(stderr, "** draw_timeht_plot **\n");
  }

  /*
   * set pointers
   */

  tdata = &Glob->tdata;
  frame = Glob->timeht_plot_frame;

  /*
   * clear window if X
   */

  if (dev == XDEV)
    XClearWindow(Glob->rdisplay, frame->x->drawable);

  /*
   * get track data - return if no data
   */

  if (load_plot_data()) {
    XMapSubwindows(Glob->rdisplay, frame->x->drawable);
    XFlush(Glob->rdisplay);
    Glob->timeht_status = CURRENT;
    return;
  }

  /*
   * get margins depending on X or postscript
   */

  switch (dev) {

  case XDEV:

    yaxis_margin = Glob->x_timeht_yaxis_margin / (double) (frame->x->width);
    xaxis_margin = Glob->x_xaxis_margin / (double) (frame->x->height);
    top_margin = Glob->x_top_margin / (double) (frame->x->height);
    text_margin_x = Glob->x_text_margin / (double) (frame->x->xscale);
    text_margin_y = Glob->x_text_margin / (double) (frame->x->yscale);
    xaxis_margin_d = (double) Glob->x_xaxis_margin / frame->x->yscale;
    break;

  case PSDEV:

    yaxis_margin = Glob->ps_timeht_yaxis_margin / (double) (frame->ps->width);
    xaxis_margin = Glob->ps_xaxis_margin / (double) (frame->ps->height);
    top_margin = Glob->ps_top_margin / (double) (frame->ps->height);
    text_margin_x = Glob->ps_text_margin / (double) (frame->ps->xscale);
    text_margin_y = Glob->ps_text_margin / (double) (frame->ps->yscale);
    xaxis_margin_d = Glob->ps_xaxis_margin / (double) (frame->ps->yscale);
    break;

  } /* switch */

  /*
   * compute world coordinate window limits based on the data limits
   * and the margins
   */

  plot_min_x = (double) tdata->plot_start_time;
  plot_max_x = (double) tdata->plot_end_time;
  plot_min_y = tdata->min_base - 0.5;
  plot_max_y = tdata->max_top + 0.5;

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
    ytick_length = Glob->x_tick_length / frame->x->xscale;
    xtick_margin = xtick_length * TICK_CLEARANCE_FRACTION;
    ytick_margin = ytick_length * TICK_CLEARANCE_FRACTION;
    xaxis_endmargin = Glob->x_xaxis_endmargin / frame->x->xscale;
    yaxis_endmargin = Glob->x_yaxis_endmargin / frame->x->yscale;
    header_clearance_x = Glob->x_header_clearance / frame->x->xscale;
    header_clearance_y = Glob->x_header_clearance / frame->x->yscale;
    break;

  case PSDEV:
    GPsSetGeomFrame(frame, frame->ps->xmin, frame->ps->ymin,
		    frame->ps->width, frame->ps->height);
    xtick_length = Glob->ps_tick_length / frame->ps->yscale;
    ytick_length = Glob->ps_tick_length / frame->ps->xscale;
    xtick_margin = xtick_length * TICK_CLEARANCE_FRACTION;
    ytick_margin = ytick_length * TICK_CLEARANCE_FRACTION;
    xaxis_endmargin = Glob->ps_xaxis_endmargin / frame->x->xscale;
    yaxis_endmargin = Glob->ps_yaxis_endmargin / frame->x->yscale;
    header_clearance_x = Glob->ps_header_clearance / frame->x->xscale;
    header_clearance_y = Glob->ps_header_clearance / frame->x->yscale;
    break;

  }

  /*
   * plot time height profile data
   */

  if (Glob->timeht_mode != FALSE) {
    
    for (iscan = 0; iscan < tdata->nscans; iscan++) {
      
      if (iscan == 0)
	rect_min_x = (double) tdata->plot_start_time;
      else
	rect_min_x =
	  ((double) tdata->scan[iscan].time +
	   (double) tdata->scan[iscan - 1].time) * 0.5;
      
      if (iscan == tdata->nscans - 1)
	rect_max_x = (double) tdata->plot_end_time;
      else
	rect_max_x =
	  ((double) tdata->scan[iscan].time +
	   (double) tdata->scan[iscan + 1].time) * 0.5;
      
      /*
       * loop through the ht levels
       */
      
      for (ilayer = 0; ilayer < tdata->max_layers; ilayer++) {
	
	/*
	 * process data if it is not a missing val, which is indicated by
	 * a large negative number
	 */

	if (tdata->scan[iscan].timeht_flag[ilayer]) {

	  data_value = tdata->scan[iscan].timeht_data[ilayer];
	
	  /*
	   * search for color level which is appropriate for the data
	   */
	  
	  valid_color_found = FALSE;
	  
	  for (icolor = 0; icolor < colors->nlevels; icolor++) {
	    
	    if (colors->level[icolor].start <= data_value &&
		data_value <= colors->level[icolor].end) {
	      valid_color_found = TRUE;
	      break;
	    }
	    
	  } /* icolor */
	  
	  if (valid_color_found) {
	    
	    rect_min_y = (tdata->scan[iscan].min_z +
			  (ilayer - 0.5) * tdata->scan[iscan].delta_z);
	    
	    switch (dev) {
	      
	    case XDEV:
	      
	      GFillRectangle(dev, frame,
			     colors->level[icolor].gc,
			     frame->psgc,
			     rect_min_x, rect_min_y,
			     rect_max_x - rect_min_x,
			     tdata->scan[iscan].delta_z);
	      break;
	      
	    case PSDEV:
	      
	      GFillRectangle(dev, frame,
			     frame->x->gc,
			     colors->level[icolor].psgc,
			     rect_min_x, rect_min_y,
			     rect_max_x - rect_min_x,
			     tdata->scan[iscan].delta_z);
	      break;
	      
	    } /* switch */
	    
	  } /* if (valid_color_found == TRUE) */
	  
	} /* if (tdata->scan[iscan].timeht_flag[ilayer]) */
	
      } /* ilayer */
      
    } /* iscan */

  } /* if (Glob->timeht_mode != FALSE) */

  /*
   * top and base
   */
  
  offset = (char *) &tdata->scan[0].top - (char *) tdata->scan;
  draw_ht_trace(dev, frame, Glob->top_base_gc,
		&Glob->top_base_psgc, tdata, offset);
  
  offset = (char *) &tdata->scan[0].base - (char *) tdata->scan;
  draw_ht_trace(dev, frame, Glob->top_base_gc,
		&Glob->top_base_psgc, tdata, offset);
  
  /*
   * if requested, plot height lines
   */

  if (Glob->timeht_htmaxz != FALSE) {

    offset = (char *) &tdata->scan[0].ht_of_dbz_max - (char *) tdata->scan;
    draw_ht_trace(dev, frame, Glob->ht_maxdbz_gc,
		  &Glob->ht_maxdbz_psgc, tdata, offset);
    
    if (Glob->timeht_htmaxz == SELECTED_ALL) {
      val_offset = (char *) &tdata->scan[0].dbz_max - (char *) tdata->scan;
      draw_trace_vals(dev, frame, Glob->ticklabel_gc,
		      &Glob->ht_maxdbz_psgc,
		      Glob->ps_ticklabel_fontsize,
		      tdata, offset, val_offset);
    }
    
  } /* if (Glob->timeht_htmaxz == TRUE) */
  
  if (Glob->timeht_centroids != FALSE) {
    
    offset = (char *) &tdata->scan[0].refl_centroid_z - (char *) tdata->scan;
    draw_ht_trace(dev, frame, Glob->ht_refl_centroid_gc,
		  &Glob->ht_refl_centroid_psgc, tdata, offset);
      
    if (Glob->timeht_centroids == SELECTED_ALL) {
      offset = (char *) &tdata->scan[0].vol_centroid_z - (char *) tdata->scan;
      draw_ht_trace(dev, frame, Glob->ht_centroid_gc,
		    &Glob->ht_centroid_psgc, tdata, offset);
    }
      
  } /* if (Glob->timeht_centroids == TRUE) */
  
  /*
   * draw in divider between tick marks and data
   */

  if (dev == PSDEV)
    PsSetLineWidth(frame->psgc->file, Glob->ps_divider_line_width);

  if (yaxis_margin != 0)
    GDrawLine(dev, frame, Glob->divider_gc, frame->psgc,
	      plot_max_x, plot_min_y, plot_max_x, plot_max_y);

  if (xaxis_margin != 0)
    GDrawLine(dev, frame, Glob->divider_gc, frame->psgc,
	      plot_min_x, plot_min_y, plot_max_x, plot_min_y);

  if (top_margin != 0)
    GDrawLine(dev, frame, Glob->divider_gc, frame->psgc,
	      plot_min_x, plot_max_y, plot_max_x, plot_max_y);

  /*
   * if printer, set tick line width and ticklabel fontsize
   */

  if (dev == PSDEV) {

    PsSetLineWidth(frame->psgc->file,
		   Glob->ps_tick_line_width);

    PsSetFont(frame->psgc->file, frame->psgc->fontname,
	      Glob->ps_ticklabel_fontsize);

  }

  compute_ticks(tdata->plot_start_time,
		tdata->plot_end_time,
		&nticksx,
		&first_tick_time,
		&tick_interval);

  /*
   * x axis tick marks and labels
   */

  text_y = (frame->w_ymin + xtick_length + xtick_margin);
  tick_x = (double) first_tick_time;
  tick_time.unix_time = first_tick_time;
  uconvert_from_utime(&tick_time);

  for (i = 0; i < nticksx; i++) {

    if (tick_x > frame->w_xmin + xaxis_endmargin &&
	tick_x < frame->w_xmax - xaxis_endmargin) {

      GDrawLine(dev, frame, Glob->tick_gc, frame->psgc, 
		tick_x, frame->w_ymin,
		tick_x, frame->w_ymin + xtick_length);

      DateTime tickTime(tick_time.unix_time +
                        Glob->track_shmem->timeOffsetSecs);
      sprintf(label, "%.2d%.2d", tickTime.getHour(), tickTime.getMin());

      GDrawString(dev, frame, Glob->ticklabel_gc, frame->x->font,
		  frame->psgc,
		  XJ_CENTER, YJ_ABOVE, tick_x, text_y, label);
    }

    tick_time.unix_time += tick_interval;
    uconvert_from_utime(&tick_time);
    tick_x = (double) tick_time.unix_time;

  }

  if (dev == XDEV) {

    GDrawString(dev, frame,
		Glob->ticklabel_gc,
		Glob->x_ticklabel_font,
		frame->psgc,
		XJ_LEFT, YJ_ABOVE,
		frame->w_xmin + header_clearance_x, text_y,
		Glob->track_shmem->time_label);

  } else {

    char timeText[128];
    sprintf(timeText, "Time (%s)", Glob->track_shmem->time_label);
    GDrawString(dev, frame,
		Glob->ticklabel_gc,
		Glob->x_ticklabel_font,
		frame->psgc,
		XJ_LEFT, YJ_ABOVE,
		frame->w_xmin + header_clearance_x, text_y,
		timeText);

  }

  /*
   * y axis tick marks and labels
   */

  GLinearTicks(plot_min_y, plot_max_y, APPROX_N_TICKS,
	       &nticksy, &tickmin_y, &delta_tick_y);

  text_x = frame->w_xmax - ytick_length - ytick_margin;
  tick_y = tickmin_y;

  for (i = 0; i < nticksy; i++) {

    if (tick_y > frame->w_ymin + yaxis_endmargin &&
	tick_y < frame->w_ymax - yaxis_endmargin) {

      GDrawLine(dev, frame, Glob->tick_gc, frame->psgc, 
		frame->w_xmax, tick_y, 
		frame->w_xmax - ytick_length, tick_y);

      sprintf(label, "%g", tick_y);

      GDrawString(dev, frame, Glob->ticklabel_gc,
		  frame->x->font, frame->psgc,
		  XJ_RIGHT, YJ_CENTER,
		  text_x, tick_y, label);

    }
    
    tick_y += delta_tick_y;

  }

  GDrawString(dev, frame,
	      Glob->ticklabel_gc,
	      Glob->x_ticklabel_font,
	      frame->psgc,
	      XJ_RIGHT, YJ_BELOW,
	      text_x,
	      frame->w_ymax - header_clearance_y,
	      "Ht(km)");

  /*
   * plot in image time as a tick mark if X
   */

  if (dev == XDEV)
    GDrawLine(dev, frame, Glob->current_gc, frame->psgc,
	      (double) Glob->track_shmem->time,
	      frame->w_ymin,
	      (double) Glob->track_shmem->time,
	      frame->w_ymin + xtick_length);

  /*
   * write in header
   */

  if (Glob->track_shmem->track_type == COMPLEX_TRACK)
    sprintf(track_num_str, "Track %ld",
	    (long) Glob->track_shmem->complex_track_num);
  else
    sprintf(track_num_str, "Track %ld/%ld",
	    (long) Glob->track_shmem->complex_track_num,
	    (long) Glob->track_shmem->simple_track_num);
    
  if (Glob->track_shmem->case_num > 0) {
    sprintf(case_str, "Case %d", (int) Glob->track_shmem->case_num);
  } else {
    case_str[0] = '\0';
  }
    
  plot_start.unix_time = tdata->plot_start_time;
  uconvert_from_utime(&plot_start);
  plot_end.unix_time = tdata->plot_end_time;
  uconvert_from_utime(&plot_end);

  if (plot_start.year != plot_end.year ||
      plot_start.month != plot_end.month ||
      plot_start.day != plot_end.day) {
    
    sprintf(date_str, "%.4d/%.2d/%.2d - %.4d/%.2d/%.2d",
	    plot_start.year, plot_start.month, plot_start.day,
	    plot_end.year, plot_end.month, plot_end.day);
    
  } else {

    sprintf(date_str, "%.4d/%.2d/%.2d",
	    plot_start.year, plot_start.month, plot_start.day);

  }

  if (dev == XDEV) {
    
    sprintf(hstring,
	    "%s  %s %s",
	    tdata->timeht_label,
	    track_num_str, case_str);

  } else {

    sprintf(hstring,
	    "%s   %s   %s   %s",
	    tdata->timeht_label,
	    track_num_str, case_str,
	    date_str);
    
    PsSetFont(frame->psgc->file, frame->psgc->fontname,
	      Glob->ps_header_fontsize);

  }
    
  GDrawString(dev, frame, Glob->header_gc, Glob->x_header_font,
	      frame->psgc, XJ_LEFT, YJ_BELOW,
	      frame->w_xmin + header_clearance_x,
	      frame->w_ymax - header_clearance_y,
	      hstring);
    
  /*
   * draw frame border if postscript
   */

  if (dev == PSDEV) {
    GDrawPsFrameBorder(frame, Glob->ps_border_line_width);
  }

  /*
   * draw cursor y position
   */

  if (dev == XDEV &&
      Glob->timeht_cursor_active) {

    char cursor_str[256];
    double cross_arm_x, cross_arm_y;
    
    /*
     * write position into string
     */

    sprintf(cursor_str, "y:%g km", Glob->timeht_cursor_y);
    
    /*
     * put string on image
     */

    GDrawImageString(dev, frame, Glob->text_gc, Glob->x_text_font,
		     frame->psgc, XJ_LEFT, YJ_ABOVE,
		     frame->w_xmin + text_margin_x,
		     frame->w_ymin + text_margin_y + xaxis_margin_d,
		     cursor_str);
    
    /*
     * draw cursor
     */

    cross_arm_x = (frame->w_xmax - frame->w_xmin) / 80.0;
    cross_arm_y = (frame->w_ymax - frame->w_ymin) / 60.0;
    
    GDrawLine(dev, frame, Glob->crosshair_gc, frame->psgc,
	      Glob->timeht_cursor_x - cross_arm_x, Glob->timeht_cursor_y,
	      Glob->timeht_cursor_x + cross_arm_x, Glob->timeht_cursor_y);
    
    GDrawLine(dev, frame, Glob->crosshair_gc, frame->psgc,
	      Glob->timeht_cursor_x, Glob->timeht_cursor_y - cross_arm_y,
	      Glob->timeht_cursor_x, Glob->timeht_cursor_y + cross_arm_y);
    
  } /* if (dev == XDEV && Glob->timeht_cursor_active) */
  
  XMapSubwindows(Glob->rdisplay, frame->x->drawable);
  XFlush(Glob->rdisplay);

  Glob->timeht_status = CURRENT;

}

