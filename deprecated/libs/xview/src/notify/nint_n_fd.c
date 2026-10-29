#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)nint_n_fd.c 20.12 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Nint_n_fd.c - Implement the nint_next_fd_func interface.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>
#include <xview_private/nint.h>

pkg_private     Notify_value
nint_next_fd_func(nclient, type, fd)
    Notify_client   nclient;
    NTFY_TYPE       type;
    int             fd;
{
    Notify_func     func;

    /* Check arguments */
    if (ndet_check_fd(fd))
	return (NOTIFY_UNEXPECTED);
    if ((func = nint_next_callout(nclient, type)) == NOTIFY_FUNC_NULL)
	return (NOTIFY_UNEXPECTED);
    return (func(nclient, fd));
}
