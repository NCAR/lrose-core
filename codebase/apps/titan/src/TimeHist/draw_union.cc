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
 * draw_union.c
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

#define LOG_10(a) ((a) < 1.0 ? (0.0) : (log10(a)))

#define TICK_CLEARANCE_FRACTION 0.5
#define APPROX_N_TICKS (si32) 5

/*
 * file scope prototypes
 */

static void _draw_observed(int dev,
			   gframe_t *frame,
			   thist_track_data_t *tdata,
			   int n_active,
			   int *fnum,
			   double *delta_cycle);

static void _draw_y_axes(int dev,
			 gframe_t *frame,
			 thist_track_data_t *tdata,
			 int n_active,
			 int *fnum,
			 int *start_cycle,
			 int *end_cycle,
			 double *delta_cycle,
			 double plot_min_x, double plot_max_x,
			 double plot_min_y, double plot_max_y);

static void _draw_x_ticks(int dev,
			  gframe_t *frame,
			  thist_track_data_t *tdata);

static void _draw_y_ticks(int dev,
			  gframe_t *frame,
			  int *start_cycle,
			  int *end_cycle,
			  double plot_min_x, double plot_max_x);

static void _load_observed_fit(thist_track_data_t *tdata,
			       int act_num,
			       int *fnum,
			       double *delta_cycle,
			       GPoint *gpoints);

/************************************************************************
 * draw_union_title()
 *
 * draw title for time history plot
 */

void draw_union_title(int dev)

{

  gframe_t *frame;
  char tstring[128];

  if (Glob->debug) {
    fprintf(stderr, "** draw_union_title **\n");
  }

  frame = Glob->union_title_frame;

  /*
   * clear window
   */

  if (dev == XDEV) {
    XSetWindowBackground(Glob->rdisplay, frame->x->drawable,
			 Glob->background);
    XClearWindow(Glob->rdisplay, frame->x->drawable);
  }

  sprintf(tstring, "STORM TIME HISTORY");

  GDrawString(dev, frame, frame->x->gc, frame->x->font,
	      frame->psgc, XJ_CENTER, YJ_CENTER,
	      (double) 0.5, (double) 0.5, tstring);

  XFlush(Glob->rdisplay);

}

/************************************************************************
 * draw_union_button()
 *
 * Draws a button in the time history window
 *
 */

void draw_union_button(si32 n,
		       ui32 background)

