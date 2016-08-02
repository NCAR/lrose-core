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
 * set_gcs.c: set graphic contexts, colors etc
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1990   
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "Rview.hh"
using namespace std;

static void setup_color_gc(GC *gc,
			   Colormap cmap,
			   const char *res_color_name,
			   const char *def_color_name);

static void setup_dim_gc(GC *gc,
			 Colormap cmap,
			 const char *res_color_name,
			 const char *def_color_name);

static void setup_color_and_dim_gc(GC *gc,
				   GC *dim_gc,
				   Colormap cmap,
				   const char *res_color_name,
				   const char *def_color_name);

static void check_for_getgc_error(GC gc,
				  const char *colorname);

static void check_for_getxcolor_error(XColor *x_color,
				      const char *colorname);

static void print_x_ids(Colormap cmap);

void set_xgcs()

{

  XColor *color;
  Colormap cmap;
  char *colstr;
  ui32 width;
  si32 i;
  unsigned int xor_color_num;

  if (Glob->debug) {
    fprintf(stderr, "** set_xgcs **\n");
  }

  /*
   * get default color map
   */

  cmap = Glob->cmap;

  /*
   * foreground and background colors
   */

  color = xGetXColor(Glob->rdisplay, cmap, &Glob->color_index,
		     Glob->foregroundstr);
  check_for_getxcolor_error(color, Glob->foregroundstr);
  Glob->foreground = color->pixel;

  color = xGetXColor(Glob->rdisplay, cmap, &Glob->color_index,
		     Glob->backgroundstr);
  check_for_getxcolor_error(color, Glob->backgroundstr);
  Glob->background = color->pixel;

  /*
   * title frame gcs
   */

  setup_color_gc(&Glob->cappi_title_frame->x->gc, cmap,
		 "x_titletext_color", X_TITLETEXT_COLOR);

  XSetFont(Glob->rdisplay, Glob->cappi_title_frame->x->gc,
	   Glob->x_title_font->fid);

  XCopyGC(Glob->rdisplay, Glob->cappi_title_frame->x->gc,
	  GCForeground | GCBackground | GCFont,
	  Glob->vsection_title_frame->x->gc);

  XCopyGC(Glob->rdisplay, Glob->cappi_title_frame->x->gc,
	  GCForeground | GCBackground | GCFont,
	  Glob->help_title_frame->x->gc);

  /*
   * button frame gc's
   */

  setup_color_gc(&Glob->cappi_button_frame[0]->x->gc, cmap,
		 "x_buttontext_color", X_BUTTONTEXT_COLOR);

  XSetFont(Glob->rdisplay, Glob->cappi_button_frame[0]->x->gc,
	   Glob->x_button_font->fid);
  
  for (i = 1; i < N_CAPPI_BUTTONS; i++)
    XCopyGC(Glob->rdisplay, Glob->cappi_button_frame[0]->x->gc,
	    GCForeground | GCBackground | GCFont,
	    Glob->cappi_button_frame[i]->x->gc);
  
  for (i = 0; i < N_VSECTION_BUTTONS; i++)
    XCopyGC(Glob->rdisplay, Glob->cappi_button_frame[0]->x->gc,
	    GCForeground | GCBackground | GCFont,
	    Glob->vsection_button_frame[i]->x->gc);

  for (i = 0; i < N_HELP_BUTTONS; i++)
    XCopyGC(Glob->rdisplay, Glob->cappi_button_frame[0]->x->gc,
	    GCForeground | GCBackground | GCFont,
	    Glob->help_button_frame[i]->x->gc);

  /*
   * scale frame gc
   */

  setup_color_gc(&Glob->main_scale_frame->x->gc, cmap,
			 "x_scaletext_color", X_SCALETEXT_COLOR);

  XSetFont(Glob->rdisplay, Glob->main_scale_frame->x->gc,
	   Glob->x_scale_font->fid);

  /*
   * tick mark gc
   */

  setup_color_gc(&Glob->tick_gc, cmap,
		 "x_tick_color", X_TICK_COLOR);

  width = (ui32)
    uGetParamLong(Glob->prog_name, "x_tick_width", X_TICK_WIDTH);

  XSetLineAttributes(Glob->rdisplay, Glob->tick_gc, width,
		     LineSolid, CapButt, JoinMiter);

  /*
   * vert section position line GC and zoom box GC
   */

  setup_color_gc(&Glob->vsection_pos_gc, cmap,
		 "x_vsection_pos_color", X_VSECTION_POS_COLOR);

  width = uGetParamLong(Glob->prog_name, "x_vsection_pos_width",
		X_VSECTION_POS_WIDTH);

  XSetLineAttributes(Glob->rdisplay, Glob->vsection_pos_gc, width,
		     LineOnOffDash, CapButt, JoinMiter);

  setup_color_gc(&Glob->zoom_box_gc, cmap,
		 "x_zoom_box_color", X_ZOOM_BOX_COLOR);

  width = uGetParamLong(Glob->prog_name, "x_zoom_box_width",
		X_ZOOM_BOX_WIDTH);

  XSetLineAttributes(Glob->rdisplay, Glob->zoom_box_gc, width,
		     LineOnOffDash, CapButt, JoinMiter);

  /*
   * range ring gc
   */

  setup_color_gc(&Glob->ring_gc, cmap,
		 "x_ring_color", X_RING_COLOR);

  width = (ui32)
    uGetParamLong(Glob->prog_name, "x_ring_width",
		X_RING_WIDTH);

  XSetLineAttributes(Glob->rdisplay, Glob->ring_gc, width,
		     LineSolid, CapButt, JoinMiter);

  XSetFont(Glob->rdisplay, Glob->ring_gc, Glob->x_ringlabel_font->fid);

  /*
   * tick label gc
   */

  setup_color_gc(&Glob->ticklabel_gc, cmap,
		 "x_ticklabel_color", X_TICKLABEL_COLOR);

  XSetFont(Glob->rdisplay, Glob->ticklabel_gc, Glob->x_ticklabel_font->fid);

  /*
   * contour label gc
   */

  setup_color_gc(&Glob->contourlabel_gc, cmap,
		 "x_contourlabel_color", X_CONTOURLABEL_COLOR);

  XSetFont(Glob->rdisplay, Glob->contourlabel_gc, Glob->x_contourlabel_font->fid);

  /*
   * track number gc
   */

  setup_color_gc(&Glob->track_annotation_gc, cmap,
		 "x_track_annotation_color", X_TRACK_ANNOTATION_COLOR);

  XSetFont(Glob->rdisplay, Glob->track_annotation_gc,
	   Glob->x_track_annotation_font->fid);

  setup_color_gc(&Glob->track_case_gc, cmap,
		 "x_track_case_color", X_TRACK_CASE_COLOR);

  XSetFont(Glob->rdisplay, Glob->track_annotation_gc,
	   Glob->x_track_annotation_font->fid);

  /*
   * text gc
   */

  setup_color_gc(&Glob->text_gc, cmap,
		 "x_text_color", X_TEXT_COLOR);

  XSetFont(Glob->rdisplay, Glob->text_gc, Glob->x_text_font->fid);

  /*
   * positive contour gc
   */

  setup_color_gc(&Glob->pos_contour_gc, cmap,
		 "x_pos_contour_color", X_POS_CONTOUR_COLOR);

  XSetLineAttributes(Glob->rdisplay, Glob->pos_contour_gc, 0,
		     LineSolid, CapButt, JoinMiter);

  /*
   * zero contour gc
   */

  setup_color_gc(&Glob->zero_contour_gc, cmap,
		 "x_zero_contour_color", X_ZERO_CONTOUR_COLOR);

  XSetLineAttributes(Glob->rdisplay, Glob->zero_contour_gc, 0,
		     LineSolid, CapButt, JoinMiter);

  /*
   * negative contour gc
   */

  setup_color_gc(&Glob->neg_contour_gc, cmap,
		 "x_neg_contour_color", X_NEG_CONTOUR_COLOR);

  XSetLineAttributes(Glob->rdisplay, Glob->neg_contour_gc, 0,
		     LineSolid, CapButt, JoinMiter);

  /*
   * crosshair gc
   */

  setup_color_gc(&Glob->crosshair_gc, cmap,
		 "x_crosshair_color", X_CROSSHAIR_COLOR);

  width = (ui32)
    uGetParamLong(Glob->prog_name, "x_crosshair_width",
		X_CROSSHAIR_WIDTH);

  XSetLineAttributes(Glob->rdisplay, Glob->crosshair_gc, width,
		     LineSolid, CapButt, JoinMiter);

  /*
   * storm shape gc's
   */

  setup_color_and_dim_gc(&Glob->past_storm_gc[0],
			 &Glob->past_storm_dim_gc[0],
			 cmap,
			 "x_past_storm_color",
			 X_PAST_STORM_COLOR);

  setup_color_and_dim_gc(&Glob->past_storm_gc[1],
			 &Glob->past_storm_dim_gc[1],
			 cmap,
			 "x_past_storm_color_2",
			 X_PAST_STORM_COLOR_2);

  setup_color_and_dim_gc(&Glob->future_storm_gc[0],
			 &Glob->future_storm_dim_gc[0],
			 cmap,
			 "x_future_storm_color",
			 X_FUTURE_STORM_COLOR);

  setup_color_and_dim_gc(&Glob->future_storm_gc[1],
			 &Glob->future_storm_dim_gc[1],
			 cmap,
			 "x_future_storm_color_2",
			 X_FUTURE_STORM_COLOR_2);

  setup_color_and_dim_gc(&Glob->forecast_storm_gc[0],
			 &Glob->forecast_storm_dim_gc[0],
			 cmap,
			 "x_forecast_storm_color",
			 X_FORECAST_STORM_COLOR);

  setup_color_and_dim_gc(&Glob->forecast_storm_gc[1],
			 &Glob->forecast_storm_dim_gc[1],
			 cmap,
			 "x_forecast_storm_color_2",
			 X_FORECAST_STORM_COLOR_2);

  setup_color_and_dim_gc(&Glob->current_storm_gc,
			 &Glob->current_storm_dim_gc,
			 cmap,
			 "x_current_storm_color",
			 X_CURRENT_STORM_COLOR);

  width = (ui32)
    uGetParamLong(Glob->prog_name,
		"x_current_storm_line_width",
		X_CURRENT_STORM_LINE_WIDTH);

  XSetLineAttributes(Glob->rdisplay, Glob->current_storm_gc,
		     width * Glob->width_mult_selected,
		     LineSolid, CapButt, JoinRound);

  XSetLineAttributes(Glob->rdisplay, Glob->current_storm_dim_gc, width,
		     LineSolid, CapButt, JoinRound);

  width = (ui32)
    uGetParamLong(Glob->prog_name,
		"x_past_storm_line_width",
		X_PAST_STORM_LINE_WIDTH);

  XSetLineAttributes(Glob->rdisplay, Glob->past_storm_gc[0],
		     width * Glob->width_mult_selected,
		     LineSolid, CapButt, JoinRound);
  
  XSetLineAttributes(Glob->rdisplay, Glob->past_storm_dim_gc[0], width,
		     LineSolid, CapButt, JoinRound);

  XSetLineAttributes(Glob->rdisplay, Glob->past_storm_gc[1],
		     width * Glob->width_mult_selected,
		     LineSolid, CapButt, JoinRound);

  XSetLineAttributes(Glob->rdisplay, Glob->past_storm_dim_gc[1], width,
		     LineSolid, CapButt, JoinRound);

  width = (ui32)
    uGetParamLong(Glob->prog_name,
		"x_future_storm_line_width",
		X_FUTURE_STORM_LINE_WIDTH);

  XSetLineAttributes(Glob->rdisplay, Glob->future_storm_gc[0],
		     width * Glob->width_mult_selected,
		     LineSolid, CapButt, JoinRound);

  XSetLineAttributes(Glob->rdisplay, Glob->future_storm_dim_gc[0], width,
		     LineSolid, CapButt, JoinRound);

  XSetLineAttributes(Glob->rdisplay, Glob->future_storm_gc[1],
		     width * Glob->width_mult_selected,
		     LineSolid, CapButt, JoinRound);

  XSetLineAttributes(Glob->rdisplay, Glob->future_storm_dim_gc[1], width,
		     LineSolid, CapButt, JoinRound);

  width = (ui32)
    uGetParamLong(Glob->prog_name,
		"x_forecast_storm_line_width",
		X_FORECAST_STORM_LINE_WIDTH);

  XSetLineAttributes(Glob->rdisplay, Glob->forecast_storm_gc[0], width,
		     LineSolid, CapButt, JoinRound);

  XSetLineAttributes(Glob->rdisplay, Glob->forecast_storm_dim_gc[0], width,
		     LineSolid, CapButt, JoinRound);

  XSetLineAttributes(Glob->rdisplay, Glob->forecast_storm_gc[1], width,
		     LineSolid, CapButt, JoinRound);

  XSetLineAttributes(Glob->rdisplay, Glob->forecast_storm_dim_gc[1], width,
		     LineSolid, CapButt, JoinRound);

  /*
   * storm vector gcs
   */

  setup_color_and_dim_gc(&Glob->past_vector_gc,
			 &Glob->past_vector_dim_gc,
			 cmap,
			 "x_past_vector_color",
			 X_PAST_VECTOR_COLOR);

  setup_color_and_dim_gc(&Glob->future_vector_gc,
			 &Glob->future_vector_dim_gc,
			 cmap,
			 "x_future_vector_color",
			 X_FUTURE_VECTOR_COLOR);

  setup_color_and_dim_gc(&Glob->forecast_vector_gc,
			 &Glob->forecast_vector_dim_gc,
			 cmap,
			 "x_forecast_vector_color",
			 X_FORECAST_VECTOR_COLOR);

  width = (ui32)
    uGetParamLong(Glob->prog_name,
		"x_storm_vector_width", X_STORM_VECTOR_WIDTH);
  
  XSetLineAttributes(Glob->rdisplay, Glob->past_vector_gc, width,
		     LineSolid, CapButt, JoinMiter);

  XSetLineAttributes(Glob->rdisplay, Glob->past_vector_dim_gc, width,
		     LineSolid, CapButt, JoinMiter);

  XSetLineAttributes(Glob->rdisplay, Glob->future_vector_gc, width,
		     LineSolid, CapButt, JoinMiter);

  XSetLineAttributes(Glob->rdisplay, Glob->future_vector_dim_gc, width,
		     LineSolid, CapButt, JoinMiter);

  XSetLineAttributes(Glob->rdisplay, Glob->forecast_vector_gc, width,
		     LineSolid, CapButt, JoinMiter);

  XSetLineAttributes(Glob->rdisplay, Glob->forecast_vector_dim_gc, width,
		     LineSolid, CapButt, JoinMiter);

  /*
   * window border color
   */

  colstr = uGetParamString(Glob->prog_name, "x_border_color",
			 X_BORDER_COLOR);
  color = xGetXColor(Glob->rdisplay, cmap, &Glob->color_index, colstr);
  check_for_getxcolor_error(color, colstr);
  Glob->border_color = color->pixel;
  Glob->border_gc = XCreateGC(Glob->rdisplay,
			      DefaultRootWindow(Glob->rdisplay), 0, 0);
  XSetBackground(Glob->rdisplay, Glob->border_gc, Glob->border_color);
  XSetForeground(Glob->rdisplay, Glob->border_gc, Glob->border_color);

  /*
   * plot frame default gc
   */

  XSetBackground(Glob->rdisplay, Glob->cappi_plot_frame->x->gc,
		 Glob->background);
  XSetForeground(Glob->rdisplay, Glob->cappi_plot_frame->x->gc,
		 Glob->border_color);
  XSetBackground(Glob->rdisplay, Glob->vsection_plot_frame->x->gc,
		 Glob->background);
  XSetForeground(Glob->rdisplay, Glob->vsection_plot_frame->x->gc,
		 Glob->border_color);
  XSetFont(Glob->rdisplay, Glob->cappi_plot_frame->x->gc,
	   Glob->x_text_font->fid);
  XSetFont(Glob->rdisplay, Glob->vsection_plot_frame->x->gc,
	   Glob->x_text_font->fid);

  /*
   * GC for clearing pixmaps
   */

  Glob->pixmap_gc = XCreateGC(Glob->rdisplay,
			      DefaultRootWindow(Glob->rdisplay), 0, 0);
  XSetBackground(Glob->rdisplay, Glob->pixmap_gc, Glob->background);
  XSetForeground(Glob->rdisplay, Glob->pixmap_gc, Glob->background);
  
  /*
   * set GC for copying areas from pixmap to plot window
   */

  Glob->copyarea_gc = XCreateGC(Glob->rdisplay,
				DefaultRootWindow(Glob->rdisplay), 0, 0);
  XSetPlaneMask(Glob->rdisplay, Glob->copyarea_gc,
		(ui32) 0xffffffff);
  XSetFunction(Glob->rdisplay, Glob->copyarea_gc, GXcopy);
  
  /*
   * highlight_background color
   */
  
  colstr = uGetParamString(Glob->prog_name, "x_hlight_background",
			 X_HLIGHT_BACKGROUND);
  color = xGetXColor(Glob->rdisplay, cmap, &Glob->color_index, colstr);
  check_for_getxcolor_error(color, colstr);
  Glob->hlight_background = color->pixel;

  /*
   * Xor GC for rubberband lines
   */
  
  xor_color_num = uGetParamLong(Glob->prog_name, "x_xor_color_num", 50000);

  Glob->xor_gc = XCreateGC(Glob->rdisplay,
			   DefaultRootWindow(Glob->rdisplay), 0, 0);
  XSetForeground(Glob->rdisplay, Glob->xor_gc, xor_color_num);
  XSetFunction(Glob->rdisplay, Glob->xor_gc, GXxor);
  XSetLineAttributes(Glob->rdisplay, Glob->xor_gc, 1,
		     LineSolid, CapButt, JoinMiter);

  /*
   * print x id values for debug purposes
   */

  if (Glob->debug)
    print_x_ids(cmap);

}

