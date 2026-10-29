#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ndetremove.c 20.13 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ndet_remove.c - Notify_remove implementation.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>
#include <xview_private/ndis.h>

extern          Notify_error
notify_remove(nclient)
    Notify_client   nclient;
{
    NTFY_CLIENT    *client;

    NTFY_BEGIN_CRITICAL;
    /* Flush any pending notification from dispatcher */
    notify_flush_pending(nclient);
    if ((client = ntfy_find_nclient(ndet_clients, nclient,
				&ndet_client_latest)) != NTFY_CLIENT_NULL) {
	ntfy_remove_client(&ndet_clients, client, &ndet_client_latest,
			   NTFY_NDET);
	/*
	 * Have all conditions reevaluated next time around the loop. This is
	 * a little heavy handed but presumably notify_remove is not called
	 * that often.  An alternative to this is to have this routine loop
	 * over all the conditions and make notify_set_*_func(nclient,
	 * NOTIFY_FUNC_NULL, ...) calls.
	 */
	ndet_flags |= NDET_CONDITION_CHANGE;
    }
    NTFY_END_CRITICAL;
    return (NOTIFY_OK);
}
