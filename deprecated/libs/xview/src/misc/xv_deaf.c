#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)xv_deaf.c 1.13 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * xv_deaf.c
 *
 * int
 * xv_deaf(window, on)
 * Xv_Window	window;
 * Bool		on;
 * 
 * If 'on' is True, makes 'window' and all it's children deaf i.e.
 * ignores these events. See libxview/win/win_input.c
 * KeyPress
 * KeyRelease
 * ButtonPress
 * ButtonRelease
 * MotionNotify
 *
 * Returns XV_OK if success, XV_ERROR otherwise.
 *
 */
#include <stdio.h>
#include <X11/Xlib.h>
#include <xview_private/i18n_impl.h>
#include <xview_private/windowimpl.h>
#include <xview_private/draw_impl.h>
#include <xview/server.h>
#include <xview/screen.h>
#include <xview/window.h>
#include <xview/cursor.h>

/*
 * xv_deaf(parent, on)
 * makes the object 'parent' and it's descendants deaf if
 * 'on' is True. Otherwise, it restores their original input
 * state.
 */
xv_deaf(parent, on)
Xv_window	parent;
Bool		on;
{
    Xv_server   	server;
    Xv_screen   	screen;
    Xv_cursor		busyPointer = XV_ZERO;

    screen = xv_get(parent, XV_SCREEN);
    server = xv_get(screen, SCREEN_SERVER);

    /*
     * Check if busy pointer has been created on server
     * already
     */
    busyPointer = xv_get(server, XV_KEY_DATA, CURSOR_BUSY_PTR);

    /*
     * If it hasn't, create and store it on server as key data
     */
    if (!busyPointer) {
        busyPointer = xv_create(screen, CURSOR, 
				CURSOR_SRC_CHAR, OLC_BUSY_PTR, 
                                CURSOR_MASK_CHAR, OLC_BUSY_MASK_PTR,
                                NULL);
        xv_set(server, XV_KEY_DATA, CURSOR_BUSY_PTR, busyPointer, NULL);
    }

    if (window_set_tree_flag(parent, busyPointer, TRUE, on) != XV_OK)  {
        xv_error(parent,
                ERROR_STRING, 
                XV_MSG("xv_deaf:attempt to make windows deaf/hear failed"), 
                NULL);
        return(XV_ERROR);
    }

    return(XV_OK);
}