/*****************************************************************************
 * setup_color_and_dim_gc
 *
 */

static void setup_color_and_dim_gc(GC *gc,
				   GC *dim_gc,
				   Colormap cmap,
				   const char *res_color_name,
				   const char *def_color_name)

{

  if (Glob->debug) {
    fprintf(stderr, "** setup_color_and_dim_gc **\n");
  }

  setup_color_gc(gc, cmap, res_color_name, def_color_name);
  setup_dim_gc(dim_gc, cmap, res_color_name, def_color_name);

}

/*****************************************************************************
 * setup_color_gc
 *
 */

static void setup_color_gc(GC *gc,
			   Colormap cmap,
			   const char *res_color_name,
			   const char *def_color_name)

{

  GC tmp_gc;
  char *colstr;

  if (Glob->debug) {
    fprintf(stderr, "** setup_color_gc **\n");
  }

  colstr = uGetParamString(Glob->prog_name,
			 res_color_name, def_color_name);

  tmp_gc = xGetColorGC(Glob->rdisplay, cmap, &Glob->color_index, colstr);

  check_for_getgc_error(tmp_gc, colstr);

  *gc = XCreateGC(Glob->rdisplay,
		  DefaultRootWindow(Glob->rdisplay), 0, 0);

  XCopyGC(Glob->rdisplay, tmp_gc, GCForeground, *gc);

  XSetBackground(Glob->rdisplay, *gc, Glob->background);

}

