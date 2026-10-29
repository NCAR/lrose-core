#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)xv_init_x.c 20.27 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/xv_debug.h>
#include <stdio.h>		/* stderr & BUFSIZ */
#include <sys/types.h>
#include <xview/pkg.h>
#include <xview/xv_error.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <string.h>
#include <xview_private/i18n_impl.h>

Xv_private int (*xv_x_error_proc)();
Xv_private void (*xv_xlib_error_proc)();


Xv_private void
xv_set_default_font(display, screen, font)
    register Display   *display;
    int             	screen;
    Font             	font;
{
    GC              gc = DefaultGC(display, screen);

    XSetFont(display, gc, font);
}


#ifdef _XV_DEBUG
Xv_private void
xv_dump_fonts(display)
    Display        *display;
{
    char          **temp;
    int             font_count, i;

    temp = (char **) XListFonts(display, "*", 100, &font_count);
    (void) fprintf(stderr,
		   XV_MSG("There are %d fonts available, named:\n"), 
		   font_count);
    for (i = 0; i < font_count; i++) {
	(void) fprintf(stderr, "\t%s\n", temp[i]);
    }
    XFreeFontNames(temp);
}

#endif


Xv_private void
xv_x_error_handler(dpy, event)
    Display        *dpy;
    XErrorEvent    *event;
{
    int		    result;

    /* BUG: Watch out for the case where XView sets the focus to an unviewable
     * 	    window.
     */
    if ((event->error_code == BadMatch) &&
	(event->request_code == X_SetInputFocus))
	return;

    if (xv_x_error_proc)
	result = xv_x_error_proc(dpy, event);
    else
	result = XV_ERROR;
    if (result == XV_OK)
	/* Application's X Error Handler says to ignore this X error.
	 * Continue execution.
	 */
	return;

    if (xv_xlib_error_proc)
	/* Call default Xlib X Error Handler and abort program */
	xv_xlib_error_proc(dpy, event);  /* should exit(1) */
    else
	/* Using X11R3 */
	xv_error(XV_NULL,
	    ERROR_SERVER, event,
	    ERROR_SEVERITY, ERROR_NON_RECOVERABLE,
	    NULL);
}
