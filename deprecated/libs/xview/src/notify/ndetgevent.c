#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ndetgevent.c 20.12 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ndet_g_event.c - Implement the notify_get_event_func interface.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>

extern          Notify_func
notify_get_event_func(nclient, when)
    Notify_client   nclient;
    Notify_event_type when;
{
    NTFY_TYPE       type;

    /* Check arguments */
    if (ndet_check_when(when, &type))
	return (NOTIFY_FUNC_NULL);
    return (ndet_get_func(nclient, type, NTFY_DATA_NULL, NTFY_IGNORE_DATA));
}
