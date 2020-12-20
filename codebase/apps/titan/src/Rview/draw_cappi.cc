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
 * draw_cappi.c
 *
 * cappi window drawing routines
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * June 1990
 *
 *************************************************************************/

#include "Rview.hh"
#include <toolsa/servmap.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/DateTime.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxContour.hh>
#include <iostream>
using namespace std;

#define HDR_MAX 256
#define LABEL_MAX 256
  
#define N_AZIMUTH_LINES 6

/*
 * file scope variables
 */

static int Xaxismargin, Yaxismargin, Topmargin;
static int Approx_nticks;
static int RingCenter_on_plot;
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
static double RingCenterX, RingCenterY;
static bool suppressRings = false;
static double Azimuth[N_AZIMUTH_LINES] = {0.0, 30.0, 60.0,
					  90.0, 120.0, 150.0};

static string cappiFieldName;
static date_time_t CappiTime;
static bool HaveCappiData = false;

/*
 * file scope prototypes
 */

static int cappi_pixel_value(const Mdvx::field_header_t &floatFhdr,
			     fl32 *float_cappi,
			     double x, double y,
			     double offset_x, double offset_y,
			     double *val_p,
			     char *units);

static double compute_range(double x, double y);
     
static void draw_annotation(int dev,
			    gframe_t *frame,
			    int n_tracks_plotted);

static void draw_cursor_info(int dev,
			     gframe_t *frame,
			     const Mdvx::field_header_t &floatFhdr,
			     fl32 *float_cappi,
			     double offset_x, double offset_y);

static void draw_header(int dev,
			gframe_t *frame,
			const Mdvx::field_header_t &byteFhdr,
			date_time_t *track_time,
			int n_tracks_plotted,
			double dbz_threshold);
     
static void draw_margins(int dev,
			 gframe_t *frame,
			 const Mdvx::field_header_t &byteFhdr);

static void draw_range_rings(int dev,
			     gframe_t *frame);

static void draw_vsection_line(gframe_t *frame);

static void load_resources(int dev, gframe_t *frame);

static int read_cappi(Mdvx::master_header_t &mhdr,
		      MdvxField &floatField,
		      MdvxField &byteField);

static double km_diagonal(gframe_t *frame);

static void setLocaltimeOffsetSecs();

/********************
 * get the cappi time
 */

const date_time_t &get_cappi_time()
{
  return CappiTime;
}

string get_cappi_field_name()
{
  if (HaveCappiData) {
    return cappiFieldName;
  } else {
    return "No gridded data available";
  }
}

/************************************************************************
 * draw_cappi_title: draw cappi title
 */

void draw_cappi_title(int dev)

{

  gframe_t *frame;

  if (Glob->verbose) {
    fprintf(stderr, "** draw_cappi_title **\n");
  }

  frame = Glob->cappi_title_frame;

  /*
   * If device is an X window,
   * save the frame drawable in 'window', and set it temporarily 
   * to the pixmap. Also initialize the pixmap.
   * Drawing will be done to the pixmap.
   */
  
  Drawable window = 0;
  if (dev == XDEV) {
    window = frame->x->drawable;
    frame->x->drawable = frame->x->pixmap;
    XFillRectangle(Glob->rdisplay, frame->x->drawable,
		   Glob->pixmap_gc, 0, 0,
		   frame->x->width, frame->x->height);
  }
  
  /*
   * clear window
   */

//   if (dev == XDEV) {
//     XSetWindowBackground(Glob->rdisplay, frame->x->drawable,
// 			 Glob->background);
//     safe_XClearWindow(Glob->rdisplay, frame->x->drawable);
//   }

  string fieldName(get_cappi_field_name());
  GDrawString(dev, frame, frame->x->gc, frame->x->font,
	      frame->psgc, XJ_CENTER, YJ_CENTER,
	      (double) 0.5, (double) 0.5,
	      (char *) fieldName.c_str());

  /*
   * set the plotframe drawable back to the window
   */
  
  if (dev ==  XDEV){
    
    frame->x->drawable = window;
    
    // copy backing pixmap to window
    
    XCopyArea(frame->x->display,
	      frame->x->pixmap,
	      frame->x->drawable,
	      Glob->copyarea_gc, 0, 0,
	      frame->x->width, frame->x->height, 0, 0);

  }

}

/************************************************************************
 * draw_cappi_button: draw button in button window
 */

void draw_cappi_button(si32 n, ui32 background)

