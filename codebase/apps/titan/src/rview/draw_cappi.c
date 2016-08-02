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
 * draw_cappi.c
 *
 * cappi window drawing routines
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * June 1990
 *
 *************************************************************************/

#include "rview.h"
#include <toolsa/servmap.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>

#define HDR_MAX 256
#define LABEL_MAX 256
  
#define N_AZIMUTH_LINES 6

/*
 * file scope variables
 */

static int Xaxismargin, Yaxismargin, Topmargin;
static int Approx_nticks;
static int Radar_on_plot;
static long Nticksx, Nticksy;

static double X_ticklength, Ps_ticklength;
static double Xticklength, Yticklength, Xtickmargin, Ytickmargin;
static double Xaxisendmargin, Yaxisendmargin;
static double Tick_clearance;
static double Tickmin_x, Tickmin_y;
static double Delta_tickx, Delta_ticky;
static double Text_line_spacing;
static double Xaxis_margin_d;
static double Text_margin_x, Text_margin_y;
static double Min_x, Max_x, Min_y, Max_y;

static double Azimuth[N_AZIMUTH_LINES] = {0.0, 30.0, 60.0,
					  90.0, 120.0, 150.0};
/*
 * file scope prototypes
 */

static int cappi_pixel_value(cd_grid_info_t *grid_info,
			     cd_reply_t *reply,
			     ui08 *image_data,
			     double x,
			     double y,
			     double *val_p,
			     char **units_p);

static double compute_range(double x, double y,
			    cd_grid_info_t *grid_info);
     
static void draw_annotation(int dev,
			    gframe_t *frame,
			    ui08 *image_data,
			    int n_tracks_plotted);

static void draw_cursor_info(int dev,
			     gframe_t *frame,
			     cd_grid_info_t *grid_info,
			     cd_reply_t *reply,
			     ui08 *image_data);

static void draw_header(int dev,
			gframe_t *frame,
			cd_grid_info_t *grid_info,
			cd_reply_t *reply,
			date_time_t *cappi_time,
			date_time_t *track_time,
			ui08 *image_data,
			int n_tracks_plotted,
			double dbz_threshold);
     
static void draw_margins(int dev,
			 gframe_t *frame,
			 cd_grid_info_t *grid_info);

static void draw_range_rings(int dev,
			     gframe_t *frame,
			     cd_grid_info_t *grid_info);

static void load_resources(int dev, gframe_t *frame,
			   cd_grid_info_t *grid_info);

static ui08 *read_cappi_plane(date_time_t **time_ptr,
			      cd_grid_info_t **info_ptr,
			      cd_reply_t **reply_ptr);

/************************************************************************
 * draw_cappi_title: draw cappi title
 */

void draw_cappi_title(int dev)

{

  gframe_t *frame;
  char tstring[BUFSIZ];

  if (Glob->debug) {
    fprintf(stderr, "** draw_cappi_title **\n");
  }

  frame = Glob->cappi_title_frame;

  /*
   * clear window
   */

  if (dev == XDEV) {
    XSetWindowBackground(Glob->rdisplay, frame->x->drawable,
			 Glob->background);
    safe_XClearWindow(Glob->rdisplay, frame->x->drawable);
  }

  if (Glob->cappi_info) {

    sprintf(tstring,"%s %s",
	    Glob->cappi_info->source_name,
	    Glob->cappi_info->field_name);

  } else {

    sprintf(tstring,"CAPPI");

  }
  
  GDrawString(dev, frame, frame->x->gc, frame->x->font,
	      frame->psgc, XJ_CENTER, YJ_CENTER,
	      (double) 0.5, (double) 0.5, tstring);

}

/************************************************************************
 * draw_cappi_button: draw button in button window
 */

void draw_cappi_button(si32 n,
		       ui32 background)

