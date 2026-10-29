#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ndet_g_fds.c 1.3 93/06/28 Copyr 1985 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1991 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ndet_g_fds.c - Implement notify_get_fd_func call.
 */

#ifdef POLL

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>

extern          Notify_func
notify_get_fd_func(nclient, fd)
    Notify_client   nclient;
    int             fd;
{
    return (ndet_get_fd_func(nclient, fd, NTFY_FD));
}

#endif /* POLL */
