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
 * set_gcs.c: TimeHist routine
 *
 * set graphic contexts, colors etc
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "TimeHist.hh"
using namespace std;

static void check_for_getgc_error(GC gc,
				  const char *colorname);

static void check_for_getxcolor_error(XColor *x_color,
				      const char *colorname);

static void print_x_ids(Colormap cmap);

static void setup_gc_from_color(GC *gc,
				Colormap cmap,
				const char *colstr);

static void setup_gc_from_param(GC *gc,
				Colormap cmap,
				const char *res_color_name,
				const char *def_color_name);

void set_xgcs(void)

{
  
  XColor *color;
  Colormap cmap;
  char *colstr;
  int function;
  ui32 width;
  si32 i;
  
  /*
   * get default color map
   */
  
  cmap = DefaultColormap(Glob->rdisplay, Glob->rscreen);
  
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
  
  Glob->foreground_gc = XCreateGC(Glob->rdisplay,
				  DefaultRootWindow(Glob->rdisplay), 0, 0);
  XSetForeground(Glob->rdisplay, Glob->foreground_gc, Glob->foreground);

  Glob->background_gc = XCreateGC(Glob->rdisplay,
				  DefaultRootWindow(Glob->rdisplay), 0, 0);
  XSetForeground(Glob->rdisplay, Glob->background_gc, Glob->background);

  /*
   * text gc
   */

  setup_gc_from_param(&Glob->text_gc, cmap,
		      "x_text_color", X_TEXT_COLOR);

  XSetFont(Glob->rdisplay, Glob->text_gc, Glob->x_text_font->fid);

  /*
   * title frame gcs
   */
  
  setup_gc_from_param(&Glob->tscale_title_frame->x->gc, cmap,
		      "x_titletext_color", X_TITLETEXT_COLOR);
  
  XSetFont(Glob->rdisplay, Glob->tscale_title_frame->x->gc,
	   Glob->x_title_font->fid);
  
  XCopyGC(Glob->rdisplay, Glob->tscale_title_frame->x->gc,
	  GCForeground | GCBackground | GCFont,
	  Glob->thist_title_frame->x->gc);
  
  XCopyGC(Glob->rdisplay, Glob->tscale_title_frame->x->gc,
	  GCForeground | GCBackground | GCFont,
	  Glob->timeht_title_frame->x->gc);
  
  XCopyGC(Glob->rdisplay, Glob->tscale_title_frame->x->gc,
	  GCForeground | GCBackground | GCFont,
	  Glob->rdist_title_frame->x->gc);
  
  XCopyGC(Glob->rdisplay, Glob->tscale_title_frame->x->gc,
	  GCForeground | GCBackground | GCFont,
	  Glob->help_title_frame->x->gc);

  XCopyGC(Glob->rdisplay, Glob->tscale_title_frame->x->gc,
	  GCForeground | GCBackground | GCFont,
	  Glob->union_title_frame->x->gc);
  
  /*
   * button frame gc's
   */
  
  setup_gc_from_param(&Glob->tscale_button_frame[0]->x->gc, cmap,
		      "x_buttontext_color", X_BUTTONTEXT_COLOR);
  
  XSetFont(Glob->rdisplay, Glob->tscale_button_frame[0]->x->gc,
	   Glob->x_button_font->fid);
  
  for (i = 1; i < N_TSCALE_BUTTONS; i++)
    XCopyGC(Glob->rdisplay, Glob->tscale_button_frame[0]->x->gc,
	    GCForeground | GCBackground | GCFont,
	    Glob->tscale_button_frame[i]->x->gc);
  
  for (i = 0; i < N_THIST_BUTTONS; i++)
    XCopyGC(Glob->rdisplay, Glob->tscale_button_frame[0]->x->gc,
	    GCForeground | GCBackground | GCFont,
	    Glob->thist_button_frame[i]->x->gc);
  
  for (i = 0; i < N_TIMEHT_BUTTONS; i++)
    XCopyGC(Glob->rdisplay, Glob->tscale_button_frame[0]->x->gc,
	    GCForeground | GCBackground | GCFont,
	    Glob->timeht_button_frame[i]->x->gc);
  
  for (i = 0; i < N_RDIST_BUTTONS; i++)
    XCopyGC(Glob->rdisplay, Glob->tscale_button_frame[0]->x->gc,
	    GCForeground | GCBackground | GCFont,
	    Glob->rdist_button_frame[i]->x->gc);
  
  for (i = 0; i < N_UNION_BUTTONS; i++)
    XCopyGC(Glob->rdisplay, Glob->tscale_button_frame[0]->x->gc,
	    GCForeground | GCBackground | GCFont,
	    Glob->union_button_frame[i]->x->gc);
  
  for (i = 0; i < N_HELP_BUTTONS; i++)
    XCopyGC(Glob->rdisplay, Glob->tscale_button_frame[0]->x->gc,
	    GCForeground | GCBackground | GCFont,
	    Glob->help_button_frame[i]->x->gc);

  /*
   * scale frame gcs
   */
  
  setup_gc_from_param(&Glob->timeht_scale_frame->x->gc, cmap,
		      "x_scaletext_color", X_SCALETEXT_COLOR);
  
  XSetFont(Glob->rdisplay, Glob->timeht_scale_frame->x->gc,
	   Glob->x_scale_font->fid);
  
  XCopyGC(Glob->rdisplay, Glob->timeht_scale_frame->x->gc,
	  GCForeground | GCBackground | GCFont,
	  Glob->rdist_scale_frame->x->gc);
  
  /*
   * tick mark gc
   */
  
  setup_gc_from_param(&Glob->tick_gc, cmap,
		      "x_tick_color", X_TICK_COLOR);
  
  width = (ui32) uGetParamLong(Glob->prog_name,
			       "x_tick_line_width", X_TICK_LINE_WIDTH);
  
  XSetLineAttributes(Glob->rdisplay, Glob->tick_gc, width,
		     LineSolid, CapButt, JoinMiter);
  
  /*
   * divider line gc
   */
  
  setup_gc_from_param(&Glob->divider_gc, cmap,
		      "x_divider_color", X_DIVIDER_COLOR);
  
  width = (ui32) uGetParamLong(Glob->prog_name,
			      "x_divider_line_width", X_DIVIDER_LINE_WIDTH);
  
  XSetLineAttributes(Glob->rdisplay, Glob->divider_gc, width,
		     LineSolid, CapButt, JoinMiter);
  
  /*
   * graph line gc's
   */
  
  setup_gc_from_param(&Glob->thist_graph_gc[THIST_VOL_FIELD], cmap,
		      "x_thist_vol_color", X_THIST_VOL_COLOR);
  
  setup_gc_from_param(&Glob->thist_graph_gc[THIST_AREA_FIELD], cmap,
		      "x_thist_area_color", X_THIST_AREA_COLOR);
  
  setup_gc_from_param(&Glob->thist_graph_gc[THIST_PFLUX_FIELD], cmap,
		      "x_thist_pflux_color", X_THIST_PFLUX_COLOR);
  
  setup_gc_from_param(&Glob->thist_graph_gc[THIST_MASS_FIELD], cmap,
		      "x_thist_mass_color", X_THIST_MASS_COLOR);
  
  setup_gc_from_param(&Glob->thist_graph_gc[THIST_VIL_FIELD], cmap,
		      "x_thist_vil_color", X_THIST_VIL_COLOR);
  
  setup_gc_from_param(&Glob->thist_forecast_gc, cmap,
		      "x_thist_forecast_color", X_THIST_FORECAST_COLOR);
  
  setup_gc_from_param(&Glob->ht_maxdbz_gc, cmap,
		      "x_ht_maxdbz_color", X_HT_MAXDBZ_COLOR);
  
  setup_gc_from_param(&Glob->ht_centroid_gc, cmap,
		      "x_ht_centroid_color", X_HT_CENTROID_COLOR);
  
  setup_gc_from_param(&Glob->ht_refl_centroid_gc, cmap,
		      "x_ht_refl_centroid_color", X_HT_REFL_CENTROID_COLOR);
  
  setup_gc_from_param(&Glob->top_base_gc, cmap,
		      "x_top_base_color", X_TOP_BASE_COLOR);
  
  setup_gc_from_param(&Glob->union_graph_gc[UNION_0_FIELD], cmap,
		      "x_union_0_color", X_UNION_0_COLOR);
  
  setup_gc_from_param(&Glob->union_graph_gc[UNION_1_FIELD], cmap,
		      "x_union_1_color", X_UNION_1_COLOR);
  
  setup_gc_from_param(&Glob->union_graph_gc[UNION_2_FIELD], cmap,
		      "x_union_2_color", X_UNION_2_COLOR);
  
  setup_gc_from_param(&Glob->union_graph_gc[UNION_3_FIELD], cmap,
		      "x_union_3_color", X_UNION_3_COLOR);
  
  /*
   * time bars in tscale window
   */
  
  setup_gc_from_color(&Glob->current_gc, cmap,
		      Glob->track_shmem->current_color);
  
  width = (ui32)uGetParamLong(Glob->prog_name,
			    "x_current_time_line_width", 3);
  
  XSetLineAttributes(Glob->rdisplay, Glob->current_gc, width,
		     LineSolid, CapButt, JoinMiter);
  
  setup_gc_from_color(&Glob->past_gc, cmap,
		      Glob->track_shmem->past_color);
  
  setup_gc_from_color(&Glob->future_gc, cmap,
		      Glob->track_shmem->future_color);
  
  setup_gc_from_color(&Glob->forecast_gc, cmap,
		      Glob->track_shmem->forecast_color);
  
  /*
   * tick label gc
   */
  
  setup_gc_from_param(&Glob->ticklabel_gc, cmap,
		      "x_ticklabel_color", X_TICKLABEL_COLOR);
  
  XSetFont(Glob->rdisplay, Glob->ticklabel_gc, Glob->x_ticklabel_font->fid);
  
  /*
   * header gc
   */
  
  setup_gc_from_param(&Glob->header_gc, cmap,
		      "x_header_color", X_HEADER_COLOR);
  
  XSetFont(Glob->rdisplay, Glob->header_gc, Glob->x_header_font->fid);
  
  /*
   * crosshair gc
   */

  setup_gc_from_param(&Glob->crosshair_gc, cmap,
		      "x_crosshair_color", X_CROSSHAIR_COLOR);
  
  width = (ui32)uGetParamLong(Glob->prog_name,
			     "x_crosshair_line_width", X_CROSSHAIR_WIDTH);
  
  XSetLineAttributes(Glob->rdisplay, Glob->crosshair_gc, width,
		     LineSolid, CapButt, JoinMiter);
  
  /*
   * window border color
   */
  
  colstr = uGetParamString(Glob->prog_name, "x_border_color",
			 X_BORDER_COLOR);
  color = xGetXColor(Glob->rdisplay, cmap, &Glob->color_index, colstr);
  check_for_getxcolor_error(color, colstr);
  Glob->border_color = color->pixel;
  
  /*
   * plot frame default gcs
   */
  
  XSetBackground(Glob->rdisplay, Glob->tscale_plot_frame->x->gc,
		 Glob->background);
  XSetForeground(Glob->rdisplay, Glob->tscale_plot_frame->x->gc,
		 Glob->border_color);
  XSetFont(Glob->rdisplay, Glob->tscale_plot_frame->x->gc,
	   Glob->x_header_font->fid);
  
  XSetBackground(Glob->rdisplay, Glob->thist_plot_frame->x->gc,
		 Glob->background);
  XSetForeground(Glob->rdisplay, Glob->thist_plot_frame->x->gc,
		 Glob->border_color);
  XSetFont(Glob->rdisplay, Glob->thist_plot_frame->x->gc,
	   Glob->x_header_font->fid);
  
  XSetBackground(Glob->rdisplay, Glob->timeht_plot_frame->x->gc,
		 Glob->background);
  XSetForeground(Glob->rdisplay, Glob->timeht_plot_frame->x->gc,
		 Glob->border_color);
  XSetFont(Glob->rdisplay, Glob->timeht_plot_frame->x->gc,
	   Glob->x_header_font->fid);
  
  XSetBackground(Glob->rdisplay, Glob->rdist_plot_frame->x->gc,
		 Glob->background);
  XSetForeground(Glob->rdisplay, Glob->rdist_plot_frame->x->gc,
		 Glob->border_color);
  XSetFont(Glob->rdisplay, Glob->rdist_plot_frame->x->gc,
	   Glob->x_header_font->fid);
  
  XSetBackground(Glob->rdisplay, Glob->union_plot_frame->x->gc,
		 Glob->background);
  XSetForeground(Glob->rdisplay, Glob->union_plot_frame->x->gc,
		 Glob->border_color);
  XSetFont(Glob->rdisplay, Glob->union_plot_frame->x->gc,
	   Glob->x_header_font->fid);
  
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
  
  colstr = uGetParamString(Glob->prog_name,
			 "x_hlight_background",
			 X_HLIGHT_BACKGROUND);
  color = xGetXColor(Glob->rdisplay, cmap, &Glob->color_index, colstr);
  check_for_getxcolor_error(color, colstr);
  Glob->hlight_background = color->pixel;
  
  /*
   * Xor GC for rubberband lines
   */
  
  Glob->xor_gc = XCreateGC(Glob->rdisplay,
			   DefaultRootWindow(Glob->rdisplay), 0, 0);
  function = GXxor;
  XSetForeground(Glob->rdisplay, Glob->xor_gc, (ui32) 1);
  XSetFunction(Glob->rdisplay, Glob->xor_gc, function);
  XSetLineAttributes(Glob->rdisplay, Glob->xor_gc, 1,
		     LineSolid, CapButt, JoinMiter);
  
  /*
   * print x id values for debug purposes
   */
  
  if (Glob->debug == TRUE)
    print_x_ids(cmap);
  
}

