#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ninti_wait.c 20.12 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Nint_i_wait.c - Implement the notify_interpose_wait3_func interface.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>
#include <xview_private/nint.h>

extern          Notify_error
notify_interpose_wait3_func(nclient, func, pid)
    Notify_client   nclient;
    Notify_func     func;
    int             pid;
{
    /* Check arguments */
    if (ndet_check_pid(pid))
	return (notify_errno);
    return (nint_interpose_func(nclient, func, NTFY_WAIT3,
				(NTFY_DATA) pid, NTFY_USE_DATA));
}
