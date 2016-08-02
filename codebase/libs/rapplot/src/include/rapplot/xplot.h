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

/******************************************************************
 * xplot.h: header file for x utility routines
 ******************************************************************/

#ifndef xplot_h
#define xplot_h

#ifdef __cplusplus
extern "C" {
#endif

#include <toolsa/umisc.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

  /*
   * structure for x frame attributes - XREF
   */

  typedef struct {
    Display *display;
    Drawable drawable;
    unsigned int width, height;
    double xscale, yscale;
    GC gc;
    XFontStruct *font;
  } xref_t;

  /*
   * structures for color list
   */

  typedef struct x_color_list_s {
    GC gc;
    XColor x_color;
    char *colorname;
    int duplicate;
    struct x_color_list_s *next;
  } x_color_list_t;

  typedef struct {
    x_color_list_t *first_entry;
    long n_entries;
  } x_color_list_index_t;

  /*
   * function prototypes
   */

  extern GC xGetColorGC(Display *display,
			Colormap cmap,
			x_color_list_index_t *list_index,
			const char *colorname);

  extern XColor *xGetXColor(Display *display,
			    Colormap cmap,
			    x_color_list_index_t *list_index,
			    const char *colorname);

  extern XFontStruct *xLoadFont(Display *display,
				const char *fontname);

  extern char *xGetResString(Display *display,
			     const char *name,
			     char *res_string,
			     char *hard_def);

  extern double xGetResDouble(Display *display,
			      const char *name,
			      const char *res_string,
			      double hard_def);

  extern float xGetResFloat(Display *display,
			    const char *name,
			    const char *res_string,
			    float hard_def);

  extern long xGetResLong(Display *display,
			  const char *name,
			  const char *res_string,
			  long hard_def);

  extern void xFreeColorList(Display *display,
			     Colormap cmap,
			     x_color_list_index_t *list_index);

  extern void xFreeFont(Display *display,
			XFontStruct *font);

  extern void xFreeGC(Display *display,
		      GC gc);


#ifdef __cplusplus
}
#endif

#endif
