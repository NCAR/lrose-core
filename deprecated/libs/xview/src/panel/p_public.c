#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_public.c 20.14 93/06/28 Copyr 1984 Sun Micro";
#endif
#endif

/*****************************************************************************/
/* panel_public.c                                  */
/*	
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license. 
 */
/*****************************************************************************/

#include <xview_private/panel_impl.h>

/* utilities for event location translation */

/*
 * translate a panel-space event to a window-space event.
 */
Sv1_public Event *
panel_window_event(client_panel, event)
    Panel           client_panel;
    register Event *event;
{

    canvas_window_event(client_panel, event);
    return event;
}


/*
 * translate a window-space event to a panel-space event.
 */
Sv1_public Event *
panel_event(client_panel, event)
    Panel           client_panel;
    register Event *event;
{
    canvas_event(client_panel, event);
    return event;
}
