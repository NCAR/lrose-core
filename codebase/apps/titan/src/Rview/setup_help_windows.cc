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
 * setup_help_windows.c
 *
 * compute window sizes for help, and set up the windows
 *
 * RAP, NCAR, Boulder CO
 *
 * May 1992
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "Rview.hh"
using namespace std;

void setup_help_windows()

{
  
  XSizeHints hint;
  XWindowChanges changes;
  Window window;

  int i;
  int titlex, titley;
  int tmp_x, buttonx[N_HELP_BUTTONS], buttony;
  int textx, texty;
  ui32 change_mask;
  ui32 titlewidth;
  ui32 buttonwidth[N_HELP_BUTTONS], bwidth, width_left;
  ui32 textwidth, textheight;
  
  static si32 subborder, mainborder;
  static ui32 titleheight, buttonheight;
  static ui32 minmainheight, minmainwidth;
  static int first_call = TRUE;
  
  static ui32 help_width = 0;
  static ui32 help_height = 0;

  if (Glob->debug) {
    fprintf(stderr, "** setup_help_windows **\n");
  }

  /*
   * return now if window geometry has not changed
   */

  if (help_width == Glob->help_width &&
      help_height == Glob->help_height) {

    return;

  } else {
      
    /*
     * store dimensions locally for later comparison
     */

    help_width = Glob->help_width;
    help_height = Glob->help_height;

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
    
    titleheight = 2 * (Glob->x_title_font->ascent +
		       Glob->x_title_font->descent);

    buttonheight = 2 * (Glob->x_button_font->ascent +
			Glob->x_button_font->descent);
    
  } /* if (first_call) */
  
  /*
   * compute window sizes
   */
  
  textwidth = Glob->help_width - 2 * subborder;
  textheight = (Glob->help_height - titleheight - buttonheight
		- 6 * subborder);
  
  titlewidth = textwidth;
  
  bwidth = (Glob->help_width / N_HELP_BUTTONS) - 2 * subborder;
  width_left = Glob->help_width;
  
  for (i = 0; i < N_HELP_BUTTONS; i++) {
    buttonwidth[i] = bwidth;
    width_left -= bwidth + 2 * subborder;
  }
  
  /*
   * distribute extra width in some of the buttons, and check that width
   * does not exceed maximum
   */
  
  for (i = 0; i < N_HELP_BUTTONS; i++) {
    
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
  tmp_x = Glob->help_width - 2 * subborder;
  
  for (i = N_HELP_BUTTONS - 1; i >= 0; i--) {
    buttonx[i] = tmp_x - buttonwidth[i];
    tmp_x -= (buttonwidth[i] + 2 * subborder);
  }
  
  titlex = 0;
  titley = buttony + buttonheight + 2 * subborder;
  
  textx = 0;
  texty = titley + titleheight + 2 * subborder;
  
  if (first_call) {
    
    /*
     * set up text frame
     */
    
    hint.x = Glob->help_x;
    hint.y = Glob->help_y;
    hint.width = Glob->help_width;
    hint.height = Glob->help_height;
    hint.min_width = minmainwidth;
    hint.min_height = minmainheight;
    hint.flags = PPosition | PSize | PMinSize;
    
    /*
     * create help window
     */
    
    Glob->help_window =
      XCreateSimpleWindow (Glob->rdisplay,
			   DefaultRootWindow(Glob->rdisplay),
			   Glob->help_x, Glob->help_y,
			   Glob->help_width, Glob->help_height,
			   mainborder, Glob->foreground,
			   Glob->background);
    
    XSetStandardProperties(Glob->rdisplay, Glob->help_window,
			   "Rview Help",
			   "Rv_help",
			   None, Glob->argv, Glob->argc, &hint);
    
    /*
     * create subwindows
     */
    
    /*
     * button windows
     */
    
    for (i = 0; i < N_HELP_BUTTONS; i++) {
      
      window = XCreateSimpleWindow(Glob->rdisplay,
				   Glob->help_window,
				   buttonx[i], buttony,
				   buttonwidth[i], buttonheight,
				   subborder, Glob->foreground,
				   Glob->background);
      
      XSetWindowBorder(Glob->rdisplay, window, Glob->border_color);
      
      GXInitFrame(Glob->help_button_frame[i], Glob->rdisplay, window,
		  Glob->x_button_font);
      
      GXSetGeomFrame(Glob->help_button_frame[i], buttonwidth[i],
		     buttonheight);
      
    } /* i */
    
    /*
     * title window
     */
    
    window = XCreateSimpleWindow(Glob->rdisplay,
				 Glob->help_window,
				 titlex, titley,
				 titlewidth, titleheight,
				 subborder,
				 Glob->foreground, Glob->background);
    
    XSetWindowBorder(Glob->rdisplay, window, Glob->border_color);
    
    GXInitFrame(Glob->help_title_frame, Glob->rdisplay, window,
		Glob->x_title_font);
    
    GXSetGeomFrame(Glob->help_title_frame, titlewidth, titleheight);
    
    /*
     * text window
     */
    
    window = XCreateSimpleWindow(Glob->rdisplay,
				 Glob->help_window,
				 textx, texty,
				 textwidth, textheight,
				 subborder,
				 Glob->foreground, Glob->background);
    
    XSetWindowBorder(Glob->rdisplay, window, Glob->border_color);
    
    GXInitFrame(Glob->help_text_frame, Glob->rdisplay, window,
		Glob->x_text_font);
    
    GXSetGeomFrame(Glob->help_text_frame, textwidth, textheight);
    
    first_call = FALSE;
    
  } else {
    
    /*
     * resize windows, recompute frame geometry
     */
    
    change_mask = CWWidth | CWHeight;
    changes.width = Glob->help_width;
    changes.height = Glob->help_height;
    
    XConfigureWindow(Glob->rdisplay, Glob->help_window,
		     change_mask, &changes);
    
    for (i = 0; i < N_HELP_BUTTONS; i++) {
      
      XMoveResizeWindow(Glob->rdisplay,
			Glob->help_button_frame[i]->x->drawable,
			buttonx[i], buttony,
			buttonwidth[i], buttonheight);
      GXSetGeomFrame(Glob->help_button_frame[i], buttonwidth[i],
		     buttonheight);
      
    }
    
    XMoveResizeWindow(Glob->rdisplay,
		      Glob->help_title_frame->x->drawable,
		      titlex, titley,
		      titlewidth, titleheight);
    GXSetGeomFrame(Glob->help_title_frame, titlewidth, titleheight);
    
    XMoveResizeWindow(Glob->rdisplay,
		      Glob->help_text_frame->x->drawable,
		      textx, texty,
		      textwidth, textheight);
    GXSetGeomFrame(Glob->help_text_frame, textwidth, textheight);
    
  } /* if (first_call) */
  
  /*
   * debug output for X resource Id's
   */
  
  if (Glob->debug) {
    
    fprintf(stderr, "help window = %ld\n", Glob->help_window);
    
    for (i = 0; i < N_HELP_BUTTONS; i++)
      fprintf(stderr, "help button window[%d] = %ld\n",
	      i, Glob->help_button_frame[i]->x->drawable);
    
    fprintf(stderr, "help title window = %ld\n",
	    Glob->help_title_frame->x->drawable);
    
    fprintf(stderr, "help text window = %ld\n",
	    Glob->help_text_frame->x->drawable);
    
  } /* if (Glob->debug) */
  
}