{

  static si32 first_call = TRUE;
  static char **button_label;
  char *label_str = NULL;
  int i;
  gframe_t *frame;

  if (Glob->debug) {
    fprintf(stderr, "** draw_union_button **\n");
  }

  frame = Glob->union_button_frame[n];

  /*
   * if first call, load up the label strings
   */

  if (first_call == TRUE) {

    button_label = (char **) umalloc (N_UNION_BUTTONS * sizeof(char *));

    if (Glob->union_mode == UNION_MODE_FLOATS) {
      label_str = (char *) umalloc((strlen(UNION_BUTTON_FLOAT_LABELS) + 1));
      strcpy(label_str, UNION_BUTTON_FLOAT_LABELS);
    } else if (Glob->union_mode == UNION_MODE_HAIL) {
      label_str = (char *) umalloc((strlen(UNION_BUTTON_HAIL_LABELS) + 1));
      strcpy(label_str, UNION_BUTTON_HAIL_LABELS);
    }

    for (i = 0; i < N_UNION_BUTTONS; i++) {
      
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

/************************************************************************
 * draw_union_plot()
 *
 * Draws the storm time history
 *
 */

void draw_union_plot(int dev)

{

  int i;
  int n_cycles;
  int n_active;
  int fnum[UNION_N_FIELDS];
  int start_cycle[UNION_N_FIELDS], end_cycle[UNION_N_FIELDS];
  double delta_cycle[UNION_N_FIELDS];

  double plot_min_x, plot_max_x, plot_min_y, plot_max_y;
  double plot_x_range, window_x_range;
  double window_min_x, window_max_x;
  double plot_y_range;
  double window_min_y, window_max_y;
  double xaxis_margin = 0, yaxis_margin = 0, top_margin = 0;
  double header_clearance_x = 0, header_clearance_y = 0;
  double text_margin_x = 0, text_margin_y = 0;
  double xaxis_margin_d = 0;

  char hstring[BUFSIZ];
  char track_num_str[32];
  char case_str[32];
  char date_str[32];

  date_time_t plot_start, plot_end;

  gframe_t *frame;
  thist_track_data_t *tdata;

  if (Glob->debug) {
    fprintf(stderr, "** draw_union_plot **\n");
  }

  /*
   * set pointers
   */

  frame = Glob->union_plot_frame;
  tdata = &Glob->tdata;

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
    Glob->union_status = CURRENT;
    return;
  }

  /*
   * get margins depending on X or postscript
   */

  switch (dev) {

  case XDEV:

    yaxis_margin = Glob->x_union_yaxis_margin / (double) (frame->x->width);
    xaxis_margin = Glob->x_xaxis_margin / (double) (frame->x->height);
    top_margin = Glob->x_top_margin / (double) (frame->x->height);
    text_margin_x = Glob->x_text_margin / (double) (frame->x->xscale);
    text_margin_y = Glob->x_text_margin / (double) (frame->x->yscale);
    xaxis_margin_d = (double) Glob->x_xaxis_margin / frame->x->yscale;
    break;

  case PSDEV:

    yaxis_margin = Glob->ps_union_yaxis_margin / (double) (frame->ps->width);
    xaxis_margin = Glob->ps_xaxis_margin / (double) (frame->ps->height);
    top_margin = Glob->ps_top_margin / (double) (frame->ps->height);
    text_margin_x = Glob->ps_text_margin / (double) (frame->ps->xscale);
    text_margin_y = Glob->ps_text_margin / (double) (frame->ps->yscale);
    xaxis_margin_d = Glob->ps_xaxis_margin / (double) (frame->ps->yscale);
    break;

  } /* switch */

  /*
   * compute time world coordinate window limits based on the data limits
   * and the margins
   */

  plot_min_x = (double) tdata->plot_start_time;
  plot_max_x = (double) tdata->plot_end_time;

  /*
   * compute the limits of the y-data, using log scales.
   * We use the first active field as the baseline.
   * Always require at least 2 cycles on the plot.
   */
  
  n_cycles = 2;
  n_active = 0;

  for (i = 0; i < UNION_N_FIELDS; i++) {
    if (Glob->union_field_active[i]) {
      fnum[n_active] = i;
      start_cycle[n_active] = (int) LOG_10(tdata->min_union_val[i]);
      end_cycle[n_active] = (int) LOG_10(tdata->max_union_val[i]) + 1;
      n_cycles = MAX(n_cycles, end_cycle[n_active] - start_cycle[n_active]);
      n_active++;
    }
  } /* i */

  if (n_active < 1) {
    Glob->union_status = CURRENT;
    return;
  }

  for (i = 0; i < n_active; i++) {
    end_cycle[i] = start_cycle[i] + n_cycles;
    delta_cycle[i] = start_cycle[0] - start_cycle[i];
  } /* i */
  
  plot_min_y = (double) start_cycle[0];
  plot_max_y = (double) end_cycle[0];

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
    header_clearance_x = Glob->x_header_clearance / frame->x->xscale;
    header_clearance_y = Glob->x_header_clearance / frame->x->yscale;
    break;

  case PSDEV:
    GPsSetGeomFrame(frame, frame->ps->xmin, frame->ps->ymin,
		    frame->ps->width, frame->ps->height);
    header_clearance_x = Glob->x_header_clearance / frame->x->xscale;
    header_clearance_y = Glob->x_header_clearance / frame->x->yscale;
    break;

  }
  
  /*
   * draw in vert axes
   */
  
  _draw_y_axes(dev, frame, tdata, n_active, fnum,
	       start_cycle, end_cycle, delta_cycle,
	       plot_min_x, plot_max_x,
	       plot_min_y, plot_max_y);

  /*
   * draw in y ticks
   */
  
  _draw_y_ticks(dev, frame, start_cycle, end_cycle,
		plot_min_x, plot_max_x);

  /*
   * plot data
   */
  
  _draw_observed(dev, frame, tdata, n_active, fnum, delta_cycle);

  // clear x margin

  if (dev == XDEV) {
    if (xaxis_margin != 0) {
      int margin_pixels = (int) (xaxis_margin / frame->x->xscale);
      GFillRectangle(dev, frame,
		     Glob->pixmap_gc,
		     frame->psgc,
		     plot_min_x,
		     plot_min_y - margin_pixels,
		     plot_max_x - plot_min_x,
		     margin_pixels);
    }
  }

  /*
   * draw in margins
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
   * draw in x ticks
   */
  
  _draw_x_ticks(dev, frame, tdata);

  /*
   * write in header
   */

  if (Glob->track_shmem->track_type == COMPLEX_TRACK) {
    sprintf(track_num_str, "Track %d",
	    (int) Glob->track_shmem->complex_track_num);
  } else {
    sprintf(track_num_str, "Track %d/%d",
	    (int) Glob->track_shmem->complex_track_num,
	    (int) Glob->track_shmem->simple_track_num);
  }

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
	    "%s %s",
	    track_num_str, case_str);

  } else {

    sprintf(hstring,
	    "%s   %s  %s",
	    track_num_str, case_str, date_str);

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
      Glob->union_cursor_active) {

    char cursor_str[256];
    double cross_arm_x, cross_arm_y;
    double y_value = pow(10.0, Glob->union_cursor_y);
    
    /*
     * write position into string
     */

    sprintf(cursor_str, "y:%g %s",
	    y_value, tdata->union_units[fnum[0]]);
    
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
    cross_arm_y = (frame->w_ymax - frame->w_ymin) / 40.0;
    
    GDrawLine(dev, frame, Glob->crosshair_gc, frame->psgc,
	      Glob->union_cursor_x - cross_arm_x, Glob->union_cursor_y,
	      Glob->union_cursor_x + cross_arm_x, Glob->union_cursor_y);
    
    GDrawLine(dev, frame, Glob->crosshair_gc, frame->psgc,
	      Glob->union_cursor_x, Glob->union_cursor_y - cross_arm_y,
	      Glob->union_cursor_x, Glob->union_cursor_y + cross_arm_y);
    
  } /* if (dev == XDEV && Glob->union_cursor_active) */
  
  XMapSubwindows(Glob->rdisplay, frame->x->drawable);
  XFlush(Glob->rdisplay);

  Glob->union_status = CURRENT;

}

