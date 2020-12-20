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
 * help.c
 *
 * help routines
 * 
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * May 1992
 *
 *************************************************************************/

#include "Rview.hh"
using namespace std;

static void print_text(gframe_t *frame,
		       char *text);

/************************************************************************
 * draw_help_title()
 *
 */

void draw_help_title()
{

  char tstring[BUFSIZ];
  gframe_t *frame;

  if (Glob->verbose) {
    fprintf(stderr, "** draw_help_title **\n");
  }

  frame = Glob->help_title_frame;

  /*
   * clear window
   */

  XSetWindowBackground(Glob->rdisplay, frame->x->drawable,
		       Glob->background);
  XClearWindow(Glob->rdisplay, frame->x->drawable);

  sprintf(tstring, "%s",
	  "RVIEW HELP - click in desired window, "
	  "CLOSE when done");

  GDrawString(XDEV, frame, frame->x->gc, frame->x->font,
	      frame->psgc, XJ_CENTER, YJ_CENTER,
	      (double) 0.5, (double) 0.5, tstring);

  XFlush(Glob->rdisplay);

}

/************************************************************************
 * draw_help_button()
 *
 *************************************************************************/

void draw_help_button(si32 n,
		      ui32 background)

{

  static si32 first_call = TRUE;
  static char **button_label;
  char *label_str;
  int i;
  gframe_t *frame;

  if (Glob->verbose) {
    fprintf(stderr, "** draw_help_button **\n");
  }

  frame = Glob->help_button_frame[n];

  /*
   * if first call, load up the label strings
   */

  if (first_call) {

    button_label = (char **) umalloc (N_HELP_BUTTONS * sizeof(char *));

    label_str = (char *) umalloc((ui32) (strlen(HELP_BUTTON_LABELS) + 1));
    strcpy(label_str, HELP_BUTTON_LABELS);

    for (i = 0; i < N_HELP_BUTTONS; i++) {

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
 * draw_help_text()
 *
 * Display help text
 *
 *************************************************************************/

void draw_help_text()

{

  char text[BUFSIZ];
  char *printer;
  gframe_t *frame;
  si32 code;

  if (Glob->verbose) {
    fprintf(stderr, "** draw_help_text **\n");
  }

  /*
   * set local variables
   */

  frame = Glob->help_text_frame;
  code = Glob->help_code;

  XRaiseWindow(Glob->rdisplay, Glob->help_window);

  switch (code) {

  case HELP_CAPPI_PLOT:

    if (Glob->use_time_hist)
      sprintf(text, "%s",
	      "RVIEW CAPPI WINDOW OPTIONS\n \n"
	      "  LEFT   BUTTON - drag to zoom\n"
	      "  MIDDLE BUTTON - drag to pan\n"
	      "  RIGHT  BUTTON - drag for vertical section\n"
	      "Cancel the above by ending close to the start point.\n \n"
	      "A click without moving has the following results:\n"
	      "  LEFT   BUTTON - print cursor value and posn in title bar.\n"
	      "                  Range and bearing is from origin.\n"
	      "  MIDDLE BUTTON - cancel cursor printout\n"
	      "  RIGHT  BUTTON - print cursor value and posn in title bar.\n"
	      "                  Range and bearing is from prev point.\n \n"
	      "TRACK SELECTION for time_hist\n"
	      "  Double-click LEFT   - select simple track\n"
	      "  Double-click MIDDLE - select partial track\n"
	      "  Double-click RIGHT  - select entire complex track \n \n"
	      "The selection may also be made by clicking SELECT in\n"
	      "the time-scale window, and then single-clicking in the main\n"
	      "plot window, using LEFT and RIGHT as above.");
    else
      sprintf(text, "%s",
	      "RVIEW CAPPI WINDOW OPTIONS\n \n"
	      "  LEFT   BUTTON - drag to zoom\n"
	      "  MIDDLE BUTTON - drag to pan\n"
	      "  RIGHT  BUTTON - drag for vertical section\n \n"
	      "Cancel the above by ending close to the start point.\n \n"
	      "TRACK SELECTION for time_hist\n"
	      "  Double-click LEFT  - select simple track\n"
	      "  Double-click RIGHT - select entire complex track.");
    
    print_text(frame, text);
    
    break;

  case HELP_VSECTION_PLOT:

    sprintf(text, "%s",
	    "RVIEW VERTICAL SECTION WINDOW OPTIONS\n \n"
	    "  Double-click ANY - move altitude to desired position.");
    
    print_text(frame, text);
    
    break;

  case CAPPI_HELP:

    if (Glob->use_time_hist)
      sprintf(text, "%s",
	      "HELP - PROVIDE THIS WINDOW\n \n"
	      "To obtain help on any rview action, click the\n"
	      "right button in the appropriate window.\n \n"
	      "For time_hist help, select HELP on the time_scale window.\n \n"
	      "Click on CLOSE when you are done.");
    else
      sprintf(text, "%s",
	      "HELP - PROVIDE THIS WINDOW\n \n"
	      "To obtain help on any rview action, click the\n"
	      "right button in the appropriate window.\n \n"
	      "Click on CLOSE when you are done.");

    print_text(frame, text);
    
    break;
    
  case CAPPI_LEVEL:
    
    sprintf(text, "%s%s%g%s%s%s%g%s",
	    "LEVEL - CHANGE CAPPI LEVEL\n \n",
	    "  LEFT   BUTTON - down by ", Glob->delta_z, " km\n",
	    "  MIDDLE BUTTON - composite plot (max from all heights)\n",
	    "  RIGHT  BUTTON - up by ", Glob->delta_z, " km");

    print_text(frame, text);
    
    break;
    
  case CAPPI_FIELD:
    
    sprintf(text, "%s",
	    "FIELD - CHANGE IMAGE FIELD\n \n"
	    "  LEFT   BUTTON - down a field\n"
	    "  MIDDLE BUTTON - change to first field\n"
	    "  RIGHT  BUTTON - up a field");

    print_text(frame, text);
    
    break;  /* end of field change actions */
    
  case CAPPI_ZOOM:
    
    sprintf(text, "%s",
	    "ZOOM - CHANGE ZOOM STATE\n"
	    "There are 3 zoom states, and you may move between them\n \n"
	    "  LEFT   BUTTON - zoom out (if applicable)\n"
	    "  MIDDLE BUTTON - cancel all zooms\n"
	    "  RIGHT  BUTTON - zoom in (if applicable)\n \n"
	    "For help on zooming, click in main cappi window.");

    print_text(frame, text);
    
    break;
    
  case CAPPI_CONT:
    
    sprintf(text, "%s",
	    "CONT - CONTOUR OPTION\n \n"
	    "  LEFT   BUTTON - contours off\n"
	    "  MIDDLE BUTTON - contours on, labels off\n"
	    "  RIGHT  BUTTON - contours on, labels on");

    print_text(frame, text);
    
    break;
    
  case CAPPI_IMAGE:
    
    sprintf(text, "%s",
	    "IMAGE - IMAGE PLOT OPTION\n \n"
	    "  LEFT   BUTTON - image off\n"
	    "  MIDDLE BUTTON - image on filled contours\n"
	    "  RIGHT  BUTTON - image on");
    
    print_text(frame, text);
    
    break;
    
  case CAPPI_RINGS:
    
    sprintf(text, "%s",
	    "RINGS - RANGE RINGS OPTION\n \n"
	    "  LEFT   BUTTON - range rings off\n"
	    "  RIGHT  BUTTON - range rings on");

    print_text(frame, text);
    
    break;
    
  case CAPPI_MAPS:
    
    sprintf(text, "%s",
	    "MAPS - MAPS OVERLAY OPTION\n \n"
	    "  LEFT   BUTTON - no maps\n"
	    "  MIDDLE BUTTON - limited maps\n"
	    "  RIGHT  BUTTON - all maps");

    print_text(frame, text);
    
    break;
    
  case CAPPI_TRACKS:

    if (Glob->use_time_hist)
      sprintf(text, "%s\n",
	      "TRACKS - TRACK PLOT OPTION\n \n"
	      "  LEFT   BUTTON - no tracks plotted\n"
	      "  MIDDLE BUTTON - only selected track plotted\n"
	      "  RIGHT  BUTTON - all tracks plotted\n \n"
	      "SELECTION - click in main window as follows :\n"
	      "  Double-click LEFT  - select simple track\n"
	      "  Double-click RIGHT - select entire complex track \n \n"
	      "The selection may also be made by clicking SELECT in\n"
	      "the time-scale window, and then single-clicking in the main\n"
	      "plot window, using LEFT and RIGHT as above.");
    else
      sprintf(text, "%s\n",
	      "TRACKS - TRACK PLOT OPTION\n \n"
	      "  LEFT   BUTTON - no tracks plotted\n"
	      "  MIDDLE BUTTON - only selected track plotted\n"
	      "  RIGHT  BUTTON - all tracks plotted\n \n"
	      "SELECTION - click in main window as follows :\n"
	      "  Double-click LEFT  - select simple track\n"
	      "  Double-click RIGHT - select entire complex track");

    print_text(frame, text);
    
    break;
    
  case CAPPI_TRACK_GRAPHIC:
    
    sprintf(text, "%s",
	    "TTYPE - TRACK GRAPHIC TYPE\n \n"
	    "  LEFT   BUTTON - toggle vectors on/off\n"
	    "  MIDDLE BUTTON - toggle shape ellipses/polygons/none\n"
	    "  RIGHT  BUTTON - toggle fill on/off\n");

    print_text(frame, text);
    
    break;
    
  case CAPPI_TRACK_ANNOTATE:
    
    sprintf(text, "%s",
	    "ANNOT - TRACK ANNOTATION\n \n"
	    "  LEFT   BUTTON - decrease annotation selection\n"
	    "  MIDDLE BUTTON - no annotation\n"
	    "  RIGHT  BUTTON - increase annotation selection\n"
	    "Annotation options are:\n"
	    "  None\n"
	    "  Speed (km/hr)"
	    "  Max dbz\n"
	    "  Vil\n"
	    "  Tops (km)\n"
	    "  Track number\n");

    print_text(frame, text);
    
    break;
    
  case CAPPI_PAST:
    
    sprintf(text, "%s%g%s",
	    "PAST - OPTION TO PLOT PAST DATA\n \n"
	    "  LEFT   BUTTON - no past plotted\n"
	    "  MIDDLE BUTTON - ",
	    Glob->past_plot_period / 3600.0, " hr past plotted\n"
	    "  RIGHT  BUTTON - all avaiable past plotted\n");

    print_text(frame, text);
    
    break;
    
  case CAPPI_FORECAST:
    
    sprintf(text, "%s%g%s%ld",
	    "FCAST - OPTION TO PLOT FORECAST DATA\n \n"
	    "  LEFT   BUTTON - no forecast plotted\n"
	    "  MIDDLE BUTTON - forecast plotted from present only\n"
	    "  RIGHT  BUTTON - forecast plotted at all scan times\n \n"
	    "Forecast interval        : ",
	    Glob->forecast_interval / 60.0, " min\n"
	    "Number of forecast steps : ", (long) Glob->n_forecast_steps);

    print_text(frame, text);
    
    break;
    
  case CAPPI_FUTURE:
    
    sprintf(text, "%s%g%s",
	    "FUTURE - OPTION TO PLOT FUTURE DATA\n \n"
	    "  LEFT   BUTTON - no future plotted\n"
	    "  MIDDLE BUTTON - ",
	    Glob->future_plot_period / 3600.0, " hr future plotted\n"
	    "  RIGHT  BUTTON - all avaiable future plotted\n");

    print_text(frame, text);
    
    break;
    
  case CAPPI_TIME:
    
    sprintf(text, "%s%s%g%s%s%g%s",
	    "TIME - CHANGE PLOT TIME\n \n",
	    "  LEFT   BUTTON - ahead by ",
	    Glob->scan_delta_t / 60.0, " min\n",
	    "  RIGHT  BUTTON - back  by ",
	    Glob->scan_delta_t / 60.0, " min\n");

    print_text(frame, text);
    
    break;
    
  case CAPPI_COPY:

    printer = uGetParamString(Glob->prog_name,
			    "ps_printer", PS_PRINTER),

    sprintf(text, "%s%s%s",
	    "COPY - MAKE HARD COPY\n \n"
	    "  LEFT  BUTTON - print hard copy to printer ",
	    printer, "\n"
	    "  MIDDLE BUTTON - print copy of window to web file\n"
	    "  RIGHT BUTTON - print copy of window to time-named gif file\n");

    print_text(frame, text);
    
    break;
    
  case CAPPI_QUIT:
    
    sprintf(text, "%s",
	    "QUIT\n \n"
	    "  ANY BUTTON - quit rview\n");
    
    print_text(frame, text);
    
    break;
    
  case VSECTION_INTERP:
    
    sprintf(text, "%s",
	    "INTERPOLATION - interpolate vert section\n \n"
	    "  LEFT   BUTTON - interp off\n"
	    "  RIGHT  BUTTON - interp on");
    
    print_text(frame, text);
    
    break;
    
  case VSECTION_IMAGE:
    
    sprintf(text, "%s",
	    "IMAGE - IMAGE PLOT OPTION\n \n"
	    "  LEFT   BUTTON - image off\n"
	    "  MIDDLE BUTTON - image on filled contours\n"
	    "  RIGHT  BUTTON - image on");
    
    print_text(frame, text);
    
    break;
    
  case VSECTION_CONT:
    
    sprintf(text, "%s",
	    "CONT - CONTOUR OPTION\n \n"
	    "  LEFT   BUTTON - contours off\n"
	    "  MIDDLE BUTTON - contours on, labels off\n"
	    "  RIGHT  BUTTON - contours on, labels on");

    print_text(frame, text);
    
    break;
    
  case VSECTION_COPY:
    
    printer = uGetParamString(Glob->prog_name,
			    "ps_printer", PS_PRINTER),
    
    sprintf(text, "%s%s%s",
	    "COPY - MAKE HARD COPY\n \n"
	    "  LEFT  BUTTON - print hard copy to printer ",
	    printer, "\n"
	    "  RIGHT BUTTON - print copy of window to xwd file\n");
    
    print_text(frame, text);
    
    break;
    
  case VSECTION_CLOSE:
    
    sprintf(text, "%s",
	    "CLOSE\n \n"
	    "  ANY BUTTON - close vertical section\n");
    
    print_text(frame, text);
    
    break;

  case HELP_CLOSE:
    
    XWithdrawWindow(Glob->rdisplay, Glob->help_window,
                    Glob->rscreen);
    
    Glob->help = FALSE;
    break;

  } /* switch */
  
  XFlush(Glob->rdisplay);

}

/**********************************************************************
 * print_text()
 *
 * Places the text in the help text window
 */

static void print_text(gframe_t *frame,
		       char *text)

{

  static int first_call = TRUE;
  static double text_margin_x, text_margin_y;
  static double text_line_spacing;

  char *line;
  int direction, ascent, descent;
  double textx, texty;
  XCharStruct overall;


  if (first_call) {

    /*
     * get margins and line spacing
     */

    text_margin_x =
      uGetParamDouble(Glob->prog_name,
		    "x_text_margin",
		    X_TEXT_MARGIN) / frame->x->xscale;
    
    text_margin_y =
      uGetParamDouble(Glob->prog_name,
		    "x_text_margin",
		    X_TEXT_MARGIN) / frame->x->yscale;

    XQueryTextExtents(Glob->rdisplay,
		      Glob->x_text_font->fid,
		      "AaGgJjWwQqYyZz",
		      14,
		      &direction, &ascent, &descent, &overall);

    text_line_spacing = ((double) (ascent + descent) * 1.4 /
			 frame->x->yscale);

  }

  XClearWindow(Glob->rdisplay, frame->x->drawable);

  line = strtok(text, "\n\0");

  textx = frame->w_xmin + text_margin_x;
  texty = frame->w_ymax - text_margin_y;
  
  while (line != NULL) {
    
    GDrawString(XDEV, frame, Glob->text_gc, Glob->x_text_font,
		frame->psgc, XJ_LEFT, YJ_BELOW,
		textx, texty,line);
    
    texty -= text_line_spacing;
    
    line = strtok((char *) NULL, "\n\0");

  } /* while */

}
