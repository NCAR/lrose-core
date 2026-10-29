#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ntfy_reset.c 20.11 93/06/28 Copyr 1986 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ntfy_reset.c - Notify_reset implementation.
 */

#include <view2_private/ntfy.h>
#include <view2_private/ndet.h>
#include <view2_private/ndis.h>
#include <view2_private/nint.h>

extern void
notify_reset(level)
    int             level;
{
    register NTFY_CLIENT *ntfy_client;
    extern void     ntfy_reset_paranoid();

    /* Reset common globals */
    ntfy_sigs_blocked = 0;
    ntfy_interrupts = 0;
    /* Reset variables used in paranoid enumerator */
    ntfy_reset_paranoid();

    /* Reset detector */
    for (ntfy_client = ndet_clients; ntfy_client;
	 ntfy_client = ntfy_client->next)
	/* Reset all detector client's event processing flag */
	ntfy_client->flags &= ~NCLT_EVENT_PROCESSING;
    ndet_flags = NDET_CONDITION_CHANGE;

    /* Reset dispatcher */
    while (ndis_clients)
	notify_flush_pending(ndis_clients->nclient);
    ndis_flags = 0;

    /* Reset interposer */
    nint_stack_next = 0;
}
