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
 * setup_rdist_windows.c - TimeHist routine
 *
 * compute time height profile window sizes, and set up the windows
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

void setup_rdist_windows(void)

{

  XTextProperty wname, iname;
  XSizeHints hint;
  XWindowChanges changes;
  ui32 change_mask;
  Window window;

  char *wstring, *istring;

  int i;
  int tmp_x;
  int title_x, title_y;
  int button_x[N_RDIST_BUTTONS], button_y;
  int plot_x, plot_y;
  int scale_x, scale_y;

  ui32 title_width;
  ui32 button_width[N_RDIST_BUTTONS], b_width, width_left;
  ui32 plot_width, plot_height;
  ui32 scale_height;

  static int first_call = TRUE;
  static si32 sub_border, main_border;
  static ui32 title_height, button_height;
  static ui32 scale_width;
  static ui32 min_height, min_width;

  static ui32 prev_width = 0;
  static ui32 prev_height = 0;

  if (Glob->verbose) {
    fprintf(stderr, "** setup_rdist_windows **\n");
  }

  /*
   * return now if window geometry has not changed
   */
  
  if (prev_width == Glob->x_rdist_width &&
      prev_height == Glob->x_rdist_height) {
    
    return;

  } else {
    
    /*
     * store dimensions locally for later comparison
     */

    prev_width = Glob->x_rdist_width;
    prev_height = Glob->x_rdist_height;

  }

  if (first_call == TRUE) {

    /*
     * get X resources
     */
  
    main_border = uGetParamLong(Glob->prog_name,
				"x_main_border_width", X_MAIN_BORDER_WIDTH);
    
    sub_border = uGetParamLong(Glob->prog_name,
			       "x_sub_border_width", X_SUB_BORDER_WIDTH);
    
    min_width = (ui32)
      uGetParamLong(Glob->prog_name,
		    "x_min_width", X_MIN_WIDTH);
    
    min_height = (ui32)
      uGetParamLong(Glob->prog_name,
		    "x_min_height", X_MIN_HEIGHT);
    
#ifdef TITLE_WINDOWS
    title_height = 2 * (Glob->x_title_font->ascent +
			Glob->x_title_font->descent);
#else
    title_height = 1;
#endif

    button_height = 2 * (Glob->x_button_font->ascent +
			 Glob->x_button_font->descent);
    
    scale_width = Glob->x_rdist_scale_width;

  }

  /*
   * compute window sizes
   */

  title_width = Glob->x_rdist_width - 2 * sub_border;

  plot_width = title_width - scale_width - 2 * sub_border;
  plot_height = Glob->x_rdist_height - title_height -
    button_height - 6 * sub_border;

  scale_height = plot_height;

  b_width = (title_width / N_RDIST_BUTTONS) - 2 * sub_border;
  width_left = Glob->x_rdist_width;
  for (i = 0; i < N_RDIST_BUTTONS; i++) {
    button_width[i] = b_width;
    width_left -= b_width + 2 * sub_border;
  }

  /*
   * distribute extra width in some of the buttons, and check that width
   * does not exceed maximum
   */

  for (i = 0; i < N_RDIST_BUTTONS; i++) {

    if(width_left > 0) {
      button_width[i]++;
      width_left--;
    }

    if (button_width[i] > Glob->x_max_button_width)
      button_width[i] = Glob->x_max_button_width;

  }
  
  /*
   * compute window positions relative to rdist main window
   */

  button_y = 0;
  tmp_x = Glob->x_rdist_width - 2 * sub_border;

  for (i = N_RDIST_BUTTONS - 1; i >= 0; i--) {
    button_x[i] = tmp_x - button_width[i];
    tmp_x -= (button_width[i] + 2 * sub_border);
  }

#ifdef TITLE_WINDOWS
  title_x = 0;
  title_y = button_y + button_height + 2 * sub_border;
  plot_x = 0;
  plot_y = title_y + title_height + 2 * sub_border;
#else 
  title_x = 0;
  title_y = button_y + button_height;
  plot_x = 0;
  plot_y = title_y + title_height;
#endif

  scale_x = plot_x + plot_width + 2 * sub_border;
  scale_y = plot_y;

  if (first_call == TRUE) {

    hint.x = Glob->x_rdist_x;
    hint.y = Glob->x_rdist_y;
    hint.width = Glob->x_rdist_width;
    hint.height = Glob->x_rdist_height;
    hint.min_width = min_width;
    hint.min_height = min_height;
    hint.flags = PPosition | PSize | PMinSize | USPosition | USSize;

    wstring = (char *) "Reflectivity distribution (RAL-NCAR)";
    istring = (char *) "Rdist";
    XStringListToTextProperty(&wstring, 1, &wname);
    XStringListToTextProperty(&istring, 1, &iname);

    /*
     * create simple window
     */

    Glob->rdist_window =
      XCreateSimpleWindow (Glob->rdisplay,
			   DefaultRootWindow(Glob->rdisplay),
			   Glob->x_rdist_x, Glob->x_rdist_y,
			   Glob->x_rdist_width, Glob->x_rdist_height,
			   main_border,
			   Glob->foreground, Glob->background);
    
    XSetWMProperties(Glob->rdisplay, Glob->rdist_window,
		     &wname, &iname,
		     Glob->argv, Glob->argc,
		     &hint, 0, 0);

    /*
     * button windows
     */

    for (i = 0; i < N_RDIST_BUTTONS; i++) {

      window =
	XCreateSimpleWindow(Glob->rdisplay,
			    Glob->rdist_window,
			    button_x[i], button_y,
			    button_width[i], button_height,
			    sub_border,
			    Glob->foreground, Glob->background);

      XSetWindowBorder(Glob->rdisplay, window, Glob->border_color);

      GXInitFrame(Glob->rdist_button_frame[i], Glob->rdisplay, window,
		  Glob->x_button_font);

      GXSetGeomFrame(Glob->rdist_button_frame[i],
		     button_width[i], button_height);

    } /* i */

     /*
     * title window
     */

    window =
      XCreateSimpleWindow(Glob->rdisplay,
			  Glob->rdist_window,
			  title_x, title_y,
			  title_width, title_height,
			  sub_border,
			  Glob->foreground, Glob->background);

    XSetWindowBorder(Glob->rdisplay, window, Glob->border_color);
    
    GXInitFrame(Glob->rdist_title_frame, Glob->rdisplay, window,
		Glob->x_title_font);

    GXSetGeomFrame(Glob->rdist_title_frame, title_width, title_height);

    /*
     * scale window
     */

    window =
      XCreateSimpleWindow(Glob->rdisplay,
			  Glob->rdist_window,
			  scale_x, scale_y,
			  scale_width, scale_height,
			  sub_border,
			  Glob->foreground, Glob->background);

    XSetWindowBorder(Glob->rdisplay, window, Glob->border_color);

    GXInitFrame(Glob->rdist_scale_frame, Glob->rdisplay, window,
		Glob->x_scale_font);

    GXSetGeomFrame(Glob->rdist_scale_frame, scale_width, scale_height);

    /*
     * plot window
     */

    window =
      XCreateSimpleWindow(Glob->rdisplay,
			  Glob->rdist_window,
			  plot_x, plot_y,
			  plot_width, plot_height,
			  sub_border,
			  Glob->foreground, Glob->background);
				      
    XSetWindowBorder(Glob->rdisplay, window, Glob->border_color);
    
    GXInitFrame(Glob->rdist_plot_frame, Glob->rdisplay, window,
		Glob->x_header_font);

    GXSetGeomFrame(Glob->rdist_plot_frame, plot_width, plot_height);

    /*
     * set input sensitivity of the windows
     */

    set_rdist_sens();

    first_call = FALSE;

  } else {

    /*
     * resize windows, recompute frame geometry
     */

    change_mask = CWWidth | CWHeight;
    changes.width = Glob->x_rdist_width;
    changes.height = Glob->x_rdist_height;

    XConfigureWindow(Glob->rdisplay, Glob->rdist_window,
		     change_mask, &changes);

    for (i = 0; i < N_RDIST_BUTTONS; i++) {

      XMoveResizeWindow(Glob->rdisplay,
			Glob->rdist_button_frame[i]->x->drawable,
			button_x[i], button_y,
			button_width[i], button_height);
      GXSetGeomFrame(Glob->rdist_button_frame[i],
		     button_width[i], button_height);

    }
  
    XMoveResizeWindow(Glob->rdisplay,
		      Glob->rdist_title_frame->x->drawable,
		      title_x, title_y,
		      title_width, title_height);
    GXSetGeomFrame(Glob->rdist_title_frame, title_width, title_height);
  
    XMoveResizeWindow(Glob->rdisplay,
		      Glob->rdist_scale_frame->x->drawable,
		      scale_x, scale_y,
		      scale_width, scale_height);
    GXSetGeomFrame(Glob->rdist_scale_frame, scale_width, scale_height);

    XMoveResizeWindow(Glob->rdisplay,
		      Glob->rdist_plot_frame->x->drawable,
		      plot_x, plot_y,
		      plot_width, plot_height);
    GXSetGeomFrame(Glob->rdist_plot_frame, plot_width, plot_height);

  } /* if (first_call == TRUE) */

  /*
   * debug output for X resource Id's
   */

  if (Glob->verbose == TRUE) {

    fprintf(stderr, "rdist window = %ld\n", Glob->rdist_window);

    for (i = 0; i < N_RDIST_BUTTONS; i++)
      fprintf(stderr, "rdist button window[%d] = %ld\n",
	      i, Glob->rdist_button_frame[i]->x->drawable);

    fprintf(stderr, "rdist title window = %ld\n",
	    Glob->rdist_title_frame->x->drawable);

    fprintf(stderr, "rdist plot window = %ld\n",
	    Glob->rdist_plot_frame->x->drawable);

    fprintf(stderr, "rdist scale window = %ld\n",
	    Glob->rdist_scale_frame->x->drawable);

  } /* if (Glob->verbose == TRUE) */

}