{

  static si32 first_call = TRUE;
  static char **button_label;
  char *label_str;
  int i;
  gframe_t *frame;

  if (Glob->debug) {
    fprintf(stderr, "** draw_cappi_button **\n");
  }

  frame = Glob->cappi_button_frame[n];

  /*
   * if first call, load up the label strings
   */

  if (first_call) {

    button_label = (char **) umalloc (N_CAPPI_BUTTONS * sizeof(char *));

    label_str = (char *) umalloc((ui32) (strlen(CAPPI_BUTTON_LABELS) + 1));
    strcpy(label_str, CAPPI_BUTTON_LABELS);

    for (i = 0; i < N_CAPPI_BUTTONS; i++) {

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

}

/************************************************************************
 * draw_cappi_plot.c
 *
 * draws cappi to pixmap
 *
 * In X, goes to a pixmap. In PS, goes to an output file.
 *
 */

void draw_cappi_plot(int dev,
		     g_color_scale_t *colors)

{
  
  static double dbz_threshold;
  
  static date_time_t *cappi_time;
  static date_time_t *track_time = NULL;
  
  static cd_grid_info_t *grid_info;
  static cd_reply_t *reply;
  static ui08 *image_data;

  si32 n_tracks_plotted = 0;

  double image_min_x, image_min_y, image_max_x, image_max_y;
  gframe_t *frame;
  zoom_t *zoom;
  
  Drawable window;
  GRectangle clip_rectangle[1];

  if (Glob->debug) {
    fprintf(stderr, "** draw_cappi_plot **\n");
  }

  /*
   * set local pointers
   */
  
  if (dev == XDEV)
    frame = Glob->cappi_plot_frame;
  else
    frame = Glob->cappi_ps_plot_frame;
  
  /*
   * calculate the zoom geometry
   */
  
  zoom_calc(dev, frame);
  
  /*
   * get image data from server - returns a NULL if no image
   * data is available
   */

  image_data = read_cappi_plane(&cappi_time, &grid_info, &reply);
  Glob->cappi_info = grid_info;

  /*
   * adjust the color scale to suit the scale and bias of the data
   */
  
  if (reply) {
    if (dev == XDEV) {
      GAdjustGCScale(colors, reply->scale, reply->bias);
    } else {
      GAdjustPsgcScale(colors, reply->scale, reply->bias);
    }
  }
  
  zoom = Glob->zoom + Glob->zoom_level;
  Min_x = zoom->min_x;
  Max_x = zoom->max_x;
  Min_y = zoom->min_y;
  Max_y = zoom->max_y;
  
  /*
   * If device is an X window,
   * save the frame drawable in 'window', and set it temporarily 
   * to 'pixmap'. Also initialize the pixmap.
   */
  
  if (dev == XDEV) {
    
    window = frame->x->drawable;
    
    frame->x->drawable = zoom->pixmap;
    
    XFillRectangle(Glob->rdisplay, frame->x->drawable,
		   Glob->pixmap_gc, 0, 0,
		   frame->x->width, frame->x->height);
    
  }
  
  /*
   * load resources
   */
  
  load_resources(dev, frame, grid_info);

  /*
   * set clipping - this is to keep the margins clear in PSDEV
   * X will clip to the drawn window and the margins will be cleared
   * by overwriting later on.
   */
  
  clip_rectangle->x = Min_x;
  clip_rectangle->y = Min_y;
  clip_rectangle->width = (Max_x - Min_x);
  clip_rectangle->height = (Max_y - Min_y);
  
  if (dev == PSDEV)
    PsGsave(frame->psgc->file);
  
  GSetClipRectangles(dev, frame, Glob->ring_gc, frame->psgc,
		     clip_rectangle, (int) 1);

  /*
   * plot image if requested
   */
  
  if (image_data && Glob->plot_image) {
    
    image_min_x = grid_info->min_x + reply->x1 * grid_info->dx;
    image_max_x = grid_info->min_x + (reply->x2 + 1) * grid_info->dx;
    image_min_y = grid_info->min_y + reply->y1 * grid_info->dy;
    image_max_y = grid_info->min_y + (reply->y2 + 1) * grid_info->dy;
  
    GDrawImageProgDetail(dev, frame, colors, image_min_x, image_min_y, 
			 image_max_x - image_min_x,
			 image_max_y - image_min_y,
			 reply->nx, reply->ny,
			 IMAGE_START_BOTLEFT,
			 image_data,
			 reply->bad_data_val, 1);
    
  }
  
  /*
   * plot range rings if required
   */
  
  draw_range_rings(dev, frame, grid_info);

  /*
   * plot in storm tracks
   */
  
  if (Glob->plot_tracks && Glob->track_data_available) {

    draw_tracks(dev, frame, &track_time,
		&dbz_threshold, &n_tracks_plotted);
    
  } /* if (Glob->plot_tracks && Glob->track_data_available) */
  
  /*
   * draw in maps if required
   */

  if (Glob->plot_maps) {
    draw_maps(dev, frame);
  }
  
  /*
   * plot in contours if desired
   */
  
  if (image_data && Glob->plot_cappi_contours) {
    
    GDrawContours(dev, frame,
		  Glob->pos_contour_gc,
		  Glob->zero_contour_gc,
		  Glob->neg_contour_gc,
		  &Glob->pos_contour_psgc,
		  &Glob->zero_contour_psgc,
		  &Glob->neg_contour_psgc,
		  image_min_x, image_min_y, 
		  image_max_x - image_min_x,
		  image_max_y - image_min_y,
		  reply->nx,
		  reply->ny,
		  IMAGE_START_BOTLEFT,
		  image_data,
		  reply->scale, reply->bias,
		  &Glob->fcontrol[Glob->field].contours,
		  reply->bad_data_val);
    
  }
  
  /*
   * draw in aircraft position
   */

  if (cappi_time)
    draw_ac_posn(dev, frame, cappi_time);
  
  /*
   * draw in verification data
   */

  if (cappi_time)
    draw_verify(dev, frame, cappi_time);

  /*
   * Annotation messages to the plot window
   */
  
  draw_annotation(dev, frame, image_data, n_tracks_plotted);

  /*
   * restore graphics state from clipping operation in postscript
   */
  
  if (dev == PSDEV)
    PsGrestore(frame->psgc->file);
  
  /*
   * draw cursor position information to window header
   */
  
  draw_cursor_info(dev, frame, grid_info, reply, image_data);

  /*
   * draw in details in the margins
   */
  
  draw_margins(dev, frame, grid_info);

  /*
   * write in header and unit labels
   */

  draw_header(dev, frame, grid_info, reply, cappi_time, track_time,
	      image_data, n_tracks_plotted, dbz_threshold);

  /*
   * set the plotframe drawable back to the window
   */
  
  if (dev ==  XDEV){
    frame->x->drawable = window;
  }
  
  /*
   * draw title
   */

  if (dev ==  XDEV){
    draw_cappi_title(dev);
  }
  
}

/************************************************************************
 * expose_cappi_pixmap: redraws plot on expose
 */

void expose_cappi_pixmap(void)

{

  gframe_t *frame;
  zoom_t *zoom;

  if (Glob->debug) {
    fprintf(stderr, "** expose_cappi_pixmap **\n");
  }

  frame = Glob->cappi_plot_frame;
  zoom = Glob->zoom + Glob->zoom_level;

  zoom_calc(XDEV, frame);

  /*
   * copy the pixmap to the window
   */

  XCopyArea(frame->x->display,
	    zoom->pixmap,
	    frame->x->drawable,
	    Glob->copyarea_gc, 0, 0,
	    frame->x->width, frame->x->height, 0, 0);
  
  /*
   * draw in line showing location of vertical section
   * if one has been selected
   */

  if (Glob->vsection.active) {

    GDrawLine(XDEV, frame, Glob->vsection_pos_gc, frame->psgc, 
	      Glob->vsection.start_x, Glob->vsection.start_y,
	      Glob->vsection.end_x, Glob->vsection.end_y);

  }

  /*
   * draw in zoom box rectangle if appropriate
   */

  if (Glob->zoom_level < NZOOM - 1) {

    zoom  = Glob->zoom + Glob->zoom_level + 1;

    if (zoom->active) {

      GDrawRectangle(XDEV, frame, Glob->zoom_box_gc, frame->psgc,
		     zoom->min_x, zoom->min_y,
		     zoom->max_x - zoom->min_x,
		     zoom->max_y - zoom->min_y);

    } /* if (zoom->active) */

  } /* if (Glob->zoom_level ... */

  safe_XFlush(Glob->rdisplay);
  
}

/******************
 * compute_range()
 */

static double compute_range(double x, double y,
			    cd_grid_info_t *grid_info)
     
{
  
  double dx, dy;

  dx = x - grid_info->source_x;
  dy = y - grid_info->source_y;
  
  return (sqrt(x * x + y * y));

}
/************************************************************************
 * cappi_pixel_value()
 *
 * gets the cappi pixel value at a given (x, y)
 *
 * returns 0 on success, -1 on failure
 */

static int cappi_pixel_value(cd_grid_info_t *grid_info,
			     cd_reply_t *reply,
			     ui08 *image_data,
			     double x,
			     double y,
			     double *val_p,
			     char **units_p)

{

  si32 ix, iy, jx, jy;
  si32 index;
  double value;

  if (!image_data)
    return (-1);

  ix = (si32) floor((x - grid_info->min_x) / grid_info->dx);
  iy = (si32) floor((y - grid_info->min_y) / grid_info->dy);

  if (ix < reply->x1 || ix > reply->x2 ||
      iy < reply->y1 || iy > reply->y2) {

    /*
     * point outside current data area
     */

    return (-1);

  } else {

    /*
     * point within current data
     */

    jx = ix - reply->x1;
    jy = iy - reply->y1;

    index = jy * reply->nx + jx;

    value = (double) image_data[index] * reply->scale + reply->bias;
    *val_p = value;
    *units_p = grid_info->field_units;

    return (0);

  } /* if (ix < reply->x1 || ix > reply->x2 ... */

}

/****************************************
 * draw_annotation()
 *
 * Put in the annotation
 */

static void draw_annotation(int dev,
			    gframe_t *frame,
			    ui08 *image_data,
			    int n_tracks_plotted)

{

  char label[LABEL_MAX];
  double textx, texty;
  
  /*
   * Messages to the plot window
   */
  
  textx = Min_x + Text_margin_x;
  texty = Max_y - Text_margin_y;
  
  if (image_data == NULL && Glob->plot_image) {
    
    GDrawString(dev, frame, Glob->text_gc, Glob->x_text_font,
		frame->psgc, XJ_LEFT, YJ_BELOW,
		textx, texty,
		"image not available");
    
    texty -= Text_line_spacing;
    
  }
  
  if (Glob->plot_tracks) {
    
    if (!Glob->track_data_available) {
      
      GDrawString(dev, frame, Glob->text_gc, Glob->x_text_font,
		  frame->psgc, XJ_LEFT, YJ_BELOW,
		  textx, texty,
		  "Track data not available");
      
      texty -= Text_line_spacing;
      
    } else if (n_tracks_plotted > 0) {

      if (Glob->annotate_tracks) {

	switch (Glob->annotate_tracks) {

	case SPEED_ANNOTATION:
	  strcpy(label, "Annot: forecast speeds in km/hr");
	  break;

	case MAX_DBZ_ANNOTATION:
	  strcpy(label, "Annot: max dbz");
	  break;
	  
	case VIL_ANNOTATION:
	  strcpy(label, "Annot: vil (kg/m2)");
	  break;
	  
	case TOPS_ANNOTATION:
	  strcpy(label, "Annot: tops (km)");
	  break;
	  
	case NUMBER_ANNOTATION:
	  strcpy(label, "Annot: track numbers");
	  break;
	    
	} /* switch */

	GDrawString(dev, frame, Glob->text_gc, Glob->x_text_font,
		    frame->psgc, XJ_LEFT, YJ_BELOW,
		    textx, texty, label);
	texty -= Text_line_spacing;
      
      } /* if (Glob->annotate_tracks) */

    } /* if (!Glob->track_data_available) */
    
  } /* if (Glob->plot_tracks) */

}

/****************************************
 * draw_cursor_info()
 *
 * Put cursor location info in window header.
 * Not for PostScript.
 */

static void draw_cursor_info(int dev,
			     gframe_t *frame,
			     cd_grid_info_t *grid_info,
			     cd_reply_t *reply,
			     ui08 *image_data)

{

  char cursor_str[LABEL_MAX * 5];
  char pixval_str[LABEL_MAX];
  char bearing_str[LABEL_MAX];
  char xy_str[LABEL_MAX];
  char latlon_str[LABEL_MAX];
  char *units;
  char dirn_units[16], dist_units[16];
  double pixval;
  double cross_arm;
  double lat, lon;
    
  if (dev == PSDEV) {
    return;
  }

  if (Glob->cursor_active) {

    /*
     * range and bearing into string
     */
    
    if (Glob->cursor_magnetic) {
      sprintf(dirn_units, "M");
    } else {
      sprintf(dirn_units, "T");
    }

    if (Glob->cursor_dist_nm) {
      sprintf(dist_units, "nm");
    } else {
      sprintf(dist_units, "km");
    }

    sprintf(bearing_str, "%.0f%s %.0f%s",
	    Glob->cursor_bearing, dirn_units,
	    Glob->cursor_dist, dist_units);
    
    /*
     * pixel value into string
     */

    if (cappi_pixel_value(grid_info, reply, image_data,
			  Glob->cursor_x, Glob->cursor_y,
			  &pixval, &units) == 0) {
      sprintf(pixval_str, "%.1f %s", pixval, units);
    } else {
      strcpy(pixval_str, "");
    }

    /*
     * X,Y posn into string
     */

    sprintf(xy_str, "(%.2f, %.2f)km", Glob->cursor_x, Glob->cursor_y);

    /*
     * lat/lon into string
     */
    
    PJGLatLonPlusDxDy(Glob->grid.proj_origin_lat,
		      Glob->grid.proj_origin_lon,
		      Glob->cursor_x, Glob->cursor_y,
		      &lat, &lon);
    
    sprintf(latlon_str, "(%.4f%s, %.4f%s)",
	    fabs(lat), (lat < 0? "S" : "N"),
	    fabs(lon), (lon < 0? "W" : "E"));
	    
    /*
     * put them together
     */

    sprintf(cursor_str, "%s %s %s %s",
	    pixval_str, xy_str, latlon_str, bearing_str);
    
    /*
     * put string into CAPPI window header
     */
    
    set_cappi_window_label(cursor_str);
    
#ifdef NOTNOW

    /*
     * put string on image
     */
    
    GDrawImageString(dev, frame, Glob->text_gc, Glob->x_text_font,
		     frame->psgc, XJ_LEFT, YJ_ABOVE,
		     frame->w_xmin + Text_margin_x,
		     frame->w_ymin + Text_margin_y + Xaxis_margin_d,
		     cursor_str);
#endif
	
    /*
     * draw cursor
     */
    
    cross_arm = (frame->w_xmax - frame->w_xmin) / 80.0;

    GDrawLine(dev, frame, Glob->crosshair_gc, frame->psgc, 
	      Glob->cursor_x - cross_arm, Glob->cursor_y,
	      Glob->cursor_x + cross_arm, Glob->cursor_y);
    
    GDrawLine(dev, frame, Glob->crosshair_gc, frame->psgc, 
	      Glob->cursor_x, Glob->cursor_y - cross_arm,
	      Glob->cursor_x, Glob->cursor_y + cross_arm);
    
  } else {
    
    set_cappi_window_label_default();
    
  } /* if (Glob->cursor_active) */

}
  
/****************************************
 * draw_header()
 *
 * Draw in the header information
 */

static void draw_header(int dev,
			gframe_t *frame,
			cd_grid_info_t *grid_info,
			cd_reply_t *reply,
			date_time_t *cappi_time,
			date_time_t *track_time,
			ui08 *image_data,
			int n_tracks_plotted,
			double dbz_threshold)
     
{
  
  char hstring[HDR_MAX], hstring1[HDR_MAX], hstring2[HDR_MAX];
  char hstring3[HDR_MAX], hstring4[HDR_MAX], hstring5[HDR_MAX];
  char hstring6[HDR_MAX], hstring7[HDR_MAX];

  hstring1[0] = '\0';
  hstring2[0] = '\0';
  hstring3[0] = '\0';
  hstring4[0] = '\0';
  hstring5[0] = '\0';
  hstring6[0] = '\0';
  hstring7[0] = '\0';

  /*
   * date, time and altitude
   */
  
  if (cappi_time)
    sprintf(hstring1, "%s  ", utimestr(cappi_time));

  if (image_data != NULL) {

    if (Glob->plot_composite) {

      sprintf(hstring2, "Composite");

    } else {

      if (reply && reply->z1 >= 0 && reply->z2 >= 0)
	sprintf(hstring2, "%g(%s)",
		Glob->z_cappi, grid_info->units_label_z);
      
    } /* if (Glob->plot_composite */  

  } else {

    if (Glob->plot_composite) {

      sprintf(hstring2, "Composite");

    } else {

      if (reply && reply->z1 >= 0 && reply->z2 >= 0)
	sprintf(hstring2, "%g(km)", Glob->z_requested);
      
    } /* if (Glob->plot_composite */  

  } /* if (image_data != NULL) */
  
  /*
   * track number and times
   */
  
  if (Glob->plot_tracks &&
      Glob->track_data_available &&
      n_tracks_plotted > 0) {
      
    if (image_data == NULL) {
      
      sprintf(hstring3,
	      " Tracks to %s", utimestr(track_time));
      
    } else if (cappi_time &&
	       cappi_time->unix_time != track_time->unix_time) {

      sprintf(hstring3,
	      " Tracks to %.2d:%.2d:%.2d",
	      track_time->hour, track_time->min, track_time->sec);
	
    } /* if (image_data == NULL) */
      
    sprintf(hstring4, " Tz %g", dbz_threshold);
    
    /*
     * past track length if applicable
     */
    
    if (Glob->plot_past == PAST_LIMITED) {
      
      sprintf(hstring5, " Past %.1g(hr)",
	      Glob->past_plot_period / 3600.0);
      
    } /* if (Glob->plot_past == PAST_LIMITED) */
    
    /*
     * future track length if applicable
     */
    
    if (Glob->plot_future == FUTURE_LIMITED) {
      
      sprintf(hstring6, " Future %.1g(hr)",
	      Glob->future_plot_period / 3600.0);
      
    } /* if (Glob->plot_past == FUTURE_LIMITED) */
    
    /*
     * forecast track length if applicable
     */
    
    if (Glob->plot_forecast) {
      
      sprintf(hstring7, " Forecast %.1g(hr)",
	      (double) Glob->n_forecast_steps * Glob->forecast_interval /
	      3600.0);
      
    } /* if (Glob->plot_past == FUTURE_LIMITED) */
    
  } /* if (Glob->plot_tracks ...  */
  
  /*
   *  put them all together
   */

  memset(hstring, 0, HDR_MAX);
  STRconcat(hstring, hstring1, HDR_MAX);
  STRconcat(hstring, hstring2, HDR_MAX);
  STRconcat(hstring, hstring3, HDR_MAX);
  STRconcat(hstring, hstring4, HDR_MAX);
  STRconcat(hstring, hstring5, HDR_MAX);
  STRconcat(hstring, hstring6, HDR_MAX);
  STRconcat(hstring, hstring7, HDR_MAX);

  if (dev == PSDEV) {
    PsSetFont(frame->psgc->file, frame->psgc->fontname,
	      Glob->ps_text_fontsize);
  }
  
  if (Glob->draw_copy_header || dev != PSDEV) {
    GDrawString(dev, frame, Glob->text_gc, Glob->x_text_font,
		frame->psgc, XJ_LEFT, YJ_BELOW,
		frame->w_xmin + Text_margin_x,
		frame->w_ymax - Text_margin_y,
		hstring);
  }
  
}
  
/****************************************
 * draw_margins()
 *
 * Clear the margins, draw in margin info
 */

static void draw_margins(int dev,
			 gframe_t *frame,
			 cd_grid_info_t *grid_info)

{

  char label[LABEL_MAX];

  int i, ilabel;

  double textx, texty;
  double gridx, gridy;

  /*
   * if X, clear the rectangles for the margins
   */
  
  if (Topmargin != 0)
    GFillRectangle(dev, frame,
		   Glob->pixmap_gc,
		   frame->psgc,
		   Min_x,
		   Max_y,
		   frame->w_xmax - frame->w_xmin,
		   frame->w_ymax - Max_y);
  
  if (Xaxismargin != 0)
    GFillRectangle(dev, frame,
		   Glob->pixmap_gc,
		   frame->psgc,
		   frame->w_xmin,
		   frame->w_ymin,
		   frame->w_xmax - frame->w_xmin,
		   Min_y - frame->w_ymin);
  
  if (Yaxismargin != 0)
    GFillRectangle(dev, frame,
		   Glob->pixmap_gc,
		   frame->psgc,
		   Max_x,
		   frame->w_ymin,
		   frame->w_xmax - Max_x,
		   frame->w_ymax - frame->w_ymin);
  
  /*
   * if printer, set crosshair line width
   */
  
  if (dev == PSDEV)
    PsSetLineWidth(frame->psgc->file,
		   xGetResDouble(Glob->rdisplay, Glob->prog_name,
				 "ps_crosshair_width", PS_CROSSHAIR_WIDTH));
  
  /*
   * draw in lines between grid and data
   */
  
  
  if (Yaxismargin != 0)
    GDrawLine(dev, frame, frame->x->gc, frame->psgc,
	      Max_x, Min_y, Max_x, Max_y);
  
  if (Xaxismargin != 0)
    GDrawLine(dev, frame, frame->x->gc, frame->psgc,
	      Min_x, Min_y, Max_x, Min_y);
  
  if (Topmargin != 0)
    GDrawLine(dev, frame, frame->x->gc, frame->psgc,
	      Min_x, Max_y, Max_x, Max_y);
  
  /*
   * if printer, set tick line width and ticklabel fontsize
   */
  
  if (dev == PSDEV) {
    PsSetLineWidth(frame->psgc->file,
		   xGetResDouble(Glob->rdisplay,
				 Glob->prog_name,
				 "ps_tick_width", PS_TICK_WIDTH));
    PsSetFont(frame->psgc->file, frame->psgc->fontname,
	      Glob->ps_ticklabel_fontsize);
  }
  
  /*
   * grid and scale marks
   */
  
  texty = (frame->w_ymin + Xticklength + Xtickmargin);
  gridx = Tickmin_x;
  
  for (i = 0; i < Nticksx; i++) {
    
    if (gridx > Min_x + Xaxisendmargin && gridx < Max_x - Xaxisendmargin) {
      
      ilabel = (si32) floor((double)(gridx + 0.5));
      
      if (ilabel != 0 || Radar_on_plot == FALSE || Xaxismargin != 0) {
	GDrawLine(dev, frame, Glob->tick_gc, frame->psgc, 
		  gridx, frame->w_ymin,
		  gridx, frame->w_ymin + Xticklength);
	if (Delta_tickx >= 1.0) {
	  sprintf(label, "%d", ilabel);
	} else if (Delta_tickx >= 0.1) {
	  sprintf(label, "%.1f", gridx);
	} else {
	  sprintf(label, "%.2f", gridx);
	}
	GDrawString(dev, frame, Glob->ticklabel_gc, frame->x->font,
		    frame->psgc,
		    XJ_CENTER, YJ_ABOVE, gridx, texty, label);
      }
      
    }
    
    gridx += Delta_tickx;
    
  }
  
  textx = frame->w_xmax - Yticklength - Ytickmargin;
  gridy = Tickmin_y;
  
  for (i = 0; i < Nticksy; i++) {
    
    if (gridy > Min_y + Yaxisendmargin && gridy < Max_y - Yaxisendmargin) {
      
      ilabel = (si32) floor((double)(gridy + 0.5));
      
      if (ilabel != 0 || Radar_on_plot == FALSE || Yaxismargin != 0) {
	GDrawLine(dev, frame, Glob->tick_gc, frame->psgc, 
		  frame->w_xmax, gridy, 
		  frame->w_xmax - Yticklength, gridy);
	if (Delta_ticky >= 1.0) {
	  sprintf(label, "%d", ilabel);
	} else if (Delta_ticky >= 0.1) {
	  sprintf(label, "%.1f", gridy);
	} else {
	  sprintf(label, "%.2f", gridy);
	}
	GDrawString(dev, frame, Glob->ticklabel_gc, frame->x->font,
		    frame->psgc,
		    XJ_RIGHT, YJ_CENTER, textx, gridy, label);
      }
      
    }
    
    gridy += Delta_ticky;
    
  }

  if (grid_info) {
    GDrawString(dev, frame, Glob->text_gc, Glob->x_text_font,
		frame->psgc, XJ_LEFT, YJ_ABOVE,
		frame->w_xmin + Text_margin_x,
		frame->w_ymin + Text_margin_y,
		grid_info->units_label_x);
    
    GDrawString(dev, frame, Glob->text_gc, Glob->x_text_font,
		frame->psgc, XJ_RIGHT, YJ_BELOW,
		frame->w_xmax - Text_margin_x,
		frame->w_ymax - Text_margin_y,
		grid_info->units_label_y);
  }
    
}

/****************************************
 * draw_range_rings()
 *
 * Draw range rings and azimuth lines
 */

static void draw_range_rings(int dev,
			     gframe_t *frame,
			     cd_grid_info_t *grid_info)

{

  char label[LABEL_MAX];
  
  int i, j;
  int startring, endring;

  double min_angle, max_angle;
  double radius;
  double ringlabelx, ringlabely;
  double minrange, maxrange, cornerrange;
  double ringint;
  double corner_angle[4], angle;
  double xx1, xx2, yy1, yy2;

  /*
   * plot range rings if required
   */
  
  if (grid_info && Glob->plot_rings) {
    
    /*
     * set ring label font size and line width
     */
    
    if (dev == PSDEV) {
      PsSetFont(frame->psgc->file, frame->psgc->fontname,
		Glob->ps_ringlabel_fontsize);
      PsSetLineWidth(frame->psgc->file,
		     xGetResDouble(Glob->rdisplay,
				   Glob->prog_name,
				   "ps_ring_width", PS_RING_WIDTH));
    }
    
    /*
     * compute max range plotted by considering each corner in turn
     */
    
    maxrange = compute_range(Min_x, Min_y, grid_info);
    minrange = compute_range(Min_x, Min_y, grid_info);
    
    cornerrange = compute_range(Min_x, Max_y, grid_info);
    if (cornerrange < minrange) minrange = cornerrange;
    if (cornerrange > maxrange) maxrange = cornerrange;
    
    cornerrange = compute_range(Max_x, Min_y, grid_info);
    if (cornerrange < minrange) minrange = cornerrange;
    if (cornerrange > maxrange) maxrange = cornerrange;
    
    cornerrange = compute_range(Max_x, Max_y, grid_info);
    if (cornerrange < minrange) minrange = cornerrange;
    if (cornerrange > maxrange) maxrange = cornerrange;
    
    if (Min_x <= grid_info->source_x &&
	Max_x >= grid_info->source_x) {
      
      minrange = MIN (fabs(grid_info->source_y - Min_y),
		      fabs(grid_info->source_y - Max_y));
      
    } else if (Min_y <= grid_info->source_y &&
	       Max_y >= grid_info->source_y) {
      
      minrange = MIN (fabs(grid_info->source_x - Min_x),
		      fabs(grid_info->source_x - Max_x));
      
    }
    
    if (Radar_on_plot)
      minrange = 0;
    
    if (Delta_tickx > Delta_ticky) {
      startring = (si32) (minrange / Delta_tickx);
      endring = (si32) (maxrange / Delta_tickx) + 1;
      ringint = Delta_tickx;
    } else {
      startring = (si32) (minrange / Delta_ticky);
      endring = (si32) (maxrange / Delta_ticky) + 1;
      ringint = Delta_ticky;
    }
    
    if (startring < 0)
      startring = 0;
    
    /*
     * compute the angles from the radar to the corners
     */
    
    if (Radar_on_plot) {
      
      min_angle = 0.0;
      max_angle = 360.0;
      
    } else {
      
      min_angle = LARGE_DOUBLE;
      max_angle = -LARGE_DOUBLE;
      
      /*
       * compute the angles from the radar to the corner points
       */
      
      corner_angle[0] = atan2((double)(Min_y -grid_info->source_y),
			      (double)(Min_x -grid_info->source_x));
      corner_angle[1] = atan2((double)(Min_y -grid_info->source_y),
			      (double)(Max_x -grid_info->source_x));
      corner_angle[2] = atan2((double)(Max_y -grid_info->source_y),
			      (double)(Min_x -grid_info->source_x));
      corner_angle[3] = atan2((double)(Max_y -grid_info->source_y),
			      (double)(Max_x -grid_info->source_x));
      
      for (i = 0; i < 4; i++)
	corner_angle[i] *= (double) 180.0 / M_PI;
      
      /*
       * if the plot rectangle straddles the 180 degree line,
       * add 360 to the negative angles
       */
      
      if (Max_x < grid_info->source_x &&
	  Max_y > grid_info->source_y &&
	  Min_y < grid_info->source_y) {
	for (i = 0; i < 4; i++)
	  if (corner_angle[i] < 0)
	    corner_angle[i] += 360.0;
      }
      
      /*
       * select the min and max angles
       */
      
      for (i = 0; i < 4; i++) {
	if (corner_angle[i] < min_angle)
	  min_angle = (double) corner_angle[i];
	if (corner_angle[i] > max_angle)
	  max_angle = (double) corner_angle[i];
      }
      
    } /* if (Radar_on_plot) */
    
    /*
     * draw in azimuth lines
     */
    
    for (i = 0; i < N_AZIMUTH_LINES; i++) {
      
      xx2 = maxrange * cos(Azimuth[i] * DEG_TO_RAD);
      yy2 = maxrange * sin(Azimuth[i] * DEG_TO_RAD);
      xx1 = - xx2;
      yy1 = - yy2;
      
      GDrawLine(dev, frame, Glob->ring_gc, frame->psgc,
		xx1 + grid_info->source_x,
		yy1 + grid_info->source_y,
		xx2 + grid_info->source_x,
		yy2 + grid_info->source_y);
      
    } /* i */
    
    /*
     * draw range rings
     */
    
    for (i  = startring; i <= endring; i++) {
      
      radius = (i + 1) * ringint;
      
      GDrawArc(dev, frame, Glob->ring_gc, frame->psgc,
	       grid_info->source_x, grid_info->source_y,
	       radius, radius,
	       min_angle, max_angle, 0.0, 360);
      
      /*
       * plot in range numbers
       */
      
      sprintf(label, "%d", (int) floor((double)(radius + 0.5)));
      
      if (Radar_on_plot) {
	
	for (j = 1; j <= 7; j += 2) {
	  
	  angle = (double) j * M_PI / (double) 4.0;
	  ringlabelx = grid_info->source_x + radius * cos(angle);
	  ringlabely = grid_info->source_y + radius * sin(angle);
	  GDrawImageString(dev, frame, Glob->ring_gc,
			   Glob->x_ringlabel_font,
			   frame->psgc, XJ_CENTER, YJ_CENTER,
			   ringlabelx, ringlabely, label);
	  
	}
	
      } else {
	
	angle = (double) (((min_angle + max_angle) / 2.0) * 
			  M_PI / 180.0);
	ringlabelx = grid_info->source_x + radius * cos(angle);
	ringlabely = grid_info->source_y + radius * sin(angle);
	GDrawImageString(dev, frame, Glob->ring_gc, Glob->x_ringlabel_font,
			 frame->psgc, XJ_CENTER, YJ_CENTER,
			 ringlabelx, ringlabely, label);
	
      } /* if (Radar_on_plot) */
      
    } /* i */
    
  } /* if (Glob->plot_rings) */
  
}

/******************
 * load_resources()
 *
 * Load up plotting resource values - X or PS
 */

static void load_resources(int dev, gframe_t *frame,
			   cd_grid_info_t *grid_info)

{

  int direction, ascent, descent;
  double unitscale;
  XCharStruct overall;

  switch (dev) {
    
  case XDEV:
    
    Xaxismargin = xGetResLong(Glob->rdisplay, Glob->prog_name,
			      "x_xaxismargin", X_XAXISMARGIN);

    Yaxismargin = xGetResLong(Glob->rdisplay, Glob->prog_name,
			      "x_yaxismargin", X_YAXISMARGIN);

    Topmargin = xGetResLong(Glob->rdisplay, Glob->prog_name,
			    "x_topmargin", X_TOPMARGIN);

    Xaxisendmargin = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				   "x_xaxisendmargin",
				   X_XAXISENDMARGIN) / frame->x->xscale;

    Yaxisendmargin = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				   "x_yaxisendmargin",
				   X_YAXISENDMARGIN) / frame->x->yscale;
    
    Tick_clearance = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				   "x_tick_clearance",
				   X_TICK_CLEARANCE);

    X_ticklength = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				 "x_ticklength",
				 X_TICKLENGTH);

    Xticklength = X_ticklength / frame->x->yscale;
    Yticklength = X_ticklength / frame->x->xscale;
    Xtickmargin = Tick_clearance / frame->x->xscale;
    Ytickmargin = Tick_clearance / frame->x->yscale;
    
    Xaxis_margin_d = Xaxismargin / frame->x->yscale;

    Text_margin_x =
      xGetResDouble(Glob->rdisplay, Glob->prog_name,
		    "x_text_margin",
		    X_TEXT_MARGIN) / frame->x->xscale;
    
    Text_margin_y =
      xGetResDouble(Glob->rdisplay, Glob->prog_name,
		    "x_text_margin",
		    X_TEXT_MARGIN) / frame->x->yscale;

    XQueryTextExtents(Glob->rdisplay,
		      Glob->x_text_font->fid,
		      "AaGgJjWwQqYyZz",
		      14,
		      &direction, &ascent, &descent, &overall);

    Text_line_spacing = ((double) (ascent + descent) * 1.4 /
			 frame->x->yscale);
		      
    break;
    
  case PSDEV:

    unitscale = xGetResDouble(Glob->rdisplay, Glob->prog_name,
			      "ps_unitscale",
			      PS_UNITSCALE);
    
    Xaxismargin = (si32) (xGetResDouble(Glob->rdisplay, Glob->prog_name,
					"ps_xaxismargin",
					PS_XAXISMARGIN) * unitscale);

    Yaxismargin = (si32) (xGetResDouble(Glob->rdisplay, Glob->prog_name,
					"ps_yaxismargin",
					PS_YAXISMARGIN) * unitscale);
    Topmargin = (si32) (xGetResDouble(Glob->rdisplay, Glob->prog_name,
				      "ps_topmargin",
				      PS_TOPMARGIN) * unitscale);
    
    Xaxisendmargin =
      xGetResDouble(Glob->rdisplay, Glob->prog_name, "ps_xaxisendmargin",
		    PS_XAXISENDMARGIN) * unitscale / frame->ps->xscale;
    Yaxisendmargin =
      xGetResDouble(Glob->rdisplay, Glob->prog_name, "ps_yaxisendmargin",
		    PS_YAXISENDMARGIN) * unitscale / frame->ps->yscale;
    
    Tick_clearance =
      xGetResDouble(Glob->rdisplay, Glob->prog_name, "ps_tick_clearance",
		    PS_TICK_CLEARANCE) * unitscale;
    
    Ps_ticklength = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				  "ps_ticklength",
				  PS_TICKLENGTH) * unitscale;
    
    Xticklength = Ps_ticklength / frame->ps->yscale;
    Yticklength = Ps_ticklength / frame->ps->xscale;
    Xtickmargin = Tick_clearance / frame->ps->xscale;
    Ytickmargin = Tick_clearance / frame->ps->yscale;
    
    Xaxis_margin_d = (double) Xaxismargin * unitscale / frame->ps->yscale;

    Text_margin_x =
      xGetResDouble(Glob->rdisplay, Glob->prog_name, "ps_text_margin",
		    PS_TEXT_MARGIN) * unitscale / frame->ps->xscale;
    Text_margin_y =
      xGetResDouble(Glob->rdisplay, Glob->prog_name, "ps_text_margin",
		    PS_TEXT_MARGIN) * unitscale / frame->ps->yscale;
    
    Text_line_spacing = (Glob->ps_text_fontsize * 1.4) / frame->ps->yscale;

    break;
    
  }
  
  Approx_nticks = xGetResLong(Glob->rdisplay, Glob->prog_name,
			      "approx_nticks",
			      APPROX_NTICKS);
  
  /*
   * compute linear tick parameters
   */
  
  GLinearTicks(Min_x, Max_x, Approx_nticks,
	       &Nticksx, &Tickmin_x, &Delta_tickx);
  
  GLinearTicks(Min_y, Max_y, Approx_nticks,
	       &Nticksy, &Tickmin_y, &Delta_ticky);

  /*
   * deduce if radar is on plot
   */
  
  if (grid_info &&
      Min_x <= grid_info->source_x && Max_x >= grid_info->source_x &&
      Min_y <= grid_info->source_y && Max_y >= grid_info->source_y) {
    
    Radar_on_plot = TRUE;
    
  } else {
    
    Radar_on_plot = FALSE;
    
  }
  
}

