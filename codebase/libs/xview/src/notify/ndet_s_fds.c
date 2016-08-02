#ifndef	lint
#ifdef sccs
static char     sccsid[] = "@(#)ndet_s_fds.c 1.3 93/06/28 Copyr 1991 Sun Micro";
#endif
#endif

/*
 *	(c) Copyright 1991 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Ndet_s_fds.c - Implement notify_set_fd_func call.
 */

#include <xview_private/ntfy.h>
#include <xview_private/ndet.h>

extern          Notify_func
notify_set_input_func(nclient, func, fd)
    Notify_client   nclient;
    Notify_func     func;
    struct poll     fd;
{
    return (ndet_set_poll_func(nclient, func, fd, NTFY_FD));
}
