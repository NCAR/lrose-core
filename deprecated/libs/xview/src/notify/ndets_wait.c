#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ndets_wait.c 20.14 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ndet_s_wait.c - Implement the notify_set_wait3_func interface.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>
#include <xview_private/ndis.h>
#include <xview_private/nint.h>
#include <signal.h>

extern          Notify_func
notify_set_wait3_func(nclient, func, pid)
    Notify_client   nclient;
    Notify_func     func;
    int             pid;
{
    Notify_func     old_func = NOTIFY_FUNC_NULL;
    register NTFY_CLIENT *client;
    NTFY_CONDITION *condition;

    NTFY_BEGIN_CRITICAL;
    /*
     * Don't check pid so that we can avoid the race condition of pid having
     * exited before calling this routine.  It still needs to be reaped.
     */
    /* Find/create client that corresponds to nclient */
    if ((client = ntfy_new_nclient(&ndet_clients, nclient,
				   &ndet_client_latest)) == NTFY_CLIENT_NULL)
	goto Done;
    /* Find/create condition */
    if ((condition = ntfy_new_condition(&(client->conditions), NTFY_WAIT3,
	    &(client->condition_latest), (NTFY_DATA) pid, NTFY_USE_DATA)) ==
	NTFY_CONDITION_NULL)
	goto Done;
    ntfy_add_to_table(client, condition, NTFY_WAIT3);
    /* Exchange functions */
    old_func = nint_set_func(condition, func);
    /* Remove condition if func is null */
    if (func == NOTIFY_FUNC_NULL) {
	/* Remove from dispatcher */
	ndis_flush_wait3(nclient, pid);
	ntfy_unset_condition(&ndet_clients, client, condition,
			     &ndet_client_latest, NTFY_NDET);
    } else
	/* Make sure we are catching SIGCHLD before return */
	ndet_enable_sig(SIGCHLD);
    /*
     * Have notifier check for SIGCHLD changes next time around loop. Will
     * confirm signal handling at this time.
     */
    ndet_flags |= NDET_WAIT3_CHANGE;
    /*
     * Fake a SIGCHLD so that we do a wait3 in the case of pid having exited
     * before calling or during this routine. It still needs to be reaped.
     */
    if (func != NOTIFY_FUNC_NULL)
	sigaddset( &ndet_sigs_received, SIGCHLD );
Done:
    NTFY_END_CRITICAL;
    return (old_func);
}
