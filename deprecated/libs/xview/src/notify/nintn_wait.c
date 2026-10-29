#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)nintn_wait.c 20.13 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Nint_n_wait.c - Implement the notify_next_wait3_func interface.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>
#include <xview_private/nint.h>

extern          Notify_value
notify_next_wait3_func(nclient, pid, status, rusage)
    Notify_client   nclient;
    int             pid;
#ifdef SYSV_WAIT
    int *status;
#else /* SYSV_WAIT */
    union_wait_t *status;
#endif /* SYSV_WAIT */
    struct rusage  *rusage;
{
    Notify_func     func;

    /* Don't check pid because may be exiting */
    if ((func = nint_next_callout(nclient, NTFY_WAIT3)) == NOTIFY_FUNC_NULL)
	return (NOTIFY_UNEXPECTED);
    return (func(nclient, pid, status, rusage));
}