/*********************************************************
 * _draw_observed()
 *
 * Draw in observed data for active fields
 */

static void _draw_observed(int dev,
			   gframe_t *frame,
			   thist_track_data_t *tdata,
			   int n_active,
			   int *fnum,
			   double *delta_cycle)

{

  int iact, iscan;
  int field_num;
  GPoint *gpoints, *gp;
  
  if (dev == PSDEV) {
    PsGsave(frame->psgc->file);
  }

  /*
   * allocate points
   */

  gpoints = (GPoint *) umalloc(tdata->nscans * sizeof(GPoint));

  /*
   * plot time history data
   */
  
  for (iact = 0; iact < n_active; iact++) {

    field_num = fnum[iact];
    
    if (dev == PSDEV) {
      PsSetLineStyle(frame->psgc->file,
		     &Glob->union_graph_psgc[field_num]);
    }

    /*
     * load up observed points
     */

    gp = gpoints;
    for (iscan = 0; iscan < tdata->nscans; iscan++, gp++) {
      gp->x = (double) tdata->scan[iscan].time;
      gp->y = (LOG_10(tdata->scan[iscan].union_data[field_num]) +
		delta_cycle[iact]);
    }

    /*
     * draw observed data
     */
    
    GDrawLines(dev, frame, Glob->union_graph_gc[field_num], frame->psgc,
	       gpoints, tdata->nscans, CoordModeOrigin);

    if (Glob->union_fit) {

      /*
       * load up and draw fitted line
       */
    
      _load_observed_fit(tdata, iact, fnum, delta_cycle, gpoints);
      GDrawLines(dev, frame, Glob->union_graph_gc[field_num], frame->psgc,
		 gpoints, tdata->nscans, CoordModeOrigin);

    }

  } /* iact */

  if (dev == PSDEV) {
    PsGrestore(frame->psgc->file);
  }

  ufree(gpoints);
  
}

