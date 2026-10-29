#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_view.c 20.31 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#include <xview_private/panel_impl.h>
#include <xview/defaults.h>
#include <xview/notify.h>


Pkg_private int panel_view_init();


/*ARGSUSED*/
Pkg_private int
panel_view_init(parent, view_public, avlist)
    Panel           parent;
    Panel_view      view_public;
    Attr_attribute  avlist[];
{
    Xv_Window       pw;
    Xv_Screen       screen;

    if (view_public) {
	/* Scrollable Panel: register the new paint window */
	pw = (Xv_Window) xv_get(view_public, CANVAS_VIEW_PAINT_WINDOW);
    } else
	pw = parent;
    screen = (Xv_Screen) xv_get(pw, XV_SCREEN);
    if (pw != XV_ZERO) {
	(void) xv_set(pw,
		      WIN_RETAINED,
		      ((int) xv_get(screen, SCREEN_RETAIN_WINDOWS)),
		      WIN_NOTIFY_SAFE_EVENT_PROC, panel_notify_event,
		      WIN_NOTIFY_IMMEDIATE_EVENT_PROC, panel_notify_event,
		      WIN_CONSUME_EVENTS,
			  WIN_UP_EVENTS, WIN_ASCII_EVENTS,
			  KBD_USE, LOC_DRAG,
			  WIN_MOUSE_BUTTONS,
			  ACTION_RESCALE,
			  ACTION_OPEN, ACTION_FRONT,
			  ACTION_CUT, ACTION_COPY, ACTION_PASTE,
			  ACTION_SELECT_FIELD_FORWARD, ACTION_FIND_FORWARD,
	/* BUG: enable IM_ISO if ecd_input enabled */
			  ACTION_HELP,
			  WIN_EDIT_KEYS,
			  KEY_RIGHT(8), KEY_RIGHT(10), KEY_RIGHT(12),
			  KEY_RIGHT(14),
			  0,
		      NULL);
	return (XV_OK);
    } else {
	return (XV_ERROR);
    }
}
