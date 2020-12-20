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
 * setup_cappi_windows.c
 *
 * compute window sizes, and set up the windows
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1990   
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "Rview.hh"
#include <X11/cursorfont.h>
using namespace std;

#define DEFAULT_MAIN_NAME "TITAN (RAL-NCAR)"
#define DEFAULT_ICON_NAME "TITAN"

/* XFontStruct *load_font(); */

void setup_cappi_windows()

{

  static int first_call = TRUE;
  static si32 subborder, mainborder;
  static long x_xaxismargin, x_yaxismargin, x_topmargin;
  static ui32 titleheight, scalewidth, buttonheight;
  static ui32 minmainheight, minmainwidth;
  static ui32 vsection_x_from_main, vsection_y_from_main;
  static ui32 help_x_from_main, help_y_from_main;
  
  static Cursor plot_window_cursor;

  char *wstring, *istring;
  int i, izoom;
  int tmp_x;
  int titlex, titley;
  int buttonx[N_CAPPI_BUTTONS], buttony;
  int plotx, ploty;
  int scalex, scaley;

  ui32 main_height, main_width;
  ui32 titlewidth;
  ui32 buttonwidth[N_CAPPI_BUTTONS], bwidth, width_left;
  ui32 requested_imagewidth, requested_imageheight;
  ui32 plotwidth, plotheight;
  ui32 scaleheight;
  ui32 imagewidth, imageheight;

  double aspect;

  zoom_t *zoom;
  XSizeHints hint;
  XTextProperty wname, iname;
  XWindowChanges changes;
  ui32 change_mask;
  Window window;

  static ui32 cappi_width = 0;
  static ui32 cappi_height = 0;

  if (Glob->verbose) {
    fprintf(stderr, "** setup_cappi_windows **\n");
  }

  /*
   * return now if window geometry has not changed
   */

  if (cappi_width == Glob->mainwidth &&
      cappi_height == Glob->mainheight) {

    return;

  } else {
      
    /*
     * store dimensions locally for later comparison
     */

    cappi_width = Glob->mainwidth;
    cappi_height = Glob->mainheight;

  }

  if (first_call) {

    /*
     * get X resources
     */
  
    mainborder = uGetParamLong(Glob->prog_name,
			     "x_mainborder", X_MAINBORDER);

    subborder = uGetParamLong(Glob->prog_name,
			    "x_subborder", X_SUBBORDER);

    minmainwidth = (ui32)
      uGetParamLong(Glob->prog_name,
		  "x_minmainwidth", X_MINMAINWIDTH);

    minmainheight = (ui32)
      uGetParamLong(Glob->prog_name,
		  "x_minmainheight", X_MINMAINHEIGHT);

    /*
     * get margins from the parameters resources
     */

    x_xaxismargin = uGetParamLong(Glob->prog_name,
				"x_xaxismargin", X_XAXISMARGIN);
    x_yaxismargin = uGetParamLong(Glob->prog_name,
				"x_yaxismargin", X_YAXISMARGIN);
    x_topmargin = uGetParamLong(Glob->prog_name,
			      "x_topmargin", X_TOPMARGIN);
  

    titleheight = 2 * (Glob->x_title_font->ascent +
		       Glob->x_title_font->descent);
    buttonheight = 2 * (Glob->x_button_font->ascent +
			Glob->x_button_font->descent);
    scalewidth = (ui32) uGetParamLong(Glob->prog_name,
					    "x_scalewidth", X_SCALEWIDTH);

    vsection_x_from_main =
      uGetParamLong(Glob->prog_name,
		  "x_vsection_x_from_main", X_VSECTION_X_FROM_MAIN);
    vsection_y_from_main =
      uGetParamLong(Glob->prog_name,
		  "x_vsection_y_from_main", X_VSECTION_Y_FROM_MAIN);

    help_x_from_main =
      uGetParamLong(Glob->prog_name,
		  "x_help_x_from_main", X_HELP_X_FROM_MAIN);
    help_y_from_main =
      uGetParamLong(Glob->prog_name,
		  "x_help_y_from_main", X_HELP_Y_FROM_MAIN);

    plot_window_cursor = XCreateFontCursor(Glob->rdisplay, XC_crosshair);

  } /* if (first_call) */

  /*
   * compute window sizes
   */

  requested_imagewidth = (Glob->mainwidth - scalewidth - 4 * subborder
			  - x_yaxismargin);
  requested_imageheight = (Glob->mainheight - titleheight - buttonheight
			   - 6 * subborder - x_xaxismargin - x_topmargin);

  /*
   * ensure image aspect ratio is correct
   */
 
  aspect = (double) requested_imageheight / (double) requested_imagewidth;

  if (aspect < Glob->full_aspect) {
    
    imageheight = requested_imageheight;
    imagewidth = (ui32) ((double) imageheight / Glob->full_aspect + 0.5);
    
  } else {
      
    imagewidth = requested_imagewidth;
    imageheight = (ui32) ((double) imagewidth * Glob->full_aspect + 0.5);
    
  } /* if (aspect ....... */
    
  plotwidth = imagewidth + x_yaxismargin;
  plotheight = imageheight + x_xaxismargin + x_topmargin;

  titlewidth = plotwidth + scalewidth + 2 * subborder;

  main_height = titleheight + buttonheight + plotheight + 6 * subborder;
  main_width = titlewidth + 2 * subborder;

  scaleheight = plotheight;

  bwidth = (main_width / N_CAPPI_BUTTONS) - 2 * subborder;
  width_left = main_width;
  for (i = 0; i < N_CAPPI_BUTTONS; i++) {
    buttonwidth[i] = bwidth;
    width_left -= bwidth + 2 * subborder;
  }

  /*
   * distribute extra width in some of the buttons, and check that width
   * does not exceed maximum
   */

  for (i = 0; i < N_CAPPI_BUTTONS; i++) {

    if(width_left > 0) {
      buttonwidth[i]++;
      width_left--;
    }

    if (buttonwidth[i] > Glob->max_button_width)
      buttonwidth[i] = Glob->max_button_width;

  }
  
  /*
   * compute window positions relative to main window
   */

  buttony = 0;
  tmp_x = main_width - 2 * subborder;

  for (i = N_CAPPI_BUTTONS - 1; i >= 0; i--) {
    buttonx[i] = tmp_x - buttonwidth[i];
    tmp_x -= (buttonwidth[i] + 2 * subborder);
  }

  titlex = 0;
  titley = buttony + buttonheight + 2 * subborder;

  plotx = 0;
  ploty = buttony + buttonheight + titleheight + 4 * subborder;

  scalex = plotx + plotwidth + 2 * subborder;
  scaley = ploty;

  if (first_call) {

    /*
     * compute main x and y position
     */

    if (Glob->mainx_sign < 0) {
      Glob->mainx = DisplayWidth(Glob->rdisplay, Glob->rscreen) +
	Glob->mainx - main_width;
    }

    if (Glob->mainy_sign < 0) {
      Glob->mainy = DisplayHeight(Glob->rdisplay, Glob->rscreen) +
	Glob->mainy - main_height;
    }

    Glob->vsection_x = Glob->mainx + vsection_x_from_main;
    Glob->vsection_y = Glob->mainy + vsection_y_from_main;

    Glob->help_x = Glob->mainx + help_x_from_main;
    Glob->help_y = Glob->mainy + help_y_from_main;

    Glob->mainwidth = main_width;
    Glob->mainheight = main_height;

    hint.x = Glob->mainx;
    hint.y = Glob->mainy;
    hint.width = Glob->mainwidth;
    hint.height = Glob->mainheight;
    hint.min_width = minmainwidth;
    hint.min_height = minmainheight;
    hint.flags = PPosition | PSize | PMinSize | USPosition | USSize;

    wstring = (char *) DEFAULT_MAIN_NAME;
    istring = (char *) DEFAULT_ICON_NAME;
    XStringListToTextProperty(&wstring, 1, &wname);
    XStringListToTextProperty(&istring, 1, &iname);

    /*
     * create simple window
     */

    Glob->main_window =
      XCreateSimpleWindow (Glob->rdisplay,
			   DefaultRootWindow(Glob->rdisplay),
			   Glob->mainx, Glob->mainy,
			   Glob->mainwidth, Glob->mainheight,
			   mainborder, Glob->foreground,
			   Glob->background);
    
    XSetWMProperties(Glob->rdisplay, Glob->main_window,
		     &wname, &iname,
		     Glob->argv, Glob->argc,
		     &hint, 0, 0);

    /*
     * create subwindows
     */

    /*
     * button windows
     */

    for (i = 0; i < N_CAPPI_BUTTONS; i++) {

      window = XCreateSimpleWindow(Glob->rdisplay,
				   Glob->main_window,
				   buttonx[i], buttony,
				   buttonwidth[i], buttonheight,
				   subborder,
				   Glob->foreground, Glob->background);

      XSetWindowBorder(Glob->rdisplay, window, Glob->border_color);

      GXInitFrame(Glob->cappi_button_frame[i], Glob->rdisplay, window,
		  Glob->x_button_font);

      GXSetGeomFrame(Glob->cappi_button_frame[i],
		     buttonwidth[i], buttonheight);

    } /* i */

    /*
     * title window
     */

    GXCreateFrame(Glob->cappi_title_frame,
		  Glob->rdisplay,
		  Glob->main_window,
		  titlex, titley,
		  titlewidth, titleheight,
		  subborder,
		  Glob->foreground, Glob->background,
		  Glob->border_color,
		  Glob->x_title_font);

    /*
     * plot window
     */

    GXCreateFrame(Glob->cappi_plot_frame,
		  Glob->rdisplay,
		  Glob->main_window,
		  plotx, ploty,
		  plotwidth, plotheight,
		  subborder,
		  Glob->foreground, Glob->background,
		  Glob->border_color,
		  Glob->x_text_font);
    
    /*
     * scale window
     */

    GXCreateFrame(Glob->main_scale_frame,
		  Glob->rdisplay,
		  Glob->main_window,
		  scalex, scaley,
		  scalewidth, scaleheight,
		  subborder,
		  Glob->foreground, Glob->background,
		  Glob->border_color,
		  Glob->x_scale_font);
    
  } else {

    /*
     * resize windows, recompute frame geometry
     */

    change_mask = CWWidth | CWHeight;
    changes.width = main_width;
    changes.height = main_height;

    XConfigureWindow(Glob->rdisplay, Glob->main_window,
		     change_mask, &changes);

    for (i = 0; i < N_CAPPI_BUTTONS; i++) {

      XMoveResizeWindow(Glob->rdisplay,
			Glob->cappi_button_frame[i]->x->drawable,
			buttonx[i], buttony,
			buttonwidth[i], buttonheight);
      GXSetGeomFrame(Glob->cappi_button_frame[i],
		     buttonwidth[i], buttonheight);

    } /* i */
  
    GXResetFrame(Glob->cappi_title_frame, 
		 titlex, titley,
		 titlewidth, titleheight);
  
    GXResetFrame(Glob->cappi_plot_frame, 
		 plotx, ploty,
		 plotwidth, plotheight);

    GXResetFrame(Glob->main_scale_frame, 
		 scalex, scaley,
		 scalewidth, scaleheight);

  } /* if (first_call) */

  /*
   * sort out pixmaps
   */

  for (izoom = 0; izoom < NZOOM; izoom++) {

    zoom = Glob->zoom + izoom;

    if (!first_call)
      XFreePixmap(Glob->rdisplay, zoom->pixmap);
      
    zoom->pixmap =
      XCreatePixmap(Glob->rdisplay,
		    Glob->main_window,
		    plotwidth, plotheight,
		    XDefaultDepth(Glob->rdisplay, Glob->rscreen));

    zoom->current = FALSE;

  } /* izoom */

  /*
   * set the plot window cursor
   */
  
  XDefineCursor(Glob->rdisplay, Glob->cappi_plot_frame->x->drawable,
		plot_window_cursor);
    

  Glob->cappi_active = TRUE;

  /*
   * debug output for X resource Id's
   */

  if (Glob->verbose) {

    fprintf(stderr, "main window = %ld\n", Glob->main_window);

    for (i = 0; i < N_CAPPI_BUTTONS; i++)
      fprintf(stderr, "cappi button window[%d] = %ld\n",
	      i, Glob->cappi_button_frame[i]->x->drawable);

    fprintf(stderr, "cappi title window = %ld\n",
	    Glob->cappi_title_frame->x->drawable);

    fprintf(stderr, "cappi plot window = %ld\n",
	    Glob->cappi_plot_frame->x->drawable);

    fprintf(stderr, "main scale window = %ld\n",
	    Glob->main_scale_frame->x->drawable);

    for (izoom = 0; izoom < NZOOM; izoom++)
      fprintf(stderr, "zoom[%d].pixmap = %ld\n",
	      izoom, Glob->zoom[izoom].pixmap);

    fprintf(stderr, "cappi plot window cursor = %ld\n",
	    plot_window_cursor);

  } /* if (Glob->verbose) */

  /*
   * reset first_call flag
   */

  first_call = FALSE;

}

void set_cappi_window_label(const char *label)

{

  char *wstring;
  XTextProperty wname;
  
  wstring = strdup(label);
  XStringListToTextProperty(&wstring, 1, &wname);
  
  XSetWMName(Glob->rdisplay, Glob->main_window, &wname);

  free(wstring);

}


void set_cappi_window_label_default()

{

  set_cappi_window_label(DEFAULT_MAIN_NAME);

}