/*********************************************************
 * _load_observed_fit()
 *
 * Loads up fit to the observed data into the points array.
 */

static void _load_observed_fit(thist_track_data_t *tdata,
			       int act_num,
			       int *fnum,
			       double *delta_cycle,
			       GPoint *gpoints)

{

  int nscans, iscan;
  int field_num;
  GPoint *gp;
  double sum_ad, sum_adt, sum_adt2;
  double start_time, delta_time;
  double scan_duration;
  double A, D, T, C;
  double dtt, dtt2, yy;
  
  /*
   * plot time history data
   */
  
  field_num = fnum[act_num];
    
  /*
   * accumulate data for the Gaussian fit
   */
  
  nscans = tdata->nscans;
  sum_ad = 0.0;
  sum_adt = 0.0;
  sum_adt2 = 0.0;
  start_time = (double) tdata->scan[0].time;

  for (iscan = 0; iscan < nscans; iscan++, gp++) {
    
    delta_time = (double) tdata->scan[iscan].time - start_time;
    
    if (iscan == 0) {
      scan_duration =
	((double) tdata->scan[1].time -
	 (double) tdata->scan[0].time) / 2.0;
    } else if (iscan == (nscans - 1)) {
      scan_duration =
	((double) tdata->scan[nscans-1].time -
	 (double) tdata->scan[nscans-2].time) / 2.0;
    } else {
      scan_duration =
	((double) tdata->scan[iscan+1].time -
	 (double) tdata->scan[iscan-1].time) / 2.0;
    }
    
    sum_ad += tdata->scan[iscan].union_data[field_num] * scan_duration;
   
    sum_adt += (tdata->scan[iscan].union_data[field_num] *
		scan_duration * delta_time);

    sum_adt2 += (tdata->scan[iscan].union_data[field_num] *
		 scan_duration * delta_time * delta_time);

  }

  /*
   * compute parameters
   */

  A = sum_ad;
  T = sum_adt / A;
  D = sqrt((sum_adt2 - ((sum_adt * sum_adt) / A)) / A);
  C = A / (D * sqrt(2.0 * M_PI));

  /*
   * load up gpoints
   */

  gp = gpoints;
  for (iscan = 0; iscan < nscans; iscan++, gp++) {
    delta_time = (double) tdata->scan[iscan].time - start_time;
    dtt = (delta_time - T) / D;
    dtt2 = dtt * dtt;
    yy = C * exp((-1.0 * dtt2) / 2.0);
    gp->x = (double) tdata->scan[iscan].time;
    gp->y = (LOG_10(yy) + delta_cycle[act_num]);
  }

  return;

}

/*********************************************************
 * _draw_y_axes()
 *
 * Draw in y_axes for active fields
 */

static void _draw_y_axes(int dev,
			 gframe_t *frame,
			 thist_track_data_t *tdata,
			 int n_active,
			 int *fnum,
			 int *start_cycle,
			 int *end_cycle,
			 double *delta_cycle,
			 double plot_min_x, double plot_max_x,
			 double plot_min_y, double plot_max_y)

