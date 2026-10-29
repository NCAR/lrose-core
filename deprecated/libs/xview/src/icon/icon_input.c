#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)icon_input.c 20.18 93/06/28 Copyr 1984 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <stdio.h>
#include <xview/xview.h>

/* ARGSUSED */
Notify_value
icon_input(icon_public, event, arg, type)
    Icon            icon_public;
    Event          *event;
    Notify_arg      arg;
    Notify_event_type type;
{
    switch (event_action(event)) {

      case WIN_REPAINT:
	    icon_display(icon_public, 0, 0);
	    return (NOTIFY_DONE);
	    break;

      default:
	return (NOTIFY_IGNORED);
	break;
    }
}
