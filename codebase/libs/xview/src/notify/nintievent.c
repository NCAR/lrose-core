#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)nintievent.c 20.12 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Nint_i_event.c - Implement the notify_interpose_event_func interface.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>
#include <xview_private/nint.h>

extern          Notify_error
notify_interpose_event_func(nclient, func, when)
    Notify_client   nclient;
    Notify_func     func;
    Notify_event_type when;
{
    NTFY_TYPE       type;

    /* Check arguments */
    if (ndet_check_when(when, &type))
	return (notify_errno);
    return (nint_interpose_func(nclient, func, type, NTFY_DATA_NULL,
				NTFY_IGNORE_DATA));
}