{
  
  char label[128];
  int iact;
  int field_num;
  int icycle;
  int direction, ascent, descent;
  double xx, delta_x = 0;
  double yy;
  double text_line_spacing;
  double val5;
  XCharStruct overall;

  if (dev == XDEV) {

    XQueryTextExtents(Glob->rdisplay,
		      Glob->x_text_font->fid,
		      "Abcdefjhijklmno",
		      14,
		      &direction, &ascent, &descent, &overall);

    text_line_spacing = ((double) (ascent + descent) * 0.9 /
			 frame->x->yscale);

  } else {
    
    text_line_spacing =
      (Glob->ps_ticklabel_fontsize * 1.1 ) / frame->ps->yscale;

  }

  if (n_active > 1) {
    delta_x = (plot_max_x - plot_min_x) / (double) n_active;
  }
  
  if (dev == PSDEV) {
    PsGsave(frame->psgc->file);
  }

  /*
   * plot vertical axis lines
   */
  
  xx = plot_max_x;
  for (iact = 0; iact < n_active; iact++, xx -= delta_x) {

    field_num = fnum[iact];

    /*
     * draw in vertica axis
     */
    
    if (dev == PSDEV) {
      PsSetLineStyle(frame->psgc->file,
		     &Glob->union_graph_psgc[field_num]);
    }

    GDrawLine(dev, frame, Glob->union_graph_gc[field_num], frame->psgc,
	      xx, plot_min_y, xx, plot_max_y);

    /*
     * draw in axis labels at even cycles (10 ** n)
     */
    
    for (icycle = start_cycle[iact] + 1; icycle < end_cycle[iact]; icycle++) {

      sprintf(label, "%d", icycle);
      yy = (double) icycle + delta_cycle[iact] + text_line_spacing / 2.0;
      
      GDrawImageString(dev, frame, Glob->ticklabel_gc,
		       frame->x->font, frame->psgc,
		       XJ_LEFT, YJ_ABOVE,
		       xx, yy, label);

      GDrawImageString(dev, frame, Glob->ticklabel_gc,
		       frame->x->font, frame->psgc,
		       XJ_RIGHT, YJ_BELOW,
		       xx, yy, "10");

    } /* icycle */

    /*
     * draw in axis labels at 0.5 points (0.5)
     */
    
    for (icycle = start_cycle[0]; icycle < end_cycle[0]; icycle++) {

      val5 = pow(10.0, (double) icycle) * 5.0;
      yy = LOG_10(fabs(val5));
      
      GDrawImageString(dev, frame, Glob->ticklabel_gc,
		       frame->x->font, frame->psgc,
		       XJ_CENTER, YJ_CENTER,
		       xx, yy, ".5");

    } /* icycle */

    /*
     * draw in labels and units
     */

    if (dev == XDEV) {
      yy = plot_max_y;
    } else {
      yy = plot_max_y - 0.5 * text_line_spacing;
    }

    GDrawImageString(dev, frame, Glob->ticklabel_gc,
		     frame->x->font, frame->psgc,
		     XJ_CENTER, YJ_BELOW,
		     xx, yy,
		     tdata->union_label[field_num]);

    yy -= text_line_spacing;

    GDrawImageString(dev, frame, Glob->ticklabel_gc,
		     frame->x->font, frame->psgc,
		     XJ_CENTER, YJ_BELOW,
		     xx, yy,
		     tdata->union_units[field_num]);
    
  } /* iact */
  
  if (dev == PSDEV) {
    PsGrestore(frame->psgc->file);
  }
      
}

/*********************************************************
 * _draw_x_ticks()
 *
 * Draw in x tick marks
 */

static void _draw_x_ticks(int dev,
			  gframe_t *frame,
			  thist_track_data_t *tdata)

