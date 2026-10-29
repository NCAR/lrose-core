#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)nintnevent.c 20.12 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Nint_n_event.c - Implement the notify_next_event_func interface.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>
#include <xview_private/nint.h>

extern          Notify_value
notify_next_event_func(nclient, event, arg, when)
    Notify_client   nclient;
    Notify_event    event;
    Notify_arg      arg;
    Notify_event_type when;
{
    Notify_func     func;
    NTFY_TYPE       type;

    /* Check arguments */
    if (ndet_check_when(when, &type))
	return (NOTIFY_UNEXPECTED);
    if ((func = nint_next_callout(nclient, type)) == NOTIFY_FUNC_NULL)
	return (NOTIFY_UNEXPECTED);
    return (func(nclient, event, arg, when));
}
