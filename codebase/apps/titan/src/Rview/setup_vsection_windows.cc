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
 * setup_vsection_windows.c
 *
 * compute window sizes for vert section, and set up the windows
 *
 * RAP, NCAR, Boulder CO
 *
 * April 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "Rview.hh"
#include <X11/cursorfont.h>
using namespace std;

XFontStruct *load_font();

void setup_vsection_windows()

{

  XTextProperty wname, iname;
  XSizeHints hint;
  XWindowChanges changes;
  ui32 change_mask;
  Window window;
  char *wstring, *istring;
  int i;
  int titlex, titley;
  int tmp_x, buttonx[N_VSECTION_BUTTONS], buttony;
  int plotx, ploty;
  ui32 titlewidth;
  ui32 buttonwidth[N_VSECTION_BUTTONS], bwidth, width_left;
  ui32 plotwidth, plotheight;
  ui32 imagewidth, imageheight;
  
  static si32 subborder, mainborder;
  static long x_xaxismargin, x_yaxismargin, x_topmargin;
  static ui32 titleheight, buttonheight;
  static ui32 minmainheight, minmainwidth;
  static Cursor plot_window_cursor;
  static int first_call = TRUE;

  static ui32 vsection_width = 0;
  static ui32 vsection_height = 0;

  if (Glob->verbose) {
    fprintf(stderr, "** setup_vsection_windows **\n");
  }

  /*
   * return now if window geometry has not changed
   */

  if (vsection_width == Glob->vsection_width &&
      vsection_height == Glob->vsection_height) {

    return;

  } else {
      
    /*
     * store dimensions locally for later comparison
     */

    vsection_width = Glob->vsection_width;
    vsection_height = Glob->vsection_height;

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
    x_topmargin = 0;
    
    titleheight = 2 * (Glob->x_title_font->ascent +
		       Glob->x_title_font->descent);

    buttonheight = 2 * (Glob->x_button_font->ascent +
			Glob->x_button_font->descent);
    
    plot_window_cursor = XCreateFontCursor(Glob->rdisplay, XC_top_left_arrow);
    
  } /* if (first_call) */
  
  /*
   * compute window sizes
   */
  
  imagewidth = Glob->vsection_width - 2 * subborder - x_yaxismargin;
  imageheight = Glob->vsection_height - titleheight - buttonheight
    - 6 * subborder - x_xaxismargin - x_topmargin;
  
  plotwidth = imagewidth + x_yaxismargin;
  plotheight = imageheight + x_xaxismargin + x_topmargin;
  
  titlewidth = plotwidth;
  
  bwidth = (Glob->vsection_width / N_VSECTION_BUTTONS) - 2 * subborder;
  width_left = Glob->vsection_width;
  
  for (i = 0; i < N_VSECTION_BUTTONS; i++) {
    buttonwidth[i] = bwidth;
    width_left -= bwidth + 2 * subborder;
  }
  
  /*
   * distribute extra width in some of the buttons, and check that width
   * does not exceed maximum
   */
  
  for (i = 0; i < N_VSECTION_BUTTONS; i++) {
    
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
  tmp_x = Glob->vsection_width - 2 * subborder;
  
  for (i = N_VSECTION_BUTTONS - 1; i >= 0; i--) {
    buttonx[i] = tmp_x - buttonwidth[i];
    tmp_x -= (buttonwidth[i] + 2 * subborder);
  }
  
  titlex = 0;
  titley = buttony + buttonheight + 2 * subborder;
  
  plotx = 0;
  ploty = titley + titleheight + 2 * subborder;
  
  
  if (first_call) {
    
    /*
     * set up plot frame
     */
    
    hint.x = Glob->vsection_x;
    hint.y = Glob->vsection_y;
    hint.width = Glob->vsection_width;
    hint.height = Glob->vsection_height;
    hint.min_width = minmainwidth;
    hint.min_height = minmainheight;
    hint.flags = PPosition | PSize | PMinSize | USPosition | USSize;

    wstring = (char *) "Vertical section (RAL-NCAR)";
    istring = (char *) "Vsect";
    XStringListToTextProperty(&wstring, 1, &wname);
    XStringListToTextProperty(&istring, 1, &iname);

    /*
     * create simple window
     */
    
    Glob->vsection_window =
      XCreateSimpleWindow (Glob->rdisplay,
			   DefaultRootWindow(Glob->rdisplay),
			   Glob->vsection_x, Glob->vsection_y,
			   Glob->vsection_width, Glob->vsection_height,
			   mainborder, Glob->foreground,
			   Glob->background);
    
    XSetWMProperties(Glob->rdisplay, Glob->vsection_window,
		     &wname, &iname,
		     Glob->argv, Glob->argc,
		     &hint, 0, 0);

    /*
     * create subwindows
     */
    
    /*
     * button windows
     */
    
    for (i = 0; i < N_VSECTION_BUTTONS; i++) {
      
      window = XCreateSimpleWindow(Glob->rdisplay,
				   Glob->vsection_window,
				   buttonx[i], buttony,
				   buttonwidth[i], buttonheight,
				   subborder, Glob->foreground,
				   Glob->background);
      
      XSetWindowBorder(Glob->rdisplay, window, Glob->border_color);
      
      GXInitFrame(Glob->vsection_button_frame[i], Glob->rdisplay, window,
		  Glob->x_button_font);
      
      GXSetGeomFrame(Glob->vsection_button_frame[i], buttonwidth[i],
		     buttonheight);
      
    } /* i */
    
    /*
     * title window
     */
    
    window = XCreateSimpleWindow(Glob->rdisplay,
				 Glob->vsection_window,
				 titlex, titley,
				 titlewidth, titleheight,
				 subborder,
				 Glob->foreground, Glob->background);
    
    XSetWindowBorder(Glob->rdisplay, window, Glob->border_color);
    
    GXInitFrame(Glob->vsection_title_frame, Glob->rdisplay, window,
		Glob->x_title_font);
    
    GXSetGeomFrame(Glob->vsection_title_frame, titlewidth, titleheight);
    
    /*
     * plot window
     */
    
    window = XCreateSimpleWindow(Glob->rdisplay,
				 Glob->vsection_window,
				 plotx, ploty,
				 plotwidth, plotheight,
				 subborder,
				 Glob->foreground, Glob->background);
    
    XSetWindowBorder(Glob->rdisplay, window, Glob->border_color);
    
    GXInitFrame(Glob->vsection_plot_frame, Glob->rdisplay, window,
		Glob->x_text_font);
    
    GXSetGeomFrame(Glob->vsection_plot_frame, plotwidth, plotheight);
    
    /*
     * create pixmap
     */
    
    Glob->vsection.pixmap =
      XCreatePixmap(Glob->rdisplay,
		    Glob->vsection_window,
		    plotwidth, plotheight,
		    XDefaultDepth(Glob->rdisplay, Glob->rscreen));
    
    /*
     * set the plot window cursor
     */
  
    XDefineCursor(Glob->rdisplay, Glob->vsection_plot_frame->x->drawable,
		  plot_window_cursor);
  
    first_call = FALSE;
    
  } else {
    
    /*
     * resize windows, recompute frame geometry
     */
    
    change_mask = CWWidth | CWHeight;
    changes.width = Glob->vsection_width;
    changes.height = Glob->vsection_height;
    
    XConfigureWindow(Glob->rdisplay, Glob->vsection_window,
		     change_mask, &changes);
    
    for (i = 0; i < N_VSECTION_BUTTONS; i++) {
      
      XMoveResizeWindow(Glob->rdisplay,
			Glob->vsection_button_frame[i]->x->drawable,
			buttonx[i], buttony,
			buttonwidth[i], buttonheight);
      GXSetGeomFrame(Glob->vsection_button_frame[i], buttonwidth[i],
		     buttonheight);
      
    }
    
    XMoveResizeWindow(Glob->rdisplay,
		      Glob->vsection_title_frame->x->drawable,
		      titlex, titley,
		      titlewidth, titleheight);
    GXSetGeomFrame(Glob->vsection_title_frame, titlewidth, titleheight);
    
    XMoveResizeWindow(Glob->rdisplay,
		      Glob->vsection_plot_frame->x->drawable,
		      plotx, ploty,
		      plotwidth, plotheight);
    GXSetGeomFrame(Glob->vsection_plot_frame, plotwidth, plotheight);
    
    /*
     * resize pixmap
     */
    
    XFreePixmap(Glob->rdisplay, Glob->vsection.pixmap);
    
    Glob->vsection.pixmap =
      XCreatePixmap(Glob->rdisplay,
		    Glob->vsection_window,
		    plotwidth, plotheight,
		    XDefaultDepth(Glob->rdisplay, Glob->rscreen));
    
  } /* if (first_call) */
  
  /*
   * debug output for X resource Id's
   */
  
  if (Glob->verbose) {
    
    fprintf(stderr, "vsection window = %ld\n", Glob->vsection_window);
    
    for (i = 0; i < N_VSECTION_BUTTONS; i++)
      fprintf(stderr, "vsection button window[%d] = %ld\n",
	      i, Glob->vsection_button_frame[i]->x->drawable);
    
    fprintf(stderr, "vsection title window = %ld\n",
	    Glob->vsection_title_frame->x->drawable);
    
    fprintf(stderr, "vsection plot window = %ld\n",
	    Glob->vsection_plot_frame->x->drawable);
    
    fprintf(stderr, "vsection pixmap = %ld\n",
	    Glob->vsection.pixmap);
    
    fprintf(stderr, "vsection plot window cursor = %ld\n",
	    plot_window_cursor);
    
  } /* if (Glob->verbose) */

}