{

  double xtick_length, xtick_margin;
  double xaxis_endmargin;
  double tick_x;
  double text_y;
  double header_clearance_x;
  long nticks, first_tick_time, tick_interval;

  char label[80];

  date_time_t tick_time;

  /*
   * set variables
   */
  
  if (dev == XDEV) {
    xtick_length = Glob->x_tick_length / frame->x->yscale;
    xtick_margin = xtick_length * TICK_CLEARANCE_FRACTION;
    xaxis_endmargin = Glob->x_xaxis_endmargin / frame->x->xscale;
    header_clearance_x = Glob->x_header_clearance / frame->x->xscale;
  } else {
    xtick_length = Glob->ps_tick_length / frame->ps->yscale;
    xtick_margin = xtick_length * TICK_CLEARANCE_FRACTION;
    xaxis_endmargin = Glob->ps_xaxis_endmargin / frame->x->xscale;
    header_clearance_x = Glob->ps_header_clearance / frame->x->xscale;
    PsSetLineWidth(frame->psgc->file,
		   Glob->ps_tick_line_width);
    PsSetFont(frame->psgc->file, frame->psgc->fontname,
	      Glob->ps_ticklabel_fontsize);
  }
  
  /*
   * compute ticks
   */

  compute_ticks(tdata->plot_start_time,
		tdata->forecast_end_time,
		&nticks,
		&first_tick_time,
		&tick_interval);

  /*
   * x axis tick marks and labels
   */

  text_y = (frame->w_ymin + xtick_length + xtick_margin);
  tick_x = (double) first_tick_time;
  tick_time.unix_time = first_tick_time;
  uconvert_from_utime(&tick_time);

  for (int i = 0; i < nticks; i++) {
    
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
    sprintf(label, "%s", Glob->track_shmem->time_label);
  } else {
    sprintf(label, "Time (%s)", Glob->track_shmem->time_label);
  }
  
  GDrawString(dev, frame,
	      Glob->ticklabel_gc,
	      Glob->x_ticklabel_font,
	      frame->psgc,
	      XJ_LEFT, YJ_ABOVE,
	      frame->w_xmin + header_clearance_x, text_y,
	      label);

  /*
   * plot in tick for current image time
   */

  if (dev == XDEV) {
    GDrawLine(dev, frame, Glob->current_gc, frame->psgc,
	      (double) Glob->track_shmem->time,
	      frame->w_ymin,
	      (double) Glob->track_shmem->time,
	      frame->w_ymin + xtick_length);
  }

}

/*********************************************************
 * _draw_y_ticks()
 *
 * Draw in y tick marks
 */

static void _draw_y_ticks(int dev,
			  gframe_t *frame,
			  int *start_cycle,
			  int *end_cycle,
			  double plot_min_x, double plot_max_x)

{

  double ytick_length;
  double ytick_margin;

  /*
   * set variables
   */
  
  if (dev == XDEV) {
    ytick_length = Glob->x_tick_length / frame->x->xscale;
    ytick_margin = ytick_length * TICK_CLEARANCE_FRACTION;
  } else {
    ytick_length = Glob->ps_tick_length / frame->ps->xscale;
    ytick_margin = ytick_length * TICK_CLEARANCE_FRACTION;
    PsSetLineWidth(frame->psgc->file,
		   Glob->ps_tick_line_width);
    PsSetFont(frame->psgc->file, frame->psgc->fontname,
	      Glob->ps_ticklabel_fontsize);
  }
  
  /*
   * y axis tick marks and labels
   */
  
  for (int icycle = start_cycle[0]; icycle < end_cycle[0]; icycle++) {

    double base_val = pow(10.0, (double) icycle);

    for (int i = 1; i <= 10; i++) {

      if (i == 10 && icycle != end_cycle[0]) 
	break;

      double tick_val = (double) i * base_val;
      double tick_y = LOG_10(fabs(tick_val));
      double tick_length;

      if (i == 1 || i == 5 || i == 10) {
	tick_length = ytick_length * 2.0;
	if (dev == PSDEV) {
	  PsSetLineWidth(frame->psgc->file,
			 Glob->ps_divider_line_width);
	}
	GDrawLine(dev, frame, Glob->divider_gc, frame->psgc,
		  plot_min_x, tick_y, plot_max_x, tick_y);
      } else {
	tick_length = ytick_length;
      }
	
      if (dev == PSDEV) {
	PsSetLineWidth(frame->psgc->file,
		       Glob->ps_tick_line_width);
      }
      
      GDrawLine(dev, frame, Glob->tick_gc, frame->psgc, 
		frame->w_xmax, tick_y, 
		frame->w_xmax - tick_length, tick_y);

    } /* i */

  } /* icycle */

}


