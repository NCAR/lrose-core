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
#ifndef XUTILS_H
#define XUTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <errno.h>


#include <X11/Xlib.h>

#include <rapplot/xrs.h>
#include <rapplot/xudr.h>
#include <rapplot/rascon.h>

/*********************************************************************
 * XUTIL_dump_pixmap_xwd() : Dump an X11 pixmap to an xwd file.
 *
 * The XWindowAttributes argument can be retrieved from the X
 * server using the following calls on the associated window:
 * 
 *   XWindowAttributes attrib;
 *   XGetWindowAttributes(dpy, window, &attrib);
 */

extern void XUTIL_dump_pixmap_xwd(Display *dpy, Pixmap p, 
				  XWindowAttributes *win_info, FILE *fp);

/*********************************************************************
 * Window_Dump: dump a window to a file which must already be open for
 *              writing.
 */

/*
 * Dump a X11 drawable.
 * The XWindowAttributes argument is optional if the Drawable is
 * a Window, but required if the Drawable is a Pixmap.
 * The reason we need this argument is that the XGetWindowAttributes
 * call does not seem to work on Pixmaps in X11R6. If you do 
 * need to dump a backing store pixmap (which would be the case about 
 * 99% of the time), you should call XGetWindowAttributes on its 
 * associated Window before calling this function, like this:
 * 
 *   XWindowAttributes attrib;
 *   XGetWindowAttributes( dpy, window, &attrib);
 *
 *   XwuDumpWindow( dpy, pixmap, &attrib, stdout);
 *
 */

extern void
  XwuDumpWindow( Display * dpy, Drawable window,
		 XWindowAttributes * win_info, FILE *  out);

#ifdef __cplusplus
}
#endif



#endif
