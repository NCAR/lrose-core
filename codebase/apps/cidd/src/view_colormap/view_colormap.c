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
/*********************************************************************
 * view_colormap.c: view the colors in the colormap
 *
 * RAP, NCAR, Boulder CO
 *
 * march 1991 
 *
 * Mike Dixon
 *
 *********************************************************************/

/*
 * includes 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <sys/types.h>

/*
 * defines
 */

#define UPDATE_INTERVAL 5

#define RECTANGLE_SIZE (u_int) 25
#define MAINBORDER 2
#define MAINX 50
#define MAINY 50

#define BUTTON_FONT "6x10"
#define LABEL_FONT "6x10"

void main(argc, argv)
     int argc;
     char **argv;

{

  char *display_name;
  
  int rscreen;
  Display *rdisplay;
  Window main_window;
  GC fill_gc;      
  
  int mainx = MAINX;
  int mainy = MAINY;
  unsigned int mainwidth;
  unsigned int mainheight;
  
  XFontStruct *button_font;
  XFontStruct *label_font;
  XSizeHints hint;
  

  unsigned long irow, icol, icolor;
  unsigned long ncolors, nside;
  int x, y;
  
  int geometrystatus;
  int default_depth;
  char usage[2048];

  int i;

  /*
   * load up usage string
   */

  sprintf(usage, "%s%s%s%s%s%s%s",
	  "Usage:\n\n", argv[0], " [options] as below:\n\n",
	  "       [ -d, -display name] display name\n",
	  "       [ -g, -geometry geom] geometry as per X manual\n",
	  "       [ --, -help, -man] produce this list.\n",
	  "\n");

  /*
   * parse the command line arguments
   */

  display_name = NULL;

  for (i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      printf("%s", usage);
      exit(0);
    }
	
    if (i < argc - 1) {
      
      if (!strcmp(argv[i], "-display") || !strcmp(argv[i], "-d")) {
	
	display_name = (char *) malloc(strlen(argv[i+1]) + 1);
	strcpy(display_name, argv[i+1]);
	
      } else if (!strcmp(argv[i], "-geometry") || !strcmp(argv[i], "-g")) {
	
	geometrystatus = XParseGeometry(argv[i+1], &mainx, &mainy,
					&mainwidth, &mainheight);
	if(!(geometrystatus & XValue))
	  mainx = MAINX;
	
	if(!(geometrystatus & YValue))
	  mainy = MAINY;
	
      }

    } /* if (i < argc - 1) */
    
  } /* i */

  /*
   * setup display and screen
   */

  if((rdisplay = XOpenDisplay(display_name)) == NULL) {
    fprintf(stderr, "ERROR - view_colormap\n");
    fprintf(stderr,
	    "Cannot open display '%s'\n", display_name);
    exit(1);
  }

  rscreen = DefaultScreen(rdisplay);

  /*
   * set xfonts
   */

  if((button_font = XLoadQueryFont(rdisplay, BUTTON_FONT)) == 0) {
    fprintf(stderr, "ERROR - view_colormap\n");
    fprintf(stderr, "Cannot find  font '%s'\n", BUTTON_FONT);
    exit(1);
  }

  if((label_font = XLoadQueryFont(rdisplay, LABEL_FONT)) == 0) {
    fprintf(stderr, "ERROR - view_colormap\n");
    fprintf(stderr, "Cannot find  font '%s'\n", LABEL_FONT);
    exit(1);
  }

  /*
   * get the default depth
   */

  default_depth = DefaultDepth(rdisplay, rscreen);

  /*
   * compute the size of the colormap array
   */

  ncolors = (long) (pow((double) 2.0, (double) default_depth) + 0.5);
  nside = (long) (sqrt((double) ncolors) + 0.5);
  
  /*
   * create the window
   */

  mainwidth = nside * RECTANGLE_SIZE;
  mainheight = nside * RECTANGLE_SIZE;

  hint.min_width = 50;
  hint.min_height = 50;
  hint.flags = PMinSize;

  main_window = XCreateSimpleWindow (rdisplay,
				     DefaultRootWindow(rdisplay),
				     mainx, mainy,
				     mainwidth, mainheight,
				     MAINBORDER,
				     WhitePixel(rdisplay, rscreen),
				     BlackPixel(rdisplay, rscreen));
    
  XSetStandardProperties(rdisplay, main_window,
			 "view_colormap", "view_colormap",
			 None, argv, argc, &hint);

  /*
   * solicit events
   */

  XSelectInput(rdisplay, main_window, ExposureMask);

  /*
   * map the window
   */

  XMapRaised (rdisplay, main_window);
  XFlush(rdisplay);

  /*
   * create the fill rectangle gc
   */

  fill_gc = XCreateGC(rdisplay, main_window, 0, 0);

  /*
   * fill rectangles with the color map cell colors
   */

  while (1) {

    for (irow = 0; irow < nside; irow++) {

      for (icol = 0; icol < nside; icol++) {

	icolor = (irow * nside + icol);
	y = irow * RECTANGLE_SIZE;

	XSetForeground(rdisplay, fill_gc, icolor);

	x = icol * RECTANGLE_SIZE;

	XFillRectangle(rdisplay, main_window, fill_gc, x, y,
		       RECTANGLE_SIZE, RECTANGLE_SIZE);

      } /* icol */

    } /* irow */

    sleep(UPDATE_INTERVAL);

  } /* while */

}