/*****************************************************************************
 * setup_dim_gc
 *
 */

static void setup_dim_gc(GC *gc,
			 Colormap cmap,
			 const char *res_color_name,
			 const char *def_color_name)

{

  GC tmp_gc;
  char *colstr, dim_colstr[40];
  XColor *xcolor;

  if (Glob->debug) {
    fprintf(stderr, "** setup_dim_gc **\n");
  }

  colstr = uGetParamString(Glob->prog_name,
			 res_color_name, def_color_name);

  xcolor = xGetXColor(Glob->rdisplay, cmap, &Glob->color_index, colstr);

  sprintf(dim_colstr, "#%.4x%.4x%.4x",
	  (si32) ((double) xcolor->red *
		  Glob->color_dim_percent / 100.0 + 0.5),
	  (si32) ((double) xcolor->green *
		  Glob->color_dim_percent / 100.0 + 0.5),
	  (si32) ((double) xcolor->blue *
		  Glob->color_dim_percent / 100.0 + 0.5));

  tmp_gc = xGetColorGC(Glob->rdisplay, cmap, &Glob->color_index, dim_colstr);

  check_for_getgc_error(tmp_gc, colstr);

  *gc = XCreateGC(Glob->rdisplay,
		  DefaultRootWindow(Glob->rdisplay), 0, 0);

  XCopyGC(Glob->rdisplay, tmp_gc, GCForeground, *gc);

  XSetBackground(Glob->rdisplay, *gc, Glob->background);

}

