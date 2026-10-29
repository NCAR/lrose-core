#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ndet_s_poll.c 1.3 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1991 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ndet_s_poll.c - Implement notify_set_fd_func call.
 */

#ifdef POLL
#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>

#include <poll.h>

extern          Notify_func
notify_set_fd_func(nclient, func, poll_fd)
    Notify_client   nclient;
    Notify_func     func;
    struct poll    *poll_fd;
{
    return (ndet_set_fd_func(nclient, func, poll_fd, NTFY_FD));
}
#endif POLL
