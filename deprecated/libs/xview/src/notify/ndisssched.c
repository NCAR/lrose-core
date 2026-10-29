#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ndisssched.c 20.12 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ndis_s_sched.c - Implement the notify_set_sheduler_func.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndis.h>

extern          Notify_func
notify_set_scheduler_func(scheduler_func)
    Notify_func     scheduler_func;
{
    register Notify_func old_func;

    NTFY_BEGIN_CRITICAL;
    old_func = ndis_scheduler;
    ndis_scheduler = scheduler_func;
    if (ndis_scheduler == NOTIFY_FUNC_NULL)
	ndis_scheduler = ndis_default_scheduler;
    NTFY_END_CRITICAL;
    return (old_func);
}