/*****************************************************************************
 * check_for_getgc_error
 *
 */

static void check_for_getgc_error(GC gc,
				  const char *colorname)

{

  if (gc == NULL) {
    fprintf(stderr, "ERROR - %s:set_xgcs.\n", Glob->prog_name);
    fprintf(stderr, "Getting GC for color '%s'\n", colorname);
    tidy_and_exit(1);
  }

}

/*****************************************************************************
 * check_for_getxcolor_error
 *
 */

static void check_for_getxcolor_error(XColor *x_color,
				      const char *colorname)

{

  if (x_color == NULL) {
    fprintf(stderr, "ERROR - %s:set_xgcs.\n", Glob->prog_name);
    fprintf(stderr, "Getting GC for color '%s'\n", colorname);
    tidy_and_exit(1);
  }

}

/*****************************************************************************
 * print_x_ids
 *
 */

static void print_x_ids(Colormap cmap)

{

  fprintf(stderr, "cmap = %ld\n", cmap);

  fprintf(stderr, "tick_gc = %ld\n",
	  (long) Glob->tick_gc);

  fprintf(stderr, "vsection_pos_gc = %ld\n",
	  (long) Glob->vsection_pos_gc);

  fprintf(stderr, "zoom_box_gc = %ld\n",
	  (long) Glob->zoom_box_gc);

  fprintf(stderr, "ring_gc = %ld\n",
	  (long) Glob->ring_gc);

  fprintf(stderr, "ticklabel_gc = %ld\n",
	  (long) Glob->ticklabel_gc);

  fprintf(stderr, "track_annotation_gc = %ld\n",
	  (long) Glob->track_annotation_gc);

  fprintf(stderr, "track_case_gc = %ld\n",
	  (long) Glob->track_case_gc);

  fprintf(stderr, "text_gc = %ld\n",
	  (long) Glob->text_gc);

  fprintf(stderr, "pos_contour_gc = %ld\n",
	  (long) Glob->pos_contour_gc);

  fprintf(stderr, "zero_contour_gc = %ld\n",
	  (long) Glob->zero_contour_gc);

  fprintf(stderr, "neg_contour_gc = %ld\n",
	  (long) Glob->neg_contour_gc);

  fprintf(stderr, "crosshair_gc = %ld\n",
	  (long) Glob->crosshair_gc);

  fprintf(stderr, "past_storm_gc[0] = %ld\n",
	  (long) Glob->past_storm_gc[0]);

  fprintf(stderr, "past_storm_gc[1] = %ld\n",
	  (long) Glob->past_storm_gc[1]);

  fprintf(stderr, "past_storm_dim_gc[0] = %ld\n",
	  (long) Glob->past_storm_gc[0]);

  fprintf(stderr, "past_storm_dim_gc[1] = %ld\n",
	  (long) Glob->past_storm_gc[1]);

  fprintf(stderr, "future_storm_gc[0] = %ld\n",
	  (long) Glob->future_storm_gc[0]);

  fprintf(stderr, "future_storm_gc[1] = %ld\n",
	  (long) Glob->future_storm_gc[1]);

  fprintf(stderr, "future_storm_dim_gc[0] = %ld\n",
	  (long) Glob->future_storm_dim_gc[0]);

  fprintf(stderr, "future_storm_dim_gc[1] = %ld\n",
	  (long) Glob->future_storm_dim_gc[1]);

  fprintf(stderr, "forecast_storm_gc[0] = %ld\n",
	  (long) Glob->forecast_storm_gc[0]);

  fprintf(stderr, "forecast_storm_gc[1] = %ld\n",
	  (long) Glob->forecast_storm_gc[1]);

  fprintf(stderr, "forecast_storm_dim_gc[0] = %ld\n",
	  (long) Glob->forecast_storm_dim_gc[0]);

  fprintf(stderr, "forecast_storm_dim_gc[1] = %ld\n",
	  (long) Glob->forecast_storm_dim_gc[1]);

  fprintf(stderr, "current_storm_gc = %ld\n",
	  (long) Glob->current_storm_gc);

  fprintf(stderr, "current_storm_dim_gc = %ld\n",
	  (long) Glob->current_storm_dim_gc);

  fprintf(stderr, "forecast_vector_gc = %ld\n",
	  (long) Glob->forecast_vector_gc);

  fprintf(stderr, "forecast_vector_dim_gc = %ld\n",
	  (long) Glob->forecast_vector_dim_gc);

  fprintf(stderr, "past_vector_gc = %ld\n",
	  (long) Glob->past_vector_gc);

  fprintf(stderr, "past_vector_dim_gc = %ld\n",
	  (long) Glob->past_vector_dim_gc);

  fprintf(stderr, "future_vector_gc = %ld\n",
	  (long) Glob->future_vector_gc);

  fprintf(stderr, "future_vector_dim_gc = %ld\n",
	  (long) Glob->future_vector_dim_gc);

  fprintf(stderr, "pixmap_gc = %ld\n",
	  (long) Glob->pixmap_gc);
  
  fprintf(stderr, "copyarea_gc = %ld\n",
	  (long) Glob->copyarea_gc);
  
  fprintf(stderr, "xor_gc = %ld\n",
	  (long) Glob->xor_gc);

}
