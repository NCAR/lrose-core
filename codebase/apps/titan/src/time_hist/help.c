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
 * help.c
 *
 * help routines
 * 
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * May 1992
 *
 *************************************************************************/

#define X11R3 

#include "time_hist.h"

static void print_text(gframe_t *frame, char *text);

/************************************************************************
 * draw_help_title()
 *
 */

void draw_help_title(void)

{

  char tstring[BUFSIZ];
  gframe_t *frame;

  if (Glob->debug) {
    fprintf(stderr, "** draw_help_title **\n");
  }

  frame = Glob->help_title_frame;

  /*
   * clear window
   */

  XSetWindowBackground(Glob->rdisplay, frame->x->drawable,
		       Glob->background);
  safe_XClearWindow(Glob->rdisplay, frame->x->drawable);

  sprintf(tstring, "%s%s",
	  "TIME_HIST HELP - click in desired window, ",
	  "CLOSE when done");

  GDrawString(XDEV, frame, frame->x->gc, frame->x->font,
	      frame->psgc, XJ_CENTER, YJ_CENTER,
	      (double) 0.5, (double) 0.5, tstring);

  safe_XFlush(Glob->rdisplay);

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

  if (Glob->debug) {
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
  safe_XClearWindow(Glob->rdisplay, frame->x->drawable);

  /*
   * write the label to the window
   */

  GDrawString(XDEV, frame, frame->x->gc, frame->x->font,
	      frame->psgc, XJ_CENTER, YJ_CENTER,
	      (double) 0.5, (double) 0.5, button_label[n]);

  safe_XFlush(Glob->rdisplay);

}

/************************************************************************
 * draw_help_text()
 *
 * Display help text
 *
 *************************************************************************/

void draw_help_text(void)

{

  char text[BUFSIZ];
  char *printer;
  gframe_t *frame;
  int code;

  if (Glob->debug) {
    fprintf(stderr, "** draw_help_text **\n");
  }

  /*
   * set local variables
   */

  frame = Glob->help_text_frame;
  code = Glob->help_code;

  XRaiseWindow(Glob->rdisplay, Glob->help_window);

  switch (code) {

  case HELP_PLOT:

    sprintf(text, "%s",
	    "CHANGE TIME FRAME\n \n"
	    "  ANY BUTTON - Double click to change time\n"
	    "  If realtime mode, click on NOW to get current data");
    
    print_text(frame, text);
    
    break;

  case TSCALE_HELP:

    sprintf(text, "%s",
	    "HELP - PROVIDE THIS WINDOW\n \n"
	    "To obtain help on any time_hist action, click any\n"
	    "mouse button in the appropriate window.\n \n"
	    "For rview help, select HELP on the cappi window.\n \n"
	    "Click on CLOSE when you are done.");
    
    print_text(frame, text);
    
    break;
    
  case TSCALE_SELECT:
    
    sprintf(text, "%s",
	    "SELECT TRACKS\n \n"
	    "  Click on SELECT with any button\n"
	    "  Then, move into cappi window, and click on the\n"
	    "  track of interest, as follows:\n \n"
	    "    LEFT   BUTTON - select simple track\n"
	    "    MIDDLE BUTTON - select partial track\n"
	    "    RIGHT  BUTTON - select complex track");

    print_text(frame, text);
    
    break;
    
  case TSCALE_TRACK_SET:
    
    sprintf(text, "%s",
	    "CHANGE TRACK SET SELECTION\n \n"
	    "  LEFT   BUTTON - tracks at current time\n"
	    "  RIGHT  BUTTON - all tracks during ops period");

    print_text(frame, text);
    
    break;
    
  case TSCALE_DATE:
    
    sprintf(text, "%s",
	    "CHANGE PLOT DATE\n \n"
	    "  LEFT   BUTTON - back by a day\n"
	    "  RIGHT  BUTTON - forward by a day");

    print_text(frame, text);
    
    break;
    
  case TSCALE_TIME:
    
    sprintf(text, "%s",
	    "CHANGE PLOT TIME\n \n"
	    "  LEFT   BUTTON - back one scan \n\n"
	    "  RIGHT  BUTTON - ahead by one scan \n\n");

    print_text(frame, text);
    
    break;
    
  case TSCALE_NOW:
    
    sprintf(text, "%s",
	    "CHANGE TO CURRENT DATA\n \n"
	    "  ANY    BUTTON - change to current time");

    print_text(frame, text);
    
    break;
    
  case TSCALE_QUIT:
    
    sprintf(text, "%s",
	    "QUIT\n \n"
	    "  ANY BUTTON - quit time_hist\n");
    
    print_text(frame, text);
    
    break;
    
  case THIST_VOL:
    
    sprintf(text, "%s",
	    "VOLUME PLOT OPTION\n \n"
	    "  LEFT   BUTTON - off\n"
	    "  RIGHT  BUTTON - on ");

    print_text(frame, text);
    
    break;
    
  case THIST_AREA:

    sprintf(text, "%s",
	    "AREA PLOT OPTION\n \n"
	    "  LEFT   BUTTON - off\n"
	    "  RIGHT  BUTTON - on ");

    print_text(frame, text);
    
    break;
    
  case THIST_PFLUX:
    
    sprintf(text, "%s",
	    "PRECIP FLUX PLOT OPTION\n \n"
	    "  LEFT   BUTTON - off\n"
	    "  RIGHT  BUTTON - on ");

    print_text(frame, text);
    
    break;
    
  case THIST_MASS:
    
    sprintf(text, "%s",
	    "MASS PLOT OPTION\n \n"
	    "  LEFT   BUTTON - off\n"
	    "  RIGHT  BUTTON - on ");

    print_text(frame, text);
    
    break;
    
  case THIST_FORECAST:
    
    sprintf(text, "%s%g%s%ld",
	    "FCAST - OPTION TO PLOT FORECAST DATA\n \n"
	    "  LEFT   BUTTON - no forecast plotted\n"
	    "  MIDDLE BUTTON - forecast plotted from present only\n"
	    "  RIGHT  BUTTON - forecast plotted at all scan times\n \n"
	    "    Forecast interval        : ",
	    Glob->track_shmem->forecast_interval / 60.0, " min\n"
	    "    Number of forecast steps : ",
	    (long) Glob->track_shmem->n_forecast_steps);

    print_text(frame, text);
    
    break;
    
  case THIST_FIT:
    
    sprintf(text, "%s",
	    "OPTION TO FIT GAUSSIAN TO TIME HISTORIES\n \n"
	    "  LEFT BUTTON - Fit OFF\n \n"
	    "  RIGHT BUTTON - Fit ON\n");
    
    print_text(frame, text);
    
    break;

  case THIST_COPY:
    
    printer = xGetResString(Glob->rdisplay, Glob->prog_name,
			    "ps_printer", PS_PRINTER),

    sprintf(text, "%s%s%s",
	    "COPY - MAKE HARD COPY\n \n"
	    "  ANY BUTTON - print time history to printer ",
	    printer, "\n");

    print_text(frame, text);
    
    break;
    
  case TIMEHT_MAXZ:
    
    sprintf(text, "%s",
	    "TIME HEIGHT PROFILE - MAX REFLECTIVITY\n \n"
	    "  LEFT   BUTTON - no data - height lines only if selected\n"
	    "  RIGHT  BUTTON - max reflectivity");

    print_text(frame, text);
    
    break;
    
  case TIMEHT_MEANZ:

    sprintf(text, "%s",
	    "TIME HEIGHT PROFILE - MEAN REFLECTIVITY\n \n"
	    "  LEFT   BUTTON - no data - height lines only if selected\n"
	    "  RIGHT  BUTTON - mean reflectivity");
    
    print_text(frame, text);
    
    break;
    
  case TIMEHT_MASS:
    
    sprintf(text, "%s",
	    "TIME HEIGHT PROFILE - MASS\n \n"
	    "  LEFT   BUTTON - no data - height lines only if selected\n"
	    "  RIGHT  BUTTON - mass");

    print_text(frame, text);
    
    break;
    
  case TIMEHT_VORTICITY:
    
    sprintf(text, "%s",
	    "TIME HEIGHT PROFILE - VORTICITY\n \n"
	    "  LEFT   BUTTON - no data - height lines only if selected\n"
	    "  RIGHT  BUTTON - vorticity");

    print_text(frame, text);
    
    break;
    
  case TIMEHT_CENTROIDS:
    
    sprintf(text, "%s%s%s%s",
	    "VOLUME- AND REFL-WEIGHTED CENTROIDS\n \n",
	    "  LEFT   BUTTON - centroids off\n",
	    "  RIGHT  BUTTON - only Z-wt centroid on\n",
	    "  RIGHT  BUTTON - both vol and Z-wt centroids on");

    print_text(frame, text);
    
    break;
    
  case TIMEHT_HTMAXZ:
    
    sprintf(text, "%s",
	    "HEIGHT TRACE OF MAX REFLECTIVITY\n \n"
	    "  LEFT   BUTTON - height trace off\n"
	    "  MIDDLE BUTTON - height trace on\n"
	    "  RIGHT  BUTTON - height trace on, values plotted");

    print_text(frame, text);
    
    break;
    
  case TIMEHT_COPY:
    
    printer = xGetResString(Glob->rdisplay, Glob->prog_name,
			    "ps_printer", PS_PRINTER),

    sprintf(text, "%s%s%s",
	    "COPY - MAKE HARD COPY\n \n"
	    "  ANY BUTTON - print time-height profile to printer ",
	    printer,
	    "\n");

    print_text(frame, text);
    
    break;

  case RDIST_VOL:
    
    sprintf(text, "%s",
	    "CUMULATIVE VOLUME HISTOGRAM - % Volume vs, Reflectivity\n \n"
	    "  ANY BUTTON - volume histogram on\n");
    
    print_text(frame, text);
    
    break;

  case RDIST_AREA:
    
    sprintf(text, "%s",
	    "CUMULATIVE AREA HISTOGRAM - % Area vs, Reflectivity\n \n"
	    "  ANY BUTTON - area histogram on\n");
    
    print_text(frame, text);
    
    break;

  case RDIST_FLIP:
    
    sprintf(text, "%s",
	    "FLIP DATA REPRESENTATION\n \n"
	    "  ANY BUTTON - Flip vertical plotting\n");
    
    print_text(frame, text);
    
    break;

  case RDIST_FIT:
    
    sprintf(text, "%s",
	    "OPTION TO FIT DISTRIBUTION TO HISTOGRAM\n \n"
	    "  LEFT BUTTON - Fit OFF\n \n"
	    "  RIGHT BUTTON - Fit ON\n");
    
    print_text(frame, text);
    
    break;

  case RDIST_COPY:
    
    printer = xGetResString(Glob->rdisplay, Glob->prog_name,
			    "ps_printer", PS_PRINTER),

    sprintf(text, "%s%s%s",
	    "COPY - MAKE HARD COPY\n \n"
	    "  ANY BUTTON - print refl distribution to printer ",
	    printer,
	    "\n");

    print_text(frame, text);
    
    break;

  case HELP_CLOSE:
    
#ifdef X11R3
    XWithdrawWindow(Glob->rdisplay, Glob->help_window,
		    Glob->rscreen);
#else
    XIconifyWindow(Glob->rdisplay, Glob->help_window,
		   Glob->rscreen);
#endif
    
    Glob->help = FALSE;
    break;

  } /* switch */
  
  safe_XFlush(Glob->rdisplay);
  Glob->help_status = CURRENT;

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
      xGetResDouble(Glob->rdisplay, Glob->prog_name,
		    "x_text_margin",
		    X_TEXT_MARGIN) / frame->x->xscale;
    
    text_margin_y =
      xGetResDouble(Glob->rdisplay, Glob->prog_name,
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

  safe_XClearWindow(Glob->rdisplay, frame->x->drawable);

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
