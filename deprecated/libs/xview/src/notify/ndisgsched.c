#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ndisgsched.c 20.12 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ndis_g_sched.c - Implement the notify_get_sheduler_func.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndis.h>

extern          Notify_func
notify_get_scheduler_func()
{
    return (ndis_scheduler);
}
