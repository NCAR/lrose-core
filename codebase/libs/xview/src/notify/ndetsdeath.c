#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ndetsdeath.c 20.12 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ndet_s_destroy.c - Implement the notify_set_destroy_func interface.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>
#include <xview_private/ndis.h>
#include <xview_private/nint.h>
#include <signal.h>

extern          Notify_func
notify_set_destroy_func(nclient, func)
    Notify_client   nclient;
    Notify_func     func;
{
    Notify_func     old_func = NOTIFY_FUNC_NULL;
    register NTFY_CLIENT *client;
    NTFY_CONDITION *condition;

    NTFY_BEGIN_CRITICAL;
    /* Find/create client that corresponds to nclient */
    if ((client = ntfy_new_nclient(&ndet_clients, nclient,
				   &ndet_client_latest)) == NTFY_CLIENT_NULL)
	goto Done;
    /* Find/create condition */
    if ((condition = ntfy_new_condition(&(client->conditions), NTFY_DESTROY,
	  &(client->condition_latest), NTFY_DATA_NULL, NTFY_IGNORE_DATA)) ==
	NTFY_CONDITION_NULL)
	goto Done;
    /* Exchange functions */
    old_func = nint_set_func(condition, func);
    /* Remove condition if func is null */
    if (func == NOTIFY_FUNC_NULL) {
	ndis_flush_condition(nclient, NTFY_DESTROY,
			     NTFY_DATA_NULL, NTFY_IGNORE_DATA);
	ntfy_unset_condition(&ndet_clients, client, condition,
			     &ndet_client_latest, NTFY_NDET);
    } else
	/* Make sure we are catching SIGTERM before return */
	ndet_enable_sig(SIGTERM);
    /*
     * Have notifier check for SIGTERM changes next time around loop. Will
     * notice and adjust signal handling at this time.
     */
    ndet_flags |= NDET_DESTROY_CHANGE;
Done:
    NTFY_END_CRITICAL;
    return (old_func);
}