/*********************************************************************
 * read_cappi_plane()
 *
 * Reads a cappi plane from the server.
 *
 * Returns a pointer to the data plane, NULL if failure.
 *
 */

static ui08 *read_cappi_plane(date_time_t **time_ptr,
			      cd_grid_info_t **info_ptr,
			      cd_reply_t **reply_ptr)

{
  
  static date_time_t cappi_time;
  static cd_reply_t reply;
  static cd_grid_info_t grid_info;

  si32 pht_index;

  field_control_t *fcontrol;
  zoom_t *zoom;

  cd_command_t *command;
  cdata_index_t *cdata_index;

  if (Glob->debug) {
    fprintf(stderr, "** read_cappi_plane **\n");
  }

  /*
   * initialize
   */

  MEM_zero(cappi_time);
  MEM_zero(reply);
  MEM_zero(grid_info);

  *time_ptr = &cappi_time;
  *reply_ptr = &reply;
  *info_ptr = &grid_info;

  /*
   * return NULL if nfields is not positive
   */
  
  if (Glob->nfields < 1) {
    return (NULL);
  }

  /*
   * set local pointers
   */

  fcontrol = Glob->fcontrol + Glob->field;
  cdata_index = &fcontrol->cdata_index;
  zoom = Glob->zoom + Glob->zoom_level;
  command = &cdata_index->command;
  
  /*
   * load up the command struct
   */

  MEM_zero(*command);

  if (Glob->mode == ARCHIVE) {
    
    command->primary_com = (GET_INFO |
			    GET_DATA | GET_PLANE_HEIGHTS);
    command->time_min = Glob->time - fcontrol->time_window;
    command->time_max = Glob->time + fcontrol->time_window;
    command->time_cent = Glob->time;
    cdata_index->want_realtime = FALSE;
    
  } else {

    command->primary_com = (GET_MOST_RECENT | GET_INFO |
			    GET_DATA | GET_PLANE_HEIGHTS);
    cdata_index->want_realtime = TRUE;
    
  }

  if (Glob->plot_composite) {
    command->second_com = GET_MAX_XY_PLANE;
  } else {
    command->second_com = GET_XY_PLANE;
  }
  
  command->lat_origin = Glob->grid.proj_origin_lat;
  command->lon_origin = Glob->grid.proj_origin_lon;
  command->ht_origin = 0;
  
  command->min_x = zoom->min_x;
  command->min_y = zoom->min_y;
  command->max_x = zoom->max_x;
  command->max_y = zoom->max_y;
  command->min_z = Glob->z_requested - Glob->delta_z;
  command->max_z = Glob->z_requested + Glob->delta_z;

  command->data_field = fcontrol->field;

  if (Glob->debug) {
    fprintf(stderr, "----> cdata_command:\n");
    fprintf(stderr, "primary_com: %d\n", command->primary_com);
    fprintf(stderr, "time_min: %s\n", utimstr(command->time_min));
    fprintf(stderr, "time_max: %s\n", utimstr(command->time_max));
    fprintf(stderr, "time_cent: %s\n", utimstr(command->time_cent));
    fprintf(stderr, "second_com: %d\n", command->second_com);
    fprintf(stderr, "lat_origin: %g\n", command->lat_origin);
    fprintf(stderr, "lon_origin: %g\n", command->lon_origin);
    fprintf(stderr, "min_x: %g\n", command->min_x);
    fprintf(stderr, "min_y: %g\n", command->min_y);
    fprintf(stderr, "min_z: %g\n", command->min_z);
    fprintf(stderr, "max_x: %g\n", command->max_x);
    fprintf(stderr, "max_y: %g\n", command->max_y);
    fprintf(stderr, "max_z: %g\n", command->max_z);
  }
  
  /*
   * get the data
   */
  
  if (cdata_read(cdata_index)) {
    return (NULL);
  }

  /*
   * set return values
   */

  grid_info = cdata_index->grid_info;
  reply = cdata_index->reply;
  cappi_time.unix_time = reply.time_cent;
  uconvert_from_utime(&cappi_time);
  
  /*
   * set variables according to the reply
   */
  
  if (reply.z1 >= 0 && !Glob->plot_composite) {

    if (cdata_index->plane_heights != NULL) {
      pht_index =
	reply.z1 * N_PLANE_HEIGHT_VALUES + PLANE_MIDDLE_INDEX;
      /*
       * plane heights applicable
       */
      Glob->z_cappi = cdata_index->plane_heights[pht_index];
    } else {
      Glob->z_cappi = (grid_info.min_z +
		       grid_info.dz * ((double) reply.z1 + 0.5));
    }

  } /* if (reply.z1 >= 0 && !Glob->plot_composite) */

  if (!Glob->use_time_hist) {
    Glob->track_shmem->time = cappi_time.unix_time;
  }

  /*
   * Return plane pointer
   */

  return (cdata_index->data);

}