{

  static si32 first_call = TRUE;
  static char **button_label;
  char *label_str;
  int i;
  gframe_t *frame;

  if (Glob->verbose) {
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
  XClearWindow(Glob->rdisplay, frame->x->drawable);

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

void draw_cappi_plot(int dev, g_color_scale_t *colors)

{
  
  static double dbz_threshold;
  static date_time_t *track_time = NULL;
  
  int n_tracks_plotted = 0;

  double image_min_x, image_min_y, image_max_x, image_max_y;
  gframe_t *frame;
  zoom_t *zoom;
  
  GRectangle clip_rectangle[1];

  if (Glob->verbose) {
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
   * get cappi data
   */

  MdvxField byteField;
  MdvxField floatField;
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);
    
  ui08 *byte_cappi = NULL;
  fl32 *float_cappi = NULL;
  HaveCappiData = false;
  Glob->cappi_plotted = false;
  Glob->tracks_plotted = false;

  if (read_cappi(mhdr, floatField, byteField) == 0) {
    HaveCappiData = true;
    float_cappi = (fl32*) floatField.getVol();
    byte_cappi = (ui08*) byteField.getVol();
  }
  const Mdvx::field_header_t &byteFhdr = byteField.getFieldHeader();
  const Mdvx::field_header_t floatFhdr = floatField.getFieldHeader();

  /*
   * compute ringCenter position in (x, y)
   */

  suppressRings = false;
  if (Glob->center_rings_on_origin) {
    Glob->proj.latlon2xy(Glob->proj.getCoord().proj_origin_lat,
			 Glob->proj.getCoord().proj_origin_lon,
			 RingCenterX, RingCenterY);
  } else {
    Glob->proj.latlon2xy(mhdr.sensor_lat, mhdr.sensor_lon,
			 RingCenterX, RingCenterY);
    if (mhdr.sensor_lat == 0 && mhdr.sensor_lon == 0) {
      suppressRings = true;
    }
  }
  
  /*
   * adjust the color scale to suit the scale and bias of the data
   */
  if (HaveCappiData) {
    if (dev == XDEV) {
      GAdjustGCScale(colors, byteFhdr.scale, byteFhdr.bias);
    } else {
      GAdjustPsgcScale(colors, byteFhdr.scale, byteFhdr.bias);
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
   * to the pixmap. Also initialize the pixmap.
   * Drawing will be done to the pixmap.
   */
  
  Drawable window = 0;
  if (dev == XDEV) {
    window = frame->x->drawable;
    frame->x->drawable = frame->x->pixmap;
    XFillRectangle(Glob->rdisplay, frame->x->drawable,
		   Glob->pixmap_gc, 0, 0,
		   frame->x->width, frame->x->height);
  }
  
  /*
   * load resources
   */
  
  load_resources(dev, frame);

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
		     clip_rectangle, 1);

  /*
   * plot image if requested
   */
  
  double offset_x = 0.0;
  double offset_y = 0.0;

  if (HaveCappiData) {
    
    // compute offset from display origin to data origin

    if (byteFhdr.proj_type != Mdvx::PROJ_LATLON) {
      Glob->proj.latlon2xy(byteFhdr.proj_origin_lat, byteFhdr.proj_origin_lon,
			   offset_x, offset_y);
    }
    
    if (Glob->plot_cappi_image == IMAGE_ON) {
      
      image_min_x = byteFhdr.grid_minx - byteFhdr.grid_dx / 2.0 + offset_x;
      image_max_x = image_min_x + byteFhdr.nx * byteFhdr.grid_dx;
      image_min_y = byteFhdr.grid_miny - byteFhdr.grid_dy / 2.0 + offset_y;
      image_max_y = image_min_y + byteFhdr.ny * byteFhdr.grid_dy;
      
      GDrawImageProgDetail(dev, frame, colors, image_min_x, image_min_y, 
			   image_max_x - image_min_x,
			   image_max_y - image_min_y,
			   byteFhdr.nx, byteFhdr.ny,
			   IMAGE_START_BOTLEFT,
			   byte_cappi,
			   (ui08) byteFhdr.missing_data_value, 1);

      Glob->cappi_plotted = true;
    
    } else if (Glob->plot_cappi_image == IMAGE_ON_CONTOURED) {

      MdvxContour cont;
      for (int ilevel = 0; ilevel < colors->nlevels; ilevel++) {
	cont.addVal(colors->level[ilevel].start);
      }
      cont.addVal(colors->level[colors->nlevels - 1].end);

      if (cont.computeTriangles(floatField) == 0) {
	draw_contour_triangles(dev, frame, colors, cont);
      }

      Glob->cappi_plotted = true;
    
    } // if (Glob->plot_cappi_image ...

  } // if (HaveCappiData ...
  
  /*
   * plot range rings if required
   */
  
  draw_range_rings(dev, frame);

  /*
   * plot in storm tracks
   */
  
  if (Glob->plot_tracks && Glob->track_data_available) {

    draw_titan(dev, frame, &track_time,
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
  
  if (HaveCappiData && Glob->plot_cappi_contours) {

    MdvxContour cont;

    if (Glob->fcontrol[Glob->field].contour_int > 0) {
      cont.setVals(Glob->fcontrol[Glob->field].contour_min,
		   Glob->fcontrol[Glob->field].contour_max,
		   Glob->fcontrol[Glob->field].contour_int);
    } else {
      for (int ilevel = 0; ilevel < colors->nlevels; ilevel++) {
	cont.addVal(colors->level[ilevel].start);
      }
      cont.addVal(colors->level[colors->nlevels - 1].end);
    }

    int iret;
    if (Glob->plot_cappi_contours == CONTOURS_ON_WITH_LABELS) {
      iret = cont.computeLines(floatField);
    } else {
      iret = cont.computeSegments(floatField);
    }

    if (iret == 0) {
      draw_contours(dev, frame,
		    Glob->pos_contour_gc,
		    Glob->zero_contour_gc,
		    Glob->neg_contour_gc,
		    &Glob->pos_contour_psgc,
		    &Glob->zero_contour_psgc,
		    &Glob->neg_contour_psgc,
		    cont);
    }
    
  } // if (HaveCappidata && Glob->plot_cappi_contours) {
  
  /*
   * draw in aircraft position
   */
  
  if (HaveCappiData) {
    if (Glob->mode == REALTIME) {
      time_t now = time(NULL);
      draw_ac_posn(dev, frame, now);
    } else {
      draw_ac_posn(dev, frame, CappiTime.unix_time);
    }
  }
  
  /*
   * draw in verification data
   */

  if (HaveCappiData) {
    draw_verify(dev, frame, &CappiTime);
  }

  /*
   * draw in symbolic product data
   */


  if (Glob->plot_products) {
    time_t FrameTime = Glob->time;
    time_t DataTime;
    if (HaveCappiData) {
      DataTime = CappiTime.unix_time;
    } else {
      DataTime = Glob->time;
    }
    if (Glob->mode == REALTIME) {
      Glob->_prodMgr->getDataRealtime(FrameTime, DataTime);
    } else {
      Glob->_prodMgr->getDataArchive(FrameTime, DataTime);
    }

    double diagonal = km_diagonal(frame);
    double iconScale = log10(400.0 / diagonal) + 1.0;
    if (iconScale < 0.05) {
      iconScale = 0.0;
    }
    
    Glob->_prodMgr->setIconScale(iconScale);

    Glob->_prodMgr->draw(Glob->cmap, &Glob->color_index,
			 dev, frame, Glob->proj);
  }
  
  /*
   * Annotation messages to the plot window
   */
  
  draw_annotation(dev, frame, n_tracks_plotted);

  /*
   * restore graphics state from clipping operation in postscript
   */
  
  if (dev == PSDEV) {
    PsGrestore(frame->psgc->file);
  }
  
  /*
   * draw cursor position information to window header
   */
  
  draw_cursor_info(dev, frame, floatFhdr, float_cappi, offset_x, offset_y);

  /*
   * draw in details in the margins
   */
  
  draw_margins(dev, frame, byteFhdr);

  /*
   * write in header and unit labels
   */

  draw_header(dev, frame, byteFhdr, track_time,
	      n_tracks_plotted, dbz_threshold);

  /*
   * set the plotframe drawable back to the window
   */
  
  if (dev ==  XDEV){

    frame->x->drawable = window;

    // copy backing pixmap to zoom pixmap and window
    
    XCopyArea(frame->x->display,
	      frame->x->pixmap,
	      zoom->pixmap,
	      Glob->copyarea_gc, 0, 0,
	      frame->x->width, frame->x->height, 0, 0);

    XCopyArea(frame->x->display,
	      frame->x->pixmap,
	      frame->x->drawable,
	      Glob->copyarea_gc, 0, 0,
	      frame->x->width, frame->x->height, 0, 0);

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

void expose_cappi_pixmap()

{

  gframe_t *frame;
  zoom_t *zoom;

  if (Glob->verbose) {
    fprintf(stderr, "** expose_cappi_pixmap **\n");
  }

  frame = Glob->cappi_plot_frame;
  zoom = Glob->zoom + Glob->zoom_level;

  zoom_calc(XDEV, frame);

  /*
   * copy the pixmap to the window and backing pixmap
   */

  XCopyArea(frame->x->display,
	    zoom->pixmap,
	    frame->x->drawable,
	    Glob->copyarea_gc, 0, 0,
	    frame->x->width, frame->x->height, 0, 0);
  
  XCopyArea(frame->x->display,
	    zoom->pixmap,
	    frame->x->pixmap,
	    Glob->copyarea_gc, 0, 0,
	    frame->x->width, frame->x->height, 0, 0);
  
  /*
   * draw in line showing location of vertical section
   * if one has been selected
   */

  if (Glob->vsection.active) {
    draw_vsection_line(frame);
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

  XFlush(Glob->rdisplay);
  
}

/******************
 * compute_range()
 */

static double compute_range(double x, double y)
     
{
  
  double dx, dy;
  dx = x - RingCenterX;
  dy = y - RingCenterY;
  return (sqrt(dx * dx + dy * dy));

}
/************************************************************************
 * cappi_pixel_value()
 *
 * gets the cappi pixel value at a given (x, y)
 *
 * returns 0 on success, -1 on failure
 */

static int cappi_pixel_value(const Mdvx::field_header_t &floatFhdr,
			     fl32 *float_cappi,
			     double x, double y,
			     double offset_x, double offset_y,
			     double *val_p,
			     char *units)

{

  if (!HaveCappiData) {
    cerr << "Don't have cappi data" << endl;
    return (-1);
  }
  
  int ix = (int) floor((x - offset_x - floatFhdr.grid_minx) / floatFhdr.grid_dx + 0.5);
  int iy = (int) floor((y - offset_y - floatFhdr.grid_miny) / floatFhdr.grid_dy + 0.5);

  if (ix < 0  || ix >= floatFhdr.nx ||
      iy < 0  || iy >= floatFhdr.ny) {

    /*
     * point outside current data area
     */

    return (-1);

  } else {

    int index = iy * floatFhdr.nx + ix;
    double value = float_cappi[index];
    *val_p = value;
    STRncopy(units, floatFhdr.units, MDV_UNITS_LEN);

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
			    int n_tracks_plotted)

{

  char label[LABEL_MAX];
  double textx, texty;
  
  /*
   * Messages to the plot window
   */
  
  textx = Min_x + Text_margin_x;
  texty = Max_y - Text_margin_y;
  
  if (!HaveCappiData && Glob->plot_cappi_image) {
    
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
	    
	case HAIL_CAT_ANNOTATION:
	  strcpy(label, "Annot: FOKR storm category");
	  break;
	    
	case HAIL_PROB_ANNOTATION:
	  strcpy(label, "Annot: Probability of hail");
	  break;
	    
	case HAIL_MASS_ALOFT_ANNOTATION:
	  strcpy(label, "Annot: Hail mass aloft (ktons)");
	  break;
	    
	case HAIL_VIHM_ANNOTATION:
	  strcpy(label, "Annot: Vertically integrated hail mass (kg/m2)");
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
			     const Mdvx::field_header_t &floatFhdr,
			     fl32 *float_cappi,
			     double offset_x, double offset_y)

{

  char cursor_str[LABEL_MAX * 5];
  char pixval_str[LABEL_MAX];
  char bearing_str[LABEL_MAX];
  char xy_str[LABEL_MAX];
  char latlon_str[LABEL_MAX];
  char units[MDV_UNITS_LEN];
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

    if (cappi_pixel_value(floatFhdr, float_cappi,
			  Glob->cursor_x, Glob->cursor_y,
			  offset_x, offset_y,
			  &pixval, units) == 0) {
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

    Glob->proj.xy2latlon(Glob->cursor_x, Glob->cursor_y, lat, lon);

    sprintf(latlon_str, "(%.4f%s, %.4f%s)",
	    fabs(lat), (lat < 0? "S" : "N"),
	    fabs(lon), (lon < 0? "W" : "E"));
	    
    /*
     * put them together
     */

    if (Glob->proj.getProjType() == Mdvx::PROJ_LATLON) {
      sprintf(cursor_str, "%s %s %s",
	      pixval_str, latlon_str, bearing_str);
    } else {
      sprintf(cursor_str, "%s %s %s %s",
	      pixval_str, xy_str, latlon_str, bearing_str);
    }
    
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
			const Mdvx::field_header_t &byteFhdr,
			date_time_t *track_time,
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
  
  if (HaveCappiData)
  {
    DateTime cappiTime(CappiTime.unix_time);
    if (Glob->localtime) {
      cappiTime.set(CappiTime.unix_time + Glob->timeOffsetSecs);
    }
    sprintf(hstring1, "%s %s ", cappiTime.dtime(),
            Glob->track_shmem->time_label);
  }
  
  if (HaveCappiData) {

    if (Glob->plot_composite ||
        byteFhdr.vlevel_type == Mdvx::VERT_TYPE_COMPOSITE) {
      
      sprintf(hstring2, "Composite");
      
    } else {
      
      if (byteFhdr.vlevel_type != Mdvx::VERT_TYPE_SURFACE &&
	  byteFhdr.vlevel_type != Mdvx::VERT_TYPE_CROSS_SEC &&
	  byteFhdr.vlevel_type != Mdvx::VERT_SATELLITE_IMAGE &&
	  byteFhdr.vlevel_type != Mdvx::VERT_FIELDS_VAR_ELEV &&
	  byteFhdr.vlevel_type != Mdvx:: VERT_VARIABLE_ELEV) {
	sprintf(hstring2, "%g(%s)", Glob->z_cappi,
		Mdvx::vertTypeZUnits(byteFhdr.vlevel_type));
      }
      
    } /* if (Glob->plot_composite */  

  } else {
    
    if (Glob->plot_composite) {

      sprintf(hstring2, "Composite");

    } else {
      
      sprintf(hstring2, "%g(km)", Glob->z_cappi);
      
    } /* if (Glob->plot_composite */  

  } /* if (HaveCappiData) */
  
  /*
   * track number and times
   */
  
  if (Glob->plot_tracks && Glob->track_data_available &&
      n_tracks_plotted > 0) {
      
    DateTime trackTime(track_time->unix_time);
    if (Glob->localtime) {
      trackTime.set(track_time->unix_time + Glob->timeOffsetSecs);
    }
    if (HaveCappiData &&
	CappiTime.unix_time != track_time->unix_time) {
      sprintf(hstring3,
	      " Tracks to %.2d:%.2d:%.2d",
	      trackTime.getHour(), trackTime.getMin(), trackTime.getSec());
    } else {
      sprintf(hstring3, " Tracks to %s", trackTime.dtime());
    }
      
    sprintf(hstring4, " Tz %g", dbz_threshold);
    
    /*
     * past track length if applicable
     */
    
    if (Glob->plot_past == PAST_LIMITED) {
      sprintf(hstring5, " Past %.1g(hr)",
	      Glob->past_plot_period / 3600.0);
    }
    
    /*
     * future track length if applicable
     */
    
    if (Glob->plot_future == FUTURE_LIMITED) {
      sprintf(hstring6, " Future %.1g(hr)",
	      Glob->future_plot_period / 3600.0);
    }
    
    /*
     * forecast track length if applicable
     */
    
    if (Glob->plot_forecast) {
      sprintf(hstring7, " Forecast %.1g(hr)",
	      (double) Glob->n_forecast_steps * Glob->forecast_interval /
	      3600.0);
    }
    
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
			 const Mdvx::field_header_t &byteFhdr)

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
		   uGetParamDouble(Glob->prog_name,
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
		   uGetParamDouble(Glob->prog_name,
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
      
      if (ilabel != 0 || RingCenter_on_plot == FALSE || Xaxismargin != 0) {
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
      
      if (ilabel != 0 || RingCenter_on_plot == FALSE || Yaxismargin != 0) {
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

  if (HaveCappiData) {
    GDrawString(dev, frame, Glob->text_gc, Glob->x_text_font,
		frame->psgc, XJ_LEFT, YJ_ABOVE,
		frame->w_xmin + Text_margin_x,
		frame->w_ymin + Text_margin_y,
		Mdvx::projType2XUnits(byteFhdr.proj_type));
    GDrawString(dev, frame, Glob->text_gc, Glob->x_text_font,
		frame->psgc, XJ_RIGHT, YJ_BELOW,
		frame->w_xmax - Text_margin_x,
		frame->w_ymax - Text_margin_y,
		Mdvx::projType2YUnits(byteFhdr.proj_type));
  }
    
}

/****************************************
 * draw_range_rings()
 *
 * Draw range rings and azimuth lines
 */

static void draw_range_rings(int dev,
			     gframe_t *frame)

{

  if (suppressRings) {
    if (Glob->debug) {
      cerr << "Suppressing rings because sensor location is at 0,0" << endl;
    }
    return;
  }

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
  
  if (HaveCappiData && Glob->plot_rings) {
    
    /*
     * set ring label font size and line width
     */
    
    if (dev == PSDEV) {
      PsSetFont(frame->psgc->file, frame->psgc->fontname,
		Glob->ps_ringlabel_fontsize);
      PsSetLineWidth(frame->psgc->file,
		     uGetParamDouble(Glob->prog_name,
				     "ps_ring_width", PS_RING_WIDTH));
    }
    
    /*
     * compute max range plotted by considering each corner in turn
     */
    
    maxrange = compute_range(Min_x, Min_y);
    minrange = compute_range(Min_x, Min_y);
    
    cornerrange = compute_range(Min_x, Max_y);
    if (cornerrange < minrange) minrange = cornerrange;
    if (cornerrange > maxrange) maxrange = cornerrange;
    
    cornerrange = compute_range(Max_x, Min_y);
    if (cornerrange < minrange) minrange = cornerrange;
    if (cornerrange > maxrange) maxrange = cornerrange;
    
    cornerrange = compute_range(Max_x, Max_y);
    if (cornerrange < minrange) minrange = cornerrange;
    if (cornerrange > maxrange) maxrange = cornerrange;
    
    if (Min_x <= RingCenterX &&
	Max_x >= RingCenterX) {
      
      minrange = MIN (fabs(RingCenterY - Min_y),
		      fabs(RingCenterY - Max_y));
      
    } else if (Min_y <= RingCenterY &&
	       Max_y >= RingCenterY) {
      
      minrange = MIN (fabs(RingCenterX - Min_x),
		      fabs(RingCenterX - Max_x));
      
    }
    
    if (minrange > 500) {
      if (Glob->debug) {
        cerr << "Suppressing rings because minrange exceeds 500 km" << endl;
      }
      return;
    }
    
    if (RingCenter_on_plot) {
      minrange = 0;
    }

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
     * compute the angles from the ringCenter to the corners
     */
    
    if (RingCenter_on_plot) {
      
      min_angle = 0.0;
      max_angle = 360.0;
      
    } else {
      
      min_angle = LARGE_DOUBLE;
      max_angle = -LARGE_DOUBLE;
      
      /*
       * compute the angles from the ringCenter to the corner points
       */
      
      corner_angle[0] = atan2((double)(Min_y -RingCenterY),
			      (double)(Min_x -RingCenterX));
      corner_angle[1] = atan2((double)(Min_y -RingCenterY),
			      (double)(Max_x -RingCenterX));
      corner_angle[2] = atan2((double)(Max_y -RingCenterY),
			      (double)(Min_x -RingCenterX));
      corner_angle[3] = atan2((double)(Max_y -RingCenterY),
			      (double)(Max_x -RingCenterX));
      
      for (i = 0; i < 4; i++)
	corner_angle[i] *= (double) 180.0 / M_PI;
      
      /*
       * if the plot rectangle straddles the 180 degree line,
       * add 360 to the negative angles
       */
      
      if (Max_x < RingCenterX &&
	  Max_y > RingCenterY &&
	  Min_y < RingCenterY) {
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
      
    } /* if (RingCenter_on_plot) */
    
    /*
     * draw in azimuth lines
     */
    
    for (i = 0; i < N_AZIMUTH_LINES; i++) {
      
      xx2 = maxrange * cos(Azimuth[i] * DEG_TO_RAD);
      yy2 = maxrange * sin(Azimuth[i] * DEG_TO_RAD);
      xx1 = - xx2;
      yy1 = - yy2;
      
      GDrawLine(dev, frame, Glob->ring_gc, frame->psgc,
		xx1 + RingCenterX,
		yy1 + RingCenterY,
		xx2 + RingCenterX,
		yy2 + RingCenterY);
      
    } /* i */
    
    /*
     * draw range rings
     */
    
    for (i  = startring; i <= endring; i++) {
      
      radius = (i + 1) * ringint;
      
      GDrawArc(dev, frame, Glob->ring_gc, frame->psgc,
	       RingCenterX, RingCenterY,
	       radius, radius,
	       min_angle, max_angle, 0.0, 360);
      
      /*
       * plot in range numbers
       */
      
      sprintf(label, "%d", (int) floor((double)(radius + 0.5)));
      
      if (RingCenter_on_plot) {
	
	for (j = 1; j <= 7; j += 2) {
	  
	  angle = (double) j * M_PI / (double) 4.0;
	  ringlabelx = RingCenterX + radius * cos(angle);
	  ringlabely = RingCenterY + radius * sin(angle);
	  GDrawImageString(dev, frame, Glob->ring_gc,
			   Glob->x_ringlabel_font,
			   frame->psgc, XJ_CENTER, YJ_CENTER,
			   ringlabelx, ringlabely, label);
	  
	}
	
      } else {
	
	angle = (double) (((min_angle + max_angle) / 2.0) * 
			  M_PI / 180.0);
	ringlabelx = RingCenterX + radius * cos(angle);
	ringlabely = RingCenterY + radius * sin(angle);
	GDrawImageString(dev, frame, Glob->ring_gc, Glob->x_ringlabel_font,
			 frame->psgc, XJ_CENTER, YJ_CENTER,
			 ringlabelx, ringlabely, label);
	
      } /* if (RingCenter_on_plot) */
      
    } /* i */
    
  } /* if (Glob->plot_rings) */
  
}

/******************
 * load_resources()
 *
 * Load up plotting resource values - X or PS
 */

static void load_resources(int dev, gframe_t *frame)

{

  int direction, ascent, descent;
  double unitscale;
  XCharStruct overall;

  switch (dev) {
    
  case XDEV:
    
    Xaxismargin = uGetParamLong(Glob->prog_name,
			      "x_xaxismargin", X_XAXISMARGIN);

    Yaxismargin = uGetParamLong(Glob->prog_name,
			      "x_yaxismargin", X_YAXISMARGIN);

    Topmargin = uGetParamLong(Glob->prog_name,
			    "x_topmargin", X_TOPMARGIN);

    Xaxisendmargin = uGetParamDouble(Glob->prog_name,
				   "x_xaxisendmargin",
				   X_XAXISENDMARGIN) / frame->x->xscale;

    Yaxisendmargin = uGetParamDouble(Glob->prog_name,
				   "x_yaxisendmargin",
				   X_YAXISENDMARGIN) / frame->x->yscale;
    
    Tick_clearance = uGetParamDouble(Glob->prog_name,
				   "x_tick_clearance",
				   X_TICK_CLEARANCE);

    X_ticklength = uGetParamDouble(Glob->prog_name,
				 "x_ticklength",
				 X_TICKLENGTH);

    Xticklength = X_ticklength / frame->x->yscale;
    Yticklength = X_ticklength / frame->x->xscale;
    Xtickmargin = Tick_clearance / frame->x->xscale;
    Ytickmargin = Tick_clearance / frame->x->yscale;
    
    Xaxis_margin_d = Xaxismargin / frame->x->yscale;

    Text_margin_x =
      uGetParamDouble(Glob->prog_name,
		    "x_text_margin",
		    X_TEXT_MARGIN) / frame->x->xscale;
    
    Text_margin_y =
      uGetParamDouble(Glob->prog_name,
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

    unitscale = uGetParamDouble(Glob->prog_name,
			      "ps_unitscale",
			      PS_UNITSCALE);
    
    Xaxismargin = (si32) (uGetParamDouble(Glob->prog_name,
					"ps_xaxismargin",
					PS_XAXISMARGIN) * unitscale);

    Yaxismargin = (si32) (uGetParamDouble(Glob->prog_name,
					"ps_yaxismargin",
					PS_YAXISMARGIN) * unitscale);
    Topmargin = (si32) (uGetParamDouble(Glob->prog_name,
				      "ps_topmargin",
				      PS_TOPMARGIN) * unitscale);
    
    Xaxisendmargin =
      uGetParamDouble(Glob->prog_name, "ps_xaxisendmargin",
		    PS_XAXISENDMARGIN) * unitscale / frame->ps->xscale;
    Yaxisendmargin =
      uGetParamDouble(Glob->prog_name, "ps_yaxisendmargin",
		    PS_YAXISENDMARGIN) * unitscale / frame->ps->yscale;
    
    Tick_clearance =
      uGetParamDouble(Glob->prog_name, "ps_tick_clearance",
		    PS_TICK_CLEARANCE) * unitscale;
    
    Ps_ticklength = uGetParamDouble(Glob->prog_name,
				  "ps_ticklength",
				  PS_TICKLENGTH) * unitscale;
    
    Xticklength = Ps_ticklength / frame->ps->yscale;
    Yticklength = Ps_ticklength / frame->ps->xscale;
    Xtickmargin = Tick_clearance / frame->ps->xscale;
    Ytickmargin = Tick_clearance / frame->ps->yscale;
    
    Xaxis_margin_d = (double) Xaxismargin * unitscale / frame->ps->yscale;

    Text_margin_x =
      uGetParamDouble(Glob->prog_name, "ps_text_margin",
		    PS_TEXT_MARGIN) * unitscale / frame->ps->xscale;
    Text_margin_y =
      uGetParamDouble(Glob->prog_name, "ps_text_margin",
		    PS_TEXT_MARGIN) * unitscale / frame->ps->yscale;
    
    Text_line_spacing = (Glob->ps_text_fontsize * 1.4) / frame->ps->yscale;

    break;
    
  }
  
  Approx_nticks = uGetParamLong(Glob->prog_name,
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
   * deduce if ringCenter is on plot
   */
  
  if (HaveCappiData) {
    if (Min_x <= RingCenterX && Max_x >= RingCenterX &&
	Min_y <= RingCenterY && Max_y >= RingCenterY) {
      RingCenter_on_plot = TRUE;
    } else {
      RingCenter_on_plot = FALSE;
    }
  } else {
    RingCenter_on_plot = FALSE;
  }
  
}

/*********************************************************************
 * read_cappi()
 *
 * Reads a plane using Mdvx object.
 *
 * Side effect - fills CappiMdvx object
 *               sets CappiTime struct
 *               sets Glob->field_units
 *               sets Glob->cappi_z
 *
 * Returns 0 on success, -1 on failure.
 */

static int read_cappi(Mdvx::master_header_t &mhdr,
		      MdvxField &floatField,
		      MdvxField &byteField)

{
  
  if (Glob->verbose) {
    fprintf(stderr, "** read_cappi **\n");
  }

  /*
   * return error if nfields is not positive
   */

  if (Glob->nfields < 1) {
    return (-1);
  }

  /*
   * set field control pointer
   */
  
  field_control_t &fcontrol = Glob->fcontrol[Glob->field];

  // set up the mdvx object for the read

  DsMdvx mdvx;
  mdvx.clearRead();

  if (Glob->mode == ARCHIVE) {
    mdvx.setReadTime(Mdvx::READ_FIRST_BEFORE,
		     fcontrol.url,
		     fcontrol.time_window,
                     Glob->time + Glob->field_data_time_offset_secs);
  } else {
    mdvx.setReadTime(Mdvx::READ_LAST,
                     fcontrol.url,
                     fcontrol.time_window);
  }

  // first read in all of the headers to get the vlevel info

  if (Glob->debug) {
    mdvx.printReadRequest(cerr);
  }
  if (mdvx.readAllHeaders()) {
    if (Glob->debug) {
      cerr << mdvx.getErrStr() << endl;
    }
    return (-1);
  }

  // get vlevel info for the requested field

  if (fcontrol.field_num < 0) {
    Glob->fhdr = mdvx.getFieldHeaderFile(0);
    Glob->vhdr = mdvx.getVlevelHeaderFile(0);
    for (int i = 0; i < mdvx.getMasterHeaderFile().n_fields; i++) {
      if (!strcmp(mdvx.getFieldHeaderFile(i).field_name,
		  fcontrol.field_name)) {
	Glob->fhdr = mdvx.getFieldHeaderFile(i);
	Glob->vhdr = mdvx.getVlevelHeaderFile(i);
      }
    }
  } else {
    Glob->fhdr = mdvx.getFieldHeaderFile(fcontrol.field_num);
    Glob->vhdr = mdvx.getVlevelHeaderFile(fcontrol.field_num);
  }
  if (Glob->debug) {
    for (int i = 0; i < Glob->fhdr.nz; i++) {
      cerr << "vlevel[" << i << "]: " << Glob->vhdr.level[i] << endl;
    }
  }

  // then set other info

  if (fcontrol.field_num < 0) {
    mdvx.addReadField(fcontrol.field_name);
  } else {
    mdvx.addReadField(fcontrol.field_num);
  }
  
  if (Glob->plot_composite) {
    mdvx.setReadComposite();
  } else {
    mdvx.setReadVlevelLimits(Glob->z_requested, Glob->z_requested);
  }

  zoom_t *zoom = Glob->zoom + Glob->zoom_level;
  double min_lat, min_lon, max_lat, max_lon;
  Glob->proj.xy2latlon(zoom->min_x, zoom->min_y,
		       min_lat, min_lon);
  Glob->proj.xy2latlon(zoom->max_x, zoom->max_y,
		       max_lat, max_lon);

  mdvx.setReadHorizLimits(min_lat, min_lon, max_lat, max_lon);
  
  mdvx.setReadEncodingType(Mdvx::ENCODING_ASIS);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_ASIS);

  if (Glob->debug) {
    mdvx.printReadRequest(cerr);
  }
  
  // do the read
  
  if (mdvx.readVolume()) {
    if (Glob->debug) {
      cerr << mdvx.getErrStr() << endl;
    }
    return (-1);
  }

  // set the local time offset from UTC

  setLocaltimeOffsetSecs();

  // copy to objects

  mhdr = mdvx.getMasterHeader();

  floatField = *mdvx.getFieldByNum(0);
  floatField.convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);

  byteField = *mdvx.getFieldByNum(0);
  byteField.convertType(Mdvx::ENCODING_INT8, Mdvx::COMPRESSION_NONE);

  if (Glob->field_name_from_params) {
    cappiFieldName = fcontrol.description;
  } else {
    cappiFieldName = floatField.getFieldName();
  }
  
  // set time
  
  CappiTime.unix_time = mhdr.time_centroid;
  uconvert_from_utime(&CappiTime);
  
  // set cappi height and plane num
  
  Glob->z_cappi = floatField.getVlevelHeader().level[0];
  if (!Glob->plot_composite) {
    Glob->z_requested = Glob->z_cappi;
  }
  double minDiff = 1.0e10;
  for (int i = 0; i < Glob->fhdr.nz; i++) {
    double diff = fabs(Glob->z_cappi - Glob->vhdr.level[i]);
    if (diff < minDiff) {
      Glob->plane_num = i;
      minDiff = diff;
    }
  }
  if (Glob->debug) {
    cerr << "plane num: " << Glob->plane_num << endl;
  }

  // set field units
  
  Glob->field_units = floatField.getUnits();

  if (!Glob->use_time_hist) {
    Glob->track_shmem->time = CappiTime.unix_time;
  }

  return 0;

}

static void draw_vsection_line(gframe_t *frame)

{

  const vector<Mdvx::vsect_waypt_t> &waypts = getVsectWayPts();
  size_t nWayPts = waypts.size();

//   MemBuf lineBuf;
//   GPoint *gpts = (GPoint *) lineBuf.reserve(waypts.size() * sizeof(GPoint));
  
//   for (size_t i = 0; i < waypts.size(); i++) {
//     Glob->proj.latlon2xy(waypts[i].lat, waypts[i].lon,
// 			 gpts[i].x, gpts[i].y);
//   }

//   GDrawLines(XDEV, frame, Glob->vsection_pos_gc, frame->psgc,
// 	     gpts, waypts.size(),
// 	     CoordModeOrigin);
  
  const vector<Mdvx::vsect_samplept_t> &samplepts = getVsectSamplePts();
  size_t nSamplePts = samplepts.size();

  if (nSamplePts < 1) {
    return;
  }

  MemBuf lineBuf;
  GPoint *gpts = (GPoint *)
    lineBuf.reserve((nSamplePts + 2) * sizeof(GPoint));
  
  Glob->proj.latlon2xy(waypts[0].lat, waypts[0].lon,
		       gpts[0].x, gpts[0].y);

  for (size_t i = 0; i < nSamplePts; i++) {
    Glob->proj.latlon2xy(samplepts[i].lat, samplepts[i].lon,
			 gpts[i+1].x, gpts[i+1].y);
  }

  Glob->proj.latlon2xy(waypts[nWayPts-1].lat, waypts[nWayPts-1].lon,
		       gpts[nSamplePts+1].x, gpts[nSamplePts+1].y);

  GDrawLines(XDEV, frame, Glob->vsection_pos_gc, frame->psgc,
	     gpts, nSamplePts + 2,
	     CoordModeOrigin);
  
//   for (size_t i = 0; i < samplepts.size(); i++) {
//     char str[16];
//     sprintf(str, "%d", samplepts[i].segNum);
//     double x, y;
//     Glob->proj.latlon2xy(samplepts[i].lat, samplepts[i].lon, x, y);
//     GDrawString(XDEV, frame, Glob->ticklabel_gc, Glob->x_ticklabel_font,
// 		frame->psgc, XJ_CENTER, YJ_CENTER,
// 	       x, y, str);
//   }


}

////////////////////////////////
// get km across display diagonal

static double km_diagonal(gframe_t *frame)

{

  double diagonal;

  if (Glob->proj.getProjType() == Mdvx::PROJ_LATLON) {

    double dy = (frame->w_ymax - frame->w_ymin) * KM_PER_DEG_AT_EQ;
    double meanLat = (frame->w_ymax + frame->w_ymin) / 2.0;
    double latRad = meanLat * RAD_PER_DEG;
    double dx =
      (frame->w_xmax - frame->w_xmin) * KM_PER_DEG_AT_EQ * cos(latRad);
    diagonal = sqrt(dx * dx + dy * dy);
    
  } else {
    
    double dx = frame->w_xmax - frame->w_xmin;
    double dy = frame->w_ymax - frame->w_ymin;
    diagonal = sqrt(dx * dx + dy * dy);

  }

  return diagonal;

}

////////////////////////////////
// set time offset 

static void setLocaltimeOffsetSecs()

{
  
  Glob->timeOffsetSecs = 0;
  if (Glob->localtime) {
    time_t now = time(NULL);
    struct tm *gmtStruct = gmtime(&now);
    DateTime gmt(gmtStruct->tm_year,
                 gmtStruct->tm_mon,
                 gmtStruct->tm_mday,
                 gmtStruct->tm_hour,
                 gmtStruct->tm_min,
                 gmtStruct->tm_sec);
    struct tm *localStruct = localtime(&now);
    DateTime local(localStruct->tm_year,
                   localStruct->tm_mon, 
                   localStruct->tm_mday,
                   localStruct->tm_hour,
                   localStruct->tm_min,
                   localStruct->tm_sec);
    Glob->timeOffsetSecs = local.utime() - gmt.utime();
  }
  Glob->track_shmem->timeOffsetSecs = Glob->timeOffsetSecs;

}
