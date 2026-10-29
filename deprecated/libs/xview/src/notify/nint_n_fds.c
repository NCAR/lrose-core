#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)nint_n_fds.c 1.3 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1991 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Nint_n_fds.c - Implement the notify_next_fd_func interface.
 */

#ifdef POLL

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>
#include <xview_private/nint.h>

extern          Notify_value
notify_next_fd_func(nclient, poll_fd)
    Notify_client   nclient;
    struct poll	   *poll_fd;
{
    return (nint_next_fds_func(nclient, NTFY_FD, poll_fd));
}

#endif /* POLL */