/*****************************************************************************
 * setup_gc_from_param
 *
 */

static void setup_gc_from_param(GC *gc,
				Colormap cmap,
				const char *res_color_name,
				const char *def_color_name)

{
  
  GC tmp_gc;
  char *colstr;
  
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
 * setup_gc_from_color
 *
 */

static void setup_gc_from_color(GC *gc,
				Colormap cmap,
				const char *colstr)

{
  
  GC tmp_gc;
  
  tmp_gc = xGetColorGC(Glob->rdisplay, cmap, &Glob->color_index, colstr);
  
  check_for_getgc_error(tmp_gc, colstr);
  
  *gc = XCreateGC(Glob->rdisplay,
		  DefaultRootWindow(Glob->rdisplay), 0, 0);
  
  XCopyGC(Glob->rdisplay, tmp_gc, GCForeground, *gc);
  
/*   XSetBackground(Glob->rdisplay, *gc, Glob->background); */
  
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
  fprintf(stderr, "text_gc = %ld\n", (long) Glob->text_gc);
  fprintf(stderr, "tick_gc = %ld\n", (long) Glob->tick_gc);
  fprintf(stderr, "divider_gc = %ld\n", (long) Glob->divider_gc);
  fprintf(stderr, "thist_graph_gc = %ld\n", (long) Glob->thist_graph_gc);
  fprintf(stderr, "thist_forecast_gc = %ld\n",
	  (long) Glob->thist_forecast_gc);
  fprintf(stderr, "ht_maxdbz_gc = %ld\n", (long) Glob->ht_maxdbz_gc);
  fprintf(stderr, "ht_centroid_gc = %ld\n", (long) Glob->ht_centroid_gc);
  fprintf(stderr, "ht_refl_centroid_gc = %ld\n",
	  (long) Glob->ht_refl_centroid_gc);
  fprintf(stderr, "union_graph_gc = %ld\n", (long) Glob->union_graph_gc);
  fprintf(stderr, "top_base_gc = %ld\n", (long) Glob->top_base_gc);
  fprintf(stderr, "ticklabel_gc = %ld\n", (long) Glob->ticklabel_gc);
  fprintf(stderr, "header_gc = %ld\n", (long) Glob->header_gc);  
  fprintf(stderr, "pixmap_gc = %ld\n", (long) Glob->pixmap_gc);
  fprintf(stderr, "crosshair_gc = %ld\n", (long) Glob->crosshair_gc);
  fprintf(stderr, "copyarea_gc = %ld\n", (long) Glob->copyarea_gc);
  fprintf(stderr, "xor_gc = %ld\n", (long) Glob->xor_gc);
  
}

